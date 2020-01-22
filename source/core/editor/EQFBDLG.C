/*! \file
	Description: Dialogs used within Translation Processor

	Copyright Notice:

	Copyright (C) 1990-2016, International Business Machines
	Corporation and others. All rights reserved
*/

#define INCL_EQF_TP               // public translation processor functions
#define INCL_EQF_PRINT            // print functions
#define INCL_EQF_EDITORAPI        // editor API
#define INCL_EQF_TAGTABLE         // tagtable defines and functions
#define INCL_EQF_FOLDER           // folder list and document list functions
#define INCL_EQF_TM
#include <eqf.h>                  // General Translation Manager include file

#include "EQFTPI.H"               // Translation Processor priv. include file
#include "EQFB.ID"                // Translation Processor IDs
#include "EQFBDLG.ID"             // dialog control IDs
// Add for R012027
#include "SpecialCharDlg.h"
// Add end

VOID EQFBOpenFillListboxes( HWND hwndDlg, POPENIDA pIda );
VOID EQFBKeysFillListbox( HWND hwndDlg, USHORT id,
                          PKEYPROFTABLE pKey, PFUNCTIONTABLE pFunc,
                          BYTE bEditor );
static VOID EQFBKeysPrint ( HWND );
static MRESULT DrawKeysFuncList ( HWND hwndDlg, LPARAM mp2, BOOL fDifColor, BYTE bEditor );

BOOL EQFBCheckForMnemonic( PKEYSIDA, WPARAM, LPARAM);
VOID EQFBLeaveCaptMode( HWND, PKEYSIDA );


extern PPROPSYSTEM pSysProp;           // address sysprop ptr of EQFBMAIN.C



#define BUILDPATH( buffer, drive, dir, file ) \
   { *(buffer) = drive;         \
     *((buffer)+1) = COLON;     \
     *((buffer)+2) = BACKSLASH; \
     *((buffer)+3) = EOS;       \
     strcat( buffer, dir );     \
     strcat( buffer, "\\" );    \
     strcat( buffer, file ); }  \

#define GETSELKEY( hwndDlg, pIda )  \
   {                                                                    \
     SHORT sItem = QUERYSELECTION( hwndDlg, ID_TB_KEYS_FUNCTION_LB );   \
     if ( sItem != LIT_NONE )                                           \
     {                                                                  \
        pIda->sItem = sItem;                                            \
        pIda->pKey  = GETITEMHANDLE( hwndDlg, ID_TB_KEYS_FUNCTION_LB,   \
                                     sItem, PKEYPROFTABLE );            \
     } /* endif */                                                      \
   }



MRESULT EQFBKeysInit ( HWND, WPARAM, LPARAM );
MRESULT EQFBKeysChar( HWND, USHORT, WPARAM, LPARAM );
MRESULT EQFBKeysCommand( HWND, WPARAM, LPARAM );
MRESULT EQFBKeysClose( HWND, WPARAM, LPARAM );

MRESULT EQFBOpenInit ( HWND, WPARAM, LPARAM );
MRESULT EQFBOpenControl( HWND, SHORT, SHORT );
MRESULT EQFBOpenChar( HWND, WPARAM, LPARAM );
MRESULT EQFBOpenCommand( HWND, WPARAM, LPARAM );
MRESULT EQFBOpenClose( HWND, WPARAM, LPARAM );

MRESULT EQFBCommandCommand( HWND, WPARAM, LPARAM );


USHORT EQFBQueryLineHeight( VOID );
static ULONG  ulEQFBLineWidth  = 0;    // static value for line width
static USHORT usEQFBLineHeight = 0;    // static value for line heigth

SHORT UtlLoadDocNames( PSZ pszSearch, USHORT atrb, HWND hlbox, USHORT flg);

  /********************************************************************/
  /* define subclassing proc for assigning key strokes                */
  /********************************************************************/
  WNDPROC lpOldProc = NULL;
  MRESULT APIENTRY EQFBKeysAssignSubProc ( HWND, WINMSG, WPARAM, LPARAM );

/*////////////////////////////////////////////////////////////////////////////
:h2.EQFBKeysDlgProc - dialog procedure for keys dialog
*/
// Description:
//    Allows the assignment of keys to translation browser functions.
//
//   Flow (message driven):
//       case WM_INITDLG:
//         call EQFBKeysInit to initialize the dialog controls;
//       case WM_CHAR:
//         call EQFBKeysChar to handle character input;
//       case WM_COMMAND
//         call EQFBKeysCommand to handle user commands;
//       case WM_MEASUREITEM:
//         return listbox item height
//       case WM_DRAWITEM:
//         draw a listbox item
//       case WM_CLOSE
//         call EQFBKeysClose to end the dialog;
//
// Arguments:
//  mp2 of WM_INITDLG msg = PKEYSDATA pKeysData ptr to data structure containing
//                          last used values and key assignments.
//
// Returns:
//  USHORT usRC      FALSE          - dialog was cancelled or closed
//                   TRUE           - user pressed 'Set' button
//
// Prereqs:
//   None
//
// SideEffects:
//   if usRC <> 0 is returned, the KEYSDATA structure contains the assignments
//   made by the user
//
// External references:
//   None
//
//////////////////////////////////////////////////////////////////////////////
INT_PTR CALLBACK EQFBKEYSDLGPROC
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
      case WM_EQF_QUERYID: HANDLEQUERYID( ID_TB_KEYS_DLG, mp2 ); break;

      case WM_INITDLG:
          SETWINDOWID( hwndDlg, ID_TB_KEYS_DLG );
          ulEQFBLineWidth = 0;
          mResult = DIALOGINITRETURN( EQFBKeysInit( hwndDlg, mp1, mp2 ));
          break;

      case WM_CHAR:
         mResult = EQFBKeysChar( hwndDlg, (USHORT)msg, mp1, mp2 );
         break;


      case WM_COMMAND:
         mResult = EQFBKeysCommand( hwndDlg, mp1, mp2 );
         break;

      case WM_MEASUREITEM:
         MEASUREITEM( mp2, EQFBQueryLineHeight(), mResult );
         break;

      case WM_DRAWITEM:
 		     {
           PKEYSIDA pIda = ACCESSDLGIDA(hwndDlg, PKEYSIDA);
		       BYTE bEditor = (pIda->pDoc->hwndRichEdit) ? EDIT_RTF : EDIT_STANDARD;
           mResult = DrawKeysFuncList( hwndDlg, mp2, TRUE, bEditor );
		     }
         break;

      case WM_EQF_CLOSE:
         mResult = EQFBKeysClose( hwndDlg, mp1, mp2 );
         break;

      default:
         mResult = WinDefDlgProc( hwndDlg, msg, mp1, mp2 );
         break;
   } /* endswitch */
   return mResult;

} /* end of EQFBKeysDlgProc */

/**********************************************************************/
/* enforce that all keys are passed back to our standard procedure... */
/**********************************************************************/
MRESULT APIENTRY EQFBKeysAssignSubProc
(
   HWND hwndBtn,
   WINMSG msg,
   WPARAM mp1,
   LPARAM mp2
)
{
  MRESULT  mResult = FALSE;         // return code from subclassing

  switch ( msg )
  {
     case WM_GETDLGCODE:
       /***********************************************************/
       /* if we are in capture mode -- we want all key strokes ...*/
       /***********************************************************/
       mResult = DLGC_WANTALLKEYS;
       break;
     case WM_SYSCHAR:
//       break;
     case WM_SYSKEYDOWN:
     case WM_KEYDOWN:
     case WM_CHAR:
        if (mp1 != VK_ESCAPE )
        {
          // we have to use the handle of the dialog window...
          mResult = EQFBKeysChar( GetParent(hwndBtn), (USHORT)msg, mp1, mp2 );
        }
        else
        {
          HWND hwndDlg = GetParent( hwndBtn );
          EQFBLeaveCaptMode( hwndDlg, ACCESSDLGIDA(hwndDlg, PKEYSIDA));
        } /* endif */
        break;
     default :
       mResult = (MRESULT) CallWindowProc( (WNDPROC) lpOldProc, hwndBtn,
                                           msg, mp1, mp2 );
       break;
  } /* endswitch */
  return mResult;
}


//-----------------------------------------------------------------------------
// Internal function
//-----------------------------------------------------------------------------
// Function name:     DrawKeysFuncList
//-----------------------------------------------------------------------------
// Description:       Draw the OwnerDraw item in our listbox...
//-----------------------------------------------------------------------------
// Parameters:        LPARAM   mp2   -- struct passed for selected draw item
//                    BOOL     fDiffCol -- use different colors for displ...
//-----------------------------------------------------------------------------
// Returncode type:   MRESULT
//-----------------------------------------------------------------------------
// Returncodes:       FALSE: we handled message
//-----------------------------------------------------------------------------
// Function flow:     get the item structure;
//                    if change in state,
//                      get pointer to function structure
//                      get item text, find tab,
//                      display first part in blue, second part depending if
//                       function assignement was changed...
//                    return ...
//                    PS: Functionality for windows is same, but the item str.
//                        has a different setting....
//-----------------------------------------------------------------------------
static
MRESULT DrawKeysFuncList
(
  HWND hwndDlg,
  LPARAM mp2,                       // pointer to item structure
  BOOL   fDifColor,                 // force display in different colors ...
  BYTE bEditor                      // editor: EDIT_RTF or EDIT_STANDARD
)
{
  MRESULT  mResult = MRFROMSHORT( FALSE );      // result value of procedure
  CHAR     chWork[101];
  PSZ      pszKey;                    // begin of key in key name
  PFUNCTIONENTRY pFunc;               // ptr to function entry of current item
  DWORD   dwRGB_HIGHLIGHT = GetSysColor(COLOR_HIGHLIGHT);
  DWORD   dwRGB_HIGHLIGHTTEXT = CLR_RED;
  DWORD   dwRGB_WINDOW = CLR_WHITE;
  DWORD   dwRGB_WINDOWTEXT = CLR_BLACK;
  BOOL    fHighContrast = FALSE;

    LPDRAWITEMSTRUCT lpDisp = (LPDRAWITEMSTRUCT)mp2;
    if ( (lpDisp->itemID != -1) )
    {
     // Query extent required to draw the function strings (if not done yet)
     if ( ulEQFBLineWidth == 0L )
     {
       PFUNCTIONTABLE  pFuncList = get_FuncTab();
       PKEYPROFTABLE pKeyTemp = get_KeyTable(); 

       while ( pKeyTemp->Function != LAST_FUNC )
       {
         PSZ pszFuncName = (pFuncList + pKeyTemp->Function)->szDescription;

         USHORT usFuncValid = (bEditor & pKeyTemp->bEditor) ? 1 : 0;

         if ( (*pszFuncName) && usFuncValid)
         {
           LONG lCX, lCY;
           TEXTSIZE( lpDisp->hDC, pszFuncName, lCX, lCY );
           ulEQFBLineWidth = max( ulEQFBLineWidth, ((ULONG)lCX+2L) );
         } /* endif */
         pKeyTemp++;
       } /* endwhile */
       ulEQFBLineWidth += 2 * UtlQueryULong(QL_AVECHARWIDTH);
     }



      if ( lpDisp->itemAction & (ODA_DRAWENTIRE | ODA_SELECT) )
      {
		fHighContrast = UtlIsHighContrast();    // test for High Contrast
		if (fHighContrast)
		{
			  dwRGB_HIGHLIGHTTEXT = GetSysColor(COLOR_HIGHLIGHTTEXT);
			  dwRGB_WINDOW = GetSysColor(COLOR_WINDOW);
			  dwRGB_WINDOWTEXT = GetSysColor(COLOR_WINDOWTEXT);
		}
        // Modify for R012027 start
        SendMessage( lpDisp->hwndItem, LB_GETTEXT, lpDisp->itemID, (LPARAM) (LPCSTR) chWork );

        if (strnicmp(chWork, STR_INSERT_CHAR, strlen(STR_INSERT_CHAR)) != 0)
        {
            pFunc = (PFUNCTIONENTRY) SendMessage(lpDisp->hwndItem, LB_GETITEMDATA, lpDisp->itemID, 0L);
        }
        else
        {
            SPECCHARKEY* pFuncW;
            pFuncW = (SPECCHARKEY *) SendMessageW(lpDisp->hwndItem, LB_GETITEMDATA, lpDisp->itemID, 0L);
            pFunc = new FUNCTIONENTRY;
            pFunc->ucCode = pFuncW->ucCode;
            pFunc->ucState = pFuncW->ucState;
            pFunc->fChange = pFuncW->fChange;
        }
        // Add end

        pszKey = strchr( chWork, '\t' );
        *pszKey++ = EOS;

        if (  !(lpDisp->itemState & ODS_SELECTED)  )
        { // draw not selected items
			    FillRect(lpDisp->hDC, &(lpDisp->rcItem), (HBRUSH) (COLOR_WINDOW+1));
			    lpDisp->rcItem.left += 2; // provide a small left border

			    DRAWTEXT(lpDisp->hDC, chWork, lpDisp->rcItem, dwRGB_WINDOWTEXT,
					     dwRGB_WINDOW, DT_LEFT);
			    lpDisp->rcItem.left -= 2;

			    lpDisp->rcItem.left += LOWORD(ulEQFBLineWidth); // KEYNAME_OFFSET;
			    if (!fHighContrast)
			    {
			      DRAWTEXT(lpDisp->hDC, pszKey, lpDisp->rcItem,
					     (pFunc->fChange && fDifColor) ? CLR_RED : CLR_BLACK,
					     dwRGB_WINDOW, DT_LEFT);
		        }
		        else
		        {
			      DRAWTEXT(lpDisp->hDC, pszKey, lpDisp->rcItem,
					      dwRGB_WINDOWTEXT ,
					      dwRGB_WINDOW, DT_LEFT);
		        }
			    lpDisp->rcItem.left -= LOWORD(ulEQFBLineWidth);    //KEYNAME_OFFSET;
	      }
	      else
		{ // draw selected item
			FillRect(lpDisp->hDC, &(lpDisp->rcItem), (HBRUSH) (COLOR_HIGHLIGHT+1));
			lpDisp->rcItem.left += 2; // provide a small left border

			if (!fHighContrast)
			{
			  DRAWTEXT(lpDisp->hDC, chWork, lpDisp->rcItem, CLR_WHITE,
					 dwRGB_HIGHLIGHT, DT_LEFT);
			  lpDisp->rcItem.left -= 2;

			  lpDisp->rcItem.left += LOWORD(ulEQFBLineWidth); // KEYNAME_OFFSET;
			  DRAWTEXT(lpDisp->hDC, pszKey, lpDisp->rcItem,
					 (pFunc->fChange && fDifColor) ? CLR_RED : CLR_WHITE,
					 dwRGB_HIGHLIGHT, DT_LEFT);
		    }
		    else
		    {
				DRAWTEXT(lpDisp->hDC, chWork, lpDisp->rcItem, dwRGB_HIGHLIGHTTEXT,
					 dwRGB_HIGHLIGHT, DT_LEFT);
			    lpDisp->rcItem.left -= 2;

			    lpDisp->rcItem.left += LOWORD(ulEQFBLineWidth); // KEYNAME_OFFSET;
			    DRAWTEXT(lpDisp->hDC, pszKey, lpDisp->rcItem,
				     dwRGB_HIGHLIGHTTEXT,
					 dwRGB_HIGHLIGHT, DT_LEFT);
		    }
			lpDisp->rcItem.left -= LOWORD(ulEQFBLineWidth);    //KEYNAME_OFFSET;
	    }
      } /* endif */
      /**********************************************************/
      /* item gets or loses the focus ...                       */
      /**********************************************************/
      if (  lpDisp->itemAction & ODA_FOCUS  )
      {
        DrawFocusRect( lpDisp->hDC, &lpDisp->rcItem );
      } /* endif */
    } /* endif */
    mResult = MRFROMSHORT( TRUE );

  return mResult;
} /* end of function DrawKeysFuncList */
/*////////////////////////////////////////////////////////////////////////////
:h2.EQFBKeysInit - initialization for keys dialog
*/
// Description:
//    Initialize all dialog controls and allocate required memory.
//
//   Flow:
//      allocate and anchor dialog IDA;
//      fill function listbox with functions and assigned keys;
//      select first function or last used function if available;
//
// Arguments:
//  mp2 of WM_INITDLG msg = PKEYSDATA pKeysData ptr to data structure containing
//                          last used values and key assignments.
//
// Returns:
//  MRESULT(FALSE)
//
// Prereqs:
//   None
//
// SideEffects:
//   - ptr to IDA is stored in dialog word QWL_USER
//
// External references:
//   UtlAlloc
//
//////////////////////////////////////////////////////////////////////////////
MRESULT EQFBKeysInit
(
   HWND    hwndDlg,                    // handle of dialog window
   WPARAM  mp1,                        // first parameter of WM_INITDLG
   LPARAM  mp2                         // second parameter of WM_INITDLG
)
{
   PKEYSIDA    pIda;                   // dialog instance data area ptr
   BOOL        fOK = TRUE;             // internal OK flag
   PKEYSDATA   pKeysData;              // ptr to parameter passing structure
   BYTE        bEditor;

   mp1 = mp1;                          // suppress 'unreferenced parameter' msg
   //
   // create and anchor IDA
   //
   fOK = UtlAlloc((PVOID *) &pIda, 0L, (LONG) sizeof(KEYSIDA), ERROR_STORAGE );
   if ( fOK )
   {
      fOK = ANCHORDLGIDA( hwndDlg, pIda );
   } /* endif */

   if ( fOK )
   {
      //
      // copy data from keys data structure
      //
      pKeysData = (PKEYSDATA) mp2;
      pIda->pOrgKeyList  = pKeysData->pKeyList;
      pIda->pFuncList    = pKeysData->pFuncList;
      pIda->pResKeys     = pKeysData->pResKeys;
      pIda->hwndFrame    = pKeysData->hwndFrame;
      pIda->pDoc         = pKeysData->pDoc;

      //
      //--- get copy of key table ---
      //
      fOK = UtlAlloc((PVOID *) &pIda->pNewKeyList,
                      0L,
                      (LONG) (sizeof(KEYPROFTABLE) * (LAST_FUNC+1)),
                      ERROR_STORAGE );

   } /* endif */
   if ( fOK )
   {
      memcpy( pIda->pNewKeyList, pIda->pOrgKeyList,
              sizeof(KEYPROFTABLE) * (LAST_FUNC+1));

      //--- hide capture key text ---
      HIDECONTROL( hwndDlg, ID_TB_KEYS_KEY_TEXT );

      //
      // fill function listbox and reset change flags
      //

      bEditor = (pIda->pDoc->hwndRichEdit) ? EDIT_RTF : EDIT_STANDARD;

      EQFBKeysFillListbox( hwndDlg, ID_TB_KEYS_FUNCTION_LB,
                           pIda->pNewKeyList, pIda->pFuncList,
                           bEditor);

      //
      // select first item in listbox (triggering WM_CONTROL with LN_SELECT)
      //
      SELECTITEM( hwndDlg, ID_TB_KEYS_FUNCTION_LB, 0 );
   } /* endif */

   return ( MRFROMSHORT(!fOK) );
} /* end of EQFBKeysInit */

