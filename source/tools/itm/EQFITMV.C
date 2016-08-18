/*! \file
	Description: Program for ITM visualization - general routines

	Copyright Notice:

	Copyright (C) 1990-2015, International Business Machines
	Corporation and others. All rights reserved
*/

#define INCL_EQF_MORPH
#define INCL_EQF_TM               // general Transl. Memory functions
#define INCL_EQF_TP               // public translation processor functions
#define INCL_EQF_ANALYSIS         // analysis functions
#define INCL_EQF_EDITORAPI        // editor API

// use import DLL defines for hResMod and dbcs_cp ...
#define DLLIMPORTRESMOD

#include <eqf.h>                  // General Translation Manager include file
#include "EQFB.ID"
#include <eqfitmd.id>             // id file for pulldowns

#define ITMINIT_TABLES 1
#include "EQFITM.H"
#include <malloc.h>

#include <eqftmtag.h>

/**********************************************************************/
/* test output ...                                                    */
/**********************************************************************/
#ifdef ITMTEST
  FILE *fOut;                          // test output
#endif

BOOL VisDocStart ( HAB hab, HWND hwnd, PITMVISDOC pstVisDoc );
static VOID VisDocFillAnchor (PITMIDA);
BOOL VisDocAlloc ( PITMVISDOC );
static VOID  VisActNew(PITMVISDOC, PITMVISDOC);
static VOID  VisActNewInDoc(PITMVISDOC);
static QSTATUS SetToggleCol ( PUSHORT, QSTATUS, QSTATUS, QSTATUS );

static VOID PrepStyles ( PTBDOCUMENT  pDoc );
  /********************************************************************/
  /* get size of window                                               */
  /* ulTotal      2                                                   */
  /* ulUnaligned  2                                                   */
  /* ulCrossedOut 2                                                   */
  /* alignment    1                                                   */
  /********************************************************************/
// #define ITM_STATUSBARHEIGHT  15
//#define ITM_ACTWND     10
#define ITM_CROSSEDOUT  2
#define ITM_UNALIGNED   2
#define ITM_TOTAL       2
#define ITM_ALIGN       1
#define ITM_IRREGULAR   2

static VOID ITMFillStatusBar( HWND, HPS  );

static PFNWP ITMFrameWndProc;          // Pointer to orginal child Frame Proc
BOOL VisDocInit ( PITMIDA );

static void  VisDocFillCrossedOut(PITMVISDOC);

static BOOL ITMFindNextIrregular(PITMIDA, PITMVISDOC, ULONG, ULONG,
                                 SHORT, PULONG, PULONG, PBOOL);
static BOOL ITMFindNextCrossed(PITMIDA, ULONG, SHORT, PULONG, PULONG, PBOOL);
static BOOL ITMLoopForNextCrossed ( PITMVISDOC, ULONG, ULONG, SHORT,
                                    PULONG, PULONG);

static void ITMVisMark(PTBDOCUMENT, ULONG);

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     ITMDocViewWndProc
//------------------------------------------------------------------------------
// Function call:     window procedure
//------------------------------------------------------------------------------
// Description:       window proc for visual itm documents
//------------------------------------------------------------------------------
// Parameters:        HWND hwnd   window handle
//                    USHORT msg  message number
//                    WPARAM mp1  message parameter1
//                    LPARAM mp2  message parameter2
//------------------------------------------------------------------------------
// Returncode type:   MRESULT
//------------------------------------------------------------------------------
// Returncodes:       from default window procedure
//
//------------------------------------------------------------------------------
// Function flow:     dependant on message do case processing
//                    WM_CREATE: do nothing
//                    WM_ERASEBACKGROUND
//                      get the background cleared in the system default color
//                    WM_PAINT
//                      repaint the visual document
//                    WM_SIZE
//                      size the visual document window
//                    WM_CHAR
//                      call routine which handles all WM_CHAR msg
//                    WM_BUTTON1DOWN
//                      set cursor according to mouseclick
//                    WM_BUTTON1UP
//                      release mouse button1
//                    WM_BUTTON1DBCLK
//                      mark current segment on mouse doubleclick
//                    WM_BUTTON2DBLCLK
//                      unmark segment on mousebutton 2 doubleclick
//                    WM_HSCROLL
//                      scroll horizontally
//                    WM_VSCROLL
//                      scroll vertically
//                    WM_CLOSE
//                      free all allocated areas
//                      force a close of workbench
//                    default:
//                      pass on the message to the default window procedure..
//                  endswitch
//                  return mResult
//------------------------------------------------------------------------------

MRESULT APIENTRY ITMDOCVIEWWNDPROC
(
  HWND hwnd,
  WINMSG msg,
  WPARAM mp1,
  LPARAM mp2
)
{
    MRESULT mResult = FALSE;           // window proc return value
    PITMVISDOC pVisDoc = ACCESSWNDIDA(hwnd, PITMVISDOC);

    switch( msg )
    {
         case WM_KEYDOWN:
           /***********************************************************/
           /* ignore CTRL key if it comes from an extended key...     */
           /***********************************************************/
           if ( mp2 & 0x020000000 )
           {
             /*********************************************************/
             /* ignore the request                                    */
             /*********************************************************/
             break;
           }
           else
           {
             /***********************************************************/
             /* check if help key is pressed -> in all other cases fall */
             /* through to normal processing...                         */
             /***********************************************************/
             if ( mp1 == VK_F1 )
             {
               /**************************************************************/
               /* Trigger help by posting HM_HELPSUBITEM_NOT_FOUND to TWB    */
               /**************************************************************/
               PostMessage( (HWND)UtlQueryULong( QL_TWBFRAME ),
                            HM_HELPSUBITEM_NOT_FOUND,
                            0,
                            MP2FROM2SHORT( ID_ITM_WINDOW, ID_ITM_WINDOW ));
               break;
             } /* endif */
           } /* endif */
         case WM_SYSCHAR:
         case WM_SYSKEYDOWN:
         case WM_CHAR:           // determine character and pass it to editor
           mResult = HandleITMWMCharEx( hwnd, mp1, mp2, (USHORT)msg );
           break;
        case WM_BUTTON1DOWN:    // Position cursor to pointer
          {
            UPDSTATUSBAR( (PITMIDA)pVisDoc->pITMIda );
            mResult = EQFBDispClass ( hwnd, msg, mp1, mp2, pVisDoc->pDoc);
          }
          break;

        case WM_RBUTTONDOWN:
          /* Draw the "floating" popup in the app's client area */
          {
            RECT rc;
            POINT point;

            point.x = LOWORD(mp2);
            point.y = HIWORD(mp2);

            GetClientRect( hwnd, (LPRECT)&rc);
            if (PtInRect ((LPRECT)&rc, point))
            {
              HMENU hMenu;
              HMENU hMenuTrackPopup;
              HMODULE hResMod;
              hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);

              /* Get the menu for the popup from the resource file. */
              hMenu = LoadMenu( hResMod, MAKEINTRESOURCE( ID_POPUP_MENU ) );
              if ( hMenu )
              {
                PTBDOCUMENT pDoc = pVisDoc->pDoc;

                hMenuTrackPopup = GetSubMenu( hMenu, ID_ITM_POPUP );
                if ( pDoc->pBlockMark )
                {
                  SETAABITEM( hMenuTrackPopup, IDM_UNMARKSEG,
                              (((PEQFBBLOCK)pDoc->pBlockMark)->pDoc == pDoc ));

                } /* endif */

                ClientToScreen( hwnd, (LPPOINT)&point );

                /* Draw and track the "floating" popup */
                TrackPopupMenu( hMenuTrackPopup, 0, point.x, point.y, 0,
                                ((PITMIDA)pVisDoc->pITMIda)->hwnd,
                                NULL );

                /* Destroy the menu since were are done with it. */
                DestroyMenu( hMenu );
              } /* endif */

              mResult = FALSE;
            } /* endif */
          }
          break;
        case WM_CLOSE:
          {
            HMENU    hwndMenu;
            HWND     hwndTemp = ((PITMIDA)pVisDoc->pITMIda)->hwnd ;
            /*******************************************************************/
            /* Attention: AAB now anchored at TWB main window....              */
            /*******************************************************************/
            hwndMenu = GetMenu( hwndTemp );
            ItmSetAAB(hwndMenu, FALSE);
            VisDocFree (pVisDoc );
            /**********************************************************/
            /* destroy the window                                     */
            /**********************************************************/
            WinDestroyWindow( hwnd );

            if ( !SHORT1FROMMP1(mp1) )
            {
              /********************************************************/
              /* force a close of workbench                           */
              /********************************************************/
              WinPostMsg( hwndTemp, WM_CLOSE, 0L, 0L );
            } /* endif */
          }
          break;
        case WM_KILLFOCUS:
          HideCaret( hwnd );
          DestroyCaret();
          break;
        case WM_SETFOCUS:
          {
            PTBDOCUMENT pDoc = pVisDoc->pDoc;
            PFINDDATA   pCurFindData = NULL;
            /************************************************************/
            /* set correct doc. window to be active for find            */
            /************************************************************/
            pCurFindData = EQFBGetFindData();
            if ( pCurFindData && pCurFindData->pDoc )
            {
              pCurFindData->pDoc = pDoc;
            } /* endif */

            ((PITMIDA)pVisDoc->pITMIda)->hwndFocus = hwnd;
            WinSendMsg( pDoc->next->hwndFrame, WM_NCACTIVATE, FALSE, NULL );
            WinSendMsg( hwnd, WM_NCACTIVATE, TRUE, NULL );
            CreateCaret(hwnd, (HBITMAP)NULL,
                        pDoc->vioCurShapes[pDoc->usCursorType].cx,
                        pDoc->vioCurShapes[pDoc->usCursorType].cEnd);
            SetCaretPos( pDoc->lCursorCol * pDoc->cx,pDoc->lCursorRow * pDoc->cy);
            ShowCaret( hwnd );
            /************************************************************/
		    /* get rid of any selection (if in CUA mode )               */
		    /************************************************************/
			if ( pDoc->pBlockMark && ((PEQFBBLOCK)pDoc->pBlockMark)->pDoc )
			{
				  memset( pDoc->pBlockMark, 0, sizeof( pDoc->pBlockMark ));
				  pDoc->Redraw |= REDRAW_ALL;        // force redraw the screen
				  EQFBRefreshScreen( pDoc );
			} /* endif */

		    /********************************************************/
		    /* display IME conversion window as hot-spot conversion */
		    /* window at cursor place                               */
		    /********************************************************/
		    if ( pDoc->hlfIME )
		    {
				ImeMoveConvertWin(pDoc, hwnd,
								  (SHORT)(pDoc->lCursorCol * pDoc->cx),
								  (SHORT)(pDoc->lCursorRow * pDoc->cy) );

				ImeSetFont( pDoc, hwnd, &pDoc->lf );
		    } /* endif */

          }
          break;
       case WM_ACTIVATE:
          WinSetFocus( HWND_DESKTOP, hwnd );
          break;
        case WM_MOUSEACTIVATE:
          /********************************************************/
          /* inform TENV that the active window has changed.....  */
          /********************************************************/
          if ( (hwnd != GetFocus()))
          {
            WinSetFocus( HWND_DESKTOP, hwnd );
          } /* endif */
          mResult = MA_ACTIVATE;
          break;
        /**************************************************************/
        /* forward menu selection to our frame                        */
        /**************************************************************/
       case WM_MENUCHAR:
       case WM_MENUSELECT:
         mResult = SendMessage( ((PITMIDA)pVisDoc->pITMIda)->hwnd,
                                msg, mp1, mp2 );
         break;
       case WM_WINDOWPOSCHANGING:
		  /************************************************************/
		  /* adjust positions/sizes of window to match our display    */
		  /* characteristics (character size chosen)                  */
		  /************************************************************/
		  if ( pVisDoc != NULL )
		  {
			WINDOWPOS FAR * pSWP = (WINDOWPOS FAR *) mp2;
			ULONG  ulDelta;
			PTBDOCUMENT pDoc = NULL;
			pDoc = pVisDoc->pDoc;
			if ( pDoc && pDoc->cx && pDoc->cy)
			{
			   ulDelta = 2*WinQuerySysValue(HWND_DESKTOP,SM_CXFIXEDFRAME) - 2;
			   if ( GetWindowLong( pDoc->hwndFrame, GWL_STYLE ) & WS_VSCROLL )
			   {
				 ulDelta += WinQuerySysValue(HWND_DESKTOP, SV_CXVSCROLL );
			   } /* endif */
			   pSWP->cx = ulDelta  +
							((pSWP->cx - ulDelta) / pDoc->cx ) * pDoc->cx;

			   /*******************************************************/
			   /* horizontal size ...                                 */
			   /*******************************************************/
			   ulDelta = 2*WinQuerySysValue(HWND_DESKTOP,SM_CXFIXEDFRAME);
			   if ( GetWindowLong( pDoc->hwndFrame, GWL_STYLE ) & WS_HSCROLL )
			   {
				 ulDelta += WinQuerySysValue(HWND_DESKTOP, SV_CYHSCROLL );
			   } /* endif */
			   if ( GetWindowLong( pDoc->hwndFrame, GWL_STYLE ) & WS_CAPTION )
			   {
				 ulDelta += WinQuerySysValue(HWND_DESKTOP, SV_CYTITLEBAR);
			   } /* endif */

			   pSWP->cy = ulDelta +
						  ((pSWP->cy - ulDelta) / pDoc->cy ) * pDoc->cy;
			} /* endif */
		  }
		  break;
        default:
          if ( pVisDoc && pVisDoc->pDoc )
          {
            mResult = EQFBDispClass ( hwnd, msg, mp1, mp2, pVisDoc->pDoc);
          }
          else
          {
            mResult = WinDefWindowProc ( hwnd, msg, mp1, mp2 );
          } /* endif */
          break;

    } /* switch */

    return mResult ;
}


//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     VisualStart
//------------------------------------------------------------------------------
// Function call:     VisualStart(hab, hwnd, pITMIda)
//------------------------------------------------------------------------------
// Description:       start the visualization
//------------------------------------------------------------------------------
// Parameters:        HAB   hab             anchor
//                    HWND  hwnd            window handle
//                    PITMIDA pITMIda       ITM ida
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Returncodes:       VOID
//------------------------------------------------------------------------------
// Function flow:     init editing colors and keys
//                    set focusline to line number 10
//                    register our window class
//                    allocate for each visual document
//                    init styles,colors
//                    fill anchors and alignments in VisDoc structure
//                    count lf's of all segments and store the count
//                    set both files parallel
//                    create both Visdoc windows
//                    if ok
//                      fill  status bar structure
//                    activate aab items
//                    create statusbar window
//                    if ok
//                      activate 1st segment
//                      refresh both documents
//                    else
//                      set fKill flag
//                    endif
//------------------------------------------------------------------------------

