/*! \file
	Description: this module contains the document instance window procedure

	Copyright Notice:

	Copyright (C) 1990-2016, International Business Machines
	Corporation and others. All rights reserved
*/

#define WM_MOUSEWHEEL                   0x020A

#define INCL_AVIO
#define INCL_DEV
#define INCL_EQF_ANALYSIS
#define INCL_EQF_TM               // general Transl. Memory functions
#define INCL_EQF_TP               // public translation processor functions
#define INCL_EQF_EDITORAPI        // editor API
#define INCL_EQF_TAGTABLE         // tagtable defines and functions
#define INCL_EQF_FOLDER           // folder list and document list functions
#define INCL_EQF_MORPH
#include <eqf.h>                  // General Translation Manager include file

#include <eqfdoc00.h>             // document instance ida...

#include "EQFTPI.H"                    // Translation Processor priv. include file
#include "EQFB.ID"                     // Translation Processor IDs
#include "EQFBDLG.ID"                  // dialog PM IDs
#include "EQFHELP.ID"                  // help ids for tutorial
#include "eqfstart.h"
#include "ReImportDoc.h"


static void EQFBIncOffset ( PTBDOCUMENT pDoc,         // document ida
                            LONG     lInc );         // increment
static void EQFBSetOffset ( PTBDOCUMENT pDoc,         // document ida
                            LONG     lInc );         // increment

static MRESULT EQFBButton1Down(  HWND hwnd, WINMSG msg, WPARAM mp1,
                              LPARAM mp2, PTBDOCUMENT pDoc );

                                      // orginal child Frame Proc
static PFNWP TBFrameWndProc;

PFINDDATA    pFindData = NULL; // ptr to data structure for find dialog

static PSPELLDATA   pGlobalSpellData = NULL; // ptr to global spelldata structure

void ResetSpellData( PSPELLDATA pGlobalSpellData, PSTEQFGEN pstEQFGen );
PSPELLDATA AllocSpellData( PTBDOCUMENT pDoc, PSTEQFGEN pstEQFGen, BOOL fResetIgnoreData );

/**********************************************************************/
/* ruler data                                                         */
/* Standard ruler up to 1000 columns -- otherwise we use defaults only*/
/**********************************************************************/
static const PSZ pRuler = "\
----+----1----+----2----+----3----+----4----+----5----+----6----+----7----+----8\
----+----9----+---10----+---11----+---12----+---13----+---14----+---15----+---16\
----+---17----+---18----+---19----+---20----+---21----+---22----+---23----+---24\
----+---25----+---26----+---27----+---28----+---29----+---30----+---31----+---32\
----+---33----+---34----+---35----+---36----+---37----+---38----+---39----+---40\
----+---41----+---42----+---43----+---44----+---45----+---46----+---47----+---48\
----+---49----+---50----+---51----+---52----+---53----+---54----+---55----+---56\
----+---57----+---58----+---59----+---60----+---61----+---62----+---63----+---64\
----+---65----+---66----+---67----+---68----+---69----+---70----+---71----+---72\
----+---73----+---74----+---75----+---76----+---77----+---78----+---79----+---80\
----+---81----+---82----+---83----+---84----+---85----+---86----+---87----+---88\
----+---89----+---90----+---91----+---92----+---93----+---94----+---95----+---96\
----+---97----+---98----+---99----+--100----+---";


static const PSZ pRulerDef = "\
--------------------------------------------------------------------------------\
--------------------------------------------------------------------------------\
--------------------------------------------------------------------------------\
--------------------------------------------------------------------------------\
--------------------------------------------------------------------------------\
--------------------------------------------------------------------------------\
--------------------------------------------------------------------------------\
--------------------------------------------------------------------------------";

// buffer for modified ruler
static CHAR szRulerDefMod[4096];

/*******************************************************/
/*           Class EQFB window procedure            */
/*******************************************************/

#undef WinDefWindowProc
#define WinDefWindowProc( a, b, c, d ) DefMDIChildProc( a, b, c, d )

static VOID EQFBSpellPullDown ( HWND, POINT, PTBDOCUMENT, USHORT, PSZ_W, USHORT, USHORT );
 void HandlePopupTEMenu( HWND hwnd, POINT point, SHORT sMenuID, PTBDOCUMENT pDoc );

  #ifdef _PTM
    #include <commdlg.h>
  #endif

VOID EQFBMouseToCursor ( PTBDOCUMENT pDoc, LONG x, LONG y, TBROWOFFSET *pCursor );

#define TRIPLE_ID   456

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:
//------------------------------------------------------------------------------
// Function call:     _
//------------------------------------------------------------------------------
// Description:       this function will contain the document instance window
//                    procedure
//------------------------------------------------------------------------------
// Input parameter:   if PM:
//                    HWND         hwnd         - window handle
//                    USHORT       msg          - message
//                    MPARAM       mp1
//                    MPARAM       mp2
//                    if WINDOWS:
//                    HWND         hwnd         - window handle
//                    USHORT       msg          - message
//                    WORD         mp1
//                    LONG         mp2
//------------------------------------------------------------------------------
// Output parameter:  MRESULT      mResult       - return value from PM calls
//               or   long                       - return in windows
//------------------------------------------------------------------------------
// Returncode type:   _
//------------------------------------------------------------------------------
// Returncodes:       _
//------------------------------------------------------------------------------
// Prerequesits:      _
//------------------------------------------------------------------------------
// Side effects:      _
//------------------------------------------------------------------------------
// Samples:           _
//------------------------------------------------------------------------------
// Function flow:     The following cases are handled:
//                    WM_CHAR:
//                    - determine the pressed character via the keyboard
//                      mapping Table (EQFBCONF) and route it to the
//                      appropriate editor function
//                      Force a repaint of the line
//                    WM_COMMAND:
//                    - do the handling of all actionbar items ( EQFBAABCmd)
//                    WM_INITMENU:
//                    - set the initial settings of the actionbar items
//                      according to the type of document and the cursor
//                      position
//                    WM_CLOSE:
//                    - depending on the active doctype do the following
//                      action:
//                        SOURCE_DOC :    hide window
//                        OTHER_DOC  :    quit document
//                        TARGET_DOC :    file target document and quit
//                                        source doc
//                    WM_HELP:
//                    - pass a message to the Workbench to invoke the appropr.
//                      help messages...
//
//                    default:
//                       pass it on to our EQFBDispClass window proc
//------------------------------------------------------------------------------
MRESULT APIENTRY
EQFBWNDPROC( HWND hwnd,
              WINMSG msg,
              WPARAM mp1,
              LPARAM mp2 )
{
    PTBDOCUMENT  pDoc;                 // pointer to doc ida

    USHORT usAction;                    // action status
    void (*function)( PTBDOCUMENT );             // and function to be processed
    MRESULT   mResult = FALSE;                   // return value from window proc

    USHORT   usStatus;                  // current status
    USHORT   usFunction;                // current function ID
    BOOL     fDBCSOK = TRUE;
                                        // Pointer to instance variables
    pDoc = ACCESSWNDIDA( hwnd, PTBDOCUMENT );
                                // Get instance variables
    switch( msg )
    {

      case WM_INPUTLANGCHANGE:
      {
            // User changed input locale
        HKL NewInputLocale = (HKL) mp2;
        CHAR szLocaleData[6] ;

        GetLocaleInfo(MAKELCID(LOWORD(NewInputLocale), SORT_DEFAULT) , LOCALE_IDEFAULTANSICODEPAGE, szLocaleData, 6);
//@@TODO!!        pDoc->ulAnsiCodePage  = strtoul(szLocaleData, NULL, 10);
        }
      break;
       /***************************************************************/
       /* Attention: Under Windows WM_CHAR is split up in WM_KEYDOWN  */
       /*            and WM_CHAR. We can treat both using the same    */
       /*            case...                                          */
       /***************************************************************/

         case WM_KEYDOWN:
           /***********************************************************/
           /* check if help key is pressed -> in all other cases fall */
           /* through to normal processing...                         */
           /***********************************************************/
           if ( mp1 == VK_F1 )
           {
             /**************************************************************/
             /* Trigger help by posting HM_HELPSUBITEM_NOT_FOUND to TWB    */
             /**************************************************************/
             USHORT usID;
             switch ( pDoc->docType )
             {
                case SSOURCE_DOC:
                  usID = ID_TWBS_ORIG_WINDOW;
                  break;
                case STARGET_DOC:
                  usID = ID_TWBS_SOURCE_WINDOW;
                  break;
                case SERVDICT_DOC:
                  usID = ID_TWBS_DICT_WINDOW;
                  break;
                case SERVPROP_DOC:
                case SERVSOURCE_DOC:
                  usID = ID_TWBS_PROP_WINDOW;
                  break;
                case TRNOTE_DOC:
                  usID = ID_TWBS_TRNOTE_WINDOW;
                  break;
                default:
                case OTHER_DOC:
                  usID = ID_TWBS_OTHER_WINDOW;
                  break;
             } /* endswitch */

           // GQ Test: try to convert key to Unicode char
           if ( msg == WM_KEYDOWN )
           {
             HKL hkl = GetKeyboardLayout( 0 );
             BYTE kb[256];

             GetKeyboardState(kb);
             WCHAR uc[5] = {};

             int i = 0;
             switch( ToUnicodeEx( mp1, MapVirtualKey( mp1, MAPVK_VK_TO_VSC), kb, uc, 4, 0, hkl ))
             {
              case -1: /* dead key */ i = -1; break;
              case  0: /* no idea! */; i = 0; break;
              case  1:
              case  2:
              case  3:
              case  4:
                i = 4; 
                break;
             }
           }

             PostMessage( (HWND)UtlQueryULong( QL_TWBFRAME ),
                          HM_HELPSUBITEM_NOT_FOUND,
                          0,
                          MP2FROM2SHORT( usID, usID ));
             break;
           } /* endif */
//       case WM_SYSKEYUP:
         case WM_SYSCHAR:
//         case WM_SYSDEADCHAR:
//         case WM_DEADCHAR:
         case WM_SYSKEYDOWN:
         case WM_CHAR:           // determine character and pass it to editor
         /* dalia (start) */
         /* - If Shift+Numlock combination is detected activate PushMode     */
         /* - If Shift+Pad Slash combination is detected deactivate PushMode */
         /* - If Alt+Pad Slash combination is detected toggle Auto PushMode  */
         /* - If Auto PushMode is active, Check if key ends Auto PushMode    */

            if ( EQFBMapKey( msg, mp1, mp2, &usFunction, &pDoc->ucState, TPRO_MAPKEY) )
            {
              if (IsDBCS_CP(pDoc->ulOemCodePage) && (msg == WM_CHAR))
              {
                 pDoc->usChar      = (USHORT)mp1;

              }
              else
              {
                 pDoc->usChar      = (USHORT)mp1;

                 pDoc->usDBCS2Char = 0;
                 if ((pDoc->ucState & ST_CTRL) && (pDoc->ucState & ST_SHIFT)
                      && ('A' <= pDoc->usChar) && (pDoc->usChar <= 'Z') )
                 {
                   pDoc->usChar = pDoc->usChar + 'a' - 'A';
                 } /* endif */
                 fDBCSOK = TRUE;
              } /* endif */

              // Add for R012027 start
              UCHAR uState;
              EQFBKeyState((USHORT) mp1, &pDoc->ucCode, &uState);
              // Add end

              if (fDBCSOK )     // do not process half of DBCS character
              {
				 PFUNCTIONTABLE pFuncTab = get_FuncTab();
                 usAction     = (pFuncTab + usFunction)->usAction;
                 function     = (pFuncTab + usFunction)->function;
                 usStatus = EQFBCurrentState( pDoc );
                 if ((usStatus & usAction) == usAction )
                 {
                     // fix for V0042 - Arabic original document
                     if ( (pDoc->docType == SSOURCE_DOC) &&
                          ((pFuncTab + LEFT_FUNC)->function != EQFBFuncLeft))
                     { // redo the BIDILRSwap -- Source doc is always displayed LTR!!
                        FUNCTIONTABLE Temp;

                        Temp = EQFBBidiRedoLRSwap(usFunction);
                        function = Temp.function;
                        usAction = Temp.usAction;
                     }

                    (*function)( pDoc );           // execute the function
                    /****************************************************/
                    /* check if it is a function where pDoc will be     */
                    /* freed and therefore is invalid furthermore       */
                    /****************************************************/
                    if (!((usFunction == QUIT_FUNC)||(usFunction == FILE_FUNC)||(usFunction == REIMPORT_FUNC) )
                         && pDoc->pSegTables )  // be sure noone else closed doc..
                    {
                      EQFBRefreshScreen( pDoc );  // refresh the screen
                    } /* endif */
                 }
                 else if (usFunction == BACKSPACE_FUNC)
                 {
					 // SVT506: S000005: allow backspace if cursor after last char
					 // of active segment, i.e. on first char of next segment
					 EQFBFuncLeft(pDoc);
					 usStatus = EQFBCurrentState( pDoc);
					 if (usStatus == ACTSEG)
					 {
						 EQFBFuncDeleteChar(pDoc);
				     }
				     else
				     {
						EQFBFuncRight(pDoc);   // position cursor as before
						EQFBFuncNothing( pDoc ); // ignore keystroke
				     }
				     EQFBRefreshScreen( pDoc );  // refresh the screen
			     }
                 else
                 {
                    EQFBFuncNothing( pDoc ); // ignore keystroke
                 } /* endif */

                 mResult = MRFROMSHORT(TRUE);// indicate message is processed
              } /* endif */
            }
            else
            {
               mResult = WinDefWindowProc ( GETPARENT(hwnd), msg, mp1, mp2 );
            } /* endif */
            break;
        case WM_COMMAND:
            EQFBWndProc_Command (hwnd, mp1, mp2 );
            break;

       /***************************************************************/
       /* ATTENTION: Under Windows WM_INITMENUPOPUP will be similar   */
       /*            to WM_INITMENU, except that mp2 contains the     */
       /*            zero based position of the SubMenu (POPUP).      */
       /*            This change is taken into consideration in the   */
       /*            EQFB.ID file.                                    */
       /***************************************************************/
        case WM_INITMENUPOPUP:
        case WM_EQF_INITMENU:
            EQFBInitMenu( pDoc, LOWORD( mp2 ));
            mResult = WinDefWindowProc ( hwnd, msg, mp1, mp2 );
            break;
        case WM_INITMENU:
           /*******************************************************************/
           /* modify System menu to look like a secondary window...           */
           /*******************************************************************/
           {
             HMENU hSysMenu = GetSystemMenu( hwnd, FALSE );
             if ( hSysMenu != NULL )
             {
               CHAR chText[80];
               LONG lNum;
               HMENU hSysParent = GetSystemMenu( GETPARENT(hwnd), FALSE);

               chText[0] = EOS;
               GetMenuString( hSysParent, SC_CLOSE,
                              chText, sizeof( chText ), MF_BYCOMMAND);
               if ( chText[0] )
               {
                 ModifyMenu( hSysMenu, SC_CLOSE, MF_BYCOMMAND, SC_CLOSE,
                             chText );
               } /* endif */
               RemoveMenu( hSysMenu, SC_TASKLIST, MF_BYCOMMAND );
               /*******************************************************/
               /* remove separator line - separator id = 0            */
               /*******************************************************/
               lNum = GetMenuItemCount( hSysMenu );
               if ( (lNum > 1) &&  !GetMenuItemID( hSysMenu, lNum-1)  )
               {
                 RemoveMenu( hSysMenu, lNum-1, MF_BYPOSITION );
               } /* endif */
             }
            }
            mResult = EQFBDispClass( hwnd, msg, mp1, mp2, pDoc );
            break;

        case WM_CLOSE:        // if it is the source doc only hide it
            switch ( pDoc->docType )
            {
               case SSOURCE_DOC:
                  SendMessage( ((PSTEQFGEN)pDoc->pstEQFGen)->hwndTWBS,
                                 WM_EQF_SETFOCUS,
                                 0, MP2FROMP( pDoc->twin->hwndFrame ));
                  WinShowWindow( pDoc->hwndFrame , FALSE );
                  break;
               case TRNOTE_DOC:
                {
                  PTBDOCUMENT  pDocCurrent;
                  pDocCurrent = pDoc->next;
                  while ((pDocCurrent ->docType != STARGET_DOC ) &&
                          (pDocCurrent != pDoc ) )
                  {
                    pDocCurrent = pDocCurrent->next;
                  } /* endwhile */

                  if (pDocCurrent->docType == STARGET_DOC)
                  {
                      SendMessage( ((PSTEQFGEN)pDocCurrent->pstEQFGen)->hwndTWBS,
                                   WM_EQF_SETFOCUS,
                                   0, MP2FROMP( pDocCurrent->hwndFrame ));
                    WinShowWindow( pDoc->hwndFrame , FALSE );
                  }
                  else
                  {
                    pDoc = EQFBFuncQuit( pDoc       );     // quit document
                  }
                }
                  break;
               case STARGET_DOC:
                  {
                    USHORT usRc;
                    pDoc->EQFBFlags.AutoMode  = FALSE;     // reset auto mode
                    usRc = EQF_XDOCNUM( (PSTEQFGEN)pDoc->pstEQFGen, 1, (PSZ)pDoc->pInBuf );
                    if ( !usRc && *pDoc->pInBuf )
                    {
                      // close current document and activate new one...
                      CHAR chObjName[MAX_EQF_PATH];
                      PSTEQFGEN pstEQFGen = (PSTEQFGEN)pDoc->pstEQFGen;
                      PTBDOCUMENT pDocCurrent = pDoc;
                      if (pDocCurrent->docType == STARGET_DOC)
                        pDocCurrent = pDocCurrent->next;
                      while ( pDocCurrent != pDoc && pDocCurrent->docType != STARGET_DOC )
                         pDocCurrent = pDocCurrent->next;
                      if (pDocCurrent == pDoc)
                         pDocCurrent = NULL;

                      EQFBFuncFile( pDoc );
                      if ( !EQF_XDOCNUM(pstEQFGen, 0, chObjName) )
                      {
                        EQFBTenvStart( pDocCurrent, chObjName, pstEQFGen );
                      }
                      else
                      {
                        EQFBQuit (hwnd);                       // quit documents
                      } /* endif */
                    }
                    else
                    {
                      EQFBQuit (hwnd);                       // quit documents
                    }
                  }
                  break;
               case OTHER_DOC:
                  pDoc = EQFBFuncQuit( pDoc       );     // quit document
                  break;
            } /* endswitch */

            break;

        case WM_EQF_AUTOTRANS:
            if ( pDoc->EQFBFlags.AutoMode )
            {
               EQFBFuncAutoTrans( pDoc );
            } /* endif */
            break;

        case WM_WINDOWPOSCHANGING:
          /************************************************************/
          /* adjust positions/sizes of window to match our display    */
          /* characteristics (character size chosen)                  */
          /************************************************************/
          {
            WINDOWPOS FAR * pSWP = (WINDOWPOS FAR *) mp2;
            ULONG  ulDelta;

            if ( pDoc )
            {
               ulDelta = 2*WinQuerySysValue(HWND_DESKTOP,SV_CXSIZEBORDER) - 1;
               if ( GetWindowLong( pDoc->hwndFrame, GWL_STYLE ) & WS_VSCROLL )
               {
                 ulDelta += WinQuerySysValue(HWND_DESKTOP, SV_CXVSCROLL );
               } /* endif */
               pSWP->cx = ulDelta  +
                            ((pSWP->cx - ulDelta) / pDoc->cx ) * pDoc->cx;

               /*******************************************************/
               /* horizontal size ...                                 */
               /*******************************************************/
               ulDelta = 2*WinQuerySysValue(HWND_DESKTOP,SV_CYSIZEBORDER);
               if ( GetWindowLong( pDoc->hwndFrame, GWL_STYLE ) & WS_HSCROLL )
               {
                 ulDelta += WinQuerySysValue(HWND_DESKTOP, SV_CYHSCROLL );
               } /* endif */
               if ( GetWindowLong( pDoc->hwndFrame, GWL_STYLE ) & WS_CAPTION )
               {
                 ulDelta += WinQuerySysValue(HWND_DESKTOP, SV_CYTITLEBAR);
               } /* endif */

               if ( ((PSTEQFGEN)pDoc->pstEQFGen)->bStatusBar[pDoc->docType] & TP_WND_STATUSBAR )
               {
                 ulDelta += WinQuerySysValue(HWND_DESKTOP, SV_CYMENU);
               } /* endif */
               if ( ((PSTEQFGEN)pDoc->pstEQFGen)->bStatusBar[pDoc->docType] & TP_WND_RULER )
               {
                 pDoc->ulRulerSize = pDoc->cy;
                 ulDelta += pDoc->cy;
               } /* endif */
               pSWP->cy = ulDelta +
                          ((pSWP->cy - ulDelta) / pDoc->cy ) * pDoc->cy;
            } /* endif */
          }
          break;

       case WM_GETMINMAXINFO:
          /************************************************************/
          /* check that the windows min/max size is not jeopardized   */
          /************************************************************/
          {
            // minimum to be sized at least 4 rows
            if ( pDoc )
            {
              MINMAXINFO FAR* plpmmi = (MINMAXINFO FAR*) mp2;

              plpmmi->ptMinTrackSize.y =
                 (USHORT) ( WinQuerySysValue( HWND_DESKTOP, SV_CYTITLEBAR)  +
                            WinQuerySysValue(HWND_DESKTOP, SV_CYHSCROLL ) +
                            2 * WinQuerySysValue(HWND_DESKTOP,SV_CYSIZEBORDER))+
                            WinQuerySysValue( HWND_DESKTOP, SV_CYMENU ) +  //statusbar
                            4 * pDoc->cy;

              // minimium width 40 characters...
              plpmmi->ptMinTrackSize.x = MIN_CX_SIZE * pDoc->cx;

              /********************************************************/
              /* limit our maximum size to TranslationEnvironment --  */
              /* is jeopardized by Windows due to MDI limitations..   */
              /********************************************************/
              if ( pDoc->pstEQFGen )
              {
                HWND hwndTemp;
                RECT rect;

                hwndTemp = ((PSTEQFGEN)pDoc->pstEQFGen)->hwndTWBS;
                GetClientRect( hwndTemp, &rect );
                plpmmi->ptMaxSize.x = rect.right +
                                      2*WinQuerySysValue(HWND_DESKTOP,SV_CXSIZEBORDER)  ;
                plpmmi->ptMaxSize.y = rect.bottom +
                                      2*WinQuerySysValue(HWND_DESKTOP,SV_CYSIZEBORDER)  ;
              } /* endif */
            } /* endif */
          }
          break;

        case WM_IME_STARTCOMPOSITION:
          // delete any selection prior to start with ime ...
          // this avoids problems if selection contains protected characters
          pDoc->fImeStartComposition = TRUE;         // P017862!
          if ( EQFBFuncDelSel( pDoc ) )
		  {
		    EQFBCurSegFromCursor( pDoc );
		  } /* endif */
		  pDoc->fImeStartComposition = FALSE;
          mResult = EQFBDispClass( hwnd, msg, mp1, mp2, pDoc );
          break;

        default:
          mResult = EQFBDispClass( hwnd, msg, mp1, mp2, pDoc );

    } /* switch */

    return mResult;

}

/*////////////////////////////////////////////////////////////////////
:h3.EQFBQuit  - handle close/quit document request
*/
//
// Description: this funcion will ask the user if he wants to quit
//              a document
// Flow:
//       if document was changed issue message and let user decide
//       if he wants to quit source and target doc
/////////////////////////////////////////////////////////////////////

void EQFBQuit
(
   HWND hwnd                                          // handle of tgt doc
)
{
   PTBDOCUMENT pDoc;

   // get ida of source document
   pDoc = ACCESSWNDIDA( hwnd, PTBDOCUMENT );
   EQFBFuncClose( (PSTEQFGEN)pDoc->pstEQFGen );      // shut down editor

} /* quit  */





