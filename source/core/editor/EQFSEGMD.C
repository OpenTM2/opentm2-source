//+----------------------------------------------------------------------------+
//| EQFSEGMD.C                                                                 |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|          Copyright (C) 1990-2014, International Business Machines          |
//|          Corporation and others. All rights reserved                       |
//|                                                                            |
//|                                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+
//+----------------------------------------------------------------------------+
//| Author: Gerhard Queck                                                      |
//|                                                                            |
//+----------------------------------------------------------------------------+
//| Description: Combines all functions and definitions related to segment     |
//|              metadata                                                      |
//|                                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+
#define INCL_EQF_MORPH            // morph settings for Arabic/Hebrew detection
#define INCL_EQF_TP               // public translation processor functions
#define INCL_EQF_EDITORAPI        // public EDITOR API functions
#define INCL_EQF_TAGTABLE         // tagtable defines and functions
#include <eqf.h>                  // General Translation Manager include file

#include "EQFTPI.H"               // Translation Processor priv. include file
#include "EQFBDLG.ID"             // IDs of dialog controls
#include "EQFMETADATA.H"          // Metadata defines
#include "eqfutmdi.h"             // MDI utilities


// metadata in hartsome XLIFF editor
//checkbox "Approved"
//combobox "needs-translation", "needs-review-translation", "new", "signed-off", "translated", "final", "needs-adaption", "needs-10n", "need-review"
//translation notes (list, header: autothor, date )

// tasks processed in MD dialog
#define REFRESH_TASK  1
#define UPDATE_TASK   2

// list of translation unit states
PSZ pszTranslStates[] = { "Needs translation", "Needs review translation", "New", "Signed-off", "Translated", "Final", 
                           "Needs adaption", "Needs review", NULL };

// list of comment styles
PSZ pszStyles[] = { "", "Source bug", "Others", "S-T Inconsistency", "Q&A", "Global Memory", NULL };



// enumeration of controls within segment properties dialog
typedef enum _SEGPROP_CONTROLS 
{
  COMMENT_BOX_CONTROL, STYLE_LABEL_CONTROL, STYLE_CBS_CONTROL, COMMENT_LABEL_CONTROL, COMMENT_EF_CONTROL, 
  CONTEXT_BOX_CONTROL, CONTEXT_EF_CONTROL, STATUS_BOX_CONTROL, STATUS_CBS_CONTROL, APPROVED_CHK_CONTROL, 
  HIST_BOX_CONTROL, CREATED_DATE_LABEL_CONTROL, CREATED_DATE_EF_CONTROL, CREATED_BY_LABEL_CONTROL, CREATED_BY_EF_CONTROL, 
  MODIFIED_DATE_LABEL_CONTROL, MODIFIED_DATE_EF_CONTROL, MODIFIED_BY_LABEL_CONTROL, MODIFIED_BY_EF_CONTROL,
  NOTES_BOX_CONTROL, NOTES_LIST_CONTROL, NOTES_ADD_PB_CONTROL, NOTES_DEL_PB_CONTROL, NOTES_SHOW_PB_CONTROL,
  LAST_CONTROL
} SEGPROP_CONTROLS;

// structure containing size info for control
typedef struct _SEGPROP_CONTROL_INFO
{
  int         id;                                // ID of control
  HWND        hwnd;                              // window handle of control
  SWP         swpOrg;                            // original window positoon and size
  SWP         swpActual;                         // actual window positoon and size
} SEGPROP_CONTROL_INFO, *PSEGPROP_CONTROL_INFO;

// must be in same order as SEGPROP_CONTROLS enumeration as this enum is used as index into this table
SEGPROP_CONTROL_INFO ControlsInfo[] =
{
  { ID_TB_MD_COMMENT_BOX, NULLHANDLE, 0, 0 },
  { ID_TB_MD_STYLE_LABEL, NULLHANDLE, 0, 0 },
  { ID_TB_MD_STYLE_CBS, NULLHANDLE, 0, 0 },
  { ID_TB_MD_COMMENT_LABEL, NULLHANDLE, 0, 0 },
  { ID_TB_MD_COMMENT_MLE, NULLHANDLE, 0, 0 },
  { ID_TB_MD_CONTEXT_BOX, NULLHANDLE, 0, 0 },
  { ID_TB_MD_CONTEXT_EF, NULLHANDLE, 0, 0 },
  { ID_TB_MD_STATUS_BOX, NULLHANDLE, 0, 0 },
  { ID_TB_MD_STATUS_CBS, NULLHANDLE, 0, 0 }, 
  { ID_TB_MD_APPROVED_CHK, NULLHANDLE, 0, 0 },
  { ID_TB_MD_HIST_BOX, NULLHANDLE, 0, 0 },
  { ID_TB_MD_CREATED_DATE_LABEL, NULLHANDLE, 0, 0 },
  { ID_TB_MD_CREATED_DATE_EF, NULLHANDLE, 0, 0 },
  { ID_TB_MD_CREATED_BY_LABEL, NULLHANDLE, 0, 0 },
  { ID_TB_MD_CREATED_BY_EF, NULLHANDLE, 0, 0 },
  { ID_TB_MD_MODIFIED_DATE_LABEL, NULLHANDLE, 0, 0 },
  { ID_TB_MD_MODIFIED_DATE_EF, NULLHANDLE, 0, 0 },
  { ID_TB_MD_MODIFIED_BY_LABEL, NULLHANDLE, 0, 0 },
  { ID_TB_MD_MODIFIED_BY_EF, NULLHANDLE, 0, 0 },
  { ID_TB_MD_NOTES_BOX, NULLHANDLE, 0, 0 },
  { ID_TB_MD_NOTES_LIST, NULLHANDLE, 0, 0 },
  { ID_TB_MD_NOTES_ADD_PB, NULLHANDLE, 0, 0 },
  { ID_TB_MD_NOTES_DEL_PB, NULLHANDLE, 0, 0 },
  { ID_TB_MD_NOTES_SHOW_PB, NULLHANDLE, 0, 0 },
  { 0, NULL, 0, 0 },
};

// metadata dialog window data
typedef struct _MD_DLG_DATA
{
  PTBDOCUMENT      pDoc;                         // active document       
  HWND             hwndDlg;                      // handle of metadata dialog
  CHAR_W           szUserID[40];                 // current user of workstation
  PMD_COMMENT_DATA pMDComment;                   // ptr to comment data area
  PMD_NOTELIST_DATA pMDNoteList;                 // ptr to notelist data area
  PMD_HIST_DATA    pMDHistory;                   // ptr to history data area
  PMD_STATUS_DATA  pMDStatus;                    // ptr to status data area
  CHAR_W           szBuffer1[1024];              // general purpose buffer 
  CHAR_W           szBuffer2[1024];              // general purpose buffer 
  BOOL             fSegModified;                 // for UPDATE_TASK only: segment has been modified
  BOOL             fXLiff;                       // TRUE = show full dialog
  BOOL             fCommentStyleChanged;         // TRUE = comment style has been changed by the user
  CHAR_W           szContext[MAX_SEGMENT_SIZE + 1];// buffer for context string
} MD_DLG_DATA, *PMD_DLG_DATA;

// global MDDialogData 
PMD_DLG_DATA pMDData = NULL;           // ptr to data structure for metadata dialog

// prototypes
MRESULT APIENTRY METADATADIALOG( HWND hwndDlg, WINMSG msg, WPARAM mp1, LPARAM mp2 );
MRESULT MDDialogCommand( HWND hwndDlg, WPARAM mp1, LPARAM mp2 );
MRESULT MDDialogClose( HWND hwndDlg, WPARAM mp1, LPARAM mp2 );
MRESULT MDDialogInit( HWND hwndDlg, WPARAM mp1, LPARAM  mp2 );
BOOL    MDUpdateMetadataFromDialog( PMD_DLG_DATA pMDData, PTBSEGMENT pSeg ); 
//MRESULT APIENTRY MDNoteDlgProc( HWND hwndDlg, WINMSG msg, WPARAM mp1, LPARAM mp2 );
MRESULT CALLBACK MDNoteDlgProc( HWND hwndDlg, WINMSG msg, WPARAM mp1, LPARAM mp2 );
void MDUtilLongToDateTimeString( LONG lTime, PSZ_W pszString, int iBufLen );
void MDShowNoteDialog( HWND hwndParent, PMD_DLG_DATA pMDData, PMD_NOTE_DATA pNote );

// externally called functions

//
// End dialog (called when TE is terminated 
//
void MDEndDialog( PTBDOCUMENT pDoc )
{
  // save "dialog was active flag"
  if ( pDoc && pMDData )
  {
    EQFINFO     ErrorInfo;       // error code of property handler calls
    PPROPFOLDERLIST pFllProp;    // ptr to folder list properties
    PVOID       hFllProp;        // handle of folder list properties
    OBJNAME     szFllObjName;    // buffer for folder list object name
    BOOL        fDialogWasActive;
    
    fDialogWasActive = (pMDData->hwndDlg != NULLHANDLE) && WinIsWindowVisible( pMDData->hwndDlg );

    UtlMakeEQFPath( szFllObjName, NULC, SYSTEM_PATH, NULL );
    strcat( szFllObjName, BACKSLASH_STR );
    strcat( szFllObjName, DEFAULT_FOLDERLIST_NAME );
    hFllProp = OpenProperties( szFllObjName, NULL, PROP_ACCESS_READ, &ErrorInfo );
    if ( hFllProp )
    {
      if ( SetPropAccess( hFllProp, PROP_ACCESS_WRITE) )
      {
        pFllProp = (PPROPFOLDERLIST)MakePropPtrFromHnd( hFllProp );
        pFllProp->fSegPropDialogActive = (EQF_BOOL)fDialogWasActive;
        SaveProperties( hFllProp, &ErrorInfo);
      } /* endif */
      CloseProperties( hFllProp, PROP_QUIT, &ErrorInfo );
    } /* endif */
  } /* endif */

  // end dialog
  if ( pDoc && pMDData && (pMDData->pDoc == pDoc ) && pMDData->hwndDlg )
  {
    MDDialogClose( pMDData->hwndDlg, 0, 0L );
  } /* endif */
}

//
// Return "dialog was active flag"
//
BOOL MDDialogWasActive()
{
  EQFINFO     ErrorInfo;       // error code of property handler calls
  PPROPFOLDERLIST pFllProp;    // ptr to folder list properties
  PVOID       hFllProp;        // handle of folder list properties
  OBJNAME     szFllObjName;    // buffer for folder list object name
  BOOL        fDialogWasActive = FALSE;
  
  UtlMakeEQFPath( szFllObjName, NULC, SYSTEM_PATH, NULL );
  strcat( szFllObjName, BACKSLASH_STR );
  strcat( szFllObjName, DEFAULT_FOLDERLIST_NAME );
  hFllProp = OpenProperties( szFllObjName, NULL, PROP_ACCESS_READ, &ErrorInfo );
  if ( hFllProp )
  {
    pFllProp = (PPROPFOLDERLIST)MakePropPtrFromHnd( hFllProp );
    fDialogWasActive = pFllProp->fSegPropDialogActive;
    CloseProperties( hFllProp, PROP_QUIT, &ErrorInfo );
  } /* endif */
  return( fDialogWasActive );
}

//
// Hide / Show dialog 
//
void MDShowSegPropDialog( BOOL fShow )
{
  fShow;

  if ( pMDData && pMDData->hwndDlg )
  {
    WinShowWindow( pMDData->hwndDlg, FALSE);
  } /* endif */
}

