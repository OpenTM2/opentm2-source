/*! \file
	Description: Dialogs used within Translation Processor

	Copyright Notice:

	Copyright (C) 1990-2015, International Business Machines
	Corporation and others. All rights reserved
*/

#define INCL_EQF_TP               // public translation processor functions
#define INCL_EQF_PRINT            // print functions
#define INCL_EQF_EDITORAPI        // editor API
#define INCL_EQF_TAGTABLE         // tagtable defines and functions
#define INCL_EQF_MORPH
#include <eqf.h>                  // General Translation Manager include file

#include "eqfstart.h"             // help processing

#include "EQFTPI.H"               // Translation Processor priv. include file
#include "EQFB.ID"                // Translation Processor IDs
#include "EQFBDLG.ID"             // dialog control IDs

#undef _WPTMIF                         // we don't care about WP I/F
#include "eqfhelp.id"                  // help resource IDs
#include "eqfhlp1.h"                   // first part of help tables
#include "eqfmsg.htb"                          // message help table

static BOOL EQFBGetProfInit ( HWND hwndDlg );
static VOID EQFBDoProfInit ( HWND ,WPARAM,LPARAM );
MRESULT EQFBProfInitCommand ( HWND, WPARAM, LPARAM );
static VOID UpdateDocSettings ( PTBDOCUMENT pDoc );
static VOID UpdateAutoLineWrapSettings(PTBDOCUMENT, BOOL, USHORT);
static SHORT GetNumber ( PSZ pszText );
static LONG  GetLongNumber ( PSZ pszText );

#define GOTOLINE_LENGTH     12         // number of digits for line and segment number
#define MAXTOCLEN          128         // size of buffer of TOC entry

MRESULT EQFBEntryCommand( HWND, WPARAM, LPARAM );

MRESULT EQFBRMarginCommand( HWND, WPARAM, LPARAM );
MRESULT EQFBSettingsCommand( HWND, WPARAM, LPARAM );
MRESULT EQFBGotoLineCommand( HWND, WPARAM, LPARAM );
MRESULT EQFBSettingsControl( HWND, SHORT, SHORT );

static VOID EQFBDoSettings ( HWND, WPARAM, LPARAM );
static BOOL EQFBGetSettings ( HWND );


#include "commctrl.h"          // common controls

LONG FAR PASCAL ProfSpinSubclassProc( HWND, SHORT, WPARAM, LPARAM );

static FARPROC pfnOrgProfSpinProc;         // original spinbutton procedure

MRESULT EQFBTOCGotoCommand( HWND, WPARAM, LPARAM );
MRESULT EQFBTOCGotoControl ( HWND, SHORT, SHORT );
static VOID EQFBTOCGotoInit ( HWND  hwndDlg, WPARAM mp1, LPARAM mp2 );
static BOOL EQFBTOCGotoFillLB ( HWND hwndDlg, PTOCGOTOIDA  pTOCIda );
static VOID EQFBTOCGotoFillBuffer(PTBDOCUMENT, PTOKENENTRY *, PCHAR_W, PUSHORT);

INT_PTR CALLBACK EQFBPROP_EDITOR_DLGPROC ( HWND, WINMSG, WPARAM, LPARAM );
INT_PTR CALLBACK EQFBPROP_DISPLAY_DLGPROC ( HWND, WINMSG, WPARAM, LPARAM );
INT_PTR CALLBACK EQFBPROP_DICTIONARY_DLGPROC ( HWND, WINMSG, WPARAM, LPARAM );
INT_PTR CALLBACK EQFBPROP_TRANSLMEM_DLGPROC ( HWND, WINMSG, WPARAM, LPARAM );
INT_PTR CALLBACK EQFBPROP_MESSAGES_DLGPROC ( HWND, WINMSG, WPARAM, LPARAM );
INT_PTR CALLBACK EQFBPROP_BACKTASKS_DLGPROC ( HWND, WINMSG, WPARAM, LPARAM );
INT_PTR CALLBACK EQFBPROP_AUTSUBST_DLGPROC (HWND, WINMSG, WPARAM, LPARAM );

BOOL    PropertySheetLoad ( HWND hwndDlg, LPARAM mp2 );
MRESULT PropertySheetNotification ( HWND hwndDlg, WPARAM mp1, LPARAM mp2 );
MRESULT EQFBPropCommand ( HWND, WPARAM, LPARAM );


/*////////////////////////////////////////////////////////////////////////////
:h2.EQFBEntryDlgProc - dialog procedure for entry sentence dialog
*/
// Description:
//    Allows to enter sentence and sends it to translation memory and dict.
//
//   Flow (message driven):
//       case WM_INITDLG:
//         initialize the dialog controls;
//       case WM_COMMAND
//         call EQFBEntryCommand to handle user commands;
//       case WM_CLOSE
//         get previously active segment and send it to TM
//
// Arguments:
//
// Returns:
//
// Prereqs:
//   None
//
// External references:
//   None
//
//////////////////////////////////////////////////////////////////////////////
INT_PTR CALLBACK EQFBENTRYDLGPROC
(
   HWND hwndDlg,                       /* handle of dialog window             */
   WINMSG msg,
   WPARAM mp1,
   LPARAM mp2
)
{
   MRESULT  mResult = MRFROMSHORT( FALSE );      // result value of procedure

   PENTRYDATA   pEntryData;                      // pointer to entry data
   PTBDOCUMENT  pDoc;                            // pointer to document
   USHORT       usRc;                            // return code from TM
   SHORT        sMatchFound;                     // match found
   PSZ          pData;                           // pointer to data
   PTBSEGMENT   pSeg;                            // pointer to segment..

   switch ( msg )
   {
      case WM_EQF_QUERYID: HANDLEQUERYID( ID_TB_ENTRY_DLG, mp2 ); break;

     case WM_INITDLG:
          SETWINDOWID( hwndDlg, ID_TB_ENTRY_DLG );
          // remember adress of user area
          pEntryData = (PENTRYDATA) mp2;

          ANCHORDLGIDA( hwndDlg, pEntryData );
          MLESETTEXTLIMIT( hwndDlg, ID_TB_ENTRY_MLE, (MAX_SEGMENT_SIZE-1));

          SetCtrlFnt (hwndDlg, pEntryData->pDoc->lf.lfCharSet,
                           ID_TB_ENTRY_MLE , 0 );

          MLESETTEXTW( hwndDlg, ID_TB_ENTRY_MLE, pEntryData->chSegBuffer );
          mResult = DIALOGINITRETURN( mResult );
          break;

      case WM_COMMAND:
         mResult = EQFBEntryCommand( hwndDlg, mp1, mp2 );
         break;

      case WM_EQF_CLOSE:
         pEntryData = ACCESSDLGIDA(hwndDlg, PENTRYDATA);
         if ( pEntryData->fChanged)
         {
            PSZ_W pszContext = NULL;

            // get previously active segment and send it to TM as foreground
            pDoc = pEntryData->pDoc;

            pSeg = EQFBGetSegW( pDoc->twin, pDoc->ulWorkSeg );
            pszContext = EQFBGetContext( pDoc->twin, pSeg, pDoc->ulWorkSeg );
            usRc = EQFTRANSSEG3W(pSeg->pDataW, pszContext, pSeg->pvMetadata, // ptr to seg data
                                (USHORT)pDoc->ulWorkSeg, // active seg number
                                TRUE,                    // foreground segment
                                FALSE,                   // no automatic mode
                                &sMatchFound);
            if (usRc != EQFRC_OK )
            {
               pData = EQFERRINS();           // get error message
               UtlError( EQFERRID(), MB_CANCEL, 1, &pData, EQF_ERROR );
            } /* endif */
         } /* endif */

         DelCtrlFont (hwndDlg, ID_TB_ENTRY_MLE);
         //--- get rid off dialog ---
         DISMISSDLG( hwndDlg, TRUE );
         break;

      default:
         mResult = WinDefDlgProc( hwndDlg, msg, mp1, mp2 );
         break;
   } /* endswitch */

   return mResult;
} /* end of EQFBEntryDlgProc */


/*////////////////////////////////////////////////////////////////////////////
:h2.EQFBEntryCommand - process WM_COMMAND messages of entry sentence dialog
*/
// Description:
//    Handle WM_COMMAND messages (= pressing of pushbuttons) of
//    entry sentence dialog panel.
//
//   Flow (message driven):
//      case 'LookUp' pushbutton:
//         get data from MLE and send it to Translation memory and dictionary
//         window
//      case CANCEL pushbutton or DID_CANCEL (= ESCAPE key):
//         post a WM_CLOSE message to dialog, mp1 = 0;
//
// Arguments:
//   SHORT1FROMMP(mp1) = ID of control sending the WM_COMMAND message
//
// Returns:
//  MRESULT(TRUE)  = command is processed
//
// Prereqs:
//   None
//
// SideEffects:
//
// External references:
//   None
//
//////////////////////////////////////////////////////////////////////////////
MRESULT EQFBEntryCommand
(
   HWND hwndDlg,
   WPARAM mp1,
   LPARAM mp2
)
{
   MRESULT mResult = MRFROMSHORT(TRUE);     // TRUE = command is processed
   PENTRYDATA   pEntryData;                 // pointer to entry data struct
   USHORT       usRc;                       // return code from TM
   SHORT        sMatchFound;                // number of found match
   PSZ          pData;                      //  pointer to error data

   mp2 = mp2;                               // get rid off compiler warning
   pEntryData = ACCESSDLGIDA(hwndDlg, PENTRYDATA);


   switch ( WMCOMMANDID( mp1, mp2 ) )
   {
          case ID_TB_ENTRY_HELP_PB:
             mResult = UtlInvokeHelp();
             break;
      case ID_TB_ENTRY_LOOKUP_PB:          // lookup sentence
         // get data filled in from buffer
         if ( pEntryData )
         {
           LONG l;
           PSZ_W p = pEntryData->chSegBuffer;
           PSZ_W q = p;
            USHORT usLen;
            MLEQUERYTEXTLENGTH( hwndDlg, ID_TB_ENTRY_MLE, usLen);
            MLEGETTEXTW( hwndDlg, ID_TB_ENTRY_MLE,
                        pEntryData->chSegBuffer, l );
            pEntryData->chSegBuffer[ usLen ] = EOS;

            /**********************************************************/
            /* convert CR LF into LF only                             */
            /**********************************************************/
            while ( *p )
            {
              if ( *p == CR )
              {
                /******************************************************/
                /* ignore                                             */
                /******************************************************/
                p++;
              }
              else
              {
                *q++ = *p++;
              } /* endif */
            } /* endwhile */
            *q = EOS;

            pEntryData->fChanged = TRUE;                 // proposal window upd.
            EQFCLEAR( EQFF_STANDARD );                   // clear send a head
            usRc = EQFTRANSSEGW(pEntryData->chSegBuffer, // pointer to seg data
                                0,                       // dummy number
                                TRUE,                    // foreground segment
                                FALSE,                   // no automatic mode
                                &sMatchFound);
            if (usRc != EQFRC_OK )
            {
               pData = EQFERRINS();           // get error message
               UtlError( EQFERRID(), MB_CANCEL, 1, &pData, EQF_ERROR );
            } /* endif */
         }
         else
         {
            BEEP( WA_WARNING );
         } /* endif */

         UtlDispatch(); // no additional check on IDA necessary - we leave here
         break;

      case ID_TB_ENTRY_MLE:
         if ( WMCOMMANDCMD( mp1, mp2 ) == EN_KILLFOCUS )
         {
           ClearIME( hwndDlg );
         } /* endif */
         break;

      case ID_TB_ENTRY_CANCEL_PB:
      case DID_CANCEL:
         POSTEQFCLOSE( hwndDlg, FALSE );
         break;

      default:
         mResult = WinDefDlgProc( hwndDlg, WM_COMMAND, mp1, mp2 );
         break;
   } /* endswitch */

   return( mResult );
} /* end of EQFBEntryCommand */


/*////////////////////////////////////////////////////////////////////////////
:h3.GetNumber - get the value of the margin
*/
// Description:
//    convert ascii string into value
//    return 0 if a character was entered
//
//
// Arguments:
//   PSZ      ptr to string to be converted
//
// Returns:
//   SHORT    value
//
// Prereqs:
//   None
//
// SideEffects:
//
// External references:
//   None
//
//////////////////////////////////////////////////////////////////////////////
static
SHORT GetNumber
(
   PSZ pszText
)
{
   SHORT sValue = 0;

   while ( *pszText == BLANK )
   {
      pszText++;
   } /* endwhile */
   while ( isdigit(*pszText) )
   {
      sValue = (sValue * 10) + (*pszText++ - '0');
   } /* endwhile */
   if ( *pszText )                  // some characters left
   {
      sValue = 0;
   } /* endif */

   return sValue;
}

/*////////////////////////////////////////////////////////////////////////////
:h3.GetLongNumber - get the number value in lng
*/
// Description:
//    convert ascii string into value
//    return 0 if a character was entered
//
//
// Arguments:
//   PSZ      ptr to string to be converted
//
// Returns:
//   LONG     value
//
// Prereqs:
//   None
//
// SideEffects:
//
// External references:
//   None
//
//////////////////////////////////////////////////////////////////////////////
static
LONG GetLongNumber
(
   PSZ pszText
)
{
   LONG lValue = 0;

   while ( *pszText == BLANK )
   {
      pszText++;
   } /* endwhile */
   while ( isdigit(*pszText) )
   {
      lValue = (lValue * 10) + (*pszText++ - '0');
   } /* endwhile */
   if ( *pszText )                  // some characters left
   {
      lValue = 0;
   } /* endif */

   return lValue;
}


//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     EQFBGotoLineDlgProc
//------------------------------------------------------------------------------
// Function call:     EQFBGotoLineDlgProc via WinLoadDlg
//------------------------------------------------------------------------------
// Description:       allow to go to a specified line
//------------------------------------------------------------------------------
// Parameters:        HWND hwndDlg    dialog handle
//                    USHORT  msg     message id
//                    WPARAM  mp1     message parameter 1
//                    LPARAM  mp2     message parameter 2
//------------------------------------------------------------------------------
// Returncode type:   MRESULT
//------------------------------------------------------------------------------
// Returncodes:       _
//------------------------------------------------------------------------------
// Function flow:     handle the following messages
//                      WM_INITDLG:
//                        call the init settings function
//                      WM_COMMAND:
//                        call the handle Command settings function
//                      WM_CLOSE
//                        Dismiss the dialog
//                      default:
//                        call the default dialog proc..
//------------------------------------------------------------------------------