/*////////////////////////////////////////////////////////////////////
:h3.Command - EQF Translation Browser ActionBar commands
*/
//
// Description: this funcion will contain the handling of selected
//              action bar items
// Flow:
//       Determine the selected action bar item and do the handlind:
//
//         IDM_FIND:
//         -  call the find dialog
//            if result was okay call EQFBFind to find the selected
//             item, else ignore the request
//         IDM_RFIND:
//         -  start the repeated find operation
//            if no previous find was available an error message will
//             be issued
//         IDM_OPEN
//         -  start the open dialog
//         IDM_FONTCOL:
//           only available for Read/Write docs
//         IDM_PRINT:
//         - print the current document
//         IDM_QUIT:
//         -  Quit current document
//            If doc changed, query user to cancel request
//         IDM_FILE:
//         -  save and quit current document
//         IDM_UNDO:
//         -  Undo changes to current segment
//         IDM_UNMARK:
//         -  unmark any avialable mark ( EQFBFuncMarkUnmark )
//         IDM_FINDMARK:
//         -  find the mark ( EQFBFuncMarkFind )
//         IDM_GOTOLINE:
//         -  goto the specified line
//         IDM_QUERYLINE:
//         -  query cursor position
//         IDM_TOP
//         -  move to the top of the document ( EQFBFuncTopDoc )
//         IDM_BOTTOM:
//         -  move to the bottom of the document ( EQFBFuncBottomDoc )
//         IDM_SOL   :
//         -  move to the start of line ( EQFBFuncStartLine )
//         IDM_EOL   :
//         -  move to the end of line ( EQFBFuncEndLine )
//         IDM_SOS   :
//         -  move to the start of segm ( EQFBFuncStartSeg )
//         IDM_EOS   :
//         -  move to the end of the segment ( EQFBFuncEndSeg )
//         IDM_SPLIT:
//         -  split the line at the cursor ( EQFBFuncSplitLine )
//         IDM_JOIN:
//         -  join the next line to the current line ( EQFBFuncJoinLine)
//         IDM_REFLOW:
//         -  reflow the active segment ( EQFBFuncLineWrap)
//         IDM_CYCLE:
//         -  move to the next document in the ring
//         IDM_CUT:
//         -  Copy marked area to clipboard and clear it in document
//         IDM_COPY:
//         -  Copy marked area to clipboard
//         IDM_CLEAR:
//         -  delete marked area ( EQFBFuncMarkDelete )
//         IDM_PASTE:
//         -  Paste contents from clipboard at cursor position
//         IDM_TRANSSEG:
//         -  translate the segment ( EQFBTSeg )
//         IDM_TRANSNEW:
//         -  position at the next untransl.
//         IDM_UNTRANS:
//         -  untranslate the segment ( EQFBFuncUnTrans )
//         IDM_DICTLOOK:
//         -  activate dictionary lookup with the marked area or the
//            cursor word ( EQFDICTLOOK )
//         IDM_EDITTERM:
//         -  edit   term in dictionary
//         IDM_ADDABBREV:
//         -  add an abbreviation to system morphology
//         IDM_TRANSWND:
//         -  activate translation window
//         IDM_DICTWND:
//         -  activate dictionary window ( EQFBActDict )
//         IDM_TMWND:
//         -  activate translation mem window ( EQFBActProp )
//         IDM_DISPORG:
//         -  display the orignal synchronized and activate it
//         IDM_SRCPROPWND:
//         -  display the source of the proposal
//         IDM_GOTO:
//         -  goto the active segment ( EQFBGotoActSegment )
//         IDM_JOINSEG:
//         -  join segments
//         IDM_SPLITSEG:
//         -  split previously joined segments
//         IDM_MARKSEG:
//         -  mark active segment
//         IDM_GOTOMARK:
//         -  goto marked segment
//         IDM_CLEARMARK:
//         -  clear any available mark
//         IDM_POSTEDIT:
//         -  switch to postediting mode ( EQFBSetPostEdit )
//         IDM_AUTOTRANS:
//         -  switch to automatic translation ( EQFBAutoTranslate )
//         IDM_PROOFSEG:
//         - spellcheck current segment
//         IDM_PROOFALL:
//         - spellcheck all translated segments
//         IDM_AUTOSPELL:
//         - automatically spellcheck
//         IDM_NEXTMISSPELL:
//         - goto next misspelled word
//         IDM_HIDE:
//         -  hide the tagging information
//         IDM_PROTECTED:
//         -  protect the tagging information
//         IDM_UNPROTECTED:
//         -  allow change of all information
//         IDM_SHRINK:
//         -  shrink NOP segments
//         IDM_COMPACT:
//         -  shrink NOP segments and inline segments
//         IDM_DICTCPY:
//         -  copy dictionary entries into translation
//         IDM_COPYPROPBLOCK
//         -  copy proposal block
//         PID_HELP_FOR_HELP
//         -  post the help request to the main window
//         PID_HELP_MI_TUTORIAL
//         -  invoke the tutorial for the editor
//         DEFAULT:
//         -  check if it is a proposal or dictionary request, if so copy it
//////////////////////////////////////////////////////////////////////
void EQFBWndProc_Command
(
  HWND hwnd,
  WPARAM mp1,
  LPARAM mp2
)
{
    PTBDOCUMENT pDoc;                  // pointer to document
    USHORT      usMatch;
    BOOL        fUpdate = TRUE;        // force screen update
    BOOL        fFound = FALSE;
    USHORT      usCmdId = SHORT1FROMMP1(mp1);

    mp2;                               // get rid of compiler warning

    pDoc = ACCESSWNDIDA( hwnd, PTBDOCUMENT );

    switch ( usCmdId )
    {

      case IDM_FIND:
        EQFBFuncFind( pDoc );
        break;

      case IDM_CFIND:
        EQFBFuncCFind( pDoc );
        break;

      case IDM_RFIND:
        break;

      case IDM_OPEN:
        EQFBFuncOpen( pDoc );
        break;

      case IDM_FONTCOL:
        EQFBFuncFonts( pDoc );
        break;

      case IDM_KEYS:
        EQFBFuncKeys( pDoc );
        break;

      case IDM_ENTRY:
        EQFBFuncEntry( pDoc );
        break;

      case IDM_COMMAND:
        EQFBCommand( pDoc );
        fUpdate = FALSE;               // no refresh necessary any more
        break;


      case IDM_SETTINGS:
        EQFBSettings( pDoc );
        break;

      case IDM_FONTSIZE:
        EQFBFuncFontSize( pDoc );
        break;

      case IDM_SAVE:
        EQFBFuncSave( pDoc );             // Save current document. Stay
        break;
#ifdef _PTM
      case IDM_SAVEAS:
        EQFBFuncSaveAs( pDoc );           // Save current document external.Stay
        break;
#endif
      case IDM_PRINT:
        EQFBDocPrint( pDoc );             // Print current document
        break;

      case IDM_QUIT:      // Quit current document - same behaviour as CLOSE
        switch ( pDoc->docType )
        {
           case SSOURCE_DOC:
              WinShowWindow( pDoc->hwndFrame , FALSE );
                 SendMessage( ((PSTEQFGEN)pDoc->pstEQFGen)->hwndTWBS,
                              WM_EQF_SETFOCUS,
                              0, MP2FROMP( pDoc->twin->hwndFrame ));
              break;
           case STARGET_DOC:
              EQFBQuit (hwnd);                       // quit source doc
              fUpdate = FALSE;
              break;
           case OTHER_DOC:
              pDoc = EQFBFuncQuit( pDoc );           // quit document
              break;
           case TRNOTE_DOC:
              WinPostMsg( hwnd, WM_CLOSE, 0, 0L );
              break;
        } /* endswitch */
        break;

      case IDM_REIMPORT:
        EQFBFuncReImportDoc( pDoc ); 
        fUpdate = FALSE;
        break;

      case IDM_FILE:                       // save target
        WinPostMsg( hwnd, WM_CLOSE, 0, 0L );
        fUpdate = FALSE;
        break;

      case IDM_UNDO:
        EQFBFuncUndo ( pDoc );          // Undo changes to current line
        break;

      case IDM_UNMARK:
        EQFBFuncMarkClear( pDoc );      // unmark
        break;

      case IDM_FINDMARK:
        EQFBFuncMarkFind( pDoc );      // find the mark
        break;

      case IDM_GOTOLINE:
        EQFBGotoLine( pDoc );          // goto the specified line
        break;

      case IDM_GOTOSEG:
        EQFBGotoSegment( pDoc );       // goto the specified segment
        break;

      case IDM_TOCGOTO:
        EQFBFuncTOCGoto( pDoc );       // goto the specified table of contents entry
        break;

      case IDM_GOUPDSEGMENT:
        EQFBFuncGoUpdSegment( pDoc );       // goto the specified table of contents entry
        break;

      case IDM_QUERYLINE:
        EQFBQueryLine( pDoc );         // find the cursor line in the file
        break;

      case IDM_TOP   :
        EQFBFuncTopDoc( pDoc );        // move to the top of the document
        break;

      case IDM_BOTTOM:
        EQFBFuncBottomDoc( pDoc );           // move to the bottom of the document
        break;

      case IDM_SOL   :
        EQFBFuncStartLine( pDoc );           // move to the start of line
        break;

      case IDM_EOL   :
        EQFBFuncEndLine( pDoc );             // move to the end of line
        break;

      case IDM_SOS   :
        EQFBFuncStartSeg ( pDoc );           // move to the start of segment
        break;

      case IDM_EOS   :
        EQFBFuncEndSeg ( pDoc );             // move to the end of segment
        break;

      case IDM_SPLIT:
        EQFBFuncSplitLine( pDoc );           // split the line at the cursor
        break;

      case IDM_JOIN:
        EQFBFuncJoinLine( pDoc );            // join next line with current one
        break;

      case IDM_REFLOW:
        EQFBFuncLineWrap( pDoc );            // reflow the current segment
        break;

      case IDM_WRAP:
        EQFBFuncToggleLineWrap( pDoc );
        break;

      case IDM_CYCLE:
        EQFBFuncNextDoc( pDoc );          // Next document
        break;

      case IDM_CUT:                       // Copy to clipboard and cut
        EQFBFuncCutToClip( pDoc );
        break;

      case IDM_COPY:                      // Copy to clipboard
        EQFBFuncCopyToClip( pDoc );
        break;

      case IDM_CLEAR:
        EQFBFuncMarkDelete( pDoc );       // Clear mark
        break;

      case IDM_PASTE:                     // paste from clipboard
        EQFBFuncPasteFromClip( pDoc );
        break;

      case IDM_COPYPROPBLOCK:             // copy proposal block
        EQFBFuncPropMarkCopy( pDoc );
        break;

     case IDM_TRANSSEG:                   // translate the segment
        EQFBTSeg( pDoc );
        break;
     case IDM_TRANSNEW:                   // position at the next untransl.
        EQFBTSegNext( pDoc );
        break;
     case IDM_TRANSNEW_EXACT:             // position at the next untransl. with EXACT matches
        EQFBTSegNextExact( pDoc );
        break;
     case IDM_TRANSNEW_FUZZY:             // position at the next untransl. with FUZZY matches
        EQFBTSegNextFuzzy( pDoc );
        break;
     case IDM_TRANSNEW_NONE:              // position at the next untransl. with NO matches
        EQFBTSegNextNone( pDoc );
        break;
     case IDM_TRANSNEW_MT:                // position at the next untransl. with MT matches
        EQFBTSegNextMT( pDoc );
        break;
     case IDM_TRANSNEW_GLOBAL:            // position at the next untransl. with GLOBAL MEMORY matches
        EQFBTSegNextGlobal( pDoc );
        break;
     case IDM_UNTRANS:                    // untranslate the active segment
        EQFBTUnTrans( pDoc );
        break;
     case IDM_DICTLOOK:                   // activate dictionary lookup
        EQFBDictLook( pDoc );
        break;
     case IDM_EDITTERM:                   // activate dictionary edit
        EQFBFuncEditTerm( pDoc );
        break;
     case IDM_ADDABBREV:                  // add abbreviation to abbrev.list
        EQFBAddAbbrev( pDoc );
        break;
     case IDM_EDITABBREV:                 // edit abbreviations
        EQFBEditAbbrev( pDoc );
        break;
     case IDM_EDITADD:                     // edit addenda terms
        EQFBEditAddenda( pDoc );
        break;
     case IDM_TRANSWND:                   // activate translation window
        EQFBActTrans( pDoc );
        break;
     case IDM_DICTWND:                    // activate dictionary window
        EQFBActDict( pDoc );
        break;
     case IDM_TMWND:                      // activate translation mem window
        EQFBActProp( pDoc );
        break;
     case IDM_DISPORG:                    // display the original synchronized
        EQFBFuncDispOrg( pDoc );
        break;
     case IDM_SRCPROPWND:                 // display the original synchronized
        EQFBFuncDispSrcProp (pDoc);
        break;
     case IDM_SEGPROPWND:                 // display the segment property window
        EQFBFuncDispSegProp (pDoc);
        break;
     case IDM_GOTO:                       // goto the active segment
        EQFBGotoActSegment( pDoc );
        break;
     case IDM_JOINSEG:                    // join segments
        EQFBJoinSeg( pDoc );
        break;
     case IDM_SPLITSEG:                   // split segments
        EQFBSplitSeg( pDoc );
        break;
     case IDM_MARKSEG:                    // mark active segment
        EQFBMark( pDoc );
        break;
     case IDM_GOTOMARK:                   // goto marked segment
        EQFBFindMark( pDoc );
        break;
     case IDM_CLEARMARK:                  // clear any available mark
        EQFBClearMark( pDoc );
        break;
     case IDM_POSTEDIT:                   // switch to postediting mode
        EQFBSetPostEdit( pDoc );
        break;
     case IDM_AUTOTRANS:                  // switch to automatic translation
        EQFBAutoTranslate( pDoc );
        break;
     case IDM_SHOWTRANS:                  // show translation preview
        EQFBFuncShowTrans( pDoc );
        break;
     case IDM_VISIBLESPACE:
        EQFBFuncVisibleSpace(pDoc );      // toggle visible whitespace on/off
        break;
     case IDM_PROOFSEG:                   // spellcheck of current segment
        EQFBFuncSpellSeg(pDoc);
        break;
     case IDM_PROOFALL:                   // spellcheck for file
        EQFBFuncSpellFile(pDoc);
        fUpdate = FALSE;
        break;
     case IDM_AUTOSPELL:                   // autospellcheck on/off
        EQFBFuncSpellAuto(pDoc);
        EQFBWriteProfile(pDoc);
        break;
     case IDM_NEXTMISSPELL:                 // goto next misspelled
        EQFBFuncNextMisspelled(pDoc);
        break;
     case IDM_PROTECTED:                  // switch to protected mode
        EQFBChangeStyle( pDoc,  DISP_PROTECTED);
        break;
     case IDM_HIDE:                       // switch to hided mode
        EQFBChangeStyle( pDoc,  DISP_HIDE);
        break;
     case IDM_UNPROTECTED:                // switch to unprotected mode
        EQFBChangeStyle( pDoc,  DISP_UNPROTECTED);
        break;
     case IDM_SHRINK:                     // switch to shrink style
        EQFBChangeStyle( pDoc,  DISP_SHRINK);
        break;
     case IDM_COMPACT:                    // switch to compact style
        EQFBChangeStyle( pDoc,  DISP_COMPACT);
        break;
     case IDM_SHORTEN:                    // switch to shorten style
        EQFBChangeStyle( pDoc,  DISP_SHORTEN);
        break;
     case IDM_SC_HORZ:
       EQFBSetResetFrameCtrls( pDoc, hwnd, FCF_HORZSCROLL );
       break;
     case IDM_SC_VERT:
       EQFBSetResetFrameCtrls( pDoc, hwnd, FCF_VERTSCROLL );
       break;
     case IDM_SC_TITLE:
         EQFBSetResetFrameCtrls( pDoc, hwnd, (FCF_TITLEBAR | FCF_SYSMENU) );
       break;

     case IDM_SC_STATUS:
       {
         BOOL fShowWindow = !WinIsWindowVisible( pDoc->hStatusBarWnd );
         BYTE bStatus = ((PSTEQFGEN)pDoc->pstEQFGen)->bStatusBar[pDoc->docType];
         if ( !fShowWindow )
         {
           WinShowWindow( pDoc->hStatusBarWnd, FALSE );
           bStatus &= ~TP_WND_STATUSBAR;
         }
         else
         {
           bStatus |= TP_WND_STATUSBAR;
         } /* endif */
         /*************************************************************/
         /* store settings for next load                              */
         /*************************************************************/
         ((PSTEQFGEN)pDoc->pstEQFGen)->bStatusBar[pDoc->docType] = bStatus;
         /********************************************************************/
         /* force the update of the window ...                               */
         /********************************************************************/
         {
           RECT  rc;
           GetClientRect(hwnd, &rc);
           PostMessage( hwnd, WM_SIZE,
                        0, MAKELPARAM( rc.right, rc.bottom ));
         }
       }
       break;

     case IDM_SC_RULER:
       {
         BOOL fShowWindow = !WinIsWindowVisible( pDoc->hRulerWnd );
         BYTE bStatus = ((PSTEQFGEN)pDoc->pstEQFGen)->bStatusBar[pDoc->docType];
         if ( !fShowWindow )
         {
           WinShowWindow( pDoc->hRulerWnd, FALSE );
           bStatus &= ~TP_WND_RULER;
           pDoc->ulRulerSize = 0;
         }
         else
         {
           WinShowWindow( pDoc->hRulerWnd, TRUE );
           bStatus |= TP_WND_RULER;
           pDoc->ulRulerSize = pDoc->cy;
         } /* endif */
         /*************************************************************/
         /* store settings for next load                              */
         /*************************************************************/
         ((PSTEQFGEN)pDoc->pstEQFGen)->bStatusBar[pDoc->docType] = bStatus;
         /********************************************************************/
         /* force the update of the window ...                               */
         /********************************************************************/
         {
           RECT  rc;
           GetClientRect(hwnd, &rc);
           PostMessage( hwnd, WM_SIZE,
                        0, MAKELPARAM( rc.right, rc.bottom ));
         }
       }
       break;

     case PID_SYS_CLOSE:
       WinPostMsg( hwnd, WM_SYSCOMMAND, SC_CLOSE, NULL );
       break;
     case PID_SYS_SIZE:
       WinSendMsg( hwnd, WM_SYSCOMMAND, SC_SIZE, NULL );
       break;
     case PID_SYS_MOVE:
       WinSendMsg( hwnd, WM_SYSCOMMAND, SC_MOVE, NULL );
       break;
     case IDM_HOTPOPUP:
       {
         POINT  lppt;
         SHORT  sPopUpId;
         GetCaretPos( &lppt );
         /********************************************************/
         /* find correct popup ...                               */
         /********************************************************/
         switch ( pDoc->docType )
         {
           case TRNOTE_DOC:       // TRNOTE document
             sPopUpId = ID_TPRO_POPUP_TRNOTE;
             break;
           case OTHER_DOC:        // other documents besides ....
             sPopUpId = (pDoc->hwndRichEdit)? ID_TRTF_POPUP_OTHER : ID_TPRO_POPUP_OTHER;
             break;
           case SSOURCE_DOC:      // source segmented document
             sPopUpId = (pDoc->hwndRichEdit)? ID_TRTF_POPUP_SRC : ID_TPRO_POPUP_SRC;
             break;
           case STARGET_DOC:      // target segmented document
             sPopUpId = (pDoc->hwndRichEdit)? ID_TRTF_POPUP_TRANS : ID_TPRO_POPUP_TRANS;
             break;
           case SERVPROP_DOC:     // proposal window
             sPopUpId = ID_SRV_POPUP_TM;
             break;
           case SERVDICT_DOC:     // dictionary window
             sPopUpId = ID_SRV_POPUP_DCT;
             break;
           case SERVSOURCE_DOC:   // source window for proposals
             sPopUpId = ID_SRV_POPUP_SRCP;
             break;
           default:
             sPopUpId = 0;
             break;
         } /* endswitch */
         if (sPopUpId)
         {
           HandlePopupTEMenu( hwnd, lppt, sPopUpId, pDoc );
         } /* endif */
       }
       break;

     default:
        usMatch = usCmdId;

        if ( (usMatch >= IDM_DICTCPY)  && (usMatch < (IDM_DICTCPY+26)) )
        {
          pDoc->usChar = usMatch - IDM_DICTCPY + CHARACTER_A ;
          EQFBGetDictMatch( pDoc );
        }
        else if ( (usMatch >= IDM_DICTCPY+26) && (usMatch <= (IDM_DICTCPY+51)) )
        {
          /************************************************************/
          /* force for IDM_DICTCPY+27 usChar = A and so on            */
          /************************************************************/
          pDoc->usChar = usMatch -(IDM_DICTCPY+26) + 'A' ;
          EQFBGetDictMatch( pDoc );
        }
        else if ( (usMatch >= IDM_PROPCPY)  && (usMatch <= IDM_PROPCPY+10) )
        {
          pDoc->usChar = (usMatch - IDM_PROPCPY) + CHARACTER_0;
          EQFBGetPropMatch( pDoc );
        }
        else if ( (usMatch >= IDM_OTHERWND) && (usMatch <= IDM_LASTOTHER) )
        {
          while ( !fFound )
          {
            pDoc = pDoc->next;
            if (((pDoc->docType == OTHER_DOC) || (pDoc->docType == TRNOTE_DOC))
                 && ( pDoc->usWndId == usMatch )  )
            {
               fFound = TRUE;
            } /* endif */
          } /* endwhile */
          /********************************************************/
          /* user wants to go to this document                    */
          /********************************************************/
          pDoc->Redraw |= REDRAW_ALL;
                                      // activate document window
          WinShowWindow( pDoc->hwndFrame, TRUE );

          WinSetActiveWindow( HWND_DESKTOP, pDoc->hwndFrame );
        }
        else if ( (usMatch >= IDM_DOC_ENVIRONMENT) &&
                  (usMatch <= IDM_LASTDOC_ENVIRONMENT) )
        {
          PSZ pOutDocObjName = NULL;

          if ( UtlAlloc( (PVOID *)&pOutDocObjName, 0L, (LONG)MAX_LONGPATH, NOMSG ) )
          {
            USHORT usRc;
            usRc = EQF_XDOCNUM( (PSTEQFGEN)pDoc->pstEQFGen, (USHORT)(usMatch-IDM_DOC_ENVIRONMENT),
                                       pOutDocObjName );
            if ( !usRc && *pOutDocObjName )
            {
              usRc = EQFBTenvStart( pDoc, pOutDocObjName, (PSTEQFGEN)pDoc->pstEQFGen );
            } /* endif */
            UtlAlloc( (PVOID *)&pOutDocObjName, 0L, 0L, NOMSG );
          } /* endif */
        }
        else
        {
          /************************************************************/
          /* Should not happen under OS/2, but under Windows we have  */
          /* to pass those messages to the default procedure....      */
          /************************************************************/
          DefWindowProc( hwnd, WM_COMMAND, mp1, mp2 );
        } /* endif */
        break;
    } /* command switch */
    /******************************************************************/
    /* refresh the screen only if necessary ....                      */
    /******************************************************************/
    if ( fUpdate )
    {
      EQFBRefreshScreen( pDoc );             // refresh the screen
    } /* endif */
} /* Command  */

/*////////////////////////////////////////////////////////////////////
:h3.EQFBGenProcessing - EQF Translation Browser ActionBar commands
*/
//
// Description: this funcion will contain the handling of selected
//              action bar items
//////////////////////////////////////////////////////////////////////
VOID EQFBGenProcessing
(
    HWND        hwndParent,            // handle of parent
    PTBDOCUMENT pDoc,                  // pointer to document
    USHORT      usCmdId                // id of request
)
{
    BOOL        fUpdate = TRUE;        // force screen update

    switch ( usCmdId )
    {

      case IDM_FIND:
        EQFBFuncFind( pDoc );
        break;

      case IDM_CFIND:
        EQFBFuncCFind( pDoc );
        break;

      case IDM_RFIND:
        break;

      case IDM_OPEN:
        EQFBFuncOpen( pDoc );
        break;

      case IDM_FONTCOL:
        EQFBFuncFonts( pDoc );
        break;

      case IDM_KEYS:
        EQFBFuncKeys( pDoc );
        break;

      case IDM_ENTRY:
        EQFBFuncEntry( pDoc );
        break;

      case IDM_COMMAND:
        EQFBCommand( pDoc );
        fUpdate = FALSE;               // no refresh necessary any more
        break;

      case IDM_SETTINGS:
        EQFBSettings( pDoc );
        break;

      case IDM_FONTSIZE:
        EQFBFuncFontSize( pDoc );
        break;

      case IDM_SAVE:
        EQFBFuncSave( pDoc );             // Save current document. Stay
        break;

//      case IDM_SAVEAS:
//        EQFBFuncSaveAs( pDoc );           // Save current document external.Stay
//        break;

      case IDM_PRINT:
        EQFBDocPrint( pDoc );             // Print current document
        break;

//      case IDM_QUIT:      // Quit current document - same behaviour as CLOSE
//        EQFBQuit (hwnd);                       // quit source doc
//              pDoc = EQFBFuncQuit( pDoc );           // quit document
//        break;

      case IDM_REIMPORT:
        EQFBFuncReImportDoc( pDoc );             // Save current document. Stay
        break;

      case IDM_FILE:                       // save target
        WinPostMsg( hwndParent, WM_CLOSE, 0, 0L );
        fUpdate = FALSE;
        break;

      case IDM_UNDO:
        EQFBFuncUndo ( pDoc );          // Undo changes to current line
        break;

      case IDM_UNMARK:
        EQFBFuncMarkClear( pDoc );      // unmark
        break;

      case IDM_FINDMARK:
        EQFBFuncMarkFind( pDoc );      // find the mark
        break;

      case IDM_GOTOLINE:
        EQFBGotoLine( pDoc );          // goto the specified line
        break;

      case IDM_GOTOSEG:
        EQFBGotoSegment( pDoc );       // goto the specified segment
        break;

      case IDM_TOCGOTO:
        EQFBFuncTOCGoto( pDoc );       // goto the specified table of contents entry
        break;
      case IDM_GOUPDSEGMENT:
        EQFBFuncGoUpdSegment( pDoc );       // goto the specified table of contents entry
        break;

      case IDM_QUERYLINE:
        EQFBQueryLine( pDoc );         // find the cursor line in the file
        break;

      case IDM_TOP   :
        EQFBFuncTopDoc( pDoc );        // move to the top of the document
        break;

      case IDM_BOTTOM:
        EQFBFuncBottomDoc( pDoc );           // move to the bottom of the document
        break;

      case IDM_SOL   :
        EQFBFuncStartLine( pDoc );           // move to the start of line
        break;

      case IDM_EOL   :
        EQFBFuncEndLine( pDoc );             // move to the end of line
        break;

      case IDM_SOS   :
        EQFBFuncStartSeg ( pDoc );           // move to the start of segment
        break;

      case IDM_EOS   :
        EQFBFuncEndSeg ( pDoc );             // move to the end of segment
        break;

      case IDM_SPLIT:
        EQFBFuncSplitLine( pDoc );           // split the line at the cursor
        break;

      case IDM_JOIN:
        EQFBFuncJoinLine( pDoc );            // join next line with current one
        break;

      case IDM_REFLOW:
        EQFBFuncLineWrap( pDoc );            // reflow the current segment
        break;

      case IDM_WRAP:
        EQFBFuncMarginAct( pDoc );            // activate/deactivate linewrap
        if (pDoc->pUndoSegW && pDoc->fAutoLineWrap && !pDoc->fLineWrap)
        {
          USHORT usBufSize = 0;
          USHORT usOffset = 0;
          EQFBBufRemoveSoftLF(pDoc->hwndRichEdit, pDoc->pUndoSegW,&usBufSize, &usOffset );
        }
        EQFBWriteProfile( pDoc );
        EQFBGotoSeg( pDoc, pDoc->TBCursor.ulSegNum,
                           pDoc->TBCursor.usSegOffset); // reset cursor on current position
        break;

      case IDM_CYCLE:
        EQFBFuncNextDoc( pDoc );          // Next document
        break;

      case IDM_CUT:                       // Copy to clipboard and cut
        {
          DOCTYPE usDoc = pDoc->docType;
          pDoc->docType = STARGET_DOC;
          EQFBFuncCutToClip( pDoc );
          pDoc->docType = usDoc;
        }
        break;

      case IDM_COPY:                      // Copy to clipboard
        EQFBFuncCopyToClip( pDoc );
        break;

      case IDM_CLEAR:
        EQFBFuncMarkDelete( pDoc );       // Clear mark
        break;

      case IDM_PASTE:                     // paste from clipboard
        EQFBFuncPasteFromClip( pDoc );
        break;
      case IDM_COPYPROPBLOCK:             // copy proposal block
        EQFBFuncPropMarkCopy( pDoc );
        break;

     case IDM_PROTECTED:                  // switch to protected mode
        EQFBChangeStyle( pDoc,  DISP_PROTECTED);
        break;
     case IDM_HIDE:                       // switch to hided mode
        EQFBChangeStyle( pDoc,  DISP_HIDE);
        break;
     case IDM_UNPROTECTED:                // switch to unprotected mode
        EQFBChangeStyle( pDoc,  DISP_UNPROTECTED);
        break;
     case IDM_SHRINK:                     // switch to shrink style
        EQFBChangeStyle( pDoc,  DISP_SHRINK);
        break;
     case IDM_COMPACT:                    // switch to compact style
        EQFBChangeStyle( pDoc,  DISP_COMPACT);
        break;
     case IDM_SHORTEN:                    // switch to shorten style
        EQFBChangeStyle( pDoc,  DISP_SHORTEN);
        break;

     case IDM_HOTPOPUP:
       {
         POINT  lppt;
         SHORT  sPopUpId;
         GetCaretPos( &lppt );
         /********************************************************/
         /* find correct popup ...                               */
         /********************************************************/
         switch ( pDoc->docType )
         {
           case TRNOTE_DOC:       // TRNOTE document
             sPopUpId = ID_TPRO_POPUP_TRNOTE;
             break;
           case OTHER_DOC:        // other documents besides ....
             sPopUpId = (pDoc->hwndRichEdit)? ID_TRTF_POPUP_OTHER : ID_TPRO_POPUP_OTHER;
             break;
           case SSOURCE_DOC:      // source segmented document
             sPopUpId = (pDoc->hwndRichEdit)? ID_TRTF_POPUP_SRC : ID_TPRO_POPUP_SRC;
             break;
           case STARGET_DOC:      // target segmented document
             sPopUpId = (pDoc->hwndRichEdit)? ID_TRTF_POPUP_TRANS : ID_TPRO_POPUP_TRANS;
             break;
           case SERVPROP_DOC:     // proposal window
             sPopUpId = ID_SRV_POPUP_TM;
             break;
           case SERVDICT_DOC:     // dictionary window
             sPopUpId = ID_SRV_POPUP_DCT;
             break;
           case SERVSOURCE_DOC:   // source window for proposals
             sPopUpId = ID_SRV_POPUP_SRCP;
             break;
           default:
             sPopUpId = 0;
             break;
         } /* endswitch */
         if (sPopUpId)
         {
           HandlePopupTEMenu( hwndParent, lppt, sPopUpId, pDoc );
         } /* endif */
       }
       break;

    } /* command switch */
    /******************************************************************/
    /* refresh the screen only if necessary ....                      */
    /******************************************************************/
    if ( fUpdate )
    {
      EQFBRefreshScreen( pDoc );             // refresh the screen
    } /* endif */
} /* Command  */

