//+----------------------------------------------------------------------------+
//|EQFQDPRD.C                                                                  |
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
//|  This file contains the functions for the QDPR print dialog handling       |
//+----------------------------------------------------------------------------+
//|Entry Points:                                                               |
//|                                                                            |
//|                                                                           |
//|+-- status ("H"=Header,"D"=Design,"C"=Code,"T"=Test, " "=complete,          |
//|            "Q"=Quick-and-dirty )                                           |
//+----------------------------------------------------------------------------+
//|Externals:                                                                  |
//|                                                                            |
//|                                                                           |
//|+-- status ("H"=Header,"D"=Design,"C"=Code,"T"=Test, " "=complete,          |
//|            "Q"=Quick-and-dirty )                                           |
//+----------------------------------------------------------------------------+
//|Internals:                                                                  |
//|                                                                            |
//|    QDPRPrintDialog                                                         |
//|    QDPRSetFileDlg                                                          |
//|    QDPRReadProps                                                           |
//|    QDPRWriteProps                                                          |
//|    QDPRSetLastUsedValues                                                   |
//|    QDPRSaveLastUsedValues                                                  |
//|    QDPRProcessFormatFile                                                   |
//|    QDPRScanDescTag                                                         |
//|                                                                           |
//|+-- status ("H"=Header,"D"=Design,"C"=Code,"T"=Test, " "=complete,          |
//|            "Q"=Quick-and-dirty )                                           |
//+----------------------------------------------------------------------------+
//|To be done / known limitations / caveats:                                   |
//|  in QDPRScanDescTag the line number information has to be inserted         |
//|  should be returned within EQFBFastTokenize                                |
//|                                                                            |
//+----------------------------------------------------------------------------+

/**********************************************************************/
/*                           include files                            */
/**********************************************************************/

#define INCL_EQF_ANALYSIS         // analysis functions
#define INCL_EQF_TAGTABLE         // tagtable defines and functions
#define INCL_EQF_FILT             // dictionary filter functions
#define INCL_EQF_ASD              // dictionary access functions (Asd...)
#define INCL_EQF_LDB              // dictionary data encoding functions
#define INCL_EQF_DLGUTILS         // dialog utilities
#define INCL_EQF_DICTPRINT        // dictionary print functions
#include <eqf.h>                  // General Translation Manager include file

#include "OtmDictionaryIF.H"
#include "EQFLDBI.H"              // internal header file for LDB services
#include "EQFQDPRI.H"             // internal header file for dictionary print
#include "EQFQDPR.ID"             // IDs for dictionary print


                                        /******************************/
                                        /* check for name attributes  */
                                        /******************************/
VOID QDPRFillNameFields( PQDPR_DLG_IDA psctIDA, PSZ pszNameBuffer);


//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name: QDPRPrintDialog                                              |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//|  MRESULT EXPENTRY QDPRPrintDialog( hwnd, msg, mp1, mp2 )                   |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//|  This is the dialog procedure for the Dictionary Print dialog.             |
//|  It handles the interaction with the user and sets the values necessary    |
//|  for further processing in the QDPR input/output structure.                |
//+----------------------------------------------------------------------------+
//|Input parameter:                                                            |
//|  HWND    hwnd     handle of window                                         |
//|  USHORT  msg      type of message                                          |
//|  MPARAM  mp1      first message parameter                                  |
//|  MPARAM  mp2      second message parameter                                 |
//+----------------------------------------------------------------------------+
//|Output parameter:                                                           |
//|  none                                                                      |
//+----------------------------------------------------------------------------+
//|Returncode type: MRESULT                                                    |
//+----------------------------------------------------------------------------+
//|Returncodes:                                                                |
//|  TRUE                                                                      |
//|  FALSE                                                                     |
//+----------------------------------------------------------------------------+
//|Prerequesits:                                                               |
//|  - the function expects to receive in the WM_INITDLG message the           |
//|    pointer to the QDPR input/output structure                              |
//+----------------------------------------------------------------------------+
//|Side effects:                                                               |
//|  - the QDPR input/output structure is set correctly                        |
//+----------------------------------------------------------------------------+
//|Samples:                                                                    |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                                                                            |
//|  SWITCH on the message received                                            |
//|    CASE WM_INITDLG                                                         |
//|      Create IDA and save input/output structure to it                      |
//|      Store the pointer to the IDA in dialog window word                    |
//|      Post a WM_INIT_QDPR_DLG message                                       |
//|                                                                            |
//|    CASE WM_EQF_INITIALIZE                                                  |
//|      Activate filter                                                       |
//|      Read QDPR properties (QDPRReadProps)                                  |
//|      Set last-used-values in the dialog (QDPRSetLastUsedValues)            |
//|                                                                            |
//|    CASE WM_COMMAND                                                         |
//|      SWITCH on the control sending the message                             |
//|        CASE "Print" pushbutton                                             |
//|          Save last-used-values (QDPRSaveLastUsedValues)                    |
//|          Set user selections in the input/output structure                 |
//|          Leave the dialog                                                  |
//|                                                                            |
//|        CASE "Cancel" pushbutton                                            |
//|          Leave the dialog without printing and without saving              |
//|            any last-used-values                                            |
//|                                                                            |
//|        CASE "Set" pushbutton                                               |
//|          Load "Set File" dialog (QDPRSetFileDlg)                           |
//|                                                                            |
//|    CASE WM_CONTROL                                                         |
//|      SWITCH on notificaion message send                                    |
//|        CASE BN_CLICKED                                                     |
//|          Set the print destination according to radio button clicked       |
//|        CASE CBN_ENTER                                                      |
//|        CASE CBN_LBSELECT                                                   |
//|          Write the description of the format file into the entry field     |
//|                                                                            |
//|    CASE WM_CLOSE                                                           |
//|      Clean up                                                              |
//|      Dismiss the dialog                                                    |
//+----------------------------------------------------------------------------+

INT_PTR CALLBACK QDPRPRINTDIALOG(

   HWND   hwnd,                        // handle of window
   WINMSG msg,                         // type of message
   WPARAM mp1,                         // first message parameter
   LPARAM mp2 )                        // second message parameter