//
// Start metadata dialog
//
BOOL MDStartDialog( PTBDOCUMENT pDoc )
{
  if ( !pMDData )
  {
    UtlAlloc( (PVOID *) &pMDData, 0L, (LONG) sizeof(MD_DLG_DATA), ERROR_STORAGE );
    if ( pMDData )
    {
    } /* endif */
  } /* endif */

  if ( pMDData )
  {
    PLOADEDTABLE pTable = (PLOADEDTABLE)pDoc->pDocTagTable;
    pMDData->fXLiff = (strcmp( pTable->szName, "EQFXLIFF" ) == 0 );

    if ( (pMDData->pDoc != pDoc) && pMDData->hwndDlg )
    {
      // end any metadata dialog for another document
      MDDialogClose( pMDData->hwndDlg, 0, 0L );
    } /* endif */

    pMDData->pDoc = pDoc;                     // set active document

    if ( !pMDData->hwndDlg )
    {
      FARPROC lpfnProc = MakeProcInstance(  (FARPROC)(METADATADIALOG), (HAB) UtlQueryULong( QL_HAB ) );
      pMDData->hwndDlg = CreateDialogParamW( hResMod, (PSZ_W)MAKEINTRESOURCE( ID_TB_METADATA_DLG),
                                            pDoc->hwndClient, (DLGPROC)lpfnProc,
                                            (LPARAM)pMDData );
      //pMDData->hwndDlg = CreateMDIDialogParam( hResMod, MAKEINTRESOURCE(DISP_ENTRY_DLG), pDoc->hwndClient,
      //                                         (DLGPROC)lpfnProc, (LPARAM)pMDData , FALSE,
      //                                          (HPOINTER) UtlQueryULong(QL_DICTENTRYDISPICO)); //hiconDICTDISP );

      FreeProcInstance( lpfnProc );
    } /* endif */

    if ( pMDData->hwndDlg )
    {
      WinSetWindowPos( pMDData->hwndDlg, HWND_TOP, 0,0,0,0, EQF_SWP_ACTIVATE | EQF_SWP_SHOW );
    } /* endif */
  } /* endif */

  return( (pMDData != NULL) && (pMDData->hwndDlg!= NULLHANDLE) ); 
} /* end of function MDStartDialog */

//
// Refresh metadata in metadata dialog
//
BOOL MDRefreshMetadata( PTBDOCUMENT pDoc, PTBSEGMENT pSeg, PSZ_W pszDisplayContext  )
{
  if ( pMDData )
  {
    if ( (pMDData->pDoc == pDoc) && pMDData->hwndDlg )
    {
      PLOADEDTABLE pTable = (PLOADEDTABLE)pDoc->pDocTagTable;
      pMDData->fXLiff = (strcmp( pTable->szName, "EQFXLIFF" ) == 0 );
      if ( pszDisplayContext  )
      {
        wcscpy( pMDData->szContext, pszDisplayContext );
      }
      else
      {
        pMDData->szContext[0] = 0;
      } /* endif */         
      SendMessage( pMDData->hwndDlg, WM_EQF_PROCESSTASK, REFRESH_TASK, MP2FROMP( pSeg ) );
    } /* endif */
  } /* endif */

  return( TRUE );
}

//
// Get current metadata from metadata dialog
//
BOOL MDGetMetadata( PTBDOCUMENT pDoc, PTBSEGMENT pSeg, BOOL fSegModified  )
{
  if ( pMDData )
  {
    if ( (pMDData->pDoc == pDoc) && pMDData->hwndDlg )
    {
      pMDData->fSegModified = fSegModified;
      SendMessage( pMDData->hwndDlg, WM_EQF_PROCESSTASK, UPDATE_TASK, MP2FROMP( pSeg ) );
    } /* endif */
  } /* endif */
  return( TRUE );
}

//
// check if comment data has been changed by the user
//
BOOL MDCommentHasChanged( PTBDOCUMENT pDoc )
{
  BOOL fChanged = FALSE;

  if ( pMDData )
  {
    if ( (pMDData->pDoc == pDoc) && pMDData->hwndDlg )
    {
      fChanged = pMDData->fCommentStyleChanged || SendDlgItemMessage( pMDData->hwndDlg, ID_TB_MD_COMMENT_MLE , EM_GETMODIFY, 0, 0 );
    } /* endif */
  } /* endif */

  return( fChanged );
}

//
// Check if metadata dialog is visible
//
BOOL MDIsWindowVisible()
{
  BOOL  fVisible = FALSE;

  if ( pMDData )
  {
    if ( pMDData->hwndDlg )
    {
      fVisible = WinIsWindowVisible( pMDData->hwndDlg );
    } /* endif */
  } /* endif */

  return( fVisible );

}

//
// refresh status info
//
void MDDialogRefreshStatus( HWND hwndDlg, PMD_DLG_DATA pMDData )
{
  if ( (pMDData != NULL) && (pMDData->pMDStatus != NULL) )
  {
    SETTEXTW( hwndDlg, ID_TB_MD_STATUS_CBS, pMDData->pMDStatus->szStatus );
    SETCHECK( hwndDlg, ID_TB_MD_APPROVED_CHK, pMDData->pMDStatus->fApproved );
  }
  else
  {
    SETTEXT( hwndDlg, ID_TB_MD_STATUS_CBS, "" );
    SETCHECK_FALSE( hwndDlg, ID_TB_MD_APPROVED_CHK );
  } /* endif */
}

//
// refresh comment info
//
void MDDialogRefreshComment( HWND hwndDlg, PMD_DLG_DATA pMDData )
{
  if ( (pMDData != NULL) && (pMDData->pMDComment != NULL) )
  {
    CHAR_W szStyle[40];
    SHORT sItem;
    // GQ: we have to copy the style to a new buffer, sometimes the direct access to pMDData->pMDComment->szStyle fails... 
    wcscpy( szStyle, pMDData->pMDComment->szStyle ); 
    sItem = (SHORT)SendDlgItemMessageW( hwndDlg, ID_TB_MD_STYLE_CBS, CB_FINDSTRINGEXACT, 0, (LPARAM)szStyle ); 
    if( sItem != CB_ERR ) 
    { 
      SendDlgItemMessage( hwndDlg, ID_TB_MD_STYLE_CBS, CB_SETCURSEL, sItem, 0L );  
    } 
    pMDData->fCommentStyleChanged = FALSE;
    //SETTEXTW( hwndDlg, ID_TB_MD_COMMENT_MLE, pMDData->pMDComment->szComment );
    {
      SETTEXTEX       SetTextOption;

      SetTextOption.codepage = 1200;
      SetTextOption.flags = ST_DEFAULT;
      SendDlgItemMessage( hwndDlg, ID_TB_MD_COMMENT_MLE , EM_SETTEXTEX, (WPARAM)&SetTextOption, (LPARAM)&(pMDData->pMDComment->szComment));
      SendDlgItemMessage( hwndDlg, ID_TB_MD_COMMENT_MLE , EM_SETMODIFY, (WPARAM)FALSE, (LPARAM)0 );
    }
  }
  else
  {
    SendDlgItemMessage( hwndDlg, ID_TB_MD_STYLE_CBS, CB_SETCURSEL, 0, 0L );  
    SETTEXT( hwndDlg, ID_TB_MD_COMMENT_MLE, "" );
  } /* endif */
}


//
// refresh history info
//
void MDDialogRefreshHistory( HWND hwndDlg, PMD_DLG_DATA pMDData )
{
  if ( (pMDData != NULL) && (pMDData->pMDHistory != NULL) )
  {
    MDUtilLongToDateTimeString( pMDData->pMDHistory->lCreationDate, pMDData->szBuffer1, sizeof(pMDData->szBuffer1) );
    SETTEXTW( hwndDlg, ID_TB_MD_CREATED_DATE_EF, pMDData->szBuffer1 );
    SETTEXTW( hwndDlg, ID_TB_MD_CREATED_BY_EF, pMDData->pMDHistory->szCreator  );
    MDUtilLongToDateTimeString( pMDData->pMDHistory->lModificationDate, pMDData->szBuffer1, sizeof(pMDData->szBuffer1) );
    SETTEXTW( hwndDlg, ID_TB_MD_MODIFIED_DATE_EF, pMDData->szBuffer1  );
    SETTEXTW( hwndDlg, ID_TB_MD_MODIFIED_BY_EF, pMDData->pMDHistory->szModifier  );
  }
  else
  {
    SETTEXT( hwndDlg, ID_TB_MD_CREATED_DATE_EF, "" );
    SETTEXT( hwndDlg, ID_TB_MD_CREATED_BY_EF, "" );
    SETTEXT( hwndDlg, ID_TB_MD_MODIFIED_DATE_EF, "" );
    SETTEXT( hwndDlg, ID_TB_MD_MODIFIED_BY_EF, "" );
  } /* endif */
}

//
// refresh list of notes
//
void MDDialogRefreshNotes( HWND hwndDlg, PMD_DLG_DATA pMDData )
{
  // clear note list
  DELETEALL( hwndDlg, ID_TB_MD_NOTES_LIST );

  if ( pMDData == NULL ) return;

  // fill list with notes

  if ( pMDData->pMDNoteList )
  {
    PMD_NOTE_DATA pNote = (PMD_NOTE_DATA)(((PBYTE)(pMDData->pMDNoteList)) + sizeof(MD_NOTELIST_DATA));
    int i = 0;
    while ( i < pMDData->pMDNoteList->lNumberOfNotes )
    {
      SHORT sItem = LIT_NONE;

      // setup listbox text for note
      if ( pNote->szAuthor[0] != 0 )
      {
        wcscpy( pMDData->szBuffer1, pNote->szAuthor );
      }
      else
      {
        wcscpy( pMDData->szBuffer1, L"n/a" );
      } /* endif */
      wcscat( pMDData->szBuffer1, L" : " );

  
      // copy up to 40 characters of note text to buffer
      {
        int iChars = 100;
        PSZ_W pszTemp = pMDData->szBuffer1 + wcslen(pMDData->szBuffer1);
        PSZ_W pszNoteText = pNote->szNote;
        while ( iChars && *pszNoteText)
        {
          if ( *pszNoteText == L'\r' )
          {
            // ignore this one
            pszNoteText++;
          }
          else if ( *pszNoteText == L'\n' )
          {
            // replace with blank
            *pszTemp++ = L' ';
            pszNoteText++;
            iChars--;
          }
          else
          {
            *pszTemp++ = *pszNoteText++;
            iChars--;
          } /* endif */
        } /*endwhile */
        *pszTemp = 0;
      }

      // add to listbox and set item handle to note data
      sItem = INSERTITEMENDW( hwndDlg, ID_TB_MD_NOTES_LIST, pMDData->szBuffer1 );
      if ( sItem != LIT_NONE )
      {
        SETITEMHANDLE( hwndDlg, ID_TB_MD_NOTES_LIST, sItem, pNote );
      } /* endif */

      // continue with next note
      pNote = (PMD_NOTE_DATA)( ((PBYTE)pNote) + pNote->lSize);
      i++;
    } /*endwhile */
  } /* endif */
} /* MDDialogRefreshNotes */

static FARPROC pfnOrgComboBoxProc;        // original combo box window procedure

