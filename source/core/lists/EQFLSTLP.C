/*! \file
	Built-in list processors of the list handler

	Copyright Notice:

	Copyright (C) 1990-2016, International Business Machines
	Corporation and others. All rights reserved
*/

#define INCL_EQF_MORPH            // morphologic functions
#define INCL_EQF_LIST             // terminology list functions
#include <eqf.h>                  // General Translation Manager include file

#include "eqflp.h"                // Defines for generic list handlers
#include "eqflisti.h"             // Private List Handler defines

#include "eqflist.id"                  // List Handler IDs

#include "eqfutbtn.h"                  // Button utilities

typedef struct _NEWTERMSDATA
{
  CHAR     szListType[MAX_EQF_PATH];   // buffer for list type name
  CHAR     szBuffer[256];              // multi-purpose buffer
  CHAR     szItemBuffer[512];          // buffer for listbox items
  OBJNAME  szObjBuffer;                // buffer for list object names
  CHAR     szPathBuffer[CCHMAXPATH];   // buffer for path names
} NEWTERMSDATA, *PNEWTERMSDATA;



#define IDFORVIEW( view )                               \
   ( view == NOTMARKED_FLAG ) ? ID_LISTWORK_MKNOT_PB :  \
   ( view == DELMARK_FLAG )   ? ID_LISTWORK_MKDEL_PB :  \
   ( view == EXCLMARK_FLAG )  ? ID_LISTWORK_MKEXC_PB :  \
   ( view == DICTMARK_FLAG )  ? ID_LISTWORK_MKDCT_PB : ID_LISTWORK_MKALL_PB

#define VIEWFORID( id )                               \
   ( id == ID_LISTWORK_MKNOT_PB) ? NOTMARKED_FLAG  :  \
   ( id == ID_LISTWORK_MKDEL_PB) ? DELMARK_FLAG    :  \
   ( id == ID_LISTWORK_MKEXC_PB) ? EXCLMARK_FLAG   :  \
   ( id == ID_LISTWORK_MKDCT_PB) ? DICTMARK_FLAG   : VIEWALL_FLAG