{
  /********************************************************************/
  /*                         local variables                          */
  /********************************************************************/
  MRESULT              mResult = FALSE; // dlg procedure return value
  USHORT               usRC = QDPR_NO_ERROR; // returncode from functions called
  PSZ                  pReplAddr[1];    // parms table for UtlError
  PQDPR_IN_OUTPUT      psctInOut;       // input/output structure
  PQDPR_DLG_IDA        psctIDA;         // dialog IDA
  PQDPR_PROPS          psctProps = NULL;    // QDPR properties
  USHORT               usMBRc;          // return from msgbox
  SHORT                sRC;             // buffer for SHORT return codes

  switch ( msg )
  {
    case WM_EQF_QUERYID: HANDLEQUERYID( ID_QDPR_PRTDLG, mp2 ); break;

    case WM_INITDLG :
      {
        /**************************************************************/
        /*  Get input/output structure from second message parameter  */
        /**************************************************************/
        psctInOut = (PQDPR_IN_OUTPUT) PVOIDFROMMP2( mp2 );

        /**************************************************************/
        /*                     Create dialog ida                      */
        /**************************************************************/
        if ( UtlAlloc( (PVOID *) &psctIDA, 0L, (LONG)sizeof( QDPR_DLG_IDA ),
                       ERROR_STORAGE ) )
        {
          /************************************************************/
          /* save the pointer to the IDA in the dialog reserved memory*/
          /************************************************************/
          ANCHORDLGIDA( hwnd, psctIDA );

          /************************************************************/
          /*        save the input/output structure in the IDA        */
          /************************************************************/
          psctIDA->psctInOutput = psctInOut;
          psctInOut->usRC = QDPR_NO_ERROR;

          /************************************************************/
          /* Set text limit of description field                      */
          /************************************************************/
          SETTEXTLIMIT( hwnd, ID_QDPR_PRTDLG_DESC_EF, QDPR_MAX_STRING );

          // set text limit for file name to the length of the last
          // used file name field in the print dictionary properties
          SETTEXTLIMIT( hwnd, ID_QDPR_PRTDLG_NAME_EF, MAX_PATH144 );

          /************************************************************/
          /*    post a WM_EQF_INITIALIZE to continue initialization   */
          /************************************************************/
          WinPostMsg( hwnd, WM_EQF_INITIALIZE, NULL, NULL );
        }
        else
        {
          /************************************************************/
          /*         allocation failed, so dismiss the dialog         */
          /*                  and set the returncode                  */
          /************************************************************/
          psctInOut->usRC = QDPR_NO_MEMORY;
          WinPostMsg( hwnd, WM_CLOSE, NULL, NULL );
        } /* endif */
      }
    break;

    case WM_EQF_INITIALIZE :
      {
        /**************************************************************/
        /*                     get pointer to IDA                     */
        /**************************************************************/
        psctIDA = ACCESSDLGIDA( hwnd, PQDPR_DLG_IDA );

        if ( psctIDA != NULL )
        {
          /************************************************************/
          /*             set mouse pointer to hour glass              */
          /************************************************************/
          SETCURSOR( SPTR_WAIT );

          /************************************************************/
          /*   set the printer as print destination                   */
          /************************************************************/
          psctIDA->psctInOutput->fPrinterDest = TRUE;
          SETCHECK_TRUE( hwnd, ID_QDPR_PRTDLG_PRINTER_RB );
          ENABLECTRL( hwnd, ID_QDPR_PRTDLG_SET_PB, FALSE );

          /************************************************************/
          /*     create the path to the format information files      */
          /* use psctIDA->psctInOutput->szPrintDest as work variable  */
          /************************************************************/
          UtlMakeEQFPath( psctIDA->psctInOutput->szPrintDest, NULC,
                          PRT_PATH, NULL );
          strcat( psctIDA->psctInOutput->szPrintDest, BACKSLASH_STR );
          strcat( psctIDA->psctInOutput->szPrintDest, DEFAULT_PATTERN_NAME );
          strcat( psctIDA->psctInOutput->szPrintDest, QDPR_FORMAT_EXT );

          /************************************************************/
          /*     load the file names in the format combobox           */
          /************************************************************/
          if ( UtlLoadFileNames( psctIDA->psctInOutput->szPrintDest,
                                 FILE_NORMAL,
                                 WinWindowFromID( hwnd,
                                                  ID_QDPR_PRTDLG_FORMAT_CBS ),
                                 NAMFMT_NODRV | NAMFMT_NODIR |
                                 NAMFMT_NOROOT )
               == UTLERROR )
          {
            usRC = QDPR_PATH_NOT_FOUND;
          } /* endif */

          if ( usRC == QDPR_NO_ERROR )
          {
            /**********************************************************/
            /*            set name of dictionary to print             */
            /**********************************************************/
            SETTEXT( hwnd, ID_QDPR_PRTDLG_PRINT_TXT2,
                     psctIDA->psctInOutput->szDictName );

            /**********************************************************/
            /*            set dictionary description                  */
            /**********************************************************/
            if ( AsdRetPropPtr( psctIDA->psctInOutput->hUCB,
                                psctIDA->psctInOutput->hDCB,
                                &(psctIDA->pDictProp) ) == LX_RC_OK_ASD )
            {
                  if(psctIDA->pDictProp->szLongDesc[0] == EOS)
                  {
                    OEMTOANSI( psctIDA->pDictProp->szDescription );
                    SETTEXT( hwnd, ID_QDPR_PRTDLG_DICDESCR_EF, psctIDA->pDictProp->szDescription );
                    ANSITOOEM( psctIDA->pDictProp->szDescription );
                  }
                  else
                  {
                    OEMTOANSI( psctIDA->pDictProp->szLongDesc );
                    SETTEXT( hwnd,ID_QDPR_PRTDLG_DICDESCR_EF, psctIDA->pDictProp->szLongDesc );
                    ANSITOOEM( psctIDA->pDictProp->szLongDesc );
                }
            } /* endif */

            /********************************************************/
            /*                   activate filter                    */
            /********************************************************/
            WinSendMsg( WinWindowFromID( hwnd, ID_QDPR_PRTDLG_FILTER_CB ),
                        WM_EQF_FILTSETDICT, NULL,
                        MP2FROMP(psctIDA->psctInOutput->hDCB) );

            /********************************************************/
            /*                 read QDPR properties                 */
            /********************************************************/
            if ( usRC == QDPR_NO_ERROR )
            {
              usRC = QDPRReadProps( &psctProps );
            } /* endif */

            if ( usRC == QDPR_NO_ERROR )
            {
              /******************************************************/
              /*                set last-used values                */
              /******************************************************/
              QDPRSetLastUsedValues( hwnd, ID_QDPR_PRTDLG, psctIDA,
                                     &(psctProps->sctLastUsedValues) );

              if ( psctProps != NULL )
              {
                /****************************************************/
                /*          deallocate property structure           */
                /****************************************************/
                UtlAlloc( (PVOID *) &psctProps, 0L, 0L, NOMSG );
              } /* endif */
            }
            else
            {
              /******************************************************/
              /*    check if only the property file could not be    */
              /*                       opened                       */
              /******************************************************/
              if ( usRC == QDPR_OPEN_PROPFILE )
              {
                /******************************************************/
                /*    select first item in combobox                  */
                /******************************************************/
                CBSELECTITEM( hwnd, ID_QDPR_PRTDLG_FORMAT_CBS, 0 );
                usRC = QDPR_NO_ERROR;
              } /* endif */
            } /* endif */

            // force processing of currently selected format
            QDPRPrintDlgControl( hwnd, ID_QDPR_PRTDLG_FORMAT_CBS, CBN_SELCHANGE );

          } /* endif */

          /************************************************************/
          /*                   reset mouse pointer                    */
          /************************************************************/
          SETCURSOR( SPTR_ARROW );

          /*************************************************************/
          /*                  check the returncode                     */
          /*             if an error occurred dismiss the dialog       */
          /*************************************************************/
          if ( usRC != QDPR_NO_ERROR )
          {
            psctIDA->psctInOutput->usRC = usRC;
            WinPostMsg( hwnd, WM_CLOSE, NULL, NULL );
          }
          else
          {
             SetCtrlFnt (hwnd, GetCharSet(), ID_QDPR_PRTDLG_DICDESCR_EF,
                         ID_QDPR_PRTDLG_DESC_EF );
          } /* endif */
        } /* endif */
      }
    break;

    case WM_COMMAND :
      {
        /**************************************************************/
        /*                     get pointer to IDA                     */
        /**************************************************************/
        psctIDA = ACCESSDLGIDA( hwnd, PQDPR_DLG_IDA );

        if ( psctIDA != NULL )
        {
          switch ( WMCOMMANDID( mp1, mp2 ) )
          {
            case ID_QDPR_PRTDLG_HELP_PB :
              UtlInvokeHelp();
              break;

            case ID_QDPR_PRTDLG_PRINT_PB :
              {
                /******************************************************/
                /*            dictionary property pointer             */
                /*    if an error occurred treat the dictionary as    */
                /*           unprotected and uncopyrighted            */
                /******************************************************/
                if ( AsdRetPropPtr( psctIDA->psctInOutput->hUCB,
                                    psctIDA->psctInOutput->hDCB,
                                    &(psctIDA->pDictProp ) )
                     != LX_RC_OK_ASD )
                {
                  usRC = QDPR_LOAD_DICTIONARY;
                } /* endif */

                /******************************************************/
                /*         check if a copyrighted dictionary          */
                /*                  is to be printed                  */
                /* if so show error message and close dialog, so that */
                /*         user can select another dictionary         */
                /******************************************************/
                if ( ( psctIDA->pDictProp->fCopyRight ) &&
                     ( usRC == QDPR_NO_ERROR ) )
                {
                  pReplAddr[0] = psctIDA->psctInOutput->szDictName;
                  UtlErrorHwnd( QDPR_PRINT_COPYRIGHT_DICT, MB_CANCEL,
                                1, &pReplAddr[0], EQF_ERROR, hwnd );
                  WinPostMsg( hwnd, WM_CLOSE, NULL, NULL );
                  usRC = QDPR_LOAD_DICTIONARY;   // dummy error code !!!
                } /* endif */

                /****************************************************/
                /* check if a protected dictionary is to be printed */
                /*                    to a file                     */
                /* if so show error message and let the dialog stay */
                /*   so that the user may select Printer as print   */
                /*                   destionation                   */
                /****************************************************/
                if ( ( psctIDA->pDictProp->fProtected ) &&
                     !(psctIDA->psctInOutput->fPrinterDest) &&
                     ( usRC == QDPR_NO_ERROR ) )
                {
                  pReplAddr[0] = psctIDA->psctInOutput->szDictName;
                  UtlErrorHwnd( QDPR_PRINT_PROTECTED_DICT, MB_CANCEL,
                                2, &pReplAddr[0], EQF_ERROR, hwnd );
                  usRC = QDPR_LOAD_DICTIONARY;   // dummy error code !!!
                } /* endif */

                /**************************************************/
                /*             save last-used-values              */
                /**************************************************/
                if ( usRC == QDPR_NO_ERROR )
                {
                  usRC = QDPRSaveLastUsedValues( hwnd, ID_QDPR_PRTDLG, psctIDA );
                } /* endif */

                /**************************************************/
                /*  Get user confirmation for existing files      */
                /**************************************************/
                if ( usRC == QDPR_NO_ERROR )
                {
                  if ( !psctIDA->psctInOutput->fPrinterDest &&
                       UtlFileExist( psctIDA->psctInOutput->szPrintDest ) )
                  {
                    /********************************************/
                    /*    file exists => overwrite yes/no?      */
                    /********************************************/
                    pReplAddr[0] = psctIDA->psctInOutput->szPrintDest;
                    usMBRc = UtlErrorHwnd( FILE_EXISTS, MB_YESNO,
                               1, &pReplAddr[0], EQF_QUERY, hwnd );
                    if ( usMBRc == MBID_NO )
                    {
                      usRC = QDPR_USER_ERROR;
                      SETFOCUS( hwnd, ID_QDPR_PRTDLG_NAME_EF );
                    } /* endif */
                  } /* endif */
                } /* endif */

                /******************************************************/
                /* Check lock and lock dictionary being printed       */
                /******************************************************/
                if ( usRC == QDPR_NO_ERROR )
                {
                  /****************************************************/
                  /* Build dictionary property name                   */
                  /****************************************************/
                  UtlMakeEQFPath( psctIDA->szWorkBuffer, NULC, SYSTEM_PATH, NULL );
                  strcat( psctIDA->szWorkBuffer, BACKSLASH_STR );
                  strcat( psctIDA->szWorkBuffer, psctIDA->pDictProp->PropHead.szName );

                  /****************************************************/
                  /* Check if dictionary is locked by a query for     */
                  /* a symbol consisting of the dictionary property   */
                  /* name                                             */
                  /****************************************************/
                  sRC = QUERYSYMBOL( psctIDA->szWorkBuffer );

                  /****************************************************/
                  /* Handle locked dictionary or set lock for         */
                  /* dictionary                                       */
                  /****************************************************/
                  if ( sRC != -1 )
                  {
                    pReplAddr[0] = psctIDA->psctInOutput->szDictName,
                    UtlError( ERROR_DICT_LOCKED, MB_CANCEL, 1, pReplAddr,
                              EQF_ERROR );
                    usRC = QDPR_LOAD_DICTIONARY;   // dummy error code !!!
                  }
                  else
                  {
                    SETSYMBOL( psctIDA->szWorkBuffer );
                    psctIDA->psctInOutput->fDictHasBeenLocked = TRUE;
                  } /* endif */
                } /* endif */

                /******************************************************/
                /* Check if output file is locked                     */
                /******************************************************/
                if ( usRC == QDPR_NO_ERROR )
                {
                  if ( !psctIDA->psctInOutput->fPrinterDest )
                  {
                    SHORT  sRC = QUERYSYMBOL(psctIDA->psctInOutput->szPrintDest);
                    if ( sRC != -1 )
                    {
                      pReplAddr[0] = psctIDA->psctInOutput->szPrintDest;
                      UtlError( ERROR_PROP_OBJECTBUSY, MB_CANCEL, 1, pReplAddr,
                                EQF_ERROR );
                      usRC = QDPR_LOAD_DICTIONARY;   // dummy error code !!!
                    }
                    else
                    {
                      SETSYMBOL( psctIDA->psctInOutput->szPrintDest );
                    } /* endif */
                  } /* endif */
                } /* endif */

                /******************************************************/
                /* Start printing                                     */
                /******************************************************/
                if ( usRC == QDPR_NO_ERROR )
                {
                  psctIDA->psctInOutput->fStartPrinting = TRUE;
                  if ( !(psctIDA->psctInOutput->fPrinterDest) )
                  {
                    psctIDA->psctInOutput->fReplaceFile = TRUE; // default
                  } /* endif */

                  WinPostMsg( hwnd, WM_CLOSE, NULL, NULL );
                }
                else if ( psctIDA->psctInOutput->fDictHasBeenLocked )
                {
                  REMOVESYMBOL( psctIDA->szWorkBuffer );
                  psctIDA->psctInOutput->fDictHasBeenLocked = FALSE;
                } /* endif */

                /******************************************************/
                /* Reset error code if an user error occured          */
                /******************************************************/
                if ( usRC == QDPR_USER_ERROR )
                {
                  usRC = QDPR_NO_ERROR;
                } /* endif */
              }
            break;

            case ID_QDPR_PRTDLG_CANCEL_PB :
            case DID_CANCEL :
              {
                psctIDA->psctInOutput->fStartPrinting = FALSE;
                WinPostMsg( hwnd, WM_CLOSE, NULL, NULL );
              }
            break;

            case ID_QDPR_PRTDLG_SET_PB :
              {
                /******************************************************/
                /*             pop up the set file dialog             */
                /******************************************************/
                if ( !psctIDA->psctInOutput->fPrinterDest )
                {

                  {
                    OPENFILENAME ofn;
                    BOOL fOk = TRUE;

                    TCHAR szTitle[64];
                    TCHAR szT[MAX_LONGPATH];

                    static TCHAR szFilterLoad[] = TEXT("Print Format (*.*)\0*.*\0\0");

                    ofn.lStructSize                 = sizeof(ofn);
                    ofn.hInstance                   = (HINSTANCE)UtlQueryULong(QL_HAB);
                    ofn.lpstrFilter                 = szFilterLoad;
                    ofn.lpstrCustomFilter           = NULL;
                    ofn.nMaxCustFilter              = 0;
                    ofn.nFilterIndex                = 1;
                    ofn.lpstrFileTitle              = szTitle;               // output
                    ofn.nMaxFileTitle               = sizeof(szTitle);
                    ofn.lpstrInitialDir             = NULL;
                    ofn.lpstrTitle                  = "Select File" ;
                    ofn.nFileOffset                 = 0;
                    ofn.nFileExtension              = 0;
                    ofn.lCustData                   = 0L;
                    ofn.lpfnHook                    = NULL;
                    ofn.lpTemplateName              = NULL;
                    ofn.hwndOwner                   = hwnd;
                    ofn.lpstrFile                   = szT;
                    ofn.nMaxFile                    = sizeof(szT);
                    ofn.Flags                       = OFN_HIDEREADONLY |
                                                      OFN_NONETWORKBUTTON ;

                    ofn.lpstrDefExt                 = NULL;

                    szTitle[0] = TEXT('\0');  // output
                    szT[0] = TEXT('\0');      // input
                    QUERYTEXT (hwnd, ID_QDPR_PRTDLG_NAME_EF, szT);


                    fOk = GetSaveFileName(&ofn);

                    if (fOk )
                    {

                      SETTEXT (hwnd, ID_QDPR_PRTDLG_NAME_EF, szT);
                      QUERYTEXT( hwnd, ID_QDPR_PRTDLG_NAME_EF,
                                 psctIDA->psctInOutput->szPrintDest );
                    } /* endif */


                  }
                } /* endif */
              }
            break;
          case ID_QDPR_PRTDLG_PRINTER_RB :
          case ID_QDPR_PRTDLG_FILE_RB:
          case ID_QDPR_PRTDLG_FORMAT_CBS :
            mResult = QDPRPrintDlgControl( hwnd, WMCOMMANDID( mp1, mp2 ),
                                           WMCOMMANDCMD( mp1, mp2 ) );
            break;

          default:
            IsFilterMessage( WinWindowFromID( hwnd, ID_QDPR_PRTDLG_FILTER_CB ),
                             msg, mp1, mp2 );

          } /* endswitch */
        } /* endif */
      }
    break;

    case WM_CLOSE :
      {
        /**************************************************************/
        /*                       deallocate IDA                       */
        /**************************************************************/
        psctIDA = ACCESSDLGIDA( hwnd, PQDPR_DLG_IDA );

        if ( psctIDA != NULL )
        {
          if ( psctIDA->apszFormatFilenames != NULL )
          {
            UtlAlloc( (PVOID *) (PVOID *)&(psctIDA->apszFormatFilenames), 0L, 0L, NOMSG );
          } /* endif */

          UtlAlloc( (PVOID *) &psctIDA, 0L, 0L, NOMSG );
        } /* endif */

        DelCtrlFont (hwnd, ID_QDPR_PRTDLG_DICDESCR_EF);
        /**************************************************************/
        /*                     dismiss the dialog                     */
        /**************************************************************/
        WinDismissDlg( hwnd, TRUE );
      }
    break;

    default :
      {
        mResult = WinDefDlgProc( hwnd, msg, mp1, mp2 );
      }
    break;
  } /* endswitch */


  return( mResult );

} /* end of function QDPRPrintDialog */


