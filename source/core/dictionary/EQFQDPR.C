//+----------------------------------------------------------------------------+
//|EQFQDPR.C                                                                   |
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
//|  Marc Hoffmann                                                             |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//|  QDPR main function QDPRDictionaryPrint                                    |
//|  QDPR print and analyze process main functions                             |
//+----------------------------------------------------------------------------+
//|Entry Points:                                                               |
//|                                                                            |
//|    QDPRDictionaryPrint                                                     |
//|                                                                           |
//|+-- status ("H"=Header,"D"=Design,"C"=Code,"T"=Test, " "=complete,          |
//|            "Q"=Quick-and-dirty )                                           |
//+----------------------------------------------------------------------------+
//|Externals:                                                                  |
//|                                                                            |
//|    QDPRDictionaryPrint                                                     |
//|    SetOutputFileNameDlg                                                    |
//|                                                                           |
//|+-- status ("H"=Header,"D"=Design,"C"=Code,"T"=Test, " "=complete,          |
//|            "Q"=Quick-and-dirty )                                           |
//+----------------------------------------------------------------------------+
//|Internals:                                                                  |
//|                                                                            |
//|    QDPRPrintProcessWindow                                                  |
//|    QDPRPrintThread                                                         |
//|    QDPRAnalyze                                                             |
//|    QDPRPrint                                                               |
//|                                                                           |
//|+-- status ("H"=Header,"D"=Design,"C"=Code,"T"=Test, " "=complete,          |
//|            "Q"=Quick-and-dirty )                                           |
//+----------------------------------------------------------------------------+
//|To be done / known limitations / caveats:                                   |
//|                                                                            |
//+----------------------------------------------------------------------------+

/**********************************************************************/
/*                           include files                            */
/**********************************************************************/
#define INCL_EQF_FILT             // dictionary filter functions
#define INCL_EQF_ANALYSIS         // analysis functions
#define INCL_EQF_SLIDER           // slider utility functions
#define INCL_EQF_DICTPRINT        // dictionary print functions
#define INCL_EQF_DLGUTILS         // dialog utilities
#define INCL_EQF_ASD              // dictionary access functions (Asd...)
#define INCL_EQF_LDB              // dictionary data encoding functions
#include <eqf.h>                  // General Translation Manager include file
#include <process.h>
#include <stddef.h>
#include "EQFLDBI.H"              // internal header file of QLDB!!!
#include "EQFQDPRI.H"             // internal header file for dictionary print
#include "EQFQDPR.ID"             // IDs for dictionary print
#include "OtmDictionaryIF.H"



//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name: QDPRDictionaryPrint                                          |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//|  USHORT QDPRDictionaryPrint( pszDictName )                                 |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//|  This is the "main" function of QDPR.                                      |
//|                                                                            |
//|  It loads the "Dictionary Print" dialog in which the user can make         |
//|  several selections. The function will also check if the file the          |
//|  user may have selected exists and whether it should then be replaced      |
//|  or the output be appended to it.                                          |
//|  The function will also start the analyze and print process by creating    |
//|  a process window that will then handle everything else.                   |
//+----------------------------------------------------------------------------+
//|Input parameter:                                                            |
//|  PSZ    pszDictName;      // dictionary name                               |
//+----------------------------------------------------------------------------+
//|Output parameter:                                                           |
//|  none                                                                      |
//+----------------------------------------------------------------------------+
//|Returncode type: USHORT                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:                                                                |
//|  QDPR_NO_ERROR         - everything was OK                                 |
//|  QDPR_NO_MEMORY        - not enough memory                                 |
//|  QDPR_RESDLL_FAILURE   - failure in loading the ressource DLL              |
//|  QDPR_CREATING_PROCWND - error in creating the process window              |
//+----------------------------------------------------------------------------+
//|Prerequesits:                                                               |
//|  none                                                                      |
//+----------------------------------------------------------------------------+
//|Side effects:                                                               |
//|  none                                                                      |
//+----------------------------------------------------------------------------+
//|Samples:                                                                    |
//|                                                                            |
//|  usRC = QDPRDictionaryPrint( "QDPRSAMP" );                                 |
//|                                                                            |
//|  This will print the dictionary QDPRSAMP.                                  |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                                                                            |
//|  Allocate storage for input/output structure                               |
//|  Open dictionary                                                           |
//|  Display the dictionary print dialog (QDPRPrintDialog)                     |
//|  IF everything is OK and printing should be done                           |
//|    IF user has selected file as print destination                          |
//|      Check if file exists                                                  |
//|      IF file exists                                                        |
//|        Display message box to ask for append or replace file               |
//|    Start analyze and print process by displaying the process               |
//|      window (QDPRPrintProcessWindow)                                       |
//|  ELSE                                                                      |
//|    Show error                                                              |
//|    Close dictionary opened in QDPRPrintDialog                              |
//+----------------------------------------------------------------------------+

USHORT QDPRDictionaryPrint
(
	PSZ pszDictName
)
{
  /********************************************************************/
  /*                         local variables                          */
  /********************************************************************/
  USHORT           usRC = QDPR_NO_ERROR;          // function returncode
  PQDPR_IN_OUTPUT  psctInOut;                     // input/output structure

  /********************************************************************/
  /*           allocate storage for input/output structure            */
  /********************************************************************/
  if ( UtlAlloc( (PVOID *) &psctInOut, 0L, (LONG)sizeof(QDPR_IN_OUTPUT), ERROR_STORAGE ) )
  {
    /******************************************************************/
    /*     put the dictionary name in the input/output structure      */
    /******************************************************************/
    strncpy( psctInOut->szDictName, pszDictName, sizeof(psctInOut->szDictName)-1 );

    /******************************************************************/
    /*                      open the dictionary                       */
    /******************************************************************/
    usRC = QDPROpenDictionary( psctInOut->szDictName, &(psctInOut->hUCB),
                               &(psctInOut->hDCB) );

    /******************************************************************/
    /*                load the dictionary print dialog                */
    /******************************************************************/
    if ( usRC == QDPR_NO_ERROR )
    {
      INT_PTR      iRC;
	  HMODULE hResMod;
	  hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);

      DIALOGBOX( DIALOG_OWNER, QDPRPRINTDIALOG, hResMod, ID_QDPR_PRTDLG,
                 (PVOID)psctInOut, iRC );
    } /* endif */

    if ( ( usRC = psctInOut->usRC ) == QDPR_NO_ERROR )
    {
      if ( psctInOut->fStartPrinting )
      {
        sprintf( psctInOut->szObjName, "PRINT:%s", psctInOut->szDictName );
        if ( !CreateProcessWindow( psctInOut->szObjName, QDPRCallBack, psctInOut ) )
        {
          usRC = QDPR_CREATING_PROCWND;
          UtlError( QDPR_INIT_FAILURE, MB_CANCEL, 0, NULL, EQF_ERROR );
        } /* endif */
      }
      else
      {
        /**************************************************************/
        /*                 printing should not start                  */
        /*                  so make some cleaning up                  */
        /**************************************************************/
        if ( psctInOut != NULL )
        {
          QDPRCloseDictionary( psctInOut->hUCB, psctInOut->hDCB,
                               psctInOut->fDictHasBeenLocked );

          /************************************************************/
          /*            deallocate input/output structure             */
          /************************************************************/
          QDPRDeallocateInOutStruct( &psctInOut );
        } /* endif */
      } /* endif */
    } /* endif */

    /******************************************************************/
    /*                   check if an error occurred                   */
    /*                   if so do some cleaning up                    */
    /*        otherwise this is done in QDPRPrintProcessWindow        */
    /******************************************************************/
    if ( usRC != QDPR_NO_ERROR )
    {
      if ( psctInOut != NULL )
      {
        QDPRCloseDictionary( psctInOut->hUCB, psctInOut->hDCB,
                             psctInOut->fDictHasBeenLocked );

        /**************************************************************/
        /*             deallocate input/output structure              */
        /**************************************************************/
        QDPRDeallocateInOutStruct( &psctInOut );
      } /* endif */
    } /* endif */
  }
  else
  {
    usRC = QDPR_NO_MEMORY;
  } /* endif */

  return( usRC );

} /* end of function QDPRDictionaryPrint */