/**********************************************************************/
/* Update the frame ctrls as requested by the user ....               */
/**********************************************************************/
void
EQFBSetResetFrameCtrls
(
  PTBDOCUMENT pDoc,
  HWND hwnd,
  ULONG ulStyle
)
{
  ULONG ulCurStyle;
  ulCurStyle = STYLEFROMWINDOW( hwnd );

  /********************************************************************/
  /* if style already set, toggle it, otherwise set it...             */
  /********************************************************************/
  if ( ulCurStyle & ulStyle )
  {
    ulCurStyle &= ~ulStyle;
  }
  else
  {
    ulCurStyle |= ulStyle;
  } /* endif */

  SETWINDOWSTYLE( hwnd, ulCurStyle );
  /********************************************************************/
  /* save changes for next time                                       */
  /********************************************************************/
  ulCurStyle &= AVAILSTYLES;
  switch ( pDoc->docType )
  {
    case SERVDICT_DOC:
      ((PSTEQFGEN)pDoc->pstEQFGen)->flDictStyle = ulCurStyle;
      break;
    case SERVPROP_DOC:
      ((PSTEQFGEN)pDoc->pstEQFGen)->flPropStyle = ulCurStyle;
      break;
    case SERVSOURCE_DOC:
      ((PSTEQFGEN)pDoc->pstEQFGen)->flSrcStyle = ulCurStyle;
      break;

    case OTHER_DOC:
    case TRNOTE_DOC:
      ((PSTEQFGEN)pDoc->pstEQFGen)->flEditOtherStyle = ulCurStyle;
      break;
    case SSOURCE_DOC:
      ((PSTEQFGEN)pDoc->pstEQFGen)->flEditSrcStyle = ulCurStyle;
      break;
    case STARGET_DOC:
      ((PSTEQFGEN)pDoc->pstEQFGen)->flEditTgtStyle = ulCurStyle;
      break;
  } /* endswitch */

  /********************************************************************/
  /* force the update of the window ...                               */
  /********************************************************************/
  SetWindowPos( hwnd, NULL, 0,0,0,0,
                SWP_DRAWFRAME | SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER);
  {
    RECT  rc;
    GetClientRect(hwnd, &rc);
    PostMessage( hwnd, WM_SIZE,
                 0, MAKELPARAM( rc.right, rc.bottom ));
  }
}

/**********************************************************************/
/* Function for popup menus                                           */
/**********************************************************************/
void HandlePopupTEMenu( HWND hwnd, POINT point, SHORT sMenuID, PTBDOCUMENT pDoc)
{
  HMENU hMenu;
  HMENU hMenuTrackPopup;
  ULONG ulFrameFlags;
  HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);


  /* Get the menu for the popup from the resource file. */
  hMenu = LoadMenu( hResMod, MAKEINTRESOURCE( ID_POPUP_MENU ) );
  if ( !hMenu )
      return;

  hMenuTrackPopup = GetSubMenu( hMenu, sMenuID );

  /********************************************************************/
  /* set checkmarks for FrameCtrls...                                 */
  /********************************************************************/
  ulFrameFlags = GetWindowLong( hwnd, GWL_STYLE );

  SETAABITEMCHECK( hMenuTrackPopup, IDM_SC_HORZ, (ulFrameFlags & WS_HSCROLL) );
  SETAABITEMCHECK( hMenuTrackPopup, IDM_SC_VERT, (ulFrameFlags & WS_VSCROLL) );
  SETAABITEMCHECK( hMenuTrackPopup, IDM_SC_TITLE, (ulFrameFlags & WS_CAPTION) );
  SETAABITEMCHECK( hMenuTrackPopup, IDM_SC_STATUS,
                   WinIsWindowVisible( pDoc->hStatusBarWnd ) );
  SETAABITEMCHECK( hMenuTrackPopup, IDM_SC_RULER,
                   WinIsWindowVisible( pDoc->hRulerWnd ) );

  if ( pDoc->pBlockMark )
  {
    SETAABITEM( hMenuTrackPopup, IDM_UNMARK,
                ((PEQFBBLOCK)pDoc->pBlockMark)->pDoc );
  } /* endif */

  /* Convert the mouse point to screen coordinates since that is what
   * TrackPopup expects.
   */
  ClientToScreen( hwnd, (LPPOINT)&point );

  /* Draw and track the "floating" popup */
  TrackPopupMenu( hMenuTrackPopup, 0, point.x, point.y, 0,
                  (HWND)UtlQueryULong( QL_TWBFRAME ), NULL );

  /* Destroy the menu since were are done with it. */
  DestroyMenu( hMenu );
}



/*//////////////////////////////////////////////////////////////
:h3.EQFBDocWndCreate ( PTBDOCUMENT,PLOADSTRUCT)
*///////////////////////////////////////////////////////////////
// Description:
// Flow: - create standard window
//       if create is ok:
//       - Get screen device context
//       - Get a micro PS for clipping
//       - Get an AVIO PS for editor use
//       - Get cell size of AVIO char
//       - Get length of video buffer
//       else: warning message
//
// Parameters:
//  PTBDOCUMENT   - pointer to document ida
//  PLOADSTRUCT   - pointer to load structure
//
////////////////////////////////////////////////////////////////
BOOL EQFBDocWndCreate ( PTBDOCUMENT pNewdoc,   // pointer to document structure
                        PLOADSTRUCT pLoad )  // pointer to load structure
{
    HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);

    BOOL  fOK = TRUE;                         // success indicator

    HAB       hab;                            // anchor block handle
    CHAR      chDesc[ MAX_DESCRIPTION+1 ];        // title text
    USHORT    usWinId = 0;                    // window help id/resource id
    PDOCUMENT_IDA  pIda = NULL;               // pointer to document struct.
    PSTEQFGEN pstEQFGen = NULL;               // generic structure

   hab = WinQueryAnchorBlock( pNewdoc->hwndClient );
                                          // prepare title bar for windows
   switch ( pLoad->docType )
   {
      case SSOURCE_DOC:                   // source
         WinLoadString( hab, hResMod, IDS_ORIGINAL,
                        sizeof(chDesc) - 1, chDesc );
         usWinId = ID_TWBS_ORIG_WINDOW;
         break;
      case STARGET_DOC:                   // translation
         WinLoadString( hab, hResMod, IDS_TRANS,
                        sizeof(chDesc) - 1, chDesc );
         usWinId = ID_TWBS_SOURCE_WINDOW;
         break;
      case OTHER_DOC :                    // titlebar for other docs
         strncpy( chDesc, pLoad->pFileName, MAX_DESCRIPTION );
         chDesc[MAX_DESCRIPTION-1] = EOS;
         usWinId = ID_TWBS_OTHER_WINDOW;
         break;
      case TRNOTE_DOC:                   // source
         WinLoadString( hab, hResMod, SID_TRNOTE_TITLE,
                        sizeof(chDesc) - 1, chDesc );
         usWinId = ID_TWBS_TRNOTE_WINDOW;
         break;
   } /* endswitch */
   /*******************************************************************/
   /* load string for loading document                                */
   /*******************************************************************/
   WinLoadString( hab, hResMod, IDS_TB_STB_LOADING,
                  sizeof( pNewdoc->chLoading)-1, pNewdoc->chLoading );

   if ((usWinId == ID_TWBS_OTHER_WINDOW) || (usWinId == ID_TWBS_TRNOTE_WINDOW))
   {
     strcpy( pNewdoc->chTitle, chDesc );
   }
   else
   {
     ULONG    ulLength;
     PSZ      pData[1];                // pointer to data

     if (pLoad->pstEQFGen && ((PSTEQFGEN)pLoad->pstEQFGen)->szLongName[0])
     {
       pData[0] = (PSZ)((PSTEQFGEN)pLoad->pstEQFGen)->szLongName;
     }
     else
     {
       pData[0] = UtlGetFnameFromPath( pLoad->pFileName );
     } /* endif */


     OEMTOANSI( pData[0] );
     DosInsMessage( &pData[0], 1,
                    chDesc, sizeof( chDesc ) - 1,
                    pNewdoc->chTitle, sizeof( pNewdoc->chTitle ) -1,
                    &ulLength );
     ANSITOOEM( pData[0] );
   } /* endif */

   {
     /******************************************************************/
     /* do special handling for some of the languages like Thai, Bidi  */
     /******************************************************************/
     pNewdoc->docType = pLoad->docType;
     pstEQFGen = (PSTEQFGEN)pNewdoc->pstEQFGen;
     pIda = (PDOCUMENT_IDA)(pstEQFGen->pDoc);
     EQFBDocSetCodePage(pNewdoc, pIda);

     if (IS_RTL(pNewdoc))
     {
       pIda = (PDOCUMENT_IDA)(((PSTEQFGEN)pNewdoc->pstEQFGen)->pDoc);
       SwitchKeyboardForSpecLangs( pIda );
     }

     if ( pLoad->usEditor == RTFEDIT_EDITOR )
     {
          CHAR_W chTitleW[256];
          PSZ_W  pTitleW = UtlDirectAnsi2Unicode( pNewdoc->chTitle, chTitleW, 0 );

          pNewdoc->hwndClient = pNewdoc->hwndFrame =
         CreateWindowExW( 0, //WS_EX_RTLREADING,
                          L"EQFBRTF",
                          pTitleW, //pNewdoc->chTitle,    //window caption
                          WS_CHILD |
                          WS_CLIPSIBLINGS |
                          pLoad->flFrameStyle,
                          CW_USEDEFAULT,
                          CW_USEDEFAULT,
                          CW_USEDEFAULT,
                          CW_USEDEFAULT,
                          pLoad->hwndParent, //parent wnd handle
                          NULL,
                          (HINSTANCE)hab,               //program instance handle
                          NULL );
     }
     else
     {
            CHAR_W chTitleW[256];
            PSZ_W  pTitleW = UtlDirectAnsi2Unicode( pNewdoc->chTitle, chTitleW, 0 );

       pNewdoc->hwndClient = pNewdoc->hwndFrame =
           CreateWindowExW ( 0,// WS_EX_RTLREADING,
                             L"EQFB",
                             pTitleW,    //window caption
                             WS_CHILD |
                             WS_CLIPSIBLINGS |
                             pLoad->flFrameStyle,
                             CW_USEDEFAULT,
                             CW_USEDEFAULT,
                             CW_USEDEFAULT,
                             CW_USEDEFAULT,
                             pLoad->hwndParent, //parent wnd handle
                             NULL,
                             (HINSTANCE)hab,               //program instance handle
                             NULL );
       } /* endif */
     }

   if ( pNewdoc->hwndFrame )
   {
     VIOFONTCELLSIZE* pvioFontSize = get_vioFontSize();
     if ( pLoad->docType == STARGET_DOC )
     {
       ResetSpellData( pGlobalSpellData, pstEQFGen );
     } /* endif */
      ANCHORWNDIDA( pNewdoc->hwndFrame, pNewdoc );
      ANCHORWNDIDA( pNewdoc->hwndClient, pNewdoc );


     if ( pLoad->usEditor == RTFEDIT_EDITOR )
     {
       /***************************************************************/
       /* Create a richedit control for the window                   */
       /***************************************************************/
       fOK = EQFBCreateRichEditCtrl( pNewdoc, EQFBWNDPROCRTFCTRL );
     } /* endif */


      EQFBSetNewCellSize( pNewdoc,
//                         vioFontSize[ pLoad->docType ].cx,
//                         vioFontSize[ pLoad->docType ].cy );
                           (pvioFontSize + pLoad->docType)->cx,
                           (pvioFontSize + pLoad->docType)->cy );

    {
      /****************************************************************/
      /* add statusbar definitions                                    */
      /****************************************************************/
      SWP     swp;
      ULONG   ulCY = WinQuerySysValue( HWND_DESKTOP, SV_CYMENU );
      WinQueryWindowPos( pNewdoc->hwndFrame, &swp );

      pNewdoc->hStatusBarWnd =
          WinCreateWindow( pNewdoc->hwndFrame, TP_STATUSBAR,
                           "",
                           WS_CHILD | 0,
                           0 , 0 , swp.cx, ulCY,
                           pNewdoc->hwndFrame, HWND_TOP,
                           ID_TP_STATUSBAR,            // idResources,
                           NULL, NULL);

      ANCHORWNDIDA( pNewdoc->hStatusBarWnd, pNewdoc );

      /****************************************************************/
      /* load resources for statusbar                                 */
      /****************************************************************/
      WinLoadString( hab, hResMod, IDS_TB_STB_COL,
                     sizeof(pNewdoc->chColStatusText) - 1,
                     pNewdoc->chColStatusText );

      WinLoadString( hab, hResMod, IDS_TB_STB_SEGMENT,
                     sizeof(pNewdoc->chSegStatusText) - 1,
                     pNewdoc->chSegStatusText );

      WinLoadString( hab, hResMod, IDS_TB_STB_LINE,
                     sizeof(pNewdoc->chLineStatusText) - 1,
                     pNewdoc->chLineStatusText );

      WinLoadString( hab, hResMod, IDS_TB_STB_INSERT,
                     sizeof(pNewdoc->chInsStatusText) - 1,
                     pNewdoc->chInsStatusText );

    }

    {
      /****************************************************************/
      /* add ruler definitions                                        */
      /****************************************************************/
      SWP     swp;
      ULONG   ulCY = pNewdoc->cy;
      WinQueryWindowPos( pNewdoc->hwndFrame, &swp );

      pNewdoc->hRulerWnd =
          WinCreateWindow( pNewdoc->hwndFrame, TP_RULER,
                           "",
                           WS_CHILD,
                           0 , 0 , swp.cx, ulCY,
                           pNewdoc->hwndFrame, HWND_TOP,
                           ID_TP_RULER,
                           NULL, NULL);

      ANCHORWNDIDA( pNewdoc->hRulerWnd, pNewdoc );
    }

   }
   else
   {
      // display message - now only a warning beep
      WinAlarm( HWND_DESKTOP, WA_ERROR );
      fOK = FALSE;
   } /* endif */
   return ( fOK );
}





/*//////////////////////////////////////////////////////////////
:h3.EQFBHScroll
*///////////////////////////////////////////////////////////////
// Description:
//     Handle horizontal scroll by calling editor
//     functions directly
// Flow: - do action according to parameter
//           Lineleft,Lineright,Pageleft,Pageright,
//             Sliderposition
//       -  display screen/update cursor/slider
//          if necessary
// Parameters:
//  same as for normal window function
//
////////////////////////////////////////////////////////////////
void EQFBHScroll
(
   PTBDOCUMENT pDoc,                             // window handle
   USHORT usSBType,                     // scroll bar type
   USHORT usSBThumbPosition             // scroll bar position
)
{
    BOOL fUpdate = TRUE ;               // do a screen update per default

    switch ( usSBType )
    {

      case SB_LINELEFT:                 // Window to left
    /* Adjust horizontal scrolling appropriately for Target document */
        if (pDoc->fTARight)
        {
		  EQFBFuncScrollLeft( pDoc );     // scroll the screen left
	    }
	    else
	    {
          EQFBFuncScrollRight( pDoc );    // scroll the screen right
	    }
        break;

      case SB_LINERIGHT:                // Window to right
    /* Adjust horizontal scrolling appropriately for Target document */
        if (pDoc->fTARight)
		{
		  EQFBFuncScrollRight( pDoc );     // scroll the screen left
		}
		else
		{
		  EQFBFuncScrollLeft( pDoc );    // scroll the screen right
	    }
        break;

      case SB_PAGELEFT:                 // Oper clicked left of slider
    /* Adjust horizontal scrolling appropriately for Target document */
        EQFBIncOffset ( pDoc, (- pDoc->lScrnCols) );
        break;

      case SB_PAGERIGHT:                // Oper clicked right of slider
    /* Adjust horizontal scrolling appropriately for Target document */
        EQFBIncOffset ( pDoc, pDoc->lScrnCols);
        break;

      case SB_SLIDERPOSITION:
    /* Adjust horizontal scrolling appropriately for Target document */
        EQFBSetOffset ( pDoc, usSBThumbPosition);
        break;

      default:
        fUpdate = FALSE;               // no update yet
        break;
    } // switch
    if ( fUpdate )
    {
       EQFBScreenData( pDoc );        // display screen
       EQFBScreenCursor( pDoc );      // update cursor/sliders
    } /* endif */
}

/*//////////////////////////////////////////////////////////////
:h3.EQFBVScroll
*///////////////////////////////////////////////////////////////
// Description:
//     Handle veritcal scroll by calling editor
//     functions directly
// Flow: - do action according to parameter
//           LineUp,LineDown,PageUp,PageDown,
//             Sliderposition
//       -  display screen/update cursor/slider
//          if necessary
// Parameters:
//  same as for normal window function
//
////////////////////////////////////////////////////////////////
void EQFBVScroll
(
   PTBDOCUMENT pDoc,                   // document pointer
   USHORT usSBType,
   USHORT usThumbPosition
)
{
    BOOL fUpdate = TRUE ;               // do a screen update per default
    ULONG      ulSeg;                   // number of segment
    PTBSEGMENT pSeg;                    // pointer to segment data


    switch ( usSBType )
    {

      case SB_LINEUP:
        if ( pDoc->hwndRichEdit )
        {
          SendMessage( pDoc->hwndRichEdit, EM_LINESCROLL, 0L, -1L );
        }
        else
        {
          EQFBFuncScrollDown(pDoc, TRUE );// scroll the screen down one line
        } /* endif */
        break;

      case SB_LINEDOWN:                 // Oper clicked down arrow
        if ( pDoc->hwndRichEdit )
        {
          SendMessage( pDoc->hwndRichEdit, EM_LINESCROLL, 0L, 1L );
        }
        else
        {
          EQFBFuncScrollUp(pDoc, TRUE  ); // scroll the screen up one line
        } /* endif */
        break;

      case SB_PAGEUP:                   // Oper clicked above slider
        EQFBFuncPageUp( pDoc );         // scroll up one page
        break;

      case SB_PAGEDOWN:                 // Oper clicked below slider
        EQFBFuncPageDown( pDoc );       // scroll down one page
        break;

      case SB_SLIDERPOSITION:
                                       // go to a specific segment
        if ( pDoc->ulMaxSeg > MAX_VSCROLLRANGE )
        {
//          ulSeg  = usThumbPosition << 1;
            double dbMaxSeg = (double)pDoc->ulMaxSeg;
            double dbRange = (double)MAX_VSCROLLRANGE;
            double dbValue = (double)usThumbPosition * dbMaxSeg / dbRange;
            ulSeg = (ULONG)(dbValue+0.999);
        }
        else
        {
          ulSeg  = usThumbPosition;
        } /* endif */

        // if segment number is 0 increment it since Seg 0 might cause problems
        if ( ulSeg == 0 )
        {
           ulSeg = 1;
        } /* endif */
        // check if requested segment is visible (i.e. not part of a joined one
        // if so pSeg will be Null and you have to scan backward.
        pSeg = EQFBGetVisSeg( pDoc, &ulSeg );    // check if segment is visible
        if ( !pSeg )
        {
           ulSeg = pDoc->ulMaxSeg - 1;
           EQFBGetPrevVisSeg( pDoc, &ulSeg );    // find last visible segment
        } /* endif */
        if ( pDoc->ulMaxSeg > 1 )                // do not scroll if only one segment is available
        {
          EQFBGotoSeg( pDoc, ulSeg , 0);
        } /* endif */

        break;
      default:
        fUpdate = FALSE;               // no update yet
        break;
    } // switch
    if ( fUpdate )
    {
       EQFBScreenData( pDoc );        // display screen
       EQFBScreenCursor( pDoc );      // update cursor/sliders
    } /* endif */

}


/*//////////////////////////////////////////////////////////////
:h3.EQFBIncOffset ( PTBDOCUMENT,SHORT)
*///////////////////////////////////////////////////////////////
// Description:
//     Do a signed increment of sSidescroll, keeping
//     it above zero and below MAX_SEGMENT_SIZE-VIDEOWIDTH
// Flow:
// Parameters:
//  PTBDOCUMENT   - pointer to document ida
//  SHORT         - increment
//
////////////////////////////////////////////////////////////////
static void EQFBIncOffset ( PTBDOCUMENT pDoc,        // document ida
                            LONG lInc )          // increment
{
  LONG lOffset;                                     // offsetin document

   lOffset = pDoc->lSideScroll + lInc;
   lOffset = max(lOffset, 0);
   lOffset = min(lOffset, MAX_SEGMENT_SIZE - pDoc->lScrnCols);
   pDoc->lSideScroll = lOffset;
   EQFBDBCS2ND(pDoc,(lInc>0));          //adjust to the right if sInc>0

   pDoc->Redraw |= REDRAW_ALL;          // Repaint data area
}


/*//////////////////////////////////////////////////////////////
:h3.EQFBSetOffset ( PTBDOCUMENT,SHORT)
*///////////////////////////////////////////////////////////////
// Description:
//     Set sSideScroll, keeping it above zero and below
//     MAX_SEGMENT_SIZE-VIDEOWIDTH
// Flow:
// Parameters:
//  PTBDOCUMENT   - pointer to document ida
//  SHORT         - increment
//
////////////////////////////////////////////////////////////////
static void EQFBSetOffset ( PTBDOCUMENT pDoc,     // document ida
                            LONG        lInc )      // increment
{
   LONG  lOffset;                                 // offsetin document

   lOffset = lInc;
   lOffset = max(lOffset, 0);
   lOffset = min(lOffset, MAX_SEGMENT_SIZE - pDoc->lScrnCols);
   pDoc->lSideScroll = lOffset;
   EQFBDBCS2ND(pDoc,(lInc>0));                    //adjust to right if sInc>0
   pDoc->Redraw |= REDRAW_ALL;                    // Repaint data area
}


/*//////////////////////////////////////////////////////////////
:h3.EQFBCurrentState ( PTBDOCUMENT)
*///////////////////////////////////////////////////////////////
// Description:
//     get the current status (of the cursor location )
//     Status is one of the following:
//       ACTSEG   within active segment or in postedit mode
//       STARGET  within target document
//       OVERALL  in any other place
// Flow:
//     check if we are in STARGET_DOC, else set mode to OVERALL
//     check if we are in active segment or in post edit mode (ACTSEG)
//     else set mode to ACTSEG.
//
// Parameters:
//  PTBDOCUMENT   - pointer to document ida
//
//
////////////////////////////////////////////////////////////////

USHORT EQFBCurrentState
(
  PTBDOCUMENT pDoc                                 // pointer to document ida
)
{
   USHORT usStatus;                                // current status


   if ( pDoc->docType != STARGET_DOC )
   {
      usStatus = OVERALL;
      if ( pDoc->docType == EEA_DOC )
      {
          usStatus = ACTSEG;
      } /* endif */
   }
   else if ( pDoc->ulWorkSeg )
   {
      if ( EQFBStillInActSeg( pDoc ) )
      {
         usStatus = ACTSEG;
      }
      else if ( pDoc->EQFBFlags.PostEdit  )
      {
         usStatus = (pDoc->pTBSeg->qStatus == QF_XLATED) ? ACTSEG : STARGET;
      }
      else
      {
         usStatus = STARGET;
      } /* endif */
   }
   else
   {
      usStatus = STARGET;
   } /* endif */

   if ( pDoc->EQFBFlags.EndOfSeg && pDoc->pTBSeg )
   {
     /*****************************************************************/
     /* check if endofseg still valid                                 */
     /* pDoc->pTBSeg is the correct segment because EndOfSeg is only  */
     /* set in the active segment                                     */
     /*****************************************************************/
                                            // check if endofseg still valid
     EQFBCheckEndOfSeg(pDoc,pDoc->pTBSeg,pDoc->TBCursor.usSegOffset);
   } /* endif */

   return (usStatus);                              // return the status
}
//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     EQFBChangeStyle
//------------------------------------------------------------------------------
// Function call:     _
//------------------------------------------------------------------------------
// Description:       set a new style for the document
//                    Style is one of the following:
//                      IDM_UNPROTECTED   user can enter everywhere
//                      IDM_PROTECTED     tags are protected
//                      IDM_HIDE          tags (nops are hided)
//                      IDM_SHRINK        nop segments are shrinked to a
//                                            single character
//                      IDM_COMPACT       nop segments and inline tags
//                                              are shrinked...
//                      IDM_SHORTEN       only 1st word of nop is displayed
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT   - pointer to document ida
//                    USHORT        - selected style
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:     check if desired style is allowed in document
//                    if ok
//                     make a checkmark at selected AAB item
//                     decheckmark the other items
//                     force screen update
//                     update TBCursor.ulSegNum and usSEgOffset
//                    else
//                     beep
//------------------------------------------------------------------------------


