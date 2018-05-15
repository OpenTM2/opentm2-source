/*! \file
	EQF Folder Handler Document Processing Option dialog

	Copyright Notice:

	Copyright (C) 1990-2014, International Business Machines
	Corporation and others. All rights reserved
*/

#define INCL_EQF_FOLDER           // folder list and document list functions
#define INCL_EQF_TM               // Translation Memory functions
#define INCL_EQF_TP               // public translation processor functions
#define INCL_EQF_EDITORAPI        // editor API
#define INCL_EQF_TAGTABLE         // tagtable defines and functions
#include <eqf.h>                  // General Translation Manager include file
#include "eqfdde.h"               // batch mode definitions
#include "eqffol00.h"             // our .h stuff
#include "eqffol.id"              // our ID file

/**********************************************************************/
/* Private data types                                                 */
/**********************************************************************/

// structure to store strings loaded from the resource file
typedef struct _STRINGDATA
{
  SHORT       sID;                     // ID of string
  CHAR        szString[50];            // data of string
} STRINGDATA;

/**********************************************************************/
/* Static data                                                        */
/**********************************************************************/

// table to store strings loaded from the resource file
// Note: - the entries must be in the same order as the enumeration
//         DOSSTAT defined in EQFFOL00.H
//       - the szString value is filled during WM_INIDLG handling
STRINGDATA StringData[] =
{
//   String ID                      String      Enum value
   { SID_FOLIMPDOC_NEWER_STATE,     "" },       // NEWER_DOCSTAT
   { SID_FOLIMPDOC_NEW_STATE,       "" },       // NEW_DOCSTAT
   { SID_FOLIMPDOC_EQUAL_STATE,     "" },       // EQUAL_DOCSTAT
   { SID_FOLIMPDOC_OLDER_STATE,     "" },       // OLDER_DOCSTAT
   { SID_FOLIMPDOC_NONE_STATE,      "" },       // NONE_DOCSTAT
   { SID_FOLIMPDOC_NONEW_STATE,     "" },       // OLDEXIST_DOCSTAT
   { SID_FOLIMPDOC_COPY_SRCTGT,     "" },       // COPYSRCTGT_DOCDISP
   { SID_FOLIMPDOC_COPY_SRC,        "" },       // COPYSRC_DOCDISP
   { SID_FOLIMPDOC_COPY_SRC_DEL_TGT,"" },       // COPYSRCDELTGT_DOCDISP
   { SID_FOLIMPDOC_COPY_TGT,        "" },       // COPYTGT_DOCDISP
   { SID_FOLIMPDOC_COPY_NONE,       "" },       // IGNORE_DOCDISP
   { 0,                             "" },       // end-of-table indicator
};


/**********************************************************************/
/* Prototype section                                                  */
/**********************************************************************/
MRESULT FolImpDocDlgCommand( HWND, SHORT, SHORT );
BOOL FolImpDocMakeListItem
(
  PFOLIMPDOCDATA pDoc,                 // document data
  PSZ            pszBuffer,            // ptr to buffer area
  LONG           lNum                  // number of item in document array
);

/**********************************************************************/
/* Definitions for our column listbox                                 */
/**********************************************************************/
static CHAR ColHdr[4][80];             // Buffer for column header texts
static CLBCOLDATA ColTable[] =
{ { ColHdr[0],   CLB_MAX_DOC_LENGTH,   TEXT_DATA,      DT_LEFT          },
  { ColHdr[1],    8,                   TEXT_DATA,      DT_LEFT          },
  { ColHdr[2],    8,                   TEXT_DATA,      DT_LEFT          },
  { ColHdr[3],   30,                   TEXT_DATA,      DT_LEFT          },
  { "",           5,                   TEXT_DATA,      DT_LEFT          },
  { NULL,         0,                   TEXT_DATA,      0                } };

static SHORT sLastUsedView[MAX_VIEW+1] = { 0, 1, 2, 3, CLBLISTEND };
static SHORT sDefaultView[MAX_VIEW+1]  = { 0, 1, 2, 3, CLBLISTEND };
static SHORT sNameView[MAX_VIEW+1]     = { 0, 1, 2, 3, CLBLISTEND };
static SHORT sDetailsView[MAX_VIEW+1]  = { 0, 1, 2, 3, CLBLISTEND };
static SHORT sSortCriteria[MAX_VIEW+1] = { 0, 1, 2, 3, CLBLISTEND };