//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name: QDPRPrintProcessWindow                                       |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//|  MRESULT EXPENTRY QDPRPrintProcessWindow( hwnd, msg, mp1, mp2 )            |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//|  This is the window procedure for the print process object window.         |
//|                                                                            |
//|  Under OS/2 tis procedure starts a thread for the actual printing          |
//|  Under Windows the print process is performed in small steps each          |
//|  of which is executed on receiving WM_EQF_PROCESSTASK messages             |
//+----------------------------------------------------------------------------+
//|Input parameter:                                                            |
//|  HWND    hwnd     handle of window                                         |
//|  USHORT  msg      type of message                                          |
//|  MPARAM  mp1      first message parameter                                  |
//|  MPARAM  mp2      second message parameter                                 |
//+----------------------------------------------------------------------------+
MRESULT QDPRCallBack
(
  PPROCESSCOMMAREA pCommArea,          // ptr to commmunication area
  HWND             hwnd,               // handle of window
  WINMSG           msg,                // type of message
  WPARAM           mp1,                // first message parameter
  LPARAM           mp2                 // second message parameter
)
{
  MRESULT              mResult = FALSE;      // window procedure return value
  PQDPR_THREAD         psctIDA;              // pointer to thread IDA

  mp1;
  switch ( msg )
  {
    case WM_CREATE :
      {
        BOOL fOK = TRUE;

        /**************************************************************/
        /*          allocate storage for the thread IDA and           */
        /*                for the stack of the thread                 */
        /**************************************************************/
        fOK = UtlAlloc( (PVOID *) &psctIDA, 0L, (LONG)sizeof(QDPR_THREAD), ERROR_STORAGE );

        if ( fOK )
        {
          PSZ        apszParms[2];     // message parameter table
		  HMODULE hResMod;
		  hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);

          /************************************************************/
          /*      save pointer to input/output structure in IDA       */
          /************************************************************/
          psctIDA->psctInOutput = (PQDPR_IN_OUTPUT)PVOIDFROMMP2(mp2);

          /************************************************************/
          /*  save pointer to thread IDA in window reserved storage   */
          /************************************************************/
          pCommArea->pUserIDA = psctIDA;

          /************************************************************/
          /*                 initialize thread values                 */
          /************************************************************/
          psctIDA->usThreadStatus = QDPR_THREAD_WAIT;
          psctIDA->usNextThreadStatus = QDPR_ANAST_START;
          psctIDA->usThreadError = QDPR_NO_ERROR;
          psctIDA->usSyntaxError = QDPR_NO_ERROR;

          /****************************************************************/
          /* supply all information required to create the process        */
          /* window                                                       */
          /****************************************************************/
          pCommArea->sProcessWindowID = ID_QDPR_PROCESS_WINDOW;
          pCommArea->sProcessObjClass = clsDICTPRINT;
          pCommArea->Style            = PROCWIN_TEXTSLIDER;
          pCommArea->sSliderID        = ID_SLIDERWINDOW;
          pCommArea->sTextID          = 1;
          LOADSTRING( NULLHANDLE, hResMod, SID_QDPR_PROC_TITLE,
                      pCommArea->szTitle );
          pCommArea->hIcon            = (HPOINTER) UtlQueryULong(QL_DICTPRINTICON); //hiconDICTPRINT;
          pCommArea->fNoClose         = FALSE;
          pCommArea->swpSizePos.x     = 100;
          pCommArea->swpSizePos.y     = 100;
          pCommArea->swpSizePos.cx    = (SHORT) UtlQueryULong( QL_AVECHARWIDTH ) * 60;
          pCommArea->swpSizePos.cy    = (SHORT) UtlQueryULong( QL_PELSPERLINE ) * 10;
          pCommArea->asMsgsWanted[0]  = WM_EQF_PROCESSTASK;
          pCommArea->asMsgsWanted[1]  = WM_TIMER;
          pCommArea->asMsgsWanted[2]  = 0;
          pCommArea->usComplete       = 0;

          /************************************************************/
          /* Build first text line                                    */
          /************************************************************/
          LOADSTRING( NULLHANDLE, hResMod, SID_QDPR_PRINTING, psctIDA->szWorkBuffer );
          apszParms[0] = psctIDA->psctInOutput->szDictName;
          {
            ULONG MsgLen;

            DosInsMessage( apszParms, 1, psctIDA->szWorkBuffer,
                           strlen(psctIDA->szWorkBuffer),
                           pCommArea->szText, sizeof(pCommArea->szText),
                           &MsgLen );
          }
          /************************************************************/
          /* Build second text line                                   */
          /************************************************************/
          if ( psctIDA->psctInOutput->fPrinterDest )
          {
            LOADSTRING( NULLHANDLE, hResMod, SID_QDPR_PRINTTO_PRINTER,
                        psctIDA->szWorkBuffer3 );
          }
          else
          {
            ULONG MsgLen;

            LOADSTRING( NULLHANDLE, hResMod, SID_QDPR_PRINTTO_FILE,
                        psctIDA->szWorkBuffer );
            apszParms[0] = psctIDA->psctInOutput->szPrintDest;
            DosInsMessage( apszParms, 1, psctIDA->szWorkBuffer,
                          strlen(psctIDA->szWorkBuffer),
                           pCommArea->szText2,
                           sizeof(pCommArea->szText2),
                           &MsgLen );
          } /* endif */
        }
        else
        {
          /************************************************************/
          /*          an error occurred, so close the window          */
          /************************************************************/
          mResult = MRFROMSHORT(TRUE); // indicate an error
        } /* endif */
      }
    break;

    case WM_EQF_INITIALIZE :
      {
        USHORT     usRC;               // buffer for return codes

        /**************************************************************/
        /*                     get pointer to IDA                     */
        /**************************************************************/
        psctIDA =(PQDPR_THREAD) pCommArea->pUserIDA;

        if ( psctIDA != NULL )
        {
          /************************************************************/
          /*          get number of total dictionary entries          */
          /************************************************************/
          usRC = AsdNumEntries( psctIDA->psctInOutput->hDCB,
                                &(psctIDA->ulDictEntries) );

          if ( usRC == LX_RC_OK_ASD )
          {
            usRC = QDPR_NO_ERROR;
            psctIDA->ulDictEntries = max( 1L, psctIDA->ulDictEntries );
          }
          else
          {
            AsdRCHandling( usRC, psctIDA->psctInOutput->hDCB, NULL,
                           NULL, NULL, NULLHANDLE );
          } /* endif */

          /************************************************************/
          /*                     start the process                    */
          /************************************************************/
          if ( usRC == QDPR_NO_ERROR )
          {
            WinPostMsg( hwnd, WM_EQF_PROCESSTASK, 0, 0L );
          } /* endif */

          /************************************************************/
          /*    check if an error occurred, if so close the window    */
          /************************************************************/
          if ( usRC != QDPR_NO_ERROR )
          {
            EqfRemoveObject( TWBFORCE, hwnd );
          } /* endif */
        } /* endif */
      }
    break;

    /******************************************************************/
    /* Process next step of print process                             */
    /******************************************************************/
    case WM_EQF_PROCESSTASK :
      psctIDA =(PQDPR_THREAD) pCommArea->pUserIDA;

      /****************************************************************/
      /* Perform next step                                            */
      /****************************************************************/
      if ( psctIDA->fStopThread )
      {
        /**************************************************************/
        /* Set thread finished flag, so close can continue            */
        /**************************************************************/
        psctIDA->usThreadStatus = QDPR_THREAD_FINISHED;
        psctIDA->usNextThreadStatus = QDPR_THREAD_FINISHED;
      }
      else if ( ( psctIDA->usThreadStatus >= QDPR_ANAST_START ) &&
                ( psctIDA->usThreadStatus <= QDPR_ANAST_FINISHED ) )
      {
        /**************************************************************/
        /*                    call analyze process                    */
        /**************************************************************/
        QDPRAnalyze( psctIDA );
      }
      else
      {
        if ( ( psctIDA->usThreadStatus >= QDPR_PRTST_START ) &&
             ( psctIDA->usThreadStatus <= QDPR_PRTST_FINISHED ) )
        {
          /************************************************************/
          /*                    call print process                    */
          /************************************************************/
          QDPRPrint( psctIDA );
        } /* endif */
      } /* endif */

      /**********************************************************/
      /*               check if an error occurred               */
      /**********************************************************/
      if ( psctIDA->usThreadError != QDPR_NO_ERROR )
      {
        /**********************************************************/
        /* Report the error condition                             */
        /**********************************************************/
        QDPRReportError( psctIDA );

        /********************************************************/
        /*                  end the processing                  */
        /********************************************************/
        psctIDA->usThreadStatus = QDPR_THREAD_FINISHED;
        EqfRemoveObject( TWBFORCE, hwnd );
      }
      else if ( psctIDA->usNextThreadStatus != QDPR_THREAD_FINISHED )
      {
        ULONG      ulPercent;          // current completion rate

        /****************************************************/
        /*                 move the slider                  */
        /*          only if the percentage changed          */
        /****************************************************/
        ulPercent = psctIDA->ulEntriesProcessed * 100L /
                    psctIDA->ulDictEntries;

        if ( ulPercent > psctIDA->ulOldPercent )
        {
          WinSendMsg( hwnd, WM_EQF_UPDATESLIDER,
                      MP1FROMSHORT((SHORT)ulPercent), NULL );
          psctIDA->ulOldPercent = ulPercent;
        } /* endif */

        /****************************************************************/
        /* Prepare for next step                                        */
        /****************************************************************/
        psctIDA->usThreadStatus = psctIDA->usNextThreadStatus;
        UtlDispatch();
        WinPostMsg( hwnd, WM_EQF_PROCESSTASK, NULL, NULL );
      }
      else
      {
        /****************************************************/
        /*      thread has finished, so end processing      */
        /****************************************************/
        psctIDA->usThreadStatus = QDPR_THREAD_FINISHED;
        EqfRemoveObject( TWBFORCE, hwnd );
      } /* endif */
      break;

    case WM_CLOSE :
      {
        /**************************************************************/
        /*                 get pointer to thread IDA                  */
        /**************************************************************/
        psctIDA =(PQDPR_THREAD) pCommArea->pUserIDA;

        if ( psctIDA != NULL )
        {
          mResult = MRFROMSHORT( TRUE ); // = do not close right now
          /************************************************************/
          /*           ask user if he really wants to stop            */
          /************************************************************/
          if ( UtlError( QDPR_REALLY_STOP_PROCESSING,
                             MB_YESNO | MB_DEFBUTTON2, 0,
                             NULL, EQF_QUERY ) == MBID_YES )
          {
            psctIDA->fNoShowSuccessMsg = TRUE;
            psctIDA->fStopThread = TRUE; // = continue shutdown of process
            WinStartTimer( (HAB)UtlQueryULong( QL_HAB ), hwnd,
                           QDPR_TIMER, QDPR_TIMER_DELAY );
          } /* endif */
        } /* endif */
      }
    break;

    case WM_EQF_TERMINATE :
      {
        /**************************************************************/
        /*                     get pointer to IDA                     */
        /**************************************************************/
        psctIDA = (PQDPR_THREAD)pCommArea->pUserIDA;

        if ( psctIDA != NULL )
        {
          /************************************************************/
          /*                     stop the thread                      */
          /* (wait until thread (OS/2) or process (Windows) has ended)*/
          /************************************************************/
          if ( psctIDA->usThreadStatus != QDPR_THREAD_FINISHED )
          {
            psctIDA->fStopThread = TRUE;
            while ( (psctIDA != NULL) && WinIsWindow( NULL, hwnd ) &&
                    (psctIDA->usThreadStatus != QDPR_THREAD_FINISHED) )
            {
              /********************************************************/
              /* Wait until process has ended                         */
              /********************************************************/
              UtlDispatch();
              pCommArea =(PPROCESSCOMMAREA) AccessGenProcCommArea( hwnd );
              if ( pCommArea != NULL )
              {
                psctIDA = (PQDPR_THREAD)pCommArea->pUserIDA;
              }
              else
              {
                psctIDA = NULL;
              } /* endif */
            } /* endwhile */
          } /* endif */

          /************************************************************/
          /*    check if the print destination file is still open     */
          /*             due to an error, if so close it              */
          /*   and check if the file is smaller than QDPR_FILE_KEEP   */
          /*                if it is smaller delete it                */
          /*                                                          */
          /* delete only if diskfull      (happens in PrintDestClose) */
          /* do not delete if user closes                             */
          /************************************************************/
          if ( psctIDA != NULL )
          {
            REMOVESYMBOL( psctIDA->psctInOutput->szPrintDest );
            if ( psctIDA->psctPrintDest != NULL )
            {
              USHORT usTempRC;

              QDPRPrintDestClose( &(psctIDA->psctPrintDest), &usTempRC );
            } /* endif */

            if ( psctIDA->psctInOutput != NULL )
            {
              /**********************************************************/
              /*                    close dictionary                    */
              /**********************************************************/
              QDPRCloseDictionary( psctIDA->psctInOutput->hUCB,
                                   psctIDA->psctInOutput->hDCB,
                                   psctIDA->psctInOutput->fDictHasBeenLocked );

            } /* endif */

            /************************************************************/
            /*                   show success message                   */
            /*             or no-entries-processed message              */
            /************************************************************/
            if ( ( psctIDA->usThreadError == QDPR_NO_ERROR ) &&
                 ( psctIDA->fNoShowSuccessMsg == FALSE ) )
            {
              (psctIDA->pReplAddr)[0] = psctIDA->psctInOutput->szDictName;

              if ( psctIDA->fEntriesPrinted )
              {
                /****************************************************/
                /*             move the slider to 100%              */
                /****************************************************/
                WinSendMsg( hwnd, WM_EQF_UPDATESLIDER, MP1FROMSHORT(100), NULL );
                UtlError( QDPR_PRINT_SUCCESSFULLY_ENDED, MB_OK, 1,
                              &(psctIDA->pReplAddr)[0], EQF_INFO );
              }
              else
              {
                UtlError( QDPR_NO_ENTRIES_PROCESSED, MB_OK, 1,
                              &(psctIDA->pReplAddr)[0], EQF_INFO );

                /********************************************************/
                /*      delete output file if no entries processed      */
                /*                                                      */
                /********************************************************/
                if ( !( psctIDA->psctInOutput->fPrinterDest ) )
                {
                  UtlDelete( psctIDA->psctInOutput->szPrintDest, 0L, FALSE );
                } /* endif */
              } /* endif */
            } /* endif */

            /************************************************************/
            /*       deallocate other storage from the thread IDA       */
            /************************************************************/
            QDPRDeallocateIDAStorage( psctIDA );

            /************************************************************/
            /*                  deallocate thread IDA                   */
            /************************************************************/
            UtlAlloc( (PVOID *) &psctIDA, 0L, 0L, NOMSG );
          } /* endif */
        } /* endif */
      }
    break;
  } /* endswitch */

  return( mResult );

} /* end of function QDPRPrintProcessWindow */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name: QDPRReportError                                              |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//|  VOID QDPRReportError( pThreadIDA )                                        |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//|  This function reports any error condition raised in the thread.           |
//+----------------------------------------------------------------------------+
//|Input parameter:                                                            |
//|  PQDPR_THREAD    pThreadIDA;    // pointer to thread IDA                   |
//+----------------------------------------------------------------------------+
//|Returncode type: VOID                                                       |
//+----------------------------------------------------------------------------+
VOID QDPRReportError
(
  PQDPR_THREAD    psctIDA              // pointer to thread IDA
)
{
  switch ( psctIDA->usThreadError )
  {
    case QDPR_SYNTAX_ERROR :
      {
        /**************************************************/
        /*     the line number for the error code is      */
        /*     known only at this location, so set it     */
        /*           in the UtlError parameters           */
        /**************************************************/
        sprintf( psctIDA->szWorkBuffer2, "%d",
                 psctIDA->usLineNumber );
        (psctIDA->pReplAddr)[1] = psctIDA->szWorkBuffer2;

        UtlError( psctIDA->usSyntaxError, MB_CANCEL,
                  psctIDA->usNoOfSyErrParms,
                  &(psctIDA->pReplAddr)[0], EQF_ERROR );
      }
    break;
    case QDPR_ERROR_WRITE_TO_DEST_FILE :
      {
        if ( psctIDA->psctInOutput->fPrinterDest )
        {
          psctIDA->usThreadError =
                   QDPR_ERROR_WRITE_TO_PRINTER;
          UtlError( psctIDA->usThreadError, MB_CANCEL, 0,
                    NULL, EQF_ERROR );
        }
        else
        {
          (psctIDA->pReplAddr)[0] =
                    psctIDA->psctInOutput->szPrintDest;
          if ( psctIDA->usDosRC != NO_ERROR )
          {
            UtlError( psctIDA->usDosRC,
                      MB_CANCEL, 1,
                      &(psctIDA->pReplAddr)[0], DOS_ERROR );
          }
          else
          {
            UtlError( psctIDA->usThreadError, MB_CANCEL, 1,
                      &(psctIDA->pReplAddr)[0], EQF_ERROR );
          } /* endif */

          /**************************************************/
          /* delete file if disc full or something else     */
          /* happened                                       */
          /**************************************************/
          if ( psctIDA->psctPrintDest != NULL )
          {
            USHORT usTempRC;

            REMOVESYMBOL( psctIDA->psctInOutput->szPrintDest );

            QDPRPrintDestClose( &(psctIDA->psctPrintDest), &usTempRC );
            UtlDelete( psctIDA->psctInOutput->szPrintDest, 0L, FALSE );
          } /* endif */
        } /* endif */
      }
    break;
    case QDPR_ERROR_OPEN_DEST_FILE :
      {
        if ( psctIDA->psctInOutput->fPrinterDest )
        {
          psctIDA->usThreadError =
                   QDPR_ERROR_OPEN_PRINTER;
          UtlError( psctIDA->usThreadError, MB_CANCEL, 0,
                    NULL, EQF_ERROR );
        }
        else
        {
          (psctIDA->pReplAddr)[0] =
                    psctIDA->psctInOutput->szPrintDest;
          UtlError( psctIDA->usThreadError, MB_CANCEL, 1,
                    &(psctIDA->pReplAddr)[0], EQF_ERROR );
        } /* endif */
      }
    break;
    case QDPR_LOAD_DICTIONARY :
    case QDPR_READ_DICTIONARY :
      {
        (psctIDA->pReplAddr)[0] =
                  psctIDA->psctInOutput->szDictName;
                                                     /* 21@KIT1138A */
        switch ( psctIDA->RCType )
        {
          case LDB_RC :
            UtlError( psctIDA->usExtRC, MB_CANCEL, 1,
                      &(psctIDA->pReplAddr)[0], QLDB_ERROR );
            break;

          case FILT_RC :
            /************************************************/
            /* Filter uses Dos error codes                  */
            /*    so use the DOS_ERROR handler of UtlError  */
            /************************************************/
            UtlError( psctIDA->usExtRC, MB_CANCEL, 1,
                      &(psctIDA->pReplAddr)[0], DOS_ERROR );
            break;

          case ASD_RC :
          default :
            AsdRCHandling( psctIDA->usExtRC,
                           psctIDA->psctInOutput->hDCB, NULL,
                           NULL, NULL, NULLHANDLE );
            break;
        } /* endswitch */
      }
    break;
    case QDPR_NO_MEMORY :
      {
        UtlError( ERROR_STORAGE, MB_CANCEL, 0, NULL,
                  EQF_ERROR );
      }
    break;
    case QDPR_PROGRAM_ERROR :
      {
        UtlError( ERROR_INTERNAL, MB_CANCEL, 0, NULL,
                  EQF_ERROR );
      }
    break;
    case QDPR_NOMSG_ERROR :
      {
      }
    break;
    default :
      {
        UtlError( psctIDA->usThreadError, MB_CANCEL, 0,
                  NULL, EQF_ERROR );
      }
    break;
  } /* endswitch */
} /* end of function QDPRReportError */