//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name: QDPRPrintDlgControl                                          |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//|  This function handles the WM_CONTROL message for the QDPRPrintDlg         |
//+----------------------------------------------------------------------------+
MRESULT QDPRPrintDlgControl
(
   HWND   hwnd,                        // dialog handle
   SHORT  sId,                         // id in action
   SHORT  sNotification                // notification
)
{
  MRESULT              mResult = FALSE; // dlg procedure return value
  USHORT               usRC = QDPR_NO_ERROR; // returncode from functions called
  PQDPR_DLG_IDA        psctIDA;         // dialog IDA

  /**************************************************************/
  /*                     get pointer to IDA                     */
  /**************************************************************/
  psctIDA = ACCESSDLGIDA( hwnd, PQDPR_DLG_IDA );

  if ( psctIDA != NULL )
  {
    switch ( sId )
    {
      case ID_QDPR_PRTDLG_PRINTER_RB :
        switch ( sNotification )
        {
          case BN_CLICKED :
            psctIDA->psctInOutput->fPrinterDest = TRUE;
            ENABLECTRL( hwnd, ID_QDPR_PRTDLG_SET_PB, FALSE );
            ENABLECTRL( hwnd, ID_QDPR_PRTDLG_NAME_EF, FALSE );
            ENABLECTRL( hwnd, ID_QDPR_PRTDLG_FILENAME_TEXT, FALSE );
            break;
        } /* endswitch */
        break;

      case ID_QDPR_PRTDLG_FILE_RB:
        switch ( sNotification )
        {
          case BN_CLICKED :
            psctIDA->psctInOutput->fPrinterDest = FALSE;
            ENABLECTRL( hwnd, ID_QDPR_PRTDLG_SET_PB, TRUE );
            ENABLECTRL( hwnd, ID_QDPR_PRTDLG_NAME_EF, TRUE );
            ENABLECTRL( hwnd, ID_QDPR_PRTDLG_FILENAME_TEXT, TRUE );
            break;
        } /* endswitch */
        break;

      case ID_QDPR_PRTDLG_FORMAT_CBS :
        switch ( sNotification )
        {
          case CBN_SELCHANGE :
            {
              SHORT sItem;             // combobox item index

              /************************************************/
              /*  the format file has been changed            */
              /*   so show the new description value in the   */
              /*              description field               */
              /************************************************/
              sItem = CBQUERYSELECTION( hwnd, ID_QDPR_PRTDLG_FORMAT_CBS );
              if ( sItem != LIT_NONE )
              {
                CBQUERYITEMTEXT( hwnd, ID_QDPR_PRTDLG_FORMAT_CBS, sItem,
                                 psctIDA->psctInOutput->szFormatFile );
              }
              else
              {
                psctIDA->psctInOutput->szFormatFile[0] = NULC;
              } /* endif */

              if ( (psctIDA->psctInOutput->szFormatFile)[0] != NULC )
              {
                /**********************************************/
                /*   load and process the format info file    */
                /**********************************************/
                usRC = QDPRProcessFormatFile(psctIDA);

                if ( usRC == QDPR_NO_ERROR )
                {
                  /************************************************/
                  /* do not cancel dlg if error occurs in         */
                  /* this subroutine -> do not set usRc           */
                  /************************************************/
                  QDPRScanDescTag( psctIDA->psctInOutput->szFormatFile,
                                   psctIDA->szWorkBuffer, QDPR_MAX_STRING,
                                   psctIDA->psctInOutput->pTokFormatFile );
                } /* endif */
              }
              else
              {
                psctIDA->szWorkBuffer[0] = EOS;
              } /* endif */

              if ( usRC == QDPR_NO_ERROR )
              {
                SETTEXT( hwnd, ID_QDPR_PRTDLG_DESC_EF, psctIDA->szWorkBuffer );
              } /* endif */
            }
            break;
        } /* endswitch */
        break;
    } /* endswitch */
  } /* endif */

  return ( mResult );
} /* end of function QDPRPrintDlgControl */