BOOL LstSetMLEText
(
PSZ_W       pszText,                 // pointer to text to set
HWND        hwndMLE                  // handle of MLE
);
BOOL LstEditSaveList
(
HWND        hwnd,                    // handle of edit list dialog window
PLISTEDITIDA pIda                    // pointer to dialog IDA
);

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     LSTWORKWITHLISTSDLG
//------------------------------------------------------------------------------
// Function call:     LSTWORKWITHLISTSDLG( HWND  hwnd, USHORT msg,
//                                         MPARAM mp1, MPARAM mp2 );
//------------------------------------------------------------------------------
// Description:       Process messages for the work with list dialog and
//                    perform all required functions.
//------------------------------------------------------------------------------
// Input parameter:  HWND    hwnd     handle of window
//                   USHORT  msg      type of message
//                   MPARAM  mp1      first message parameter
//                   MPARAM  mp2      second message parameter
//------------------------------------------------------------------------------
// Returncode type:  MRESULT
//------------------------------------------------------------------------------
// Returncodes:      depends on message type
//                   normal return codes are:
//                   TRUE  = message has been processed
//                   FALSE = message has not been processed
//------------------------------------------------------------------------------
// Function flow:     switch msg
//                      case WM_INITDLG:
//                        anchor IDA
//                        fill-in title bar text
//                        load mark bitmaps
//                        subclass view mode buttons
//                        set initial view mode to VIEW_ALL
//                        fill exclusion list combo box
//                        read list file
//                        fill terms list box
//                        set enable state of mark pushbuttons
//                        dismiss the dialog if initialization failed
//                      case WM_MEASUREITEM:
//                        return list box item height
//                      case WM_DRAWITEM:
//                        address IDA
//                        get pointer to term data
//                        clear item rectangle
//                        draw mark bitmap if any
//                        draw item text
//                      case WM_CONTROL:
//                        switch control ID
//                          case exclusion list box
//                            if entry field of combo box contains data and
//                               terms are selected in terms list box then
//                              enable exclusion list button
//                            else
//                              disable exclusion list button
//                            endif
//                          case term list box selections
//                            if items are selected in list box then
//                              enable mark pushbuttons
//                            else
//                              disable mark pushbuttons
//                            endif
//                        endswitch
//                      case WM_COMMAND:
//                        switch command value
//                          case DID_CANCEL:
//                          case cancel pushbutton:
//                            post close message to end the dialog
//                          case unmark pushbutton:
//                          case mark for delete pushbutton:
//                            loop over all selected terms
//                              get pointer to term data
//                              set approbriate mark flag
//                              deselect or delete list box item
//                            endloop
//                            disable mark pushbuttons
//                          case exclusion list pushbutton
//                            prepare selected exlusion list name
//                            loop over all selected terms
//                              get pointer to term data
//                              set exclusion list mark flag and excl. name
//                              deselect or delete list box item
//                            endloop
//                            disable mark pushbuttons
//                          case mark for dictionary pushbutton
//                            call mark for dictionary dialog
//                          case view mode pushbutton
//                            hilite new view mode button
//                            refresh terms list box
//                            disable mark pushbuttons
//                          case save pushbutton:
//                            set flags for currently selected terms
//                            call LstWriteSGMLList to save the list
//                          case process marks pushbutton:
//                            call LstWorkProcessMarks to process the marks
//                        endswitch
//                      case WM_CLOSE:
//                        if list has been changed then
//                          get user confirmation
//                          switch user decision
//                            case 'YES'
//                              save list
//                              dismiss dialog
//                            case 'NO'
//                              dismiss dialog
//                            default
//                              stay in dialog
//                          endswitch
//                        else
//                          dismiss the dialog
//                        endif
//                      case WM_DESTROY
//                        free areas for loaded list
//                        delete bitmaps
//                      default
//                        pass message to default dialog procedure
//                    endswitch
//------------------------------------------------------------------------------
INT_PTR CALLBACK LSTWORKWITHLISTSDLG
(
HWND   hwnd,
WINMSG msg,
WPARAM mp1,
LPARAM mp2
)
{
  MRESULT      mResult = FALSE;        // function return code
  PWWLDLGIDA   pIda;                   // pointer to dialog IDA
  SHORT        sItem;                  // index of list box items
  SHORT        sItems;                 // number of list box items
  HBITMAP      hbm;                    // handle of selected bitmap
  USHORT       usI;                    // general loop index
  PTERM        pTerm;                  // pointer to term data
  USHORT       usRC;                   // return code of called functions
  BOOL         fOK;                    // internal OK flag
  PSZ          pszParm;                // pointer to error message parameter
  HPROP        hDicProp;               // handle of dict properties
  PPROPDICTIONARY pDicProp;            // ptr to dict props
  EQFINFO      ErrorInfo;              // error return code

  switch ( msg )
  {
    case WM_EQF_QUERYID:
      pIda = ACCESSDLGIDA( hwnd, PWWLDLGIDA );
      if ( pIda->usListType == NTL_TYPE )
      {
        HANDLEQUERYID( ID_LISTWORK_NTL_DLG, mp2 );
      }
      else
      {
        HANDLEQUERYID( ID_LISTWORK_FTL_DLG, mp2 );
      } /* endif */
      break;

    case WM_INITDLG :
      {
        HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);

        /****************************************************************/
        /* Anchor IDA                                                   */
        /****************************************************************/
        usRC = NO_ERROR;
        pIda = (PWWLDLGIDA)mp2;
        pIda->hwnd = hwnd;
        ANCHORDLGIDA( hwnd, pIda );

        /****************************************************************/
        /* Switch dialog ID to allow different helps for the dialog     */
        /****************************************************************/
        if ( pIda->usListType == NTL_TYPE )
        {
          SETWINDOWID( hwnd, ID_LISTWORK_NTL_DLG );
        }
        else
        {
          SETWINDOWID( hwnd, ID_LISTWORK_FTL_DLG );
        } /* endif */

        /****************************************************************/
        /* Set text limits                                              */
        /****************************************************************/
        CBSETTEXTLIMIT( hwnd, ID_LISTWORK_EXCL_CB, MAX_FNAME - 1 );

        /****************************************************************/
        /* Fill-in title bar text                                       */
        /****************************************************************/
        LOADSTRING( NULLHANDLE, hResMod, ( pIda->usListType == NTL_TYPE ) ?
                    SID_LSTWORK_NTL_TITLE_TEXT  :
                    SID_LSTWORK_FTL_TITLE_TEXT, pIda->szBuffer );
        pszParm = pIda->szListName;
        {
          ULONG Length;

          DosInsMessage( &pszParm, 1, pIda->szBuffer,
                         (strlen( pIda->szBuffer ) + 1),
                         pIda->szBuffer2,
                         sizeof( pIda->szBuffer2 ), &Length );
        }
        SETTEXTHWND( hwnd, pIda->szBuffer2 );

        /****************************************************************/
        /* Load bitmaps and strings used as marks in term listbox       */
        /****************************************************************/
        LOADSTRING( NULLHANDLE, hResMod, SID_LISTWORK_MKNOT_PB, pIda->szNotMarkedText );
        LOADSTRING( NULLHANDLE, hResMod, SID_LISTWORK_MKALL_PB, pIda->szAllMarkedText );

        pIda->hbmDict   = LoadBitmap( hResMod, MAKEINTRESOURCE(DICT_BMP) );
        pIda->hbmDel    = LoadBitmap( hResMod, MAKEINTRESOURCE(DELETE_BMP) );
        pIda->hbmExcl   = LoadBitmap( hResMod, MAKEINTRESOURCE(EXCL_BMP) );
        pIda->hbmNoMark = LoadBitmap( hResMod, "Unmarked" );
        pIda->hbmDictG   = LoadBitmap( hResMod, "Dictionary");
        pIda->hbmDelG    = LoadBitmap( hResMod, "Deleted" );
        pIda->hbmExclG   = LoadBitmap( hResMod, "Excluded" );
        pIda->hbmNoMarkG = LoadBitmap( hResMod, MAKEINTRESOURCE(NOMARKG_BMP) );
       // pIda->hbmAllG    = LoadBitmap( hResMod, MAKEINTRESOURCE(ALL_BMP) );
        pIda->hbmAllG    = LoadBitmap( hResMod, "All");
        /****************************************************************/
        /* Set initial view mode                                        */
        /****************************************************************/
        pIda->usView = VIEWALL_FLAG;       // switch to view all mode

        /****************************************************************/
        /* Fill exclusion list combobox                                 */
        /****************************************************************/
        EqfSend2Handler( LISTHANDLER, WM_EQF_INSERTNAMES,
                         MP1FROMHWND( WinWindowFromID( hwnd, ID_LISTWORK_EXCL_CB) ),
                         MP2FROMP( EXCLUSIONLISTOBJ ) );

        /****************************************************************/
        /* Create dictionary name dummy box                             */
        /****************************************************************/
        if ( usRC == NO_ERROR )
        {
          pIda->hwndDictLB = WinCreateWindow( hwnd, WC_LISTBOX,
                                              "",
                                              WS_CHILD | LBS_STANDARD,
                                              0, 0, 10, 10,
                                              hwnd, HWND_TOP,
                                              1, NULL, NULL );
          if ( !pIda->hwndDictLB )
          {
            UtlErrorHwnd( 0, MB_CANCEL, 0, NULL, SYSTEM_ERROR, hwnd );
            usRC = ERROR_INVALID_FUNCTION;
          } /* endif */
        } /* endif */

        /****************************************************************/
        /* End dialog in case of errors during initialization or start  */
        /* second part of initialization                                */
        /****************************************************************/
        if ( usRC )
        {
          WinDismissDlg( hwnd, FALSE );
        }
        else
        {
          ShowWindow( hwnd, SW_SHOW );

          WinPostMsg( hwnd, WM_EQF_INITIALIZE, NULL, NULL );
        } /* endif */
      }
      break;

    case WM_EQF_INITIALIZE:
      /****************************************************************/
      /* Get pointer to IDA                                           */
      /****************************************************************/
      pIda = ACCESSDLGIDA( hwnd, PWWLDLGIDA );
      pIda->fDisabled = TRUE;

      /*******************************************************************/
      /* switch to hourglass pointer                                     */
      /*******************************************************************/
      SETCURSOR( SPTR_WAIT );

      /****************************************************************/
      /* Fill dictionary name dummy list box                          */
      /****************************************************************/
      EqfSend2Handler( DICTIONARYHANDLER, WM_EQF_INSERTNAMES, MP1FROMHWND( pIda->hwndDictLB ), 0L );

      /****************************************************************/
      /* Remove protected or copyrighted dictionaries from listbox    */
      /****************************************************************/
      sItems = QUERYITEMCOUNTHWND( pIda->hwndDictLB );
      sItem  = 0;
      while ( sItem < sItems )
      {
        BOOL fIsNew = FALSE;
        CHAR szShortName[MAX_FILESPEC];

        fOK = FALSE;                   // remove name from listbox per default

        /**************************************************************/
        /* Get dictionary name                                        */
        /**************************************************************/
        QUERYITEMTEXTHWND( pIda->hwndDictLB, sItem, pIda->szBuffer );
        ANSITOOEM( pIda->szBuffer );

        ObjLongToShortName( pIda->szBuffer , szShortName, DICT_OBJECT, &fIsNew );

        /**************************************************************/
        /* Get dictionary properties                                  */
        /**************************************************************/
        UtlMakeEQFPath( pIda->szBuffer2, NULC, SYSTEM_PATH, NULL );
        strcat( pIda->szBuffer2, BACKSLASH_STR );
        strcat( pIda->szBuffer2, szShortName );
        strcat( pIda->szBuffer2, EXT_OF_DICPROP );
        hDicProp = OpenProperties( pIda->szBuffer2, NULL, PROP_ACCESS_READ, &ErrorInfo);
        if ( hDicProp )
        {
          pDicProp = (PPROPDICTIONARY) MakePropPtrFromHnd( hDicProp );

          if ( !pDicProp->fCopyRight && !pDicProp->fProtected )
          {
            fOK = TRUE;                 // Dictionary is available for add
          } /* endif */
          CloseProperties( hDicProp, PROP_QUIT, &ErrorInfo );
        } /* endif */

        /**************************************************************/
        /* Delete from listbox if dictionary is protected or          */
        /* copyrighted                                                */
        /**************************************************************/
        if ( fOK )
        {
          sItem++;                     // check next one
        }
        else
        {
          DELETEITEMHWND( pIda->hwndDictLB, sItem );
          sItems--;
        } /* endif */
      } /* endwhile */

      /****************************************************************/
      /* Read list file                                               */
      /****************************************************************/
      usRC = LstReadSGMLList( pIda->szListPath, &pIda->ListHeader,
                              &pIda->pTermTable, &pIda->pContextTable,
                              &pIda->pPool, FALSE, (LISTTYPES)pIda->usListType, SGMLFORMAT_UNICODE);
      pIda->fListChanged = FALSE;

      /****************************************************************/
      /* Fill terms listbox                                           */
      /****************************************************************/
      if ( !usRC )
      {
        LstWorkFillTermsLB( pIda );
      } /* endif */

      /********************************************************************/
      /* Force setting of mark button enable state by posting a WM_CONTROL */
      /* message for the term list box with a notification code of        */
      /* LN_SELECT.                                                       */
      /********************************************************************/
      if ( !usRC )
      {
        SetCtrlFnt(hwnd, GetCharSet(), ID_LISTWORK_TERM_LB, 0 );
        SENDNOTIFICATION( hwnd, ID_LISTWORK_TERM_LB, LN_SELECT );
      } /* endif */

      /*******************************************************************/
      /* restore old mouse pointer                                       */
      /*******************************************************************/
      pIda->fDisabled = FALSE;
      SETCURSOR( SPTR_ARROW );

      /****************************************************************/
      /* Leave dialog in case of errors                               */
      /****************************************************************/
      if ( usRC )
      {
        WinDismissDlg( hwnd, FALSE );
      } /* endif */
      break;

    case WM_MEASUREITEM:
      {
        LPMEASUREITEMSTRUCT pMIS = (LPMEASUREITEMSTRUCT)PVOIDFROMMP2(mp2);
        SWP                 swp;

        WinQueryWindowPos( GetDlgItem( hwnd, pMIS->CtlID ), &swp );
        pMIS->itemWidth  = swp.cx;
        pMIS->itemHeight = (USHORT)UtlQueryULong( QL_CHARHEIGHT ) + 4;
        mResult = TRUE;
      }

      break;

    case WM_DRAWITEM:                // draw a listbox item
      {
        /**********************************************************/
        /* draw listbox item or button                            */
        /**********************************************************/
        LPDRAWITEMSTRUCT lpDisp = (LPDRAWITEMSTRUCT)mp2;

        switch ( lpDisp->CtlType )
        {
          case ODT_LISTBOX :
            if ( (lpDisp->itemID != -1) )
            {
              pIda = ACCESSDLGIDA( hwnd, PWWLDLGIDA );
              pTerm = (PTERM)QUERYITEMHANDLE( hwnd, lpDisp->CtlID, lpDisp->itemID );

              if ( lpDisp->itemAction & (ODA_DRAWENTIRE | ODA_SELECT | ODS_FOCUS) )
              {
                RECT         rect;     // drawing rectangle
                POINTL       pt;
                /**************************************************************/
                /* Get a copy of the item rectangle                           */
                /**************************************************************/
                memcpy( &rect, &lpDisp->rcItem, sizeof(rect) );

                /***********************************************************/
                /* Draw item                                               */
                /***********************************************************/
                if (!UtlIsHighContrast())
				{
                  FillRect( lpDisp->hDC, &lpDisp->rcItem, (HBRUSH) (COLOR_WINDOW+1) );
			    }
			    else
			    {
					FillRect( lpDisp->hDC, &lpDisp->rcItem, (HBRUSH) (COLOR_HIGHLIGHT) );
			    }


                switch ( pTerm->Flags.fMark  )
                {
                  case DELMARK_FLAG :
                    hbm = pIda->hbmDel;
                    break;
                  case EXCLMARK_FLAG :
                    hbm = pIda->hbmExcl;
                    break;
                  case DICTMARK_FLAG :
                    hbm = pIda->hbmDict;
                    break;
                  case NOTMARKED_FLAG :
                  default:
                    hbm = pIda->hbmNoMark;
                    break;
                } /* endswitch */

                if ( hbm )
                {
                  pt.x = rect.left;
                  pt.y = rect.top;
                  WinDrawBitmap( lpDisp->hDC, hbm, NULL, &pt,
                                 GetSysColor(COLOR_WINDOWTEXT), GetSysColor(COLOR_WINDOW), DBM_NORMAL );
                } /* endif */
                rect.left += 22L;

                DRAWTEXTW(lpDisp->hDC, pTerm->pszName, rect,
				           GetSysColor(COLOR_WINDOWTEXT),
				           GetSysColor(COLOR_WINDOW),
				           DT_LEFT | DT_NOPREFIX);
                if (  lpDisp->itemState & ODS_SELECTED  )
                {
					if (!UtlIsHighContrast())
					{
                      InvertRect( lpDisp->hDC, &lpDisp->rcItem );
				    }
				    else
				    {
                       WinDrawBitmap( lpDisp->hDC, hbm, NULL, &pt,
                             GetSysColor(COLOR_HIGHLIGHTTEXT), GetSysColor(COLOR_HIGHLIGHT), DBM_NORMAL );
					   DRAWTEXTW(lpDisp->hDC, pTerm->pszName, rect,
				                 GetSysColor(COLOR_HIGHLIGHTTEXT),
				                 GetSysColor(COLOR_HIGHLIGHT),
				                 DT_LEFT | DT_NOPREFIX);
				    }
                } /* endif */
              } /* endif */
              if (lpDisp->itemState & (ODS_FOCUS|ODS_SELECTED)
                  && (lpDisp->itemAction | (ODA_FOCUS)))
			      DrawFocusRect(lpDisp->hDC, &lpDisp->rcItem);

            } /* endif */

            /**********************************************************/
            /* let windows do the dirty work of displaying the frame..*/
            /**********************************************************/
            mResult = WinDefWindowProc( hwnd, msg, mp1, mp2 );
            break;

          case ODT_BUTTON :
            if ( lpDisp->itemAction & (ODA_DRAWENTIRE | ODA_SELECT) )
            {
              HBITMAP     hbmBitmap = NULL;   // handle of bitmap to paint
              BOOL        fHighlite = FALSE;  // TRUE = hilite button
              PSZ         pszText = NULL;     // ptr to text being painted

              pIda = ACCESSDLGIDA( hwnd, PWWLDLGIDA );
              switch ( lpDisp->CtlID )
              {
                case ID_LISTWORK_MKALL_PB       :
                fHighlite = pIda->usView == VIEWALL_FLAG;
                  hbmBitmap = pIda->hbmAllG;
                  pszText   = NULL;
                  break;
                case ID_LISTWORK_MKNOT_PB   :
                  fHighlite = pIda->usView == NOTMARKED_FLAG;
                  hbmBitmap = pIda->hbmNoMarkG;
                  pszText   = NULL;
                  break;
                case ID_LISTWORK_MKDEL_PB   :
                  fHighlite = pIda->usView == DELMARK_FLAG;
                  hbmBitmap = pIda->hbmDelG;
                  pszText   = NULL;
                  break;
                case ID_LISTWORK_MKEXC_PB  :
                  fHighlite = pIda->usView == EXCLMARK_FLAG;
                  hbmBitmap = pIda->hbmExclG;
                  pszText   = NULL;
                  break;
                case ID_LISTWORK_MKDCT_PB  :
                  fHighlite = pIda->usView == DICTMARK_FLAG;
                  hbmBitmap = pIda->hbmDictG;
                  pszText   = NULL;
                  break;
              } /* endswitch */

              FillRect( lpDisp->hDC, &lpDisp->rcItem, (HBRUSH)(COLOR_BTNFACE + 1) );

              if ( hbmBitmap )
              {
                POINTL  pt;

                pt.x = lpDisp->rcItem.left;
                pt.y = lpDisp->rcItem.top;
                WinDrawBitmap( lpDisp->hDC, hbmBitmap, NULL, &pt,
                               SYSCLR_WINDOWTEXT, COLOR_BTNFACE, DBM_NORMAL );
              }
              else if ( pszText )
              {
                DrawText( lpDisp->hDC, pszText, -1,
                          &lpDisp->rcItem,
                          DT_LEFT | DT_NOPREFIX );
              } /* endif */

              if ( fHighlite )
              {
                InvertRect( lpDisp->hDC, &lpDisp->rcItem );
              } /* endif */


            } /* endif */
            if (lpDisp->itemState & (ODS_FOCUS | ODS_SELECTED))
			      DrawFocusRect(lpDisp->hDC, &lpDisp->rcItem);
            break;
        } /* endswitch */
      }
      break;

    case WM_COMMAND :
      pIda = ACCESSDLGIDA( hwnd, PWWLDLGIDA );
      if ( pIda->fDisabled )
      {
        WinAlarm( HWND_DESKTOP, WA_ERROR );
      }
      else
      {
        switch (WMCOMMANDID( mp1, mp2 ))
        {
		  case ID_LISTWORK_HELP_PB:
		    mResult = UtlInvokeHelp();
		    break;
          case DID_CANCEL:
          case ID_LISTWORK_CANCEL_PB:
            WinPostMsg( hwnd, WM_EQF_CLOSE, 0L, 0L );
            break;

          case ID_LISTWORK_UNMARK_PB:
          case ID_LISTWORK_DELMARK_PB:
            sItem = LIT_FIRST;
            while ( (sItem = QUERYNEXTSELECTION( hwnd,
                                                 ID_LISTWORK_TERM_LB,
                                                 sItem )) != LIT_NONE )
            {
              pTerm = (PTERM)QUERYITEMHANDLE( hwnd, ID_LISTWORK_TERM_LB, sItem );

              switch (SHORT1FROMMP1( mp1))
              {
                case ID_LISTWORK_UNMARK_PB:
                  pTerm->Flags.fMark = NOTMARKED_FLAG;
                  pIda->fListChanged = TRUE;
                  break;
                case ID_LISTWORK_DELMARK_PB:
                  pTerm->Flags.fMark = DELMARK_FLAG;
                  pIda->fListChanged = TRUE;
                  break;
              } /* endswitch */

              if ( (pIda->usView == VIEWALL_FLAG) ||
                   (pTerm->Flags.fMark == pIda->usView) )
              {
                SETITEMTEXTW( hwnd, ID_LISTWORK_TERM_LB, sItem, pTerm->pszName );
              }
              else
              {
                DELETEITEM( hwnd, ID_LISTWORK_TERM_LB, sItem );
                sItem = ( sItem == 0 ) ? LIT_FIRST : sItem - 1;
              } /* endif */
            } /* endwhile */
            ENABLECTRL( hwnd, ID_LISTWORK_UNMARK_PB,   FALSE );
            ENABLECTRL( hwnd, ID_LISTWORK_DELMARK_PB,  FALSE );
            ENABLECTRL( hwnd, ID_LISTWORK_DICTMARK_PB, FALSE );
            ENABLECTRL( hwnd, ID_LISTWORK_EXCLMARK_PB, FALSE );

            SendDlgItemMessage( hwnd, ID_LISTWORK_TERM_LB, LB_SETSEL,
                                MP1FROMSHORT(FALSE), MP2FROMSHORT(-1) );
            SETFOCUS( hwnd, ID_LISTWORK_TERM_LB );
            break;

          case ID_LISTWORK_EXCLMARK_PB:
            fOK = TRUE;

            /************************************************************/
            /* Prepare selected exclusion list name                     */
            /************************************************************/
            QUERYTEXT( hwnd, ID_LISTWORK_EXCL_CB, pIda->szBuffer );
            UtlStripBlanks( pIda->szBuffer );
            strupr( pIda->szBuffer );
            if ( !pIda->szBuffer[0] )
            {
              UtlErrorHwnd( ERROR_LST_MARK_NO_NAME, MB_CANCEL, 0, NULL, EQF_ERROR, hwnd );
              SETFOCUS( hwnd, ID_LISTWORK_EXCL_CB );
              fOK = FALSE;
            } /* endif */

            if ( fOK )
            {
              /************************************************************/
              /* Check if list name is valid (=exists of alphanumeric     */
              /* characters only)                                         */
              /************************************************************/
              if ( fOK  )
              {
                pszParm = pIda->szBuffer;
                while ( *pszParm && isalnum(*pszParm) )
                {
                  pszParm++;
                } /* endwhile */
                if ( *pszParm != EOS )
                {
                  pszParm = pIda->szBuffer;
                  UtlErrorHwnd( ERROR_LST_NAME_INVALID, MB_CANCEL, 1,
                                &pszParm, EQF_ERROR , hwnd );
                  SETFOCUS( hwnd, ID_LISTWORK_EXCL_CB );
                  fOK = FALSE;
                } /* endif */
              } /* endif */
            } /* endif */

            if ( fOK )
            {
              if ( !pIda->pszExclList ||
                   ( strcmp( pIda->pszExclList, pIda->szBuffer ) != 0 ) )
              {
                pIda->pszExclList = PoolAddString( pIda->pPool,
                                                   pIda->szBuffer );
                if ( !pIda->pszExclList  )
                {
                  UtlErrorHwnd( ERROR_STORAGE, MB_CANCEL, 0, NULL, EQF_ERROR , hwnd );
                  fOK = FALSE;
                } /* endif */
              } /* endif */
            } /* endif */

            /************************************************************/
            /* Mark selected terms for exclusion list                   */
            /************************************************************/
            if ( fOK )
            {
              sItem = LIT_FIRST;
              while ( (sItem = QUERYNEXTSELECTION( hwnd,
                                                   ID_LISTWORK_TERM_LB,
                                                   sItem )) != LIT_NONE )
              {
                pTerm = (PTERM)QUERYITEMHANDLE( hwnd, ID_LISTWORK_TERM_LB, sItem );

                pTerm->Flags.fMark = EXCLMARK_FLAG;
                pIda->fListChanged = TRUE;
                pTerm->pszDestination = pIda->pszExclList;
                if ( (pIda->usView == VIEWALL_FLAG) ||
                     (pTerm->Flags.fMark == pIda->usView) )
                {
                  SETITEMTEXTW( hwnd, ID_LISTWORK_TERM_LB, sItem, pTerm->pszName );
                }
                else
                {
                  DELETEITEM( hwnd, ID_LISTWORK_TERM_LB, sItem );
                  sItem = ( sItem == 0 ) ? LIT_FIRST : sItem - 1;
                } /* endif */
              } /* endwhile */
              ENABLECTRL( hwnd, ID_LISTWORK_UNMARK_PB,   FALSE );
              ENABLECTRL( hwnd, ID_LISTWORK_DELMARK_PB,  FALSE );
              ENABLECTRL( hwnd, ID_LISTWORK_DICTMARK_PB, FALSE );
              ENABLECTRL( hwnd, ID_LISTWORK_EXCLMARK_PB, FALSE );

              SendDlgItemMessage( hwnd, ID_LISTWORK_TERM_LB, LB_SETSEL,
                                  MP1FROMSHORT(FALSE), MP2FROMSHORT(-1) );

              SETFOCUS( hwnd, ID_LISTWORK_TERM_LB );
            } /* endif */
            break;

          case ID_LISTWORK_DICTMARK_PB:
            {
              HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
              DIALOGBOX( hwnd, LSTMARKDICTDLG, hResMod, ID_LISTMARK_NTL_DLG,
                         pIda, fOK );
              SETFOCUS( hwnd, ID_LISTWORK_TERM_LB );
            }
            break;

          case ID_LISTWORK_MKALL_PB:
          case ID_LISTWORK_MKNOT_PB:
          case ID_LISTWORK_MKDEL_PB:
          case ID_LISTWORK_MKEXC_PB:
          case ID_LISTWORK_MKDCT_PB:
            usI = pIda->usView;
            pIda->usView = VIEWFORID( SHORT1FROMMP1(mp1) );

            /**********************************************************/
            /* Invalidate pushbuttons (correct style for drawing      */
            /* is taken from pIda->usView)                            */
            /**********************************************************/
            InvalidateRgn( GetDlgItem( hwnd, IDFORVIEW( usI ) ), NULL, FALSE );
            InvalidateRgn( GetDlgItem( hwnd, SHORT1FROMMP1(mp1) ), NULL, FALSE );

            pIda->fDisabled  = TRUE;
            LstWorkFillTermsLB( pIda );
            pIda->fDisabled  = FALSE;
            ENABLECTRL( hwnd, ID_LISTWORK_UNMARK_PB, FALSE );
            ENABLECTRL( hwnd, ID_LISTWORK_DELMARK_PB, FALSE );
            ENABLECTRL( hwnd, ID_LISTWORK_DICTMARK_PB, FALSE );
            ENABLECTRL( hwnd, ID_LISTWORK_EXCLMARK_PB, FALSE );
            SETFOCUS( hwnd, ID_LISTWORK_TERM_LB );
            break;

          case ID_LISTWORK_SAVE_PB:
            /************************************************************/
            /* Set term selected flags and top-index flag               */
            /************************************************************/
            pIda->fDisabled  = TRUE;
            LstWorkSetSelFlags( pIda );

            /************************************************************/
            /* Write the list                                           */
            /************************************************************/
            usRC = LstWriteSGMLList( pIda->szListPath,
                                     &pIda->ListHeader,
                                     pIda->pTermTable,
                                     pIda->pContextTable,
                                     FALSE,
                                     (LISTTYPES)pIda->usListType, SGMLFORMAT_UNICODE );

            /************************************************************/
            /* Clear term selected flags and top-index flag             */
            /************************************************************/
            LstWorkClearSelFlags( pIda );

            /**********************************************************/
            /* Handle return code of LstWriteSGMLList function        */
            /**********************************************************/
            if ( usRC )
            {
              pIda->fDisabled  = FALSE;
            }
            else
            {
              /**********************************************************/
              /* Show list save information                             */
              /**********************************************************/
              pszParm = pIda->szListName;
              UtlErrorHwnd( INFO_LST_LIST_SAVED, MB_OK, 1, &pszParm, EQF_INFO , hwnd );
              pIda->fListChanged = FALSE;

              /**********************************************************/
              /* remove the dialog                                      */
              /**********************************************************/

              DelCtrlFont(hwnd, ID_LISTWORK_TERM_LB );

              WinDismissDlg( hwnd, FALSE );
            } /* endif */


            break;

          case ID_LISTWORK_PROCMARK_PB:
            LstWorkProcessMarks( pIda );
            break;


          case ID_LISTWORK_EXCL_CB :
          case ID_LISTWORK_TERM_LB :
            mResult = LstWWLControl( hwnd, WMCOMMANDID( mp1, mp2 ),
                                     WMCOMMANDCMD( mp1, mp2 ) );
            break;

        } /* endswitch */
      } /* endif */
      break;

    case WM_EQF_CLOSE:
      pIda = ACCESSDLGIDA( hwnd, PWWLDLGIDA );
      if ( pIda->fListChanged )
      {
        pszParm = pIda->szListName;
        switch ( UtlErrorHwnd( ERROR_LST_LIST_UPDATED,
                               MB_YESNOCANCEL,
                               1,
                               &pszParm,
                               EQF_QUERY , hwnd ) )
        {
          case MBID_YES :
            /**********************************************************/
            /* Save the list without completed message                */
            /**********************************************************/
            LstWorkSetSelFlags( pIda );
            usRC = LstWriteSGMLList( pIda->szListPath,
                                     &pIda->ListHeader,
                                     pIda->pTermTable,
                                     pIda->pContextTable,
                                     FALSE,
                                     (LISTTYPES)pIda->usListType, SGMLFORMAT_UNICODE );

            /**********************************************************/
            /* remove the dialog                                      */
            /**********************************************************/
            if ( !usRC )
            {
              DelCtrlFont(hwnd, ID_LISTWORK_TERM_LB );
              WinDismissDlg( hwnd, FALSE );
            } /* endif */
            break;

          case MBID_NO :

            DelCtrlFont(hwnd, ID_LISTWORK_TERM_LB );
            WinDismissDlg( hwnd, FALSE );
            break;

          default :
            /**********************************************************/
            /* do nothing: stay in dialog                             */
            /**********************************************************/
            break;
        } /* endswitch */
      }
      else
      {
        DelCtrlFont(hwnd, ID_LISTWORK_TERM_LB );
        WinDismissDlg( hwnd, FALSE );
      } /* endif */
      break;

    case WM_DESTROY:
      pIda = ACCESSDLGIDA( hwnd, PWWLDLGIDA );
      if ( pIda != NULL )
      {
        if ( pIda->pTermTable )    LstDestroyTermTable( pIda->pTermTable );
        pIda->pTermTable = NULL;
        if ( pIda->pContextTable ) LstDestroyContextTable( pIda->pContextTable );
        pIda->pContextTable = NULL;
        if ( pIda->pPool )         PoolDestroy( pIda->pPool );
        pIda->pPool = NULL;

        if ( pIda->hbmDict )   DeleteObject( pIda->hbmDict );
        pIda->hbmDict = NULL;
        if ( pIda->hbmExcl )   DeleteObject( pIda->hbmExcl );
        pIda->hbmExcl = NULL;
        if ( pIda->hbmDel  )   DeleteObject( pIda->hbmDel );
        pIda->hbmDel = NULL;
        if ( pIda->hbmNoMark ) DeleteObject( pIda->hbmNoMark );
        pIda->hbmNoMark = NULL;

      } /* endif */
      /****************************************************************/
      /* Do not free IDA (is owned by list processor ...)             */
      /****************************************************************/
      break;

    default :
      mResult = WinDefDlgProc( hwnd, msg, mp1, mp2 );
      break;
  } /* endswitch */

  return( mResult );

} /* end of function LSTWORKWITHLISTSDLG */