INT_PTR CALLBACK EQFBGOTOLINEDLGPROC
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
      case WM_EQF_QUERYID: HANDLEQUERYID( ID_TB_GOTO_DLG, mp2 ); break;

      case WM_INITDLG:
          SETWINDOWID( hwndDlg, ID_TB_GOTO_DLG );
          SETTEXTLIMIT( hwndDlg, ID_TB_GOTO_EF, GOTOLINE_LENGTH );
          ANCHORDLGIDA( hwndDlg, mp2 );
          mResult = DIALOGINITRETURN( mResult );

          {
            PTBDOCUMENT pDoc = (PTBDOCUMENT)mp2;
            if ( pDoc->flags.fGotoSegMode )
            {
              SETTEXT( hwndDlg, ID_TB_GOTO_STATIC, "Segment" );
              SETTEXTHWND( hwndDlg, "Go to segment" );
            } /* endif */
          }
          break;

      case WM_COMMAND:
         mResult = EQFBGotoLineCommand( hwndDlg, mp1, mp2 );
         break;

      case WM_EQF_CLOSE:
         //--- get rid off dialog ---
         DISMISSDLG( hwndDlg, TRUE );
         break;


      default:
         mResult = WinDefDlgProc( hwndDlg, msg, mp1, mp2 );
         break;
   } /* endswitch */

   return mResult;
} /* end of EQFBGotoLineDlgProc */

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     EQFBGotoLineCommand
//------------------------------------------------------------------------------
// Function call:     EQFBGotoLineCommand( hwndDlg, mp1, mp2 );
//------------------------------------------------------------------------------
// Description:       handle the commandline message
//------------------------------------------------------------------------------
// Parameters:        HWND  hwndDlg   handle of dialog
//                    WPARAM  mp1     message parameter 1
//                    LPARAM  mp2     message parameter 2
//------------------------------------------------------------------------------
// Returncode type:   MRESULT
//------------------------------------------------------------------------------
// Returncodes:       TRUE
//------------------------------------------------------------------------------
// Function flow:     handle the command messages, i.e.
//                      case GOTO_PB:
//                        get the number and try to position at this line.
//                        display error message if number is invalid or line
//                         too big.
//                      case DID_CANCEL:
//                      case GOTO_CANCEL:
//                        post a WM_CLOSE message
//                      default:
//                        none
//
//                    return TRUE;
//------------------------------------------------------------------------------

MRESULT EQFBGotoLineCommand
(
   HWND hwndDlg,
   WPARAM mp1,
   LPARAM mp2
)
{
   MRESULT mResult = MRFROMSHORT(TRUE);     // TRUE = command is processed
   CHAR  chText[ GOTOLINE_LENGTH+1];      // for conversion of integers/ascii
   PSZ   pData;                           // pointer to error text
   LONG    lValue;                          // value of entered input text
   PTBDOCUMENT  pDoc;                       // pointer to document ida

   mp2 = mp2;                               // get rid off compiler warning

   switch ( WMCOMMANDID( mp1, mp2 ) )
   {
          case ID_TB_GOTO_HELP_PB:
             mResult = UtlInvokeHelp();
             break;
      case ID_TB_GOTO_GOTO_PB:              // get the line number and position
         // get text/value of entry field
         QUERYTEXT( hwndDlg, ID_TB_GOTO_EF, chText);
         // check for validity
         lValue = GetLongNumber( chText );
         pDoc = ACCESSDLGIDA(hwndDlg, PTBDOCUMENT);
         if ( pDoc->flags.fGotoSegMode )
         {
           if ( (lValue >= 1) && (lValue < (LONG)pDoc->ulMaxSeg) )
           {
             // postion at segment and close
             EQFBGotoSeg( pDoc, lValue, 0 );
             POSTEQFCLOSE( hwndDlg, FALSE );
           }
           else
           {
             // display error message
             pData = chText;
             UtlError( TB_NOVALIDSEGNUM, MB_CANCEL, 1, &pData, EQF_ERROR );
             SETFOCUS( hwndDlg, ID_TB_GOTO_EF );
           } /* endif */

         }
         else
         {
          if ( lValue > 0l )
          {
            /***********************************************************/
            /* position at line and close                              */
            /***********************************************************/
            if ( EQFBFindLine( pDoc, lValue )  )
            {
              POSTEQFCLOSE( hwndDlg, FALSE );
            }
            else
            {
              /*********************************************************/
              /* line number too high ...                              */
              /*********************************************************/
              pData = chText;
              UtlError( TB_NOVALIDLINENUM, MB_CANCEL, 1, &pData, EQF_ERROR );
              SETFOCUS( hwndDlg, ID_TB_GOTO_EF );
            } /* endif */
          }
          else
          {
            // display error message
            pData = chText;
            UtlError( TB_NOVALIDLINENUM, MB_CANCEL, 1, &pData, EQF_ERROR );
            SETFOCUS( hwndDlg, ID_TB_GOTO_EF );
          } /* endif */
         } /* endif */
         break;


      case ID_TB_GOTO_CANCEL_PB:
      case DID_CANCEL:
         POSTEQFCLOSE( hwndDlg, FALSE );
         break;

      default:
         mResult = WinDefDlgProc( hwndDlg, WM_COMMAND, mp1, mp2 );
         break;
   } /* endswitch */


   return( mResult );
} /* end of EQFBGotoLineCommand */


/**********************************************************************/
/* Subclass procedure for spinbuttons to handle WM_CLOSE message      */
/* (This message is sent by the dialog handler of Windows to the      */
/* spinbutton and not to the dialog procedure when the ESCAPE key     */
/* is pressed. The spinbutton removeds itself on receiving WM_CLOSE)  */
/**********************************************************************/
LONG FAR PASCAL ProfSpinSubclassProc
(
  HWND hwnd,
  SHORT msg,
  WPARAM mp1,
  LPARAM mp2
)
{
  switch ( msg )
  {
    case WM_CLOSE:
      /****************************************************************/
      /* Redirect WM_CLOSE to dialog procedure                        */
      /****************************************************************/
      PostMessage( GetParent( hwnd ), WM_CLOSE, 0, 0L );
      return( 0L );
  } /* endswitch */
  return( CallWindowProc( (WNDPROC) pfnOrgProfSpinProc, hwnd, msg, mp1, mp2 ) );
} /* end of function SpinSubclassProc */

/**********************************************************************/
/* force update of TBDOCUMENT structure of all loaded documents       */
/**********************************************************************/
static
VOID UpdateDocSettings
(
  PTBDOCUMENT pDoc
)
{
  USHORT   usLen;                   // length of style abbreviation
  USEROPT* pEQFBUserOpt = get_EQFBUserOpt();
  
    ASCII2Unicode(pEQFBUserOpt->chTRNoteAbbr, pDoc->chTRNoteLFAbbrW, 0L);           // and compact
    usLen = (USHORT)UTF16strlenCHAR(pDoc->chTRNoteLFAbbrW);
    pDoc->chTRNoteLFAbbrW[usLen] = LF;
    pDoc->chTRNoteLFAbbrW[usLen+1] = EOS;

    ASCII2Unicode(pEQFBUserOpt->szInTagAbbr, pDoc->szInTagAbbrW, 0L);
    ASCII2Unicode(pEQFBUserOpt->szOutTagAbbr, pDoc->szOutTagAbbrW, 0L);   // and compact

    ASCII2Unicode(pEQFBUserOpt->szInTagAbbr, pDoc->szInTagLFAbbrW, 0L);  //set abbreviations for SHRINK
    ASCII2Unicode(pEQFBUserOpt->szOutTagAbbr, pDoc->szOutTagLFAbbrW, 0L);           // and compact
    usLen = (USHORT)UTF16strlenCHAR(pDoc->szInTagLFAbbrW);
    pDoc->szInTagLFAbbrW[usLen] = LF;
    pDoc->szInTagLFAbbrW[usLen+1] = EOS;

    usLen = (USHORT)UTF16strlenCHAR(pDoc->szOutTagLFAbbrW);
    pDoc->szOutTagLFAbbrW[usLen] = LF;
    pDoc->szOutTagLFAbbrW[usLen+1] = EOS;
// get the visible whitespaces in Unicode -- remember, they are stored in ANSI
    UtlGetUTF16VisibleWhiteSpace(pDoc, pEQFBUserOpt, 0L);

    EQFBChangeStyle(pDoc,(USHORT)pDoc->DispStyle);        // update screen
}


//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     EQFBFuncTOCGoto
//------------------------------------------------------------------------------
// Function call:     EQFBFuncTOCGoto(pDoc)
//------------------------------------------------------------------------------
// Description:       invoke the TOC goto  dialog ...
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT  pDoc   pointer to document structure
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:     invoke the dialog and handle error conditions ..
//------------------------------------------------------------------------------

VOID
EQFBFuncTOCGoto
(
   PTBDOCUMENT pDoc
)
{
   INT_PTR        iRc;
   HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);


   DIALOGBOXW( pDoc->hwndClient, EQFBTOCGOTODLGPROC, hResMod,
              ID_TB_TOCGOTO_DLG, pDoc, iRc );

   if ( iRc == DID_ERROR )
   {
     UtlError( ERROR_DIALOG_LOAD_FAILED, MB_CANCEL, 0 , NULL, EQF_ERROR );
   } /* endif */
} /* end of function EQFBFuncTOCGoto */

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     EQFBTOCGotoDlgProc
//------------------------------------------------------------------------------
// Function call:     EQFBTOCGotoDlgProc  via WinLoadDlg
//------------------------------------------------------------------------------
// Description:       allow to go to a table of contents item
//------------------------------------------------------------------------------
// Parameters:        HWND hwndDlg    dialog handle
//                    USHORT  msg     message id
//                    WPARAM  mp1     message parameter 1
//                    LPARAM  mp2     message parameter 2
//------------------------------------------------------------------------------
// Returncode type:   MRESULT
//------------------------------------------------------------------------------
// Returncodes:       _
//------------------------------------------------------------------------------
// Function flow:     handle the following messages
//                      WM_INITDLG:
//                        call the init          function
//                      WM_COMMAND:
//                        call the handle
//                      WM_CLOSE
//                        Dismiss the dialog
//                      default:
//                        call the default dialog proc..
//------------------------------------------------------------------------------

INT_PTR CALLBACK EQFBTOCGOTODLGPROC
(
   HWND hwndDlg,                       /* handle of dialog window             */
   WINMSG msg,
   WPARAM mp1,
   LPARAM mp2
)
{
   MRESULT  mResult = MRFROMSHORT( FALSE );                // result value of procedure
   BOOL     fOK;
   PTOCGOTOIDA  pTOCIda;

   switch ( msg )
   {
      case WM_EQF_QUERYID: HANDLEQUERYID( ID_TB_TOCGOTO_DLG, mp2 ); break;

      case WM_INITDLG:
          SETWINDOWID( hwndDlg, ID_TB_TOCGOTO_DLG );
          /*******************************************************************/
          /* create and anchor IDA                                           */
          /*******************************************************************/
          fOK = UtlAlloc((PVOID *) &pTOCIda, 0L,
                         (LONG) sizeof(TOCGOTOIDA), ERROR_STORAGE );
          if ( fOK )
          {
             fOK = ANCHORDLGIDA( hwndDlg, pTOCIda );
          } /* endif */
          pTOCIda->pDoc = (PTBDOCUMENT) mp2;
          pTOCIda->hwndLB = pTOCIda->pDoc->hwndTOCGotoLB;
          EQFBTOCGotoInit( hwndDlg,mp1,mp2 );
          mResult = DIALOGINITRETURN( mResult );
          break;

        case DM_GETDEFID:
          /************************************************************/
          /* check if user pressed the ENTER key, but wants only to   */
          /* select/deselect an item of the listbox via a simulated   */
          /* (keystroke) double click.                                */
          /************************************************************/
          if ((GetFocus() == GetDlgItem(hwndDlg, ID_TB_TOCGOTO_LB)) &&
               (GetKeyState(VK_RETURN) & 0x8000)  )
          {
            mResult = EQFBTOCGotoControl( hwndDlg, ID_TB_TOCGOTO_LB,
                                           LN_ENTER );
          } /* endif */
          break;

      case WM_COMMAND:
         mResult = EQFBTOCGotoCommand( hwndDlg, mp1, mp2 );
         break;

      case WM_EQF_CLOSE:
         //--- get rid off dialog ---
         pTOCIda = ACCESSDLGIDA(hwndDlg, PTOCGOTOIDA);
         if ( pTOCIda )
         {
           UtlAlloc((PVOID *) &pTOCIda, 0L, 0L, NOMSG );
         } /* endif */
         DelCtrlFont (hwndDlg, ID_TB_TOCGOTO_LB);
         DISMISSDLG( hwndDlg, TRUE );
         break;


      default:
         mResult = WinDefDlgProc( hwndDlg, msg, mp1, mp2 );
         break;
   } /* endswitch */

   return mResult;
} /* end of EQFBTOCGotoDlgProc */

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     EQFBTOCGotoCommand
//------------------------------------------------------------------------------
// Function call:     EQFBTOCGotoCommand( hwndDlg, mp1, mp2 );
//------------------------------------------------------------------------------
// Description:       handle the commandline message
//------------------------------------------------------------------------------
// Parameters:        HWND  hwndDlg   handle of dialog
//                    WPARAM  mp1     message parameter 1
//                    LPARAM  mp2     message parameter 2
//------------------------------------------------------------------------------
// Returncode type:   MRESULT
//------------------------------------------------------------------------------
// Returncodes:       TRUE
//------------------------------------------------------------------------------
// Function flow:     handle the command messages, i.e.
//                      case GOTO_PB:
//                        get the number and try to position at this line.
//                        display error message if number is invalid or line
//                         too big.
//                      case DID_CANCEL:
//                      case GOTO_CANCEL:
//                        post a WM_CLOSE message
//                      default:
//                        none
//
//                    return TRUE;
//------------------------------------------------------------------------------

  MRESULT EQFBTOCGotoCommand
  (
     HWND hwndDlg,
     WPARAM mp1,
     LPARAM mp2
  )
  {
     MRESULT mResult = MRFROMSHORT(TRUE);        // TRUE = command is processed
     PTBDOCUMENT  pDoc;                          // pointer to document ida
     USHORT       usGotoSegNum = 0;
     PTOCGOTOIDA  pTOCIda;
     SHORT        sItem;
     BOOL         fOK = TRUE;

     mp2 = mp2;                               // get rid off compiler warning

     pTOCIda = ACCESSDLGIDA(hwndDlg, PTOCGOTOIDA);

     switch ( SHORT1FROMMP1(mp1) )
     {
                case ID_TB_TOCGOTO_HELP_PB:
          mResult = UtlInvokeHelp();
          break;
        case ID_TB_TOCGOTO_GOTO_PB:
           sItem = QUERYSELECTION( hwndDlg, ID_TB_TOCGOTO_LB );
           if ( sItem == LIT_NONE )
           {
              UtlError( NO_FILE_SELECTED, MB_CANCEL, 0, NULL, EQF_ERROR );
              fOK = FALSE;
           }
           else
           {
              usGotoSegNum = GETITEMHANDLE( hwndDlg, ID_TB_TOCGOTO_LB, \
                                            sItem, USHORT );
           } /* endif */

           if ( usGotoSegNum == 0 )
           {
             WinAlarm( HWND_DESKTOP, WA_WARNING );    // issue a beep
           }
           else
           {
             pDoc = pTOCIda->pDoc;
             EQFBGotoSeg(pDoc, usGotoSegNum, 0);
             pDoc->Redraw |= REDRAW_ALL;   // force repaint...
             EQFBScreenData( pDoc );
             WinShowWindow( pDoc->hwndFrame, TRUE );
             POSTEQFCLOSE( hwndDlg, TRUE );
           } /* endif */
           break;


        case ID_TB_TOCGOTO_CANCEL_PB:
        case DID_CANCEL:
           POSTEQFCLOSE( hwndDlg, FALSE );
           break;
        case ID_TB_TOCGOTO_LB:
          mResult = EQFBTOCGotoControl( hwndDlg, ID_TB_TOCGOTO_LB,
                                        WMCOMMANDCMD( mp1, mp2 ) );
          break;
        default:
           mResult = WinDefDlgProc( hwndDlg, WM_COMMAND, mp1, mp2 );
           break;
     } /* endswitch */


     return( mResult );
  } /* end of EQFBTOCGotoCommand */