/*////////////////////////////////////////////////////////////////////////////
:h2.EQFBKeysChar - process WM_CHAR messages of keys dialog
*/
// Description:
//    Capture WM_CHAR messages in keys dialog panel.
//
//   Flow:
//      if IDA flag fCapture is set
//         if message contains data for a valid key
//            reset fCapture flag
//            store valid key data for active function;
//         else
//            issue a beep;
//         endif;
//      else
//         pass message to default dialog procedure;
//      endif;
//
// Arguments:
//   SHORT1FROMMP(mp1) = ID of control sending the WM_CHAR message
//
// Returns:
//  MRESULT(TRUE)  command is processed
//
// Prereqs:
//   None
//
// SideEffects:
//   - IDA fields are updated if required
//
// External references:
//   None
//
//////////////////////////////////////////////////////////////////////////////
MRESULT EQFBKeysChar
(
   HWND hwndDlg,
   USHORT usMsg,
   WPARAM mp1,
   LPARAM mp2
)
{
   MRESULT  mResult = MRFROMSHORT(TRUE);// TRUE = command is processed
   PKEYSIDA pIda;                      // dialog instance data area ptr
   UCHAR    ucCode = 0;                    // generated key code
   UCHAR    ucState = 0;                   // generated key state
   BOOL     fKey = FALSE;              // valid key flag
   PRESKEYTABLE pResKey = NULL;        // ptr for processing of reserved keys
   PKEYPROFTABLE pKey;                 // ptr for processing of assigned keys
   CHAR     szKeyName[40];             // buffer for key names
   PSZ      pszErrParms[2];            // error parameter table
   SHORT    sItem;                     // index of listbox items
   USHORT   usMBCode;                  // message box return code
   BOOL     fFoundInResKey = FALSE;    // true if in list of reserved keys
   USHORT   usOldAssignStatus = NOT_ASSIGNABLE;
   UCHAR    ucOldCode;
   UCHAR    ucOldState;

   usMsg;                              // get rid of compiler warnings

   pIda = ACCESSDLGIDA(hwndDlg, PKEYSIDA);

   if ( pIda->fCapture )
   {
     //---  get the selected key string
      GETSELKEY( hwndDlg, pIda );
      //--- convert message into key code and state ---

      switch ( usMsg )
      {
        case WM_KEYDOWN:
          /********************************************************************/
          /* ignore CTRL key if it comes from an extended key...              */
          /********************************************************************/
          if ( mp2 & 0x020000000 )
          {
            ucState = 0;
            fKey = FALSE;
          }
          else
          {
            fKey = EQFBKeyState( (USHORT)mp1, &ucCode, &ucState );
          } /* endif */
          break;
        case WM_SYSKEYDOWN:
          /***************************************************************/
          /* for some reasons F10 arrives as a SYSKEY                    */
          /***************************************************************/
          if (!(mp2 & 0x20000000) )
          {
            fKey = EQFBKeyState( (USHORT)mp1, &ucCode, &ucState );
          }
          else
          {
            fKey = EQFBKeyState( (USHORT)mp1, &ucCode, &ucState );
            ucState |= ST_ALT;
          } /* endif */
          break;
        case WM_CHAR:
          ucCode = (UCHAR) LOWORD( mp1 );
          ucState = 0;
          fKey = TRUE;
          if ( (ucCode == VK_ENTER )  ||
               (ucCode == VK_TAB )    ||
               (ucCode == VK_ESC )    ||
               (ucCode == VK_BACKSPACE) )
          {
            ucState |= ST_VK;
            if ( (ucCode == VK_TAB) && (GetKeyState(VK_SHIFT) & 0x8000) )
            {
              /**********************************************************/
              /* ignore shift status and set backtab                    */
              /**********************************************************/
              ucCode = VK_BACKTAB;
              ucState  &= ~ST_SHIFT;
            } /* endif */
          } /* endif*/
          /****************************************************************/
          /* refine for dictionary proposal copy...                       */
          /****************************************************************/
          if ( GetKeyState (VK_CTRL) & 0x8000 )
          {
            if ( (ucCode <= 'Z' - 'A' + 1) || (ucCode == 127)
                  || (ucCode == 28) || (ucCode == 29) )
            {
              fKey = FALSE;
            } /* endif */
          } /*endif*/
          break;

        case WM_SYSCHAR:
          if ( mp2 & 0x20000000 )
          {
            fKey = TRUE;
            ucState = ST_ALT;
            ucCode = (UCHAR) LOWORD( mp1 );

            if ( (ucCode == VK_ENTER )  ||
                 (ucCode == VK_TAB )    ||
                 (ucCode == VK_ESC )    ||
                 (ucCode == VK_BACKSPACE) )
            {
              ucState |= ST_VK;
              if ( (ucCode == VK_TAB) && (GetKeyState(VK_SHIFT) & 0x8000) )
              {
                /**********************************************************/
                /* ignore shift status and set backtab                    */
                /**********************************************************/
                ucCode = VK_BACKTAB;
                ucState  &= ~ST_SHIFT;
              } /* endif */
            } /* endif*/
          } /* endif */
          break;
        case WM_SYSDEADCHAR:
        case WM_DEADCHAR:
          ucState = ucCode = 0;
          break;
        default:
          MessageBeep( (UINT) -1 );
          ucState = ucCode = 0;
          break;
      } /* endswitch */

      //--- check if key is no character key ---
      if ( fKey )
      {
         if ( ucState == 0 )
         {
            EQFBKeyName( szKeyName, ucCode, ucState );
            pszErrParms[0] = szKeyName;
            usMBCode = UtlError( TB_KEY_IS_CHARACTER, MB_YESNO, 1,
                              pszErrParms, EQF_QUERY );
            if ( usMBCode == MBID_NO )
            {
               EQFBLeaveCaptMode( hwndDlg, pIda );
            } /* endif */
            fKey = FALSE;           // key is invalid
         } /* endif */
      } /* endif */

      //--- check if key is no accelerator key
      if ( fKey )
      {
         if ( EQFBCheckForMnemonic( pIda, mp1, mp2 ) )
         {
            EQFBKeyName( szKeyName, ucCode, ucState );
            pszErrParms[0] = szKeyName;
            usMBCode = UtlError( TB_KEY_ACCELERATOR, MB_YESNO, 1,
                                 pszErrParms, EQF_QUERY );
            if ( usMBCode == MBID_NO )
            {
               EQFBLeaveCaptMode( hwndDlg, pIda );
            } /* endif */

            fKey = FALSE;           // key is invalid
         } /* endif */
      } /* endif */


      //--- check if key is a valid (=not reserved key) ---
      if ( fKey )
      {
         pResKey = pIda->pResKeys;
         usOldAssignStatus = NOT_ASSIGNABLE;
         while ( fKey && (pResKey->ucCode || pResKey->ucState)
                 && (!fFoundInResKey) )
         {
            if ( (pResKey->ucState == ucState) && (pResKey->ucCode == ucCode) )
            {
               EQFBKeyName( szKeyName, ucCode, ucState );
               pszErrParms[0] = szKeyName;
               if (pResKey->usAssignStatus == NOT_ASSIGNABLE )
               {
                 if ( (pResKey->Function == GETPROPMATCH_FUNC)  ||
                      (pResKey->Function == GETDICTMATCH_FUNC))
                 {
                    usMBCode = UtlError( TB_KEY_RESERVED_FOR_TP, MB_YESNO, 1,
                                         pszErrParms, EQF_QUERY );
                 }
                 else
                 {
                    usMBCode = UtlError( TB_KEY_RESERVED_FOR_OS2, MB_YESNO, 1,
                                        pszErrParms, EQF_QUERY );
                 } /* endif */
                 if ( usMBCode == MBID_NO )
                 {
                    EQFBLeaveCaptMode( hwndDlg, pIda );
                 } /* endif */
                 fKey = FALSE;           // key is invalid
               }
               else
               {
                 usOldAssignStatus = pResKey->usAssignStatus;
                 fFoundInResKey = TRUE;
               } /* endif */
            }
            else
            {
               pResKey++;
            } /* endif */
         } /* endwhile */
      } /* endif */

      //--- check if key is a free (=not used yet) ---
      if ( fKey )
      {
         pKey = pIda->pNewKeyList;           // start at begin of key list
         while ( fKey && (pKey->Function != LAST_FUNC) )
         {
            if ( (pKey->ucState || pKey->ucCode) &&
                 (pKey->ucState == ucState) &&
                 (pKey->ucCode == ucCode) &&
                 (pKey != pIda->pKey) )
            {
               EQFBKeyName( szKeyName, ucCode, ucState );
               pszErrParms[0] = szKeyName;
               pszErrParms[1] = (pIda->pFuncList+pKey->Function)->szDescription;
               usMBCode = UtlError( TB_KEY_IN_USE, MB_YESNOCANCEL, 2,
                                    pszErrParms, EQF_QUERY );
               switch (usMBCode)
               {
                  case MBID_YES:
                     // release key assignment
                     pKey->ucState = NULC;
                     pKey->ucCode  = NULC;
                     pKey->fChange = TRUE;

                     // locate listbox item belonging to old function
                     sprintf( pIda->szItemName, "%s\t%s",
                              (pIda->pFuncList+pKey->Function)->szDescription,
                              szKeyName );
                     sItem = SEARCHITEM( hwndDlg, ID_TB_KEYS_FUNCTION_LB,
                                           pIda->szItemName );

                     // change listbox item text
                     if ( sItem != LIT_NONE )
                     {
                        EQFBKeyName( szKeyName,
                                     pKey->ucCode,
                                     pKey->ucState );
                        sprintf( pIda->szItemName, "%s\t%s",
                                 (pIda->pFuncList+pKey->Function)->szDescription,
                                 szKeyName );
                        SETITEMTEXT( hwndDlg, ID_TB_KEYS_FUNCTION_LB,
                                     sItem, pIda->szItemName  );
                     } /* endif */

                     break;
                  case MBID_NO:
                     fKey = FALSE;        // key is invalid
                     break;
                  default:                // case cancel
                     fKey = FALSE;
                     EQFBLeaveCaptMode( hwndDlg, pIda );
                     break;
               } /*endswitch*/
            }
            else
            {
               pKey++;
            } /* endif */
         } /* endwhile */

         // Add for R012027 start
         // check for special char
         SPECCHARKEYVEC* pSpecCharKeyVec = GetSpecCharKeyVec();
         for (size_t iInx = 0; iInx < (*pSpecCharKeyVec).size(); iInx++)
         {
             wchar_t  wstrVecStr[MAX_SPEC_CHAR_STR_LEN];
             wchar_t  wstrItmTxt[MAX_SPEC_CHAR_STR_LEN];
             memset(wstrVecStr, 0x00, sizeof(wstrVecStr));
             memset(wstrItmTxt, 0x00, sizeof(wstrItmTxt));

             wcscpy(wstrVecStr, (*pSpecCharKeyVec)[iInx].wstrDispChar);
             SHORT nSelItm = QUERYSELECTION(hwndDlg, ID_TB_KEYS_FUNCTION_LB);
             QUERYITEMTEXTW(hwndDlg, ID_TB_KEYS_FUNCTION_LB, nSelItm, wstrItmTxt);

             if (((*pSpecCharKeyVec)[iInx].ucState || (*pSpecCharKeyVec)[iInx].ucCode) &&
                 ((*pSpecCharKeyVec)[iInx].ucState == ucState) &&
                 ((*pSpecCharKeyVec)[iInx].ucCode == ucCode)   &&
                 (wcsicmp(wstrVecStr, wstrItmTxt) != 0))
             {
                 wchar_t wstrKeyName[MAX_BUF_SIZE];
                 memset(wstrKeyName, 0x00, sizeof(wstrKeyName));
                 EQFBKeyNameW(wstrKeyName, ucCode, ucState);

                 wchar_t * wpszErrParms[2];

                 wpszErrParms[0] = wstrKeyName;
                 wpszErrParms[1] = wstrVecStr;
                 usMBCode = UtlErrorW(TB_KEY_IN_USE, MB_YESNOCANCEL, 2, wpszErrParms, EQF_QUERY, TRUE);
                 switch (usMBCode)
                 {
                 case MBID_YES:
                     // release key assignment
                     (*pSpecCharKeyVec)[iInx].ucState = NULC;
                     (*pSpecCharKeyVec)[iInx].ucCode  = NULC;
                     (*pSpecCharKeyVec)[iInx].fChange = TRUE;

                     // locate listbox item belonging to old function
                     sItem = SEARCHITEMW(hwndDlg, ID_TB_KEYS_FUNCTION_LB, wstrVecStr);
                     sItem = (SHORT)SendDlgItemMessageW(hwndDlg, ID_TB_KEYS_FUNCTION_LB, LB_FINDSTRINGEXACT, 0, (LPARAM) wstrVecStr);

                     // change listbox item text
                     if (sItem != LIT_NONE)
                     {
                         EQFBKeyNameW(wstrKeyName, (*pSpecCharKeyVec)[iInx].ucCode, (*pSpecCharKeyVec)[iInx].ucState);
                         RemoveKeyFromItem(wstrVecStr);
                         swprintf(wstrVecStr, L"%s\t%s", wstrVecStr, wstrKeyName);
                         ReplaceSpecCharW(iInx, wstrVecStr);
                         SETITEMTEXTW(hwndDlg, ID_TB_KEYS_FUNCTION_LB, sItem, (*pSpecCharKeyVec)[iInx].wstrDispChar);
                     }

                     break;
                 case MBID_NO:
                     fKey = FALSE;        // key is invalid
                     break;
                 default:                // case cancel
                     fKey = FALSE;
                     EQFBLeaveCaptMode(hwndDlg, pIda);
                     break;
                 } /*endswitch*/
             }
         }
         // Add end
      } /* endif */

      //--- assign the new key to active function ---
      if ( fKey )
      {
         if ( pIda->pKey )
         {
            // process key assignments for functions in table
            //--- assign the new key and set the change flag ---
            // Modify for R012027
            wchar_t wstrItemName[MAX_BUF_SIZE];
            memset(wstrItemName, 0x00, sizeof(wstrItemName));
            QUERYITEMTEXTW(hwndDlg, ID_TB_KEYS_FUNCTION_LB, pIda->sItem, wstrItemName);

            if (wcsnicmp(wstrItemName, STR_TITLE_INSERT_CHAR_W, wcslen(STR_TITLE_INSERT_CHAR_W)) != 0)
            {
                ucOldState = pIda->pKey->ucState;
                ucOldCode = pIda->pKey->ucCode;
                pIda->pKey->fChange = TRUE;
                pIda->pKey->ucCode  = ucCode;
                pIda->pKey->ucState = ucState;
                EQFBKeyName( szKeyName, pIda->pKey->ucCode, pIda->pKey->ucState );
                sprintf( pIda->szItemName, "%s\t%s",
                         (pIda->pFuncList+pIda->pKey->Function)->szDescription,
                         szKeyName );
                SETITEMTEXT( hwndDlg, ID_TB_KEYS_FUNCTION_LB,
                             pIda->sItem, pIda->szItemName  );
            }
            else
            {
                SPECCHARKEY * pSpecCharKey = QuerySpecChar(wstrItemName);

                if (NULL != pSpecCharKey)
                {
                    ucOldState = pSpecCharKey->ucState;
                    ucOldCode  = pSpecCharKey->ucCode;
                    pSpecCharKey->fChange = TRUE;
                    pSpecCharKey->ucCode  = ucCode;
                    pSpecCharKey->ucState = ucState;

                    wchar_t wstrKeyName[MAX_BUF_SIZE];
                    memset(wstrKeyName, 0x00, sizeof(wstrKeyName));
                    EQFBKeyNameW(wstrKeyName, ucCode, ucState);

                    RemoveKeyFromItem(wstrItemName);
                    swprintf(wstrItemName, L"%s\t%s", wstrItemName, wstrKeyName);
                    wcscpy(pSpecCharKey->wstrDispChar, wstrItemName);
                    SETITEMTEXTW(hwndDlg, ID_TB_KEYS_FUNCTION_LB, pIda->sItem, wstrItemName);
                }
                else
                {
                    ucOldState = NULL;
                    ucOldCode  = NULL;
                }
            }
            // Modify end

            if (fFoundInResKey && (usOldAssignStatus == ASSIGNABLE))
            {
              pResKey->usAssignStatus = ASSIGNED_TO_OTHER;
            } /* endif */
            //--- if old key was "ASSIGNED_TO_OTHER" in list of reserved keys,
            //--- reset it now to "ASSIGNABLE"
            //--- do it only if "new" key is not identical to old keysetting!
            if ((ucOldState != ucState) || (ucOldCode != ucCode) )
            {
              pResKey = pIda->pResKeys;
              fFoundInResKey = FALSE;
              while (!fFoundInResKey && (pResKey->ucCode || pResKey->ucState) )
              {
                if ((pResKey->ucState == ucOldState)
                      && ( pResKey->ucCode == ucOldCode)
                      && ( pResKey->usAssignStatus == ASSIGNED_TO_OTHER) )
                {
                  fFoundInResKey = TRUE;
                  pResKey->usAssignStatus = ASSIGNABLE;
                }
                else
                {
                  pResKey++;
                } /* endif */
              } /* endwhile */
            } /* endif */
         }
         else
         {
            // process key assignments for functions not yet in table

         } /* endif */

         //--- leave capture mode ---
         EQFBLeaveCaptMode( hwndDlg, pIda );
      } /* endif */
   }
   else
   {
      mResult = WinDefDlgProc( hwndDlg, WM_CHAR, mp1, mp2 );
   } /* endif */
   return( mResult );
} /* end of EQFBKeysChar */


