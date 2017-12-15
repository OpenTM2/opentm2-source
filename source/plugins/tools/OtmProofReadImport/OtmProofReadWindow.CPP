/*! \file
	Description: Proof Read Import: User interface related code

	Copyright Notice:

	Copyright (C) 1990-2017, International Business Machines
	Corporation and others. All rights reserved
*/

#define INCL_EQF_FOLDER           // folder list and document list functions
#define INCL_EQF_TM               // Translation Memory functions
#define INCL_EQF_TP               // public translation processor functions
#define INCL_EQF_EDITORAPI        // editor API
#define INCL_EQF_TAGTABLE         // tagtable defines and functions
#define INCL_EQF_ANALYSIS         // analysis functions
#define INCL_EQF_MORPH            // morphologic functions
#include <eqf.h>                  // General Translation Manager include file
#include "eqfdde.h"               // batch mode definitions
#include "eqffol00.h"             // our .h stuff
#include "eqffol.id"              // our ID file
#include "eqftmi.h"               // TM internal definitions
#include "EQFHLOG.H"            // Translation Processor priv. include file
#include "eqflp.h"
#include <windowsx.h>
#include <commctrl.h>

#include <process.h>              /* _beginthread, _endthread */
  #include <direct.h>

#include "eqfutmdi.h"           // MDI utilities
#include "richedit.h"           // MDI utilities

#include "OtmProposal.h"
#include "OtmProofReadEntry.h"
#include "core\memory\MemoryFactory.h"
#include "cxmlwriter.h"

#include "OtmProofReadEntry.h"
#include "OtmProofReadList.h"

#include "OtmProofReadImportPlugin.h"
#include "OtmProofReadWindow.h"
#include "OtmProofReadImportPlugin.id"

// color text names (in same order as base entry defines plus subtext defines)
PSZ pszPRITextTypes[] = {"Modified entry: normal text", "Modified entry: Changed text", "Modified entry: Inserted text", 
                         "Modified entry: Normal text when entry is selected", "Modified entry: Changed text when entry is selected", "Modified entry: Inserted text when entry is selected",  
                         "Modified entry: Normal text when entry has the input focus", "Modified entry: Changed text when entry has the input focus", "Modified entry: Inserted text when entry has the input focus", 
                         "Processed entry: Normal text", "Processed entry: Changed text", "Processed entry: Inserted text", 
                         "Processed entry: Normal text when entry is selected", "Processed entry: Changed text when entry is selected", "Processed entry: Inserted text when entry is selected",  
                         "Processed entry: Normal text when entry has the input focus", "Processed entry: Changed text when entry has the input focus", "Processed entry: Inserted text when entry has the input focus", 
                         "Unchanged entry: Normal text", "Unchanged entry: Changed text", "Unchanged entry: Inserted text", 
                         "Unchanged entry: Normal text when entry is selected", "Unchanged entry: Changed text when entry is selected", "Unchanged entry: Inserted text when entry is selected",  
                         "Unchanged entry: Normal text when entry has the input focus", "Unchanged entry: Changed text when entry has the input focus", "Unchanged entry: Inserted text when entry has the input focus", 
                         NULL };

// default foreground and background colors (in same order as texttype names)
//                                   Normal               Changed           Changed-to        
COLORREF aclrPRIDefaultForeground[50] = { RGB(0,0,0),       RGB(0,0,0),       RGB(0,0,0),            // Modified entry: normal element
                                     RGB(0,0,0),          RGB(0,0,0),       RGB(0,0,0),            // Modified entry: selected element 
                                     RGB(0,0,0),          RGB(0,0,0),       RGB(0,0,0),            // Modified entry: focus element 
                                     RGB(0,0,0),          RGB(0,0,0),       RGB(0,0,0),            // Processed entry: normal element
                                     RGB(0,0,0),          RGB(0,0,0),       RGB(0,0,0),            // Processed entry: selected element 
                                     RGB(0,0,0),          RGB(0,0,0),       RGB(0,0,0),            // Processed entry: focus element 
                                     RGB(0,0,0),          RGB(0,0,0),       RGB(0,0,0),            // Unchanged entry: normal element
                                     RGB(0,0,0),          RGB(0,0,0),       RGB(0,0,0),            // Unchanged entry: selected element 
                                     RGB(0,0,0),          RGB(0,0,0),       RGB(0,0,0),        };  // Unchanged entry: focus element 
COLORREF aclrPRIDefaultBackground[50] = { RGB(255,255,255), RGB(255,80,80),   RGB(100,255,100),    // Modified entry: normal element
                                     RGB(230,230,230),    RGB(255,80,80),   RGB(100,255,100),      // Modified entry: selected element 
                                     RGB(230,230,255),    RGB(255,80,80),   RGB(100,255,100),      // Modified entry: focus element 
                                     RGB(235,255,235),    RGB(235,80,80),   RGB(100,255,100),      // Processed entry: normal element
                                     RGB(230,230,230),    RGB(255,80,80),   RGB(100,255,100),      // Processed entry: selected element 
                                     RGB(230,230,255),    RGB(255,80,80),   RGB(100,255,100),      // Processed entry: focus element 
                                     RGB(245,245,245),    RGB(245,245,245), RGB(245,245,245),      // Unchanged entry: normal element
                                     RGB(225,235,225),    RGB(225,235,225), RGB(225,235,225),      // Unchanged entry: selected element 
                                     RGB(230,230,255),    RGB(255,80,80),   RGB(100,255,100),  };  // Unchanged entry: focus element 

// user drawn checkbox coordinates in pixel
#define CHECKBOX_XPOS 6
#define CHECKBOX_YPOS 6
#define CHECKBOX_XSIZE 12
#define CHECKBOX_YSIZE 12

// default width and height of dialog window
#define DEFAULT_WINDOW_HEIGHT 600
#define DEFAULT_WINDOW_WIDTH  800


#define PROOFREADLIST_FORMAT_FILTERS "ValidationList (*.VALXML)\0*.VALXML\0\0\0"

// global data for subclassing and listbox on-spot editing
HWND hEdit = NULL;             // handle of currently active edit control
WNDPROC ListboxOldProc;        // list view control original windows procedure
WNDPROC EditOldProc;           // edit control original windows procedure
int iEditListboxItem;             // index of item currently edited
HWND hwndDialog;             // handle of proof read import window
CHAR_W szEditText[2400];     // buffer for edited text
CHAR_W szOrgClipBoardText[2400]; // buffer for original clipboard text

static BOOL ProofReadGetLastUsedValues( PPROOFREADDATA pIda );
static BOOL ProofReadSaveLastUsedValues( PPROOFREADDATA pIda );
void ProofReadDrawItem( PPROOFREADDATA pIda, LPDRAWITEMSTRUCT lpDrawItem );
int ProofReadForceRefreshOfItemHeights( PPROOFREADDATA  pIda );
int ProofReadMeasureItem( PPROOFREADDATA  pIda, LPMEASUREITEMSTRUCT pMeasureItem );
int ProofReadGetTextHeight( PPROOFREADDATA  pIda, HWND hwndControl, HDC hdc, OtmProofReadEntry *pEntry, int iTextColumn );
void ProofReadDrawText( PPROOFREADDATA pIda, HDC hdc, PRECT prcClip, PPOINT pPt, const CHAR_W *pszString, int iLen, int iLineHeight, int iBaseColorIndex,  DRAWTYPE DrawType );
int ProofReadBuildFullColumnText( PPROOFREADDATA pIda, OtmProofReadEntry *pEntry, int iTextColumn );
int ProofReadDrawFrame( HDC hdc, LPRECT prc );
BOOL ProofReadResizeColumns( HWND hwnd, PPROOFREADDATA pIda );
BOOL ProofReadAdjustResultColumns( HWND hwnd, PPROOFREADDATA pIda );
BOOL ProofReadOpenDoc( PPROOFREADDATA pIda );
void ProofReadRefreshListbox( PPROOFREADDATA pIda );
BOOL ProofReadIsOnSpotEditingActive();
void ProofReadEndOnSpotEditing();
long _stdcall ProofReadListboxProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
long _stdcall ProofReadEditProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
BOOL ProofReadToggleCheckbox( PPROOFREADDATA pIda, HWND hwndListbox, int iItem );
void ProofReadSetProcessPB( PPROOFREADDATA pIda );
void ProofReadSelectAll( PPROOFREADDATA pIda, BOOL fSelect );
BOOL ProofReadSaveList( PPROOFREADDATA pIda );
BOOL ProofReadLoadList( PPROOFREADDATA pIda );
BOOL ProofReadFreeTagTable( PPROOFREADDATA pIda );
BOOL ProofReadLoadTagTable( PPROOFREADDATA pIda, const char *pszTagTable );
BOOL ProofReadFindDifferences( PPROOFREADDATA pIda, OtmProofReadEntry *pEntry );
BOOL ProofReadMakeDiffLists( PPROOFREADDATA pIda, int iStartIndex, int iEndIndex );
BOOL ProofReadFillDocumentList( PPROOFREADDATA pIda );
int ProofReadDrawItemColumn( PPROOFREADDATA pIda, HDC hdc, int iColumn, const CHAR_W *pszString, LPRECT prcClip, OtmProofReadEntry *pEntry, PPROOFREADCHANGE pChanges, int iFoundEntries, int TextColumn, int iBaseColorIndex );
BOOL ProofReadImport( PPROOFREADDATA pIda );
BOOL ProofReadLoadXMLFiles( PPROOFREADDATA pIda, std::vector<std::string> *pvXMLFiles, HWND hwndParent  );
BOOL ProofReadSetDefaults( PPROOFREADDATA pIda );
BOOL ProofReadEditCell( PPROOFREADDATA pIda, HWND hwndListbox, int iItem );
BOOL ProofReadIsLineSplitChar( PPROOFREADDATA pIda, CHAR_W c );

BOOL ProofReadWindow
(
  std::vector<std::string> *pvXMLFiles,  // vector with the fully qualified file names of the proof read results files in XML format
  HINSTANCE        hDLL                 // DLL instance handle
)
{
  BOOL        fOK = TRUE;              // internal O.K. flag
  PPROOFREADDATA pIda;                 // ptr to IDA of dialog
  HWND        hwndProofReadWindow;     // handle of global-find-and-change dialog

  // Allocate IDA of Proof rad import window
  fOK = UtlAlloc( (PVOID *)&pIda, 0L, (LONG)sizeof(PROOFREADDATA), ERROR_STORAGE );

  // Fill-in IDA fields
  if ( fOK )
  {
    pIda->pvProofReadXML = pvXMLFiles;
    pIda->pList = new( OtmProofReadList );
  } /* endif */

  // load proof read entries
  if ( fOK )
  {
    HWND hwndDesktop = (HWND)UtlQueryULong( QL_TWBFRAME );
    fOK = ProofReadLoadXMLFiles( pIda, pvXMLFiles, hwndDesktop );
  }

  // Start proof read import window
  if ( fOK )
  {
    hwndProofReadWindow = CreateMDIDialogParam( hDLL, MAKEINTRESOURCE(ID_PROOFREADIMPORT_DIALOG),
                                             (HWND)UtlQueryULong( QL_TWBCLIENT ),
                                             (FARPROC)ProofReadDlgProc,
                                             MP2FROMP(pIda), TRUE,
                                             (HPOINTER) UtlQueryULong(QL_DICTENTRYDISPICO)); 

  } /* endif */

  return( fOK );
} /* end of function ProofReadWindow */