//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     EQFBTOCGotoInit
//------------------------------------------------------------------------------
// Function call:     EQFBTOCGotoInit(hwndDlg );
//------------------------------------------------------------------------------
// Description:       do the initial settings
//------------------------------------------------------------------------------
// Parameters:        HWND  hwndDlg     dialog handle
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:     fill entry field with right margin
//------------------------------------------------------------------------------
static VOID
EQFBTOCGotoInit
(
  HWND  hwndDlg,
  WPARAM mp1,
  LPARAM mp2
)
{
   PTOCGOTOIDA   pTOCIda;
   BOOL          fLBFilled = FALSE;
   mp1;
   mp2;

   pTOCIda = ACCESSDLGIDA(hwndDlg, PTOCGOTOIDA);

   SetCtrlFnt (hwndDlg, GetCharSet(),
                    ID_TB_TOCGOTO_LB, ID_TB_TOCGOTO_LB );

   DELETEALL( hwndDlg, ID_TB_TOCGOTO_LB );
   fLBFilled = EQFBTOCGotoFillLB(hwndDlg, pTOCIda);
   if (fLBFilled )
   {
     SELECTITEM( hwndDlg, ID_TB_TOCGOTO_LB, 0 );
   }
   else
   {
     ENABLECTRL( hwndDlg, ID_TB_TOCGOTO_GOTO_PB, FALSE );
   } /* endif */

} /* end of function EQFBTOCGotoInit */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     EQFBTOCGotoFillLB
//------------------------------------------------------------------------------
// Function call:     EQFBTOCGotoFillLB(hwndDlg );
//------------------------------------------------------------------------------
// Description:       fill the TOC listbox with table of contents
//------------------------------------------------------------------------------
// Parameters:        HWND  hwndDlg     dialog handle
//------------------------------------------------------------------------------
// Returncode type:   BOOL      0 no entry found
//------------------------------------------------------------------------------
// Function flow:     if TOC user exit available
//                       invoke user exit to fill lb
//                    else
//                     start with 1st seg of file
//                     while not at end of file
//                       tokenize segment data
//                       while token available in segment
//                        if tokenid >=0 ( not text) and  classid is CLS_HEAD
//                          add all following tokens at end of toc lb
//                          replace linefeeds by blanks and convert
//                          characters from OEM to ANSI
//                        endif
//                      endwhile
//                      goto next segment
//                     endwhile
//------------------------------------------------------------------------------

static BOOL
EQFBTOCGotoFillLB
(
  HWND         hwndDlg,
  PTOCGOTOIDA  pTOCIda
)
{
   ULONG         ulSegNum;
   PTBSEGMENT    pSeg;
   PCHAR_W       pRest = NULL;          // ptr to start of not-processed bytes
   USHORT        usColPos = 0;          // column pos used by EQFTagTokenize
   PTOKENENTRY   pTok;
   PTBDOCUMENT   pDoc;
   SHORT         sItem = LIT_NONE;
   USHORT        usLen;
// USHORT        usIndex = 0;


   pDoc = pTOCIda->pDoc;

   if (pDoc->pfnTocGoto )
   {
     /*****************************************************************/
     /* call userexit EQFTOCGOTO                                      */
     /*****************************************************************/
     sItem = (SHORT) pDoc->pfnTocGoto((LONG)pDoc,
                       WinWindowFromID(hwndDlg,ID_TB_TOCGOTO_LB ));
   }
   else
   {
     /*****************************************************************/
     /* find  entries for table of contents without userexit          */
     /*****************************************************************/
     CHAR_W  chItemBuf[MAXTOCLEN];
     ulSegNum = 1;
     pSeg = EQFBGetVisSeg(pDoc, &ulSegNum);
     while ( pSeg )
     {
       TATagTokenizeW( pSeg->pDataW,
                       (PLOADEDTABLE)pDoc->pDocTagTable,
                       TRUE,
                       &pRest,
                       &usColPos,
                       (PTOKENENTRY) pDoc->pTokBuf,
                       TOK_BUFFER_SIZE / sizeof(TOKENENTRY) );
       pTok = (PTOKENENTRY) pDoc->pTokBuf;
       while ( pTok->sTokenid != ENDOFLIST )
       {
         if ( pTok->sTokenid >= 0 )
         {
           if ( pTok->ClassId == CLS_HEAD )
           {
             /*****************************************************/
             /* add to listbox without CLS_HEAD token and without */
             /* linefeeds                                         */
             /*****************************************************/
             pTok++;
             EQFBTOCGotoFillBuffer(pDoc, &pTok, &chItemBuf[0], &usLen);
             /*********************************************************/
             /* if buffer is empty, add next segment into text buffer */
             /*********************************************************/
             if ( (usLen == 0) ||
                  ((usLen == 1) && (chItemBuf[0] == BLANK ) ))
             {
               ulSegNum ++;
               pSeg = EQFBGetVisSeg(pDoc, &ulSegNum);
               if (pSeg )
               {
                  TATagTokenizeW( pSeg->pDataW,
                                  (PLOADEDTABLE)pDoc->pDocTagTable,
                                  TRUE,
                                  &pRest,
                                  &usColPos,
                                  (PTOKENENTRY) pDoc->pTokBuf,
                                  TOK_BUFFER_SIZE / sizeof(TOKENENTRY) );
                  pTok = (PTOKENENTRY) pDoc->pTokBuf;
                  EQFBTOCGotoFillBuffer(pDoc, &pTok, &chItemBuf[0], &usLen);
               } /* endif */
             } /* endif */
             /*********************************************************/
             /* add non-empty lines only                              */
             /*********************************************************/
             if (usLen > 0 )
             {
               if (!((usLen == 1) && (chItemBuf[0] == BLANK ) ))
               {
                 sItem = INSERTITEMENDW( hwndDlg, ID_TB_TOCGOTO_LB, &chItemBuf);
                 if ( sItem != LIT_NONE )
                 {
                   SETITEMHANDLE( hwndDlg, ID_TB_TOCGOTO_LB, sItem, pSeg->ulSegNum );
                 } /* endif */
               } /* endif */
             } /* endif */
           } /* endif */
         } /* endif */
         if (pTok->sTokenid != ENDOFLIST )
         {
           pTok++;
         } /* endif */
       } /* endwhile */
       ulSegNum ++;
       pSeg = EQFBGetVisSeg(pDoc, &ulSegNum);
     } /* endwhile */
     if (sItem == LIT_NONE )
     {
       HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
       /******************************************************************/
       /* if no toc    found in document, "no entry available" is dis-   */
       /* played in toc listbox                                          */
       /******************************************************************/
       CHAR  chItemString[MAXTOCLEN];
       WinLoadString ((HAB) UtlQueryULong( QL_HAB ), hResMod, SID_NO_ENTRY,
                      sizeof(chItemString), chItemString );
       INSERTITEMEND( hwndDlg, ID_TB_TOCGOTO_LB, &chItemString);
     } /* endif */

   } /* endif */

   return ( sItem != LIT_NONE );

} /* end of function EQFBTOCGotoFillLB */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     EQFBTOCGotoFillBuffer
//------------------------------------------------------------------------------
// Function call:     EQFBTOCGotoFillBuffer(hwndDlg);
//------------------------------------------------------------------------------
// Description:       fill buffer with given data
//------------------------------------------------------------------------------
// Parameters:        HWND  hwndDlg     dialog handle
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:     fill buffer with given data
//------------------------------------------------------------------------------
static VOID
EQFBTOCGotoFillBuffer
(
  PTBDOCUMENT  pDoc,
  PTOKENENTRY *ppTokStart,
  PCHAR_W      pBuffer,
  PUSHORT      pusLen
)
{
   PTOKENENTRY  pTok;
   LONG         lCurLen = 0;
   CHAR_W       chTemp;
   USHORT       usIndex;
   int          i;        // index

   pDoc;
   pTok = *ppTokStart;
   lCurLen = 0L;
//   memset(pBuffer, 0,MAXTOCLEN);
   for (i=0; i< MAXTOCLEN; i++)
   {
   pBuffer[i] = 0;
   }

   while (pTok->sTokenid != ENDOFLIST )
   {
     if (pTok->sTokenid < 0 )
     {
       lCurLen += pTok->usLength;
       if (lCurLen < MAXTOCLEN )
       {
         chTemp = *(pTok->pDataStringW+pTok->usLength);
         *(pTok->pDataStringW+pTok->usLength) = EOS;

         UTF16strcat(pBuffer, pTok->pDataStringW);
         *(pTok->pDataStringW+pTok->usLength) = chTemp;
       } /* endif */
     } /* endif */
     pTok++;
   } /* endwhile */
   /*********************************************************/
   /* replace linefeeds and softlinefeeds by blanks         */
   /*********************************************************/
   usIndex = 0;
   while (usIndex < lCurLen )
   {
     switch ( *(pBuffer+usIndex) )
     {
       case LF:
           *(pBuffer + usIndex) = BLANK;
         break;
       case SOFTLF_CHAR:
           if (*(pBuffer + usIndex + 1) != SOFTLF_CHAR )
           {
             *(pBuffer+usIndex) = BLANK;
           }
           else
           {
             usIndex++;        //skip duplicated softlf
           } /* endif */
         break;
       default :
         break;
     } /* endswitch */
     usIndex++;
   } /* endwhile */
   *pusLen = (USHORT) lCurLen;
   *ppTokStart = pTok;
} /* end of function EQFBTOCGotoFillBuffer  */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     EQFBTOCGotoControl
//------------------------------------------------------------------------------
// Function call:     EQFBTOCGotoControl(hwndDlg );
//------------------------------------------------------------------------------
// Description:       initiate same handling as if GOTO pushbutton is pressed
//------------------------------------------------------------------------------
// Parameters:        HWND  hwndDlg     dialog handle
//                    SHORT  sId        id in action
//                    SHORT  sNotification   notification
//------------------------------------------------------------------------------
// Returncode type:   MRESULT
//------------------------------------------------------------------------------
// Function flow:     goto the selected toc listbox item
//------------------------------------------------------------------------------
MRESULT EQFBTOCGotoControl
(
   HWND   hwndDlg,                     // dialog handle
   SHORT  sId,                         // id in action
   SHORT  sNotification                // notification
)
{
   PTOCGOTOIDA pIda;
   MRESULT     mResult = FALSE;        // result of message processing


   pIda = ACCESSDLGIDA(hwndDlg, PTOCGOTOIDA);

   if (sId == ID_TB_TOCGOTO_LB )
   {
     switch ( sNotification )
     {
       case  LN_ENTER:
         EQFBTOCGotoCommand( hwndDlg, MP1FROMSHORT( ID_TB_TOCGOTO_GOTO_PB ), 0L );
         break;
       default :
         break;
     } /* endswitch */
   } /* endif */
   return( mResult );
} /* end of EQFBTOCGOTOControl */



static
VOID UpdateAutoLineWrapSettings
(
  PTBDOCUMENT pDoc,
  BOOL        fAutoLineWrap,
  USHORT      sMargin
)
{
  if (fAutoLineWrap )
  {
    pDoc->fAutoLineWrap = TRUE;
    pDoc->sRMargin = (SHORT)pDoc->lScrnCols;
    if (pDoc->fLineWrap )
    {
      EQFBSoftLFInsert( pDoc );
    } /* endif */
  }
  else
  {
    pDoc->sRMargin = sMargin;
    pDoc->fAutoLineWrap = FALSE;
    if (pDoc->fLineWrap )
    {
      EQFBSoftLFRemove( pDoc );
      if (pDoc->pUndoSegW )
      {
        USHORT  usBufSize = 0;
        USHORT  usOffset = 0;
        EQFBBufRemoveSoftLF( pDoc->hwndRichEdit, pDoc->pUndoSegW, &usBufSize, &usOffset);
      } /* endif */
    } /* endif */
  } /* endif */
  if (pDoc->fAutoSpellCheck && pDoc->pvSpellData )
  { // force that thread recalcs pusHLType of Screen
           PSPELLDATA pSpellData = (PSPELLDATA) pDoc->pvSpellData;
           pSpellData->TBFirstLine.ulSegNum = 0;
           pSpellData->TBFirstLine.usSegOffset = (USHORT)-1; // cannot be segoffs
  }

}

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     EQFBProfInitDlgProc
//------------------------------------------------------------------------------
// Function call:     EQFBProfInitDlgProc via WinLoadDlg
//------------------------------------------------------------------------------
// Description:       allow to do some special profile settings
//------------------------------------------------------------------------------
// Parameters:        HWND hwndDlg    dialog handle
//                    USHORT  msg     message id
//                    WPARAM  mp1     message parameter 1
//                    LPARAM  mp2     message parameter 2
//------------------------------------------------------------------------------
// Returncode type:   MRESULT
//------------------------------------------------------------------------------
// Returncodes:       _
//------------------------------------------------------------------------------
// Function flow:     handle the following messages
//                      WM_INITDLG:
//                        call the initial values function
//                      WM_COMMAND:
//                        call the handle Command settings function
//                      WM_NOTIFY
//                        handle any notifications for the TabCtrl
//                      WM_CLOSE
//                        Dismiss the dialog
//                      default:
//                        call the default dialog proc..
//------------------------------------------------------------------------------

INT_PTR CALLBACK EQFBPROFINITDLGPROC
(
   HWND hwndDlg,                       /* handle of dialog window             */
   WINMSG msg,
   WPARAM mp1,
   LPARAM mp2
)
{
   MRESULT  mResult = MRFROMSHORT( FALSE );                // result value of procedure
   LONG     lTabCtrl;
   PPROFONEIDA  pIda;

   switch ( msg )
   {
      case WM_EQF_QUERYID: HANDLEQUERYID( ID_TB_PROPERTIES_DLG, mp2 ); break;

      case WM_INITDLG:
          PropertySheetLoad( hwndDlg, mp2 );
          mResult = DIALOGINITRETURN( mResult );
          break;


      case WM_COMMAND:
         mResult = EQFBPropCommand( hwndDlg, mp1, mp2 );
         break;

      case WM_NOTIFY:
         mResult = PropertySheetNotification( hwndDlg, mp1, mp2 );
         break;

      case WM_HELP:
         /*************************************************************/
         /* pass on a HELP_WM_HELP request                            */
         /*************************************************************/
         EqfDisplayContextHelp( (HWND)((LPHELPINFO) mp2)->hItemHandle,
                                &hlpsubtblPropSettingsDlg[0] );
         mResult = TRUE;  // message processed
         break;


      case WM_EQF_CLOSE:
         //--- get rid off dialog ---
         pIda = ACCESSDLGIDA(hwndDlg, PPROFONEIDA);
         if ( pIda )
         {
           USHORT nItem = 0;
           /***********************************************************/
           /* free all allocated pages as well as the registration    */
           /* of the modeless dialog                                  */
           /***********************************************************/
           while ( pIda->hwndPages[nItem] )
           {
             UtlUnregisterModelessDlg( pIda->hwndPages[nItem] );
             DestroyWindow( pIda->hwndPages[nItem] );
             nItem++;
           } /* endwhile */

           UtlAlloc((PVOID *) &pIda, 0L, 0L, NOMSG );
         } /* endif */
         DISMISSDLG( hwndDlg, TRUE );
         break;

      case TCM_SETCURSEL:
                 pIda = ACCESSDLGIDA(hwndDlg, PPROFONEIDA);
         lTabCtrl = TabCtrl_GetCurSel( GetDlgItem( hwndDlg, ID_TB_PROP_TABCTRL ) );
         ShowWindow( pIda->hwndPages[ lTabCtrl ], SW_HIDE );
         TabCtrl_SetCurSel( GetDlgItem( hwndDlg, ID_TB_PROP_TABCTRL ), mp1 );
         ShowWindow( pIda->hwndPages[ mp1 ], SW_SHOW );
         break;

      default:
         mResult = WinDefDlgProc( hwndDlg, msg, mp1, mp2 );
         break;
   } /* endswitch */

   return mResult;
} /* end of EQFBProfInitDlgProc */