/*////////////////////////////////////////////////////////////////////////////
:h2.EQFBKeysCommand - process WM_COMMAND messages of keys dialog
*/
// Description:
//    Handle WM_COMMAND messages (= pressing of pushbuttons) of
//    keys dialog panel.
//
//   Flow (message driven):
//      case 'Set' pushbutton:
//         update callers KEYSDATA structure with actual values;
//         post a WM_CLOSE message to dialog, mp1 = TRUE;
//      case 'Default' pushbutton:
//         reset key assignments to default values;
//      case 'Assign key' pushbutton:
//         display 'press key' text;
//         set fCapture flag in IDA;
//      case 'Clear key' pushbutton:
//         clear key assignment of selected function
//      case CANCEL pushbutton or DID_CANCEL (= ESCAPE key):
//         post a WM_CLOSE messgae to dialog, mp1 = FALSE;
//
// Arguments:
//   SHORT1FROMMP(mp1) = ID of control sending the WM_COMMAND message
//
// Returns:
//  MRESULT(TRUE)  command is processed
//
// Prereqs:
//   None
//
// SideEffects:
//   - callers KEYSDATA structure is updated if required
//
// External references:
//   None
//
//////////////////////////////////////////////////////////////////////////////
MRESULT EQFBKeysCommand
(
   HWND hwndDlg,
   WPARAM mp1,
   LPARAM mp2
)
{
   MRESULT mResult = MRFROMSHORT(TRUE);// TRUE = command is processed
   PKEYSIDA    pIda;                   // dialog instance data area ptr
   SHORT       sItem;                  // listbox item index
   HAB         hab;                    // anchor block handle
   CHAR        szKeyName[40];          // buffer for key names
   BYTE        bEditor = EDIT_STANDARD;

   mp2 = mp2;                          // get rid off compiler warning

   pIda = ACCESSDLGIDA(hwndDlg, PKEYSIDA);

   switch ( WMCOMMANDID( mp1, mp2 ) )
   {
	    case ID_TB_KEYS_HELP_PB:
	      mResult = UtlInvokeHelp();
	      break;
        case ID_TB_KEYS_KEY_TEXT:
          sItem = 0;
          break;

      case ID_TB_KEYS_ASSIGN_PB:
         SHOWCONTROL( hwndDlg, ID_TB_KEYS_KEY_TEXT );
         SETFOCUS( hwndDlg, ID_TB_KEYS_KEY_TEXT );
         pIda->fCapture = TRUE;
         hab = GETINSTANCE( hwndDlg );

         /*************************************************************/
         /* subclass the control                                      */
         /*************************************************************/
         lpOldProc =
            (WNDPROC) SetWindowLong( GetDlgItem( hwndDlg, ID_TB_KEYS_KEY_TEXT),
                                     GWL_WNDPROC,
                                     (LPARAM) (WNDPROC) EQFBKeysAssignSubProc );
         ENABLECTRL( hwndDlg, ID_TB_KEYS_FUNCTION_LB, FALSE );
         ENABLECTRL( hwndDlg, ID_TB_KEYS_SET_PB, FALSE );
         ENABLECTRL( hwndDlg, ID_TB_KEYS_CLEAR_PB, FALSE );
         ENABLECTRL( hwndDlg, ID_TB_KEYS_CANCEL_PB, FALSE );
         ENABLECTRL( hwndDlg, ID_TB_KEYS_DEFAULT_PB, FALSE );
         ENABLECTRL( hwndDlg, ID_TB_KEYS_ASSIGN_PB, FALSE );
         ENABLECTRL( hwndDlg, ID_HELP, FALSE );
         ENABLECTRL( hwndDlg, ID_TB_KEYS_PRINT_PB, FALSE );
         // Add for R012027
         ENABLECTRL( hwndDlg, ID_TB_KEYS_ADD_NEW_CHAR, FALSE );
         ENABLECTRL( hwndDlg, ID_TB_KEYS_EDIT_CHAR, FALSE );
         ENABLECTRL( hwndDlg, ID_TB_KEYS_REMOVE_CHAR, FALSE );
         // Add end
         break;
      case ID_TB_KEYS_CLEAR_PB:
          {
              SHORT nSelItm = QUERYSELECTION(hwndDlg, ID_TB_KEYS_FUNCTION_LB);
              wchar_t wstrItmTxt[MAX_SPEC_CHAR_STR_LEN];
              memset(wstrItmTxt, 0x00, sizeof(wstrItmTxt));
              QUERYITEMTEXTW(hwndDlg, ID_TB_KEYS_FUNCTION_LB, nSelItm, wstrItmTxt);

              if (wcsnicmp(wstrItmTxt, STR_TITLE_INSERT_CHAR_W, wcslen(STR_TITLE_INSERT_CHAR_W)) != 0)
              {
                  GETSELKEY( hwndDlg, pIda );
                  if ( pIda->pKey )
                  {
                      //--- clear key assignment ---
                      pIda->pKey->fChange = TRUE;
                      pIda->pKey->ucCode  = NULC;
                      pIda->pKey->ucState = NULC;
                      EQFBKeyName( szKeyName, pIda->pKey->ucCode, pIda->pKey->ucState );

                      sprintf( pIda->szItemName, "%s\t%s",
                          (pIda->pFuncList+pIda->pKey->Function)->szDescription,
                          szKeyName );
                      SETITEMTEXT( hwndDlg, ID_TB_KEYS_FUNCTION_LB,
                          pIda->sItem, pIda->szItemName );
                  } /* endif */
              }
              else
              {
                  wchar_t wstrNewItmTxt[MAX_SPEC_CHAR_STR_LEN];
                  memset(wstrNewItmTxt, 0x00, sizeof(wstrNewItmTxt));
                  int nItem = ClearSpecCharKey(wstrItmTxt, wstrNewItmTxt);

                  if (nItem > LB_ERR)
                  {
                      SETITEMTEXTW(hwndDlg, ID_TB_KEYS_FUNCTION_LB, nSelItm, wstrNewItmTxt);
                  }
              }

              SETFOCUS( hwndDlg, ID_TB_KEYS_FUNCTION_LB );
          }
         break;
      case ID_TB_KEYS_DEFAULT_PB:
         //--- reset assignments back to default values ---
          sItem = QUERYSELECTION( hwndDlg, ID_TB_KEYS_FUNCTION_LB );
          memcpy( pIda->pNewKeyList, get_DefKeyTable(), sizeof(KEYPROFTABLE) * (LAST_FUNC+1));
          ClearAllSpecCharKey();                      // Add for R012027
          bEditor = (pIda->pDoc->hwndRichEdit) ? EDIT_RTF : EDIT_STANDARD;
          EQFBKeysFillListbox( hwndDlg, ID_TB_KEYS_FUNCTION_LB,
              pIda->pNewKeyList, pIda->pFuncList,
              bEditor);
          SELECTITEM( hwndDlg, ID_TB_KEYS_FUNCTION_LB, sItem );
          SETFOCUS( hwndDlg, ID_TB_KEYS_FUNCTION_LB );
          break;
      case ID_TB_KEYS_SET_PB:
         memcpy( pIda->pOrgKeyList,
                 pIda->pNewKeyList,
                 sizeof(KEYPROFTABLE) * (LAST_FUNC+1));
         BackupSpecCharKeyVec();                // Add for R012027
         POSTEQFCLOSE( hwndDlg, TRUE );
         break;
      case ID_TB_KEYS_PRINT_PB:
         EQFBKeysPrint( GETHANDLEFROMID( hwndDlg, ID_TB_KEYS_FUNCTION_LB ) );
         break;
      case ID_TB_KEYS_CANCEL_PB:
      case DID_CANCEL:
          RestoreSpecCharKeyVec();                // Add for R012027
          POSTEQFCLOSE( hwndDlg, FALSE );
          break;

      // Add for R012027
      case ID_TB_KEYS_ADD_NEW_CHAR:
          {
              SpecialCharDlg * pSpecialCharDlg;
              pSpecialCharDlg = new SpecialCharDlg();
              int nRes = pSpecialCharDlg->SpecialCharDlgOpen(hwndDlg, MODE_NEW);
              if (IDOK != nRes)
              {
                  break;
              }

              wchar_t wstrChar[MAX_SPEC_CHAR_SIZE];
              memset(wstrChar, 0x00, sizeof(wstrChar));
              nRes = pSpecialCharDlg->GetSpcialChar(wstrChar);
              if (NO_ERROR != nRes)
              {
                  break;
              }

              wchar_t wstrUnicode[MAX_BUF_SIZE];
              memset(wstrUnicode, 0x00, sizeof(wstrUnicode));
              nRes = pSpecialCharDlg->GetCharUnicode(wstrUnicode);
              if (NO_ERROR != nRes)
              {
                  break;
              }

              wchar_t wstrName[MAX_BUF_SIZE];
              memset(wstrName, 0x00, sizeof(wstrName));
              nRes = pSpecialCharDlg->GetCharName(wstrName);
              if (NO_ERROR != nRes)
              {
                  break;
              }

              wchar_t wstrKey[MAX_BUF_SIZE];
              memset(wstrKey, 0x00, sizeof(wstrKey));
              nRes = pSpecialCharDlg->GetCharKey(wstrKey);
              if (NO_ERROR != nRes)
              {
                  break;
              }

              wchar_t wstrDispChar[MAX_BUF_SIZE];
              memset(wstrDispChar, 0x00, sizeof(wstrDispChar));
              swprintf(wstrDispChar, L"%s %s %s %s\t%s", STR_TITLE_INSERT_CHAR_W, wstrChar, wstrUnicode, wstrName, wstrKey);

              BOOL bDuplicate = CheckDupCharW(wstrDispChar, hwndDlg);
              if (bDuplicate)
              {
                  MessageBoxA(hwndDlg, STR_ERR_DUP_CHAR, STR_SPEC_CHAR_DLG_TITLE, MB_OK | MB_ICONEXCLAMATION);
                  break;
              }

              int nItem = AddToSpecCharVec(0, 0, FALSE, EDIT_STANDARD | EDIT_RTF, wstrDispChar);
              SPECCHARKEYVEC* pSpecCharKeyVec = GetSpecCharKeyVec();
              SHORT sItem = INSERTITEMW(hwndDlg, ID_TB_KEYS_FUNCTION_LB, wstrDispChar);

              if ((sItem != LIT_NONE) && (nItem >= 0))
              {
                  SETITEMHANDLEW(hwndDlg, ID_TB_KEYS_FUNCTION_LB, sItem, &(*pSpecCharKeyVec)[nItem]);
                  SELECTITEM(hwndDlg, ID_TB_KEYS_FUNCTION_LB, sItem);
              }
          }
          break;

      case ID_TB_KEYS_EDIT_CHAR:
          {
              SpecialCharDlg * pSpecialCharDlg;
              pSpecialCharDlg = new SpecialCharDlg();
              GETSELKEY(hwndDlg, pIda);
              if (pIda->sItem == LIT_NONE)
              {
                  break;
              }

              wchar_t wstrItmTxt[MAX_BUF_SIZE];
              memset(wstrItmTxt, 0x00, sizeof(wstrItmTxt));
              QUERYITEMTEXTW(hwndDlg, ID_TB_KEYS_FUNCTION_LB, pIda->sItem, wstrItmTxt);
              int nItem = pSpecialCharDlg->SetSpcialCharStr(wstrItmTxt);
              if (LB_ERR == nItem)
              {
                  break;
              }

              // open the dialog
              int nRes = pSpecialCharDlg->SpecialCharDlgOpen(hwndDlg, MODE_EDIT);
              if (IDOK != nRes)
              {
                  break;
              }

              wchar_t wstrChar[MAX_SPEC_CHAR_SIZE];
              memset(wstrChar, 0x00, sizeof(wstrChar));
              nRes = pSpecialCharDlg->GetSpcialChar(wstrChar);
              if (NO_ERROR != nRes)
              {
                  break;
              }

              wchar_t wstrUnicode[MAX_BUF_SIZE];
              memset(wstrUnicode, 0x00, sizeof(wstrUnicode));
              nRes = pSpecialCharDlg->GetCharUnicode(wstrUnicode);
              if (NO_ERROR != nRes)
              {
                  break;
              }

              wchar_t wstrName[MAX_BUF_SIZE];
              memset(wstrName, 0x00, sizeof(wstrName));
              nRes = pSpecialCharDlg->GetCharName(wstrName);
              if (NO_ERROR != nRes)
              {
                  break;
              }

              wchar_t wstrKey[MAX_BUF_SIZE];
              memset(wstrKey, 0x00, sizeof(wstrKey));
              nRes = pSpecialCharDlg->GetCharKey(wstrKey);
              if (NO_ERROR != nRes)
              {
                  break;
              }

              wchar_t wstrDispChar[MAX_BUF_SIZE];
              memset(wstrDispChar, 0x00, sizeof(wstrDispChar));
              swprintf(wstrDispChar, L"%s %s %s %s\t%s", STR_TITLE_INSERT_CHAR_W, wstrChar, wstrUnicode, wstrName, wstrKey);

              ReplaceSpecCharW(nItem, wstrDispChar);

              SETITEMTEXTW(hwndDlg, ID_TB_KEYS_FUNCTION_LB, pIda->sItem, wstrDispChar);
              SELECTITEMMS(hwndDlg, ID_TB_KEYS_FUNCTION_LB, pIda->sItem);
          }
          break;

      case ID_TB_KEYS_REMOVE_CHAR:
          {
              GETSELKEY(hwndDlg, pIda);
              pIda->pKey = NULL;
              if (pIda->sItem != LIT_NONE)
              {
                  int nID = MessageBoxA(hwndDlg, STR_WARN_REMOVE_CONFIRM, STR_SPEC_CHAR_DLG_TITLE_D, MB_YESNO | MB_ICONEXCLAMATION);
                  if (nID != IDYES)
                  {
                      break;
                  }
                  wchar_t wstrItmTxt[MAX_BUF_SIZE];
                  memset(wstrItmTxt, 0x00, sizeof(wstrItmTxt));
                  QUERYITEMTEXTW(hwndDlg, ID_TB_KEYS_FUNCTION_LB, pIda->sItem, wstrItmTxt);
                  DELETEITEMW(hwndDlg, ID_TB_KEYS_FUNCTION_LB, pIda->sItem);
                  RemoveSpecCharFromVecW(wstrItmTxt);
              }
          }
          break;

       default:
         mResult = WinDefDlgProc( hwndDlg, WM_COMMAND, mp1, mp2 );
         break;
   } /* endswitch */

   switch (WMCOMMANDCMD( mp1, mp2 ))
   {
       // Add for R012027 start
   case LBN_SELCHANGE:
       {
           SHORT nSelItm = QUERYSELECTION(hwndDlg, ID_TB_KEYS_FUNCTION_LB);
           char strItmTxt[MAX_BUF_SIZE];
           memset(strItmTxt, 0x00, sizeof(strItmTxt));
           QUERYITEMTEXT(hwndDlg, ID_TB_KEYS_FUNCTION_LB, nSelItm, strItmTxt);

           if (strnicmp(strItmTxt, STR_INSERT_CHAR, strlen(STR_INSERT_CHAR)) == 0)
           {
               ENABLECTRL(hwndDlg, ID_TB_KEYS_EDIT_CHAR, TRUE);
               ENABLECTRL(hwndDlg, ID_TB_KEYS_REMOVE_CHAR, TRUE);
           }
           else
           {
               ENABLECTRL(hwndDlg, ID_TB_KEYS_EDIT_CHAR, FALSE);
               ENABLECTRL(hwndDlg, ID_TB_KEYS_REMOVE_CHAR, FALSE);
           }
       }
       break;
       // Add end
   }

   return( mResult );
} /* end of EQFBKeysCommand */

