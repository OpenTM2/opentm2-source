/*! \file
	Description: Dialogs used within Translation Processor

	Copyright Notice:

	Copyright (C) 1990-2014, International Business Machines
	Corporation and others. All rights reserved
*/

//-----------------------------------------------------------------------------
// EQFBDLG.C - EQF Translation Browser - Code For Dialogs
//-----------------------------------------------------------------------------

#define INCL_EQF_TP               // public translation processor functions
#define INCL_EQF_PRINT            // print functions
#define INCL_EQF_EDITORAPI        // editor API
#define INCL_EQF_TAGTABLE         // tagtable defines and functions
#include <eqf.h>                  // General Translation Manager include file

#include "EQFTPI.H"               // Translation Processor priv. include file
#include "EQFB.ID"                // Translation Processor IDs
#include "EQFBDLG.ID"                  // dialog control IDs

CHAR aszColorNames[MAXVIOCOLOR][MAX_COLOR_NAME];
CHAR aszTextNames[MAXCOLOUR][MAX_TEXTTYPE_NAME];
CHAR aszWindowNames[MAX_DIF_DOC][MAX_TEXTTYPE_NAME];

#define ACCESSCTRLIDA( hwnd, type ) \
    (type)GetWindowLong( hwnd, GWL_USERDATA )

  /********************************************************************/
  /* SetWindowLong returns the value of the last long set and 0L      */
  /* in case of error - for the first call it's the same, therefore   */
  /* try again to be sure....                                         */
  /********************************************************************/
  #define ANCHORCTRLIDA( hwnd, pIda ) \
   ((SetWindowLong( hwnd, GWL_USERDATA, (LONG)((PVOID) pIda) )) ?                   \
                    1 : SetWindowLong( hwnd, GWL_USERDATA, (LONG)((PVOID) pIda) ) != 0L )


MRESULT EQFBFontColInit ( HWND, WPARAM, LPARAM );
MRESULT EQFBFontColControl( HWND, SHORT, SHORT );
MRESULT EQFBFontColCommand( HWND, WPARAM, LPARAM );
MRESULT EQFBFontColClose( HWND, WPARAM, LPARAM );

MRESULT APIENTRY EQFBFONTCOLSAMPLESUBPROC ( HWND, WINMSG, WPARAM, LPARAM );
MRESULT APIENTRY EQFBFONTSIZESAMPLESUBPROC ( HWND, WINMSG, WPARAM, LPARAM );



//dalia (start)
//#ifdef _USE_AVIO
  static VOID GetFontSizes ( HWND, PFONTSIZEDATA );          // get fonts
//#else
  static VOID GetFonts ( HWND, PFONTSIZEDATA );              // get fonts
//#endif
//dalia (end)


MRESULT EQFBFontSizeControl( HWND, SHORT, SHORT );
MRESULT EQFBFontSizeCommand( HWND, WPARAM, LPARAM );

PFNWP OrgStaticProc;

int CALLBACK EnumAllFontSizes
  (
    LOGFONT       * pLF,                 // address of logical font structure
    NEWTEXTMETRIC * pNTM,                // address of physical font struct
    SHORT         sFontType,             // font type
    PBYTE         pData                  // address of application defined data
  );

int CALLBACK EnumAllFonts
  (
    LOGFONT       * pLF,                 // address of logical font structure
    NEWTEXTMETRIC * pNTM,                // address of physical font struct
    SHORT         sFontType,             // font type
    ENUMFACE FAR  * lpef
  );


static VOID FillSizeLB ( HWND, PFONTSIZEDATA, USHORT); // get sizes of spec.font
int FontListSort ( const void * pElem1, const void * pElem2 );
   LONG   lGLogPixy;
   LONG   lGLogPixx;

static WORD EQFBGetCharWidth ( PVIOFONTCELLSIZE , PSZ ); // calc charwidth


#ifdef _USE_AVIO
#else
  static VOID FillSizeLB ( HWND, PFONTSIZEDATA, USHORT);   // get sizes of spec.font
#endif
/*////////////////////////////////////////////////////////////////////////////
:H2.EQFBFONTCOLDLGPROC - dialog procedure for font/color dialog
*/
// Description:
//    Allow the selection of colors and fonts for the translation
//    browser text elements and windows.
//
//   Flow (message driven):
//       case WM_INITDLG:
//         call EQFBFontColInit to initialize the dialog controls;
//       case  WM_CONTROL:
//         call EQFBFontColControl to handle user interaction;
//       case WM_COMMAND:
//         call EQFBFontColCommand to handle user commands;
//       case WM_CLOSE:
//         call EQFBFontColClose to end the dialog;
//
// Arguments:
//  mp2 of WM_INITDLG msg = PFONTCOLDATA pFontColData ptr to data structure
//                                       containing current font/color
//                                       assignments
// Returns:
//  BOOL  fOK        TRUE           - dialog completed normally
//                   FALSE          - dialog was canceled or closed
//
// Prereqs:
//   None
//
// SideEffects:
//   if TRUE is returned, the FONTCOLDATA structure contains the selections
//   made by the user
//
// External references:
//   None
//
//////////////////////////////////////////////////////////////////////////////
INT_PTR CALLBACK EQFBFONTCOLDLGPROC
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
      case WM_EQF_QUERYID: HANDLEQUERYID( ID_TB_FONTCOL_DLG, mp2 ); break;

      case WM_INITDLG:
          SETWINDOWID( hwndDlg, ID_TB_FONTCOL_DLG );
          mResult = DIALOGINITRETURN( EQFBFontColInit( hwndDlg, mp1, mp2 ));
          break;

      case WM_COMMAND:
         mResult = EQFBFontColCommand( hwndDlg, mp1, mp2 );
         break;

      case WM_EQF_CLOSE:
         mResult = EQFBFontColClose( hwndDlg, mp1, mp2 );
         break;

      default:
         mResult = WinDefDlgProc( hwndDlg, msg, mp1, mp2 );
         break;
   } /* endswitch */

   return mResult;
} /* end of EQFBFONTCOLDLGPROC */