MRESULT EXPENTRY ProofReadDlgProc( HWND hwnd, WINMSG msg, WPARAM mp1, LPARAM mp2 )
{
   MRESULT  mResult = MRFROMSHORT( FALSE );                // result value of procedure
   PPROOFREADDATA pIda;
   BOOL     fOK = TRUE;

   switch ( msg )
   {
      case WM_EQF_QUERYID: HANDLEQUERYID( ID_PROOFREADIMPORT_DIALOG, mp2 ); break;

      case WM_INITDLG:
        {
          pIda = (PPROOFREADDATA)PVOIDFROMMP2(mp2);
          ANCHORDLGIDA( hwnd, pIda );
          pIda->hwnd = hwnd;
          pIda->sBorderSize  = (SHORT) WinQuerySysValue (HWND_DESKTOP, SV_CXSIZEBORDER);
          pIda->hResultListBox = GetDlgItem( hwnd, ID_PROOFREAD_TABLE_LV );


          // get dialog position and size
          WinQueryWindowPos( hwnd, &pIda->swpDlg );

          // build unique dialog object name
          sprintf( pIda->szObjName, "ProofRead:%s", pIda->pList->getFolder( 0 ).c_str() );

          UtlRegisterModelessDlg( hwnd );

          // get last used values
          if ( fOK ) fOK = ProofReadGetLastUsedValues( pIda );

          // add imported file to the title bar
          //strcpy( pIda->szStatusLine, "Proofread Document Import: " );   
          //strcat( pIda->szStatusLine, pIda->szProofReadXML );
          //SetWindowText( hwnd, pIda->szStatusLine );

          // create status bar
          if ( fOK )
          {
            int statwidths[] = {300, 400, -1};

            pIda->hStatus = CreateWindowEx( 0, STATUSCLASSNAME, NULL, WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP, 0, 0, 0, 0, hwnd, (HMENU)ID_PROOFREAD_STATUSBAR, GetModuleHandle(NULL), NULL);

            SendMessage( pIda->hStatus, SB_SETPARTS, 3, (LPARAM)statwidths);

            SendMessage( pIda->hStatus, SB_SETTEXT, 0, (LPARAM)"0 entries of 0 selected");
            ShowWindow( pIda->hStatus, SW_SHOW);
          }

          // fill color data area with default values
          pIda->ColorData.hwndOwner = hwnd;
          pIda->ColorData.sID = 0;
          strcpy( pIda->ColorData.szTitle, "Set Validation Document Import Colors" );
          int i = 0;
          while( pszPRITextTypes[i] != NULL ) 
          {
            strcpy( pIda->ColorData.ColorSetting[i].szElement, pszPRITextTypes[i] );
            pIda->ColorData.ColorSetting[i].cForeground = aclrPRIDefaultForeground[i];
            pIda->ColorData.ColorSetting[i].cDefaultForeground = aclrPRIDefaultForeground[i];
            pIda->ColorData.ColorSetting[i].cBackground = aclrPRIDefaultBackground[i];
            pIda->ColorData.ColorSetting[i].cDefaultBackground = aclrPRIDefaultBackground[i];
            i++;
          } /* endwhile */

          // set any user defined colors
          if ( pIda->pLastUsed && ((pIda->pLastUsed->aclrBackground[0] != 0) || (pIda->pLastUsed->aclrForeground[0] != 0) ) )
          {
              int i = 0;
              while( pIda->ColorData.ColorSetting[i].szElement[0] != EOS )
              {
                pIda->ColorData.ColorSetting[i].cForeground = pIda->pLastUsed->aclrForeground[i];
                pIda->ColorData.ColorSetting[i].cBackground = pIda->pLastUsed->aclrBackground[i];
                i++;
              } /* endwhile */
          } /* endif */

          // preset font settings
          {
            // this code has been borrowed from function SetCtrlFont in file EQFOSWIN.C
            HFONT   hFontDlg;

            memset( &(pIda->lf), 0, sizeof(pIda->lf) );
            hFontDlg = (HFONT)SendMessage( hwnd, WM_GETFONT, 0, 0L );
            if ( hFontDlg != NULL )
            {
              if ( GetObject( hFontDlg, sizeof(pIda->lf), (LPSTR) &(pIda->lf) ))
              {
                pIda->lf.lfCharSet  = (UCHAR)GetCharSet();
                if (pIda->lf.lfHeight > 0 )
                {
                  pIda->lf.lfHeight -=  SHEIGHTINCTRL;
                }
                else
                {
                  pIda->lf.lfWeight = FW_NORMAL;
                } /* endif */
                pIda->lf.lfOutPrecision = OUT_TT_PRECIS;
                strcpy( pIda->lf.lfFaceName, "Arial" );
              }
            }
          }

          // set other last used values
          if ( pIda->pLastUsed )
          {
              pIda->fShowUnchanged = pIda->pLastUsed->fShowUnchanged;
              pIda->fShowModified = pIda->pLastUsed->fShowModified;
              pIda->fShowProcessed = pIda->pLastUsed->fShowProcessed;
              pIda->fProcessDoc = pIda->pLastUsed->fProcessDoc;
              pIda->fProcessMem = pIda->pLastUsed->fProcessMem;
              pIda->fShowDifferences = pIda->pLastUsed->fShowDifferences;

              if ( pIda->pLastUsed->lfFaceName[0] != 0 )
              {
                // get last used font settings
                memset( &(pIda->lf), 0, sizeof(pIda->lf) );
                pIda->lf.lfHeight = pIda->pLastUsed->lfHeight;
                pIda->lf.lfWidth = pIda->pLastUsed->lfWidth;
                pIda->lf.lfEscapement = pIda->pLastUsed->lfEscapement;
                pIda->lf.lfOrientation = pIda->pLastUsed->lfOrientation;
                pIda->lf.lfWeight = pIda->pLastUsed->lfWeight;
                pIda->lf.lfItalic = pIda->pLastUsed->lfItalic;
                pIda->lf.lfUnderline = pIda->pLastUsed->lfUnderline;
                pIda->lf.lfStrikeOut = pIda->pLastUsed->lfStrikeOut;
                pIda->lf.lfCharSet = pIda->pLastUsed->lfCharSet;
                pIda->lf.lfOutPrecision = pIda->pLastUsed->lfOutPrecision;
                pIda->lf.lfClipPrecision = pIda->pLastUsed->lfClipPrecision;
                pIda->lf.lfQuality = pIda->pLastUsed->lfQuality;
                pIda->lf.lfPitchAndFamily = pIda->pLastUsed->lfPitchAndFamily;
                strcpy( pIda->lf.lfFaceName, pIda->pLastUsed->lfFaceName );
              } /* endif */
          } /* endif */

          // create font to be used for the controls
          {
            pIda->hFontControl = CreateFontIndirect( &(pIda->lf) );
          }

          // get handles and sizes of dialog controls
          if ( fOK )
          {
            int i = 0;

            // get handles and sizes at bottom of dialog window
            pIda->hwndButton[0] = GetDlgItem( hwnd, ID_PROOFIMP_CLOSE_PB );
            pIda->hwndButton[1] = GetDlgItem( hwnd, ID_PROOFIMP_HELP_PB );
            for ( i = 0; i < NUM_OF_PRI_BUTTONS; i++ )
            {
              RECT rect;
              GetWindowRect( pIda->hwndButton[i], &rect );
              pIda->sButtonWidth[i] = (SHORT)(rect.right - rect.left);
              if ( i == 0 ) pIda->sButtonHeight = (SHORT)(rect.bottom - rect.top);
            } /* endfor */
          } /* endif */

          // use remembered size and position (if any available)
          if ( fOK )
          {
            if ( pIda->pLastUsed->swpSizePos.cx != 0 )
            {
              EQFSWP2SWP( pIda->pLastUsed->swpSizePos, pIda->swpDlg );
            } /* endif */
          }

          // set inital column widths
          ProofReadAdjustResultColumns( hwnd, pIda );

          // set inital state of checkboxes and push buttons
          SETCHECK( hwnd, ID_PROOFIMP_SHOWUNCHANGED_CHK, pIda->fShowUnchanged );
          SETCHECK( hwnd, ID_PROOFIMP_SHOWMODIFIED_CHK, pIda->fShowModified );
          SETCHECK( hwnd, ID_PROOFIMP_SHOWPROCESSED_CHK, pIda->fShowProcessed );
          SETCHECK( hwnd, ID_PROOFIMP_UPDTRANS_CHK, pIda->fProcessDoc );
          SETCHECK( hwnd, ID_PROOFIMP_UPDMEM_CHK, pIda->fProcessMem );
          SETCHECK( hwnd, ID_PROOFIMP_SHOWDIFF_CHK, pIda->fShowDifferences );

          // subclass list box control and set global variables
          hEdit = NULL;        
          ListboxOldProc	= (WNDPROC)SetWindowLong( pIda->hResultListBox, GWL_WNDPROC, (LONG)ProofReadListboxProc );
          hwndDialog = hwnd;


          // postpone sizing of dialog window
          if ( fOK ) PostMessage( hwnd, WM_EQF_PROCESSTASK, MP1FROMSHORT(INITDLG_TASK), 0L );

          // destroy window in case of errors
          if ( !fOK )
          {
            WinDestroyWindow( hwnd );
            return 0;
          } /* endif */
        }
        return 0;
        break;

    case WM_EQF_PROCESSTASK:
      {
        PPROOFREADDATA pIda = ACCESSDLGIDA( hwnd, PPROOFREADDATA );
        if ( pIda != NULL )
        {
          switch ( SHORT1FROMMP1(mp1) )
          {
            case INITDLG_TASK :
              {
                HWND hwndFrame = GetParent( hwnd );

                // use position and size of result area for minimum dialog width
                {
                  RECT rect;

                  GetWindowRect( hwnd, &rect );
                  pIda->iMinDialogWidth = rect.right - rect.left;
                  pIda->iMinDialogHeight = rect.bottom - rect.top;
                }

                // ensure that dialog is in visible screen area
                if ( fOK )
                {
                  // the following computation is based on old OS/2 windows coordiantes (0,0 = lower left corner)

                  int cxScreen = WinQuerySysValue( HWND_DESKTOP, SV_CXSCREEN );
                  int cyScreen = WinQuerySysValue( HWND_DESKTOP, SV_CYSCREEN );

                  if ( (pIda->swpDlg.cy + pIda->swpDlg.y) > cyScreen )
                  {
                    pIda->swpDlg.y = (SHORT)(cyScreen - pIda->swpDlg.cy);
                  }
                  if ( (pIda->swpDlg.cx + pIda->swpDlg.x) > cxScreen )
                  {
                    pIda->swpDlg.x = (SHORT)(cxScreen - pIda->swpDlg.cx);
                  }
                } /* endif */

                WinSetWindowPos( hwndFrame, HWND_TOP, pIda->swpDlg.x, pIda->swpDlg.y, pIda->swpDlg.cx, pIda->swpDlg.cy, EQF_SWP_SIZE | EQF_SWP_MOVE | EQF_SWP_SHOW | EQF_SWP_ACTIVATE );

                ProofReadMakeDiffLists( pIda, 0, pIda->pList->size() - 1 );
                ProofReadFillDocumentList( pIda );
                ProofReadRefreshListbox( pIda );
                ProofReadSetProcessPB( pIda );
              }
              break;

            case ENABLE_TASK:
              {
                BOOL fEnable = (BOOL)mp2;
                ENABLECTRL( hwnd, ID_PROOFIMP_UPDTRANS_CHK, fEnable);
                ENABLECTRL( hwnd, ID_PROOFIMP_UPDMEM_CHK, fEnable);
                ENABLECTRL( hwnd, ID_PROOFIMP_SHOWDIFF_CHK, fEnable);
                ENABLECTRL( hwnd, ID_PROOFIMP_DOCUMENTS_LB, fEnable);
                ENABLECTRL( hwnd, ID_PROOFIMP_SEGMENTS_GB, fEnable);
                ENABLECTRL( hwnd, ID_PROOFREAD_TABLE_LV, fEnable);
                ENABLECTRL( hwnd, ID_PROOFIMP_SELECTALL_PB, fEnable);
                ENABLECTRL( hwnd, ID_PROOFIMP_DESELECTALL_PB, fEnable);
                ENABLECTRL( hwnd, ID_PROOFIMP_REFRESH_PB, fEnable);
                ENABLECTRL( hwnd, ID_PROOFIMP_OPEN_PB, fEnable);
                ENABLECTRL( hwnd, ID_PROOFIMP_IMPORT_PB, fEnable);
                ENABLECTRL( hwnd, ID_PROOFIMP_LOAD_PB, fEnable);
                ENABLECTRL( hwnd, ID_PROOFIMP_SAVE_PB, fEnable);
                ENABLECTRL( hwnd, ID_PROOFIMP_CLOSE_PB, fEnable);
                if ( fEnable )
                {
                  ProofReadSetProcessPB( pIda );
                }
                else
                {
                  ENABLECTRL( hwnd, ID_PROOFIMP_AUTOUPDATE_PB, fEnable);
                } /* endif */
                ENABLECTRL( hwnd, ID_PROOFIMP_SHOWUNCHANGED_CHK, fEnable);
                ENABLECTRL( hwnd, ID_PROOFIMP_SHOWMODIFIED_CHK, fEnable);
                ENABLECTRL( hwnd, ID_PROOFIMP_SHOWPROCESSED_CHK, fEnable);
              }
              break;

            case REFRESH_TASK:
              ProofReadRefreshListbox( pIda );
              break;

            case REFRESHITEM_TASK:
              {
                RECT rectItem;
                int iItem = (int)mp2;
                if ( iItem != -1 )
                {
                  SendMessage( pIda->hResultListBox, LB_GETITEMRECT, (WPARAM)iItem, (LPARAM)&rectItem );
                  InvalidateRect( pIda->hResultListBox, &rectItem, TRUE );
                } /* endif */
              }
              break;

            case SETFOCUS_TASK:
              SetFocus( GetDlgItem( hwnd, (int)mp2 ) );
              break;

            case UPDATEPROCESSPB_TASK:
              ProofReadSetProcessPB( pIda );
              break;

          } /* endswitch */
        } /* endif */
      }
      break;

    case WM_CLOSE:
      {
        PPROOFREADDATA pIda = ACCESSDLGIDA( hwnd, PPROOFREADDATA );

        pIda->pLastUsed->fShowModified = pIda->fShowModified;
        pIda->pLastUsed->fShowUnchanged = pIda->fShowUnchanged;
        pIda->pLastUsed->fShowProcessed = pIda->fShowProcessed;
        pIda->pLastUsed->fProcessDoc = pIda->fProcessDoc;
        pIda->pLastUsed->fProcessMem = pIda->fProcessMem;
        pIda->pLastUsed->fShowDifferences = pIda->fShowDifferences;

        // copy current color setting to last used values
        {
            int i = 0;
            while( pIda->ColorData.ColorSetting[i].szElement[0] != EOS )
            {
              pIda->pLastUsed->aclrForeground[i] = pIda->ColorData.ColorSetting[i].cForeground;
              pIda->pLastUsed->aclrBackground[i] = pIda->ColorData.ColorSetting[i].cBackground;
              i++;
            } /* endwhile */
        } 

        // save last used values
        {
          UtlSaveWindowPos( GetParent( hwnd ), &(pIda->pLastUsed->swpSizePos) );
          ProofReadSaveLastUsedValues( pIda );
        }
        SendMessage( (HWND)UtlQueryULong( QL_TWBCLIENT ), WM_MDIDESTROY, MP1FROMHWND(hwnd), 0L ) ;
        return 0;
      }
      break;

  case WM_GETMINMAXINFO:
    {
      MINMAXINFO FAR *lpMinMax = (MINMAXINFO *)PVOIDFROMMP2(mp2);
      pIda = ACCESSDLGIDA( hwnd, PPROOFREADDATA );
      lpMinMax->ptMinTrackSize.x = pIda->iMinDialogWidth;
      lpMinMax->ptMinTrackSize.y = pIda->iMinDialogHeight;
    }

   case WM_SIZE :
     pIda = ACCESSDLGIDA( hwnd, PPROOFREADDATA );
     // resize inner window only if normal sizing request...
     if ( (pIda != NULL) && ((mp1 == SIZENORMAL) || (mp1 == SIZEFULLSCREEN)) )
     {
       SHORT   sWidth  = LOWORD( mp2 );      // new width of dialog
       SHORT   sHeight = HIWORD( mp2 );      // new height of dialog
       LONG   lBorderSize  = WinQuerySysValue (HWND_DESKTOP, SV_CXSIZEBORDER);
       LONG   cxAvail = sWidth - (2 * lBorderSize);
       RECT rect;
       LONG  yTextStartPos;          // vertical start position for text area

        // we re-position/re-size all dialog controls ...
        HDWP hdwp = BeginDeferWindowPos( 11 );

        // resize options-and-documents groupbox
        GetWindowRect( GetDlgItem( hwnd, ID_PROOFIMP_FILTER_GB ), &rect );
        MapWindowPoints( HWND_DESKTOP, hwnd, (LPPOINT)&rect, 2 );
        hdwp = DeferWindowPos( hdwp, GetDlgItem( hwnd, ID_PROOFIMP_FILTER_GB ),
                              HWND_TOP, 0, 0,
                              (cxAvail > (rect.left + 3)) ? (cxAvail - rect.left - 3) : 0,
                              rect.bottom - rect.top,
                              SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER );

          // resize documents listbox and groupbox
          GetWindowRect( GetDlgItem( hwnd, ID_PROOFIMP_DOCUMENTS_GB ), &rect );
          MapWindowPoints( HWND_DESKTOP, hwnd, (LPPOINT)&rect, 2 );
          hdwp = DeferWindowPos( hdwp, GetDlgItem( hwnd, ID_PROOFIMP_DOCUMENTS_GB ),
                                HWND_TOP, 0, 0,
                                (cxAvail > (rect.left + 6)) ? (cxAvail - rect.left - 6) : 0,
                                rect.bottom - rect.top,
                                SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER );
          yTextStartPos = rect.bottom + 2;

          GetWindowRect( GetDlgItem( hwnd, ID_PROOFIMP_DOCUMENTS_LB ), &rect );
          MapWindowPoints( HWND_DESKTOP, hwnd, (LPPOINT)&rect, 2 );
          hdwp = DeferWindowPos( hdwp, GetDlgItem( hwnd, ID_PROOFIMP_DOCUMENTS_LB ),
                                HWND_TOP, 0, 0,
                                (cxAvail > (rect.left + 8)) ? (cxAvail - rect.left - 8) : 0,
                                rect.bottom - rect.top,
                                SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER );

           // resize and reposition result area
           {
             RECT rect;
             LONG lResultButtonTop;
             LONG lGroupBoxBottom = 0;
             LONG lGroupBoxWidth = 0;
             LONG lGroupBoxLeft = 0;

             // resize text groupbox
             GetWindowRect( GetDlgItem( hwnd, ID_PROOFIMP_SEGMENTS_GB ), &rect );
             MapWindowPoints( HWND_DESKTOP, hwnd, (LPPOINT)&rect, 2 );
             rect.bottom = sHeight - pIda->sBorderSize - 4;
             lGroupBoxWidth = cxAvail - (2 * pIda->sBorderSize);
             hdwp = DeferWindowPos( hdwp, GetDlgItem( hwnd, ID_PROOFIMP_SEGMENTS_GB ),
                                    HWND_TOP,
                                    0, 0,
                                    lGroupBoxWidth,
                                    rect.bottom - rect.top,
                                    SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER );
             lGroupBoxBottom = rect.bottom;
             lGroupBoxLeft = rect.left;
             

             // adjust position of result area buttons
             LONG lCurXPos;
             HWND hwndButton = GetDlgItem( hwnd, ID_PROOFIMP_SELECTALL_PB );
             
             GetWindowRect( hwndButton, &rect );
             MapWindowPoints( HWND_DESKTOP, hwnd, (LPPOINT)&rect, 2 );
             lResultButtonTop = rect.top;
             hdwp = DeferWindowPos( hdwp, hwndButton, HWND_TOP, rect.left, lGroupBoxBottom - (rect.bottom - rect.top) - 6, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER );
             lCurXPos = rect.right + 5;
             
             hwndButton = GetDlgItem( hwnd, ID_PROOFIMP_DESELECTALL_PB );
             GetWindowRect( hwndButton, &rect );
             MapWindowPoints( HWND_DESKTOP, hwnd, (LPPOINT)&rect, 2 );
             hdwp = DeferWindowPos( hdwp, hwndButton, HWND_TOP, lCurXPos, lGroupBoxBottom - (rect.bottom - rect.top) - 6, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER );
             lCurXPos += (rect.right - rect.left) + 20;

             hwndButton = GetDlgItem( hwnd, ID_PROOFIMP_AUTOUPDATE_PB );
             GetWindowRect( hwndButton, &rect );
             MapWindowPoints( HWND_DESKTOP, hwnd, (LPPOINT)&rect, 2 );
             hdwp = DeferWindowPos( hdwp, hwndButton, HWND_TOP, lCurXPos, lGroupBoxBottom - (rect.bottom - rect.top) - 6, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER );
             lCurXPos += (rect.right - rect.left) + 5;

             hwndButton = GetDlgItem( hwnd, ID_PROOFIMP_OPEN_PB );
             GetWindowRect( hwndButton, &rect );
             MapWindowPoints( HWND_DESKTOP, hwnd, (LPPOINT)&rect, 2 );
             hdwp = DeferWindowPos( hdwp, hwndButton, HWND_TOP, lCurXPos, lGroupBoxBottom - (rect.bottom - rect.top) - 6, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER );
             lCurXPos += (rect.right - rect.left) + 20;

             hwndButton = GetDlgItem( hwnd, ID_PROOFIMP_IMPORT_PB );
             GetWindowRect( hwndButton, &rect );
             MapWindowPoints( HWND_DESKTOP, hwnd, (LPPOINT)&rect, 2 );
             hdwp = DeferWindowPos( hdwp, hwndButton, HWND_TOP, lCurXPos, lGroupBoxBottom - (rect.bottom - rect.top) - 6, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER );
             lCurXPos += (rect.right - rect.left) + 20;

             hwndButton = GetDlgItem( hwnd, ID_PROOFIMP_SAVE_PB );
             GetWindowRect( hwndButton, &rect );
             MapWindowPoints( HWND_DESKTOP, hwnd, (LPPOINT)&rect, 2 );
             hdwp = DeferWindowPos( hdwp, hwndButton, HWND_TOP, lCurXPos, lGroupBoxBottom - (rect.bottom - rect.top) - 6, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER );
             lCurXPos += (rect.right - rect.left) + 5;

             hwndButton = GetDlgItem( hwnd, ID_PROOFIMP_LOAD_PB );
             GetWindowRect( hwndButton, &rect );
             MapWindowPoints( HWND_DESKTOP, hwnd, (LPPOINT)&rect, 2 );
             hdwp = DeferWindowPos( hdwp, hwndButton, HWND_TOP, lCurXPos, lGroupBoxBottom - (rect.bottom - rect.top) - 6, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER );
             lCurXPos += (rect.right - rect.left) + 5;

             hwndButton = GetDlgItem( hwnd, ID_PROOFIMP_CLOSE_PB );
             GetWindowRect( hwndButton, &rect );
             MapWindowPoints( HWND_DESKTOP, hwnd, (LPPOINT)&rect, 2 );
             hdwp = DeferWindowPos( hdwp, hwndButton, HWND_TOP, lCurXPos, lGroupBoxBottom - (rect.bottom - rect.top) - 6, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER );
             lCurXPos += (rect.right - rect.left) + 20;

             hwndButton = GetDlgItem( hwnd, ID_PROOFIMP_HELP_PB );
             GetWindowRect( hwndButton, &rect );
             MapWindowPoints( HWND_DESKTOP, hwnd, (LPPOINT)&rect, 2 );
             hdwp = DeferWindowPos( hdwp, hwndButton, HWND_TOP, lCurXPos, lGroupBoxBottom - (rect.bottom - rect.top) - 6, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER );
             lCurXPos += (rect.right - rect.left) + 5;

             // resize result control
             GetWindowRect( GetDlgItem( hwnd, ID_PROOFIMP_SEGMENTS_GB ), &rect );
             MapWindowPoints( HWND_DESKTOP, hwnd, (LPPOINT)&rect, 2 );
             hdwp = DeferWindowPos( hdwp, GetDlgItem( hwnd, ID_PROOFIMP_SEGMENTS_GB ),
                                    HWND_TOP,
                                    0, 0,
                                    lGroupBoxWidth - 2 * (rect.left - lGroupBoxLeft ),
                                    lResultButtonTop - rect.top - 8,
                                    SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER );

             GetWindowRect( GetDlgItem( hwnd, ID_PROOFREAD_HEADER_TEXT ), &rect );
             MapWindowPoints( HWND_DESKTOP, hwnd, (LPPOINT)&rect, 2 );

             hdwp = DeferWindowPos( hdwp, GetDlgItem( hwnd, ID_PROOFREAD_HEADER_TEXT ),
                                    HWND_TOP,
                                    0, 0,
                                    lGroupBoxWidth - 2 * (rect.left - lGroupBoxLeft ),
                                    rect.bottom - rect.top,
                                    SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER );

             GetWindowRect( GetDlgItem( hwnd, ID_PROOFREAD_TABLE_LV ), &rect );
             MapWindowPoints( HWND_DESKTOP, hwnd, (LPPOINT)&rect, 2 );

             hdwp = DeferWindowPos( hdwp, GetDlgItem( hwnd, ID_PROOFREAD_TABLE_LV ),
                                    HWND_TOP,
                                    0, 0,
                                    lGroupBoxWidth - 2 * (rect.left - lGroupBoxLeft ),
                                    lResultButtonTop - rect.top - 12,
                                    SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER );
           }

           {
             LONG  lXPos;                  // current x (horizontal) position
             LONG  lYPos;                  // current y (vertical) position (starting from bottom to top!)
             int i;

             // we re-position/re-size all dialog controls ...
             HDWP hdwp = BeginDeferWindowPos( 11 );

            // re-arrange status bar
            //SendMessage( pIda->hStatus, WM_SIZE, 0, 0);

            // re-arrange pushbuttons
            {
              LONG  lGap;                   // gap between pushbuttons
              LONG  lTotGaps;               // total size of gaps between pushbuttons
              LONG  lTotSize = 0;           // total width of pushbuttons
              LONG  lCorrect;               // correction value

              for ( i = 0; i < NUM_OF_PRI_BUTTONS; i++ )
              {
                lTotSize += pIda->sButtonWidth[i];
              } /* endfor */

              lTotGaps = (cxAvail > lTotSize) ? (cxAvail - lTotSize) : 0;
              lGap = lTotGaps / NUM_OF_PRI_BUTTONS;
              lCorrect = (cxAvail > lTotSize) ? ((cxAvail - (lGap * 6) - lTotSize) / 2) : 0;
              lXPos    = pIda->sBorderSize;
              lYPos    = sHeight - pIda->sBorderSize - pIda->sButtonHeight /* - lStatusLineHeight */ - 4;
              for ( i = 0; (i < NUM_OF_PRI_BUTTONS) && (hdwp != NULL); i++ )
              {
                lXPos += i ? (pIda->sButtonWidth[i-1] + lGap) : ((lGap / 2) + lCorrect);
                hdwp = DeferWindowPos( hdwp, pIda->hwndButton[i], HWND_TOP, lXPos, lYPos, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER );
              } /* endfor */
            }
           }

           // do actual dialog control re-positioning
           if ( hdwp != NULL )
           {
             EndDeferWindowPos( hdwp );
           } /* endif */

           // adjust status bar 
           {
             int iWidths[3];

             iWidths[0] = cxAvail / 3;
             iWidths[1] = iWidths[0];
             iWidths[2] = -1;
             SendMessage( pIda->hStatus, SB_SETPARTS, 3, (LPARAM)iWidths );
           }

           // adjust result control columns
           ProofReadResizeColumns( hwnd, pIda );

       // save current size and position as last valid window size and position
       UtlSaveWindowPos( GetParent( hwnd ), &(pIda->pLastUsed->swpSizePos) );

     } /* endif */
     break;

  case WM_COMMAND:
    {
      PPROOFREADDATA pIda = ACCESSDLGIDA( hwnd, PPROOFREADDATA );
      switch ( WMCOMMANDID( mp1, mp2 ) )
      {
        //case ID_PROOFREAD_COLOR_PB:
        //  {
        //    if ( UtlColorChooserDlg( &(pIda->ColorData) ) == 0 )
        //    {
        //      // refresh result list
        //      InvalidateRect( hwnd, ID_PROOFREAD_TABLE_LV, NULL, TRUE );
        //    } /* endif */
        //  } 
        //  break;

        case ID_PROOFIMP_SETCOLOR_PB:
          if ( UtlColorChooserDlg( &(pIda->ColorData) ) == 0 )
          {
            // refresh result list
            InvalidateRect( pIda->hResultListBox, NULL, TRUE );
          } /* endif */
          break;

        case ID_PROOFIMP_SETFONT_PB:
          {
            CHOOSEFONT ChooseFontData;

            memset( &ChooseFontData, 0, sizeof(ChooseFontData) );
            ChooseFontData.lStructSize = sizeof(ChooseFontData);
            ChooseFontData.hwndOwner = hwnd;
            ChooseFontData.Flags = CF_FORCEFONTEXIST | CF_SCREENFONTS | CF_INITTOLOGFONTSTRUCT; //  | CF_USESTYLE;
            ChooseFontData.lpLogFont = &(pIda->lf);

            if ( ChooseFont( &ChooseFontData ) )
            {
              // free any previously created font
              if ( pIda->hFontControl ) DeleteObject( pIda->hFontControl );

              // create new font to be used
              pIda->hFontControl = CreateFontIndirect( &(pIda->lf) );

              // update last used values
              if ( pIda->pLastUsed )
              {
                pIda->pLastUsed->lfHeight = pIda->lf.lfHeight;
                pIda->pLastUsed->lfWidth = pIda->lf.lfWidth;
                pIda->pLastUsed->lfEscapement = pIda->lf.lfEscapement;
                pIda->pLastUsed->lfOrientation = pIda->lf.lfOrientation;
                pIda->pLastUsed->lfWeight = pIda->lf.lfWeight;
                pIda->pLastUsed->lfItalic = pIda->lf.lfItalic;
                pIda->pLastUsed->lfUnderline = pIda->lf.lfUnderline;
                pIda->pLastUsed->lfStrikeOut = pIda->lf.lfStrikeOut;
                pIda->pLastUsed->lfCharSet = pIda->lf.lfCharSet;
                pIda->pLastUsed->lfOutPrecision = pIda->lf.lfOutPrecision;
                pIda->pLastUsed->lfClipPrecision = pIda->lf.lfClipPrecision;
                pIda->pLastUsed->lfQuality = pIda->lf.lfQuality;
                pIda->pLastUsed->lfPitchAndFamily = pIda->lf.lfPitchAndFamily;
                strcpy( pIda->pLastUsed->lfFaceName, pIda->lf.lfFaceName );
              } /* endif */

              // refresh the contents of the listbox to force a re-computation of the item heights
              SendMessage( pIda->hResultListBox, WM_SETREDRAW, FALSE, 0 );
              int iTopIndex = SendMessage( pIda->hResultListBox, LB_GETTOPINDEX, 0, 0 );
              ProofReadRefreshListbox( pIda );
              if ( iTopIndex >= 0 ) SendMessage( pIda->hResultListBox, LB_SETTOPINDEX, iTopIndex, 0 );
              SendMessage( pIda->hResultListBox, WM_SETREDRAW, TRUE, 0 );

              // refresh result list
              //InvalidateRect( pIda->hResultListBox, NULL, TRUE );
            } /* endif */
          }
          break;

        case ID_PROOFIMP_SAVE_PB:
          ProofReadSaveList( pIda );
          break;

        case ID_PROOFIMP_LOAD_PB:
          if ( ProofReadLoadList( pIda ) )
          {
            // refresh differences list
            ProofReadMakeDiffLists( pIda, 0, pIda->pList->size() - 1 );


            // refresh result list
            InvalidateRect( pIda->hResultListBox, NULL, TRUE );
          } /* endif */
          break;

        case ID_PROOFIMP_HELP_PB:
          UtlInvokeHelp();
          break;

        case DID_CANCEL:
          // do not close the dialog when the on-spot editing is active, just end on-spot editing instead
          if ( ProofReadIsOnSpotEditingActive() ) 
          {
            ProofReadEndOnSpotEditing();
          }
          else
          {
            WinPostMsg( hwnd, WM_CLOSE, 0L, 0 );
          }
          break;

        case ID_PROOFIMP_CLOSE_PB:
          WinPostMsg( hwnd, WM_CLOSE, 0L, 0 );
          break;

        //case ID_ProofRead_COLLAPSE_PB:
        //  {
        //    RECT rcCurrent;
        //    int iChangeInHeight = 0;

        //    // toggle collapse flag
        //    pIda->fCollapsed = !pIda->fCollapsed;

        //    // hide or show options and document list
        //    int iShowCmd =  pIda->fCollapsed ? SW_HIDE : SW_SHOW ;
        //    ShowWindow( GetDlgItem( hwnd, ID_ProofRead_WILDCARD_GB ), iShowCmd );
        //    ShowWindow( GetDlgItem( hwnd, ID_ProofRead_WILDCARDSINGLE_TEXT ), iShowCmd );
        //    ShowWindow( GetDlgItem( hwnd, ID_ProofRead_WILDCARDSINGLE_CB ), iShowCmd );
        //    ShowWindow( GetDlgItem( hwnd, ID_ProofRead_WILDCARDMULT_TEXT ), iShowCmd );
        //    ShowWindow( GetDlgItem( hwnd, ID_ProofRead_WILDCARDMULT_CB ), iShowCmd );
        //    ShowWindow( GetDlgItem( hwnd, ID_ProofRead_PARAMETERS_GB ), iShowCmd );
        //    ShowWindow( GetDlgItem( hwnd, ID_ProofRead_MEMUPD_CHK ), iShowCmd );
        //    ShowWindow( GetDlgItem( hwnd, ID_ProofRead_CONFCHANGES_CHK ), iShowCmd );
        //    ShowWindow( GetDlgItem( hwnd, ID_ProofRead_TRANSLTEXT_CHK ), iShowCmd );
        //    ShowWindow( GetDlgItem( hwnd, ID_ProofRead_WHOLEWORD_CHK ), iShowCmd );
        //    ShowWindow( GetDlgItem( hwnd, ID_ProofRead_CASE_GB ), iShowCmd );
        //    ShowWindow( GetDlgItem( hwnd, ID_ProofRead_RESPECT_RB ), iShowCmd );
        //    ShowWindow( GetDlgItem( hwnd, ID_ProofRead_IGNORE_RB ), iShowCmd );
        //    ShowWindow( GetDlgItem( hwnd, ID_ProofRead_SEARCHIN_GB ), iShowCmd );
        //    ShowWindow( GetDlgItem( hwnd, ID_ProofRead_SEARCHTARGET_RB ), iShowCmd );
        //    ShowWindow( GetDlgItem( hwnd, ID_ProofRead_SEARCHSOURCE_RB ), iShowCmd );
        //    ShowWindow( GetDlgItem( hwnd, ID_ProofRead_SEARCHBOTH_RB ), iShowCmd );
        //    ShowWindow( GetDlgItem( hwnd, ID_ProofRead_DISPLAY_GB ), iShowCmd );
        //    ShowWindow( GetDlgItem( hwnd, ID_ProofRead_SHOWSOURCEANDTARGET_CHK ), iShowCmd );
        //    ShowWindow( GetDlgItem( hwnd, ID_ProofRead_RESPECTLF_CHK ), iShowCmd );
        //    ShowWindow( GetDlgItem( hwnd, ID_ProofRead_SHOW_GB ), iShowCmd );
        //    ShowWindow( GetDlgItem( hwnd, ID_ProofRead_SEGSBEFORE_TEXT ), iShowCmd );
        //    ShowWindow( GetDlgItem( hwnd, ID_ProofRead_SEGSAROUND_CB ), iShowCmd );
        //    ShowWindow( GetDlgItem( hwnd, ID_ProofRead_DOCS_GB ), iShowCmd );
        //    ShowWindow( GetDlgItem( hwnd, ID_ProofRead_DOCS_LB ), iShowCmd );

        //    // re-size options-and-documents groupbox
        //    GetWindowRect( GetDlgItem( hwnd, ID_ProofRead_OPTDOCS_GB ), &rcCurrent );
        //    MapWindowPoints( HWND_DESKTOP, hwnd, (LPPOINT)&rcCurrent, 2 );
        //    int iNewHeight = pIda->fCollapsed ? 16 : (pIda->rcOrgOptsAndDocsGB.bottom - pIda->rcOrgOptsAndDocsGB.top);
        //    iChangeInHeight = rcCurrent.bottom - rcCurrent.top - iNewHeight;
        //    SetWindowPos( GetDlgItem( hwnd, ID_ProofRead_OPTDOCS_GB ), HWND_TOP, 0, 0, rcCurrent.right - rcCurrent.left, iNewHeight, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER );

        //    // re-size result area
        //    GetWindowRect( GetDlgItem( hwnd, ID_ProofRead_TEXT_GB ), &rcCurrent );
        //    MapWindowPoints( HWND_DESKTOP, hwnd, (LPPOINT)&rcCurrent, 2 );
        //    SetWindowPos( GetDlgItem( hwnd, ID_ProofRead_TEXT_GB ), HWND_TOP, rcCurrent.left, rcCurrent.top - iChangeInHeight, 
        //                  rcCurrent.right - rcCurrent.left, rcCurrent.bottom - rcCurrent.top + iChangeInHeight, SWP_NOACTIVATE | SWP_NOZORDER );
        //    GetWindowRect( GetDlgItem( hwnd, ID_ProofRead_HEADER_TEXT ), &rcCurrent );
        //    MapWindowPoints( HWND_DESKTOP, hwnd, (LPPOINT)&rcCurrent, 2 );
        //    SetWindowPos( GetDlgItem( hwnd, ID_ProofRead_HEADER_TEXT ), HWND_TOP, rcCurrent.left, rcCurrent.top - iChangeInHeight, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER );
        //    GetWindowRect( GetDlgItem( hwnd, ID_ProofRead_RESULT_LISTBOX ), &rcCurrent );
        //    MapWindowPoints( HWND_DESKTOP, hwnd, (LPPOINT)&rcCurrent, 2 );
        //    SetWindowPos( GetDlgItem( hwnd, ID_ProofRead_RESULT_LISTBOX ), HWND_TOP, rcCurrent.left, rcCurrent.top - iChangeInHeight, 
        //                  rcCurrent.right - rcCurrent.left, rcCurrent.bottom - rcCurrent.top + iChangeInHeight, SWP_NOACTIVATE | SWP_NOZORDER );

        //    // change collapse button text
        //    SetWindowText( GetDlgItem( hwnd, ID_ProofRead_COLLAPSE_PB ), pIda->fCollapsed ? "+" : "-" );
        //  }
        //  break;

        case ID_PROOFIMP_OPEN_PB :
          ProofReadOpenDoc( pIda );
          break;

        case ID_PROOFIMP_AUTOUPDATE_PB :
          ProofReadProcessList( pIda->pList, pIda->fProcessDoc, pIda->fProcessMem, hwnd, pIda->iProcessRuns++ );
          break;

        case ID_PROOFIMP_SELECTALL_PB:
          ProofReadSelectAll( pIda, TRUE );
          break;

        case ID_PROOFIMP_DESELECTALL_PB:
          ProofReadSelectAll( pIda, FALSE );
          break;

        case ID_PROOFIMP_IMPORT_PB:
          ProofReadImport( pIda );
          break;

        case ID_PROOFIMP_SHOWDIFF_CHK:
          if ( WMCOMMANDCMD( mp1, mp2 ) == BN_CLICKED )
          {
            pIda->fShowDifferences = QUERYCHECK( hwnd, ID_PROOFIMP_SHOWDIFF_CHK );
            InvalidateRect( pIda->hResultListBox, NULL, TRUE );
          } /* endif */
          break;

        case ID_PROOFIMP_SHOWMODIFIED_CHK:
          if ( WMCOMMANDCMD( mp1, mp2 ) == BN_CLICKED )
          {
            pIda->fShowModified = QUERYCHECK( hwnd, ID_PROOFIMP_SHOWMODIFIED_CHK );
            ProofReadRefreshListbox( pIda );
          } /* endif */
          break;

        case ID_PROOFIMP_SHOWUNCHANGED_CHK:
          if ( WMCOMMANDCMD( mp1, mp2 ) == BN_CLICKED )
          {
            pIda->fShowUnchanged = QUERYCHECK( hwnd, ID_PROOFIMP_SHOWUNCHANGED_CHK );
            ProofReadRefreshListbox( pIda );
          } /* endif */
          break;

        case ID_PROOFIMP_SHOWPROCESSED_CHK:
          if ( WMCOMMANDCMD( mp1, mp2 ) == BN_CLICKED )
          {
            pIda->fShowProcessed = QUERYCHECK( hwnd, ID_PROOFIMP_SHOWPROCESSED_CHK );
            ProofReadRefreshListbox( pIda );
          } /* endif */
          break;

        case ID_PROOFIMP_UPDTRANS_CHK:
          if ( WMCOMMANDCMD( mp1, mp2 ) == BN_CLICKED )
          {
            pIda->fProcessDoc = QUERYCHECK( hwnd, ID_PROOFIMP_UPDTRANS_CHK );
            ProofReadSetProcessPB( pIda );
          } /* endif */
          break;

        case ID_PROOFIMP_UPDMEM_CHK:
          if ( WMCOMMANDCMD( mp1, mp2 ) == BN_CLICKED )
          {
            pIda->fProcessMem = QUERYCHECK( hwnd, ID_PROOFIMP_UPDMEM_CHK );
            ProofReadSetProcessPB( pIda );
          } /* endif */
          break;
       } /* endswitch */
     } 
     break;

    case WM_EQF_INITMENU:
    case WM_INITMENU:
    case WM_INITMENUPOPUP:
      UtlMenuDisableAll( hwnd, SHORT1FROMMP1( mp1) );
      break;

    case WM_EQF_TOOLBAR_ENABLED:
      break;


    case WM_DRAWITEM:                // draw a listbox/combobox item
      {
        LPDRAWITEMSTRUCT lpDisp = (LPDRAWITEMSTRUCT)mp2;
        PPROOFREADDATA pIda = ACCESSDLGIDA( hwnd, PPROOFREADDATA );
        ProofReadDrawItem( pIda, lpDisp );
        return( TRUE );
      }
      break;

  case WM_MEASUREITEM:
    {
      PPROOFREADDATA pIda = ACCESSDLGIDA( hwnd, PPROOFREADDATA );
      LPMEASUREITEMSTRUCT pMeasureItem = (LPMEASUREITEMSTRUCT)mp2; 
      ProofReadMeasureItem( pIda, pMeasureItem );
      return( TRUE );
    }
    break;    

    case WM_DESTROY:
      pIda = ACCESSDLGIDA( hwnd, PPROOFREADDATA );
      if (pIda)
      {
        if ( pIda->pLastUsed ) UtlAlloc ((PVOID *)&(pIda->pLastUsed), 0L, 0L, NOMSG);
        UtlAlloc ((PVOID *)&pIda, 0L, 0L, NOMSG);
      } /* endif */
      UtlUnregisterModelessDlg( hwnd );
      break;

    case WM_KEYDOWN:
      if ( mp1 == VK_ESC )
      {
        pIda = ACCESSDLGIDA( hwnd, PPROOFREADDATA );
        if (pIda)
        {
        } /* endif */
      }
      else
      {
         mResult = WinDefDlgProc( hwndDlg, msg, mp1, mp2 );
      } /* endif */
      break;

      default:
         mResult = WinDefDlgProc( hwndDlg, msg, mp1, mp2 );
         break;
   } /* endswitch */

   return mResult;
} /* end of ProofReadDlgProc */