//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     PropertySheetLoad
//------------------------------------------------------------------------------
// Function call:     PropertySheetLoad( hwndDlg, mp2 );
//------------------------------------------------------------------------------
// Description:       handle changes on the tab page
//------------------------------------------------------------------------------
// Parameters:        HWND hwndDlg   handle of the dialog
//                    LPARAM  mp2    message parameter 2
//------------------------------------------------------------------------------
// Returncode type:   MRESULT
//------------------------------------------------------------------------------
// Returncodes:       return code from default window proc or FALSE
//------------------------------------------------------------------------------
// Function flow:     create any pages,
//                    load the tabctrl text
//                    load the (modeless) dialog, register it and position into
//                      tab area
//                    return
//------------------------------------------------------------------------------

BOOL PropertySheetLoad
(
  HWND hwndDlg,
  LPARAM mp2
)
{
  BOOL      fOK = TRUE;
  TC_ITEM   TabCtrlItem;
  USHORT    nItem = 0;
  HWND      hwndTabCtrl;
  HINSTANCE hInst;
  PPROFONEIDA  pIda;                             // dialog ida
  CHAR      szBuffer[80];

  SETWINDOWID( hwndDlg, ID_TB_PROFONE_DLG );
  fOK = UtlAlloc((PVOID *) &pIda, 0L, (LONG) sizeof(PROFONEIDA), ERROR_STORAGE );
  if ( fOK )
  {
     fOK = ANCHORDLGIDA( hwndDlg, pIda );
  } /* endif */
  if ( fOK )
  {
    RECT rect;
    USHORT usI;
    HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);

    // remember adress of user area
    pIda->pDoc = (PTBDOCUMENT) mp2;
    hInst = GETINSTANCE( hwndDlg );
    hwndTabCtrl = GetDlgItem( hwndDlg, ID_TB_PROP_TABCTRL );


    TabCtrlItem.mask = TCIF_TEXT;
    /******************************************************************/
    /* create the appropriate TAB control and load the associated     */
    /* dialog                                                         */
    /******************************************************************/
    LOADSTRING( hab, hResMod, IDS_TB_TABCTRL_EDITOR, szBuffer );
    TabCtrlItem.pszText = szBuffer;
    SendMessage(hwndTabCtrl, TCM_INSERTITEM, nItem, (LPARAM)&TabCtrlItem);
    pIda->hwndPages[nItem] =
                 CreateDialogParam( hInst,
                                    MAKEINTRESOURCE( ID_TB_PROP_EDITOR_DLG ),
                                    hwndDlg,
                                    EQFBPROP_EDITOR_DLGPROC,
                                    (LPARAM)pIda );

    UtlRegisterModelessDlg( pIda->hwndPages[nItem] );
    nItem++;

    LOADSTRING( hab, hResMod, IDS_TB_TABCTRL_DISPLAY, szBuffer );
    TabCtrlItem.pszText = szBuffer;
    SendMessage(hwndTabCtrl, TCM_INSERTITEM, nItem, (LPARAM)&TabCtrlItem);
    pIda->hwndPages[nItem] =
                 CreateDialogParam( hInst,
                                    MAKEINTRESOURCE( ID_TB_PROP_DISPLAY_DLG ),
                                    hwndDlg,
                                    EQFBPROP_DISPLAY_DLGPROC,
                                    (LPARAM)pIda );
    UtlRegisterModelessDlg( pIda->hwndPages[nItem] );
    nItem++;

    LOADSTRING( hab, hResMod, IDS_TB_TABCTRL_DICTIONARY, szBuffer );
    TabCtrlItem.pszText = szBuffer;
    SendMessage(hwndTabCtrl, TCM_INSERTITEM, nItem, (LPARAM)&TabCtrlItem);
    pIda->hwndPages[nItem] =
                 CreateDialogParam( hInst,
                                    MAKEINTRESOURCE( ID_TB_PROP_DICTIONARY_DLG ),
                                    hwndDlg,
                                    EQFBPROP_DICTIONARY_DLGPROC,
                                    (LPARAM)pIda );
    UtlRegisterModelessDlg( pIda->hwndPages[nItem] );
    nItem++;

    LOADSTRING( hab, hResMod, IDS_TB_TABCTRL_TRANSLMEM, szBuffer );
    TabCtrlItem.pszText = szBuffer;
    SendMessage(hwndTabCtrl, TCM_INSERTITEM, nItem, (LPARAM)&TabCtrlItem);
    pIda->hwndPages[nItem] =
                 CreateDialogParam( hInst,
                                    MAKEINTRESOURCE( ID_TB_PROP_TRANSLMEM_DLG ),
                                    hwndDlg,
                                    EQFBPROP_TRANSLMEM_DLGPROC,
                                    (LPARAM)pIda );
    UtlRegisterModelessDlg( pIda->hwndPages[nItem] );
    nItem++;

    LOADSTRING( hab, hResMod, IDS_TB_TABCTRL_MESSAGES, szBuffer );
    TabCtrlItem.pszText = szBuffer;
    SendMessage(hwndTabCtrl, TCM_INSERTITEM, nItem, (LPARAM)&TabCtrlItem);
    pIda->hwndPages[nItem] =
                 CreateDialogParam( hInst,
                                    MAKEINTRESOURCE( ID_TB_PROP_MESSAGES_DLG ),
                                    hwndDlg,
                                    EQFBPROP_MESSAGES_DLGPROC,
                                    (LPARAM)pIda );
    UtlRegisterModelessDlg( pIda->hwndPages[nItem] );
    nItem++;
#ifdef R004422_BACKSAVE
    LOADSTRING( hab, hResMod, IDS_TB_TABCTRL_BACKTASKS, szBuffer );
    TabCtrlItem.pszText = szBuffer;
    SendMessage(hwndTabCtrl, TCM_INSERTITEM, nItem, (LPARAM)&TabCtrlItem);
    pIda->hwndPages[nItem] =
                 CreateDialogParam( hInst,
                                    MAKEINTRESOURCE( ID_TB_PROP_BACKTASKS_DLG ),
                                    hwndDlg,
                                    EQFBPROP_BACKTASKS_DLGPROC,
                                    (LPARAM)pIda );
    UtlRegisterModelessDlg( pIda->hwndPages[nItem] );
    nItem++;
#endif

    LOADSTRING( hab, hResMod, IDS_TB_TABCTRL_AUTSUBST, szBuffer );
    TabCtrlItem.pszText = szBuffer;
    SendMessage(hwndTabCtrl, TCM_INSERTITEM, nItem, (LPARAM)&TabCtrlItem);
    pIda->hwndPages[nItem] =
                 CreateDialogParam( hInst,
                                    MAKEINTRESOURCE( ID_TB_PROP_AUTSUBST_DLG ),
                                    hwndDlg,
                                    EQFBPROP_AUTSUBST_DLGPROC,
                                    (LPARAM)pIda );
    UtlRegisterModelessDlg( pIda->hwndPages[nItem] );
    nItem++;

    /******************************************************************/
    /* adjust dialog postions                                         */
    /******************************************************************/
    GetClientRect( hwndTabCtrl, &rect );
    TabCtrl_AdjustRect( hwndTabCtrl, FALSE, &rect );
    MapWindowPoints( hwndTabCtrl, hwndDlg, (POINT *) &rect, 2 );
    for (usI=0; usI < nItem; usI++)
    {
      SetWindowPos( pIda->hwndPages[usI], HWND_TOP,
                    rect.left, rect.top,
                    rect.right-rect.left, rect.bottom-rect.top, 0 );
    } /* endfor */
    ShowWindow( pIda->hwndPages[0], SW_SHOW );
    SetFocus( pIda->hwndPages[0] );

  } /* endif */

  if ( !fOK )
  {
    POSTEQFCLOSE( hwndDlg, FALSE );
  } /* endif */

  return fOK;
}


//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     PropertySheetNotification
//------------------------------------------------------------------------------
// Function call:     PropertySheetNotifiaction( hwndDlg, mp1, mp2);
//------------------------------------------------------------------------------
// Description:       handle changes on the tab page
//------------------------------------------------------------------------------
// Parameters:        HWND hwndDlg   handle of the dialog
//                    WPARAM  mp1    message parameter 1
//                    LPARAM  mp2    message parameter 2
//------------------------------------------------------------------------------
// Returncode type:   MRESULT
//------------------------------------------------------------------------------
// Returncodes:       return code from default window proc or FALSE
//------------------------------------------------------------------------------
// Function flow:     switch ( pNMHdr->code )
//                      case TCN_SELCHANGE:
//                        activate new page
//                      case TCN_SELCHANGING
//                        hide the dialog
//                    return
//------------------------------------------------------------------------------

MRESULT PropertySheetNotification
(
  HWND hwndDlg,
  WPARAM  mp1,
  LPARAM  mp2
)
{
  NMHDR * pNMHdr;
  LONG       lTabCtrl;
  MRESULT      mResult = FALSE;
  PPROFONEIDA  pIda;
  pNMHdr = (LPNMHDR)mp2;

  mp1;
  switch ( pNMHdr->code )
  {
    case TCN_SELCHANGE:
      pIda = ACCESSDLGIDA(hwndDlg, PPROFONEIDA);
      if ( pIda )
      {
        lTabCtrl = TabCtrl_GetCurSel(GetDlgItem( hwndDlg, ID_TB_PROP_TABCTRL ));
        ShowWindow( pIda->hwndPages[ lTabCtrl ], SW_SHOW );
      } /* endif */
      break;
    case TCN_SELCHANGING:
      pIda = ACCESSDLGIDA(hwndDlg, PPROFONEIDA);
      if ( pIda )
      {
        /**************************************************************/
        /* Issue a direct call to the appropriate dialog proc with    */
        /* WM_COMMAND, ID_TB_PROP_SET_PB and the second parameter set */
        /* to 1L to force only consistency checking                   */
        /**************************************************************/
        PFNWP pfnWp;
        lTabCtrl = TabCtrl_GetCurSel(GetDlgItem( hwndDlg, ID_TB_PROP_TABCTRL ));
        pfnWp = (PFNWP) GetWindowLong( pIda->hwndPages[ lTabCtrl ], DWL_DLGPROC );

        mResult = pfnWp( pIda->hwndPages[lTabCtrl], WM_COMMAND,
                         ID_TB_PROP_SET_PB, 1L);
        if ( mResult )
        {
          /************************************************************/
          /* stick on the side                                        */
          /* we have to post the request again since one of the system*/
          /* routines thinks that we still want to change the page..  */
          /************************************************************/
          WinPostMsg( hwndDlg, TCM_SETCURSEL, lTabCtrl, 0L );
        } /* endif */
        ShowWindow( pIda->hwndPages[ lTabCtrl ], SW_HIDE );
      } /* endif */
      break;
    case TTN_NEEDTEXT:
      {
        HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
        TOOLTIPTEXT *pToolTipText = (TOOLTIPTEXT *) mp2;
        if ( pToolTipText )
        {
          //int idCtrl = GetDlgCtrlID( (HWND) pNMHdr->idFrom );
          switch ( pToolTipText->hdr.idFrom )
          {
            case 0:      // first page
              LOADSTRING( hab, hResMod, IDS_TB_TABCTRL_TTIP_EDITOR,
                          pToolTipText->szText );
              break;
            case 1:      // second page
              LOADSTRING( hab, hResMod, IDS_TB_TABCTRL_TTIP_DISPLAY,
                          pToolTipText->szText );
              break;
            case 2:      // third page
              LOADSTRING( hab, hResMod, IDS_TB_TABCTRL_TTIP_DICTIONARY,
                          pToolTipText->szText );
              break;
            case 3:      // fourth page
              LOADSTRING( hab, hResMod, IDS_TB_TABCTRL_TTIP_TRANSLMEM,
                          pToolTipText->szText );
              break;
            case 4:      // fivth page
              LOADSTRING( hab, hResMod, IDS_TB_TABCTRL_TTIP_MESSAGES,
                          pToolTipText->szText );
              break;
            case 5:      // sixth page
              LOADSTRING( hab, hResMod, IDS_TB_TABCTRL_TTIP_BACKTASKS,
                          pToolTipText->szText );
              break;
            case 6:      // seventh page
              LOADSTRING( hab, hResMod, IDS_TB_TABCTRL_TTIP_AUTSUBST,
                          pToolTipText->szText );
              break;

          } /* endswitch */
        } /* endif */
      }
      break;
    default:
      break;
  } /* endswitch */
  return mResult;
}


//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     EQFBPROP_EDITOR_DLGPROC
//------------------------------------------------------------------------------
// Function call:     EQFBPROP_EDITOR_DLGPROC( hwndDlg, mp1, mp2);
//------------------------------------------------------------------------------
// Description:       handle the Editor page
//------------------------------------------------------------------------------
// Parameters:        HWND hwndDlg   handle of the dialog
//                    WPARAM  mp1    message parameter 1
//                    LPARAM  mp2    message parameter 2
//------------------------------------------------------------------------------
// Returncode type:   MRESULT
//------------------------------------------------------------------------------
// Returncodes:       return code from default window proc or FALSE
//------------------------------------------------------------------------------
// Function flow:     switch (msg)
//                     case WM_INITDLG:
//                          do initial settings
//                     case WM_COMMAND:
//                          handel Set pushbutton
//                     default:
//                          default dialog proc
//                    return
//------------------------------------------------------------------------------

INT_PTR CALLBACK EQFBPROP_EDITOR_DLGPROC
(
   HWND hwndDlg,                       /* handle of dialog window             */
   WINMSG msg,
   WPARAM mp1,
   LPARAM mp2
)
{
  MRESULT      mResult = MRFROMSHORT( FALSE );                 // result value of procedure
  PPROFONEIDA  pIda;
  USEROPT* pEQFBUserOpt = get_EQFBUserOpt();

  switch ( msg )
  {
    case WM_INITDLG:
      SETWINDOWID( hwndDlg, ID_TB_PROP_EDITOR_DLG );
      pIda = (PPROFONEIDA) mp2;
      ANCHORDLGIDA( hwndDlg, pIda );
      SETCHECK( hwndDlg, ID_TB_PROFONE_CUABACK,     pEQFBUserOpt->fCUABksp );
      SETCHECK( hwndDlg, ID_TB_PROFONE_AUTOINSMODE, pEQFBUserOpt->fCrsInsert );
      SETCHECK( hwndDlg, ID_TB_PROFONE_INSPROP,     pEQFBUserOpt->fInsProposal );
      SETCHECK( hwndDlg, ID_TB_PROFONE_INSSOSI,     pEQFBUserOpt->UserOptFlags.bConvSOSI );

      if (pIda->pDoc->hwndRichEdit )
      {
        ENABLECTRL( hwndDlg, ID_TB_PROFONE_CUABACK, FALSE );
        ENABLECTRL( hwndDlg, ID_TB_PROFONE_INSPROP, FALSE );
      } /* endif */

      SETCHECK( hwndDlg, ID_TB_PROFONE_BIDILOGICDISPLAY, pEQFBUserOpt->UserOptFlags.bBidiLogicDisplay);

      break;

    case WM_COMMAND:
      switch ( WMCOMMANDID( mp1, mp2 ) )
      {
         case ID_TB_PROP_SET_PB:           // get new value and set it
           /****************************************************************/
           /* if mp2 == 1L we have to validate the page, if it is 0L we    */
           /* have to copy the content of the dialog back into the struct. */
           /****************************************************************/
           pIda = ACCESSDLGIDA(hwndDlg, PPROFONEIDA);
           if ( mp2 == 1L )
           {
             /**************************************************************/
             /* no checking for correct input necessary                    */
             /**************************************************************/
           }
           else
           {
             /**************************************************************/
             /* get the active settings ....                               */
             /**************************************************************/
             if (!pIda->pDoc->hwndRichEdit )
             {
               pEQFBUserOpt->fCUABksp =
                       (EQF_BOOL) QUERYCHECK( hwndDlg, ID_TB_PROFONE_CUABACK );
               pEQFBUserOpt->fInsProposal =
                       (EQF_BOOL) QUERYCHECK( hwndDlg, ID_TB_PROFONE_INSPROP );
               pEQFBUserOpt->fNoCUASel = 0;
             } /* endif */
             pEQFBUserOpt->fCrsInsert =
                     (EQF_BOOL) QUERYCHECK( hwndDlg, ID_TB_PROFONE_AUTOINSMODE );
             pEQFBUserOpt->UserOptFlags.bConvSOSI =
                    QUERYCHECK( hwndDlg, ID_TB_PROFONE_INSSOSI );

             pEQFBUserOpt->UserOptFlags.bBidiLogicDisplay =
                      QUERYCHECK( hwndDlg, ID_TB_PROFONE_BIDILOGICDISPLAY );

           } /* endif */
           break;
      } /* endswitch */
      break;

    case WM_HELP:
       /*************************************************************/
       /* pass on a HELP_WM_HELP request                            */
       /*************************************************************/
       EqfDisplayContextHelp( (HWND)((LPHELPINFO) mp2)->hItemHandle,
                              &hlpsubtblPropEditorDlg[0] );
       mResult = TRUE;  // message processed
       break;

    default:
       mResult = WinDefDlgProc( hwndDlg, msg, mp1, mp2 );
       break;
  } /* endswitch */

  return mResult;
};