/*////////////////////////////////////////////////////////////////////////////
:h2.EQFBFontColInit - initialization for font/color dialog
*/
// Description:
//    Initialize all dialog controls and allocate required memory.
//
//   Flow:
//      - allocate and anchor dialog IDA;
//      - fill text elements listbox;
//      - fill color listboxes;
//      - create VIO PS for sample text control;
//      - subclass sample text control
//      - fill font listbox
//
// Arguments:
//  mp2 of WM_INITDLG msg = PFONTCOLDATA pFontColData ptr to data structure
//                                       containing current font/color
//                                       assignments
//
// Returns:
//  MRESULT(FALSE)
//
// Prereqs:
//   None
//
// SideEffects:
//   - ptr to IDA is stored in dialog word QWL_USER
//   - ptr to table of available fonts is stored in dialog IDA
//   - last used text element is selected thus triggering a WM_CONTROL message
//
// External references:
//   UtlAlloc
//
//////////////////////////////////////////////////////////////////////////////
MRESULT EQFBFontColInit
(
   HWND    hwndDlg,                    // handle of dialog window
   WPARAM  mp1,                        // first parameter of WM_INITDLG
   LPARAM  mp2                         // second parameter of WM_INITDLG
)
{
   MRESULT     mResult = FALSE;        // result of message processing
   PFONTCOLIDA pIda;                   // dialog instance data area ptr
   BOOL        fOK = TRUE;             // internal OK flag
   USHORT      usI;                    // loop index
   SHORT       sItem;                  // index of a listbox item
   HAB         hab;
   HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);

   mp1 = mp1;                          // suppress 'unreferenced parameter' msg
   mp2 = mp2;                          // suppress 'unreferenced parameter' msg

   hab = GETINSTANCE( hwndDlg );
   //
   //  anchor IDA
   //

   // remember address of user area
   pIda = (PFONTCOLIDA) mp2;
   fOK = (BOOL) ANCHORDLGIDA( hwndDlg, pIda );

   // load strings from resource
   if ( fOK )
   {
      // sample text
      LOADSTRING( hab, hResMod, IDS_SAMPLETEXT, pIda->szSampleText );
   } /* endif */

   //
   // remember passed arguments
   //
   if ( fOK )
   {
      fOK = UtlAlloc((PVOID *) &pIda->pTextTable, 0L, (LONG) (sizeof(TEXTTYPETABLE) * MAXCOLOUR),
                      ERROR_STORAGE );
      if ( fOK )
      {
         memcpy( pIda->pTextTable, pIda->pFontColData->pTextTypeTable,
                 sizeof(TEXTTYPETABLE) * MAXCOLOUR );
      } /* endif */
   } /* endif */

   //
   // fill text elements listbox
   //
   if ( fOK )
   {
      for ( usI = 0; usI < MAXCOLOUR; usI++ )
      {
         // load names for text types
         aszTextNames[usI][0] = EOS;
         LOADSTRING( hab, hResMod, IDS_TB_COL_EXIT+usI, aszTextNames[usI] );
         if ( aszTextNames[usI][0] != EOS )
         {
           switch ( IDS_TB_COL_EXIT+usI )
           {
             case IDS_TB_COL_NOP:
             case IDS_TB_COL_P_NOP:
             case IDS_TB_COL_P_TOBE:
               sItem = INSERTITEM( hwndDlg, ID_TB_FONTCOL_ITEM_LB,
                                   aszTextNames[usI] );
               if ( sItem != LIT_NONE )
               {
                  SETITEMHANDLE( hwndDlg, ID_TB_FONTCOL_ITEM_LB, sItem,
                                 MP2FROMSHORT( usI ) );
               } /* endif */
               break;
             case IDS_TB_COL_EXIT:
             case IDS_TB_COL_XLATED:
             case IDS_TB_COL_ACTIVE:
             case IDS_TB_COL_TOBE:
             case IDS_TB_COL_P_XLATED:
             case IDS_TB_COL_P_ACTIVE:
             case IDS_TB_COL_SRV_PROPSRCINS:
             case IDS_TB_COL_SRV_PROP0PREFIX:
             case IDS_TB_COL_SRV_PROPSRCDEL:
             case IDS_TB_COL_SRV_PROPNPREFIX:
             case IDS_TB_COL_SRV_DICTHEAD:
             case IDS_TB_COL_SRV_DICTTRANS:
             case IDS_TB_COL_SRV_DICTPREFIX:
             case IDS_TB_COL_SRV_PROPSRCEQU:
             case IDS_TB_COL_SRV_PROPSRCUNEQU:
             case IDS_TB_COL_ACTIVE_TAG:
             case IDS_TB_COL_ADDINFO:
             case IDS_TB_COL_DICTINDIC:
             case IDS_TB_COL_UNMATCHTAG:
             case IDS_TB_COL_TFROMSCRATCH:
             case IDS_TB_COL_TMODPROPOSAL:
             case IDS_TB_COL_TCOPYPROPOSAL:
             case IDS_TB_COL_STANDARD_TRNOTE:
             case IDS_TB_COL_TRNOTE_11:
             case IDS_TB_COL_TRNOTE_12:
             case IDS_TB_COL_TRNOTE_2:
             case IDS_TB_COL_MACHINE_MATCH_NORMAL:
             case IDS_TB_COL_MACHINE_MATCH_PROT:
             case IDS_TB_COL_FUZZY_MATCH_NORMAL:
             case IDS_TB_COL_FUZZY_MATCH_PROT:
             case IDS_TB_COL_DICTSTYLEPREF:
             case IDS_TB_COL_DICTSTYLENOT:
                if ((pIda->pDoc->docType == OTHER_DOC)
                    || (pIda->pDoc->docType == SSOURCE_DOC)
                    || (pIda->pDoc->docType == STARGET_DOC )
                    || (pIda->pDoc->docType == SERVPROP_DOC)
                    || (pIda->pDoc->docType == SERVDICT_DOC)
                    || (pIda->pDoc->docType == SERVSOURCE_DOC )
                    || (pIda->pDoc->docType == TRNOTE_DOC ))
               {
                 sItem = INSERTITEM( hwndDlg, ID_TB_FONTCOL_ITEM_LB,
                                     aszTextNames[usI] );
                 if ( sItem != LIT_NONE )
                 {
                    SETITEMHANDLE( hwndDlg, ID_TB_FONTCOL_ITEM_LB, sItem,
                                   MP2FROMSHORT( usI ) );
                 } /* endif */
               } /* endif */
               break;
             case IDS_TB_COL_ITM_ANCHOR_1:
             case IDS_TB_COL_ITM_ANCHOR_2:
             case IDS_TB_COL_ITM_ANCHOR_3:
             case IDS_TB_COL_ITM_VALID_01:
             case IDS_TB_COL_ITM_VALID_10:
             case IDS_TB_COL_ITM_VALID_11_1:
             case IDS_TB_COL_ITM_VALID_11_2:
             case IDS_TB_COL_ITM_VALID_11_3:
             case IDS_TB_COL_ITM_CROSSED_OUT:
             case IDS_TB_COL_ITM_NOP_ANCHOR_1:
             case IDS_TB_COL_ITM_NOP_ANCHOR_2:
             case IDS_TB_COL_ITM_NOP_ANCHOR_3:
             case IDS_TB_COL_ITM_VISACT:
             case IDS_TB_COL_ITM_OVERCROSS:
               if ((pIda->pDoc->docType == VISSRC_DOC)
                   || (pIda->pDoc->docType == VISTGT_DOC) )
               {
                 sItem = INSERTITEM( hwndDlg, ID_TB_FONTCOL_ITEM_LB,
                                     aszTextNames[usI] );
                 if ( sItem != LIT_NONE )
                 {
                    SETITEMHANDLE( hwndDlg, ID_TB_FONTCOL_ITEM_LB, sItem,
                                   MP2FROMSHORT( usI ) );
                 } /* endif */
               } /* endif */
               break;
             default:
               break;
           } /* endswitch */
         } /* endif */
      } /* endfor */
   } /* endif */

   //
   // fill color listboxes
   //
   if ( fOK )
   {
      for ( usI = 0; usI < MAXVIOCOLOR; usI++ )
      {
         // get default text for fonts
         // (based on IDS_TB_COL_BLACK)
         aszColorNames[usI][0] = EOS;
         LOADSTRING( hab, hResMod, IDS_TB_COL_BLACK+usI, aszColorNames[usI] );
         sItem = INSERTITEM( hwndDlg, ID_TB_FONTCOL_FOREGROUND_LB,
                             aszColorNames[usI] );
         if ( sItem != LIT_NONE )
         {
            SETITEMHANDLE( hwndDlg, ID_TB_FONTCOL_FOREGROUND_LB, sItem,
                           MP2FROMSHORT( usI ) );
         } /* endif */

         sItem = INSERTITEM( hwndDlg, ID_TB_FONTCOL_BACKGROUND_LB,
                             aszColorNames[usI] );
         if ( sItem != LIT_NONE )
         {
            SETITEMHANDLE( hwndDlg, ID_TB_FONTCOL_BACKGROUND_LB, sItem,
                           MP2FROMSHORT( usI ) );
         } /* endif */
      } /* endfor */
   } /* endif */

   //
   // subclass sample text control and create associated AVIO PS
   //
   pIda->hwndSample = GETHANDLEFROMID( hwndDlg, ID_TB_FONTCOL_SAMPLE_GB );
   ANCHORCTRLIDA( pIda->hwndSample, pIda );
   OrgStaticProc = SUBCLASSWND( pIda->hwndSample, EQFBFONTCOLSAMPLESUBPROC );

   //
   // select first text element item, thus triggering selection of colors
   // and fonts (through processing of LN_SELECT message)
   //
    if ( fOK )
    {
       SELECTITEM( hwndDlg, ID_TB_FONTCOL_ITEM_LB, 0 );
       UtlCheckDlgPos( hwndDlg, FALSE );
    } /* endif */


   return ( mResult );
} /* end of EQFBFontColInit */

/*////////////////////////////////////////////////////////////////////////////
:h2.EQFBFontColSampleSubProc - subclass procedure for sample text control
*/
// Description:
//    Do special handling for WM_PAINT and WM_SIZE messgae, pass all other
//    message to original window procedure.
//
//   Flow:
//      case WM_PAINT:
//      - write (paint) sapmle text using active font/color settings
//      case WM_SIZE
//      - call default AVIO window procedure
//      case OTHER:
//      - call control's original window procedure
//
// Arguments:
//  mp2 of WM_INITDLG msg = PFONTCOLDATA pFontColData ptr to data structure
//                                       containing current font/color
//                                       assignments
//
// Returns:
//  MRESULT(FALSE)
//
// Prereqs:
//   None
//
// SideEffects:
//   - ptr to IDA is stored in dialog word QWL_USER
//   - ptr to table of available fonts is stored in dialog IDA
//   - last used text element is selected thus triggering a WM_CONTROL message
//
// External references:
//   UtlAlloc
//
//////////////////////////////////////////////////////////////////////////////
MRESULT APIENTRY EQFBFONTCOLSAMPLESUBPROC
(
   HWND hwnd,
   WINMSG msg,
   WPARAM mp1,
   LPARAM mp2
)
{

   HDC      hdc;                       // device context
   HFONT    hFont;                     // handle of created font
   LOGFONT  lf;                        // logical font structure
   RECT     rcl;                       // rectangle to be painted
   COLORREF clrBkRGB;
   COLORREF clrFGRGB;
   PAINTSTRUCT ps;                        // pointer to paint struct
   PFONTCOLIDA pIda;

   switch ( msg )
   {
      case WM_PAINT:
         pIda = ACCESSCTRLIDA(hwnd, PFONTCOLIDA);
         hdc = BeginPaint(hwnd, &ps );

         if (pIda->pTextType)
         {
           if ( !pIda->pTextType->fReverse )
           {
             clrFGRGB = COLORRGBTABLE[ pIda->pTextType->sFGColor];
             clrBkRGB = COLORRGBTABLE[ pIda->pTextType->sBGColor];
           }
           else
           {
             clrFGRGB = COLORRGBTABLE[ pIda->pTextType->sBGColor];
             clrBkRGB = COLORRGBTABLE[ pIda->pTextType->sFGColor];
           } /* endif */

           memset(&lf, 0, sizeof(lf));
           hdc = GetDC( hwnd );
           SelectObject( hdc, GetStockObject( ANSI_FIXED_FONT ));

           lf.lfPitchAndFamily = FIXED_PITCH | FF_DONTCARE;
           GetTextFace( hdc, sizeof( lf.lfFaceName ), lf.lfFaceName );

           /***********************************************************/
           /* get the default font size                               */
           /***********************************************************/
           {
             VIOFONTCELLSIZE vioDefCellSize;
             /*********************************************************/
             /* in DBCS case, GetDefCellsize returns FaceName of      */
             /* MS Mincho, is this correct???                         */
             /*********************************************************/
             EQFBGetDefCellSize( &vioDefCellSize, lf.lfFaceName );
             lf.lfHeight = LOWORD(vioDefCellSize.cy);
             lf.lfWidth  = LOWORD(vioDefCellSize.cx);
           }
           lf.lfQuality  = DEFAULT_QUALITY;

           lf.lfWeight = FW_NORMAL;
           lf.lfItalic = 0;
           lf.lfUnderline = (BYTE) pIda->pTextType->fUnderscore;
           lf.lfPitchAndFamily = FIXED_PITCH | FF_DONTCARE;
           lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
           lf.lfOutPrecision = OUT_STROKE_PRECIS;
           // if system pref.lang is DBCS, use SHIFTJIS_CHARSET
           if (IsDBCS_CP(GetLangOEMCP(NULL) ))
           {
             lf.lfCharSet = SHIFTJIS_CHARSET;
           }

           hFont = CreateFontIndirect( &lf );
           hFont = (HFONT)SelectObject( hdc, hFont );
             // limit sample text to size of client rectangle for painting...
           GetClientRect( hwnd, &rcl );
		   SetTextColor(hdc, clrFGRGB);
		   SetBkColor(hdc,clrBkRGB);

		   ExtTextOut( hdc, 0, 0, ETO_CLIPPED | ETO_OPAQUE, &rcl,
					   pIda->szSampleText, strlen(pIda->szSampleText ),
					   (LPINT) NULL );

           DeleteObject( SelectObject( hdc, hFont ) );
           ReleaseDC (hwnd, hdc);


         } /* endif */

         EndPaint(hwnd, &ps);
         return 0L;

      case WM_ERASEBKGND:
         return 0L;
   } /* endswitch */
   return ( CALLWINDOWPROC( OrgStaticProc, hwnd,msg,mp1,mp2) );

}

/*////////////////////////////////////////////////////////////////////////////
:h2.EQFBFontColControl - process WM_CONTROL messages of font/color dialog
*/
// Description:
//    Handle WM_CONTROL messages (= user selections in listboxes) of
//    font/color dialog panel.
//
//   Flow (message driven):
//      case LN_SELECT from text element listbox:
//         make selected text element the active one;
//         set selection of color and font listbox to values of text element;
//      case LN_SELECT from foreground color listbox:
//         set foreground color of active text element to selected value;
//         invalidate sample text control;
//      case LN_SELECT from background color listbox:
//         set background color of active text element to selected value;
//         invalidate sample text control;
//      case LN_SELECT from font listbox:
//         set font of active text element to selected value;
//         invalidate sample text control;
//      case BN_CLICKED from underscore checkbox:
//         set underscore flag of active text element to button check state;
//         invalidate sample text control;
//      case BN_CLICKED from reverse checkbox:
//         set reverse flag of active text element to button check state;
//         invalidate sample text control;
//
// Arguments:
//   SHORT1FROMMP(mp1) = ID of control sending the WM_CONTROL message
//   SHORT2FROMMP(mp1) = notification code
//
// Returns:
//  MRESULT(FALSE)
//
// Prereqs:
//   None
//
// SideEffects:
//   - values in text element table are updated
//   - sample text is repainted
//
// External references:
//   None
//
//////////////////////////////////////////////////////////////////////////////