//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name: QDPRReadProps                                                |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//|  USHORT QDPRReadProps( ppsctProps )                                        |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//|  This function opens the QDPR property file and reads the content into     |
//|  a QDPR property structure allocated by the function.                      |
//|                                                                            |
//|  If an error occurrs the pointer to the property structure returned is     |
//|  NULL.                                                                     |
//+----------------------------------------------------------------------------+
//|Input parameter:                                                            |
//|  none                                                                      |
//+----------------------------------------------------------------------------+
//|Output parameter:                                                           |
//|  PQDPR_PROPS *ppsctProps;          // pointer to the property structure    |
//+----------------------------------------------------------------------------+
//|Returncode type: USHORT                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:                                                                |
//|  QDPR_NO_ERROR         - everything was OK                                 |
//|  QDPR_NO_MEMORY        - not enough memory                                 |
//|  QDPR_OPEN_PROPFILE    - property file could not be opened                 |
//+----------------------------------------------------------------------------+
//|Prerequesits:                                                               |
//|  none                                                                      |
//+----------------------------------------------------------------------------+
//|Side effects:                                                               |
//|  none                                                                      |
//+----------------------------------------------------------------------------+
//|Samples:                                                                    |
//|                                                                            |
//|  usRC = QDPRReadProps( &psctProps );                                       |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                                                                            |
//|  Build path and filename of property file                                  |
//|  Load the property file into the property structure (UtlLoadFile)          |
//+----------------------------------------------------------------------------+

USHORT QDPRReadProps
(
  PQDPR_PROPS *ppsctProps          // pointer to the property structure
)
{
  /********************************************************************/
  /*                         local variables                          */
  /********************************************************************/
  USHORT       usRC = QDPR_NO_ERROR;                 // function returncode
  USHORT       usBytesRead = 0;                      // no of bytes read
  PSZ          pszPropFile = NULL;                   // property filename
  PSZ          pszTemp = NULL;                       // work buffer

  *ppsctProps = NULL;

  /********************************************************************/
  /*                    allocate temporary storage                    */
  /********************************************************************/
  if ( UtlAlloc( (PVOID *) &pszPropFile, 0L, (LONG)( 2 * QDPR_MAX_PATH_FILENAME ),
                 ERROR_STORAGE ) )
  {
    pszTemp = (PSZ)( pszPropFile + QDPR_MAX_PATH_FILENAME );
  }
  else
  {
    usRC = QDPR_NO_MEMORY;
  } /* endif */

  /********************************************************************/
  /*                   build path to property file                    */
  /********************************************************************/
  if ( usRC == QDPR_NO_ERROR )
  {
    UtlMakeEQFPath( pszTemp, NULC, PROPERTY_PATH, NULL );
    sprintf( pszPropFile, "%s\\%s", pszTemp, QDPR_PROP_FILENAME );
  } /* endif */

  /********************************************************************/
  /*                check if the property file exists                 */
  /********************************************************************/
  if ( ( usRC == QDPR_NO_ERROR ) && UtlFileExist( pszPropFile ) )
  {
    /******************************************************************/
    /* load the content of the property file into the property struct */
    /******************************************************************/
    if ( !( UtlLoadFile( pszPropFile, (PVOID *)ppsctProps, &usBytesRead,
                         FALSE, FALSE ) ) )
    {
      /****************************************************************/
      /*               error occurred during file load                */
      /*           deallocate allocated property structure            */
      /****************************************************************/
      if ( *ppsctProps != NULL )
      {
        UtlAlloc( (PVOID *) ppsctProps, 0L, 0L, NOMSG );
      } /* endif */
      *ppsctProps = NULL;
      usRC = QDPR_OPEN_PROPFILE;
    } /* endif */
  }
  else
  {
    usRC = QDPR_OPEN_PROPFILE;
  } /* endif */

  if ( usRC == QDPR_NO_ERROR )
  {
    /******************************************************************/
    /*            check if the number of bytes read is ok             */
    /******************************************************************/
    if ( ( usBytesRead != sizeof( QDPR_PROPS ) ) && ( usBytesRead != 0 ) )
    {
      /****************************************************************/
      /*           deallocate allocated property structure            */
      /****************************************************************/
      if ( *ppsctProps != NULL )
      {
        UtlAlloc( (PVOID *) ppsctProps, 0L, 0L, NOMSG );
      } /* endif */
      *ppsctProps = NULL;
      usRC = QDPR_OPEN_PROPFILE;
    } /* endif */
  } /* endif */

  /********************************************************************/
  /*              deallocate temporary allocated storage              */
  /********************************************************************/
  if ( pszPropFile != NULL )
  {
    UtlAlloc( (PVOID *) &pszPropFile, 0L, 0L, NOMSG );
  } /* endif */

  return( usRC );

} /* end of function QDPRReadProps */



//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name: QDPRWriteProps                                               |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//|  USHORT QDPRWriteProps( psctProps )                                        |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//|  This function opens the QDPR property file and writes the content of      |
//|  the property structure to the file.                                       |
//+----------------------------------------------------------------------------+
//|Input parameter:                                                            |
//|  PQDPR_PROPS psctProps;          // pointer to the property structure      |
//+----------------------------------------------------------------------------+
//|Output parameter:                                                           |
//|  none                                                                      |
//+----------------------------------------------------------------------------+
//|Returncode type: USHORT                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:                                                                |
//|  QDPR_NO_ERROR         - everything was OK                                 |
//|  QDPR_NO_MEMORY        - not enough memory                                 |
//|  QDPR_OPEN_PROPFILE    - property file could not be opened                 |
//+----------------------------------------------------------------------------+
//|Prerequesits:                                                               |
//|  - a valid and filled property structure                                   |
//+----------------------------------------------------------------------------+
//|Side effects:                                                               |
//|  none                                                                      |
//+----------------------------------------------------------------------------+
//|Samples:                                                                    |
//|                                                                            |
//|  usRC = QDPRWriteProps( psctProps );                                       |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                                                                            |
//|  Build path and filename of property file                                  |
//|  Write the property structure to the property file (UtlWriteFile)          |
//+----------------------------------------------------------------------------+

USHORT QDPRWriteProps
(
  PQDPR_PROPS psctProps          // pointer to the property structure
)
{
  /********************************************************************/
  /*                         local variables                          */
  /********************************************************************/
  USHORT       usRC = QDPR_NO_ERROR;                 // function returncode
  PSZ          pszPropFile = NULL;                   // property filename
  PSZ          pszTemp = NULL;                       // work buffer

  /********************************************************************/
  /*                    allocate temporary storage                    */
  /********************************************************************/
  if ( UtlAlloc( (PVOID *) &pszPropFile, 0L, (LONG)( 2 * QDPR_MAX_PATH_FILENAME ),
                 ERROR_STORAGE ) )
  {
    pszTemp = (PSZ)( pszPropFile + QDPR_MAX_PATH_FILENAME );
  }
  else
  {
    usRC = QDPR_NO_MEMORY;
  } /* endif */

  /********************************************************************/
  /*                   build path to property file                    */
  /********************************************************************/
  if ( usRC == QDPR_NO_ERROR )
  {
    UtlMakeEQFPath( pszTemp, NULC, PROPERTY_PATH, NULL );
    sprintf( pszPropFile, "%s\\%s", pszTemp, QDPR_PROP_FILENAME );
  } /* endif */

  /********************************************************************/
  /* write the contents of the property structure to the property file*/
  /********************************************************************/
  if ( usRC == QDPR_NO_ERROR )
  {
    if ( !( UtlWriteFile( pszPropFile, sizeof( QDPR_PROPS ), psctProps,
                          FALSE ) ) )
    {
      usRC = QDPR_OPEN_PROPFILE;
    } /* endif */
  } /* endif */

  /********************************************************************/
  /*              deallocate temporary allocated storage              */
  /********************************************************************/
  if ( pszPropFile != NULL )
  {
    UtlAlloc( (PVOID *) &pszPropFile, 0L, 0L, NOMSG );
  } /* endif */

  return( usRC );

} /* end of function QDPRWriteProps */



//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name: QDPRSetLastUsedValues                                        |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//|  USHORT QDPRSetLastUsedValues( hwndDlg, usIDDlg, psctIDA, psctLUValues )   |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//|  This function sets the last-used-values in the specified dialog.          |
//+----------------------------------------------------------------------------+
//|Input parameter:                                                            |
//|  HWND                 hwndDlg;       // dialog handle                      |
//|  USHORT               usIDDlg;       // dialog ID                          |
//|  PQDPR_DLG_IDA        psctIDA;       // dialog IDA                         |
//|  PQDPR_LAST_USED_VALS psctLUValues;  // last-used-values structure         |
//+----------------------------------------------------------------------------+
//|Output parameter:                                                           |
//|  none                                                                      |
//+----------------------------------------------------------------------------+
//|Returncode type: USHORT                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:                                                                |
//|  QDPR_NO_ERROR         - everything was OK                                 |
//+----------------------------------------------------------------------------+
//|Prerequesits:                                                               |
//|  - a valid and filled last-used-values structure                           |
//+----------------------------------------------------------------------------+
//|Side effects:                                                               |
//|  - sets the pushbuttons, radiobuttons, listboxes and comboboxes to         |
//|    the last-used-values                                                    |
//+----------------------------------------------------------------------------+
//|Samples:                                                                    |
//|                                                                            |
//|  usRC = QDPRSetLastUsedValues( hwnd, ID_MAIN_DLG, psctIDA, psctLUVals );   |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                                                                            |
//|  SWITCH on dialog ID                                                       |
//|    CASE "Dictionary Print" dialog                                          |
//|      Mark last-used filter                                                 |
//|      Mark last-used format info file                                       |
//|      IF "Printer" radiobutton was last used                                |
//|        Select "Printer" radiobutton                                        |
//|      ELSE                                                                  |
//|        Select "File" radiobutton                                           |
//+----------------------------------------------------------------------------+
USHORT QDPRSetLastUsedValues(

  HWND                 hwndDlg,       // dialog handle
  USHORT               usIDDlg,       // dialog ID
  PQDPR_DLG_IDA        psctIDA,       // dialog IDA
  PQDPR_LAST_USED_VALS psctLUValues ) // last-used-values structure

