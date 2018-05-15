//+----------------------------------------------------------------------------+
//|EQFMEMMP.C                                                                  |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|      Copyright (C) 1990-2015, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+
//|Author: Markus Conrad                                                       |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//| Translation Memory Merge                                                   |
//|                                                                            |
//| The function EQFMemMergeStart initialize the translation                                                                            |
//| memory merge process.                                                                                                               |
//| The function EQFMemMergeProcess extraxts a segment from                                                                             |
//| a translation memory to be merged                                                                                                   |
//| and adds it to the translation memory.                                                                                              |
//| The function EQFMemMergeEnd terminates the merge process                                                                            |

#define INCL_EQF_ANALYSIS         // analysis functions
#define INCL_EQF_TM               // general Transl. Memory functions
#define INCL_EQF_TMREMOTE         // remote Transl. Memory functions (U-Code)
#define INCL_EQF_DLGUTILS         // dialog utilities

#include "core\PluginManager\PluginManager.h"
#include "core\PluginManager\OtmMemoryPlugin.h"
#include "core\PluginManager\OtmMemory.h"
#include "MemoryFactory.h"

#define INCL_EQFMEM_DLGIDAS       // include dialog IDA definitions
#include <EQFTMI.H>               // Private header file of Translation Memory
#include <EQFMEM.ID>              // Translation Memory IDs

#define MEM_START_MERGE     USER_TASK + 1
#define MEM_MERGE_TASK      USER_TASK + 2
#define MEM_END_MERGE       USER_TASK + 3

extern HELPSUBTABLE pid_memmerge_subtable[];

USHORT MemFuncMergeStart( PMEM_MERGE_IDA pMIDA );
VOID MemFuncMergeProcess( PMEM_MERGE_IDA pMIDA);
VOID MemFuncMergeEnd( PMEM_MERGE_IDA pMIDA );



