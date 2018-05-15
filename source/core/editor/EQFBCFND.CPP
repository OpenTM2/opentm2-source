//+----------------------------------------------------------------------------+
// EQFBCFND.C                                                                  |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|          Copyright (C) 1990-2015, International Business Machines          |
//|          Corporation and others. All rights reserved                       |
//+----------------------------------------------------------------------------+
//|Author:                                                                     |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Description: This file contains all routines concerned with the concordance |
//|             search                                                         |
//+----------------------------------------------------------------------------+
#define INCL_EQF_MORPH            // morph settings for Arabic/Hebrew detection
#define INCL_EQF_TM               // general Transl. Memory functions
#define INCL_EQF_FOLDER           // folder related functions (required for memory selection dialog)
#define INCL_EQF_TP               // public translation processor functions
#define INCL_EQF_EDITORAPI        // public EDITOR API functions
#define INCL_EQF_TAGTABLE         // tagtable defines and functions
#define INCL_EQF_ANALYSIS         // analysis functions
#include <eqf.h>                  // General Translation Manager include file

#include <eqftmi.h>
#include "EQFTPI.H"               // Translation Processor priv. include file
#include <eqftai.h>               // private include file of Text Analysis
#include <eqfdoc00.h>
#include "EQFBDLG.ID"             // dialog control IDs
#include <process.h>              /* _beginthread, _endthread */


extern HELPSUBTABLE hlpsubtblTPCFindDlg[];

#include "OtmProposal.h"
#include "EditorMemLookupData.h"
#include "core\memory\MemoryFactory.h"
#include "vector"

#define CFIND_TIMERID 9123
#define CFIND_TIMER_INTERVALL 100

// max number of matches passed fom thread to dialog
#define MAX_FOUNDMATCHES 20

// max number of matches passed fom thread to dialog
#define MAX_MARKS 5

// max number of additional search memories
#define MAX_ADDMEMNUM  20

//  size of buffer to prepare a single match
#define MATCHBUFSIZE     8000

// defines for search history
#define MAX_CFIND_SEARCH_HIST  20

// data area for found matches
typedef struct _MARKINFO
{
  int         iStart;                            // start of mark
  int         iEnd;                              // end of mark
} MARKINFO, *PMARKINFO;

// data area for found matches
typedef struct _FOUNDMATCH
{
  BOOL        fUsed;                             // TRUE = this entry is filled with a match 
  CHAR_W      szSource[MAX_SEGMENT_SIZE];        // source of match
  CHAR_W      szTarget[MAX_SEGMENT_SIZE];        // target of match
  CHAR        szDoc[MAX_LONGFILESPEC];           // document of match
  ULONG       ulSegNum;                          // segment numnber of match
  CHAR        szMarkup[MAX_FILESPEC];            // markup table name
  MARKINFO    SourceMarks[MAX_MARKS];            // marks in source 
  MARKINFO    TargetMarks[MAX_MARKS];            // marks in target 
  CHAR        szTgtLang[MAX_LANG_LENGTH];        //language name of target
} FOUNDMATCH, *PFOUNDMATCH;

// states of our lookup thread
typedef enum _THREADSTATE 
{ 
  CST_IDLE,                                      // thread is idle and waiting for work
  CST_STOPPED,                                   // thread has been stopped
  CST_PROCESSMEM,                                // search memory segments
  CST_WAITFORBUFFER,                             // wait for a free buffer
  CST_STARTSEARCH,                               // start searching 
  CST_SEARCHDONE,                                // search is complete 
  CST_NEXTMEM                                    // continue with next memory
} THREADSTATE; 

// area for CFIND last used values (V1 format)
typedef struct _CFINDLU_V1
{
   int        iAddMem;                             // number of additional  search memories
   char       szAddMem[MAX_ADDMEMNUM][MAX_LONGFILESPEC];  // array with names of additional search memories
   CHAR_W     szHistory[MAX_CFIND_SEARCH_HIST][MAX_FINDCHANGE_LEN+1]; // search history
   BOOL       fSource;                             // TRUE = find in source
   BOOL       fTarget;                             // TRUE = find in target
   BOOL       fIgnoreCase;                         // ignorecase (TRUE = IGNORE)
   SWP        swpSizePos;                          // size and positionof window
   BOOL       fPrimaryMem;                         // TRUE = search primary memory
   char       szBuffer[16000];
} CFINDLU_V1, *PCFINDLU_V1;

// area for CFIND last used values (current format)
typedef struct _CFINDLU
{
   int        iAddMem;                             // number of additional  search memories
   char       szAddMem[MAX_ADDMEMNUM][MAX_LONGFILESPEC];  // array with names of additional search memories
   CHAR_W     szHistory[MAX_CFIND_SEARCH_HIST][MAX_SEGMENT_SIZE+1]; // search history
   BOOL       fSource;                             // TRUE = find in source
   BOOL       fTarget;                             // TRUE = find in target
   BOOL       fIgnoreCase;                         // ignorecase (TRUE = IGNORE)
   SWP        swpSizePos;                          // size and positionof window
   BOOL       fPrimaryMem;                         // TRUE = search primary memory
   char       szBuffer[16000];
} CFINDLU, *PCFINDLU;


// data area for concordance search dialog
typedef struct _CFINDDATA
{
   CHAR_W chFind[ MAX_SEGMENT_SIZE + 1];         // buffer for find string
   CHAR_W chFindTarget[ MAX_SEGMENT_SIZE + 1];   // buffer for normalized find
   BOOL       fSource;                             // TRUE = find in source
   BOOL       fTarget;                             // TRUE = find in target
   BOOL       fIgnoreCase;                         // ignorecase (TRUE = IGNORE)
   PTBDOCUMENT pDoc;                               // active document
   HWND       hwndFindDlg;                         // modeless find dialog
   BOOL       fTimerProcessing;                    // TRUE = curently processing timer message
   BOOL       fPrimaryMem;                         // TRUE = search primary memory
   int        iAddMem;                             // number of additional  search memories
   char       szAddMem[MAX_ADDMEMNUM][MAX_LONGFILESPEC];  // array with names of additional search memories
   FOUNDMATCH FoundMatch[MAX_FOUNDMATCHES];        // table with found matches
   CHAR_W     szMatchBuffer[MATCHBUFSIZE];         // buffer to prepare a single match
   CHAR       szBuffer[4096];                      // general purpose buffer 
   BOOL       fSearching;                          // TRUE = we are currently searching
   HWND       hwndMemLBA;                          // listbox containing names of available memories
   CHAR       szPropFile[MAX_EQF_PATH];            // name of file containing last used values
   PCFINDLU   pLU;                                 // data area with last used values
   int        iCurSearchOffs;                      // curent search offset in result area
   int        iCurLen;                             // current length of text in result area
   CHAR       szSourceLang[MAX_LANG_LENGTH];       // document source language
   ULONG      ulSourceCP;                          // codepage for document source language

   // window positions and offsets
   SHORT      sTitleHeight;                        // height of title bar
   SHORT      sBorderSize;                         // border width
   SHORT      sScreenXSize;                        // screen width
   SHORT      sScreenYSize;                        // screen heigth
   SHORT      sMinX;                               // minimum window width
   SHORT      sMinY;                               // minimum window heigth
   SHORT      sButtonOffs;                         // offset of pushbutton relative to right border
   SHORT      sResultBoxOrgX;                      // original width of result box
   SHORT      sResultBoxOrgY;                      // original height of result box
   SHORT      sResultFieldOrgX;                    // original width of result box
   SHORT      sResultFieldOrgY;                    // original height of result field 
   SHORT      sStatusLineOrgY;                     // original Y position of status line
   SHORT      sStatusLineOrgX;                     // original width of status line
   OtmMemory  *pPrimMem;                           // pointer to primary memory object
   char       szPrimMemPath[MAX_LONGFILESPEC];     // path of primary memory 
   char       szPrimMemName[MAX_LONGFILESPEC];     // name of primary memory 
   CHAR_W     szResFind[256];                      // buffer for find in result string

   // commnication with search thread
   BOOL      fStop;                                // TRUE = stop at once
   BOOL      fThreadAct;                           // TRUE = thread si up and running
   THREADSTATE ThreadState;                        // current state of thread
   CHAR      szStatusLine[1024];                   // text for status line

   // data area of search thead
   int        iCurMem;                             // currently processed memory
   char       szCurMemPath[MAX_LONGFILESPEC];      // path of currently processed memory
   char       szCurMemName[MAX_LONGFILESPEC];      // name of currently processed memory
   OtmMemory  *pCurrentMem;                        // pointer to current memory object
   OtmProposal *pProposal;                         // pointer to proposal data
   CHAR_W     szSegData[MAX_SEGMENT_SIZE];         // buffer for segment data
   int        iHits;                               // number of found strings
   int        iComplete;                           // completion ratio
   BOOL       fMemOpen;                            // TRUE = a memory has been openend
   BOOL       fFirstLookup;                        // TRUE = start at begin of memory
   CHAR_W     szMatchData[MAX_SEGMENT_SIZE];       // buffer for segment data of matching proposal
} CFINDDATA, *PCFINDDATA;




MRESULT EQFBCFindInit( HWND, WPARAM, LPARAM );
MRESULT EQFBCFindCommand( HWND, WPARAM, LPARAM );
VOID    EQFBCFindFill( HWND, PCFINDDATA );          // fill find data struct
VOID    EQFBCFindButtons( HWND, BOOL );// enable/disable buttons and other controls
static void CFindStartThread( PCFINDDATA pCFindData );
static VOID CFindSearchThread( PVOID pvCFindData );
static void CFindInsertMatch( HWND hwndDlg, PCFINDDATA pCFindData, int iMatch );
static void CFindResize( HWND hwndDlg, PCFINDDATA pCFindData, SHORT sNewWidth, SHORT sNewHeight );
static BOOL GetRelativeWindowPos( HWND hwndControl, SWP FAR *pSwp );
static void CFindSetColor( HWND hwndMLE, int iStart, int iEnd, COLORREF colorText, COLORREF colorBackground );
static SHORT CFindStrNLFCmp( PSZ_W pData, PSZ_W pSearch );
VOID CFindGetMemsFromLB( HWND hwndDlg, PCFINDDATA pCFindData );