/*////////////////////////////////////////////////////////////////////////////
:h2.EQFBKeysClose - process WM_CLOSE messages of keys dialog
*/
// Description:
//    Handle WM_CLOSE messages (= dialog termination requests) of
//    keys dialog panel.
//
//   Flow:
//      free allocated storage
//      dismiss dialog
//
// Arguments:
//   SHORT1FROMMP(mp1) = flag beeing returned using WinDismissDlg
//
// Returns:
//  MRESULT(FALSE)
//
// Prereqs:
//   None
//
// SideEffects:
//   - dialog is removed
//   - caller receives flag specifed in mp1
//
// External references:
//   UtlAlloc
//
//////////////////////////////////////////////////////////////////////////////
MRESULT EQFBKeysClose
(
   HWND hwndDlg,
   WPARAM mp1,
   LPARAM mp2
)
{
   MRESULT mResult = FALSE;
   PKEYSIDA    pIda;

   mp1 = mp1;                          // supress 'unreferenced parameter' msg
   mp2 = mp2;                          // supress 'unreferenced parameter' msg

   //-- free allocated storage ---
   pIda = ACCESSDLGIDA(hwndDlg, PKEYSIDA);
   EQFBLeaveCaptMode( hwndDlg, pIda );
   UtlAlloc((PVOID *) &pIda->pNewKeyList, 0L, 0L, NOMSG );
   UtlAlloc((PVOID *) &pIda, 0L, 0L, NOMSG );

   //--- get rid off dialog ---
   DISMISSDLG( hwndDlg, TRUE );

   return( mResult );
} /* end of EQFBKeysClose */

/*////////////////////////////////////////////////////////////////////////////
:h2.EQFBKeysFillListbox - fill function listbox
*/
// Description:
//    Handle WM_CLOSE messages (= dialog termination requests) of
//    keys dialog panel.
//
//   Flow:
//      disable listbox update;
//      delete all listbox items;
//      for all functions in key assignment table
//         get function names
//         concatenate function name and name of assigned key;
//         insert item into listbox;
//         store pointer to key assignment table entry in item handle;
//      endfor;
//      enable listbox update;
//
// Arguments:
//   HWND          hwndLB - handle of the dialog
//   USHORT        usId   - id of the control
//   PKEYPROFTABLE pKey   - pointer to key assignment table
//   PFUNCTIONTABLE pFunc - pointer to function list
//
// Returns:
//   nothing
//
// Prereqs:
//   None
//
// SideEffects:
//   None
//
// External references:
//   EQFBKeyName
//
//////////////////////////////////////////////////////////////////////////////
VOID EQFBKeysFillListbox
(
  HWND hwndDlg,
  USHORT  usId,
  PKEYPROFTABLE pKey,
  PFUNCTIONTABLE  pFuncList,           // function list
  BYTE            bEditor
)
{
   SHORT       sItem;                  // listbox item index
   PSZ         pszFuncName;            // pointer to function name
   HAB         hab;                    // anchor block handle
   PKEYPROFTABLE pKeyStart = pKey;     // start of key table
   PKEYPROFTABLE pKeyTemp;             // temp pointer
   CHAR        szItemName[256];        // buffer for item names
   USHORT      usFuncValid = 0;
   HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);

   ENABLEUPDATE_FALSE( hwndDlg, usId );
   DELETEALL( hwndDlg, usId );


   pKeyTemp = pKey;
   pszFuncName = (pFuncList + pKey->Function)->szDescription;
   // check if resource already loaded, if not load it
   if ( ! *pszFuncName )
   {
      pKeyTemp = pKey;
      hab = GETINSTANCE( hwndDlg );
      while ( pKeyTemp->Function != LAST_FUNC )    // while not end of key list
      {
         // load name for function types
         LOADSTRING(hab,hResMod,ID_TB_KEYS_DLG+pKeyTemp->Function,
                    (pFuncList + pKeyTemp->Function)->szDescription );

         pKeyTemp++;
      }
   } /* endif */

   while ( pKey->Function != LAST_FUNC )        // while not end of key list
   {
      pszFuncName = (pFuncList + pKey->Function)->szDescription;
      if (bEditor & pKey->bEditor )
      {
        usFuncValid = 1;
      }
      else
      {
        usFuncValid = 0;
      } /* endif */
//    if ( *pszFuncName )              // if not empty ...
      if ( (*pszFuncName) && usFuncValid)
      {
         sprintf( szItemName, "%s\t", pszFuncName );
         EQFBKeyName( szItemName + strlen(szItemName),
                      pKey->ucCode,
                      pKey->ucState );

         // GQ:  only insert if no entry for this function is in listbox (e.g. due to corrupted key lists)
         {
           pKeyTemp = pKeyStart;
           while ( (pKeyTemp < pKey) && (pKey->Function != pKeyTemp->Function) )
           {
             pKeyTemp++;
           } /*endwhile */

           if ( pKeyTemp == pKey )
           {
            sItem = INSERTITEM( hwndDlg, usId, szItemName );
            if ( sItem != LIT_NONE )
            {
              SETITEMHANDLE( hwndDlg, usId, sItem, pKey );
            } /* endif */
           } /* endif */
         }

      } /* endif */
      pKey->fChange = FALSE;
      pKey++;
   } /* endwhile */

   // Add for R012027
   SPECCHARKEYVEC* pSpecCharKeyVec = GetSpecCharKeyVec();

   for (size_t iInx = 0; iInx < (*pSpecCharKeyVec).size(); iInx++)
   {
       if (wcsnicmp((*pSpecCharKeyVec)[iInx].wstrDispChar, STR_TITLE_INSERT_CHAR_W, wcslen(STR_TITLE_INSERT_CHAR_W)) != 0)
       {
           continue;
       }
       wchar_t wstrItemStr[MAX_BUF_SIZE];
       memset(wstrItemStr, 0x00, sizeof(wstrItemStr));
       wcscpy(wstrItemStr, (*pSpecCharKeyVec)[iInx].wstrDispChar);
       sItem = INSERTITEMW(hwndDlg, usId, wstrItemStr);
       if (sItem != LB_ERR)
       {
           SETITEMHANDLEW(hwndDlg, usId, sItem, &(*pSpecCharKeyVec)[iInx]);
       }
   }
   // Add end

   ENABLEUPDATE_TRUE( hwndDlg, usId);
} /* end of EQFBKeyFillListbox */

/*////////////////////////////////////////////////////////////////////////////
:h2.EQFBOpenDlgProc - dialog procedure for open dialog
*/
// Description:
//    Allows the selection of a Troja document or a text file for open.
//
//   Flow (message driven):
//       case WM_INITDLG:
//         call EQFBOpenInit to initialize the dialog controls;
//       case WM_CONTROL:
//         call EQFBOpenControl to handle user interaction;
//       case WM_CHAR:
//         call EQFBOpenChar to handle key pressed by user:
//       case WM_COMMAND
//         call EQFBOpenCommand to handle user commands;
//       case WM_CLOSE
//         call EQFBOpenClose to end the dialog;
//
// Arguments:
//  mp2 of WM_INITDLG msg = POPENDATA pOpenData ptr to data structure containing
//                          last used values.
//
// Returns:
//  USHORT usRC      FALSE          - dialog was cancelled or closed
//                   TRUE           - file has been selected, OPENDATA contains
//                                    file type and name
//
// Prereqs:
//   None
//
// SideEffects:
//   if usRC = TRUE is returned, the OPENDATA structure contains the selections
//   made by the user
//
// External references:
//   None
//
//////////////////////////////////////////////////////////////////////////////
INT_PTR CALLBACK EQFBOPENDLGPROC
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
      case WM_EQF_QUERYID: HANDLEQUERYID( ID_TB_OPEN_DLG, mp2 ); break;

      case WM_INITDLG:
          SETWINDOWID( hwndDlg, ID_TB_OPEN_DLG );
          mResult = DIALOGINITRETURN( EQFBOpenInit( hwndDlg, mp1, mp2 ));
          break;
       case WM_COMMAND:
         mResult = EQFBOpenCommand( hwndDlg, mp1, mp2 );
         break;

       case DM_GETDEFID:
         mResult = EQFBOpenChar( hwndDlg, mp1, mp2 );
         break;

      case WM_EQF_CLOSE:
         mResult = EQFBOpenClose( hwndDlg, mp1, mp2 );
         break;

      default:
         mResult = WinDefDlgProc( hwndDlg, msg, mp1, mp2 );
         break;
   } /* endswitch */

   return mResult;
} /* end of EQFBOpenDlgProc */