INT_PTR CALLBACK MEMMERGEDLG( HWND hwnd,
                              WINMSG msg,
                              WPARAM mp1,
                              LPARAM mp2 )
{
   /* This function fills the TM handle pMIDA->htm if a translation memory
      has been selected as a target for the translation memory to be merged
      If no TM is selected then or the dialog has been canceled then
      pMIDA->htm will be set to NULL. */

   PMEM_MERGE_IDA   pMIDA;                           // Pointer to merge IDA
   USHORT           usRc = TRUE;                     // Process return code
   MRESULT          mResult = (MRESULT)FALSE;        // Function return code
   CHAR             szObjName[sizeof(MEMORY_ALL)+1]; // String to hold "MEMORY_ALL"
   SHORT            sSelectItem = 0;                 // Index of item
   USHORT           usTmRc;                          // TM return code
   BOOL             fOk = TRUE;
   PSZ              pszReplace[4];           // pointer arry for replace strings
   USHORT           usResponse;              // return from UtlError

   switch ( msg )
   {
      //---------------------------------------------------------------------
      case WM_EQF_QUERYID: HANDLEQUERYID( ID_MEMMERGE_DLG, mp2 ); break;


     case WM_HELP:
       /*************************************************************/
       /* pass on a HELP_WM_HELP request                            */
       /*************************************************************/
        EqfDisplayContextHelp( (HWND)((LPHELPINFO) mp2)->hItemHandle,
                               &pid_memmerge_subtable[0] );
        mResult = TRUE;  // message processed
        break;


      // Initialize and display the dialogbox
      case WM_INITDLG:

        // Address the merge IDA
        pMIDA = (PMEM_MERGE_IDA)PVOIDFROMMP2( mp2 );

        // Assign pMIDA to the window pointer
        if ( ANCHORDLGIDA( hwnd, pMIDA ) == FALSE )
        {
          // Issue the system error message
          UtlError( 0, MB_CANCEL, 0, NULL, SYSTEM_ERROR );
          usRc = FALSE;
        } /* endif */

        // Prepare the dialog box
        if (usRc)
        {
          // Save the TM listbox handle in the IDA
          pMIDA->hwndMemLb = GETHANDLEFROMID( hwnd, DID_MEMMERGE_TO );

          SETCOLOR( hwnd, DID_MEMMERGE_FROM, ulColor );

          // Display the name of the translation memory to be merged
          OEMTOANSI( pMIDA->szNameMergeMem );
          SETTEXT( hwnd, DID_MEMMERGE_FROM, pMIDA->szNameMergeMem );
          ANSITOOEM( pMIDA->szNameMergeMem );

          // Load the translation memory listbox with all TMs
          strcpy( szObjName, MEMORY_ALL );
          EqfSend2Handler( MEMORYHANDLER, WM_EQF_INSERTNAMES,
                           MP1FROMHWND( pMIDA->hwndMemLb ),   // List box handle
                           MP2FROMP( szObjName ));            // Object name
          UtlSetHorzScrollingForLB( pMIDA->hwndMemLb );

          // Remove the TM to be merged from the list box
          OEMTOANSI( pMIDA->szNameMergeMem );
          sSelectItem = SEARCHITEMHWND( pMIDA->hwndMemLb, pMIDA->szNameMergeMem );
          ANSITOOEM( pMIDA->szNameMergeMem );
          if ( sSelectItem != LIT_NONE && sSelectItem != LIT_ERROR )
          {
            DELETEITEMHWND( pMIDA->hwndMemLb, sSelectItem );
          } /* endif */
        } /* endif */
        break;

  case WM_COMMAND:
        switch ( WMCOMMANDID( mp1, mp2 ) )             // Switch on control ID
        {
		  case PID_PB_HELP:
			mResult = UtlInvokeHelp();
			break;
          case PID_PB_CANCEL:       // CANCEL button selected
          case DID_CANCEL:          // ESC key pressed
            // Get access to the IDA
            pMIDA = ACCESSDLGIDA(hwnd, PMEM_MERGE_IDA);
            if ( pMIDA->pMergeMem != NULL )
            {
              MemoryFactory *pFactory = MemoryFactory::getInstance();
              pFactory->closeMemory( pMIDA->pMergeMem );
              pMIDA->pMergeMem = NULL;
            } /* endif */
            POSTEQFCLOSE( hwnd, FALSE );
            break;

          case PID_PB_OK:
            // Get access to the IDA
            pMIDA = ACCESSDLGIDA(hwnd, PMEM_MERGE_IDA);

            // Disable the Merge and Cancel button
            EnableOKAndCANCELButton( hwnd, FALSE );

            // Query text from translation memory listbox
            sSelectItem = QUERYSELECTIONHWND( pMIDA->hwndMemLb );

            if ( sSelectItem != LIT_NONE )
            {
              int iRC = 0;

              QUERYITEMTEXTHWND( pMIDA->hwndMemLb, sSelectItem, pMIDA->szNameMem );
              ANSITOOEM( pMIDA->szNameMem );

              // open the memory
              MemoryFactory *pFactory = MemoryFactory::getInstance();
              pMIDA->pOutputMem = pFactory->openMemory( NULL, pMIDA->szNameMem, NONEXCLUSIVE, &iRC );
              if ( pMIDA->pOutputMem == NULL )
              {
                fOk = FALSE;
                pFactory->showLastError( NULL, pMIDA->szNameMem, NULL, hwnd );
              }

              if ( fOk )
              {
                // get first markup tables
                pMIDA->pOutputMem->getMarkupName( 0, pMIDA->szTargetMemMarkup, sizeof(pMIDA->szTargetMemMarkup) );
                pMIDA->pMergeMem->getMarkupName( 0, pMIDA->szTargetMemSourceLang, sizeof(pMIDA->szTargetMemSourceLang) );

                // compare source languages of the memories
                pMIDA->pOutputMem->getSourceLanguage( pMIDA->szTargetMemSourceLang, sizeof(pMIDA->szTargetMemSourceLang) );
                pMIDA->pMergeMem->getSourceLanguage( pMIDA->szSourceMemSourceLang, sizeof(pMIDA->szSourceMemSourceLang) );

                if ( strcmp( pMIDA->szSourceMemSourceLang, pMIDA->szTargetMemSourceLang ) )
                {
                  //--- display warning message that source languages are not identical and query for continue
                  pszReplace[0] = pMIDA->szNameMergeMem; //source TM
                  pszReplace[1] = pMIDA->szNameMem;      //target TM
                  pszReplace[2] = pMIDA->szSourceMemSourceLang;
                  pszReplace[3] = pMIDA->szTargetMemSourceLang;

                  usResponse = UtlError(  ERROR_MEM_DIFFERENT_SOURCE_LANG, MB_YESNO | MB_DEFBUTTON2, 4, pszReplace, EQF_QUERY );
                  if ( usResponse == MBID_NO ) //--- do not continue with merge
                  {
                    //--- stop further processing keep dialog on screen
                    fOk = FALSE;
                  } /* endif */
                } /* endif */
              } /* endif */


              if ( fOk )
              {
                // The TM was opened successfully, close the dialog by issuing a close message
                POSTEQFCLOSE( hwnd, FALSE );
              }
              else
              {
                pFactory->closeMemory( pMIDA->pOutputMem );
                pMIDA->pOutputMem = NULL;
              } /* endif */
            }
            else
            {
              // Issue the message "No memory database has been selected"
              UtlError( WARNING_MEM_NOT_SELECTED, MB_CANCEL, 0, NULL, EQF_ERROR );
            } /* endif */

            // Enable the Merge and Cancel button if the dialog is not ready to be closed
            if ( pMIDA->pOutputMem == NULL )
            {
              EnableOKAndCANCELButton( hwnd, TRUE );
            } /* endif */

            break;
        } /*end switch*/
        break;

      case WM_EQF_CLOSE:
        // Distroy dialog
        WinDismissDlg( hwnd, 0 );
        break;
      //------------------------------------------------------------------------
      default :
        // Return to default dialog procedure
        mResult = WinDefDlgProc( hwnd, msg, mp1, mp2 );
        break;
   } /*end switch*/

   if ( !usRc )
   {
     POSTEQFCLOSE( hwnd, FALSE );
   } /* endif */
   return mResult;
}/* end of function MemMergeDlg */