//
// Subclass procedure for our comboboxes
//
LONG FAR PASCAL ComboBoxSubclassProc
(
  HWND hwnd,
  WINMSG msg,
  WPARAM mp1,
  LPARAM mp2
)
{
  //MRESULT mResult;
  //HWND    hwndDlg;

  switch ( msg )
  {
    case WM_KEYDOWN:
      switch ( mp1 )
      {
        case VK_ESC:
//            PostMessage( hwndDlg, WM_EQF_COMMAND, MP1FROMSHORT(LOOKUP_PUBO_OK), 0L );
//          return();
//      MessageBox( hwndDlg, "Escape has been pressed", "Info", MB_OK );
  	break;


          break;

      } /* endswitch */
      break;

  } /* endswitch */
  return( CallWindowProc( (WNDPROC)pfnOrgComboBoxProc, hwnd, msg, mp1, mp2 ) );
} /* end of function ComboBoxSubclassProc */




//
// dialog procedure for segment metadata window
//
MRESULT APIENTRY METADATADIALOG
(
  HWND hwndDlg,                                  // handle of dialog window 
  WINMSG msg,
  WPARAM mp1,
  LPARAM mp2
)
{
  MRESULT  mResult = MRFROMSHORT( FALSE );      // result value of procedure

  switch ( msg )
  {
  case WM_EQF_QUERYID: HANDLEQUERYID( ID_TB_METADATA_DLG, mp2 ); break;

  case WM_INITDLG:
    SETWINDOWID( hwndDlg, ID_TB_METADATA_DLG );
    mResult = DIALOGINITRETURN( MDDialogInit( hwndDlg, mp1, mp2 ));
    UtlRegisterModelessDlg( hwndDlg );

    // get initial sizes of controls
    {
      int i = 0;
      while ( ControlsInfo[i].id )
      {
        RECT rect;
        POINT pt;

        ControlsInfo[i].hwnd = GetDlgItem( hwndDlg, ControlsInfo[i].id );  
        GetWindowRect( ControlsInfo[i].hwnd, &rect );
        pt.x = rect.left;
        pt.y = rect.top;
        ScreenToClient( hwndDlg, &pt );
        ControlsInfo[i].swpOrg.x   = (SHORT)pt.x;
        ControlsInfo[i].swpOrg.y   = (SHORT)pt.y;
        ControlsInfo[i].swpOrg.cx  = (SHORT)(rect.right - rect.left);
        ControlsInfo[i].swpOrg.cy  = (SHORT)(rect.bottom - rect.top);
        i++;
      } /*endwhile */

      // set initial size
      SetWindowPos( hwndDlg, HWND_TOP, 0, 0, ControlsInfo[CONTEXT_BOX_CONTROL].swpOrg.x + ControlsInfo[CONTEXT_BOX_CONTROL].swpOrg.x + 20 ,
                    ControlsInfo[CONTEXT_BOX_CONTROL].swpOrg.y + ControlsInfo[CONTEXT_BOX_CONTROL].swpOrg.cy + 20, SWP_NOMOVE | SWP_NOZORDER );
      SetCtrlFnt ( hwndDlg, GetCharSet(), ID_TB_MD_COMMENT_MLE, 0);

    }

    // subclass combobox edit field to catch escape key
    {
      POINT pt;
      //char szBuf[80];
      HWND hwndCombo;
      HWND hwndEdit;

      pt.x = 1; pt.y = 1;
      hwndCombo = GetDlgItem( hwndDlg, ID_TB_MD_STYLE_CBS );
      //sprintf( szBuf, "hwndCombo is %lu", (ULONG)hwndCombo );
      //MessageBox( hwndDlg, szBuf, "Info", MB_OK );
      //hwndEdit = GetWindow( hwndCombo, GW_CHILD );
      //sprintf( szBuf, "hwndEdit using GetWindow(..., GW_CHILD) is %lu", (ULONG)hwndEdit );
      //MessageBox( hwndDlg, szBuf, "Info", MB_OK );
      //hwndEdit = FindWindowEx( hwndCombo, NULL, "Edit", NULL);
      //sprintf( szBuf, "hwndEdit using FindWindowEx(..., EDIT) is %lu", (ULONG)hwndEdit );
      //MessageBox( hwndDlg, szBuf, "Info", MB_OK );
      hwndEdit = ChildWindowFromPoint( hwndCombo, pt );
      //sprintf( szBuf, "hwndEdit using ChildWindowFromPoint(...) is %lu", (ULONG)hwndEdit );
      //MessageBox( hwndDlg, szBuf, "Info", MB_OK );

      pfnOrgComboBoxProc = (FARPROC)SetWindowLong( hwndEdit, GWL_WNDPROC, (LONG)ComboBoxSubclassProc );
     }
    break;

  case WM_EQF_PROCESSTASK:
    switch ( mp1 )
    {
      case REFRESH_TASK:
        {
          PTBSEGMENT pSeg = (PTBSEGMENT)mp2;
          PMD_METADATA  pMetaData = (PMD_METADATA)pSeg->pvMetadata;
          BOOL fHistData = FALSE;                // TRUE = history data in metadata area

          // free all dialog related metadata structures
          if ( pMDData->pMDComment )  UtlAlloc( (PVOID *)&(pMDData->pMDComment), 0, 0, NOMSG );
          if ( pMDData->pMDNoteList ) UtlAlloc( (PVOID *)&(pMDData->pMDNoteList), 0, 0, NOMSG );
          if ( pMDData->pMDHistory )  UtlAlloc( (PVOID *)&(pMDData->pMDHistory), 0, 0, NOMSG );
          if ( pMDData->pMDStatus )   UtlAlloc( (PVOID *)&(pMDData->pMDStatus), 0, 0, NOMSG );

          SETTEXTW( hwndDlg, ID_TB_MD_CONTEXT_EF, pMDData->szContext );
 
          if ( pMetaData != NULL )
          {

            // refresh dialog fields with metadata from segment
            PMD_ELEMENT_HDR pCurElement = MDGetFirstElement( pMetaData );
            while ( pCurElement && (pCurElement->Type != MD_ENDOFLIST_TYPE) )
            {
              switch ( pCurElement->Type )
              {
                case MD_NOTELIST_TYPE:
                  if ( UtlAlloc( (PVOID *)&(pMDData->pMDNoteList), 0, pCurElement->lSize, ERROR_STORAGE ) )
                  {
                    memcpy( pMDData->pMDNoteList, pCurElement, pCurElement->lSize );
                  } /* endif */
                  break;
                case MD_HISTORY_TYPE:
                  if ( UtlAlloc( (PVOID *)&(pMDData->pMDHistory), 0, pCurElement->lSize, ERROR_STORAGE ) )
                  {
                    memcpy( pMDData->pMDHistory, pCurElement, pCurElement->lSize );
                    fHistData = TRUE;
                  } /* endif */
                  break;
                case MD_STATUS_TYPE:
                  if ( UtlAlloc( (PVOID *)&(pMDData->pMDStatus), 0, pCurElement->lSize, ERROR_STORAGE ) )
                  {
                    memcpy( pMDData->pMDStatus, pCurElement, pCurElement->lSize );
                  } /* endif */
                  break;
                case MD_COMMENT_TYPE:
                  if ( UtlAlloc( (PVOID *)&(pMDData->pMDComment), 0, pCurElement->lSize, ERROR_STORAGE ) )
                  {
                    memcpy( pMDData->pMDComment, pCurElement, pCurElement->lSize );
                  } /* endif */
                  break;
                default:
                  break;
              } /*endswitch */
              pCurElement = MDGetNextElement( pCurElement );
            } /*endwhile */
          } /* endif */

          MDDialogRefreshComment( hwndDlg, pMDData );
          MDDialogRefreshNotes( hwndDlg, pMDData );
          MDDialogRefreshHistory( hwndDlg, pMDData );
          MDDialogRefreshStatus( hwndDlg, pMDData );


          // hide / show XLIFF based controls
          if ( pMDData->fXLiff )
          {
            SHOWCONTROL( hwndDlg, ID_TB_MD_NOTES_BOX );
            SHOWCONTROL( hwndDlg, ID_TB_MD_NOTES_LIST );
            SHOWCONTROL( hwndDlg, ID_TB_MD_NOTES_ADD_PB );
            SHOWCONTROL( hwndDlg, ID_TB_MD_NOTES_DEL_PB );
            SHOWCONTROL( hwndDlg, ID_TB_MD_NOTES_SHOW_PB );
            SHOWCONTROL( hwndDlg, ID_TB_MD_STATUS_CBS );
          }
          else
          {
            HIDECONTROL( hwndDlg, ID_TB_MD_NOTES_BOX );
            HIDECONTROL( hwndDlg, ID_TB_MD_NOTES_LIST );
            HIDECONTROL( hwndDlg, ID_TB_MD_NOTES_ADD_PB );
            HIDECONTROL( hwndDlg, ID_TB_MD_NOTES_DEL_PB );
            HIDECONTROL( hwndDlg, ID_TB_MD_NOTES_SHOW_PB );
            HIDECONTROL( hwndDlg, ID_TB_MD_STATUS_CBS );
          } /* endif */             
 
        }
        break;
      case UPDATE_TASK:
        {
          PMD_DLG_DATA  pMDData = ACCESSDLGIDA( hwndDlg, PMD_DLG_DATA );
          PTBSEGMENT pSeg = (PTBSEGMENT)mp2;
          if ( pMDData && pSeg ) MDUpdateMetadataFromDialog( pMDData, pSeg ); 
        }
        break;
      default:
        break;
    } /*endswitch */

  case WM_COMMAND:
    mResult = MDDialogCommand( hwndDlg, mp1, mp2 );
    break;

  case WM_CHAR:
    {
      USHORT usFunction = 0;
      UCHAR ucState = 0;

      if ( EQFBMapKey( msg, mp1, mp2, &usFunction, &ucState, TPRO_MAPKEY) )
      {
      } /* endif */
    }
    break;

  case WM_CLOSE:
    mResult = MDDialogClose( hwndDlg, mp1, mp2 );
    break;

 case WM_ACTIVATE:
    if ( mp1 )
    {
      PMD_DLG_DATA  pMDData = NULL;             // pointer to find data
      PTBDOCUMENT   pDoc = NULL;                // pointer to document

      pMDData = ACCESSDLGIDA( hwndDlg, PMD_DLG_DATA );
      if ( pMDData )
      {
        pDoc = pMDData->pDoc;

        //// bring parent of find dialog on top...
        //if ( pDoc )
        //{
        //  BringWindowToTop( pDoc->hwndFrame );
        //} /* endif */
        //BringWindowToTop( hwndDlg );
      } /* endif */
      //WinPostMsg( hwndDlg, WM_EQF_SETFOCUS, 0, MP2FROMHWND( WinWindowFromID(hwndDlg, 4711 )));
    } /* endif */
    mResult = FALSE;
    break;

  case WM_EQF_SETFOCUS:
    // set focus to entry field and select it to ease-up working after reactivation...
    //SETFOCUS( hwndDlg, 4711 );
    //SETEFSEL( hwndDlg, 4711, 0, -1 );
    break;

  case WM_DESTROY:
    UtlUnregisterModelessDlg( hwndDlg );
    break;

  case WM_GETMINMAXINFO:
    {
      PMD_DLG_DATA  pMDData = ACCESSDLGIDA( hwndDlg, PMD_DLG_DATA );
      MINMAXINFO FAR *lpMinMax;      // ptr to min/max info structure
      int yBorder = WinQuerySysValue( HWND_DESKTOP, SV_CYSIZEBORDER );
      int yTitle = WinQuerySysValue( HWND_DESKTOP, SV_CYTITLEBAR );

      lpMinMax = (MINMAXINFO *)PVOIDFROMMP2(mp2);
      if ( pMDData->fXLiff  )
      {
        lpMinMax->ptMinTrackSize.x = ControlsInfo[STATUS_BOX_CONTROL].swpOrg.x + ControlsInfo[STATUS_BOX_CONTROL].swpOrg.cx + 10;
        lpMinMax->ptMinTrackSize.y = ControlsInfo[NOTES_BOX_CONTROL].swpOrg.y + ControlsInfo[NOTES_BOX_CONTROL].swpOrg.cy + 10;
      }
      else
      {
        lpMinMax->ptMinTrackSize.x = ControlsInfo[CONTEXT_BOX_CONTROL].swpOrg.x + ControlsInfo[CONTEXT_BOX_CONTROL].swpOrg.cx + 10;
        lpMinMax->ptMinTrackSize.y = ControlsInfo[CONTEXT_BOX_CONTROL].swpOrg.y + ControlsInfo[CONTEXT_BOX_CONTROL].swpOrg.cy + yTitle + yBorder + yBorder;
        lpMinMax->ptMaxTrackSize.x = 9999;
        lpMinMax->ptMaxTrackSize.y = lpMinMax->ptMinTrackSize.y;
      } /* endif */         
    }
    break;

  case WM_SIZE :
    // resize inner window only if normal sizing request...
    if ( (mp1 == SIZENORMAL) || (mp1 == SIZEFULLSCREEN) )
    {
      PMD_DLG_DATA  pMDData = ACCESSDLGIDA( hwndDlg, PMD_DLG_DATA );
      int   iWidth  = LOWORD( mp2 );      // new width of dialog
      int   iHeight = HIWORD( mp2 );      // new height of dialog

      if ( pMDData )
      {
        int iControls = 0;
        int i = 0;

        // copy original pos and size to actal pos and size
        while ( ControlsInfo[i].id )
        {
          memcpy( &(ControlsInfo[i].swpActual), &(ControlsInfo[i].swpOrg), sizeof(SWP) );
          i++; iControls++;
        } /*endwhile */

        // compute dialog inner width and heigth
        iWidth = iWidth - (2 * GetSystemMetrics( SM_CXFRAME ));
        iHeight = iHeight - (2 * GetSystemMetrics( SM_CYFRAME ));

        // adjust width of controls
        {
          // start with groupboxes
          int iNewCX = iWidth - (2 * ControlsInfo[STATUS_BOX_CONTROL].swpActual.x);
          if ( iNewCX > 0 )
          {
            ControlsInfo[STATUS_BOX_CONTROL].swpActual.cx = (SHORT)iNewCX;
            ControlsInfo[CONTEXT_BOX_CONTROL].swpActual.cx = (SHORT)iNewCX;
            ControlsInfo[HIST_BOX_CONTROL].swpActual.cx = (SHORT)iNewCX;
            ControlsInfo[NOTES_BOX_CONTROL].swpActual.cx = (SHORT)iNewCX;
            ControlsInfo[COMMENT_BOX_CONTROL].swpActual.cx = (SHORT)iNewCX;
          } /* endif */

          // adjust width of some inner controls
          ControlsInfo[STYLE_CBS_CONTROL].swpActual.cx = ControlsInfo[COMMENT_BOX_CONTROL].swpActual.cx - 
                                                          ControlsInfo[STYLE_CBS_CONTROL].swpActual.x +
                                                          ControlsInfo[COMMENT_BOX_CONTROL].swpActual.x - 12; 
          ControlsInfo[COMMENT_EF_CONTROL].swpActual.cx = ControlsInfo[COMMENT_BOX_CONTROL].swpActual.cx - 
                                                          ControlsInfo[COMMENT_EF_CONTROL].swpActual.x +
                                                          ControlsInfo[COMMENT_BOX_CONTROL].swpActual.x - 12; 
          ControlsInfo[CONTEXT_EF_CONTROL].swpActual.cx = ControlsInfo[CONTEXT_EF_CONTROL].swpActual.cx - 
                                                          ControlsInfo[CONTEXT_EF_CONTROL].swpActual.x +
                                                          ControlsInfo[CONTEXT_EF_CONTROL].swpActual.x - 12; 
 
          ControlsInfo[STATUS_CBS_CONTROL].swpActual.cx = ControlsInfo[STATUS_BOX_CONTROL].swpActual.cx - 
                                                          ControlsInfo[STATUS_CBS_CONTROL].swpActual.x +
                                                          ControlsInfo[STATUS_BOX_CONTROL].swpActual.x - 12; 
          ControlsInfo[NOTES_LIST_CONTROL].swpActual.cx = ControlsInfo[NOTES_BOX_CONTROL].swpActual.cx - 
                                                          ControlsInfo[NOTES_LIST_CONTROL].swpActual.x +
                                                          ControlsInfo[NOTES_BOX_CONTROL].swpActual.x - 12;

          // distribute notes buttons within available space
          {
            int iGaps = ControlsInfo[NOTES_BOX_CONTROL].swpActual.cx - ControlsInfo[NOTES_ADD_PB_CONTROL].swpActual.cx -
                        ControlsInfo[NOTES_DEL_PB_CONTROL].swpActual.cx - ControlsInfo[NOTES_SHOW_PB_CONTROL].swpActual.cx;
            if ( iGaps >= 4)
            {
              iGaps = iGaps / 4;
              ControlsInfo[NOTES_ADD_PB_CONTROL].swpActual.x = (SHORT)(ControlsInfo[NOTES_BOX_CONTROL].swpActual.x + iGaps);
              ControlsInfo[NOTES_DEL_PB_CONTROL].swpActual.x = (SHORT)(ControlsInfo[NOTES_ADD_PB_CONTROL].swpActual.x + 
                                                               ControlsInfo[NOTES_ADD_PB_CONTROL].swpActual.cx + iGaps);
              ControlsInfo[NOTES_SHOW_PB_CONTROL].swpActual.x = (SHORT)(ControlsInfo[NOTES_DEL_PB_CONTROL].swpActual.x + 
                                                               ControlsInfo[NOTES_DEL_PB_CONTROL].swpActual.cx + iGaps);
            } /* endif */
          }
          
          // adjust height of NOTES groupbox and listbox
          {
            int iDelta = 0;
            ControlsInfo[NOTES_BOX_CONTROL].swpActual.cy = (SHORT)(iHeight - ControlsInfo[NOTES_BOX_CONTROL].swpActual.y - 10); 
            iDelta = ControlsInfo[NOTES_BOX_CONTROL].swpActual.cy - ControlsInfo[NOTES_BOX_CONTROL].swpOrg.cy;

            if ( pMDData->fXLiff )
            {
              ControlsInfo[NOTES_ADD_PB_CONTROL].swpActual.y = (SHORT)(ControlsInfo[NOTES_ADD_PB_CONTROL].swpOrg.y + iDelta); 
              ControlsInfo[NOTES_DEL_PB_CONTROL].swpActual.y = (SHORT)(ControlsInfo[NOTES_DEL_PB_CONTROL].swpOrg.y + iDelta); 
              ControlsInfo[NOTES_SHOW_PB_CONTROL].swpActual.y = (SHORT)(ControlsInfo[NOTES_SHOW_PB_CONTROL].swpOrg.y + iDelta); 
            } /* endif */               

            ControlsInfo[NOTES_LIST_CONTROL].swpActual.cy = (SHORT)(ControlsInfo[NOTES_LIST_CONTROL].swpOrg.cy + iDelta);
          }

        }

        // reposition controls
        {
          HDWP hdwp = BeginDeferWindowPos( iControls );

          for( i = 0; i < iControls; i++ )
          {
            hdwp = DeferWindowPos( hdwp, ControlsInfo[i].hwnd, HWND_TOP, ControlsInfo[i].swpActual.x,
                                   ControlsInfo[i].swpActual.y, ControlsInfo[i].swpActual.cx, ControlsInfo[i].swpActual.cy,
                                   SWP_NOACTIVATE | SWP_NOZORDER );
          } /* endfor */

          if ( hdwp != NULL ) EndDeferWindowPos( hdwp );
        }
      } /* endif */
    } /* endif */
    break;


  default:
    mResult = FALSE;
    break;
  } /* endswitch */

  WinDefDlgProc( hwndDlg, msg, mp1, mp2 );

  return mResult;
} /* end of METADATADIALOG */