VOID EQFBChangeStyle
(
  PTBDOCUMENT  pDoc,                         // pointer to document
  USHORT   usStyle                           // selected style
)
{
  HMENU hwndTemp;                            // pointer to menu hwnd
  PTBSEGMENT pSeg;                           // pointer to segment
  BOOL fOK = TRUE;                           // true if style allowed
  ULONG  ulSegNum;                           // ulSegNum during loop
/**********************************************************************/
/* check if desired new style is allowed in document                  */
/**********************************************************************/
  switch ( pDoc->docType)
  {
    case SSOURCE_DOC:
       if ( usStyle == DISP_UNPROTECTED )
       {
          fOK = FALSE;
       }
       break;
     case  OTHER_DOC:
        if ( usStyle != DISP_PROTECTED )
        {
          fOK = FALSE;
        } /* endif */
       break;
     default  :
       break;
  } /* endswitch */
  if ( fOK )
  {
     if ( (pDoc->ulWorkSeg) && ! pDoc->EQFBFlags.PostEdit && pDoc->pTBSeg )
     {
       pDoc->pTBSeg->SegFlags.Expanded = FALSE;
     } /* endif */
     pDoc->DispStyle = (DISPSTYLE)usStyle;                      // set selected style
     hwndTemp = GetMenu(pDoc->hwndClient);

     SETAABITEMCHECK( hwndTemp, IDM_UNPROTECTED, (usStyle == DISP_UNPROTECTED));
     SETAABITEMCHECK( hwndTemp, IDM_PROTECTED, (usStyle == DISP_PROTECTED) );
     SETAABITEMCHECK( hwndTemp, IDM_HIDE, (usStyle == DISP_HIDE) );
     SETAABITEMCHECK( hwndTemp, IDM_SHRINK, (usStyle == DISP_SHRINK) );
     SETAABITEMCHECK( hwndTemp, IDM_COMPACT, (usStyle == DISP_COMPACT) );
     SETAABITEMCHECK( hwndTemp, IDM_SHORTEN, (usStyle == DISP_SHORTEN) );

     if (pDoc->docType == STARGET_DOC)
     {
       USEROPT* pEQFBUserOpt = get_EQFBUserOpt();
       pEQFBUserOpt->DispTrans = usStyle;
       EQFBWriteProfile(pDoc);
     }

     /*****************************************************************/
     /* force recompute of all start-stop tables                      */
     /*****************************************************************/
     for (ulSegNum = 1 ;ulSegNum < (pDoc->ulMaxSeg)  ; ulSegNum++ )
     {
       pSeg = EQFBGetSegW( pDoc, ulSegNum );
       if ( pSeg )
       {
          UtlAlloc( (PVOID *)&(pSeg->pusBPET) ,0L ,0L , NOMSG); // free old segment table
          if (pSeg->pusHLType )
          {
            UtlAlloc((PVOID*) &(pSeg->pusHLType), 0L,0L,NOMSG);
            pSeg->SegFlags.Spellchecked = FALSE;
          } /* endif */
       } /* endif */
     } /* endfor */

     ulSegNum = pDoc->TBCursor.ulSegNum;                             /* @39A */
     pSeg = EQFBGetVisSeg( pDoc, &ulSegNum );                        /* @39C */
     if ( ! pSeg )                                                   /* @39A */
     {                                                               /* @39A */
       ulSegNum = pDoc->ulMaxSeg - 1;                                /* @39A */
       pSeg = EQFBGetPrevVisSeg( pDoc, &ulSegNum);                   /* @39A */
       if ( !ulSegNum )                                              /* @39A */
       {                                                             /* @39A */
         ulSegNum = 1;                                               /* @39A */
       } /* endif */                                                 /* @39A */
     } /* endif */                                                   /* @39A */

     /*****************************************************************/
     /* if segnum changed, do a positioning at the beginning of the   */
     /* segment                                                       */
     /*****************************************************************/
     if ( pDoc->TBCursor.ulSegNum != ulSegNum )
     {
       pDoc->TBCursor.ulSegNum = ulSegNum;                         /* @39A */
       pDoc->TBCursor.usSegOffset = 0;                             /* @39A */
     } /* endif */
     EQFBGotoSeg( pDoc, pDoc->TBCursor.ulSegNum,
                        pDoc->TBCursor.usSegOffset);               /* @39C */

     if (pDoc->fLineWrap && pDoc->fAutoLineWrap)
     {
       if ( pDoc->lScrnCols )   // donot adjust if wnd is minimized
       {
         EQFBFuncMarginAct(pDoc);
         EQFBFuncMarginAct(pDoc);
         if (pDoc->fAutoSpellCheck && pDoc->pvSpellData )
	 	 { // force that thread recalcs pusHLType
	 	    PSPELLDATA pSpellData = (PSPELLDATA) pDoc->pvSpellData;
	 	    pSpellData->TBFirstLine.ulSegNum = 0;
	 	    pSpellData->TBFirstLine.usSegOffset = (USHORT)-1; // cannot be segoffs
	     }
       } /* endif */
     }

  }
  else
  {
     WinAlarm( HWND_DESKTOP, WA_WARNING );    // issue a beep
  } /* endif */
  return;
}

//------------------------------------------------------------------------------
// Internal function
// External function
//------------------------------------------------------------------------------
// Function name:     EQFBExpandSeg
//------------------------------------------------------------------------------
// Function call:     _
//------------------------------------------------------------------------------
// Description:       _
//------------------------------------------------------------------------------
// Input parameter:   _
// Parameters:        _
//------------------------------------------------------------------------------
// Output parameter:  _
//------------------------------------------------------------------------------
// Returncode type:   _
//------------------------------------------------------------------------------
// Returncodes:       _
//------------------------------------------------------------------------------
// Prerequesits:      _
//------------------------------------------------------------------------------
// Side effects:      _
//------------------------------------------------------------------------------
// Samples:           _
//------------------------------------------------------------------------------
// Function flow:     _
//------------------------------------------------------------------------------

VOID
EQFBExpandSeg
(
  PTBDOCUMENT  pDoc                          // pointer to document
)
{
  PTBSEGMENT pSeg;
  /********************************************************************/
  /* expand allowed in active segment and not postedit                */
  /********************************************************************/
  if ( pDoc->DispStyle == DISP_COMPACT &&
        (pDoc->TBCursor.ulSegNum == pDoc->ulWorkSeg) &&
                 ! pDoc->EQFBFlags.PostEdit )
  {
    pSeg =  EQFBGetSegW( pDoc,
                       pDoc->TBCursor.ulSegNum );     // get current segment
    pSeg->SegFlags.Expanded = TRUE;
    EQFBGotoSeg( pDoc, pDoc->TBCursor.ulSegNum, 0);
    if (pSeg->pusHLType)
    {
      UtlAlloc((PVOID*) &(pSeg->pusHLType), 0L,0L,NOMSG);
      pSeg->SegFlags.Spellchecked = FALSE;
      if (pDoc->fAutoSpellCheck && pDoc->pvSpellData )
	  { // force that thread recalcs pusHLType
	     PSPELLDATA pSpellData = (PSPELLDATA) pDoc->pvSpellData;
	     pSpellData->TBFirstLine.ulSegNum = 0;
	     pSpellData->TBFirstLine.usSegOffset = (USHORT)-1; // cannot be segoffs
	  }
    } /* endif */
    UtlAlloc( (PVOID *)&(pSeg->pusBPET) ,0L ,0L , NOMSG);       // free old segment table
    pDoc->Redraw |= REDRAW_ALL;
  }
  else
  {
     WinAlarm( HWND_DESKTOP, WA_WARNING );    // issue a beep
  } /* endif */

  return;
} /* end of function EQFBExpandSeg */

//------------------------------------------------------------------------------
// Internal function
// External function
//------------------------------------------------------------------------------
// Function name:     EQFBCompressSeg
//------------------------------------------------------------------------------
// Function call:     _
//------------------------------------------------------------------------------
// Description:       _
//------------------------------------------------------------------------------
// Input parameter:   _
// Parameters:        _
//------------------------------------------------------------------------------
// Output parameter:  _
//------------------------------------------------------------------------------
// Returncode type:   _
//------------------------------------------------------------------------------
// Returncodes:       _
//------------------------------------------------------------------------------
// Prerequesits:      _
//------------------------------------------------------------------------------
// Side effects:      _
//------------------------------------------------------------------------------
// Samples:           _
//------------------------------------------------------------------------------
// Function flow:     _
//------------------------------------------------------------------------------

VOID
EQFBCompressSeg
(
  PTBDOCUMENT  pDoc                          // pointer to document
)
{
  PTBSEGMENT pSeg;
  if ( (pDoc->TBCursor.ulSegNum == pDoc->ulWorkSeg) &&
                 ! pDoc->EQFBFlags.PostEdit )
  {
    pSeg =  EQFBGetSegW( pDoc,
                       pDoc->TBCursor.ulSegNum );          // get current segment
    pSeg->SegFlags.Expanded = FALSE;
    EQFBGotoSeg( pDoc, pDoc->TBCursor.ulSegNum, 0);
    UtlAlloc( (PVOID *)&(pSeg->pusBPET) ,0L ,0L , NOMSG);       // free old segment table
    if (pSeg->pusHLType)
    {
      UtlAlloc((PVOID*) &(pSeg->pusHLType), 0L,0L,NOMSG);
      pSeg->SegFlags.Spellchecked = FALSE;
      if (pDoc->fAutoSpellCheck && pDoc->pvSpellData )
	  { // force that thread recalcs pusHLType
	    PSPELLDATA pSpellData = (PSPELLDATA) pDoc->pvSpellData;
	    pSpellData->TBFirstLine.ulSegNum = 0;
	    pSpellData->TBFirstLine.usSegOffset = (USHORT)-1; // cannot be segoffs
	  }
    } /* endif */
    pDoc->Redraw |= REDRAW_ALL;
  }
  else
  {
     WinAlarm( HWND_DESKTOP, WA_WARNING );    // issue a beep
  } /* endif */
  return;
} /* end of function EQFBCompressSeg */

/*//////////////////////////////////////////////////////////////
:h3.EQFBRefreshScreen ( )
*///////////////////////////////////////////////////////////////
// Description:
//     refresh the target document and the source document if necessary
// Flow:
//     Update screen data of target
//     update cursor position of target cursor
//     if source screen is displayed
//         Update screen data of target
//         update cursor position of target cursor
//     set target cursor to triple mode if necessary
//
// Parameters:
//  PTBDOCUMENT   - pointer to document ida
//
//
////////////////////////////////////////////////////////////////
VOID EQFBRefreshScreen
(
   PTBDOCUMENT  pDoc
)
{
   PTBDOCUMENT pDocTwin;
   if ( pDoc )
   {
     if ( pDoc->Redraw != REDRAW_NONE )
     {
        EQFBScreenData( pDoc );          // display screen
     } /* endif */
     EQFBScreenCursor( pDoc );           // update cursor and sliders
                                         // check if source needs update, too
     pDocTwin = pDoc->twin;
     if ( pDocTwin && (pDocTwin->docType == SSOURCE_DOC)
          &&    WinIsWindowVisible( pDocTwin->hwndClient) )
     {
        if ( pDocTwin->Redraw != REDRAW_NONE )
        {
           EQFBScreenData( pDocTwin );      // display screen
           EQFBScreenCursor( pDocTwin );    // update cursor and sliders
        } /* endif */
     } /* endif */

     /*******************************************************************/
     /* check if still at end of seg                                    */
     /* pDoc->pTBSeg is correct because endofseg is only set if         */
     /* cursor is in active segment                                     */
     /*******************************************************************/
     if ( pDoc->EQFBFlags.EndOfSeg && pDoc->pTBSeg )
     {
       EQFBCheckEndOfSeg(pDoc,pDoc->pTBSeg,pDoc->TBCursor.usSegOffset);
     } /* endif */
   } /* endif */
}



/*//////////////////////////////////////////////////////////////
:h3.EQFBFuncFind ( )
*///////////////////////////////////////////////////////////////
// Description:
//     invoke the find dialog
// Flow:
//     in case of first invocation allocate data
//     call dialog
//
// Parameters:
//  PTBDOCUMENT   - pointer to document ida
//
//
////////////////////////////////////////////////////////////////
VOID EQFBFuncFind
(
   PTBDOCUMENT pDoc                             // pointer to document ida
)
{
   INT_PTR      iRc;                                       // return code

   HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);

   EQFBSetFindData( NULL );

   if ( pFindData )
   {
    /******************************************************************/
    /* provide modeless dialog ...                                    */
    /* (loop thru list of documents and find if we have an open one.. */
    /******************************************************************/
    if ( (pFindData->pDoc != pDoc) && pFindData->hwndFindDlg )
    {
      /****************************************************************/
      /* ensure that no find dlg is pending                           */
      /****************************************************************/
      EQFBFindCloseModeless( pFindData->hwndFindDlg, 0, 0L );
    } /* endif */
    pFindData->pDoc = pDoc;                     // set active document
    /******************************************************************/
    /* activate a modal dialog in case of Translation Memory edit,    */
    /* otherwise do it as modeless                                    */
    /******************************************************************/
    if ( pDoc->pTMMaint )
    {
      DIALOGBOX( pDoc->hwndClient, EQFBFINDDLGPROC, hResMod, ID_TB_FIND_DLG,
                 pFindData, iRc);

    }
    else
    {

      iRc = 0;

       if ( !pFindData->hwndFindDlg )
       {
         FARPROC lpfnProc = MakeProcInstance(  (FARPROC)(EQFBFINDDLGPROCMODELESS), \
                                              (HAB) UtlQueryULong( QL_HAB ) );
         pFindData->hwndFindDlg = CreateDialogParamW( hResMod,
                                                (PSZ_W)MAKEINTRESOURCE( ID_TB_FIND_DLG),
                                                pDoc->hwndClient, (DLGPROC)lpfnProc,
                                                (LPARAM)pFindData );
         FreeProcInstance( lpfnProc );
       } /* endif */


      if ( pFindData->hwndFindDlg )
      {
        WinSetWindowPos( pFindData->hwndFindDlg, HWND_TOP, 0,0,0,0,
                         EQF_SWP_ACTIVATE | EQF_SWP_SHOW );
      } /* endif */

    } /* endif */
     if ( iRc == DID_ERROR )
     {
       UtlError( ERROR_DIALOG_LOAD_FAILED, MB_CANCEL, 0 , NULL, EQF_ERROR );
     } /* endif */
   } /* endif */

}



/*//////////////////////////////////////////////////////////////
:h3.EQFBSetFindData ( )
*///////////////////////////////////////////////////////////////
// Description:
//     invoke the find dialog
// Flow:
//     in case of first invocation allocate data
//     call dialog
//
// Parameters:
//  PTBDOCUMENT   - pointer to document ida
//
//
////////////////////////////////////////////////////////////////
VOID EQFBSetFindData
(
   PSZ_W       pString                 // search string
)
{
   if ( ! pFindData )
   {
      UtlAlloc( (PVOID *) &pFindData, 0L, (LONG) sizeof(FINDDATA), ERROR_STORAGE );
      if ( pFindData )
      {
         pFindData->fForward = TRUE;       // init selection criteria
         pFindData->fIgnoreCase = TRUE;
      } /* endif */
   } /* endif */

   if ( pFindData && pString )
   {
     UTF16strcpy( pFindData->chFind, pString );
   } /* endif */
}

/*//////////////////////////////////////////////////////////////
:h3.EQFBFuncCFind ( )
*///////////////////////////////////////////////////////////////
// Description:
//     invoke the concordance search dialog
// Flow:
//     in case of first invocation allocate data
//     call dialog
//
// Parameters:
//  PTBDOCUMENT   - pointer to document ida
//
//
////////////////////////////////////////////////////////////////
VOID EQFBFuncCFind
(
   PTBDOCUMENT pDoc                             // pointer to document ida
)
{
  EQFBCFind( pDoc );
}


/*//////////////////////////////////////////////////////////////
:h3.EQFBFuncOpen ( )
*///////////////////////////////////////////////////////////////
// Description:
//     invoke the open dialog
// Flow:
//     in case of first invocation allocate data
//     call dialog
//
// Parameters:
//  PTBDOCUMENT   - pointer to document ida
//
//
////////////////////////////////////////////////////////////////
VOID EQFBFuncOpen
(
   PTBDOCUMENT pDoc                   // pointer to document ida
)
{
   INT_PTR       iRc;                 // call dialog
   LOADSTRUCT    LoadStruct;          // load structure
   PSTEQFGEN     pstEQFGen;           // pointer to generic edit structure
   SWP           swpFrame;            // size and position of frame window
   HWND          hwnd = pDoc->hwndClient;
   OPENDATA*     pOpenData = get_OpenData();
   HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
   
   DIALOGBOX( pDoc->hwndClient, EQFBOPENDLGPROC, hResMod, ID_TB_OPEN_DLG,
              pOpenData, iRc);

   if ( iRc == DID_ERROR )
   {
      UtlError( ERROR_DIALOG_LOAD_FAILED, MB_CANCEL, 0 , NULL, EQF_ERROR );
   }
   else if ( iRc )
   {
      USHORT usI = 0;
      EQFBWriteProfile(pDoc);
      if ( pOpenData->fTroja )
      {
        /**************************************************************/
        /* open internal document                                     */
        /**************************************************************/
        if ( EQF_XDOCINLIST( (PSTEQFGEN)pDoc->pstEQFGen, pOpenData->szDocName, &usI ) != 0)
        {
          iRc = EQF_XDOCADD( (PSTEQFGEN)pDoc->pstEQFGen, pOpenData->szDocName );
          if (iRc == 0)
          {
            iRc = EQFBTenvStart ( pDoc, pOpenData->szDocName, (PSTEQFGEN)pDoc->pstEQFGen );
          } /* endif */
        }
        else
        {
          // activate it
          iRc =  EQF_XDOCNUM( (PSTEQFGEN)pDoc->pstEQFGen, usI, (PSZ)pDoc->pInBuf );
          if ( !iRc && *pDoc->pInBuf )
          {
            iRc = EQFBTenvStart( pDoc, (PSZ)pDoc->pInBuf, (PSTEQFGEN)pDoc->pstEQFGen );
          } /* endif */
        }
      }
      else
      {
        /**************************************************************/
        /* open other document                                        */
        /**************************************************************/
        pstEQFGen = (PSTEQFGEN)pDoc->pstEQFGen;
        LoadStruct.hwndParent = pstEQFGen->hwndTWBS;
        LoadStruct.pDoc = pDoc;          // set base document

        LoadStruct.fsFlagStyle = EQF_SWP_MOVE | EQF_SWP_SIZE | EQF_SWP_SHOW | EQF_SWP_ACTIVATE;
        LoadStruct.fReadOnly = TRUE;

        if ( pstEQFGen->flEditOtherStyle )
        {
          LoadStruct.flFrameStyle = (pstEQFGen->flEditOtherStyle & AVAILSTYLES) |
                                       FCF_SIZEBORDER;
        }
        else
        {
          LoadStruct.flFrameStyle = FCF_TITLEBAR   | FCF_MENU   | FCF_SYSMENU |
                                    FCF_SIZEBORDER | FCF_MAXBUTTON |
                                    FCF_VERTSCROLL | FCF_HORZSCROLL;
        } /* endif */

        // set size an position to same values as active window
        WinQueryWindowPos( GETPARENT( hwnd ), &swpFrame );
        RECTL_XLEFT(LoadStruct.rclPos)   = (LONG) swpFrame.x;
        RECTL_XRIGHT(LoadStruct.rclPos)  = (LONG) (swpFrame.cx + swpFrame.x);
        RECTL_YTOP(LoadStruct.rclPos)    = (LONG) (swpFrame.cy + swpFrame.y);
        RECTL_YBOTTOM(LoadStruct.rclPos) = (LONG) swpFrame.y;

        // move window one titlebar height down
        RECTL_YTOP(LoadStruct.rclPos)    -= WinQuerySysValue(HWND_DESKTOP, SV_CYTITLEBAR);
        RECTL_YBOTTOM(LoadStruct.rclPos) -= WinQuerySysValue(HWND_DESKTOP, SV_CYTITLEBAR);
        EQFBValidatePositions( &LoadStruct.rclPos, OTHER_DOC );


        LoadStruct.pFileName = ( pOpenData->fTroja ) ?
                                    pOpenData->szDocName : pOpenData->szFileName;
        LoadStruct.docType = OTHER_DOC;
        LoadStruct.pszEQFTagTable = (PSZ)pstEQFGen->szEQFTagTable;
        LoadStruct.pszTagTable = (PSZ)pstEQFGen->szTagTable;
        LoadStruct.pstEQFGen   = pstEQFGen;

        EQFBDocLoad( &LoadStruct);       // load document and pass LoadStruct
        LoadStruct.pDoc->Redraw |= REDRAW_ALL;   // force repaint...
        /****************************************************************/
        /* set the display style and disable the SpellCheck ...         */
        /****************************************************************/
        LoadStruct.pDoc->DispStyle = DISP_PROTECTED;   // protect SGML tags
        /****************************************************************/
        /* set focus on newly loaded document...                        */
        /****************************************************************/
        SendMessage( pstEQFGen->hwndTWBS, WM_EQF_SETFOCUS,
                     0, MP2FROMHWND( LoadStruct.pDoc->hwndFrame ));
      } /* endif */
   } /* endif */
}



/*//////////////////////////////////////////////////////////////
:h3.EQFBFuncFonts( )
*///////////////////////////////////////////////////////////////
// Description:
//     invoke the fonts dialog
// Flow:
//     in case of first invocation allocate data
//     call dialog
//
// Parameters:
//  PTBDOCUMENT   - pointer to document ida
//
//
////////////////////////////////////////////////////////////////
VOID EQFBFuncFonts
(
   PTBDOCUMENT  pDoc
)
{
   PFONTCOLDATA pFontColData;         // ptr to data structure for font dialog
   INT_PTR      iRc;                  // success indicator
   PTBDOCUMENT  pDocStart;            // ptr to current pdoc
   PFONTCOLIDA   pIda;                // ptr to dialog IDA

   UtlAlloc( (PVOID *) &pFontColData, 0L, (LONG) sizeof(FONTCOLDATA), ERROR_STORAGE );
   if ( pFontColData )
   {
      HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
      pFontColData->pTextTypeTable = get_TextTypeTable();
      UtlAlloc( (PVOID *) &pIda, 0L, (LONG) sizeof(FONTCOLIDA), ERROR_STORAGE );
      if ( pIda )
      {
        pIda->pDoc = pDoc;
        pIda->pFontColData = pFontColData;
        DIALOGBOX( pDoc->hwndClient, EQFBFONTCOLDLGPROC, hResMod,
                   ID_TB_FONTCOL_DLG, pIda, iRc );

        if ( iRc == DID_ERROR )
        {
          UtlError( ERROR_DIALOG_LOAD_FAILED, MB_CANCEL, 0 , NULL, EQF_ERROR );
        }
        else if ( iRc )
        {
           pDocStart = pDoc ;                     // pointer to active document

           // scan through all documents and issue a reset together with a
           // forced redraw of the screen
           do
           {
              pDoc->Redraw |= REDRAW_ALL;
              EQFBRefreshScreen( pDoc );  // refresh the screen
              pDoc = pDoc->next;
           } while ( pDoc != pDocStart  ); /* enddo */
           EQFBWriteProfile(pDoc);
        } /* endif */
        UtlAlloc( (PVOID *) &pIda,0L, 0L, NOMSG );
      } /* endif */
      UtlAlloc( (PVOID *) &pFontColData, 0L, 0L, NOMSG );
   } /* endif */
}


/*//////////////////////////////////////////////////////////////
:h3.EQFBFuncKeys( )
*///////////////////////////////////////////////////////////////
// Description:
//     invoke the keys dialog
// Flow:
//     in case of first invocation allocate data
//     call dialog
//
// Parameters:
//  PTBDOCUMENT   - pointer to document ida
//
//
////////////////////////////////////////////////////////////////
VOID EQFBFuncKeys
(
   PTBDOCUMENT  pDoc
)
{
   PKEYSDATA    pKeysData;            // ptr to data structure for keys dialog
   INT_PTR      iRc;
   PDOCUMENT_IDA pIdaDoc;
   PSTEQFGEN     pstEQFGen;
   HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);

   UtlAlloc( (PVOID *) &pKeysData, 0L, (LONG) sizeof(KEYSDATA), ERROR_STORAGE );
   if ( pKeysData )
   {
      pKeysData->pKeyList  = get_KeyTable();
      pKeysData->pResKeys  = get_ResKeyTab();
      pKeysData->pFuncList = get_FuncTab();
      pKeysData->hwndFrame = pDoc->hwndFrame;
      pKeysData->pDoc = pDoc;

      DIALOGBOX( pDoc->hwndClient, EQFBKEYSDLGPROC, hResMod,
                 ID_TB_KEYS_DLG, pKeysData, iRc );

      if ( iRc == DID_ERROR )
      {
        UtlError( ERROR_DIALOG_LOAD_FAILED, MB_CANCEL, 0 , NULL, EQF_ERROR );
      }
      else if ( iRc )
      {
         EQFBAdaptKeyTable( get_KeyTable(), pDoc );
         EQFBWriteProfile(pDoc);
         pstEQFGen = (PSTEQFGEN)pDoc->pstEQFGen;

         if (pstEQFGen )
         {
           pIdaDoc = (PDOCUMENT_IDA)(pstEQFGen->pDoc);
           if (pIdaDoc )
           {
             memset (pIdaDoc->chDictPrefixes,0,
                     sizeof(pIdaDoc->chDictPrefixes) );
           } /* endif */
           // force update of service windows
           WinPostMsg(pstEQFGen->hwndTWBS,
                      WM_EQF_COLCHANGED, NULL, NULL );
        } /* endif */
      } /* endif */
      UtlAlloc( (PVOID *) &pKeysData, 0L, 0L, NOMSG );
   } /* endif */
}

/*//////////////////////////////////////////////////////////////
:h3.EQFBFuncEntry( )
*///////////////////////////////////////////////////////////////
// Description:
//     invoke the entry sentence dialog
// Flow:
//     invoke the entry dialog or display error message if problems in
//     call dialog
//
// Parameters:
//  PTBDOCUMENT   - pointer to document ida
//
//
////////////////////////////////////////////////////////////////
VOID EQFBFuncEntry
(
   PTBDOCUMENT pDoc                             // pointer to document ida
)
{
   INT_PTR      iRc;                            // return code
   PENTRYDATA   pEntryData;                     // pointer to data
   HWND         hwnd;
   HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);

/**********************************************************************/
/* test sentence dialog only possible if not in postedit              */
/**********************************************************************/
   if ( !pDoc->EQFBFlags.PostEdit)
   {
      hwnd = pDoc->hwndClient;
      UtlAlloc( (PVOID *) &pEntryData, 0L, (LONG) sizeof(ENTRYDATA), ERROR_STORAGE );
      if ( pEntryData )
      {
        pEntryData->pDoc = pDoc;       // get pointer to document
        DIALOGBOX( pDoc->hwndClient, EQFBENTRYDLGPROC, hResMod,
                   ID_TB_ENTRY_DLG, pEntryData, iRc );

        if ( iRc == DID_ERROR )
        {
          UtlError( ERROR_DIALOG_LOAD_FAILED, MB_CANCEL, 0 , NULL, EQF_ERROR );
        } /* endif */
        UtlAlloc( (PVOID *) &pEntryData, 0L, 0L, NOMSG );
      } /* endif */
   }
   else
   {
       WinAlarm( HWND_DESKTOP, WA_WARNING );    // issue a beep
   } /* endif */
}