/* !brief function handling the GUI mode of the memory merge */
BOOL MemGUIMergeMemory
( 
  OtmMemory *pMemory, 
  PLISTCOMMAREA pCommArea,
  PSZ pszMemName, 
  PMEM_IDA      pIDA,                      // Pointer to the Main Memory data area
  HWND hwnd
)
{
  BOOL fOK = TRUE;
  PMEM_MERGE_IDA    pMIDA = NULL;         // Pointer to the export IDA
  SHORT sRC = 0;

  // Allocate storage for the MEM_MERGE_IDA.
  fOK = UtlAlloc( (PVOID *)&pMIDA, 0L, (LONG)sizeof(MEM_MERGE_IDA), ERROR_STORAGE );
  if ( fOK )
  {
    int   iDlgRc = 1;
    HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);

    // Fill  MEM_MERGE_IDA 
    pMIDA->pMergeMem = pMemory;
    strcpy( pMIDA->szNameMergeMem, pszMemName );
    pMIDA->fDisplayMsg = TRUE;               

    DIALOGBOX( QUERYACTIVEWINDOW(), MEMMERGEDLG, hResMod, ID_MEMMERGE_DLG, pMIDA, iDlgRc );

    if ( iDlgRc || !WinIsWindow( (HAB)NULL, hwnd))  // WM_DESTROYed ?
    {
      fOK = FALSE;
    } /* endif */

    // If TM handle (output TM) is not set then the dialog failed and usRc is set to FALSE
    if ( !pMIDA->pOutputMem )
    {
      fOK = FALSE;
    } /* endif */
  } /* endif */

  // If usRc post a  WM_EQF_MEMLOAD_START message else clean up allocated storage.
  if ( fOK )
  {
    // build object name
    strcpy( pCommArea->szBuffer, MEMMERGE_PROC_OBJPREFIX );
    strcat( pCommArea->szBuffer, pIDA->szMemName );
    sRC = QUERYSYMBOL( pCommArea->szBuffer );
    if ( sRC == -1 )
    {
        // memory is not in use ==> lock this memory
        SETSYMBOL( pCommArea->szBuffer );
        fOK = CreateProcessWindow( pCommArea->szBuffer, MemMergeCallBack, pMIDA );
    }
    else
    {
      PSZ pTemp = pIDA->szMemName;

      UtlError( ERROR_MEM_NOT_ACCESSIBLE, MB_CANCEL, 1, &pTemp, EQF_ERROR );
      fOK = FALSE;
    } /* endif */

    if ( !fOK )
    {
      /* free IDA, otherwise it will be freed in CallBack */
      UtlAlloc( (PVOID *)&pMIDA, 0L, 0L, NOMSG );
    } /* endif */
  }
  else
  {
    // Free the MEM_MERGE_IDA area
    UtlAlloc( (PVOID *)&pMIDA, 0L, 0L, NOMSG);
  } /* endif */
  return( fOK );
}


/*********************************************/
/* Callback function for Merge ProcessWindow */
/*********************************************/