static VOID EQFBBlanksAndLFSubst
(
    PTBDOCUMENT  pDoc,
    PSZ_W        pTextStart,
    ULONG        ulSegNum,
    PTBSEGMENT   pSeg
);

BOOL CFindCheckForAnsiConv
(
        PTBDOCUMENT pDoc
);
static void CFindStopThread( PCFINDDATA pCFindData );

PCFINDDATA pCFindData = NULL;

// interface called by EQFBWPRC.C
VOID EQFBCFind
(
   PTBDOCUMENT pDoc                             // pointer to document ida
)
{
  // allocate CFindData structure if not done yet
  if ( !pCFindData )
  {
    UtlAlloc( (PVOID *)&pCFindData, 0L, (LONG) sizeof(CFINDDATA), ERROR_STORAGE );
  } /* endif */

  // close any dialog open for another document
  if ( pCFindData )
  {
    if ( (pCFindData->pDoc != pDoc) && pCFindData->hwndFindDlg )
    {
      EQFBCFindClose( pCFindData->hwndFindDlg, 0, 0L );
    } /* endif */
  } /* endif */

  // activate dialog
  if ( pCFindData )
  {
    if ( !pCFindData->hwndFindDlg )
    {
      memset( pCFindData, 0, sizeof(CFINDDATA));   // clear find data area
    } /* endif */

    pCFindData->pDoc = pDoc;                     // set active document

    if ( !pCFindData->hwndFindDlg )
    {
      HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);

      FARPROC lpfnProc = MakeProcInstance(  (FARPROC)(EQFBCFINDDLGPROC), (HAB) UtlQueryULong( QL_HAB ) );
      pCFindData->hwndFindDlg = CreateDialogParamW( hResMod,
                                              (PSZ_W)MAKEINTRESOURCE( ID_TB_CFIND_DLG),
                                              pDoc->hwndClient, (DLGPROC)lpfnProc,
                                              (LPARAM)pCFindData );
      FreeProcInstance( lpfnProc );
    } /* endif */


    if ( pCFindData->hwndFindDlg )
    {
      WinSetWindowPos( pCFindData->hwndFindDlg, HWND_TOP, 0,0,0,0, EQF_SWP_ACTIVATE | EQF_SWP_SHOW );
    } /* endif */
  } /* endif */
}

// interface called to close any open concordance search window for document being closed
VOID EQFBCFindTerminate
(
   PTBDOCUMENT pDoc                             // pointer to document ida
)
{
  if ( pCFindData )
  {
    if ( (pCFindData->pDoc == pDoc) && pCFindData->hwndFindDlg )
    {
      EQFBCFindClose( pCFindData->hwndFindDlg, 0, 0L );
      pCFindData->pDoc = NULL;                     // set active document
    } /* endif */
  } /* endif */
}


INT_PTR CALLBACK EQFBCFINDDLGPROC
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
  case WM_EQF_QUERYID: HANDLEQUERYID( ID_TB_CFIND_DLG, mp2 ); break;

  case WM_INITDLG:
    SETWINDOWID( hwndDlg, ID_TB_CFIND_DLG );
    mResult = DIALOGINITRETURN( EQFBCFindInit( hwndDlg, mp1, mp2 ));
    UtlRegisterModelessDlg( hwndDlg );
    break;

  case WM_COMMAND:
    mResult = EQFBCFindCommand( hwndDlg, mp1, mp2 );
    break;

  case WM_SIZE:
    {
      PCFINDDATA pCFindData = ACCESSDLGIDA(hwndDlg, PCFINDDATA );
      if ( (pCFindData != NULL) && ((mp1 == SIZENORMAL) || (mp1 == SIZEFULLSCREEN)) )
      {
        CFindResize( hwndDlg, pCFindData, LOWORD(mp2), HIWORD(mp2) ); 
      } /* endif */
    }
    break;

  case WM_GETMINMAXINFO:
    {
      MINMAXINFO FAR *lpMinMax = (MINMAXINFO *)PVOIDFROMMP2(mp2);
      PCFINDDATA pCFindData = ACCESSDLGIDA(hwndDlg, PCFINDDATA );
      lpMinMax->ptMinTrackSize.x = pCFindData->sMinX;
      lpMinMax->ptMinTrackSize.y = pCFindData->sMinY;
    }
    break;

  case WM_CLOSE:
    mResult = EQFBCFindClose( hwndDlg, mp1, mp2 );
    break;

 case WM_ACTIVATE:
    if ( mp1 )
    {
//      EQFBCFindButtons( hwndDlg, TRUE );
      BringWindowToTop( hwndDlg );
      WinPostMsg( hwndDlg, WM_EQF_SETFOCUS, 0, MP2FROMHWND( WinWindowFromID(hwndDlg, ID_TB_CFIND_FIND_EF)));
    } /* endif */
    mResult = FALSE;
    break;

  case WM_EQF_SETFOCUS:
    SETFOCUS( hwndDlg, ID_TB_CFIND_FIND_EF );
    SETEFSEL( hwndDlg, ID_TB_CFIND_FIND_EF, 0, -1 );
    break;

  case WM_DESTROY:
    {
      PCFINDDATA pCFindData = ACCESSDLGIDA(hwndDlg, PCFINDDATA );
      if ( pCFindData )
      {
        if ( pCFindData->hwndMemLBA ) WinDestroyWindow( pCFindData->hwndMemLBA );
        if ( pCFindData->pLU) UtlAlloc( (PVOID *)&(pCFindData->pLU), 0, 0, NOMSG );
        if ( pCFindData->pProposal != NULL ) 
        {
          free( pCFindData->pProposal ); 
          pCFindData->pProposal = NULL;
        } /* endif */
      } /* endif */
    }
    UtlUnregisterModelessDlg( hwndDlg );
    break;

  case WM_HELP:
      EqfDisplayContextHelp( (HWND)((LPHELPINFO) mp2)->hItemHandle, &hlpsubtblTPCFindDlg[0] );
      mResult = TRUE;  // message processed
      break;


  default:
    mResult = FALSE;
    break;
  } /* endswitch */

  return mResult;
} /* end of EQFBCFINDDLGPROC */


static void CFindResize
( 
  HWND hwndDlg,
  PCFINDDATA pCFindData, 
  SHORT sNewWidth, 
  SHORT sNewHeight 
)
{
  SWP swp;
  SHORT cyDiff = sNewHeight - pCFindData->sMinY + pCFindData->sTitleHeight;
  SHORT cxDiff = sNewWidth - pCFindData->sMinX;

  GetRelativeWindowPos( WinWindowFromID( hwndDlg, ID_TB_CFIND_FIND_PB ), &swp );
  MoveWindow( WinWindowFromID( hwndDlg, ID_TB_CFIND_FIND_PB ), sNewWidth - pCFindData->sButtonOffs,swp.y, swp.cx, swp.cy, TRUE );

  GetRelativeWindowPos( WinWindowFromID( hwndDlg, ID_TB_CFIND_STOP_PB ), &swp );
  MoveWindow( WinWindowFromID( hwndDlg, ID_TB_CFIND_STOP_PB ), sNewWidth - pCFindData->sButtonOffs,swp.y, swp.cx, swp.cy, TRUE );

  GetRelativeWindowPos( WinWindowFromID( hwndDlg, ID_TB_CFIND_CANCEL_PB ), &swp );
  MoveWindow( WinWindowFromID( hwndDlg, ID_TB_CFIND_CANCEL_PB ), sNewWidth - pCFindData->sButtonOffs,swp.y, swp.cx, swp.cy, TRUE );

  GetRelativeWindowPos( WinWindowFromID( hwndDlg, ID_TB_CFIND_HELP_PB ), &swp );
  MoveWindow( WinWindowFromID( hwndDlg, ID_TB_CFIND_HELP_PB ), sNewWidth - pCFindData->sButtonOffs,swp.y, swp.cx, swp.cy, TRUE );

  GetRelativeWindowPos( WinWindowFromID( hwndDlg, ID_TB_CFIND_RESULT_GB ), &swp );
  MoveWindow( WinWindowFromID( hwndDlg, ID_TB_CFIND_RESULT_GB ), swp.x, swp.y, pCFindData->sResultBoxOrgX + cxDiff, 
    pCFindData->sResultBoxOrgY + cyDiff, TRUE );

  GetRelativeWindowPos( WinWindowFromID( hwndDlg, ID_TB_CFIND_RESULT_EF ), &swp );
  MoveWindow( WinWindowFromID( hwndDlg, ID_TB_CFIND_RESULT_EF ), swp.x, swp.y, pCFindData->sResultFieldOrgX + cxDiff, 
    pCFindData->sResultFieldOrgY + cyDiff, TRUE );

  GetRelativeWindowPos( WinWindowFromID( hwndDlg, ID_TB_CFIND_STATUS_TEXT ), &swp );
  MoveWindow( WinWindowFromID( hwndDlg, ID_TB_CFIND_STATUS_TEXT ), swp.x, pCFindData->sStatusLineOrgY + cyDiff, 
    pCFindData->sStatusLineOrgX + cxDiff, swp.cy, TRUE );

} /* end of CFindResize */