//
// Metadata dialog: WM_INITDLG processing
//
MRESULT MDDialogInit
(
  HWND    hwndDlg,                               // handle of dialog window
  WPARAM  mp1,                        // first parameter of WM_INITDLG
  LPARAM  mp2                         // second parameter of WM_INITDLG
)
{
  MRESULT     mResult = FALSE;        // result of message processing
  PMD_DLG_DATA  pMDData;              // pointer to ida for find init
  //PTBDOCUMENT pDoc;                   // pointer to document

  mp1;                                // suppress 'unreferenced parameter' msg

  pMDData = (PMD_DLG_DATA) mp2;
  if ( pMDData )
  {

    ANCHORDLGIDA( hwndDlg, pMDData );

    // fill status combobox
    {
      int i = 0;
      while ( pszTranslStates[i] )
      {
        SHORT sItem = CBINSERTITEM( hwndDlg, ID_TB_MD_STATUS_CBS, pszTranslStates[i] );
        CBSETITEMHANDLE( hwndDlg, ID_TB_MD_STATUS_CBS, sItem, i );
        i++;
      } /*endwhile */
    }

    // fill style combobox
    {
      int i = 0;
      while ( pszStyles[i] )
      {
        SHORT sItem = CBINSERTITEM( hwndDlg, ID_TB_MD_STYLE_CBS, pszStyles[i] );
        CBSETITEMHANDLE( hwndDlg, ID_TB_MD_STYLE_CBS, sItem, i );
        i++;
      } /*endwhile */
    }

    // get window size/position from folderlist properties
    {
      EQFINFO     ErrorInfo;              // error code of property handler calls
      PPROPFOLDERLIST pFllProp = NULL;    // ptr to folder list properties
      PVOID       hFllProp;               // handle of folder list properties
      OBJNAME     szFllObjName;           // buffer for folder list object name

      UtlMakeEQFPath( (PSZ)szFllObjName, NULC, SYSTEM_PATH, NULL );
      strcat( (PSZ)szFllObjName, BACKSLASH_STR );
      strcat( (PSZ)szFllObjName, DEFAULT_FOLDERLIST_NAME );
      hFllProp = OpenProperties( (PSZ) szFllObjName, NULL, PROP_ACCESS_READ, &ErrorInfo );
      if ( hFllProp )
      {
        pFllProp = (PPROPFOLDERLIST)MakePropPtrFromHnd( hFllProp );

        // get last use dialog position (if available)
        if ( pFllProp->swpSegPropSizePos.x || pFllProp->swpSegPropSizePos.y )
        {
          EQF_PSWP pSwp = &(pFllProp->swpSegPropSizePos);

          // get client rectange of our parent window
          RECT rectParent;
          GetClientRect( pMDData->pDoc->hwndClient, &rectParent );

          // use saved values only if valid
          //if ( ((pSwp->x + pSwp->cx) < rectParent.right) && ((pSwp->y + 20) > rectParent.bottom) )
          {
            WinSetWindowPos( hwndDlg, HWND_TOP, pSwp->x, pSwp->y, pSwp->cx, pSwp->cy, EQF_SWP_MOVE | EQF_SWP_SIZE );
          } /* endif */
        } /* endif */
      } /* endif */
      if (hFllProp) CloseProperties( hFllProp, PROP_QUIT, &ErrorInfo );
    }
  } /* endif */

  // initialize metadata fields in dialog IDA
  {
    BOOL fOK = TRUE;
    USHORT usPriv = 0;

    UtlGetLANUserIDW( pMDData->szUserID, &usPriv, FALSE );

    fOK = UtlAlloc( (PVOID *)&(pMDData->pMDNoteList), 0, sizeof(MD_NOTELIST_DATA), ERROR_STORAGE );
    if ( fOK )
    {
      pMDData->pMDNoteList->Hdr.Type = MD_NOTELIST_TYPE;
      pMDData->pMDNoteList->Hdr.lSize = sizeof(MD_NOTELIST_DATA);
    } /* endif */

    if ( fOK ) fOK = UtlAlloc( (PVOID *)&(pMDData->pMDHistory), 0, sizeof(MD_HIST_DATA), ERROR_STORAGE );
    if ( fOK )
    {
      pMDData->pMDHistory->Hdr.Type = MD_HISTORY_TYPE;
      pMDData->pMDHistory->Hdr.lSize = sizeof(MD_HIST_DATA);
    } /* endif */

    if ( fOK ) fOK = UtlAlloc( (PVOID *)&(pMDData->pMDStatus), 0, sizeof(MD_STATUS_DATA), ERROR_STORAGE );
    if ( fOK )
    {
      pMDData->pMDStatus->Hdr.Type = MD_STATUS_TYPE;
      pMDData->pMDStatus->Hdr.lSize = sizeof(MD_STATUS_DATA);
    } /* endif */

    if ( fOK ) fOK = UtlAlloc( (PVOID *)&(pMDData->pMDComment), 0, sizeof(MD_COMMENT_DATA), ERROR_STORAGE );
    if ( fOK )
    {
      pMDData->pMDComment->Hdr.Type = MD_COMMENT_TYPE;
      pMDData->pMDComment->Hdr.lSize = sizeof(MD_COMMENT_DATA);
    } /* endif */
  }

  return( mResult );
} /* end of MDDialogInit */