//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name: QDPRAnalyze                                                  |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//|  VOID QDPRAnalyze( psctIDA )                                               |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//|  This function handles the analyzing of the selected format information    |
//|  file.                                                                     |
//|  The program flow of QDPRAnalyze is not sequential, but it is              |
//|  "status-based" (i.e. the function will receive a status indicator         |
//|  telling it what action is to be taken next). This processing is done in   |
//|  order to not stop processing of messages (e.g. user-interacions) in the   |
//|  QDPRPrintProcessWindow window procedure. The status is passed via the     |
//|  thread structure.                                                         |
//|                                                                            |
//|  The function will perform the following actions. It will scan the format  |
//|  information file,                                                         |
//|  check the syntax of the tags and system variables used, load the three    |
//|  buffers (pagehead, entry and pagefoot buffers) and set up the four        |
//|  field cross-reference tables (pagehead, entry, repeat and pagefoot        |
//|  field cross-reference tables).                                            |
//|  All information needed is passed via the thread structure.                |
//+----------------------------------------------------------------------------+
//|Input parameter:                                                            |
//|  PQDPR_THREAD    psctIDA;     // pointer to thread IDA                     |
//+----------------------------------------------------------------------------+
//|Output parameter:                                                           |
//|  none                                                                      |
//+----------------------------------------------------------------------------+
//|Returncode type: VOID (returncodes are passed via the thread IDA)           |
//+----------------------------------------------------------------------------+
//|Returncodes:                                                                |
//|  none                                                                      |
//+----------------------------------------------------------------------------+
//|Prerequesits:                                                               |
//|  - a correctly set up thread IDA                                           |
//|  - a correctly set up input/output structure                               |
//+----------------------------------------------------------------------------+
//|Side effects:                                                               |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Samples:                                                                    |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                                                                            |
//|  SWITCH on analyze status                                                  |
//|    CASE QDPR_ANAST_START                                                   |
//|      Initialize the analyze process                                        |
//|    CASE QDPR_ANAST_PROCESS_FILE                                            |
//|      WHILE not end of token list is reached and no error occurred          |
//|        SWITCH on token read                                                |
//|          CASE description tag                                              |
//|            CALL QDPRReadOverDescription                                    |
//|          CASE header tag                                                   |
//|            CALL QDPRReadHeader                                             |
//|          CASE pagehead tag                                                 |
//|            CALL QDPRReadPagehead                                           |
//|          CASE entry tag                                                    |
//|            CALL QDPRReadEntry                                              |
//|          CASE pagefoot tag                                                 |
//|            CALL QDPRReadPagefoot                                           |
//|          CASE trailer tag                                                  |
//|            CALL QDPRReadTrailer                                            |
//|          CASE set tag                                                      |
//|            CALL QDPRReadSetTag                                             |
//|          CASE comment tag                                                  |
//|            CALL QDPRReadOverComment                                        |
//|          CASE TEXT                                                         |
//|            Ignore it                                                       |
//|    CASE QDPR_ANAST_FINISHED                                                |
//|      Finish the analyze process                                            |
//+----------------------------------------------------------------------------+