#ifdef UNUSED_CODE
static VOID EQFBBlanksAndLFSubst
(
    PTBDOCUMENT  pDoc,
    PSZ_W        pTextStart,
    ULONG        ulSegNum,
    PTBSEGMENT   pSeg
)
{
    BOOL   fWorkchng;                   // store EQFBFlags.workchng
    BOOL   fTyped;                      // store SegFlags.Typed
    PSZ_W p1 = pSeg->pDataW;
    PSZ_W p2 = pSeg->pDataW;
    CHAR_W c;

    if ( !IsDBCS_CP(pDoc->ulOemCodePage))
    {
      /******************************************************/
      /* if we are dealing with the work seg, get rid of    */
      /* pending blanks                                     */
      /******************************************************/
      if ( ulSegNum == pDoc->ulWorkSeg )
      {
        while ( (c = *p1 = *p2) != NULC )
        {
          if ( c == LF )
          {
            *p1 = BLANK;
            while ( *p1 == BLANK )
            {
              p1--;
            } /* endwhile */
            p1++;
            *p1 = LF;
          } /* endif */
          p1++;
          p2++;
        } /* endwhile */
        *p1 = EOS;
        fWorkchng = pDoc->EQFBFlags.workchng;

        // P016416: if SegFlags.Typed is changed to TRUE, color of segment can change to
        // "Translated as MODIFIED proposal" during POstedit - reviewing
        fTyped = pDoc->pTBSeg->SegFlags.Typed;  // P016416:
        EQFBUpdateChangedSeg(pDoc);      //update segment ids
        pDoc->EQFBFlags.workchng = (USHORT) fWorkchng;
        pDoc->pTBSeg->SegFlags.Typed = (USHORT) fTyped;
      } /* endif */
    }
    /******************************************************/
    /* substitute LF through blanks                       */
    /******************************************************/
    p1 = pTextStart;
    p2 = pSeg->pDataW;
    while ( (c = *p1 = *p2) != NULC )
    {
        if ( (c == LF) || (c == SOFTLF_CHAR) )
        {
          if (!IsDBCS_CP(pDoc->ulOemCodePage) )           //for SBCS only
          {
            *p1 = BLANK;
          }
          else
          {
            /**************************************************/
            /* do not insert a blank, if a DBCS string is fol.*/
            /**************************************************/
            if (! EQFIsDBCSChar(*(p2+1), pDoc->ulOemCodePage)  )
            {
              *p1 = BLANK;
            }
            else
            {
              /************************************************/
              /* nothing to do -- LFs treated in StrNLFCmp    */
              /************************************************/
            } /* endif */
          } /* endif */
        } /* endif */
        p1++;
        p2++;
    } /* endwhile */
    *p1 = EOS;

    return;

}
#endif


// copy any cliboard text to find field
VOID CFindPasteToEF( HWND hwndDlg, PCFINDDATA pCFindData )
{
  PSZ_W  pchClipText = NULL;                // pointer to clipboard
  HANDLE  hClipMemory;

  pCFindData;

  OpenClipboard( hwndDlg );
  hClipMemory = GetClipboardData(CF_UNICODETEXT);
  pchClipText = (PSZ_W)GlobalLock (hClipMemory);

  // if Unicode text in clipboard
  if ( pchClipText )
  {
    SETTEXTW( hwndDlg, ID_TB_CFIND_FIND_EF, pchClipText );
  }
  else
  {
    // try with text format only
    hClipMemory = GetClipboardData(CF_TEXT);
    pchClipText = (PSZ_W)GlobalLock (hClipMemory);

    if ( pchClipText )
    {
      SETTEXT( hwndDlg, ID_TB_CFIND_FIND_EF, ((PSZ)pchClipText) );
    } /* endif */
  } /* endif */

  GlobalUnlock(hClipMemory);
  CloseClipboard();
}