//
// Metadata dialog: WM_COMMAND processing
//
MRESULT MDDialogCommand
(
  HWND hwndDlg,
  WPARAM mp1,
  LPARAM mp2
)
{
  MRESULT mResult = MRFROMSHORT(TRUE);     // TRUE = command is processed
  int iCmd =  WMCOMMANDID( mp1, mp2 );

  mp2;

 switch ( iCmd )
 {
 // case ID_TB_FIND_HELP_PB:
 //   mResult = UtlInvokeHelp();
 //   break;
 
   case  ID_TB_MD_NOTES_LIST:
     {
       int cmd = WMCOMMANDCMD( mp1, mp2 );
       switch ( cmd )
       {
        case LN_ENTER:
          // show double-clicked note
          {
            PMD_DLG_DATA  pMDData = ACCESSDLGIDA( hwndDlg, PMD_DLG_DATA );
            PMD_NOTE_DATA pNote = NULL;
            SHORT sItem = LIT_NONE;

            sItem = QUERYSELECTION( hwndDlg, ID_TB_MD_NOTES_LIST );
            if ( sItem >= 0 )
            {
              pNote = (PMD_NOTE_DATA )QUERYITEMHANDLE( hwndDlg, ID_TB_MD_NOTES_LIST, sItem ); 
            } /* endif */

            MDShowNoteDialog( hwndDlg, pMDData, pNote );
          }
          break;
       } /*endswitch */
     }
     break;

  case ID_TB_MD_NOTES_ADD_PB: 
    {
      PMD_DLG_DATA  pMDData = ACCESSDLGIDA(hwndDlg, PMD_DLG_DATA );
      MDShowNoteDialog( hwndDlg, pMDData, NULL );
    }
    break;

  case ID_TB_MD_NOTES_SHOW_PB: 
    {
      PMD_DLG_DATA  pMDData = ACCESSDLGIDA(hwndDlg, PMD_DLG_DATA );
      PMD_NOTE_DATA pNote = NULL;
      SHORT sItem = LIT_NONE;

      sItem = QUERYSELECTION( hwndDlg, ID_TB_MD_NOTES_LIST );
      if ( sItem >= 0 )
      {
        pNote = (PMD_NOTE_DATA) QUERYITEMHANDLE( hwndDlg, ID_TB_MD_NOTES_LIST, sItem ); 
      } /* endif */

      MDShowNoteDialog( hwndDlg, pMDData, pNote );
    }
    break;

  case ID_TB_MD_NOTES_DEL_PB: 
    {
      PMD_DLG_DATA  pMDData = ACCESSDLGIDA(hwndDlg, PMD_DLG_DATA );
      PMD_NOTE_DATA pNote = NULL;
      SHORT sItem = LIT_NONE;

      sItem = QUERYSELECTION( hwndDlg, ID_TB_MD_NOTES_LIST );
      if ( sItem >= 0 )
      {
        pNote = (PMD_NOTE_DATA) QUERYITEMHANDLE( hwndDlg, ID_TB_MD_NOTES_LIST, sItem ); 
        if ( pNote != NULL )
        {
          // remove note from note list
          {
            PBYTE pbNoteList = (PBYTE)pMDData->pMDNoteList;
            PBYTE pbNote = (PBYTE)pNote;
            int iNewSize = pMDData->pMDNoteList->Hdr.lSize - pNote->lSize;
            int iRest = pMDData->pMDNoteList->Hdr.lSize - (pbNote - pbNoteList) - pNote->lSize;
            if ( iRest >= 0 )
            {
              if ( iRest  )
              {
                memmove( pbNote, pbNote + pNote->lSize, iRest );
              } /* endif */

              // resize note list 
              UtlAlloc( (PVOID *)&(pMDData->pMDNoteList), pMDData->pMDNoteList->Hdr.lSize, iNewSize, ERROR_STORAGE );
              pMDData->pMDNoteList->Hdr.lSize = iNewSize;
              pMDData->pMDNoteList->lNumberOfNotes -= 1;
            } /* endif */
          }

          // refresh lists of notes
          MDDialogRefreshNotes( hwndDlg, pMDData );
        } /* endif */
      } /* endif */
    }
    break;

  case ID_TB_MD_STYLE_CBS: 
    {
      PMD_DLG_DATA  pMDData = ACCESSDLGIDA(hwndDlg, PMD_DLG_DATA );
      int iCommand = WMCOMMANDCMD( mp1, mp2 );
      if ( (iCommand == CBN_EFCHANGE) || (iCommand == CBN_SELCHANGE) )
      {
        pMDData->fCommentStyleChanged = TRUE;        
      } /* end */         
    }
    break;



 // case ID_TB_FIND_CANCEL_PB:
 // case DID_CANCEL:
 //   EQFBFindFill ( hwndDlg, pMDData);      // get data from dialog
 //   POSTCLOSE( hwndDlg, FALSE );
 //   break;


  default:
    mResult = FALSE;
    break;
 } /* endswitch */

  return( mResult );
} /* end of MDDialogCommand */

//
// Show note dialog for new or existing note
//
void MDShowNoteDialog( HWND hwndParent, PMD_DLG_DATA pMDData, PMD_NOTE_DATA pNote )
{
  PMD_NOTE_DATA pNewNote = NULL;
  int iRc = 0;

  // make work copy of current note
  if ( pNote )
  {
    if ( UtlAlloc( (PVOID *)&pNewNote, 0, pNote->lSize, ERROR_STORAGE ) )
    {
      memcpy( pNewNote, pNote, pNote->lSize );
    } /* endif */
  } /* endif */

  DIALOGBOX( hwndParent, (DLGPROC) MDNoteDlgProc, hResMod, (DLGPROC)ID_TB_MD_NOTE_DLG, &pNewNote, iRc );

  if ( pNewNote )
  {
    BOOL fOK = TRUE;

    if ( pNote != NULL )
    {
      PMD_NOTELIST_DATA pNewList = NULL;

      // replace note with updated one

      // allocate new note list 
      fOK = UtlAlloc( (PVOID *)&pNewList, 0, pMDData->pMDNoteList->Hdr.lSize + pNewNote->lSize - pNote->lSize, ERROR_STORAGE );

      // copy all notes to new list and replace modified one
      if ( fOK )
      {
        PMD_NOTE_DATA pSource =  (PMD_NOTE_DATA)(((PBYTE)pMDData->pMDNoteList) + sizeof(MD_NOTELIST_DATA));
        PMD_NOTE_DATA pTarget = (PMD_NOTE_DATA)(((PBYTE)pNewList) + sizeof(MD_NOTELIST_DATA));
        int i = 0;

        pNewList->Hdr.Type = MD_NOTELIST_TYPE;
        pNewList->Hdr.lSize = sizeof(MD_NOTELIST_DATA);
        pNewList->lNumberOfNotes = 0;

        while ( i < pMDData->pMDNoteList->lNumberOfNotes )
        {
          if ( pSource == pNote )
          {
            memcpy( pTarget, pNewNote, pNewNote->lSize );
          }
          else
          {
            memcpy( pTarget, pSource, pSource->lSize );
          } /* endif */

          pSource = (PMD_NOTE_DATA)(((PBYTE)pSource) + pSource->lSize);
          pTarget = (PMD_NOTE_DATA)(((PBYTE)pTarget) + pTarget->lSize);
          pNewList->Hdr.lSize += pTarget->lSize;
          pNewList->lNumberOfNotes += 1;
          i++;
        } /*endwhile */
      } /* endif */

      // replace notelist with new one
      if ( fOK )
      {
        UtlAlloc( (PVOID *)&(pMDData->pMDNoteList), 0, 0, NOMSG );
        pMDData->pMDNoteList = pNewList;
      } /* endif */
    }
    else
    {
      // create a new notelist area if none yet
      if ( pMDData->pMDNoteList == NULL )
      {
        if ( UtlAlloc( (PVOID *)&(pMDData->pMDNoteList), 0, sizeof(MD_NOTELIST_DATA), ERROR_STORAGE ) )
        {
          pMDData->pMDNoteList->Hdr.lSize = sizeof(MD_NOTELIST_DATA);
          pMDData->pMDNoteList->Hdr.Type = MD_NOTELIST_TYPE;
          pMDData->pMDNoteList->lNumberOfNotes = 0;
        } /* endif */
      } /* endif */

      // enlarge notelist area and add new note 
      if ( pMDData->pMDNoteList  )
      {
        if ( UtlAlloc( (PVOID *)&(pMDData->pMDNoteList), pMDData->pMDNoteList->Hdr.lSize, pMDData->pMDNoteList->Hdr.lSize + pNewNote->lSize,
                      ERROR_STORAGE ) )
        {
          memcpy( ((PBYTE)pMDData->pMDNoteList) + pMDData->pMDNoteList->Hdr.lSize, pNewNote, pNewNote->lSize ); 
          pMDData->pMDNoteList->lNumberOfNotes += 1;
          pMDData->pMDNoteList->Hdr.lSize      += pNewNote->lSize;
          UtlAlloc( (PVOID *)&pNewNote, 0, 0, NOMSG );
        } /* endif */
      } /* endif */
    } /* endif */

    // refresh lists of notes
    MDDialogRefreshNotes( hwndParent, pMDData );
  } /* endif */
} /* end of MDShowNoteDialog */