VOID QDPRAnalyze
(
	 PQDPR_THREAD  psctIDA
)

{
  /********************************************************************/
  /*                         local variables                          */
  /********************************************************************/
  USHORT         usRC = QDPR_NO_ERROR;           // error code
  USHORT         usRetCode;                      // other returncode(s)
  USHORT         i;                              // a counter
  PSZ_W          *ppszData = NULL;               // data arary pointer
  PSZ            pszRun;                         // for QDPR_CRLF_USED macro
  PTOKENENTRY    pTok = NULL;                    // pointer to token buffer
  BOOL           fDescriptionProcessed = FALSE;  // processed description ?
  BOOL           fHeaderProcessed = FALSE;       // processed header part ?
  BOOL           fPageheadProcessed = FALSE;     // processed pagehead part ?
  BOOL           fEntryProcessed = FALSE;        // processed entry part ?
  BOOL           fPagefootProcessed = FALSE;     // processed pagefoot part ?
  BOOL           fTrailerProcessed = FALSE;      // processed trailer part ?



  if ( psctIDA != NULL )
  {
    /******************************************************************/
    /*                     clear the work buffer                      */
    /******************************************************************/
    memset( psctIDA->szWorkBuffer, NULC, QDPR_MAX_STRING );

    switch ( psctIDA->usThreadStatus )
    {
      /****************************************************************/
      /*         make initializations for the analyze process         */
      /****************************************************************/
      case QDPR_ANAST_START :
        {
          /************************************************************/
          /*                   initialize variables                   */
          /************************************************************/
          psctIDA->usPageLength = 0;
          psctIDA->usLineLength = 0;
          psctIDA->ulPageNumber = 0L;
          psctIDA->pszPageEject = NULL;

          /************************************************************/
          /*            get pointer to dictionary profile             */
          /************************************************************/
                                                                /* 5@KIT1138C */
          psctIDA->usExtRC = AsdRetPropPtr( psctIDA->psctInOutput->hUCB,
                                            psctIDA->psctInOutput->hDCB,
                                            &psctIDA->pDictProp );
          if ( psctIDA->usExtRC != LX_RC_OK_ASD )
          {
            usRC = QDPR_LOAD_DICTIONARY;
            psctIDA->RCType = ASD_RC;                           /* 1@KIT1138A */
          } /* endif */

          if ( usRC == QDPR_NO_ERROR )
          {
            /**********************************************************/
            /*   create current, first-page and last-page templates   */
            /* 1.) get the number of fields on each level             */
            /* 2.) allocate storage for data array                    */
            /* 3.) create the 3 QLDB trees                            */
            /**********************************************************/
            if ( QLDBFillFieldTables( psctIDA->pDictProp,
                                      &(psctIDA->ausNoOfFields[0]),
                                      &(psctIDA->ausFieldTable[0]) )
                 != QLDB_NO_ERROR )
            {
              usRC = QDPR_LOAD_DICTIONARY;
            }
            else
            {
              psctIDA->usTotalFields = 0;
              for ( i = 0; i < QLDB_MAX_LEVELS; i++ )
              {
                psctIDA->usTotalFields = psctIDA->usTotalFields + (psctIDA->ausNoOfFields)[i];
              } /* endfor */

              /********************************************************/
              /*           allocate temporarily storage for           */
              /*                creating the templates                */
              /********************************************************/
              if ( UtlAlloc( (PVOID *) &ppszData, 0L,
                             (LONG)( psctIDA->usTotalFields *
                                     sizeof( PSZ_W ) + 1 ), NOMSG ) )
              {
                for ( i = 0; i < psctIDA->usTotalFields; i++ )
                {
                  ppszData[i] = (PSZ_W)( ppszData +
                                       ( psctIDA->usTotalFields *
                                         sizeof( PSZ_W ) ) );
                } /* endfor */

                /******************************************************/
                /*            create the current template             */
                /******************************************************/
                if ( ( usRetCode = QLDBCreateTree( psctIDA->ausNoOfFields,
                                                   ppszData,
                                       &(psctIDA->psctCurrentTemplate) ) )
                     != QLDB_NO_ERROR )

                {
                  if ( usRetCode == QLDB_NO_MEMORY )
                  {
                    usRC = QDPR_NO_MEMORY;
                  }
                  else
                  {
                    usRC = QDPR_LOAD_DICTIONARY;
                  } /* endif */
                } /* endif */

                if ( usRC == QDPR_NO_ERROR )
                {
                  /****************************************************/
                  /*          create the first-page template          */
                  /****************************************************/
                  if ( ( usRetCode = QLDBCreateTree( psctIDA->ausNoOfFields,
                                                     ppszData,
                                         &(psctIDA->psctFirstPageTemplate) ) )
                       != QLDB_NO_ERROR )

                  {
                    if ( usRetCode == QLDB_NO_MEMORY )
                    {
                      usRC = QDPR_NO_MEMORY;
                    }
                    else
                    {
                      usRC = QDPR_LOAD_DICTIONARY;
                    } /* endif */
                  } /* endif */
                } /* endif */

                if ( usRC == QDPR_NO_ERROR )
                {
                  /****************************************************/
                  /*          create the last-page template           */
                  /****************************************************/
                  if ( ( usRetCode = QLDBCreateTree( psctIDA->ausNoOfFields,
                                                     ppszData,
                                         &(psctIDA->psctLastPageTemplate) ) )
                       != QLDB_NO_ERROR )

                  {
                    if ( usRetCode == QLDB_NO_MEMORY )
                    {
                      usRC = QDPR_NO_MEMORY;
                    }
                    else
                    {
                      usRC = QDPR_LOAD_DICTIONARY;
                    } /* endif */
                  } /* endif */
                } /* endif */

                /******************************************************/
                /*    deallocate the temporarily allocated storage    */
                /******************************************************/
                if ( ppszData != NULL )
                {
                  UtlAlloc( (PVOID *) (PVOID *)&ppszData, 0L, 0L, NOMSG );
                } /* endif */
              }
              else
              {
                usRC = QDPR_NO_MEMORY;
              } /* endif */
            } /* endif */
          } /* endif */

          psctIDA->usNextThreadStatus = QDPR_ANAST_PROCESS_FILE;
        }
      break;

      /****************************************************************/
      /*   process the loaded and tokenized format information file   */
      /****************************************************************/
      case QDPR_ANAST_PROCESS_FILE :
        {
          /************************************************************/
          /*      loop over the tokens until the end is reached       */
          /************************************************************/
          pTok = psctIDA->psctInOutput->pTokFormatFile;

          while ( ( pTok->sTokenid != ENDOFLIST ) &&
                  ( usRC == QDPR_NO_ERROR ) )
          {
            switch ( pTok->sTokenid )
            {
              case QDPR_DESCRIPTION_TOKEN :
                {
                  /****************************************************/
                  /*          description tag has been found          */
                  /*  check if already one description tag has been   */
                  /*                    processed                     */
                  /****************************************************/
                  if ( !fDescriptionProcessed )
                  {
                    usRC = QDPRReadOverDescription( &pTok );

                    /**************************************************/
                    /* if a syntax error has been found this can only */
                    /*mean that the description tag appeared a second */
                    /*  time (otherwise the syntax errror would have  */
                    /*         been found by QDPRScanDescTag)         */
                    /**************************************************/
                    if ( usRC == QDPR_SYNTAX_ERROR )
                    {
                      QDPRSyntaxError( psctIDA, QDPR_SYER_SAME_TAG_TWICE,
                                       QDPR_DESCRIPTION_TOKEN,
                                       QDPR_EMPTY_STRING, 0,
                                       QDPR_NO_QDPR_TAG, QDPR_NO_QDPR_TAG,
                                       &usRC, &(psctIDA->usSyntaxError) );
                    } /* endif */
                  }
                  else
                  {
                    QDPRSyntaxError( psctIDA, QDPR_SYER_SAME_TAG_TWICE,
                                     QDPR_DESCRIPTION_TOKEN,
                                     QDPR_EMPTY_STRING, 0,
                                     QDPR_NO_QDPR_TAG, QDPR_NO_QDPR_TAG,
                                     &usRC, &(psctIDA->usSyntaxError) );
                  } /* endif */
                }
              break;
              case QDPR_DICTFRONT_TOKEN :
                {
                  /****************************************************/
                  /*     check if this part was alread processed      */
                  /*               if so show an error                */
                  /****************************************************/
                  if ( !fHeaderProcessed )
                  {
                    /**************************************************/
                    /*  read the header part into the corresponding   */
                    /*                    buffers                     */
                    /**************************************************/
                    usRC = QDPRReadHeader( psctIDA, &pTok );
                    fHeaderProcessed = TRUE;
                  }
                  else
                  {
                    QDPRSyntaxError( psctIDA, QDPR_SYER_SAME_TAG_TWICE,
                                     QDPR_DICTFRONT_TOKEN,
                                     QDPR_EMPTY_STRING, 0,
                                     QDPR_NO_QDPR_TAG, QDPR_NO_QDPR_TAG,
                                     &usRC, &(psctIDA->usSyntaxError) );
                  } /* endif */
                }
              break;
              case QDPR_PAGEHEAD_TOKEN :
                {
                  /****************************************************/
                  /*     check if this part was alread processed      */
                  /*               if so show an error                */
                  /****************************************************/
                  if ( !fPageheadProcessed )
                  {
                    usRC = QDPRReadPagehead( psctIDA, &pTok );
                    fPageheadProcessed = TRUE;
                  }
                  else
                  {
                    QDPRSyntaxError( psctIDA, QDPR_SYER_SAME_TAG_TWICE,
                                     QDPR_PAGEHEAD_TOKEN,
                                     QDPR_EMPTY_STRING, 0,
                                     QDPR_NO_QDPR_TAG, QDPR_NO_QDPR_TAG,
                                     &usRC, &(psctIDA->usSyntaxError) );
                  } /* endif */
                }
              break;
              case QDPR_ENTRY_TOKEN :
                {
                  /****************************************************/
                  /*     check if this part was alread processed      */
                  /*               if so show an error                */
                  /****************************************************/
                  if ( !fEntryProcessed )
                  {
                    usRC = QDPRReadEntry( psctIDA, &pTok );
                    fEntryProcessed = TRUE;
                  }
                  else
                  {
                    QDPRSyntaxError( psctIDA, QDPR_SYER_SAME_TAG_TWICE,
                                     QDPR_ENTRY_TOKEN,
                                     QDPR_EMPTY_STRING, 0,
                                     QDPR_NO_QDPR_TAG, QDPR_NO_QDPR_TAG,
                                     &usRC, &(psctIDA->usSyntaxError) );
                  } /* endif */
                }
              break;
              case QDPR_PAGEFOOT_TOKEN :
                {
                  /****************************************************/
                  /*     check if this part was alread processed      */
                  /*               if so show an error                */
                  /****************************************************/
                  if ( !fPagefootProcessed )
                  {
                    usRC = QDPRReadPagefoot( psctIDA, &pTok );
                    fPagefootProcessed = TRUE;
                  }
                  else
                  {
                    QDPRSyntaxError( psctIDA, QDPR_SYER_SAME_TAG_TWICE,
                                     QDPR_PAGEFOOT_TOKEN,
                                     QDPR_EMPTY_STRING, 0,
                                     QDPR_NO_QDPR_TAG, QDPR_NO_QDPR_TAG,
                                     &usRC, &(psctIDA->usSyntaxError) );
                  } /* endif */
                }
              break;
              case QDPR_DICTBACK_TOKEN :
                {
                  /****************************************************/
                  /*     check if this part was alread processed      */
                  /*               if so show an error                */
                  /****************************************************/
                  if ( !fTrailerProcessed )
                  {
                    usRC = QDPRReadTrailer( psctIDA, &pTok );
                    fTrailerProcessed = TRUE;
                  }
                  else
                  {
                    QDPRSyntaxError( psctIDA, QDPR_SYER_SAME_TAG_TWICE,
                                     QDPR_DICTBACK_TOKEN,
                                     QDPR_EMPTY_STRING, 0,
                                     QDPR_NO_QDPR_TAG, QDPR_NO_QDPR_TAG,
                                     &usRC, &(psctIDA->usSyntaxError) );
                  } /* endif */
                }
              break;
              case QDPR_SET_TOKEN :
                {
                  /****************************************************/
                  /*             a set tag has been found             */
                  /****************************************************/
                  usRC = QDPRReadSetTag( psctIDA, &pTok );
                }
              break;
              case QDPR_COMMENT_TOKEN :
                {
                  /****************************************************/
                  /*            comment tag has been found            */
                  /****************************************************/
                  usRC = QDPRReadOverComment( &pTok );

                  /****************************************************/
                  /*   if a syntax error occurred (i.e. the comment   */
                  /*           tag was not properly closed)           */
                  /****************************************************/
                  if ( usRC == QDPR_SYNTAX_ERROR )
                  {
                    QDPRSyntaxError( psctIDA, QDPR_SYER_TAG_NOT_CLOSED,
                                     QDPR_COMMENT_TOKEN, QDPR_EMPTY_STRING, 0,
                                     QDPR_COMMENT_ETOKEN, QDPR_NO_QDPR_TAG,
                                     &usRC, &(psctIDA->usSyntaxError) );
                  } /* endif */
                }
              break;
              case QDPR_COMMENT_ETOKEN :
                {
                  /****************************************************/
                  /*      end-comment tag has been found, i.e. a      */
                  /*           start-comment tag is missing           */
                  /****************************************************/
                  QDPRSyntaxError( psctIDA, QDPR_SYER_MISSING_START_TAG,
                                   QDPR_COMMENT_ETOKEN, QDPR_EMPTY_STRING, 0,
                                   QDPR_COMMENT_TOKEN, QDPR_NO_QDPR_TAG,
                                   &usRC, &(psctIDA->usSyntaxError) );
                }
              break;
              case TEXT_TOKEN :
                {
                  /****************************************************/
                  /*  text outside any tags found, so just ignore it  */
                  /*                  and do nothing                  */
                  /*       (except moving on to the next token)       */
                  /****************************************************/
                  pTok++;
                }
              break;
              default :
                {
                  QDPRSyntaxError( psctIDA, QDPR_SYER_WRONG_TAG_ORDER,
                                   QDPR_NO_QDPR_TAG, QDPR_EMPTY_STRING, 0,
                                   QDPR_NO_QDPR_TAG, QDPR_NO_QDPR_TAG,
                                   &usRC, &(psctIDA->usSyntaxError) );
                }
              break;
            } /* endswitch */
          } /* endwhile */

          /************************************************************/
          /*             check if an entry tag was given              */
          /************************************************************/
          if ( usRC == QDPR_NO_ERROR )
          {
            if ( psctIDA->psctEntryBuffer == NULL )
            {
              QDPRSyntaxError( psctIDA, QDPR_SYER_NO_ENTRY_TAG_FOUND,
                               QDPR_NO_QDPR_TAG, QDPR_EMPTY_STRING, 0,
                               QDPR_NO_QDPR_TAG, QDPR_NO_QDPR_TAG,
                               &usRC, &(psctIDA->usSyntaxError) );
            } /* endif */
          } /* endif */

          psctIDA->usNextThreadStatus = QDPR_ANAST_FINISHED;
        }
      break;

      /****************************************************************/
      /*                  finish the analyze process                  */
      /****************************************************************/
      case QDPR_ANAST_FINISHED :
        {
          /************************************************************/
          /*     if the values for the possible system variables      */
          /*  have not been set initialize them with their defaults   */
          /************************************************************/
          if ( usRC == QDPR_NO_ERROR )
          {
            if ( psctIDA->pszPageEject == NULL )
            {
              if ( UtlAlloc( (PVOID *) &(psctIDA->pszPageEject), 0L,
                             (LONG)max( (strlen(QDPR_DEF_PAGE_EJECT) + 1),
                                        MIN_ALLOC),
                             NOMSG ) )
              {
                strcpy( psctIDA->pszPageEject, QDPR_DEF_PAGE_EJECT );
              }
              else
              {
                usRC = QDPR_NO_MEMORY;
              } /* endif */
            } /* endif */

            if ( usRC == QDPR_NO_ERROR )
            {
              if ( psctIDA->ulPageNumber == 0L )
              {
                psctIDA->ulPageNumber = QDPR_DEF_PAGE_NO;
              } /* endif */

              if ( psctIDA->usPageLength == 0 )
              {
                psctIDA->usPageLength = QDPR_DEF_PAGE_LENGTH;
              } /* endif */

              if ( psctIDA->usLineLength == 0 )
              {
                psctIDA->usLineLength = QDPR_DEF_LINE_LENGTH;
              } /* endif */
            } /* endif */
          } /* endif */

          /************************************************************/
          /*  check if the pageheader and pagefooter take more lines  */
          /*                  than one page is long                   */
          /************************************************************/
          if ( usRC == QDPR_NO_ERROR )
          {
            if ( psctIDA->usCaPageheadLines >= psctIDA->usPageLength )
            {
              QDPRSyntaxError( psctIDA, QDPR_SYER_TOO_LONG_PAGEHEAD,
                               QDPR_NO_QDPR_TAG, QDPR_EMPTY_STRING, 0,
                               QDPR_NO_QDPR_TAG, QDPR_NO_QDPR_TAG,
                               &usRC, &(psctIDA->usSyntaxError) );
            }
            else
            {
              if ( psctIDA->usCaPageheadLines + psctIDA->usCaPagefootLines
                   >= psctIDA->usPageLength )
              {
                QDPRSyntaxError( psctIDA, QDPR_SYER_TOO_LONG_PAGEHEADFOOT,
                                 QDPR_NO_QDPR_TAG, QDPR_EMPTY_STRING, 0,
                                 QDPR_NO_QDPR_TAG, QDPR_NO_QDPR_TAG,
                                 &usRC, &(psctIDA->usSyntaxError) );
              } /* endif */
            } /* endif */
          } /* endif */

          /************************************************************/
          /*           check if only LFs are used or CRLFs            */
          /************************************************************/
          if ( usRC == QDPR_NO_ERROR )
          {
            QDPR_CRLF_USED( psctIDA->fCRLFUsed,
                            psctIDA->psctInOutput->pTokFormatFile->pDataString,
                            pszRun );
          } /* endif */

          psctIDA->usNextThreadStatus = QDPR_PRTST_START;
        }
      break;
    } /* endswitch */

    /******************************************************************/
    /*                     return the error code                      */
    /******************************************************************/
    psctIDA->usThreadError = usRC;

    /******************************************************************/
    /*          set the line number where the error occurred          */
    /******************************************************************/
    if ( usRC != QDPR_NO_ERROR )
    {
      psctIDA->usLineNumber =
        QDPRLineNumbers( psctIDA->psctInOutput->pTokFormatFile->pDataString,
                         pTok->pDataString, 0 ) + 1;
    } /* endif */
  } /* endif */

} /* end of function QDPRAnalyze */