/*////////////////////////////////////////////////////////////////////////////
:h2.EQFBOpenInit - initialization for open dialog
*/
// Description:
//    Initialize all dialog controls and allocate required memory.
//
//   Flow:
//      allocate and anchor dialog IDA;
//      get lists of available and EQF drives;
//      create and position drive buttons;
//      use last used values to set dialog controls;
//
// Arguments:
//  mp2 of WM_INITDLG msg = POPENDATA pOpenData ptr to data structure containing
//                          last used values.
//
// Returns:
//  MRESULT(FALSE)
//
// Prereqs:
//   None
//
// SideEffects:
//   - ptr to IDA is stored in dialog word QWL_USER
//   - IDA contains list of available drives and EQF drives
//
// External references:
//   UtlAlloc, UtlFileExist, UtlCreateDriveButtons, UtlSetPosDriveButtons2,
//   UtlGetDriveList, UtlLoadFileNames, UtlError
//
//////////////////////////////////////////////////////////////////////////////
MRESULT EQFBOpenInit
(
   HWND    hwndDlg,                    // handle of dialog window
   WPARAM  mp1,                        // first parameter of WM_INITDLG
   LPARAM  mp2                         // second parameter of WM_INITDLG
)
{
   MRESULT     mResult = MRFROMSHORT(TRUE); // result of message processing
   HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
   USHORT      usButtonID;             // ID of button to be checked
   HWND        hwndLB;                 // handle of listbox
   POPENIDA    pIda;                   // ptr to dialog IDA
   PSZ         pszTemp;                // temporary string pointer
   SHORT       sItem;                  // listbox item index
   HAB         hab;                    // anchor block
   mp1 = mp1;                          // suppress 'unreferenced parameter' msg

   //--- create and anchor IDA ---
   UtlAlloc((PVOID *) &pIda, 0L, (LONG) sizeof(OPENIDA), ERROR_STORAGE );
   if ( pIda)
   {
      ANCHORDLGIDA( hwndDlg, pIda );
   } /* endif */


   //--- remember address of OPENDATA structure ---
   if ( pIda )
   {
      pIda->pOpenData = (POPENDATA) mp2;
   } /* endif */

   // load strings from resource
   if ( pIda )
   {
      hab = GETINSTANCE( hwndDlg );
      // Name:
      LOADSTRING( hab, hResMod, IDS_TB_OPEN_NAME, pIda->szName );
      // Documents Name:
      LOADSTRING( hab, hResMod, IDS_TB_OPEN_DOCNAME, pIda->szDocName );
      // Folders
      LOADSTRING( hab, hResMod, IDS_TB_OPEN_FOLDERS, pIda->szFolderText );
      // Documents
      LOADSTRING( hab, hResMod, IDS_TB_OPEN_DOCUMENTS, pIda->szDocuments );
      // Directories
      LOADSTRING( hab, hResMod, IDS_TB_OPEN_DIRECTORIES, pIda->szDirectoryText );
      // Files
      LOADSTRING( hab, hResMod, IDS_TB_OPEN_FILES, pIda->szFiles );
   } /* endif */

   //--- get lists of available and Eqf drives, get directory names ---
   if ( pIda )
   {
	  pSysProp = GetSystemPropPtr();
      UtlGetDriveList( (BYTE *) pIda->szDrives );
      if ( pSysProp )
      {
         strcpy( pIda->szEqfDrives, pSysProp->szDriveList );
         pIda->chPrimDrive = pSysProp->szPrimaryDrive[0];
      }
      else
      {
         pIda->szEqfDrives[0] = EOS;
         pIda->chPrimDrive = ' ';
      } /* endif */

      //--- create and position drive buttons ---
      UtlDriveButtons( hwndDlg, pIda->szDrives,
                       ID_FROMDRIVEA_BTN,
                       WS_GROUP | WS_TABSTOP,
                       WS_VISIBLE,
                       GETHANDLEFROMID(hwndDlg,ID_TB_OPEN_DRIVE_GB),
                       GETHANDLEFROMID(hwndDlg,ID_TB_OPEN_DUMMYDRIVE_PB),
                       NULLHANDLE );


      //--- get and prepare data from OPENDATA structure ---
      pIda->fTroja = pIda->pOpenData->fTroja;

      strcpy( pIda->szFilePattern, pIda->pOpenData->szFilePattern );
      SETTEXT( hwndDlg, ID_TB_OPEN_FILNAME_EF, pIda->szFilePattern );

      strcpy( pIda->szDocPattern, pIda->pOpenData->szDocPattern );
      SETTEXT( hwndDlg, ID_TB_OPEN_DOCNAME_EF, pIda->szDocPattern );

       SetCtrlFnt (hwndDlg, GetCharSet(),
                    ID_TB_OPEN_FILNAME_EF, ID_TB_OPEN_DOCNAME_EF );
           SetCtrlFnt (hwndDlg, GetCharSet(),
                    ID_TB_OPEN_DIR_LB, ID_TB_OPEN_FOLDER_LB );
       SetCtrlFnt (hwndDlg, GetCharSet(),
                    ID_TB_OPEN_FILE_LB, ID_TB_OPEN_DOC_LB );

      if ( pIda->pOpenData->szFileName[0] == EOS )
      {
         // no file name given; set default values
         pIda->chDrive = 'C';
         pIda->szDirectory[0] = EOS;
         pIda->szFile[0] = EOS;
      }
      else
      {
         // use given file name as last used value
         pIda->chDrive = pIda->pOpenData->szFileName[0];
         strcpy( pIda->szDirectory, pIda->pOpenData->szFileName + 2 );
         pszTemp = UtlSplitFnameFromPath( pIda->szDirectory );
         strcpy( pIda->szFile, ( pszTemp) ? pszTemp : EMPTY_STRING );
      } /* endif */

      //--- activate last used values ---
      SETDRIVE( hwndDlg, IDFROMDRIVE( ID_FROMDRIVEA_BTN, pIda->chDrive ), TRUE );
      EQFBOpenFillListboxes( hwndDlg, pIda );
      SELECTITEM( hwndDlg, ID_TB_OPEN_DIR_LB, 0 );
      sItem = SEARCHITEM( hwndDlg, ID_TB_OPEN_FILE_LB, pIda->szFile );
      if ( sItem != LIT_NONE )
      {
         // select last used directory
         SELECTITEM( hwndDlg, ID_TB_OPEN_FILE_LB, sItem );
      }
      else
      {
         // select first item
         SELECTITEM( hwndDlg, ID_TB_OPEN_FILE_LB, 0 );
      } /* endif */

      //--- fill folder listbox ---
      DELETEALL( hwndDlg, ID_TB_OPEN_FOLDER_LB );
      hwndLB = GETHANDLEFROMID( hwndDlg, ID_TB_OPEN_FOLDER_LB );

     EqfSend2Handler( FOLDERLISTHANDLER, WM_EQF_INSERTNAMES,
                      MP1FROMHWND( hwndLB ), 0L );

      // set default values
      pIda->szFolder[0] = EOS;
      pIda->szDocument[0] = EOS;
      if ( pIda->pOpenData->szDocName[0] )
      {
         // use given document name as last used value
         strcpy( pIda->szPath, pIda->pOpenData->szDocName + 3 );
         pszTemp = strchr( pIda->szPath, BACKSLASH );
         if ( pszTemp )
         {
            pszTemp = strchr( pIda->szPath, BACKSLASH );
            if ( pszTemp )
            {
               Utlstrccpy( pIda->szFolder, pszTemp + 1, '.' );
            } /* endif */
         } /* endif */
         pszTemp = UtlGetFnameFromPath( pIda->pOpenData->szDocName );
         if ( pszTemp )
         {
            strcpy( pIda->szDocument, pszTemp );
         } /* endif */
      } /* endif */
   } /* endif */

   //--- activate last used values ---
   if ( pIda )
   {
      OEMTOANSI(pIda->szFolder);
      sItem = SEARCHITEM( hwndDlg, ID_TB_OPEN_FOLDER_LB, pIda->szFolder );
      ANSITOOEM(pIda->szFolder);
      if ( sItem != LIT_NONE )
      {
         // search last used document
         SELECTITEM( hwndDlg, ID_TB_OPEN_FOLDER_LB, sItem );
         sItem = SEARCHITEM( hwndDlg, ID_TB_OPEN_DOC_LB,
                               pIda->szDocument );

         // select last used document or first one if last used not found
         if ( sItem != 0)
         {
            SELECTITEM( hwndDlg, ID_TB_OPEN_DOC_LB, sItem );
         }
         else
         {
            SELECTITEM( hwndDlg, ID_TB_OPEN_DOC_LB, 0 );
         } /* endif */
      }
      else
      {
         SELECTITEM( hwndDlg, ID_TB_OPEN_FOLDER_LB, 0 );
      } /* endif */

      // set 'troja document' or 'text file' radio button (this will initialize
      // all other controls of the dialog panel)
      if ( pSysProp )
      {
         usButtonID = (pIda->pOpenData->fTroja ) ? ID_TB_OPEN_DOC_RB :
                                                   ID_TB_OPEN_FILE_RB;
      }
      else
      {
         ENABLECTRL( hwndDlg, ID_TB_OPEN_DOC_RB, FALSE );
         usButtonID = ID_TB_OPEN_FILE_RB;
      } /* endif */

      pIda->fInit = TRUE;                        // set initialization flag
      CLICK( hwndDlg, usButtonID );
      pIda->fInit = FALSE;                // clear initialization flag

   } /* endif */
   return ( mResult );
} /* end of EQFBOpenInit */

/*////////////////////////////////////////////////////////////////////////////
:h2.EQFBOpenControl - process WM_CONTROL messages of open dialog
*/
// Description:
//    Handle WM_CONTROL messages (= radio button and listbox selections) of
//    open dialog panel.
//
//   Flow (message driven):
//      case 'Troja document' radio button:
//         set troja flag in IDA;
//         change static text controls to document mode;
//         enable/set  source and target radio button;
//         disable drive icons and select all EQF drives;
//         hide current path static control;
//         hide directory and file listbox, hide filename entry field;
//         show folder and document listbox, show docname entry field;
//      case 'Text file' radio button:
//         clear troja flag in IDA;
//         change static text controls to file mode;
//         disable/clear  source and target radio button;
//         enable drive icons and POST a WM_COMMAND for last used drive button;
//         show and fill current path static control;
//         hide folder and document listbox, hide docname entry field;
//         show directory and file listbox, show filename entry field;
//      case 'Source' radio button:
//         set source flag in IDA;
//         POST a WM_CONTROL for dir/folder listbox to refresh file listbox;
//      case 'Target' radio button:
//         clear source flag in IDA;
//         POST a WM_CONTROL for dir/folder listbox to refresh file listbox;
//      case LN_SELECT in dir/folder listbox:
//         if troja flag is set
//            fill file listbox with documents of folder
//         endif;
//      case LN_ENTER in dir/folder listbox:
//         if troja flag is cleared
//            if directory selected is '..'
//               remove last directory from current path
//            else
//               add directory to current path
//            endif;
//            refresh directory and file listbox
//         endif;
//
// Arguments:
//   sId           = ID of control sending the WM_CONTROL message
//   sNotification = notification code
//
// Returns:
//  MRESULT(FALSE)
//
// Prereqs:
//   None
//
// SideEffects:
//   - values in IDA are updated
//
// External references:
//   None
//
//////////////////////////////////////////////////////////////////////////////
MRESULT EQFBOpenControl
(
   HWND   hwndDlg,                     // dialog handle
   SHORT  sId,                         // id in action
   SHORT  sNotification                // notification
)
{
   MRESULT     mResult = FALSE;        // result of message processing
   PCHAR       pchDrive;               // ptr for drive list processing
   POPENIDA    pIda;                   // ptr to dialog IDA
   SHORT       sItem;                  // listbox item index
   PSZ         pszTemp;                // temporary pointer
   HWND        hwndLB;                 // listbox window handle
   ULONG       ulBytesRead;            // no of bytes read by UtlLoadFile
   BOOL        fOK = TRUE;             // internal OK flag
   USHORT      usNoOfItems;            // no of items in a listbox


   pIda = ACCESSDLGIDA(hwndDlg, POPENIDA);

   switch ( sId )
   {
      case ID_TB_OPEN_DOC_RB:
         //--- set open mode flag ---
         pIda->fTroja = TRUE;

         //--- change static text controls ---
         SETTEXT( hwndDlg, ID_TB_OPEN_FNAME_TEXT, pIda->szName );
         SETTEXT( hwndDlg, ID_TB_OPEN_DIR_TEXT, pIda->szFolderText );
         SETTEXT( hwndDlg, ID_TB_OPEN_FILE_TEXT, pIda->szDocuments );

         //--- de-activate controls for file open ---
         HIDECONTROL( hwndDlg, ID_TB_OPEN_FILNAME_EF );
         HIDECONTROL( hwndDlg, ID_TB_OPEN_DIR_LB );
         HIDECONTROL( hwndDlg, ID_TB_OPEN_FILE_LB );

         //--- activate controls for document open ---
         SHOWCONTROL( hwndDlg, ID_TB_OPEN_DOCNAME_EF );
         SHOWCONTROL( hwndDlg, ID_TB_OPEN_FOLDER_LB );
         SHOWCONTROL( hwndDlg, ID_TB_OPEN_DOC_LB );

         //--- disable drive icons and hilite EQF system drives ---
         pchDrive = pIda->szDrives;
         while ( *pchDrive )
         {
            ENABLECTRL( hwndDlg,
                           IDFROMDRIVE( ID_FROMDRIVEA_BTN, *pchDrive ),
                           FALSE );
            if ( strchr( pIda->szEqfDrives, *pchDrive ) )
            {
               SETDRIVE( hwndDlg,
                         IDFROMDRIVE( ID_FROMDRIVEA_BTN, *pchDrive ),
                         TRUE );
            }
            else
            {
               SETDRIVE( hwndDlg,
                         IDFROMDRIVE( ID_FROMDRIVEA_BTN, *pchDrive ),
                         FALSE );
            } /* endif */
            pchDrive++;
         } /* endwhile */

         break;

      case ID_TB_OPEN_FILE_RB:
         //--- set open mode flag ---
         pIda->fTroja = FALSE;

         //--- change static text controls ---
         SETTEXT( hwndDlg, ID_TB_OPEN_FNAME_TEXT, pIda->szName );
         SETTEXT( hwndDlg, ID_TB_OPEN_DIR_TEXT, pIda->szDirectoryText );
         SETTEXT( hwndDlg, ID_TB_OPEN_FILE_TEXT, pIda->szFiles );

         //--- disable source and target buttons and uncheck them ---
         ENABLECTRL( hwndDlg, ID_TB_OPEN_SOURCE_RB, FALSE );
         ENABLECTRL( hwndDlg, ID_TB_OPEN_TARGET_RB, FALSE );
         SETCHECK_FALSE( hwndDlg, ID_TB_OPEN_TARGET_RB );
         SETCHECK_FALSE( hwndDlg, ID_TB_OPEN_SOURCE_RB );

         //--- de-activate controls for document open ---
         HIDECONTROL( hwndDlg, ID_TB_OPEN_DOCNAME_EF );
         HIDECONTROL( hwndDlg, ID_TB_OPEN_FOLDER_LB );
         HIDECONTROL( hwndDlg, ID_TB_OPEN_DOC_LB );

         //--- activate controls for file open ---
         SHOWCONTROL( hwndDlg, ID_TB_OPEN_FILNAME_EF );
         SHOWCONTROL( hwndDlg, ID_TB_OPEN_DIR_LB );
         SHOWCONTROL( hwndDlg, ID_TB_OPEN_FILE_LB );

         //--- enable drive icons and hilite last used drive ---
         pchDrive = pIda->szDrives;
         while ( *pchDrive )
         {
            ENABLECTRL( hwndDlg,
                           IDFROMDRIVE( ID_FROMDRIVEA_BTN, *pchDrive ),
                           TRUE );
               SETDRIVE( hwndDlg,
                         IDFROMDRIVE( ID_FROMDRIVEA_BTN, *pchDrive ),
                         FALSE );
            pchDrive++;
         } /* endwhile */
         /*************************************************************/
         /* select the currently selected drive                       */
         /*************************************************************/
         SETDRIVE( hwndDlg,
                   IDFROMDRIVE( ID_FROMDRIVEA_BTN, pIda->chDrive ),
                   TRUE  );


         break;

      case ID_TB_OPEN_DIR_LB:
         switch ( sNotification )
         {
            case LN_ENTER:
                  // activate directory
                  sItem = QUERYSELECTION( hwndDlg, ID_TB_OPEN_DIR_LB );
                  if ( sItem != LIT_NONE )
                  {
                     QUERYITEMTEXT( hwndDlg, ID_TB_OPEN_DIR_LB, sItem,
                                    pIda->szPath );
                     if ( strcmp( pIda->szPath, PARENT_DIR_NAME ) == 0 )
                     {
                        pszTemp = strrchr( pIda->szDirectory, BACKSLASH );
                        if ( pszTemp && (pszTemp != pIda->szDirectory) )
                        {
                           *pszTemp = EOS;
                        }
                        else
                        {
                           pIda->szDirectory[0] = EOS;
                        } /* endif */
                     }
                     else
                     {
                        strcat( pIda->szDirectory, "\\" );
                        strcat( pIda->szDirectory, pIda->szPath );
                     } /* endif */

                     EQFBOpenFillListboxes( hwndDlg, pIda );
                  } /* endif */
               break;
         } /* endswitch */
         break;

      case ID_TB_OPEN_FOLDER_LB:
         switch ( sNotification )
         {
            case LN_SELECT:
               {
                 CHAR szShortName[MAX_FILESPEC];
                 // prepare refresh of document listbox
                 ENABLEUPDATE_FALSE( hwndDlg, ID_TB_OPEN_DOC_LB );
                 DELETEALL( hwndDlg, ID_TB_OPEN_DOC_LB );

                 // get folder properties
                 sItem = QUERYSELECTION( hwndDlg, ID_TB_OPEN_FOLDER_LB );
                 if ( sItem != LIT_NONE )
                 {
                    BOOL fIsNew;

                    QUERYITEMTEXT( hwndDlg, ID_TB_OPEN_FOLDER_LB, sItem,
                                   pIda->szPath );
                    ANSITOOEM( pIda->szPath );
                    ObjLongToShortName( pIda->szPath, szShortName, FOLDER_OBJECT, &fIsNew );
                    sprintf( pIda->szFolder, "%s\\%s\\%s%s",
                             pSysProp->PropHead.szPath,
                             pSysProp->szPropertyPath,
                             szShortName,
                             EXT_FOLDER_MAIN );

                    // free space for folder properties previously allocated
                    if ( pIda->pFolProp )
                    {
                       UtlAlloc((PVOID *) &pIda->pFolProp, 0L, 0L, NOMSG );
                    } /* endif */

                    fOK = UtlLoadFileL( pIda->szFolder,
                                       (PVOID *)&pIda->pFolProp,
                                       &ulBytesRead,
                                       FALSE,
                                       TRUE );

                 }
                 else
                 {
                    fOK = FALSE;
                 } /* endif */

                 // update document listbox
                 if ( fOK )
                 {
                    sprintf( pIda->szFolder, "%s\\%s%s",
                             pSysProp->PropHead.szPath,
                             szShortName,
                             EXT_FOLDER_MAIN );
                    /****************************************************/
                    /* prefill Doc pattern if not yet done              */
                    /****************************************************/
                    if ( !pIda->szDocPattern[0] )
                    {
                      strcpy( pIda->szDocPattern, DEFAULT_PATTERN );
                    } /* endif */
                    // create document name
                    sprintf( pIda->szPath, "%c%s\\%s\\%s",
                             pIda->pFolProp->chDrive,
                             pIda->szFolder + 1,
                             pSysProp->szDirSourceDoc,
                             pIda->szDocPattern );
                    hwndLB = GETHANDLEFROMID( hwndDlg, ID_TB_OPEN_DOC_LB );
                    usNoOfItems = UtlLoadDocNames( pIda->szPath,
                                                   FILE_NORMAL,
                                                   hwndLB,
                                                    0 );
                    // select document if only one is displayed in listbox
                    if ( usNoOfItems == 1 )
                    {
                       SELECTITEM( hwndDlg, ID_TB_OPEN_DOC_LB, 0 );
                    } /* endif */
                 } /* endif */
                 {
                       HWND hwndLB = GetDlgItem(hwndDlg, ID_TB_OPEN_FOLDER_LB);
                       UtlSetHorzScrollingForLB(hwndLB);
                       hwndLB = GetDlgItem(hwndDlg, ID_TB_OPEN_DOC_LB);
                       UtlSetHorzScrollingForLB(hwndLB);
                 }
                 // end update of document listbox
                 ENABLEUPDATE_TRUE( hwndDlg, ID_TB_OPEN_DOC_LB );
               }
               break;
         } /* endswitch */
         break;
       case  ID_TB_OPEN_FILE_LB :
       case  ID_TB_OPEN_DOC_LB :
         switch ( sNotification )
         {
           case  LN_ENTER:
             EQFBOpenCommand( hwndDlg, MP1FROMSHORT( ID_TB_OPEN_OPEN_PB ), 0L);
             break;
           default :
             break;
         } /* endswitch */
         break;
   } /* endswitch */

   return( mResult );
} /* end of EQFBOpenControl */