static BOOL ProofReadGetLastUsedValues( PPROOFREADDATA pIda )
{
  PPROOFREADIMPORTLASTUSED pLastUsed = NULL;
  BOOL fOK = TRUE;

  // build name of our file containing the last used values
  CHAR szLastUsedValues[MAX_LONGPATH];
  UtlMakeEQFPath( szLastUsedValues, NULC, PROPERTY_PATH, NULL );
  strcat( szLastUsedValues, "\\ProofReadImportLastUsedValues.DAT" );

  // allocate last values area
  fOK = UtlAlloc( (PVOID *)&pLastUsed, 0, sizeof(PROOFREADIMPORTLASTUSED), ERROR_STORAGE );
  if ( !fOK ) return( FALSE );

  // anchor last used values in IDA
  pIda->pLastUsed = pLastUsed;

  // preset the area with the default values
  ProofReadSetDefaults( pIda );

  // load existing file, use values from folder list properties if load fails
  FILE *hfLastUsed = fopen( szLastUsedValues, "rb" );

  if ( hfLastUsed != NULL )
  {
    // load last used values (don't care if loaded data is shorter than our last used values area)
    fread( pLastUsed, 1, sizeof(PROOFREADIMPORTLASTUSED), hfLastUsed );
    fclose( hfLastUsed );
  }
  return( TRUE );
}