VOID VisualStart
(
 HAB     hab,
 HWND    hwnd,
 PITMIDA pITMIda
)
{
  BOOL        fOK = TRUE;                        // success indicator
  BOOL        fReset;
  HMENU       hwndMenu;
  PTBDOCUMENT pDoc;
  PTBROWOFFSET pTBRow;

  ENABLEUPDATEHWND_FALSE( hwnd );
  fOK =  EQFBInit();                   // init colors and keys
  /********************************************************************/
  /* adjust profile contents to VISITM requirements                   */
  /********************************************************************/
  EQFBFuncChangeFocusLine ( 10);
  /********************************************************************/
  /* register our view window class                                   */
  /********************************************************************/
  {
     HMODULE hResMod;
     WNDCLASSW wndclass;
     wndclass.style = CS_DBLCLKS;
     wndclass.lpfnWndProc = ITMDOCVIEWWNDPROC;
     wndclass.cbClsExtra = 0;
     wndclass.cbWndExtra = sizeof(PSZ);
     wndclass.hInstance = (HINSTANCE) hab;
     hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
     wndclass.hIcon = LoadIcon( hResMod, MAKEINTRESOURCE( DOC_ICON ) );
     wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
     wndclass.hbrBackground = (HBRUSH) GetStockObject (WHITE_BRUSH);
     wndclass.lpszMenuName = NULL;
     wndclass.lpszClassName = ITM_DOCVIEW_W;

     RegisterClassW(&wndclass);
  }

   /*******************************************************************/
   /* register the document view class                                */
   /*******************************************************************/
  {
    WNDCLASSW wndclass;
    wndclass.style = CS_DBLCLKS;
    wndclass.lpfnWndProc = ITMSTATUSBARWNDPROC;
    wndclass.cbClsExtra = 0;
    wndclass.cbWndExtra = sizeof(PSZ);
    wndclass.hInstance = (HINSTANCE) hab;
    wndclass.hIcon = NULL;
    wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
    wndclass.hbrBackground = (HBRUSH) GetStockObject (WHITE_BRUSH);
    wndclass.lpszMenuName = NULL;
    wndclass.lpszClassName = ITM_STATUSBAR_W;

    RegisterClassW(&wndclass);
  }
  /********************************************************************/
  /* create both windows                                              */
  /********************************************************************/
  fOK = VisDocInit ( pITMIda );
  /********************************************************************/
  /* sset the init values for pDoc variables                          */
  /********************************************************************/
  if ( fOK )
  {
    PrepStyles(&(pITMIda->TBSourceDoc));
    PrepStyles(&(pITMIda->TBTargetDoc));

    EQFBGetColors(pITMIda->pColorTable);        //get color table
    VisDocFillLF(pITMIda,
                 1, (pITMIda->TBSourceDoc.ulMaxSeg - 1),
                 1, (pITMIda->TBTargetDoc.ulMaxSeg - 1));

    pITMIda->fParallel = TRUE;
    ITMAdjustLF ( pITMIda,
                 1, (pITMIda->TBSourceDoc.ulMaxSeg - 1),
                 1, (pITMIda->TBTargetDoc.ulMaxSeg - 1 ));

    pITMIda->fHorizontal = FALSE;
    pITMIda->fAutoWrap = TRUE;         // set menu value for AutoWrap
    fOK = VisDocStart(hab, hwnd, &(pITMIda->stVisDocSrc));
  } /* endif */
  if ( fOK  )
  {
    fOK = VisDocStart(hab, hwnd, &(pITMIda->stVisDocTgt));
  } /* endif */
  if ( fOK )
  {
  /********************************************************************/
  /* activate AAB and create status bar...                            */
  /********************************************************************/
    ITMCountStatBar (pITMIda);
     /*******************************************************************/
     /* Attention: AAB now anchored at TWB main window....              */
     /*******************************************************************/
     hwndMenu = GetMenu( pITMIda->hwnd );

    ItmSetAAB(hwndMenu, TRUE);
  } /* endif */

  if ( fOK )
  {
    SWP     swp;
    USHORT  usCY = (USHORT)WinQuerySysValue( HWND_DESKTOP, SV_CYMENU );
    WinQueryWindowPos( hwnd, &swp );

    pITMIda->hStatusBarWnd =
        CreateWindowW( ITM_STATUSBAR_W,
                      L"",                //window caption
                      WS_CHILD |
                      WS_CLIPSIBLINGS |
                      FCF_BORDER | WS_VISIBLE,
                      0 , 0 , swp.cx, usCY,
                      hwnd,              //parent wnd handle
                      NULL,
                      (HINSTANCE)UtlQueryULong( QL_HAB ),
                      NULL );

    ANCHORWNDIDA( pITMIda->hStatusBarWnd, pITMIda );

  } /* endif */
  if ( fOK )
  {
    fReset = FALSE;

    if ( !pITMIda->usNumPrepared || !pITMIda->stVisDocSrc.ulVisActSeg )
    {
      pITMIda->stVisDocSrc.ulVisActSeg = 1;
    }
    VisActivateSeg(pITMIda, &(pITMIda->stVisDocSrc), CURRENT,
                      fReset, pITMIda->stVisDocSrc.ulVisActSeg );
    /********************************************************************/
    /* position scollbars for source and target                         */
    /********************************************************************/
    pDoc = &pITMIda->TBTargetDoc;
    pTBRow = pDoc->TBRowOffset+1 + pDoc->lCursorRow; // get first element
    if ( pTBRow->ulSegNum )     // document already loaded?
    {
      SetScrollbar( pDoc );
    } /* endif */
    pDoc = &pITMIda->TBSourceDoc;
    pTBRow = pDoc->TBRowOffset+1 + pDoc->lCursorRow; // get first element
    if ( pTBRow->ulSegNum )     // document already loaded?
    {
      SetScrollbar( pDoc );
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* force update of whole window                                     */
  /********************************************************************/
  ENABLEUPDATEHWND_TRUE( hwnd );
  if ( fOK )
  {
      RECT rc;
      GetClientRect( hwnd, (LPRECT)&rc);
      ITMSetWindowPos( pITMIda, (SHORT)rc.right, (SHORT)rc.bottom );
  } /* endif */

  if ( !fOK )
  {
    pITMIda->fKill = TRUE;
  } /* endif */

} /* end of function VisualStart */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     VisDocInit
//------------------------------------------------------------------------------
// Function call:     VisDocInit(pITMIda)
//------------------------------------------------------------------------------
// Description:       init visdoc structure
//------------------------------------------------------------------------------
// Parameters:        PITMIDA   pITMIda
//------------------------------------------------------------------------------
// Returncode type:   BOOL
//------------------------------------------------------------------------------
// Returncodes:       TRUE   success
//                    FALSE  error occurred
//------------------------------------------------------------------------------
// Function flow:     set all pDoc ptrs in Visdoc structure
//                    set pDOc next, previous, twin
//                    init that no user changes yet occurred
//                    alloc structures in visdoc
//                    if ok
//                      if not already prepared
//                         fill index and anchor structs in visdoc
//                      else
//                         set ptrs of index and anchor structs in visdoc
//                         to the structs read in from continuation file
//------------------------------------------------------------------------------
BOOL
VisDocInit
(
  PITMIDA   pITMIda
)
{
  BOOL     fOK = TRUE;

  pITMIda->stVisDocTgt.pITMIda = pITMIda->stVisDocSrc.pITMIda = pITMIda;
  pITMIda->stVisDocSrc.pDoc = &pITMIda->TBSourceDoc;
  pITMIda->stVisDocTgt.pDoc = &pITMIda->TBTargetDoc;

  pITMIda->TBSourceDoc.docType = VISSRC_DOC;
  pITMIda->TBTargetDoc.docType = VISTGT_DOC;
  pITMIda->TBSourceDoc.twin = &(pITMIda->TBTargetDoc);
  pITMIda->TBTargetDoc.twin = &(pITMIda->TBSourceDoc);
  pITMIda->TBSourceDoc.next = &(pITMIda->TBTargetDoc);
  pITMIda->TBTargetDoc.next = &(pITMIda->TBSourceDoc);
  pITMIda->TBSourceDoc.prev = &(pITMIda->TBTargetDoc);
  pITMIda->TBTargetDoc.prev = &(pITMIda->TBSourceDoc);
  pITMIda->stVisDocSrc.fChanged = FALSE;
  pITMIda->stVisDocTgt.fChanged = FALSE;

  /******************************************************************/
  /* prepare the input for the visualisation...                     */
  /******************************************************************/
  fOK = VisDocAlloc(&(pITMIda->stVisDocSrc));
  if ( fOK )
  {
    fOK = VisDocAlloc(&(pITMIda->stVisDocTgt));
  } /* endif */
  if ( fOK )
  {
    /********************************************************************/
    /* fill index entry in list of alignments in both visdocs           */
    /********************************************************************/
    if ( !pITMIda->usNumPrepared )
    {
      VisDocFillIndex (pITMIda, 1, 1, 1 );   //files currently aligned
      VisDocFillAnchor(pITMIda);
      VisDocFillCrossedOut(&(pITMIda->stVisDocSrc));
      VisDocFillCrossedOut(&(pITMIda->stVisDocTgt));
    }
    else
    {
      pITMIda->stVisDocSrc.pulNumAligned = pITMIda->pulSrcNumAlign;
      pITMIda->stVisDocSrc.pulAnchor  = pITMIda->pulSrcAnchor;
      pITMIda->stVisDocSrc.pVisState  = pITMIda->pSrcVisState;
      pITMIda->stVisDocTgt.pulNumAligned = pITMIda->pulTgtNumAlign;
      pITMIda->stVisDocTgt.pulAnchor = pITMIda->pulTgtAnchor;
      pITMIda->stVisDocTgt.pVisState = pITMIda->pTgtVisState;
    } /* endif */
  }
  /********************************************************************/
  /* setup our special ITM keys                                       */
  /********************************************************************/
  EQFBBuildITMKeyTable( &DefITMKeyTable[0], LAST_ITMFUNC );

  return(fOK);
} /* end of function VisDocInit */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     VisDocAlloc
//------------------------------------------------------------------------------
// Function call:     VisDocAlloc(pVisDoc)
//------------------------------------------------------------------------------
// Description:       allocate all space for VisDoc structure
//------------------------------------------------------------------------------
// Parameters:        PITMVISDOC pVisDoc     VisDoc structure
//------------------------------------------------------------------------------
// Returncode type:   BOOL
//------------------------------------------------------------------------------
// Returncodes:       TRUE   alloc ok
//                    FALSE  alloc failed
//------------------------------------------------------------------------------
// Function flow:     get number of segments in document
//                    huge alloc alignment array
//                    huge alloc anchor array
//                    huge alloc Visual states array
//                    if alloc failed return FALSE
//------------------------------------------------------------------------------
BOOL
VisDocAlloc
(
   PITMVISDOC   pstVisDoc
)
{
  BOOL         fOK;
  ULONG        ulAllocLen;
  PTBDOCUMENT  pTBDoc;
  PITMIDA      pITMIda;
  ULONG        ulI;

  pTBDoc = pstVisDoc->pDoc;
  pITMIda = (PITMIDA) pstVisDoc->pITMIda;
  /********************************************************************/
  /* alloc Visdoc structure                                           */
  /********************************************************************/
  ulAllocLen = pTBDoc->ulMaxSeg + 1;
  fOK = TRUE;

  if ( !pITMIda->usNumPrepared )
  {
    ALLOCHUGE( pstVisDoc->pulNumAligned, ULONG*, ulAllocLen, sizeof( ULONG));
    ALLOCHUGE( pstVisDoc->pulAnchor, ULONG*, ulAllocLen, sizeof( ULONG ) );
    ALLOCHUGE( pstVisDoc->pVisState, FLAGVIS*, ulAllocLen, sizeof( FLAGVIS) );

    if ( !pstVisDoc->pulNumAligned )
    {
      fOK = FALSE;
    } /* endif */
    if ( !pstVisDoc->pulAnchor )
    {
      fOK = FALSE;
    } /* endif */
    if ( !pstVisDoc->pVisState )
    {
      fOK = FALSE;
    } /* endif */

    if ( fOK )
    {
      for ( ulI=0;ulI < pTBDoc->ulMaxSeg ;ulI++ )
      {
        pstVisDoc->pVisState[ulI].UserAnchor = FALSE;
        pstVisDoc->pVisState[ulI].CrossedOut = FALSE;
        pstVisDoc->pVisState[ulI].UserJoin   = FALSE;
        pstVisDoc->pVisState[ulI].OverCross  = FALSE;
        pstVisDoc->pVisState[ulI].UserSplit  = FALSE;
      } /* endfor */
      pstVisDoc->ulVisActSeg = 0;

    } /* endif */
  } /* endif */
  ALLOCHUGE( pstVisDoc->pbLFNum, BYTE*, ulAllocLen, sizeof( BYTE) );

  if ( !pstVisDoc->pbLFNum  )
  {
    fOK = FALSE;
  } /* endif */

   return (fOK);
} /* end of function VisDocAlloc */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     VisDocFree
//------------------------------------------------------------------------------
// Function call:     VisDocFree (pVisDoc)
//------------------------------------------------------------------------------
// Description:       free allocated area in visual document
//------------------------------------------------------------------------------
// Parameters:         PITMVISDOC   pstVisDoc        Visual document structure
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Returncodes:       none
//------------------------------------------------------------------------------
// Function flow:     free alignment array, anchor array and VisState
//                     in the visdoc structure
//                    free Buffer for tokens and blockmark
//------------------------------------------------------------------------------
VOID
VisDocFree
(
   PITMVISDOC   pstVisDoc
)
{
  PTBDOCUMENT  pTBDoc;

  pTBDoc = pstVisDoc->pDoc;

  FREEHUGE( pstVisDoc->pulNumAligned );
  FREEHUGE( pstVisDoc->pulAnchor );
  FREEHUGE( pstVisDoc->pbLFNum  );
  FREEHUGE( pstVisDoc->pVisState );

   pstVisDoc->pulNumAligned = NULL;
   pstVisDoc->pulAnchor     = NULL;
   pstVisDoc->pbLFNum       = NULL;
   pstVisDoc->pVisState     = NULL;
   pstVisDoc->ulVisActSeg = 0;

   UtlAlloc( (PVOID *) &pTBDoc->pTokBuf, 0L, 0L, NOMSG );
   UtlAlloc( (PVOID *) &pTBDoc->pBlockMark, 0L, 0L, NOMSG );
   if (pTBDoc->pUndoSeg) UtlAlloc( (PVOID *) &pTBDoc->pUndoSeg, 0L, 0L, NOMSG);
   if (pTBDoc->pUndoSegW) UtlAlloc( (PVOID *) &pTBDoc->pUndoSegW, 0L, 0L, NOMSG);
   if (pTBDoc->pEQFBWorkSegmentW) UtlAlloc( (PVOID *) &pTBDoc->pEQFBWorkSegmentW,
                                             0L, 0L, NOMSG );
   if ( pTBDoc->pContext )  UtlAlloc((PVOID *) &pTBDoc->pContext, 0L, 0L, NOMSG );

	 /********************************************************/
	 /* free memory allocated for DBCS input methods ...     */
	 /********************************************************/

	 if ( pTBDoc->hlfIME )
	 {
       ImeMoveConvertWin( pTBDoc, pTBDoc->hwndClient, -1, -1);
	   GlobalFree(pTBDoc->hlfIME);
	   pTBDoc->hlfIME = 0;
	 } /* endif */

   return ;

} /* end of function VisDocFree */
//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     VisDocStart
//------------------------------------------------------------------------------
// Function call:     VisDocStart(hab, hwnd, pstVisDoc)
//------------------------------------------------------------------------------
// Description:       create Visdoc window
//------------------------------------------------------------------------------
// Parameters:        HAB   hab       anchor block
//                    HWND  hwnd      window handle
//                    PITMVISDOC pstVisDoc  ptr to Visdoc structure
//------------------------------------------------------------------------------
// Returncode type:   BOOL
//------------------------------------------------------------------------------
// Returncodes:       TRUE     success
//------------------------------------------------------------------------------
// Function flow:     init VisState of Visdocument
//                    set pSeg->qStatus for document
//                    create Visdoc window
//                    if ok
//                      anchor our VisDoc structure
//                      position the window
//                      prepare the AVIO stuff
//                      allocate buffer and Blockmark
//                    if still ok
//                      size visdoc window
//                      position at top of documnet
//                    return success indicator
//------------------------------------------------------------------------------

BOOL VisDocStart
(
 HAB     hab,
 HWND    hwnd,
 PITMVISDOC pstVisDoc
)
{
  ULONG flFrameFlags = FCF_VERTSCROLL  | FCF_HORZSCROLL | FCF_DLGBORDER |
                       FCF_TITLEBAR;
  PVIOFONTCELLSIZE pVioFont;                     // pointer to font
  BOOL          fOK = TRUE;                        // success indicator
  PTBDOCUMENT   pTBDoc;
  SWP           swp;
  ULONG         ulSegNum;
  PITMIDA       pITMIda = NULL;
  PSZ           pFile;                           // pointer to source filename

  hab;
  pTBDoc = pstVisDoc->pDoc;
/**********************************************************************/
/* set pSeg->qStatus to get the VisITM colouring                      */
/**********************************************************************/
  ulSegNum = 1;                      //start segnum
  VisSetQStatus( pstVisDoc, ulSegNum, pTBDoc->ulMaxSeg );

/********************************************************/
/* allow for DBCS input methods ...                     */
/********************************************************/
pTBDoc->hlfIME = GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE,
									(LONG)sizeof(LOGFONT));
/**********************************************************************/
/* display document view                                              */
/**********************************************************************/
pTBDoc->hwndClient = pTBDoc->hwndFrame =
      CreateWindowW( ITM_DOCVIEW_W,
                      L"",                //window caption
                      WS_CHILD |
                      WS_CLIPSIBLINGS |
                      flFrameFlags,
                      CW_USEDEFAULT,
                      CW_USEDEFAULT,
                      CW_USEDEFAULT,
                      CW_USEDEFAULT,
                      hwnd,              //parent wnd handle
                      NULL,
                      (HINSTANCE)UtlQueryULong( QL_HAB ),
                      NULL );
  if ( pTBDoc->hwndFrame )
  {
    HMODULE hResMod;
    hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);

  /********************************************************************/
  /* anchor our ITM ida and activate frame subproc                    */
  /********************************************************************/
    ANCHORWNDIDA( pTBDoc->hwndFrame, pstVisDoc );
    ANCHORWNDIDA( pTBDoc->hwndClient, pstVisDoc );
    pITMIda = (PITMIDA) pstVisDoc->pITMIda;
    if ( pTBDoc->docType == VISSRC_DOC )
    {
      PSZ  pBuffer = pITMIda->szBuffer;
      pFile = UtlGetFnameFromPath(pITMIda->chSourceFile);
      if ( !pFile )
      {
        pFile = pITMIda->chSourceFile;
      } /* endif */
      WinLoadString( NULLHANDLE, hResMod, IDS_ITM_SRCDOC,
                     100, pBuffer );
      strcat( pBuffer, " " );
      strcat( pBuffer, pFile );

      WinSetWindowText( pTBDoc->hwndFrame, pBuffer );
    }
    else
    {
      PSZ  pBuffer = pITMIda->szBuffer;
      pFile = UtlGetFnameFromPath(pITMIda->chTargetFile);
      if ( !pFile )
      {
        pFile = pITMIda->chTargetFile;
      } /* endif */
      WinLoadString( NULLHANDLE, hResMod, IDS_ITM_TGTDOC,
                     100, pBuffer );
      strcat( pBuffer, " " );
      strcat( pBuffer, pFile );


      WinSetWindowText( pTBDoc->hwndFrame, pBuffer );
    } /* endif */
    /******************************************************************/
    /* prepare the avio stuff....                                     */
    /******************************************************************/
    // without this call the font size maybe 0 before open document
    EQFBReadProfile();
    
    pVioFont = get_vioFontSize();
    if ( pTBDoc->docType == VISSRC_DOC )
    {
      EQFBSetNewCellSize( pTBDoc, (pVioFont + VISSRC_DOC)->cx,
                          (pVioFont + VISSRC_DOC)->cy );
    }
    else
    {
      EQFBSetNewCellSize( pTBDoc,(pVioFont + VISTGT_DOC)->cx,
                          (pVioFont + VISTGT_DOC)->cy );
    } /* endif */

    fOK =  UtlAlloc((PVOID *) &pTBDoc->pEQFBWorkSegmentW, 0L,
                        (LONG) MAX_SEGMENT_SIZE * sizeof(CHAR_W), ERROR_STORAGE );

    if ( fOK )
    {
      fOK = UtlAlloc( (PVOID *) &pTBDoc->pTokBuf, 0L, (LONG) TOK_BUFFER_SIZE,
                    ERROR_STORAGE );
    } /* endif */

    if ( fOK )                    // allocate buffer for marking area
    {
       fOK = UtlAlloc( (PVOID *) &pTBDoc->pBlockMark, 0L,
                       (LONG) max( sizeof( EQFBBLOCK ), MIN_ALLOC) ,
                       ERROR_STORAGE );
    } /* endif */

    if ( fOK && (pTBDoc->docType == VISTGT_DOC ))   //alloc space for UNDO func
    {
       //if alloc fails, message is displayed, user can go ahead,
       //but UNDO is not possible
       if ( UtlAlloc((PVOID *)&pTBDoc->pUndoSegW,
                     0L, (LONG)MAX_SEGMENT_SIZE * sizeof(CHAR_W),ERROR_STORAGE) )
       {
         pTBDoc->fUndoState = FALSE;           //init fUndoState
         pTBDoc->usUndoSegOff = 0;
       } /* endif */
    } /* endif */
   }
   else
   {
     fOK = FALSE;
   }
   if ( fOK )
   {
       PITMIDA pITMIda;
       pTBDoc->sSrcLanguage = -1;          // init!!

       pITMIda = (PITMIDA) pstVisDoc->pITMIda;

       if (pTBDoc->docType == VISTGT_DOC )
       {
          MorphGetLanguageID( pITMIda->szTargetLang, &pTBDoc->sSrcLanguage);
       }
       else
       {
          MorphGetLanguageID( pITMIda->szSourceLang, &pTBDoc->sSrcLanguage);
       }
   }

   if ( fOK )
   {
     WinQueryWindowPos( hwnd, &swp );
     /*******************************************************************/
     /* get position of main window and size our two document view      */
     /* windows inside....                                              */
     /*******************************************************************/
     ITMVertWindowPos(pTBDoc, hwnd, swp); // for TOP97 nec is this ok?
     if (pTBDoc->fAutoLineWrap )
     {
       pTBDoc->fLineWrap = pITMIda->fAutoWrap;   // set correct style
       if ( pTBDoc->fLineWrap )
       {
         VisDocAddSoftLF(pTBDoc, pITMIda); // add wrapping accord. to wnd size
       } /* endif */
     } /* endif */
     EQFBFuncTopDoc( pTBDoc );         // get to the beginning

//   ENABLEUPDATEHWND( hwnd, TRUE );

     if ( pTBDoc->docType == VISSRC_DOC )
     {
        WinSetActiveWindow(HWND_DESKTOP, pTBDoc->hwndFrame );
     } /* endif */
   } /* endif */
   return (fOK);
} /* end of function VisDocStart */

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     ITMVertWindowPos
//------------------------------------------------------------------------------
// Function call:     ITMVertWindowPos(pTBDoc, hwnd, swp)
//------------------------------------------------------------------------------
// Description:       position visdoc windows vertically
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT  pTBDoc,
//                    HWND         hwnd,
//                    SWP          swp
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Returncodes:       none
//------------------------------------------------------------------------------
// Function flow:     set size&position of src and tgt visdoc window
//------------------------------------------------------------------------------
VOID
ITMVertWindowPos
(
  PTBDOCUMENT  pTBDoc,
  HWND         hwnd,
  SWP          swp
)
{
   USHORT  usCY = (USHORT) WinQuerySysValue( HWND_DESKTOP, SV_CYMENU );
   if ( pTBDoc->docType == VISSRC_DOC )
   {
     WinSetWindowPos( pTBDoc->hwndFrame, // Set size and pos
                    hwnd,
                    0,
                    usCY,
                    (SHORT)(swp.cx/2),
                    (SHORT)(swp.cy - usCY),
                    (USHORT)(EQF_SWP_SIZE | EQF_SWP_MOVE | EQF_SWP_SHOW));
   }
   else
   {
     WinSetWindowPos( pTBDoc->hwndFrame, // Set size and pos
                    hwnd,
                    (USHORT)(swp.cx/2),
                    usCY,
                    (SHORT)(swp.cx/2),
                    (SHORT)(swp.cy - usCY),
                    (USHORT)(EQF_SWP_SIZE | EQF_SWP_MOVE | EQF_SWP_SHOW ));
   } /* endif */

  return;
} /* end of function ITMVertWindowPos(pTBDoc, swp) */