static CLBCTLDATA FolImpDocCLBData =
{  sizeof(CLBCTLDATA),                 // size of control structure
   5,                                  // we have 5 data columns
   1,                                  // one character space between columns
   SYSCLR_WINDOWSTATICTEXT,            // paint title in color of static text
   SYSCLR_WINDOW,                      // background is normal window background
   SYSCLR_WINDOWTEXT,                  // paint item in color of window text
   SYSCLR_WINDOW,                      // background is normal window background
   '\x15',                             // use X15 character as data seperator
   sLastUsedView,                      // set current (= last used) view list
   sDefaultView,                       // set default view list
   sDetailsView,                       // set user set details view list
   sNameView,                          // set view list for 'name' view option
   sSortCriteria,                      // set sort criteria list
   ColTable };                         // set address of column definition table

INT_PTR CALLBACK FOLIMPDOCDLGPROC
(
  HWND hwnd,
  WINMSG msg,
  WPARAM mp1,
  LPARAM mp2
)
{
  PFOLIMPIDA      pIda = NULL;                  // pointer to instance area
  MRESULT  mResult = MRFROMSHORT( FALSE );      // result value of procedure

  switch (msg)
  {
    case WM_EQF_QUERYID:
      HANDLEQUERYID( ID_FOLIMPDOC_DLG, mp2 );
      break;

    case WM_INITDLG:
      {
        BOOL fOK = TRUE;
        HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);

        /**************************************************************/
        /* Anchor IDA                                                 */
        /**************************************************************/
        if ( fOK )
        {
          pIda = (PFOLIMPIDA)PVOIDFROMMP2(mp2);

          ANCHORDLGIDA( hwnd, pIda );
        } /* endif */

        /**************************************************************/
        /* Set initial state of dialog controls                       */
        /**************************************************************/
        if ( fOK )
        {
          ENABLECTRL( hwnd, ID_FOLIMPDOC_REPLACE_PB,  FALSE );
          ENABLECTRL( hwnd, ID_FOLIMPDOC_IGNORE_PB,   FALSE );
        } /* endif */

        /**************************************************************/
        /* Load strings from resource file                            */
        /**************************************************************/
        if ( fOK )
        {
          SHORT i = 0;

          // load column listbox title strings
          LOADSTRING( NULLHANDLE, hResMod, SID_FOLIMPDOC_DOCUMENT_HD, ColHdr[0] );
          LOADSTRING( NULLHANDLE, hResMod, SID_FOLIMPDOC_SOURCE_HD,   ColHdr[1] );
          LOADSTRING( NULLHANDLE, hResMod, SID_FOLIMPDOC_TRANSL_HD,   ColHdr[2] );
          LOADSTRING( NULLHANDLE, hResMod, SID_FOLIMPDOC_ACTION_HD,   ColHdr[3] );

          // load string used within column listbox
          while ( StringData[i].sID != 0 )
          {
            LOADSTRING( NULLHANDLE, hResMod, StringData[i].sID,
                        (StringData[i].szString) );
            i++;
          } /* endwhile */

          // set columns in list control
          {
            LVCOLUMN Column;
            int iWidth[4] = { 20, 10, 10, 50 };        // column widths in chars
            int iRest = 0;  
            int i = 0;
            HWND hwndList = GetDlgItem( hwnd, ID_FOLIMPDOC_DOC_LB );

            // get width of control
            {
              RECT Rect;
              GetWindowRect( hwndList, &Rect );
              iRest = Rect.right - Rect.left;
            }

            // create columns
            for( i = 0; i < 4; i++ )
            {
              memset( &Column, 0, sizeof(Column) );
              Column.mask = LVCF_TEXT | LVCF_ORDER | LVCF_WIDTH;
              Column.pszText = ColHdr[i];

              // use rest of width for last column
              if ( i == 3)
              {
                Column.cx = max( iRest, (int)(10 * UtlQueryULong( QL_AVECHARWIDTH )) );
              }
              else
              {
                Column.cx = iWidth[i] * UtlQueryULong( QL_AVECHARWIDTH );
                iRest -= Column.cx;
              } /* endif */
              Column.iOrder = i;
              ListView_InsertColumn( hwndList, i, &Column );
            } /* endfor */
          }
        } /* endif */

        /**************************************************************/
        /* Fill document listbox                                      */
        /**************************************************************/
        if ( fOK )
        {
           PFOLIMPDOCDATA pDoc = pIda->pDocData;
           LVITEM Item;
           HWND hwndList = GetDlgItem( hwnd, ID_FOLIMPDOC_DOC_LB );
           int iIndex = 0;

           while ( pDoc->pszDocObjName != NULL )
           {
             int iItem = 0;
             char szName[MAX_FILESPEC];

             memset( &Item, 0, sizeof(Item) );
             Item.mask = LVIF_TEXT | LVIF_PARAM;
             Item.lParam = iIndex;
             if ( pDoc->pszLongName != NULL )
             {
               OEMTOANSI( pDoc->pszLongName );
               Item.pszText = pDoc->pszLongName;
             }
             else
             {
               strcpy( szName, UtlGetFnameFromPath( pDoc->pszDocObjName ) );
               Item.pszText = szName;
             }

             iItem = ListView_InsertItem( hwndList, &Item );

             if ( pDoc->pszLongName != NULL )
             {
               ANSITOOEM( pDoc->pszLongName );
             } /* endif */


             // source state
             Item.iItem = iItem;
             Item.iSubItem = 1;
             Item.mask = LVIF_TEXT; 
             Item.pszText = StringData[pDoc->bSourceState].szString;
             ListView_SetItem( hwndList, &Item ); 

             // target state
             Item.iItem = iItem;
             Item.iSubItem = 2;
             Item.mask = LVIF_TEXT; 
             Item.pszText = StringData[pDoc->bTargetState].szString;
             ListView_SetItem( hwndList, &Item ); 

             // action
             Item.iItem = iItem;
             Item.iSubItem = 3;
             Item.mask = LVIF_TEXT; 
             Item.pszText = StringData[pDoc->bDisposition].szString;
             ListView_SetItem( hwndList, &Item ); 

              // continue with next document
              pDoc++;
              iIndex++;
           } /* endwhile */
        } /* endif */

        /**************************************************************/
        /* Show the dialog window                                     */
        /**************************************************************/
        if ( fOK )
        {
          /**********************************************************/
          /* Keep dialog within TWB                                 */
          /**********************************************************/
          {
            SWP  swpDlg, swpTWB;

            /********************************************************/
            /* Get dialog size/position                             */
            /********************************************************/
            WinQueryWindowPos( hwnd, &swpDlg );

            /********************************************************/
            /* Ensure that dialog is not outside of the TWB         */
            /********************************************************/
            UtlKeepInTWB( &swpDlg );

            /********************************************************/
            /* Center dialog within TWB                             */
            /********************************************************/
            WinQueryWindowPos( (HWND)UtlQueryULong( QL_TWBCLIENT ), &swpTWB );
            if ( (swpDlg.x > 0) && ((swpDlg.x + swpDlg.cx) < swpTWB.cx) )
            {
              swpDlg.x = (swpTWB.cx - swpDlg.cx) / 2;
            } /* endif */
            if ( (swpDlg.y > 0) && ((swpDlg.y + swpDlg.cy) < swpTWB.cy) )
            {
              swpDlg.y = (swpTWB.cy - swpDlg.cy) / 2;
            } /* endif */

            WinSetWindowPos( hwnd, HWND_TOP,
                             swpDlg.x, swpDlg.y, swpDlg.cx, swpDlg.cy,
                             EQF_SWP_MOVE |
                             EQF_SWP_SHOW | EQF_SWP_ACTIVATE );
          }
        }
      }
      SETFOCUS( hwnd, ID_FOLIMPDOC_DOC_LB );
      mResult = MRFROMSHORT(TRUE);
      break;

      case WM_COMMAND:
         mResult = FolImpDocDlgCommand( hwnd, WMCOMMANDID( mp1, mp2 ),
                                     WMCOMMANDCMD( mp1, mp2 ) );
         break;

      case WM_CLOSE:
         WinDismissDlg( hwnd, SHORT1FROMMP1(mp1) );
         break;

      case WM_NOTIFY:
        if ( (int)mp1 == ID_FOLIMPDOC_DOC_LB )
        {
          int iSelected = ListView_GetSelectedCount( GetDlgItem( hwnd, ID_FOLIMPDOC_DOC_LB ) );

          ENABLECTRL( hwnd, ID_FOLIMPDOC_REPLACE_PB, (iSelected > 0) );
          ENABLECTRL( hwnd, ID_FOLIMPDOC_IGNORE_PB, (iSelected > 0) );
        } /* endif */
        break;
      case WM_DESTROY:
         mResult = WinDefDlgProc( hwnd, msg, mp1, mp2 );
         break;

      default:
         mResult = WinDefDlgProc( hwnd, msg, mp1, mp2 );
         break;
  } /* endswitch */

  return ( mResult );

} /* end FolImpDocDlgProc */