static BOOL ProofReadSaveLastUsedValues( PPROOFREADDATA pIda )
{
  // build name of our file containing the last used values
  CHAR szLastUsedValues[MAX_LONGPATH];
  UtlMakeEQFPath( szLastUsedValues, NULC, PROPERTY_PATH, NULL );
  strcat( szLastUsedValues, "\\ProofReadImportLastUsedValues.DAT" );

  UtlWriteFile( szLastUsedValues, sizeof(PROOFREADIMPORTLASTUSED), (PVOID)pIda->pLastUsed, FALSE );

  return( TRUE );
}



BOOL ProofReadAdjustResultColumns( HWND hwnd, PPROOFREADDATA pIda )
{
  RECT rect;

  GetWindowRect( pIda->hResultListBox, &rect );

  pIda->aiListViewColWidth[0] = CHECKBOX_COL_WIDTH;
  pIda->aiListViewColWidth[1] = DOCNUM_COL_WIDTH;
  pIda->aiListViewColWidth[2] = SEGNUM_COL_WIDTH;
  int iColWidth = (rect.right - rect.left - CHECKBOX_COL_WIDTH - DOCNUM_COL_WIDTH - SEGNUM_COL_WIDTH) / 7;
  pIda->aiListViewColWidth[3] = 
  pIda->aiListViewColWidth[4] = 
  pIda->aiListViewColWidth[5] = 2 * iColWidth;  
  pIda->aiListViewColWidth[6] = iColWidth;  
  return( TRUE );
}

BOOL ProofReadResizeColumns( HWND hwnd, PPROOFREADDATA pIda )
{
  ProofReadAdjustResultColumns( hwnd, pIda );

  // force a repaint of the header and the result list
  SETTEXT( hwnd, ID_FOLFIND_HEADER_TEXT, "y" );
  ProofReadForceRefreshOfItemHeights( pIda );
  return( TRUE );
}


void ProofReadDrawCheckbox( PPROOFREADDATA pIda, HDC hdc, PRECT prc, BOOL fSelected )
{
  MoveToEx( hdc, prc->left + CHECKBOX_XPOS, prc->top + CHECKBOX_YPOS, NULL );
  LineTo( hdc, prc->left + CHECKBOX_XPOS + CHECKBOX_XSIZE, prc->top + CHECKBOX_YPOS );
  LineTo( hdc, prc->left + CHECKBOX_XPOS + CHECKBOX_XSIZE, prc->top + CHECKBOX_YPOS + CHECKBOX_YSIZE );
  LineTo( hdc, prc->left + CHECKBOX_XPOS, prc->top + CHECKBOX_YPOS + CHECKBOX_YSIZE );
  LineTo( hdc, prc->left + CHECKBOX_XPOS, prc->top + CHECKBOX_YPOS );

  if ( fSelected )
  {
    LineTo( hdc, prc->left + CHECKBOX_XPOS + CHECKBOX_XSIZE, prc->top + CHECKBOX_YPOS + CHECKBOX_YSIZE );
    MoveToEx( hdc, prc->left + CHECKBOX_XPOS, prc->top + CHECKBOX_YPOS + CHECKBOX_YSIZE, NULL );
    LineTo( hdc, prc->left + CHECKBOX_XPOS + CHECKBOX_XSIZE, prc->top + CHECKBOX_YPOS );
  }
}