//-----------------------------------------------------------------------------
// Internal function
//-----------------------------------------------------------------------------
// Function name:     UtlLoadDocNames
//-----------------------------------------------------------------------------
// Function call:
//-----------------------------------------------------------------------------
// Description:       load document names and display as long name if approp.
//-----------------------------------------------------------------------------
// Parameters:        _
//-----------------------------------------------------------------------------
// Returncode type:   _
//-----------------------------------------------------------------------------
// Returncodes:       _
//-----------------------------------------------------------------------------
// Function flow:     _
//-----------------------------------------------------------------------------
SHORT UtlLoadDocNames( PSZ pszSearch, USHORT atrb, HWND hlbox, USHORT flg)
{
  FILEFINDBUF ResultBuf;               // DOS file find struct
  USHORT  usCount = 1;
  HDIR    hDirHandle = HDIR_CREATE;    // DosFind routine handle
  USHORT  usTotal;
  USHORT  usRC = 0;                    // return code of Dos... alias Utl...
  char    *p, *p2;
  USHORT  fdir = atrb & FILE_DIRECTORY;
  SHORT   sRetCode;                    // return code of function
  BOOL    fCombo = FALSE;              // is-a-combobox flag
  BOOL    fMsg = !(flg & NAMFMT_NOERROR);
  PSZ     pszLongFileName = NULL;      // pointer for long filenames

  UtlAlloc( (PVOID *) &pszLongFileName, 0L, 2L * MAX_LONGPATH, NOMSG );

  if ( !pszLongFileName )
  {
    usRC = ERROR_NOT_ENOUGH_MEMORY;
  } /* endif */

  if ( !usRC )
  {
    ISCOMBOBOX( hlbox, fCombo );

    usRC = UtlFindFirst( pszSearch, &hDirHandle, atrb, &ResultBuf,
                             sizeof( ResultBuf), &usCount, 0L,
                             fMsg);
  } /* endif */

  while ( usCount && !usRC )
  {
     p2 = RESBUFNAME(ResultBuf);
  #ifdef WIN32BIT
     if( fdir && !(fdir & ResultBuf.dwFileAttributes) )
  #else
     if( fdir && !(fdir & ResultBuf.attrib) )
  #endif

     {
       /***************************************************************/
       /* File does not match requested file type                     */
       /***************************************************************/
       p2 = NULL;                    // do not insert into listbox
     }
     else if( strcmp( p2, CURRENT_DIR_NAME ) == 0 )
     {
       /***************************************************************/
       /* File is current directory (".")                             */
       /***************************************************************/
         p2 = NULL;                    // do not insert into listbox
     }
     else if ( strcmp( p2, PARENT_DIR_NAME ) == 0 )
     {
       /***************************************************************/
       /* Process parent directory name ".."                          */
       /***************************************************************/
       if ( !fdir || (flg & NAMFMT_NOROOT) )
       {
          p2 = NULL;               // do not insert into listbox
       }
       else
       {
          p = strchr( pszSearch, BACKSLASH );
          if ( (p == NULL) || (p == strrchr( pszSearch, BACKSLASH) ) )
          {
             // root directory shouldn't have any parent directory !!!
             p2 = NULL;            // do not insert into listbox
          } /* endif */
       } /* endif */
     }
     else
     {
       /***************************************************************/
       /* Process file names using the supplied process flags         */
       /***************************************************************/
       if( flg & NAMFMT_NOEXT )
       {
          if( (p = strrchr( p2, DOT)) != NULL )
          {
             *p = '\0';
          } /* endif */
       } /* endif */

       if ( flg & NAMFMT_NODIR )
       {
          p2 = ( (p=strrchr( p2, BACKSLASH)) != NULL ) ? p + 1 : p2;
       }
       else if ( flg & NAMFMT_NODRV )
       {
          p2 = ( *(p2+1) == COLON) ? p2 + 2 : p2;
       } /* endif */
     } /* endif */

     /*****************************************************************/
     /* Insert string into listbox                                    */
     /*****************************************************************/
     if ( p2 )
     {
       /***************************************************************/
       /* fill in document object name into pObjName                  */
       /***************************************************************/
       PSZ pTemp;
       PSZ pObjName = pszLongFileName + MAX_LONGPATH;
       strcpy( pObjName, pszSearch );
       UtlSplitFnameFromPath( pObjName );
       pTemp = UtlGetFnameFromPath( pObjName );
       strcpy( pTemp, p2 );
       usRC =  DocQueryInfo2(pObjName,
                             NULL,NULL,NULL,NULL,
                             pszLongFileName,
                             NULL,NULL,FALSE);

       if ( ! *pszLongFileName || usRC )
       {
         strcpy( pszLongFileName, p2 );
       } /* endif */

       if ( fCombo )
       {
         CBINSERTITEMHWND( hlbox, pszLongFileName );
       }
       else
       {
         INSERTITEMHWND( hlbox, pszLongFileName );
       } /* endif */
     } /* endif */

     /*****************************************************************/
     /* Get next file                                                 */
     /*****************************************************************/
     usRC = UtlFindNext( hDirHandle, &ResultBuf, sizeof( ResultBuf),
                                 &usCount, fMsg );

  } /* endwhile */

  // close search file handle
  if ( hDirHandle != HDIR_CREATE ) UtlFindClose( hDirHandle, FALSE );

  /********************************************************************/
  /* Get total number of list box entries                             */
  /********************************************************************/

  if ( fCombo )
  {
    usTotal = (USHORT)CBQUERYITEMCOUNTHWND( hlbox );
  }
  else
  {
    usTotal = (USHORT)QUERYITEMCOUNTHWND( hlbox );
  } /* endif */

  /********************************************************************/
  /* Select first item on request                                     */
  /********************************************************************/
  if ( (flg & NAMFMT_TOPSEL) && usTotal )
  {
    if ( fCombo )
    {
      CBSELECTITEMHWND(hlbox, 0);
    }
    else
    {
      SELECTITEMHWND(hlbox, 0);
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* Build return code                                                */
  /********************************************************************/
  if ( ( usRC == 0 )                   ||
       ( usRC == ERROR_NO_MORE_FILES ) ||
       ( usRC == ERROR_FILE_NOT_FOUND ) )
  {
     sRetCode = usTotal;
  }
  else
  {
     sRetCode = UTLERROR;
  } /* endif */

  UtlAlloc( (PVOID *) &pszLongFileName, 0L, 0L, NOMSG );

  /********************************************************************/
  /* Return return code to caller                                     */
  /********************************************************************/
  return( sRetCode );
} /* end of function UtlLoadDocNames */

/*////////////////////////////////////////////////////////////////////////////
:h2.EQFBOpenChar - process WM_CHAR messages of open dialog
*/
// Description:
//    Handle WM_CHAR messages in open dialog panel.
//
//   Flow:
//      if VK_ENTER or VK_NEWLINE is pressed and
//       focus is not on pushbuttons and
//       file name entry field has been changed then
//         if document mode
//            split given name in folder and document
//            if folder is given and valid
//               remove folder from name;
//               select specified folder;
//            endif
//            use name as search pattern for documents;
//            refresh documents listbox;
//         else
//            split given name in drive, path, name
//            if drive given and valid
//               remove drive from name;
//               activate specified drive;
//            endif
//            if path given and valid
//               remove path from name;
//               set current path to specified path;
//            endif
//            use name as search pattern;
//            refresh directory and file listbox
//         endif
//      else
//         pass message to default message procedure;
//      endif;
//
//
// Arguments:
//   SHORT1FROMMP(mp1) = character data
//   SHORT2FROMMP(mp1) = character data
//
// Returns:
//  mResult           : FALSE = message not processed
//                      TRUE  = message has been processed
//
// Prereqs:
//   None
//
// SideEffects:
//   - values in IDA are updated
//
// External references:
//   None
//
//////////////////////////////////////////////////////////////////////////////

MRESULT EQFBOpenChar
(
   HWND hwndDlg,
   WPARAM mp1,
   LPARAM mp2
)
{
   MRESULT mResult = FALSE;            // result of message processing

   POPENIDA pIda;                      // ptr to dialog IDA
   SHORT       sItem;                  // listbox item index
   BOOL        fOK = TRUE;             // internal OK flag
   HWND        hwndFocus;              // window having the focus
   PSZ         pszDrive;               // ptr to drive info in path
   PSZ         pszPath;                // ptr to directory info in path
   PSZ         pszName;                // ptr to file name in path
   CHAR        chDrive = EOS;          // selected drive


   mp2 = mp2;                          // supress 'unreferenced parameter' msg
   mp1;

   if ( GetKeyState(VK_RETURN) & 0x8000  )
   {
      pIda = ACCESSDLGIDA(hwndDlg, POPENIDA);
      hwndFocus = GETFOCUS();
      if ( ( hwndFocus == GetDlgItem( hwndDlg, ID_TB_OPEN_FILNAME_EF )) ||
           ( hwndFocus == GetDlgItem( hwndDlg, ID_TB_OPEN_DOCNAME_EF )))
      {
         if ( pIda->fTroja )
         {
            QUERYTEXT( hwndDlg, ID_TB_OPEN_DOCNAME_EF, pIda->szPath );
         }
         else
         {
            QUERYTEXT( hwndDlg, ID_TB_OPEN_FILNAME_EF, pIda->szPath );
         } /* endif */

         // split drive from path
         pszPath = strchr( pIda->szPath, COLON );
         if ( pszPath )
         {
            pszDrive = pIda->szPath;
            *pszPath = EOS;
            pszPath ++;
         }
         else
         {
            pszDrive = NULL;
            pszPath = pIda->szPath;
         } /* endif */

         // skip first backslash in path or file name
         if ( *pszPath == BACKSLASH )
         {
            pszPath++;
         } /* endif */

         // split filename from path
         pszName = strrchr( pszPath, BACKSLASH );
         if ( pszName )
         {
            *pszName = EOS;
            pszName++;
         }
         else
         {
            pszName = pszPath;
            pszPath = NULL;
         } /* endif */

         // process input considering current open mode
         if ( pIda->fTroja )
         {
            // in troja mode no drive is allowed and the path must be
            // a folder name
            if ( pszDrive )
            {
                fOK = FALSE;
            }
            else if ( !pszPath )                  // no folder name specified
            {
               // use given name as document search pattern
               strcpy( pIda->szDocPattern, pszName );

               // force a refresh of document listbox
               EQFBOpenControl( hwndDlg, ID_TB_OPEN_FOLDER_LB,
                                LN_SELECT );

            }
            else
            {
               // check folder name (= search for folder in listbox)
               sItem = SEARCHITEM( hwndDlg, ID_TB_OPEN_DIR_LB, pszPath );
               if ( sItem != LIT_NONE )
               {
                  // use given name as document search pattern
                  strcpy( pIda->szDocPattern, pszName );

                  // select specified folder
                  SELECTITEM( hwndDlg, ID_TB_OPEN_DIR_LB, sItem );
               }
               else
               {
                  fOK = FALSE;         // folder name is invalid
               } /* endif */
            } /* endif */
         }
         else
         {
            // check specified drive
            if ( pszDrive )
            {
               strupr( pszDrive );
               if ( ( strlen( pszDrive ) != 1 ) ||
                    !strchr( pIda->szDrives, *pszDrive ) )
               {
                  fOK = FALSE;         // drive is invalid
               }
               else
               {
                  chDrive = *pszDrive; // set new drive
               } /* endif */
            }
            else
            {
               chDrive = pIda->chDrive;          // use current drive
            } /* endif */

            // check directory information
            if ( fOK )
            {
               if ( pszPath )
               {
                  sprintf( pIda->szPath, "%c:\\%s", chDrive, pszPath );
                  if ( EQFBFileExists( pIda->szPath ) )
                  {
                     sprintf( pIda->szDirectory, "\\%s", pszPath );
                  }
                  else
                  {
                     fOK = FALSE;
                  } /* endif */
               }
               else
               {
                  pIda->szDirectory[0] = EOS;
               } /* endif */
            } /* endif */

            // if ok use specified values
            if ( fOK )
            {
               // set new drive if required
               if ( chDrive != pIda->chDrive )
               {
                  if ( pIda->chDrive != EOS )
                  {
                     SETDRIVE( hwndDlg,
                               IDFROMDRIVE( ID_FROMDRIVEA_BTN, pIda->chDrive ),
                               FALSE );
                  } /* endif */
                  pIda->chDrive = chDrive;
                  SETDRIVE( hwndDlg, IDFROMDRIVE( ID_FROMDRIVEA_BTN, pIda->chDrive ),
                            TRUE );
               } /* endif */

               //--- refresh contents of listboxes ---
               if ( pszName && *pszName )
               {
                  strcpy( pIda->szFilePattern, pszName );
               }
               else
               {
                  strcpy( pIda->szFilePattern, "*.*" );
               } /* endif */

               EQFBOpenFillListboxes( hwndDlg, pIda );


            } /* endif */
         } /* endif */

         // report any pending error
         if ( !fOK )
         {
            if ( pIda->fTroja )
            {
              QUERYTEXT( hwndDlg, ID_TB_OPEN_DOCNAME_EF, pIda->szPath );
            }
            else
            {
              QUERYTEXT( hwndDlg, ID_TB_OPEN_FILNAME_EF, pIda->szPath );
            } /* endif */
            pszPath = pIda->szPath;
            UtlError( ERROR_FILENAME_NOT_VALID, MB_CANCEL, 1, &pszPath,
                      EQF_ERROR );
         } /* endif */
         mResult = MRFROMSHORT(TRUE);  // do not process as default pushbutton
      }
      else if ( ( hwndFocus == GetDlgItem( hwndDlg, ID_TB_OPEN_DIR_LB )) )
      {
        /**************************************************************/
        /* force update of files listbox and reset search pattern...  */
        /**************************************************************/
        strcpy( pIda->szFilePattern, "*.*" );

        // force a refresh of files listbox
        EQFBOpenControl( hwndDlg, ID_TB_OPEN_DIR_LB, LN_ENTER );
        mResult = TRUE;                  // do not process as default pushbutton
      }
      else
      {
         mResult = WinDefDlgProc( hwndDlg, WM_CHAR, mp1, mp2 );
      } /* endif */
   }
   else
   {
      mResult = WinDefDlgProc( hwndDlg, WM_CHAR, mp1, mp2 );
   } /* endif */

   return( mResult );

} /* end of EQFBOpenChar */

/*////////////////////////////////////////////////////////////////////////////
:h2.EQFBOpenFillListboxes - fill file and directory listboxes
*/
// Description:
//    Fill the directory and file listbox of the open dialog.
//
//   Flow:
//      build path using current values in IDA;
//      update file name field;
//      fill directory listbox using UtlLoadFileNames(.., FILE_DIRECTORY, .. );
//      fill file listbox using UtlLoadFileNames(.., FILE_NORMAL, .. );
//
// Arguments:
//   HWND     hwndDlg  = handle of open dialog window
//   POPENIDA pIda     = ptr to open dialog IDA
//
// Returns:
//   Nothing
//
// Prereqs:
//   None
//
// SideEffects:
//   - file and directory listbox are filled
//
// External references:
//   UtlLoadFileNames
//
//////////////////////////////////////////////////////////////////////////////
VOID EQFBOpenFillListboxes( HWND hwndDlg, POPENIDA pIda )
{
   HWND  hwndLB;                       // handle of currently processed listbox
   SHORT sNoOfItems;                   // no of items in a listbox

   BOOL   fLongFileNames = TRUE;         // use-long-file-name-functions flag

   // under Windows we can use long file name functions for all
   // Windows Version higher than or equal to 3.95
   DWORD dwVersion = GetVersion();
   BYTE  bWinMajVersion = LOBYTE( LOWORD( dwVersion ) );
   BYTE  bWinMinVersion = HIBYTE( LOWORD( dwVersion ) );
   if ( bWinMajVersion <= 2 )
   {
     fLongFileNames = FALSE;
   }
   else if ( (bWinMajVersion == 3) && (bWinMinVersion < 95) )
   {
     fLongFileNames = FALSE;
   } /* endif */

   //--- build directory path ---
   sprintf( pIda->szPath, "%c:%s\\%s", pIda->chDrive, pIda->szDirectory,
            DEFAULT_PATTERN );

   //--- refresh directory listbox ---
   hwndLB = GETHANDLEFROMID( hwndDlg, ID_TB_OPEN_DIR_LB );
   ENABLEUPDATE_FALSE( hwndDlg, ID_TB_OPEN_DIR_LB );
   DELETEALL( hwndDlg, ID_TB_OPEN_DIR_LB );
   sNoOfItems = UtlLoadFileNames( pIda->szPath, FILE_DIRECTORY,
                                  hwndLB, NAMFMT_TOPSEL | NAMFMT_NOERROR );
   ENABLEUPDATE_TRUE( hwndDlg, ID_TB_OPEN_DIR_LB );

   if ( sNoOfItems == UTLERROR )
   {
     //--- refresh files listbox --- i.e. delete everything if old nothing
     //--- available any more...
     DELETEALL( hwndDlg, ID_TB_OPEN_FILE_LB );
   }
   else
   {
     //--- build files path ---
     if ( !pIda->szFilePattern[0] )
     {
       strcpy( pIda->szFilePattern, DEFAULT_PATTERN );
     } /* endif */
     sprintf( pIda->szPath, "%c:%s\\%s", pIda->chDrive, pIda->szDirectory,
                                         pIda->szFilePattern );

     //--- update file name field ---
     SETTEXT( hwndDlg, ID_TB_OPEN_FILNAME_EF, pIda->szPath );

     //--- refresh files listbox ---
     ENABLEUPDATE_FALSE( hwndDlg, ID_TB_OPEN_FILE_LB );
     DELETEALL( hwndDlg, ID_TB_OPEN_FILE_LB );
     hwndLB = GETHANDLEFROMID( hwndDlg, ID_TB_OPEN_FILE_LB );
     if (fLongFileNames )
     {
       sNoOfItems = UtlLoadLongFileNames( pIda->szPath, FILE_NORMAL, hwndLB, 0 );
     }
     else
     {
       sNoOfItems = UtlLoadFileNames( pIda->szPath, FILE_NORMAL, hwndLB, 0 );
     } /* endif */

     // select file if only one is displayed in listbox
     if ( sNoOfItems == 1 )
     {
        SELECTITEM( hwndDlg, ID_TB_OPEN_FILE_LB, 0 );
     } /* endif */
     ENABLEUPDATE_TRUE( hwndDlg, ID_TB_OPEN_FILE_LB );
   } /* endif */
   {
      HWND hwndLB = GetDlgItem(hwndDlg, ID_TB_OPEN_FILE_LB);
      UtlSetHorzScrollingForLB(hwndLB);
      hwndLB = GetDlgItem(hwndDlg, ID_TB_OPEN_DIR_LB);
      UtlSetHorzScrollingForLB(hwndLB);
   }
} /* end of EQFBOpenFillListboxes */

/*////////////////////////////////////////////////////////////////////////////
:h2.EQFBOpenCommand - process WM_COMMAND messages of open dialog
*/
// Description:
//    Handle WM_COMMAND messages (= pressing of pushbuttons) of
//    open dialog panel.
//
//   Flow (message driven):
//      case 'Open' pushbutton:
//         update callers OPENDATA structure with actual values;
//         post a WM_CLOSE message to dialog, mp1 = TRUE;
//      case CANCEL pushbutton or DID_CANCEL (= ESCAPE key):
//         post a WM_CLOSE messgae to dialog, mp1 = FALSE;
//
// Arguments:
//   SHORT1FROMMP(mp1) = ID of control sending the WM_COMMAND message
//
// Returns:
//  MRESULT(TRUE)  command is processed
//
// Prereqs:
//   None
//
// SideEffects:
//   - dialog is removed
//   - callers OPENDATA structure is updated if required
//
// External references:
//   None
//
//////////////////////////////////////////////////////////////////////////////
MRESULT EQFBOpenCommand
(
   HWND hwndDlg,                       // dialog handle
   WPARAM mp1,
   LPARAM mp2
)
{
   MRESULT mResult = MRFROMSHORT(TRUE);// TRUE = command is processed
   POPENIDA pIda;                      // ptr to dialog IDA
   SHORT sItem;                        // index of listbox item
   BOOL        fOK = TRUE;             // internal OK flag
   SHORT sId = WMCOMMANDID( mp1, mp2 );     // id of button

   mp2;
   pIda = ACCESSDLGIDA(hwndDlg, POPENIDA);

   switch ( sId )
   {
	  case ID_TB_OPEN_HELP_PB:
	    mResult = UtlInvokeHelp();
	    break;
      case ID_TB_OPEN_OPEN_PB:
         // check if a document/file name is selected
         if ( pIda->fTroja )
         {
            sItem = QUERYSELECTION( hwndDlg, ID_TB_OPEN_DOC_LB );
            if ( sItem == LIT_NONE )
            {
               UtlError( NO_FILE_SELECTED, MB_CANCEL, 0, NULL, EQF_ERROR );
               fOK = FALSE;
            }
            else
            {
               QUERYITEMTEXT( hwndDlg, ID_TB_OPEN_DOC_LB, sItem,
                              pIda->szDocument );
            } /* endif */
         }
         else
         {
            sItem = QUERYSELECTION( hwndDlg, ID_TB_OPEN_FILE_LB );
            if ( sItem == LIT_NONE )
            {
               UtlError( NO_FILE_SELECTED, MB_CANCEL, 0, NULL, EQF_ERROR );
               fOK = FALSE;
            }
            else
            {
               QUERYITEMTEXT( hwndDlg, ID_TB_OPEN_FILE_LB, sItem,
                              pIda->szFile );
            } /* endif */
         } /* endif */

         // fill callers open data structure
         if ( fOK )
         {
            pIda->pOpenData->fTroja = (EQF_BOOL) pIda->fTroja;
            strcpy( pIda->pOpenData->szFilePattern, pIda->szFilePattern );
            strcpy( pIda->pOpenData->szDocPattern, pIda->szDocPattern );
            sprintf( pIda->pOpenData->szFileName, "%c:%s\\%s",
                     pIda->chDrive, pIda->szDirectory, pIda->szFile );
            if (pIda->fTroja)
			{
				pSysProp = GetSystemPropPtr();
            	if ( pSysProp )
            	{
            	  BOOL fIsNew;
            	  CHAR chTemp = pIda->szFolder[0];

            	 /*******************************************************/
            	 /* use document object name -- take care of folders on */
            	 /* secondary drives                                    */
            	 /*******************************************************/

                  pIda->szFolder[0] = pIda->pFolProp->chDrive;
            	  FolLongToShortDocName(pIda->szFolder,
                                   pIda->szDocument,
                                   pIda->szLongDocName,
                                   &fIsNew);
            	  pIda->szFolder[0] = chTemp;

            	  if(fIsNew)
            	  {
            	    fOK = FALSE;
            	  }
            	  else
            	  {
            	    sprintf( pIda->pOpenData->szDocName, "%c%s\\%s",
                        pIda->pFolProp->chDrive,
                        pIda->szFolder + 1,
                        pIda->szLongDocName );
            	  }/* end if */
	            }
            	else
            	{
            	   sprintf( pIda->pOpenData->szDocName, "%c%s\\%s",
                        pIda->pFolProp->chDrive,
                        pIda->szFolder + 1,
                        pIda->szDocument );
            	} /* endif */
		   	} /* endif fTroja*/
         } /* endif */

         // close dialog by posting WM_CLOSE
         if ( fOK )
         {
            POSTEQFCLOSE( hwndDlg, TRUE );
         } /* endif */
         break;

      case ID_TB_OPEN_CANCEL_PB:
      case DID_CANCEL:
         POSTEQFCLOSE( hwndDlg, FALSE );
         break;

        case ID_TB_OPEN_DOC_RB:
        case ID_TB_OPEN_FILE_RB:
        case ID_TB_OPEN_DIR_LB:
        case ID_TB_OPEN_FOLDER_LB:
          mResult = EQFBOpenControl( hwndDlg, sId, WMCOMMANDCMD( mp1, mp2 ) );
          break;

      case ID_TB_OPEN_DOCNAME_EF:
         if ( WMCOMMANDCMD( mp1, mp2 ) == EN_KILLFOCUS )
         {
           ClearIME( hwndDlg );
         } /* endif */
         break;

      default:
         //--- check for drive buttons ---
         if ( ( sId >= ID_FROMDRIVEA_BTN )  && ( sId <= ID_FROMDRIVEZ_BTN )  )
         {
            //--- deselect any previously selected drive button ---
            if ( pIda->chDrive != EOS )
            {
               SETDRIVE( hwndDlg,
                         IDFROMDRIVE( ID_FROMDRIVEA_BTN, pIda->chDrive ),
                         FALSE );
            } /* endif */
            pIda->chDrive = DRIVEFROMID( ID_FROMDRIVEA_BTN, sId );
            SETDRIVE( hwndDlg, sId, TRUE );

            //--- refresh contents of listboxes ---
            pIda->szDirectory[0] = EOS;
            EQFBOpenFillListboxes( hwndDlg, pIda );
         }
         else
         {
           mResult = WinDefDlgProc( hwndDlg, WM_COMMAND, mp1, mp2 );
         } /* endif */
         break;


   } /* endswitch */

   return( mResult );
} /* end of EQFBOpenCommand */

/*////////////////////////////////////////////////////////////////////////////
:h2.EQFBOpenClose - process WM_CLOSE messages of open dialog
*/
// Description:
//    Handle WM_CLOSE messages (= dialog termination requests) of
//    open dialog panel.
//
//   Flow:
//      free allocated storage
//      dismiss dialog
//
// Arguments:
//   SHORT1FROMMP(mp1) = type to be returned using WinDismissDlg
//
// Returns:
//  MRESULT(FALSE)
//
// Prereqs:
//   None
//
// SideEffects:
//   - caller receives type specifed in mp1
//
// External references:
//   UtlAlloc
//
//////////////////////////////////////////////////////////////////////////////
MRESULT EQFBOpenClose
(
   HWND hwndDlg,
   WPARAM mp1,
   LPARAM mp2
)
{
   MRESULT mResult = FALSE;
   POPENIDA pIda;                      // ptr to dialog IDA

   mp1 = mp1;                          // supress 'unreferenced parameter' msg
   mp2 = mp2;                          // supress 'unreferenced parameter' msg

   pIda = ACCESSDLGIDA(hwndDlg, POPENIDA);

   DelCtrlFont (hwndDlg, ID_TB_OPEN_DOCNAME_EF);
   DelCtrlFont (hwndDlg, ID_TB_OPEN_DIR_LB);
   DelCtrlFont (hwndDlg, ID_TB_OPEN_FILE_LB);

   //--- free allocated memory ---
   if ( pIda )
   {
      if ( pIda->pFolProp )
      {
         UtlAlloc((PVOID *) &pIda->pFolProp, 0L, 0L, NOMSG );
      } /* endif */
      UtlAlloc((PVOID *) &pIda, 0L, 0L, NOMSG );
   } /* endif */

   //--- get rid off dialog ---
   DISMISSDLG( hwndDlg, SHORT1FROMMP1( mp1 ) );

   return( mResult );
} /* end of EQFBOpenClose */



/*////////////////////////////////////////////////////////////////////////////
:h2.EQFBCheckForMnemonic
*/
// Description:
//    checks if the passed key is already used as mnemonic
//
//   Flow:
//
// Arguments:
//   PKEYSIDA  pointer to ida
//   MPARAM    message parameter
//
// Returns:
//   BOOL      TRUE   - if key is an mnemonic
//             FALSE  -  else
// Prereqs:
//   None
//
// SideEffects:
//
// External references:
//   None
//
//////////////////////////////////////////////////////////////////////////////
BOOL EQFBCheckForMnemonic
(
   PKEYSIDA pIda,
   WPARAM   mp1,
   LPARAM   mp2
)
{
   BOOL  fAccel = FALSE;

   CHAR  chStr[ 40 ];                  // space for string
   LONG  lIndex = 0L;                   // index
   USHORT  usI;                        // starting index
   PSZ    pResult;                     // pointer to first character

   mp2;
   pIda;
   // get actionbar handle and check if mnemonic matches
   /*******************************************************************/
   /* check if alt-key is pressed                                     */
   /*******************************************************************/
   if ( GetKeyState (VK_ALT) & 0x8000 )
   {
     HMENU   hwndMenu;                 // menu handle
     LONG    lItems;                  // number of items in AAB

     /*****************************************************************/
     /* go through list of AAB Items and check if text contains '&'   */
     /* sign                                                          */
     /* Note: We do not have to check to AABs because under windows   */
     /*       only one is active at the time we are checking for the  */
     /*       key...                                                  */
     /*****************************************************************/

     hwndMenu = GetMenu( (HWND) UtlQueryULong( QL_TWBFRAME ) );
     lItems = GetMenuItemCount( hwndMenu );
     usI = 0;
     while ( !fAccel && (usI < lItems) )
     {
       lIndex = GetMenuString( hwndMenu, usI,
                               chStr, sizeof(chStr), MF_BYPOSITION );
       usI ++;                           // point to next element
       if ( lIndex )
       {
          pResult = strchr( chStr, '&' );        // is it mnemonic
          fAccel = pResult &&
                   (toupper((USHORT)*(pResult+1)) == toupper( (USHORT) mp1 ));
       } /* endif */
     } /* endwhile */
   } /*endif*/
   return (fAccel);
}

/*////////////////////////////////////////////////////////////////////////////
:h2.EQFBLeaveCaptMode
*/
// Description:
//    leave capture mode and enable all pushbuttons
//
//   Flow:
//
// Arguments:
//   HWND      window handle
//   PKEYSIDA  ptr to key ida
//
// Returns:
//   NONE
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
VOID EQFBLeaveCaptMode
(
   HWND        hwndDlg,
   PKEYSIDA    pIda                    // dialog instance data area ptr
)
{

   //--- leave capture mode ---
   HIDECONTROL( hwndDlg, ID_TB_KEYS_KEY_TEXT );
   SETFOCUS( hwndDlg, ID_TB_KEYS_ASSIGN_PB );
   pIda->fCapture = FALSE;
   if (lpOldProc )
   {
     SetWindowLong( GetDlgItem( hwndDlg, ID_TB_KEYS_KEY_TEXT),
                    GWL_WNDPROC, (LPARAM) lpOldProc  );
     lpOldProc = NULL;
   } /* endif */

   ENABLECTRL( hwndDlg, ID_TB_KEYS_FUNCTION_LB, TRUE );
   ENABLECTRL( hwndDlg, ID_TB_KEYS_SET_PB, TRUE );
   ENABLECTRL( hwndDlg, ID_TB_KEYS_CLEAR_PB, TRUE );
   ENABLECTRL( hwndDlg, ID_TB_KEYS_CANCEL_PB, TRUE );
   ENABLECTRL( hwndDlg, ID_TB_KEYS_DEFAULT_PB, TRUE );
   ENABLECTRL( hwndDlg, ID_TB_KEYS_ASSIGN_PB, TRUE );
   ENABLECTRL( hwndDlg, ID_HELP, TRUE );
   ENABLECTRL( hwndDlg, ID_TB_KEYS_PRINT_PB, TRUE );
   // Add for R012027
   ENABLECTRL( hwndDlg, ID_TB_KEYS_ADD_NEW_CHAR, TRUE );
   ENABLECTRL( hwndDlg, ID_TB_KEYS_EDIT_CHAR, TRUE );
   ENABLECTRL( hwndDlg, ID_TB_KEYS_REMOVE_CHAR, TRUE );
   // Add end
   /*******************************************************************/
   /* set focus back to listbox                                       */
   /*******************************************************************/
   SETFOCUS( hwndDlg, ID_TB_KEYS_FUNCTION_LB );
}



//-----------------------------------------------------------------------------
// Function name: EQFBCommandDlgProc - dialog procedure for Command dialog
//-----------------------------------------------------------------------------
// Description:   allows to select a function and execute it
//-----------------------------------------------------------------------------
// Parameters:    HWND hwndDlg,      handle of dialog window
//                USHORT msg,
//                WPARAM mp1,
//                LPARAM mp2
//-----------------------------------------------------------------------------
// Returncode type:  MRESULT APIENTRY
//-----------------------------------------------------------------------------
// Returncodes:   mResult (result value of procedure)
//-----------------------------------------------------------------------------
// Prerequesits:  none
//-----------------------------------------------------------------------------
// Side effects:  none
//-----------------------------------------------------------------------------
// Function call: EQFBCommandDlgProc(hwndDlg,msg,mp1,mp2)
//-----------------------------------------------------------------------------
// Function flow:   case WM_INITDLG:
//                    initialize the dialog controls;
//                  case WM_COMMAND
//                    call EQFBCommandCommand to handle user commands;
//                  case WM_CLOSE
//
//
//
//
//
//-----------------------------------------------------------------------------
INT_PTR CALLBACK EQFBCOMMANDDLGPROC
(
   HWND hwndDlg,                       /* handle of dialog window             */
   WINMSG msg,
   WPARAM mp1,
   LPARAM mp2
)
{
   MRESULT  mResult = MRFROMSHORT( FALSE );                // result value of procedure
   BYTE     bEditor = EDIT_STANDARD;
   PTPEXECUTE pTPExec;

   switch ( msg )
   {
      case WM_EQF_QUERYID: HANDLEQUERYID( ID_TB_COMMAND_DLG, mp2 ); break;

     case WM_INITDLG:
          SETWINDOWID( hwndDlg, ID_TB_COMMAND_DLG );
          ulEQFBLineWidth = 0;
          ANCHORDLGIDA( hwndDlg, mp2 );
          // fill listbox with available functions
          pTPExec = ACCESSDLGIDA(hwndDlg, PTPEXECUTE);
          bEditor = (pTPExec->pDoc->hwndRichEdit) ? EDIT_RTF : EDIT_STANDARD;
          EQFBKeysFillListbox( hwndDlg, ID_TB_COMMAND_LB,
                               get_KeyTable(), get_FuncTab(), bEditor );
          mResult = DIALOGINITRETURN( mResult );
          break;

      case WM_COMMAND:
         mResult = EQFBCommandCommand( hwndDlg, mp1, mp2 );
         break;

      case WM_EQF_CLOSE:
         //--- get rid off dialog ---
         DISMISSDLG( hwndDlg, TRUE );
         break;

      case WM_MEASUREITEM:
         MEASUREITEM( mp2, EQFBQueryLineHeight(), mResult );
         break;

      case WM_DRAWITEM:
         pTPExec = ACCESSDLGIDA(hwndDlg, PTPEXECUTE);
         bEditor = (pTPExec->pDoc->hwndRichEdit) ? EDIT_RTF : EDIT_STANDARD;
         mResult = DrawKeysFuncList( hwndDlg, mp2, FALSE, bEditor );
         break;

      default:
         mResult = WinDefDlgProc( hwndDlg, msg, mp1, mp2 );
         break;
   } /* endswitch */
   return mResult;
} /* end of EQFBCommandDlgProc */


//-----------------------------------------------------------------------------
// Function name: EQFBCommandCommand - process WM_COMMAND messages of
//                execute dialog
//-----------------------------------------------------------------------------
// Description:
//                Handle WM_COMMAND messages (= pressing of pushbuttons) of
//                command dialog panel.
//-----------------------------------------------------------------------------
// Parameters:    SHORT1FROMMP(mp1) = ID of control sending the WM_COMMAND
//                message
//-----------------------------------------------------------------------------
// Returncode type:  MRESULT(TRUE)  = command is processed
//-----------------------------------------------------------------------------
// Returncodes:   TRUE = command is processed
//-----------------------------------------------------------------------------
// Prerequesits:  none
//-----------------------------------------------------------------------------
// Side effects:  none
//-----------------------------------------------------------------------------
// Function call: EQFBCommandCommand (hwndDlg,mp1,mp2)
//-----------------------------------------------------------------------------
// Function flow:     case 'Execute' pushbutton:
//                       get data from listbox and execute it
//                    case CANCEL pushbutton or DID_CANCEL (= ESCAPE key):
//                       post a WM_CLOSE message to dialog, mp1 = 0;
//
//
//
//
//
//-----------------------------------------------------------------------------
MRESULT EQFBCommandCommand
(
   HWND hwndDlg,
   WPARAM mp1,
   LPARAM mp2
)
{
   MRESULT mResult = MRFROMSHORT(TRUE);     // TRUE = command is processed
   SHORT        sItem;                      // index of selected listbox item

   mp2 = mp2;                               // get rid off compiler warning

   switch ( WMCOMMANDID( mp1, mp2 ) )
   {
	  case ID_TB_COMMAND_HELP_PB:
	    mResult = UtlInvokeHelp();
	     break;
      case ID_TB_COMMAND_EXECUTE_PB:          // lookup sentence
         sItem = QUERYSELECTION( hwndDlg, ID_TB_COMMAND_LB );
         if ( sItem != LIT_NONE )
         {
            PTPEXECUTE pTPExec;

            pTPExec = ACCESSDLGIDA(hwndDlg, PTPEXECUTE);
            pTPExec->pKey  = (PKEYPROFTABLE) QUERYITEMHANDLE( hwndDlg,
                                                     ID_TB_COMMAND_LB, sItem );
            POSTEQFCLOSE( hwndDlg, FALSE );
         } /* endif */
         break;

      case ID_TB_COMMAND_CANCEL_PB:
      case DID_CANCEL:
         POSTEQFCLOSE( hwndDlg, FALSE );
         break;

      default:
         mResult = WinDefDlgProc( hwndDlg, WM_COMMAND, mp1, mp2 );
         break;
   } /* endswitch */


   return( mResult );
} /* end of EQFBCommandCommand */

//-----------------------------------------------------------------------------
// Internal function
//-----------------------------------------------------------------------------
// Function name:     EQFBKeysPrint   -  print the current keys settings
//-----------------------------------------------------------------------------
// Function call:     EQFBKeysPrint  ( HWND );
//
//-----------------------------------------------------------------------------
// Description  :      print the current settings of the keys
//
//                     Flow:
//                        for all functions in key assignment table
//                           get function names
//                           print them on the printer
//                        endfor;
//
//
//-----------------------------------------------------------------------------
// Parameters:        HWND   hwnd
//
//-----------------------------------------------------------------------------
// Returncode type:   VOID
//-----------------------------------------------------------------------------
// Returncodes:        nothing
//-----------------------------------------------------------------------------

static
VOID EQFBKeysPrint
(
   HWND        hwndLB                  // handle of the listbox
)
{
   SHORT       sItem = 0;              // listbox item index
   CHAR        szItemName[256];        // buffer for item names
#ifdef _QPRU
   PVOID         hPrint = NULL;         // print handle
   ULONG         ulLastPos;             // last printing position
#else
   HPRINT        hPrint = NULLHANDLE;   // print handle
#endif

   BOOL        fOK ;                   // success indicator
   LONG        lItemLen = 1L;           // length of item
   HAB         hab;                    // anchor block handle
   CHAR        szKeyName[100 ];        // key name buffer
   CHAR_W      szTempNameW[100];
   PSZ         pTab;                   // position of TAB character
   HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);

   /*******************************************************************/
   /* set hour glass                                                  */
   /*******************************************************************/
   SETCURSOR( SPTR_WAIT );
   hab = GETINSTANCE( hwndLB );
   // open the printer device context
   LOADSTRING( hab, hResMod, IDS_TB_ASSKEYS_PRTJOB, szItemName );
#ifdef _QPRU
   fOK = (UtlPrintOpen2( szItemName, NULL, &hPrint, TRUE ) == NO_ERROR);
#else
   fOK = UtlPrintOpen( &hPrint, szItemName, NULLHANDLE );
#endif
   LOADSTRING( hab, hResMod, IDS_TB_ASSKEYS_HEADER, szItemName );

   if ( fOK )
   {
     lItemLen = strlen( szItemName );
     szItemName[lItemLen++] = LF;
     szItemName[lItemLen++] = LF;
     szItemName[lItemLen++] = EOS;

     EQFAnsiToOem( szItemName, szItemName );

     ASCII2Unicode( szItemName, szTempNameW, 0L );
#ifdef _QPRU
     fOK = !UtlPrintLine2( hPrint, szTempNameW, 0l, 0, 0l, &ulLastPos, TRUE );
#else
     fOK = UtlPrintLineW( hPrint, szTempNameW );
#endif
   } /* endif */

   while ( lItemLen && fOK)               // while not end of key list
   {
     lItemLen = QUERYITEMTEXTHWND( hwndLB, sItem, szItemName );
     sItem++;
     if ( lItemLen > 0)
     {
       pTab = strchr( szItemName, '\t' );
       *pTab = EOS;
       pTab++;
       /***************************************************************/
       /* print only functions which have a key assigned              */
       /***************************************************************/
       if ( *pTab != '*' )
       {
         sprintf( szKeyName, "%-40s%s\n", szItemName, pTab );

         EQFAnsiToOem( szKeyName, szKeyName );

         ASCII2Unicode( szKeyName, szTempNameW, 0L );
#ifdef _QPRU
         fOK = !UtlPrintLine2( hPrint, szTempNameW, 0l, 0, 0l, &ulLastPos, TRUE );
#else
         fOK = UtlPrintLineW( hPrint, szTempNameW );
#endif
       } /* endif */
     }
     else
     {
       // set it explicitly in the windows case ...
       lItemLen = 0L;
     } /* endif */
   } /* endwhile */

   /*************************************************************/
   /* close the print device and reset pointer                  */
   /*************************************************************/

#ifdef _QPRU
  if ( hPrint )         UtlPrintClose2( &hPrint );
#else
  if ( hPrint )         UtlPrintClose( hPrint );
#endif

   SETCURSOR( SPTR_ARROW );
} /* end of EQFBKeysPrint */


/*////////////////////////////////////////////////////////////////////////////
:h2.EQFBQueryLineHeight
*/
// Description:
//    Returns the line height of the default font
//
//   Flow:
//       if static variable usEQFBLineHeight is not set
//         query font metrics of default font;
//         set static variable usEQFBLineHeight to lExternalLeading value of
//         default font;
//       endif;
//       return static variable usEQFBLineHeight
//
// Arguments:
//  None
//
// Returns:
//  USHORT usLineHeight             - height of default font
//
// Prereqs:
//   None
//
// SideEffects:
//   if not already done, the static variable usEQFBLineHeight is set
//
// External references:
//   None
//
//////////////////////////////////////////////////////////////////////////////
USHORT EQFBQueryLineHeight( VOID )
{
   if ( !usEQFBLineHeight )            // if no line height computed yet ...
   {
      // ... compute / query the line heigth now
      GETLINEHEIGHT( HWND_DESKTOP, usEQFBLineHeight );
   } /* endif */
   return( usEQFBLineHeight );
} /* endof EQFBQueryLineHeight */


// IDA of comment dialog
typedef struct _COMMENTDLGIDA
{
  HWND        hwndParent;                        // parent handle
  PTBDOCUMENT pDoc;                              // document
  PVOID       pvAddInfo;                         // additional info data of segment
  SHORT       x;                                 // x position of click
  SHORT       y;                                 // y position of click
  CHAR_W      szStyle[80];                       // style
  CHAR_W      szComment[EQF_SEGLEN];             // comment
} COMMENTDLGIDA, *PCOMMENTDLGIDA;

// start the proposal comment dialog
BOOL EQFBShowSegmentComment
(
  HWND        hwnd,                    // parent window handle
  PTBDOCUMENT pDoc,                    // document
  PVOID       pvAddInfo,               // additional info data of segment
  SHORT       x,                       // x position of click
  SHORT       y                        // y position of click
)
{
  LONG  Rc;
  PCOMMENTDLGIDA pIda = NULL;

  if ( UtlAlloc((PVOID *) &pIda, 0L, (LONG) sizeof(COMMENTDLGIDA), ERROR_STORAGE ) )
  {
    HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
    pIda->hwndParent = hwnd;
    pIda->pDoc = pDoc;
    pIda->pvAddInfo = pvAddInfo;
    pIda->x = x;
    pIda->y = y;
    DIALOGBOX( EqfQueryTwbClient()/*hwnd*/, EQFBCOMMENTDLGPROC, hResMod, ID_TB_COMMENT_DLG, pIda, Rc );
  } /* endif */

  return( TRUE );
}


// proposal comment dialog
INT_PTR CALLBACK EQFBCOMMENTDLGPROC
(
  HWND hwndDlg,                       /* handle of dialog window             */
  WINMSG msg,
  WPARAM mp1,
  LPARAM mp2
)
{
  PCOMMENTDLGIDA pIda = NULL;
  MRESULT  mResult = MRFROMSHORT( FALSE );      // result value of procedure

  switch ( msg )
  {
    case WM_EQF_QUERYID: HANDLEQUERYID( ID_TB_COMMENT_DLG, mp2 ); break;

    case WM_INITDLG:
      {
        HADDDATAKEY hKey;
        RECT  rect;
        pIda = (PCOMMENTDLGIDA)mp2;
        SETWINDOWID( hwndDlg, ID_TB_COMMENT_DLG );
        ANCHORDLGIDA( hwndDlg, pIda );
        SetCtrlFnt( hwndDlg, GetCharSet(), ID_TB_COMMENT_MLE, 0 );
        hKey = MADSearchKey( (PSZ_W)pIda->pvAddInfo, L"Note" );
        if ( hKey != NULL )
        {
          SETTEXTEX       SetTextOption;
          MADGetAttr( hKey, L"style", pIda->szStyle, sizeof(pIda->szStyle) / sizeof(CHAR_W), L"" );
          SETTEXTW( hwndDlg, ID_TB_COMMENT_STYLE_EF, pIda->szStyle );
          MDAGetValueForKey( hKey, pIda->szComment, sizeof(pIda->szComment) / sizeof(CHAR_W), L"" );
          SetTextOption.codepage = 1200;
          SetTextOption.flags = ST_DEFAULT;
          SendDlgItemMessage( hwndDlg, ID_TB_COMMENT_MLE, EM_SETTEXTEX, (WPARAM)&SetTextOption, (LPARAM)pIda->szComment );
        } /* endif */
        SETFOCUS( hwndDlg, ID_TB_COMMENT_OK_PB );
        GetWindowRect( pIda->hwndParent, &rect );
        SetWindowPos( hwndDlg, HWND_TOP, pIda->x + rect.left, pIda->y + rect.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_SHOWWINDOW );
      }
      break;

    case WM_COMMAND:
      pIda = ACCESSDLGIDA( hwndDlg, PCOMMENTDLGIDA );
      switch ( WMCOMMANDID( mp1, mp2 ) )
      {
        case ID_TB_COMMENT_OK_PB:
        case DID_CANCEL:
          WinPostMsg( hwndDlg, WM_CLOSE, 0L, 0L );
          break;
        default:
          break;
      }
      break;

    case WM_CLOSE:
      DelCtrlFont( hwndDlg, ID_TB_COMMENT_MLE );
      WinDismissDlg( hwndDlg, TRUE );
      break;

    case WM_DESTROY:
      pIda = ACCESSDLGIDA( hwndDlg, PCOMMENTDLGIDA );
      if ( pIda ) UtlAlloc( (PVOID *) &pIda, 0L, 0L, NOMSG );
      break;

    default:
      mResult = FALSE;
      break;
  } /* endswitch */

  return mResult;
} /* end of EQFBCommentDlgProc */

BOOL CheckDupCharW(const wchar_t * wstrTarChar, HWND hwndDlg)
{
    BOOL bDuplicate = FALSE;

    int nCnt = QUERYITEMCOUNT(hwndDlg, ID_TB_KEYS_FUNCTION_LB);
    for (int iInx = 0; iInx < nCnt; iInx++)
    {
        wchar_t wstrItm[MAX_BUF_SIZE];
        memset(wstrItm, 0x00, sizeof(wstrItm));
        QUERYITEMTEXTW(hwndDlg, ID_TB_KEYS_FUNCTION_LB, iInx, wstrItm);

        if ((NULL == wstrItm) || wcslen(wstrItm) == 0)
        {
            continue;
        }

        // skip string not start with "Insert character"
        if (wcsnicmp(wstrItm, STR_TITLE_INSERT_CHAR_W, wcslen(STR_TITLE_INSERT_CHAR_W)) != 0)
        {
            continue;
        }

        bDuplicate = CompareSpecCharW(wstrTarChar, wstrItm, FALSE, FALSE);
        if (bDuplicate)
        {
            return bDuplicate;
        }
    }

    return bDuplicate;
}