MRESULT MemMergeCallBack
(
  PPROCESSCOMMAREA pCommArea,          // ptr to commmunication area
  HWND             hwnd,               // handle of process window
  WINMSG           message,            // message to be processed
  WPARAM           mp1,                // first message parameter
  LPARAM           mp2                 // second message parameter
)
{
  PMEM_MERGE_IDA    pMIDA;             // pointer to instance area
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
        pMIDA               = (PMEM_MERGE_IDA)PVOIDFROMMP2(mp2);
        pCommArea->pUserIDA = pMIDA;
        HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);

        /****************************************************************/
        /* supply all information required to create the process        */
        /* window                                                       */
        /****************************************************************/
        pCommArea->sProcessWindowID = ID_MEMMERGE_WINDOW;
        pCommArea->sProcessObjClass = clsMEMORYMERGE;
        pCommArea->Style            = PROCWIN_TEXTSLIDER;
        pCommArea->sSliderID        = ID_MEMMERGE_SLIDER;
        pCommArea->sTextID          = ID_MEMMERGE_TEXT;
        LOADSTRING( NULLHANDLE, hResMod, MEM_METITLE, pCommArea->szBuffer );
        OEMTOANSI( pMIDA->szNameMergeMem );
        sprintf( pCommArea->szTitle, "%s %s", pCommArea->szBuffer,
                 pMIDA->szNameMergeMem );
        ANSITOOEM( pMIDA->szNameMergeMem );
        LOADSTRING( NULLHANDLE, hResMod, MEM_MELINE2, pCommArea->szBuffer );
        OEMTOANSI( pMIDA->szNameMem );
        sprintf( pCommArea->szText, "%s %s", pCommArea->szBuffer,
                 pMIDA->szNameMem );
        ANSITOOEM( pMIDA->szNameMem );
        pCommArea->hIcon            = (HPOINTER) UtlQueryULong(QL_TMEMMERGEICON); //hiconTMEMMERGE;
        pCommArea->fNoClose         = FALSE;
        pCommArea->swpSizePos.x     = 100;
        pCommArea->swpSizePos.y     = 100;
        pCommArea->swpSizePos.cx    = (SHORT) UtlQueryULong( QL_AVECHARWIDTH ) * 60;
        pCommArea->swpSizePos.cy    = (SHORT) UtlQueryULong( QL_PELSPERLINE ) * 10;
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
                  MP1FROMSHORT( MEM_START_MERGE ), 0L );
      break;

    /******************************************************************/
    /* WM_CLOSE:                                                      */
    /*                                                                */
    /* Prepare/initialize shutdown of process                         */
    /******************************************************************/
    case WM_CLOSE:
      pMIDA = (PMEM_MERGE_IDA)pCommArea->pUserIDA;
      if ( pMIDA->pMergeMem != NULL )
      {
         EQFMemMergeEnd( pCommArea, hwnd, MP2FROMSHORT(1) );
      } /* endif */
      mResult = MRFROMSHORT( TRUE );  // = stop close process
      break;

    /******************************************************************/
    /* WM_DESTROY:                                                    */
    /*                                                                */
    /* Cleanup all resources used by the process                      */
    /******************************************************************/
    case WM_DESTROY:
      pMIDA = (PMEM_MERGE_IDA)pCommArea->pUserIDA;

      if ( pMIDA )
      {
        if ( pMIDA->pMergeMem != NULL )
        {
          EQFMemMergeEnd( pCommArea, hwnd, MP2FROMSHORT(1) );
        } /* endif */
        if ( pCommArea->pUserIDA != NULL )
        {
          UtlAlloc( (PVOID *) &pMIDA, 0L, 0L, NOMSG );
          pCommArea->pUserIDA = NULL;
        } /* endif */
      } /* endif */

      REMOVESYMBOL( pCommArea->szObjName );
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
        case MEM_START_MERGE:
          EQFMemMergeStart( pCommArea, hwnd );
          break;

        case MEM_MERGE_TASK:
          EQFMemMergeProcess( pCommArea, hwnd );
          break;

        case MEM_END_MERGE:
          EQFMemMergeEnd( pCommArea, hwnd, mp2 );
          break;
      } /* endswitch */
      break;
  } /* endswitch */
  return( mResult );
} /* end of function MemMergeCallBack */