// handle WM_DRAWITEM message for our ownerdrawn listbox
void ProofReadDrawItem( PPROOFREADDATA pIda, LPDRAWITEMSTRUCT lpDrawItem )
{
  WCHAR szString[256];
  UINT uFirstColWidth;
  RECT rcClip;
  int iBaseColorIndex = 0;

  if ( lpDrawItem->hwndItem == GetDlgItem( pIda->hwnd, ID_PROOFREAD_HEADER_TEXT ) )
  {
    PSZ_W pszText;

    // only handle normal draw operations
    if ( (lpDrawItem->itemAction != ODA_FOCUS) && (lpDrawItem->itemAction != ODA_SELECT) && (lpDrawItem->itemAction != ODA_DRAWENTIRE) ) return;
   
    DWORD dwBackColor = pIda->ColorData.ColorSetting[iBaseColorIndex].cBackground;
    SetBkColor( lpDrawItem->hDC, dwBackColor );
    SetTextColor( lpDrawItem->hDC, pIda->ColorData.ColorSetting[iBaseColorIndex].cForeground );

    // erase rectangle
    {
      HBRUSH hBrush = CreateSolidBrush( dwBackColor );
      FillRect( lpDrawItem->hDC, &(lpDrawItem->rcItem), hBrush ); 
      DeleteObject( hBrush );
    }

    // Set up the new clipping rect for the first column text and draw it
    rcClip.left = lpDrawItem->rcItem.left;
    rcClip.right = lpDrawItem->rcItem.left + pIda->aiListViewColWidth[0];
    rcClip.top = lpDrawItem->rcItem.top;
    rcClip.bottom = lpDrawItem->rcItem.bottom;
    ProofReadDrawFrame( lpDrawItem->hDC, &rcClip );
    pszText = L"Sel";
    ExtTextOutW( lpDrawItem->hDC, rcClip.left + 2, rcClip.top + 1, ETO_CLIPPED, &rcClip, pszText, wcslen(pszText), NULL);

    // draw document number column
    rcClip.left = rcClip.right;
    rcClip.right = lpDrawItem->rcItem.left + pIda->aiListViewColWidth[1];
    rcClip.top = lpDrawItem->rcItem.top;
    rcClip.bottom = lpDrawItem->rcItem.bottom;
    ProofReadDrawFrame( lpDrawItem->hDC, &rcClip );
    pszText = L"Document";
    ExtTextOutW( lpDrawItem->hDC, rcClip.left + 2, rcClip.top + 1, ETO_CLIPPED, &rcClip, pszText, wcslen(pszText), NULL);

    // draw segment number column
    rcClip.left = rcClip.right;
    rcClip.right = rcClip.left + pIda->aiListViewColWidth[2];
    ProofReadDrawFrame( lpDrawItem->hDC, &rcClip );
    pszText = L"Segment";
    ExtTextOutW( lpDrawItem->hDC, rcClip.left + 2, rcClip.top + 1, ETO_CLIPPED, &rcClip, pszText, wcslen(pszText), NULL);

    // draw source column header
    rcClip.left = rcClip.right;
    rcClip.right = rcClip.left + pIda->aiListViewColWidth[3];
    ProofReadDrawFrame( lpDrawItem->hDC, &rcClip );
    pszText = L"Source";
    ExtTextOutW( lpDrawItem->hDC, rcClip.left + 2, rcClip.top + 1, ETO_CLIPPED, &rcClip, pszText, wcslen(pszText), NULL);

    // draw target column header
    rcClip.left = rcClip.right;
    rcClip.right = rcClip.left + pIda->aiListViewColWidth[4];
    ProofReadDrawFrame( lpDrawItem->hDC, &rcClip );
    pszText = L"Target";
    ExtTextOutW( lpDrawItem->hDC, rcClip.left + 2, rcClip.top + 1, ETO_CLIPPED, &rcClip, pszText, wcslen(pszText), NULL);

    // draw modified target
    rcClip.left = rcClip.right;
    rcClip.right = rcClip.left + pIda->aiListViewColWidth[5];
    ProofReadDrawFrame( lpDrawItem->hDC, &rcClip );
    pszText = L"Modified Target";
    ExtTextOutW( lpDrawItem->hDC, rcClip.left + 2, rcClip.top + 1, ETO_CLIPPED, &rcClip, pszText, wcslen(pszText), NULL);

    // draw comment
    rcClip.left = rcClip.right;
    rcClip.right = lpDrawItem->rcItem.right;
    ProofReadDrawFrame( lpDrawItem->hDC, &rcClip );
    pszText = L"Comment";
    ExtTextOutW( lpDrawItem->hDC, rcClip.left + 2, rcClip.top + 1, ETO_CLIPPED, &rcClip, pszText, wcslen(pszText), NULL);
  }
  else
  {
    HFONT hFontOld = NULL;

    // If there are no list box items, skip this message. 
    if (lpDrawItem->itemID == -1) return; 

    if ( pIda->hFontControl ) hFontOld = (HFONT)SelectObject( lpDrawItem->hDC, pIda->hFontControl );

    // only handle normal draw operations
    if ( (lpDrawItem->itemAction != ODA_FOCUS) && (lpDrawItem->itemAction != ODA_SELECT) && (lpDrawItem->itemAction != ODA_DRAWENTIRE) ) return;

    iBaseColorIndex = PRI_NORMAL_ENTRY;
    if ( lpDrawItem->itemState & ODS_FOCUS ) 
    { 
      iBaseColorIndex = PRI_FOCUS_ENTRY;
    } 
    else if ( lpDrawItem->itemState & ODS_SELECTED ) 
    { 
      iBaseColorIndex = PRI_SELECTED_ENTRY;
    } 


    //// erase rectangle
    //{
    //  HBRUSH hBrush = CreateSolidBrush( dwBackColor );
    //  FillRect( lpDrawItem->hDC, &(lpDrawItem->rcItem), hBrush ); 
    //  DeleteObject( hBrush );
    //}

    // get the entry for the painted item
    OtmProofReadEntry *pEntry = (OtmProofReadEntry *) QUERYITEMHANDLEHWND( pIda->hResultListBox, lpDrawItem->itemID );
    int iOrgItemHeight = pEntry->getItemHeight();
    int iDrawItemHeight = 0;

    if ( pEntry->getProcessed() )
    {
      iBaseColorIndex += PRI_PROCESSED_ENTRY;
    }
    else if ( pEntry->isUnChanged() )
    {
      iBaseColorIndex += PRI_UNCHANGED_ENTRY;
    } /* endif */

    SetBkColor( lpDrawItem->hDC, pIda->ColorData.ColorSetting[iBaseColorIndex].cBackground );

    // Calculate the width of the first column.
    uFirstColWidth = pIda->aiListViewColWidth[0];

    // draw checkbox column
    rcClip.left = lpDrawItem->rcItem.left;
    rcClip.right = lpDrawItem->rcItem.left + pIda->aiListViewColWidth[0];
    rcClip.top = lpDrawItem->rcItem.top;
    rcClip.bottom = lpDrawItem->rcItem.bottom;

    SetTextColor( lpDrawItem->hDC, pIda->ColorData.ColorSetting[iBaseColorIndex].cForeground );
    SetBkColor( lpDrawItem->hDC, pIda->ColorData.ColorSetting[iBaseColorIndex].cBackground );
    // erase rectangle
    {
      HBRUSH hBrush = CreateSolidBrush( pIda->ColorData.ColorSetting[iBaseColorIndex].cBackground );
      FillRect( lpDrawItem->hDC, &rcClip, hBrush ); 
      DeleteObject( hBrush );
    }
    // draw grid lines
    ProofReadDrawFrame( lpDrawItem->hDC, &rcClip );
    if ( !pEntry->isUnChanged() )
    {
      ProofReadDrawCheckbox( pIda, lpDrawItem->hDC, &rcClip, pEntry->getSelected() );
    } /* endif */
    
    // draw document number column
    rcClip.left = rcClip.right;
    rcClip.right = lpDrawItem->rcItem.left + pIda->aiListViewColWidth[1];
    rcClip.top = lpDrawItem->rcItem.top;
    rcClip.bottom = lpDrawItem->rcItem.bottom;
    swprintf( szString, L"%ld", pEntry->getDocumentNumber() + 1 );
    ProofReadDrawItemColumn( pIda, lpDrawItem->hDC, 1, szString, &rcClip, pEntry, NULL, 0, FALSE, iBaseColorIndex );

    // draw segment number column
    rcClip.left = rcClip.right;
    rcClip.right = rcClip.left + pIda->aiListViewColWidth[2];
    swprintf( szString, L"%lu", pEntry->getSegmentNumber() );
    ProofReadDrawItemColumn( pIda, lpDrawItem->hDC, 2, szString, &rcClip, pEntry, NULL, 0, FALSE, iBaseColorIndex );

    // draw source text
    rcClip.left = rcClip.right;
    rcClip.right = rcClip.left + pIda->aiListViewColWidth[3];
    iDrawItemHeight = ProofReadDrawItemColumn( pIda, lpDrawItem->hDC, 3, pEntry->getSource(), &rcClip, pEntry, NULL, 0, 0, iBaseColorIndex );

    // draw target text
    rcClip.left = rcClip.right;
    rcClip.right = rcClip.left + pIda->aiListViewColWidth[4];
    PPROOFREADCHANGE pChangeList = ( pIda->fShowDifferences ) ? (PPROOFREADCHANGE)pEntry->getTargetChangeList() : NULL;
    int iHeight = ProofReadDrawItemColumn( pIda, lpDrawItem->hDC, 4, pEntry->getTarget(), &rcClip, pEntry, pChangeList, 0, 1, iBaseColorIndex );
    iDrawItemHeight = max( iHeight, iDrawItemHeight );
    


    // draw modified target text
    rcClip.left = rcClip.right;
    rcClip.right = rcClip.left + pIda->aiListViewColWidth[5];
    const wchar_t *pszText = pEntry->getNewTarget();
    if ( *pszText == 0 ) pszText = pEntry->getModTarget();
    pChangeList = ( pIda->fShowDifferences ) ? (PPROOFREADCHANGE)pEntry->getModTargetChangeList() : NULL;
    iHeight = ProofReadDrawItemColumn( pIda, lpDrawItem->hDC, 5, pszText, &rcClip, pEntry, pChangeList, 0, 2, iBaseColorIndex );
    iDrawItemHeight = max( iHeight, iDrawItemHeight );

    // draw comment text
    rcClip.left = rcClip.right;
    rcClip.right = lpDrawItem->rcItem.right;
    iHeight = ProofReadDrawItemColumn( pIda, lpDrawItem->hDC, 6, pEntry->getComment(), &rcClip, pEntry, NULL, 0, 3, iBaseColorIndex );
    iDrawItemHeight = max( iHeight, iDrawItemHeight );

    // draw focus rectangle if the item the focus
    if (lpDrawItem->itemState & ODS_FOCUS) 
    { 
        DrawFocusRect( lpDrawItem->hDC, &(lpDrawItem->rcItem) ); 
    } 

    if ( hFontOld ) SelectObject( lpDrawItem->hDC, hFontOld );

    // resize item if actual draw height is larger than item height
    if ( iDrawItemHeight > iOrgItemHeight )
    {
      pEntry->setItemHeight( iDrawItemHeight );
      SendMessage( pIda->hResultListBox, LB_SETITEMHEIGHT, lpDrawItem->itemID, iDrawItemHeight );
    }
  } /* endif */

  return; 
} 

BOOL ProofReadIsLineSplitChar( PPROOFREADDATA pIda, CHAR_W c ) 
{
  // test for whitespace or punctuation
  if ( iswspace( c) || (c == L'-') || (c == L'/') || (c == L'.') || (c == L',') || (c == L';') )
  {
    return( TRUE );
  }

  // test for DBCS character
  CHAR_W chW[2];
	char ch[5];

	chW[0] = c; chW[1] = EOS;
	memset (ch, 0, sizeof(ch));
  int len = WideCharToMultiByte( pIda->ulOEMCP, 0, chW, 1, ch, sizeof(ch), NULL, NULL );
  return( len == 2 );
}

// get text height in number of lines
int ProofReadGetTextHeight( PPROOFREADDATA pIda, HDC hdc, int iAvail, PSZ_W pszString, int iLen )
{
  SIZE size;
  int iLines = 0;

  do 
  {
    // check if text fits into available space
    GetTextExtentPoint32W( hdc, pszString, iLen, &size );

    // if there is not enough room truncate text until it fits
    if ( size.cx > iAvail )
    {
      int i = iLen - 1;
      BOOL fTextFits = FALSE;
      do
      {
        // find preceeding whitespace char
        while ( (i > 0) && !ProofReadIsLineSplitChar( pIda, pszString[i] ) ) i--;

        // split text at found position and test if it fits
        if ( i != 0 ) 
        {
          GetTextExtentPoint32W( hdc, pszString, i, &size );
          if ( size.cx > iAvail )
          {
            // try a shorter string
            i--;
          }
          else
          {
            fTextFits = TRUE;
          }
        }
      } while ( (i != 0) && !fTextFits );

      if ( fTextFits )
      {
        // go to next line
        iLines++;

        // continue with remaining text
        pszString += i;
        iLen -= i;
      }
      else
      {
        // we have to clip the text
        iLines++;
        iLen = 0;
      }
    }
    else
    {
      // text fits into clipping rectangle
      iLines++;
      iLen = 0;
    } /* endif */
  } while ( iLen != 0 );
  return( iLines );
}

void ProofReadSetTextAndBackgroundStyle( PPROOFREADDATA pIda, HDC hdc, RECT *prc, DRAWTYPE DrawType, int iBaseColorIndex )
{
  int iColorIndex = 0;

  prc; 

  switch( DrawType )
  {
    case NORMALTEXT_DRAWTYPE:     iColorIndex = iBaseColorIndex + PRI_NORMAL_TEXT; break;
    case CHANGED_DRAWTYPE:        iColorIndex = iBaseColorIndex + PRI_CHANGED_TEXT; break;
    case INSERTED_DRAWTYPE:       iColorIndex = iBaseColorIndex + PRI_INSERTED_TEXT; break;
  } /* endswitch */

  SetTextColor( hdc, pIda->ColorData.ColorSetting[iColorIndex].cForeground );
  SetBkColor( hdc, pIda->ColorData.ColorSetting[iColorIndex].cBackground );

  return;
}

// draw text and handle any linebreaks within the text
void ProofReadDrawMultiLineText( PPROOFREADDATA pIda, HDC hdc, PRECT prcClip, PPOINT pPt, const CHAR_W *pszString, int iLen, int iLineHeight, int iBaseColorIndex,  DRAWTYPE DrawType )
{
  //if ( pIda->fRespectLineFeeds )
  //{
  //  // copy the string to our buffer and use the text in the buffer
  //  wcsncpy( pIda->szColTextBuffer, pszString, iLen );
  //  pIda->szColTextBuffer[iLen] = 0;
  //  pszString = pIda->szColTextBuffer;

  //  // split text at line feeds and draw the resulting text lines
  //  PSZ_W pszLineFeed = NULL;
  //  do
  //  {
  //    pszLineFeed = wcschr( pszString, L'\n' );
  //    if ( pszLineFeed != NULL )
  //    {
  //      // draw current line of text
  //      int iLineLen = pszLineFeed - pszString;
  //      if ( iLineLen > 0 ) ProofReadDrawText( pIda, hdc, prcClip, pPt, pszString, iLineLen, iLineHeight, iBaseColorIndex, DrawType );

  //      // go to next line
  //      pPt->x = prcClip->left + 2;
  //      pPt->y += iLineHeight;

  //      // process the rest of the text
  //      pszString = pszLineFeed + 1;
  //      iLen -= iLineLen + 1;

  //      if ( *pszString == 0 ) iLen = 0;
  //    }
  //    else
  //    {
  //      // draw remaining text
  //      if ( iLen > 0 ) ProofReadDrawText( pIda, hdc, prcClip, pPt, pszString, iLen, iLineHeight, iBaseColorIndex, DrawType );
  //    } /* endif */
  //  } while ( (pszLineFeed != NULL) && (iLen != 0) ); /* endwhile */
  //}
  //else
  //{
    // copy string to our buffer and replace LFs with blank before drawing the text
    wcsncpy( pIda->szColTextBuffer, pszString, iLen );
    pIda->szColTextBuffer[iLen] = 0;
    for( int i = 0; i < iLen; i++ )
    {
      if ( (pIda->szColTextBuffer[i] == L'\n') || (pIda->szColTextBuffer[i] == L'\r') ) pIda->szColTextBuffer[i] = L' ';
    } /* endfor */

    if ( pIda->szColTextBuffer[0] != 0 )
    {
      ProofReadDrawText( pIda, hdc, prcClip, pPt, pIda->szColTextBuffer, iLen, iLineHeight, iBaseColorIndex, DrawType );
    } /* endif */
  //} /* endif */
}

void ProofReadDrawText( PPROOFREADDATA pIda, HDC hdc, PRECT prcClip, PPOINT pPt, const CHAR_W *pszString, int iLen, int iLineHeight, int iBaseColorIndex,  DRAWTYPE DrawType )
{
  SIZE size;
  RECT rc;

  do 
  {
    // check if text fits into available space
    GetTextExtentPoint32W( hdc, pszString, iLen, &size );
    rc.top = pPt->y;
    rc.bottom = pPt->y + size.cy;
    rc.left = pPt->x;
    rc.right = pPt->x + size.cx;

    // if there is not enough room truncate text until it fits
    if ( rc.right > prcClip->right )
    {
      int i = iLen - 1;
      BOOL fTextFits = FALSE;
      do
      {
        // find preceeding whitespace or DBCS char
        while ( (i > 0) && !ProofReadIsLineSplitChar( pIda, pszString[i] ) ) i--;

        // split text at found position and test if it fits
        if ( i != 0 ) 
        {
          GetTextExtentPoint32W( hdc, pszString, i, &size );
          if ( (pPt->x + size.cx) > prcClip->right )
          {
            // try a shorter string
            i--;
          }
          else
          {
            fTextFits = TRUE;
          }
        }
      } while ( (i != 0) && !fTextFits );

      if ( fTextFits )
      {
        // draw fitting part of text
        rc.top = pPt->y;
        rc.bottom = pPt->y + size.cy;
        rc.left = pPt->x;
        rc.right = pPt->x + size.cx;

        ProofReadSetTextAndBackgroundStyle( pIda, hdc, &rc, DrawType, iBaseColorIndex );
        ExtTextOutW( hdc, pPt->x, pPt->y, (DrawType == NORMALTEXT_DRAWTYPE ) ? ETO_CLIPPED : (ETO_CLIPPED | ETO_OPAQUE), &rc, pszString, i, NULL);
        //// apply strike through
        //if ( DrawType == CHANGED_DRAWTYPE )
        //{
        //  int iXPos = rc.top + ((rc.bottom - rc.top) / 2) + 1;
        //  MoveToEx( hdc, rc.left, iXPos, (LPPOINT) NULL); 
        //  LineTo( hdc, rc.right, iXPos );
        //} /* endif */

        // go to next line
        pPt->x = prcClip->left + 2;
        pPt->y += iLineHeight;

        // continue with remaining text
        pszString += i;
        iLen -= i;
      }
      else if ( size.cx < (prcClip->right - prcClip->left - 2) )
      {
        // draw text in next line
        // go to next line
        pPt->x = prcClip->left + 2;
        pPt->y += iLineHeight;
      }
      else
      {
        // draw text but clip it 
        rc.top = pPt->y;
        rc.bottom = pPt->y + size.cy;
        rc.left = pPt->x;
        rc.right = pPt->x + size.cx;
        if ( rc.right > prcClip->right )  rc.right = prcClip->right;
        ProofReadSetTextAndBackgroundStyle( pIda, hdc, &rc, DrawType, iBaseColorIndex );
        ExtTextOutW( hdc, pPt->x, pPt->y, (DrawType == NORMALTEXT_DRAWTYPE ) ? ETO_CLIPPED : (ETO_CLIPPED | ETO_OPAQUE), &rc, pszString, iLen, NULL);
        //// apply strike through
        //if ( DrawType == CHANGED_DRAWTYPE )
        //{
        //  int iXPos = rc.top + ((rc.bottom - rc.top) / 2) + 1;
        //  MoveToEx( hdc, rc.left, iXPos, (LPPOINT) NULL); 
        //  LineTo( hdc, rc.right, iXPos );
        //} /* endif */

        // go to next line
        pPt->x = prcClip->left + 2;
        pPt->y += iLineHeight;

        // continue with remaining text
        iLen = 0;
      }
    }
    else
    {
      // draw remaining text
      ProofReadSetTextAndBackgroundStyle( pIda, hdc, &rc, DrawType, iBaseColorIndex );
      ExtTextOutW( hdc, pPt->x, pPt->y, (DrawType == NORMALTEXT_DRAWTYPE ) ? ETO_CLIPPED : (ETO_CLIPPED | ETO_OPAQUE), &rc, pszString, iLen, NULL);
      //// apply strike through
      //if ( DrawType == CHANGED_DRAWTYPE )
      //{
      //  int iXPos = rc.top + ((rc.bottom - rc.top) / 2) + 1;
      //  MoveToEx( hdc, rc.left, iXPos, (LPPOINT) NULL); 
      //  LineTo( hdc, rc.right, iXPos );
      //} /* endif */
      pPt->x += size.cx;
      iLen = 0;
    } /* endif */
  } while ( iLen != 0 );
}