{
  /********************************************************************/
  /*                         local variables                          */
  /********************************************************************/
  USHORT       usRC = QDPR_NO_ERROR;              // function returncode
  SHORT        sIndex;                            // item index of listbox

  if ( ( psctIDA != NULL ) && ( psctLUValues != NULL ) )
  {
    /******************************************************************/
    /*   check for which dialog the last-used-values have to be set   */
    /******************************************************************/
    switch ( usIDDlg )
    {
      case ID_QDPR_PRTDLG :
        {

          /************************************************************/
          /*                   Set last-used filter                   */
          /************************************************************/
          SETTEXT( hwndDlg, ID_QDPR_PRTDLG_FILTER_CB, psctLUValues->szFilter );

          /************************************************************/
          /*              Set last-used format info file              */
          /************************************************************/
          CBSEARCHSELECT( sIndex, hwndDlg, ID_QDPR_PRTDLG_FORMAT_CBS,
                          psctLUValues->szFormatFile );

          /************************************************************/
          /*             set last-used print destination              */
          /************************************************************/
          if ( psctLUValues->fPrintDest )
          {
            CLICK( hwndDlg, ID_QDPR_PRTDLG_PRINTER_RB );
            SETCHECK_FALSE ( hwndDlg, ID_QDPR_PRTDLG_FILE_RB);
          }
          else
          {
            CLICK( hwndDlg, ID_QDPR_PRTDLG_FILE_RB );
            SETCHECK_FALSE( hwndDlg, ID_QDPR_PRTDLG_PRINTER_RB );
          } /* endif */
          SETTEXT( hwndDlg, ID_QDPR_PRTDLG_NAME_EF, psctLUValues->szFileName );
          strcpy( psctIDA->psctInOutput->szPrintDest, psctLUValues->szFileName );
        }
      break;

    } /* endswitch */
  } /* endif */

  return( usRC );

} /* end of function QDPRSetLastUsedValues */



//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name: QDPRSaveLastUsedValues                                       |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//|  USHORT QDPRSaveLastUsedValues( hwndDlg, usIDDlg, psctIDA )                |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//|  This function saves the values currently set in the specified dialog.     |
//|                                                                            |
//|  It also check for the validity of these values (e.g. if the user has      |
//|  selected a filter or format or if the file or printer name is correct).   |
//|  If an error occurrs the old last-used-values will not be destroyed and    |
//|  an error message is displayed.                                            |
//+----------------------------------------------------------------------------+
//|Input parameter:                                                            |
//|  HWND                 hwndDlg;       // dialog handle                      |
//|  USHORT               usIDDlg;       // dialog ID                          |
//|  PQDPR_DLG_IDA        psctIDA;       // dialog IDA                         |
//+----------------------------------------------------------------------------+
//|Output parameter:                                                           |
//|  none                                                                      |
//+----------------------------------------------------------------------------+
//|Returncode type: USHORT                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:                                                                |
//|  QDPR_NO_ERROR         - everything was OK                                 |
//|  QDPR_USER_ERROR       - the user made an error in his selections          |
//+----------------------------------------------------------------------------+
//|Prerequesits:                                                               |
//|  none                                                                      |
//+----------------------------------------------------------------------------+
//|Side effects:                                                               |
//|  - the property file is updated                                            |
//+----------------------------------------------------------------------------+
//|Samples:                                                                    |
//|                                                                            |
//|  usRC = QDPRSaveLastUsedValues( hwndDlg, ID_MAIN_DLG, psctIDA );           |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                                                                            |
//|  SWITCH on dialog ID                                                       |
//|    CASE "Dictionary Print" dialog                                          |
//|      Save format information file name                                     |
//|      Save filter name                                                      |
//|      check format file against filter                                      |
//|      IF "Printer" radiobutton is selected                                  |
//|        Save that "Printer" radiobutton was last used                       |
//|      ELSE                                                                  |
//|        Save that "File" radiobutton was last used                          |
//|                                                                            |
//|    CASE "Set File" dialog                                                  |
//|      Check if the filename is valid                                        |
//|      Save filename                                                         |
//+----------------------------------------------------------------------------+


USHORT QDPRSaveLastUsedValues(

  HWND                 hwndDlg,       // dialog handle
  USHORT               usIDDlg,       // dialog ID
  PQDPR_DLG_IDA        psctIDA )      // dialog IDA