//
// Metadata dialog: WM_CLOSE processing
//
MRESULT MDDialogClose
(
  HWND hwndDlg,
  WPARAM mp1,
  LPARAM mp2
)
{
  MRESULT           mResult = FALSE;
  PMD_DLG_DATA      pMDData = NULL;
  PTBDOCUMENT       pDoc = NULL; 

  pMDData = ACCESSDLGIDA( hwndDlg, PMD_DLG_DATA );
  pDoc = pMDData->pDoc;

  mp1; mp2; 

  // force repain of screen, we might have to update some areas...
  if (pDoc )
  {
    pDoc->Redraw = REDRAW_ALL;
    WinSetActiveWindow( HWND_DESKTOP, pDoc->hwndFrame );
  } /* endif */

  // DelCtrlFont (hwndDlg, ID_TB_FIND_CHANGE_EF);

  if ( pMDData->hwndDlg )
  {
    HWND hwndTemp = pMDData->hwndDlg;

    // save current dialog position
    { 
      EQFINFO     ErrorInfo;       // error code of property handler calls
      PPROPFOLDERLIST pFllProp;    // ptr to folder list properties
      PVOID       hFllProp;        // handle of folder list properties
      OBJNAME     szFllObjName;    // buffer for folder list object name

      UtlMakeEQFPath( szFllObjName, NULC, SYSTEM_PATH, NULL );
      strcat( szFllObjName, BACKSLASH_STR );
      strcat( szFllObjName, DEFAULT_FOLDERLIST_NAME );
      hFllProp = OpenProperties( szFllObjName, NULL, PROP_ACCESS_READ, &ErrorInfo );
      if ( hFllProp )
      {
        if ( SetPropAccess( hFllProp, PROP_ACCESS_WRITE) )
        {
          pFllProp = (PPROPFOLDERLIST)MakePropPtrFromHnd( hFllProp );
          UtlSaveWindowPos( pMDData->hwndDlg, &(pFllProp->swpSegPropSizePos) );
          SaveProperties( hFllProp, &ErrorInfo);
        } /* endif */
        CloseProperties( hFllProp, PROP_QUIT, &ErrorInfo );
      } /* endif */
    }
    DelCtrlFont( pMDData->hwndDlg, ID_TB_MD_COMMENT_MLE );

    pMDData->hwndDlg = NULLHANDLE;

    if (  pMDData->pMDNoteList ) UtlAlloc( (PVOID *)&(pMDData->pMDNoteList), 0, 0, NOMSG );
    if (  pMDData->pMDHistory )  UtlAlloc( (PVOID *)&(pMDData->pMDHistory), 0, 0, NOMSG );
    if (  pMDData->pMDHistory )  UtlAlloc( (PVOID *)&(pMDData->pMDStatus), 0, 0, NOMSG );
    if (  pMDData->pMDComment )  UtlAlloc( (PVOID *)&(pMDData->pMDComment), 0, 0, NOMSG );

    WinDestroyWindow( hwndTemp );
  } /* endif */
  pMDData->pDoc = NULL;

  return( mResult );
} /* end of MDDialogClose */

//
// Get first element of a metadata area
//
PMD_ELEMENT_HDR MDGetFirstElement( PMD_METADATA pData )
{
  PMD_ELEMENT_HDR pElement = NULL;

  if ( pData )
  {
    pElement = (PMD_ELEMENT_HDR)(((PBYTE)pData) + sizeof(MD_METADATA) );
  } /* endif */
  return( pElement );
} /* end of MDGetFirstElement */

//
// Get next element of a metadata area
//
PMD_ELEMENT_HDR MDGetNextElement( PMD_ELEMENT_HDR pElement )
{
  if ( pElement )
  {
    pElement = (PMD_ELEMENT_HDR)(((PBYTE)pElement) + pElement->lSize);
  } /* endif */
  return( pElement );
} /* end of MDGetNextElement */

//
// Create an empty metadata area
//
BOOL MDCreateMetaData( PMD_METADATA *ppData )
{
  BOOL             fOK = TRUE;
  int              iSize = 0;

  iSize = sizeof(MD_METADATA) + sizeof(MD_ELEMENT_HDR);

  fOK = UtlAlloc( (PVOID *)ppData, 0, iSize, ERROR_STORAGE );

  if ( fOK )
  {
    PMD_ELEMENT_HDR pElement = MDGetFirstElement( *ppData );
    PMD_METADATA pData = *ppData;
    pData->lSize = iSize;
    pElement->lSize = sizeof(MD_ELEMENT_HDR);
    pElement->Type  = MD_ENDOFLIST_TYPE;
  } /* endif */
  return( fOK );
} /* end of MDCreateArea */

//
// Release a metadata area
//
BOOL MDFreeMetaData( PMD_METADATA *ppData )
{
  BOOL             fOK = TRUE;

  if ( ppData && *ppData )
  {
    UtlAlloc( (PVOID *)ppData, 0, 0, NOMSG );
    *ppData = NULL;
  } /* endif */
  return( fOK );
} /* end of MDFreeMetaData */


//
// Find first metadata element of the given type
//
PMD_ELEMENT_HDR MDFindElementByType( PMD_METADATA pData, MD_TYPE Type )
{
  PMD_ELEMENT_HDR pElement = MDGetFirstElement( pData );

  while ( pElement && (pElement->Type != MD_ENDOFLIST_TYPE) && (pElement->Type != Type) )
  {
    pElement = MDGetNextElement( pElement );
  } /*endwhile */
  if ( pElement->Type != Type )
  {
    pElement = NULL;
  } /* endif */
  return( pElement );
} /* end of MDFindElementByType */

//
// Add a metadata element to a metadata area
//
BOOL MDAddElement( PMD_METADATA *ppData, PMD_ELEMENT_HDR pElement )
{
  BOOL             fOK = TRUE;
  int              iNewSize = 0;

  // create new metadata area if there is none yet
  if ( *ppData == NULL )
  {
    fOK = MDCreateMetaData( ppData );
  } /* endif */

  // compute new size of metadata area
  iNewSize = (*ppData)->lSize + pElement->lSize;

  // reallocate data area
  fOK = UtlAlloc( (PVOID *)ppData, (*ppData)->lSize, iNewSize, ERROR_STORAGE );

  // add new element right before end-of-list element
  if ( fOK )
  {
    // find end-of-list element
    PMD_ELEMENT_HDR pEndOfList = MDFindElementByType( *ppData, MD_ENDOFLIST_TYPE );

    if ( pEndOfList )
    {
      // make room for new element
      memmove( ((PBYTE)pEndOfList) + pElement->lSize, pEndOfList, sizeof(MD_ELEMENT_HDR) );

      // insert new element
      memcpy( pEndOfList, pElement, pElement->lSize );

      // adjust metadata area size
      (*ppData)->lSize = iNewSize;

    }
    else
    {
      fOK = FALSE;
    } /* endif */
  } /* endif */

  return( fOK );
} /* end of MDAddElement */

//
// Add comment metadata to metadata area
//
void MDAddCommentData( PVOID *ppvData, PSZ_W pszComment, PSZ_W pszStyle )
{
  PMD_COMMENT_DATA pMDComment = NULL;
  PMD_METADATA *ppData = (PMD_METADATA *)ppvData;

  int iSize = sizeof(MD_COMMENT_DATA) + ((wcslen(pszComment) + 1) * sizeof(CHAR_W));

  UtlAlloc( (PVOID *)&pMDComment, 0, iSize, ERROR_STORAGE );

  if ( pMDComment )
  {
    pMDComment->Hdr.lSize = iSize;
    pMDComment->Hdr.Type  = MD_COMMENT_TYPE;
    wcscpy( pMDComment->szStyle, pszStyle );
    wcscpy( pMDComment->szComment, pszComment );

    MDAddElement( ppData, (PMD_ELEMENT_HDR)pMDComment );

    UtlAlloc( (PVOID *)&pMDComment, 0, 0, NOMSG );
  } /* endif */  
} /* end of MDAddCommentData */



//
// Update dialog data area with comment data from dialog fields
//
void MDDialogPrepareCommentData( PMD_DLG_DATA pMDData )
{
   GETTEXTLENGTHEX gtl;

  // get new length of comment data
  int iTextLen, iSize;

  gtl.flags = GT_DEFAULT;
  gtl.codepage = 1200;
  iTextLen = SendDlgItemMessage( pMDData->hwndDlg, ID_TB_MD_COMMENT_MLE, EM_GETTEXTLENGTHEX, (WPARAM)&gtl, 0 );
  iSize = sizeof(MD_COMMENT_DATA) + ((iTextLen + 1) * sizeof(CHAR_W));

  if ( (pMDData->pMDComment == NULL) || (pMDData->pMDComment->Hdr.lSize != iSize) )
  {
    if ( pMDData->pMDComment != NULL ) UtlAlloc( (PVOID *)&(pMDData->pMDComment), 0, 0, NOMSG );
    UtlAlloc( (PVOID *)&(pMDData->pMDComment), 0, iSize, ERROR_STORAGE );
  } /* endif */

  if ( pMDData->pMDComment )
  {
    CHAR_W szStyle[40];
    pMDData->pMDComment->Hdr.lSize = iSize;
    pMDData->pMDComment->Hdr.Type  = MD_COMMENT_TYPE;
    // GQ: we have to use a local buffer for the style string, sometimes the direct access to pMDData->pMDComment->szStyle fails...
    //     this could be caused by pMDData->pMDComment->szStyle residing on a uneven address boundary (OTM uses 1 byte alignment in structures)
    QUERYTEXTW( pMDData->hwndDlg, ID_TB_MD_STYLE_CBS, szStyle);
    wcscpy( pMDData->pMDComment->szStyle, szStyle );
    {
      GETTEXTEX getTextEx;
      HWND hwndEdit = GetDlgItem( pMDData->hwndDlg, ID_TB_MD_COMMENT_MLE);
      memset( &getTextEx, 0, sizeof(getTextEx) );
      getTextEx.cb = (iTextLen + 1) * sizeof(CHAR_W);
      getTextEx.codepage = 1200;
      getTextEx.flags = GT_DEFAULT;

      SendMessage( hwndEdit, EM_GETTEXTEX, (WPARAM)&getTextEx, (LPARAM)pMDData->pMDComment->szComment );
    }

    // free empty comment entries
    UtlStripBlanksW( pMDData->pMDComment->szComment );
    if ( (wcslen(pMDData->pMDComment->szComment) == 0) && (wcslen(pMDData->pMDComment->szStyle) == 0) )
    {
      UtlAlloc( (PVOID *)&(pMDData->pMDComment), 0, 0, NOMSG );
    } /* endif */       
  } /* endif */
} /* end of MDDialogPrepareCommentData */



//
// Update dialog data area with status data from dialog fields
//
void MDDialogPrepareStatusData( PMD_DLG_DATA pMDData )
{
  if ( pMDData->pMDStatus == NULL )
  {
    UtlAlloc( (PVOID *)&(pMDData->pMDStatus), 0, sizeof(MD_STATUS_DATA), ERROR_STORAGE );
  } /* endif */
  if ( pMDData->pMDStatus )
  {
    pMDData->pMDStatus->Hdr.lSize = sizeof(MD_STATUS_DATA);
    pMDData->pMDStatus->Hdr.Type  = MD_STATUS_TYPE;
    pMDData->pMDStatus->fApproved = QUERYCHECK( pMDData->hwndDlg, ID_TB_MD_APPROVED_CHK );
    QUERYTEXTW( pMDData->hwndDlg, ID_TB_MD_STATUS_CBS, pMDData->pMDStatus->szStatus );
  } /* endif */
} /* end of MDDialogPrepareStatusData */