//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name: QDPRPrint                                                    |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//|  VOID QDPRPrint( psctIDA )                                                 |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//|  This function handles the print process of the selected dictionary        |
//|  together with the selected format information file and filter.            |
//|                                                                            |
//|  The program flow of QDPRPrint is not sequential, but it is                |
//|  "status-based" (i.e. the function will receive a status indicator         |
//|  telling it what action is to be taken next). This processing is done in   |
//|  order to not stop processing of messages (e.g. user-interacions) in the   |
//|  QDPRPrintProcessWindow window procedure. The status is passed via the     |
//|  thread structure.                                                         |
//|                                                                            |
//|  The function will perform the following actions. It will open the print   |
//|  destination for printing to it. Then it will print out the header part.   |
//|  Then the pagehead buffer, entry buffer and pagefoot buffers are           |
//|  processed including evaluation of <repeat> and <var> tags. After          |
//|  printing of the buffer has finished and all entries from the dictionary   |
//|  are printed, the trailer part is printed and then the print destination   |
//|  is closed and the procedure finishes.                                     |
//+----------------------------------------------------------------------------+
//|Input parameter:                                                            |
//|  PQDPR_THREAD    psctIDA;     // pointer to thread IDA                     |
//+----------------------------------------------------------------------------+
//|Output parameter:                                                           |
//|  none                                                                      |
//+----------------------------------------------------------------------------+
//|Returncode type: VOID (returncodes are passed via the thread IDA)           |
//+----------------------------------------------------------------------------+
//|Returncodes:                                                                |
//|  none                                                                      |
//+----------------------------------------------------------------------------+
//|Prerequesits:                                                               |
//|  - a correctly set up thread IDA                                           |
//|  - a correctly set up input/output structure                               |
//|  - filled pagehead, entry and pagefoot buffers                             |
//|  - created pagehead, entry, repeat and pagefoot field cross- reference     |
//|    tables                                                                  |
//+----------------------------------------------------------------------------+
//|Side effects:                                                               |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Samples:                                                                    |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                                                                            |
//|  SWITCH on the print status                                                |
//|    CASE QDPR_PRTST_START                                                   |
//|      Initialize everything needed for printing                             |
//|    CASE QDPR_PRTST_PRINT_HEADER                                            |
//|      CALLL QDPRPrintHeader                                                 |
//|    CASE QDPR_PRTST_START_PAGEHEAD                                          |
//|      Initialize everything needed for pagehead printing                    |
//|    CASE QDPR_PRTST_PRINT_PAGEHEAD                                          |
//|      CALL QDPRPrintPagehead                                                |
//|    CASE QDPR_PRTST_START_ENTRY                                             |
//|      Initialize everything needed for entry printing                       |
//|    CASE QDPR_PRTST_PRINT_ENTRY                                             |
//|      CALL QDPRPrintEntry                                                   |
//|    CASE QDPR_PRTST_START_PAGEFOOT                                          |
//|      Initialize everything needed for pagefoot printing                    |
//|    CASE QDPR_PRTST_PRINT_PAGEFOOT                                          |
//|      CALL QDPRPrintPagefoot                                                |
//|    CASE QDPR_PRTST_PRINT_TRAILER                                           |
//|      CALLL QDPRPrintTrailer                                                |
//|    CASE QDPR_PRTST_FINISHED                                                |
//|      Finish print process                                                  |
//+----------------------------------------------------------------------------+