/*//////////////////////////////////////////////////////////////
:h3.EQFBFuncMarginAct( )
*///////////////////////////////////////////////////////////////
// Description:
//     toggle line wrap active/deactive status for documents
// Flow:
//     In case of autolinewrap insert/remove soft linefeeds
//
// Parameters:
//  PTBDOCUMENT   - pointer to document ida
//
//
////////////////////////////////////////////////////////////////
VOID EQFBFuncToggleLineWrap
(
   PTBDOCUMENT pDoc                             // pointer to document ida
)
{
  EQFBFuncMarginAct( pDoc );            // activate/deactivate linewrap
  if (pDoc->docType == STARGET_DOC && pDoc->fAutoLineWrap)
  {
    EQFBFuncMarginAct(pDoc->twin);
  }
  if (pDoc->pUndoSegW && pDoc->fAutoLineWrap && !pDoc->fLineWrap)
  {
    USHORT usBufSize = 0;
    USHORT usOffset = 0;
    EQFBBufRemoveSoftLF(pDoc->hwndRichEdit, pDoc->pUndoSegW,&usBufSize, &usOffset );
  }
  EQFBWriteProfile( pDoc );
  EQFBGotoSeg( pDoc, pDoc->TBCursor.ulSegNum,
                      pDoc->TBCursor.usSegOffset); // reset cursor on current position
  if (pDoc->fAutoSpellCheck && pDoc->pvSpellData )
  { // force that thread recalcs pusHLType of screen
	  PSPELLDATA pSpellData = (PSPELLDATA) pDoc->pvSpellData;
	  pSpellData->TBFirstLine.ulSegNum = 0;
	  pSpellData->TBFirstLine.usSegOffset = (USHORT)-1; // cannot be segoffs
	}
}

VOID EQFBFuncMarginAct
(
   PTBDOCUMENT pDoc                             // pointer to document ida
)
{
// if ( pDoc->docType == STARGET_DOC )
// {
	  BOOL fReflowAllowed = FALSE;
	  if (pDoc->EQFBFlags.Reflow ||
	      (!pDoc->EQFBFlags.Reflow && pDoc->fAutoLineWrap))
	  {
		  fReflowAllowed = TRUE;
      }
      if ( fReflowAllowed )             // reflow allowed
      {
         USEROPT* pEQFBUserOpt = get_EQFBUserOpt();
         pDoc->fLineWrap = !pDoc->fLineWrap;
         pEQFBUserOpt->fLineWrap = (EQF_BOOL)pDoc->fLineWrap;
         if ( pDoc->fAutoLineWrap)
         {
           if (pDoc->fLineWrap)
           {
             pDoc->sRMargin = (SHORT)pDoc->lScrnCols;
             pEQFBUserOpt->sRMargin = (SHORT)pDoc->lScrnCols;
             EQFBSoftLFInsert(pDoc);
           }
           else
           {
             EQFBSoftLFRemove(pDoc);
           } /* endif */
         } /* endif */

         pDoc->Redraw |= REDRAW_ALL;
      } /* endif */
// } /* endif */
}
/*//////////////////////////////////////////////////////////////
:h3.EQFBFuncSpellSeg( )
*///////////////////////////////////////////////////////////////
// Description:
//     spellcheck current segment
// Flow:
//
// Parameters:
//  PTBDOCUMENT   - pointer to document ida
//
//
////////////////////////////////////////////////////////////////
VOID EQFBFuncSpellSeg
(
   PTBDOCUMENT pDoc                             // pointer to document ida
)
{
   INT_PTR      iRc;                             // return code
   BOOL         fOK = TRUE;                      // true if spellcheck allowed
   PTBSEGMENT   pTBSeg;                          // pointer to segment
   PSPELLDATA   pSpellData = NULL;

  if (pDoc->docType == STARGET_DOC)
  {
    pTBSeg = EQFBGetVisSeg(pDoc, &(pDoc->TBCursor.ulSegNum));
    fOK = ((pDoc->EQFBFlags.PostEdit && pTBSeg->qStatus == QF_XLATED) || (!pDoc->EQFBFlags.PostEdit && pTBSeg->qStatus == QF_CURRENT));
    if (fOK)
    {
      // GQ 2017/10/13 New approach for spelldata area:
      // the spell dialog uses a private area to avoid side effects with auto-spellcheck running in the background
      // we share however the pIgnoreData list of the spell data area associated with the document
      pSpellData = AllocSpellData( pDoc, (PSTEQFGEN)pDoc->pstEQFGen, FALSE );

      if ( pSpellData )
      {
        HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
        pSpellData->pDoc = pDoc;
        pSpellData->fSegOnly = TRUE;

        DIALOGBOX( pDoc->hwndClient, EQFBSPELLDLGPROC, hResMod, ID_TB_SPELL_DLG, pSpellData, iRc );

        if ( iRc == DID_ERROR )
        {
          UtlError( ERROR_DIALOG_LOAD_FAILED, MB_CANCEL, 0 , NULL, EQF_ERROR );
        } /* endif */
                // release local spell check data area
        if ( pSpellData ) UtlAlloc( (PVOID *)&pSpellData, 0, 0, NOMSG );
      } /* endif */
     }
     else
     {
       //in this segment segment spellcheck not possible -- beep
       WinAlarm( HWND_DESKTOP, WA_WARNING );    // issue a beep
     } /* endif */
  }
  else
  {
     //in otherdoc and ssource doc  spellcheck not allowed
     // beep
     WinAlarm( HWND_DESKTOP, WA_WARNING );    // issue a beep
  } /* endif */
}   /* end spellseg */
/*//////////////////////////////////////////////////////////////
:h3.EQFBFuncSpellFile( )
*///////////////////////////////////////////////////////////////
// Description:
//     spellcheck all translated segments in file
// Flow:
//
// Parameters:
//  PTBDOCUMENT   - pointer to document ida
//
//
////////////////////////////////////////////////////////////////
VOID EQFBFuncSpellFile
(
   PTBDOCUMENT pDoc                             // pointer to document ida
)
{
   INT_PTR        iRc = 0;                            // return code
   PSPELLDATA pSpellData = NULL;

  if (pDoc->docType == STARGET_DOC)
  {
    // GQ 2017/10/13 New approach for spelldata area:
    // the spell dialog uses a private area to avoid side effects with auto-spellcheck running in the background
    // we share however the pIgnoreData list of the spell data area associated with the document
    pSpellData = AllocSpellData( pDoc, (PSTEQFGEN)pDoc->pstEQFGen, FALSE );

    if ( pSpellData )
    {
      pSpellData->fSegOnly = FALSE;
      pSpellData->pDoc = pDoc;
      HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);

      DIALOGBOX( pDoc->hwndClient, EQFBSPELLDLGPROC, hResMod, ID_TB_SPELL_DLG, pSpellData, iRc );

      if ( iRc == DID_ERROR )
      {
        UtlError( ERROR_DIALOG_LOAD_FAILED, MB_CANCEL, 0 , NULL, EQF_ERROR );
      }
      else if ( iRc == SPELLCHECK_GO_TO_NEXT_DOC )   // current document has been processed completeley, go to the next document in the list
      {
          //PCHAR_W pSavedIgnoreNextFree = NULL;         // saved ptr next free pos in IgnoreData
          //PCHAR_W pSavedIgnoreData = NULL;             // saved temp addenda buffer
          //USHORT  usSavedIgnoreLen = 0;                // saved length of allocated ignore data

        USHORT usI = 0;
        PSTEQFGEN pstEQFGen = (PSTEQFGEN)pDoc->pstEQFGen;

        // save spelldata pointer in pstEqfGen structure
        pstEQFGen->pvSpellData = pDoc->pvSpellData;

        // load the next document
        if ( EQF_XDOCINLIST( pstEQFGen, pstEQFGen->pszCurSpellCheckDoc, &usI ) != 0)
        {
          pstEQFGen->fLoadedBySpellcheck = TRUE; 
          int iRc = EQF_XDOCADD( pstEQFGen, pstEQFGen->pszCurSpellCheckDoc );
          if (iRc == 0)
          {
            PDOCUMENT_IDA pIdaDoc = (PDOCUMENT_IDA)pstEQFGen->pDoc;
            iRc = EQFBTenvStart ( pDoc, pstEQFGen->pszCurSpellCheckDoc, pstEQFGen );

            UtlDispatch(); // process all start messages

            // update titlebar and object name
            //SetDocWindowText( pIdaDoc, pIdaDoc->hwnd, pstEQFGen->pszCurSpellCheckDoc );
            //EqfChangeObjectName( pDoc->hwnd, pszNewDoc );
          } /* endif */
        }
        else
        {
          int iRc = 0;

          // activate it
          pstEQFGen->fLoadedBySpellcheck = TRUE; 
          iRc =  EQF_XDOCNUM( pstEQFGen, usI, (PSZ)pDoc->pInBuf );
          if ( !iRc && *pDoc->pInBuf )
          {
            iRc = EQFBTenvStart( pDoc, (PSZ)pDoc->pInBuf, pstEQFGen );
          } /* endif */
        }


        // save and close the current document
        EQFBFuncFile( pDoc );
        UtlDispatch(); 

        // start spellchecking of next document
        if ( pstEQFGen->pNewSpellCheckDoc != NULL ) 
        {
          // restore any saved tem. addendum data
          //if ( pSavedIgnoreData != NULL )
          //{
          //  PTBDOCUMENT pNewDoc = (PTBDOCUMENT)pstEQFGen->pNewSpellCheckDoc;


            //if ( pNewDoc->pvSpellData != NULL )
            //{
            //  pSavedIgnoreNextFree = pSpellData->pIgnoreNextFree;
            //  pSavedIgnoreData = pSpellData->pIgnoreData;
            //  usSavedIgnoreLen = pSpellData->usIgnoreLen;
            //} /* endif */
          } /* endif */

          PostMessage( ((PTBDOCUMENT)(pstEQFGen->pNewSpellCheckDoc))->hwndClient, WM_COMMAND, MAKELONG( IDM_PROOFALL, 0), 0L );
        } /* endif */

      }
      else if ( iRc == SPELLCHECK_LAST_DOC_DONE  )   // the current document has been processed and no more document is following
      {
        // release local spell check data area
        if ( pSpellData ) UtlAlloc( (PVOID *)&pSpellData, 0, 0, NOMSG );

        // save and close the current document
        PostMessage( pDoc->hwndClient, WM_COMMAND, MAKELONG( IDM_FILE, 0), 0L );
      }
      else
      {
        if (pDoc->fAutoSpellCheck && pDoc->pvSpellData )
    		{ 
          // force that thread recalcs pusHLType
		      PSPELLDATA pThreadSpellData = (PSPELLDATA) pDoc->pvSpellData;
		      pThreadSpellData->TBFirstLine.ulSegNum = 0;
		      pThreadSpellData->TBFirstLine.usSegOffset = (USHORT)-1; // cannot be segoffs
  	    }
      } /* endif */

  }
  else
  {
     //in otherdoc and ssource doc  spellcheck not allowed -- beep
     WinAlarm( HWND_DESKTOP, WA_WARNING );    // issue a beep
  } /* endif */
}   /*  end spellfile */


/*//////////////////////////////////////////////////////////////
:h3.EQFBFuncFontSize( )
*///////////////////////////////////////////////////////////////
//   Description:
//       invoke the fontsize dialog
//   Flow:
//       invoke the font size dialog or display error message if problems in
//       calling dialog
//
//   Parameters:
//  PTBDOCUMENT   - pointer to document ida
//
//
////////////////////////////////////////////////////////////////
VOID EQFBFuncFontSize
(
   PTBDOCUMENT pDoc                             // pointer to document ida
)
{
   INT_PTR       iRc;                            // return code
   PFONTSIZEDATA pvioFontSize;                             // ptr to default font sizes
   USHORT       i;

   HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);

   UtlAlloc( (PVOID *) &pvioFontSize, 0L, (LONG) sizeof(FONTSIZEDATA), ERROR_STORAGE );
   if ( pvioFontSize )
   {
      pvioFontSize->hwnd = pDoc->hwndClient;
      pvioFontSize->pDoc = pDoc;
      // get currently active settings
      memcpy( pvioFontSize->vioFontCell, get_vioFontSize(), sizeof(VIOFONTCELLSIZE) * MAX_DIF_DOC);


      for ( i=0; i<MAX_DIF_DOC; i++ )
      {
        strcpy(pvioFontSize->chFontFacename[i], get_aszFontFacesGlobal() + (i * LF_FACESIZE) );
      } /* endfor */


      DIALOGBOX( pDoc->hwndClient, EQFBFONTSIZEDLGPROC, hResMod,
                 ID_TB_FONTSIZE_DLG, pvioFontSize, iRc );

      if ( iRc == DID_ERROR )
      {
        UtlError( ERROR_DIALOG_LOAD_FAILED, MB_CANCEL, 0 , NULL, EQF_ERROR );
      } /* endif */
      UtlAlloc( (PVOID *) &pvioFontSize, 0L, 0L, NOMSG );
   } /* endif */
}



/*////////////////////////////////////////////////////////////////////////////
:h3.EQFBFuncChangeToProt - change to protect style
*/
//  Description:
//       change to protect style
//
//  Flow:
//
//  Parameters:
//     PTBDOCUMENT       pointer to document instance data
//
//
//  Note:
//     simple function, but necessary to calling sequence
//////////////////////////////////////////////////////////////////////////////
VOID  EQFBFuncChangeToProt
(
    PTBDOCUMENT  pDoc                              // pointer to document ida
)
{
        EQFBChangeStyle( pDoc,  DISP_PROTECTED);
}

/*////////////////////////////////////////////////////////////////////////////
:h3.EQFBFuncChangeToHide - change to hide style
*/
//  Description:
//       change to hide style
//
//  Flow:
//
//  Parameters:
//     PTBDOCUMENT       pointer to document instance data
//
//
//  Note:
//     simple function, but necessary to calling sequence
//////////////////////////////////////////////////////////////////////////////
VOID  EQFBFuncChangeToHide
(
    PTBDOCUMENT  pDoc                              // pointer to document ida
)
{
        EQFBChangeStyle( pDoc,  DISP_HIDE);
}
/*////////////////////////////////////////////////////////////////////////////
:h3.EQFBFuncChangeToUnprot - change to unprot style
*/
//  Description:
//       change to unprot style
//
//  Flow:
//
//  Parameters:
//     PTBDOCUMENT       pointer to document instance data
//
//
//  Note:
//     simple function, but necessary to calling sequence
//////////////////////////////////////////////////////////////////////////////
VOID  EQFBFuncChangeToUnprot
(
    PTBDOCUMENT  pDoc                              // pointer to document ida
)
{
    EQFBChangeStyle( pDoc,  DISP_UNPROTECTED );
}
//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     EQFBFuncChangeToShrink
//------------------------------------------------------------------------------
// Description:       change the style to shrink
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT  pDoc           pointer to document ida
//------------------------------------------------------------------------------
// Function flow:     call EQFBChangeStyle with parameter DISP_SHRINK
//------------------------------------------------------------------------------

VOID EQFBFuncChangeToShrink
(
    PTBDOCUMENT  pDoc                              // pointer to document ida
)
{
        EQFBChangeStyle( pDoc,  DISP_SHRINK );
} /* end of function EQFBFuncChangeToShrink */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     EQFBFuncChangeToCompact
//------------------------------------------------------------------------------
// Description:       change the style to compact
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT  pDoc           pointer to document ida
//------------------------------------------------------------------------------
// Function flow:     call EQFBChangeStyle with parameter DISP_COMPACT
//------------------------------------------------------------------------------

VOID EQFBFuncChangeToCompact
(
    PTBDOCUMENT  pDoc                              // pointer to document ida
)
{
   EQFBChangeStyle( pDoc,  DISP_COMPACT );
} /* end of function EQFBFuncChangeToCompact */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     EQFBFuncChangeToShorten
//------------------------------------------------------------------------------
// Description:       change the style to shorten
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT  pDoc           pointer to document ida
//------------------------------------------------------------------------------
// Function flow:     call EQFBChangeStyle with parameter DISP_SHORTEN
//------------------------------------------------------------------------------

VOID EQFBFuncChangeToShorten
(
    PTBDOCUMENT  pDoc                              // pointer to document ida
)
{
   EQFBChangeStyle( pDoc,  DISP_SHORTEN );
} /* end of function EQFBFuncChangeToShorten */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     EQFBFuncChangeToWYSIWYG
//------------------------------------------------------------------------------
// Description:       change the style to WYSIWYG
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT  pDoc           pointer to document ida
//------------------------------------------------------------------------------
// Function flow:     call EQFBChangeStyle with parameter DISP_SHORTEN
//------------------------------------------------------------------------------

VOID EQFBFuncChangeToWYSIWYG
(
    PTBDOCUMENT  pDoc                              // pointer to document ida
)
{
  if ( pDoc->hwndRichEdit )
  {
    EQFBChangeStyle( pDoc,  DISP_WYSIWYG );
  }
  else
  {
    EQFBFuncNothing( pDoc );
  } /* endif */
} /* end of function EQFBFuncChangeToShorten */

/*////////////////////////////////////////////////////////////////////////////
:h3.EQFBVioSetNewDocSize - set all necessary doc info
*/
//  Description:
//       set screen columns and row to currently available size
//
//  Parameters:
//     PTBDOCUMENT       pointer to document structure
//
//
//////////////////////////////////////////////////////////////////////////////

VOID EQFBVioSetNewDocSize
(
   PTBDOCUMENT  pDoc
)
{

     PTBROWOFFSET pTBRow;                                  // table row offset
     // set row and columns of currently available size
     if ( pDoc->xClient == 0) pDoc->xClient = 100;
     if ( pDoc->yClient == 0) pDoc->yClient = 100;

     pDoc->lScrnCols = pDoc->xClient / pDoc->cx ;
     pDoc->lScrnRows = pDoc->yClient / pDoc->cy ;

     if (pDoc->fLineWrap && pDoc->fAutoLineWrap)
     {
       ULONG ulActLine;
       if ( pDoc->lScrnCols )         // donot adjust if wnd minimized
       {
         EQFBFuncMarginAct(pDoc);                  // removes the SoftLF
         EQFBFuncMarginAct(pDoc);                            // inserts the SoftLFs again
       } /* endif */
       ulActLine = EQFBQueryActLine( pDoc,
                                     (pDoc->TBCursor).ulSegNum,
                                     (pDoc->TBCursor).usSegOffset);
       pDoc->ulVScroll = ulActLine - pDoc->lCursorRow;
     }

     pTBRow = pDoc->TBRowOffset+1 + pDoc->lCursorRow; // get first element
                                       // check if still in visible area
     if ( pDoc->lCursorRow >= pDoc->lScrnRows )
     {
        pDoc->ulVScroll += (pDoc->lCursorRow - pDoc->lScrnRows + 1);
        pDoc->lCursorRow = pDoc->lScrnRows - 1;  // position at end
     } /* endif */

                                      // adjust column
     if ( pDoc->lCursorCol > pDoc->lScrnCols )
     {
         pDoc->lSideScroll += (pDoc->lCursorCol-pDoc->lScrnCols + 1);
         pDoc->lCursorCol   =  pDoc->lScrnCols - 1;
         EQFBDBCS2ND( pDoc, TRUE );           //adjust to the left if
         pDoc->lDBCSCursorCol = pDoc->lCursorCol;
     } /* endif */

     // adjust structure for arabic specificas
     ReAllocArabicStruct( pDoc );


     if ( pTBRow->ulSegNum )     // document already loaded?
     {
       EQFBScrnLinesFromSeg                   // build line table
              ( pDoc,
                pDoc->lCursorRow,              // starting row
                (pDoc->lScrnRows-pDoc->lCursorRow), // number of rows
                pTBRow );                     // starting segment

       EQFBFillPrevTBRow ( pDoc, pDoc->lCursorRow+1 );     // fill top of table

       pDoc->Redraw = REDRAW_ALL;            // redraw the screen
       EQFBScreenData( pDoc );
     } /* endif */
}

//------------------------------------------------------------------------------
// Function name: EQFBCommand
// External function
//------------------------------------------------------------------------------
// Function call:     EQFBCommand(PTBDOCUMENT)
//
//------------------------------------------------------------------------------
// Description:       open the execute command dialog
//
//------------------------------------------------------------------------------
// Parameter:         PTBDOCUMENT pDoc   ptr to document instance
//
//------------------------------------------------------------------------------
// Returncodes:       none
//
//------------------------------------------------------------------------------
// Function flow:     call dialog for execute command
//                    if not possible
//                       display error message
//                    endif
//------------------------------------------------------------------------------

VOID
EQFBCommand
(
   PTBDOCUMENT pDoc
)
{
   INT_PTR      iRc;
   TPEXECUTE    tpExecute;
   HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);

   tpExecute.pDoc = pDoc;
   tpExecute.pKey = NULL;

   DIALOGBOX( pDoc->hwndClient, EQFBCOMMANDDLGPROC, hResMod,
              ID_TB_COMMAND_DLG, &tpExecute, iRc );

   if ( iRc == DID_ERROR )
   {
     UtlError( ERROR_DIALOG_LOAD_FAILED, MB_CANCEL, 0 , NULL, EQF_ERROR );
   }
   else if ( tpExecute.pKey )
   {
     EQFBExecuteKey( pDoc, tpExecute.pKey );
   } /* endif */
} /* end of function EQFBCommand */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     EQFBSettings
//------------------------------------------------------------------------------
// Function call:     EQFBSettings( pDoc )
//------------------------------------------------------------------------------
// Description:       invoke the settings dialog ...
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT  pDoc   pointer to document structure
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:     invoke the dialog and handle error conditions ..
//------------------------------------------------------------------------------

VOID
EQFBSettings
(
   PTBDOCUMENT pDoc
)
{
   HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
   INT_PTR     iRc;
   DIALOGBOX( pDoc->hwndClient, EQFBPROFINITDLGPROC, hResMod,
              ID_TB_PROPERTIES_DLG, pDoc, iRc );

   if ( iRc == DID_ERROR )
   {
     UtlError( ERROR_DIALOG_LOAD_FAILED, MB_CANCEL, 0 , NULL, EQF_ERROR );
   } /* endif */
} /* end of function EQFBSettings */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     EQFBGotoLine
//------------------------------------------------------------------------------
// Function call:     EQFBGotoLine( pDoc )
//------------------------------------------------------------------------------
// Description:       invoke the Goto Line dialog ...
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT  pDoc   pointer to document structure
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:     invoke the dialog and handle error conditions ..
//------------------------------------------------------------------------------

VOID
EQFBGotoLine
(
   PTBDOCUMENT pDoc
)
{
   HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
   INT_PTR       iRc;

   if ( hResMod == NULL ) hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);

   pDoc->flags.fGotoSegMode = FALSE;
   DIALOGBOX( pDoc->hwndClient, EQFBGOTOLINEDLGPROC, hResMod,
              ID_TB_GOTO_DLG, pDoc, iRc );

   if ( iRc == DID_ERROR )
   {
     UtlError( ERROR_DIALOG_LOAD_FAILED, MB_CANCEL, 0 , NULL, EQF_ERROR );
   } /* endif */
} /* end of function EQFBGotoLine */

// invoke the goto line dialog in "goto segment" mode
VOID EQFBGotoSegment
(
   PTBDOCUMENT pDoc
)
{
   HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
   INT_PTR       iRc;

   pDoc->flags.fGotoSegMode = TRUE;

   DIALOGBOX( pDoc->hwndClient, EQFBGOTOLINEDLGPROC, hResMod, ID_TB_GOTO_DLG, pDoc, iRc );

   if ( iRc == DID_ERROR )
   {
     UtlError( ERROR_DIALOG_LOAD_FAILED, MB_CANCEL, 0 , NULL, EQF_ERROR );
   } /* endif */
} /* end of function EQFBGotoLine */


//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     EQFBExecuteKey
//------------------------------------------------------------------------------
// Function call:     EQFBExecuteKey(PTBDOCUMENT,PKEYPROFTABLE)
//------------------------------------------------------------------------------
// Description:       execute the key selected in the command dialog
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT pDoc   ptr to document instance
//                    PKEYPROFTABLE pKey ptr to key table
//------------------------------------------------------------------------------
// Returncodes:       none
//------------------------------------------------------------------------------
// Function flow:     get selected function
//                    if function is allowed at current position
//                      execute function
//                      refresh screen
//                    else
//                      do nothing
//                    endif
//------------------------------------------------------------------------------