MRESULT EQFBCFindInit
(
HWND    hwndDlg,                    // handle of dialog window
WPARAM  mp1,                        // first parameter of WM_INITDLG
LPARAM  mp2                         // second parameter of WM_INITDLG
)
{
  MRESULT     mResult = FALSE;        // result of message processing
  PCFINDDATA   pCFindData;              // pointer to ida for find init
  PTBDOCUMENT pDoc;                   // pointer to document
  USHORT      usId;                   // user id
  BOOL        fAnsiToOEM = TRUE;
  int         xPos = 100;              // intial dialog position 
  int         yPos = 50;
  int         xSize = 670;              // intial dialog position 
  int         ySize = 480;
  mp1 = mp1;                          // suppress 'unreferenced parameter' msg

  pCFindData = (PCFINDDATA) mp2;
  if ( pCFindData )
  {
    ANCHORDLGIDA( hwndDlg, pCFindData );
    pDoc = pCFindData->pDoc ;            // point to active document

    pCFindData->fSearching = FALSE;     // we are not in search mode

    fAnsiToOEM = CFindCheckForAnsiConv(pDoc);

    // load/create file with last used values
    {
      ULONG ulLen = 0;
      UtlMakeEQFPath( pCFindData->szPropFile, NULC, PROPERTY_PATH, NULL );
      strcat( pCFindData->szPropFile, "\\CFIND.LUV" );

      ulLen = sizeof(CFINDLU);
      ulLen = sizeof(CFINDLU_V1);
      ulLen = 0;


      if ( !UtlLoadFileL( pCFindData->szPropFile, (PVOID *)&pCFindData->pLU, &ulLen, FALSE, FALSE ) )
      {
        UtlAlloc( (PVOID *)&(pCFindData->pLU), 0, sizeof(CFINDLU), ERROR_STORAGE );
      }
      else
      {
        // convert LU to new format if necessary
        if ( ulLen == sizeof(CFINDLU_V1) )
        {
          int i = 0;
          PCFINDLU_V1 pOld = (PCFINDLU_V1)pCFindData->pLU;
          pCFindData->pLU = NULL;
          UtlAlloc( (PVOID *)&(pCFindData->pLU), 0, sizeof(CFINDLU), ERROR_STORAGE );

          pCFindData->pLU->iAddMem = pOld->iAddMem;
          memcpy( pCFindData->pLU->szAddMem, pOld->szAddMem, sizeof(pOld->szAddMem) );
          for( i = 0; i < MAX_CFIND_SEARCH_HIST; i++ )
          {
            wcscpy( pCFindData->pLU->szHistory[i], pOld->szHistory[i] ); 
          } /* endfor */
          pCFindData->pLU->fSource, pOld->fSource;
          pCFindData->pLU->fTarget, pOld->fTarget;
          pCFindData->pLU->fIgnoreCase, pOld->fIgnoreCase;
          memcpy( &(pCFindData->pLU->swpSizePos), &(pOld->swpSizePos), sizeof(pOld->swpSizePos) );

          UtlAlloc( (PVOID *)&pOld, 0, 0, NOMSG );
        } /* endif */
      } /* endif */
    }

    // setup document object name and retrieve document memory
    if ( pDoc && pDoc->pstEQFGen )
    {
      PSTEQFGEN pstEQFGen = (PSTEQFGEN)pDoc->pstEQFGen;
      PDOCUMENT_IDA pIda = (PDOCUMENT_IDA)pstEQFGen->pDoc;
      PTPMEMLOOKUPDATA pMemData = (PTPMEMLOOKUPDATA)pIda->pvMemLookup;

       
      pCFindData->pPrimMem = pMemData->MemoryList[0];
      strcpy( pCFindData->szPrimMemPath, pIda->szMemory[0] );
      strcpy( pCFindData->szPrimMemName, pIda->szDocMemory );
      strcpy( pCFindData->szSourceLang, pIda->szDocSourceLang );
      pCFindData->ulSourceCP = GetLangOEMCP( pCFindData->szSourceLang );
      sprintf( pCFindData->szBuffer, "Primary memory (%s)", pCFindData->szPrimMemName );
      SETTEXT( hwndDlg, ID_TB_CFIND_PRIMMEM_CHK, pCFindData->szBuffer );
    } /* endif */

    // allocate Proposal object
    if ( pCFindData->pProposal == NULL ) pCFindData->pProposal = new( OtmProposal );

    // replace find combobox with Unicode enabled counterpart
    {
      WINDOWPLACEMENT Placement;
      HWND hwndFindEF   = GetDlgItem( hwndDlg, ID_TB_CFIND_FIND_EF );

      GetWindowPlacement( hwndFindEF, &Placement );
      DestroyWindow( hwndFindEF );
      hwndFindEF = CreateWindowExW( 0, L"ComboBox", L"", 
                                CBS_DROPDOWN | CBS_AUTOHSCROLL | CBS_SORT | WS_VSCROLL | WS_GROUP | WS_TABSTOP | WS_VISIBLE | WS_CHILD,
                                Placement.rcNormalPosition.left, Placement.rcNormalPosition.top, 
                                Placement.rcNormalPosition.right - Placement.rcNormalPosition.left, 
                                UtlQueryULong( QL_PELSPERLINE ) * 10,
                                hwndDlg, (HMENU)ID_TB_CFIND_FIND_EF, (HINSTANCE)(HAB)UtlQueryULong( QL_HAB ), 0 );
      SetWindowLong( hwndFindEF, GWL_ID, (LONG)ID_TB_CFIND_FIND_EF  );
      SetWindowPos( hwndFindEF, HWND_TOP, 0, 0, Placement.rcNormalPosition.right - Placement.rcNormalPosition.left, 
                                UtlQueryULong( QL_PELSPERLINE ) * 10, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOREDRAW );

    }

    // preset Find Entry field and select it
    SETTEXTLIMIT( hwndDlg, ID_TB_CFIND_FIND_EF, MAX_SEGMENT_SIZE );

    // Fill ID_TB_CFIND_FIND_EF
    if ( pCFindData->chFind[0] != EOS )
    {
      CBDELETEALL( hwndDlg, ID_TB_CFIND_FIND_EF );
      CBINSERTITEMENDW( hwndDlg,ID_TB_CFIND_FIND_EF , pCFindData->chFind );
      CBSELECTITEM (hwndDlg, ID_TB_CFIND_FIND_EF , 0 );
    } /* endif */

    // Insert History into ID_TB_CFIND_FIND_EF
    if ( pCFindData->pLU )
    {
      int i=0;
      while ( i < MAX_CFIND_SEARCH_HIST )
      {
        if ( pCFindData->pLU->szHistory[i][0] != EOS )
        {
          SHORT sItem = CBSEARCHITEMW( hwndDlg, ID_TB_CFIND_FIND_EF, pCFindData->pLU->szHistory[i]);

          if (sItem == LIT_NONE)
          {
            CBINSERTITEMENDW( hwndDlg, ID_TB_CFIND_FIND_EF , pCFindData->pLU->szHistory[i]);
          }
        } /* endif */
        i++;
      } /* endwhile */

      // get last use dialog size and position (if available)
      if ( (pCFindData->pLU->swpSizePos.cx != 0) && (pCFindData->pLU->swpSizePos.cy != 0) )
      {
        xPos = pCFindData->pLU->swpSizePos.x;
        yPos = pCFindData->pLU->swpSizePos.y;
        xSize = pCFindData->pLU->swpSizePos.cx;
        ySize = pCFindData->pLU->swpSizePos.cy;
      } /* endif */

      pCFindData->fPrimaryMem  = pCFindData->pLU->fPrimaryMem;
      pCFindData->iAddMem =  pCFindData->pLU->iAddMem;
      memcpy( pCFindData->szAddMem, pCFindData->pLU->szAddMem, sizeof(pCFindData->szAddMem) );
      pCFindData->fIgnoreCase = pCFindData->pLU->fIgnoreCase;
      pCFindData->fSource = pCFindData->pLU->fSource;
      pCFindData->fTarget = pCFindData->pLU->fTarget;

    } /* endif */

    // set init for 1st call
    if ( IS_RTL_ARABIC( pCFindData->pDoc ))
    {
      pCFindData->fIgnoreCase = FALSE;     // for Arabic, ignorecase -search should be handled as respect-case
      ENABLECTRL( hwndDlg, ID_TB_CFIND_IGNORECASE_CHK, FALSE );  // Arabic: only respect-case is allowed
    }

    SETCHECK( hwndDlg, ID_TB_CFIND_IGNORECASE_CHK, pCFindData->fIgnoreCase );
    SETCHECK( hwndDlg, ID_TB_CFIND_PRIMMEM_CHK, pCFindData->fPrimaryMem );
    if ( pCFindData->iAddMem )
    {
      int i = 0;
      SETCHECK( hwndDlg, ID_TB_CFIND_ADDMEM_CHK, (pCFindData->iAddMem != 0) );
      while ( i < pCFindData->iAddMem )
      {
        OEMTOANSI( pCFindData->szAddMem[i] );
        INSERTITEMEND( hwndDlg, ID_TB_CFIND_ADDMEM_LB, pCFindData->szAddMem[i] );
        ANSITOOEM( pCFindData->szAddMem[i] );
        i++;
      } /* endwhile */
    }
    else
    {
      SETCHECK_FALSE( hwndDlg, ID_TB_CFIND_ADDMEM_CHK );
      ENABLECTRL( hwndDlg, ID_TB_CFIND_MEMCHANGE_PB, FALSE );
      ENABLECTRL( hwndDlg, ID_TB_CFIND_ADDMEM_LB, FALSE );
    } /* endif */


    usId = (pCFindData->fSource) ? ID_TB_CFIND_SOURCE_RB : ID_TB_CFIND_TARGET_RB;
    CLICK( hwndDlg, usId );

    pCFindData->sTitleHeight = (SHORT) WinQuerySysValue (HWND_DESKTOP, SV_CYTITLEBAR);
    pCFindData->sBorderSize  = (SHORT) WinQuerySysValue (HWND_DESKTOP, SV_CXSIZEBORDER);
    pCFindData->sScreenXSize = (SHORT) WinQuerySysValue (HWND_DESKTOP, SV_CXSCREEN);
    pCFindData->sScreenYSize = (SHORT) WinQuerySysValue (HWND_DESKTOP, SV_CYSCREEN);

    // use initial window size as minimum size and comoute offsets used for window resize operations
    {
      EQF_SWP swpDlg, swpControl; 

      WinQueryWindowPos( hwndDlg, &swpDlg );
      pCFindData->sMinX = swpDlg.cx;
      pCFindData->sMinY = swpDlg.cy;

      WinQueryWindowPos( WinWindowFromID( hwndDlg, ID_TB_CFIND_FIND_PB ), &swpControl );
      pCFindData->sButtonOffs = swpDlg.cx - swpControl.x;

      WinQueryWindowPos( WinWindowFromID( hwndDlg, ID_TB_CFIND_RESULT_GB ), &swpControl );
      pCFindData->sResultBoxOrgX = swpControl.cx;
      pCFindData->sResultBoxOrgY = swpControl.cy;

      WinQueryWindowPos( WinWindowFromID( hwndDlg, ID_TB_CFIND_RESULT_EF ), &swpControl );
      pCFindData->sResultFieldOrgX = swpControl.cx;
      pCFindData->sResultFieldOrgY = swpControl.cy;

      WinQueryWindowPos( WinWindowFromID( hwndDlg, ID_TB_CFIND_STATUS_TEXT ), &swpControl );
      pCFindData->sStatusLineOrgX = swpControl.cx;
      pCFindData->sStatusLineOrgY = swpControl.y;
      GetRelativeWindowPos( WinWindowFromID( hwndDlg, ID_TB_CFIND_STATUS_TEXT ), &swpControl );
      pCFindData->sStatusLineOrgX = swpControl.cx;
      pCFindData->sStatusLineOrgY = swpControl.y;

    }

    pCFindData->hwndMemLBA = WinCreateWindow( hwndDlg, WC_LISTBOX, "", WS_CHILD | LBS_STANDARD, 0, 0, 0, 0, 
                                             hwndDlg, HWND_TOP, 4712, NULL, NULL );

    EqfSend2Handler( MEMORYHANDLER, WM_EQF_INSERTNAMES, MP1FROMHWND( pCFindData->hwndMemLBA), MP2FROMP( MEMORY_ALL )  );

    SetCtrlFnt ( hwndDlg, GetCharSet(), ID_TB_CFIND_RESFIND_EF, 0 );

    SETTEXTLIMIT( hwndDlg, ID_TB_CFIND_RESFIND_EF, sizeof(pCFindData->szResFind) / sizeof(CHAR_W) );

    CFindPasteToEF( hwndDlg, pCFindData );

  } /* endif */

  // disable stop button (only active while searching)
  ENABLECTRL( hwndDlg, ID_TB_CFIND_STOP_PB, FALSE );

  // disable change buton and add memory listbox
  ENABLECTRL( hwndDlg, ID_TB_CFIND_ADDMEM_LB, FALSE );
  ENABLECTRL( hwndDlg, ID_TB_CFIND_MEMCHANGE_PB, FALSE );

  // preset primary memory checkbox
  SETCHECK_TRUE( hwndDlg, ID_TB_CFIND_PRIMMEM_CHK );

  // set size and position the dialog window
  //WinSetWindowPos( hwndDlg, HWND_TOP, xPos, yPos, xSize, ySize, EQF_SWP_MOVE | EQF_SWP_SIZE | EQF_SWP_ACTIVATE | EQF_SWP_SHOW  );
  SetWindowPos( hwndDlg, HWND_TOP, xPos, yPos, xSize, ySize, SWP_SHOWWINDOW );

  SETTEXT( hwndDlg, ID_TB_CFIND_STATUS_TEXT, "" );

  pCFindData->iCurLen = 0;


  return( mResult );
} /* end of EQFBCFindInit */

void CFindProcessStop( PCFINDDATA pCFindData )
{
  KillTimer( pCFindData->hwndFindDlg, CFIND_TIMERID );
  pCFindData->fSearching = FALSE;
  CFindStopThread( pCFindData );
  EQFBCFindButtons( pCFindData->hwndFindDlg, TRUE );
  ENABLECTRL( pCFindData->hwndFindDlg, ID_TB_CFIND_STOP_PB, FALSE );
}

VOID CFindShowMemSelectDlg( HWND hwndDlg, PCFINDDATA pCFindData )
{
  BOOL fOK = TRUE;
  PSELECTDLGIDA pSelDlgIda = NULL;

  fOK = UtlAlloc( (PVOID *)&pSelDlgIda, 0, sizeof(SELECTDLGIDA), ERROR_STORAGE );

  if ( fOK)
  {
    pSelDlgIda->fMemSel = TRUE;
    pSelDlgIda->iDialogID = ID_FOLMEMSEL_DLG;
    pSelDlgIda->hwndAvailLB = pCFindData->hwndMemLBA;
    pSelDlgIda->iMaxSelected = MAX_ADDMEMNUM;
    strcpy( pSelDlgIda->szTitleBar, "Select Translation Memory databases to be searched" );

    // get currently selected memories
    CFindGetMemsFromLB( hwndDlg, pCFindData );

    // copy selected memories to select dialog IDA
    {
      int i = 0;
      while ( i < pCFindData->iAddMem )
      {
        strcpy( pSelDlgIda->aSelected[i], pCFindData->szAddMem[i] );
        i++;
      } /* endwhile */
    }
  } /* endif */

  if ( fOK )
  {
    HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
    DIALOGBOX( hwndDlg, FOLMEMSEL_DLGPROC, hResMod, ID_FOLMEMSEL_DLG, pSelDlgIda, fOK );
  } /* endif */

  if ( fOK )
  {
    int i = 0;
    DELETEALL( hwndDlg, ID_TB_CFIND_ADDMEM_LB );
    while ( i < pSelDlgIda->iSelected )
    {
      OEMTOANSI( pSelDlgIda->aSelected[i] );
      INSERTITEMEND( hwndDlg, ID_TB_CFIND_ADDMEM_LB, pSelDlgIda->aSelected[i] );
      i++;
    } /* endwhile */
  } /* endif */

  if ( pSelDlgIda ) UtlAlloc( (PVOID *)&pSelDlgIda, 0, 0, NOMSG );
}

