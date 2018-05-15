//------------------------------------------------------------------------------
// EQFUTDLG.CPP                                                                   
//------------------------------------------------------------------------------
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|      Copyright (C) 1990-2015, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+

#define INCL_EQF_DLGUTILS         // dialog utilities
#define INCL_EQF_ANALYSIS         // analysis functions
#define INCL_EQF_TAGTABLE         // tagtable defines and functions
#define INCL_EQF_FOLDER           // folder list and document list functions
#define INCL_EQF_DLGUTILS         // dialog utilities
#define INCL_EQF_TP               // public translation processor functions

#include <eqf.h>                  // General Translation Manager include file
#include <eqfutils.id>            // IDs used by dialog utilities

#include <time.h>
#include <io.h>
#include <process.h>
#include <string>
#include <vector>
#include <OptionsDialog.h>              // API calls
#include <core\utilities\LogWriter.h>


// dialog data area for options dialog
typedef struct _OPTIONSDLGIDA
{
  // data from caller
  SHORT             sID;             // ID to be used for options dialog
  std::vector<std::string> *pvLabels; // Vector with labels for option fields 
  std::vector<std::string> *pvData;   // Vector with data for option fields  (on return filled with user supplied data)
  std::vector<std::string> *pvDescr;  // Vector with descriptions for option fields
  char szTitle[1000];               // title bar text  
  PFN_OPTIONSDIALOGCHECKDATA pfnCheckData; // input data checking callback
  long    lHandle;                    // handle passed to the callback function

  // our internal data
  std::vector<HWND> *phwndLabels;     // list of our fields labels
  std::vector<HWND> *phwndFields;     // list of our input fields
  std::vector<HWND> *phwndDescr;      // list of our description labels
  char szFieldData[1000];             // buffer for field data
  char szErrorMsg[1000];              // error message text
  LogWriter *pLog;
} OPTIONSDLGIDA, *POPTIONSDLGIDA;

// helper function to create a new dialog controls based on an existing one
HWND CloneControl
(
  HWND   hwndDlg,                                // dialog window handle
  HWND   hwndBase,                               // handle of control being cloned
  HWND   hwndBefore,                             // handle of control which is before the new control in Z-order
  int    iID,                                    // ID to be used for cloned control
  int    iOffset,                                // vertical offset between base control and cloned control  
  int    iSpecialStyle,                          // special style for this control (-1 = no style modification)
  int    iSpecialStyleMask                       // special style mask (to clear special style flags from previous control)
)
{
  WINDOWINFO WindowInfo;
  CHAR szClassName[40];
  HWND hwndNew = NULLHANDLE;
  WINDOWPLACEMENT Placement;
  HFONT   hFont;
  
  memset( &WindowInfo, 0, sizeof(WindowInfo) );
  GetWindowInfo( hwndBase, &WindowInfo );
  GetWindowPlacement( hwndBase, &Placement );
  GetClassName( hwndBase, szClassName, sizeof(szClassName) );
  if ( iSpecialStyle != -1 )
  {
    WindowInfo.dwStyle &= ~iSpecialStyleMask;
    WindowInfo.dwStyle |= iSpecialStyle;
  } /* end */     
  hwndNew = CreateWindowEx( WindowInfo.dwExStyle, szClassName, "", WindowInfo.dwStyle,  
                            Placement.rcNormalPosition.left, Placement.rcNormalPosition.top + iOffset, 
                            Placement.rcNormalPosition.right - Placement.rcNormalPosition.left, 
                            Placement.rcNormalPosition.bottom - Placement.rcNormalPosition.top, 
                            hwndDlg, (HMENU)iID, NULLHANDLE, 0 );

   hFont = (HFONT)SendMessage( hwndBase, WM_GETFONT, 0, 0L );
   if ( hwndNew != NULLHANDLE )
   {
    if ( hFont != NULL ) SendMessage( hwndNew, WM_SETFONT, (WPARAM)hFont, 0L ); 
    SetWindowPos( hwndNew, hwndBefore, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE );
   } /* endif */      

  return( hwndNew );
}