//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     ITMSetWindowPos
//------------------------------------------------------------------------------
// Function call:     ITMSetWindowPos( pITMIda, cx, cy)
//------------------------------------------------------------------------------
// Description:       position ITM windows inside client area
//------------------------------------------------------------------------------
// Parameters:        PITMIDA      pIda,
//                    SHORT        cx,
//                    SHORT        cy
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Returncodes:       none
//------------------------------------------------------------------------------
// Function flow:     set size & position of src/tgt windows and status
//------------------------------------------------------------------------------
VOID ITMSetWindowPos
(
  PITMIDA pIda,
  SHORT   cx,
  SHORT   cy
)
{
  PTBDOCUMENT pDoc;
  PTBROWOFFSET pTBRow;
  /*******************************************************************/
  /* usCY statusbar height                                           */
  /*******************************************************************/
  SHORT sCY = (SHORT) WinQuerySysValue( HWND_DESKTOP, SV_CYMENU );

 if ( pIda->fHorizontal )
 {
   SetWindowPos( pIda->TBSourceDoc.hwndFrame, pIda->hwnd,
                 0, 0, cx , (cy - sCY) / 2,
                 SWP_NOZORDER | SWP_SHOWWINDOW | EQF_SWP_NOADJUST |
                 SWP_NOACTIVATE );
   SetWindowPos( pIda->TBTargetDoc.hwndFrame, pIda->hwnd,
                 0, (cy - sCY) / 2, cx, (cy - sCY) / 2,
                 SWP_NOZORDER | SWP_SHOWWINDOW | EQF_SWP_NOADJUST |
                 SWP_NOACTIVATE );
   SetWindowPos( pIda->hStatusBarWnd, pIda->hwnd,
                 0, cy - sCY,
                 cx, sCY,
                 SWP_NOZORDER | SWP_SHOWWINDOW | EQF_SWP_NOADJUST |
                 SWP_NOACTIVATE );
 }
 else
 {
   SetWindowPos( pIda->TBSourceDoc.hwndFrame, pIda->hwnd,
                 0, 0,
                 (USHORT)(cx/2 ),
                 cy - sCY,
                 SWP_NOZORDER | SWP_SHOWWINDOW | EQF_SWP_NOADJUST |
                 SWP_NOACTIVATE );
   SetWindowPos( pIda->TBTargetDoc.hwndFrame, pIda->hwnd,
                 (USHORT)(cx/2),
                 0,
                 (USHORT)(cx/2),
                 cy - sCY,
                 SWP_NOZORDER | SWP_SHOWWINDOW | EQF_SWP_NOADJUST |
                 SWP_NOACTIVATE );
   SetWindowPos( pIda->hStatusBarWnd, pIda->hwnd,
                 0, cy - sCY,
                 cx, sCY,
                 SWP_NOZORDER | SWP_SHOWWINDOW | EQF_SWP_NOADJUST |
                 SWP_NOACTIVATE );
 } /* endif */
  /********************************************************************/
  /* position scollbars for source and target                         */
  /********************************************************************/
  pDoc = &pIda->TBTargetDoc;
  pTBRow = pDoc->TBRowOffset+1 + pDoc->lCursorRow; // get first element
  if ( pTBRow->ulSegNum )     // document already loaded?
  {
    SetScrollbar( pDoc );
  } /* endif */
  pDoc = &pIda->TBSourceDoc;
  pTBRow = pDoc->TBRowOffset+1 + pDoc->lCursorRow; // get first element
  if ( pTBRow->ulSegNum )     // document already loaded?
  {
    SetScrollbar( pDoc );
  } /* endif */

  UPDSTATUSBAR( pIda );
  return;
} /* end of function ITMSetWindowPos */
//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     ITMTile(pITMIda)
//------------------------------------------------------------------------------
// Function call:     ITMTile(pITMIda, fHorizontal)
//------------------------------------------------------------------------------
// Description:       toggle positioning of both visdoc windows
//------------------------------------------------------------------------------
// Parameters:        PITMIDA   pITMIda,
//                    BOOL      fHorizontal
//------------------------------------------------------------------------------
// Returncode type:   void
//------------------------------------------------------------------------------
// Returncodes:       none
//------------------------------------------------------------------------------
// Function flow:     if windows should not be horizontally
//                      set to vertical visdoc windows
//                    else
//                      set windows horizontally
//                    paint whole window again
//------------------------------------------------------------------------------
VOID
ITMTile
(
  PITMIDA   pITMIda,
  BOOL      fHorizontal
)
{
  SWP       swp;
  PITMVISDOC pVisDoc;


  pVisDoc = ACCESSWNDIDA(pITMIda->hwndFocus, PITMVISDOC);

/**********************************************************************/
/* clear window to avoid scrambled borders (half characters)          */
/**********************************************************************/
  InvalidateRect( pITMIda->TBSourceDoc.hwndFrame, NULL, TRUE);
  InvalidateRect( pITMIda->TBTargetDoc.hwndFrame, NULL, TRUE);

  {
    RECT rect;
    GetClientRect( pITMIda->hwnd, &rect );
    swp.cx = (SHORT)rect.right;
    swp.cy = (SHORT)rect.bottom;
  }
  if ( !fHorizontal )
  {
    /******************************************************************/
    /* set to vertical visdoc windows                                 */
    /******************************************************************/
    pITMIda->fHorizontal = FALSE;
    ITMSetWindowPos( pITMIda, swp.cx, swp.cy );
  }
  else
  {
    /******************************************************************/
    /* set to horizontal visdoc windows                               */
    /******************************************************************/
    pITMIda->fHorizontal = TRUE;
    ITMSetWindowPos( pITMIda, swp.cx, swp.cy );
  } /* endif */

  WinSetActiveWindow(HWND_DESKTOP, pVisDoc->pDoc->hwndFrame );

  WinSendMsg( pITMIda->hwnd, WM_PAINT, NULL, NULL );

  return;
} /* end of function ITMTile(pITMIda) */

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     VisSetQStatus
//------------------------------------------------------------------------------
// Function call:     VisSetQStatus( pstVisDoc, ulStartSeg, ulEndSeg)
//------------------------------------------------------------------------------
// Description:       set qStatus according to aligning status
//------------------------------------------------------------------------------
// Parameters:        PITMVISDOC pstVisDoc     visual doc structure
//                    ULONG      ulSegNum      starting segment
//                    ULONG      ulEndSeg      end segment
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Returncodes:       none
//------------------------------------------------------------------------------
// Function flow:     currently: reset qStatus of total document
//                    start mit toggle col one
//                    while not at endseg
//                      if any kind of nop seg:
//                         if segment is anchor
//                           set QF_NOP_ANCHOR with according Toggle col
//                         else
//                           set QF_NOP
//                      else
//                        if segment is crossed out
//                          set qStatus = QF_CROSSED_OUT
//                        else
//                          if segment is anchor
//                             if overcross, set QF_OVERCROSS
//                             else set QF_ANCHOR with toggle col
//                          else
//                             get index in alignment structure
//                             switch (type of alignment)
//                               case 1:0 :
//                                    set qStatus = QF_VALID_10
//                               case 0:1
//                                    set qStatus = QF_VALID_01
//                               case 1:1
//                               case 1:2
//                               case 2:1
//                                    set qStatus = QF_VALID_11 with
//                                     according toggle col
//                               case 2:2
//                                    set qStatus to QF_VALID_11 with
//                                    toggle col; sshould not occur
//                             endswitch
//                          endif
//                        endif
//                      endif
//                    endwhile
//------------------------------------------------------------------------------

VOID
VisSetQStatus
(
 PITMVISDOC pstVisDoc,
 ULONG      ulStartSeg,
 ULONG      ulEndSeg
)
{
  ULONG   ulSegNum;
  PITMIDA pITMIda;
  BOOL    fSrcDoc;
  PTBSEGMENT pSeg;
  PTBDOCUMENT pDoc;
  ULONG      ulIndex;
  USHORT      usColToggle;
  BOOL        fStyleProtect;


  pITMIda = (PITMIDA) pstVisDoc->pITMIda;
  pDoc = pstVisDoc->pDoc;
/**********************************************************************/
/* for tests: reset qstatus of total document                         */
/**********************************************************************/
  ulStartSeg = 1;
  ulEndSeg = pDoc->ulMaxSeg;

  if ( pDoc->docType == VISSRC_DOC )
  {
    fSrcDoc = TRUE;
  }
  else
  {
    fSrcDoc = FALSE;
  } /* endif */

  /********************************************************************/
  /* toggle NOP_ANCHOR color only if not shrink or compact            */
  /* if shrink or compact, use QF_NOP only                            */
  /********************************************************************/
  if (pDoc->DispStyle == DISP_PROTECTED )
  {
    fStyleProtect = TRUE;
  }
  else
  {
    fStyleProtect = FALSE;
  } /* endif */

  usColToggle = COL1;
  ulSegNum =  ulStartSeg;
  pSeg = EQFBGetVisSeg(pDoc, &ulSegNum);

  while ( pSeg && (ulSegNum <= ulEndSeg))
  {
    if ( (pSeg->qStatus == QF_NOP)
         || (pSeg->qStatus == QF_NOP_ANCHOR_1)
         || (pSeg->qStatus == QF_NOP_ANCHOR_2)
         || (pSeg->qStatus == QF_NOP_ANCHOR_3)
         || (pSeg->qStatus == QF_CROSSED_OUT_NOP) )
    {
      if ( pstVisDoc->pulAnchor[ulSegNum] && fStyleProtect )
      {
        pSeg->qStatus = (USHORT)SetToggleCol (&usColToggle, QF_NOP_ANCHOR_1, QF_NOP_ANCHOR_2, QF_NOP_ANCHOR_3 );
      }
      else if (pstVisDoc->pVisState[ulSegNum].CrossedOut)
      {
        pSeg->qStatus = QF_CROSSED_OUT_NOP;
      }
      else
      {
        pSeg->qStatus = QF_NOP;
      } /* endif */
    }
    else
    {
      if ( pstVisDoc->pVisState[ulSegNum].CrossedOut )
      {
        pSeg->qStatus = QF_CROSSED_OUT;
      }
      else if (pstVisDoc->pulAnchor[ulSegNum] )
      {
        if ( pstVisDoc->pVisState[ulSegNum].OverCross )
        {
          pSeg->qStatus = QF_OVERCROSS;
        }
        else
        {
          pSeg->qStatus = (USHORT)SetToggleCol (&usColToggle, QF_ANCHOR_1, QF_ANCHOR_2, QF_ANCHOR_3 );
        } /* endif */
      }
      else if (pstVisDoc->pulNumAligned[ulSegNum])
      {
        ulIndex = pstVisDoc->pulNumAligned[ulSegNum];

        switch (pITMIda->Aligned.pbType[ulIndex])
        {
          case  ONE_ONE:
          case  ONE_TWO:
          case  TWO_ONE:
            /**********************************************************/
            /* set 12 status only if extra required by user           */
            /**********************************************************/
            pSeg->qStatus = (USHORT)SetToggleCol (&usColToggle, QF_VALID_11_1, QF_VALID_11_2, QF_VALID_11_3 );
            break;
          case  ONE_NUL:
            pSeg->qStatus = QF_VALID_10;
            break;
          case  NUL_ONE:
            pSeg->qStatus = QF_VALID_01;
            break;
          case  TWO_TWO:              //bad alignment
            pSeg->qStatus = (USHORT)SetToggleCol (&usColToggle, QF_VALID_11_1, QF_VALID_11_2, QF_VALID_11_3 );
            break;
        } /* switch */
      } /* endif */
    } /* endif */
    ulSegNum++;
    pSeg = EQFBGetVisSeg(pDoc, &ulSegNum);
  } /* endwhile */

} /* end of function VisSetQStatus( pstVisDoc, ulSegNum, pTBDoc->ulMaxSeg ) */

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     VisActivateSeg
//------------------------------------------------------------------------------
// Function call:     VisActivateSeg(pITMIda, pstVisDoc, sDirection,
//                                   fReset, usStartSeg)
//------------------------------------------------------------------------------
// Description:       activate specified segment
//------------------------------------------------------------------------------
// Parameters:        PITMIDA       pITMIda,
//                    PITMVISDOC    pstVisDoc,
//                    SHORT         sDirection,
//                    BOOL          fReset,
//                    USHORT        usStartSeg
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Returncodes:       _
//------------------------------------------------------------------------------
// Function flow:     find current alignment
//                    if found
//                     if fReset is set, reset qstatus of old active alignment
//                    set active alignment to current alignment found in
//                     alignment structure
//                    activate new actibe alignment
//------------------------------------------------------------------------------
VOID
VisActivateSeg
(
  PITMIDA       pITMIda,
  PITMVISDOC    pstVisDoc,
  SHORT         sDirection,
  BOOL          fReset,              // true: reset old qstatus of actseg
  ULONG         ulStartSeg           //in which doc?
)
{
  PITMVISDOC    pstVisSrc;
  PITMVISDOC    pstVisTgt;
  ULONG         ulIndex;
  ULONG         ulVisActSegOld;

  ulIndex = FindCurAlignIndex(pstVisDoc, ulStartSeg, sDirection);

  if ( ulIndex )
  {
    /******************************************************************/
    /* reset qstatus of old visactseg                                 */
    /******************************************************************/
    ulVisActSegOld = pstVisDoc->ulVisActSeg;
    pstVisSrc = &(pITMIda->stVisDocSrc);
    pstVisTgt = &(pITMIda->stVisDocTgt);
    if ( ulVisActSegOld && fReset)
    {
      VisActReset(pstVisSrc, pstVisTgt);
    } /* endif */

    pstVisSrc->ulVisActSeg = pITMIda->Aligned.pulSrc[ulIndex];
    pstVisTgt->ulVisActSeg = pITMIda->Aligned.pulTgt1[ulIndex];

    VisActNew (pstVisSrc, pstVisTgt);                //goto new alignment
    UPDSTATUSBAR( (PITMIDA)pstVisDoc->pITMIda );     //update statusbar
  } /* endif */

} /* end of function VisActivateSeg */