int ProofReadDrawItemColumn( PPROOFREADDATA pIda, HDC hdc, int iColumn, const CHAR_W *pszString, LPRECT prcClip, OtmProofReadEntry *pEntry, PPROOFREADCHANGE pChanges, int iFoundEntries, int TextColumn, int iBaseColorIndex )
{
  int iCurPos = 0; // current position within text
  int iLineHeight = 0;
  int iOffsDelta = 0;              // delta between old offset in result entry and new offset because of changes in the text


  // get text line height
  {
    TEXTMETRIC tm;
    GetTextMetrics( hdc, &tm );
    iLineHeight = tm.tmHeight + 4;
  }
  // get text length
  int iRemaining = wcslen( pszString );

  SetTextColor( hdc, pIda->ColorData.ColorSetting[iBaseColorIndex].cForeground );
  SetBkColor( hdc, pIda->ColorData.ColorSetting[iBaseColorIndex].cBackground );

  // erase rectangle
  {
    HBRUSH hBrush = CreateSolidBrush( pIda->ColorData.ColorSetting[iBaseColorIndex].cBackground );
    FillRect( hdc, prcClip, hBrush ); 
    DeleteObject( hBrush );
  }

  // draw grid lines
  ProofReadDrawFrame( hdc, prcClip );

  // start output at upper left corner and update reference point
  POINT pt;
  pt.x = prcClip->left + 2;
  pt.y = prcClip->top + 1; 
  SIZE size;

  switch( iColumn )
  {
    case 0:
      // draw checkbox
      break;

    case 1:
    case 2:
      {
        // draw document number or segment number
        GetTextExtentPoint32W( hdc, pszString, iRemaining, &size );
        int iPos = (prcClip->right - prcClip->left - size.cx - 4) / 2;
        ExtTextOutW( hdc, pt.x + iPos, pt.y, ETO_CLIPPED, prcClip, pszString, iRemaining, NULL);
      }
      break;

    default:
      // draw segment text
      iRemaining = wcslen( pszString );
      while ( iRemaining )
      {
        // outut of current text
        if ( pChanges == NULL )
        {
          // output of remaining text
          ProofReadDrawMultiLineText( pIda, hdc, prcClip, &pt, pszString + iCurPos, iRemaining, iLineHeight, iBaseColorIndex, NORMALTEXT_DRAWTYPE );
          iRemaining = 0;
        }
        else
        {
          // output text up to next position

          int iCurOffs = (int)pChanges->usOffs;

          int iOutputLen = iCurOffs - iCurPos;
          if ( iOutputLen != 0 )
          {
            ProofReadDrawMultiLineText( pIda, hdc, prcClip, &pt, pszString + iCurPos, iOutputLen, iLineHeight, iBaseColorIndex, NORMALTEXT_DRAWTYPE );
            iRemaining -= iOutputLen;
            iCurPos += iOutputLen;
          }

          // output of changes string
          switch ( pChanges->usType )
          {
          case MARK_INSERTED: 
              ProofReadDrawMultiLineText( pIda, hdc, prcClip, &pt, pszString + iCurPos, (int)pChanges->usLen, iLineHeight, iBaseColorIndex, INSERTED_DRAWTYPE );
              break;
            default:
              ProofReadDrawMultiLineText( pIda, hdc, prcClip, &pt, pszString + iCurPos, (int)pChanges->usLen, iLineHeight, iBaseColorIndex, CHANGED_DRAWTYPE );
              break;
          } /* endswitch */

          iRemaining -= (int)pChanges->usLen;
          iCurPos += (int)pChanges->usLen;

          // go to next change entry
          if ( pChanges != NULL )
          {
            pChanges++;
            if ( pChanges->usLen == 0 )
            {
              pChanges = NULL; // reached end of list
            }
          }
        }
      }
      break;
  } /* endswitch */
  return ( pt.y + iLineHeight - prcClip->top );
}

// draw a frame
int  ProofReadDrawFrame( HDC hdc, LPRECT prc )
{
  MoveToEx( hdc, prc->left, prc->top, NULL );
  LineTo( hdc, prc->right, prc->top );
  LineTo( hdc, prc->right, prc->bottom );
  LineTo( hdc, prc->left, prc->bottom );
  LineTo( hdc, prc->left, prc->top );
  return( 0 );
}

void ProofReadUpdateSelectionStatus( PPROOFREADDATA pIda )
{
  int iItems = QUERYITEMCOUNTHWND( pIda->hResultListBox );
  int iSelected = QUERYSELECTIONHWND( pIda->hResultListBox );
  char szString[40];
  if ( iSelected != -1 )
  {
    sprintf( szString, "item %ld of %ld selected", iSelected + 1, iItems );
  }
  else
  {
    sprintf( szString, "no item of %ld selected", iItems );
  } /* endif */
  SendMessage( pIda->hStatus, SB_SETTEXT, 0, (LPARAM)szString);
}

// build complete text string for given column in column text buffer of pIda
int ProofReadBuildFullColumnText( PPROOFREADDATA pIda, OtmProofReadEntry *pEntry, int iTextColumn )
{
  pIda->szColTextBuffer[0] = 0;

  switch( iTextColumn )
  {
    case 0: 
      wcscpy( pIda->szColTextBuffer, pEntry->getSource() ); 
      break;
    case 1: 
      wcscpy( pIda->szColTextBuffer, pEntry->getTarget() ); 
      break;
    case 2: 
      if ( wcslen( pEntry->getNewTarget() ) == 0 )
      {
        wcscpy( pIda->szColTextBuffer, pEntry->getModTarget() ); 
      }
      else
      {
        wcscpy( pIda->szColTextBuffer, pEntry->getNewTarget() ); 
      } /* endif */
      break;
  } /* endswitch */

  return( wcslen( pIda->szColTextBuffer ) );
}

// (re)fill table containing line break offsets within given text and provided rectangle
int ProofReadGetTextHeight( PPROOFREADDATA  pIda, HWND hwndControl, HDC hdc, OtmProofReadEntry *pEntry, int iTextColumn )
{
  TEXTMETRIC tm;
  GetTextMetrics( hdc, &tm );
  int iLineHeight = tm.tmHeight + 2;

  // get complete text to be displayed
  int iLength = ProofReadBuildFullColumnText( pIda, pEntry, iTextColumn );

  // replace any LF with blank if we are displaying flow text
  //if ( !pIda->fRespectLineFeeds )
  //{
    for ( int i = 0; i < iLength; i++ )
    {
      if ( pIda->szColTextBuffer[i] == L'\n' ) pIda->szColTextBuffer[i] = L' ';
    }
  //} /* endif */

  // get height of text using the DrawText function
  PSZ_W pszText = pIda->szColTextBuffer;
  RECT rc;
  rc.top = 0;
  rc.left = 0;
  rc.right = pIda->aiListViewColWidth[iTextColumn+3];
  rc.bottom = iLineHeight;
  int iRes = DrawTextW( hdc, pszText, wcslen(pszText), &rc, DT_WORDBREAK | DT_CALCRECT );
  return( rc.bottom + 4 );
}

int ProofReadMeasureItem( PPROOFREADDATA  pIda, LPMEASUREITEMSTRUCT pMeasureItem )
{
  HDC hdc = GetDC( pIda->hResultListBox );
  HFONT hFontOld = NULL;

  if ( pIda->hFontControl ) hFontOld = (HFONT)SelectObject( hdc, pIda->hFontControl );

  TEXTMETRIC tm;
  GetTextMetrics( hdc, &tm );
  OtmProofReadEntry *pEntry = (OtmProofReadEntry *)pMeasureItem->itemData;
  if ( pEntry == NULL ) return( 0 );
  int iMaxHeight = 0;
  //if ( pIda->CurDisplayMode == SOURCE_AND_TARGET_COLUMN_MODE )
  //{
    int iSourceHeight = ProofReadGetTextHeight( pIda, pIda->hResultListBox, hdc, pEntry, 0 );
    int iTargetHeight = ProofReadGetTextHeight( pIda, pIda->hResultListBox, hdc, pEntry, 1 );
    int iModTargetHeight = ProofReadGetTextHeight( pIda, pIda->hResultListBox, hdc, pEntry, 2 );
    iMaxHeight = max( iSourceHeight, iTargetHeight );
    iMaxHeight = max( iMaxHeight, iModTargetHeight );
  //}
  //else if ( pIda->CurDisplayMode == ONLY_SOURCE_COLUMN_MODE )
  //{    iMaxHeight = ProofReadGetTextHeight( pIda, pIda->hResultListBox, hdc, pEntry, FALSE );
  //}
  //else
  //{
  //  iMaxHeight = ProofReadGetTextHeight( pIda, pIda->hResultListBox, hdc, pEntry, TRUE );
  //} /* endif */
  if ( hFontOld ) SelectObject( hdc, hFontOld );
  ReleaseDC( NULL, hdc );
  pMeasureItem->itemHeight = iMaxHeight + 4;
  pEntry->setItemHeight( pMeasureItem->itemHeight );

  return( 0 );
}

// force a refresh of the height of the listbox items
int ProofReadForceRefreshOfItemHeights( PPROOFREADDATA  pIda )
{
  // use a brute force approach: delete all items, refill the listbox, restore current top item and current selected item and allow redraw...
  SendMessage( pIda->hResultListBox, WM_SETREDRAW, FALSE, 0 );
  int iSelItem = SendMessage( pIda->hResultListBox, LB_GETCURSEL, 0, 0 );
  int iTopIndex = SendMessage( pIda->hResultListBox, LB_GETTOPINDEX, 0, 0 );

  ProofReadRefreshListbox( pIda );

  if ( iTopIndex != -1 ) SendMessage( pIda->hResultListBox, LB_SETTOPINDEX, iTopIndex, 0 );
  if ( iSelItem != -1 ) SendMessage( pIda->hResultListBox, LB_SETCURSEL, iSelItem, 0 );
  SendMessage( pIda->hResultListBox, WM_SETREDRAW, TRUE, 0 );
  InvalidateRect( pIda->hResultListBox, NULL, TRUE );
  return( 0 );
} 

BOOL ProofReadOpenDoc( PPROOFREADDATA pIda )
{
  BOOL fOK = TRUE;
  POPENANDPOS pOpen = NULL;
  OtmProofReadEntry *pEntry = NULL;

  int iSelItem = QUERYSELECTIONHWND( pIda->hResultListBox );
  if ( iSelItem == -1 )
  {
    return( FALSE );
  }

  // get selected entry
  if ( fOK )
  {
    pEntry = (OtmProofReadEntry *)QUERYITEMHANDLEHWND( pIda->hResultListBox, iSelItem );
    if ( pEntry == NULL ) fOK = FALSE;
  } /* endif */
  
  // build folder object name
  if ( fOK )
  {
    OtmProofReadEntry *pEntry = (OtmProofReadEntry *)QUERYITEMHANDLEHWND( pIda->hResultListBox, iSelItem );
    strcpy( pIda->szFolLongName, pIda->pList->getFolder(0).c_str() );
    BOOL fIsNew = !SubFolNameToObjectName( pIda->szFolLongName, pIda->szFolObjName );
    if ( fIsNew )
    {
      fOK = FALSE;
      PSZ pszParm = pIda->szFolLongName;
      UtlError( ERROR_XLATE_FOLDER_NOT_EXIST, MB_CANCEL, 1, &pszParm, EQF_ERROR );
    } /* endif */
  } /* endif */

  // build document object name
  if ( fOK )
  {
    BOOL fIsNew = FALSE;
    int iDocNum = pEntry->getDocumentNumber();
    strcpy( pIda->szDocLongName, pIda->pList->getDocument( iDocNum ).c_str() );
    FolLongToShortDocName( pIda->szFolObjName, pIda->szDocLongName, pIda->szDocShortName, &fIsNew );
    if ( fIsNew )
    {
      fOK = FALSE;
      sprintf( (PSZ)pIda->szBuffer, "Document %s does not exist in folder %2 or cannot be accessed", pIda->szDocLongName, pIda->szFolLongName );
      MessageBox( pIda->hwnd, (PSZ)pIda->szBuffer, "OpenTM2 Error", MB_OK );
    }
    else
    {
      strcpy( pIda->szDocObjName, pIda->szFolObjName );
      strcat( pIda->szDocObjName, BACKSLASH_STR );
      strcat( pIda->szDocObjName, pIda->szDocShortName );
    } /* endif */
  } /* endif */

  // send open request
  if ( fOK )
  {
    fOK = UtlAlloc( (PVOID *)&pOpen, 0L, (LONG)sizeof(OPENANDPOS), ERROR_STORAGE );
    if ( fOK )
    {
      // Setup open-and-pos structure and pass request to document handler 
      strcpy( pOpen->szDocName, pIda->szDocObjName );
      pOpen->ulSeg = pEntry->getSegmentNumber();
      pOpen->usLen = 0;
      pOpen->chFind[0] = 0;
      pOpen->usOffs= 0;
      EqfPost2Handler( DOCUMENTHANDLER, WM_EQF_PROCESSTASK, MP1FROMSHORT(OPEN_AND_POSITION_TASK), MP2FROMP(pOpen) );
    }
  } /* endif */
} /* end of ProofReadOpenDoc */

void ProofReadRefreshListbox( PPROOFREADDATA pIda )
{
  SendMessage( pIda->hResultListBox, LB_RESETCONTENT, 0, 0 );

  // fill proof read entries listbox
  if ( pIda->pList != NULL )
  {
    int iEntries = pIda->pList->size();
    for( int i = 0; i < iEntries; i++ )
    {
      OtmProofReadEntry *pEntry = (*pIda->pList)[i];
      pEntry->setItemHeight( 0 );
      const CHAR_W *pszTarget = pEntry->getTarget(); 
      const CHAR_W *pszModTarget = pEntry->getModTarget(); 
      BOOL fModified = !pEntry->isUnChanged();
      BOOL fShow = FALSE;
      BOOL fProcessed = pEntry->getProcessed();
      if ( fProcessed && pIda->fShowProcessed ) fShow = TRUE;
      if ( !fProcessed && fModified && pIda->fShowModified ) fShow = TRUE;
      if ( !fProcessed && !fModified && pIda->fShowUnchanged ) fShow = TRUE;
      if ( fShow )
      {
        INSERTITEMENDHWND( pIda->hResultListBox, pEntry );
      } /* endif */
    } /* endfor */
  } /* endif */
}

// test if on-spot editing in the batch list is active
BOOL ProofReadIsOnSpotEditingActive()
{
  return( hEdit != NULL );
}

// end on-spot editing of a batch list entry
void ProofReadEndOnSpotEditing()
{
  HWND hwndTemp = hEdit;
  hEdit = NULL;
  DestroyWindow( hwndTemp );
}