// options dialog procedure
INT_PTR CALLBACK UTLOPTIONDLG
(
  HWND   hwnd,                        // handle of window
  WINMSG msg,                         // type of message
  WPARAM mp1,                         // first message parameter
  LPARAM mp2                          // second message parameter
)
{
  MRESULT              mResult = FALSE;// dlg procedure return value
  POPTIONSDLGIDA       pIDA;           // dialog IDA
  BOOL                 fOK;            // internal O.K. flag

  switch ( msg )
  {
    case WM_EQF_QUERYID:
      pIDA = ACCESSDLGIDA( hwnd, POPTIONSDLGIDA );
      HANDLEQUERYID( pIDA->sID, mp2 );
      break;

    case WM_INITDLG :
      {
        pIDA = (POPTIONSDLGIDA)PVOIDFROMMP2( mp2 );
        ANCHORDLGIDA( hwnd, pIDA );
        HWND hwndFocus = NULL;
        pIDA->pLog->writef( "  Creating %ld option rows", pIDA->pvData->size() );

        // allocate vector for our dialog control tables
        pIDA->phwndFields = new(std::vector<HWND>)(pIDA->pvData->size(),NULL);
        pIDA->phwndLabels = new(std::vector<HWND>)(pIDA->pvData->size(),NULL);
        pIDA->phwndDescr = new(std::vector<HWND>)(pIDA->pvData->size(),NULL);

        // create controls for option fields
        int iControlID = ID_OPTION_DESC_TEXT + 1;
        int iVerticalOffset = 26;
        for ( std::size_t i = 0; i < pIDA->pvData->size(); i++ )
        {
          // get special style for entry field
          std::string strLabel = (*pIDA->pvLabels)[i];
          int iStyle = 0;
          if ( strLabel[0] =='@' )
          {
            // password field
            iStyle = ES_PASSWORD;
            strLabel.erase( 0, 1 );
          } 
          else if ( strLabel[0] =='!' )
          {
            // read-only field
            iStyle = ES_READONLY;
            strLabel.erase( 0, 1 );
          } /* endif */             

          // get/create controls for this row
          if ( i == 0 )
          {
            // first row: use existing fields
            (*pIDA->phwndLabels)[i] = GetDlgItem( hwnd, ID_OPTION_LABEL_TEXT );
            (*pIDA->phwndFields)[i] = GetDlgItem( hwnd, ID_OPTION_DATA_EF );
            (*pIDA->phwndDescr)[i] = GetDlgItem( hwnd, ID_OPTION_DESC_TEXT  );
            if ( iStyle & ES_READONLY )
            {
              SendMessage( (*pIDA->phwndFields)[i], EM_SETREADONLY, TRUE, 0L );
            }
            else
            {
              hwndFocus = (*pIDA->phwndFields)[i];
            } /* end */               
          }
          else
          {
            // any other row: create controls based on previous row
            (*pIDA->phwndLabels)[i] = CloneControl( hwnd, (*pIDA->phwndLabels)[i-1], (*pIDA->phwndDescr)[i-1], iControlID++, iVerticalOffset, -1, 0 );
            (*pIDA->phwndFields)[i] = CloneControl( hwnd, (*pIDA->phwndFields)[i-1], (*pIDA->phwndLabels)[i] , iControlID++, iVerticalOffset, iStyle, ES_READONLY | ES_PASSWORD );
            (*pIDA->phwndDescr)[i]  = CloneControl( hwnd, (*pIDA->phwndDescr)[i-1], (*pIDA->phwndFields)[i], iControlID++, iVerticalOffset, -1, 0 );
            if ( (hwndFocus == NULL) && ((iStyle & ES_READONLY) == 0) )
            {
              hwndFocus = (*pIDA->phwndFields)[i];
            } /* end */               
          } /* endif */             

          // fill row fields
          SetWindowText( (*pIDA->phwndLabels)[i], strLabel.c_str() );
          SetWindowText( (*pIDA->phwndFields)[i], (*pIDA->pvData)[i].c_str() );
          SetWindowText( (*pIDA->phwndDescr)[i], (*pIDA->pvDescr)[i].c_str() );
        } /* endfor */           

        // reposition push buttons
        {
          WINDOWPLACEMENT Placement;
          GetWindowPlacement( (*pIDA->phwndFields)[pIDA->pvData->size()-1], &Placement );
          int iVertPos = Placement.rcNormalPosition.bottom + iVerticalOffset;

          GetWindowPlacement( GetDlgItem( hwnd, ID_OPTIONSDLG_OK_PB ), &Placement );
          SetWindowPos( GetDlgItem( hwnd, ID_OPTIONSDLG_OK_PB ), NULLHANDLE, Placement.rcNormalPosition.left, iVertPos, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER );
          GetWindowPlacement( GetDlgItem( hwnd, ID_OPTIONSDLG_CANCEL_PB ), &Placement );
          SetWindowPos( GetDlgItem( hwnd, ID_OPTIONSDLG_CANCEL_PB ), NULLHANDLE, Placement.rcNormalPosition.left, iVertPos, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER  );
          GetWindowPlacement( GetDlgItem( hwnd, ID_OPTIONSDLG_HELP_PB ), &Placement );
          SetWindowPos( GetDlgItem( hwnd, ID_OPTIONSDLG_HELP_PB ), NULLHANDLE, Placement.rcNormalPosition.left, iVertPos, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER );
        }


        // adjust dialog size and keep dialog within TWB
        pIDA->pLog->write( "  Adjusting dialog size and position" );
        {
          SWP  swpDlg, swpTWB;
          WINDOWPLACEMENT Placement;
          GetWindowPlacement( GetDlgItem( hwnd, ID_OPTIONSDLG_OK_PB ), &Placement );

          // Get dialog size/position
          WinQueryWindowPos( hwnd, &swpDlg );
          swpDlg.cy = (SHORT)(Placement.rcNormalPosition.bottom + (2 * GetSystemMetrics( SM_CYDLGFRAME )) + GetSystemMetrics( SM_CYCAPTION ) + iVerticalOffset);

          // Center dialog within TWB
          WinQueryWindowPos( (HWND)UtlQueryULong( QL_TWBCLIENT ), &swpTWB );
          if ( (swpDlg.x > 0) && ((swpDlg.x + swpDlg.cx) < swpTWB.cx) )
          {
            swpDlg.x = (swpTWB.cx - swpDlg.cx) / 2;
          } /* endif */
          if ( (swpDlg.y > 0) && ((swpDlg.y + swpDlg.cy) < swpTWB.cy) )
          {
            swpDlg.y = (swpTWB.cy - swpDlg.cy) / 2;
          } /* endif */
          WinSetWindowPos( hwnd, HWND_TOP, swpDlg.x, swpDlg.y, swpDlg.cx, swpDlg.cy, EQF_SWP_MOVE | EQF_SWP_SHOW | EQF_SWP_SIZE | EQF_SWP_ACTIVATE );
        }

        // fill dialog fields with supplied data
        SETTEXTHWND( hwnd, pIDA->szTitle );

        SetFocus( hwndFocus );

        // leave the focus where we put it
        mResult = MRFROMSHORT(DIALOGINITRETURN(TRUE));
      }
      break;

    case WM_COMMAND :
      {
        switch ( WMCOMMANDID( mp1, mp2 ) )
        {
          case ID_OPTIONSDLG_OK_PB :

            // access dialog IDA
            fOK = TRUE;
            pIDA = ACCESSDLGIDA( hwnd, POPTIONSDLGIDA );
            pIDA->pLog->write( "  OK button pressed by user" );

            // get user data for options
            pIDA->pLog->write( "  Getting user input..." );
            for ( std::size_t i = 0; i < pIDA->pvData->size(); i++ )
            {
              GetWindowText( (*pIDA->phwndFields)[i], pIDA->szFieldData, sizeof(pIDA->szFieldData) );
              (*pIDA->pvData)[i].assign( pIDA->szFieldData );
            } /* endfor */           

            // call data check callback when specified
            if ( pIDA->pfnCheckData != NULL )
            {
              int iFieldInError = 0;
              pIDA->pLog->write( "  Calling supplied check field data callback..." );
              pIDA->szErrorMsg[0] = 0;
              pIDA->pfnCheckData( pIDA->pvData, &iFieldInError, pIDA->szErrorMsg, sizeof(pIDA->szErrorMsg), pIDA->lHandle, &fOK );
              pIDA->pLog->writef( "  Check field data callback terminated, fOK is %s", fOK ? "true" : "false" );
              if ( !fOK )
              {
                pIDA->pLog->writef( "  Field in-error is %ld, supplied error message text is %s", iFieldInError, pIDA->szErrorMsg );
                if ( pIDA->szErrorMsg[0] == 0 ) strcpy( pIDA->szErrorMsg, "Incorrect input, please retry" );
                MessageBox( hwnd, pIDA->szErrorMsg, "Error",  MB_CANCEL);
                SetFocus( (*pIDA->phwndFields)[iFieldInError] );
              } /* end */                 
            } /* end */               

            // leave dialog
            if ( fOK )
            {
              pIDA->pLog->write( "  Leaving dialog box..." );
              WinDismissDlg( hwnd, TRUE );
            } /* endif */
            break;

          case ID_OPTIONSDLG_CANCEL_PB :
          case DID_CANCEL :
            pIDA = ACCESSDLGIDA( hwnd, POPTIONSDLGIDA );
            pIDA->pLog->write( "  Dialog box termianted by user" );
            WinDismissDlg( hwnd, FALSE );
            break;

          case ID_OPTIONSDLG_HELP_PB:
            UtlInvokeHelp();
            break;
        } /* endswitch */
      }
      break;

    case WM_DESTROY :
      {
        // cleanup
        pIDA = (POPTIONSDLGIDA)PVOIDFROMMP2( mp2 );
        if ( pIDA != NULL )
        {
          // destroy the controls for all but the first row         
          for ( std::size_t i = 1; i < pIDA->pvData->size(); i++ )
          {
            DestroyWindow( (*pIDA->phwndLabels)[i] );
            DestroyWindow( (*pIDA->phwndFields)[i] );
            DestroyWindow( (*pIDA->phwndDescr)[i] );
          } /* endfor */           

          delete( pIDA->phwndLabels );
          delete( pIDA->phwndFields );
          delete( pIDA->phwndDescr );
        } /* end */           

      }
      break;

    default :
      mResult = WinDefDlgProc( hwnd, msg, mp1, mp2 );
      break;
  } /* endswitch */

  return( mResult );

} /* end of function UTLOPTIONDLG */