/**********************************************************************/
/* Handling for WM_COMMAND message                                    */
/**********************************************************************/
MRESULT FolImpDocDlgCommand
(
   HWND hwnd,                          // dialog handle
   SHORT sId,                          // id of button
   SHORT sNotification                 // notification type
)
{
  PFOLIMPIDA pIda;                     // ptr to folder import IDA
  MRESULT mResult = MRFROMSHORT(TRUE);
  BOOL        fOK;                    // internal OK flag

  sNotification;

  // --- get IDA pointer ---
  pIda = ACCESSDLGIDA( hwnd, PFOLIMPIDA );

  switch ( sId )
  {
     case ID_FOLIMPDOC_HELP_PB:
       UtlInvokeHelp();
       break;

     case ID_FOLIMPDOC_SET_PB:
        fOK = TRUE;                   // assume everything is o.k.

        if ( fOK )
        {
          POSTCLOSE( hwnd, TRUE );
        } /* endif */
        break;

     case ID_FOLIMPDOC_REPLACE_PB:
     case ID_FOLIMPDOC_IGNORE_PB:
        {
          int iSelected = -1;
          HWND hwndList = GetDlgItem( hwnd, ID_FOLIMPDOC_DOC_LB );
          pIda = ACCESSDLGIDA( hwnd, PFOLIMPIDA );

          do 
          {
            iSelected = ListView_GetNextItem( hwndList, iSelected, LVNI_SELECTED );

            if ( iSelected >= 0 )
            {
               PFOLIMPDOCDATA pDoc;
               LVITEM Item;

               memset( &Item, 0, sizeof(Item) );
               Item.mask = LVIF_PARAM;
               Item.iItem = iSelected;
               ListView_GetItem( hwndList, &Item );
               pDoc = pIda->pDocData + Item.lParam;

               if ( sId == ID_FOLIMPDOC_IGNORE_PB )
               {
                 pDoc->bDisposition = IGNORE_DOCDISP;
               }
               else
               {
                 if ( pDoc->bTargetState == NONE_DOCSTAT )
                 {
                   pDoc->bDisposition = COPYSRC_DOCDISP;
                 }
                  else if ( pDoc->bTargetState == OLDEXIST_DOCSTAT )
                 {
                   pDoc->bDisposition = COPYSRCDELTGT_DOCDISP;
                  }
                 else if ( pDoc->bSourceState == EQUAL_DOCSTAT )
                 {
                    pDoc->bDisposition = COPYTGT_DOCDISP;
                 }
                 else
                 {
                   pDoc->bDisposition = COPYSRCTGT_DOCDISP;
                 } /* endif */
               } /* endif */

               // update item disposition
               memset( &Item, 0, sizeof(Item) );
               Item.mask = LVIF_TEXT;
               Item.iItem = iSelected;
               Item.iSubItem = 3;
               Item.pszText = StringData[pDoc->bDisposition].szString;
               ListView_SetItem( hwndList, &Item );
            } /* endif */
          } while ( iSelected >= 0 ); /* enddo */
        }
        break;

      break;
       break;

     case ID_FOLIMPDOC_CANCEL_PB:
     case DID_CANCEL:
        POSTCLOSE( hwnd, 0 );
        break;
  } /* endswitch */

  return( mResult );
} /* end of FolImpDocDlgCommand */