VOID CALLBACK CFindTimerProc
(
  HWND hwnd,         // handle to window
  UINT uMsg,         // WM_TIMER message
  UINT_PTR idEvent,  // timer identifier
  DWORD dwTime       // current system time
)
{
  PCFINDDATA  pCFindData;                    // pointer to find data

  uMsg; idEvent; dwTime;

  pCFindData = ACCESSDLGIDA(hwnd, PCFINDDATA );

  // check if thread has new proposals to display
  if ( !pCFindData->fTimerProcessing )
  {
    int i = 0;

    pCFindData->fTimerProcessing = TRUE;

    if ( pCFindData->szStatusLine[0] == '!'  )
    {
      pCFindData->szStatusLine[0] = ' ' ;
      SETTEXT( pCFindData->hwndFindDlg, ID_TB_CFIND_STATUS_TEXT, pCFindData->szStatusLine );
      WinMessageBox( HWND_DESKTOP, pCFindData->hwndFindDlg, pCFindData->szStatusLine, "Concordance Search Error", 9998, MB_ERROR );
    }
    else
    {
      SETTEXT( pCFindData->hwndFindDlg, ID_TB_CFIND_STATUS_TEXT, pCFindData->szStatusLine );
    } /* endif */   
	
    for( i = 0; i < MAX_FOUNDMATCHES; i++ )
    {
      if ( pCFindData->FoundMatch[i].fUsed )
      {
        CFindInsertMatch( hwnd, pCFindData, i );
        pCFindData->FoundMatch[i].fUsed = FALSE;
      } /* endif */
    } /* endfor */
    if ( pCFindData->ThreadState == CST_SEARCHDONE )
    {
      CFindProcessStop( pCFindData );
    } /* endif */
    pCFindData->fTimerProcessing = FALSE;
  } /* endif */
}


/*////////////////////////////////////////////////////////////////////////////
:h2.EQFBCFindCommand - process WM_COMMAND messages of find dialog
*/
// Description:
//    Handle WM_COMMAND messages (= pressing of pushbuttons) of
//    find dialog panel.
//
//   Flow (message driven):
//      case 'Find' pushbutton:
//         update callers CFINDDATA structure with actual values;
//         post a WM_CLOSE message to dialog, mp1 = FIND;
//      case 'Change' pushbutton:
//         update callers CFINDDATA structure with actual values;
//         post a WM_CLOSE messagge to dialog, mp1 = CHANGE;
//      case CANCEL pushbutton or DID_CANCEL (= ESCAPE key):
//         post a WM_CLOSE messgae to dialog, mp1 = 0;
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
//   - callers CFINDDATA structure is updated if required
//
// External references:
//   None
//
//////////////////////////////////////////////////////////////////////////////
MRESULT EQFBCFindCommand
(
  HWND hwndDlg,
  WPARAM mp1,
  LPARAM mp2
)
{
  MRESULT mResult = MRFROMSHORT(TRUE);     // TRUE = command is processed
  PCFINDDATA  pCFindData;                    // pointer to find data
  PTBDOCUMENT  pDoc;                       // pointer to document

  mp2;
  pCFindData = ACCESSDLGIDA(hwndDlg, PCFINDDATA );
  pDoc = pCFindData->pDoc;

  switch ( WMCOMMANDID( mp1, mp2 ) )
  {

    case ID_TB_CFIND_HELP_PB:
      mResult = UtlInvokeHelp();
      break;

    case ID_TB_CFIND_FIND_PB:              // find selected value
      EQFBCFindFill ( hwndDlg, pCFindData);// get data from dialog
      if ( pCFindData->chFind[0] != 0 )
      {
        int iTimer = 0;
        pCFindData->fSearching = TRUE;
        pCFindData->iCurSearchOffs = 0;
        SETTEXTW( hwndDlg, ID_TB_CFIND_RESULT_EF, L"" );
        pCFindData->iCurLen = 0;
        pCFindData->fStop = FALSE;
        memset( pCFindData->FoundMatch, 0, sizeof(pCFindData->FoundMatch) );
        EQFBCFindButtons( hwndDlg, FALSE );
        ENABLECTRL( hwndDlg, ID_TB_CFIND_STOP_PB, TRUE );
        iTimer = SetTimer( hwndDlg, CFIND_TIMERID, (UINT)CFIND_TIMER_INTERVALL, CFindTimerProc );
        CFindStartThread( pCFindData );
      } /* endif */
      break;

    case ID_TB_CFIND_RESFIND_PB:              // find selected value in result area
      EQFBCFindFill ( hwndDlg, pCFindData);// get data from dialog
      if ( pCFindData->szResFind[0] != 0 )
      {
        HWND hwndEdit = GetDlgItem( hwndDlg, ID_TB_CFIND_RESULT_EF );
        FINDTEXTEX FindData;
        int iOffs = 0;

        memset( &FindData, 0, sizeof(FindData) );
        FindData.chrg.cpMin = pCFindData->iCurSearchOffs;
        FindData.chrg.cpMax = -1;
        FindData.lpstrText = (LPCSTR)pCFindData->szResFind;
        iOffs = SendMessage( hwndEdit, EM_FINDTEXTEXW, (pCFindData->fIgnoreCase ? 0 : FR_MATCHCASE ), (LPARAM)&FindData );
        if ( iOffs == - 1 )
        {
          sprintf( pCFindData->szBuffer, "end of result area reached", pCFindData->szResFind );
          SETTEXT( hwndDlg, ID_TB_CFIND_STATUS_TEXT, pCFindData->szBuffer );
          pCFindData->iCurSearchOffs = 0;
        }
        else
        {
          sprintf( pCFindData->szBuffer, "string \"%S\" found", pCFindData->szResFind );
          SendMessage( hwndEdit, EM_EXSETSEL, 0, (LPARAM)&(FindData.chrgText) );
          SETTEXT( hwndDlg, ID_TB_CFIND_STATUS_TEXT, pCFindData->szBuffer );
          pCFindData->iCurSearchOffs = FindData.chrgText.cpMax;
        } /* endif */
      } /* endif */
      break;

    case ID_TB_CFIND_STOP_PB:
      CFindProcessStop( pCFindData );
      break;

    case ID_TB_CFIND_CANCEL_PB:
    case DID_CANCEL:
      POSTCLOSE( hwndDlg, FALSE );
      break;

    case ID_TB_CFIND_PRIMMEM_CHK:
    case ID_TB_CFIND_ADDMEM_CHK:
      {
        BOOL fPrimMem = QUERYCHECK( hwndDlg, ID_TB_CFIND_PRIMMEM_CHK );
        BOOL fAddMem = QUERYCHECK( hwndDlg, ID_TB_CFIND_ADDMEM_CHK );
        QUERYTEXTW( hwndDlg, ID_TB_CFIND_FIND_EF, pCFindData->chFind );

        ENABLECTRL( hwndDlg, ID_TB_CFIND_MEMCHANGE_PB, fAddMem );
        ENABLECTRL( hwndDlg, ID_TB_CFIND_ADDMEM_LB, fAddMem );

        if ( fAddMem && (QUERYITEMCOUNT( hwndDlg, ID_TB_CFIND_ADDMEM_LB) == 0 ) )
        {
          fAddMem = FALSE;
        } /* endif */
        ENABLECTRL( hwndDlg, ID_TB_CFIND_FIND_PB, ( (pCFindData->chFind[0] != 0) && (fPrimMem || fAddMem)&& !pCFindData->fSearching  ));
      }
      break;

    case ID_TB_CFIND_MEMCHANGE_PB:
      {
        BOOL fPrimMem = QUERYCHECK( hwndDlg, ID_TB_CFIND_PRIMMEM_CHK );
        BOOL fAddMem = QUERYCHECK( hwndDlg, ID_TB_CFIND_ADDMEM_CHK );
        QUERYTEXTW( hwndDlg, ID_TB_CFIND_FIND_EF, pCFindData->chFind );

        CFindShowMemSelectDlg( hwndDlg, pCFindData );

        if ( fAddMem && (QUERYITEMCOUNT( hwndDlg, ID_TB_CFIND_ADDMEM_LB) == 0 ) )
        {
          fAddMem = FALSE;
        } /* endif */

        ENABLECTRL( hwndDlg, ID_TB_CFIND_FIND_PB, ( (pCFindData->chFind[0] != 0) && (fPrimMem || fAddMem)&& !pCFindData->fSearching  ));
      }
      break;

    case ID_TB_CFIND_FIND_EF:
      {
        SHORT sCommand = WMCOMMANDCMD( mp1, mp2 );
        switch ( sCommand )
        {
          case EN_KILLFOCUS:
            ClearIME( hwndDlg );
            break;
          case CBN_EDITCHANGE:
            {
              BOOL fPrimMem = QUERYCHECK( hwndDlg, ID_TB_CFIND_PRIMMEM_CHK );
              BOOL fAddMem = QUERYCHECK( hwndDlg, ID_TB_CFIND_ADDMEM_CHK );
              if ( fAddMem && (QUERYITEMCOUNT( hwndDlg, ID_TB_CFIND_ADDMEM_LB) == 0 ) )
              {
                fAddMem = FALSE;
              } /* endif */
              QUERYTEXTW( hwndDlg, ID_TB_CFIND_FIND_EF, pCFindData->chFind );
              ENABLECTRL( hwndDlg, ID_TB_CFIND_FIND_PB, ( (pCFindData->chFind[0] != 0) && (fPrimMem || fAddMem)&& !pCFindData->fSearching  ));
            }
            break;
          case CBN_SELCHANGE:
            {
              SHORT sItem = 0;
              BOOL fPrimMem = QUERYCHECK( hwndDlg, ID_TB_CFIND_PRIMMEM_CHK );
              BOOL fAddMem = QUERYCHECK( hwndDlg, ID_TB_CFIND_ADDMEM_CHK );
              CBQUERYSELECTEDITEMTEXT( sItem, hwndDlg, ID_TB_CFIND_FIND_EF, pCFindData->szBuffer );
              ENABLECTRL( hwndDlg, ID_TB_CFIND_FIND_PB, ( (pCFindData->szBuffer[0] != 0) && (fPrimMem || fAddMem)&& !pCFindData->fSearching  ));
            }
            break;
          default:
              break;
        } /*endswitch */
      if ( WMCOMMANDCMD( mp1, mp2 ) == EN_KILLFOCUS )
      {
        ClearIME( hwndDlg );
      } /* endif */
      }
      break;

    case ID_TB_CFIND_RESFIND_EF:
      {
        SHORT sCommand = WMCOMMANDCMD( mp1, mp2 );
        switch ( sCommand )
        {
          case EN_CHANGE:
            {
              QUERYTEXTW( hwndDlg, ID_TB_CFIND_RESFIND_EF, pCFindData->szResFind );
              ENABLECTRL( hwndDlg, ID_TB_CFIND_RESFIND_PB, (pCFindData->szResFind[0] != 0) );
            }
            break;
          case CBN_SELCHANGE:
            {
              SHORT sItem = 0;
              BOOL fPrimMem = QUERYCHECK( hwndDlg, ID_TB_CFIND_PRIMMEM_CHK );
              BOOL fAddMem = QUERYCHECK( hwndDlg, ID_TB_CFIND_ADDMEM_CHK );
              CBQUERYSELECTEDITEMTEXT( sItem, hwndDlg, ID_TB_CFIND_FIND_EF, pCFindData->szBuffer );
              ENABLECTRL( hwndDlg, ID_TB_CFIND_FIND_PB, ( (pCFindData->szBuffer[0] != 0) && (fPrimMem || fAddMem)&& !pCFindData->fSearching  ));
            }
            break;
          default:
              break;
        } /*endswitch */
      if ( WMCOMMANDCMD( mp1, mp2 ) == EN_KILLFOCUS )
      {
        ClearIME( hwndDlg );
      } /* endif */
      }
      break;


    default:
      mResult = FALSE;
      break;
    } /* endswitch */

  return( mResult );
} /* end of EQFBCFindCommand */