MRESULT EQFBFontColControl
(
   HWND   hwndDlg,
   SHORT  sLB,                         // listbox id
   SHORT  sAction                      // requested action
)
{
   MRESULT       mResult = FALSE;
   PFONTCOLIDA   pIda;                 // ptr to dialog IDA
   SHORT         sItem;                // index of listbox items
   HWND          hwndLB;               // handle of listbox controls
   USHORT       usColor;               // color value

   //--- access dialog IDA ---
   pIda = ACCESSDLGIDA(hwndDlg, PFONTCOLIDA);

   switch ( sLB )
   {
      case ID_TB_FONTCOL_ITEM_LB:
         if ( sAction == LN_SELECT )
         {
            hwndLB = GETHANDLEFROMID( hwndDlg, ID_TB_FONTCOL_ITEM_LB );
            sItem = QUERYSELECTION( hwndDlg, ID_TB_FONTCOL_ITEM_LB );
            if ( sItem != LIT_NONE )
            {
               pIda->pTextType = pIda->pTextTable +
                                 GETITEMHANDLE( hwndDlg, ID_TB_FONTCOL_ITEM_LB,
                                                sItem, USHORT );

               //--- select foreground color ---
               usColor = pIda->pTextType->sFGColor;
               sItem = SEARCHITEM( hwndDlg, ID_TB_FONTCOL_FOREGROUND_LB,
                                     aszColorNames[usColor] );
               if ( sItem != LIT_NONE )
               {
                  SELECTITEM( hwndDlg, ID_TB_FONTCOL_FOREGROUND_LB, sItem );
               } /* endif */

               //--- select background color ---
               usColor = pIda->pTextType->sBGColor;
               sItem = SEARCHITEM( hwndDlg, ID_TB_FONTCOL_BACKGROUND_LB,
                                     aszColorNames[usColor] );
               if ( sItem != LIT_NONE )
               {
                  SELECTITEM( hwndDlg, ID_TB_FONTCOL_BACKGROUND_LB, sItem );
               } /* endif */


               SETCHECK( hwndDlg, ID_TB_FONTCOL_UNDSCORE_CHK,
                         pIda->pTextType->fUnderscore );
               SETCHECK( hwndDlg, ID_TB_FONTCOL_REVERSE_CHK,
                         pIda->pTextType->fReverse );

               INVALIDATERECT( pIda->hwndSample, NULL, TRUE );
            } /* endif */
         } /* endif */
         break;

      case ID_TB_FONTCOL_FOREGROUND_LB:
         if ( sAction == LN_SELECT )
         {
           sItem = QUERYSELECTION( hwndDlg, ID_TB_FONTCOL_FOREGROUND_LB );
           if ( sItem != LIT_NONE )
           {
             pIda->pTextType->sFGColor =
               GETITEMHANDLE(hwndDlg,ID_TB_FONTCOL_FOREGROUND_LB,sItem,USHORT);
             INVALIDATERECT( pIda->hwndSample, NULL, TRUE );
           } /* endif */
         } /* endif */
         break;

      case ID_TB_FONTCOL_BACKGROUND_LB:
         if ( sAction == LN_SELECT )
         {
           sItem = QUERYSELECTION( hwndDlg, ID_TB_FONTCOL_BACKGROUND_LB );
           if ( sItem != LIT_NONE )
           {
             pIda->pTextType->sBGColor =
               GETITEMHANDLE(hwndDlg,ID_TB_FONTCOL_BACKGROUND_LB,sItem,USHORT);
             INVALIDATERECT( pIda->hwndSample, NULL, TRUE );
           } /* endif */
         } /* endif */
         break;


      case ID_TB_FONTCOL_UNDSCORE_CHK:
         if ( sAction == BN_CLICKED )
         {
            pIda->pTextType->fUnderscore =
                QUERYCHECK( hwndDlg, ID_TB_FONTCOL_UNDSCORE_CHK );
            INVALIDATERECT( pIda->hwndSample, NULL, TRUE );
         } /* endif */
         break;

      case ID_TB_FONTCOL_REVERSE_CHK:
         if ( sAction == BN_CLICKED )
         {
            pIda->pTextType->fReverse =
               QUERYCHECK( hwndDlg, ID_TB_FONTCOL_REVERSE_CHK );
            INVALIDATERECT( pIda->hwndSample, NULL, TRUE );
         } /* endif */
         break;

   } /* endswitch */
   return( mResult );
} /* end of EQFBFontColControl */

/*////////////////////////////////////////////////////////////////////////////
:h2.EQFBFontColCommand - process WM_COMMAND messages of font/color dialog
*/
// Description:
//    Handle WM_COMMAND messages (= pressing of pushbuttons) of
//    font/color dialog panel.
//
//   Flow (message driven):
//      case CHANGE pushbutton:
//         update callers FONTCOLDATA structure with actual values;
//         post a WM_CLOSE message to dialog, mp1 = TRUE;
//      case DEFAULT pushbutton:
//         reset actual font/color data to default values
//         post a WM_CONTROL( text element listbox, LN_SELECT ) to force
//            a refresh;
//      case CANCEL pushbutton or DID_CANCEL (= ESCAPE key):
//         post a WM_CLOSE message to dialog, mp1 = FALSE;
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
//   - callers FONTCOLDATA structure is updated in case of CHANGE
//
// External references:
//   None
//
//////////////////////////////////////////////////////////////////////////////
MRESULT EQFBFontColCommand
(
   HWND hwndDlg,
   WPARAM  mp1,
   LPARAM  mp2
)
{
   MRESULT mResult = MRFROMSHORT(TRUE);// TRUE = command is processed
   PFONTCOLIDA   pIda;                 // ptr to dialog IDA
   PTBDOCUMENT pDoc;                      // ptr to doc struct in ida
   PSTEQFGEN   pstEQFGen;                 // pointer to generic struct
   SHORT      sLBId = WMCOMMANDID( mp1, mp2 );
   SHORT      sNotification = WMCOMMANDCMD( mp1, mp2 );

   mp2;
   //--- access dialog IDA ---
   pIda = ACCESSDLGIDA(hwndDlg, PFONTCOLIDA);

   switch ( sLBId )
   {
	  case ID_TB_FONTCOL_HELP_PB:
	    mResult = UtlInvokeHelp();
	    break;
      case ID_TB_FONTCOL_CHANGE_PB:
         memcpy( pIda->pFontColData->pTextTypeTable, pIda->pTextTable,
                 sizeof(TEXTTYPETABLE) * MAXCOLOUR );
         // force a reset of service windows, too.
         pDoc = pIda->pDoc;
         pstEQFGen = (PSTEQFGEN) pDoc->pstEQFGen;
         if ( pstEQFGen)
         {
           WinPostMsg( pstEQFGen->hwndTWBS,
                     WM_EQF_COLCHANGED, NULL, NULL);
         } /* endif */
         POSTEQFCLOSE( hwndDlg, FALSE );
         break;

      case ID_TB_FONTCOL_DEFAULT_PB:
         memcpy( pIda->pTextTable, get_DefTextTypeTable(),
                 sizeof(TEXTTYPETABLE) * MAXCOLOUR );
         /*************************************************************/
         /* deselect it first (i.e. force update)                     */
         /*************************************************************/
         DESELECTITEM( hwndDlg, ID_TB_FONTCOL_ITEM_LB, 0 );
         SELECTITEM( hwndDlg, ID_TB_FONTCOL_ITEM_LB, 0 );
         break;

      case ID_TB_FONTCOL_CANCEL_PB:
      case DID_CANCEL:
         POSTEQFCLOSE( hwndDlg, FALSE );
         break;

        case ID_TB_FONTCOL_ITEM_LB:
        case ID_TB_FONTCOL_FOREGROUND_LB:
        case ID_TB_FONTCOL_BACKGROUND_LB:
        case ID_TB_FONTCOL_UNDSCORE_CHK:
        case ID_TB_FONTCOL_REVERSE_CHK:
          mResult = EQFBFontColControl( hwndDlg, sLBId, sNotification );
          break;

      default:
         mResult = WinDefDlgProc( hwndDlg, WM_COMMAND, mp1, mp2 );
         break;
   } /* endswitch */

   return( mResult );
} /* end of EQFBFontColCommand */

/*////////////////////////////////////////////////////////////////////////////
:h2.EQFBFontColClose - process WM_CLOSE messages of font/color dialog
*/
// Description:
//    Handle WM_CLOSE messages (= dialog termination requests) of
//    font/color dialog panel.
//
//   Flow:
//      free allocated storage
//      dismiss dialog
//
// Arguments:
//   SHORT1FROMMP(mp1) = flag to be returned using WinDismissDlg
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
MRESULT EQFBFontColClose
(
   HWND hwndDlg,
   WPARAM mp1,
   LPARAM mp2
)
{
   MRESULT mResult = FALSE;
   PFONTCOLIDA   pIda;                 // ptr to dialog IDA

   mp1 = mp1;                          // supress 'unreferenced parameter' msg
   mp2 = mp2;                          // supress 'unreferenced parameter' msg

   //--- access dialog IDA ---
   pIda = ACCESSDLGIDA(hwndDlg, PFONTCOLIDA);

   //--- free allocated memory ---
   if ( pIda )
   {
      if ( pIda->pTextTable )
      {
         UtlAlloc((PVOID *) &pIda->pTextTable, 0L, 0L, NOMSG );
      } /* endif */
   } /* endif */
   //--- get rid off dialog ---
   DISMISSDLG( hwndDlg, TRUE );

   return( mResult );
} /* end of EQFBFontColClose */