//
// Update dialog data area with history data from dialog fields
//
void MDDialogPrepareHistoryData( PMD_DLG_DATA pMDData, BOOL fModified )
{
  if ( fModified )
  {
    if ( pMDData->pMDHistory == NULL )
    {
      UtlAlloc( (PVOID *)&(pMDData->pMDHistory), 0, sizeof(MD_HIST_DATA), ERROR_STORAGE );
    } /* endif */
    if ( pMDData->pMDHistory )
    {
      pMDData->pMDHistory->Hdr.lSize = sizeof(MD_HIST_DATA);
      pMDData->pMDHistory->Hdr.Type  = MD_HISTORY_TYPE;
      if ( pMDData->pMDHistory->lCreationDate == 0 )
      {
        wcscpy( pMDData->pMDHistory->szCreator, pMDData->szUserID );
        UtlTime( &(pMDData->pMDHistory->lCreationDate) );
      } /* endif */
      wcscpy( pMDData->pMDHistory->szModifier, pMDData->szUserID );
      UtlTime( &(pMDData->pMDHistory->lModificationDate) );
    } /* endif */
  } /* endif */
} /* end of MDDialogPrepareHistoryData */


//
// Update dialog data area with note data from dialog fields
//
void MDDialogPrepareNoteListData( PMD_DLG_DATA pMDData )
{
  pMDData;

  // nothing to do, notelist area contains the correct information already
} /* end of MDDialogPrepareNoteListData */

//
// Update segment data with data from dialog
//
BOOL MDUpdateMetadataFromDialog( PMD_DLG_DATA  pMDData, PTBSEGMENT pSeg )
{
  BOOL             fOK = TRUE;
  PMD_METADATA     pOldData = (PMD_METADATA)pSeg->pvMetadata;
  PMD_METADATA     pNewData = NULL;
  BOOL             fNoteList = FALSE;    
  BOOL             fHistory = FALSE;    
  BOOL             fStatus = FALSE;    
  BOOL             fComment = FALSE;    

  // prepare metadata handled within dialog
  MDDialogPrepareCommentData( pMDData );
  MDDialogPrepareStatusData( pMDData );
  MDDialogPrepareHistoryData( pMDData, pMDData->fSegModified );
  MDDialogPrepareNoteListData( pMDData );


  // copy all existing metadata elements to new metadata element area and update
  // all elements maintained within the dialog
  if ( pOldData )
  {
    PMD_ELEMENT_HDR pCurElement = MDGetFirstElement( pOldData );
    while ( pCurElement && (pCurElement->Type != MD_ENDOFLIST_TYPE) )
    {
      switch ( pCurElement->Type )
      {
        case MD_NOTELIST_TYPE:
          if ( pMDData->pMDNoteList && pMDData->pMDNoteList->lNumberOfNotes )
          {
            fOK = MDAddElement( &pNewData, (PMD_ELEMENT_HDR)pMDData->pMDNoteList );
          } /* endif */
          fNoteList = TRUE;
          break;
        case MD_HISTORY_TYPE:
          if ( pMDData->pMDHistory )
          {
            fOK = MDAddElement( &pNewData, (PMD_ELEMENT_HDR)pMDData->pMDHistory );
          } /* endif */
          fHistory = TRUE;
          break;
        case MD_STATUS_TYPE:
          if ( pMDData->pMDStatus )
          {
            fOK = MDAddElement( &pNewData, (PMD_ELEMENT_HDR)pMDData->pMDStatus );
          } /* endif */
          fStatus = TRUE;
          break;
        case MD_COMMENT_TYPE:
          if ( pMDData->pMDComment )
          {
            fOK = MDAddElement( &pNewData, (PMD_ELEMENT_HDR)pMDData->pMDComment );
          } /* endif */
          fComment = TRUE;
          break;
        default:
          // copy not processed elements to new metadata
          fOK = MDAddElement( &pNewData, pCurElement );
          break;
      } /*endswitch */
      pCurElement = MDGetNextElement( pCurElement );

    } /*endwhile */
  } /* endif */

  // copy remaining dialog metadata elements to new area
  if ( !fComment && pMDData->pMDComment ) fOK = MDAddElement( &pNewData, (PMD_ELEMENT_HDR)pMDData->pMDComment );
  if ( !fNoteList && pMDData->pMDNoteList && pMDData->pMDNoteList->lNumberOfNotes ) fOK = MDAddElement( &pNewData, (PMD_ELEMENT_HDR)pMDData->pMDNoteList );
  if ( !fHistory && pMDData->pMDHistory )  fOK = MDAddElement( &pNewData, (PMD_ELEMENT_HDR)pMDData->pMDHistory );
  if ( !fStatus && pMDData->pMDStatus )   fOK = MDAddElement( &pNewData, (PMD_ELEMENT_HDR)pMDData->pMDStatus );

  // replace segment metadata with new area
  if ( pNewData )
  {
    if ( pSeg->pvMetadata ) UtlAlloc( (PVOID *)&(pSeg->pvMetadata), 0, 0, NOMSG ); 
    pSeg->pvMetadata = pNewData;
  } /* endif */

  return( fOK );
} /* end of MDUpdateMetadataFromDialog */

//
// dialog procedure for segment metadata window
//
//MRESULT APIENTRY MDNoteDlgProc
MRESULT CALLBACK MDNoteDlgProc
(
  HWND hwndDlg,                                  // handle of dialog window 
  WINMSG msg,
  WPARAM mp1,
  LPARAM mp2
)
{
  MRESULT  mResult = MRFROMSHORT( FALSE );      // result value of procedure

  switch ( msg )
  {
    case WM_HELP:
//      EqfDisplayContextHelp( ((LPHELPINFO) mp2)->hItemHandle, &hlpsubtblConnectTMDlg[0] );
      mResult = TRUE;  // message processed
      break;

    case WM_EQF_QUERYID: HANDLEQUERYID( ID_TB_MD_NOTE_DLG, mp2 ); break;

    case WM_INITDLG:
      {
        PMD_NOTE_DATA *ppNote = (PMD_NOTE_DATA *)mp2;
        PMD_NOTE_DATA pNote = *ppNote;

        SETWINDOWID( hwndDlg, ID_TB_MD_NOTE_DLG );
        ANCHORDLGIDA( hwndDlg, ppNote );
        SETTEXTLIMIT( hwndDlg, ID_TB_MD_NOTE_AUTHOR_EF, sizeof(pNote->szAuthor) / sizeof(CHAR_W) );

        if ( pNote != NULL )
        {
          SetWindowText( hwndDlg, "Modify note" );
          SetDlgItemText( hwndDlg, ID_TB_MD_NOTE_SAVE_PB, "Save note" );
          SETTEXTW( hwndDlg, ID_TB_MD_NOTE_AUTHOR_EF, pNote->szAuthor );
          SETTEXTW( hwndDlg, ID_TB_MD_NOTE_NOTE_MLE, pNote->szNote );
        }
        else
        {
          CHAR_W szTemp[40]; 
          USHORT usPriv;
          LONG lTime = 0;

          SetWindowText( hwndDlg, "Add a note" );
          SetDlgItemText( hwndDlg, ID_TB_MD_NOTE_SAVE_PB, "Add note" );
          UtlGetLANUserIDW( szTemp, &usPriv, FALSE );
          SETTEXTW( hwndDlg, ID_TB_MD_NOTE_AUTHOR_EF, szTemp );

          UtlTime( &lTime );
          MDUtilLongToDateTimeString( lTime, szTemp, 40 );
          SETTEXTW( hwndDlg, ID_TB_MD_NOTE_NOTE_MLE, szTemp );
        } /* endif */
      }
      break;

    case WM_COMMAND:
      switch ( WMCOMMANDID( mp1, mp2 ) )
      {
        case ID_TB_MD_NOTE_SAVE_PB:
          {
            PVOID pvNote = ACCESSDLGIDA( hwndDlg, PVOID );
            PMD_NOTE_DATA *ppNote = (PMD_NOTE_DATA *)pvNote;
            PMD_NOTE_DATA pNote = *ppNote;
            PMD_NOTE_DATA pNewNote = NULL;

            // create and fill new note area
            {
              int iTextLen = SendDlgItemMessage( hwndDlg, ID_TB_MD_NOTE_NOTE_MLE, WM_GETTEXTLENGTH, 0, 0 );
              int iSize = sizeof(MD_NOTE_DATA) + ((iTextLen + 1) * sizeof(CHAR_W));

              if ( UtlAlloc( (PVOID *)&pNewNote, 0, iSize, ERROR_STORAGE ) )
              {
                pNewNote->lSize = iSize;
                QUERYTEXTW( hwndDlg, ID_TB_MD_NOTE_AUTHOR_EF, pNewNote->szAuthor );
                GetDlgItemTextW( hwndDlg, ID_TB_MD_NOTE_NOTE_MLE, pNewNote->szNote, iTextLen + 1 );
              } /* endif */
            }

            // update caller's note area pointer
            if ( pNewNote )
            {
              if ( pNote ) UtlAlloc( (PVOID *)&pNote, 0, 0, NOMSG );
              *ppNote = pNewNote;
            } /* endif */
            WinPostMsg( hwndDlg, WM_CLOSE, 0L, 0L );
          }
          break;
        case ID_TB_MD_NOTE_CANCEL_PB:
        case DID_CANCEL:
          WinPostMsg( hwndDlg, WM_CLOSE, 0L, 0L );
          break;
      default:
          break;
      } /*endswitch */
      break;

    case WM_CLOSE:
      // UtlAlloc( (PVOID *) &pIda, 0L, 0L, NOMSG );          // free IDA
      WinDismissDlg( hwndDlg, TRUE );
      break;

    default:
      mResult = WinDefDlgProc( hwndDlg, msg, mp1, mp2 );
      break;
  } /* endswitch */

  return mResult;
} /* end of MDNoteDlgProc */

//
// create a date and time string from an LONG time value
//
void MDUtilLongToDateTimeString( LONG lTime, PSZ_W pszString, int iBufLen )
{
  int iLen;
  UtlLongToDateStringW( lTime, pszString, iBufLen );
  iLen = wcslen( pszString );
  pszString[iLen++] = L' '; 
  if ( iLen < iBufLen )
  {
    UtlLongToTimeStringW( lTime, pszString + iLen, iBufLen - iLen );
  } /* endif */
} /* end of MDUtilLongToDateTimeString */

//
// get the first proposal from the supplied metadata
//
BOOL MDGetFirstProposal( PVOID pvMetaData, PMD_PROPOSAL pProposal )
{
  BOOL fAvailable = FALSE;

  if ( pvMetaData )
  {
    PMD_METADATA pData = (PMD_METADATA)pvMetaData;
    pProposal->pvNext = MDGetFirstElement( pData );
    fAvailable = MDGetNextProposal( pProposal );
  } /* endif */

  return( fAvailable );
} /* end of MDGetFirstProposal */