//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     ITMFuncNextAnchor
//------------------------------------------------------------------------------
// Function call:     ITMFuncNextAnchor (pITMIda, sDirection)
//------------------------------------------------------------------------------
// Description:       goto next anchor , skips overcross anchors
//                    called from goto menuitem
//------------------------------------------------------------------------------
// Parameters:        PITMIDA    pITMIda,               ITMida
//                    SHORT      sDirection             NEXT or PREVIOUS
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Returncodes:       NONE
//------------------------------------------------------------------------------
// Function flow:     if old active alignment exists
//                       pick source of old active alignment
//                    else pick 1st segment
//                    find next anchor in source visdoc
//                    if found
//                      reset old active alignment
//                    set new active alignment to next anchor found
//------------------------------------------------------------------------------

VOID
ITMFuncNextAnchor
(
   PITMIDA    pITMIda,
   SHORT      sDirection
)
{
  PITMVISDOC    pstVisSrc;
  PITMVISDOC    pstVisTgt;
  ULONG         ulSegNum;
  ULONG         ulVisActSegOld, ulCursorSegStart;
  PTBSEGMENT    pSeg;
  PITMVISDOC    pVisDoc;


  pstVisSrc = &(pITMIda->stVisDocSrc);
  pstVisTgt = &(pITMIda->stVisDocTgt);
  /********************************************************************/
  /* activate current cursor alignment if cursor not in active        */
  /* alignment                                                        */
  /********************************************************************/
  pVisDoc = ACCESSWNDIDA(pITMIda->hwndFocus, PITMVISDOC );

  ulCursorSegStart = pVisDoc->pDoc->TBCursor.ulSegNum;

  if ( ulCursorSegStart != pVisDoc->ulVisActSeg )
  {
    ITMFuncSynch(pITMIda, pVisDoc);              // find alignment
  } /* endif */
  /********************************************************************/
  /* start searching at current active segment or segnum 1            */
  /********************************************************************/
  ulVisActSegOld = pstVisSrc->ulVisActSeg;
  if ( ulVisActSegOld )
  {
    ulSegNum = ulVisActSegOld + sDirection;
  }
  else
  {
    ulSegNum = 1;
  } /* endif */
  if ( sDirection == NEXT )
  {
    pSeg = EQFBGetVisSeg(pstVisSrc->pDoc, &ulSegNum); //assure seg is visible
  }
  else
  {
    pSeg = EQFBGetPrevVisSeg(pstVisSrc->pDoc, &ulSegNum); //assure seg is visible
  } /* endif */
  /********************************************************************/
  /* skip overcross anchors                                           */
  /********************************************************************/
  ulSegNum = FindNextAnchor ( pstVisSrc, ulSegNum, sDirection);
  if ( ulSegNum )
  {
    /******************************************************************/
    /* reset qstatus of old visactseg                                 */
    /******************************************************************/
    if ( ulVisActSegOld )
    {
      VisActReset(pstVisSrc, pstVisTgt);
    } /* endif */

    /******************************************************************/
    /* set new active segment                                         */
    /******************************************************************/
    pstVisSrc->ulVisActSeg = ulSegNum;
    pstVisTgt->ulVisActSeg = pstVisSrc->pulAnchor[ulSegNum];

    VisActNew (pstVisSrc, pstVisTgt);
    UPDSTATUSBAR( (PITMIDA)pVisDoc->pITMIda );
  } /* endif */

  if ( (!ulSegNum ) ||
      ( (sDirection == PREVIOUS) && (ulSegNum >= ulCursorSegStart)) ||
      ( (sDirection == NEXT) && (ulSegNum <= ulCursorSegStart)) )

  {
    ITMUtlError( (PITMIDA) pVisDoc->pITMIda, ITM_NOFOUND, MB_CANCEL, 0, NULL, EQF_WARNING);
  } /* endif */

  return;
} /* end of function VOID ITMFuncNextAnchor(PITMIDA) */

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     FindNextAnchor
//------------------------------------------------------------------------------
// Function call:     PITMVISDOC   pITMVisDoc,         VisDoc
//                    USHORT       ulSegNum,           start segment
//                    SHORT        sDirection          Next or Previous
//------------------------------------------------------------------------------
// Description:       find next anchor; skip overcross and deleted anchors
//------------------------------------------------------------------------------
// Parameters:        USHORT    ulSegNum  must be visible seg (EQFBGetVisSeg!)
//------------------------------------------------------------------------------
// Returncode type:   USHORT       ulSegNum   0 if not found
//------------------------------------------------------------------------------
// Function flow:     while not found and segnum in range
//                      if segnum is anchor and not overcross
//                        Found!
//                      else
//                        get next segnum in specified direction
//                    endwhile
//                    if not found return 0
//------------------------------------------------------------------------------

ULONG FindNextAnchor
(
  PITMVISDOC   pITMVisDoc,
  ULONG        ulSegNum,                         //start segnum, visible
  SHORT        sDirection
)
{
  BOOL       fFound = FALSE;

  while ( !fFound && (ulSegNum < (pITMVisDoc->pDoc->ulMaxSeg) )
            && (ulSegNum > 0) )
  {
    if ( pITMVisDoc->pulAnchor[ulSegNum]
         && !pITMVisDoc->pVisState[ulSegNum].OverCross )
    {
      fFound = TRUE;
    }
    else
    {
      ulSegNum += sDirection;
      /****************************************************************/
      /* not nec if only visible segs have anchors                    */
      /* this is not true! 13.3.94                                    */
      /****************************************************************/
//    pSeg = EQFBGetVisSeg(pITMVisDoc->pDoc, &ulSegNum);
    } /* endif */
  } /* endwhile */
  if ( !fFound )
  {
    ulSegNum = 0;
  } /* endif */

  /********************************************************************/
  /* ulSegNum may not be visible!                                     */
  /********************************************************************/
  return (ulSegNum);
} /* end of function FindNextAnchor */

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     VisActReset
//------------------------------------------------------------------------------
// Function call:     VisActReset(pstVisSrc, pstVisTgt)
//------------------------------------------------------------------------------
// Description:       reset qstatus of old visactive segment
//                    set ulVisActSegnum to 0
//------------------------------------------------------------------------------
// Parameters:        PITMVISDOC  pstVisSrc,
//                    PITMVISDOC  pstVisTgt
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Returncodes:       none
//------------------------------------------------------------------------------
// Function flow:     set qstatus of old visactive segment
//                    back to stored qstatus
//------------------------------------------------------------------------------
VOID
VisActReset
(
 PITMVISDOC  pstVisSrc,
 PITMVISDOC  pstVisTgt
)
{
  PTBSEGMENT   pSeg;
  USHORT       usResult;

  if ( pstVisSrc->ulVisActSeg )
  {
     pSeg = EQFBGetSegW(pstVisSrc->pDoc, pstVisSrc->ulVisActSeg);
     pSeg->qStatus = (USHORT)pstVisSrc->qVisActState;

     EQFBWorkSegOut(pstVisSrc->pDoc);
  } /* endif */

  if ( pstVisTgt->ulVisActSeg )
  {
    // issue warning determining if user wants to save or not
    usResult = MBID_NO;                    // ignore segment if nothing ch.

    if ( pstVisTgt->pDoc->EQFBFlags.workchng )
    {
       usResult = UtlError( TB_CHANGESEGMENT, MB_YESNOCANCEL,
                            0, NULL, EQF_QUERY);
    } /* endif */


    pSeg = EQFBGetSegW(pstVisTgt->pDoc, pstVisTgt->ulVisActSeg);

    // GQ: use the actual tbCursor segment, as it may be different from the ulVisActSeg!
//    pSeg = EQFBGetSegW(pstVisTgt->pDoc, pstVisTgt->pDoc->TBCursor.ulSegNum );

    switch ( usResult )
    {
      case MBID_YES:
         pSeg->qStatus = (USHORT)pstVisTgt->qVisActState;
         EQFBWorkSegOut(pstVisTgt->pDoc);
         break;
       case MBID_NO:                            // ignore changes
         pSeg->qStatus = (USHORT)pstVisTgt->qVisActState;
         pSeg->SegFlags.Typed = FALSE;
         pSeg->SegFlags.Copied = FALSE;
         pSeg->usModWords = 0;
         memset(&pSeg->CountFlag, 0, sizeof( pSeg->CountFlag));
         if ( pstVisTgt->pDoc->pSaveSegW )
         {
           pSeg->pDataW = pstVisTgt->pDoc->pSaveSegW;
         } /* endif */
         pstVisTgt->pDoc->EQFBFlags.workchng = FALSE;
         EQFBCompSeg( pSeg );
         pstVisTgt->pDoc->pSaveSegW = NULL;                 // reset save seg

         break;
      default:                                  // set back to active seg
//           EQFBGotoSeg( pDoc, pDoc->tbActSeg.ulSegNum, 0);
         break;

    } /* endswitch */
  } /* endif */

   pstVisTgt->ulVisActSeg = 0;
   pstVisSrc->ulVisActSeg = 0;

} /* end of function VisActReset */


//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     VisActNew
//------------------------------------------------------------------------------
// Function call:     _
//------------------------------------------------------------------------------
// Description:       activate segment in both docs
//------------------------------------------------------------------------------
// Parameters:        PITMVISDOC  pstVisSrc
//                    PITMVISDOC  pstVisTgt
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Returncodes:       _
//------------------------------------------------------------------------------
// Function flow:     activate segment in both docs
//------------------------------------------------------------------------------
static VOID
VisActNew
(
 PITMVISDOC  pstVisSrc,
 PITMVISDOC  pstVisTgt
)
{
  if (( pstVisSrc->ulVisActSeg) && (pstVisTgt->ulVisActSeg) )
  {
    VisActNewInDoc(pstVisSrc);
    VisActNewInDoc(pstVisTgt);
  } /* endif */

} /* end of function VisActNew   */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     VisActNewInDoc
//------------------------------------------------------------------------------
// Function call:     _
//------------------------------------------------------------------------------
// Description:       position at new active alignment
//------------------------------------------------------------------------------
// Parameters:        PITMVISDOC  pstVisDoc
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Returncodes:       _
//------------------------------------------------------------------------------
// Function flow:     set qstatus to VISACT in new active alignment
//                    store old qstatus
//                    position both documents at new active alignment
//------------------------------------------------------------------------------
static VOID
VisActNewInDoc
(
 PITMVISDOC  pstVisDoc
)
{
  PTBDOCUMENT  pDoc;
  PTBSEGMENT   pSeg;

  pDoc = pstVisDoc->pDoc;

  pSeg = EQFBGetSegW(pDoc, pstVisDoc->ulVisActSeg);
  pstVisDoc->qVisActState = (QSTATUS) pSeg->qStatus;
  pSeg->qStatus = QF_VISACT;
  memcpy (&(pDoc->tbActSeg), pSeg, sizeof(TBSEGMENT));
  pSeg->SegFlags.Current = TRUE;         // it's the active segment

  EQFBGotoSeg( pDoc, pstVisDoc->ulVisActSeg, 1);
  EQFBFuncLeft( pDoc );

   EQFBWorkSegIn( pDoc );                 // copy contents of current segment
   pDoc->fFuzzyCopied = FALSE;                    // no fuzzy match right now
   if ( pDoc->pUserSettings && pDoc->pUserSettings->fCrsInsert )
   {
      pDoc->EQFBFlags.inserting = TRUE;
      EQFBScreenCursorType( pDoc );
   } /* endif */
} /* end of function VisActNewInDoc   */

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     ITMFuncNextLone  (PITMIDA)
//------------------------------------------------------------------------------
// Function call:     _
//------------------------------------------------------------------------------
// Description:       Goto menu: Next unaligned segment
//------------------------------------------------------------------------------
// Parameters:        PITMIDA    pITMIda,
//                    SHORT      sDirection
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Returncodes:       _
//------------------------------------------------------------------------------
// Function flow:     find index in alignment table of current active alignment
//                    while not found and index in allowed range
//                      if alignment is 0:1 or 1:0
//                         if not still at current position
//                           found!
//                         else go ahead
//                      else
//                         store current alignment index as 'last'
//                         increase index in specified direction
//                      endif
//                    endwhile
//                    if found
//                      set new active segment to last alignment index found
//                      activate that last alignment
//------------------------------------------------------------------------------

VOID
ITMFuncNextLone
(
   PITMIDA    pITMIda,
   SHORT      sDirection
)
{
  PITMVISDOC    pstVisSrc;
  PITMVISDOC    pstVisTgt;
  BOOL          fFound;
  ULONG         ulIndex;
  ULONG         ulVisActSegOld;
  ULONG         ulLastAligned;
  ULONG         ulStartAligned;
  PITMVISDOC    pVisDoc;


  pstVisSrc = &(pITMIda->stVisDocSrc);
  pstVisTgt = &(pITMIda->stVisDocTgt);
  /********************************************************************/
  /* activate current cursor alignment                                */
  /********************************************************************/
  pVisDoc = ACCESSWNDIDA(pITMIda->hwndFocus, PITMVISDOC );

  ITMFuncSynch(pITMIda, pVisDoc);

  ulVisActSegOld = pstVisSrc->ulVisActSeg;
  ulStartAligned = pstVisSrc->pulNumAligned[ulVisActSegOld];
  /********************************************************************/
  /* find index of current alignment                                  */
  /* ulStartAligned = 0 if none found                                 */
  /********************************************************************/
  ulLastAligned = FindCurAlignIndex(pstVisSrc, ulVisActSegOld, sDirection);
  /********************************************************************/
  /* active alignment cannot be a 1:0 / 0:1 alignment ;               */
  /* hence remember the last alignment                                */
  /********************************************************************/
  if ( ulLastAligned )
  {
    ulIndex = ulLastAligned;

    fFound = FALSE;
    while ( !fFound && (ulIndex < (pITMIda->Aligned.ulUsed) )
              && (ulIndex > 0) )
    {
      if ( (pITMIda->Aligned.pbType[ulIndex] == NUL_ONE) ||
           (pITMIda->Aligned.pbType[ulIndex] == ONE_NUL) )
      {
         /**************************************************************/
         /* make sure that we do not stay at current alignment         */
         /**************************************************************/
         if ( ulLastAligned != ulStartAligned )   //force moving actseg
         {
           fFound = TRUE;
         }
         else
         {
           ulIndex += sDirection;
         } /* endif */
      }
      else
      {
        ulLastAligned = ulIndex;
        ulIndex += sDirection;
      } /* endif */
    } /* endwhile */
  }
  else
  {
    fFound = FALSE;
  } /* endif */
  if ( fFound  )
  {
    /******************************************************************/
    /* reset qstatus of old visactseg                                 */
    /******************************************************************/
    if ( ulVisActSegOld && ulLastAligned )
    {
      VisActReset(pstVisSrc, pstVisTgt);

      pstVisSrc->ulVisActSeg = pITMIda->Aligned.pulSrc[ulLastAligned];
      pstVisTgt->ulVisActSeg = pITMIda->Aligned.pulTgt1[ulLastAligned];

      VisActNew (pstVisSrc, pstVisTgt);   // position at new active align
      UPDSTATUSBAR( (PITMIDA)pVisDoc->pITMIda );
    } /* endif */
  }
  else
  {
    ITMUtlError( pITMIda, ITM_NOFOUND, MB_CANCEL, 0, NULL, EQF_WARNING);
  } /* endif */

} /* end of function VOID ITMFuncNextLone(PITMIDA) */