/*////////////////////////////////////////////////////////////////////////////
:h2.EQFBFontSizeDlgProc - dialog procedure for fontsize dialog
*/
// Description:
//    Allows to select the value of the font size
//
//   Flow (message driven):
//       case WM_INITDLG:
//         fill listbox and init selected font size
//       case WM_COMMAND
//         call EQFBFontSizeCommand to handle user commands;
//       case WM_CLOSE
//         end the dialog
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
INT_PTR CALLBACK EQFBFONTSIZEDLGPROC
(
   HWND hwndDlg,                       /* handle of dialog window             */
   WINMSG msg,
   WPARAM mp1,
   LPARAM mp2
)
{
   MRESULT  mResult = MRFROMSHORT( FALSE );      // result value of procedure
   PFONTSIZEDATA pFontSizeData;                  // pointer to font size data
   HWND         hwnd;                            // window handle of entry field
   USHORT       i;                               // index
   SHORT        sItem;                           // item of window
   HAB          hab;                                       // anchor block handle
   HDC          hdc;

   switch ( msg )
   {
      case WM_EQF_QUERYID:
          HANDLEQUERYID( ID_TB_FONTSIZE_DLG, mp2 );
         break;

     case WM_INITDLG:
       {
         HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);

          // remember adress of user area
          SETWINDOWID( hwndDlg, ID_TB_FONTSIZE_DLG );

          pFontSizeData = (PFONTSIZEDATA) mp2;
          ANCHORDLGIDA( hwndDlg, pFontSizeData );

          SetCtrlFnt (hwndDlg, pFontSizeData->pDoc->lf.lfCharSet,
                      ID_TB_FONTSIZE_FONTS_LB, ID_TB_FONTSIZE_FONTS_LB );

          // load sample text string from resource
          hab = GETINSTANCE( hwndDlg );

          LOADSTRING( hab, hResMod, IDS_SAMPLETEXT, pFontSizeData->szSampleText);

          memset(pFontSizeData->szDummyText, BLANK,
                 strlen(pFontSizeData->szSampleText));

          hwnd = GETHANDLEFROMID( hwndDlg, ID_TB_FONTSIZE_WINDOW_LB );
          // fill window names into listbox with values
          for ( i=0; i<MAX_DIF_DOC; i++ )
          {
             // load names for text types
             aszWindowNames[i][0] = EOS;
             LOADSTRING( hab, hResMod, IDS_TB_FONT_OTHER+i, aszWindowNames[i]);
             if ( aszWindowNames[i][0] )
             {
               switch ( IDS_TB_FONT_OTHER+i )
               {
                 case IDS_TB_FONT_VISSRC:
                 case IDS_TB_FONT_VISTGT:
                   /***************************************************/
                   /* in case of Visual ITM do the following...       */
                   /***************************************************/
                   if ((pFontSizeData->pDoc->docType == VISSRC_DOC)
                       || (pFontSizeData->pDoc->docType == VISTGT_DOC) )
                   {
                      sItem = INSERTITEM( hwndDlg, ID_TB_FONTSIZE_WINDOW_LB,
                                          aszWindowNames[i] );
                      if ( sItem != LIT_NONE )
                      {
                        SETITEMHANDLE( hwndDlg,ID_TB_FONTSIZE_WINDOW_LB,
                                        sItem,(LONG)i);
                      } /* endif */
                   } /* endif */
                   break;
                 default:
//                   case IDS_TB_FONT_OTHER:
//                   case IDS_TB_FONT_SSOURCE:
//                   case IDS_TB_FONT_STARGET:
//                   case IDS_TB_FONT_SERVPROP:
//                   case IDS_TB_FONT_SERVDICT:
//                   case IDS_TB_FONT_SERVSOURCE:
                   if ((pFontSizeData->pDoc->docType == OTHER_DOC)
                       || (pFontSizeData->pDoc->docType == SSOURCE_DOC)
                       || (pFontSizeData->pDoc->docType == STARGET_DOC )
                       || (pFontSizeData->pDoc->docType == SERVPROP_DOC)
                       || (pFontSizeData->pDoc->docType == SERVDICT_DOC)
                       || (pFontSizeData->pDoc->docType == SERVSOURCE_DOC )
                       || (pFontSizeData->pDoc->docType == TRNOTE_DOC ))
                   {
                      // For TMEdit environment only: only add entry for
                      // Original window
                      if ( (pFontSizeData->pDoc->pTMMaint == NULL) ||
                           ((IDS_TB_FONT_OTHER+i) == IDS_TB_FONT_SSOURCE) ||
                           ((IDS_TB_FONT_OTHER+i) == IDS_TB_FONT_STARGET) )
                      {
                        sItem = INSERTITEM( hwndDlg, ID_TB_FONTSIZE_WINDOW_LB,
                                            aszWindowNames[i] );
                        if ( sItem != LIT_NONE )
                        {
                          SETITEMHANDLE( hwndDlg,ID_TB_FONTSIZE_WINDOW_LB,
                                           sItem,(LONG)i);
                        } /* endif */
                      } /* endif */
                   } /* endif */
                   break;
               } /* endswitch */
             } /* endif */
          } /* endfor */

          hdc = GetDC( pFontSizeData->hwnd ) ;
          lGLogPixy = GetDeviceCaps(hdc, LOGPIXELSY);
          lGLogPixx = GetDeviceCaps(hdc, LOGPIXELSX);
          ReleaseDC (pFontSizeData->hwnd, hdc);

          // fill the available fonts in the fonts listbox
          GetFonts ( hwndDlg, pFontSizeData );             // get fonts

          //
          // subclass sample text control and create associated AVIO PS
          //
          pFontSizeData->hwndSample = GETHANDLEFROMID( hwndDlg,
                                                     ID_TB_FONTSIZE_SAMPLE_GB);
          ANCHORCTRLIDA( pFontSizeData->hwndSample, pFontSizeData );

          OrgStaticProc = SUBCLASSWND( pFontSizeData->hwndSample,
                                       EQFBFONTSIZESAMPLESUBPROC );

          // select the first window lb item and the appropriate font size
          // and trigger the WM_CONTROL
          SELECTITEM( hwndDlg, ID_TB_FONTSIZE_WINDOW_LB, 0 );

          mResult = DIALOGINITRETURN( mResult );
        }
        break;

      case WM_COMMAND:
         mResult = EQFBFontSizeCommand( hwndDlg, mp1, mp2 );
         break;

      case WM_EQF_CLOSE:
         DelCtrlFont ( hwndDlg, ID_TB_FONTSIZE_FONTS_LB );
         //--- get rid off dialog ---
         DISMISSDLG( hwndDlg, TRUE );
         break;

      default:
         mResult = WinDefDlgProc( hwndDlg, msg, mp1, mp2 );
         break;
   } /* endswitch */
   return mResult;
} /* end of EQFBFontSizeDlgProc */

/*////////////////////////////////////////////////////////////////////////////
:h2.EQFBFontSizeCommand - process WM_COMMAND messages of fontsize dialog
*/
// Description:
//    Handle WM_COMMAND messages (= pressing of pushbuttons) of
//    margins dialog panel.
//
//   Flow (message driven):
//      case 'Set' pushbutton:
//         get the selected values
//         set global and pdoc font sizes and write profile
//
//      case 'Default' pushbutton:
//         copy default value into font size structure
//         DO NOT SET these values in the profile !
//
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
MRESULT EQFBFontSizeCommand
(
   HWND hwndDlg,
   WPARAM mp1,
   LPARAM mp2
)
{
   MRESULT mResult = MRFROMSHORT(TRUE);     // TRUE = command is processed
   PVIOFONTCELLSIZE pvioCellSize;           // pointer to cell size struct
   PVIOFONTCELLSIZE pGlobalVioFontSize;
   PFONTSIZEDATA    pFontSizeData;          // pointer to font size data
   VIOFONTCELLSIZE  vioDefCellSize;         // default cell size
   USHORT  i;                               // index
   CHAR        chFaceName[LF_FACESIZE];          // font name
   CHAR* paszFontFacesGlobal;


   mp2 = mp2;                               // get rid off compiler warning

   switch ( WMCOMMANDID( mp1, mp2 ) )
   {
	  case ID_TB_FONTSIZE_HELP_PB:
	    mResult = UtlInvokeHelp();
	      break;
      case ID_TB_FONTSIZE_SET_PB:          // get new value and set it
         // set fontsize and write profile, close dlg
         pFontSizeData = ACCESSDLGIDA(hwndDlg, PFONTSIZEDATA);
         pvioCellSize = pFontSizeData->vioFontCell;
		     pGlobalVioFontSize = get_vioFontSize();
         memcpy(pGlobalVioFontSize, pvioCellSize, sizeof(VIOFONTCELLSIZE) * MAX_DIF_DOC);

		     paszFontFacesGlobal = get_aszFontFacesGlobal();
         for ( i=0; i<MAX_DIF_DOC; i++ )
         {
           strcpy(paszFontFacesGlobal, pFontSizeData->chFontFacename[i] );
		       paszFontFacesGlobal += LF_FACESIZE;
         } /* endfor */

         // write profile
         EQFBWriteProfile(pFontSizeData->pDoc);
         // issue a reset of all open documents
         EQFBResetSize( pFontSizeData->pDoc );
         // close dialog
         POSTEQFCLOSE( hwndDlg, FALSE );
         break;

      case ID_TB_FONTSIZE_DEFAULT_PB:     // copy default value into struct
         pFontSizeData = ACCESSDLGIDA(hwndDlg, PFONTSIZEDATA);
         pvioCellSize = pFontSizeData->vioFontCell;
         EQFBGetDefCellSize( &vioDefCellSize, chFaceName );
         for ( i=0; i<MAX_DIF_DOC; i++ )
         {
           /***********************************************************/
           /* fill only defaults of those documenttypes which are also*/
           /* displayed in windows lb                                 */
           /***********************************************************/
            switch ( IDS_TB_FONT_OTHER+i )
            {
              case IDS_TB_FONT_VISSRC:
              case IDS_TB_FONT_VISTGT:
                /***************************************************/
                /* in case of Visual ITM do the following...       */
                /***************************************************/
                if ((pFontSizeData->pDoc->docType == VISSRC_DOC)
                    || (pFontSizeData->pDoc->docType == VISTGT_DOC) )
                {
                   *pvioCellSize = vioDefCellSize;

                  strcpy( pFontSizeData->chFontFacename[i],
                           chFaceName );
                } /* endif */
                break;
              default:
//                case IDS_TB_FONT_OTHER:
//                case IDS_TB_FONT_SSOURCE:
//                case IDS_TB_FONT_STARGET:
//                case IDS_TB_FONT_SERVPROP:
//                case IDS_TB_FONT_SERVDICT:
//                case IDS_TB_FONT_SERVSOURCE:
                if ((pFontSizeData->pDoc->docType == OTHER_DOC)
                    || (pFontSizeData->pDoc->docType == SSOURCE_DOC)
                    || (pFontSizeData->pDoc->docType == STARGET_DOC )
                    || (pFontSizeData->pDoc->docType == SERVPROP_DOC)
                    || (pFontSizeData->pDoc->docType == SERVDICT_DOC)
                    || (pFontSizeData->pDoc->docType == SERVSOURCE_DOC )
                    || (pFontSizeData->pDoc->docType == TRNOTE_DOC ))
                {
                   *pvioCellSize = vioDefCellSize;

                   strcpy( pFontSizeData->chFontFacename[i],
                           chFaceName );
                } /* endif */
                break;
            } /* endswitch */
            pvioCellSize++;
         } /* endfor */
         /*************************************************************/
         /* select the first window                                   */
         /*************************************************************/
         DESELECTITEM( hwndDlg, ID_TB_FONTSIZE_WINDOW_LB, 0 );
         SELECTITEM( hwndDlg, ID_TB_FONTSIZE_WINDOW_LB, 0 );
         INVALIDATERECT( pFontSizeData->hwndSample, NULL, TRUE );
         break;

      case ID_TB_FONTSIZE_CANCEL_PB:
      case DID_CANCEL:
         POSTEQFCLOSE( hwndDlg, FALSE );
         break;

        case ID_TB_FONTSIZE_SIZES_LB:
        case ID_TB_FONTSIZE_WINDOW_LB:
        case ID_TB_FONTSIZE_FONTS_LB:
           mResult = EQFBFontSizeControl( hwndDlg, WMCOMMANDID( mp1, mp2  ),
                                        WMCOMMANDCMD( mp1, mp2 ));
           break;
       default:
         mResult = WinDefDlgProc( hwndDlg, WM_COMMAND, mp1, mp2 );
         break;
   } /* endswitch */

   return( mResult );
} /* end of EQFBFontSizeCommand */