// result listbox subclass procedure
long _stdcall ProofReadListboxProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message){
		case WM_KEYDOWN:
      if ( wParam == VK_ESC )
		  {
        if ( ProofReadIsOnSpotEditingActive() )
        {
          ProofReadEndOnSpotEditing();
        } /* endif */
      }
      else if ( wParam == VK_F2 )
		  {
        // get selected item
        int iItem = SendMessage( hwnd, LB_GETCURSEL, 0, 0);

        // start edit mode 
        if ( iItem >= 0 )
        {
          PPROOFREADDATA pIda = ACCESSDLGIDA( hwndDialog, PPROOFREADDATA );
          ProofReadEditCell( pIda, hwnd, iItem );
        } /* endif */
        return( 0 );
      }
      else
      {
        return( CallWindowProc(ListboxOldProc, hwnd, message, wParam, lParam) );
      } /* endif */
      break;

		case WM_CHAR:
		  {
        UCHAR ucCode = (UCHAR) LOWORD( wParam );
        if ( ucCode == ' ' )
        {
          // get selected item
          int iItem = SendMessage( hwnd, LB_GETCURSEL, 0, 0);

          // toggle selection checkbox
          if ( iItem >= 0 )
          {
            PPROOFREADDATA pIda = ACCESSDLGIDA( hwndDialog, PPROOFREADDATA );
            ProofReadToggleCheckbox( pIda, hwnd, iItem );
          } /* endif */
        }
        else
        {
          CallWindowProc(ListboxOldProc, hwnd, message, wParam, lParam);
        } /* endif */
      }
      return( 0 );
      break;

		case WM_LBUTTONDOWN:
		  {
        CallWindowProc(ListboxOldProc, hwnd, message, wParam, lParam);

        long x, y;
			  x = (long)LOWORD(lParam);
			  y = (long)HIWORD(lParam);
        POINT p = { x, y };
        ClientToScreen( hwnd, &p ); 
        int iItem = LBItemFromPt( hwnd, p, 0);

			  if ( iItem != -1)
        {
          PPROOFREADDATA pIda = ACCESSDLGIDA( hwndDialog, PPROOFREADDATA );
          RECT rectItem;
          SendMessage( hwnd, LB_GETITEMRECT, (WPARAM)iItem, (LPARAM)&rectItem );
          y -= rectItem.top;

          // user drawn checkbox coordinates in pixel
          long xStart = CHECKBOX_XPOS;
          long xStop = xStart + CHECKBOX_XSIZE;
          long yStart = CHECKBOX_YPOS;
          long yStop = yStart + CHECKBOX_YSIZE;

          // verify that the double-click occured in the modified target column
          if ( (x >= xStart) && (x <= xStop) && (y >= yStart) && (y <= yStop) )
          {
            ProofReadToggleCheckbox( pIda, hwnd, iItem );
          } /* endif */
			  } /* endif */
      }
			return 0;
			break;

		case WM_LBUTTONDBLCLK:
		{
      CallWindowProc(ListboxOldProc, hwnd, message, wParam, lParam);
			if (hEdit != NULL){ SendMessageW( hEdit, WM_KILLFOCUS, 0, 0 );};

			long x, y;
			x = (long)LOWORD(lParam);
			y = (long)HIWORD(lParam);
      POINT p = { x, y };
      ClientToScreen( hwnd, &p ); 
      int iItem = LBItemFromPt( hwnd, p, 0);

			if ( iItem != -1)
      {
        PPROOFREADDATA pIda = ACCESSDLGIDA( hwndDialog, PPROOFREADDATA );

        // compute start and stop location of the editable area (modified target column) in the listbox item
        long xStart = 0;
        for( int i = 0; i < 5; i++ ) xStart += pIda->aiListViewColWidth[i];
        long xStop = xStart + pIda->aiListViewColWidth[5];

        // verify that the double-click occured in the modified target column
        if ( (x >= xStart) && (x <= xStop) )
        {
          ProofReadEditCell( pIda, hwnd, iItem );
        } /* endif */
			}
			return 0;
			break;
		}
	}
	return CallWindowProc(ListboxOldProc, hwnd, message, wParam, lParam);
}

// window procedure for subclassed edit control used for on-spot editing
long _stdcall ProofReadEditProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
  {
    case WM_KEYDOWN:
      if ( wParam == VK_ESC )
      {
        if ( hEdit != NULL ) 
        {
          hEdit = NULL;
          PostMessage( hwndDialog, WM_EQF_PROCESSTASK, WPARAM(SETFOCUS_TASK), LPARAM(ID_PROOFREAD_TABLE_LV) );
  			  DestroyWindow(hwnd);
        }
        return( TRUE );
      } /* endif */
      else if ( wParam == VK_ENTER )
      {
        if ( hEdit != NULL ) 
        {
          PPROOFREADDATA pIda = ACCESSDLGIDA( hwndDialog, PPROOFREADDATA );
          SendMessageW( hwnd, WM_GETTEXT, sizeof(szEditText) / sizeof(CHAR_W), (LPARAM)szEditText );
          OtmProofReadEntry *pEntry = (OtmProofReadEntry *)QUERYITEMHANDLEHWND( pIda->hResultListBox, iEditListboxItem );
          pEntry->setNewTarget( szEditText );
          ProofReadFindDifferences( pIda, pEntry );
          hEdit = NULL;
          PostMessage( hwndDialog, WM_EQF_PROCESSTASK, WPARAM(REFRESHITEM_TASK), LPARAM(iEditListboxItem) );
          PostMessage( hwndDialog, WM_EQF_PROCESSTASK, WPARAM(SETFOCUS_TASK), LPARAM(ID_PROOFREAD_TABLE_LV) );
  			  DestroyWindow(hwnd);
        }
         return( TRUE );
      } /* endif */
      break;

		case WM_KILLFOCUS:
		{
      if ( hEdit != NULL ) // get current text only when the edit control is available
      {
        PPROOFREADDATA pIda = ACCESSDLGIDA( hwndDialog, PPROOFREADDATA );
        SendMessageW( hwnd, WM_GETTEXT, sizeof(szEditText) / sizeof(CHAR_W), (LPARAM)szEditText );
        OtmProofReadEntry *pEntry = (OtmProofReadEntry *)QUERYITEMHANDLEHWND( pIda->hResultListBox, iEditListboxItem );
        pEntry->setNewTarget( szEditText );
        ProofReadFindDifferences( pIda, pEntry );
        hEdit = NULL;
        PostMessage( hwndDialog, WM_EQF_PROCESSTASK, WPARAM(REFRESHITEM_TASK), LPARAM(iEditListboxItem) );
  			DestroyWindow(hwnd);
      }
			break;
		}
	}

	return CallWindowProcW(EditOldProc, hwnd, message, wParam, lParam);
}


/*! \brief Toggles the checkbox of a listbox item and changes the selected state of the associated OtmProofReadEntry 
  \param hwndListbox handle of the proof read entry listbox
  \param iItem index of the listbox item
  \returns new state of checkbox
*/
BOOL ProofReadToggleCheckbox( PPROOFREADDATA pIda, HWND hwndListbox, int iItem )
{
  OtmProofReadEntry *pEntry = (OtmProofReadEntry *)QUERYITEMHANDLEHWND( hwndListbox, iItem);
  if ( !pEntry->isUnChanged() )
  {
    BOOL fSelected = !pEntry->getSelected();
    pEntry->setSelected( fSelected );
    RECT rectItem;
    SendMessage( hwndListbox, LB_GETITEMRECT, (WPARAM)iItem, (LPARAM)&rectItem );
    InvalidateRect( hwndListbox, &rectItem, TRUE );
    ProofReadSetProcessPB( pIda );
    return( fSelected );
  } /* endif */
  return( FALSE);
}

/*! \brief Enables or disables the process pushbutton depending on the active options and the number of selected entries
  \param pIda pointer to dialog's instance data area
*/
void ProofReadSetProcessPB( PPROOFREADDATA pIda )
{
  int iSelected = pIda->pList->getNumOfSelected();
  BOOL fEnable = (iSelected != 0) && (pIda->fProcessDoc || pIda->fProcessMem);
  ENABLECTRL( pIda->hwnd, ID_PROOFIMP_AUTOUPDATE_PB, fEnable );
}

/*! \brief Select or deselect all changed entries in the list
  \param pIda pointer to dialog's instance data area
  \param fSelect new selection flag for the entries
*/
void ProofReadSelectAll( PPROOFREADDATA pIda, BOOL fSelect )
{
  int iEntries = pIda->pList->size();
  for( int i = 0; i < iEntries; i++ )
  {
    OtmProofReadEntry *pEntry = (*pIda->pList)[i];
    if ( !pEntry->isUnChanged() )
    {
      pEntry->setSelected( fSelect );
    } /* endif */
  } /* endfor */
  ProofReadSetProcessPB( pIda );
  InvalidateRect( pIda->hResultListBox, NULL, TRUE );
}

/*! \brief Get the last used save/load directory
  \param pIda pointer to dialog's instance data area
  \param pszLastUsedDir pointer to buffer for the last used save/load directory
  \returns TRUE when the last used save/load directory has been filled
*/
BOOL ProofGetSaveDir( PPROOFREADDATA pIda, PSZ pszLastUsedDir )
{
  if ( pIda->pLastUsed && (pIda->pLastUsed->szSaveDir[0] != 0) )
  {
    strcpy( pszLastUsedDir, pIda->pLastUsed->szSaveDir );
  }
  else
  {
    UtlMakeEQFPath( pszLastUsedDir, NULC, SYSTEM_PATH, NULL );
    strcat( pszLastUsedDir, "\\ProofReadLists" );
    UtlMkMultDir( pszLastUsedDir, FALSE );
  } /* endif */
  return( TRUE );
}

/*! \brief Save the current list of proof read entries to the selected file
  \param pIda pointer to dialog's instance data area
  \returns TRUE when list has been saved successfully
*/
BOOL ProofReadSaveList( PPROOFREADDATA pIda )
{
  BOOL fOK = TRUE;

  // call standard save as dialog
  OPENFILENAME OpenFileName;
  memset( &OpenFileName, 0, sizeof(OpenFileName) );
  OpenFileName.lStructSize        = sizeof(OpenFileName);
  OpenFileName.hwndOwner          = pIda->hwnd;
  OpenFileName.hInstance          = NULLHANDLE;
  OpenFileName.lpstrFilter        = PROOFREADLIST_FORMAT_FILTERS;
  OpenFileName.lpstrCustomFilter  = NULL;
  OpenFileName.nMaxCustFilter     = 0;
  OpenFileName.nFilterIndex       = 0;
  ProofGetSaveDir( pIda, pIda->szListFileDir );
  if ( pIda->szListFileName[0] == 0 )
  {
    strcpy( pIda->szListFileName, pIda->szListFileDir );
    strcat( pIda->szListFileName, "\\" );
    strcat( pIda->szListFileName, pIda->pList->getFolder( 0 ).c_str() );
    strcat( pIda->szListFileName, "." );
    strcat( pIda->szListFileName, EXT_OF_PROOFREADLIST );
  } /* endif */
  OpenFileName.lpstrFile          = pIda->szListFileName;
  OpenFileName.nMaxFile           = sizeof(pIda->szListFileName);
  OpenFileName.lpstrFileTitle     = NULL;
  OpenFileName.nMaxFileTitle      = 0;
  OpenFileName.lpstrInitialDir    = pIda->szListFileName ;
  OpenFileName.lpstrTitle         = "Save current validation list";
  OpenFileName.Flags              = OFN_ENABLESIZING | OFN_EXPLORER | OFN_LONGNAMES | OFN_NODEREFERENCELINKS | OFN_NOTESTFILECREATE | OFN_NOCHANGEDIR | OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;
  OpenFileName.nFileOffset        = 0;
  OpenFileName.nFileExtension     = 0;
  OpenFileName.lpstrDefExt        = EXT_OF_PROOFREADLIST;
  OpenFileName.lCustData          = 0L;
  OpenFileName.lpfnHook           = NULL;
  OpenFileName.lpTemplateName     = NULL;

  fOK = GetSaveFileName( &OpenFileName ) != 0;
  if ( fOK )
  {
    fOK = pIda->pList->save( pIda->szListFileName, pIda->hwnd );
  } /* endif */

  return( fOK );
}

/*! \brief Load a list of proof read entries from the selected file
  \param pIda pointer to dialog's instance data area
  \returns TRUE when list has been loaded successfully
*/
BOOL ProofReadLoadList( PPROOFREADDATA pIda )
{
  BOOL fOK = TRUE;
  OPENFILENAME OpenFileName;

  memset( &OpenFileName, 0, sizeof(OpenFileName) );
  OpenFileName.lStructSize = sizeof(OpenFileName);
  OpenFileName.hwndOwner = pIda->hwnd;
  ProofGetSaveDir( pIda, pIda->szListFileDir );
  OpenFileName.lpstrInitialDir = pIda->szListFileDir ;
  OpenFileName.lpstrFilter        = PROOFREADLIST_FORMAT_FILTERS;
  OpenFileName.lpstrCustomFilter = NULL;
  OpenFileName.lpstrFileTitle = NULL;
  OpenFileName.nMaxFileTitle = 0;
  OpenFileName.lpstrTitle = "Load Validation List";
  OpenFileName.lpfnHook = NULL;
  OpenFileName.lCustData = NULL;
  OpenFileName.Flags = OFN_FILEMUSTEXIST | OFN_LONGNAMES | OFN_ALLOWMULTISELECT | OFN_ENABLEHOOK | OFN_EXPLORER | OFN_NOCHANGEDIR | OFN_PATHMUSTEXIST;
  OpenFileName.lpstrFile = pIda->szListFileName;
  OpenFileName.nMaxFile = sizeof(pIda->szListFileName);
  fOK = GetOpenFileName( &OpenFileName );
  if ( fOK )
  {
    pIda->pList->clear();
    SendMessage( pIda->hResultListBox, LB_RESETCONTENT, 0, 0 );
    fOK = (pIda->pList->load( pIda->szListFileName, pIda->hwnd ) == 0 );
    ProofReadRefreshListbox( pIda );
  } /* endif */

  return( fOK );
}

/*! \brief Copy the tokens of a fuzzy token list to a start/stop list
  \param pIda pointer to dialog's instance data area
  \param pToken pointer to a fuzzy token list
  \param pChanges pointer to a list of PROOFREADCHANGE entries
  \param pModToken pointer to a fuzzy token list for the modified text
  \returns number of entries added to the list
*/
int ProofReadCopyToChangesList( PPROOFREADDATA pIda, PFUZZYTOK pToken, PPROOFREADCHANGE pChanges, PFUZZYTOK pModToken )
{
  int iEntries = 0;                       
  USHORT usLastType = 0;

  // process tokens of source list
  while ( pToken->ulHash && (iEntries < (MAX_PROOFREADCHANGES - 1) ) ) 
  {
    if ( pToken->sType == MARK_EQUAL) 
    {
      // look for whitespace changes
      USHORT usLen = pToken->usStop - pToken->usStart + 1;
      USHORT usModLen = pModToken->usStop - pModToken->usStart + 1;
      if ( (usLen != usModLen) || (memcmp( pToken->pData, pModToken->pData, usLen ) != 0) )
      {
        // skip equal part of the token
        USHORT usPos = 1;
        while ( (usPos < usLen) && (usPos < usModLen) && (pToken->pData[usPos] == pModToken->pData[usPos]) ) usPos++;

        // add change entry for whitespace difference
        if ( usPos < usLen )
        {
          pChanges[iEntries].usOffs = pToken->usStart + usPos;
          pChanges[iEntries].usLen  = usLen - usPos;
          pChanges[iEntries].usType = usLastType = (USHORT)MARK_MODIFIED;
          iEntries++;
        } /* endif */
      }
      else
      {
        // no differences found, reset current token type
        usLastType = 0;
      } /* endif */
    }
    else if ( (pToken->sType == MARK_INSERTED) || (pToken->sType == MARK_MODIFIED) ) 
    {
      if ( (iEntries != 0) && ((USHORT)pToken->sType == usLastType) )
      {
        // update end position of previous entry
        pChanges[iEntries-1].usLen = pToken->usStop - pChanges[iEntries-1].usOffs + 1;
      }
      else
      {
        // add new entry
        pChanges[iEntries].usOffs = pToken->usStart;
        pChanges[iEntries].usLen  = pToken->usStop - pToken->usStart + 1;
        pChanges[iEntries].usType = usLastType = (USHORT)pToken->sType;
        iEntries++;
      } /* endif */       
    }
    else
    {
      // reset current token type
      usLastType = 0;
    } /* endif */
    pToken++;
    pModToken++;
  }

  // terminate start stop list
  pChanges[iEntries].usOffs = 0;
  pChanges[iEntries].usLen = 0;
  pChanges[iEntries].usType = 0;
  iEntries++;

  return( iEntries );
}