//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     EQFBPROP_DISPLAY_DLGPROC
//------------------------------------------------------------------------------
// Function call:     EQFBPROP_DISPLAY_DLGPROC( hwndDlg, mp1, mp2);
//------------------------------------------------------------------------------
// Description:       handle the Display page
//------------------------------------------------------------------------------
// Parameters:        HWND hwndDlg   handle of the dialog
//                    WPARAM  mp1    message parameter 1
//                    LPARAM  mp2    message parameter 2
//------------------------------------------------------------------------------
// Returncode type:   MRESULT
//------------------------------------------------------------------------------
// Returncodes:       return code from default window proc or FALSE
//------------------------------------------------------------------------------
// Function flow:     switch (msg)
//                     case WM_INITDLG:
//                          do initial settings
//                     case WM_COMMAND:
//                          handel Set pushbutton
//                     default:
//                          default dialog proc
//                    return
//------------------------------------------------------------------------------

INT_PTR CALLBACK EQFBPROP_DISPLAY_DLGPROC
(
   HWND hwndDlg,                       /* handle of dialog window             */
   WINMSG msg,
   WPARAM mp1,
   LPARAM mp2
)
{
  MRESULT      mResult = MRFROMSHORT( FALSE );   // result value of procedure
  CHAR         chText[RMARGIN_LENGTH +1];        // buffer for margin value
  CHAR         chTextShr[MAXSHRKLEN+1];          // buffer for shrink abbreviations
  CHAR         chTRNote[MAXTRNOTE_SIZE+2];       // buffer for trnote abbr
  CHAR         chTextActLine[FOCUS_LENGTH+1];    // buffer for focus line
  CHAR         chTextMargin[RMARGIN_LENGTH +1];  // buffer for margin value
  HAB          hab;                              // dialog anchor
  PPROFONEIDA  pIda;
  SHORT        sItem;
  SHORT        sI;
  BOOL         fAbbrOK = TRUE;                   // true if abbreviation ok
  SHORT        sActLine;                         // active line
  SHORT        sMargin;                          // value of entered input text
  PSZ          pData;
  BOOL         fAutoOld, fAutoNew;                         // old/ new AUTOLinewrap setting
  CHAR         c;
  USHORT       usJ;
  USHORT       usIDFocus;
  PTBDOCUMENT  pDocTemp;
  USEROPT*     pEQFBUserOpt = get_EQFBUserOpt();

  switch ( msg )
  {
    case WM_INITDLG:
      {
        HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);

        hab = GETINSTANCE( hwndDlg );
        pIda = (PPROFONEIDA) mp2;
        ANCHORDLGIDA( hwndDlg, pIda );
        SETWINDOWID( hwndDlg, ID_TB_PROP_DISPLAY_DLG );
        SETCHECK( hwndDlg, ID_TB_PROFONE_SEGBOUNDSIGN, pEQFBUserOpt->fSegBound );
        SETCHECK( hwndDlg, ID_TB_PROFONE_VISIBLESPACE, pEQFBUserOpt->UserOptFlags.bVisibleSpace);

        /********************************************************************/
        /* shrink                                                           */
        /********************************************************************/
        SETTEXTLIMIT( hwndDlg, ID_TB_PROFONE_SHRINK_EF, MAXSHRKLEN );
        OEMSETTEXT( hwndDlg, ID_TB_PROFONE_SHRINK_EF, pEQFBUserOpt->szOutTagAbbr );
        SETEFSEL( hwndDlg, ID_TB_PROFONE_SHRINK_EF, 0, MAXSHRKLEN );
        /********************************************************************/
        /* compact                                                          */
        /********************************************************************/
        SETTEXTLIMIT( hwndDlg, ID_TB_PROFONE_COMPACT_EF, MAXSHRKLEN );
        OEMSETTEXT( hwndDlg, ID_TB_PROFONE_COMPACT_EF, pEQFBUserOpt->szInTagAbbr );
        SETEFSEL( hwndDlg, ID_TB_PROFONE_COMPACT_EF, 0, MAXSHRKLEN );

        /********************************************************************/
        /* trnote                                                           */
        /********************************************************************/
        SETTEXTLIMIT( hwndDlg, ID_TB_PROFONE_TRNOTE_EF, MAXTRNOTE_SIZE );
        OEMSETTEXT( hwndDlg, ID_TB_PROFONE_TRNOTE_EF, pEQFBUserOpt->chTRNoteAbbr );
        SETEFSEL( hwndDlg, ID_TB_PROFONE_TRNOTE_EF, 0, MAXTRNOTE_SIZE );
        /********************************************************************/
        /* fill entry field with line number of active segment              */
        /********************************************************************/
        SETTEXTLIMIT( hwndDlg, ID_TB_PROFONE_ACTLINE_EF, FOCUS_LENGTH );
        itoa( pEQFBUserOpt->sFocusLine, chText, 10 );
        OEMSETTEXT( hwndDlg, ID_TB_PROFONE_ACTLINE_EF, chText );
        SETEFSEL( hwndDlg, ID_TB_PROFONE_ACTLINE_EF, 0, FOCUS_LENGTH );

        if (!pIda->pDoc->hwndRichEdit )
        {
          /********************************************************************/
          /* fill cbs with right margin possibilities                         */
          /********************************************************************/
          SETTEXTLIMIT( hwndDlg, ID_TB_PROFONE_RMARGIN_CBS, RMARGIN_LENGTH );
          LOADSTRING( hab, hResMod, IDS_TB_PROFONE_AUTO, pIda->szMarginBuffer );
          sItem = CBINSERTITEMEND( hwndDlg, ID_TB_PROFONE_RMARGIN_CBS,
                                 pIda->szMarginBuffer);

          for ( sI = 50; sI < 95; sI= sI+5 )
          {
            itoa(sI, pIda->szBuffer, 10);
            sItem = CBINSERTITEMEND( hwndDlg, ID_TB_PROFONE_RMARGIN_CBS,
                                   pIda->szBuffer);
          } /* endfor */
          /*****************************************************************/
          /* select the current value correctly                            */
          /*****************************************************************/
          if (pIda->pDoc->fAutoLineWrap )
          {
            CBSELECTITEM( hwndDlg, ID_TB_PROFONE_RMARGIN_CBS, 0);
          }
          else
          {
            itoa( pEQFBUserOpt->sRMargin, chText, 10 );
            SETTEXT( hwndDlg, ID_TB_PROFONE_RMARGIN_CBS, chText );
          } /* endif */
        }
        else
        {
          ShowWindow( WinWindowFromID( hwndDlg, ID_TB_PROFONE_RMARGIN_CBS),
                      SW_HIDE );
          ShowWindow( WinWindowFromID( hwndDlg, ID_TB_PROFONE_RMARGIN_STATIC),
                      SW_HIDE );
        } /* endif */

       /********************************************************************/
       /* fill cbs with all possibilities                                  */
       /********************************************************************/
       pIda->szBuffer[1] = EOS;
       for ( sI = 32; sI < 256; sI++ )
       {
         if ( ((sI >= 48) && (sI <= 57)) ||
              ((sI >= 65) && (sI <= 90)) ||
              ((sI >= 97) && (sI <= 122)) )
         {
           /*************************************************************/
           /* ignore character -- it is not selectable                  */
           /*************************************************************/
         }
         else
         {
           pIda->szBuffer[0] = (BYTE)sI;
           CBINSERTITEMEND( hwndDlg, ID_TB_PROFONE_VISIBLEBLANK_EF, pIda->szBuffer);
           CBINSERTITEMEND( hwndDlg, ID_TB_PROFONE_VISIBLEHARDRETURN_EF, pIda->szBuffer);
           CBINSERTITEMEND( hwndDlg, ID_TB_PROFONE_SEGMENTBOUND_EF, pIda->szBuffer);
         } /* endif */
       } /* endfor */


       /*****************************************************************/
       /* select last used values                                       */
       /*****************************************************************/
       pIda->szBuffer[0] = (BYTE)pEQFBUserOpt->bVisibleBlank;
       if (pIda->szBuffer[0] == 0)
       {
         pIda->szBuffer[0] = VISIBLE_BLANK;
       }

       sI = (SHORT)SendDlgItemMessage( hwndDlg, ID_TB_PROFONE_VISIBLEBLANK_EF, CB_FINDSTRING,  (WPARAM) -1, (LONG) &pIda->szBuffer[0] );
       CBSELECTITEM( hwndDlg, ID_TB_PROFONE_VISIBLEBLANK_EF, (sI >=0) ? sI : 0);


       pIda->szBuffer[0] = (BYTE)pEQFBUserOpt->bVisibleLineFeed;
       if (pIda->szBuffer[0] == 0)
       {
         pIda->szBuffer[0] = VISIBLE_LINEFEED;
       }

       sI = (SHORT)SendDlgItemMessage( hwndDlg, ID_TB_PROFONE_VISIBLEHARDRETURN_EF, CB_FINDSTRING, (WPARAM) -1, (LONG) &pIda->szBuffer[0] );
       CBSELECTITEM( hwndDlg, ID_TB_PROFONE_VISIBLEHARDRETURN_EF, (sI >=0) ? sI : 0);


       pIda->szBuffer[0] = (BYTE)pEQFBUserOpt->bSegmentBoundary;
       if (pIda->szBuffer[0] == 0)
       {
         pIda->szBuffer[0] = POSTEDITSEGBOUND_SEGDATA;
       }

       sI = (SHORT)SendDlgItemMessage( hwndDlg, ID_TB_PROFONE_SEGMENTBOUND_EF, CB_FINDSTRING, (WPARAM) -1, (LONG) &pIda->szBuffer[0] );
       CBSELECTITEM( hwndDlg, ID_TB_PROFONE_SEGMENTBOUND_EF, (sI >=0) ? sI : 0);

       SetCtrlFnt (hwndDlg, pIda->pDoc->lf.lfCharSet,
                   ID_TB_PROFONE_VISIBLEBLANK_EF, ID_TB_PROFONE_VISIBLEHARDRETURN_EF );
       SetCtrlFnt (hwndDlg, pIda->pDoc->lf.lfCharSet,
                   ID_TB_PROFONE_SEGMENTBOUND_EF, 0 );

       if ( !pEQFBUserOpt->UserOptFlags.bVisibleSpace || pIda->pDoc->hwndRichEdit )
       {
         ENABLECTRL( hwndDlg, ID_TB_PROFONE_VISIBLEBLANK_STATIC, FALSE );
         ENABLECTRL( hwndDlg, ID_TB_PROFONE_VISIBLEHARDRETURN_STATIC, FALSE );

         ENABLECTRL( hwndDlg, ID_TB_PROFONE_VISIBLEBLANK_EF, FALSE );
         ENABLECTRL( hwndDlg, ID_TB_PROFONE_VISIBLEHARDRETURN_EF, FALSE );
       } /* endif */

       if ( !pEQFBUserOpt->fSegBound )
       {
         ENABLECTRL( hwndDlg, ID_TB_PROFONE_SEGMENTBOUND_STATIC, FALSE );
         ENABLECTRL( hwndDlg, ID_TB_PROFONE_SEGMENTBOUND_EF, FALSE );
       }

       /*****************************************************************/
       /* select the current value correctly                            */
       /*****************************************************************/
       if (pIda->pDoc->fAutoLineWrap )
       {
         CBSELECTITEM( hwndDlg, ID_TB_PROFONE_RMARGIN_CBS, 0);
       }
       else
       {
         itoa( pEQFBUserOpt->sRMargin, chText, 10 );
         SETTEXT( hwndDlg, ID_TB_PROFONE_RMARGIN_CBS, chText );
       } /* endif */


         SetCtrlFnt (hwndDlg, pIda->pDoc->lf.lfCharSet,
                     ID_TB_PROFONE_COMPACT_EF,ID_TB_PROFONE_SHRINK_EF );
      }
      break;

    case WM_DESTROY:
      DelCtrlFont (hwndDlg, ID_TB_PROFONE_COMPACT_EF);
      DelCtrlFont (hwndDlg, ID_TB_PROFONE_VISIBLEBLANK_EF);
      DelCtrlFont (hwndDlg, ID_TB_PROFONE_SEGMENTBOUND_EF );

      break;

    case WM_COMMAND:
      switch ( WMCOMMANDID( mp1, mp2 ) )
      {
         case ID_TB_PROP_SET_PB:           // get new value and set it
           /********************************************************************/
           /* to be filled ...                                                 */
           /********************************************************************/
            pIda = ACCESSDLGIDA(hwndDlg, PPROFONEIDA);
            usIDFocus = ID_TB_PROFONE_SHRINK_EF;
            fAutoNew = FALSE;
            sMargin = MINMARGIN + 2;

           /********************************************************************/
           /* update the shrink and compact abbreviations                      */
           /********************************************************************/
           OEMQUERYTEXT( hwndDlg, ID_TB_PROFONE_COMPACT_EF, chText );
           OEMQUERYTEXT( hwndDlg, ID_TB_PROFONE_SHRINK_EF, chTextShr );
           OEMQUERYTEXT( hwndDlg, ID_TB_PROFONE_TRNOTE_EF, chTRNote  );
           OEMQUERYTEXT( hwndDlg, ID_TB_PROFONE_ACTLINE_EF, chTextActLine );
           OEMQUERYTEXT( hwndDlg, ID_TB_PROFONE_RMARGIN_CBS, chTextMargin );

           if ( mp2 == 1L )
           {
             /*********************************************************/
             /* check  validity only                                  */
             /*********************************************************/
             /********************************************************************/
             /* get active line number   and check range                         */
             /********************************************************************/
             sActLine = GetNumber( chTextActLine );
             if ( !(sActLine >= MINFOCUS && sActLine <= MAXFOCUS) )
             {
                // display error message
                fAbbrOK = FALSE;
                pData = chTextActLine;
                UtlError( TB_NOVALIDFOCUS, MB_CANCEL, 1, &pData, EQF_ERROR );
                SETFOCUS( hwndDlg, ID_TB_PROFONE_ACTLINE_EF );
             } /* endif */

             /********************************************************************/
             /* get right margin and check range                                 */
             /********************************************************************/
             QUERYTEXT( hwndDlg, ID_TB_PROFONE_RMARGIN_CBS, chTextMargin );
             fAutoOld = pIda->pDoc->fAutoLineWrap;
             // check for validity
             if ( stricmp( pIda->szMarginBuffer, chTextMargin ) != 0 )
             {
               sMargin = GetNumber( chTextMargin );
               if ( !(sMargin >= MINMARGIN && sMargin <= MAXMARGIN) )
               {
                  // display error message
                  fAbbrOK = FALSE;
                  pData = chTextMargin;
                  UtlError( TB_NOVALIDMARGIN, MB_CANCEL, 1, &pData, EQF_ERROR );
                  SETFOCUS( hwndDlg, ID_TB_PROFONE_RMARGIN_CBS );
               } /* endif */
             } /* endif */

             /********************************************************************/
             /* check if abbreviations are valid                                                                 */
             /* first check the shrink field, later on the compact field         */
             /********************************************************************/
             if (fAbbrOK )
             {
               usIDFocus = ID_TB_PROFONE_SHRINK_EF;           // id of focus window
               usJ = 0;
               fAbbrOK = FALSE;
               while ( ((c = chTextShr[usJ++]) != 0) && !fAbbrOK )
               {
                 fAbbrOK = (c != BLANK);
               } /* endwhile */
               if ( fAbbrOK )
               {
                 fAbbrOK = FALSE;
                 usIDFocus = ID_TB_PROFONE_COMPACT_EF;        // id of focus window
                 usJ = 0;
                 while ( ((c = chText[usJ++]) != 0) && !fAbbrOK )
                 {
                   fAbbrOK = (c != BLANK);
                 } /* endwhile */

                 if (fAbbrOK )
                 {
                   usIDFocus = ID_TB_PROFONE_TRNOTE_EF;               // id of trnote entryfield
                   usJ = 0;
                   while ( ((c = chTRNote[usJ++]) != 0) && !fAbbrOK )
                   {
                     fAbbrOK = (c != BLANK);
                   } /* endwhile */
                 } /* endif */
               } /* endif */

               /********************************************************************/
               /* display error message ...                                        */
               /********************************************************************/
               if ( !fAbbrOK )
               {
                 fAbbrOK = (UtlError(TB_NOVALIDABBR,MB_YESNO,0,NULL,EQF_QUERY) == MBID_YES);
               } /* endif */
             } /* endif */

             /*********************************************************/
             /* return TRUE indicating an error happened              */
             /*********************************************************/
             mResult = !fAbbrOK;
           }
           else
           {
             /**************************************************************/
             /* get the active settings ....                               */
             /**************************************************************/
             pEQFBUserOpt->fSegBound =
                     (EQF_BOOL) QUERYCHECK( hwndDlg, ID_TB_PROFONE_SEGBOUNDSIGN );
             pEQFBUserOpt->UserOptFlags.bVisibleSpace =
                     QUERYCHECK( hwndDlg, ID_TB_PROFONE_VISIBLESPACE );

             /********************************************************************/
             /* get active line number   and check range                         */
             /********************************************************************/
             sActLine = GetNumber( chTextActLine );
             if ( sActLine >= MINFOCUS && sActLine <= MAXFOCUS )
             {
                // set sActLine
                pEQFBUserOpt->sFocusLine = sActLine;
             }
             else
             {
                // display error message
                fAbbrOK = FALSE;
                pData = chTextActLine;
                UtlError( TB_NOVALIDFOCUS, MB_CANCEL, 1, &pData, EQF_ERROR );
                SETFOCUS( hwndDlg, ID_TB_PROFONE_ACTLINE_EF );
             } /* endif */

             /********************************************************************/
             /* get right margin and check range                                 */
             /********************************************************************/
             QUERYTEXT( hwndDlg, ID_TB_PROFONE_RMARGIN_CBS, chTextMargin );
             fAutoOld = pIda->pDoc->fAutoLineWrap;
             // check for validity
             if ( stricmp( pIda->szMarginBuffer, chTextMargin ) == 0 )
             {
               USHORT* pusRightMargin = get_usRightMargin();
               fAutoNew = TRUE;
               *pusRightMargin = AUTOSIZE;
               pIda->pDoc->fAutoLineWrap = TRUE;
               pIda->pDoc->sRMargin = (SHORT)pIda->pDoc->lScrnCols;
               pEQFBUserOpt->sRMargin = (SHORT)pIda->pDoc->lScrnCols;
             }
             else
             {
               sMargin = GetNumber( chTextMargin );
               if ( sMargin >= MINMARGIN && sMargin <= MAXMARGIN )
               {
                  // set usRightMargin, pDoc margin and write profile, close dlg
                  USHORT* pusRightMargin = get_usRightMargin();
                  *pusRightMargin = sMargin;          //contains initial value, not last used
                  /*****************************************************************/
                  /* only initial value is stored in profile, not last used value  */
                  /* hence pDoc->sRMargin is NOT changed!!                         */
                  /*****************************************************************/
                  pEQFBUserOpt->sRMargin = sMargin;
                  fAutoNew = FALSE;
               }
               else
               {
                  // display error message
                  fAbbrOK = FALSE;
                  pData = chTextMargin;
                  UtlError( TB_NOVALIDMARGIN, MB_CANCEL, 1, &pData, EQF_ERROR );
                  SETFOCUS( hwndDlg, ID_TB_PROFONE_RMARGIN_CBS );
               } /* endif */
             } /* endif */
             if (fAbbrOK )
             {
               /******************************************************************/
               /* force update of autolinewrap of all     documents              */
               /******************************************************************/
               if (fAutoOld && !fAutoNew )
               {
                 /*************************************************************/
                 /* autolinewrap is turned off, hard right margin is set      */
                 /*************************************************************/
                 UpdateAutoLineWrapSettings( pIda->pDoc, FALSE, sMargin);
                 if (pIda->pDoc->docType == STARGET_DOC )
                 {
                   UpdateAutoLineWrapSettings( pIda->pDoc->twin, FALSE, sMargin);
                 } /* endif */

                 pDocTemp = pIda->pDoc->next;
                 while (pIda->pDoc != pDocTemp )
                 {
                   UpdateAutoLineWrapSettings( pDocTemp, FALSE, sMargin);
                   pDocTemp = pDocTemp->next;
                 } /* endwhile */
                 if (!pIda->pDoc->EQFBFlags.Reflow )
                                 { // do not allow hard LF!
                                    pIda->pDoc->fLineWrap = FALSE;               // P018278
                                    pEQFBUserOpt->fLineWrap = FALSE;
                 }

               } /* endif */
               if (!fAutoOld && fAutoNew )
               {
                 /****************************************************************/
                 /* autolinewrap is turned ON, soft lf's must be inserted        */
                 /****************************************************************/
                 UpdateAutoLineWrapSettings( pIda->pDoc, TRUE, sMargin);
                 if (pIda->pDoc->docType == STARGET_DOC )
                 {
                   UpdateAutoLineWrapSettings( pIda->pDoc->twin, TRUE, sMargin);
                 } /* endif */
                 pDocTemp = pIda->pDoc->next;
                 while ( pIda->pDoc != pDocTemp )
                 {
                   UpdateAutoLineWrapSettings(pDocTemp, TRUE, sMargin);
                   pDocTemp = pDocTemp->next;
                 } /* endwhile */
               } /* endif */
               /******************************************************************/
               /* if only change in character number in margin setting, adjust   */
               /* sRMargin here                                                  */
               /******************************************************************/
               if (!fAutoOld && !fAutoNew )
               {
                 pIda->pDoc->sRMargin = sMargin;
                 pDocTemp = pIda->pDoc->next;
                 while ( pIda->pDoc != pDocTemp )
                 {
                   pDocTemp->sRMargin = sMargin;
                   pDocTemp = pDocTemp->next;
                 } /* endwhile */
               } /* endif */
             } /* endif */

             /********************************************************************/
             /* check if abbreviations are valid                                                                 */
             /* first check the shrink field, later on the compact field         */
             /********************************************************************/
             if (fAbbrOK )
             {
               usIDFocus = ID_TB_PROFONE_SHRINK_EF;           // id of focus window
               usJ = 0;
               fAbbrOK = FALSE;
               while ( ((c = chTextShr[usJ++]) != 0) && !fAbbrOK )
               {
                 fAbbrOK = (c != BLANK);
               } /* endwhile */
               if ( fAbbrOK )
               {
                 fAbbrOK = FALSE;
                 usIDFocus = ID_TB_PROFONE_COMPACT_EF;        // id of focus window
                 usJ = 0;
                 while ( ((c = chText[usJ++]) != 0) && !fAbbrOK )
                 {
                   fAbbrOK = (c != BLANK);
                 } /* endwhile */

                 if (fAbbrOK )
                 {
                   usIDFocus = ID_TB_PROFONE_TRNOTE_EF;               // id of trnote entryfield
                   usJ = 0;
                   while ( ((c = chTRNote[usJ++])  != 0) && !fAbbrOK )
                   {
                     fAbbrOK = (c != BLANK);
                   } /* endwhile */
                 } /* endif */
               } /* endif */

               /*******************************************************/
               /* get settings for substitutions characters in ANSI   */
               /*******************************************************/
               if ( pEQFBUserOpt->UserOptFlags.bVisibleSpace )
               {
                 QUERYTEXT( hwndDlg, ID_TB_PROFONE_VISIBLEBLANK_EF, pIda->szBuffer);
                 pEQFBUserOpt->bVisibleBlank = pIda->szBuffer[0];
                 QUERYTEXT( hwndDlg, ID_TB_PROFONE_VISIBLEHARDRETURN_EF, pIda->szBuffer);
                 pEQFBUserOpt->bVisibleLineFeed = pIda->szBuffer[0];
               }
               if ( pEQFBUserOpt->fSegBound )
               {
                 QUERYTEXT( hwndDlg, ID_TB_PROFONE_SEGMENTBOUND_EF, pIda->szBuffer);
                 pEQFBUserOpt->bSegmentBoundary = pIda->szBuffer[0];
               }

               /********************************************************************/
               /* display error message ...                                        */
               /********************************************************************/
               if ( !fAbbrOK )
               {
                 fAbbrOK = (UtlError(TB_NOVALIDABBR,MB_YESNO,0,NULL,EQF_QUERY) == MBID_YES);
               } /* endif */
             } /* endif */

             /********************************************************************/
             /* save settings                                                    */
             /********************************************************************/
             if ( fAbbrOK )
             {
               PTBDOCUMENT pDocTemp;

               strcpy(pEQFBUserOpt->szOutTagAbbr,chTextShr);
               strcpy(pEQFBUserOpt->szInTagAbbr,chText);
               strcpy(pEQFBUserOpt->chTRNoteAbbr,chTRNote);

               /******************************************************************/
               /* force update of current document settings                      */
               /******************************************************************/
               UpdateDocSettings( pIda->pDoc );

               /******************************************************************/
               /* force update of all loaded document settings                   */
               /******************************************************************/
               pDocTemp = pIda->pDoc->next;
               while ( pIda->pDoc != pDocTemp )
               {
                 UpdateDocSettings( pDocTemp );
                 pDocTemp = pDocTemp->next;
               } /* endwhile */
             }
             else
             {
                SETFOCUS( hwndDlg, usIDFocus );
                /*********************************************************/
                /* return TRUE indicating an error happened              */
                /*********************************************************/
                mResult = !fAbbrOK;
             } /* endif */
           } /* endif */
           break;

         case ID_TB_PROFONE_SEGBOUNDSIGN:
           if ( WMCOMMANDCMD(mp1,mp2) == BN_CLICKED )
           {
             BOOL b = (BOOL) QUERYCHECK( hwndDlg, ID_TB_PROFONE_SEGBOUNDSIGN );
             ENABLECTRL( hwndDlg, ID_TB_PROFONE_SEGMENTBOUND_STATIC, b );
             ENABLECTRL( hwndDlg, ID_TB_PROFONE_SEGMENTBOUND_EF, b );
           } /* endif */
           break;

         case ID_TB_PROFONE_VISIBLESPACE:
           if ( WMCOMMANDCMD(mp1,mp2) == BN_CLICKED )
           {
             BOOL b = (BOOL) QUERYCHECK( hwndDlg, ID_TB_PROFONE_VISIBLESPACE );
             ENABLECTRL( hwndDlg, ID_TB_PROFONE_VISIBLEBLANK_STATIC, b );
             ENABLECTRL( hwndDlg, ID_TB_PROFONE_VISIBLEHARDRETURN_STATIC, b );
             ENABLECTRL( hwndDlg, ID_TB_PROFONE_VISIBLEBLANK_EF, b );
             ENABLECTRL( hwndDlg, ID_TB_PROFONE_VISIBLEHARDRETURN_EF, b );
           } /* endif */
           break;
      } /* endswitch */
      break;

    case WM_HELP:
      /*************************************************************/
      /* pass on a HELP_WM_HELP request                            */
      /*************************************************************/
      EqfDisplayContextHelp( (HWND)((LPHELPINFO) mp2)->hItemHandle,
                             &hlpsubtblPropDisplayDlg[0] );
      mResult = TRUE;  // message processed
      break;

    default:
       mResult = WinDefDlgProc( hwndDlg, msg, mp1, mp2 );
       break;
  } /* endswitch */

  return (MRESULT) mResult;
};

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     EQFBPROP_DICTIONARY_DLGPROC
//------------------------------------------------------------------------------
// Function call:     EQFBPROP_DICTIONARY_DLGPROC( hwndDlg, mp1, mp2);
//------------------------------------------------------------------------------
// Description:       handle the Dictionary page
//------------------------------------------------------------------------------
// Parameters:        HWND hwndDlg   handle of the dialog
//                    WPARAM  mp1    message parameter 1
//                    LPARAM  mp2    message parameter 2
//------------------------------------------------------------------------------
// Returncode type:   MRESULT
//------------------------------------------------------------------------------
// Returncodes:       return code from default window proc or FALSE
//------------------------------------------------------------------------------
// Function flow:     switch (msg)
//                     case WM_INITDLG:
//                          do initial settings
//                     case WM_COMMAND:
//                          handel Set pushbutton
//                     default:
//                          default dialog proc
//                    return
//------------------------------------------------------------------------------