//+----------------------------------------------------------------------------+
// Internal function
//+----------------------------------------------------------------------------+
// Function name:     EQFBGetCharWidth
//+----------------------------------------------------------------------------+
// Function call:     EQFBGetCharWidth
//+----------------------------------------------------------------------------+
// Description:       get character width from vioFontCellSIze
//+----------------------------------------------------------------------------+
// Parameters:        PVIOFONTCELLSIZE pvioFOntCell
//                    PSZ              pFaceName
//+----------------------------------------------------------------------------+
// Returncode type:   WORD
//+----------------------------------------------------------------------------+
// Returncodes:       WORD wTextExtent    // width of one character
//+----------------------------------------------------------------------------+
// Function flow:     get device context
//                    fill logfont structure
//                    create font
//                    get text length of "ab"
//                    release device context
//                    return length of one character
//+----------------------------------------------------------------------------+
static WORD
EQFBGetCharWidth
(
   PVIOFONTCELLSIZE   pvioCellSize,           // pointer to cell size struct
   PSZ                pszFaceName
)
{
   LOGFONT   lf;
   HFONT     hFont;
   WORD      wTextWidth;
   LONG lCX, lCY;
   TEXTMETRIC FontMetrics;

   HDC  hDC = GetDC( HWND_DESKTOP );                  //get device context


   memset (&lf, 0, sizeof(lf));                       //fill lf struct
   lf.lfCharSet = (BYTE)GetCharSet();
   lf.lfPitchAndFamily=  FIXED_PITCH | FF_DONTCARE;
   lf.lfOutPrecision = OUT_STROKE_PRECIS;
   lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
   lf.lfQuality = DEFAULT_QUALITY;

   strcpy (lf.lfFaceName, pszFaceName);               // fill specific
   lf.lfHeight = (SHORT) pvioCellSize->cy;            // lf values
   lf.lfWidth = (SHORT) pvioCellSize->cx;
   hFont = CreateFontIndirect(&lf);                   // create font
   hFont = (HFONT)SelectObject(hDC, hFont );                 // select an object
   GetTextMetrics( hDC, &FontMetrics );

   if ((FontMetrics.tmHeight ==0) || (FontMetrics.tmAveCharWidth==0))
   {
      DeleteObject( SelectObject( hDC, hFont) );
      lf.lfCharSet = (BYTE)GetCharSet()& ((IsDBCS_CP(GetLangOEMCP(NULL)) ) ? ~SHIFTJIS_CHARSET : 0xff);
      lf.lfHeight = (SHORT) pvioCellSize->cy;            // lf values
      lf.lfWidth = (SHORT) pvioCellSize->cx;

      lf.lfQuality  = DEFAULT_QUALITY;
      lf.lfPitchAndFamily = FIXED_PITCH | FF_DONTCARE;
      lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
      lf.lfOutPrecision = OUT_STROKE_PRECIS;

      hFont = CreateFontIndirect(&lf );
      hFont = (HFONT)SelectObject(hDC, hFont );
      GetTextMetrics( hDC, &FontMetrics );
   }

   TEXTSIZE( hDC, "ab", lCX, lCY );

   wTextWidth = (WORD)lCX;
   wTextWidth = wTextWidth / 2;

   DeleteObject( SelectObject( hDC, hFont ) );

   ReleaseDC( HWND_DESKTOP, hDC );

   return( wTextWidth );
} /* end of EQFBGetCharWidth    */


/*////////////////////////////////////////////////////////////////////////////
:h2.EQFBFontSizeControl - process WM_CONTROL messages of fontsize dialog
*/
// Description:
//    Handle WM_CONTROL messages (= user selections in listboxes) of
//    font size dialog panel.
//
//   Flow (message driven):
//      case LN_SELECT from window listbox:
//         make selected text element the active one;
//         find the correct font size and activate the correct value
//      case LN_SELECT from fontsize listbox:
//         get selected value and its handle
//         get selected listbox window
//         set selected value as new font size
//     case LN_SELECT from fonts listbox
//         get selected value and fill fontsize listbox accordingly
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
//
// External references:
//   None
//
//////////////////////////////////////////////////////////////////////////////

MRESULT EQFBFontSizeControl
(
   HWND hwndDlg,
   SHORT sId,
   SHORT sNotification
)
{
   MRESULT       mResult = FALSE;
   SHORT         sItem;                // index of listbox items
   HWND          hwndLB;               // handle of listbox controls
   SHORT         sHandle = 0;          // handle associated with item
   SHORT         sFontSizeHandle;      // handle associated with item
   SHORT         sFontHandle;          // handle associated with item
   PVIOFONTCELLSIZE pvioCellSize;      // pointer to cell size struct
   PVIOFONTCELLSIZE pvioSizeAva;       // pointer to available cell sizes
   PVIOSIZECOUNT pvioSizeCount;        // pointer to sizecount struct
   PFONTSIZEDATA    pFontSizeData;     // pointer to font size data
   CHAR          chText[20];           // size of string


   //--- access dialog IDA ---
   pFontSizeData = ACCESSDLGIDA(hwndDlg, PFONTSIZEDATA);
   pvioCellSize = pFontSizeData->vioFontCell;

   switch ( sId )
   {
      case ID_TB_FONTSIZE_WINDOW_LB:
         if ( sNotification == LN_SELECT )
         {
           sItem = QUERYSELECTION( hwndDlg, ID_TB_FONTSIZE_WINDOW_LB );
           if ( sItem != LIT_NONE )
           {
             sHandle =
               GETITEMHANDLE( hwndDlg, ID_TB_FONTSIZE_WINDOW_LB, sItem, SHORT);

             sItem = SEARCHITEM( hwndDlg, ID_TB_FONTSIZE_FONTS_LB,
                                pFontSizeData->chFontFacename[sHandle] );
             if (sItem != LIT_NONE )
             {
               /*********************************************************/
               /* deselect item first to force update                   */
               /*********************************************************/
               DESELECTITEM( hwndDlg, ID_TB_FONTSIZE_FONTS_LB, 0 );
               SELECTITEM( hwndDlg, ID_TB_FONTSIZE_FONTS_LB, sItem );
             }
             else
             {
               SELECTITEM( hwndDlg, ID_TB_FONTSIZE_FONTS_LB, 0 );
             } /* endif */

             INVALIDATERECT( pFontSizeData->hwndSample, NULL, TRUE );
           } /* endif */
         } /* endif */
         break;
      case ID_TB_FONTSIZE_SIZES_LB:
         if ( sNotification == LN_SELECT )
         {
           hwndLB = GETHANDLEFROMID( hwndDlg, ID_TB_FONTSIZE_SIZES_LB );
           sItem = QUERYSELECTION( hwndDlg, ID_TB_FONTSIZE_SIZES_LB );
           if ( sItem != LIT_NONE )
           {
             // get the handle which is the index into the available
             // sizes array
             sFontSizeHandle =
               GETITEMHANDLE( hwndDlg, ID_TB_FONTSIZE_SIZES_LB, sItem, SHORT);

             sItem = QUERYSELECTION( hwndDlg, ID_TB_FONTSIZE_WINDOW_LB );
             if ( sItem != LIT_NONE )
             {
               // get the handle which is the index into the available
               // windows to be set
               sHandle =
                 GETITEMHANDLE( hwndDlg, ID_TB_FONTSIZE_WINDOW_LB,sItem,SHORT);

                pvioSizeCount = (PVIOSIZECOUNT) (pFontSizeData->lOutData);
                pvioSizeAva  = (PVIOFONTCELLSIZE) (pvioSizeCount+1);
                pvioSizeAva  += sFontSizeHandle;
                pvioCellSize += sHandle;

                pvioCellSize->cx = pvioSizeAva->cx;
                pvioCellSize->cy = pvioSizeAva->cy;
             } /* endif */
             INVALIDATERECT( pFontSizeData->hwndSample, NULL, TRUE );
           } /* endif */
         } /* endif */
         break;

      case ID_TB_FONTSIZE_FONTS_LB:
         if ( sNotification == LN_SELECT )
         {
           sItem = QUERYSELECTION( hwndDlg, ID_TB_FONTSIZE_FONTS_LB );
           if ( sItem == LIT_NONE )
           {
             sItem = 0;                // use the first one ...
           } /* endif */

           sFontHandle =
             GETITEMHANDLE( hwndDlg, ID_TB_FONTSIZE_FONTS_LB, sItem, SHORT);

           sItem = QUERYSELECTION( hwndDlg, ID_TB_FONTSIZE_WINDOW_LB );
           if ( sItem != LIT_NONE )
           {
             sHandle =
               GETITEMHANDLE( hwndDlg, ID_TB_FONTSIZE_WINDOW_LB,sItem,SHORT);
             strcpy(pFontSizeData->chFontFacename[sHandle],
                    pFontSizeData->efAllFaces.szFaceName[sFontHandle] );
           } /* endif */

           FillSizeLB(hwndDlg, pFontSizeData, sFontHandle);

           //--- select correct size in the size list box ---
           pvioCellSize += sHandle;

           sprintf( chText, "%3ldx%3ld",
                    (LONG) EQFBGetCharWidth(pvioCellSize,
                                    pFontSizeData->chFontFacename[sHandle]),
                                    pvioCellSize->cy );

           sItem = SEARCHITEM( hwndDlg, ID_TB_FONTSIZE_SIZES_LB, chText);
           if ( sItem != LIT_NONE )
           {
             SELECTITEM( hwndDlg, ID_TB_FONTSIZE_SIZES_LB, sItem );
           }
           else
           {
             SELECTITEM( hwndDlg, ID_TB_FONTSIZE_SIZES_LB, 0 );
           } /* endif */
           INVALIDATERECT( pFontSizeData->hwndSample, NULL, TRUE );
         } /* endif */

         break;
   } /* endswitch */

   return( mResult );
} /* end of EQFBFontSizeControl */