//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     VisDocFillIndex
//------------------------------------------------------------------------------
// Function call:     _
//------------------------------------------------------------------------------
// Description:       fill alignment index in both visdocs from
//                    new calculated alignment table
//                    in the range usFillStart to usFIllENd
//------------------------------------------------------------------------------
// Parameters:   PITMIDA     pITMIda,
//               USHORT      usFillStart,    start index in alignment struct
//               USHORT      usSrcStart,     start of inval srcalignments
//               USHORT      usTgtStart      start of inval tgtalignments
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Returncodes:       _
//------------------------------------------------------------------------------
// Function flow:     set old alignments to 0 in inval visdoc
//                    for usFIllStart til end of alignment table
//                       get src segnum and tgtsegnum of current align
//                       store the align index in the visdoc alignment
//                       array at position src and tgt
//                    endfor
//------------------------------------------------------------------------------

VOID
VisDocFillIndex
(
  PITMIDA     pITMIda,
  ULONG       ulFillStart,                   //start index in alignment struct
  ULONG       ulSrcStart,                    //start of inval srcalignments
  ULONG       ulTgtStart                     //start of inval tgtalignments
)
{
  PITMVISDOC   pstVisSrc;
  PITMVISDOC   pstVisTgt;
  ULONG        ulI;
  ULONG        ulSrc;
  ULONG        ulTgt1;
  PTBDOCUMENT  pSrcDoc ;
  PTBDOCUMENT  pTgtDoc ;

  pstVisSrc = &(pITMIda->stVisDocSrc);
  pstVisTgt = &(pITMIda->stVisDocTgt);
  /********************************************************************/
  /* erase old alignments                                             */
  /********************************************************************/
  pSrcDoc = pstVisSrc->pDoc;
  for ( ulI = ulSrcStart;ulI < pSrcDoc->ulMaxSeg ;ulI++ )
  {
    pstVisSrc->pulNumAligned[ulI] = 0;        //index in align.struct
  } /* endfor */
  pTgtDoc = pstVisTgt->pDoc;
  for ( ulI = ulTgtStart; ulI < pTgtDoc->ulMaxSeg ;ulI++ )
  {
    pstVisTgt->pulNumAligned[ulI] = 0;        //index in align.struct
  } /* endfor */

  /********************************************************************/
  /* set new alignments                                               */
  /* alignmentstruct contains only visible segments                   */
  /********************************************************************/
  for ( ulI = ulFillStart;ulI < pITMIda->Aligned.ulUsed ;ulI++ )
  {
    ulSrc = pITMIda->Aligned.pulSrc[ulI];
    ulTgt1 = pITMIda->Aligned.pulTgt1[ulI];
    if ( ulSrc && (ulSrc < pSrcDoc->ulMaxSeg) )
    {
      pstVisSrc->pulNumAligned[ulSrc] = ulI;        //index in align.struct
    } /* endif */
    if ( ulTgt1 && (ulTgt1 < pTgtDoc->ulMaxSeg) )
    {
      pstVisTgt->pulNumAligned[ulTgt1] = ulI;
    } /* endif */
  } /* endfor */
} /* end of function VisDocFillIndex */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     VisDocFillAnchor
//------------------------------------------------------------------------------
// Function call:     _
//------------------------------------------------------------------------------
// Description:       fill anchor index in both visdocs from the anchor
//                    list created in the 1st ITM alignment
//------------------------------------------------------------------------------
// Parameters:        PITMIDA     pITMIda
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Returncodes:       _
//------------------------------------------------------------------------------
// Function flow:     while not at end of anchor list
//                      get src and tgt segnum of next anchor
//                      assure src and tgt segnum are visible
//                      fill anchor array in visdoc with corresponding
//                        anchor segnum
//------------------------------------------------------------------------------
static
VOID
VisDocFillAnchor
(
  PITMIDA     pITMIda
)
{
  PITMVISDOC   pstVisSrc;
  PITMVISDOC   pstVisTgt;
  ULONG        ulIndex;
  ULONG        ulSrc;
  ULONG        ulTgt;
  ULONG        ulUsed;
  PULONG       pSrcAnchor;
  PULONG       pTgtAnchor;
  PTBDOCUMENT  pSrcDoc;
  PTBDOCUMENT  pTgtDoc;
  PTBSEGMENT   pSeg;
  ULONG        ulSrcMaxSeg;
  ULONG        ulTgtMaxSeg;
  BOOL         fOK = TRUE;

  ulUsed = pITMIda->itmSrcNop.ulUsed;             //ulUsed equal for Src/Tgt

  pstVisSrc = &(pITMIda->stVisDocSrc);
  pstVisTgt = &(pITMIda->stVisDocTgt);
  pSrcDoc = pstVisSrc->pDoc;
  pTgtDoc = pstVisTgt->pDoc;
  ulSrcMaxSeg = pSrcDoc->ulMaxSeg;
  ulTgtMaxSeg = pTgtDoc->ulMaxSeg;

  pSrcAnchor = &(pITMIda->itmSrcNop.pulSegs[0]);
  pTgtAnchor = &(pITMIda->itmTgtNop.pulSegs[0]);

  for ( ulIndex=1;ulIndex < ulUsed ;ulIndex++)                     // entry 0 is dummy
  {
    ulSrc = pSrcAnchor[ulIndex];
    ulTgt = pTgtAnchor[ulIndex];

    if ( (ulSrc < ulSrcMaxSeg) && (ulTgt < ulTgtMaxSeg) )
    {
      pSeg = EQFBGetPrevVisSeg(pSrcDoc, &ulSrc);
      pSeg = EQFBGetPrevVisSeg(pTgtDoc, &ulTgt);
      fOK = TRUE;
      /******************************************************************/
      /* if usSrc or usTgt is 0:1 / 1:0 due to parsing the alignment    */
      /* struct (ParseAlignStruct), forget the anchor                   */
      /* if usSrc is anchored with usTgt, but aligned otherwise,        */
      /* forget the anchor!                                             */
      /******************************************************************/
      if (  pstVisSrc->pulNumAligned[ulSrc]
           !=  pstVisTgt->pulNumAligned[ulTgt] )
      {
        fOK = FALSE;
      }
    } /* endif */
    /******************************************************************/
    /* if usSrc and usTgt are visible, set this anchor                */
    /* otherwise set it to joinstart segment                          */
    /******************************************************************/
    if (fOK && (pstVisSrc->pulAnchor[ulSrc] == 0 ) &&
         (pstVisTgt->pulAnchor[ulTgt] == 0 )   )
    {
      pstVisSrc->pulAnchor[ulSrc] = ulTgt;           //usSrc is anchor with usTgt
      pstVisTgt->pulAnchor[ulTgt] = ulSrc;
    } /* endif */
  } /* endfor */
} /* end of function VisDocFillAnchor */

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     FindCurAlignIndex
//------------------------------------------------------------------------------
// Function call:     _
//------------------------------------------------------------------------------
// Description:       find current alignment
//                    starting from a given segnum find next /current/previous
//                    entry in alignment table
//------------------------------------------------------------------------------
// Parameters:        PITMVISDOC   pstVisDoc,        VisDOc structure
//                    USHORT       usStartSeg        starting segment
//                    SHORT        sDirection        Direction where to search
//------------------------------------------------------------------------------
// Returncode type:   USHORT
//------------------------------------------------------------------------------
// Returncodes:       index in alignment table searched for
//------------------------------------------------------------------------------
// Function flow:     while index not found and segnum in range
//                         pick alignment index of next segnum
//                    if not found
//                      search in other direction
//                    goto next alignment index in specified direction
//                    loop in alignment table til next alignment
//                      which is not 1:0 or 0:1
//                    if no alignment found, return 0
//------------------------------------------------------------------------------