INT_PTR CALLBACK EQFBPROP_DICTIONARY_DLGPROC
(
   HWND hwndDlg,                       /* handle of dialog window             */
   WINMSG msg,
   WPARAM mp1,
   LPARAM mp2
)
{
  MRESULT  mResult = MRFROMSHORT( FALSE );                // result value of procedure
  USEROPT* pEQFBUserOpt = get_EQFBUserOpt();
  switch ( msg )
  {
    case WM_INITDLG:

      SETWINDOWID( hwndDlg, ID_TB_PROP_DICTIONARY_DLG );
      SETCHECK( hwndDlg, ID_TB_PROFONE_ADDINFODIC,   pEQFBUserOpt->fAddInfoDic );
      SETCHECK( hwndDlg, ID_TB_PROFONE_DISPDICTNAME, pEQFBUserOpt->fDispDictName );
      SETCHECK( hwndDlg, ID_TB_PROFONE_ALLDICTTERMS, pEQFBUserOpt->fAllDictTerms );
      SETCHECK( hwndDlg, ID_TB_PROFONE_LKUPSINGLEOFCOMPOUNDS, pEQFBUserOpt->fLkupSingleOfCompounds );
      break;

    case WM_COMMAND:
      switch ( WMCOMMANDID( mp1, mp2 ) )
      {
         case ID_TB_PROP_SET_PB:           // get new value and set it
           /****************************************************************/
           /* if mp2 == 1L we have to validate the page, if it is 0L we    */
           /* have to copy the content of the dialog back into the struct. */
           /****************************************************************/
           if ( mp2 == 1L )
           {
             /**************************************************************/
             /* no checking for correct input necessary                    */
             /**************************************************************/
           }
           else
           {
             /**************************************************************/
             /* get the active settings ....                               */
             /**************************************************************/
             pEQFBUserOpt->fAddInfoDic =
                     (EQF_BOOL) QUERYCHECK( hwndDlg, ID_TB_PROFONE_ADDINFODIC );
             pEQFBUserOpt->fDispDictName =
                     (EQF_BOOL) QUERYCHECK( hwndDlg, ID_TB_PROFONE_DISPDICTNAME );
             pEQFBUserOpt->fAllDictTerms =
                     (EQF_BOOL) QUERYCHECK( hwndDlg, ID_TB_PROFONE_ALLDICTTERMS );
             pEQFBUserOpt->fLkupSingleOfCompounds =
               (EQF_BOOL) QUERYCHECK( hwndDlg, ID_TB_PROFONE_LKUPSINGLEOFCOMPOUNDS);
             break;
           } /* endif */
      } /* endswitch */
      break;

    case WM_HELP:
       /*************************************************************/
       /* pass on a HELP_WM_HELP request                            */
       /*************************************************************/
       EqfDisplayContextHelp( (HWND)((LPHELPINFO) mp2)->hItemHandle,
                              &hlpsubtblPropDictionaryDlg[0] );
       mResult = TRUE;  // message processed
       break;
    default:
       mResult = WinDefDlgProc( hwndDlg, msg, mp1, mp2 );
       break;
  } /* endswitch */

  return mResult;
};

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     EQFBPROP_TRANSLMEM_DLGPROC
//------------------------------------------------------------------------------
// Function call:     EQFBPROP_TRANSLMEM_DLGPROC( hwndDlg, mp1, mp2);
//------------------------------------------------------------------------------
// Description:       handle the Transl.Mem page
//------------------------------------------------------------------------------
// Parameters:        HWND hwndDlg   handle of the dialog
//                    WPARAM  mp1    message parameter 1
//                    LPARAM  mp2    message parameter 2
//------------------------------------------------------------------------------
// Returncode type:   MRESULT
//------------------------------------------------------------------------------
// Returncodes:       return code from default window proc or FALSE
//------------------------------------------------------------------------------
// Function flow:     switch (msg)
//                     case WM_INITDLG:
//                          do initial settings
//                     case WM_COMMAND:
//                          handel Set pushbutton
//                     default:
//                          default dialog proc
//                    return
//------------------------------------------------------------------------------