//+----------------------------------------------------------------------------+
// Internal function
//+----------------------------------------------------------------------------+
// Function name:     FillSizeLB
//+----------------------------------------------------------------------------+
// Function call:     FillSizeLB
//+----------------------------------------------------------------------------+
// Description:       Fill Size listbox with corrsponding sizes
//+----------------------------------------------------------------------------+
// Parameters:        PVIOFONTCELLSIZE pElem1
//
//+----------------------------------------------------------------------------+
// Returncode type:   INT
//+----------------------------------------------------------------------------+
// Returncodes:       _
//+----------------------------------------------------------------------------+
// Function flow:
//+----------------------------------------------------------------------------+
static VOID
FillSizeLB
(
   HWND          hwndDlg,                 // dialog handle
   PFONTSIZEDATA pFontSizeData,           // pointer to ida
   USHORT        usCurFace                // number of font to be selected
)
{
   HDC               hdc;
   PVIOSIZECOUNT     pvioSizeCount;        // pointer to sizecount struct
   PVIOFONTCELLSIZE  pvioCellSize;         // pointer to cell size struct
   FONTENUMPROC      lpEnumSizeCallBack;
   SHORT         sCount;                  // number of different values
   SHORT         sItem;                   // item index
   SHORT         i, j;                    // index in for loop
   CHAR          chText[20];                     // size of string
   LONG          lLogPixx = 0L;
   LONG          lLogPixy = 0L;
   LOGFONT       lf;
   HFONT         hFont;
   TEXTMETRIC    TM;
   SHORT         cx, cy;

   WORD          wTextWidth;

   VIOFONTCELLSIZE vioDefCellSize;               //default CellSize
   CHAR          chDefFaceName[LF_FACESIZE];     // default FOntfacename
/**********************************************************************/
/* delete all old entries in sizes listbox                            */
/**********************************************************************/
   ENABLEUPDATE_FALSE( hwndDlg, ID_TB_FONTSIZE_SIZES_LB );
   DELETEALL( hwndDlg, ID_TB_FONTSIZE_SIZES_LB );

   hdc = GetDC( pFontSizeData->hwnd ) ;

   lpEnumSizeCallBack =
     (FONTENUMPROC) MakeProcInstance( EnumAllFontSizes, GETINSTANCE(hwndDlg));

   memset( &pFontSizeData->lOutData, 0, sizeof( pFontSizeData->lOutData ));
   pvioSizeCount = (PVIOSIZECOUNT) (pFontSizeData->lOutData);
   pvioSizeCount->maxcount =
      (sizeof(pFontSizeData->lOutData) / sizeof(VIOFONTCELLSIZE)) - 2;

   EnumFontFamilies( hdc,
                    pFontSizeData->efAllFaces.szFaceName[usCurFace],
                    lpEnumSizeCallBack,
                    (LPARAM) &(pFontSizeData->lOutData) );
   FreeProcInstance((FARPROC) lpEnumSizeCallBack );
   /*******************************************************************/
   /* fill resulting fonts into the listbox                           */
   /*******************************************************************/

   memset (&lf, 0, sizeof(lf));
//   lf.lfCharSet = (BYTE)GetCharSet();
   lf.lfCharSet = (BYTE)GetCharSet() & (( IsDBCS_CP(GetLangOEMCP(NULL)) ) ? ~SHIFTJIS_CHARSET : 0xff);
   lf.lfPitchAndFamily=  FIXED_PITCH | FF_DONTCARE;
   lf.lfOutPrecision = OUT_STROKE_PRECIS;
   lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
   lf.lfQuality = DEFAULT_QUALITY;
   strcpy (lf.lfFaceName, pFontSizeData->efAllFaces.szFaceName[usCurFace]);
   hFont = CreateFontIndirect(&lf);
   hFont = (HFONT)SelectObject(hdc, hFont );
   GetTextMetrics( hdc, &TM );
   if ((TM.tmHeight == 0) || (TM.tmAveCharWidth == 0))
   {
       lf.lfCharSet = (BYTE)GetCharSet() & ((IsDBCS_CP(GetLangOEMCP(NULL)) ) ? ~SHIFTJIS_CHARSET : 0xff);
       lf.lfPitchAndFamily=  FIXED_PITCH | FF_DONTCARE;
       lf.lfOutPrecision = OUT_STROKE_PRECIS;
       lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
       lf.lfQuality = DEFAULT_QUALITY;
   }
   DeleteObject( SelectObject( hdc, hFont ) );
   /*******************************************************************/
   /* get all sizes of truetype font                                  */
   /* for truetype, do not use the sizes get from the EnumAllFontSizes*/
   /*******************************************************************/
   if ( pFontSizeData->efAllFaces.bTrueType[usCurFace] )
   {
     pvioSizeCount = (PVIOSIZECOUNT) (pFontSizeData->lOutData);
     pvioSizeCount->count = 0;
     /*****************************************************************/
     /* truetype font is scalable                                     */
     /* for DBCS, it is nec that heigth = 2*width, hence force that   */
     /*****************************************************************/
     lLogPixy = GetDeviceCaps(hdc, LOGPIXELSY);
     lLogPixx = GetDeviceCaps(hdc, LOGPIXELSX);

     for ( i= 16 ; i < 50 ; i=i+2 )
     {
       lf.lfHeight = i;
       lf.lfWidth = i / 2;
       hFont = CreateFontIndirect(&lf);
       hFont = (HFONT)SelectObject(hdc, hFont );
       GetTextMetrics( hdc, &TM );


       /***************************************************************/
       /* allow only fonts where GetTextExtent of two characters is   */
       /* the same as twice the Width of a character:is NOW ok 15.1.95*/
       /* also the digitized Aspect must be equal to the result of    */
       /* GetDeviceCaps ( for VGA sLogPixx = sLogPixy = 96 )          */
       /* if not equal, font should not be used                       */
       /***************************************************************/

       DeleteObject( SelectObject( hdc, hFont ) );
       cx  = (SHORT)TM.tmAveCharWidth;
       cy  = (SHORT)TM.tmHeight;
       if ( (lLogPixx == TM.tmDigitizedAspectX) &&
            (lLogPixy == TM.tmDigitizedAspectY))
       {
         pvioCellSize  = (PVIOFONTCELLSIZE) (pvioSizeCount+1);
         j = 0;
         while ( j < pvioSizeCount->count )
         {
           if ((pvioCellSize->cx == cx) && (pvioCellSize->cy == cy)  )
           {
             /***************************************************************/
             /* we have it already as possible size in list...              */
             /***************************************************************/
             break;
           }
           else
           {
             j++;
             pvioCellSize++;
           } /* endif */
         } /* endwhile */

         if ( (j >= pvioSizeCount->count) &&
              (j < pvioSizeCount->maxcount ))
         {
           /***********************************************************/
           /* check that Heigth and width are unchanged if object is  */
           /* selected                                                */
           /***********************************************************/
           if (  (i == TM.tmHeight) && (i == (2*TM.tmAveCharWidth)) )
           {
             pvioCellSize->cy = (LONG)TM.tmHeight;
             pvioCellSize->cx = (LONG)TM.tmAveCharWidth;
             pvioSizeCount->count++;
           } /* endif */
         } /* endif */
       } /* endif */
     } /*endfor */
   } /* endif */

   /*******************************************************************/
   /* fill resulting fonts into the listbox                           */
   /*******************************************************************/
   pvioSizeCount = (PVIOSIZECOUNT) (pFontSizeData->lOutData);
   sCount = (SHORT) pvioSizeCount->count;
   pvioCellSize  = (PVIOFONTCELLSIZE) (pvioSizeCount+1);
   for ( i=0; i < sCount ; i++)
   {
     /*****************************************************************/
     /* if not truetype: check if character extent is really cx       */
     /* allow that now!! 15.1.95 for truetype and others              */
     /*****************************************************************/
     wTextWidth = EQFBGetCharWidth(pvioCellSize, lf.lfFaceName);

     sprintf( chText, "%3ldx%3ld", (LONG) wTextWidth, pvioCellSize->cy );

     sItem = INSERTITEM( hwndDlg, ID_TB_FONTSIZE_SIZES_LB, chText );
     if ( sItem != LIT_NONE )
     {
        SETITEMHANDLE( hwndDlg, ID_TB_FONTSIZE_SIZES_LB, sItem,
                       MP2FROMSHORT( i ) );
     } /* endif */
     pvioCellSize++;
   } /* endfor */
   /*******************************************************************/
   /* make sure that default size is in sizes lb if facename matches  */
   /* to the default facename                                         */
   /*******************************************************************/
   EQFBGetDefCellSize( &vioDefCellSize, chDefFaceName );
   if (!strcmp(chDefFaceName,lf.lfFaceName)  )
   {
     /*****************************************************************/
     /* use actual width of a character in size listbox               */
     /*****************************************************************/
     wTextWidth = EQFBGetCharWidth(&vioDefCellSize, chDefFaceName);

     sprintf( chText, "%3ldx%3ld", (LONG) wTextWidth, vioDefCellSize.cy );
     sItem = SEARCHITEM( hwndDlg, ID_TB_FONTSIZE_SIZES_LB, chText);
     if ( sItem == LIT_NONE )
     {
        sItem = INSERTITEM( hwndDlg, ID_TB_FONTSIZE_SIZES_LB, chText );
        if ( sItem != LIT_NONE )
        {
          i = (SHORT) pvioSizeCount->count;
          SETITEMHANDLE( hwndDlg, ID_TB_FONTSIZE_SIZES_LB, sItem,
                          MP2FROMSHORT( i ) );
        } /* endif */
        if ( (pvioSizeCount->count < pvioSizeCount->maxcount ))
        {
          pvioCellSize  = (PVIOFONTCELLSIZE) (pvioSizeCount+1);
          pvioCellSize += pvioSizeCount->count;
          pvioCellSize->cy = (LONG)vioDefCellSize.cy;
          pvioCellSize->cx = (LONG)vioDefCellSize.cx;
          pvioSizeCount->count++;
        } /* endif */
     } /* endif */
   } /* endif */


   ReleaseDC (pFontSizeData->hwnd, hdc);

   ENABLEUPDATE_TRUE( hwndDlg, ID_TB_FONTSIZE_SIZES_LB );

  return;
}


//+----------------------------------------------------------------------------+
// Internal function
//+----------------------------------------------------------------------------+
// Function name:     GetFonts
//+----------------------------------------------------------------------------+
// Function call:     _
//+----------------------------------------------------------------------------+
// Description:       fill fonts lb with all fixed-pitch font facenames
//                    enumerate the fonts
//+----------------------------------------------------------------------------+
// Parameters:        HWND          hwndDlg,                 // dialog handle
//                    PFONTSIZEDATA pFontSizeData            // pointer to ida
//+----------------------------------------------------------------------------+
// Returncode type:   VOID
//+----------------------------------------------------------------------------+
// Function flow:     _
//+----------------------------------------------------------------------------+