//
// get the next proposal from the supplied metadata
//
BOOL MDGetNextProposal( PMD_PROPOSAL pProposal )
{
  BOOL fAvailable = FALSE;

  if ( pProposal && pProposal->pvNext )
  {
    PMD_ELEMENT_HDR pElement = (PMD_ELEMENT_HDR)pProposal->pvNext;
    memset( pProposal, 0, sizeof(MD_PROPOSAL) );
    while ( (pElement->Type != MD_PROPOSAL_TYPE) && (pElement->Type != MD_ENDOFLIST_TYPE) )
    {
      pElement = MDGetNextElement( pElement );
    } /*endwhile */
    if ( pElement->Type == MD_PROPOSAL_TYPE )
    {
      PSZ_W pszTemp;
      int iBytes = 0;

      PMD_PROPOSAL_DATA pMDProp = (PMD_PROPOSAL_DATA)pElement;
  
      pProposal->sQuality = pMDProp->sQuality;
      pProposal->ulSegmentId = pMDProp->ulSegmentId;
      pProposal->lDate = pMDProp->lModificationDate;
      
      pszTemp = (PSZ_W)&(pMDProp->Buffer);
      wcscpy( pProposal->szSource, pszTemp + pMDProp->lSourcePos );
      wcscpy( pProposal->szTarget, pszTemp + pMDProp->lTargetPos );
      iBytes = WideCharToMultiByte( CP_OEMCP, 0, pszTemp + pMDProp->lAuthorPos, -1, pProposal->szAuthorName, sizeof(pProposal->szAuthorName), NULL, NULL );
      pProposal->szAuthorName[iBytes] = NULC;
      if ( *(pszTemp + pMDProp->lDocNamePos) != 0 )
      {
        iBytes = WideCharToMultiByte( CP_OEMCP, 0, pszTemp + pMDProp->lDocNamePos, -1, pProposal->szDocName, sizeof(pProposal->szDocName), NULL, NULL );
        pProposal->szDocName[iBytes] = NULC;
      }
      else
      {
        strcpy( pProposal->szDocName, "XLIFF" );
      } /* endif */
      fAvailable = TRUE;

      pProposal->pvNext = MDGetNextElement( pElement );
    } /* endif */
  } /* endif */
  return( fAvailable );
}

//
// Write metadata to file
//
USHORT MDWriteMetaData( PTBDOCUMENT pDoc )
{
  USHORT     usRC = 0;
  HANDLE     hfOut = NULLHANDLE;

  // setup output file name and open output file
  {
    CHAR      szMetadataFile[MAX_EQF_PATH];      // buffer for output file name
    CHAR      szFolder[MAX_FILESPEC];            // buffer for folder name
    CHAR      szDocument[MAX_FILESPEC];          // buffer for document name
    CHAR      chDrive;                           // buffer for drice letter
    PSZ       pszTemp;  

    strcpy( szMetadataFile, pDoc->szDocName );
    chDrive = szMetadataFile[0];
    pszTemp = UtlSplitFnameFromPath( szMetadataFile ); 
    if ( pszTemp ) strcpy( szDocument, pszTemp );
    if ( pszTemp ) pszTemp = UtlSplitFnameFromPath( szMetadataFile ); 
    if ( pszTemp ) pszTemp = UtlSplitFnameFromPath( szMetadataFile ); 
    if ( pszTemp ) strcpy( szFolder, pszTemp ); 
    if ( pszTemp )
    {
      USHORT    usAction = 0;                      // action performed by UtlOpen

      UtlMakeEQFPath( szMetadataFile, chDrive, METADATA_PATH, szFolder );
      UtlMkDir( szMetadataFile, 0, FALSE );
      strcat( szMetadataFile, BACKSLASH_STR );
      strcat( szMetadataFile, szDocument );
      usRC = UtlOpen( szMetadataFile, &hfOut, &usAction, 0L, FILE_NORMAL, FILE_CREATE | FILE_TRUNCATE, 
                      OPEN_ACCESS_WRITEONLY | OPEN_SHARE_DENYNONE, 0L, FALSE );
    }
    else
    {
      usRC = ERROR_INVALID_DATA;
    } /* endif */
  }


  // write metadata of all segments to ouput file
  if ( !usRC )
  {
    PTBSEGMENT pSeg = NULL;                        // ptr to segment
    ULONG      ulSegNum = 1;
    ULONG      ulAddSegNum = 1;
    ULONG      ulActiveTable = STANDARDTABLE;

    pSeg = EQFBGetFromBothTables( pDoc, &ulSegNum, &ulAddSegNum, &ulActiveTable );
    while ( !usRC && pSeg )
    {
      if ( pSeg->pvMetadata )
      {
        ULONG ulWritten = 0;

        PMD_METADATA pMetadata = (PMD_METADATA)pSeg->pvMetadata;
        pMetadata->ulSegNum = pSeg->ulSegNum;
        usRC = UtlWriteL( hfOut, pMetadata, pMetadata->lSize, &ulWritten, TRUE );
      } /* endif */
      pSeg = EQFBGetFromBothTables( pDoc, &ulSegNum, &ulAddSegNum, &ulActiveTable );
    } /*endwhile */
  } /* endif */

  // write empty metadata as terminator
  if ( !usRC )
  {
    ULONG ulWritten = 0;
    MD_METADATA Metadata;
  
    memset( &Metadata, 0, sizeof(Metadata) );
    usRC = UtlWriteL( hfOut, &Metadata, sizeof(Metadata), &ulWritten, TRUE );
  } /* endif */

  // close output file
  if ( hfOut ) UtlClose( hfOut, FALSE );

  return( usRC );
} /* end of MDWriteMetaData */

//
// Load metadata from file
//
USHORT MDLoadMetaData( PTBDOCUMENT pDoc )
{
  USHORT     usRC = 0;
  HANDLE     hfIn = NULLHANDLE;

  // setup input file name and open input file
  {
    CHAR      szMetadataFile[MAX_EQF_PATH];      // buffer for output file name
    CHAR      szFolder[MAX_FILESPEC];            // buffer for folder name
    CHAR      szDocument[MAX_FILESPEC];          // buffer for document name
    CHAR      chDrive;                           // buffer for drice letter
    PSZ       pszTemp;  

    strcpy( szMetadataFile, pDoc->szDocName );
    chDrive = szMetadataFile[0];
    pszTemp = UtlSplitFnameFromPath( szMetadataFile ); 
    if ( pszTemp ) strcpy( szDocument, pszTemp );
    if ( pszTemp ) pszTemp = UtlSplitFnameFromPath( szMetadataFile ); 
    if ( pszTemp ) pszTemp = UtlSplitFnameFromPath( szMetadataFile ); 
    if ( pszTemp ) strcpy( szFolder, pszTemp ); 
    if ( pszTemp )
    {
      USHORT    usAction = 0;                      // action performed by UtlOpen

      UtlMakeEQFPath( szMetadataFile, chDrive, METADATA_PATH, szFolder );
      UtlMkDir( szMetadataFile, 0, FALSE );
      strcat( szMetadataFile, BACKSLASH_STR );
      strcat( szMetadataFile, szDocument );
      usRC = UtlOpen( szMetadataFile, &hfIn, &usAction, 0L, FILE_NORMAL, FILE_OPEN, 
                      OPEN_ACCESS_READONLY | OPEN_SHARE_DENYWRITE, 0L, FALSE );
    }
    else
    {
      usRC = ERROR_INVALID_DATA;
    } /* endif */
  }


  // read metadata and add it to approbriate segment
  if ( !usRC )
  {
    ULONG      ulRead = 0;
    MD_METADATA Metadata;

    // get metadata area header
    usRC = UtlReadL( hfIn, &Metadata, sizeof(Metadata), &ulRead, TRUE );
    if ( !usRC && (ulRead != sizeof(Metadata)))  usRC = ERROR_READ_FAULT;

    // while there are metadata areas...
    while ( !usRC && (Metadata.lSize != 0) )
    {
      PBYTE pbMetadata = NULL;

      // allocate buffer for metadata
      if ( !UtlAlloc( (PVOID *)&pbMetadata, 0, Metadata.lSize, ERROR_STORAGE ) )
      {
        usRC = ERROR_NOT_ENOUGH_MEMORY;
      } /* endif */

      // read rest of metadata
      if ( !usRC )
      {
        memcpy( pbMetadata, &Metadata, sizeof(MD_METADATA) );
        usRC = UtlReadL( hfIn, pbMetadata + sizeof(MD_METADATA), Metadata.lSize - sizeof(Metadata), &ulRead, TRUE );
        if ( !usRC && (ulRead != (Metadata.lSize - sizeof(Metadata))))  usRC = ERROR_READ_FAULT;
      } /* endif */

      // add metadata to segment
      if ( !usRC )
      {
        PTBSEGMENT pSeg = EQFBGetSegW( pDoc, Metadata.ulSegNum );
        if ( pSeg )
        {
          pSeg->pvMetadata = pbMetadata;
        }
        else
        {
          UtlAlloc( (PVOID *)&pbMetadata, 0, 0, NOMSG );
        } /* endif */
      } /* endif */

      // get header of next metadata area
      if ( !usRC )
      {
        usRC = UtlReadL( hfIn, &Metadata, sizeof(Metadata), &ulRead, TRUE );
        if ( !usRC && (ulRead != sizeof(Metadata)))  usRC = ERROR_READ_FAULT;
      } /* endif */
    } /*endwhile */
  } /* endif */


  // close input file
  if ( hfIn ) UtlClose( hfIn, FALSE );


  return( usRC );
} /* end of MDLoadMetaData */

//
// Convert the binary segment meta data format to the XML based segment meta data format
//
BOOL MDConvertToMemMetadata( PVOID pvSegMetaData, PSZ_W pszMemMetaData )
{
  BOOL             fOK = TRUE;
  PMD_METADATA     pSegMetaData = (PMD_METADATA)pvSegMetaData;

  if ( pSegMetaData )
  {
    PMD_ELEMENT_HDR pCurElement = MDGetFirstElement( pSegMetaData );
    while ( pCurElement && (pCurElement->Type != MD_ENDOFLIST_TYPE) )
    {
      switch ( pCurElement->Type )
      {
        case MD_COMMENT_TYPE:
          {
            PSZ_W pszSource, pszTarget;
            PMD_COMMENT_DATA pMDComment = (PMD_COMMENT_DATA)pCurElement;
            swprintf( pszMemMetaData, L"<Note style=\"%s\">", pMDComment->szStyle );

            // copy note text to buffer and escape special characters
            pszSource = pMDComment->szComment;
            pszTarget = pszMemMetaData + wcslen(pszMemMetaData);
            while ( *pszSource != 0 )
            {
              if ( *pszSource == L'<' )
              {
                wcscpy( pszTarget, L"&lt;" );
                pszTarget += 4;
                pszSource++;
              }
              else if ( *pszSource == L'>' )
              {
                wcscpy( pszTarget, L"&gt;" );
                pszTarget += 4;
                pszSource++;
              }
              else if ( *pszSource == L'&' )
              {
                wcscpy( pszTarget, L"&amp;" );
                pszTarget += 5;
                pszSource++;
              }
              else
              {
                *pszTarget++ = *pszSource++;
              } /* endif */                 
            } /* endwhile */               
            wcscpy( pszTarget, L"</Note>" );
            pszMemMetaData = pszTarget + wcslen(pszTarget);
          }
          break;
        case MD_NOTELIST_TYPE:
        case MD_HISTORY_TYPE:
        case MD_STATUS_TYPE:
        default:
          // ignore these elements
          break;
      } /*endswitch */
      pCurElement = MDGetNextElement( pCurElement );
    } /*endwhile */
  } /* endif */

  // terminate TM meta data buffer
  *pszMemMetaData = 0;

  return( fOK );
} /* end of MDConvertToMemMetadata */