// ================ Handle the message WM_EQF_MEMMERGE_START =================

  VOID EQFMemMergeStart
  (
  PPROCESSCOMMAREA  pCommArea,
  HWND              hWnd
  )
  {
    PMEM_MERGE_IDA    pMIDA;          // Pointer to the merge IDA
    USHORT            usRc = TRUE;    // Return code to control the process
    USHORT            usTmRc;         // Return code of Tm functions
    PSZ               pReplAddr[3];   // Pointer to an address list of replacement strings
    SERVERNAME        szServer;       // Server name of the TM into which is merged
    LONG              lBytesShort;    // Number of bytes short on the disk
    PSZ               pszTemp;        // Temporary character pointer
    CHAR              szNumber[10];   // Working field to display numbers
    USHORT            usResponse = MBID_RETRY;   // Response from message box
    USHORT            usDummy;                                         /*@RCA*/

    // Get address of MEM_MERGE_IDA
    pMIDA = (PMEM_MERGE_IDA)pCommArea->pUserIDA;
    pMIDA->fFirstExtract = TRUE;
    pMIDA->pProposal = new OtmProposal;

    // Get current time stamp and save it in the MIDA
    UtlTime( &(pMIDA->tStamp));

    // Move process-ID into mp1 and post the message WM_EQF_MEMMERGE_PROCESS
    WinPostMsg( hWnd, WM_EQF_PROCESSTASK, MP1FROMSHORT( MEM_MERGE_TASK ), NULL);
  } /* end of function EQFMemMergeStart */

// ================ Handle the message WM_EQF_MEMMERGE_PROCESS =====================

  VOID EQFMemMergeProcess
  (
  PPROCESSCOMMAREA    pCommArea,
  HWND                hWnd
  )
  {
    USHORT            usRc = TRUE;     // Return code to control a process
    PMEM_MERGE_IDA    pMIDA;           // Pointer to the merge IDA
    PSZ               pReplAddr[4];    // Arrey of pointers to replacement strings
    CHAR              szNumber[10];    // Temporary character string
    CHAR              szInvNumber[10]; // Temporary character string
    ULONG             ulProgPos;       // new position of progress indicator
    MemoryFactory *pFactory = MemoryFactory::getInstance();

    // Get address of MEM_MERGE_IDA
    pMIDA = (PMEM_MERGE_IDA)pCommArea->pUserIDA;

   int           iTmRC;               // Tm return code

   // get next memory proposal
   if ( pMIDA->fFirstExtract )
   {
     pMIDA->fFirstExtract = FALSE;
     iTmRC = pMIDA->pMergeMem->getFirstProposal( *(pMIDA->pProposal), &(pMIDA->iComplete) );
   }
   else
   {
     iTmRC = pMIDA->pMergeMem->getNextProposal( *(pMIDA->pProposal), &(pMIDA->iComplete) );
   } /* endif */

   if ( iTmRC == NO_ERROR )
   {
     // write proposal to output memory
     pMIDA->ulSegmentCounter++;
     iTmRC = pMIDA->pOutputMem->putProposal( *(pMIDA->pProposal) );
     if ( iTmRC != NO_ERROR  )
     {
       pMIDA->ulInvSegmentCounter++;
       pFactory->showLastError( NULL, pMIDA->szNameMem, pMIDA->pOutputMem, hWnd );
     } /* end */        

     pCommArea->usComplete = (USHORT)pMIDA->iComplete;
   }
   else if ( iTmRC == OtmMemory::INFO_ENDREACHED )
   {
     // no error handling for this return code
   }
   else
   {
     pFactory->showLastError( NULL, pMIDA->szNameMergeMem, pMIDA->pMergeMem, hWnd );
   } /* endif */


    switch ( iTmRC )
    {
      case NO_ERROR:
        // Post the next WM_EQF_MEMMERGE_PROCESS message but check
        // first whether an other message is in the message queue
        // and if it is dispatch it.
        pCommArea->usComplete = (USHORT)pMIDA->iComplete;
        UtlDispatch();
        WinPostMsg( hWnd, WM_EQF_PROCESSTASK, MP1FROMSHORT( MEM_MERGE_TASK ), NULL );
        break;

     case OtmMemory::INFO_ENDREACHED:
       // Do the processing only if the merge is not started via import
       // The import has it's own end processing
        if ( !pMIDA->fImport )
        {
          //--- set the progress indicator to 100 percent
          pCommArea->usComplete = 100;
          WinSendMsg( hWnd, WM_EQF_UPDATESLIDER, MP1FROMSHORT( 100 ), NULL );

          // -----------------------------------------------------
          //  Close the translation memory to be merged and the Tm
          usRc = CloseMergeTmAndTm( pMIDA, TRUE );                /*@1139C*/

          // -----------------------------------------------------
          // Prepare the addresses for the replacements strings
          // for the messages
          OEMTOANSI( pMIDA->szNameMergeMem );
          OEMTOANSI( pMIDA->szNameMem );
          pReplAddr[0] = pMIDA->szNameMergeMem;
          pReplAddr[1] = pMIDA->szNameMem;
          pReplAddr[2] = ltoa( pMIDA->ulSegmentCounter, szNumber, 10 );
          pReplAddr[3] = ltoa( pMIDA->ulInvSegmentCounter, szInvNumber, 10 );
          if ( usRc == NO_ERROR )
          {
            // Issue the message:
            // Merge of translation memory %1 into translation memory %2
            // successfully completed. %3 Segments have been merged.
            // %4 Segments were obsolete.
            UtlError( MESSAGE_MEM_MERGE_COMPLETED, MB_OK, 4,
                      &pReplAddr[0], EQF_INFO );
          }
          else
          {
            // Something in the termination process failed
            // Issue the message:
            // Termination of merge of translation memory
            // %1 into translation memory %2 failed.
            UtlError( ERROR_MEM_MERGE_TERMFAILED, MB_CANCEL, 2, &pReplAddr[0], EQF_ERROR );
          } /* endif */
          ANSITOOEM( pMIDA->szNameMergeMem );
          ANSITOOEM( pMIDA->szNameMem );
        } /* endif */

        // Issue message WM_EQF_MEMMERGE_END
        WinPostMsg( hWnd, WM_EQF_PROCESSTASK, MP1FROMSHORT( MEM_END_MERGE ), NULL );
        break;


     default:
       // Do the processing only if the merge is not started via import
       // The import has it's own end processing
       if ( !pMIDA->fImport )
       {
         // Prepare the addresses for the replacements strings
         // for the messages
         OEMTOANSI( pMIDA->szNameMergeMem );
         OEMTOANSI( pMIDA->szNameMem );
         pReplAddr[0] = pMIDA->szNameMergeMem;
         pReplAddr[1] = pMIDA->szNameMem;

         //  Close the translation memory to be merged and the Tm
         usRc = CloseMergeTmAndTm( pMIDA, FALSE );               /*@1139C*/

         // Issue the message:
         // The merge of translation memory %1 into translation memory  %1
         // has terminated abnormally.
         UtlError( ERROR_MEM_MERGE_TERMINATED, MB_CANCEL, 2,
                   &pReplAddr[0], EQF_ERROR );
         ANSITOOEM( pMIDA->szNameMergeMem );
         ANSITOOEM( pMIDA->szNameMem );

       }
       else
       {
         // Indicate that the merge terminated abnormally
         pMIDA->usImportRc = TM_IMPORT_FAILED;
       } /* endif */

       // Issue message WM_EQF_MEMMERGE_END
       WinPostMsg( hWnd, WM_EQF_PROCESSTASK, MP1FROMSHORT( MEM_END_MERGE ), NULL );
       break;
    } /* end switch */
  } /* end of function EQFMemMergeProcess */