VOID EQFBCFindButtons
(
  HWND hwndDlg,                          // dialog handle
  BOOL fEnable                           // state of the buttons
)
{
  ENABLECTRL( hwndDlg, ID_TB_CFIND_FIND_EF, fEnable ); 
  ENABLECTRL( hwndDlg, ID_TB_CFIND_PRIMMEM_CHK, fEnable );
  //ENABLECTRL( hwndDlg, ID_TB_CFIND_ADDMEM_CHK, fEnable );
  //ENABLECTRL( hwndDlg, ID_TB_CFIND_ADDMEM_LB, fEnable );
  //ENABLECTRL( hwndDlg, ID_TB_CFIND_MEMCHANGE_PB, fEnable );
  ENABLECTRL( hwndDlg, ID_TB_CFIND_SOURCE_RB, fEnable );
  ENABLECTRL( hwndDlg, ID_TB_CFIND_TARGET_RB, fEnable );
  ENABLECTRL( hwndDlg, ID_TB_CFIND_IGNORECASE_CHK, fEnable );
  ENABLECTRL( hwndDlg, ID_TB_CFIND_FIND_PB, fEnable );
  ENABLECTRL( hwndDlg, ID_TB_CFIND_RESFIND_TEXT, fEnable );
  ENABLECTRL( hwndDlg, ID_TB_CFIND_RESFIND_EF, fEnable );
  ENABLECTRL( hwndDlg, ID_TB_CFIND_RESFIND_PB, fEnable );
}

// get names of selected memories
VOID CFindGetMemsFromLB
( 
  HWND hwndDlg,                       // handle of dialog window
  PCFINDDATA  pCFindData                // pointer to find data structure
)
{
  int i = 0;

  pCFindData->iAddMem = QUERYITEMCOUNT( hwndDlg, ID_TB_CFIND_ADDMEM_LB );
  memset( pCFindData->szAddMem, 0, sizeof(pCFindData->szAddMem) );
  while ( i < pCFindData->iAddMem )
  {
    QUERYITEMTEXT( hwndDlg, ID_TB_CFIND_ADDMEM_LB, i, pCFindData->szAddMem[i] );
    ANSITOOEM( pCFindData->szAddMem[i] );
    i++;
  } /*endwhile */
}

// EQFBCFindFill - fill the CFINDDATA structure with currently selected values
VOID EQFBCFindFill
(
  HWND hwndDlg,                       // handle of dialog window
  PCFINDDATA  pCFindData                // pointer to find data structure
)
{
  //    get input from Find field
  QUERYTEXTW( hwndDlg, ID_TB_CFIND_FIND_EF, pCFindData->chFind );
  wcscpy( pCFindData->chFindTarget, pCFindData->chFind );

  //    get input from Result Find field
  QUERYTEXTW( hwndDlg, ID_TB_CFIND_RESFIND_EF, pCFindData->szResFind );

  // get selected memories
  CFindGetMemsFromLB( hwndDlg, pCFindData );

  pCFindData->fPrimaryMem = QUERYCHECK( hwndDlg, ID_TB_CFIND_PRIMMEM_CHK );
  if ( QUERYCHECK( hwndDlg, ID_TB_CFIND_ADDMEM_CHK) )
  {
    int iItem = 0;
    pCFindData->iAddMem = QUERYITEMCOUNT( hwndDlg, ID_TB_CFIND_ADDMEM_LB );
    if ( pCFindData->iAddMem > MAX_ADDMEMNUM )
    {
      pCFindData->iAddMem = MAX_ADDMEMNUM;
    } /* endif */
    while ( iItem < pCFindData->iAddMem )
    {
      QUERYITEMTEXT( hwndDlg, ID_TB_CFIND_ADDMEM_LB, iItem, pCFindData->szAddMem[iItem] );
      iItem++;
    } /*endwhile */
  }
  else
  {
    pCFindData->iAddMem = 0;
  } /* endif */

  pCFindData->fIgnoreCase = (BOOL) QUERYCHECK( hwndDlg, ID_TB_CFIND_IGNORECASE_CHK );
  if ( pCFindData->fIgnoreCase )
  {
    UtlUpperW( pCFindData->chFindTarget );
  } /* endif */
  if ( QUERYCHECK( hwndDlg, ID_TB_CFIND_SOURCE_RB ) )
  {
    pCFindData->fSource = TRUE;
    pCFindData->fTarget = FALSE;
  }
  else
  {
    pCFindData->fSource = FALSE;
    pCFindData->fTarget = TRUE;
  } /* endif */

  // Save last used values                                      
  if ( pCFindData->chFind[0] != 0 )
  {
    if ( pCFindData->pLU )
    {
      // Update Search History
      {
        int iFound = -1;
        int i;

        pCFindData->chFind[MAX_SEGMENT_SIZE] = 0;

        // is insertion necessary ????
        for (i = 0; i < MAX_CFIND_SEARCH_HIST; i++ )
        {
          if (!UTF16strcmp( pCFindData->chFind, pCFindData->pLU->szHistory[i] )) iFound = i;
        }// end for

        if ( iFound >= 0)
        {
          for ( i = iFound; i < (MAX_CFIND_SEARCH_HIST-1); i++ )
          {
            wcsncpy( pCFindData->pLU->szHistory[i], pCFindData->pLU->szHistory[i+1], MAX_SEGMENT_SIZE );
          }// end for

        } // end iFound

        for ( i = (MAX_CFIND_SEARCH_HIST-1); i >= 1; i-- )
        {
          wcsncpy( pCFindData->pLU->szHistory[i], pCFindData->pLU->szHistory[i-1], MAX_SEGMENT_SIZE );
        }// end for

        wcsncpy( pCFindData->pLU->szHistory[0] , pCFindData->chFind, MAX_SEGMENT_SIZE );
      }
    } // end if

    // fill remaining fields
    pCFindData->pLU->fPrimaryMem  = pCFindData->fPrimaryMem;
    pCFindData->pLU->iAddMem =  pCFindData->iAddMem;
    memcpy( pCFindData->pLU->szAddMem, pCFindData->szAddMem, sizeof(pCFindData->pLU->szAddMem) );
    pCFindData->pLU->fIgnoreCase = pCFindData->fIgnoreCase;
    pCFindData->pLU->fSource = pCFindData->fSource;
    pCFindData->pLU->fTarget = pCFindData->fTarget;

    // save property file
    UtlWriteFileL( pCFindData->szPropFile, sizeof(CFINDLU), pCFindData->pLU, FALSE );

    // insert ID_TB_CFIND_FIND_EF
    if ( pCFindData->chFind[0] != EOS )
    {
      CBDELETEALL( hwndDlg, ID_TB_CFIND_FIND_EF );
      CBINSERTITEMENDW( hwndDlg,ID_TB_CFIND_FIND_EF , pCFindData->chFind );
      CBSELECTITEM (hwndDlg, ID_TB_CFIND_FIND_EF , 0 );
    } /* endif */

    // Insert History into ID_TB_CFIND_FIND_EF
    {
      int i=0;

      while (i < MAX_CFIND_SEARCH_HIST && pCFindData->pLU->szHistory[i][0] != EOS)
      {
        SHORT sItem = CBSEARCHITEMW(hwndDlg, ID_TB_CFIND_FIND_EF, pCFindData->pLU->szHistory[i]);

        if (sItem == LIT_NONE)
        {
          CBINSERTITEMENDW( hwndDlg,ID_TB_CFIND_FIND_EF , pCFindData->pLU->szHistory[i]);
        }
        i++;
      }// end while
    }
  } /* endif */
}