VOID QDPRPrint
(
	PQDPR_THREAD  psctIDA
)
{
  /********************************************************************/
  /*                         local variables                          */
  /********************************************************************/
  USHORT         usRC = QDPR_NO_ERROR;           // error code
  USHORT         usFilterResult;                 // filter result
  ULONG          ulSize;                         // size to allocate
  ULONG          ulTermNo;                       // term number
  HDCB           hDict;                          // dictionary handle
  PQLDB_HTREE    phTree = NULL;                  // tree handle



  if ( psctIDA != NULL )
  {
    switch ( psctIDA->usThreadStatus )
    {
      case QDPR_PRTST_START :
        {
          /************************************************************/
          /*                open the print destination                */
          /************************************************************/
          if ( usRC == QDPR_NO_ERROR )
          {
            usRC = QDPRPrintDestOpen( &( psctIDA->psctPrintDest ),
                                      ( psctIDA->psctInOutput->fPrinterDest ) ?
                                      psctIDA->psctInOutput->szDictName :
                                      psctIDA->psctInOutput->szPrintDest,
                                      psctIDA->psctInOutput->fPrinterDest,
                                      psctIDA->psctInOutput->fReplaceFile,
                                      &(psctIDA->usDosRC) );
          } /* endif */

          /************************************************************/
          /*       reset the current buffer and FCRTs pointers        */
          /************************************************************/
          if ( usRC == QDPR_NO_ERROR )
          {
            psctIDA->psctCurHeaderExt = psctIDA->psctHeaderBuffer;
            psctIDA->pchrHeaderBuffer = psctIDA->psctHeaderBuffer->achrBuffer;
            psctIDA->psctCurPageheadExt = psctIDA->psctPageheadBuffer;
            psctIDA->pchrPageheadBuffer = psctIDA->psctPageheadBuffer->achrBuffer;
            psctIDA->psctCurEntryExt = psctIDA->psctEntryBuffer;
            psctIDA->pchrEntryBuffer = psctIDA->psctEntryBuffer->achrBuffer;
            psctIDA->psctCurPagefootExt = psctIDA->psctPagefootBuffer;
            psctIDA->pchrPagefootBuffer = psctIDA->psctPagefootBuffer->achrBuffer;
            psctIDA->psctCurTrailerExt = psctIDA->psctTrailerBuffer;
            psctIDA->pchrTrailerBuffer = psctIDA->psctTrailerBuffer->achrBuffer;

            psctIDA->psctCurHeaderFCRT = psctIDA->psctHeaderFCRT;
            psctIDA->psctCurPageheadFCRT = psctIDA->psctPageheadFCRT;
            psctIDA->psctCurEntryFCRT = psctIDA->psctEntryFCRT;
            psctIDA->psctCurRepeatFCRT = psctIDA->psctRepeatFCRT;
            psctIDA->psctCurPagefootFCRT = psctIDA->psctPagefootFCRT;
            psctIDA->psctCurTrailerFCRT = psctIDA->psctTrailerFCRT;

            /**********************************************************/
            /*                 reset the line number                  */
            /*                and set the page number                 */
            /**********************************************************/
            psctIDA->usLineNumber = 1;

            /**********************************************************/
            /*   allocate storage for the format IDA, the page buffer */
            /*          and the two process buffer areas in it        */
            /* the page buffer area will look like:                   */
            /*                                  CRLFs                 */
            /*                                   |                    */
            /*         Linelength                                    */
            /*        +-------------------------Â--+                  */
            /* Page   |                         |  |                  */
            /* length |                         |  |                  */
            /*        |                         |  |                  */
            /*        |                         |  |                  */
            /*        |                         |  |    '\0'          */
            /*        +---------------------Â-Â-Á--+      |           */
            /*        |                     | |----------+           */
            /*        +---------------------Á-+                       */
            /*                                                       */
            /*        +-- Page eject string                           */
            /**********************************************************/
            ulSize = (LONG)( psctIDA->usLineLength * psctIDA->usPageLength +
                             psctIDA->usPageLength * 2 +
                             strlen( psctIDA->pszPageEject ) + 1 );

            ulSize = max( ulSize, (LONG)( QDPR_DEF_PAGE_LENGTH *
                                          QDPR_DEF_LINE_LENGTH ) );

            if ( UtlAlloc( (PVOID *) &(psctIDA->psctFormatIDA), 0L,
                           ( ulSize +
                             (LONG)( sizeof( QDPR_FORMAT_IDA ) +
                                     2 * sizeof( QDPR_FORMAT_BUFFERS ) +
                                     2 * sizeof( QDPR_PROCESS_BUFFER ) ) ),
                           NOMSG ) )
            {
              psctIDA->psctFormatIDA->ulSizePageBuffer = ulSize;
              psctIDA->psctFormatIDA->pszPageBuffer =
                                      (PSZ)( psctIDA->psctFormatIDA + 1 );
              psctIDA->psctFormatIDA->pchrLastLF =
                                      psctIDA->psctFormatIDA->pszPageBuffer;
              psctIDA->psctFormatIDA->pchrLastChar =
                                      psctIDA->psctFormatIDA->pszPageBuffer;

              psctIDA->psctFormatIDA->psctEntry = (PQDPR_FORMAT_BUFFERS)
                       ( psctIDA->psctFormatIDA->pszPageBuffer + ulSize );
              psctIDA->psctFormatIDA->psctEntry->psctBuffer =
                       (PQDPR_PROCESS_BUFFER)
                         ( psctIDA->psctFormatIDA->psctEntry + 1 );
              psctIDA->psctFormatIDA->psctEntry->psctCurBufExt =
                  psctIDA->psctFormatIDA->psctEntry->psctBuffer;
              psctIDA->psctFormatIDA->psctEntry->pchrLastWritten =
                  psctIDA->psctFormatIDA->psctEntry->psctBuffer->achrBuffer;
              psctIDA->psctFormatIDA->psctEntry->fNewLineStarts = TRUE;

              psctIDA->psctFormatIDA->psctOther = (PQDPR_FORMAT_BUFFERS)
                       ( psctIDA->psctFormatIDA->psctEntry->psctBuffer + 1 );
              psctIDA->psctFormatIDA->psctOther->psctBuffer =
                       (PQDPR_PROCESS_BUFFER)
                         ( psctIDA->psctFormatIDA->psctOther + 1 );
              psctIDA->psctFormatIDA->psctOther->psctCurBufExt =
                  psctIDA->psctFormatIDA->psctOther->psctBuffer;
              psctIDA->psctFormatIDA->psctOther->pchrLastWritten =
                  psctIDA->psctFormatIDA->psctOther->psctBuffer->achrBuffer;
              psctIDA->psctFormatIDA->psctOther->fNewLineStarts = TRUE;
            }
            else
            {
              usRC = QDPR_NO_MEMORY;
            } /* endif */
          } /* endif */

          if ( usRC == QDPR_NO_ERROR )
          {
            /**********************************************************/
            /*                open the selected filter                */
            /**********************************************************/
            if ( (psctIDA->psctInOutput->szFilter)[0] != NULC )
            {
                                                                /* 2@KIT1138C */
              psctIDA->usExtRC = FiltOpen( psctIDA->psctInOutput->szFilter,
                                           psctIDA->psctInOutput->hDCB,
                                           &(psctIDA->hFilter) );

                                                                /* 2@KIT1138C */
              if ( ( psctIDA->usExtRC != NO_ERROR ) ||
                   ( psctIDA->hFilter == NULL ) )
              {
                usRC = QDPR_READ_DICTIONARY;
                psctIDA->RCType = FILT_RC;                      /* 1@KIT1138A */
              } /* endif */
            } /* endif */
          } /* endif */
          /************************************************************/
          /*    set current print status as previous print status     */
          /*                 and set new print status                 */
          /************************************************************/
          psctIDA->usPreviousPrintStatus = psctIDA->usThreadStatus;
          psctIDA->usNextThreadStatus = QDPR_PRTST_PRINT_HEADER;
        }
      break;

      /****************************************************************/
      /*                    print the header part                     */
      /****************************************************************/
      case QDPR_PRTST_PRINT_HEADER :
        {
          if ( psctIDA->psctHeaderBuffer != NULL )
          {
            usRC = QDPRPrintHeader( psctIDA );
          }
          else
          {
            /**********************************************************/
            /* nothing to do for the header, so set new print status  */
            /**********************************************************/
            psctIDA->usNextThreadStatus = QDPR_PRTST_FILTER;
            psctIDA->usPreviousPrintStatus = QDPR_PRTST_START_PAGEHEAD;
          } /* endif */
        }
      break;

      /****************************************************************/
      /*                   start the pagehead part                    */
      /****************************************************************/
      case QDPR_PRTST_START_PAGEHEAD :
        {
          /************************************************************/
          /*   copy the current template to the first_page template   */
          /************************************************************/
          usRC = QDPRCopyCurrentTemplates( psctIDA->psctCurrentTemplate,
                                psctIDA->psctFirstPageTemplate );

          /************************************************************/
          /*                 set the line number to 1                 */
          /************************************************************/
          psctIDA->usLineNumber = 1;
          psctIDA->usLinesPrinted = 0;

          /************************************************************/
          /*                 reset the format buffer                  */
          /************************************************************/
          if ( usRC == QDPR_NO_ERROR )
          {
            usRC = QDPRResetFormatBuffer( psctIDA->psctFormatIDA->psctOther,
                                          TRUE );
          } /* endif */

          if ( usRC == QDPR_NO_ERROR )
          {
            /**********************************************************/
            /*           check if a pagehead buffer exists            */
            /**********************************************************/
            if ( psctIDA->psctPageheadBuffer != NULL )
            {
              /********************************************************/
              /* set the pointer to the pagehead buffer to the start  */
              /********************************************************/
              psctIDA->pchrPageheadBuffer =
                       psctIDA->psctPageheadBuffer->achrBuffer;
              psctIDA->psctCurPageheadExt = psctIDA->psctPageheadBuffer;

              psctIDA->psctCurPageheadFCRT = psctIDA->psctPageheadFCRT;

              psctIDA->usNextThreadStatus = QDPR_PRTST_PRINT_PAGEHEAD;
            }
            else
            {
              /********************************************************/
              /*  no pagehead buffer exists, so start with the entry  */
              /********************************************************/
              if ( psctIDA->fPrintEntry )
              {
                psctIDA->usNextThreadStatus = QDPR_PRTST_PRINT_ENTRY;
              }
              else
              {
                psctIDA->usNextThreadStatus = QDPR_PRTST_START_ENTRY;
              } /* endif */
            } /* endif */
          } /* endif */
        }
      break;

      /****************************************************************/
      /*                   print the pagehead part                    */
      /****************************************************************/
      case QDPR_PRTST_PRINT_PAGEHEAD :
        {
          usRC = QDPRPrintPagehead( psctIDA );
        }
      break;

      /****************************************************************/
      /*                     start the entry part                     */
      /****************************************************************/
      case QDPR_PRTST_START_ENTRY :
        {
          /************************************************************/
          /*                  reset the entry buffer                  */
          /*   and the entry FCRT and initialize control variables    */
          /************************************************************/
          psctIDA->pchrEntryBuffer =
                   psctIDA->psctEntryBuffer->achrBuffer;
          psctIDA->psctCurEntryExt = psctIDA->psctEntryBuffer;

          psctIDA->psctCurEntryFCRT = psctIDA->psctEntryFCRT;
          psctIDA->psctCurRepeatFCRT = psctIDA->psctRepeatFCRT;

          psctIDA->fPrintEntry = FALSE;
          psctIDA->fEntryBufferEnd = FALSE;
          psctIDA->usEntryTotalLines = 0;
          psctIDA->usEntryLinesPrinted = 0;

          /************************************************************/
          /*                 reset the format buffer                  */
          /************************************************************/
          if ( usRC == QDPR_NO_ERROR )
          {
            usRC = QDPRResetFormatBuffer( psctIDA->psctFormatIDA->psctEntry,
                                          TRUE );
          } /* endif */

          psctIDA->usNextThreadStatus = QDPR_PRTST_PRINT_ENTRY;
        }
      break;

      /****************************************************************/
      /*                    print the entry part                      */
      /****************************************************************/
      case QDPR_PRTST_PRINT_ENTRY :
        {
          usRC = QDPRPrintEntry( psctIDA );
        }
      break;

      /****************************************************************/
      /*                   start the pagefoot part                    */
      /****************************************************************/
      case QDPR_PRTST_START_PAGEFOOT :
        {
          /************************************************************/
          /*   copy the current template to the last_page template    */
          /************************************************************/
          usRC = QDPRCopyCurrentTemplates( psctIDA->psctCurrentTemplate,
                                psctIDA->psctLastPageTemplate );

          psctIDA->usLinesPrinted = 0;

          /************************************************************/
          /*                   reset format buffer                    */
          /************************************************************/
          if ( usRC == QDPR_NO_ERROR )
          {
            usRC = QDPRResetFormatBuffer( psctIDA->psctFormatIDA->psctOther,
                                          TRUE );
          } /* endif */

          if ( usRC == QDPR_NO_ERROR )
          {
            /**********************************************************/
            /*           check if a pagefoot buffer exists            */
            /**********************************************************/
            if ( psctIDA->psctPagefootBuffer != NULL )
            {
              /********************************************************/
              /* set the pointer to the pagefoot buffer to the start  */
              /********************************************************/
              psctIDA->pchrPagefootBuffer =
                       psctIDA->psctPagefootBuffer->achrBuffer;
              psctIDA->psctCurPagefootExt = psctIDA->psctPagefootBuffer;

              psctIDA->psctCurPagefootFCRT = psctIDA->psctPagefootFCRT;

              psctIDA->usNextThreadStatus = QDPR_PRTST_PRINT_PAGEFOOT;
            }
            else
            {
              /********************************************************/
              /*              no pagefoot buffer exists               */
              /*          so print out the page eject string          */
              /*       check if filtering has to be done or not       */
              /********************************************************/
              usRC = QDPRPrintPageEject( psctIDA,
                                  psctIDA->psctFormatIDA->psctOther );

              psctIDA->ulPageNumber ++;
              if ( usRC == QDPR_NO_ERROR )
              {
                if ( psctIDA->usPreviousPrintStatus ==
                     QDPR_PRTST_START_PAGEHEAD )
                {
                  psctIDA->usNextThreadStatus = QDPR_PRTST_FILTER;
                }
                else
                {
                  psctIDA->usNextThreadStatus = QDPR_PRTST_START_PAGEHEAD;
                } /* endif */
              } /* endif */
            } /* endif */
          } /* endif */
        }
      break;

      /****************************************************************/
      /*                   print the pagefoot part                    */
      /****************************************************************/
      case QDPR_PRTST_PRINT_PAGEFOOT :
        {
          usRC = QDPRPrintPagefoot( psctIDA );
        }
      break;

      /****************************************************************/
      /*                    print the trailer part                    */
      /****************************************************************/
      case QDPR_PRTST_PRINT_TRAILER :
        {
          if ( psctIDA->psctTrailerBuffer != NULL )
          {
            usRC = QDPRPrintTrailer( psctIDA );
          }
          else
          {
            /**********************************************************/
            /* nothing to do for the trailer, so set new print status */
            /**********************************************************/
            psctIDA->usNextThreadStatus = QDPR_PRTST_FINISHED;
          } /* endif */
        }
      break;

      case QDPR_PRTST_FILTER :
        {
          /************************************************************/
          /*              destroy the former entry tree               */
          /*                       if it exists                       */
          /************************************************************/
          if ( psctIDA->psctEntry != NULL )
          {
            if ( QLDBDestroyTree( &(psctIDA->psctEntry) ) !=
                 QLDB_NO_ERROR )
            {
              usRC = QDPR_READ_DICTIONARY;
            } /* endif */
          } /* endif */

          /************************************************************/
          /*     loop as long as no filter match is found or the      */
          /*               dictionary is not at the end               */
          /************************************************************/
          usFilterResult = QDPR_NO_FILTER_MATCH;
          while ( ( usRC == QDPR_NO_ERROR ) &&
                  ( usFilterResult == QDPR_NO_FILTER_MATCH ) )
          {
            /**********************************************************/
            /*             get next term from dictionary              */
            /**********************************************************/
            psctIDA->usExtRC = AsdNxtTermW( psctIDA->psctInOutput->hDCB,
                                   psctIDA->psctInOutput->hUCB,
                                   psctIDA->aucTerm, &ulTermNo,
                                   &ulSize, &hDict );

            if ( psctIDA->usExtRC == LX_RC_OK_ASD )
            {
              /********************************************************/
              /*              allocate storage for entry              */
              /*    if the already allocated storage is not enough    */
              /********************************************************/
              if ( psctIDA->ulEntrySize < ulSize )
              {
                if ( UtlAlloc( (PVOID *) &(psctIDA->pucEntry), psctIDA->ulEntrySize,
                               ulSize * sizeof(CHAR_W), NOMSG ) )
                {
                  psctIDA->ulEntrySize = ulSize;
                }
                else
                {
                  usRC = QDPR_NO_MEMORY;
                } /* endif */
              } /* endif */

              /********************************************************/
              /*            get entry data from dictionary            */
              /********************************************************/
              if ( usRC == QDPR_NO_ERROR )
              {
                 //ulSize is no. of char_w's in aucTerm     /* 2@KIT1138C */
                psctIDA->usExtRC = AsdRetEntryW( psctIDA->psctInOutput->hDCB,
                                        psctIDA->psctInOutput->hUCB,
                                        psctIDA->aucTerm, &ulTermNo,
                                        psctIDA->pucEntry, &ulSize,
                                        &hDict );

                if ( psctIDA->usExtRC != LX_RC_OK_ASD )
                {
                  usRC = QDPR_READ_DICTIONARY;
                  psctIDA->RCType = ASD_RC;                     /* 1@KIT1138A */
                } /* endif */
              } /* endif */

              /********************************************************/
              /*        now create a QLDB record/tree and then        */
              /*        convert the record to a full QLDB tree        */
              /********************************************************/
              if ( usRC == QDPR_NO_ERROR )
              {
                phTree = NULL;
                                                                /* 2@KIT1138C */
                psctIDA->usExtRC = QLDBRecordToTree( psctIDA->ausNoOfFields,
                                                     psctIDA->pucEntry,
                                                     ulSize,
                                                     (PVOID *)&phTree );

                                                                /* 2@KIT1138C */
                if ( psctIDA->usExtRC == QLDB_NO_ERROR )
                {
                                                                /* 2@KIT1138C */
                  psctIDA->usExtRC = QLDBRecordToTree( psctIDA->ausNoOfFields,
                                               psctIDA->pucEntry,
                                               0, (PVOID *)&phTree );

                                                                /* 2@KIT1138C */
                  if ( psctIDA->usExtRC != QLDB_NO_ERROR )
                  {
                    usRC = QDPR_READ_DICTIONARY;
                    psctIDA->RCType = LDB_RC;                   /* 1@KIT1138A */
                  } /* endif */
                }
                else
                {
                  usRC = QDPR_READ_DICTIONARY;
                  psctIDA->RCType = LDB_RC;                     /* 1@KIT1138A */
                } /* endif */
              } /* endif */

              /********************************************************/
              /*                    now filter it                     */
              /********************************************************/
              if ( usRC == QDPR_NO_ERROR )
              {
                if ( psctIDA->psctInOutput->szFilter[0] != NULC )
                {
                                                                /* 2@KIT1138C */
                  psctIDA->usExtRC = FiltWork( psctIDA->hFilter,
                                               phTree, &(psctIDA->psctEntry) );

                  QLDBDestroyTree( (PVOID *)&phTree);
                                                                /* 2@KIT1138C */
                  if ( psctIDA->usExtRC == NO_ERROR )
                  {
                    if ( psctIDA->psctEntry != NULL )
                    {
                      usFilterResult = QDPR_FILTER_MATCH;
                    } /* endif */
                  }
                  else
                  {
                                                                /* 2@KIT1138C */
                    if ( psctIDA->usExtRC != ERROR_NO_MORE_FILES )
                    {
                      usRC = QDPR_READ_DICTIONARY;
                      psctIDA->RCType = FILT_RC;                /* 1@KIT1138A */
                    }
                    else
                    {
                      (psctIDA->ulEntriesProcessed)++;
                    } /* endif */
                  } /* endif */
                }
                else
                {
                  /****************************************************/
                  /*  no filter was selected, so use the whole tree   */
                  /****************************************************/
                  psctIDA->psctEntry = phTree;
                  usFilterResult = QDPR_FILTER_MATCH;
                } /* endif */
              } /* endif */
            }
            else
            {
                                                                /* 2@KIT1138C */
              if ( psctIDA->usExtRC == LX_EOF_ASD )
              {
                usFilterResult = QDPR_NO_MORE_ENTRIES;
              }
              else
              {
                usRC = QDPR_READ_DICTIONARY;
                psctIDA->RCType = ASD_RC;                       /* 1@KIT1138A */
              } /* endif */
            } /* endif */
          } /* endwhile */

          if ( usRC == QDPR_NO_ERROR )
          {
            if ( usFilterResult == QDPR_FILTER_MATCH )
            {
              /********************************************************/
              /*   filter match has been found, so copy the found     */
              /*          the entry to the current template           */
              /********************************************************/
              usRC = QDPRCopyCurrentTemplates( psctIDA->psctEntry,
                                    psctIDA->psctCurrentTemplate );

              /********************************************************/
              /*    indicate that at least one entry has bee found    */
              /*                that is to be printed                 */
              /********************************************************/
              psctIDA->fEntriesPrinted = TRUE;

              /********************************************************/
              /*               set the new print status               */
              /********************************************************/
              psctIDA->usNextThreadStatus =
                             psctIDA->usPreviousPrintStatus;
            }
            else
            {
              /********************************************************/
              /*     no more entries there, so print the trailer      */
              /********************************************************/
              psctIDA->usNextThreadStatus = QDPR_PRTST_PRINT_TRAILER;
            } /* endif */
          } /* endif */
        }
      break;

      case QDPR_PRTST_FINISHED :
        {
          /************************************************************/
          /*             close the print destination file             */
          /************************************************************/
          if ( psctIDA->psctPrintDest != NULL )
          {
            REMOVESYMBOL( psctIDA->psctInOutput->szPrintDest );

            usRC = QDPRPrintDestClose( &( psctIDA->psctPrintDest ),
                                       &(psctIDA->usDosRC) );
            psctIDA->psctPrintDest = NULL;
          } /* endif */

          if ( usRC == QDPR_NO_ERROR )
          {
            /**********************************************************/
            /*                 close selected filter                  */
            /**********************************************************/
            if ( psctIDA->hFilter != NULL )
            {
              FiltClose( psctIDA->hFilter );
            } /* endif */
            psctIDA->ulEntriesProcessed = psctIDA->ulDictEntries;
            psctIDA->usNextThreadStatus = QDPR_THREAD_FINISHED;
          } /* endif */
        }
      break;
    } /* endswitch */

    /******************************************************************/
    /*                     return the error code                      */
    /******************************************************************/
    psctIDA->usThreadError = usRC;
  } /* endif */

} /* end of function QDPRPrint */