// ================ Handle the message WM_EQF_MEMMERGE_END =========================

  VOID EQFMemMergeEnd
  (
  PPROCESSCOMMAREA  pCommArea,
  HWND              hWnd,
  LPARAM            mp2
  )
  {
    USHORT            usRc = TRUE;    // Return code to control the process
    PMEM_MERGE_IDA    pMIDA;          // Pointer to the merge IDA
    PSZ               pReplAddr[2];   // Arrey of pointers to replacement strings
    USHORT            usResponse;     // return from error message

    // Get address of MEM_MERGE_IDA
    pMIDA = (PMEM_MERGE_IDA)pCommArea->pUserIDA;

    //  Check if the termination was due to a
    //  forced termination message. mp2 is in that case not zero.
    if (SHORT1FROMMP2(mp2) != 0)
    {
      // Do the processing only if the merge is not started via import
      // The import has it's own end processing
      if ( !pMIDA->fImport )
      {
        // Prepare the addresses for the replacements strings
        // for the messages
        OEMTOANSI( pMIDA->szNameMergeMem );
        OEMTOANSI( pMIDA->szNameMem );
        pReplAddr[0] = pMIDA->szNameMergeMem;
        pReplAddr[1] = pMIDA->szNameMem;

        // Issue the message:
        // The merge of translation memory %1 into translation memory %2
        // was forced before completion.
//      UtlError( ERROR_MEM_MERGE_TERM_FORCED, MB_CANCEL, 2,
//                &pReplAddr[0], EQF_WARNING );

        usResponse = UtlError( ERROR_MEM_MERGE_TERM_FORCED, MB_YESNO, 2,
                               &pReplAddr[0], EQF_WARNING );

        ANSITOOEM( pMIDA->szNameMergeMem );
        ANSITOOEM( pMIDA->szNameMem );
        switch ( usResponse )
        {
           //------------------------------------------------------
           case MBID_YES :   //--- cancel merge process
              //  Close the translation memory to be merged and the Tm
              CloseMergeTmAndTm( pMIDA, TRUE );                  /*@1139C*/
              break;
           //--------------------------------------------------------------------
           case MBID_NO :   //--- do not cancel merge process
              //--- stop further processing
              usRc = 0;
              break;
        }/*endswitch*/

//      //  Close the translation memory to be merged and the Tm
//      CloseMergeTmAndTm( pMIDA );
      }
      else
      {
        // Indicate that the merge termination was forced
        pMIDA->usImportRc = TM_IMPORT_FORCED;
      } /* endif */
    } /* endif */

    if (usRc)
    {
      // Let all pending messages pass by
      UtlDispatch();
      pCommArea = (PPROCESSCOMMAREA)AccessGenProcCommArea( hWnd );
      if ( pCommArea != NULL )
      {
        pMIDA = (PMEM_MERGE_IDA)pCommArea->pUserIDA;
      }
      else
      {
        pMIDA = NULL;
      } /* endif */

      if ( pMIDA != NULL )
      {
        if ( pMIDA->pProposal != NULL ) free(pMIDA->pProposal); 
        pMIDA->pProposal = NULL;

        // If the merge was started via the import function then
        // invoke the import termination function
        if ( pMIDA->fImport )
        {
          // Perform the Import End processing
          EQFMemImportTrojaEnd( pMIDA );
          pCommArea->pUserIDA = NULL;
        }
        else
        {
          // Free the merge IDA
          UtlAlloc( (PVOID *) &pMIDA, 0L, 0L, NOMSG );
          pCommArea->pUserIDA = NULL;
        } /* endif */

        // Dismiss the slider window if it had been created
        EqfRemoveObject( TWBFORCE, hWnd );
      } /* endif */
    } /* endif */
  } /* end of function EQFMemMergeEnd */