MRESULT EQFBCFindClose
(
HWND hwndDlg,
WPARAM mp1,
LPARAM mp2
)
{
  MRESULT mResult = FALSE;
  PCFINDDATA  pCFindData;                    // pointer to find data
  PTBDOCUMENT  pDoc;                       // pointer to document

  pCFindData = ACCESSDLGIDA(hwndDlg, PCFINDDATA );
  pDoc = pCFindData->pDoc;

  mp1 = mp1;                          // supress 'unreferenced parameter' msg
  mp2 = mp2;                          // supress 'unreferenced parameter' msg

  WinSetActiveWindow( HWND_DESKTOP, pDoc->hwndFrame );

  if ( pCFindData->hwndFindDlg )
  {
    HWND hwndTemp = pCFindData->hwndFindDlg;

    if ( pCFindData->fSearching )
    {
      KillTimer( pCFindData->hwndFindDlg, CFIND_TIMERID );
      CFindStopThread( pCFindData );
      pCFindData->fSearching = FALSE;
    } /* endif */         

    // save current find dialog size and position
    { 
      if ( pCFindData->pLU )
      {
        RECT rect;
        GetWindowRect( pCFindData->hwndFindDlg, &rect );
        pCFindData->pLU->swpSizePos.x = (SHORT)rect.left;
        pCFindData->pLU->swpSizePos.y = (SHORT)rect.top;
        pCFindData->pLU->swpSizePos.cx = (SHORT)(rect.right - rect.left);
        pCFindData->pLU->swpSizePos.cy = (SHORT)(rect.bottom - rect.top);
        UtlWriteFileL( pCFindData->szPropFile, sizeof(CFINDLU), pCFindData->pLU, FALSE );
      } /* endif */
    }

    DelCtrlFont( hwndTemp, ID_TB_CFIND_RESFIND_EF);

    /*****************************************************************/
    /* WinDestroyWindow forces change of active window, hence forces */
    /* show/hide sequence of find dialog, which is unnecessary...    */
    /*****************************************************************/
    pCFindData->hwndFindDlg = NULLHANDLE;
    WinDestroyWindow( hwndTemp );
  } /* endif */
  pCFindData->pDoc = NULL;


  return( mResult );
} /* end of EQFBCFindClose */

BOOL CFindCheckForAnsiConv
(
        PTBDOCUMENT pDoc
)
{
  BOOL  fAnsiToOEM = TRUE;

  if (pDoc->lf.lfCharSet == THAI_CHARSET )
  {
      fAnsiToOEM = FALSE; // in these cases ANsiToOEM / OEMToANsi will produce wrong results!
  }
  return fAnsiToOEM;
}

// insert single match and set any marks
static void CFindInsertString
( 
  HWND       hwndEdit, 
  PCFINDDATA pCFindData, 
  PSZ_W      pszType,
  PSZ_W      pszText,
  PMARKINFO  pMarks
)
{
  SETTEXTEX       SetTextOption;
  int iPrefix = 0;
  int i = 0;
  CHARRANGE cr, curRange;

  SetTextOption.codepage = 1200;
  SetTextOption.flags = ST_SELECTION;

  // position to end of text area
  cr.cpMin = -1;
  cr.cpMax = -1;
  SendMessage( hwndEdit, EM_EXSETSEL, 0, (LPARAM)&cr);

  // get current position
  SendMessage( hwndEdit, EM_EXGETSEL, 0, (LPARAM)&curRange );

  swprintf( pCFindData->szMatchBuffer, L"%s: \t%s\n", pszType, pszText );
  SendMessage( hwndEdit, EM_SETTEXTEX, (WPARAM)&SetTextOption, (LPARAM)&(pCFindData->szMatchBuffer));

  iPrefix = wcslen(pszType) + 2;

  // The selection positions are based on multibyte positions NOT in unicode (widechar) positions, so we have to check
  // for DBCS characters and adjust the positions accordingly

  while ( (i < MAX_MARKS) && (pMarks->iEnd != 0) )
  {
    int iNewOffs = WideCharToMultiByte( CP_ACP, 0, pszText, pMarks->iStart, pCFindData->szBuffer, 0, NULL, NULL );
    int iNewLen  = WideCharToMultiByte( CP_ACP, 0, pszText + pMarks->iStart, pMarks->iEnd - pMarks->iStart, pCFindData->szBuffer, 0, NULL, NULL );

//    CFindSetColor( hwndEdit, curRange.cpMin + iPrefix + pMarks->iStart + 1, curRange.cpMin + iPrefix + pMarks->iEnd + 1, RGB(0,0,0), RGB(255,255,0) );
    CFindSetColor( hwndEdit, curRange.cpMin + iPrefix + iNewOffs + 1, curRange.cpMin + iPrefix + iNewOffs + iNewLen + 1, RGB(0,0,0), RGB(255,255,0) );
    pMarks++;
    i++;
  } /*endwhile */
} /* end of function CFindInsertString */

// insert a found match into the result listbox
static void CFindInsertMatch
( 
  HWND hwndDlg, 
  PCFINDDATA pCFindData, 
  int iMatch 
)
{
  SETTEXTEX SetTextOption;
  HWND hwndRichEdit = GetDlgItem( hwndDlg, ID_TB_CFIND_RESULT_EF );
  CHARRANGE cr;

  SetTextOption.codepage = 1200;
  SetTextOption.flags = ST_SELECTION;

  // position to end of text area
  cr.cpMin = -1;
  cr.cpMax = -1;
  SendMessage(hwndRichEdit, EM_EXSETSEL, 0, (LPARAM)&cr);

  swprintf( pCFindData->szMatchBuffer, L"\n---------------------%S(%lu)------------------\n", pCFindData->FoundMatch[iMatch].szDoc,
            pCFindData->FoundMatch[iMatch].ulSegNum );
  SendMessage( hwndRichEdit, EM_SETTEXTEX, (WPARAM)&SetTextOption, (LPARAM)&(pCFindData->szMatchBuffer) );

  CFindInsertString( hwndRichEdit, pCFindData, L"Source", pCFindData->FoundMatch[iMatch].szSource, 
    pCFindData->FoundMatch[iMatch].SourceMarks );

  CFindInsertString( hwndRichEdit, pCFindData, L"Target", pCFindData->FoundMatch[iMatch].szTarget, 
    pCFindData->FoundMatch[iMatch].TargetMarks );

  SendMessage(hwndRichEdit, EM_EXSETSEL, 0, (LPARAM)&cr);

#ifdef CFIND_NO_REDRAW
  SendMessage( hwndRichEdit, WM_SETREDRAW, TRUE, 0 );
  InvalidateRect( hwndRichEdit, NULL, TRUE );
#endif
} /* end of function CFindInsertMatch*/

// set the color of a text area in the rich edit control
static void CFindSetColor
(
  HWND        hwndMLE,                 // handle of rich edit control
  int         iStart,                  // start pos for colored area
  int         iEnd,                    // end pos of colored area
  COLORREF    colorText,               // text color
  COLORREF    colorBackground          // background color
)
{
  CHARFORMAT2 Format;
  CHARRANGE Range;

  Range.cpMin = iStart;
  Range.cpMax = iEnd;
  SendMessage( hwndMLE, EM_EXSETSEL, 0, (LPARAM)&Range );
//  MLESETSELHWND( hwndMLE, iStart, iEnd );
  memset( &Format, 0, sizeof(Format) );
  Format.cbSize = sizeof(Format);
  Format.dwMask = CFM_COLOR | CFM_BACKCOLOR;
  Format.crTextColor = colorText;
  Format.crBackColor = colorBackground;
  SendMessage( hwndMLE, EM_SETCHARFORMAT, SCF_SELECTION, MP2FROMP(&Format) );
} /* end of function CFindSetColor */


// start the search thread
static void CFindStartThread( PCFINDDATA pCFindData )
{
  pCFindData->fThreadAct = (EQF_BOOL)( _beginthread( CFindSearchThread, WORKTHREAD_STACKSIZE, pCFindData ) != -1);
}


// stopy any running search thread
static void CFindStopThread( PCFINDDATA pCFindData )
{
  int iRetries = 20;

  // tell thread to stop
  pCFindData->fStop = TRUE;    
  
  // wait until thread has been stopped
  while ( (iRetries != 0) && (pCFindData->ThreadState != CST_STOPPED) )
  {
    DosSleep( 50 );
    iRetries--;
  } /*endwhile */
}

// activate next memory and set new thread state
USHORT CFindOpenMem( PCFINDDATA pCFindData )
{
  USHORT usRC = 0;

  pCFindData->ThreadState = CST_PROCESSMEM;

  if (  pCFindData->iCurMem == -1 )   // no memory yet
  {
    if ( !pCFindData->fPrimaryMem )
    {
      pCFindData->iCurMem = 0;       // try additional memories
      if ( pCFindData->iCurMem >= pCFindData->iAddMem )
      {
        // no more memories left
        pCFindData->ThreadState = CST_SEARCHDONE;
      } /* endif */
    } /* endif */
  }
  else if ( pCFindData->iCurMem >= pCFindData->iAddMem )
  {
    // no more memories left
    pCFindData->ThreadState = CST_SEARCHDONE;
    sprintf( pCFindData->szStatusLine, "Search complete, %ld occurences of string found", pCFindData->iHits );
  } /* endif */

  // open memory
  if ( pCFindData->ThreadState == CST_PROCESSMEM )
  {
    pCFindData->fFirstLookup = TRUE;

    if (  pCFindData->iCurMem == -1 )   
    {
      pCFindData->pCurrentMem = pCFindData->pPrimMem;
      strcpy( pCFindData->szCurMemPath, pCFindData->szPrimMemPath );
      strcpy( pCFindData->szCurMemName, pCFindData->szPrimMemName );
      sprintf( pCFindData->szStatusLine, "searching memory %s, 0 %% complete", pCFindData->szCurMemName );


    }
    else
    {
      // open additional memory
      int iRC = 0;;
      MemoryFactory *pFactory = MemoryFactory::getInstance();

      strcpy( pCFindData->szCurMemName, pCFindData->szAddMem[pCFindData->iCurMem] );
      pCFindData->pCurrentMem = pFactory->openMemory( NULL, pCFindData->szCurMemName, READONLYACCESS, &iRC );
      if ( iRC != 0 )
      {
        pFactory->showLastError( NULL, pCFindData->szCurMemName, NULL, NULLHANDLE );
        pCFindData->ThreadState = CST_SEARCHDONE;
        sprintf( pCFindData->szStatusLine, "!Error: Search aborted, open of memory %s failed with rc=%ld", pCFindData->szCurMemName, iRC );
        usRC = (USHORT)iRC;
      }
      else
      {
        pCFindData->fMemOpen = TRUE;
        sprintf( pCFindData->szStatusLine, "searching memory %s, 0 %% complete", pCFindData->szCurMemName );
      } /* endif */
    } /* endif */
  } /* endif */

  return( usRC );
}