ULONG
FindCurAlignIndex
(
  PITMVISDOC   pstVisDoc,
  ULONG        ulStartSeg,
  SHORT        sDirection
)
{
  ULONG        ulIndex;
  ULONG        ulSegNum;
  ULONG        ulMaxSeg;
  PITMIDA      pITMIda;
  SHORT        sCurDirection;
  BOOL         fFound;

  /********************************************************************/
  /* find next segment with alignment index != 0                      */
  /********************************************************************/
  ulSegNum = ulStartSeg;
  ulIndex = pstVisDoc->pulNumAligned[ulSegNum];
  ulMaxSeg = pstVisDoc->pDoc->ulMaxSeg;
  sCurDirection = sDirection;
  if ( sCurDirection == CURRENT )
  {
    sCurDirection = NEXT;
  } /* endif */
  while ( !ulIndex && (ulSegNum < ulMaxSeg) && ulSegNum )
  {
    ulSegNum += sCurDirection;
    ulIndex = pstVisDoc->pulNumAligned[ulSegNum];
  } /* endwhile */
  /********************************************************************/
  /* if at boundery, search in other direction                        */
  /********************************************************************/
  if ( !ulIndex )
  {
    ulSegNum = ulStartSeg;
    while ( !ulIndex && ulSegNum && (ulSegNum < ulMaxSeg) )
    {
      ulSegNum -= sCurDirection;
      ulIndex = pstVisDoc->pulNumAligned[ulSegNum];
    } /* endwhile */
  } /* endif */
  /********************************************************************/
  /* get next alignment in specified direction                        */
  /********************************************************************/
  ulIndex += sDirection;

  /********************************************************************/
  /* skip all 1:0 and 0:1 alignments in alignment struct              */
  /********************************************************************/
  fFound = FALSE;
  pITMIda = (PITMIDA) pstVisDoc->pITMIda;
  while ( !fFound && (ulIndex < (pITMIda->Aligned.ulUsed) )
            && (ulIndex > 0) )
  {
    if ( (pITMIda->Aligned.pbType[ulIndex] == NUL_ONE) ||
         (pITMIda->Aligned.pbType[ulIndex] == ONE_NUL) )
    {
      ulIndex += sCurDirection;
    }
    else
    {
      fFound = TRUE;
    } /* endif */
  } /* endwhile */
  /********************************************************************/
  /* if at boundery, skip 1:0 / 0:1 in other direction                */
  /********************************************************************/
  if ( !fFound )
  {
     ulIndex -= sCurDirection;
     while ( !fFound && (ulIndex < (pITMIda->Aligned.ulUsed) )
               && (ulIndex > 0) )
     {
       if ( (pITMIda->Aligned.pbType[ulIndex] == NUL_ONE) ||
            (pITMIda->Aligned.pbType[ulIndex] == ONE_NUL) )
       {
         ulIndex -= sCurDirection;
       }
       else
       {
         fFound = TRUE;
       } /* endif */
     } /* endwhile */
  } /* endif */

  if ( !fFound  )
  {
    ulIndex = 0;
  } /* endif */
  return (ulIndex);
} /* end of function FindCurAlignIndex */

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     ITMFuncSynch
//------------------------------------------------------------------------------
// Function call:     _
//------------------------------------------------------------------------------
// Description:       synchronize to cursor segnum
//                    active alignment is set to cursor segnum
//                    or next to it
//------------------------------------------------------------------------------
// Parameters:        PITMIDA      pITMIda,
//                    PITMVISDOC   pstVisDoc
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Returncodes:       _
//------------------------------------------------------------------------------
// Function flow:     set active alignment to cursor segment
//------------------------------------------------------------------------------
VOID
ITMFuncSynch
(
  PITMIDA      pITMIda,
  PITMVISDOC   pstVisDoc
)
{
  ULONG         ulSegNum;
  PTBDOCUMENT   pDoc;

  pDoc = pstVisDoc->pDoc;
  ulSegNum = pDoc->TBCursor.ulSegNum;            // get cursor segnum
                                                 // find alignment
  VisActivateSeg(pITMIda, pstVisDoc , CURRENT, TRUE, ulSegNum);

} /* end of function ITMFuncSynch */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     SetToggleCol
//------------------------------------------------------------------------------
// Function call:     _
//------------------------------------------------------------------------------
// Description:       set qstatus
//------------------------------------------------------------------------------
// Parameters:        PUSHORT  pusColToggle
//                    QSTATUS  qStat1,
//                    QSTATUS  qStat2,
//                    QSTATUS  qStat3
//------------------------------------------------------------------------------
// Returncode type:   QSTATUS
//------------------------------------------------------------------------------
// Returncodes:       _
//------------------------------------------------------------------------------
// Function flow:     return QStatus according to color toggle state
//                    (called from VisSetQStatus)
//                    set color toggle to next
//------------------------------------------------------------------------------
static
QSTATUS
SetToggleCol
(
  PUSHORT  pusColToggle,
  QSTATUS  qStat1,
  QSTATUS  qStat2,
  QSTATUS  qStat3

)
{
  QSTATUS  qSelectStatus;

    switch ( *pusColToggle )
    {
      case  COL1:
        qSelectStatus = qStat1;
        *pusColToggle = COL2;
        break;
      case  COL2:
        qSelectStatus = qStat2;
        *pusColToggle = COL3;
        break;
      default :
        qSelectStatus = qStat3;
        *pusColToggle = COL1;
        break;
    } /* endswitch */

   return (qSelectStatus);
} /* end of function SetToggleCol */

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     ITMVISWNDPROC
//------------------------------------------------------------------------------
// Function call:     ITMVisWndProc(hwnd, msg, mp1, mp2)
//------------------------------------------------------------------------------
// Description:       wndproc for visdoc window
//------------------------------------------------------------------------------
// Parameters:        HWND hwnd,
//                    USHORT msg
//                    WPARAM mp1
//                    LPARAM mp2
//------------------------------------------------------------------------------
// Returncode type:   MRESULT APIENTRY
//------------------------------------------------------------------------------
// Returncodes:       _
//------------------------------------------------------------------------------
// Function flow:     switch (msg)
//                      case WM_CREATE:
//                      case WM_ERASEBACKGROUND:
//                      default: WinDefWIndowProc
//------------------------------------------------------------------------------
MRESULT APIENTRY
ITMVISWNDPROC
(
  HWND hwnd,
  WINMSG msg,
  WPARAM mp1,
  LPARAM mp2
)
{
  MRESULT mResult = FALSE;           // window proc return value

  switch( msg )
  {
      case WM_CREATE:
//      VisualStart ( WinQueryAnchorBlock(hwnd), hwnd, pITMIda );
        break;
      case  WM_PAINT:
        {
          PAINTSTRUCT ps;
          HDC    hdc;
          RECT   rect;

          hdc = BeginPaint(hwnd, &ps );
          GetClientRect( hwnd, &rect );
          ERASERECT( hdc, &rect, CLR_PALEGRAY );
          EndPaint(hwnd, &ps);
        }
        break;
      case WM_SIZE:
        {
          SHORT sHeigth = HIWORD(mp2);   // new height of window
          SHORT sSize   = LOWORD(mp2);   // new width size of window

          /**************************************************************/
          /* Windows only: resize/rearrange controls only for normal    */
          /* size requests                                              */
          /**************************************************************/
          if ( (mp1 == SIZENORMAL) || (mp1 == SIZEFULLSCREEN) )
          {
            PITMIDA pITMIda = ACCESSWNDIDA( hwnd, PITMIDA );

            if ( pITMIda )
            {
              ITMSetWindowPos( pITMIda, sSize, sHeigth );
            } /* endif */
          } /* endif */
        }
        break;

      default:
        mResult = WinDefWindowProc ( hwnd, msg, mp1, mp2 );
        break;

  } /* switch */

  return mResult ;
} /* end of function ITMVISWNDPROC */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     PrepStyles
//------------------------------------------------------------------------------
// Function call:     PrepStyles(pDoc)
//------------------------------------------------------------------------------
// Description:       prepare display styles and other editing stuff
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT pDoc
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Returncodes:       none
//------------------------------------------------------------------------------
// Function flow:     set initial values for the doc field (cursor row/col...)
//                    get user settings (profile settings of editor)
//                    get shrink and compact abbreviations
//------------------------------------------------------------------------------
static VOID
PrepStyles
(
 PTBDOCUMENT  pDoc
)
{
  LONG  lLen;
  ULONG   ulOEMCP;

   /* Set initial values to the document fields */
   pDoc->lCursorRow  = 0;
   pDoc->lCursorCol  = 0;
   pDoc->lDBCSCursorCol = 0;
   pDoc->lSideScroll = 0;
   pDoc->flags.changed = FALSE;
   pDoc->Redraw    = REDRAW_ALL;
   pDoc->EQFBFlags.Reflow = TRUE;             //allow reflow (insert/split)
   pDoc->usTagEnd = '.';                      //should be set as TAG_END_CHAR
   /*******************************************************************/
   /* get user settings from static field EQFBUserOpt to pDoc->       */
   /* pUserSettings                                                   */
   /*******************************************************************/
   EQFBGetUserSettings(pDoc);
   ulOEMCP = GetLangOEMCP(NULL);
   ASCII2Unicode(pDoc->pUserSettings->szInTagAbbr, pDoc->szInTagAbbrW, ulOEMCP);  //set abbreviations for SHRINK
   ASCII2Unicode(pDoc->pUserSettings->szOutTagAbbr, pDoc->szOutTagAbbrW, ulOEMCP);   // and compact
   ASCII2Unicode(pDoc->pUserSettings->szInTagAbbr, pDoc->szInTagLFAbbrW, ulOEMCP);  //set abbreviations for SHRINK
   ASCII2Unicode(pDoc->pUserSettings->szOutTagAbbr, pDoc->szOutTagLFAbbrW, ulOEMCP);           // and compact
   lLen = UTF16strlenCHAR(pDoc->szInTagLFAbbrW);
   pDoc->szInTagLFAbbrW[lLen] = LF;
   pDoc->szInTagLFAbbrW[lLen+1] = EOS;

   lLen = UTF16strlenCHAR(pDoc->szOutTagLFAbbrW);
   pDoc->szOutTagLFAbbrW[lLen] = LF;
   pDoc->szOutTagLFAbbrW[lLen+1] = EOS;

   pDoc->fAutoLineWrap = TRUE;           // initially set autolinewrap on...

   pDoc->bOperatingSystem = (BYTE) UtlGetOperatingSystemInfo();
   return;
} /* end of function PrepStyles */

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     ITMFuncStyle(PITMIDA, USHORT)
//------------------------------------------------------------------------------
// Function call:     _
//------------------------------------------------------------------------------
// Description:       user changes display style(protect/shrink/compact)
//------------------------------------------------------------------------------
// Parameters:        PITMIDA  pITMIda,
//                    USHORT   usStyle
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Returncodes:       _
//------------------------------------------------------------------------------
// Function flow:     set hour glass
//                    reset active segment
//                    delete all lf's inserted for parallelization
//                    change style in src and tgt doc window
//                    insert lf's again if parallel is on
//                    recalc QStatus of visualization
//                    set new active segments
//                    set mouse ptr again
//                    update status bar
//------------------------------------------------------------------------------
VOID
ITMFuncStyle
(
 PITMIDA  pITMIda,
 USHORT   usStyle
)
{
  PTBDOCUMENT pDoc;
  ULONG       ulVisActSegSrc;
  PITMVISDOC  pstVisSrc;
  PITMVISDOC  pstVisTgt;
  ULONG       ulVisActSegTgt;
  PITMVISDOC  pVisDoc;

  /*******************************************************************/
  /* set hour glass                                                  */
  /*******************************************************************/
  SETCURSOR( SPTR_WAIT );

  pstVisSrc = &(pITMIda->stVisDocSrc);
  pstVisTgt = &(pITMIda->stVisDocTgt);
  ulVisActSegSrc = pstVisSrc->ulVisActSeg;
  ulVisActSegTgt = pstVisTgt->ulVisActSeg;
  /******************************************************************/
  /* reset qstatus of old visactseg                                 */
  /******************************************************************/
  if ( ulVisActSegSrc )
  {
    VisActReset(pstVisSrc, pstVisTgt);
  } /* endif */
  /********************************************************************/
  /* delete lf's inserted for parallelism                             */
  /********************************************************************/
  ITMDelLF (pITMIda, 1, pITMIda->TBSourceDoc.ulMaxSeg ,
                     1, pITMIda->TBTargetDoc.ulMaxSeg );

  /********************************************************************/
  /* set pDoc->pTBSeg to avoid trap in EQFBChangeStyle                */
  /********************************************************************/
  pDoc = &(pITMIda->TBSourceDoc);
  if ( ulVisActSegSrc )
  {
    pDoc->pTBSeg = EQFBGetSegW(pDoc, ulVisActSegSrc);
  }
  else
  {
    pDoc->pTBSeg = EQFBGetSegW(pDoc, 1);
  } /* endif */
  EQFBChangeStyle( pDoc,  usStyle);
  pDoc = &(pITMIda->TBTargetDoc);
  if ( ulVisActSegTgt )
  {
    pDoc->pTBSeg = EQFBGetSegW(pDoc, ulVisActSegTgt);
  }
  else
  {
    pDoc->pTBSeg = EQFBGetSegW(pDoc, 1);
  } /* endif */
  EQFBChangeStyle( pDoc,  usStyle);

  ITMAdjustLF ( pITMIda,
                1, (pITMIda->TBSourceDoc.ulMaxSeg - 1),
                1, (pITMIda->TBTargetDoc.ulMaxSeg - 1));

  VisSetQStatus( &(pITMIda->stVisDocSrc), 1, 1);
  VisSetQStatus( &(pITMIda->stVisDocTgt), 1, 1);

  /******************************************************************/
  /* set new active segment                                         */
  /* what happens if anchor is NOP and now compact?                   */
  /******************************************************************/
  pstVisSrc->ulVisActSeg = ulVisActSegSrc;
  pstVisTgt->ulVisActSeg = ulVisActSegTgt;

  VisActNew (pstVisSrc, pstVisTgt);
  SETCURSOR( SPTR_ARROW );

  pstVisSrc->pDoc->Redraw |= REDRAW_ALL;
  pstVisTgt->pDoc->Redraw |= REDRAW_ALL;

  pVisDoc = ACCESSWNDIDA(pITMIda->hwndFocus, PITMVISDOC );

  UPDSTATUSBAR( (PITMIDA)pVisDoc->pITMIda );

  return;
} /* end of function ITMFuncStyle(PITMIDA, USHORT) */

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     ITMFuncContinue
//------------------------------------------------------------------------------
// Function call:     ITMFuncContinue(pITMIda)
//------------------------------------------------------------------------------
// Description:       user pressed save&continue
//                    alignment is saved to continuation file (== alifile)
//------------------------------------------------------------------------------
// Parameters:        PITMIDA  pITMIda
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
// Returncodes:       sRC   0 success
//                     other  error
//------------------------------------------------------------------------------
// Function flow:     if user selected 'Prepare' and files are already prepared
//                       do nothing
//                    else
//                      if filepair has already a continuation file
//                        delete old continuation file
//                      if user selected 'Prepare'
//                        init visdoc structure
//                      write alifile ( == continuation file)
//                      if user selected 'Prepare'
//                        free visdoc struct arrays
//                      else
//                        message 'successfully saved for later continuation'
//                      endif
//                    endif
//------------------------------------------------------------------------------
SHORT
ITMFuncContinue
(
  PITMIDA   pITMIda
)
{
  SHORT     sRc = 0;                   //no error occurred
  BOOL      fOK;
  ULONG     ulSrcActSeg, ulTgtActSeg;
  PTBSEGMENT  pSeg;
  /********************************************************************/
  /* save temparily for later continuation                            */
  /* then display completion msg                                      */
  /* if already Prepared, do nothing                                  */
  /********************************************************************/
  if ( !(pITMIda->fPrepare && pITMIda->usNumPrepared ))
  {
    /*******************************************************************/
    /* set hour glass                                                  */
    /*******************************************************************/
    SETCURSOR( SPTR_WAIT );

    if ( pITMIda->usNumPrepared )
    {
      EQFITMDelAli ( pITMIda );
    }
    if ( pITMIda->fPrepare )
    {
      fOK = VisDocInit ( pITMIda );
    }
    else
    {
      /****************************************************************/
      /* in visualization currently                                   */
      /* delete lf's inserted for parallelism                         */
      /****************************************************************/
      ITMDelLF (pITMIda, 1, pITMIda->TBSourceDoc.ulMaxSeg ,
                         1, pITMIda->TBTargetDoc.ulMaxSeg );

    } /* endif */

    /********************************************************************/
    /* guarantee that no seg has qStatus == QF_VISACT                   */
    /********************************************************************/

    ulSrcActSeg = pITMIda->stVisDocSrc.ulVisActSeg;
    ulTgtActSeg = pITMIda->stVisDocTgt.ulVisActSeg;
    VisActReset(&(pITMIda->stVisDocSrc), &(pITMIda->stVisDocTgt));

    /****************************************************************/
    /* save docs to have joined segments later on also              */
    /****************************************************************/
    pITMIda->TBSourceDoc.flags.changed = TRUE;
    VisDocSave( &(pITMIda->TBSourceDoc), pITMIda->chSegSourceFile,
                &(pITMIda->stVisDocSrc ) );
    pITMIda->TBTargetDoc.flags.changed = TRUE;
    VisDocSave( &(pITMIda->TBTargetDoc), pITMIda->chSegTargetFile,
                &(pITMIda->stVisDocTgt ) );


    sRc = EQFITMWriteAli ( pITMIda );

    if ( pITMIda->fPrepare )
    {
      VisDocFree ( &(pITMIda->stVisDocSrc ));
      VisDocFree ( &(pITMIda->stVisDocTgt ));
    }
    else
    {
      /****************************************************************/
      /* if parallel is on, insert lf's again                         */
      /****************************************************************/
      ITMAdjustLF ( pITMIda,
                   1, (pITMIda->TBSourceDoc.ulMaxSeg - 1),
                   1, (pITMIda->TBTargetDoc.ulMaxSeg - 1));
      /********************************************************************/
      /* guarantee that old actseg is again actseg                        */
      /********************************************************************/
      if ( ulSrcActSeg && ulTgtActSeg )
      {
        pITMIda->stVisDocSrc.ulVisActSeg = ulSrcActSeg;
        pITMIda->stVisDocTgt.ulVisActSeg = ulTgtActSeg;
  //      VisActNew(&(pITMIda->stVisDocSrc), &(pITMIda->stVisDocTgt));
        pSeg = EQFBGetSegW(&(pITMIda->TBSourceDoc), ulSrcActSeg);
        pITMIda->stVisDocSrc.qVisActState = (QSTATUS) pSeg->qStatus;
        pSeg->qStatus = QF_VISACT;

        pSeg = EQFBGetSegW(&(pITMIda->TBTargetDoc), ulTgtActSeg);
        pITMIda->stVisDocTgt.qVisActState = (QSTATUS) pSeg->qStatus;
        pSeg->qStatus = QF_VISACT;
      } /* endif */

//      pITMIda->TBSourceDoc.Redraw |= REDRAW_ALL;
//      pITMIda->TBTargetDoc.Redraw |= REDRAW_ALL;
      /****************************************************************/
      /* display message not if a couple of files are prepared        */
      /* msg is displayed at end only then                            */
      /****************************************************************/
      if ( !sRc )
      {
        ITMUtlError( pITMIda, ITM_SAVETOALI, MB_OK, 0, NULL, EQF_INFO);
      } /* endif */

    } /* endif */

    SETCURSOR( SPTR_ARROW );
  } /* endif */
  return(sRc);
} /* end of function ITMFuncContinue */

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     ITMFuncQuit
//------------------------------------------------------------------------------
// Function call:     ITMFuncQuit(pITMIda, hwnd, hab)
//------------------------------------------------------------------------------
// Description:       quit the visualization
//                    (e.g. user selected Quit)
//------------------------------------------------------------------------------
// Parameters:        PITMIDA   pITMIda
//                    HWND      hwnd,
//                    HAB       hab
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Returncodes:       none
//------------------------------------------------------------------------------
// Function flow:     if user changed alignment
//                      ask user whether to goon
//                    if user wants to quit
//                       free allocated areas
//                       close both visdoc windows
//------------------------------------------------------------------------------
VOID
ITMFuncQuit
(
   PITMIDA   pITMIda,
   HWND      hwnd,
   HAB       hab
)
{
  USHORT   usMBID = MBID_YES;

  hab;
  if ( pITMIda->stVisDocSrc.fChanged || pITMIda->stVisDocTgt.fChanged )
  {
    usMBID = ITMUtlError( pITMIda,ITM_QUITCHANGES, MB_YESNO, 0, NULL, EQF_ERROR);
  } /* endif */
  if ( usMBID == MBID_YES )
  {
    VisActReset(&(pITMIda->stVisDocSrc),&(pITMIda->stVisDocTgt));
    /****************************************************************/
    /* free allocated areas                                         */
    /****************************************************************/
    FreeAll (pITMIda);

    WinPostMsg(pITMIda->TBSourceDoc.hwndFrame, WM_CLOSE,
                  MP1FROMSHORT(TRUE), NULL);
    WinPostMsg(pITMIda->TBTargetDoc.hwndFrame, WM_CLOSE,
                  MP1FROMSHORT(TRUE), NULL);
    /****************************************************************/
    /* start timer again                                            */
    /****************************************************************/
    pITMIda->usStatus = ITM_STAT_ENDVISUAL;
    if (!pITMIda->fTimer )
    {
      if (WinStartTimer (hab, hwnd, ITM_TIMER_ID, ITM_TIMER_VAL) )
      {
         pITMIda->fTimer = TRUE;
      } /* endif */
    } /* endif */
  }
  return;
} /* end of function ITMFuncQuit */

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     ITMFuncSave
//------------------------------------------------------------------------------
// Function call:     _
//------------------------------------------------------------------------------
// Description:       save alignments to the translation memory
//------------------------------------------------------------------------------
// Parameters:        PITMIDA   pITMIda,
//                    HWND      hwnd,
//                    HAB       hab
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Returncodes:       none
//------------------------------------------------------------------------------
// Function flow:     delete lf's inserted for parallel
//                    delete soft lf's inserted for autolinewrap
//                    write aligned segments in the memory
//                    close the visdoc windows
//                    start timer, set status to ITM_STAT_ENDVISUAL
//------------------------------------------------------------------------------
VOID
ITMFuncSave
(
   PITMIDA   pITMIda,
   HWND      hwnd,
   HAB       hab
)
{
   BOOL    fOK = TRUE;
   hab;
   /*******************************************************************/
   /* set hour glass                                                  */
   /*******************************************************************/
   SETCURSOR( SPTR_WAIT );
   /****************************************************************/
   /* in visualization currently                                   */
   /* delete lf's inserted for parallelism                         */
   /****************************************************************/
   ITMDelLF (pITMIda, 1, pITMIda->TBSourceDoc.ulMaxSeg ,
                      1, pITMIda->TBTargetDoc.ulMaxSeg );

   VisActReset(&(pITMIda->stVisDocSrc),&(pITMIda->stVisDocTgt));
   /****************************************************************/
   /* write to memory and free allocated areas                     */
   /****************************************************************/
   WinShowWindow( pITMIda->hProcWnd, TRUE );  // show slider window
   // alloc pBufCB if not yet done - nec. if SGMLMem is written of ali-file!

   if ( !pITMIda->fKill && pITMIda->fSGMLITM && !pITMIda->pBufCB )
   {
     fOK = ! UtlBufOpen( &pITMIda->pBufCB, pITMIda->chSGMLMem,
                         ITM_BUFSIZE, FILE_CREATE, TRUE );
     if ( fOK && (pITMIda->usSGMLFormat == SGMLFORMAT_UNICODE))
     {
       // write Unicode prefix to outfile
       fOK = !UtlBufWrite( pITMIda->pBufCB, UNICODEFILEPREFIX,
                              (SHORT)strlen(UNICODEFILEPREFIX), TRUE );
     } /* endif */
     if ( fOK )
     {
       fOK = ! UtlBufWriteConv( pITMIda->pBufCB, NTM_BEGIN_TAGW,
                            (USHORT)UTF16strlenBYTE( NTM_BEGIN_TAGW ),
                                TRUE, pITMIda->usSGMLFormat,
                                pITMIda->ulSGMLFormatCP,
                                pITMIda->ulAnsiCP);
     }
     if ( !fOK )
     {
       pITMIda->fKill = TRUE;
     } /* endif */
   } /* endif */
   ITMFuncMemSave(pITMIda, TRUE );
   WinShowWindow( pITMIda->hProcWnd, FALSE ); // hide it again...

   SETCURSOR( SPTR_ARROW );

   WinPostMsg(pITMIda->TBSourceDoc.hwndFrame, WM_CLOSE,
                 MP1FROMSHORT(TRUE), NULL);
   WinPostMsg(pITMIda->TBTargetDoc.hwndFrame, WM_CLOSE,
                 MP1FROMSHORT(TRUE), NULL);
   /****************************************************************/
   /* start timer again                                            */
   /****************************************************************/
   WinStartTimer (hab, hwnd, ITM_TIMER_ID, ITM_TIMER_VAL);
   pITMIda->usStatus = ITM_STAT_ENDVISUAL;
   return;

} /* end of function ITMFuncSave */

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     ITMSTATUSBARWNDPROC
//------------------------------------------------------------------------------
// Function call:     _
//------------------------------------------------------------------------------
// Description:       _
//------------------------------------------------------------------------------
// Parameters:        HWND hwnd,
//                    USHORT msg
//                    WPARAM mp1
//                    LPARAM mp2
//------------------------------------------------------------------------------
// Returncode type:   MRESULT APIENTRY
//------------------------------------------------------------------------------
// Returncodes:       _
//------------------------------------------------------------------------------
// Function flow:     position statusbar and create it
//                    and fill statusbar during WM_PAINT
//------------------------------------------------------------------------------
MRESULT APIENTRY
ITMSTATUSBARWNDPROC
(
  HWND hwnd,
  WINMSG msg,
  WPARAM mp1,
  LPARAM mp2
)
{
  MRESULT mResult = FALSE;           // window proc return value

  switch( msg )
  {
      case WM_CREATE:
        break;

      case  WM_PAINT:
        {
          PAINTSTRUCT ps;
          HDC    hdc;

          hdc = BeginPaint(hwnd, &ps );
          ITMFillStatusBar( hwnd, hdc  );
          EndPaint(hwnd, &ps);
        }
        break;
      default:
        mResult = WinDefWindowProc ( hwnd, msg, mp1, mp2 );
        break;

  } /* switch */

  return mResult ;
} /* end of function ITMSTATUSBARWNDPROC */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     ITMFillStatusBar
//------------------------------------------------------------------------------
// Function call:     ITMFillStatusBar(hwnd, hps)
//------------------------------------------------------------------------------
// Description:       fill and draw status bar
//------------------------------------------------------------------------------
// Parameters:        HWND  hwnd
//                    HPS hps
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Returncodes:       none
//------------------------------------------------------------------------------
// Function flow:     display:
//                     - number of srcsegments to be translated
//                     - number of unaligned src segments
//                     - number of crossed-out src segments
//                     - source or target, whatever is active
//                     - kind of active alignment (1:1, 1:2, 2:1, 2:2)
//                     - number of crossed-out tgt segments
//                     - number of unaligned tgt segments
//                     - number of tgtsegments to be translated
//------------------------------------------------------------------------------
static VOID
ITMFillStatusBar
(
  HWND  hwnd,                          // window handle
  HPS hps                              // presentation space
)
{
  PSTATUSINFO pStatusLeft, pStatusRight;

  RECT        rectl;
  PITMIDA     pITMIda = ACCESSWNDIDA( hwnd, PITMIDA );
  SWP         swp;
  USHORT      usOneUnit;
  USHORT      ulFilledLeft; // statusbar filled up...
  PITMVISDOC  pVisDoc,pstVisSrc, pstVisTgt;
  PTBSEGMENT  pSrcSeg, pTgtSeg;

  BOOL        fSrcAct;
  USHORT      usCY = (USHORT)WinQuerySysValue( HWND_DESKTOP, SV_CYMENU );
  PTEXTTYPETABLE pTextType;
  CHAR        chBuffer[MAX_STATUSLEN + 100];
  DWORD      dwRGB_HIGHLIGHTTEXT = CLR_BLACK;   // as foreground of ACTSEG
  DWORD      dwRGB_HIGHLIGHT = CLR_YELLOW;      // as background of ACTSEG
  DWORD      dwRGB_ERASERECTCOLOR = CLR_PALEGRAY;
  DWORD      dwRGB_WINDOWTEXT = CLR_BLACK;
  DWORD      dwRGB_CROSSEDOUTTEXT = CLR_WHITE;  // foreground crossed-out segments
  DWORD      dwRGB_UNALIGNED = CLR_RED; // background for unaligned segments
  DWORD      dwRGB_WINDOW = GetSysColor(COLOR_WINDOW);
  DWORD      dwRGB_IRREGULAR = dwRGB_WINDOWTEXT;
  BOOL       fIsHighContrast = UtlIsHighContrast();
  int        iBRUSH = BLACK_BRUSH;

  pVisDoc = ACCESSWNDIDA(pITMIda->hwndFocus, PITMVISDOC );
  if ( pVisDoc )
  {
    fSrcAct = ((pVisDoc->pDoc)->docType == VISSRC_DOC) ? TRUE : FALSE;
  } /* endif */

  WinQueryWindowPos( hwnd, &swp );
  usOneUnit = swp.cx /
             (ITM_CROSSEDOUT+ITM_UNALIGNED+ITM_TOTAL+ITM_ALIGN+ITM_IRREGULAR);

  pStatusLeft = &pITMIda->stSrcInfo;
  pStatusRight = &pITMIda->stTgtInfo;

  rectl.top = 0;

  if (fIsHighContrast )
  {
	  iBRUSH = GRAY_BRUSH;
	 dwRGB_HIGHLIGHTTEXT = GetSysColor(COLOR_HIGHLIGHTTEXT);
     dwRGB_HIGHLIGHT = GetSysColor(COLOR_HIGHLIGHT);
 //    dwRGB_IGNORED = GetSysColor(COLOR_BTNTEXT);
     dwRGB_IRREGULAR = GetSysColor(COLOR_CAPTIONTEXT );
     dwRGB_CROSSEDOUTTEXT = GetSysColor(COLOR_INACTIVECAPTION);
    // dwRGB_TEXT4 = GetSysColor(COLOR_SCROLLBAR);
    // dwRGB_CROSSEDOUTTEXT = GetSysColor(COLOR_MENUTEXT);  // is black
     dwRGB_UNALIGNED = GetSysColor(COLOR_INACTIVECAPTION);
    // dwRGB_TEXT7 = GetSysColor(COLOR_BTNHIGHLIGHT);

     dwRGB_ERASERECTCOLOR = GetSysColor(COLOR_WINDOW);
     dwRGB_WINDOWTEXT = GetSysColor(COLOR_WINDOWTEXT);
     dwRGB_WINDOW = GetSysColor(COLOR_WINDOW);
  }

  /********************************************************************/
  /* display segment total source file                                */
  /********************************************************************/
  sprintf( chBuffer, "%s %ld:%ld", pITMIda->chStatSegments,
             pStatusLeft->ulSegTotal, pStatusRight->ulSegTotal  );

  rectl.left   = 0;
  rectl.right  = usOneUnit * ITM_TOTAL;
  ulFilledLeft = (USHORT)rectl.right;
  rectl.bottom    = usCY - 1;
  ERASERECT( hps, &rectl, dwRGB_ERASERECTCOLOR );
  DRAWTEXT( hps, chBuffer, rectl, dwRGB_WINDOWTEXT, dwRGB_ERASERECTCOLOR,
            DT_CENTER | DT_VCENTER );
  rectl.bottom    = usCY;
  FrameRect( hps, &rectl, (HBRUSH) GetStockObject(iBRUSH) );
  /********************************************************************/
  /* display segment irregular                                        */
  /********************************************************************/
  sprintf( chBuffer, "%s %ld:%ld", pITMIda->chStatIrregular,
             pStatusLeft->ulSegIrregular, pStatusRight->ulSegIrregular  );
  rectl.left   = ulFilledLeft;
  rectl.right  = ulFilledLeft + usOneUnit * ITM_IRREGULAR;
  ulFilledLeft = (USHORT)rectl.right;
  rectl.bottom    = usCY - 1;
  ERASERECT( hps, &rectl, dwRGB_ERASERECTCOLOR);

  if (fIsHighContrast)
  {
	  DRAWTEXT( hps, chBuffer, rectl, dwRGB_IRREGULAR, dwRGB_ERASERECTCOLOR,
               DT_CENTER | DT_VCENTER );
  }
  else
  {
    DRAWTEXT( hps, chBuffer, rectl, dwRGB_WINDOWTEXT, dwRGB_ERASERECTCOLOR,
               DT_CENTER | DT_VCENTER );
  }
  rectl.bottom    = usCY;
  FrameRect( hps, &rectl, (HBRUSH) GetStockObject(iBRUSH) );
  /********************************************************************/
  /* display segment unaligned srcfile                                */
  /********************************************************************/
  sprintf( chBuffer, "%s %ld:%ld", pITMIda->chStatUnaligned,
             pStatusLeft->ulSegUnAligned, pStatusRight->ulSegUnAligned  );
  rectl.left   = ulFilledLeft;
  rectl.right  = ulFilledLeft + usOneUnit * ITM_UNALIGNED;
  ulFilledLeft = (USHORT)rectl.right;
  rectl.bottom    = usCY - 1;
  pTextType = pITMIda->pColorTable + COLOUR_VALID_10;
  if (!fIsHighContrast)
  {
    ERASERECT( hps, &rectl, dwRGB_UNALIGNED );
    DRAWTEXT( hps, chBuffer, rectl, dwRGB_WINDOWTEXT, dwRGB_UNALIGNED,
               DT_CENTER | DT_VCENTER );
  }
  else
  {
	ERASERECT( hps, &rectl, dwRGB_ERASERECTCOLOR );
    DRAWTEXT( hps, chBuffer, rectl, dwRGB_UNALIGNED, dwRGB_ERASERECTCOLOR,
               DT_CENTER | DT_VCENTER );

  }
  rectl.bottom    = usCY;
  FrameRect( hps, &rectl, (HBRUSH) GetStockObject(iBRUSH) );
  /********************************************************************/
  /* display segment unsegcrossout source file                        */
  /********************************************************************/
  sprintf( chBuffer, "%s %u:%u", pITMIda->chStatIgnored,
             pStatusLeft->usSegCrossOut, pStatusRight->usSegCrossOut  );

  rectl.left   = ulFilledLeft;
  rectl.right  = ulFilledLeft + usOneUnit * ITM_CROSSEDOUT;
  ulFilledLeft = (USHORT) rectl.right;
  rectl.bottom    = usCY - 1;
  pTextType = pITMIda->pColorTable + COLOUR_CROSSED_OUT;
  ERASERECT( hps, &rectl, dwRGB_ERASERECTCOLOR );
  DRAWTEXT( hps, chBuffer, rectl, dwRGB_CROSSEDOUTTEXT, dwRGB_ERASERECTCOLOR,
               DT_CENTER | DT_VCENTER );
  rectl.bottom    = usCY;
  FrameRect( hps, &rectl, (HBRUSH) GetStockObject(iBRUSH) );

  /********************************************************************/
  /* fill the rest with the type of alignment                         */
  /********************************************************************/
  strcpy(chBuffer, pITMIda->chStatMatch);
  strcat(chBuffer, " ");

  pstVisSrc = &(pITMIda->stVisDocSrc);
  pstVisTgt = &(pITMIda->stVisDocTgt);

  if (( pstVisSrc->ulVisActSeg ) && (pstVisTgt->ulVisActSeg) )
  {
    pSrcSeg = EQFBGetSegW(pstVisSrc->pDoc, pstVisSrc->ulVisActSeg);
    pTgtSeg = EQFBGetSegW(pstVisTgt->pDoc, pstVisTgt->ulVisActSeg);

    if ( pSrcSeg && pTgtSeg )
    {
      if ( pSrcSeg->SegFlags.JoinStart )
      {
        if ( pTgtSeg->SegFlags.JoinStart )
        {
            strcat( chBuffer, "2:2" );
        }
        else
        {
            strcat( chBuffer, "2:1" );
        } /* endif */
      }
      else
      {
        if ( pTgtSeg->SegFlags.JoinStart )
        {
            strcat( chBuffer, "1:2" );
        }
        else
        {
            strcat( chBuffer, "1:1" );
        } /* endif */
      } /* endif */
    }
    else
    {
      chBuffer[0] = EOS;
    } /* endif */
  } /* endif */
  rectl.left   = ulFilledLeft;
  rectl.right  = swp.cx;
  ulFilledLeft = (USHORT) swp.cx;
  rectl.bottom    = usCY - 1;
  pTextType = pITMIda->pColorTable + COLOUR_VISACT;
  ERASERECT( hps, &rectl, dwRGB_HIGHLIGHT  );
  DRAWTEXT( hps, chBuffer, rectl, dwRGB_HIGHLIGHTTEXT, dwRGB_HIGHLIGHT,
               DT_CENTER | DT_VCENTER );
  rectl.bottom    = usCY;
  FrameRect( hps, &rectl, (HBRUSH) GetStockObject(iBRUSH) );

} /* end of function ITMFillStatusBar */

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     ITMCountStatBar
//------------------------------------------------------------------------------
// Function call:     _
//------------------------------------------------------------------------------
// Description:       count in both docs all variables nec for status bar
//------------------------------------------------------------------------------
// Parameters:        PITMIDA   pITMIda
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Returncodes:       none
//------------------------------------------------------------------------------
// Function flow:     count in both docs all variables nec for status bar
//------------------------------------------------------------------------------
VOID
ITMCountStatBar
(
   PITMIDA    pITMIda
)
{
    CountDoc ( &(pITMIda->stVisDocSrc), &(pITMIda->stSrcInfo));
    CountDoc ( &(pITMIda->stVisDocTgt), &(pITMIda->stTgtInfo));

} /* end of function ITMCountStatBar */

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     CountDoc
//------------------------------------------------------------------------------
// Function call:     CountDoc(pVisDoc, pStatInfo)
//------------------------------------------------------------------------------
// Description:       count in document numbers required for the status bar
//------------------------------------------------------------------------------
// Parameters:        PITMVISDOC   pVisDoc,
//                    PSTATUSINFO  pStatInfo
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Returncodes:       none
//------------------------------------------------------------------------------
// Function flow:     if doc is src
//                      unaligned is QF_VALID_10
//                    else
//                      unaligned is QF_VALID_01
//                    start with 1st segment of document
//                    while not at end of document
//                      if segment is crossed out, increas CrossOut count
//                      if segment is unaligned, increase Unaligned count
//                      goto next segment
//------------------------------------------------------------------------------
VOID
CountDoc
(
   PITMVISDOC   pVisDoc,
   PSTATUSINFO  pStatInfo
)
{
  ULONG        ulSegNum;
  PTBDOCUMENT  pDoc;
  PTBSEGMENT   pSeg;
  QSTATUS      qUnalignStatus;

  if ( pVisDoc->pDoc->docType == VISSRC_DOC )
  {
    qUnalignStatus =  QF_VALID_10;
  }
  else
  {
    qUnalignStatus =  QF_VALID_01;
  } /* endif */

  pStatInfo->usSegCrossOut = 0;
  pStatInfo->ulSegUnAligned = 0;
  pStatInfo->ulSegIrregular = 0;

  ulSegNum =  1;
  pDoc = pVisDoc->pDoc;

  pSeg = EQFBGetVisSeg(pDoc, &ulSegNum);

  while ( pSeg && (ulSegNum < pDoc->ulMaxSeg))
  {
    if ( pVisDoc->pVisState[ulSegNum].CrossedOut )
    {
      pStatInfo->usSegCrossOut++;
    } /* endif */
    if ( pSeg->qStatus == qUnalignStatus )
    {
      pStatInfo->ulSegUnAligned ++;
    } /* endif */
    if (pSeg->SegFlags.JoinStart )
    {
      pStatInfo->ulSegIrregular ++;
    } /* endif */
    ulSegNum++;
    pSeg = EQFBGetVisSeg(pDoc, &ulSegNum);
  } /* endwhile */

} /* end of function CountDoc */

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     VisDocFillCrossedOut
//------------------------------------------------------------------------------
// Function call:     VisDocFillCrossedOut(pITMIda)
//------------------------------------------------------------------------------
// Description:       set crossedOut flag if segments have been crossedOut
//                    due to NOPs with ITM_TAG_STARTX /ENDX
//------------------------------------------------------------------------------
// Parameters:        PITMIDA      pITMIda
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Returncodes:       none
//------------------------------------------------------------------------------
// Function flow:     if doc is src
//------------------------------------------------------------------------------
static VOID
VisDocFillCrossedOut
(
   PITMVISDOC   pVisDoc
)
{ ULONG        ulI;
  PTBDOCUMENT  pDoc;
  PTBSEGMENT   pSeg;

  pDoc = pVisDoc->pDoc;
  for ( ulI=1;ulI < pDoc->ulMaxSeg ;ulI++ )
  {
    pSeg = EQFBGetSegW(pDoc, ulI );
    if ((pSeg->qStatus == QF_CROSSED_OUT) ||
        (pSeg->qStatus == QF_CROSSED_OUT_NOP ))
    {
      pVisDoc->pVisState[ulI].CrossedOut = TRUE;
    } /* endif */

  } /* endfor */
} /* end of function VisDocFillCrossedOut */

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     ITMFuncNextIrregular
//------------------------------------------------------------------------------
// Function call:     ITMFuncNextIrregular(pITMIda, sDirection)
//------------------------------------------------------------------------------
// Description:       goto next non 1:1 match ; skip overcross matches
//                    called from goto menuitem
//------------------------------------------------------------------------------
// Parameters:        PITMIDA    pITMIda,               ITMida
//                    SHORT      sDirection             NEXT or PREVIOUS
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Returncodes:       NONE
//------------------------------------------------------------------------------
// Function flow:
//------------------------------------------------------------------------------