VOID  EQFBExecuteKey
(
  PTBDOCUMENT    pDoc,
  PKEYPROFTABLE  pKey
)
{
   USHORT       usAction;                   // action status
   USHORT       usStatus;                   // screen status
   void (*function)( PTBDOCUMENT );         // and function to be processed
   PFUNCTIONTABLE pFuncTab = get_FuncTab();
   
   usAction     = (pFuncTab + pKey->Function)->usAction;
   function     = (pFuncTab + pKey->Function)->function;
   usStatus = EQFBCurrentState( pDoc );
   if ((usStatus & usAction) == usAction )
   {
      (*function)( pDoc );           // execute the function
      /****************************************************/
      /* check if it is a function where pDoc will be     */
      /* freed and therefore is invalid furthermore       */
      /****************************************************/
      if (!((pKey->Function == QUIT_FUNC)
                ||(pKey->Function == FILE_FUNC)))
      {
         EQFBRefreshScreen( pDoc );  // refresh the screen
      } /* endif */
   }
   else
   {
      EQFBFuncNothing( pDoc ); // ignore keystroke
   } /* endif */
}

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     EQFBDispClass
//------------------------------------------------------------------------------
// Function call:     _
//------------------------------------------------------------------------------
// Description:       this function will contain the document instance window
//                    procedure for all AVIO windows
//------------------------------------------------------------------------------
// Input parameter:   if PM:
//                    HWND         hwnd         - window handle
//                    USHORT       msg          - message
//                    WPARAM       mp1          - message parameter
//                    LPARAM       mp2          - message parameter
//                    PTBDOCUMENT  pDoc         - pointer to document struct..
//------------------------------------------------------------------------------
// Output parameter:  MRESULT      mResult       - return value from PM calls
//------------------------------------------------------------------------------
// Returncode type:   _
//------------------------------------------------------------------------------
// Returncodes:       _
//------------------------------------------------------------------------------
// Prerequesits:      _
//------------------------------------------------------------------------------
// Side effects:      _
//------------------------------------------------------------------------------
// Samples:           _
//------------------------------------------------------------------------------
// Function flow:     The following cases are handled:
//                    WM_CREATE,
//                    - create the document instance for the document window
//                    WM_SIZE
//                    - size the window correctly and set the new window
//                      coordinates
//                    WM_PAINT
//                    - paint the window ( WinBeginPaint) and show the VIO
//                    WM_BUTTON1DOWN:
//                    -  Position cursor to pointer and set button1-down
//                       marker
//                    WM_MOUSEMOVE:
//                    - If button 1 down, send mark line to editor
//                    WM_BUTTON1UP:
//                    - reset button1 down marker
//                    WM_BUTTON1DBLCLK:
//                    - Send mark segment indication to the editor
//                    WM_BUTTON2DBLCLK:
//                    - Send un-mark to editor
//                    WM_HSCROLL:
//                    - call horizontal scroll handling ( EQFBHScroll )
//                    WM_VSCROLL:
//                    - call vertical scroll handling ( EQFBVScroll )
//                    WM_DESTROY:
//                    - free the PM specific resources, i.e.
//                        deassociate presentation space
//                        ( VioAssociate (NULL, ..)
//                        destroy presentation space ( VioDestroyPS )
//                        destroy GPI presentation space ( GpiDestroyPS )
//                    default:
//                       pass it on to default window proc WinDefWindowProc
//------------------------------------------------------------------------------
MRESULT
EQFBDispClass
(
  HWND hwnd,                           // window handle
  WINMSG msg,                          // message
  WPARAM mp1,                          // message parameter
  LPARAM mp2,                          // message parameter
  PTBDOCUMENT pDoc                     // pointer to document struct..
)
{
    PTBROWOFFSET pTBRow;
    MRESULT   mResult = FALSE;          // return value from window proc
    HWND     hwndCapt;                  // captured window handle
    HDC      hdc;
    PAINTSTRUCT ps;                                     // pointer to paint struct

    USHORT   usSBType;                  // type of scroll bar msg
    USHORT   usThumbPosition;           // position of scroll bar

                                // Get instance variables
    switch( msg )
    {

        case WM_CREATE:
            hdc = GetDC(hwnd);
            SelectObject( hdc, GetStockObject( ANSI_FIXED_FONT ));
            ReleaseDC( hwnd, hdc );
            break;


        case WM_SIZE:
            /**********************************************************/
            /*       Remember client area size in device units        */
            /**********************************************************/
            if (pDoc)
            {
              pDoc->xClient = SHORT1FROMMP2(mp2) ;
              pDoc->yClient = SHORT2FROMMP2(mp2) ;
              /****************************************************/
              /* position statusbar window and ruler              */
              /****************************************************/
              if ( pDoc->pstEQFGen )
              {
                USHORT usFlags = SWP_NOZORDER | EQF_SWP_NOADJUST |
                                 SWP_NOACTIVATE | SWP_SHOWWINDOW;
                if (((PSTEQFGEN)pDoc->pstEQFGen)->bStatusBar[pDoc->docType] & TP_WND_STATUSBAR)
                {
                  SHORT sCY = (SHORT) WinQuerySysValue( HWND_DESKTOP, SV_CYMENU );
                  pDoc->yClient -= sCY;
                  SetWindowPos( pDoc->hStatusBarWnd, hwnd,
                                0, pDoc->yClient,
                                pDoc->xClient, sCY, usFlags );
                  STATUSBAR( pDoc );
                } /* endif */
                if (((PSTEQFGEN)pDoc->pstEQFGen)->bStatusBar[pDoc->docType] & TP_WND_RULER)
                {
                  RECT rc;
                  HDC  hdc;
                  pDoc->yClient -= pDoc->cy;
                  /********************************************************************/
                  /* force the update of the window ...                               */
                  /********************************************************************/
                  GetClientRect( hwnd, &rc );
                  hdc = GetDC( pDoc->hwndClient );
                  FillRect( hdc, &rc, (HBRUSH)GetStockObject( WHITE_BRUSH ));
                  ReleaseDC (pDoc->hwndClient, hdc);
                  /****************************************************/
                  /* position ruler                                   */
                  /****************************************************/
                  SetWindowPos( pDoc->hRulerWnd, hwnd,
                                0, 0,
                                pDoc->xClient, pDoc->cy, usFlags );
                  RULER( pDoc );
                } /* endif */
              } /* endif */

              {
                RECT rc;
                HDC  hdc;
                GetClientRect( hwnd, &rc );
                if ( pDoc->hwndRichEdit )
                {
                  BOOL fRuler  = ((PSTEQFGEN)pDoc->pstEQFGen)->bStatusBar[pDoc->docType] & TP_WND_RULER;
                  SetWindowPos( pDoc->hwndRichEdit, HWND_TOP,
                                0, (fRuler) ? pDoc->cy : 0, rc.right, pDoc->yClient,
                                SWP_SHOWWINDOW );
                }
                else
                {
                  hdc = GetDC( pDoc->hwndClient );
                  FillRect( hdc, &rc, (HBRUSH)GetStockObject( WHITE_BRUSH ));
                  ReleaseDC (pDoc->hwndClient, hdc);
                } /* endif */
              }
              {
                EQFBVioSetNewDocSize( pDoc );


                pTBRow = pDoc->TBRowOffset+1 + pDoc->lCursorRow; // get first element
                if ( pTBRow->ulSegNum )     // document already loaded?
                {
                  SetScrollbar( pDoc );
                  EQFBScreenCursor( pDoc );      // position cursor and slider
                } /* endif */
              }
              /********************************************************/
              /* store current positions                              */
              /********************************************************/
              if ( pDoc->pstEQFGen )
              {
                HWND hwndOrder;
                SWP  swp;              // size window position
                RECTL rclDummy;        // rectangle structure
                PRECTL prcl;           // pointer to rectangle structure

                switch ( pDoc->docType )
                {
                   case SSOURCE_DOC:
                     prcl = &(((PSTEQFGEN)pDoc->pstEQFGen)->rclEditorSrc);
                     break;
                   case STARGET_DOC:
                     prcl = &(((PSTEQFGEN)pDoc->pstEQFGen)->rclEditorTgt);
                     break;
                   default:
                   case OTHER_DOC:
                     prcl = &rclDummy;
                     break;
                } /* endswitch */

                hwndOrder = hwnd;
                if (hwndOrder && WinQueryWindowPos ( hwndOrder, &swp) &&
                    SWP_FLAG(swp) != EQF_SWP_MAXIMIZE )
                {
                   PRECTL_XLEFT(prcl)      = (LONG)swp.x;
                   PRECTL_XRIGHT(prcl)     = (LONG)(swp.x + swp.cx);
                   PRECTL_YBOTTOM(prcl)    = (LONG)swp.y;
                   PRECTL_YTOP(prcl)       = (LONG)(swp.y + swp.cy);
                } /* endif */


              } /* endif */
              // initiate rescan for spell checking
	  		  if (pDoc->fAutoSpellCheck )
			  {
				if ( pDoc->pvSpellData )
    	        { // force that thread recalcs pusHLType of screen
		           PSPELLDATA pSpellData = (PSPELLDATA) pDoc->pvSpellData;
		           pSpellData->TBFirstLine.ulSegNum = 0;
		           pSpellData->TBFirstLine.usSegOffset = (USHORT)-1; // cannot be segoffs
	            }
			    EQFBWorkThreadTask ( pDoc, THREAD_SPELLSCRN );
              } /* endif */
            } /* endif */
            break;

        case WM_PAINT:
            hdc = BeginPaint(hwnd, &ps );
            pDoc->Redraw |= REDRAW_ALL; // enforce repaint....
            EQFBRefreshScreen( pDoc );  // refresh the screen
            EndPaint(hwnd, &ps);
            mResult = (LRESULT) FALSE;
            break;

         case WM_EQF_FONTCHANGED:
            {
              RECT rc;
              VIOFONTCELLSIZE* pvioFontSize = get_vioFontSize();
              GetClientRect(hwnd, &rc);
              if ( pDoc->pvGlyphSet ) UtlAlloc( &pDoc->pvGlyphSet, 0, 0, NOMSG );
              PostMessage( hwnd, WM_SIZE, 0, MAKELPARAM( rc.right, rc.bottom ));
              EQFBSetNewCellSize( pDoc, (pvioFontSize + pDoc->docType)->cx, (pvioFontSize + pDoc->docType)->cy );
            }
            break;

        case WM_BUTTON1DOWN:    // Position cursor to pointer
            /**********************************************************/
            /* use button1down ONLY if window has focus ...           */
            /**********************************************************/
            if ( GETFOCUS() == hwnd)
            {
              /************************************************************/
              /* get rid of any selection (if in CUA mode and no shift key*/
              /************************************************************/
              if (pDoc->usTripleClkTimerID )
              {
                                // it is a triple click: mark segment
                KillTimer(hwnd, pDoc->usTripleClkTimerID);
                pDoc->usTripleClkTimerID = 0;
                EQFBFuncMarkSegment( pDoc );
                EQFBScreenData( pDoc );
                if ( EQFBMousePosition( pDoc, msg, mp1, mp2 ) )
                {
                   SETCAPTURE( hwnd );            // capture the mouse
                } /* endif */
                EQFBRefreshScreen( pDoc );
              }
              else
              {
                mResult = EQFBButton1Down(hwnd, msg, mp1, mp2, pDoc);
              } /* endif */
            }
            else
            {
              // inform the window of focus change...
                SendMessage( ((PSTEQFGEN)pDoc->pstEQFGen)->hwndTWBS,
                              WM_EQF_SETFOCUS,
                              0, MP2FROMP( hwnd ));
            } /* endif */

            break;

        case WM_MOUSEMOVE:      // If button 1 down, send mark block to editor
            hwndCapt = GETCAPTURE;
            if ( hwndCapt == hwnd )
            {
              EQFBMousePosition( pDoc, msg, mp1, mp2 );
            }
            else
            {
              /********************************************************/
              /* if nothing captured ignore the reset of WinSetCapture*/
              /********************************************************/
              if ( hwndCapt )
              {
                RELEASECAPTURE;               // release the mouse
              } /* endif */
            } /* endif */
            mResult = WinDefWindowProc ( hwnd, msg, mp1, mp2 );
            break;

        case WM_BUTTON1UP:             // Send
            EQFBWorkSegCheck( pDoc );  // check if segment changed
            RELEASECAPTURE;            // release the mouse
            mResult =  WinDefWindowProc ( hwnd, msg, mp1, mp2 );
            break;

        case WM_TIMER:
            if (mp1 == pDoc->usTripleClkTimerID)
            {
               KillTimer(hwnd, pDoc->usTripleClkTimerID);
               pDoc->usTripleClkTimerID = 0;
            }
            break;

        case WM_BUTTON1DBLCLK:          // mark segment if in valid area
            if (pDoc->lCursorRow== min(pDoc->mouseRow,pDoc->lScrnRows-1)
                  &&  pDoc->lCursorCol == pDoc->mouseCol )
            {
              if (!EQFBOnTRNote(pDoc) )
              {
                if (!pDoc->usTripleClkTimerID)
                {
                   ULONG   ulDblClickTime;
                   ulDblClickTime = GetDoubleClickTime();
                   pDoc->usTripleClkTimerID = TRIPLE_ID;
                   SetTimer(hwnd, pDoc->usTripleClkTimerID,
                                    ulDblClickTime, NULL);
                   // now invoke action of doubleclick: mark word
                   EQFBFuncWordMark(pDoc);
                   EQFBScreenData(pDoc);
                }
                else
                {
                  KillTimer(hwnd, pDoc->usTripleClkTimerID);
                  pDoc->usTripleClkTimerID = 0;
                  // it is twice a double click, so do one double click
                  // double click should now select word
                  EQFBFuncWordMark(pDoc);
                  EQFBScreenData(pDoc);
                }
              }
            } /* endif */
            mResult = WinDefWindowProc ( hwnd, msg, mp1, mp2 );
            break;

        case WM_BUTTON2DBLCLK:  // Send un-mark to editor
            EQFBFuncMarkClear( pDoc );
            EQFBScreenData( pDoc );                   // display screen
            mResult = WinDefWindowProc ( hwnd, msg, mp1, mp2 );
            break;


        case WM_RBUTTONDOWN:
          /* Draw the "floating" popup in the app's client area */
          {
            RECT rc;
            SHORT sPopUpId;

            POINT pt;
            pt.x = LOWORD(mp2);
            pt.y = HIWORD(mp2);
            GetClientRect( hwnd, (LPRECT)&rc);
            if (PtInRect ((LPRECT)&rc, pt))
            {
              /********************************************************/
              /* find correct popup ...                               */
              /********************************************************/
              switch ( pDoc->docType )
              {
                case TRNOTE_DOC:       // TRNOTE document
                  sPopUpId = ID_TPRO_POPUP_TRNOTE;
                  break;
                case OTHER_DOC:        // other documents besides ....
                  sPopUpId = (pDoc->hwndRichEdit)? ID_TRTF_POPUP_OTHER : ID_TPRO_POPUP_OTHER;
                  break;
                case SSOURCE_DOC:      // source segmented document
                  sPopUpId = (pDoc->hwndRichEdit)? ID_TRTF_POPUP_SRC : ID_TPRO_POPUP_SRC;
                  break;
                case STARGET_DOC:      // target segmented document
                  sPopUpId = (pDoc->hwndRichEdit)? ID_TRTF_POPUP_TRANS : ID_TPRO_POPUP_TRANS;
                  /****************************************************/
                  /* if spellcheck, get correct position in offset    */
                  /* within segment and check if we have to modifiy   */
                  /* menu...                                          */
                  /****************************************************/
                  if ( pDoc->fAutoSpellCheck )
                  {
                    PTBSEGMENT pSeg;
                    EQFBMousePosition( pDoc, msg, mp1, mp2 );
                    /**************************************************/
                    /* check if cursor position is on misspelled word */
                    /* if so activate spellcheck                      */
                    /**************************************************/
                    pSeg = EQFBGetSegW( pDoc, pDoc->TBCursor.ulSegNum );
                    if ( pSeg )
                    {
                      PSTARTSTOP pHLCurrent = (PSTARTSTOP) pSeg->pusHLType;
                      if ( pHLCurrent )
                      {
                        while ( pHLCurrent->usType &&
                                (pHLCurrent->usStart > pDoc->TBCursor.usSegOffset ||
                                pHLCurrent->usStop < pDoc->TBCursor.usSegOffset ))
                        {
                          pHLCurrent++;
                        } /* endwhile */
                        if ( pHLCurrent->usType == MISSPELLED_HIGHLIGHT )
                        {
                          EQFBSpellPullDown( hwnd, pt, pDoc, ID_TPRO_POPUP_SPELL,
                                             pSeg->pDataW,
                                             pHLCurrent->usStart, pHLCurrent->usStop );
                          sPopUpId = 0;
                        } /* endif */
                      } /* endif */
                    } /* endif */
                  } /* endif */
                  break;
                default:
                  sPopUpId = 0;
                  break;
              } /* endswitch */
              if (sPopUpId)
              {
                HandlePopupTEMenu( hwnd, pt, sPopUpId, pDoc );
              } /* endif */
              mResult = FALSE;
            } /* endif */
          }
          break;
      case WM_EQF_PROCESSTASK:
        break;


        case WM_HSCROLL:
            usSBType        = (USHORT)LOWORD(mp1);
            usThumbPosition = (USHORT)HIWORD(mp1);

            EQFBHScroll ( pDoc, usSBType, usThumbPosition );
            break;

        case WM_VSCROLL:
            usSBType        = (USHORT)LOWORD(mp1);
            usThumbPosition = (USHORT)HIWORD(mp1);
            EQFBVScroll ( pDoc, usSBType, usThumbPosition );
            break;

        case WM_MOUSEWHEEL:
          {
            int zDelta = (short)HIWORD(mp1);
            if ( zDelta > 0 )
            {
              EQFBVScroll ( pDoc, SB_LINEUP, 0 );
            }
            else if ( zDelta < 0 )
            {
              EQFBVScroll ( pDoc, SB_LINEDOWN, 0 );
            } /* endif */
          }
          break;

        case WM_DESTROY:
            if ( pDoc && pFindData &&
                 (pFindData->pDoc == pDoc ) && pFindData->hwndFindDlg )
            {
              EQFBFindCloseModeless( pFindData->hwndFindDlg, 0, 0L );
            } /* endif */
            break;


        /**************************************************************/
        /* special handling of CARET for windows                      */
        /**************************************************************/
        case WM_KILLFOCUS:
          WinSendMsg( hwnd, WM_NCACTIVATE, FALSE, NULL );
          HideCaret( hwnd );
          if ( pDoc && pDoc->hCaretBitmap  )
          {
            DeleteObject( pDoc->hCaretBitmap );
            pDoc->hCaretBitmap = NULLHANDLE;
          } /* endif */
          DestroyCaret();

          /********************************************************/
          /* set IME conversion window back to defaults           */
          /********************************************************/
          if ( pDoc && pDoc->hlfIME )
          {
            ImeMoveConvertWin(pDoc, hwnd, -1, -1 );
          } /* endif */
          break;

        case WM_EQF_SETFOCUS:
        case WM_SETFOCUS:

          /************************************************************/
          /* set correct doc. window to be active for find            */
          /************************************************************/
          if ( pFindData && pFindData->pDoc )
          {
            pFindData->pDoc = pDoc;
          } /* endif */

          // activate the selected translation environment

          /************************************************************/
          /* activate the selected translation environment            */
          /************************************************************/
          if ( (pDoc->docType == STARGET_DOC) && !pDoc->fTransEnvAct )
          {
            ActTransEnv( pDoc );
          } /* endif */

          BringWindowToTop( hwnd );
          WinSendMsg( hwnd, WM_NCACTIVATE, TRUE, NULL );

          if (IS_RTL(pDoc))
          {
            if (pDoc->pUserSettings->UserOptFlags.bBidiLogicDisplay)
            {
                  EQFBSysScrnCurShape( pDoc, (CURSOR)pDoc->usCursorType );

//             pDoc->hCaretBitmap = LoadImage( hResMod,
//                                       MAKEINTRESOURCE( ID_BIDI_CARET_POP ),
//                                        IMAGE_BITMAP,
//                                        pDoc->cx,
//                                        pDoc->cy,
//                                        LR_DEFAULTCOLOR );
//
//              CreateCaret(hwnd, (HBITMAP) pDoc->hCaretBitmap,
//                          pDoc->vioCurShapes[pDoc->usCursorType].cx,
//                          pDoc->vioCurShapes[pDoc->usCursorType].cEnd);


              if ( pDoc->fTARight )
              {
                if (pDoc->pArabicStruct && pDoc->pArabicStruct->plCaretPos)
                {
                  PLONG pl = &pDoc->pArabicStruct->plCaretPos[pDoc->lCursorRow*(pDoc->lScrnCols+1)];
                  SetCaretPos( pDoc->lScrnCols  * pDoc->cx - 4 - pl[pDoc->lCursorCol],
                               pDoc->lCursorRow * pDoc->cy + pDoc->ulRulerSize );

                }
                else
                {
                  SetCaretPos( (pDoc->lScrnCols - pDoc->lCursorCol) * pDoc->cx - 4,
                                pDoc->lCursorRow * pDoc->cy + pDoc->ulRulerSize );
                }
              }
              else
              {
                SetCaretPos( pDoc->lCursorCol * pDoc->cx,
                             pDoc->lCursorRow * pDoc->cy + pDoc->ulRulerSize );

              }
            }
            else
            {
              CreateCaret(hwnd, (HBITMAP)NULL,
                          pDoc->vioCurShapes[pDoc->usCursorType].cx,
                          pDoc->vioCurShapes[pDoc->usCursorType].cEnd);

              SetCaretPos( (pDoc->lScrnCols - pDoc->lCursorCol -1) * pDoc->cx,
                            pDoc->lCursorRow * pDoc->cy + pDoc->ulRulerSize );
            }
          }
          else
          {
            CreateCaret(hwnd, (HBITMAP)NULL,
                        pDoc->vioCurShapes[pDoc->usCursorType].cx,
                        pDoc->vioCurShapes[pDoc->usCursorType].cEnd);
            SetCaretPos( pDoc->lCursorCol * pDoc->cx,
                         pDoc->lCursorRow * pDoc->cy + pDoc->ulRulerSize );
          } /* endif */


          ShowCaret( hwnd );

          /************************************************************/
          /* get rid of any selection (if in CUA mode )               */
          /************************************************************/
            if ( pDoc->pBlockMark && ((PEQFBBLOCK)pDoc->pBlockMark)->pDoc )
            {
              memset( pDoc->pBlockMark, 0, sizeof( EQFBBLOCK ));
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

          break;

       case WM_ACTIVATE:
          WinSetFocus( HWND_DESKTOP, hwnd );
          break;

        case WM_MOUSEACTIVATE:
          /********************************************************/
          /* inform TENV that the active window has changed.....  */
          /********************************************************/
          if ( (hwnd != GetFocus()) && (pDoc->pstEQFGen ))
          {
            WinSendMsg( ((PSTEQFGEN)pDoc->pstEQFGen)->hwndTWBS,
                        WM_EQF_SETFOCUS,
                        0, MP2FROMHWND( hwnd ));
          } /* endif */
          mResult = MA_ACTIVATE;  // ANDEAT;
          break;


        /**************************************************************/
        /* forward menu selection to our MDI client window, if it is  */
        /* not '+' which might activate our sysmenu ....              */
        /**************************************************************/
       case WM_MENUCHAR:
         if ( (mp1 == '-') && (GetWindowLong(hwnd, GWL_STYLE) & WS_SYSMENU) )
         {
           mResult = WinDefWindowProc ( hwnd, msg, mp1, mp2 );
         }
         else
         {
           mResult = SendMessage( ((PSTEQFGEN)pDoc->pstEQFGen)->hwndTWBS,
                                  msg,
                                  mp1, mp2 );
         } /* endif */
         break;

        case WM_MENUSELECT:
         if ( (mp1 == '-') && (GetWindowLong(hwnd, GWL_STYLE) & WS_SYSMENU) )
         {
           mResult = WinDefWindowProc ( hwnd, msg, mp1, mp2 );
         }
         else
         {
           mResult = SendMessage( ((PSTEQFGEN)pDoc->pstEQFGen)->hwndTWBS,
                                  msg,
                                  mp1, mp2 );
         } /* endif */
         break;

        default:
          mResult = DefWindowProcW( hwnd, msg, mp1, mp2 );
          break;

    } /* switch */
    return mResult;
}

/**********************************************************************/
/* adjust the scrollbar positions under Windows                       */
/* This has to be done due to the fact, that the WM_SIZE is not passed*/
/* to the window AFTER allocating pDoc.                               */
/**********************************************************************/
VOID SetScrollbar
(
  PTBDOCUMENT  pDoc
)
{
  PTBROWOFFSET pTBRow;                                     // table row offset
  ULONG        ulOldRow;
  ULONG        ulTempSeg;

  if ( !pDoc->hwndRichEdit )
  {
    pTBRow = pDoc->TBRowOffset+1 + pDoc->lCursorRow;         // get first element

    if ( GetWindowLong( pDoc->hwndFrame, GWL_STYLE ) & WS_VSCROLL )
    {
      ulOldRow = max(pDoc->ulMaxSeg - 2, 2);                 //set vertical
      ulTempSeg = (pTBRow->ulSegNum != 0) ? (pTBRow->ulSegNum - 1) : 0;
      if ((pDoc->ulMaxSeg) > MAX_VSCROLLRANGE )              // also in SetScrollPos!
      {
        double dbMaxSeg = (double)pDoc->ulMaxSeg;
        double dbRange = (double)MAX_VSCROLLRANGE;
        double dbValue = (double)ulOldRow * dbRange / dbMaxSeg;
        ulOldRow = (ULONG)(dbValue+0.9999);
        if ( ulTempSeg != 0 )
        {
          dbValue = (double)ulTempSeg * dbRange / dbMaxSeg;
          ulTempSeg = (ULONG)(dbValue+0.9999);
        } /* endif */
      } /* endif */
      SetScrollRange (pDoc->hwndFrame, SB_VERT,     //scroll slider
                      0,ulOldRow,FALSE);            // min - max

      SetScrollPos(pDoc->hwndFrame, SB_VERT,
                    ulTempSeg, TRUE );    // Position
    } /* endif */

    if ( GetWindowLong( pDoc->hwndFrame, GWL_STYLE ) & WS_HSCROLL )
    {
      SetScrollRange (pDoc->hwndFrame, SB_HORZ,
                      0, 255, FALSE);
      if (pDoc->fTARight)
      {
		if (pDoc-> lSideScroll < 255)
		{
	        SetScrollPos(pDoc->hwndFrame, SB_HORZ,255 - pDoc->lSideScroll, TRUE );
	    }
	    else
	    {
	    	SetScrollPos(pDoc->hwndFrame, SB_HORZ,0, TRUE );
	    }
	  }
      else
      {
        SetScrollPos(pDoc->hwndFrame, SB_HORZ,pDoc->lSideScroll, TRUE );
      }
    } /* endif */
  } /* endif */
}
/**********************************************************************/
/* set mouse positions ...                                            */
/**********************************************************************/
BOOL
EQFBMousePosition
(
  PTBDOCUMENT pDoc,                    // pointer to document
  WINMSG      msg,                     // passed message
  WPARAM      mp1,                     // message parameter
  LPARAM      mp2                      // message parameter
)
{
  LONG     lOldRow = pDoc->lCursorRow; // remember old cursor row/col
  LONG     lOldCol = pDoc->lCursorCol;
  BOOL     fOK = TRUE;                 // success indicator

  mp2;                                 // get rid of compiler warnings
  mp1;
    assert( (pDoc->cy > 0) );
    assert( (pDoc->cx > 0) );
    pDoc->mouseRow = ((HIWORD( mp2 )- pDoc->ulRulerSize) / pDoc->cy );
    pDoc->lCursorRow = min( pDoc->mouseRow, pDoc->lScrnRows-1 ) ;
    if ( IS_RTL( pDoc ) )
    {
      if (!pDoc->pUserSettings->UserOptFlags.bBidiLogicDisplay)
      {
        PBIDISTRUCT   pBidiStruct = pDoc->pBidiStruct;
        PUINT         pusOrder    = pBidiStruct->BidiLine[pDoc->lCursorRow].pusOrder;
        ULONG         ulOffset;
        USHORT        usPos;
        /******************************************************************/
        /* fill in cursor row                                             */
        /******************************************************************/
        pDoc->mouseCol = (LOWORD( mp2 ) / pDoc->cx );
        if ( pusOrder )
        {
          ulOffset = pBidiStruct->BidiLine[pDoc->lCursorRow].usOffset + pDoc->mouseCol - 1;
          usPos    = 0;
          while ( usPos < pDoc->lScrnCols + ulOffset )
          {
            if ( pusOrder[ usPos ] == ulOffset)
            {
              pDoc->mouseCol = usPos;
              break;
            }
            else
            {
              usPos++;
            } /* endif */
          } /* endwhile */
        } /* endif */
      }
      else
      {
        pDoc->mouseCol = (LOWORD( mp2 ) / pDoc->cx );
        if (pDoc->fTARight)
        {
          pDoc->mouseCol = pDoc->lScrnCols - pDoc->mouseCol;
                  if (pDoc->pArabicStruct && pDoc->pArabicStruct->plCaretPos)
                  {
            ULONG ulRow = min( pDoc->mouseRow, pDoc->lScrnRows-1 ) ;
            PLONG pl = &pDoc->pArabicStruct->plCaretPos[ulRow*(pDoc->lScrnCols+1)];
                        LONG  lPos = LOWORD(mp2);
                        INT   i = 0;
                        LONG  lMax = (pDoc->lScrnCols + 1) * (pDoc->cx) - 4;
            lPos = (lPos < lMax ) ? (lMax - lPos) : 0;

                        while ( (i < pDoc->lScrnCols) && (lPos >= pl[i]) )
                        {
                                i++;
                        }
                        if (i > 0) i--;
                        pDoc->mouseCol = i;
                  }
        } /* endif */
      }
    }
    else
    {
        USHORT usCx = LOWORD(mp2);
        if ( pDoc->pArabicStruct && pDoc->pArabicStruct->plCaretPos)
        {
            PLONG plCaretPos = pDoc->pArabicStruct->plCaretPos + pDoc->lCursorRow * (pDoc->lScrnCols+1);
            USHORT usCol = 0;
            while ( (usCol < pDoc->lScrnCols) && ( plCaretPos[usCol] <= usCx )  )
            {
                usCol++;
            }
            pDoc->mouseCol = usCol - 1;
        }
        else
        {
            pDoc->mouseCol = (LOWORD( mp2 ) / pDoc->cx );
        }
    }

  /********************************************************************/
  /* set to new position                                              */
  /********************************************************************/
  pDoc->lCursorRow = min( pDoc->mouseRow, pDoc->lScrnRows-1 ) ;
  pDoc->lCursorCol = pDoc->mouseCol;  // get new column position

  if ( (pDoc->lCursorRow != lOldRow) || ( pDoc->lCursorCol != lOldCol )  )
  {
    /******************************************************************/
    /* if no block selected right now - start it                      */
    /******************************************************************/
    if ( ! ((PEQFBBLOCK)pDoc->pBlockMark)->pDoc && (msg == WM_MOUSEMOVE))
    {
      EQFBFuncMarkBlock( pDoc );     // Mark current block
    } /* endif */

    EQFBCurSegFromCursor( pDoc );       // get TBCursor
    if ( pDoc->TBCursor.ulSegNum == 0 ) // stay at old position
    {
       WinAlarm( HWND_DESKTOP, WA_WARNING );
       pDoc->lCursorRow = lOldRow;
       pDoc->lCursorCol = lOldCol;
       EQFBCurSegFromCursor( pDoc );     // get TBCursor
       fOK = FALSE;                      // do not capture ...
    }
    else
    {
       EQFBDBCS2ND (pDoc, FALSE);        // correct if on DBCS 2nd
       pDoc->lDBCSCursorCol = pDoc->lCursorCol;
       /*****************************************************************/
       /* check if we have to update the mark...                        */
       /*****************************************************************/
       if ( msg != WM_BUTTON1DOWN )
       {
         PEQFBBLOCK  pstBlock = (PEQFBBLOCK)pDoc->pBlockMark;
         if ( pstBlock->ulSegNum == pDoc->TBCursor.ulSegNum )
         {
           EQFBFuncMarkBlock( pDoc );     // Mark current block
         }
         else
         if ( pstBlock->ulSegNum > pDoc->TBCursor.ulSegNum )
         {
           pstBlock->usStart = pDoc->TBCursor.usSegOffset;
           pstBlock->ulSegNum = pDoc->TBCursor.ulSegNum;
           pDoc->Redraw     |= REDRAW_ALL;
         }
         else
         {
           pstBlock->ulEndSegNum = pDoc->TBCursor.ulSegNum;
           pstBlock->usEnd = pDoc->TBCursor.usSegOffset;
           pDoc->Redraw    |= REDRAW_ALL;
         } /* endif */
         EQFBScreenData( pDoc );        // update screen
       } /* endif */

       EQFBScreenCursor( pDoc );      // position cursor and slider
    } /* endif */
  } /* endif */
  return fOK;
}



/**********************************************************************/
/* adjust x and y values that document will fit in provided window    */
/* highly environment dependant                                       */
/**********************************************************************/

 VOID EQFBDocPosOnScreen
 (
   PLOADSTRUCT pLoad       // load structure
 )
 {
   RECTL      rclParent;               // size of parent

   // adjust doc. window size to translation environment size
   WinQueryWindowRect( pLoad->hwndParent, &rclParent );
   // if window too small yTop might be zero
   if ( RECTL_YTOP(rclParent) )
   {
     if ( RECTL_YTOP(pLoad->rclPos) >
          RECTL_YTOP(rclParent) + 2*WinQuerySysValue(HWND_DESKTOP,SV_CYSIZEBORDER)
          + WinQuerySysValue(HWND_DESKTOP,SV_CYTITLEBAR)  )
     {
        RECTL_YTOP(pLoad->rclPos) = RECTL_YTOP(rclParent);
     } /* endif */
     if ( RECTL_XRIGHT(pLoad->rclPos) >
         RECTL_XRIGHT(rclParent) + 2*WinQuerySysValue(HWND_DESKTOP,SV_CXSIZEBORDER))
     {
       RECTL_XRIGHT(pLoad->rclPos) =
         RECTL_XRIGHT(rclParent) + 2*WinQuerySysValue(HWND_DESKTOP,SV_CXSIZEBORDER);
     } /* endif */
   } /* endif */


   if ( RECTL_XRIGHT(pLoad->rclPos) > RECTL_XLEFT(pLoad->rclPos) )           /* KIT0898A */
   {                                                           /* KIT0898A */
     pLoad->pDoc->xClient=                                     /* KIT0898A */
            (SHORT)(RECTL_XRIGHT(pLoad->rclPos)-RECTL_XLEFT(pLoad->rclPos)); /* KIT0898A */
   }                                                           /* KIT0898A */
   else                                                        /* KIT0898A */
   {                                                           /* KIT0898A */
     pLoad->pDoc->xClient = 10;                                /* KIT0898A */
   } /* endif */                                               /* KIT0898A */
                                                               /* KIT0898A */
   if ( RECTL_YTOP(pLoad->rclPos) > RECTL_YBOTTOM(pLoad->rclPos))/* KIT0898A */
   {                                                           /* KIT0898A */
     pLoad->pDoc->yClient =                                    /* KIT0898A */
          (SHORT) (RECTL_YTOP(pLoad->rclPos)-RECTL_YBOTTOM(pLoad->rclPos));  /* KIT0898A */
   }                                                           /* KIT0898A */
   else                                                        /* KIT0898A */
   {                                                           /* KIT0898A */
     pLoad->pDoc->yClient = 3;                                 /* KIT0898A */
   } /* endif */                                               /* KIT0898A */


   {
     /*****************************************************************/
     /* ensure that our starting size will not jeopardize our min size*/
     /*****************************************************************/
     MINMAXINFO  mmi;
     SendMessage( pLoad->pDoc->hwndFrame, WM_GETMINMAXINFO, 0, (LPARAM) &mmi );
     pLoad->pDoc->xClient = max( (SHORT)pLoad->pDoc->xClient,
                                 (SHORT)mmi.ptMinTrackSize.x);
     pLoad->pDoc->yClient = max( (SHORT)pLoad->pDoc->yClient,
                                 (SHORT)mmi.ptMinTrackSize.y);
   }


                  // set row and columns of currently available size
   pLoad->pDoc->lScrnCols = max(pLoad->pDoc->xClient / pLoad->pDoc->cx, 10);
   pLoad->pDoc->lScrnRows = max(pLoad->pDoc->yClient / pLoad->pDoc->cy, 3 );

   WinSetWindowPos( pLoad->pDoc->hwndFrame,   // set window position
                    HWND_TOP,
                    (SHORT) RECTL_XLEFT(pLoad->rclPos),
                    (SHORT) RECTL_YBOTTOM(pLoad->rclPos),
                    (SHORT) pLoad->pDoc->xClient,
                    (SHORT) pLoad->pDoc->yClient,
                    (USHORT) pLoad->fsFlagStyle );

   return;
 }

/*/////////////////////////////////////////////////////////////////////
:h3.EQFBInit - do the init handling for the translation browser
*//////////////////////////////////////////////////////////////////////
// Description:
//  This function will init the editor
// Flow:
//  Currently only our window class will be registered here
//
//  Arguments:
//     VOID
//
//  Output:
//     fOK       : success indicator  TRUE/FALSE
///////////////////////////////////////////////////////////////////////
BOOL EQFBInit ( VOID )
{

   BOOL fOK = TRUE;                    // success indicator
   HAB        hab;                     // anchor block handle
   CHAR       szSysProps[MAX_EQF_PATH];// path to system properties
   ULONG      ulBytesRead;             // no of bytes read by UtlLoadFile
   WNDCLASSW  wndclassW;
   HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
   hab = (HAB)    UtlQueryULong( QL_HAB );    /* Anchor block handle          */

   // Unicode initialization
   InitUnicode();
     /*******************************************************************/
     /* register the Editor window class                                */
     /*******************************************************************/
     wndclassW.style = CS_DBLCLKS;
     wndclassW.lpfnWndProc = EQFBWNDPROC;
     wndclassW.cbClsExtra = 0;
     wndclassW.cbWndExtra = sizeof(PSZ);
     wndclassW.hInstance = (HINSTANCE)hab;
     wndclassW.hIcon = LoadIcon( hResMod, MAKEINTRESOURCE( DOC_ICON ) );
     wndclassW.hCursor = LoadCursor(NULL, IDC_ARROW);
     wndclassW.hbrBackground = (HBRUSH)GetStockObject (WHITE_BRUSH);
     wndclassW.lpszMenuName = NULL;
     wndclassW.lpszClassName = L"EQFB";

     RegisterClassW(&wndclassW);

     wndclassW.style = CS_DBLCLKS;
     wndclassW.lpfnWndProc = EQFBWNDPROCRTF;
     wndclassW.cbClsExtra = 0;
     wndclassW.cbWndExtra = sizeof(PSZ);
     wndclassW.hInstance = (HINSTANCE)hab;
     wndclassW.hIcon = LoadIcon( hResMod, MAKEINTRESOURCE( DOC_ICON ) );
     wndclassW.hCursor = LoadCursor(NULL, IDC_ARROW);
     wndclassW.hbrBackground = (HBRUSH)GetStockObject (WHITE_BRUSH);
     wndclassW.lpszMenuName = NULL;
     wndclassW.lpszClassName = L"EQFBRTF";

     RegisterClassW(&wndclassW);

   //--- get name of system properties ---
   if ( fOK )
   {
      GetStringFromRegistry( APPL_Name, KEY_SysProp, szSysProps, sizeof( szSysProps ), "" );
      if ( !szSysProps[0] )
      {
        fOK = FALSE;
      } /* endif */
   } /* endif */

   //--- load system properties ---
   if ( fOK )
   {
      PPROPSYSTEM pNewSysProp = NULL;
      set_pSysProp(pNewSysProp);
      fOK = UtlLoadFileL( szSysProps,
                         (PVOID *)&pNewSysProp,
                         &ulBytesRead,
                         FALSE,
                         TRUE );
      set_pSysProp(pNewSysProp);
   } /* endif */

   if ( fOK )
   {
     SHORT sRC = 0;

      /*******************************************************************/
      /* do initialisation for BidiCase - if not done yet...             */
      /*******************************************************************/
      InitBIDIVars();

      fOK = ( !sRC );
   } /* endif */

   if ( fOK )
   {
      fOK = EQFBBuildKeyTable( get_KeyTable() );
   } /* endif */

  /********************************************************************/
  /* register status bar class                                        */
  /********************************************************************/
  {
	 WNDCLASSW  wndclass;
     CHAR_W szClass[80];
     wndclass.style = CS_DBLCLKS;
     wndclass.lpfnWndProc = TPSTATUSBARWNDPROC;
     wndclass.cbClsExtra = 0;
     wndclass.cbWndExtra = sizeof(PSZ);
     wndclass.hInstance = (HINSTANCE)hab;
     wndclass.hIcon = NULL;
     wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
     wndclass.hbrBackground = (HBRUSH)GetStockObject (WHITE_BRUSH);
     wndclass.lpszMenuName = NULL;
     wndclass.lpszClassName = ASCII2Unicode(TP_STATUSBAR, szClass, 0L);
     RegisterClassW(&wndclass);
  }
  /********************************************************************/
  /* register ruler class                                             */
  /********************************************************************/
    {
     WNDCLASS  wndclass;
     wndclass.style = CS_DBLCLKS;
     wndclass.lpfnWndProc = TPRULERWNDPROC;
     wndclass.cbClsExtra = 0;
     wndclass.cbWndExtra = sizeof(PSZ);
     wndclass.hInstance = (HINSTANCE)hab;
     wndclass.hIcon = NULL;
     wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
     wndclass.hbrBackground = (HBRUSH)GetStockObject (WHITE_BRUSH);
     wndclass.lpszMenuName = NULL;
     wndclass.lpszClassName = TP_RULER;

     RegisterClass(&wndclass);
    }

   return ( fOK );
}

/**********************************************************************/
/*  function to pop up the hotmenu ...                                */
/*   -- the pop-up will be displayed on the character position...     */
/**********************************************************************/
void  EQFBFuncHotPopUp
(
   PTBDOCUMENT  pDoc
)
{
   POINT  lppt;
   GetCaretPos( &lppt );
   EQFBDispClass( pDoc->hwndClient, WM_RBUTTONDOWN,
                  0, MP2FROMLONG(*((PLONG)&lppt)), pDoc );
};

#ifdef _PTM
/**********************************************************************/
/* invoke the save as dialog ...                                      */
/**********************************************************************/
void EQFBFuncSaveAs
(
  PTBDOCUMENT  pDoc
)
{
  CHAR chDocName[ MAX_EQF_PATH ];
  PSZ  pFolderObjName;
  pFolderObjName =
    ((PDOCUMENT_IDA)((PSTEQFGEN)pDoc->pstEQFGen)->pDoc)->szFolderObjName;
  sprintf( chDocName, "%s\\%s",
           pFolderObjName, UtlGetFnameFromPath( pDoc->szDocName ));

  DocSaveAsDocument( chDocName, pFolderObjName, ID_PTM_SAVEAS_DLG );
}
#endif

/**********************************************************************/
/* return    find&change dlg handle                                   */
/**********************************************************************/
 HWND EQFBFindChangeDlgHwnd()
 {
   return( (pFindData && pFindData->hwndFindDlg) ? pFindData->hwndFindDlg : NULL );
 }
/**********************************************************************/
/* Show/Hide find&change dlg...                                       */
/**********************************************************************/
 VOID EQFBShowFindChangeDlg
 (
    BOOL fShow                         // show or hide
 )
 {
   if ( pFindData && pFindData->hwndFindDlg )
   {
     WinShowWindow( pFindData->hwndFindDlg , fShow );
   } /* endif */
 }

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     TPSTATUSBARWNDPROC
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
TPSTATUSBARWNDPROC
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
        TPFillStatusBar( hwnd );
        break;
      default:
        mResult = DefWindowProcW( hwnd, msg, mp1, mp2 );
        break;

  } /* switch */

  return mResult ;
} /* end of function TPSTATUSBARWNDPROC */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     TPFillStatusBar
//------------------------------------------------------------------------------
// Function call:     TPFillStatusBar(hwnd, hps)
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