static SHORT CFindStrNLFCmp
(
  PSZ_W   pData,
  PSZ_W   pSearch
)
{
  SHORT  sRc = 0;                               // init strings are equal
  CHAR_W c, d;
  while (((d = *pSearch) != NULC) && ((c = *pData) != NULC) )
  {
    if (c == d )
    {
      pData ++;
      pSearch ++;
    }
    else if ( c == LF )
    {
      pData ++;
      if (d == BLANK )
      {
        pSearch ++;
      } /* endif */
    }
    else
    {
      sRc = c-d;
      break;
    } /* endif */
  } /* endwhile */
  if (*pSearch )
  {
    sRc = -1;
  } /* endif */
  return sRc;
}

PFOUNDMATCH CFindNexFreeMatch( PCFINDDATA pCFindData )
{
  int i = 0;
  PFOUNDMATCH pMatch = pCFindData->FoundMatch;

  while ( pMatch->fUsed && (i < MAX_FOUNDMATCHES) )
  {
    pMatch++;
    i++;
  } /*endwhile */
  if ( i >= MAX_FOUNDMATCHES )
  {
    pMatch = NULL;
  } /* endif */
  return( pMatch );
}

// copy string and mask LFs and CRs
void CFindcpywoLF( PSZ_W pszTarget, PSZ_W pszSource )
{
  while ( *pszSource )
  {
    if ( (*pszSource == L'\n') || (*pszSource == L'\r') )
    {
      *pszTarget++ = L' ';
      pszSource++;
    }
    else
    {
      *pszTarget++ = *pszSource++;
    } /* endif */
  } /*endwhile */
  *pszTarget = 0;
}


USHORT CFindCheckForMatch( PCFINDDATA pCFindData )
{
  USHORT usRC = 0;
  CHAR_W chChar;
  CHAR_W c;
  BOOL   fFound = FALSE;
  BOOL   fSegEnd = FALSE;
  int iOffs = 0;
  int iLen = 0;
  if ( pCFindData->fSource )
  {
    pCFindData->pProposal->getSource( pCFindData->szSegData, sizeof(pCFindData->szSegData) / sizeof(CHAR_W) );
  }
  else
  {
    pCFindData->pProposal->getTarget( pCFindData->szSegData, sizeof(pCFindData->szSegData) / sizeof(CHAR_W) );
  } /* end */     

  if ( pCFindData->fIgnoreCase )
  {
    UtlUpperW( pCFindData->szSegData );
  } /* endif */

  PSZ_W pszSegData = pCFindData->szSegData;

  while ( !fFound && !fSegEnd )
  {
    // Scan for the 1st letter of the target string don't use strchr, it is too slow                   
    chChar = pCFindData->chFindTarget[0];
    while ( ((c = *pszSegData) != 0) && (chChar != c) )
    {
      pszSegData++;
    } /* endwhile */

    if ( *pszSegData == 0 )
    {
      fSegEnd = TRUE;
    }
    else
    {
      // then check for complete match
      if ( CFindStrNLFCmp( pszSegData, pCFindData->chFindTarget ) == 0)
      {
        fFound = TRUE;
        iOffs = pszSegData - pCFindData->szSegData;
        iLen = wcslen( pCFindData->chFindTarget );
      }
      else         // no match; go on if possible
      {
        pszSegData++;
      } /* endif */
    } /* endif */
  } /* endwhile */

  if ( fFound )
  {
    PFOUNDMATCH pMatch = CFindNexFreeMatch( pCFindData );
    if ( pMatch )
    {
      pCFindData->iHits++;
      memset( pMatch, 0, sizeof(FOUNDMATCH) );
      pMatch->fUsed = TRUE;
      if ( pCFindData->fSource )
      {
        pMatch->SourceMarks[0].iStart = iOffs;
        pMatch->SourceMarks[0].iEnd = iOffs + iLen;
      }
      else
      {
        pMatch->TargetMarks[0].iStart = iOffs;
        pMatch->TargetMarks[0].iEnd = iOffs + iLen;
      } /* endif */

      pCFindData->pProposal->getDocName( pMatch->szDoc, sizeof(pMatch->szDoc) );
      pCFindData->pProposal->getMarkup( pMatch->szMarkup, sizeof(pMatch->szMarkup) );

      pCFindData->pProposal->getSource( pCFindData->szMatchData, sizeof(pCFindData->szMatchData) / sizeof(CHAR_W) );
      CFindcpywoLF( pMatch->szSource, pCFindData->szMatchData );
      pCFindData->pProposal->getTarget( pCFindData->szMatchData, sizeof(pCFindData->szMatchData) / sizeof(CHAR_W) );
      CFindcpywoLF( pMatch->szTarget, pCFindData->szMatchData );
      pMatch->ulSegNum = pCFindData->pProposal->getSegmentNum();
      pCFindData->pProposal->getTargetLanguage( pMatch->szTgtLang, sizeof(pMatch->szTgtLang) );
    }
    else
    {
      // no more free slots...
      pCFindData->ThreadState = CST_WAITFORBUFFER;
    } /* endif */
  } /* endif */

  return( usRC );
}

USHORT CFindSearchMem( PCFINDDATA pCFindData )
{
  USHORT usRC = 0;
  int iProgress = 0;
  MemoryFactory *pFactory = MemoryFactory::getInstance();

  if ( pCFindData->fFirstLookup )
  {
    usRC = (USHORT)pCFindData->pCurrentMem->getFirstProposal( *(pCFindData->pProposal), &iProgress );
    pCFindData->fFirstLookup = FALSE;
  }
  else
  {
    usRC = (USHORT)pCFindData->pCurrentMem->getNextProposal( *(pCFindData->pProposal), &iProgress );
  } /* endif */     

  if ( usRC == NO_ERROR )
  {
    if ( iProgress != pCFindData->iComplete )
    {
      pCFindData->iComplete = iProgress;
      sprintf( pCFindData->szStatusLine, "searching memory %s, %ld %% complete", pCFindData->szCurMemName, pCFindData->iComplete );
    } /* endif */
  } /* endif */

  if ( usRC == OtmMemory::INFO_ENDREACHED )
  {
    // close memory and goto next one
    if ( pCFindData->iCurMem != -1 )
    {
      // close any open memory
      if ( pCFindData->fMemOpen )
      {
        pFactory->closeMemory( pCFindData->pCurrentMem );
        pCFindData->pCurrentMem  = NULL;
        pCFindData->fMemOpen = FALSE;
      } /* endif */
    } /* endif */
    pCFindData->iCurMem++;
    pCFindData->ThreadState = CST_NEXTMEM;
  }
  else if ( usRC == NO_ERROR)
  {
    // look for matches and add to found matches
    CFindCheckForMatch( pCFindData );
  }
  else 
  {
    sprintf( pCFindData->szStatusLine, "!Error: search in memory %s stoppend with an error code of %u", pCFindData->szCurMemName, usRC );
    pCFindData->ThreadState = CST_SEARCHDONE;
  } /* endif */

  return( usRC );
}

// background memory search thread
VOID CFindSearchThread
(
  PVOID pvCFindData
)
{
  PCFINDDATA     pCFindData = (PCFINDDATA)pvCFindData;

  pCFindData->ThreadState = CST_STARTSEARCH;
  
  while ( !pCFindData->fStop )
  {
    switch ( pCFindData->ThreadState )
    {
      case CST_STARTSEARCH:
        pCFindData->iCurMem = -1;   
        pCFindData->pCurrentMem = NULL;
        pCFindData->iHits = 0;
        CFindOpenMem( pCFindData );
        break;

      case CST_PROCESSMEM:
        CFindSearchMem( pCFindData );
        DosSleep( 0 );
        break;

      case CST_NEXTMEM:
        CFindOpenMem( pCFindData );
        DosSleep( 0 );
        break;

      case CST_WAITFORBUFFER:
        {
          int i = 0;
          DosSleep( 50 );
          while ( (i < MAX_FOUNDMATCHES) && pCFindData->FoundMatch[i].fUsed )
          {
            i++;
          } /*endwhile */
          if (i < MAX_FOUNDMATCHES )
          {
            pCFindData->ThreadState = CST_PROCESSMEM;
          } /* endif */
        }
        break;

      case CST_IDLE:
        DosSleep( 100 );
        break;

      default:
        DosSleep( 100 );
        break;
    } /*endswitch */
  } /* endwhile */

  if ( pCFindData->fMemOpen && (pCFindData->pCurrentMem != NULL))
  {
    MemoryFactory *pFactory = MemoryFactory::getInstance();
    pFactory->closeMemory( pCFindData->pCurrentMem );
    pCFindData->pCurrentMem = NULL;
    pCFindData->fMemOpen = FALSE;
  } /* endif */
  pCFindData->ThreadState = CST_STOPPED;
  pCFindData->fThreadAct = FALSE;

  _endthread();
  return;
}

// Get the window position in coordinates relative to the parent window
static BOOL GetRelativeWindowPos( HWND hwndControl, SWP FAR *pSwp )
{
  HWND             hwndParent;         // parent window of control
  POINT            pt;                 // point for coordinate conversions
  RECT             rect;               // buffer for control rectangle

  hwndParent = GetParent( hwndControl );

  GetWindowRect( hwndControl, &rect );

  /* Convert control position to relative coordinates                 */
  pt.x = rect.left;
  pt.y = rect.top;
  ScreenToClient( hwndParent, &pt );

  /* Fill caller's size and position structure                        */
  pSwp->x = (SHORT)pt.x;
  pSwp->y = (SHORT)pt.y;
  pSwp->cx = (SHORT)(rect.right - rect.left);
  pSwp->cy = (SHORT)(rect.bottom - rect.top);

  return( TRUE );
} /* end of function GetRelativeWindowPos */