static
VOID GetFonts
(
   HWND          hwndDlg,                 // dialog handle
   PFONTSIZEDATA pFontSizeData            // pointer to ida
)
{
   FONTENUMPROC  lpEnumFontCallBack;
   HDC           hdc;
   SHORT         i;
   SHORT         sItem;

   /*******************************************************************/
   /* enumerate the fonts ....                                        */
   /*******************************************************************/
   hdc = GetDC( pFontSizeData->hwnd ) ;

//          strcpy (chFaceName, "‚l‚r ƒSƒVƒbƒN");
   lpEnumFontCallBack =
     (FONTENUMPROC) MakeProcInstance( EnumAllFonts, GETINSTANCE(hwndDlg));
    /****************************************************************/
    /* enumerate the font faces                                     */
    /****************************************************************/
    EnumFontFamilies( hdc,
                     NULL,
                     lpEnumFontCallBack,
                     (LPARAM) &(pFontSizeData->efAllFaces) );
    FreeProcInstance((FARPROC) lpEnumFontCallBack );

    ENABLEUPDATE_FALSE( hwndDlg, ID_TB_FONTSIZE_FONTS_LB );
    DELETEALL( hwndDlg, ID_TB_FONTSIZE_FONTS_LB );
    for (i=0 ;i < pFontSizeData->efAllFaces.nNumFaces ; i++ )
    {
       sItem = INSERTITEM( hwndDlg, ID_TB_FONTSIZE_FONTS_LB,
                           pFontSizeData->efAllFaces.szFaceName[i] );
       if ( sItem != LIT_NONE )
       {
         SETITEMHANDLE( hwndDlg,ID_TB_FONTSIZE_FONTS_LB,
                         sItem,(LONG)i);
       } /* endif */
    } /* endfor */
    ENABLEUPDATE_TRUE( hwndDlg, ID_TB_FONTSIZE_FONTS_LB );
   ReleaseDC(pFontSizeData->hwnd, hdc);
   return;
}



/**********************************************************************/
/* call-back function for enumerate fonts ...                         */
/**********************************************************************/
int CALLBACK EnumAllFontSizes
(
  LOGFONT       * pLF,                 // address of logical font structure
  NEWTEXTMETRIC * pNTM,                // address of physical font struct
  SHORT         sFontType,             // font type
  PBYTE         pData                  // address of application defined data
)
{
   SHORT cy = (SHORT)pNTM->tmHeight;
   SHORT cx = (SHORT)pNTM->tmAveCharWidth;

   PVIOSIZECOUNT pvioSizeCount;        // pointer to sizecount struct
   PVIOFONTCELLSIZE pvioCellSize;      // pointer to cell size struct
   SHORT i;

   sFontType;
   pLF;
/**********************************************************************/
/* for fixed-pitch AveCharWidth must be equal to MaxCharWidth         */
/* ( Petzold)                                                         */
/**********************************************************************/
   if ( (lGLogPixx == pNTM->tmDigitizedAspectX) &&
        (lGLogPixy == pNTM->tmDigitizedAspectY) )
   {
     pvioSizeCount = (PVIOSIZECOUNT) pData;
     pvioCellSize  = (PVIOFONTCELLSIZE) (pvioSizeCount+1);

     i = 0;
     while ( i < pvioSizeCount->count )
     {
       if ((pvioCellSize->cx == cx) && (pvioCellSize->cy == cy)  )
       {
         /***************************************************************/
         /* we have it already as possible size in list...              */
         /***************************************************************/
         break;
       }
       else
       {
         i++;
         pvioCellSize++;
       } /* endif */
     } /* endwhile */

     if ((i >= pvioSizeCount->count) && (i < pvioSizeCount->maxcount ))
     {
        pvioCellSize->cy = (LONG) cy;
        pvioCellSize->cx = (LONG) cx;
        pvioSizeCount->count ++;
     } /* endif */
   } /* endif */
  return 1;
}
/**********************************************************************/
/* call-back function for enumerate fonts ...                         */
/**********************************************************************/
int CALLBACK EnumAllFonts
(
  LOGFONT       * pLF,                 // address of logical font structure
  NEWTEXTMETRIC * pNTM,                // address of physical font struct
  SHORT           sFontType,           // font type
  ENUMFACE FAR *  lpef                 // ptr to enumface area
)
{
  BOOL    fOK = TRUE;

   /*******************************************************************/
   /* take font only if fixed pitch                                   */
   /*******************************************************************/
   if (sFontType && (pLF->lfPitchAndFamily & FIXED_PITCH) )
   {
     lstrcpy(lpef->szFaceName[lpef->nNumFaces], pLF->lfFaceName);
     /*****************************************************************/
     /* set TrueType indicator if Font is a truetype                  */
     /*****************************************************************/
     lpef->bTrueType[lpef->nNumFaces] =
                     (pNTM->tmPitchAndFamily & TMPF_TRUETYPE);
     if (lpef->nNumFaces == MAX_FACES -1)
     {
       fOK = FALSE;
     } /* endif */
     lpef->nNumFaces ++;
   } /* endif */

  return (fOK);
}


//+----------------------------------------------------------------------------+
// Internal function
//+----------------------------------------------------------------------------+
// Function name:     FontListSort
//+----------------------------------------------------------------------------+
// Function call:     FontListSort( pFontSize1, pFontSize2 );
//+----------------------------------------------------------------------------+
// Description:       Compare routine for two tokens (called by qsort)
//+----------------------------------------------------------------------------+
// Parameters:        PVIOFONTCELLSIZE pElem1
//                    PVIOFONTCELLSIZE pElem2
//+----------------------------------------------------------------------------+
// Returncode type:   INT
//+----------------------------------------------------------------------------+
// Returncodes:       _
//+----------------------------------------------------------------------------+
// Function flow:     compare two tokens
//+----------------------------------------------------------------------------+
int FontListSort ( const void * pElem1, const void * pElem2 )
{
  if (((PVIOFONTCELLSIZE)pElem1)->cx > ((PVIOFONTCELLSIZE) pElem2)->cx )
  {
    return 1;
  }
  else if (((PVIOFONTCELLSIZE)pElem1)->cx < ((PVIOFONTCELLSIZE) pElem2)->cx )
  {
    return -1;
  }
  else
  {
    return (((PVIOFONTCELLSIZE)pElem1)->cy > ((PVIOFONTCELLSIZE) pElem2)->cy );
  } /* endif */
}


/*////////////////////////////////////////////////////////////////////////////
:h3.EQFBResetSize - reset the size in the requested way
*/
// Description:
//    scan through all documents and issue a reset of cell size together with a
//    forced redraw of the screen if font size differ
//
//
// Arguments:
//   PTBDOCUMENT            pointer to ida
//
// Returns:
//   VOID
//
// Prereqs:
//   None
//
//
//////////////////////////////////////////////////////////////////////////////
VOID EQFBResetSize
(
   PTBDOCUMENT   pDoc                     // pointer to ida
)
{
   PTBDOCUMENT pDocStart;                 // pointer to document
   PSTEQFGEN   pstEQFGen;                 // pointer to generic struct

   pDocStart = pDoc;                      // pointer to active document

   // scan through all documents and issue a reset together with a
   // forced redraw of the screen
   do
   {
     WinPostMsg( pDoc->hwndClient, WM_EQF_FONTCHANGED, NULL, NULL);
     pDoc = pDoc->next;
   } while ( pDoc != pDocStart  ); /* enddo */

   // force a reset of service windows, too.
   pstEQFGen = (PSTEQFGEN) pDoc->pstEQFGen;
   if ( pstEQFGen )
   {
     WinPostMsg( pstEQFGen->hwndTWBS,
                 WM_EQF_FONTCHANGED, NULL, NULL);
   } /* endif */
   return;
};

/*////////////////////////////////////////////////////////////////////////////
:h2.EQFBFontSizeSampleSubProc - subclass procedure for sample text control
*/
// Description:
//    Do special handling for WM_PAINT and WM_SIZE message, pass all other
//    message to original window procedure.
//
//   Flow:
//      case WM_PAINT:
//      - write (paint) sapmle text using active font settings
//      case WM_SIZE
//      - call default AVIO window procedure
//      case OTHER:
//      - call control's original window procedure
//
// Arguments:
//  mp2 of WM_INITDLG msg = PFONTSIZEDATA pFontSizeData ptr to data structure
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
//
//////////////////////////////////////////////////////////////////////////////
MRESULT APIENTRY EQFBFONTSIZESAMPLESUBPROC
(
   HWND hwnd,
   WINMSG msg,
   WPARAM mp1,
   LPARAM mp2
)
{

   HDC      hdc;
   PAINTSTRUCT ps;                        // pointer to paint struct
   PFONTSIZEDATA pIda;
   SHORT      sItem;                   // index of item
   SHORT      sHandle;                 // handle of selected listbox item
   PVIOFONTCELLSIZE pvioCellSize;      // pointer to cell size struct
   HFONT      hFont;
   LOGFONT    lf;
   RECT       rcl;                     // rectangle to be painted
   TEXTMETRIC  TM;

   switch ( msg )
   {
      case WM_PAINT:
         hdc = BeginPaint(hwnd, &ps );
         pIda = ACCESSCTRLIDA(hwnd, PFONTSIZEDATA);
         sItem = QUERYSELECTION( GETPARENT(hwnd), ID_TB_FONTSIZE_FONTS_LB );
         if (sItem != LIT_NONE )
         {
           sHandle = GETITEMHANDLE( GETPARENT(hwnd), ID_TB_FONTSIZE_FONTS_LB, \
                                    sItem, SHORT );
           memset(&lf, 0, sizeof(lf));
           strcpy(lf.lfFaceName, pIda->efAllFaces.szFaceName[sHandle] );

           sItem = QUERYSELECTION( GETPARENT(hwnd), ID_TB_FONTSIZE_WINDOW_LB );
           if ( sItem != LIT_NONE )
           {

              sHandle =
                GETITEMHANDLE( GETPARENT(hwnd), ID_TB_FONTSIZE_WINDOW_LB, \
                               sItem, SHORT );

              //--- select correct size in the size list box ---
              pvioCellSize = pIda->vioFontCell;
              pvioCellSize += sHandle;

              lf.lfOutPrecision = OUT_STROKE_PRECIS;
              lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
              // // for Win2k systems if SHIFJIS_CHARSET is selected we did not get any size info .. therefore disable SHIFTJIS..
              lf.lfCharSet = (BYTE)GetCharSet();
              lf.lfPitchAndFamily = FIXED_PITCH | FF_DONTCARE;
              lf.lfHeight = LOWORD(pvioCellSize->cy);
              lf.lfWidth  = LOWORD(pvioCellSize->cx);
              lf.lfQuality  = DEFAULT_QUALITY;

              hFont = CreateFontIndirect( &lf );
              hFont = (HFONT)SelectObject( hdc, hFont );
              GetTextMetrics( hdc, &TM );

              if (TM.tmHeight ==0 || TM.tmAveCharWidth==0)
              {
                 lf.lfCharSet = (BYTE)GetCharSet()& (( IsDBCS_CP(GetLangOEMCP(NULL)) ) ? ~SHIFTJIS_CHARSET : 0xff);
                 DeleteObject( SelectObject( hdc, hFont ) );
                 lf.lfPitchAndFamily = FIXED_PITCH | FF_DONTCARE;
                 lf.lfHeight = LOWORD(pvioCellSize->cy);
                 lf.lfWidth  = LOWORD(pvioCellSize->cx);
                 lf.lfQuality  = DEFAULT_QUALITY;
                 hFont = CreateFontIndirect( &lf );
                 hFont = (HFONT)SelectObject( hdc, hFont );
                 GetTextMetrics( hdc, &TM );
              }

              GetClientRect( hwnd, &rcl );
              if (!UtlIsHighContrast())
              {
                SetTextColor(hdc, COLORRGBTABLE[ COL_RED ]);
                SetBkColor(hdc, COLORRGBTABLE[ COL_WHITE ]);
                // limit sample text to size of client rectangle for painting...
                ExtTextOut( hdc, 0, 0, ETO_CLIPPED | ETO_OPAQUE, &rcl,
                          pIda->szSampleText, strlen(pIda->szSampleText ),
                          (LPINT) NULL );
		      }
		      else
		      {
				DRAWTEXT(hdc, pIda->szSampleText, rcl,
				          GetSysColor(COLOR_WINDOWTEXT), GetSysColor(COLOR_WINDOW),
				          DT_LEFT );
		      }
              DeleteObject( SelectObject( hdc, hFont ) );
           } /* endif */
         } /* endif */
         EndPaint(hwnd, &ps);
         return( 0L );
   } /* endswitch */
   return ( CALLWINDOWPROC( OrgStaticProc, hwnd,msg,mp1,mp2) );
}