void FillStatusField( HDC hdc, PSZ pData, PULONG pulStart, ULONG ulCY,
                      LONG lCol )
{
  RECT        rectl;
  ULONG       ulWidth;
  LONG cx, cy;

  TEXTSIZE( hdc, pData, cx, cy );
  ulWidth = cx + 4;

  rectl.top    = 1;
  rectl.left   = *pulStart - ulWidth;
  rectl.right  = *pulStart;
  *pulStart   -= ulWidth;

  rectl.bottom    = ulCY - 1;
  if (!UtlIsHighContrast())
  {
    ERASERECT( hdc, (const RECT*) &rectl, CLR_PALEGRAY );
    DRAWTEXT( hdc, pData, rectl, lCol, CLR_PALEGRAY,
            DT_CENTER | DT_VCENTER );

    rectl.top    = 0;
    rectl.bottom    = ulCY - 0;
    FrameRect( hdc, &rectl, (HBRUSH)GetStockObject(GRAY_BRUSH) );
  }
  else
  {
	DWORD dwRGB_WINDOW = GetSysColor(COLOR_WINDOW);
	DWORD dwRGB_WINDOWTEXT = GetSysColor(COLOR_WINDOWTEXT);
	ERASERECT( hdc, (const RECT*) &rectl, dwRGB_WINDOW );
    DRAWTEXT( hdc, pData, rectl, dwRGB_WINDOWTEXT, dwRGB_WINDOW,
            DT_CENTER | DT_VCENTER );

    rectl.top    = 0;
    rectl.bottom    = ulCY - 0;
    FrameRect( hdc, &rectl, (HBRUSH)(COLOR_WINDOW+1) );

  }
}


VOID
TPFillStatusBar
(
  HWND  hwnd                           // window handle
)
{
  RECT        rectl;
  PTBDOCUMENT pTBDoc = ACCESSWNDIDA( hwnd, PTBDOCUMENT );
  SWP         swp;
  ULONG       ulStart;
  ULONG       ulCY = WinQuerySysValue( HWND_DESKTOP, SV_CYMENU );
  CHAR        chBuffer[100];
  HFONT   hFontDlg;

  PAINTSTRUCT ps;
  HDC         hdc;


  /********************************************************************/
  /* force update of necessary info - if requested                    */
  /********************************************************************/
  if ( pTBDoc->hwndRichEdit && pTBDoc->pDispFileRTF->fTBCursorReCalc )
  {
    EQFBUpdateTBCursor( pTBDoc );
  } /* endif */

  hdc = BeginPaint(hwnd, &ps );
  WinQueryWindowPos( hwnd, &swp );

  ulStart = swp.cx;
  hFontDlg = (HFONT)GetStockObject( ANSI_VAR_FONT );

  hFontDlg = (HFONT)SelectObject( hdc, hFontDlg );


  /********************************************************************/
  /* display column position                                          */
  /********************************************************************/
  sprintf( chBuffer, "%s %2.2d", pTBDoc->chColStatusText,
           (pTBDoc->lCursorCol + pTBDoc->lSideScroll + 1) );

  FillStatusField( hdc, chBuffer, &ulStart, ulCY, CLR_BLACK );


  sprintf( chBuffer, "%s %3.3ld", pTBDoc->chLineStatusText,
           (ULONG)( pTBDoc->ulVScroll + pTBDoc->lCursorRow ) );
  FillStatusField( hdc, chBuffer, &ulStart, ulCY, CLR_BLACK );

  /********************************************************************/
  /* fill in Segment number                                           */
  /********************************************************************/

  sprintf( chBuffer, "%s %3.3d", pTBDoc->chSegStatusText,
           pTBDoc->TBCursor.ulSegNum );
  FillStatusField( hdc, chBuffer, &ulStart, ulCY, CLR_BLACK );


  /********************************************************************/
  /* fill in Insert/Overlay mode...                                   */
  /********************************************************************/
  sprintf( chBuffer, "%s", pTBDoc->chInsStatusText );
  if ( pTBDoc->hwndRichEdit )
    FillStatusField( hdc, chBuffer, &ulStart, ulCY,
                     (pTBDoc->EQFBFlags.inserting) ? CLR_BLACK : CLR_DARKGRAY );
  else
    FillStatusField( hdc, chBuffer, &ulStart, ulCY,
                     (pTBDoc->usCursorType == CURSOR_REPLACE) ? CLR_DARKGRAY : CLR_BLACK );

  /********************************************************************/
  /* fill remaining area in grey                                      */
  /********************************************************************/
  rectl.top    = 1;
  rectl.left   = 0;
  rectl.right  = ulStart;
  rectl.bottom    = ulCY - 1;
  if (!UtlIsHighContrast())
  {
	  ERASERECT( hdc, (const RECT*) &rectl, CLR_PALEGRAY );
	  rectl.top       = 0;
	  rectl.bottom    = ulCY - 0;
	  FrameRect( hdc, &rectl, (HBRUSH)GetStockObject(GRAY_BRUSH) );
  }
  else
  {
	DWORD dwRGB_WINDOW = GetSysColor(COLOR_WINDOW);
	ERASERECT( hdc, (const RECT*) &rectl, dwRGB_WINDOW );

    rectl.top       = 0;
    rectl.bottom    = ulCY - 0;
	FrameRect( hdc, &rectl, (HBRUSH)(COLOR_WINDOW+1) );
  }
  DeleteObject( SelectObject( hdc, hFontDlg ));

  EndPaint(hwnd, &ps);

} /* end of function TPFillStatusBar */

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     TPRULERWNDPROC
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
// Function flow:     position and paint ruler
//------------------------------------------------------------------------------

MRESULT APIENTRY
TPRULERWNDPROC
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
        TPFillRuler( hwnd );
        break;
      default:
        mResult = DefWindowProcW( hwnd, msg, mp1, mp2 );
        break;
  } /* switch */

  return mResult ;
} /* end of function TPRULERWNDPROC */


VOID
TPFillRuler
(
  HWND  hwnd                           // window handle
)
{
  PTBDOCUMENT pTBDoc = ACCESSWNDIDA( hwnd, PTBDOCUMENT );
  COLORREF    clrBkRGB;
  COLORREF    clrFGRGB;
  HFONT       hFont;
  PSZ         pText;
  ULONG       ulLen;
  BOOL        fBidi = (IS_RTL(pTBDoc) && pTBDoc->pBidiStruct);

  PAINTSTRUCT ps;
  HDC         hdc;

  hdc = BeginPaint(hwnd, &ps );

  clrFGRGB = COLORRGBTABLE[COL_BLACK];
  clrBkRGB = COLORRGBTABLE[COL_LIGHTGRAY];

  pTBDoc->lf.lfWeight = FW_NORMAL;
  pTBDoc->lf.lfItalic = 0;
  pTBDoc->lf.lfUnderline = 0;

   // do a special handling for Win2k systems .. DBCS fonts are not recognized and treated correctly
   // if SHIFTJIS_CHARSET is set ( nec for correct table-like display in dict.window)
   // add : For Win NT J, do NOT reset SHIFTJIS_CHARSET since this causes that only rectangles are displayed
   // instead of jap. characters!
  if (IsDBCS_CP(pTBDoc->ulOemCodePage) && (pTBDoc->bOperatingSystem != OP_WINDOWSNT))
  {
       UCHAR  lfCharSet = pTBDoc->lf.lfCharSet;
       pTBDoc->lf.lfCharSet &= ~SHIFTJIS_CHARSET;
       hFont = CreateFontIndirect( &pTBDoc->lf );
       pTBDoc->lf.lfCharSet = lfCharSet;
   }
   else
   {
      hFont = CreateFontIndirect( &pTBDoc->lf );
   }

  hFont = (HFONT)SelectObject( hdc, hFont );
  if (!UtlIsHighContrast())
  {
    SetTextColor(hdc, clrFGRGB);
    SetBkColor(hdc,clrBkRGB);
  }
  else
  {
    SetTextColor(hdc, UtlGetColorref(GetSysColor(COLOR_WINDOWTEXT)));
    SetBkColor(hdc,UtlGetColorref(GetSysColor(COLOR_WINDOW)));
  }


  /********************************************************************/
  /* use standard ruler up until 800 scroll units -- otherwise we use */
  /* only the default one                                             */
  /********************************************************************/
  if ( (pTBDoc->lSideScroll < 800) && !fBidi )
  {
    pText = pRuler + pTBDoc->lSideScroll;
  }
  else
  {
    PSZ pszSource = pRulerDef;
    PSZ pszTarget = pText = szRulerDefMod;

    // replace every 5th dash with a plus sign
    int iCounter = pTBDoc->lSideScroll % 5;
    while ( *pszSource )
    {
      if ( iCounter == 0 )
      {
        *pszTarget++ = '+';
        pszSource++;
        iCounter = 4;
      }
      else
      {
        *pszTarget++ = *pszSource++;
        iCounter--;
      } /* endif */
    } /*endwhile */
    *pszTarget = EOS;
  } /* endif */

  ulLen = strlen( pText );


  if ( !fBidi && IS_RTL_TGT( pTBDoc ))
  {
    SetTextAlign( hdc, TA_RIGHT | TA_RTLREADING );
    ExtTextOut (hdc, (pTBDoc->lScrnCols + 1)*pTBDoc->cx - 4, 0,
                ETO_OPAQUE | ETO_NUMERICSLATIN,
                NULL, pText, pTBDoc->lScrnCols, NULL );
  }
  else
  {
    TextOut (hdc,0, 0, pText, ulLen );
  } /* endif */

  if (!UtlIsHighContrast())
  {
    SetTextColor(hdc, clrBkRGB);
    SetBkColor(hdc,clrFGRGB);
  }
  else
  {
    SetTextColor(hdc, UtlGetColorref(GetSysColor(COLOR_HIGHLIGHT)));
    SetBkColor(hdc,UtlGetColorref(GetSysColor(COLOR_HIGHLIGHTTEXT)));
  }
  if ( fBidi )
  {
    TextOut (hdc,pTBDoc->pBidiStruct->ulBidiCursorCol*pTBDoc->cx, 0,
                 pText+pTBDoc->pBidiStruct->ulBidiCursorCol, 1 );
  }
  else
  {
    if ( IS_RTL_TGT( pTBDoc ) )
    {
      ExtTextOut (hdc, (pTBDoc->lScrnCols + 1 - pTBDoc->lCursorCol)*pTBDoc->cx - 4, 0,
                ETO_OPAQUE | ETO_NUMERICSLATIN,
                NULL, pText+pTBDoc->lCursorCol, 1, NULL );

    }
    else
    {
      if (pTBDoc->pArabicStruct)
      {
        LONG lCaretPos  = pTBDoc->pArabicStruct->plCaretPos[pTBDoc->lCursorRow*(pTBDoc->lScrnCols+1)+pTBDoc->lCursorCol];
        SHORT sPos = (SHORT) (lCaretPos/pTBDoc->cx);
        TextOut (hdc,lCaretPos, 0, pText+sPos, 1 );
      }
      else
      {
        TextOut (hdc,pTBDoc->lCursorCol*pTBDoc->cx, 0, pText+pTBDoc->lCursorCol, 1 );
      }
    } /* endif */
  } /* endif */
  DeleteObject( SelectObject( hdc, hFont ) );

  EndPaint(hwnd, &ps);


} /* end of function TPFillRuler */



MRESULT HandleEEAWMChar
(
  HWND        hwnd,
  WINMSG      msg,
  WPARAM      mp1,
  LPARAM      mp2,
  PTBDOCUMENT pDoc
)
{
  MRESULT mResult = FALSE;
  void (*function)( PTBDOCUMENT );
  USHORT  usFunction;
  USHORT  usAction;
  USHORT  usStatus;

  if ( EQFBMapKey( msg, mp1, mp2, &usFunction, &pDoc->ucState, TPRO_MAPKEY) )
  {
    PFUNCTIONTABLE pFuncTab = get_FuncTab();
    pDoc->usChar      = (USHORT)mp1;
    pDoc->usDBCS2Char = 0;
    usAction     = (pFuncTab + usFunction)->usAction;
    function     = (pFuncTab + usFunction)->function;
    usStatus = EQFBCurrentState( pDoc );
    if ((usStatus & usAction) == usAction )
    {
       (*function)( pDoc );           // execute the function
       /****************************************************/
       /* check if it is a function where pDoc will be     */
       /* freed and therefore is invalid furthermore       */
       /****************************************************/
       if (!((usFunction == QUIT_FUNC)||(usFunction == FILE_FUNC) )
            && pDoc->pSegTables )  // be sure noone else closed doc..
       {
         EQFBRefreshScreen( pDoc );  // refresh the screen
       } /* endif */
    }
    else
    {
       EQFBFuncNothing( pDoc ); // ignore keystroke
    } /* endif */

    mResult = MRFROMSHORT(TRUE);// indicate message is processed
  }
  else
  {
    mResult = DefWindowProcW( hwnd, msg, mp1, mp2 );
  } /* endif */
  return mResult;
}