INT_PTR CALLBACK EQFBPROP_TRANSLMEM_DLGPROC
(
   HWND hwndDlg,                       /* handle of dialog window             */
   WINMSG msg,
   WPARAM mp1,
   LPARAM mp2
)
{
  MRESULT      mResult = MRFROMSHORT( FALSE );       // result value of procedure
  SHORT        sI;
  SHORT        sItem;
  HAB          hab;                        // dialog anchor
  PPROFONEIDA  pIda;
  USHORT       usJ;
  USEROPT* pEQFBUserOpt = get_EQFBUserOpt();

  switch ( msg )
  {
    case WM_INITDLG:
      {
        HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);

        /*******************************************************************/
        /* load strings form resource to fill proposal wind.style cbs      */
        /*******************************************************************/
        hab = GETINSTANCE( hwndDlg );
        pIda = (PPROFONEIDA) mp2;
        ANCHORDLGIDA( hwndDlg, pIda );
        SETWINDOWID( hwndDlg, ID_TB_PROP_TRANSLMEM_DLG );

        ENABLEUPDATE_FALSE( hwndDlg, ID_TB_PROFONE_TMWND_CBS );
        // load names for style texts
        LOADSTRING( hab, hResMod, IDS_TB_PROFONE_PROTECTED, pIda->szStyleArray[0] );
        sItem = CBINSERTITEMEND( hwndDlg, ID_TB_PROFONE_TMWND_CBS,
                               pIda->szStyleArray[0]);
        LOADSTRING( hab, hResMod, IDS_TB_PROFONE_HIDE, pIda->szStyleArray[1] );
        sItem = CBINSERTITEMEND( hwndDlg, ID_TB_PROFONE_TMWND_CBS,
                                  pIda->szStyleArray[1]);
        LOADSTRING( hab, hResMod, IDS_TB_PROFONE_COMPACT, pIda->szStyleArray[2] );
        sItem = CBINSERTITEMEND( hwndDlg, ID_TB_PROFONE_TMWND_CBS,
                                  pIda->szStyleArray[2]);
        LOADSTRING( hab, hResMod, IDS_TB_PROFONE_SHORTEN, pIda->szStyleArray[3] );
        sItem = CBINSERTITEMEND( hwndDlg, ID_TB_PROFONE_TMWND_CBS,
                                  pIda->szStyleArray[3]);

        switch ( pEQFBUserOpt->DispTM )
        {
          case DISP_PROTECTED :
            sI = 0;
            break;
          case DISP_COMPACT :
            sI = 2;
            break;
          case DISP_HIDE:
            sI = 1;
            break;
          case DISP_SHORTEN:
            sI = 3;
            break;
          default :
            sI = 0;
            break;
        } /* endswitch */
        CBSELECTITEM( hwndDlg, ID_TB_PROFONE_TMWND_CBS, sI);
        ENABLEUPDATE_TRUE( hwndDlg, ID_TB_PROFONE_TMWND_CBS );

        SETCHECK( hwndDlg, ID_TB_PROFONE_SRCPROPWND,   pEQFBUserOpt->fSrcPropWnd );
        SETCHECK( hwndDlg, ID_TB_PROFONE_NUMPROPS,     pEQFBUserOpt->fNumProp );
        SETCHECK( hwndDlg, ID_TB_PROFONE_ORIGINPROP,   pEQFBUserOpt->fOriginProp );
        SETCHECK( hwndDlg, ID_TB_PROFONE_DATEOFPROP,   pEQFBUserOpt->fDateOfProp );
        SETCHECK( hwndDlg, ID_TB_PROFONE_DISPMTALWAYS, pEQFBUserOpt->UserOptFlags.bDispMTAlways );
        SETCHECK( hwndDlg, ID_TB_PROFONE_DISPSRC,      !pEQFBUserOpt->fFullSeg );
        SETCHECK( hwndDlg, ID_TB_PROFONE_AUTOREPL,     pEQFBUserOpt->fAutoRepl);
        SETCHECK( hwndDlg, ID_TB_PROFONE_ALLEXACTPROPS,pEQFBUserOpt->UserOptFlags.bAllExactProposals );
        SETCHECK( hwndDlg, ID_TB_PROFONE_FUZZYPERCENT, pEQFBUserOpt->UserOptFlags.bDispPropQuality );
        SETCHECK( hwndDlg, ID_TB_PROFONE_DISPMEMNAME,  EQFBUserOpt.fDispMemName );
        SETCHECK( hwndDlg, ID_TB_PROFONE_DISPMEMINDICATOR,  EQFBUserOpt.fDispMemIndicator );
        SETCHECK( hwndDlg, ID_TB_PROFONE_MACHFUZZCOLOR,  EQFBUserOpt.fMachFuzzyColor );
        SETCHECK( hwndDlg, ID_TB_PROFONE_MACHFUZZDIFF,  EQFBUserOpt.fMachFuzzyDiff );
        if ( EQFBUserOpt.usFuzzyForDiv == 0)
        {
          SETTEXT( hwndDlg, ID_TB_PROFONE_FUZZYFORDIFF_EF, "80" );
        }
        else
        {
          CHAR szFuzzy[10];
          sprintf( szFuzzy, "%u", EQFBUserOpt.usFuzzyForDiv );
          SETTEXT( hwndDlg, ID_TB_PROFONE_FUZZYFORDIFF_EF, szFuzzy );
        } /* endif */         
        SETTEXTLIMIT( hwndDlg, ID_TB_PROFONE_FUZZYFORDIFF_EF, 2 );
      }
      break;

    case WM_COMMAND:
      /****************************************************************/
      /* if mp2 == 1L we have to validate the page, if it is 0L we    */
      /* have to copy the content of the dialog back into the struct. */
      /****************************************************************/
      switch ( WMCOMMANDID( mp1, mp2 ) )
      {
         case ID_TB_PROP_SET_PB:           // get new value and set it
           if ( mp2 == 1L )
           {
             /**************************************************************/
             /* no checking for correct input necessary                    */
             /**************************************************************/
           }
           else
           {
             /**************************************************************/
             /* get the active settings ....                               */
             /**************************************************************/
             pEQFBUserOpt->fSrcPropWnd =
                     (EQF_BOOL) QUERYCHECK( hwndDlg, ID_TB_PROFONE_SRCPROPWND );
             pEQFBUserOpt->fNumProp =
                     (EQF_BOOL) QUERYCHECK( hwndDlg, ID_TB_PROFONE_NUMPROPS );
             pEQFBUserOpt->fOriginProp =
                     (EQF_BOOL) QUERYCHECK( hwndDlg, ID_TB_PROFONE_ORIGINPROP );
             pEQFBUserOpt->fDateOfProp =
                     (EQF_BOOL) QUERYCHECK( hwndDlg, ID_TB_PROFONE_DATEOFPROP );
             pEQFBUserOpt->UserOptFlags.bDispMTAlways =
                      QUERYCHECK( hwndDlg, ID_TB_PROFONE_DISPMTALWAYS );
             pEQFBUserOpt->fFullSeg =
                     (EQF_BOOL) !QUERYCHECK( hwndDlg, ID_TB_PROFONE_DISPSRC );
             pEQFBUserOpt->fAutoRepl =
                     (EQF_BOOL) QUERYCHECK( hwndDlg, ID_TB_PROFONE_AUTOREPL );

             pEQFBUserOpt->UserOptFlags.bAllExactProposals =
                      QUERYCHECK( hwndDlg, ID_TB_PROFONE_ALLEXACTPROPS);
             pEQFBUserOpt->UserOptFlags.bDispPropQuality =
                      QUERYCHECK( hwndDlg, ID_TB_PROFONE_FUZZYPERCENT);
             EQFBUserOpt.fDispMemName = QUERYCHECK( hwndDlg, ID_TB_PROFONE_DISPMEMNAME );
             EQFBUserOpt.fDispMemIndicator = QUERYCHECK( hwndDlg, ID_TB_PROFONE_DISPMEMINDICATOR );
             EQFBUserOpt.fMachFuzzyColor  = QUERYCHECK( hwndDlg, ID_TB_PROFONE_MACHFUZZCOLOR );
             EQFBUserOpt.fMachFuzzyDiff = QUERYCHECK( hwndDlg, ID_TB_PROFONE_MACHFUZZDIFF );

             {
               CHAR szFuzzy[10];
               USHORT usValue;
               QUERYTEXT( hwndDlg, ID_TB_PROFONE_FUZZYFORDIFF_EF, szFuzzy );
               usValue = (USHORT)atoi( szFuzzy );
               if ( (usValue < 5) || (usValue > 99) )
               {
                 PSZ pData = szFuzzy;
                 UtlError( ERROR_INVALID_FUZZYMATCH_VALUE, MB_CANCEL, 1, &pData, EQF_ERROR );
                 SETFOCUS( hwndDlg, ID_TB_PROFONE_FUZZYFORDIFF_EF );
                 mResult = 1; // do not leave dialog
               }
               else
               {
                 EQFBUserOpt.usFuzzyForDiv = usValue;
               } /* endif */                  
             } /* endif */         



             /********************************************************************/
             /* get proposal wind. display style                                 */
             /********************************************************************/
             pIda = ACCESSDLGIDA( hwndDlg, PPROFONEIDA );
             QUERYTEXT( hwndDlg, ID_TB_PROFONE_TMWND_CBS, pIda->szBuffer );
             for ( usJ = 0; usJ < MAXSTYLES; usJ++ )
             {
               if ( strcmp( pIda->szBuffer, pIda->szStyleArray[usJ] ) == 0 )
               {
                 break;
               } /* endif */
             } /* endfor */
             switch ( usJ )
             {
               case  0:
                 pEQFBUserOpt->DispTM = DISP_PROTECTED;
                 break;
               case  1:
                 pEQFBUserOpt->DispTM = DISP_HIDE;
                 break;
               case  2:
                 pEQFBUserOpt->DispTM = DISP_COMPACT;
                 break;
               case  3:
                 pEQFBUserOpt->DispTM = DISP_SHORTEN;
                 break;
               default :
                 break;
             } /* endswitch */
           } /* endif */
           break;
      } /* endswitch */
      break;

    case WM_HELP:
       /*************************************************************/
       /* pass on a HELP_WM_HELP request                            */
       /*************************************************************/
       EqfDisplayContextHelp( (HWND)((LPHELPINFO) mp2)->hItemHandle,
                              &hlpsubtblPropTranslMemDlg[0] );
       mResult = TRUE;  // message processed
       break;

    default:
       mResult = WinDefDlgProc( hwndDlg, msg, mp1, mp2 );
       break;
  } /* endswitch */

  return mResult;
};


//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     EQFBPROP_MESSAGES_DLGPROC
//------------------------------------------------------------------------------
// Function call:     EQFBPROP_MESSAGES_DLGPROC( hwndDlg, mp1, mp2);
//------------------------------------------------------------------------------
// Description:       handle the Messages page
//------------------------------------------------------------------------------
// Parameters:        HWND hwndDlg   handle of the dialog
//                    WPARAM  mp1    message parameter 1
//                    LPARAM  mp2    message parameter 2
//------------------------------------------------------------------------------
// Returncode type:   MRESULT
//------------------------------------------------------------------------------
// Returncodes:       return code from default window proc or FALSE
//------------------------------------------------------------------------------
// Function flow:     switch (msg)
//                     case WM_INITDLG:
//                          do initial settings
//                     case WM_COMMAND:
//                          handel Set pushbutton
//                     default:
//                          default dialog proc
//                    return
//------------------------------------------------------------------------------

INT_PTR CALLBACK EQFBPROP_MESSAGES_DLGPROC
(
   HWND hwndDlg,                       /* handle of dialog window             */
   WINMSG msg,
   WPARAM mp1,
   LPARAM mp2
)
{
  MRESULT  mResult = MRFROMSHORT( FALSE );                // result value of procedure
  USEROPT* pEQFBUserOpt = get_EQFBUserOpt();
  switch ( msg )
  {
    case WM_INITDLG:

      SETWINDOWID( hwndDlg, ID_TB_PROP_MESSAGES_DLG );
      SETCHECK( hwndDlg, ID_TB_PROFONE_CHECKTAGS,   pEQFBUserOpt->fTagCheck );
      SETCHECK( hwndDlg, ID_TB_PROFONE_MSGPROPEQ,   !pEQFBUserOpt->fFuzzyMsg );
      SETCHECK( hwndDlg, ID_TB_PROFONE_MSGSRCEQ,    !pEQFBUserOpt->fSrcUnChg );
      break;

    case WM_COMMAND:
      switch ( WMCOMMANDID( mp1, mp2 ) )
      {
         case ID_TB_PROP_SET_PB:           // get new value and set it
           /****************************************************************/
           /* if mp2 == 1L we have to validate the page, if it is 0L we    */
           /* have to copy the content of the dialog back into the struct. */
           /****************************************************************/
           if ( mp2 == 1L )
           {
             /**************************************************************/
             /* no checking for correct input necessary                    */
             /**************************************************************/
           }
           else
           {
             /**************************************************************/
             /* get the active settings ....                               */
             /**************************************************************/
             pEQFBUserOpt->fTagCheck =
                     (EQF_BOOL) QUERYCHECK( hwndDlg, ID_TB_PROFONE_CHECKTAGS );
             pEQFBUserOpt->fFuzzyMsg =
                     (EQF_BOOL) !QUERYCHECK( hwndDlg, ID_TB_PROFONE_MSGPROPEQ );
             pEQFBUserOpt->fSrcUnChg =
                     (EQF_BOOL) !QUERYCHECK( hwndDlg, ID_TB_PROFONE_MSGSRCEQ );
           } /* endif */
           break;
      } /* endswitch */
      break;

    case WM_HELP:
       /*************************************************************/
       /* pass on a HELP_WM_HELP request                            */
       /*************************************************************/
       EqfDisplayContextHelp( (HWND)((LPHELPINFO) mp2)->hItemHandle,
                              &hlpsubtblPropMessageDlg[0] );
       mResult = TRUE;  // message processed
       break;

    default:
       mResult = WinDefDlgProc( hwndDlg, msg, mp1, mp2 );
       break;
  } /* endswitch */

  return mResult;
};