{
  /********************************************************************/
  /*                         local variables                          */
  /********************************************************************/
  USHORT       usRC = QDPR_NO_ERROR;           // function returncode
  PSZ          pReplAddr[3];                   // params table for UtlError
  PQDPR_PROPS  psctProps;                      // property structure
  USHORT       usDosRC;                        // return code of Dos functions
  USHORT       usAttribute;                    // buffer for file attributes
  HFILTER      hFilter;                          // filter handle
  PSZ          pszNameBuffer;                  // collects fieldnames
  PSZ          pBufferRc;                      // ptr to incorrect fieldname
  USHORT       usTempRC;                       // function return code
  USHORT       usMBRc;                         // rc from Utlerror
  PPROPDICTIONARY pDictProp;                   // ptr to dictionary properties
                                                                /* 2@KIT1239A */
  PSZ          pszFile;                        // points to filename
                                                                /* 5@KIT1240A */
   USHORT      usAction;                       // action performed by DosOpen
   HFILE       hFile;                          // buffer for file handles

  if ( psctIDA != NULL )
  {
    /******************************************************************/
    /*                  Read the current properties                   */
    /******************************************************************/
    usRC = QDPRReadProps( &psctProps );

    /******************************************************************/
    /* check if an error occurred and if only the property file could */
    /*   not be opened then allocate last-used values structure by    */
    /*                             myself                             */
    /******************************************************************/
    if ( usRC != QDPR_NO_ERROR )
    {
      if ( usRC == QDPR_OPEN_PROPFILE )
      {
        if ( UtlAlloc( (PVOID *) &psctProps, 0L, (LONG)sizeof( QDPR_PROPS ),
                       ERROR_STORAGE ) )
        {
          usRC = QDPR_NO_ERROR;
        }
        else
        {
          usRC = QDPR_NO_MEMORY;
        } /* endif */
      } /* endif */
    } /* endif */

    if ( usRC == QDPR_NO_ERROR )
    {
      /****************************************************************/
      /* check for which dialog the last-used-values have to be saved */
      /****************************************************************/
      switch ( usIDDlg )
      {
        case ID_QDPR_PRTDLG :
          {
            /**********************************************************/
            /*    get format information file name in input/output    */
            /*      structure and in last-used-values structure       */
            /**********************************************************/
            if ( usRC == QDPR_NO_ERROR )
            {
              QUERYTEXT( hwndDlg, ID_QDPR_PRTDLG_FORMAT_CBS,
                         psctProps->sctLastUsedValues.szFormatFile );

              if ( (psctProps->sctLastUsedValues.szFormatFile)[0] != NULC )
              {
                strcpy( psctIDA->psctInOutput->szFormatFile,
                        psctProps->sctLastUsedValues.szFormatFile );
              }
              else
              {
                UtlErrorHwnd( QDPR_NO_FORMAT_SELECTED, MB_CANCEL, 0, NULL,
                              EQF_ERROR, hwndDlg );

                usRC = QDPR_USER_ERROR;
              } /* endif */
            } /* endif */

            /**********************************************************/
            /*       get filter name in input/output structure        */
            /*           and in last-used-values structure            */
            /**********************************************************/
            QUERYTEXT( hwndDlg, ID_QDPR_PRTDLG_FILTER_CB,
                       psctIDA->psctInOutput->szFilter );

            strcpy( psctProps->sctLastUsedValues.szFilter,
                    psctIDA->psctInOutput->szFilter );

            /**********************************************************/
            /* Check if filter checkbox is checked but no filter      */
            /* has been selected                                      */
            /**********************************************************/
            if ( usRC == QDPR_NO_ERROR )
            {
              if ( QUERYCHECK( hwndDlg, ID_FILTER_CHECKBOX ) &&
                   (psctIDA->psctInOutput->szFilter[0] == EOS) )
              {
                UtlErrorHwnd( QDPR_NO_FILTER_SELECTED, MB_CANCEL, 0, NULL,
                              EQF_ERROR, hwndDlg );
                usRC = QDPR_USER_ERROR;
                SETFOCUS( hwndDlg, ID_FILTER_CHECKBOX );
              } /* endif */
            } /* endif */

            /**********************************************************/
            /* Check filter                                           */
            /**********************************************************/
            if ( (usRC == QDPR_NO_ERROR) &&
                 (psctIDA->psctInOutput->szFilter[0] != EOS) )
            {
              hFilter = NULL;
              usTempRC = FiltOpen( psctIDA->psctInOutput->szFilter,
                                   psctIDA->psctInOutput->hDCB,
                                   &hFilter );
              if ( usTempRC != NO_ERROR )
              {
                usRC = QDPR_USER_ERROR;
                SETFOCUS( hwndDlg, ID_FILTER_COMBO );
              }
              else
              {
                if ( AsdRetPropPtr( NULL, psctIDA->psctInOutput->hDCB,
                                    &pDictProp ) == LX_RC_OK_ASD )
                {
                  usTempRC = FiltCheckFields( hFilter, pDictProp, TRUE );
                  if ( usTempRC != NO_ERROR )
                  {
                    usRC = QDPR_USER_ERROR;
                  } /* endif */
                } /* endif */
                /********************************************************/
                /* check whether the format file contains only fields   */
                /* which are also selected in the filter                */
                /********************************************************/
                if ( UtlAlloc( (PVOID *) &pszNameBuffer,
                         0L, (LONG)( psctIDA->usFormatFileLen ), ERROR_STORAGE ) )
                {
                  /********************************************************/
                  /* fill buffer: scan tokenized formatfile for all       */
                  /* field names                                          */
                  /********************************************************/
                  QDPRFillNameFields(psctIDA, pszNameBuffer);
                  /********************************************************/
                  /* compare with filter contents :success = rc=nullptr   */
                  /* else rc = ptr to fieldname with error                */
                  /********************************************************/
                  pBufferRc = FiltCheckSelFields(hFilter, pszNameBuffer);
                  /********************************************************/
                  /* if not ok display warning                            */
                  /********************************************************/
                  if ( pBufferRc )
                  {
                    pReplAddr[0] = psctIDA->psctInOutput->szFormatFile;
                    pReplAddr[1] = pBufferRc;
                    pReplAddr[2] = psctIDA->psctInOutput->szFilter;

                    usMBRc = UtlErrorHwnd( QDPR_SYER_MISSING_FLTFIELDNAME,
                                           MB_YESNO,
                                           3,
                                           &pReplAddr[0],
                                           EQF_QUERY,
                                           hwndDlg);
                    if ( usMBRc == MBID_YES )    // continue nevertheless
                    {
                        usRC = QDPR_NO_ERROR;
                    }
                    else
                    {
                        usRC = QDPR_USER_ERROR;
                    } /* endif */
                  } /* endif */
                  /********************************************************/
                  /* free memory                                          */
                  /********************************************************/
                  UtlAlloc( (PVOID *) &pszNameBuffer, 0L, 0L, NOMSG );

                } /* endif */

                if ( hFilter != NULL )
                {
                  FiltClose( hFilter );
                } /* endif */
              } /* endif */
            } /* endif */

            if ( (usRC == QDPR_NO_ERROR) &&
                 !(psctIDA->psctInOutput->fPrinterDest) )
            {
              /********************************************************/
              /* Get output file name into last-used-values structure */
              /********************************************************/
              QUERYTEXT( hwndDlg, ID_QDPR_PRTDLG_NAME_EF,
                         psctProps->sctLastUsedValues.szFileName );
              pszFile = psctProps->sctLastUsedValues.szFileName;
              UtlStripBlanks( pszFile );

              /********************************************************/
              /* Add leading backslash if none specified to avoid     */
              /* writing of file into our program directory ...       */
              /********************************************************/
              if ( (*pszFile != EOS) && (*pszFile != BACKSLASH) )
              {
                if ( isalpha(*pszFile) && (pszFile[1] == COLON) )
                {
                  /****************************************************/
                  /* file name is prefixed with a drive letter ...    */
                  /****************************************************/
                  if ( pszFile[2] != BACKSLASH )
                  {
                    /**************************************************/
                    /* No backslash between drive and following       */
                    /* filename so insert one                         */
                    /**************************************************/
                    memmove( pszFile + 3, pszFile + 2, strlen(pszFile+2) + 1 );
                    pszFile[2] = BACKSLASH;
                  } /* endif */
                }
                else
                {
                  /****************************************************/
                  /* No drive letter prefixes the filename, so        */
                  /* prefix filename with a backslash                 */
                  /****************************************************/
                  memmove( pszFile + 1, pszFile, strlen(pszFile) + 1 );
                  pszFile[0] = BACKSLASH;
                } /* endif */
              } /* endif */
              strcpy( psctIDA->psctInOutput->szPrintDest, pszFile );

              if ( psctProps->sctLastUsedValues.szFileName[0] != EOS )
              {

                usDosRC = UtlQFileMode( pszFile,
                                     &usAttribute,
                                     0L,
                                     FALSE );
                switch ( usDosRC )
                {
                  case NO_ERROR :
                    if ( usAttribute == FILE_DIRECTORY )
                    {
                      usRC = QDPR_USER_ERROR;
                    } /* endif */
                    break;
                  case ERROR_FILE_NOT_FOUND :
                    /****************************************************/
                    /* Is ok ...                                        */
                    /****************************************************/
                    break;
                  default :
                    usRC = QDPR_USER_ERROR;
                    break;
                } /* endswitch */

                if ( usRC != QDPR_USER_ERROR )
                {
                  /****************************************************/
                  /* Check give name against name for pipes and       */
                  /* devices                                          */
                  /****************************************************/
                  hFile = NULLHANDLE;
                  usDosRC = UtlOpen( pszFile, &hFile, &usAction,
                                     0L, FILE_NORMAL, FILE_OPEN,
                                     OPEN_ACCESS_READWRITE |
                                     OPEN_SHARE_DENYREADWRITE,
                                     0L, FALSE );
                  if ( hFile ) UtlClose( hFile, FALSE );
                } /* endswitch */

                if ( usRC == QDPR_USER_ERROR )
                {
                  QUERYTEXT( hwndDlg, ID_QDPR_PRTDLG_NAME_EF,
                             psctProps->sctLastUsedValues.szFileName );
                  UtlStripBlanks( psctProps->sctLastUsedValues.szFileName );
                  SETTEXT( hwndDlg, ID_QDPR_PRTDLG_NAME_EF,
                           psctProps->sctLastUsedValues.szFileName );
                  pReplAddr[0] = psctProps->sctLastUsedValues.szFileName;
                  UtlErrorHwnd( ERROR_FILENAME_NOT_VALID, MB_CANCEL, 1,
                                pReplAddr, EQF_ERROR, hwndDlg );
                  SETFOCUS( hwndDlg, ID_QDPR_PRTDLG_NAME_EF );
                } /* endif */
              } /* endif */

            } /* endif */

            if ( usRC == QDPR_NO_ERROR )
            {
              /********************************************************/
              /*   get print destination in input/output structure    */
              /*          and in last-used-values structure           */
              /********************************************************/
              switch ( QUERYCHECK( hwndDlg, ID_QDPR_PRTDLG_PRINTER_RB ) )
              {
                case 0 :
                  {
                    psctProps->sctLastUsedValues.fPrintDest = FALSE;
                    psctIDA->psctInOutput->fPrinterDest = FALSE;
                    strncpy( psctIDA->psctInOutput->szPrintDest,
                             psctProps->sctLastUsedValues.szFileName,
                             QDPR_MAX_PATH_FILENAME - 1 );
                  }
                break;
                case 1 :
                  {
                    psctProps->sctLastUsedValues.fPrintDest = TRUE;
                    psctIDA->psctInOutput->fPrinterDest = TRUE;
                    memset( psctIDA->psctInOutput->szPrintDest,
                            NULC, QDPR_MAX_PATH_FILENAME );
                  }
                break;
              } /* endswitch */
            } /* endif */

            if ( usRC == QDPR_NO_ERROR )
            {
              /********************************************************/
              /*         check if a valid file name is given          */
              /********************************************************/
              if ( ((psctIDA->psctInOutput->szPrintDest)[0] == NULC) ||
                    !UtlCheckPath( psctIDA->psctInOutput->szPrintDest, 0L, NULL ) )
              {
                if ( !( psctIDA->psctInOutput->fPrinterDest ) )
                {
                  usRC = QDPR_USER_ERROR;
                } /* endif */
              } /* endif */

              if ( usRC == QDPR_USER_ERROR )
              {
                pReplAddr[0] = psctIDA->psctInOutput->szPrintDest;
                UtlErrorHwnd( QDPR_NO_VALID_FILE_SELECTED, MB_CANCEL, 1,
                              &pReplAddr[0], EQF_ERROR, hwndDlg );
              } /* endif */
            } /* endif */
          }
        break;

      } /* endswitch */

      /****************************************************************/
      /*                   write the new properties                   */
      /*                but only if everything went ok                */
      /****************************************************************/
      if ( usRC == QDPR_NO_ERROR )
      {
        QDPRWriteProps( psctProps );
      } /* endif */

      /****************************************************************/
      /*                deallocate property structure                 */
      /****************************************************************/
      UtlAlloc( (PVOID *) &psctProps, 0L, 0L, NOMSG );
    } /* endif */
  } /* endif */

  return( usRC );

} /* end of function QDPRSaveLastUsedValues */



//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name: QDPRProcessFormatFile                                        |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//|  USHORT QDPRProcessFormatFile( psctIDA)                                    |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//|  This function loads the format information file pszFilename into          |
//|  memory (to the location referenced by ppszFormatFile) and                 |
//|  tokenizes its content via EQFBFastTokenize.                               |
//|                                                                            |
//|  It returns a pointer to the format information file in memory and to      |
//|  the buffer area where the tokens found are listed.                        |
//+----------------------------------------------------------------------------+
//|Input parameter:                                                            |
//|  PSZ          pszFilename;     // Name of format information file to be    |
//|                                // processed                                |
//+----------------------------------------------------------------------------+
//|Output parameter:                                                           |
//|  PSZ          *ppszFormatFile; // Address of a pointer referencing         |
//|                                // the buffer area into which the           |
//|                                // format info file is loaded               |
//|  PTOKENENTRY  *ppTok;          // Pointer to start of tokenized buffer     |
//|                                // area                                     |
//+----------------------------------------------------------------------------+
//|Returncode type: USHORT                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:                                                                |
//|  QDPR_NO_ERROR         - everything was OK                                 |
//|  QDPR_NO_MEMORY        - not enough memory left                            |
//|  QDPR_PATH_NOT_FOUND   - path to format information file or to tag table   |
//|                          could not be found                                |
//+----------------------------------------------------------------------------+
//|Prerequesits:                                                               |
//|  none                                                                      |
//+----------------------------------------------------------------------------+
//|Side effects:                                                               |
//|  - allocates storage for ppszFormatFile and ppTok which has to be          |
//|    deallocated anywhere else                                               |
//+----------------------------------------------------------------------------+
//|Samples:                                                                    |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                                                                            |
//|  Load format information file into memory (UtlLoadFile)                    |
//|  Load print tag table (EQFBLoadTagTable)                                   |
//|  Allocate storage for token buffer area                                    |
//|  Tokenize the format information file (EQFBFastTokenize)                   |
//+----------------------------------------------------------------------------+