BOOL UtlOptionsDlg
(
  HWND              hwndOwner,        // owner window for this dialog box
  SHORT             sID,              // ID to be used for options dialog
  std::vector<std::string> *pvLabel,  // Vector with labels for option fields 
  std::vector<std::string> *pvData,   // Vector with data for option fields  (on return filled with user supplied data)
  std::vector<std::string> *pvDescr,  // Vector with descriptions for option fields
  std::string strTitle,               // title bar text  
  PFN_OPTIONSDIALOGCHECKDATA pfnCheckData, // input data checking callback
  long    lHandle                    // handle passed to the callback function
)
{
  BOOL             fSet;               // return code of dialog
  HMODULE hResMod;
  POPTIONSDLGIDA       pIDA;           // dialog IDA

  sID;

  hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);

  // allocate or instance data area
  if ( !UtlAlloc( (PVOID *)&pIDA, 0, sizeof(OPTIONSDLGIDA), ERROR_STORAGE ) ) return( FALSE );

  // fill IDA fields
  pIDA->pvLabels = pvLabel;
  pIDA->pvData = pvData;
  pIDA->pvDescr = pvDescr;
  strcpy( pIDA->szTitle, strTitle.c_str() );
  pIDA->pfnCheckData = pfnCheckData;
  pIDA->lHandle = lHandle;
  pIDA->pLog = new( LogWriter );

  pIDA->pLog->open( "UtlOptionsDlg" );

  pIDA->pLog->writef( "Calling options dialog box %s", strTitle.c_str() );
  DIALOGBOX( hwndOwner, UTLOPTIONDLG, hResMod, ID_OPTION_DLG, pIDA, fSet );
  pIDA->pLog->writef( "  Options dialog box ended, rc=%ld", (int)fSet );

  if ( pIDA->pLog != NULL ) 
  {
    pIDA->pLog->close();

    delete( pIDA->pLog );
  } /* endif */

  UtlAlloc( (PVOID *)&pIDA, 0, 0, NOMSG );

  return( fSet );
} /* end of function UtlOptionsDlg */