// ================ Close the Tm to be merged and Tm ============

 USHORT CloseMergeTmAndTm
 (
    PMEM_MERGE_IDA  pMIDA,             // Pointer to the merge IDA   /*@1139C*/
    BOOL            fMsg               // message flag               /*@1139A*/
 )
   // The function returns TRUE if everything OK else FALSE
 {
   MemoryFactory *pFactory = MemoryFactory::getInstance();

   if ( pMIDA->pMergeMem != NULL )
   {
     // Close the translation memory to be merged
     pFactory->closeMemory( pMIDA->pMergeMem );
     pMIDA->pMergeMem = NULL;
   } /* endif */

   if ( pMIDA->pOutputMem != NULL )
   {
     pFactory->closeMemory( pMIDA->pOutputMem );
     pMIDA->pOutputMem = NULL;
   } /* endif */

   return 0;
 } /* end of function CloseMergeTmAndTm */


//
// non-DDE I/F functions
//

USHORT MemFuncMergeTM
(
  PMEM_MERGE_IDA    pMIDA           // Pointer to the merge IDA
)
{
  pMIDA->usTask = MEM_START_MERGE;
  do
  {
    switch ( pMIDA->usTask )
    {
      case MEM_START_MERGE:
        MemFuncMergeStart( pMIDA );
        break;
      case MEM_MERGE_TASK:
        MemFuncMergeProcess( pMIDA );
        break;
    } /* endswitch */
  } while ( pMIDA->usTask != MEM_END_MERGE );
  MemFuncMergeEnd( pMIDA );
  return( 0 );
} /* end of function MemFuncMergeTM */