#define MAX_NUM_SPELLAID    7
#define MAX_TEMPSIZE        256

static
VOID EQFBSpellPullDown
(
  HWND hwnd,
  POINT point,
  PTBDOCUMENT pDoc,
  USHORT usId,
  PSZ_W  pData,
  USHORT usStart,
  USHORT usStop
)
{
  CHAR_W     chTemp[MAX_TEMPSIZE];
  CHAR       chAid[128];
  USHORT     usLength = MAX_TEMPSIZE - 1 ;
  PSZ_W      pAidStart[MAX_NUM_SPELLAID+1];
  PSZ_W      pAid;
  USHORT     usRc = 0;
  HMENU      hMenu;
  HMENU      hMenuTrackPopup;
  USHORT     usPos = 0;
  USHORT     usAABId = IDM_SPELLAID_1;
  PTBSEGMENT pSeg;
  PSZ_W      pTemp = NULL;
  PSZ_W      pTemp1 = NULL;
  CHAR_W     c;
  USHORT     ulSegNum;
  PSPELLDATA pSpellData;
  HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);

  /* Get the menu for the popup from the resource file. */
  hMenu = LoadMenu( hResMod, MAKEINTRESOURCE( ID_POPUP_MENU ) );
  if ( !hMenu )
      return;

  hMenuTrackPopup = GetSubMenu( hMenu, usId );

  chTemp[0] = EOS;
  memset( pAidStart, 0, sizeof(pAidStart));
  pTemp = pData + usStart;
  pTemp1= pTemp + usStop - usStart + 1;
  c    = *pTemp1;
  *pTemp1   = EOS;
  usRc = EQFPROOFAIDW(pTemp, chTemp, &usLength);
  *pTemp1   = c;
  if (!usRc)
  {
     // as long as data available and not more than MAX_NUM_SPELLAID items --
     // add it to popup
     pAid = chTemp;
     if ( *pAid )
     {
       while (*pAid && (usPos < MAX_NUM_SPELLAID))
       {
         Unicode2ASCII( pAid, chAid, pDoc->ulOemCodePage );

          OEMTOANSI(chAid);
          InsertMenu( hMenuTrackPopup, usPos, MF_BYPOSITION | MF_STRING,
                      usAABId,
                      (PSZ)chAid );
          pAidStart[usPos] = pAid;
          usPos++;
          usAABId++;
          pAid += UTF16strlenCHAR(pAid) + 1;
       } /* endwhile */
     }
     else
     {
       CHAR chDesc[40];
       WinLoadString( hab, hResMod, IDS_SPELL_NOAID,
                      sizeof(chDesc) - 1, chDesc );
       InsertMenu( hMenuTrackPopup, usPos, MF_BYPOSITION | MF_STRING | MF_GRAYED,
                   0,
                   chDesc );
     } /* endif */
  } /* endif */
  /* Convert the mouse point to screen coordinates since that is what
   * TrackPopup expects.
   */
  ClientToScreen( hwnd, (LPPOINT)&point );

  /* Draw and track the "floating" popup  -- adjust it half of the height */
  usId = (USHORT)TrackPopupMenu( hMenuTrackPopup, TPM_RETURNCMD, point.x, point.y+pDoc->cy/2, 0,
                         (HWND)UtlQueryULong( QL_TWBFRAME ), NULL );

  /* Destroy the menu since were are done with it. */
  DestroyMenu( hMenu );

  pSeg = EQFBGetSegW( pDoc, pDoc->TBCursor.ulSegNum );
  if ( (usId>= IDM_SPELLAID_1) && (usId<= IDM_SPELLAID_1+MAX_NUM_SPELLAID) )
  {
    /******************************************************************/
    /* replace string                                                 */
    /******************************************************************/
    pTemp = pAidStart[ usId - IDM_SPELLAID_1 ];
    if ( pDoc->TBCursor.ulSegNum == pDoc->ulWorkSeg )
    {
      SHORT sDiff = (SHORT)((usStop - usStart + 1) - UTF16strlenCHAR( pTemp ));
      memmove( pDoc->pEQFBWorkSegmentW+ usStart,
               pDoc->pEQFBWorkSegmentW+ usStart + sDiff,
               (UTF16strlenCHAR( pDoc->pEQFBWorkSegmentW+usStart+sDiff) + 1)*sizeof(CHAR_W) );
      memcpy( pDoc->pEQFBWorkSegmentW + usStart, pTemp, UTF16strlenBYTE( pTemp ));
      EQFBUpdateChangedSeg(pDoc);
      pSeg->SegFlags.Spellchecked = FALSE;
      if (pDoc->fAutoSpellCheck )
      {
        EQFBWorkThreadTask ( pDoc, THREAD_SPELLSEGMENT );
      } /* endif */
      WinPostMsg( pDoc->hwndClient, WM_PAINT, 0, NULL );
    }
    else
    {
      WinAlarm( HWND_DESKTOP, WA_WARNING );
    } /* endif */
  }
  else
  {
    /******************************************************************/
    /* fulfill function request                                       */
    /******************************************************************/
    switch ( usId )
    {
      case IDM_IGNOREALL:       // temp add word in FindData.chFind
        pSpellData = (PSPELLDATA) pDoc->pvSpellData;
        pTemp1= pTemp + usStop - usStart + 1;
        c    = *pTemp1;
        *pTemp1   = EOS;
        UTF16strcpy (&(pSpellData->FindData.chFind[0]), pTemp );
        *pTemp1   = c;
//        EQFBWorkThreadTask ( pDoc, THREAD_TEMPADD);
        EQFBTempAdd(pDoc);

        if (pDoc->fAutoSpellCheck )
        {
          ulSegNum = 1;
          pSeg = EQFBGetSegW(pDoc, ulSegNum);
          while (pSeg && ulSegNum < pDoc->ulMaxSeg )
          {
            pSeg->SegFlags.Spellchecked = FALSE;
            ulSegNum++;
            pSeg = EQFBGetSegW(pDoc, ulSegNum);
          } /* endwhile */
          /************************************************************/
          /* reset of running FileSpellCheck nec !!                   */
          /************************************************************/
          pSpellData->FindData.fChange = TRUE;

        } /* endif */
        break;
      case IDM_ADDENDA:
        pTemp1= pTemp + usStop - usStart + 1;
        c    = *pTemp1;
        *pTemp1   = EOS;
        EQFPROOFADDW(pTemp);
        *pTemp1   = c;

        if (pDoc->fAutoSpellCheck )
        {
          ulSegNum = 1;
          pSeg = EQFBGetSegW(pDoc, ulSegNum);
          while (pSeg && ulSegNum < pDoc->ulMaxSeg )
          {
            pSeg->SegFlags.Spellchecked = FALSE;
            ulSegNum++;
            pSeg = EQFBGetSegW(pDoc, ulSegNum);
          } /* endwhile */
          EQFBWorkThreadTask ( pDoc, THREAD_SPELLSCRN );
//          DosSleep( 100L );
          /************************************************************/
          /* reset of running FileSpellCheck nec !!                   */
          /************************************************************/
          pSpellData = (PSPELLDATA) pDoc->pvSpellData;
          pSpellData->FindData.fChange = TRUE;
          pDoc->Redraw |= REDRAW_ALL;
        } /* endif */
        break;
      case IDM_NEXTMISSPELL:
        EQFBFuncNextMisspelled(pDoc);
        WinPostMsg( pDoc->hwndClient, WM_PAINT, 0, NULL );
        break;
      default:
        break;
    } /* endswitch */
  } /* endif */

  return;
}

BOOL
EQFBMouseOnActSeg
(
  PTBDOCUMENT pDoc,                    // pointer to document
  LONG        x,
  LONG        y
)
{
  LONG     lOldRow = pDoc->lCursorRow; // remember old cursor row/col
  LONG     lOldCol = pDoc->lCursorCol;
  BOOL     fOK = FALSE;                // success indicator

  assert( (pDoc->cy > 0) );
  assert( (pDoc->cx > 0) );
  pDoc->mouseRow = ( (y - pDoc->ulRulerSize - pDoc->cy/2 ) / pDoc->cy );
  if (pDoc->mouseRow <= 0)
  {
    pDoc->mouseRow = 0;
  } /* endif */
  if ( IS_RTL(pDoc) && !pDoc->pUserSettings->UserOptFlags.bBidiLogicDisplay )
  {
    PBIDISTRUCT   pBidiStruct = pDoc->pBidiStruct;
    PUINT         pusOrder    = pBidiStruct->BidiLine[pDoc->lCursorRow].pusOrder;
    USHORT        usPos       = 0;
    pDoc->lCursorRow = min( pDoc->mouseRow, pDoc->lScrnRows-1 ) ;
    /******************************************************************/
    /* fill in cursor row                                             */
    /******************************************************************/
    pDoc->mouseCol = ( x / pDoc->cx );
    if ( pusOrder )
    {
      ULONG  ulOffset = pBidiStruct->BidiLine[pDoc->lCursorRow].usOffset + pDoc->mouseCol - 1;
      usPos    = 0;
      while ( usPos < pDoc->lScrnCols + ulOffset )
      {
        if ( pusOrder[ usPos ] == ulOffset)
        {
          pDoc->mouseCol = usPos;
          break;
        }
        else
        {
          usPos++;
        } /* endif */
      } /* endwhile */
    } /* endif */
  }
  else
  {
    pDoc->mouseCol = ( x / pDoc->cx );
  }

  /********************************************************************/
  /* set to new position                                              */
  /********************************************************************/
  pDoc->lCursorRow = min( pDoc->mouseRow, pDoc->lScrnRows-1 ) ;
  pDoc->lCursorCol = pDoc->mouseCol;  // get new column position

  if ( (pDoc->lCursorRow != lOldRow) || ( pDoc->lCursorCol != lOldCol )  )
  {
    EQFBCurSegFromCursor( pDoc );       // get TBCursor
    if ( pDoc->TBCursor.ulSegNum == 0 ) // stay at old position
    {
       pDoc->lCursorRow = lOldRow;
       pDoc->lCursorCol = lOldCol;
       EQFBCurSegFromCursor( pDoc );     // get TBCursor
       fOK = FALSE;                      // do not capture ...
    }
    else
    {
      if ( pDoc->TBCursor.ulSegNum == pDoc->ulWorkSeg )
      {
        fOK = TRUE;
      }
      else if ( pDoc->EQFBFlags.PostEdit  )
      {
        fOK = (pDoc->pTBSeg->qStatus == QF_XLATED);
      }
      else
      {
        fOK = FALSE;
      } /* endif */
    } /* endif */
  }
  else if ( pDoc->TBCursor.ulSegNum == pDoc->ulWorkSeg )
  {
    fOK = TRUE;
  }
  else if ( pDoc->EQFBFlags.PostEdit  )
  {
    fOK = (pDoc->pTBSeg->qStatus == QF_XLATED);
  }
  else
  {
    fOK = FALSE;
  } /* endif */
  return fOK;
}

VOID
EQFBMouseToCursor
(
  PTBDOCUMENT pDoc,                    // pointer to document
  LONG        x,
  LONG        y,
  TBROWOFFSET *pCursor
)
{
  LONG    lOldRow = pDoc->lCursorRow; // remember old cursor row/col
  LONG    lOldCol = pDoc->lCursorCol;

  assert( (pDoc->cy > 0) );
  assert( (pDoc->cx > 0) );
  pDoc->mouseRow = ( (y - pDoc->ulRulerSize - pDoc->cy/2 ) / pDoc->cy );
  if (pDoc->mouseRow <= 0)
  {
    pDoc->mouseRow = 0;
  } /* endif */
  if ( IS_RTL(pDoc) && !pDoc->pUserSettings->UserOptFlags.bBidiLogicDisplay )
  {
    PBIDISTRUCT   pBidiStruct = pDoc->pBidiStruct;
    PUINT         pusOrder    = pBidiStruct->BidiLine[pDoc->lCursorRow].pusOrder;
    USHORT        usPos       = 0;
    pDoc->lCursorRow = min( pDoc->mouseRow, pDoc->lScrnRows-1 ) ;
    /******************************************************************/
    /* fill in cursor row                                             */
    /******************************************************************/
    pDoc->mouseCol = ( x / pDoc->cx );
    if ( pusOrder )
    {
      ULONG  ulOffset = pBidiStruct->BidiLine[pDoc->lCursorRow].usOffset + pDoc->mouseCol - 1;
      usPos    = 0;
      while ( usPos < pDoc->lScrnCols + ulOffset )
      {
        if ( pusOrder[ usPos ] == ulOffset)
        {
          pDoc->mouseCol = usPos;
          break;
        }
        else
        {
          usPos++;
        } /* endif */
      } /* endwhile */
    } /* endif */
  }
  else
  {
    pDoc->mouseCol = ( x / pDoc->cx );
  }

  /********************************************************************/
  /* set to new position                                              */
  /********************************************************************/
  pDoc->lCursorRow = min( pDoc->mouseRow, pDoc->lScrnRows-1 ) ;
  pDoc->lCursorCol = pDoc->mouseCol;  // get new column position

  if ( (pDoc->lCursorRow != lOldRow) || ( pDoc->lCursorCol != lOldCol )  )
  {
    EQFBCurSegFromCursor( pDoc );                // get TBCursor
    memcpy( pCursor, &pDoc->TBCursor, sizeof( TBROWOFFSET ));

    if ( pDoc->TBCursor.ulSegNum == 0 ) // stay at old position
    {
       pDoc->lCursorRow = lOldRow;
       pDoc->lCursorCol = lOldCol;
       EQFBCurSegFromCursor( pDoc );     // get TBCursor
    } /* endif */
  }
  else
  {
    memcpy( pCursor, &pDoc->TBCursor, sizeof( TBROWOFFSET ));
  } /* endif */
  return;
}

__declspec(dllexport)
VOID EQFBOnDrop
(
  PDOCUMENT_IDA pIda,
  LONG          lxPos,
  LONG          lyPos,
  PSZ_W         pData,
  BOOL          fMove
)
{
  PTBDOCUMENT pDoc = ACCESSWNDIDA( pIda->pstEQFGen->hwndEditorTgt, PTBDOCUMENT);
  LONG lRow = ( (lyPos - pDoc->ulRulerSize - pDoc->cy/2 ) / pDoc->cy );
  LONG lCol = (lxPos / pDoc->cx );
  EQFBBLOCK stEQFBBlock;
  PEQFBBLOCK  pstBlock = (PEQFBBLOCK)pDoc->pBlockMark;                 // pointer to block struct
  BOOL        fDrop = FALSE;       // TRUE if drop allowed
  BOOL        fOK = TRUE;

  if ( pDoc )
  {
    memset( &stEQFBBlock, 0, sizeof( stEQFBBlock ));
    if (lRow <= 0)
    {
      lRow = 0;
    } /* endif */
    if ( lCol <= 0 )
    {
      lCol = 0;
    } /* endif */

    /********************************************************************/
    /* set to new position                                              */
    /********************************************************************/
    pDoc->lCursorRow = min( lRow, pDoc->lScrnRows-1 ) ;
    pDoc->lCursorCol = lCol;  // get new column position
    /******************************************************************/
    /* Drop only allowed if Cursor is not inside of marked area       */
    /******************************************************************/
    EQFBCurSegFromCursor(pDoc);
    if ((pDoc->TBCursor.ulSegNum < pstBlock->ulSegNum )
        || ((pDoc->TBCursor.ulSegNum == pstBlock->ulSegNum )
            && (pDoc->TBCursor.usSegOffset < pstBlock->usStart ) )  )
    {
      fDrop = TRUE;
    }
    else if ( (pDoc->TBCursor.ulSegNum > pstBlock->ulEndSegNum)
              || ((pDoc->TBCursor.ulSegNum == pstBlock->ulEndSegNum)
                  && ( pDoc->TBCursor.usSegOffset > pstBlock->usEnd )) )
    {
      fDrop = TRUE;
    } /* endif */

    /******************************************************************/
    /* in case of copy do a reset of pending blockmark                */
    /******************************************************************/
    if (!fDrop )
    {
      EQFBFuncResetMarkInSeg( pDoc );
    }
    else
    {
      EQFBWorkSegCheck(pDoc);        // allow input in PostEdit
      if ( !fMove )
      {
        EQFBFuncResetMarkInSeg( pDoc );
      }
      else
      {
          // redo blockmark in current segment at character input
          if ( pstBlock->pDoc == pDoc &&
               (pstBlock->ulSegNum == pDoc->ulWorkSeg) )
          {
            memcpy( &stEQFBBlock, pstBlock, sizeof( EQFBBLOCK ));
            pstBlock->pDoc = NULL;                           // reset block mark
          } /* endif */
      } /* endif */
      fOK = EQFBDoCopyData( pDoc, pData );
      pDoc->Redraw |= REDRAW_ALL;
      /******************************************************************/
      /* get rid of any previously marked area                          */
      /******************************************************************/
      if (!fOK)
      {
		  EQFBFuncResetMarkInSeg( pDoc );
      }
      else
      {
		    if ( stEQFBBlock.ulSegNum )
		    {
			    USHORT usLen = (USHORT)UTF16strlenCHAR( pData );
			    if ( pDoc->TBCursor.usSegOffset < stEQFBBlock.usStart )
			    {
			      EQFBWorkLeft( pDoc, (USHORT)(stEQFBBlock.usStart+ usLen),
							    (USHORT)( stEQFBBlock.usEnd - stEQFBBlock.usStart+1) );
			    }
			    else
			    {
			      EQFBWorkLeft( pDoc, stEQFBBlock.usStart,
							    (USHORT)( stEQFBBlock.usEnd - stEQFBBlock.usStart+1) );

			      pDoc->TBCursor.usSegOffset = pDoc->TBCursor.usSegOffset - usLen;
			    } /* endif */

			    pDoc->EQFBFlags.workchng = TRUE;                        // something changed in workseg
			    if (pDoc->pTBSeg )
			    {
			      EQFBCompSeg( pDoc->pTBSeg );                          //update length of segment
			      pDoc->pTBSeg->SegFlags.Typed = TRUE;                  // something modified
			    } /* endif */
			    EQFBScrnLinesFromSeg ( pDoc, 0, pDoc->lScrnRows, (pDoc->TBRowOffset+1) );
			    EQFBPhysCursorFromSeg(pDoc);
			    pDoc->ActSegLog.usNumTyped++;
		    } /* endif */
      } /* endif */
    } /* endif */

    pDoc->Redraw |= REDRAW_ALL;
    EQFBRefreshScreen( pDoc );

  } /* endif */
  return;
}

static
MRESULT EQFBButton1Down
(
        HWND hwnd,                           // window handle
        WINMSG msg,                          // message
        WPARAM mp1,                          // message parameter
        LPARAM mp2,                          // message parameter
    PTBDOCUMENT pDoc                     // pointer to document struct..
)
{
    MRESULT   mResult = TRUE;
    BOOL fVKShift = FALSE;


	fVKShift = GetKeyState(VK_SHIFT) & 0x8000;
	if ( fVKShift )
	{
	  msg = WM_MOUSEMOVE;            // simulate mouse move
	  /****************************************************/
	  /* shift key down ... -> start or extend marking    */
	  /****************************************************/
	  if ( ! ((PEQFBBLOCK)pDoc->pBlockMark)->pDoc )
	  {
			EQFBFuncMarkBlock( pDoc );     // Mark current block
	  } /* endif */
	}
	else
	{
	  /****************************************************/
	  /* do we have to deal with Drag/Drop                */
	  /****************************************************/
	  if ( pDoc->pBlockMark && pDoc->pstEQFGen &&
			   ((PEQFBBLOCK)pDoc->pBlockMark)->pDoc == pDoc )
	  {
			PEQFBBLOCK  pstBlock = (PEQFBBLOCK)pDoc->pBlockMark;
			TBROWOFFSET TBCursor;
			ULONG       ulStart, ulEnd, ulCur;

			EQFBMouseToCursor(pDoc, LOWORD(mp2), HIWORD(mp2), &TBCursor );

			ulStart = MAKELONG( pstBlock->usStart, pstBlock->ulSegNum );
			ulEnd   = MAKELONG( pstBlock->usEnd,   pstBlock->ulEndSegNum );
			ulCur   = MAKELONG( TBCursor.usSegOffset, TBCursor.ulSegNum );

			if ( (ulStart <= ulCur+1) && (ulCur <= ulEnd+1) )
			{
			  /************************************************/
			  /* user wants to start drag/drop                */
			  /************************************************/
			  if ( WinSendMsg( ((PSTEQFGEN)pDoc->pstEQFGen)->hwndTWBS,
								 WM_EQF_DRAGDROP, NULL, pDoc ) )
			  {
				  //    mResult = FALSE;
				  return FALSE;
			  }
			  else
			  {
					/**********************************************/
					/* it is the old i/f w/o drag/drop - so go    */
					/* ahead as normal...                         */
					/**********************************************/
			  } /* endif */
			} /* endif */
	  } /* endif */

	  if ( pDoc->pBlockMark && ((PEQFBBLOCK)pDoc->pBlockMark)->pDoc )
	  {
			memset( pDoc->pBlockMark, 0, sizeof( EQFBBLOCK ));
			pDoc->Redraw |= REDRAW_ALL;
			EQFBRefreshScreen( pDoc );
	  } /* endif */
	} /* endif */

  if ( EQFBMousePosition( pDoc, msg, mp1, mp2 ) )
  {
        SETCAPTURE( hwnd );            // capture the mouse
  } /* endif */
  if ( fVKShift )
  {
        EQFBRefreshScreen( pDoc );
  } /* endif */

return mResult;
}

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     EQFBGetFindData
//------------------------------------------------------------------------------
// Function call:     EQFBGetFindData(pFindData)
//------------------------------------------------------------------------------
// Description:       get a pointer to the finddata struct
//------------------------------------------------------------------------------
// Parameters:        PFINDDATA     pFindData
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:     set the pointer to the finddata struct
//------------------------------------------------------------------------------

PFINDDATA EQFBGetFindData()
{
  return(pFindData);
} /* end of function EQFBGetFindData */

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     EQFBDocSetCodePage
//------------------------------------------------------------------------------
// Function call:     EQFBDocSetCodePage (pDoc, pIda )
//------------------------------------------------------------------------------
// Description:       set ANSI and OEM CodePage in TBDOC struct
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT pNewdoc
//                    PDOCUMENT_IDA pIda
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:
//                    Switch ( doctype)
//                     case STARGET: set cp of targetlang if avail.
//                     case SSOURCE:
//                          VISSRC:
//                          VISTGT:
//                          TRNOTE:
//                          EEA:    set cp of srclang if avail.
//                     default:
//                     (case OTHER,SERVPROP, SERVDICT, SERVSOURCE,
//                           SERVPROP )
//                                  set cp of system pref.lang
//------------------------------------------------------------------------------

VOID EQFBDocSetCodePage
(
  PTBDOCUMENT pNewdoc,
  PDOCUMENT_IDA pIda
)
{
  PSZ           pszTemp = NULL;            // string for language property

  // if avail. set CP according to document sourc/tgt language
  if ( pIda )
  {
    switch(pNewdoc->docType)
    {
      case STARGET_DOC:
        pszTemp = pIda->szDocTargetLang ;
        break;
      case SSOURCE_DOC:
      case VISSRC_DOC:
      case VISTGT_DOC:
      case TRNOTE_DOC:
      case EEA_DOC:
        pszTemp = pIda->szDocSourceLang ;
        break;
      default:
        // default set already above
        // OTHER_DOC, SERVPROP_DOC, SERVDICT_DOC, SERVSOURCE_DOC
        pszTemp = NULL;
      break;
    }
  }
  pNewdoc->ulOemCodePage = GetLangCodePage(OEM_CP, pszTemp);
  pNewdoc->ulAnsiCodePage = GetLangCodePage(ANSI_CP, pszTemp);
  return;
} /* end of function EQFBDocSetCodePage  */

// initialize spell check data area
void ResetSpellData( PSPELLDATA pSpellData, PSTEQFGEN pstEQFGen )
{
  if ( pSpellData && pSpellData->pIgnoreData &&  !pstEQFGen->fLoadedBySpellcheck ) 
  {
    pSpellData->pIgnoreNextFree = pSpellData->pIgnoreData;
    memset( pSpellData->pIgnoreData, 0, pSpellData->usIgnoreLen * sizeof(CHAR_W));
  } /* endif */
}

// Allocate local spellcheck data area, allocate global spellcheck data area when needed
PSPELLDATA AllocSpellData( PTBDOCUMENT pDoc, PSTEQFGEN pstEQFGen, BOOL fResetIgnoreData  )
{
  PSPELLDATA pDocSpellData = (PSPELLDATA)pDoc->pvSpellData;
  PSPELLDATA pSpellData = NULL; // new spellcheck data area, function return value

  // use any given spell data area as global data area or allocate a new one
  if ( pDocSpellData != NULL )
  {
    pGlobalSpellData = pDocSpellData;
  }
  else
  {
    UtlAlloc( (PVOID *) &pGlobalSpellData, 0L, (LONG) sizeof(SPELLDATA), ERROR_STORAGE );
    if ( pGlobalSpellData)
    {                     
      UtlAlloc( (PVOID *)&(pGlobalSpellData->pIgnoreData), 0L, (LONG) MAX_SEGMENT_SIZE * sizeof(CHAR_W), ERROR_STORAGE );
      if ( pGlobalSpellData->pIgnoreData )
      {
        pGlobalSpellData->usIgnoreLen = MAX_SEGMENT_SIZE;
        pGlobalSpellData->pIgnoreNextFree = pGlobalSpellData->pIgnoreData;
      } /* endif */
      UtlAlloc( (PVOID *) &(pGlobalSpellData->pSpellWorkSegW), 0L, (LONG) (MAX_SEGMENT_SIZE + 1) * sizeof(CHAR_W), ERROR_STORAGE);
      UtlAlloc( (PVOID *) &(pGlobalSpellData->pSpellSeg), 0L, (LONG) sizeof(TBSEGMENT), ERROR_STORAGE);
      pDoc->pvSpellData = (PVOID)pGlobalSpellData;
    } /* endif */
  } /* endif */

  if ( fResetIgnoreData ) ResetSpellData( pGlobalSpellData, pstEQFGen );

  UtlAlloc( (PVOID *) &pSpellData, 0L, (LONG) sizeof(SPELLDATA), ERROR_STORAGE );
  if ( pSpellData )       
  {                                       
    // we use the ignore data list from the global spell data area
    UtlAlloc( (PVOID *) &(pSpellData->pSpellWorkSegW), 0L, (LONG) (MAX_SEGMENT_SIZE + 1) * sizeof(CHAR_W), ERROR_STORAGE);
    UtlAlloc( (PVOID *) &(pSpellData->pSpellSeg), 0L, (LONG) sizeof(TBSEGMENT), ERROR_STORAGE);
  } /* endif */
  return( pSpellData );
}