/**********************************************************************/
/* Build a document item for our column listbox                       */
/**********************************************************************/
BOOL FolImpDocMakeListItem
(
  PFOLIMPDOCDATA pDoc,                 // document data
  PSZ            pszBuffer,            // ptr to buffer area
  LONG           lNum                  // number of item in document array
)
{
  CHAR szNum[10];                      // buffer for number as character string

  // document name
  if ( pDoc->pszLongName != NULL )
  {
    OEMTOANSI( pDoc->pszLongName );
    strcpy( pszBuffer, pDoc->pszLongName );
    ANSITOOEM( pDoc->pszLongName );
  }
  else
  {
    strcpy( pszBuffer, UtlGetFnameFromPath( pDoc->pszDocObjName ) );
  }
  strcat( pszBuffer, X15_STR );

  // source state
  strcat( pszBuffer, StringData[pDoc->bSourceState].szString );
  strcat( pszBuffer, X15_STR );

  // target state
  strcat( pszBuffer, StringData[pDoc->bTargetState].szString );
  strcat( pszBuffer, X15_STR );

  // action
  strcat( pszBuffer, StringData[pDoc->bDisposition].szString );
  strcat( pszBuffer, X15_STR );

  // index in document data array
  itoa( lNum, szNum, 10 );
  strcat( pszBuffer, szNum );

  return( TRUE );
} /* end of function FolImpDocMakeListItem */