USHORT MemFuncMergeStart
(
  PMEM_MERGE_IDA    pMIDA           // Pointer to the merge IDA
)
{
  USHORT            usRc = TRUE;    // Return code to control the process
  USHORT            usTmRc;         // Return code of Tm functions
  PSZ               pReplAddr[3];   // Pointer to an address list of replacement strings
  LONG              lBytesShort;    // Number of bytes short on the disk
  PSZ               pszTemp;        // Temporary character pointer
  CHAR              szNumber[10];   // Working field to display numbers
  USHORT            usDummy;                                         /*@RCA*/
  MemoryFactory *pFactory = MemoryFactory::getInstance();

  pMIDA->szDriveMem[0] = pMIDA->szPathMem[0];
  pszTemp = pMIDA->szDriveMem;

  if ( !usRc )
  {
    // Check if merge was initialized via an import operation
    if (pMIDA->fImport)
    {
      // Terminate the merge operation via the import terminate
      pMIDA->usImportRc = TM_IMPORT_FAILED;
      EQFMemImportTrojaEnd( pMIDA );
    }
    else
    {
       if ( pMIDA->pMergeMem != NULL )
       {
         pFactory->closeMemory( pMIDA->pMergeMem );
         pMIDA->pMergeMem = NULL;
       } /* endif */

       if ( pMIDA->pOutputMem != NULL )
       {
         pFactory->closeMemory( pMIDA->pOutputMem );
         pMIDA->pOutputMem = NULL;
       } /* endif */

      if ( pMIDA->pProposal != NULL ) free(pMIDA->pProposal); 
      pMIDA->pProposal = NULL;

      // Free the merge IDA
      UtlAlloc( (PVOID *) &pMIDA, 0L, 0L, NOMSG );
    } /* endif */
  } /* endif */
  else
  {
    // The initialization of translation memory merge is OK
    // Get current time stamp and save it in the MIDA
    UtlTime( &(pMIDA->tStamp));
    pMIDA->usTask = MEM_MERGE_TASK;
  } /* endif */
  return usRc;
} /* end of function MemFuncMergeStart */

VOID MemFuncMergeProcess
(
  PMEM_MERGE_IDA    pMIDA              // Pointer to the merge IDA
)
{
   MemoryFactory *pFactory = MemoryFactory::getInstance();
   int           iTmRC = 0;            // Tm return code

   // get next memory proposal
   if ( pMIDA->fFirstExtract )
   {
     pMIDA->fFirstExtract = FALSE;
     iTmRC = pMIDA->pMergeMem->getFirstProposal( *(pMIDA->pProposal), &(pMIDA->iComplete) );
   }
   else
   {
     iTmRC = pMIDA->pMergeMem->getNextProposal( *(pMIDA->pProposal), &(pMIDA->iComplete) );
   } /* endif */

   if ( iTmRC == NO_ERROR )
   {
     // write proposal to output memory
     pMIDA->ulSegmentCounter++;
     iTmRC = pMIDA->pOutputMem->putProposal( *(pMIDA->pProposal) );
     if ( iTmRC != NO_ERROR  )
     {
       pMIDA->ulInvSegmentCounter++;
       pFactory->showLastError( NULL, pMIDA->szNameMem, pMIDA->pOutputMem, HWND_FUNCIF );
     } /* end */        
   }
   else if ( iTmRC == OtmMemory::INFO_ENDREACHED )
   {
     // no error handling for this return code
   }
   else
   {
     pFactory->showLastError( NULL, pMIDA->szNameMergeMem, pMIDA->pMergeMem, HWND_FUNCIF );
   } /* endif */


    switch ( iTmRC )
    {
      case NO_ERROR:
        // stay in process mode
        break;

     case OtmMemory::INFO_ENDREACHED:
       // Do the processing only if the merge is not started via import
       // The import has it's own end processing
       if ( !pMIDA->fImport )
       {
         //  Close the translation memory to be merged and the Tm
         CloseMergeTmAndTm( pMIDA, TRUE );                
       } /* endif */

       // switch to MEM_END_MERGE
       pMIDA->usTask = MEM_END_MERGE;
       break;


     default:
       if ( !pMIDA->fImport )
       {
         //  Close the translation memory to be merged and the Tm
         CloseMergeTmAndTm( pMIDA, FALSE );               /*@1139C*/

       }
       else
       {
         // Indicate that the merge terminated abnormally
         pMIDA->usImportRc = TM_IMPORT_FAILED;
       } /* endif */

       // switch to MEM_END_MERGE
       pMIDA->usTask = MEM_END_MERGE;
       break;
    } /* end switch */
} /* end of function MemFuncMergeProcess */

VOID MemFuncMergeEnd
(
  PMEM_MERGE_IDA    pMIDA          // Pointer to the merge IDA
)
{
    USHORT            usRc = TRUE;    // Return code to control the process

    if (usRc)
    {
      if ( pMIDA != NULL )
      {
        // If the merge was started via the import function then
        // invoke the import termination function
        if ( pMIDA->fImport )
        {
          // Perform the Import End processing
          EQFMemImportTrojaEnd( pMIDA );
        }
        else
        {
          // Free the merge IDA
          if ( pMIDA->pProposal != NULL ) free(pMIDA->pProposal); 
          pMIDA->pProposal = NULL;
          UtlAlloc( (PVOID *) &pMIDA, 0L, 0L, NOMSG );
        } /* endif */
      } /* endif */
    } /* endif */
} /* end of function MemFuncMergeEnd */