VOID
ITMFuncNextIrregular
(
   PITMIDA    pITMIda,
   SHORT      sDirection
)
{
  PITMVISDOC    pstVisSrc;
  PITMVISDOC    pstVisTgt;
  ULONG         ulVisActSegOld, ulCursorSegStart;
  PITMVISDOC    pVisDoc;
  BOOL          fFound = FALSE;
  BOOL          fCrossedFound = FALSE;
  ULONG         ulCrossedOut = 0;
  ULONG         ulIrregularIndex;
  ULONG         ulCrossedIndex;
  BOOL          fInSrcCrossed = FALSE;
  ULONG         ulSegNumFound = 0;
  BOOL          fInSrc = FALSE;
  ULONG         ulStartAligned = 0;
  ULONG         ulLastAligned = 0;


  pstVisSrc = &(pITMIda->stVisDocSrc);
  pstVisTgt = &(pITMIda->stVisDocTgt);
  /********************************************************************/
  /* activate current cursor alignment if cursor not in active        */
  /* alignment                                                        */
  /********************************************************************/
  pVisDoc = ACCESSWNDIDA(pITMIda->hwndFocus, PITMVISDOC );

  ulCursorSegStart = pVisDoc->pDoc->TBCursor.ulSegNum;

  if ( ulCursorSegStart != pVisDoc->ulVisActSeg )
  {
    ITMFuncSynch(pITMIda, pVisDoc);              // find alignment
  } /* endif */
  /********************************************************************/
  /* start searching at current active segment or segnum 1            */
  /********************************************************************/
  ulVisActSegOld = pstVisSrc->ulVisActSeg;
  ulStartAligned = pstVisSrc->pulNumAligned[ulVisActSegOld];
  /********************************************************************/
  /* find index of current alignment                                  */
  /* ulStartAligned = 0 if none found                                 */
  /********************************************************************/
  ulLastAligned = FindCurAlignIndex(pstVisSrc, ulVisActSegOld, sDirection);

  fFound = ITMFindNextIrregular(pITMIda, pstVisSrc,
                                ulStartAligned, ulLastAligned,
                               sDirection, &ulIrregularIndex,
                               &ulSegNumFound, &fInSrc);

  fCrossedFound = ITMFindNextCrossed(pITMIda, ulStartAligned,sDirection,
                                     &ulCrossedOut, &ulCrossedIndex,
                                     &fInSrcCrossed);

  if ( ulVisActSegOld && (fFound || fCrossedFound) )
  {
    /******************************************************************/
    /* reset qstatus of old visactseg                                 */
    /******************************************************************/
    VisActReset(pstVisSrc, pstVisTgt);

    EQFBFuncMarkClear(pstVisSrc->pDoc);
    EQFBFuncMarkClear(pstVisTgt->pDoc);
    /******************************************************************/
    /* set new active segment; find which one occurs first            */
    /******************************************************************/
    if (fCrossedFound && fFound )
    {
      if (sDirection == NEXT )
      {
        if (ulCrossedIndex <= ulIrregularIndex )
        {
          fFound = FALSE;
        }
        else
        {
          fCrossedFound = FALSE;   // usCrossed > usIrr
        } /* endif */
      }
      else                  // PREVIOUS
      {
        if (ulCrossedIndex >= ulIrregularIndex )
        {
          fFound = FALSE;
        }
        else
        {
          fCrossedFound = FALSE;
        } /* endif */
      } /* endif */
    } /* endif */
    if ( fCrossedFound)
    {
       pstVisSrc->ulVisActSeg = pITMIda->Aligned.pulSrc[ulCrossedIndex];
       pstVisTgt->ulVisActSeg = pITMIda->Aligned.pulTgt1[ulCrossedIndex];
       if (fInSrcCrossed )
       {
         ITMVisMark(pstVisSrc->pDoc, ulCrossedOut);
       }
       else
       {
         ITMVisMark(pstVisTgt->pDoc, ulCrossedOut);
       } /* endif */
    }
    else
    {
      pstVisSrc->ulVisActSeg = pITMIda->Aligned.pulSrc[ulIrregularIndex];
      pstVisTgt->ulVisActSeg = pITMIda->Aligned.pulTgt1[ulIrregularIndex];
      if (fInSrc )
      {
        ITMVisMark(pstVisSrc->pDoc, ulSegNumFound);
      }
      else
      {
        ITMVisMark(pstVisTgt->pDoc, ulSegNumFound);
      } /* endif */
    } /* endif */

    VisActNew (pstVisSrc, pstVisTgt);
    UPDSTATUSBAR( (PITMIDA)pVisDoc->pITMIda );
  }
  else
  {
    ITMUtlError( pITMIda, ITM_NOFOUND, MB_CANCEL, 0, NULL, EQF_WARNING);
  } /* endif */

  return;
} /* end of function VOID ITMFuncNextIrregular(PITMIDA) */

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     ITMFindNextIrregular
//------------------------------------------------------------------------------
// Function call:     ITMFindNextIrregular(pITMIda, pVisSrc,
//------------------------------------------------------------------------------
// Description:       find next match which is 1:2, 2:1, 0:1 or 1:0
//------------------------------------------------------------------------------
// Parameters:        PITMIDA     pITMIda,
//                    PVISDOC     pstVisSrc,
//                    ULONG       ulStartAligned
//                    ULONG       ulLastAligned
//                    SHORT       sDirection,
//                    PULONG      pulIrregularIndex
//                    PULONG      pulSegNumFound,
//                    BOOL        fInSrc
//------------------------------------------------------------------------------
// Returncode type:   BOOL
//------------------------------------------------------------------------------
// Returncodes:       TRUE      if an irregular match is found
//                    FALSE     if nothing found
//------------------------------------------------------------------------------
// Function flow:    find starting alignment
//                   while not at end of alignments or not found
//                     if alignment is 1:0 or 0:1 or 1:2 or 2:1
//                          irregular found, stop
//                     else
//                       goon with next alignment
//                  endwhile
//                  return found indicator and alignment index
//------------------------------------------------------------------------------
static BOOL
ITMFindNextIrregular
(
    PITMIDA     pITMIda,
    PITMVISDOC  pstVisSrc,
    ULONG       ulStartAligned,
    ULONG       ulLastAligned,
    SHORT       sDirection,
    PULONG      pulIrregularIndex,
    PULONG      pulSegNumFound,
    PBOOL       pfInSrc
)
{
   BOOL       fFound = FALSE;        // no irregular found
   ULONG      ulIndex = 0;

  pstVisSrc = NULL;
  *pfInSrc = FALSE;
  *pulSegNumFound = 0;
  *pulIrregularIndex = 0;
  /********************************************************************/
  /* active alignment cannot be a 1:0 / 0:1 alignment ;               */
  /* hence remember the last alignment                                */
  /********************************************************************/
  if ( ulLastAligned )
  {
    ulIndex = ulLastAligned;

    while ( !fFound && (ulIndex < (pITMIda->Aligned.ulUsed) )
              && (ulIndex > 0) )
    {
      if ( (pITMIda->Aligned.pbType[ulIndex] == NUL_ONE) ||
           (pITMIda->Aligned.pbType[ulIndex] == ONE_NUL) ||
           (pITMIda->Aligned.pbType[ulIndex] == TWO_ONE) ||
           (pITMIda->Aligned.pbType[ulIndex] == ONE_TWO) )
      {
         /**************************************************************/
         /* make sure that we do not stay at current alignment         */
         /**************************************************************/
         if ( ulLastAligned != ulStartAligned )   //force moving actseg
         {
           fFound = TRUE;
         }
         else
         {
           ulIndex += sDirection;
         } /* endif */
      }
      else
      {
        ulLastAligned = ulIndex;
        ulIndex += sDirection;
      } /* endif */
    } /* endwhile */
  } /* endif */
  if (fFound )
  {
    if ((pITMIda->Aligned.pbType[ulIndex] == NUL_ONE )||
        (pITMIda->Aligned.pbType[ulIndex] == ONE_NUL)  )
    {
      *pulIrregularIndex = ulLastAligned;
      if (pITMIda->Aligned.pbType[ulIndex] == ONE_NUL )
      {
        *pfInSrc = TRUE;
        *pulSegNumFound = pITMIda->Aligned.pulSrc[ulIndex];
      }
      else
      {
        *pulSegNumFound = pITMIda->Aligned.pulTgt1[ulIndex];
      } /* endif */
    }
    else
    {
      *pulIrregularIndex = ulIndex;
      if (pITMIda->Aligned.pbType[ulIndex] == TWO_ONE )
      {
        *pulSegNumFound = pITMIda->Aligned.pulSrc[ulIndex];
        *pfInSrc = TRUE;
      }
      else
      {
        *pulSegNumFound = pITMIda->Aligned.pulTgt1[ulIndex];
      } /* endif */
    } /* endif */
  } /* endif */
  return(fFound);
} /* end of function BOOL ITMFindNextIrregular */