/*! \brief Copy the tokens of a fuzzy token list to a start/stop list
  \param pIda pointer to dialog's instance data area
  \param pToken pointer to a fuzzy token list
  \param pChanges pointer to a list of PROOFREADCHANGE entries
  \returns number of entries added to the list
*/
void ProofReadGetLangInfo( PPROOFREADDATA pIda, const char *pszLanguage )
{
  if ( strcmp( pIda->szCurLanguage, pszLanguage ) != 0 )
  {
    MorphGetLanguageID( (PSZ)pszLanguage, &(pIda->sLangID) );
    pIda->ulOEMCP = GetLangOEMCP( (PSZ)pszLanguage );
  } /* endif */
}

/*! \brief Find differences between original target and modified target (or new target)
  \param pIda pointer to dialog's instance data area
  \returns TRUE when list has been loaded successfully
*/
BOOL ProofReadFindDifferences( PPROOFREADDATA pIda, OtmProofReadEntry *pEntry )
{
  PFUZZYTOK    pFuzzyTgt = NULL;
  PFUZZYTOK    pFuzzyTok = NULL;       // returned token list
  USHORT       usDiff = 0;             // number of differences
  USHORT       usWords = 0;            // number of words/tokens
  BOOL         fOK = TRUE; 

  // special mode for deleted target strings
  if ( pEntry->getModTargetDeletedFlag() )
  {
    PSZ_W pszTarget = (PSZ_W)pEntry->getTarget();

    PPROOFREADCHANGE pTargetChanges = new( PROOFREADCHANGE[MAX_PROOFREADCHANGES] );
    PPROOFREADCHANGE pModTargetChanges = new( PROOFREADCHANGE[MAX_PROOFREADCHANGES] );

    // add deleted/inserted entry
    pModTargetChanges[0].usOffs = pTargetChanges[0].usOffs = 0;
    pModTargetChanges[0].usLen  = pTargetChanges[0].usLen = wcslen(pszTarget);
    pModTargetChanges[0].usType = MARK_INSERTED;
    pTargetChanges[0].usType = MARK_DELETED;

    // terminate lists
    pModTargetChanges[1].usOffs = pTargetChanges[1].usOffs = 0;
    pModTargetChanges[1].usLen  = pTargetChanges[1].usLen = 0;
    pTargetChanges[1].usType = pModTargetChanges[1].usType = 0;

    PPROOFREADCHANGE pOldTargetChanges = (PPROOFREADCHANGE)(pEntry->getTargetChangeList() );  
    PPROOFREADCHANGE pOldModTargetChanges = (PPROOFREADCHANGE)(pEntry->getModTargetChangeList() );  
    pEntry->setTargetChangeList( pTargetChanges );
    pEntry->setModTargetChangeList( pModTargetChanges );
    if ( pOldTargetChanges ) delete( pOldTargetChanges  );
    if ( pOldModTargetChanges ) delete( pOldModTargetChanges  );
  }

  // fast exit if one or both strings are empty...
  PSZ_W pszModTarget = (PSZ_W)pEntry->getNewTarget();
  if ( *pszModTarget == EOS) pszModTarget = (PSZ_W)pEntry->getModTarget();
  PSZ_W pszTarget = (PSZ_W)pEntry->getTarget();
  if ( (*pszTarget == EOS) || (*pszModTarget == EOS) )
  {
    return( -1 );
  } /*   endif */

  // ensure that the correct tag table is loaded
  int iDocument = pEntry->getDocumentNumber();
  ProofReadLoadTagTable( pIda, pIda->pList->getDocMarkup( iDocument ).c_str() );
  ProofReadGetLangInfo( pIda, pIda->pList->getDocTargetLang( iDocument ).c_str());

  // call function to evaluate the differences
  if ( fOK )
  {
    fOK = EQFBFindDiffEx( pIda->pTagTable, pIda->bDiffInputBufer, pIda->bDiffTokenBuffer, pszTarget, pszModTarget, pIda->sLangID, (PVOID *)&pFuzzyTok, (PVOID *)&pFuzzyTgt, pIda->ulOEMCP );
  } /* endif */

  if ( fOK )
  {
    PPROOFREADCHANGE pTargetChanges = new( PROOFREADCHANGE[MAX_PROOFREADCHANGES] );
    PPROOFREADCHANGE pModTargetChanges = new( PROOFREADCHANGE[MAX_PROOFREADCHANGES] );

    int iTargetEntries = ProofReadCopyToChangesList( pIda, pFuzzyTgt, pTargetChanges, pFuzzyTok );
    int iModTargetEntries = ProofReadCopyToChangesList( pIda, pFuzzyTok, pModTargetChanges, pFuzzyTgt );

    PPROOFREADCHANGE pOldTargetChanges = (PPROOFREADCHANGE)(pEntry->getTargetChangeList() );  
    PPROOFREADCHANGE pOldModTargetChanges = (PPROOFREADCHANGE)(pEntry->getModTargetChangeList() );  
    if ( iTargetEntries <= 1 )
    {
      delete( pTargetChanges );
      pTargetChanges = NULL;
    } /* endif */
    if ( iModTargetEntries <= 1 )
    {
      delete( pModTargetChanges );
      pModTargetChanges = NULL;
    } /* endif */
    pEntry->setTargetChangeList( pTargetChanges );
    pEntry->setModTargetChangeList( pModTargetChanges );
    if ( pOldTargetChanges ) delete( pOldTargetChanges  );
    if ( pOldModTargetChanges ) delete( pOldModTargetChanges  );

    if ( pFuzzyTgt ) UtlAlloc( (PVOID *)&pFuzzyTgt, 0L, 0L, NOMSG );
    if ( pFuzzyTok ) UtlAlloc( (PVOID *)&pFuzzyTok, 0L, 0L, NOMSG );
  } /* endif */

  return  ( fOK );
}

/*! \brief Free a previously loaded tag table
  \param pIda pointer to dialog's instance data area
  \returns TRUE when successful
*/
BOOL ProofReadFreeTagTable( PPROOFREADDATA pIda )
{
  BOOL fOK = TRUE;

  if ( pIda->pTagTable ) TAFreeTagTable( pIda->pTagTable );
  pIda->pTagTable = NULL;
  memset( pIda->szCurTagTable, 0, sizeof(pIda->szCurTagTable) );

  return( fOK );
}

/*! \brief Load a tag table into memory
  \param pIda pointer to dialog's instance data area
  \param pszTagTable tag table / markup name
  \returns TRUE when successful
*/
BOOL ProofReadLoadTagTable( PPROOFREADDATA pIda, const char *pszTagTable )
{
  BOOL fOK = TRUE;

  if ( strcmp( pIda->szCurTagTable, pszTagTable ) != 0 )
  {
    // free any loaded tag table 
    ProofReadFreeTagTable( pIda );

    // load the markup table
    if ( fOK )
    {
      SHORT  sRc;

      sRc = TALoadTagTableExHwnd( (PSZ)pszTagTable, &pIda->pTagTable, FALSE, TALOADPROTTABLEFUNC , TRUE, pIda->hwnd ); 
      fOK = (sRc == NO_ERROR);
      if ( fOK ) strcpy( pIda->szCurTagTable, pszTagTable );
    } /* endif */
  }
  return( fOK );
}

/*! \brief Find differences between target and modified target for the specified list entries
  \param pIda pointer to dialog's instance data area
  \param iStartIndex index of the first entry to be processed
  \param iEndIndex index of the last entry to be processed
  \returns TRUE when successful
*/
BOOL ProofReadMakeDiffLists( PPROOFREADDATA pIda, int iStartIndex, int iEndIndex )
{
  BOOL fOK = TRUE;

  size_t i = iStartIndex;
  while( i <= iEndIndex )
  {
    fOK = ProofReadFindDifferences( pIda, (*pIda->pList)[i] );
    i++;
  } /* endif */

  return( fOK );
}

/*! \brief (Re)Fill document listbox
  \param pIda pointer to dialog's instance data area
  \returns TRUE when successful
*/
BOOL ProofReadFillDocumentList( PPROOFREADDATA pIda )
{
  BOOL fOK = TRUE;

  DELETEALL( pIda->hwnd, ID_PROOFIMP_DOCUMENTS_LB );

  if ( pIda->pList != NULL )
  {
    int iNumCurFolder = -1;
    int iNumCurDoc = -1;
    int iEntries = pIda->pList->size();
    for( int i = 0; i < iEntries; i++ )
    {
      OtmProofReadEntry *pEntry = (*pIda->pList)[i];
      int iFolder = pEntry->getFolderNumber();
      if ( iNumCurFolder != iFolder )
      {
        if ( iFolder != 0 )
        {
          INSERTITEMEND( pIda->hwnd, ID_PROOFIMP_DOCUMENTS_LB, "" );
        }
        sprintf( (PSZ)pIda->szBuffer, "Folder: %s", pIda->pList->getFolder( iFolder ).c_str() );
        INSERTITEMEND( pIda->hwnd, ID_PROOFIMP_DOCUMENTS_LB, (PSZ)pIda->szBuffer );
        iNumCurFolder = iFolder;
      } /* endif */
      int iDoc = pEntry->getDocumentNumber();
      if ( iNumCurDoc != iDoc )
      {
        sprintf( (PSZ)pIda->szBuffer, "[%3.3ld] %s", iDoc + 1, pIda->pList->getDocument( iDoc ).c_str() );
        INSERTITEMEND( pIda->hwnd, ID_PROOFIMP_DOCUMENTS_LB, (PSZ)pIda->szBuffer );
        iNumCurDoc = iDoc;
      } /* endif */
    } /* endfor */
  } /* endif */

  return( fOK );
}

/*! \brief Import one or more validations documents
  \param pIda pointer to dialog's instance data area
  \returns TRUE when successful
*/
BOOL ProofReadImport( PPROOFREADDATA pIda )
{
  BOOL fOK = TRUE;

  std::string strFilterDLL, strInputFile, strProofReadXML;
  std::vector<std::string> vInputFiles;
  std::vector<std::string> *pvXMLFiles = new std::vector<std::string>;
  std::vector<OtmProofReadImportPlugin::FILTERENTRY> vFilterList;

  fOK = OtmProofReadImportPlugin::getLastUsedValues( strInputFile, strFilterDLL );

  // get list of available filters
  OtmProofReadImportPlugin::fillFilterList( vFilterList );

  if ( fOK ) fOK = OtmProofReadImportPlugin::getImportFiles( strInputFile, vInputFiles, strFilterDLL, vFilterList, pIda->hwnd );

  if ( fOK ) OtmProofReadImportPlugin::saveLastUsedValues( strInputFile, strFilterDLL );

  for( size_t i = 0; fOK && (i < vInputFiles.size()); i++ )
  {
    strProofReadXML = vInputFiles[i];
    strProofReadXML.append( ".tempxml" );
    if ( fOK ) fOK = OtmProofReadImportPlugin::applyFilter( vInputFiles[i], strFilterDLL, strProofReadXML, vFilterList );
    if ( fOK )
    {
      pvXMLFiles->push_back( strProofReadXML );
    } /* endif */
  } /* endfor */

  if ( fOK )
  {
    fOK = ProofReadLoadXMLFiles( pIda, pvXMLFiles, pIda->hwnd );
    ProofReadFillDocumentList( pIda );
    ProofReadMakeDiffLists( pIda, 0, pIda->pList->size() - 1 );
    ProofReadRefreshListbox( pIda );  
  } /* endif */

  return( fOK );
}

/*! \brief Load a list of XML files into the proof read entry list
  \param pIda pointer to dialog's instance data area
  \returns TRUE when successful
*/
BOOL ProofReadLoadXMLFiles( PPROOFREADDATA pIda, std::vector<std::string> *pvXMLFiles, HWND hwndParent )
{
  BOOL fOK = TRUE;

  pIda->pList->clear();

  for( size_t i = 0; fOK && (i < pvXMLFiles->size()); i++ )
  {
    int iRC = pIda->pList->load( (*pvXMLFiles)[i].c_str(), hwndParent );
    if ( iRC != 0 )
    {
      char szMessage[280];
      sprintf( szMessage, "Could not load validation document %s, continue with validation document processing?", (*pvXMLFiles)[i].c_str() );
      int iMBCode = MessageBox( hwndParent, szMessage, "Validation Document Import Error", MB_OKCANCEL| MB_ICONQUESTION );
      if ( iMBCode == IDCANCEL) fOK = FALSE;
    }
    else
    {
      DeleteFile( (*pvXMLFiles)[i].c_str() );
    } /* endif */
  } /* endfor */

  return( fOK );
}

/*! \brief Preset the last used value area with the defaults 
  \param pIda pointer to dialog's instance data area
  \returns TRUE when successful
*/
BOOL ProofReadSetDefaults( PPROOFREADDATA pIda )
{
  if ( pIda->pLastUsed )
  {
    pIda->pLastUsed->fShowUnchanged = FALSE;
    pIda->pLastUsed->fShowModified = TRUE;
    pIda->pLastUsed->fShowProcessed = TRUE;
    pIda->pLastUsed->fShowDifferences = TRUE;
    pIda->pLastUsed->swpSizePos.cx = DEFAULT_WINDOW_WIDTH;
    pIda->pLastUsed->swpSizePos.cy = DEFAULT_WINDOW_HEIGHT;
    pIda->pLastUsed->swpSizePos.x = 200;
    pIda->pLastUsed->swpSizePos.y = 200;
  } /* endif */
  return( TRUE );
}

/*! \brief Start edit mode for a listbox item
  \param pIda pointer to dialog's instance data area
  \param hwndListbox window handle of the listbox control
  \param iItem item index of listbox item
  \returns TRUE when successful
*/
BOOL ProofReadEditCell( PPROOFREADDATA pIda, HWND hwndListbox, int iItem )
{
  long xStart = 0;
  for( int i = 0; i < 5; i++ ) xStart += pIda->aiListViewColWidth[i];
  long xStop = xStart + pIda->aiListViewColWidth[5];
  HFONT hFont = (HFONT)SendMessage( hwndListbox, WM_GETFONT, 0, 0L );
  OtmProofReadEntry *pEntry = (OtmProofReadEntry *)QUERYITEMHANDLEHWND( hwndListbox, iItem);
  wcscpy( szEditText, pEntry->getNewTarget() );
  if ( szEditText[0] == 0 ) wcscpy( szEditText, pEntry->getModTarget() );
  RECT rectItem;
  RECT rectEdit;
  RECT rectListbox;
  SendMessage( hwndListbox, LB_GETITEMRECT, (WPARAM)iItem, (LPARAM)&rectItem );
  GetWindowRect( hwndListbox, &rectListbox );
  MapWindowPoints( HWND_DESKTOP, hwndDialog, (LPPOINT)&rectListbox, 2 );
  rectEdit.left = xStart /* + rectListbox.left*/; 
  rectEdit.right = xStop /* + rectListbox.left*/;
  rectEdit.top = rectItem.top + rectListbox.top + 2;
  rectEdit.bottom = rectEdit.top + rectItem.bottom - rectItem.top;
	hEdit = CreateWindowExW( WS_EX_CLIENTEDGE, L"EDIT", L"", WS_CHILD | WS_VISIBLE | ES_WANTRETURN | ES_AUTOVSCROLL | ES_MULTILINE, 
    rectEdit.left, rectEdit.top, rectEdit.right - rectEdit.left, rectEdit.bottom - rectEdit.top, hwndDialog, (HMENU)4711, (HINSTANCE)(HAB)UtlQueryULong( QL_HAB ), NULL);
  if ( hFont != NULL ) SendMessage( hEdit, WM_SETFONT, (WPARAM)hFont, 0L ); 
  SetWindowTextW( hEdit, szEditText );
  SendMessage( hEdit, EM_SETSEL, (WPARAM)0, (LPARAM)-1 );
	SetFocus(hEdit);
	EditOldProc = (WNDPROC)SetWindowLongW(hEdit, GWL_WNDPROC, (LONG)ProofReadEditProc);
	iEditListboxItem = iItem;
  return( TRUE );
}