USHORT QDPRProcessFormatFile
(
  PQDPR_DLG_IDA   psctIDA
)
{
  /********************************************************************/
  /*                         local variables                          */
  /********************************************************************/
  USHORT       usRC = QDPR_NO_ERROR;              // function returncode
  USHORT       usRetCode;                         // returncode from
                                                  // called functions
  USHORT       usBytesRead = 0;                   // bytes read from
                                                  // format file
  USHORT       usColPos;                          // for EQFBFastTokenize
  ULONG        ulAllocSize = 0;                   // allocation size
  PLOADEDTABLE pTagTable = NULL;                  // pointer to loaded
                                                  // tag table
  PSZ          pszPathFileName = NULL;            // full file name
  PSZ          pszWorkBuffer = NULL;              // work buffer
  PSZ          pszFileLoad = NULL;                // pointer for loading
                                                  // the format file
  PSZ          pRest = NULL;                      // for EQFBFastTokenize
  PSZ          pReplAddr[1];                      // for UtlError
                                // Name of processed format info file
  PSZ   pszFilename = psctIDA->psctInOutput->szFormatFile;




  /********************************************************************/
  /*                    allocate temporary storage                    */
  /********************************************************************/
  if ( UtlAlloc( (PVOID *) &pszPathFileName, 0L, (LONG)( 2 * QDPR_MAX_STRING ),
                 ERROR_STORAGE ) )
  {
    pszWorkBuffer = (PSZ)( pszPathFileName + QDPR_MAX_STRING );
  }
  else
  {
    usRC = QDPR_NO_MEMORY;
  } /* endif */

  /********************************************************************/
  /*            build full path to format information file            */
  /********************************************************************/
  if ( usRC == QDPR_NO_ERROR )
  {
                                                          /* 3@KIT0892C */
    UtlMakeEQFPath( pszPathFileName, NULC, PRT_PATH, NULL );
    strcat( pszPathFileName, BACKSLASH_STR );
    strcat( pszPathFileName, pszFilename );
  } /* endif */

  /********************************************************************/
  /*         now load the format file completely into memory          */
  /********************************************************************/
  if ( usRC == QDPR_NO_ERROR )
  {
    if ( !( UtlLoadFile( pszPathFileName, (PVOID *)&pszFileLoad, &usBytesRead,
                         FALSE, TRUE ) ) )
    {
      usRC = QDPR_PATH_NOT_FOUND;
    }
    else
    {
      psctIDA->usFormatFileLen = usBytesRead;
    } /* endif */
  } /* endif */

  if ( usRC == QDPR_NO_ERROR )
  {
    /******************************************************************/
    /*    allocate storage for file buffer (loaded file size plus     */
    /*    1 byte to contain \0 which is needed by EQFBFastTokenize    */
    /*                     to tokenize correctly)                     */
    /*    and copy the file loaded to the newly allocated storage     */
    /******************************************************************/
    if ( UtlAlloc( (PVOID *) &(psctIDA->psctInOutput->pszFormatFile),
                       0L, (LONG)( usBytesRead + 1 ), ERROR_STORAGE ) )
    {
      memcpy( psctIDA->psctInOutput->pszFormatFile, pszFileLoad, usBytesRead );
    }
    else
    {
      usRC = QDPR_NO_MEMORY;
    } /* endif */

    /******************************************************************/
    /*             deallocate storage used by loaded file             */
    /******************************************************************/
    UtlAlloc( (PVOID *) &pszFileLoad, 0L, 0L, NOMSG );
  } /* endif */

  if ( usRC == QDPR_NO_ERROR )
  {
    /******************************************************************/
    /*                 now try to load the tag table                  */
    /******************************************************************/
    usRetCode = TALoadTagTable( QDPR_TAGTABLE_NAME, &pTagTable, TRUE, TRUE );

    if ( usRetCode != NO_ERROR )
    {
      usRC = QDPR_PATH_NOT_FOUND;
    } /* endif */
  } /* endif */

  if ( usRC == QDPR_NO_ERROR )
  {
    /******************************************************************/
    /*         now allocate storage for the token buffer area         */
    /******************************************************************/
    ulAllocSize = (LONG)min( QDPR_MAX_ALLOC_SIZE, 3 * usBytesRead );

    if ( !( UtlAlloc( (PVOID *) &(psctIDA->psctInOutput->pTokFormatFile),
                        0L, ulAllocSize, ERROR_STORAGE ) ) )
    {
      usRC = QDPR_NO_MEMORY;
    } /* endif */
  } /* endif */

  if ( usRC == QDPR_NO_ERROR )
  {
    /******************************************************************/
    /*                  now tokenize the format file                  */
    /******************************************************************/
    // assume system pref. lang for format file
    TAFastTokenize( psctIDA->psctInOutput->pszFormatFile,
                    pTagTable, TRUE, &pRest,
                    &usColPos, psctIDA->psctInOutput->pTokFormatFile,
                    (USHORT)(ulAllocSize / sizeof( TOKENENTRY )), 0L );

    /******************************************************************/
    /*            check if anything could not be processed            */
    /******************************************************************/
    if ( pRest != NULL )
    {
      if ( ( *pRest != QDPR_CR ) && ( *pRest != QDPR_LF ) &&
           ( *pRest != QDPR_EOF ) )
      {
        pReplAddr[0] = pszFilename;
        UtlError( QDPR_SCANNING_FORMAT_FILE, MB_CANCEL, 1, &pReplAddr[0],
                  EQF_ERROR );
        usRC = QDPR_PATH_NOT_FOUND;
      } /* endif */
    } /* endif */
  } /* endif */

  /********************************************************************/
  /*              deallocate temporary allocated storage              */
  /********************************************************************/
  if ( pTagTable )
  {
    TAFreeTagTable( pTagTable );
  } /* endif */
  if ( pszPathFileName != NULL )
  {
    UtlAlloc( (PVOID *) &pszPathFileName, 0L, 0L, NOMSG );
  } /* endif */

  return( usRC );

} /* end of function QDPRProcessFormatFile */



//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name: QDPRScanDescTag                                              |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//|  USHORT QDPRScanDescTag( pszFilename, pszDescBuffer, usBufLength,          |
//|                          pTokFormatFile )                                  |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//|  This function scans the tokenized format information file to search       |
//|  for the description tag.                                                  |
//|  It returns the text that is found in between the description and          |
//|  end-description tag.                                                      |
//|                                                                            |
//|  If an error occurred the ouput string with the text is empty.             |
//+----------------------------------------------------------------------------+
//|Input parameter:                                                            |
//|  PSZ         pszFilename;    // format information file name               |
//|  PSZ         pszDescBuffer;  // pointer to buffer receiving the            |
//|                              // description tag text                       |
//|  USHORT      usBufLength;    // length of pszDescBuffer including '\0'     |
//|  PTOKENENTRY pTokFormatFile; // pointer to start of tokenized format       |
//|                              // information file buffer area               |
//+----------------------------------------------------------------------------+
//|Output parameter:                                                           |
//|  none                                                                      |
//+----------------------------------------------------------------------------+
//|Returncode type: USHORT                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:                                                                |
//|  QDPR_NO_ERROR         - everything was OK                                 |
//|  QDPR_NO_MEMORY        - not enough memory left                            |
//|  QDPR_SYNTAX_ERROR     - a syntax error occurred, user is already informed |
//+----------------------------------------------------------------------------+
//|Prerequesits:                                                               |
//|  none                                                                      |
//+----------------------------------------------------------------------------+
//|Side effects:                                                               |
//|  none                                                                      |
//+----------------------------------------------------------------------------+
//|Samples:                                                                    |
//|                                                                            |
//|  usRC = QDPRScanDescTag( "TEST.FRM", achrDescBuffer, 200, pTok );          |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                                                                            |
//|  Search description tag                                                    |
//|  IF description tag has been found                                         |
//|    Read text into the description buffer unti an end-description tag       |
//+----------------------------------------------------------------------------+

USHORT QDPRScanDescTag(
  PSZ         pszFilename,    // format information file name
  PSZ         pszDescBuffer,  // pointer to buffer receiving the
                              // description tag text
  USHORT      usBufLength,    // length of pszDescBuffer including '\0'
  PTOKENENTRY pTokFormatFile )// pointer to start of tokenized format
                              // information file buffer area