//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     EQFBPropCommand
//------------------------------------------------------------------------------
// Function call:     EQFBPropCommand( hwndDlg, mp1, mp2);
//------------------------------------------------------------------------------
// Description:       get the settings and write it down, or ignore changes.
//------------------------------------------------------------------------------
// Parameters:        HWND hwndDlg   handle of the dialog
//                    WPARAM  mp1    message parameter 1
//                    LPARAM  mp2    message parameter 2
//------------------------------------------------------------------------------
// Returncode type:   MRESULT
//------------------------------------------------------------------------------
// Returncodes:       return code from default window proc or FALSE
//------------------------------------------------------------------------------
// Function flow:     SET pushbutton:get active setting
//                                   if ok write profile
//                                   post WM_CLOSE
//                    CANCEL pushbutton: post WM_CLOSE
//------------------------------------------------------------------------------

MRESULT EQFBPropCommand
(
   HWND hwndDlg,
   WPARAM mp1,
   LPARAM mp2
)
{
   MRESULT mResult = MRFROMSHORT(TRUE);          // TRUE = command is processed
   BOOL fOK = TRUE;
   mp2;

   switch ( WMCOMMANDID( mp1, mp2 ) )
   {
          case ID_TB_PROP_HELP_PB:
            mResult = UtlInvokeHelp();
            break;
      case ID_TB_PROP_SET_PB:           // get new value and set it
        /**************************************************************/
        /* get the active settings ....                               */
        /**************************************************************/
        {
          PPROFONEIDA pIda;            // pointer to profile ida
          PTBDOCUMENT pDoc;            // ptr to doc struct in ida
          PSTEQFGEN   pstEQFGen;       // pointer to generic struct
          USHORT nItem = 0;
          PFNWP pfnWp;
          pIda = ACCESSDLGIDA(hwndDlg, PPROFONEIDA);
          /************************************************************/
          /* issue command to all dialog windows                      */
          /************************************************************/
          while ( pIda->hwndPages[nItem] && fOK )
          {
            pfnWp = (PFNWP) GetWindowLong( pIda->hwndPages[ nItem ],
                                           DWL_DLGPROC );

            fOK =  !pfnWp( pIda->hwndPages[nItem], WM_COMMAND,
                              ID_TB_PROP_SET_PB, 0L);
            nItem++;
          } /* endwhile */

          /**************************************************************/
          /* write it as new settings ....                              */
          /**************************************************************/
          if ( fOK )
          {
            pDoc = pIda->pDoc;
            EQFBWriteProfile(pDoc);
            /************************************************************/
            /* repaint all service windows -- they might be affected    */
            /* by the changes...                                        */
            /************************************************************/
            pstEQFGen = (PSTEQFGEN) pDoc->pstEQFGen;
            if ( pstEQFGen)
            { // R007563:
                                WinPostMsg(pstEQFGen->hwndTWBS, WM_EQF_WD_MAIN_NOTIFY,
                                           NULL,
                                           MP2FROM2SHORT( 1 ,ID_TB_PROP_DICTIONARY_DLG)     );
                    }
            if ( pstEQFGen)
            {
              WinPostMsg( pstEQFGen->hwndTWBS,
                        WM_EQF_COLCHANGED, NULL, NULL);
            } /* endif */
            POSTEQFCLOSE( hwndDlg, FALSE );
          } /* endif */

        } /* endif */
        break;

      case ID_TB_PROP_CANCEL_PB:
      case DID_CANCEL:
        POSTEQFCLOSE( hwndDlg, FALSE );
        break;

      default:
         mResult = WinDefDlgProc( hwndDlg, WM_COMMAND, mp1, mp2 );
         break;
   } /* endswitch */

   return( mResult );
} /* end of EQFBPropCommand */


//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     EQFBPROP_BACKTASKSDLGPROC
//------------------------------------------------------------------------------
// Function call:     EQFBPROP_BACKTASKS_DLGPROC( hwndDlg, mp1, mp2);
//------------------------------------------------------------------------------
// Description:       handle the Background tasks page
//------------------------------------------------------------------------------
// Parameters:        HWND hwndDlg   handle of the dialog
//                    WPARAM  mp1    message parameter 1
//                    LPARAM  mp2    message parameter 2
//------------------------------------------------------------------------------
// Returncode type:   MRESULT
//------------------------------------------------------------------------------
// Returncodes:       return code from default window proc or FALSE
//------------------------------------------------------------------------------
// Function flow:     switch (msg)
//                     case WM_INITDLG:
//                          do initial settings
//                     case WM_COMMAND:
//                          handel Set pushbutton
//                     default:
//                          default dialog proc
//                    return
//------------------------------------------------------------------------------

INT_PTR CALLBACK EQFBPROP_BACKTASKS_DLGPROC
(
   HWND hwndDlg,                       /* handle of dialog window             */
   WINMSG msg,
   WPARAM mp1,
   LPARAM mp2
)
{
  MRESULT  mResult = MRFROMSHORT( FALSE );                // result value of procedure
  PPROFONEIDA pIda;                    // pointer to profile ida
  SHORT       sI;
  HAB         hab;                               // dialog anchor
  SHORT       sItem;
  USEROPT* pEQFBUserOpt = get_EQFBUserOpt();

  switch ( msg )
  {
    case WM_INITDLG:
      hab = GETINSTANCE( hwndDlg );
      pIda = (PPROFONEIDA) mp2;
      ANCHORDLGIDA( hwndDlg, pIda );
      SETWINDOWID( hwndDlg, ID_TB_PROP_BACKTASKS_DLG );

      /****************************************************************/
      /* Set last used values                                         */
      /****************************************************************/

      // GQ: 2015/10/28 Temporarely disable auto-save function as it seems to cause incorrect STARGET documents
      ENABLECTRL( hwndDlg, ID_TB_PROFONE_AUTOSAVE, FALSE );
      ENABLECTRL( hwndDlg, ID_TB_PROFONE_MINUTE_CBS, FALSE );
      
      SETCHECK_FALSE( hwndDlg, ID_TB_PROFONE_AUTOSAVE );
      //SETCHECK( hwndDlg, ID_TB_PROFONE_AUTOSAVE, pEQFBUserOpt->UserOptFlags.bBackSave );


      SETCHECK( hwndDlg, ID_TB_PROFONE_AUTOSUBST, pEQFBUserOpt->UserOptFlags.bBackSubst);

     /********************************************************************/
     /* fill cbs with minutes                                            */
     /********************************************************************/
     for ( sI = 1; sI < 5; sI++ )
     {
       itoa(sI, pIda->szBuffer, 10);
       sItem = CBINSERTITEMEND( hwndDlg, ID_TB_PROFONE_MINUTE_CBS,
                              pIda->szBuffer);
     } /* endfor */
     for ( sI = 5; sI < 40; sI= sI+5 )
     {
       itoa(sI, pIda->szBuffer, 10);
       sItem = CBINSERTITEMEND( hwndDlg, ID_TB_PROFONE_MINUTE_CBS,
                              pIda->szBuffer);
     } /* endfor */

     if (!pEQFBUserOpt->UserOptFlags.bBackSave )
     {
       CBSELECTITEM( hwndDlg, ID_TB_PROFONE_MINUTE_CBS, 0);
     }
     else
     {
       itoa( pEQFBUserOpt->sMinuteTilNextSave, pIda->szBuffer, 10 );
       CBSEARCHSELECT( sI, hwndDlg, ID_TB_PROFONE_MINUTE_CBS, pIda->szBuffer );
     } /* endif */
      break;

    case WM_COMMAND:
      switch ( WMCOMMANDID( mp1, mp2 ) )
      {
         case ID_TB_PROP_SET_PB:           // get new value and set it
           /****************************************************************/
           /* if mp2 == 1L we have to validate the page, if it is 0L we    */
           /* have to copy the content of the dialog back into the struct. */
           /****************************************************************/
           if ( mp2 == 1L )
           {
             /**************************************************************/
             /* no checking for correct input necessary                    */
             /**************************************************************/
           }
           else
           {
             /**************************************************************/
             /* get the active settings, invoke background tasks if ON     */
             /**************************************************************/
             PPROFONEIDA pIda;            // pointer to profile ida
             pIda = ACCESSDLGIDA(hwndDlg, PPROFONEIDA);
             /*********************************************************/
             /* background save                                       */
             /*********************************************************/
             pEQFBUserOpt->UserOptFlags.bBackSave =
                      QUERYCHECK( hwndDlg, ID_TB_PROFONE_AUTOSAVE );
             if ( pEQFBUserOpt->UserOptFlags.bBackSave )
             {
               CHAR     chText[RMARGIN_LENGTH+1];   // buffer for minutes
               OEMQUERYTEXT( hwndDlg, ID_TB_PROFONE_MINUTE_CBS, chText );
               pEQFBUserOpt->sMinuteTilNextSave = GetNumber( chText );
               //start thread
               if (pIda->pDoc->docType == STARGET_DOC )
               {
                 EQFBWorkThreadTask(pIda->pDoc, THREAD_AUTOSAVE);
               } /* endif */
             } /* endif */
             /*********************************************************/
             /* background automatic substitution                     */
             /*********************************************************/
             pEQFBUserOpt->UserOptFlags.bBackSubst =
                      QUERYCHECK( hwndDlg, ID_TB_PROFONE_AUTOSUBST );
             if (pEQFBUserOpt->UserOptFlags.bBackSubst
                   && (pIda->pDoc->docType == STARGET_DOC))
             {
               EQFBWorkThreadTask(pIda->pDoc, THREAD_AUTOSUBST);
             } /* endif */
           } /* endif */
           break;
      } /* endswitch */
      break;

    case WM_HELP:
       /*************************************************************/
       /* pass on a HELP_WM_HELP request                            */
       /*************************************************************/
       EqfDisplayContextHelp( (HWND)((LPHELPINFO) mp2)->hItemHandle,
                              &hlpsubtblPropBackTasksDlg[0] );
       mResult = TRUE;  // message processed
       break;

    default:
       mResult = WinDefDlgProc( hwndDlg, msg, mp1, mp2 );
       break;
  } /* endswitch */

  return mResult;
};


//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     EQFBPROP_AUTSUBSTDLGPROC
//------------------------------------------------------------------------------
// Function call:     EQFBPROP_AUTSUBST_DLGPROC( hwndDlg, mp1, mp2);
//------------------------------------------------------------------------------
// Description:       handle the Background tasks page
//------------------------------------------------------------------------------
// Parameters:        HWND hwndDlg   handle of the dialog
//                    WPARAM  mp1    message parameter 1
//                    LPARAM  mp2    message parameter 2
//------------------------------------------------------------------------------
// Returncode type:   MRESULT
//------------------------------------------------------------------------------
// Returncodes:       return code from default window proc or FALSE
//------------------------------------------------------------------------------
// Function flow:     switch (msg)
//                     case WM_INITDLG:
//                          do initial settings
//                     case WM_COMMAND:
//                          handel Set pushbutton
//                     default:
//                          default dialog proc
//                    return
//------------------------------------------------------------------------------


INT_PTR CALLBACK EQFBPROP_AUTSUBST_DLGPROC
(
   HWND hwndDlg,                       /* handle of dialog window             */
   WINMSG msg,
   WPARAM mp1,
   LPARAM mp2
)
{
  MRESULT  mResult = MRFROMSHORT( FALSE );                // result value of procedure
  PPROFONEIDA pIda;                    // pointer to profile ida
  HAB         hab;                               // dialog anchor
  USEROPT* pEQFBUserOpt = get_EQFBUserOpt();

  switch ( msg )
  {
    case WM_INITDLG:
      hab = GETINSTANCE( hwndDlg );
      pIda = (PPROFONEIDA) mp2;
      ANCHORDLGIDA( hwndDlg, pIda );
      SETWINDOWID( hwndDlg, ID_TB_PROP_AUTSUBST_DLG );

      /****************************************************************/
      /* Set last used values                                         */
      /****************************************************************/
      SETCHECK( hwndDlg, ID_TB_PROFONE_AUTOSTOP, pEQFBUserOpt->fAutoStop );
      SETCHECK( hwndDlg, ID_TB_PROFONE_EXACTCONTEXTMATCH, pEQFBUserOpt->fExactContextTMMatch);
      SETCHECK( hwndDlg, ID_TB_PROFONE_USELATESTMATCH, pEQFBUserOpt->fUseLatestMatch );

      SETCHECK( hwndDlg, ID_TB_PROFONE_ADJUSTLEADINGWS, pEQFBUserOpt->UserOptFlags.bAdjustLeadingWS );
      SETCHECK( hwndDlg, ID_TB_PROFONE_ADJUSTTRAILINGWS, pEQFBUserOpt->UserOptFlags.bAdjustTrailingWS );

      break;

    case WM_COMMAND:
      switch ( WMCOMMANDID( mp1, mp2 ) )
      {
         case ID_TB_PROP_SET_PB:           // get new value and set it
           /****************************************************************/
           /* if mp2 == 1L we have to validate the page, if it is 0L we    */
           /* have to copy the content of the dialog back into the struct. */
           /****************************************************************/
           if ( mp2 == 1L )
           {
             /**************************************************************/
             /* no checking for correct input necessary                    */
             /**************************************************************/
           }
           else
           {
             /**************************************************************/
             /* store the new settings                                     */
             /**************************************************************/
             pEQFBUserOpt->fAutoStop =
                     (EQF_BOOL) QUERYCHECK( hwndDlg, ID_TB_PROFONE_AUTOSTOP );
             pEQFBUserOpt->fExactContextTMMatch =
                     (EQF_BOOL) QUERYCHECK( hwndDlg, ID_TB_PROFONE_EXACTCONTEXTMATCH );
             pEQFBUserOpt->fUseLatestMatch =
                     (EQF_BOOL) QUERYCHECK( hwndDlg, ID_TB_PROFONE_USELATESTMATCH );
             pEQFBUserOpt->UserOptFlags.bAdjustLeadingWS =
                     QUERYCHECK( hwndDlg, ID_TB_PROFONE_ADJUSTLEADINGWS );
             pEQFBUserOpt->UserOptFlags.bAdjustTrailingWS =
                     QUERYCHECK( hwndDlg, ID_TB_PROFONE_ADJUSTTRAILINGWS);

           } /* endif */
           break;
      } /* endswitch */
      break;

    case WM_HELP:
       /*************************************************************/
       /* pass on a HELP_WM_HELP request                            */
       /*************************************************************/
       EqfDisplayContextHelp( (HWND)((LPHELPINFO) mp2)->hItemHandle,
                              &hlpsubtblPropBackTasksDlg[0] );
                              //@@ TODO
       mResult = TRUE;  // message processed
       break;

    default:
       mResult = WinDefDlgProc( hwndDlg, msg, mp1, mp2 );
       break;
  } /* endswitch */

  return mResult;
};