/*////////////////////////////////////////////////////////////////////////////
:h3.EQFBGetDefCellSize - get the default cell size
*/
//  Description:
//       get the default cell size
//
//  Flow:
//       create presentation space
//       get default cell size
//       destroy presentation space
//
//  Parameters:
//     PVIOFONTCELLSIZE  pointer to cell size structure
//
//
//////////////////////////////////////////////////////////////////////////////
VOID EQFBGetDefCellSize
(
   PVIOFONTCELLSIZE  pvioCellSize,      // FontCellSize structure
   PCHAR             pFaceName
)
{
   /*******************************************************************/
   /* use defaults for the moment                                     */
   /*******************************************************************/
   HFONT    hFont;
   LOGFONT  lf;
   HDC  hDC = GetDC( HWND_DESKTOP );
   TEXTMETRIC FontMetrics;

   if (!IsDBCS_CP(GetLangOEMCP(NULL))  )
   {
     SelectObject( hDC, GetStockObject( ANSI_FIXED_FONT ));
     GetTextMetrics( hDC, &FontMetrics );
     GetTextFace(hDC, LF_FACESIZE, (LPSTR) pFaceName );
   }
   else
   {
     memset (&lf, 0, sizeof(lf));
     // for Win2k systems if SHIFJIS_CHARSET is selected we did not get any size info
     lf.lfCharSet = (BYTE)GetCharSet();
     lf.lfPitchAndFamily=  FIXED_PITCH;
     lf.lfOutPrecision = OUT_TT_PRECIS;
     lf.lfHeight = 16;
     lf.lfWidth = 8;
     // Win2K
     strcpy (lf.lfFaceName, "MingLIU");
     strcpy( pFaceName, lf.lfFaceName);

     hFont = CreateFontIndirect(&lf );
     hFont = (HFONT)SelectObject(hDC, hFont );
     GetTextMetrics( hDC, &FontMetrics );
     DeleteObject( SelectObject( hDC, hFont ) );

     if (FontMetrics.tmHeight ==0 || FontMetrics.tmAveCharWidth==0)
     {
       lf.lfCharSet = (BYTE)GetCharSet()& (( IsDBCS_CP(GetLangOEMCP(NULL)) ) ? ~SHIFTJIS_CHARSET : 0xff);
       lf.lfPitchAndFamily=  FIXED_PITCH;
       lf.lfOutPrecision = OUT_TT_PRECIS;
       lf.lfHeight = 16;
       lf.lfWidth = 8;
       hFont = CreateFontIndirect(&lf );
       hFont = (HFONT)SelectObject(hDC, hFont );
       GetTextMetrics( hDC, &FontMetrics );
       DeleteObject( SelectObject( hDC, hFont ) );
     }
   } /* endif*/

   ReleaseDC( HWND_DESKTOP, hDC );
   pvioCellSize->cy = FontMetrics.tmHeight;
   pvioCellSize->cx = FontMetrics.tmAveCharWidth ;

}


/*////////////////////////////////////////////////////////////////////////////
:h3.EQFBSetNewCellSize - set the new cell size
*/
//  Description:
//       display the screen in the new size as requested
//
//  Flow:
//       set screen rows and columns
//       destroy any old presentation space and create an new one
//       set the new cell size, associate and get the buffer
//
//  Parameters:
//     PTBDOCUMENT       pointer to document structure
//
//
//////////////////////////////////////////////////////////////////////////////
VOID EQFBSetNewCellSize
(
   PTBDOCUMENT   pDoc,                // pointer to document structure
   ULONG         ulCx,                // new x
   ULONG         ulCy                 //   and y value
)
{
      USHORT usCursorType ;

      /****************************************************************/
      /* init our logical font structure                              */
      /****************************************************************/
      {
        HDC  hdc;
        HFONT    hFont;                     // handle of created font
        TEXTMETRIC  FontMetrics;
        LONG cx, cy;
        WORD        wTextLen;
		CHAR *paszFontFacesGlobal;


        memset(&pDoc->lf, 0, sizeof(pDoc->lf));
        /*****************************************************************************/
        /* use the appropriate character set depending on the codepage */
        /* For win2k systems don't check here for SHIFTJIS_CHARSET    */
        /* otherwise we will not get a correct size info                                          */
        /*****************************************************************************/
        pDoc->lf.lfCharSet = (BYTE)GetCharSet();
        hdc = GetDC(pDoc->hwndClient);
        pDoc->lf.lfHeight = ulCy;
        pDoc->lf.lfWidth  = ulCx;

        pDoc->lf.lfQuality  = DEFAULT_QUALITY;
        pDoc->lf.lfPitchAndFamily = FIXED_PITCH | FF_DONTCARE;
        pDoc->lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
        pDoc->lf.lfOutPrecision = OUT_STROKE_PRECIS;
        /***********************************************************/
        /* DOCTYPE has same order as IDS_TB_FONT_OTHER + i,        */
        /* hence doctype can be used as index in aszFontFacesGlobal*/
        /***********************************************************/
		paszFontFacesGlobal = get_aszFontFacesGlobal();
		paszFontFacesGlobal += (pDoc->docType * LF_FACESIZE);
        strcpy (pDoc->lf.lfFaceName, paszFontFacesGlobal);

        hFont = CreateFontIndirect( &(pDoc->lf) );
        hFont = (HFONT)SelectObject(hdc, hFont );
        GetTextMetrics( hdc, &FontMetrics );
        if (FontMetrics.tmHeight ==0 || FontMetrics.tmAveCharWidth==0)
        {
           DeleteObject( SelectObject( hdc, hFont) );
           pDoc->lf.lfCharSet = (BYTE)GetCharSet()& ((IsDBCS_CP(GetLangOEMCP(NULL)) ) ? ~SHIFTJIS_CHARSET : 0xff);
           pDoc->lf.lfHeight = ulCy;
           pDoc->lf.lfWidth  = ulCx;

           pDoc->lf.lfQuality  = DEFAULT_QUALITY;
           pDoc->lf.lfPitchAndFamily = FIXED_PITCH | FF_DONTCARE;
           pDoc->lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
           pDoc->lf.lfOutPrecision = OUT_STROKE_PRECIS;

           hFont = CreateFontIndirect(&(pDoc->lf) );
           hFont = (HFONT)SelectObject(hdc, hFont );
           GetTextMetrics( hdc, &FontMetrics );
           DeleteObject( SelectObject( hdc, hFont ) );
        }

        /*****************************************************************/
        /* check that given usCx is really the TextExtent;               */
        /* take actual TextExtent ( Greek: usCx = 0, if only TrueType    */
        /* on PC, TM.tmAveCharWidth does not give correct TextExtent,    */
        /* but must be used to create the font , 16.1.95 )               */
        /*****************************************************************/
        TEXTSIZE( hdc, "ab", cx, cy );
        wTextLen = (WORD)cx;
        pDoc->cx = (wTextLen / 2);
        pDoc->cy = ulCy;
        if ( pDoc->cx)
        {
          pDoc->usMaxScrnCols =
           (USHORT) (WinQuerySysValue(HWND_DESKTOP,SV_CXSCREEN)/pDoc->cx + 1);
        }
        else
        {
           pDoc->usMaxScrnCols =
           (USHORT) (WinQuerySysValue(HWND_DESKTOP,SV_CXSCREEN)/(pDoc->cx + 1) + 1);
        }
        if (pDoc->cy)
        {
          pDoc->usMaxScrnRows =
           (USHORT) (WinQuerySysValue(HWND_DESKTOP,SV_CYSCREEN)/pDoc->cy + 1);
        }
        else
        {
           pDoc->usMaxScrnRows =
           (USHORT) (WinQuerySysValue(HWND_DESKTOP,SV_CYSCREEN)/(pDoc->cy + 1) + 1);
        }

        pDoc->lFontLangInfo = GetFontLanguageInfo( hdc );
        /*****************************************************************************/
        /* use the appropriate character set depending on the codepage */
        /*****************************************************************************/
        pDoc->lf.lfCharSet = (BYTE)GetCharSet();

        DeleteObject( SelectObject( hdc, hFont ) );
        ReleaseDC (pDoc->hwndClient, hdc);

      }

      usCursorType = pDoc->usCursorType;           // save old cursor
      EQFBSysInit( pDoc );                         // reinit  the screen sizes
      pDoc->usCursorType = usCursorType;           // get old cursor
      EQFBSysScrnCurShape ( pDoc, (CURSOR)pDoc->usCursorType );  // set default cursor


      EQFBVioSetNewDocSize( pDoc );
}