//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     ITMFindNextCrossed
//------------------------------------------------------------------------------
// Function call:     ITMFindNextCrossed(
//------------------------------------------------------------------------------
// Description:       find next crossed out segment in specified direction
//------------------------------------------------------------------------------
// Parameters:        PITMIDA     pITMIda,
//                    USHORT      usSrcStartSegnum,
//                    SHORT       sDirection,
//                    PULONG      pulCrossedSegnum,
//                    PULONG      pulCrossedIndex,
//                    BOOL        fCrossedInSrc
//------------------------------------------------------------------------------
// Returncode type:   BOOL
//------------------------------------------------------------------------------
// Returncodes:       TRUE    Crossed out found
//                    FALSE   nothing found
//------------------------------------------------------------------------------
// Function flow:     Search for next crossed segment in srcdoc
//                    search for next crossed segment in tgtdoc
//                    find alignments next to the crossed segments found
//                    return found indicator, crossed out segment number
//                    and alignment index next to the crossed out segm.
//------------------------------------------------------------------------------
static BOOL
ITMFindNextCrossed
(
    PITMIDA     pITMIda,
    ULONG       ulStartAligned,
    SHORT       sDirection,
    PULONG      pulCrossedSegnum,
    PULONG      pulCrossedIndex,
    PBOOL       pfCrossedInSrc
)
{
  BOOL        fSrcCrossedFound = FALSE;
  BOOL        fTgtCrossedFound = FALSE;
  PITMVISDOC  pstVisSrc;
  PITMVISDOC  pstVisTgt;
  ULONG       ulSrcCrossedSegnum = 0;
  ULONG       ulTgtCrossedSegnum = 0;
  ULONG       ulSrcCrossIndex = 0;
  ULONG       ulTgtCrossIndex = 0;
  BOOL        fFound = FALSE;

  /********************************************************************/
  /* find next crossed-out segment                                    */
  /********************************************************************/
  pstVisSrc = &(pITMIda->stVisDocSrc);
  ulSrcCrossIndex = ulStartAligned;
  fSrcCrossedFound = ITMLoopForNextCrossed(pstVisSrc,
                         pstVisSrc->ulVisActSeg,
                         pstVisSrc->pDoc->ulMaxSeg,
                         sDirection,
                         &ulSrcCrossedSegnum,
                         &ulSrcCrossIndex);

  ulTgtCrossIndex = ulStartAligned;
  pstVisTgt = &(pITMIda->stVisDocTgt);
  fTgtCrossedFound = ITMLoopForNextCrossed(pstVisTgt,
                         pstVisTgt->ulVisActSeg,
                         pstVisTgt->pDoc->ulMaxSeg,
                         sDirection,
                         &ulTgtCrossedSegnum,
                         &ulTgtCrossIndex);

  if (fSrcCrossedFound || fTgtCrossedFound)
  {
    fFound = TRUE;
  } /* endif */
  if (fSrcCrossedFound && fTgtCrossedFound)
  {
    if (sDirection == NEXT)
    {
      if (ulSrcCrossIndex < ulTgtCrossIndex )
      {
         *pulCrossedIndex = ulSrcCrossIndex;
         *pulCrossedSegnum = ulSrcCrossedSegnum;
         *pfCrossedInSrc = TRUE;
      }
      else
      {
         *pulCrossedIndex = ulTgtCrossIndex;
         *pulCrossedSegnum = ulTgtCrossedSegnum;
         *pfCrossedInSrc = FALSE;
      } /* endif */
    }
    else
    {
      if (ulSrcCrossIndex > ulTgtCrossIndex )
      {
         *pulCrossedIndex = ulSrcCrossIndex;
         *pulCrossedSegnum = ulSrcCrossedSegnum;
         *pfCrossedInSrc = TRUE;
      }
      else
      {
         *pulCrossedIndex = ulTgtCrossIndex;
         *pulCrossedSegnum = ulTgtCrossedSegnum;
         *pfCrossedInSrc = FALSE;
      } /* endif */
    } /* endif */
  }
  else
  {
    if (fSrcCrossedFound )
    {
      *pulCrossedIndex = ulSrcCrossIndex;
      *pulCrossedSegnum = ulSrcCrossedSegnum;
      *pfCrossedInSrc = TRUE;
    } /* endif */
    if (fTgtCrossedFound )
    {
      *pulCrossedIndex = ulTgtCrossIndex;
      *pulCrossedSegnum = ulTgtCrossedSegnum;
      *pfCrossedInSrc = FALSE;
    } /* endif */
  } /* endif */


  return(fFound);
} /* end of function BOOL ITMFindNextCrossed */


//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     ITMLoopForNextCrossed
//------------------------------------------------------------------------------
// Function call:     ITMLoopForNextCrossed(
//------------------------------------------------------------------------------
// Description:       find next crossed out segment in specified direction
//                    and in specified document
//------------------------------------------------------------------------------
// Parameters:        PITMVISDOC  pstVisDoc
//                    ULONG       ulStartSegnum,
//                    ULONG       ulMaxSegnum
//                    SHORT       sDirection,
//                    PULONG      pulCrossedSegnum,
//                    PULONG      pulCrossIndex
//------------------------------------------------------------------------------
// Returncode type:   BOOL
//------------------------------------------------------------------------------
// Returncodes:       TRUE    found
//                    FALSE   none found
//------------------------------------------------------------------------------
// Function flow:     while not found or not at end
//                      if current segment is crossed out: found,
//                      else goto next segment
//                    endwhile
//------------------------------------------------------------------------------
static BOOL
ITMLoopForNextCrossed
(
    PITMVISDOC  pstVisDoc,
    ULONG       ulStartSegnum,
    ULONG       ulMaxSegnum,
    SHORT       sDirection,
    PULONG      pulCrossedSegnum,
    PULONG      pulCrossIndex
)
{
  BOOL    fCrossedFound = FALSE;
  ULONG   ulI;
  ULONG   ulNewCrossIndex = 0;

  ulI = ulStartSegnum;
  *pulCrossedSegnum = 0;

  while (!fCrossedFound && (ulI< ulMaxSegnum) && (ulI > 0) )
  {
    if (pstVisDoc->pVisState[ulI].CrossedOut ||
        pstVisDoc->pVisState[ulI].UserJoin   )
    {
      if (pstVisDoc->pVisState[ulI].CrossedOut )
      {
        ulNewCrossIndex = FindCurAlignIndex(pstVisDoc,
                                            ulI, sDirection);
      }
      else
      {
          ulNewCrossIndex = pstVisDoc->pulNumAligned[ulI];
      } /* endif */
      if (ulNewCrossIndex != *(pulCrossIndex) )
      {
        fCrossedFound = TRUE;
        *pulCrossedSegnum = ulI;
        *pulCrossIndex = ulNewCrossIndex;
      }
      else
      {
        ulI+=sDirection;
      } /* endif */
    }
    else
    {
      ulI+=sDirection;
    } /* endif */
  } /* endwhile */

  return(fCrossedFound);
} /* end of function BOOL ITMLoopForNextCrossed */

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     ITMVisMark
//------------------------------------------------------------------------------
// Function call:     ITMVisMark
//------------------------------------------------------------------------------
// Description:       mark the irregular sentence
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT pDoc
//                    ULONG       ulSegNum
//------------------------------------------------------------------------------
// Returncode type:   void
//------------------------------------------------------------------------------
// Function flow:     get Blockmark structure
//                    set start, end and segnum of new marked segment
//                    force redraw
//------------------------------------------------------------------------------
static void
ITMVisMark
(
    PTBDOCUMENT pDoc,
    ULONG       ulSegNum
)
{

   PTBSEGMENT pSeg;                    // pointer to segment
   PEQFBBLOCK  pstBlock;              // pointer to block struct

   pstBlock = (PEQFBBLOCK) pDoc->pBlockMark;

   pstBlock->pDoc     = pDoc;
   pstBlock->ulSegNum = ulSegNum;
   pstBlock->usStart  = 0;
   pSeg = EQFBGetSegW( pDoc, ulSegNum );
   pstBlock->usEnd    = (USHORT)(UTF16strlenCHAR( pSeg->pDataW ) );
   pDoc->Redraw |= REDRAW_ALL;
  return;
} /* end of function BOOL ITMVisMark */