{
  /********************************************************************/
  /*                         local variables                          */
  /********************************************************************/
  USHORT       usRC = QDPR_NO_ERROR;              // function returncode
  USHORT       usBuf;                             // desc buffer counter
  PSZ          pReplAddr[4];                      // for UtlError
  PSZ          pszTokenData;                      // token data pointer
  PSZ          pszRunData;                        // run-over-the-data ptr
  PSZ          pszWorkBuffer = NULL;              // work buffer
  PSZ          pszWorkBuffer2 = NULL;             // work buffer
  PSZ          pszWorkBuffer3 = NULL;             // work buffer
  PTOKENENTRY  pTok;                              // temp token pointer



  /********************************************************************/
  /*                       initialize variables                       */
  /********************************************************************/
  memset( pszDescBuffer, NULC, usBufLength );

  pTok = pTokFormatFile;

  usBuf = 0;

  /********************************************************************/
  /*                    allocate temporary storage                    */
  /********************************************************************/
  if ( UtlAlloc( (PVOID *) &pszWorkBuffer, 0L, (LONG)( 3 * QDPR_MAX_STRING ),
                 ERROR_STORAGE ) )
  {
    pszWorkBuffer2 = (PSZ)( pszWorkBuffer + QDPR_MAX_STRING );
    pszWorkBuffer3 = (PSZ)( pszWorkBuffer2 + QDPR_MAX_STRING );
  }
  else
  {
    usRC = QDPR_NO_MEMORY;
  } /* endif */

  /********************************************************************/
  /*               start to search the description tag                */
  /********************************************************************/
  while ( ( pTok->sTokenid != ENDOFLIST ) &&
          ( pTok->sTokenid != QDPR_DESCRIPTION_TOKEN ) &&
          ( usRC == QDPR_NO_ERROR ) )
  {
    if ( pTok->sTokenid == QDPR_COMMENT_TOKEN )
    {
      usRC = QDPRReadOverComment( &pTok );

      if ( usRC == QDPR_SYNTAX_ERROR )
      {
        sprintf( pszWorkBuffer, "%d",
                 QDPRLineNumbers( pTokFormatFile->pDataString,
                                  pTok->pDataString, 0 ) + 1 );
        usRC = QDPRMakeTagFromTagID( pszWorkBuffer2, QDPR_MAX_STRING,
                                     QDPR_COMMENT_TOKEN, TRUE );
        if ( usRC == QDPR_NO_ERROR )
        {
          usRC = QDPRMakeTagFromTagID( pszWorkBuffer3, QDPR_MAX_STRING,
                                       QDPR_COMMENT_ETOKEN, TRUE );
        } /* endif */

        if ( usRC == QDPR_NO_ERROR )
        {
          pReplAddr[0] = pszFilename;
          pReplAddr[1] = pszWorkBuffer;
          pReplAddr[2] = pszWorkBuffer2;
          pReplAddr[3] = pszWorkBuffer3;
          UtlError( QDPR_SYER_TAG_NOT_CLOSED, MB_CANCEL, 4,
                    &pReplAddr[0], EQF_ERROR );
          usRC = QDPR_SYNTAX_ERROR;
        } /* endif */
      } /* endif */
    } /* endif */

    if ( ( pTok->sTokenid != ENDOFLIST ) &&
         ( pTok->sTokenid != QDPR_DESCRIPTION_TOKEN ) &&
         ( usRC == QDPR_NO_ERROR ) )
    {
      pTok++;
    } /* endif */
  } /* endwhile */

  if ( pTok->sTokenid == QDPR_DESCRIPTION_TOKEN )
  {
    /******************************************************************/
    /*    description tag found, now get the description text into    */
    /*                           the buffer                           */
    /******************************************************************/
    pTok++;

    while ( ( pTok->sTokenid != QDPR_DESCRIPTION_ETOKEN ) &&
            ( pTok->sTokenid != ENDOFLIST ) &&
            ( usRC == QDPR_NO_ERROR ) )
    {
      switch ( pTok->sTokenid )
      {
        case QDPR_COMMENT_TOKEN :
          {
            usRC = QDPRReadOverComment( &pTok );

            if ( usRC == QDPR_SYNTAX_ERROR )
            {
              sprintf( pszWorkBuffer, "%d",
                       QDPRLineNumbers( pTokFormatFile->pDataString,
                                        pTok->pDataString, 0 ) + 1 );
              usRC = QDPRMakeTagFromTagID( pszWorkBuffer2, QDPR_MAX_STRING,
                                           QDPR_COMMENT_TOKEN, TRUE );
              if ( usRC == QDPR_NO_ERROR )
              {
                usRC = QDPRMakeTagFromTagID( pszWorkBuffer3, QDPR_MAX_STRING,
                                             QDPR_COMMENT_ETOKEN, TRUE );
              } /* endif */

              if ( usRC == QDPR_NO_ERROR )
              {
                pReplAddr[0] = pszFilename;
                pReplAddr[1] = pszWorkBuffer;
                pReplAddr[2] = pszWorkBuffer2;
                pReplAddr[3] = pszWorkBuffer3;
                UtlError( QDPR_SYER_TAG_NOT_CLOSED, MB_CANCEL, 4,
                          &pReplAddr[0], EQF_ERROR );
                usRC = QDPR_SYNTAX_ERROR;
              } /* endif */
            } /* endif */
          }
        break;
        case TEXT_TOKEN :
          {
            if ( usBuf < usBufLength - 1 )
            {
              if ( UtlAlloc( (PVOID *) &pszTokenData, 0L,
                             (LONG)( max( MIN_ALLOC, (pTok->usLength + 1) ) ),
                             ERROR_STORAGE ) )
              {
                strncpy( pszTokenData, pTok->pDataString, pTok->usLength );
                pszRunData = pszTokenData;

                /******************************************************/
                /*   check if one or more CRLFs can be found in the   */
                /*                     token data                     */
                /* if so do not copy them to the description buffer,  */
                /*          but replace the LF with a blank           */
                /******************************************************/
                while ( *pszRunData != NULC )
                {
                  if ( ( *pszRunData != QDPR_CR ) &&
                       ( *pszRunData != QDPR_LF ) )
                  {
                    pszDescBuffer[usBuf] = *pszRunData;
                    usBuf++;
                  }
                  else
                  {
                    if ( *pszRunData == QDPR_LF )
                    {
                      pszDescBuffer[usBuf] = QDPR_BLANK;
                      usBuf++;
                    } /* endif */
                  } /* endif */

                  pszRunData++;
                } /* endwhile */

                UtlAlloc( (PVOID *) &pszTokenData, 0L, 0L, NOMSG );
              }
              else
              {
                usRC = QDPR_NO_MEMORY;
              } /* endif */
            } /* endif */

            pTok++;
          }
        break;
        default :
          {
            sprintf( pszWorkBuffer, "%d",
                     QDPRLineNumbers( pTokFormatFile->pDataString,
                                      pTok->pDataString, 0 ) + 1 );

            pReplAddr[0] = pszFilename;
            pReplAddr[1] = pszWorkBuffer;
            UtlError( QDPR_SYER_WRONG_TAG_ORDER, MB_CANCEL, 2,
                      &pReplAddr[0], EQF_ERROR );
            usRC = QDPR_SYNTAX_ERROR;
          }
        break;
      } /* endswitch */
    } /* endwhile */

    /******************************************************************/
    /*         check if no end description tag could be found         */
    /******************************************************************/
    if ( pTok->sTokenid == ENDOFLIST )
    {
      sprintf( pszWorkBuffer, "%d",
               QDPRLineNumbers( pTokFormatFile->pDataString,
                                pTok->pDataString, 0 ) + 1 );

      usRC = QDPRMakeTagFromTagID( pszWorkBuffer2, QDPR_MAX_STRING,
                                   QDPR_DESCRIPTION_TOKEN, TRUE );
      if ( usRC == QDPR_NO_ERROR )
      {
        usRC = QDPRMakeTagFromTagID( pszWorkBuffer3, QDPR_MAX_STRING,
                                     QDPR_DESCRIPTION_ETOKEN, TRUE );
      } /* endif */

      if ( usRC == QDPR_NO_ERROR )
      {
        pReplAddr[0] = pszFilename;
        pReplAddr[1] = pszWorkBuffer;
        pReplAddr[2] = pszWorkBuffer2;
        pReplAddr[3] = pszWorkBuffer3;
        UtlError( QDPR_SYER_TAG_NOT_CLOSED, MB_CANCEL, 4, &pReplAddr[0],
                  EQF_ERROR );
        usRC = QDPR_SYNTAX_ERROR;
      } /* endif */
    } /* endif */
  } /* endif */

  /********************************************************************/
  /*              deallocate temporary allocate storage               */
  /********************************************************************/
  if ( pszWorkBuffer != NULL )
  {
    UtlAlloc( (PVOID *) &pszWorkBuffer, 0L, 0L, NOMSG );
  } /* endif */

  /********************************************************************/
  /*   if an error occurred in processing clear description buffer    */
  /********************************************************************/
  if ( usRC == QDPR_NO_MEMORY )
  {
    memset( pszDescBuffer, NULC, usBufLength );
  } /* endif */

  return( usRC );

} /* end of function QDPRScanDescTag */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     QDPRFillNameFields                                       |
//+----------------------------------------------------------------------------+
//|Function call:     QDPRFillNameFields( psctIDA, pszNameBuffer)              |
//+----------------------------------------------------------------------------+
//|Description:       fill name fields                                         |
//+----------------------------------------------------------------------------+
//|Parameters:        PQDPR_DLG_IDA   psctIDA                                  |
//|                   PSZ pszNameBuffer                                        |
//+----------------------------------------------------------------------------+
//|Returncode type:   void                                                     |
//+----------------------------------------------------------------------------+
//|Function flow:     while not end of list                                    |
//|                    if var or repeat token                                  |
//|                      if next token is name=                                |
//|                         fill the field in the buffer                       |
//|                      endif                                                 |
//|                    endif                                                   |
//|                  endwhile                                                  |
//+----------------------------------------------------------------------------+

VOID
QDPRFillNameFields
(
  PQDPR_DLG_IDA   psctIDA,        // ptr to dlg ida
  PSZ pszNameBuffer               // buffer to be filled with name specs
)
{
  USHORT usBuffree;                    // num of bytes free in buffer
  PSZ    pCurName;                     // ptr to next free pos in buffer
  USHORT usCurNameLen;                 // length of cur name field string
  PTOKENENTRY pTok;
  USHORT  usRc;

  pTok = psctIDA->psctInOutput->pTokFormatFile;
  pCurName = pszNameBuffer;
  usBuffree = psctIDA->usFormatFileLen;

  /********************************************************************/
  /* scan for var or repeat while not endoflist                       */
  /********************************************************************/

  while ( pTok->sTokenid != ENDOFLIST )
  {
    if ( (pTok->sTokenid == QDPR_VAR_TOKEN)
        || (pTok->sTokenid == QDPR_REPEAT_TOKEN) )
    {
      pTok++;
      if ( pTok->sTokenid == QDPR_NAME_ATTR )   //retrieve name spec
      {
                                                 //fill in Namebuffer
         usRc = QDPRRetrieveValue(pTok,pCurName,usBuffree);
         if ( !usRc )
         {
           usCurNameLen = (USHORT)(strlen(pCurName)+1);
           usBuffree = usBuffree - usCurNameLen;
           pCurName += usCurNameLen;
         } /* endif */
      } /* endif */
      pTok++;
    }
    else
    {
      pTok++;
    } /* endif */

  } /* endwhile */

} /* end of function  */