/**********************************************************************/
/* WM_CONTROL handling for the LstWorkWithListsDlg dialog             */
/**********************************************************************/
MRESULT LstWWLControl
(
HWND   hwnd,                        // dialog handle
SHORT  sId,                         // id in action
SHORT  sNotification                // notification
)
{
  PWWLDLGIDA   pIda;                   // pointer to dialog IDA

  switch ( sId )
  {
    case ID_LISTWORK_EXCL_CB :
      switch ( sNotification ) // switch on notification code
      {
        case CBN_EFCHANGE :
          pIda = ACCESSDLGIDA( hwnd, PWWLDLGIDA );
          QUERYTEXTW( hwnd, ID_LISTWORK_EXCL_CB, pIda->szUnicodeBuffer );
          UtlStripBlanksW( pIda->szUnicodeBuffer );
          UtlUpperW( pIda->szUnicodeBuffer );
          if ( pIda->szUnicodeBuffer[0] )
          {
            if ( QUERYNEXTSELECTION( hwnd, ID_LISTWORK_TERM_LB, LIT_FIRST ) != LIT_NONE )
            {
              ENABLECTRL( hwnd, ID_LISTWORK_EXCLMARK_PB, TRUE );
            }
            else
            {
              ENABLECTRL( hwnd, ID_LISTWORK_EXCLMARK_PB, FALSE );
            } /* endif */
          }
          else
          {
            ENABLECTRL( hwnd, ID_LISTWORK_EXCLMARK_PB, FALSE );
          } /* endif */
          break;

          /**************************************************************/
          /* case CBN_SELCHANGE only required for Windows as Windows    */
          /* handles selections in listbox differently from changes     */
          /* in the combobox entryfield                                 */
          /**************************************************************/
        case CBN_SELCHANGE :
          {
            SHORT  sItem;              // combobox item index

            pIda = ACCESSDLGIDA( hwnd, PWWLDLGIDA );
            sItem = CBQUERYSELECTION( hwnd, ID_LISTWORK_EXCL_CB );
            if ( sItem != LIT_NONE )
            {
              CBQUERYITEMTEXTW( hwnd, ID_LISTWORK_EXCL_CB, sItem, pIda->szUnicodeBuffer );
            }
            else
            {
              pIda->szUnicodeBuffer[0] = EOS;
            } /* endif */
            UtlStripBlanksW( pIda->szUnicodeBuffer );
            UtlUpperW( pIda->szUnicodeBuffer );
            if ( pIda->szUnicodeBuffer[0] )
            {
              if ( QUERYNEXTSELECTION( hwnd, ID_LISTWORK_TERM_LB, LIT_FIRST ) != LIT_NONE )
              {
                ENABLECTRL( hwnd, ID_LISTWORK_EXCLMARK_PB, TRUE );
              }
              else
              {
                ENABLECTRL( hwnd, ID_LISTWORK_EXCLMARK_PB, FALSE );
              } /* endif */
            }
            else
            {
              ENABLECTRL( hwnd, ID_LISTWORK_EXCLMARK_PB, FALSE );
            } /* endif */
          }
          break;

      } /* endswitch */
      break;

    case ID_LISTWORK_TERM_LB :
      switch ( sNotification )
      {
        case LN_SELECT :
          pIda = ACCESSDLGIDA( hwnd, PWWLDLGIDA );
          if ( QUERYNEXTSELECTION( hwnd, ID_LISTWORK_TERM_LB, LIT_FIRST ) == LIT_NONE )
          {
            ENABLECTRL( hwnd, ID_LISTWORK_UNMARK_PB, FALSE );
            ENABLECTRL( hwnd, ID_LISTWORK_DELMARK_PB, FALSE );
            ENABLECTRL( hwnd, ID_LISTWORK_DICTMARK_PB, FALSE );
            ENABLECTRL( hwnd, ID_LISTWORK_EXCLMARK_PB, FALSE );
          }
          else
          {
            ENABLECTRL( hwnd, ID_LISTWORK_UNMARK_PB, TRUE  );
            ENABLECTRL( hwnd, ID_LISTWORK_DELMARK_PB, TRUE );
            ENABLECTRL( hwnd, ID_LISTWORK_DICTMARK_PB, TRUE );
            QUERYTEXTW( hwnd, ID_LISTWORK_EXCL_CB, pIda->szUnicodeBuffer );
            UtlStripBlanksW( pIda->szUnicodeBuffer );
            if ( pIda->szUnicodeBuffer[0] )
            {
              ENABLECTRL( hwnd, ID_LISTWORK_EXCLMARK_PB, TRUE );
            }
            else
            {
              ENABLECTRL( hwnd, ID_LISTWORK_EXCLMARK_PB, FALSE );
            } /* endif */
          } /* endif */
      } /* endswitch */
      break;
  } /* endswitch */
  return( MRFROMSHORT(FALSE) );
} /* end of LstWWLControl */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     LstWorkFillTermsLB     Fill terms listbox of WWL dialog
//------------------------------------------------------------------------------
// Function call:     LstWorkFillTermsLB( PWWLDLGIDA pIda );
//------------------------------------------------------------------------------
// Description:       Fills the term listbox of the work with lists dialog
//                    with terms which match the currently selected view
//                    option.
//------------------------------------------------------------------------------
// Input parameter:   PWWLDLGIDA   pIda     pointer to WWL dialog IDA
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:     disable window update of listbox
//                    delete all listbox items
//                    while there are term tables
//                      loop over terms in termtable
//                        if current view mode is view all or
//                           current view mode matchs mark stated of term and
//                           term is not deleted then
//                           add term to terms listbox
//                           set item handle to pointer to term data
//                           if selected flag of term is set then
//                             set selected state of list box item
//                           endif
//                           if top item flag of term is set then
//                             remember term as top of list box term
//                           endif
//                        endif
//                        continue with the next term
//                      endloop
//                      continue with the next table
//                    endwhile
//                    if a top item term has been remembered then
//                      search term in listbox and make it the first visible
//                       list box term
//                    endif
//                    enable list box window update
//------------------------------------------------------------------------------
VOID LstWorkFillTermsLB( PWWLDLGIDA pIda )
{
  USHORT usI;                          // table index
  HWND   hwndLB;                       // handle of terms listbox
  SHORT  sItem;                        // index of inserted listbox item
  PTERMTABLE pTermTable;               // ptr to current term table
  PTERM  pTerm;                        // ptr to current term
  PTERM  pTopTerm = NULL;              // ptr to first visible term in listbox
  USHORT usDispatchCounter;            // call UtlDispatch then zero
  BOOL   fItemSelected = FALSE;        // an item has been selected flag
  BOOL   fOK = TRUE;                   // internal OK flag

  /****************************************************************/
  /* Get listbox handle                                           */
  /****************************************************************/
  hwndLB = WinWindowFromID( pIda->hwnd, ID_LISTWORK_TERM_LB );

  /****************************************************************/
  /* Disable listbox update and delete listbox items              */
  /****************************************************************/
  ENABLEUPDATEHWND_FALSE( hwndLB );
  DELETEALLHWND( hwndLB );

  /****************************************************************/
  /* Fill terms listbox                                           */
  /****************************************************************/
  usDispatchCounter = 100;
  pTermTable = pIda->pTermTable;
  while ( pTermTable && fOK )
  {
    pTerm = (PTERM)(pTermTable+1);
    for ( usI = 0; ((usI < pTermTable->usUsedEntries) && fOK); usI++, pTerm++ )
    {
      if (  ( !pTerm->Flags.fDeleted ) &&
            ( ( pIda->usView == VIEWALL_FLAG ) ||
              ( pIda->usView == pTerm->Flags.fMark ) ) )
      {
        UTF16strncpy( pIda->szUnicodeBuffer, pTerm->pszName,
                      sizeof(pIda->szUnicodeBuffer)/sizeof(CHAR_W) - 1 );
        pIda->szUnicodeBuffer[sizeof(pIda->szUnicodeBuffer)/sizeof(CHAR_W)-1] = EOS;

        sItem = (SHORT)INSERTITEMENDHWNDW( hwndLB, pTerm->pszName );
        if ( (sItem == LIT_ERROR) || (sItem == LIT_MEMERROR) )
        {
          /************************************************************/
          /* insert failed, maybe we exceeded the listbox memory      */
          /* restrictions                                             */
          /************************************************************/
          UtlErrorHwnd( ERROR_LST_LBFULL, MB_OK, 0, NULL, EQF_WARNING, pIda->hwnd );
          fOK = FALSE;
        }
        else
        {
          SETITEMHANDLEHWND( hwndLB, sItem, pTerm );
          if ( pTerm->Flags.fSelected )
          {
            SELECTITEMHWND( hwndLB, sItem );
            pTerm->Flags.fSelected = FALSE;
            fItemSelected = TRUE;
          } /* endif */
          if ( pTerm->Flags.fTop )
          {
            if ( !pTopTerm )             // if this is the first top term ...
            {
              pTopTerm = pTerm;
            } /* endif */
            pTerm->Flags.fTop = FALSE;
          } /* endif */
        } /* endif */

        usDispatchCounter--;
        if ( !usDispatchCounter )
        {
          UtlDispatch();
          usDispatchCounter = 100;
        } /* endif */
      } /* endif */
    } /* endfor */
    pTermTable = pTermTable->pNextTable;
  } /* endwhile */

  /********************************************************************/
  /* Set first visible term if top mark was set                       */
  /********************************************************************/
  if ( pTopTerm )
  {
    sItem = SEARCHITEMHWNDW( hwndLB, pTopTerm->pszName );
    if ( sItem != LIT_NONE )
    {
      SETTOPINDEXHWND( hwndLB, sItem );
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* Select/Deselect first item if no item has been selected yet      */
  /* (otherwise keyboard selection fails due to an OS/2 1.3 bug)      */
  /********************************************************************/
  if ( !fItemSelected )
  {
    SELECTITEMHWND( hwndLB, 0 );
    DESELECTITEMHWND( hwndLB, 0 );
  } /* endif */

  /********************************************************************/
  /* Enable listbox update                                            */
  /********************************************************************/
  ENABLEUPDATEHWND_TRUE( hwndLB  );

}

int FiltCompTerms( const void *parg1, const void *parg2 )
{
  PTERM *ppTerm1 = (PTERM *)parg1;
  PTERM *ppTerm2 = (PTERM *)parg2;
  return( strcmp( (*ppTerm1)->pszDestination, (*ppTerm2)->pszDestination ) );
}


//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     LstWorkProcessMarks
//------------------------------------------------------------------------------
// Function call:     LstWorkProcessMarks( PWWLDLGIDA pIda );
//------------------------------------------------------------------------------
// Description:       Processes all marked terms. Terms marked for delete are
//                    flagged as deleted. Terms marked for an exclusion list
//                    are added to the exclusion list selected for these terms.
//                    Terms marked for dictionaries are passed to the
//                    dictionary edit dialog using the list-to-dictionary
//                    interface functions.
//------------------------------------------------------------------------------
// Input parameter:   PWWLDLGIDA    pIda       pointer to IDA of WWL dialog
//------------------------------------------------------------------------------
// Returncode type:   USHORT
//------------------------------------------------------------------------------
// Returncodes:       0                        all marks have been processed
//                    ERROR_INVALID_FUNCTION   internal error occurred
//                    other                    error codes of called functions
//------------------------------------------------------------------------------
// Side effects:      The term list is saved to disk.
//------------------------------------------------------------------------------
// Function flow:     while not end of term tables
//                      loop over all terms in the current table
//                        if term is not deleted then
//                          switch term marking
//                            case DELMARK_FLAG:
//                              set deleted flag of term
//                              set list change flag
//                            case EXCLMARK_FLAG:
//                              increment number of exclusion terms
//                            case DICTMARK_FLAG:
//                              increment number of dictionary terms
//                          endswitch
//                        endif
//                        continue with next term in table
//                      endloop
//                      continue with next tag table
//                    endwhile
//                    allocate buffer for exclusion terms and dictionary terms
//                    fill buffers for exclusion list term and dictionary terms
//                      with term pointers
//                    sort terms in buffer on dictionary/exclusion list names
//                    loop over all terms in exclusion list buffer
//                      if term target is not for current exclusion list then
//                        if an exclusion list is open then
//                          write the open exclusion list to disk
//                          free in-memory exclusion list
//                        endif
//                        make new exclusion list the current one
//                        load exclusion list into memory
//                        prepare in-memory exclusion list
//                      endif
//                      enlarge exclusion list if necessary
//                      add current term to exclusion list
//                    endloop
//                    if an exclusion list is open then
//                      write the open exclusion list to disk
//                      free in-memory exclusion list
//                    endif
//                    call List2DictInterface to process dictionary terms
//                    if new terms list has been changed
//                      refresh contents of term listbox
//                      write changed term list to disk
//                    endif
//                    free all allocated buffers
//------------------------------------------------------------------------------
USHORT LstWorkProcessMarks
(
PWWLDLGIDA   pIda                    // pointer to WWL dialog IDA
)
{
  USHORT     usTerm;                   // index into term table
  PTERMTABLE pTermTable;               // ptr to current term table
  PTERM      pTerm;                    // ptr to current term
  USHORT     usRC = 0;                 // function return code
  BOOL       fListChanged = FALSE;     // true if list has been changed
  BOOL       fCloseDialog = FALSE;     // true if list has been changed
  BOOL       fListExists = FALSE;      // TRUE = current exclusion lists exists
  CHAR       szDestination[MAX_FNAME]; // exclusion list or dictionary name
  PEXCLUSIONLIST pExclList = NULL;     // pointer to loaded exclusion lists
  ULONG      ulBufferSize = 0;         // allocated size of terms buffer
  ULONG      ulBufferUsed = 0;         // used size of terms buffer
  PSZ_W      pTermBuffer  = NULL;      // pointer to exclusion terms buffer
  PSZ        pszParm;                  // parameter for message box calls
  USHORT     usExclEntries;            // number of terms marked for exclusion lists
  USHORT     usDictEntries;            // number of terms marked for dictionary
  PTERM      *ppExclTermList = NULL;   // ptr to exclusion list term array
  PTERM      *ppDictTermList = NULL;   // ptr to dictionary term array
  PTERM      *ppExclTerm;              // ptr to exclusion list terms
  PTERM      *ppDictTerm;              // ptr to dictionary terms
  USHORT     usI;                      // general loop counter
  BOOL       fTermsMarked = FALSE;     // terms-are-marked flag

  /*******************************************************************/
  /* switch to hourglass pointer                                     */
  /*******************************************************************/
  SETCURSOR( SPTR_WAIT );

  /********************************************************************/
  /* Loop over all terms, flag all terms marked for delete and        */
  /* count all terms marked for exclusion lists and marked for        */
  /* dictionaries                                                     */
  /********************************************************************/
  /******************************************************************/
  /* Start at first term table                                      */
  /******************************************************************/
  pTermTable = pIda->pTermTable;
  usDictEntries = usExclEntries = 0;

  /******************************************************************/
  /* Loop over all term tables                                      */
  /******************************************************************/
  while ( pTermTable )
  {
    /****************************************************************/
    /* start at first term of term table                            */
    /****************************************************************/
    pTerm = (PTERM)(pTermTable+1);

    /****************************************************************/
    /* loop over all terms in the current table                     */
    /****************************************************************/
    for ( usTerm = 0; usTerm < pTermTable->usUsedEntries; usTerm++, pTerm++ )
    {
      /**************************************************************/
      /* Process al terms not marked as deleted                     */
      /**************************************************************/
      if ( !pTerm->Flags.fDeleted )
      {
        switch ( pTerm->Flags.fMark )
        {
          case DELMARK_FLAG :
            pTerm->Flags.fDeleted = TRUE;
            fListChanged          = TRUE;
            fTermsMarked          = TRUE;
            break;

          case EXCLMARK_FLAG :
            fTermsMarked          = TRUE;
            usExclEntries++;
            break;

          case DICTMARK_FLAG :
            fTermsMarked          = TRUE;
            usDictEntries++;
            break;

          default:
            break;
        } /* endswitch */
      } /* endif */
    } /* endfor */

    /****************************************************************/
    /* continue with next term table                                */
    /****************************************************************/
    pTermTable = pTermTable->pNextTable;
  } /* endwhile */

  /********************************************************************/
  /* Issue warning message if no terms are marked for processing      */
  /********************************************************************/
  if ( !fTermsMarked )
  {
    UtlErrorHwnd( WARNING_LST_NO_TERMS_MARKED, MB_OK, 0, NULL, EQF_WARNING, pIda->hwnd );
  } /* endif */

  /********************************************************************/
  /* Allocate buffers for exclusion list terms and dictionary terms   */
  /********************************************************************/
  if ( usExclEntries )
  {
    if ( !UtlAlloc( (PVOID *) (PVOID *)&ppExclTermList, 0L,
                    (LONG)max( MIN_ALLOC, usExclEntries * sizeof(PTERM) ),
                    ERROR_STORAGE ) )
    {
      usRC = ERROR_NOT_ENOUGH_MEMORY;
    } /* endif */
  } /* endif */

  if ( (usRC == NO_ERROR) && usDictEntries )
  {
    if ( !UtlAlloc( (PVOID *) (PVOID *)&ppDictTermList, 0L,
                    (LONG)max( MIN_ALLOC, usDictEntries * sizeof(PTERM) ),
                    ERROR_STORAGE ) )
    {
      usRC = ERROR_NOT_ENOUGH_MEMORY;
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* fill buffers with exclusion list terms and dictionary terms      */
  /********************************************************************/
  if ( usRC == NO_ERROR )
  {
    if ( usDictEntries || usExclEntries )
    {
      pTermTable = pIda->pTermTable;
      ppDictTerm = ppDictTermList;
      ppExclTerm = ppExclTermList;
      while ( pTermTable )
      {
        pTerm = (PTERM)(pTermTable+1);
        for ( usTerm = 0; usTerm < pTermTable->usUsedEntries; usTerm++, pTerm++ )
        {
          if ( !pTerm->Flags.fDeleted )
          {
            switch ( pTerm->Flags.fMark )
            {
              case EXCLMARK_FLAG :
                *ppExclTerm++ = pTerm;
                break;

              case DICTMARK_FLAG :
                *ppDictTerm++ = pTerm;
                /******************************************************/
                /* if term context flag is on and context pointer is  */
                /* not set, set context pointer to first context of   */
                /* term (terms marked for dictionaries will only have */
                /* one context = context from the context MLE)        */
                /******************************************************/
                if ( pTerm->Flags.fContext && !pTerm->pszContext &&
                     pTerm->pContextList )                      /* 1@KIT123XA */
                {
                  pTerm->pszContext = LstGetContext( pIda->pContextTable,
                                                     pTerm->pContextList->ausContextID[0] );
                } /* endif */
                break;

              default:
                break;
            } /* endswitch */
          } /* endif */
        } /* endfor */
        pTermTable = pTermTable->pNextTable;
      } /* endwhile */
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* sort term tables on dictionary / exclusion list name             */
  /********************************************************************/
  if ( usRC == NO_ERROR )
  {
    if ( usDictEntries )
    {
      qsort( ppDictTermList, usDictEntries, sizeof(PTERM), FiltCompTerms );
    } /* endif */

    if ( usExclEntries )
    {
      qsort( ppExclTermList, (size_t)usExclEntries, (size_t)sizeof(PTERM),
             FiltCompTerms );
    } /* endif */

  } /* endif */

  /********************************************************************/
  /* Process exclusion list term list                                 */
  /********************************************************************/
  if ( usRC == NO_ERROR )
  {

    ppExclTerm = ppExclTermList;
    szDestination[0] = EOS;
    for ( usI = 0;
        (usI < usExclEntries) && (usRC == NO_ERROR);
        usI++, ppExclTerm++ )
    {
      pTerm = *ppExclTerm;

      /****************************************************************/
      /* handle change in exclusion list name                         */
      /****************************************************************/
      if ( strcmp( pTerm->pszDestination, szDestination) != 0 )
      {
        /**************************************************************/
        /* process any open exclusion list                            */
        /**************************************************************/
        if ( szDestination[0] )
        {
          /**************************************************************/
          /* End the term list in the term buffer                       */
          /* (There is no need to check if there is enough room left    */
          /*  in the termbuffer to add an empty element as the last     */
          /*  byte in the buffer is alway left free)                    */
          /**************************************************************/
          if (pTermBuffer)
              *((PSZ_W)((PBYTE)pTermBuffer+ulBufferUsed)) = EOS;

          /**************************************************************/
          /* Write the exclusion list to disk                           */
          /**************************************************************/
          if ( !LstWriteExclList( pIda->szBuffer, pTermBuffer ) )
          {
            /************************************************************/
            /* Write of list failed, end processing                     */
            /************************************************************/
            usRC = ERROR_WRITE_FAULT;
          }
          else
          {
            /**********************************************************/
            /* Broadcast new or modified exclusion list               */
            /**********************************************************/
            if ( fListExists )
            {
              EqfSend2AllHandlers( WM_EQFN_PROPERTIESCHANGED,
                                   MP1FROMSHORT( PROP_CLASS_LIST ),
                                   MP2FROMP(pIda->szBuffer) );
            }
            else
            {
              EqfSend2AllHandlers( WM_EQFN_CREATED,
                                   MP1FROMSHORT( clsLIST ),
                                   MP2FROMP(pIda->szBuffer) );
            } /* endif */

            /************************************************************/
            /* list has been written, free termbuffer, add list to      */
            /* list combobox if list is new                           */
            /************************************************************/
            UtlAlloc( (PVOID *) &pTermBuffer, 0L, 0L, NOMSG );
            ulBufferSize = 0;
            if ( !fListExists )
            {
              CBINSERTITEM( pIda->hwnd, ID_LISTWORK_EXCL_CB, szDestination );
            } /* endif */
          } /* endif */
        } /* endif */

        /******************************************************/
        /* make term's exclusion list the current one         */
        /******************************************************/
        strcpy( szDestination, pTerm->pszDestination );

        /******************************************************/
        /* Build fully qualified list name                    */
        /******************************************************/
        UtlMakeEQFPath( pIda->szBuffer, NULC, LIST_PATH, NULL );
        strcat( pIda->szBuffer, BACKSLASH_STR );
        strcat( pIda->szBuffer, szDestination );
        strcat( pIda->szBuffer, EXT_OF_EXCLUSION );

        /******************************************************/
        /* Check for new exclusion lists                      */
        /******************************************************/
        fListExists = UtlFileExist( pIda->szBuffer );
        if ( fListExists )
        {
          /******************************************************/
          /* open the exclusion list and prepare it for         */
          /* updates                                            */
          /******************************************************/
          pExclList = NULL;
          usRC = LstReadNoiseExclList( pIda->szBuffer, &ulBufferSize, &pExclList );

          if ( !usRC )
          {
            /****************************************************/
            /* Move term part to begin of exclusion list and    */
            /* use allocated buffer as work area for new terms  */
            /****************************************************/
            pTermBuffer = (PSZ_W) pExclList;                 // use allocated area
            ulBufferUsed = pExclList->uLength - pExclList->uStrings - strlen( UNICODEFILEPREFIX);

            memmove( (PBYTE)pTermBuffer, (PBYTE)pTermBuffer + pExclList->uStrings, ulBufferUsed);
            memset( (PBYTE)pTermBuffer + ulBufferUsed, 0, ulBufferSize - ulBufferUsed);
            pExclList = NULL;    // can't be used as exclusion list anymore
          } /* endif */
        }
        else
        {
          /****************************************************/
          /* Get user confirmation for new exclusion list     */
          /****************************************************/
          pszParm = szDestination;
          if ( UtlErrorHwnd( QUERY_LST_EXCL_CREATE,
                             MB_YESNO,
                             1,
                             &pszParm,
                             EQF_QUERY, pIda->hwnd ) == MBID_YES )
          {
            if ( UtlAlloc( (PVOID *) &pTermBuffer, 0L, (LONG)TERM_BUFFER_SIZE * sizeof(CHAR_W),
                           ERROR_STORAGE ) )
            {
              ulBufferSize = TERM_BUFFER_SIZE*sizeof(CHAR_W);
              ulBufferUsed = 0;
            }
            else
            {
              usRC = ERROR_NOT_ENOUGH_MEMORY;
            } /* endif */
          }
          else
          {
            usRC = ERROR_INVALID_FUNCTION;
          } /* endif */
        } /* endif */
      } /* endif */

      /****************************************************/
      /* enlarge term buffer if required                  */
      /****************************************************/
      if ( usRC == NO_ERROR )
      {
        if ( (UTF16strlenBYTE(pTerm->pszName) + sizeof(CHAR_W)) >=
             (ulBufferSize - ulBufferUsed) )
        {
          if ( UtlAlloc( (PVOID *) &pTermBuffer, ulBufferSize ,
                         ulBufferSize + TERM_BUFFER_SIZE * sizeof(CHAR_W),
                         ERROR_STORAGE ) )
          {
            ulBufferSize += TERM_BUFFER_SIZE*sizeof(CHAR_W);
          }
          else
          {
            usRC = ERROR_NOT_ENOUGH_MEMORY;
          } /* endif */
        } /* endif */
      } /* endif */

      /****************************************************/
      /* Add term to term buffer                          */
      /****************************************************/
      if ( usRC == NO_ERROR )
      {
        UTF16strcpy( (PSZ_W)((PBYTE)pTermBuffer + ulBufferUsed), pTerm->pszName );
        UtlUpperW( (PSZ_W)((PBYTE)pTermBuffer + ulBufferUsed) );
        ulBufferUsed += (UTF16strlenCHAR(pTerm->pszName) + 1)*sizeof(CHAR_W);
      } /* endif */

      /******************************************************/
      /* Set new term flags                                 */
      /******************************************************/
      if ( usRC == NO_ERROR )
      {
        pTerm->Flags.fDeleted = TRUE;
        fListChanged          = TRUE;
      } /* endif */

    } /* endfor */

    /******************************************************************/
    /* Close any open exclusion list                                  */
    /******************************************************************/
    if ( (usRC == NO_ERROR) && szDestination[0] )
    {
      /**************************************************************/
      /* End the term list in the term buffer                       */
      /* (There is no need to check if there is enough room left    */
      /*  in the termbuffer to add an empty element as the last     */
      /*  byte in the buffer is alway left free)                    */
      /**************************************************************/
      *((PSZ_W)((PBYTE)pTermBuffer+ulBufferUsed)) = EOS;

      /**************************************************************/
      /* Write the exclusion list to disk                           */
      /**************************************************************/
      if ( !LstWriteExclList( pIda->szBuffer, pTermBuffer ) )
      {
        /************************************************************/
        /* Write of list failed, end processing                     */
        /************************************************************/
        usRC = ERROR_WRITE_FAULT;
      }
      else
      {
        /**********************************************************/
        /* Broadcast new or modified exclusion list               */
        /**********************************************************/
        if ( fListExists )
        {
          EqfSend2AllHandlers( WM_EQFN_PROPERTIESCHANGED,
                               MP1FROMSHORT( PROP_CLASS_LIST ),
                               MP2FROMP(pIda->szBuffer) );
        }
        else
        {
          EqfSend2AllHandlers( WM_EQFN_CREATED,
                               MP1FROMSHORT( clsLIST ),
                               MP2FROMP(pIda->szBuffer) );
        } /* endif */

        /************************************************************/
        /* list has been written, free termbuffer, add list to      */
        /* list combobox if list is new                           */
        /************************************************************/
        UtlAlloc( (PVOID *) &pTermBuffer, 0L, 0L, NOMSG );
        ulBufferSize = 0;
        if ( !fListExists )
        {
          CBINSERTITEM( pIda->hwnd, ID_LISTWORK_EXCL_CB, szDestination );
        } /* endif */
      } /* endif */
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* Process dictionary term list                                     */
  /********************************************************************/
  if ( (usRC == NO_ERROR) && usDictEntries )
  {
    SETCURSOR( SPTR_ARROW );
    fListChanged |= List2DictInterface( pIda->hwnd, ppDictTermList,
                                        usDictEntries, &usRC );
    SETCURSOR( SPTR_WAIT );
  } /* endif */

  /********************************************************************/
  /* Refresh term list box and save list if list has been changed     */
  /********************************************************************/
  if ( fListChanged )
  {
    /******************************************************************/
    /* Check if list is now empty                                     */
    /******************************************************************/
    pTermTable = pIda->pTermTable;
    usExclEntries = 0;                 // used for number of entries
    while ( pTermTable )
    {
      pTerm = (PTERM)(pTermTable+1);
      for ( usTerm = 0; usTerm < pTermTable->usUsedEntries; usTerm++, pTerm++ )
      {
        if ( !pTerm->Flags.fDeleted )
        {
          usExclEntries++;
        } /* endif */
      } /* endfor */
      pTermTable = pTermTable->pNextTable;
    } /* endwhile */

    /******************************************************************/
    /* if list is empty, delete list and leave dialog else refresh    */
    /* listbox and save list to disk                                  */
    /******************************************************************/
    if ( usExclEntries == 0 )
    {
      pIda->fListChanged = FALSE;      // reset list change flag
      SETCURSOR( SPTR_ARROW );

      /****************************************************************/
      /* Delete the list                                              */
      /****************************************************************/
      UtlDelete( pIda->szListPath, 0L, TRUE );

      /****************************************************************/
      /* Broadcast list deleted notification message                  */
      /****************************************************************/
      EqfSend2AllHandlers( WM_EQFN_DELETED, MP1FROMSHORT( clsLIST ),
                           MP2FROMP(pIda->szListPath) );

      /****************************************************************/
      /* Tell user that list is deleted and dialog is removed         */
      /****************************************************************/
      pszParm = pIda->szListName;
      UtlErrorHwnd( INFO_ALL_TERMS_PROCESSED, MB_OK, 1, &pszParm, EQF_INFO, pIda->hwnd );

      /****************************************************************/
      /* Close dialog                                                 */
      /****************************************************************/
      fCloseDialog = TRUE;         // request close of dialog
    }
    else
    {
      /******************************************************************/
      /* Refresh the term list box                                      */
      /******************************************************************/
      LstWorkFillTermsLB( pIda );

      /******************************************************************/
      /* Save changed list to disk                                      */
      /******************************************************************/
      LstWorkSetSelFlags( pIda );
      if ( LstWriteSGMLList( pIda->szListPath,
                             &pIda->ListHeader,
                             pIda->pTermTable,
                             pIda->pContextTable,
                             FALSE,
                             (LISTTYPES)pIda->usListType, SGMLFORMAT_UNICODE ) == 0 )
      {
        if ( !usRC )
        {
          SETCURSOR( SPTR_ARROW );
          pszParm = pIda->szListName;
          UtlErrorHwnd( INFO_LST_MARKS_PROCESSED, MB_OK, 1, &pszParm, EQF_INFO, pIda->hwnd );
          pIda->fListChanged = FALSE;
        } /* endif */
      } /* endif */
      LstWorkClearSelFlags( pIda );
      SETCURSOR( SPTR_ARROW );
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* Cleanup                                                          */
  /********************************************************************/
  if ( pExclList )       UtlAlloc( (PVOID *) (PVOID *)&pExclList, 0L, 0L, NOMSG );
  if ( pTermBuffer )     UtlAlloc( (PVOID *) (PVOID *)&pTermBuffer, 0L, 0L, NOMSG );
  if ( ppExclTermList )  UtlAlloc( (PVOID *) (PVOID *)&ppExclTermList, 0L, 0L, NOMSG );
  if ( ppDictTermList )  UtlAlloc( (PVOID *) (PVOID *)&ppDictTermList, 0L, 0L, NOMSG );

  /********************************************************************/
  /* Close dialog if requested                                        */
  /********************************************************************/
  if ( fCloseDialog )
  {
    WinPostMsg( pIda->hwnd, WM_EQF_CLOSE, 0L, 0L );
  } /* endif */

  return( usRC );
} /* end of function LstWorkProcessMarks */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     LSTEDITLISTDIALOG   Dialog procedure of the list edit dlg
//------------------------------------------------------------------------------
// Function call:     LSTEDITLISTSDLG( HWND  hwnd, USHORT msg,
//                                     MPARAM mp1, MPARAM mp2 );
//------------------------------------------------------------------------------
// Description:       Dialog procedure for the edit list dialog. Performs all
//                    functions required to create and change exclusion lists
//                    noise lists and abbreviation lists
//------------------------------------------------------------------------------
// Input parameter:  HWND    hwnd     handle of window
//                   USHORT  msg      type of message
//                   MPARAM  mp1      first message parameter
//                   MPARAM  mp2      second message parameter
//------------------------------------------------------------------------------
// Returncode type:  MRESULT
//------------------------------------------------------------------------------
// Returncodes:      depends on message type
//                   normal return codes are:
//                   TRUE  = message has been processed
//                   FALSE = message has not been processed
//------------------------------------------------------------------------------
// Function flow:     switch msg
//                      case WM_INITDLG:
//                        anchor IDA
//                        fill-in title bar text
//                        set text limits and MLE format
//                        fill save to combobox
//                        load noise/exclusion list
//                        prepare term part of list for MLE update
//                        import term part into MLE
//                        dismiss dialog if initialization failed
//                      case WM_CONTROL:
//                        switch control
//                          case change in term MLE
//                            set list change flag
//                        endswitch
//                      case WM_COMMAND:
//                        switch command value
//                          case DID_CANCEL:
//                          case cancel pushbutton:
//                            post WM_CLOSE message to dialog
//                          case save pushbutton:
//                            get list name
//                            check name
//                            if list exists and is not displayed one then
//                              get user confirmation
//                            endif
//                            allocate term buffer
//                            export terms from MLE into term buffer
//                            convert term buffer to term list
//                            call LstWriteExclList to write list to disk
//                            broadcast create or change notification message
//                            dismiss dialog
//                        endswitch
//                      case WM_CLOSE:
//                        if list has been changed then
//                          get user confirmation
//                          switch user decision
//                            case 'YES'
//                              save list
//                              dismiss dialog
//                            case 'NO'
//                              dismiss dialog
//                            default
//                              stay in dialog
//                          endswitch
//                        else
//                          dismiss the dialog
//                        endif
//                      default:
//                        pass message to default dialog procedure
//                    endswitch
//------------------------------------------------------------------------------
INT_PTR CALLBACK LSTEDITLISTSDLG
(
HWND   hwnd,
WINMSG msg,
WPARAM mp1,
LPARAM mp2
)
{
  MRESULT   mResult = FALSE;
  PSZ       pszTemp;                   // multi purpose pointer
  PSZ_W     pszTempW;                  // multi purpose pointer
  PLISTEDITIDA pIda;                   // pointer to dialog IDA
  PEXCLUSIONLIST pExclList;            // pointer to loaded exclusion lists
  USHORT    usI;                       // loop index
  BOOL      fOK;                       // internal OK flag
  USHORT    usMBCode;                  // message box return code
  PSZ       pszSource;                 // pointer for copy operations: source
  PSZ_W     pszTarget = NULL;          // pointer for copy operations: target
  PSZ_W     pszTermBuf;                // buffer for terms
  PUSHORT   pusTermInd;                // pointer to term index
  BOOL      fListExists = TRUE;        // list is overwritten flag
  USHORT    usSaveToID;                // ID of saveto string
  USHORT    usSavePBID;                // ID of save pushbutton string
  USHORT    usRC;                      // buffer for return codes

  switch ( msg )
  {
    case WM_EQF_QUERYID:
      pIda = ACCESSDLGIDA( hwnd, PLISTEDITIDA );
      if ( pIda->szListName[0] )     // existing list ???
      {
        if ( pIda->usListType == NOISE_TYPE )
        {
          HANDLEQUERYID( ID_LISTEDIT_EDI_NOIS_DLG, mp2 );
        }
        else if ( pIda->usListType == ABR_TYPE )
        {
          HANDLEQUERYID( ID_LISTEDIT_EDI_ABR_DLG, mp2 );
        }
        else if ( pIda->usListType == ADD_TYPE )
        {
          HANDLEQUERYID( ID_LISTEDIT_EDI_ADD_DLG, mp2 );
        }
        else
        {
          HANDLEQUERYID( ID_LISTEDIT_EDI_EXCL_DLG, mp2 );
        } /* endif */
      }
      else
      {
        if ( pIda->usListType == NOISE_TYPE )
        {
          HANDLEQUERYID( ID_LISTEDIT_NEW_NOIS_DLG, mp2 );
        }
        else
        {
          HANDLEQUERYID( ID_LISTEDIT_NEW_EXCL_DLG, mp2 );
        } /* endif */
      } /* endif */
      break;

    case WM_INITDLG:
      /****************************************************************/
      /* Anchor IDA                                                   */
      /****************************************************************/
      fOK = TRUE;
      pIda = (PLISTEDITIDA)mp2;
      pIda->hwnd = hwnd;
      ANCHORDLGIDA( hwnd, pIda );
      pszTermBuf = NULL;               // no term buffer yet
      pExclList  = NULL;               // no exclusion list buffer yet
      pIda->sLangID = - 1;             // no language support yet

      /****************************************************************/
      /* Fill-in title bar text and set dialog ID                     */
      /****************************************************************/
      if ( fOK  )
      {
        HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);

        /**************************************************************/
        /* Get string ID for dialog title                             */
        /**************************************************************/
        if ( pIda->szListName[0] )     // existing list ???
        {
          usSaveToID = SID_LISTEDIT_SAVETO_TEXT;
          usSavePBID = SID_LISTEDIT_SAVE_TEXT;
          switch ( pIda->usListType )
          {
            case NOISE_TYPE :
              usI = SID_LSTEDIT_NOISETITLE_TEXT;
              SETWINDOWID( hwnd, ID_LISTEDIT_EDI_NOIS_DLG );
              break;

            case ABR_TYPE :
              usI = SID_LSTEDIT_ABRTITLE_TEXT;
              SETWINDOWID( hwnd, ID_LISTEDIT_EDI_ABR_DLG );
              break;

            case ADD_TYPE :
              usI = SID_LSTEDIT_ADDTITLE_TEXT;
              SETWINDOWID( hwnd, ID_LISTEDIT_EDI_ADD_DLG );
              break;

            case EXCL_TYPE :
            default :
              usI = SID_LSTEDIT_EXCLTITLE_TEXT;
              SETWINDOWID( hwnd, ID_LISTEDIT_EDI_EXCL_DLG );
              break;
          } /* endswitch */
        }
        else
        {
          usSaveToID = SID_LISTEDIT_NAME_TEXT;
          usSavePBID = SID_LISTEDIT_CREATE_TEXT;
          if ( pIda->usListType == NOISE_TYPE )
          {
            usI = SID_LSTNEW_NOISETITLE_TEXT;
            SETWINDOWID( hwnd, ID_LISTEDIT_NEW_NOIS_DLG );
          }
          else
          {
            usI = SID_LSTNEW_EXCLTITLE_TEXT;
            SETWINDOWID( hwnd, ID_LISTEDIT_NEW_EXCL_DLG );
          } /* endif */
        } /* endif */

        /**************************************************************/
        /* Load save to static text                                   */
        /**************************************************************/
        LOADSTRING( NULLHANDLE, hResMod, usSaveToID, pIda->szBuffer );
        SETTEXT( hwnd, ID_LISTEDIT_SAVETO_TEXT, pIda->szBuffer );

        /**************************************************************/
        /* Load save pushbutton text                                  */
        /**************************************************************/
        LOADSTRING( NULLHANDLE, hResMod, usSavePBID, pIda->szBuffer );
        SETTEXT( hwnd, ID_LISTEDIT_SAVE_PB, pIda->szBuffer );

        /**************************************************************/
        /* Load dialog title                                          */
        /**************************************************************/
        LOADSTRING( NULLHANDLE, hResMod, usI, pIda->szBuffer );

        /**************************************************************/
        /* For existing lists, insert list name                       */
        /**************************************************************/
        if ( pIda->szListName[0] )     // existing list ???
        {
          ULONG  Length;

          pszTemp = pIda->szListName;
          DosInsMessage( &pszTemp, 1, pIda->szBuffer,
                         strlen( pIda->szBuffer ) + 1,
                         pIda->szBuffer2,
                         sizeof( pIda->szBuffer2 ), &Length );
          strcpy( pIda->szBuffer, pIda->szBuffer2 );
        } /* endif */

        /**************************************************************/
        /* Set text of dialog title bar                               */
        /**************************************************************/
        SETTEXTHWND( hwnd, pIda->szBuffer );
      } /* endif */

      /****************************************************************/
      /* Set text limits and other PM stuff                           */
      /****************************************************************/
      if ( fOK )
      {

        switch ( pIda->usListType )
        {
          case ABR_TYPE :
          case ADD_TYPE :
            CBSETTEXTLIMIT( hwnd, ID_LISTEDIT_SAVETO_CB, MAX_EQF_PATH - 1 );

            MLESETTEXTLIMIT( hwnd, ID_LISTEDIT_TERM_MLE, 0xFFFF );
            break;

          case NOISE_TYPE :
          case EXCL_TYPE :
          default :
            CBSETTEXTLIMIT( hwnd, ID_LISTEDIT_SAVETO_CB, MAX_FNAME - 1 );
            MLESETTEXTLIMIT( hwnd, ID_LISTEDIT_TERM_MLE, MAX_EXCLLIST_SIZE );
            break;
        } /* endswitch */

        /**************************************************************/
        /* Set MLE format to MLFIE_NOTRANS which means that single LF */
        /* characters are used as line-end delimiters on import and   */
        /* export                                                     */
        /**************************************************************/
        SetCtrlFnt(hwnd, GetCharSet(), ID_LISTEDIT_TERM_MLE, 0);
      } /* endif */

      /****************************************************************/
      /* Fill-in lists into save-to combobox                          */
      /****************************************************************/
      if ( fOK )
      {
        switch ( pIda->usListType )
        {
          case NOISE_TYPE :
            EqfSend2Handler( LISTHANDLER, WM_EQF_INSERTNAMES,
                             MP1FROMHWND( WinWindowFromID( hwnd, ID_LISTEDIT_SAVETO_CB) ),
                             MP2FROMP( NOISELISTOBJ ) );
            break;

          case ABR_TYPE :
          case ADD_TYPE :
            break;

          case EXCL_TYPE :
          default:
            EqfSend2Handler( LISTHANDLER, WM_EQF_INSERTNAMES,
                             MP1FROMHWND( WinWindowFromID( hwnd, ID_LISTEDIT_SAVETO_CB) ),
                             MP2FROMP( EXCLUSIONLISTOBJ ) );
            break;
        } /* endswitch */
      } /* endif */

      /****************************************************************/
      /* Set default save-to name for existing lists                  */
      /****************************************************************/
      if ( fOK && pIda->szListName[0] )
      {
        SETTEXT( hwnd, ID_LISTEDIT_SAVETO_CB, pIda->szListName );
      } /* endif */

      /****************************************************************/
      /* Load noise / exclusion / abbreviation list and allocate term */
      /* buffer, import the terms into the MLE                        */
      /****************************************************************/
      if ( fOK && pIda->szListName[0] )
      {
        switch ( pIda->usListType )
        {
          case NOISE_TYPE :
          case EXCL_TYPE :            // list needs to be in Unicode!!!
            {
              ULONG     ulSize;                    // size of loaded lists

              pExclList = NULL;

              usRC = LstReadNoiseExclList( pIda->szListPath, &ulSize, &pExclList );

              if ( usRC )
              {
                fOK = FALSE;
              }
              else
              {
                UtlAlloc( (PVOID *) &pszTermBuf, 0L,
                           (LONG)max( MIN_ALLOC, ulSize * sizeof (CHAR_W) *2 ),
                                                ERROR_STORAGE );
              } /* endif */

              /****************************************************************/
              /* Prepare text part of noise / exclusion list for MLE import   */
              /****************************************************************/
              if ( fOK && pIda->szListName[0] )
              {
                /**************************************************************/
                /* Position to term offset area in noise / exclusion list     */
                /**************************************************************/
                pszTarget = pszTermBuf;
                pusTermInd = (PUSHORT) (((PSZ)pExclList) + pExclList->uFirstEntry);

                /**************************************************************/
                /* Loop through terms and add the terms to the term buffer.   */
                /* The terms are seperated using LF (CRLF for Windows)        */
                /**************************************************************/
                for ( usI = 0; usI < pExclList->usNumEntries; usI++ )
                {
                  pszTempW = (PSZ_W)((PBYTE)pExclList + pExclList->uStrings) + *pusTermInd;
                  if ( *pszTempW )
                  {
                    UTF16strcpy( pszTarget, pszTempW);
                    pszTarget   += UTF16strlenCHAR(pszTarget);

                    *pszTarget++ = CR;
                    *pszTarget++ = LF;
                  } /* endif */
                  pusTermInd++;
                } /* endfor */
              } /* endif */

              /**********************************************************/
              /* Import the terms into the MLE                          */
              /**********************************************************/
              if ( fOK && pIda->szListName[0] && pszTermBuf )
              {
                LONG ulLen;
                HWND      hwndMLE;       // handle of terms MLE

                hwndMLE = WinWindowFromID( hwnd, ID_LISTEDIT_TERM_MLE );

                ulLen = pszTarget - pszTermBuf;
                MLEIMPORTHWNDW( hwndMLE, pszTermBuf, ulLen );
                MLESETCHANGEDHWND( hwndMLE, FALSE );
              } /* endif */
            }
            break;

          case ABR_TYPE :
          case ADD_TYPE :
            {
              /********************************************************/
              /* Activate/Access language support and get terms       */
              /********************************************************/
              USHORT usTermListSize = 0;
              PSZ_W  pTermList = NULL;
              USHORT usMorphRC;
              ULONG  ulOemCP = 0L;

              usMorphRC = MorphGetLanguageID( pIda->szListName,
                                              &pIda->sLangID );
              ulOemCP = GetLangOEMCP(pIda->szListName);

              /********************************************************/
              /* Force a refresh of the addenda or abbreviation       */
              /* dictionary                                           */
              /* we force a refresh by using the MorphBuildDict       */
              /* function in the 'Refresh' mode                       */
              /********************************************************/
              if ( !usMorphRC )
              {
                usMorphRC = MorphBuildDict( pIda->sLangID,
                                (USHORT)(( pIda->usListType == ABR_TYPE ) ?
                                         ABBREV_DICT : ADDENDA_DICT),
                                0, NULL, MORPH_ZTERMLIST );
              } /* endif */

              if ( !usMorphRC )
              {
                usMorphRC = MorphListDict( pIda->sLangID,
                                           (USHORT)(( pIda->usListType == ABR_TYPE ) ?
                                                    ABBREV_DICT : ADDENDA_DICT),
                                           &usTermListSize,
                                           (PVOID *)&pTermList,
                                           MORPH_LARGE_ZTERMLIST );
              } /* endif */


              // handle errors returned by Morph... functions
              if ( usMorphRC )
              {
                USHORT usMsg;
                PSZ    pszLanguage = pIda->szListName;

                /******************************************************/
                /* Handle errors returned by Morph functions          */
                /******************************************************/
                switch ( usMorphRC )
                {
                  case MORPH_NO_MEMORY :
                    usMsg = ERROR_STORAGE;
                    break;

                  case MORPH_BUFFER_OVERFLOW :
                  case MORPH_INV_PARMS  :
                  case MORPH_INV_LANG_ID :
                    usMsg = ERROR_INTERNAL;
                    break;

                  case MORPH_NOADDENDA_DICT :
                    usMsg = EQFRS_NOADDENDA_DICT;
                    break;

                  case MORPH_FUNC_NOT_SUPPORTED :
                    if ( pIda->usListType == ABR_TYPE )
                    {
                      usMsg = ERROR_NOABBR_SUPPORT;
                    }
                    else
                    {
                      usMsg = ERROR_NOADD_SUPPORT;
                    } /* endif */
                    break;
                  case MORPH_FUNC_FAILED:
                      usMsg = ERROR_ADDENDAORABBREV_NOTPOSSIBLE;
                      UtlError( usMsg, MB_CANCEL,
					  	                0, NULL, EQF_ERROR );
					  fOK = FALSE;
                    break;
                  case MORPH_BEGIN_SERV_FAILED :
                  case MORPH_SET_CODEPAGE_FAILED :
                  case MORPH_NO_LANG_PROPS :
                  case MORPH_ACT_DICT_FAILED :
                  case MORPH_EXIT_LOAD_ERROR :
                  case MORPH_EXIT_PROCADDR_ERROR :
                  default :
                    usMsg = EQFRS_NOMORPH_DICT;
                    break;
                } /* endswitch */
                if (usMsg != MORPH_FUNC_FAILED )
                {
                  UtlError( usMsg, MB_CANCEL, 1, &pszLanguage, EQF_ERROR );
                  fOK = FALSE;
			    }
              } /* endif */

              /********************************************************/
              /* Setup buffer for MLE import and do MLE import        */
              /********************************************************/
              if ( !usMorphRC )
              {
                ULONG ulSize = MAX_ALLOC;   // size of buffer
                ULONG ulRest = MAX_ALLOC - 100L;   // room left in buffer

                /******************************************************/
                /* Allocate buffer for term import (initial size)     */
                /******************************************************/
                pszTermBuf = NULL;
                fOK = UtlAlloc( (PVOID *) &pszTermBuf, 0L, ulSize * sizeof(CHAR_W),
                                ERROR_STORAGE );

                /******************************************************/
                /* Fill term buffer with terms from term list         */
                /******************************************************/
                if ( fOK )
                {
                  pszTarget = pszTermBuf;
                  do
                  {
                    /******************************************************/
                    /* Add terms to term buffer                           */
                    /******************************************************/
                    //PSZ pszTerm = (PSZ)pTermList + sizeof(PSZ);
                    PSZ_W pszTerm = (PSZ_W)pTermList;
                   
                    while ( fOK && *pszTerm )
                    {
                      ULONG ulTermLen = wcslen(pszTerm);

                      /****************************************************/
                      /* Check if we have enough space left in our buffer */
                      /* to add the current term                          */
                      /****************************************************/
                      if ( (ulTermLen + 2) > ulRest )
                      {
                        ULONG ulOffset = pszTarget - pszTermBuf;

                        /**********************************************/
                        /* not enough space left, enlarge buffer      */
                        /**********************************************/
                        fOK = UtlAlloc( (PVOID *) &pszTermBuf, ulSize,
                                        (LONG)((ulSize + MAX_ALLOC) * sizeof(CHAR_W)),
                                        ERROR_STORAGE );
                        if ( fOK )
                        {
                          pszTarget = pszTermBuf + ulOffset;
                          ulSize += MAX_ALLOC,
                          ulRest += MAX_ALLOC;
                        } /* endif */
                      } /* endif */

                      /************************************************/
                      /* Add term to buffer                           */
                      /************************************************/
                      if ( fOK )
                      {
                        //ULONG ulUnicode = ASCII2UnicodeBuf(pszTerm, pszTarget, ulTermLen, ulOemCP);
                        wcsncpy(pszTarget,pszTerm,ulTermLen);
                        pszTarget   += ulTermLen;
                        pszTerm     += ulTermLen + 1;
                        *pszTarget++ = CR;
                        *pszTarget++ = LF;
                        ulRest -= (ulTermLen + 3);
                      } /* endif */
                    } /* endwhile */

                    // continue with next block if any
                    if ( fOK )
                    {
                      UtlAlloc( (PVOID *) &pTermList, 0L, 0L, NOMSG );
                      //pTermList = pszNext;
                      pTermList = NULL;
                    }
                  } while ( fOK && (pTermList != NULL) ); /* enddo */
                  *pszTarget++ = EOS;
                } /* endif */

                /******************************************************/
                /* Set text of entry field                            */
                /******************************************************/
               // if (stricmp(pIda->szListName, THAI_STR) != 0)
               // {
               //   OEMTOANSI( pszTermBuf );
               // }

                // display terms in edit control
                {
                  int iLen = UTF16strlenCHAR(pszTermBuf);
                  int iEditControlLen;

                  SetDlgItemTextW( hwnd, ID_LISTEDIT_TERM_MLE, pszTermBuf );
                  // check if all text has been added to edit contol..
                  MLEQUERYTEXTLENGTH( hwnd, ID_LISTEDIT_TERM_MLE, iEditControlLen );
                  if ( (iLen > 10) && (iEditControlLen <= 1) )
                  {
                    // maybe text is too large for edit control
                    // (Win95+Win98 only support up to 64KB in an Edit control)
                    fOK = FALSE;
                    UtlErrorHwnd( ERROR_LST_DISPLAY_ERROR, MB_CANCEL, 0, NULL, EQF_ERROR, hwnd );
                  } /* endif */
                }
                /******************************************************/
                /* Cleanup                                            */
                /******************************************************/
                if ( pszTermBuf ) UtlAlloc( (PVOID *) &pszTermBuf, 0L, 0L, NOMSG );
              } /* endif */


              /******************************************************/
              /* Get system abbreviation list.                      */
              /******************************************************/
              if ( ( !usMorphRC ) &&
                   ( pIda->usListType == ABR_TYPE ) ) 
              {
                 RECT rect ;
                 HWND hwndMLE;
                 HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
                 hwndMLE = GetDlgItem( hwnd, ID_LISTEDIT_TERM_MLE ) ;
                 GetWindowRect(hwndMLE,&rect) ;
                 SetWindowPos( hwndMLE, HWND_TOP, 0, 0,
                               (rect.right-rect.left)/2,
                               (rect.bottom-rect.top),
                               SWP_NOMOVE | SWP_NOZORDER | SWP_SHOWWINDOW ) ;
                 SHOWCONTROL( hwnd, ID_LISTEDIT_TERM_MLE2 );
                 LOADSTRING( NULLHANDLE, hResMod, SID_LISTEDIT_TEXT_ABBREV1, pIda->szBuffer );
                 SETTEXTHWND( GetDlgItem(hwnd,ID_LISTEDIT_TERM_TEXT), pIda->szBuffer );
                 LOADSTRING( NULLHANDLE, hResMod, SID_LISTEDIT_TEXT_ABBREV2, pIda->szBuffer );
                 SETTEXTHWND( GetDlgItem(hwnd,ID_LISTEDIT_TERM_TEXT2), pIda->szBuffer );

                 usTermListSize = 0 ;
                 usMorphRC = MorphListDict( pIda->sLangID,
                                            (USHORT)ABBREV_SYSTEM_DICT,
                                            &usTermListSize,
                                            (PVOID *)&pTermList,
                                            MORPH_LARGE_ZTERMLIST );

                 // handle errors returned by Morph... functions
                 if ( usMorphRC )
                 {
                   PSZ    pszLanguage = pIda->szListName;
                   UtlError( EQFRS_NOMORPH_DICT, MB_CANCEL, 1, &pszLanguage, EQF_ERROR );
                   fOK = FALSE;
                 } /* endif */
                
                 /********************************************************/
                 /* Setup buffer for MLE import and do MLE import        */
                 /********************************************************/
                 if ( !usMorphRC )
                 {
                   ULONG ulSize = MAX_ALLOC;   // size of buffer
                   ULONG ulRest = MAX_ALLOC - 100L;   // room left in buffer
                
                   /******************************************************/
                   /* Allocate buffer for term import (initial size)     */
                   /******************************************************/
                   pszTermBuf = NULL;
                   fOK = UtlAlloc( (PVOID *) &pszTermBuf, 0L, ulSize * sizeof(CHAR_W),
                                   ERROR_STORAGE );
                
                   /******************************************************/
                   /* Fill term buffer with terms from term list         */
                   /******************************************************/
                   if ( fOK )
                   {
                     pszTarget = pszTermBuf;
                     do
                     {
                       /******************************************************/
                       /* Add terms to term buffer                           */
                       /******************************************************/
                       //PSZ pszTerm = (PSZ)pTermList + sizeof(PSZ);
                       PSZ_W pszTerm = (PSZ_W)pTermList;
                
                       while ( fOK && *pszTerm )
                       {
                         ULONG ulTermLen = wcslen(pszTerm);
                
                         /****************************************************/
                         /* Check if we have enough space left in our buffer */
                         /* to add the current term                          */
                         /****************************************************/
                         if ( (ulTermLen + 2) > ulRest )
                         {
                           ULONG ulOffset = pszTarget - pszTermBuf;
                
                           /**********************************************/
                           /* not enough space left, enlarge buffer      */
                           /**********************************************/
                           fOK = UtlAlloc( (PVOID *) &pszTermBuf, ulSize,
                                           (LONG)((ulSize + MAX_ALLOC) * sizeof(CHAR_W)),
                                           ERROR_STORAGE );
                           if ( fOK )
                           {
                             pszTarget = pszTermBuf + ulOffset;
                             ulSize += MAX_ALLOC,
                             ulRest += MAX_ALLOC;
                           } /* endif */
                         } /* endif */
                
                         /************************************************/
                         /* Add term to buffer                           */
                         /************************************************/
                         if ( fOK )
                         {
                           //ULONG ulUnicode = ASCII2UnicodeBuf(pszTerm, pszTarget, ulTermLen, ulOemCP);
                           wcsncpy(pszTarget,pszTerm,ulTermLen);
                           pszTarget   += ulTermLen;
                           pszTerm     += ulTermLen + 1;
                           *pszTarget++ = CR;
                           *pszTarget++ = LF;
                           ulRest -= (ulTermLen + 3);
                         } /* endif */
                       } /* endwhile */
                
                       // continue with next block if any
                       if ( fOK )
                       {
                         UtlAlloc( (PVOID *) &pTermList, 0L, 0L, NOMSG );
                         //pTermList = pszNext;
                         pTermList = NULL;
                       }
                     } while ( fOK && (pTermList != NULL) ); /* enddo */
                     *pszTarget++ = EOS;
                   } /* endif */
                
                   /******************************************************/
                   /* Set text of entry field                            */
                   /******************************************************/
                   SetDlgItemTextW( hwnd, ID_LISTEDIT_TERM_MLE2, pszTermBuf );

                   /******************************************************/
                   /* Cleanup                                            */
                   /******************************************************/
                   if ( pszTermBuf ) UtlAlloc( (PVOID *) &pszTermBuf, 0L, 0L, NOMSG );
                }
              }
            }
            break;

          default:
            break;
        } /* endswitch */
      } /* endif */

      /****************************************************************/
      /* Hide save to control for noise and abbreviation lists        */
      /****************************************************************/
      if ( fOK && ( (pIda->usListType == NOISE_TYPE) ||
                    (pIda->usListType == ADD_TYPE)   ||
                    (pIda->usListType == ABR_TYPE) ) )
      {
        HIDECONTROL( hwnd, ID_LISTEDIT_SAVETO_CB );
        HIDECONTROL( hwnd, ID_LISTEDIT_SAVETO_TEXT );
      } /* endif */

      /****************************************************************/
      /* Cleanup                                                      */
      /****************************************************************/
      if ( pExclList )       UtlAlloc( (PVOID *) &pExclList, 0L, 0L, NOMSG );
      if ( pszTermBuf )      UtlAlloc( (PVOID *) &pszTermBuf, 0L, 0L, NOMSG );

      /****************************************************************/
      /* Leave dialog if initialization failed                        */
      /****************************************************************/
      if ( !fOK )
      {
        WinDismissDlg( hwnd, FALSE );
      }
      else
      {
        PostMessage( hwnd, WM_EQF_INITIALIZE, 0, 0 );
      } /* endif */
      break;

    case WM_EQF_INITIALIZE:    
      SendDlgItemMessage( hwnd, ID_LISTEDIT_TERM_MLE, EM_SETSEL, (WPARAM)-1, 0 );
      SETFOCUS( hwnd, ID_LISTEDIT_TERM_MLE );
      break;


    case WM_COMMAND:
      switch (WMCOMMANDID( mp1, mp2 ))
      { case ID_LISTEDIT_HELP_PB :
         mResult = UtlInvokeHelp();
          break;
        case DID_CANCEL:
        case ID_LISTEDIT_CANCEL_PB:
          WinPostMsg( hwnd, WM_EQF_CLOSE, 0L, 0L );
          break;

        case ID_LISTEDIT_SAVE_PB:
          pIda = ACCESSDLGIDA( hwnd, PLISTEDITIDA );
          fOK = TRUE;

          /************************************************************/
          /* Get name of save to list                                 */
          /************************************************************/
          pIda->szSaveToName[0] = EOS;
          QUERYTEXT( hwnd, ID_LISTEDIT_SAVETO_CB, pIda->szSaveToName );
         
          /************************************************************/
          /* Strip off leading and trailing blanks                    */
          /************************************************************/
          UtlStripBlanks( pIda->szSaveToName );

          /************************************************************/
          /* Check if a the save to combo box contains a list name    */
          /************************************************************/
          if ( !pIda->szSaveToName[0] &&
               (pIda->usListType != ABR_TYPE) &&
               (pIda->usListType != ADD_TYPE) )
          {
            UtlErrorHwnd( ERROR_LST_SAVE_NO_NAME, MB_CANCEL, 0, NULL, EQF_ERROR , hwnd );
            SETFOCUS( hwnd, ID_LISTEDIT_SAVETO_CB );
            fOK = FALSE;
          } /* endif */

          /************************************************************/
          /* Check if list name is valid (=exists of alphanumeric     */
          /* characters only)                                         */
          /************************************************************/
          if ( fOK && (pIda->usListType != ABR_TYPE) &&
               (pIda->usListType != ADD_TYPE)  )
          {
            pszSource = pIda->szSaveToName;
            while ( *pszSource && isalnum(*pszSource) )
            {
              pszSource++;
            } /* endwhile */
            if ( *pszSource != EOS )
            {
              pszSource = pIda->szSaveToName;
              UtlErrorHwnd( ERROR_LST_NAME_INVALID, MB_CANCEL, 1,
                            &pszSource, EQF_ERROR , hwnd );
              SETFOCUS( hwnd, ID_LISTEDIT_SAVETO_CB );
              fOK = FALSE;
            } /* endif */
          } /* endif */

          /************************************************************/
          /* Check if save to list is currently locked by another     */
          /* process (e.g. Analysis)                                  */
          /************************************************************/
          if ( fOK )
          {
            SHORT sResult = 0;

            strupr( pIda->szSaveToName );

            switch ( pIda->usListType )
            {
              case NOISE_TYPE :
                UtlMakeEQFPath( pIda->szListPath, NULC, TABLE_PATH, NULL );
                strcat( pIda->szListPath, BACKSLASH_STR );
                strcat( pIda->szListPath, pIda->szSaveToName );
                strcat( pIda->szListPath, EXT_OF_EXCLUSION );
                sResult = QUERYSYMBOL( pIda->szListPath );
                break;

              case EXCL_TYPE :
                UtlMakeEQFPath( pIda->szListPath, NULC, LIST_PATH, NULL );
                strcat( pIda->szListPath, BACKSLASH_STR );
                strcat( pIda->szListPath, pIda->szSaveToName );
                strcat( pIda->szListPath, EXT_OF_EXCLUSION );
                sResult = QUERYSYMBOL( pIda->szListPath );
                break;

              case ABR_TYPE :
              case ADD_TYPE :
                sResult = -1;
                break;
            } /* endswitch */

            if ( sResult != -1 )
            {
              PSZ pszParm = pIda->szSaveToName;
              UtlError( ERROR_LST_IN_USE, MB_CANCEL, 1, &pszParm, EQF_ERROR );
              fOK = FALSE;
              SETFOCUS( hwnd, ID_LISTEDIT_SAVETO_CB );
            } /* endif */
          } /* endif */


          /************************************************************/
          /* Get user confirmation if save to list exists and is not  */
          /* the list being worked on.                                */
          /************************************************************/
          if ( fOK && (pIda->usListType != ABR_TYPE) &&
               (pIda->usListType != ADD_TYPE)  )
          {
            strupr( pIda->szSaveToName );
            UtlMakeEQFPath( pIda->szListPath, NULC,
                            (USHORT)(( pIda->usListType == NOISE_TYPE ) ? TABLE_PATH :
                                     LIST_PATH),
                            NULL );
            strcat( pIda->szListPath, BACKSLASH_STR );
            strcat( pIda->szListPath, pIda->szSaveToName );
            strcat( pIda->szListPath, EXT_OF_EXCLUSION );
            fListExists = UtlFileExist( pIda->szListPath );
            if ( stricmp( pIda->szListName, pIda->szSaveToName) != 0 )
            {
              if ( fListExists )
              {
                pszSource = pIda->szSaveToName;
                usMBCode = UtlErrorHwnd( WARNING_LST_FILE_EXISTS,
                                         MB_YESNO | MB_DEFBUTTON2, 1,
                                         &pszSource, EQF_QUERY , hwnd );
                if ( usMBCode == MBID_NO )
                {
                  SETFOCUS( hwnd, ID_LISTEDIT_SAVETO_CB );
                  fOK = FALSE;
                } /* endif */
              } /* endif */
            } /* endif */
          } /* endif */

          /************************************************************/
          /* Get/Build list and save it                               */
          /************************************************************/
          if ( fOK )
          {
            fOK = LstEditSaveList( hwnd, pIda );
          } /* endif */

          /************************************************************/
          /* Send created/changed message                             */
          /************************************************************/
          if ( fOK && (pIda->usListType != ABR_TYPE) && (pIda->usListType != ADD_TYPE)  )
          {
            if ( fListExists )
            {
              EqfSend2AllHandlers( WM_EQFN_PROPERTIESCHANGED,
                                   MP1FROMSHORT( PROP_CLASS_LIST ),
                                   MP2FROMP(pIda->szListPath) );
            }
            else
            {
              EqfSend2AllHandlers( WM_EQFN_CREATED,
                                   MP1FROMSHORT( clsLIST ),
                                   MP2FROMP(pIda->szListPath) );
            } /* endif */
          } /* endif */

          /************************************************************/
          /* Leave dialog if save completed OK                        */
          /************************************************************/
          if ( fOK  )
          {
            DelCtrlFont(hwnd, ID_LISTEDIT_TERM_MLE);
            WinDismissDlg( hwnd, fOK );
          } /* endif */
          break;
        case ID_LISTEDIT_TERM_MLE :
          if ( WMCOMMANDCMD( mp1, mp2 ) == EN_SETFOCUS )
          {
            /**********************************************************/
            /* De-select anything in the entryfield                   */
            /**********************************************************/
            SendDlgItemMessage( hwnd, ID_LISTEDIT_TERM_MLE, EM_SETSEL,
                                MP1FROMSHORT( -1 ),
                                MP2FROMSHORT( 0 ) );
          }
          else if ( WMCOMMANDCMD( mp1, mp2 ) == EN_KILLFOCUS )
          {
            ClearIME( hwnd );
          } /* endif */
          break;
      } /* endswitch */
      break;

    case WM_EQF_CLOSE:
      pIda = ACCESSDLGIDA( hwnd, PLISTEDITIDA );
      /****************************************************************/
      /* Do change check only for existing lists                      */
      /****************************************************************/
      if ( pIda->szListName[0] && MLECHANGED( hwnd, ID_LISTEDIT_TERM_MLE ) )
      {
        pszTemp = pIda->szListName;
        usMBCode = UtlErrorHwnd( ERROR_LST_LIST_UPDATED, MB_YESNOCANCEL, 1,
                                 &pszTemp, EQF_ERROR , hwnd );
        switch ( usMBCode )
        {
          case MBID_NO:
            /******************************************************/
            /* Leave dialog, discard any changes                  */
            /******************************************************/
            DelCtrlFont(hwnd, ID_LISTEDIT_TERM_MLE);
            WinDismissDlg( hwnd, DID_CANCEL );
            break;
          case MBID_YES:
            /******************************************************/
            /* Save list: post WM_COMMAND for save pushbutton     */
            /******************************************************/
            WinPostMsg( hwnd, WM_COMMAND, MP1FROMSHORT(ID_LISTEDIT_SAVE_PB),
                        0L );
            break;
          case MBID_CANCEL:
            /******************************************************/
            /* Do nothing; i.e. stay in dialog                    */
            /******************************************************/
            break;
        } /* endswitch */
      }
      else
      {
        DelCtrlFont(hwnd, ID_LISTEDIT_TERM_MLE);
        WinDismissDlg( hwnd, DID_CANCEL );
      } /* endif */
      break;

    case WM_DESTROY:
      pIda = ACCESSDLGIDA( hwnd, PLISTEDITIDA );
      if ( pIda )
      {
        switch ( pIda->usListType )
        {
          case NOISE_TYPE :
          case EXCL_TYPE :
            break;

          case ABR_TYPE :
          case ADD_TYPE :
            if ( pIda->sLangID != - 1 )
            {
              MorphFreeLanguageID( pIda->sLangID );
            } /* endif */
            break;
        } /* endswitch */
      }
      else
      {
      } /* endif */
      /****************************************************************/
      /* Nothing to do, IDA is owned by list processor ...            */
      /****************************************************************/
      break;

    default :
      mResult = WinDefDlgProc( hwnd, msg, mp1, mp2 );
      break;
  } /* endswitch */


  return( mResult );

} /* end of function LSTEDITLISTSDLG */


BOOL LstEditSaveList
(
HWND        hwnd,                    // handle of edit list dialog window
PLISTEDITIDA pIda                    // pointer to dialog IDA
)
{
  BOOL        fOK = TRUE;              // function return code
  PSZ_W       pBuffer = NULL;          // buffer for MLE export
  PSZ_W       pszTargBuf = NULL;       // buffer for noise/exclusion list
  LONG        lTextSize = 0;               // overall length of MLE text
  PSZ_W         pTermList = NULL;        // ptr to term list (ADD and ABBR only)
  ULONG       ulSize = 0;              // current size of term list
  ULONG       ulUsed = 0;              // used bytes in term list
  ULONG       ulOemCP = 0L;
  USHORT      usRC = 0;

  if ( pIda->szListName[0] )
  {
    ulOemCP = GetLangOEMCP(pIda->szListName);
  }
  // Allocate buffer for MLE export
  if ( fOK  )
  {
    MLEQUERYTEXTLENGTH( hwnd, ID_LISTEDIT_TERM_MLE, lTextSize );
    //fOK = UtlAlloc( (PVOID *)&pBuffer, 0L, (lTextSize + 10L) * sizeof(CHAR_W), ERROR_STORAGE );
    pBuffer = new CHAR_W[lTextSize+10L];
    if(pBuffer == NULL)
        fOK = FALSE;
  } /* endif */

  // for noise and exclusion lists: allocate second buffer for list data
  if ( fOK &&
       ((pIda->usListType == NOISE_TYPE) || (pIda->usListType == EXCL_TYPE)))
  {
    LONG lNewLen = ((lTextSize * 3L / 2L) + 20) * sizeof(CHAR_W);

    if ( lNewLen > (LONG)MAX_ALLOC )
    {
      lNewLen = MAX_ALLOC;             // exclusion list may not exceed 64k!
    } /* endif */
    fOK = UtlAlloc( (PVOID *) &pszTargBuf, 0L, lNewLen, ERROR_STORAGE );
  } /* endif */

  // export MLE data into buffer
  if ( fOK  )
  {
    //MLEEXPORTW( hwnd, ID_LISTEDIT_TERM_MLE, pBuffer, lTextSize );
    lTextSize = GetDlgItemTextW( hwnd, ID_LISTEDIT_TERM_MLE, pBuffer, lTextSize );
    pBuffer[lTextSize+1] = EOS;
    
    if (stricmp(pIda->szListName, THAI_STR) != 0 )
    {
      if ( (pIda->usListType != ABR_TYPE) &&
           (pIda->usListType != ADD_TYPE) )
      {
        //AnsiUpper( pBuffer );
        UtlUpperW(pBuffer);
      } /* endif */
    }
  } /* endif */


  // Convert terms to NULL terminated strings (NOISE and EXCL) or add
  // terms to term list (ADD and ABBREV)
  if ( fOK  )
  {
    PSZ_W pszSource, pszTarget;

    if ( (pIda->usListType == ABR_TYPE) ||
         (pIda->usListType == ADD_TYPE) )
    {
      /****************************************************************/
      /* Handling for abbreviation and addendum dictionaries          */
      /****************************************************************/
      pszSource = pBuffer;
      while ( fOK && *pszSource )
      {
        PSZ_W pszTermStart, pszTermEnd;  // ptr to current term

        // skip whitespace at beginning of term
        while ( ( *pszSource == BLANK ) ||
                ( *pszSource == LF )    ||
                ( *pszSource == CR ) )
        {
          pszSource++;
        } /* endwhile */

        // remember start of term
        pszTermStart = pszSource;

        // look for end of term
        while ( *pszSource && ( *pszSource != LF ) && (*pszSource != CR) )
        {
          //*pszSource++;
            pszSource++;
        } /* endwhile */

        // Remove trailing blanks from term
        pszTermEnd = pszSource;
        while ( ( pszTermEnd > pszTermStart ) &&
                ( pszTermEnd[-1] == BLANK ) )
        {
          pszTermEnd--;
        } /* endwhile */

        // For abbreviation terms only: add closing period if there is none
        if ( pIda->usListType == ABR_TYPE )
        {
          if ( pszTermEnd > pszTermStart )
          {
            if ( *(pszTermEnd-1) != DOT )
            {
              *pszTermEnd++ = DOT;
            } /* endif */
          } /* endif */
        } /* endif */

        // Terminate current term (if any) and add tern to term list
        if ( pszTermEnd > pszTermStart )
        {
          CHAR_W chTemp;
          CHAR_W  chTerm[128]={0};
          ULONG  ulLen;
          chTemp = *pszTermEnd;
          *pszTermEnd = EOS;         // terminate current term

          /*ulLen = Unicode2ASCIIBuf( pszTermStart, chTerm,
                                    UTF16strlenCHAR(pszTermStart),
                                    sizeof(chTerm), ulOemCP );
          */
          wcsncpy(chTerm, pszTermStart, wcslen(pszTermStart));
          chTerm[sizeof(chTerm)/sizeof(CHAR_W)-1] = EOS;
          ulLen = wcslen(chTerm);
          //chTerm[ulLen] = EOS;
          // Abbreviation/Addenda need data in ASCII ...
          fOK = ( MorphAddTermToList2W( &pTermList, &ulSize, &ulUsed,
                                      chTerm, (USHORT)ulLen,
                                      0, 0L,     // no offset, no flags
                                      MORPH_LARGE_ZTERMLIST ) == NO_ERROR );
          *pszTermEnd = chTemp;;

          pszSource++;           // position to next term
        } /* endif */
      } /* endwhile */


      // create an empty term list if none created yet
      if ( fOK && (pTermList == NULL) )
      {
        fOK = ( MorphAddTermToList2W( &pTermList, &ulSize, &ulUsed, L"", 0,
                                    0, 0L,     // no offset, no flags
                                    MORPH_LARGE_ZTERMLIST ) == NO_ERROR );
      } /* endif */
    }
    else
    {
      /****************************************************************/
      /* Handling for noise and exclusion lists                       */
      /****************************************************************/
      PSZ_W pszTermStart;                //ptr to current term

      pszSource = pBuffer;
      pszTarget = pszTargBuf;

      while ( *pszSource )
      {
        pszTermStart = pszTarget;

        // skip whitespace at beginning of word
        while ( ( *pszSource == BLANK ) ||
                ( *pszSource == LF )    ||
                ( *pszSource == CR ) )
        {
          pszSource++;
        } /* endwhile */

        // copy characters up to new line or end of text
        while ( *pszSource && ( *pszSource != LF ) && (*pszSource != CR) )
        {
          *pszTarget++ = *pszSource++;
        } /* endwhile */

        // Remove trailing blanks from term
        while ( ( pszTarget > pszTermStart ) &&
                ( pszTarget[-1] == BLANK ) )
        {
          pszTarget--;
        } /* endwhile */

        // Terminate current term (if any)
        if ( pszTarget > pszTermStart )
        {
          *pszTarget++ = EOS;
          pszSource++;           // position to next term
        } /* endif */
      } /* endwhile */

      // Terminate term buffer
      *pszTarget++ = EOS;
      *pszTarget++ = EOS;
    } /* endif */
  } /* endif */


  /************************************************************/
  /* Write list to disk or to dictionary                      */
  /************************************************************/
  if ( fOK  )
  {
    switch ( pIda->usListType )
    {
      case NOISE_TYPE :
      case EXCL_TYPE :
        fOK = LstWriteExclList( pIda->szListPath, pszTargBuf );
        break;

      case ABR_TYPE :
        usRC = MorphBuildDict( pIda->sLangID, ABBREV_DICT,
                        ulSize, pTermList, MORPH_LARGE_ZTERMLIST );
        break;

      case ADD_TYPE :
        usRC = MorphBuildDict( pIda->sLangID, ADDENDA_DICT,
                        ulSize, pTermList, MORPH_LARGE_ZTERMLIST );
        break;
    } /* endswitch */
  } /* endif */

  if (usRC != 0)
  {
	  UtlErrorHwnd( ERROR_ADDENDAORABBREV_NOTPOSSIBLE, MB_CANCEL,
	                0, NULL, EQF_ERROR, hwnd );

  }
  // cleanup
  if ( pBuffer!=NULL )  delete []pBuffer; //UtlAlloc( (PVOID *)&pBuffer, 0L, 0L, NOMSG );
  if ( pszTargBuf )  UtlAlloc( (PVOID *)&pszTargBuf, 0L, 0L, NOMSG );
  if ( pTermList )   MorphFreeTermList( pTermList, MORPH_LARGE_ZTERMLIST );

  return( fOK );
} /* end of function LstEditSaveList */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     LSTMARKDICTDLG   Dialog procedure for the Mark dialog
//------------------------------------------------------------------------------
// Function call:     LSTMARKDICTDLG( HWND  hwnd, USHORT msg,
//                                    MPARAM mp1, MPARAM mp2 );
//------------------------------------------------------------------------------
// Description:       Mark terms selected in the WWL dialog for dictionaries.
//------------------------------------------------------------------------------
// Input parameter:  HWND    hwnd     handle of window
//                   USHORT  msg      type of message
//                   MPARAM  mp1      first message parameter
//                   MPARAM  mp2      second message parameter
//------------------------------------------------------------------------------
// Returncode type:  MRESULT
//------------------------------------------------------------------------------
// Returncodes:      depends on message type
//                   normal return codes are:
//                   TRUE  = message has been processed
//                   FALSE = message has not been processed
//------------------------------------------------------------------------------
// Function flow:     switch msg
//                    switch message
//                      case WM_INITDLG:
//                        anchor WWL dialog IDA
//                        fill dictionary list box with available dictionaries
//                        activate first selected term term (WM_EQF_REINIT)
//                      case WM_EQF_REINIT:
//                        get first selected term of WWL terms list box
//                        if none found then
//                          dismmiss the dialog
//                        else
//                          display term, translation, context and selected
//                           dictionary
//                          deselect the term in the WWL dialog terms list box
//                        endif
//                      case WM_CONTROL:
//                        if control = context checkbox then
//                          set new read-only state of context MLE depending
//                           on check state of context checkbox
//                        endif
//                      case WM_COMMAND:
//                        switch control ID
//                          case DID_CANCEL:
//                          case ID_LISTMARK_CANCEL_PB:
//                            dismiss the dialog
//                          case ID_LISTMARK_MARK_PB:
//                            if a translation has been specified then
//                              add translation to term data
//                            endif;
//                            if context checkbox is checked then
//                              add text from context MLE to term data
//                                replacing any existing context
//                            else
//                              clear context flag of term
//                            endif
//                            add selected dictionary to term data
//                            mark term as dictionary term
//                            if view in WWL dialog is not 'All' or dictionary
//                              remove term from WWL dialog terms list box
//                            endif
//                            deselect term in WWL dialog terms list box
//                            post WM_EQF_REINIT to process next term
//                          case ID_LISTMARK_SKIP_PB:
//                            deselect term in WWL dialog terms list box
//                            post WM_EQF_REINIT to process next term
//                        endswitch
//                    endswitch
//------------------------------------------------------------------------------
INT_PTR CALLBACK LSTMARKDICTDLG
(
HWND   hwnd,
WINMSG msg,
WPARAM mp1,
LPARAM mp2
)
{
  MRESULT   mResult = FALSE;
  PWWLDLGIDA pIda;                     // pointer to dialog IDA
  PTERM      pTerm;                    // pointer to current term
  PSZ_W      pszContext;               // pointer to current context
  USHORT     usContext;                // index for context list processing
  SHORT      sItem;                    // list box item index
  HWND       hwndMLE;                  // handle of context MLE
  BOOL       fOK;                      // internal OK flag
  LONG       lTextSize;                // size of text in context MLE

  switch ( msg )
  {
    case WM_EQF_QUERYID:
      pIda = ACCESSDLGIDA( hwnd, PWWLDLGIDA );
      if ( pIda->usListType == NTL_TYPE )
      {
        HANDLEQUERYID( ID_LISTMARK_NTL_DLG, mp2 );
      }
      else
      {
        HANDLEQUERYID( ID_LISTMARK_FTL_DLG, mp2 );
      } /* endif */
      break;


    case WM_INITDLG:
      /****************************************************************/
      /* Anchor IDA                                                   */
      /****************************************************************/
      pIda = (PWWLDLGIDA)mp2;
      ANCHORDLGIDA( hwnd, pIda );

      /****************************************************************/
      /* Switch dialog ID to allow different helps for the dialog     */
      /****************************************************************/
      if ( pIda->usListType == NTL_TYPE )
      {
        SETWINDOWID( hwnd, ID_LISTMARK_NTL_DLG );
      }
      else
      {
        SETWINDOWID( hwnd, ID_LISTMARK_FTL_DLG );
      } /* endif */

      /****************************************************************/
      /* Fill-in title bar text                                       */
      /****************************************************************/

      /****************************************************************/
      /* Fill dictionary list box                                     */
      /****************************************************************/
      UtlCopyListBox( WinWindowFromID( hwnd, ID_LISTMARK_DICTIONARY_LB), pIda->hwndDictLB );
      SetCtrlFnt(hwnd, GetCharSet(), ID_LISTMARK_TERM_EF, ID_LISTMARK_TRANS_EF );
      SetCtrlFnt(hwnd, GetCharSet(), ID_LISTMARK_CONTEXT_MLE,0);
      SETTEXTLIMIT( hwnd, ID_LISTMARK_TRANS_EF, 255);

      /****************************************************************/
      /* Activate first selected term                                 */
      /****************************************************************/
      pIda->sMarkItem = LIT_FIRST;
      WinPostMsg( hwnd, WM_EQF_REINIT, 0L, 0L );
      break;

    case WM_EQF_REINIT:
      pIda = ACCESSDLGIDA( hwnd, PWWLDLGIDA );

      /****************************************************************/
      /* Get next term to be processed = first selected term in terms */
      /* list box of WWL dialog                                       */
      /****************************************************************/
      pIda->sMarkItem = QUERYNEXTSELECTION( pIda->hwnd, ID_LISTWORK_TERM_LB, pIda->sMarkItem );
      if ( pIda->sMarkItem == LIT_NONE )
      {
        /**************************************************************/
        /* all selected entries have been processed, leave the dialog */
        /**************************************************************/
        SendDlgItemMessage( hwnd, ID_LISTWORK_TERM_LB, LB_SETSEL, MP1FROMSHORT(FALSE), MP2FROMSHORT(-1) );
        DelCtrlFont( hwnd, ID_LISTMARK_TERM_EF );
        DelCtrlFont( hwnd, ID_LISTMARK_CONTEXT_MLE );
        WinDismissDlg( hwnd, TRUE );
      }
      else
      {
        pTerm = pIda->pMarkTerm = (PTERM) QUERYITEMHANDLE( pIda->hwnd, ID_LISTWORK_TERM_LB, pIda->sMarkItem );

        /****************************************************************/
        /* Fill dialog fields with values of the term                   */
        /****************************************************************/
        SETTEXTW( hwnd, ID_LISTMARK_TERM_EF, pTerm->pszName );
        SETCHECK( hwnd, ID_LISTMARK_CONTEXT_CHK, pTerm->Flags.fContext );
        usContext = 0;
        hwndMLE = WinWindowFromID( hwnd, ID_LISTMARK_CONTEXT_MLE );
        SETTEXTHWND( hwndMLE, "" );
        if ( pTerm->pszContext )
        {
          LstSetMLEText( pTerm->pszContext, hwndMLE );
        }
        else if ( pTerm->pContextList )
        {
          PSZ_W  pszBuffer = NULL;
          LONG lBufLen = 0L;

          // get required buffer size
          usContext = 0;
          while ( usContext < pTerm->pContextList->usUsed )
          {
            pszContext = LstGetContext( pIda->pContextTable,
                                        pTerm->pContextList->ausContextID[usContext] );
            lBufLen += UTF16strlenCHAR( pszContext ) + 3;
            usContext++;
          } /* endwhile */

          // allocate buffer
          if ( lBufLen )
          {
            if ( lBufLen < MIN_ALLOC ) lBufLen = MIN_ALLOC;
            UtlAlloc( (PVOID *)&pszBuffer, 0L, lBufLen * sizeof(CHAR_W), ERROR_STORAGE );
          } /* endif */

          // fill buffer
          if ( pszBuffer )
          {
            usContext = 0;
            while ( usContext < pTerm->pContextList->usUsed )
            {
              pszContext = LstGetContext( pIda->pContextTable,
                                          pTerm->pContextList->ausContextID[usContext] );
              if ( usContext ) UTF16strcat( pszBuffer, CRLF_STRINGW );
              UTF16strcat( pszBuffer, pszContext );
              usContext++;
            } /* endwhile */
          } /* endif */

          // fill context MLE
          if ( pszBuffer )
          {
            LstSetMLEText( pszBuffer, hwndMLE );
            UtlAlloc( (PVOID *)&pszBuffer, 0L, 0L, NOMSG );
          } /* endif */
        } /* endif */
        MLESETCHANGEDHWND( hwndMLE, FALSE );
        MLESETREADONLYHWND( hwndMLE, !pTerm->Flags.fContext );

        if ( pTerm->pszTranslation )
        {
          SETTEXTW( hwnd, ID_LISTMARK_TRANS_EF, pTerm->pszTranslation );
        }
        else
        {
          SETTEXT( hwnd, ID_LISTMARK_TRANS_EF, EMPTY_STRING );
        } /* endif */

        if ( pTerm->Flags.fMark == DICTMARK_FLAG )
        {
          ObjShortToLongName( pTerm->pszDestination, pIda->szBuffer, DICT_OBJECT );
          OEMTOANSI( pIda->szBuffer );
          SEARCHSELECT( sItem, hwnd, ID_LISTMARK_DICTIONARY_LB, pIda->szBuffer );
        }
        else
        {
          sItem = LIT_NONE;
        } /* endif */

        if ( (sItem == LIT_NONE) && pIda->pszDictionary )
        {
          ObjShortToLongName( pIda->pszDictionary, pIda->szBuffer, DICT_OBJECT );
          OEMTOANSI( pIda->szBuffer );
          SEARCHSELECT( sItem, hwnd, ID_LISTMARK_DICTIONARY_LB, pIda->szBuffer );
        } /* endif */

        if ( sItem == LIT_NONE )
        {
          SELECTITEM( hwnd, ID_LISTMARK_DICTIONARY_LB, 0 );
        } /* endif */

        SETFOCUS( hwnd, ID_LISTMARK_TRANS_EF );
      } /* endif */
      break;


    case WM_COMMAND :
      switch ( WMCOMMANDID( mp1, mp2 ))
      { case ID_LISTMARK_HELP_PB:
          mResult = UtlInvokeHelp();
          break;
        case ID_LISTMARK_MARK_PB :
          /************************************************************/
          /* Address IDA and term data                                */
          /************************************************************/
          fOK = TRUE;
          pIda = ACCESSDLGIDA( hwnd, PWWLDLGIDA );
          pTerm = pIda->pMarkTerm;

          /************************************************************/
          /* Process term's translation                               */
          /************************************************************/
          QUERYTEXTW( hwnd, ID_LISTMARK_TRANS_EF, pIda->szUnicodeBuffer );

          UtlStripBlanksW( pIda->szUnicodeBuffer );
          if ( pIda->szUnicodeBuffer[0] )
          {
            /**********************************************************/
            /* Replace terms translation if translation is new or has */
            /* been changed                                           */
            /**********************************************************/
            if ( !pTerm->pszTranslation ||
                 ( UTF16strcmp( pTerm->pszTranslation, pIda->szUnicodeBuffer ) != 0 ) )
            {
              pTerm->pszTranslation = PoolAddStringW( pIda->pPool,
                                                     pIda->szUnicodeBuffer );
              if ( !pTerm->pszTranslation  )
              {
                UtlErrorHwnd( ERROR_STORAGE, MB_CANCEL, 0, NULL, EQF_ERROR , hwnd );
                fOK = FALSE;
              } /* endif */
            } /* endif */
          }
          else
          {
            /********************************************************/
            /* Remove any term translation                          */
            /********************************************************/
            pTerm->pszTranslation = NULL;
          } /* endif */

          /************************************************************/
          /* Process context flag                                     */
          /************************************************************/
          if ( fOK )
          {
            if ( QUERYCHECK( hwnd, ID_LISTMARK_CONTEXT_CHK ) )
            {
              hwndMLE = WinWindowFromID( hwnd, ID_LISTMARK_CONTEXT_MLE );
              pTerm->Flags.fContext = TRUE;

              /******************************************************/
              /* Get context and store it in string pool            */
              /******************************************************/
              MLEQUERYTEXTLENGTHHWND( hwndMLE, lTextSize );
              lTextSize = min( lTextSize,
                               (LONG)(sizeof(pIda->szUnicodeBuffer) - 2));
              MLEEXPORTHWNDW( hwndMLE, pIda->szUnicodeBuffer, lTextSize );
              pIda->szUnicodeBuffer[lTextSize] = EOS;

              pTerm->pszContext = PoolAddStringW( pIda->pPool,
                                                 pIda->szUnicodeBuffer );
              if ( !pTerm->pszContext  )
              {
                UtlErrorHwnd( ERROR_STORAGE, MB_CANCEL, 0, NULL, EQF_ERROR , hwnd );
                fOK = FALSE;
              } /* endif */
            }
            else
            {
              pTerm->Flags.fContext = FALSE;
            } /* endif */
          } /* endif */

          /************************************************************/
          /* Process selected dictionary                              */
          /************************************************************/
          if ( fOK )
          {
            sItem = QUERYSELECTION( hwnd, ID_LISTMARK_DICTIONARY_LB );
            if ( sItem != LIT_NONE )
            {
              BOOL fIsNew = FALSE;

              QUERYITEMTEXT( hwnd, ID_LISTMARK_DICTIONARY_LB, sItem, pIda->szBuffer );
              ANSITOOEM( pIda->szBuffer );
              ObjLongToShortName( pIda->szBuffer, pIda->szMarkBuffer, DICT_OBJECT, &fIsNew );

              if ( !pTerm->pszDestination || ( strcmp( pTerm->pszDestination, pIda->szMarkBuffer ) != 0 ) )
              {
                if ( !pIda->pszDictionary || ( strcmp( pIda->pszDictionary, pIda->szMarkBuffer ) != 0 ) )
                {
                  pTerm->pszDestination = PoolAddString( pIda->pPool, pIda->szMarkBuffer );
                  if ( !pTerm->pszDestination  )
                  {
                    UtlErrorHwnd( ERROR_STORAGE, MB_CANCEL, 0, NULL, EQF_ERROR , hwnd );
                    fOK = FALSE;
                  }
                  else
                  {
                    pIda->pszDictionary = pTerm->pszDestination;
                  } /* endif */
                }
                else
                {
                  /****************************************************/
                  /* Dictionary name pointer is available, use this   */
                  /* pointer as destination pointer                   */
                  /****************************************************/
                  pTerm->pszDestination = pIda->pszDictionary;
                } /* endif */
              }
              else
              {
                /******************************************************/
                /* Nothing to do: pszDestination points to the        */
                /* correct name                                       */
                /******************************************************/
              } /* endif */
            }
            else
            {
              /********************************************************/
              /* Should never get active as there is always a         */
              /* selected dictionary                                  */
              /********************************************************/
            } /* endif */
          } /* endif */

          /************************************************************/
          /* if ok mark term as dictionary term, remove or deselect   */
          /* the term in terms list box of the WWL dialog             */
          /* and activate the next term by posting WM_EQF_REINIT      */
          /************************************************************/
          if ( fOK )
          {
            pTerm->Flags.fMark = DICTMARK_FLAG;
            pIda->fListChanged = TRUE;
            if ( (pIda->usView == VIEWALL_FLAG) ||
                 (pTerm->Flags.fMark == pIda->usView) )
            {
              SETITEMTEXTW( hwnd, ID_LISTWORK_TERM_LB, pIda->sMarkItem, pTerm->pszName );
            }
            else
            {
              DELETEITEM( pIda->hwnd, ID_LISTWORK_TERM_LB, pIda->sMarkItem );
              pIda->sMarkItem = ( pIda->sMarkItem == 0 ) ? LIT_FIRST :
                                pIda->sMarkItem - 1;
            } /* endif */
            WinPostMsg( hwnd, WM_EQF_REINIT, 0L, 0L );
          } /* endif */

          /************************************************************/
          /* In case of errors end the dialog                         */
          /************************************************************/
          if ( !fOK )
          {
            WinDismissDlg( hwnd, FALSE );
          } /* endif */
          break;

        case ID_LISTMARK_SKIP_PB :
          /************************************************************/
          /* Address IDA                                              */
          /************************************************************/
          pIda = ACCESSDLGIDA( hwnd, PWWLDLGIDA );

          /************************************************************/
          /* Deselect current term in terms list box of WWL dialog    */
          /* and activate the next term by posting WM_EQF_REINIT      */
          /************************************************************/
          DESELECTITEM( pIda->hwnd, ID_LISTWORK_TERM_LB, pIda->sMarkItem );
          WinPostMsg( hwnd, WM_EQF_REINIT, 0L, 0L );
          break;
        case  DID_CANCEL:
        case  ID_LISTMARK_CANCEL_PB:
          DelCtrlFont( hwnd, ID_LISTMARK_TERM_EF );
          DelCtrlFont( hwnd, ID_LISTMARK_CONTEXT_MLE );
          WinDismissDlg( hwnd, FALSE );
          break;

          /**************************************************************/
          /* WM_CONTROL handling for Windows!                           */
          /**************************************************************/
        case ID_LISTMARK_CONTEXT_CHK :
          if ( QUERYCHECK( hwnd, ID_LISTMARK_CONTEXT_CHK ) )
          {
            MLESETREADONLY( hwnd, ID_LISTMARK_CONTEXT_MLE, FALSE );
          }
          else
          {
            MLESETREADONLY( hwnd, ID_LISTMARK_CONTEXT_MLE, TRUE );
          } /* endif */
          break;
        case ID_LISTMARK_TERM_EF:
        case ID_LISTMARK_CONTEXT_MLE:
          break;
        } /* endswitch */
      break;

    default :
      mResult = WinDefDlgProc( hwnd, msg, mp1, mp2 );
      break;
  } /* endswitch */

  return( mResult );


} /* end of function LSTEDITLISTSDLG */


//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     LstWorkSetSelFlags     Set selection and top index flags
//------------------------------------------------------------------------------
// Function call:     LstWorkSetSelFlags( PWWLDLGIDA pIda );
//------------------------------------------------------------------------------
// Description:       Sets the selection and top index flags of the terms
//                    displayed in the term listbox of the work-with-lists
//                    dialog.
//------------------------------------------------------------------------------
// Input parameter:   PWWLDLGIDA  pIda    pointer to IDA of work-with-lists dlg
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:     clear all selection and top-index flags
//                    loop over all selected list box items
//                      get pointer to term data
//                      set term's selected flag
//                    endloop
//                    get index of first visible listbox item
//                    get pointer to term data
//                    set term's top-index flag
//------------------------------------------------------------------------------
VOID LstWorkSetSelFlags
(
PWWLDLGIDA  pIda                     // pointer to IDA of work-with-lists dlg
)
{
  PTERM      pTerm;                    // ptr to current term
  SHORT      sItem;                    // index of selected items

  /********************************************************************/
  /* Clear all selection and top index flags                          */
  /********************************************************************/
  LstWorkClearSelFlags( pIda );

  /********************************************************************/
  /* Set term selected flags                                          */
  /********************************************************************/
  sItem = QUERYITEMCOUNT( pIda->hwnd, ID_LISTWORK_TERM_LB );
  if ( sItem > 0 )
  {
    sItem = LIT_FIRST;
    do
    {
      sItem = QUERYNEXTSELECTION( pIda->hwnd, ID_LISTWORK_TERM_LB, sItem );
      if ( sItem != LIT_NONE )
      {
        pTerm = (PTERM)QUERYITEMHANDLE( pIda->hwnd, ID_LISTWORK_TERM_LB, sItem );
        pTerm->Flags.fSelected = TRUE;
      } /* endif */
    } while ( sItem != LIT_NONE ); /* enddo */

    /********************************************************************/
    /* Set top index flag                                               */
    /********************************************************************/
    sItem = LBQUERYTOPINDEX( pIda->hwnd, ID_LISTWORK_TERM_LB );
    if ( sItem != LIT_NONE )
    {
      pTerm = (PTERM)QUERYITEMHANDLE( pIda->hwnd, ID_LISTWORK_TERM_LB, sItem );
      pTerm->Flags.fTop = TRUE;
    } /* endif */
  } /* endif */

} /* end of function LstWorkSetSelFlags */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     LstWorkClearSelFlags   Clear sel. and top index flags
//------------------------------------------------------------------------------
// Function call:     LstWorkClearSelFlags( PWWLDLGIDA pIda );
//------------------------------------------------------------------------------
// Description:       Clear the selection and top index flags of the terms
//                    in the term table.
//------------------------------------------------------------------------------
// Input parameter:   PWWLDLGIDA  pIda    pointer to IDA of work-with-lists dlg
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:     while not end of term tables
//                      loop over all terms in the current table
//                        reset term selection flag
//                        reset term top-index flag
//                        continue with next term in table
//                      endloop
//                      continue with next tag table
//                    endwhile
//------------------------------------------------------------------------------
VOID LstWorkClearSelFlags
(
PWWLDLGIDA  pIda                     // pointer to IDA of work-with-lists dlg
)
{
  USHORT     usTerm;                   // index into term table
  PTERMTABLE pTermTable;               // ptr to current term table
  PTERM      pTerm;                    // ptr to current term

  /******************************************************************/
  /* Start at first term table                                      */
  /******************************************************************/
  pTermTable = pIda->pTermTable;

  /******************************************************************/
  /* Loop over all term tables                                      */
  /******************************************************************/
  while ( pTermTable )
  {
    /****************************************************************/
    /* start at first term of term table                            */
    /****************************************************************/
    pTerm = (PTERM)(pTermTable+1);

    /****************************************************************/
    /* loop over all terms in the current table                     */
    /****************************************************************/
    for ( usTerm = 0; usTerm < pTermTable->usUsedEntries; usTerm++, pTerm++ )
    {
      /**************************************************************/
      /* Reset selection and top index flag                         */
      /**************************************************************/
      pTerm->Flags.fSelected = FALSE;
      pTerm->Flags.fTop      = FALSE;
    } /* endfor */

    /****************************************************************/
    /* continue with next term table                                */
    /****************************************************************/
    pTermTable = pTermTable->pNextTable;
  } /* endwhile */

} /* end of function LstWorkClearSelFlags */

/**********************************************************************/
/* Set the text of a MLE                                              */
/* perform OEM conversion and LF -> CRLF conversion                   */
/**********************************************************************/
BOOL LstSetMLEText
(
PSZ_W       pszText,                 // pointer to text to set
HWND        hwndMLE                  // handle of MLE
)
{
  PSZ_W pszBuffer = NULL;
  BOOL fOK = TRUE;
  LONG  lAllocLen;

  /********************************************************************/
  /* Allocate buffer large enough to contain converted MLE text       */
  /********************************************************************/
  lAllocLen = UTF16strlenCHAR(pszText) * 2 + 1;

  fOK = UtlAlloc( (PVOID *)&pszBuffer, 0L, lAllocLen * sizeof(CHAR_W), ERROR_STORAGE );

  /********************************************************************/
  /* Copy text to our buffer and translate LF to CRLF                 */
  /********************************************************************/
  if ( fOK )
  {
    PSZ_W pszSource, pszTarget;

    pszSource = pszText;
    pszTarget = pszBuffer;

    while ( *pszSource )
    {
      if ( *pszSource == CR )
      {
        *pszTarget++ = *pszSource++;
        if ( *pszSource == LF )
        {
          *pszTarget++ = *pszSource++;
        } /* endif */
      }
      else if ( *pszSource == LF )
      {
        *pszTarget++ = CR;
        *pszTarget++ = *pszSource++;
      }
      else
      {
        *pszTarget++ = *pszSource++;
      } /* endif */
    } /* endwhile */
    *pszTarget = EOS;
  } /* endif */

  /********************************************************************/
  /* OEM convert text and import it into MLE                          */
  /********************************************************************/
  if ( fOK )
  {
    LONG ulLen = UTF16strlenCHAR( pszBuffer );

    ulLen;
    MLEIMPORTHWNDW( hwndMLE, pszBuffer, ulLen );
  } /* endif */

  if ( pszBuffer ) UtlAlloc( (PVOID *)&pszBuffer, 0L, 0L, NOMSG );

  return( fOK );
} /* end of function LstSetMLEText */


//------------------------------------------------------------------------------
//                           End of EQFLSTLP.C
//------------------------------------------------------------------------------
