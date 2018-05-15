//+----------------------------------------------------------------------------+
//|  EQFDIC01.C - EQF Dictionary Handler dialog procedures                     |
//+----------------------------------------------------------------------------+
//|  Copyright Notice:                                                         |
//|                                                                            |
//|      Copyright (C) 1990-2012, International Business Machines              |
//|      Corporation and others. All rights reserved                           |

//+----------------------------------------------------------------------------+
//|  Description:                                                              |
//+----------------------------------------------------------------------------+
//|  Entry Points:                                                             |
//+----------------------------------------------------------------------------+
//|  Externals:                                                                |
//+----------------------------------------------------------------------------+
//|  Internals:                                                                |
//+----------------------------------------------------------------------------+
//|  Include files:                                                            |
//+----------------------------------------------------------------------------+
//|  To be done / known limitations / caveats:                                 |
//|      all places marked with 'TBD' need rework and or new code !!!          |
//|                                                                            |
//+----------------------------------------------------------------------------+
#define INCL_EQF_TM               // general Transl. Memory functions
#define INCL_EQF_TMREMOTE         // remote Transl. Memory functions (U-Code)
#define INCL_EQF_ASD              // dictionary access functions (Asd...)
#define INCL_EQF_LDB              // dictionary data encoding functions
#define INCL_EQF_DICT             // dictionary handler functions
#define INCL_EQF_DLGUTILS         // dialog utilities
#include <eqf.h>                  // General Translation Manager include file

#include <eqfdtag.h>              // include tag definitions
#include "eqfdde.h"
#include "eqfdicti.h"             // Private include file of dictionary handler
#include "eqfdasdi.h"             // internal ASD file (for field system names)
#include "OtmDictionaryIF.H"

#include "eqfrdics.h"             // remote include file

#include "eqfdic00.id"            // dialog IDs
#include "eqfdimp.id"             // dialog IDs
#include "eqfdprop.id"            // dialog IDs

#include "SHLOBJ.H"            // folder browse function

extern HELPSUBTABLE hlpsubtblDicPropDlg[];  //help processing

//+----------------------------------------------------------------------------+
//|  Open Dictionary / Dictionary Lookup                                       |
//|                                                                            |
//+----------------------------------------------------------------------------+
VOID DictionaryOpen( HWND hwndParent, PSZ pszName, PDICTIONARYIDA pIda )
{
  USHORT          usRc;                  // buffer for return code
  static CHAR     szBuffer[MAX_EQF_PATH];// buffer for path names
  static CHAR     szName[MAX_EQF_PATH];  // buffer for path names
  static CHAR     szFName[MAX_FNAME];    // buffer for dict names
  static CHAR     szDrive[MAX_DRIVE];    // buffer for drive
  BOOL            fOK = TRUE;            // internal OK flag
  PSZ             pszProfName;           // ptr to prop file string
  USHORT          usErrDict;             // dicts in error
  PSZ             pError;                // ptr to error string
  PSZ             pszServer;             // ptr to server name
  SHORT           sRC;                   // return code
  PPROPDICTIONARY pDictProp = NULL;      // pointer to dictionary properties
  HPROP           hDictProp;             // handle of dict properties
  HPROP           hDictListProp;         // handle of dictlist properties
  PPROPDICTLIST   pDictListProp;         // ptr to dictionary properties
  EQFINFO         ErrorInfo;             // error return code
  BOOL            fFree = FALSE;         // dict locked or not
  SHORT           sItem;                 // dict list box item
  PSZ             pszMsgError[5];        // QDAM error string array
  CHAR            szDictName[MAX_FNAME]; // buffer for dictionary short name
  PSZ             pszErrName;            // name to be used in error messages

  BOOL fIsNew = FALSE;                   // is-new flag
  ObjLongToShortName( pszName, szDictName, DICT_OBJECT, &fIsNew );
  strcpy( pIda->szLongName, pszName );
  OEMTOANSI( pIda->szLongName );
  pszErrName = pIda->szLongName;

  //check if symbol exists (if dict in use by another process)
  UtlMakeEQFPath( szBuffer, NULC, SYSTEM_PATH, (PSZ) NULP );
  sprintf( szName, "%s\\%s%s", szBuffer, szDictName, EXT_OF_DICTPROP );
  sRC = QUERYSYMBOL( szName );

  if ( sRC != -1 )
  {
    UtlError( ERROR_DICT_LOCKED, MB_CANCEL, 1, &pszErrName, EQF_ERROR );
    fOK = FALSE;
  }
  else
  {
    if ( pIda->usDictNum < 10 )
    {
      SETSYMBOL( szName );
      strcpy( pIda->szLookupDictName[pIda->usDictNum], szName );
      fFree = TRUE;
    }
    else
    {
      fOK = FALSE;
      UtlError( INFO_DIC_MAXSELECTED, MB_CANCEL, 0, NULL, EQF_ERROR );
    } /* endif */
  } /* endif */

  if ( fOK )
  {
    //open properties and check if they fit to dictionary
    PROPNAME ( szBuffer, szDictName );
    hDictProp = OpenProperties( szBuffer, NULL,
                                PROP_ACCESS_WRITE, &ErrorInfo);
    if ( !hDictProp )
    {
      UtlError( ERROR_OPENING_PROPS, MB_CANCEL, 1, &pszErrName, EQF_ERROR );
      fOK = FALSE;

      sItem = QUERYSELECTION( hwndParent, PID_DICTIONARY_LB );
      QUERYITEMTEXT( hwndParent, PID_DICTIONARY_LB , sItem, szBuffer );
      pszServer = UtlParseX15( szBuffer, DIC_SERVER_IND);
      if ( pszServer[0] == NULC )
      {
        sItem = QUERYSELECTION( hwndParent, PID_DICTIONARY_LB );
        //grey out dictionary as it cannot be accessed
        WinSendDlgItemMsg( hwndParent, PID_DICTIONARY_LB,
                           LM_EQF_SETITEMSTATE, MP1FROMSHORT (sItem), MP2FROMSHORT(FALSE) );
      } /* endif */
    }
    else
    {
      pDictProp =(PPROPDICTIONARY) MakePropPtrFromHnd( hDictProp );

      if ( fOK )
      {
        //fill msg parameter array for QDAM error messages
        Utlstrccpy(szFName, UtlGetFnameFromPath(pDictProp->szDictPath), DOT);
        pszMsgError[0] = pszErrName;
        //current entry
        pszMsgError[1] = EMPTY_STRING;
        //dictionary drive
        szDrive[0] = pDictProp->szDictPath[0];
        szDrive[1] = EOS;
        pszMsgError[2] = szDrive;
        //server name
        pszMsgError[3] = pDictProp->szServer;
        //source lang                                     //msga
        pszMsgError[4] = pDictProp->szSourceLang;         //msga
      } /* endif */
    } /* endif */

    if ( fOK )
    {
      /***************************************************************/
      /* Switch to wait cursor - this action may take a while ...    */
      /***************************************************************/
      SETCURSOR( SPTR_WAIT );

      usRc = AsdBegin( 2, &pIda->hUser[pIda->usDictNum] );

      if ( usRc != LX_RC_OK_ASD )
      {
        SETCURSOR( SPTR_ARROW );
        pError =  QDAMErrorString( usRc, pszMsgError );
        UtlError ( usRc, MB_CANCEL, 1, &pError, QDAM_ERROR );
        fOK = FALSE;
        fFree = TRUE;
      }
      else
      {
        //open ASD dictionary
        UtlMakeEQFPath( szBuffer, NULC, PROPERTY_PATH, (PSZ) NULP );
        strcat( szBuffer, BACKSLASH_STR );
        strcat( szBuffer, szDictName );
        strcat( szBuffer, EXT_OF_DICTPROP );
        pszProfName = szBuffer;
        usRc = AsdOpen( pIda->hUser[pIda->usDictNum],  // user ctl block hdl
                        ASD_GUARDED,      // open flags
                        1,                // nr of dict in ppszDict
                        &pszProfName,     // dictionary properties
                        &pIda->hDict[pIda->usDictNum], // dict ctl block hdl
                        &usErrDict );     // number of failing dict

        if ( usRc != LX_RC_OK_ASD )
        {
          SETCURSOR( SPTR_ARROW );
          pError =  QDAMErrorString( usRc, pszMsgError );
          UtlError ( usRc, MB_CANCEL, 1, &pError, QDAM_ERROR );
          fOK = FALSE;
          fFree = TRUE;

          //if not remote and access denied then grey out and allow delete
          if ( pDictProp->szServer[0] == NULC )
          {
            if ( (usRc == BTREE_OPEN_FAILED)   ||
                 (usRc == BTREE_ACCESS_ERROR)  ||
                 (usRc == LX_OPEN_FLD_ASD)     ||
                 (usRc == BTREE_FILE_NOTFOUND) ||
                 (usRc == BTREE_INVALID_DRIVE) )
            {
              //return sitem in dict list window
              sItem = QUERYSELECTION( hwndParent, PID_DICTIONARY_LB );
              if ( sItem != LIT_NONE )
              {
                //send msg to grey out dict in dict list window
                WinSendDlgItemMsg( hwndParent, PID_DICTIONARY_LB,
                                   LM_EQF_SETITEMSTATE,
                                   MP1FROMSHORT (sItem),
                                   MP2FROMSHORT (FALSE) );
              } /* endif */
            } /* endif */
          } /* endif */
        } /* endif */
      } /* endif */
    } /* endif */

    //close dict properties
    if ( hDictProp )
      CloseProperties( hDictProp, PROP_QUIT, &ErrorInfo);
  } /* endif */

  if ( !fOK )
  {
    if ( fFree )
    {
      sRC = REMOVESYMBOL( szName );
    } /* endif */

    if ( pIda->usDictNum < 10 )
    {
      if ( pIda->hDict[pIda->usDictNum] )
      {
        AsdClose( pIda->hUser[pIda->usDictNum], pIda->hDict[pIda->usDictNum] );
      } /* endif */
      if ( pIda->hUser[pIda->usDictNum] )
      {
        AsdEnd( pIda->hUser[pIda->usDictNum] );
      } /* endif */
    } /* endif */
  } /* endif */

  if ( fOK )
  {
    //open properties
    UtlMakeEQFPath( szBuffer, NULC, SYSTEM_PATH,(PSZ) NULP );
    hDictListProp = OpenProperties( DICT_PROPERTIES_NAME, szBuffer,
                                    PROP_ACCESS_READ, &ErrorInfo );
    if ( !hDictListProp )
    {
      pError = DICT_PROPERTIES_NAME;
      UtlError( ERROR_OPEN_PROPERTIES, MB_CANCEL, 1, &pError, EQF_ERROR);
      fOK = FALSE;
    }
    else
    {
      pDictListProp = (PPROPDICTLIST) MakePropPtrFromHnd( hDictListProp );

      //load lookup dialogs if not done yet
      LupBegin( pIda->hUser[pIda->usDictNum],  //Asd services user ctl blk
                pIda->hDict[pIda->usDictNum],  //Asd services dict ctl blk
                hwndParent,      // parent handle for dialogs and messages
                WM_EQF_WD_MAIN_NOTIFY, // message used for notifications
                &pDictListProp->rclDisp,  // size/position of display dialog
                NULL,            // size/position of edit dialog or NULL
                &pIda->hLookup[pIda->usDictNum],  //Lookup services ctl blk
                &pIda->usLUPID[pIda->usDictNum] );//Lookup ID

      CloseProperties( hDictListProp, PROP_QUIT, &ErrorInfo);
    } /* endif */
  } /* endif */

  // do lookup
  SETCURSOR( SPTR_ARROW );
  if ( fOK && pIda->hLookup[pIda->usDictNum] )
  {
    pIda->usDictNum++;
    LupLookup( pIda->hLookup[pIda->usDictNum-1], L"" );
  } /* endif */
}

//+----------------------------------------------------------------------------+
//|  Create Dictionary / Dictionary Property Function                          |
//|                                                                            |
//+----------------------------------------------------------------------------+
VOID DictionaryProp( HWND hwndParent, PSZ pszName )
{
  PDICCREATEIDA   pIda;               // ptr to instance data area
  BOOL            fOK = TRUE;         // success indicator

  //allocate memory
  fOK = UtlAlloc( (PVOID *) &pIda, 0L, (LONG) sizeof(DICCREATEIDA), ERROR_STORAGE );

  if ( fOK )
  {
    BOOL fIsNew = FALSE;         // is-new flag
	HMODULE hResMod;
	hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
    ObjLongToShortName( pszName, pIda->szDicName, DICT_OBJECT, &fIsNew );
    strcpy( pIda->szLongName, pszName );
    pIda->hwndDictLB = hwndParent;
    DIALOGBOX( EqfQueryTwbClient(), DICTIONARYPROPERTYDLGPROC,
               hResMod, ID_DICTPROP_DLG, pIda, fOK );

  } /* endif */
}  /* endDictionaryProp */




MRESULT DictPropControl
(
HWND   hwndDlg,                     // dialog handle
SHORT  sId,                         // id in action
SHORT  sNotification                // notification
)
{
  PDICCREATEIDA    pIda;               // ptr to instance data area
  SHORT            sItem;              // index of listbox items
  MRESULT          mResult = FALSE;    // dialog procedure return value

  pIda = ACCESSDLGIDA( hwndDlg, PDICCREATEIDA );

  switch ( sId )
  {
  case ID_DICPROP_ENTRYFIELDS_LB:
    if ( sNotification == LN_SELECT )
    {
      sItem = QUERYSELECTION( hwndDlg, ID_DICPROP_ENTRYFIELDS_LB );
      if ( sItem != LIT_NONE )
      {
        pIda->pEntry = (PPROFENTRY) QUERYITEMHANDLE( hwndDlg,
                                                     ID_DICPROP_ENTRYFIELDS_LB, sItem );

        switch ( pIda->pEntry->usDisplay )
        {
        case 0:
          SETCHECK_TRUE ( hwndDlg, ID_DICPROP_OMIT_RB);
          SETCHECK_FALSE( hwndDlg, ID_DICPROP_L1_RB );
          SETCHECK_FALSE( hwndDlg, ID_DICPROP_L2_RB );
          SETCHECK_FALSE( hwndDlg, ID_DICPROP_L3_RB );
          break;
        case 1:
          SETCHECK_TRUE ( hwndDlg, ID_DICPROP_L1_RB );
          SETCHECK_FALSE( hwndDlg, ID_DICPROP_OMIT_RB );
          SETCHECK_FALSE( hwndDlg, ID_DICPROP_L2_RB );
          SETCHECK_FALSE( hwndDlg, ID_DICPROP_L3_RB );
          break;
        case 2:
          SETCHECK_TRUE( hwndDlg, ID_DICPROP_L2_RB  );
          SETCHECK_FALSE( hwndDlg, ID_DICPROP_OMIT_RB );
          SETCHECK_FALSE( hwndDlg, ID_DICPROP_L1_RB );
          SETCHECK_FALSE ( hwndDlg, ID_DICPROP_L3_RB);
          break;
        case 3:
          SETCHECK_TRUE( hwndDlg, ID_DICPROP_L3_RB  );
          SETCHECK_FALSE( hwndDlg, ID_DICPROP_OMIT_RB );
          SETCHECK_FALSE( hwndDlg, ID_DICPROP_L1_RB );
          SETCHECK_FALSE( hwndDlg, ID_DICPROP_L2_RB );
          break;
        } /* endswitch */
        switch ( pIda->pEntry->usLevel )
        {
        case 1:
          ENABLECTRL( hwndDlg, ID_DICPROP_LEV1_TEXT, TRUE );
          ENABLECTRL( hwndDlg, ID_DICPROP_LEV2_TEXT, FALSE );
          ENABLECTRL( hwndDlg, ID_DICPROP_LEV3_TEXT, FALSE );
          ENABLECTRL( hwndDlg, ID_DICPROP_LEV4_TEXT, FALSE );
          break;
        case 2:
          ENABLECTRL( hwndDlg, ID_DICPROP_LEV2_TEXT, TRUE );
          ENABLECTRL( hwndDlg, ID_DICPROP_LEV1_TEXT, FALSE );
          ENABLECTRL( hwndDlg, ID_DICPROP_LEV3_TEXT, FALSE );
          ENABLECTRL( hwndDlg, ID_DICPROP_LEV4_TEXT, FALSE );
          break;
        case 3:
          ENABLECTRL( hwndDlg, ID_DICPROP_LEV3_TEXT, TRUE );
          ENABLECTRL( hwndDlg, ID_DICPROP_LEV1_TEXT, FALSE );
          ENABLECTRL( hwndDlg, ID_DICPROP_LEV2_TEXT, FALSE );
          ENABLECTRL( hwndDlg, ID_DICPROP_LEV4_TEXT, FALSE );
          break;
        case 4:
          ENABLECTRL( hwndDlg, ID_DICPROP_LEV4_TEXT, TRUE );
          ENABLECTRL( hwndDlg, ID_DICPROP_LEV1_TEXT, FALSE );
          ENABLECTRL( hwndDlg, ID_DICPROP_LEV2_TEXT, FALSE );
          ENABLECTRL( hwndDlg, ID_DICPROP_LEV3_TEXT, FALSE );
          break;
        } /* endswitch */
        switch ( pIda->pEntry->usEntryFieldType )
        {
        case 1:
          ENABLECTRL( hwndDlg, ID_DICPROP_SMALL_RB, TRUE );
          SETCHECK_TRUE( hwndDlg, ID_DICPROP_SMALL_RB );
          SETCHECK_FALSE( hwndDlg, ID_DICPROP_LARGE_RB );
          if ( !pIda->fNewDict )
            //grey out small btn if originally field large
            ENABLECTRL( hwndDlg, ID_DICPROP_LARGE_RB, FALSE );
          break;
        case 2:
          ENABLECTRL( hwndDlg, ID_DICPROP_LARGE_RB, TRUE );
          SETCHECK_TRUE( hwndDlg, ID_DICPROP_LARGE_RB  );
          SETCHECK_FALSE( hwndDlg, ID_DICPROP_SMALL_RB  );
          if ( !pIda->fNewDict )
            //grey out small btn if originally field large
            ENABLECTRL( hwndDlg, ID_DICPROP_SMALL_RB, FALSE );
          break;
        } /* endswitch */

        /********************************************************/
        /* Handle automatic lookup checkbox                     */
        /********************************************************/
        if ( (strcmp( pIda->pEntry->chSystName, HEADWORD_SYST_NAME) == 0)
             ||
             (strcmp( pIda->pEntry->chSystName, SYSNAME_TRANSLATION) == 0) )
        {
          /******************************************************/
          /* Headword and translation have always the           */
          /* checkbox switched on, set the checkbox on and      */
          /* disable the checkbox to avoid tampering with it    */
          /******************************************************/
          ENABLECTRL( hwndDlg, ID_DICPROP_AUTLOOKUP_CB, FALSE );
          SETCHECK_TRUE( hwndDlg, ID_DICPROP_AUTLOOKUP_CB  );
        }
        else
        {
          ENABLECTRL( hwndDlg, ID_DICPROP_AUTLOOKUP_CB, TRUE );
          SETCHECK( hwndDlg, ID_DICPROP_AUTLOOKUP_CB,
                    pIda->pEntry->fAutLookup );
        } /* endif */
      } /* endif */
    } /* endif */
    break;

  case ID_DICPROP_OMIT_RB:
    pIda->pEntry->usDisplay = 0;
    break;

  case ID_DICPROP_L1_RB:
    pIda->pEntry->usDisplay = 1;
    break;

  case ID_DICPROP_L2_RB:
    pIda->pEntry->usDisplay = 2;
    break;

  case ID_DICPROP_L3_RB:
    pIda->pEntry->usDisplay = 3;
    break;

  case ID_DICPROP_SMALL_RB:
    pIda->pEntry->usEntryFieldType = 1;
    break;

  case ID_DICPROP_LARGE_RB:
    pIda->pEntry->usEntryFieldType = 2;
    break;

  case ID_DICPROP_AUTLOOKUP_CB:
    pIda->pEntry->fAutLookup = QUERYCHECK( hwndDlg,
                                           ID_DICPROP_AUTLOOKUP_CB );
    break;
  } /* endswitch */

  return( mResult );
}


PSZ QDAMErrorString( USHORT usRc,               //passed dict return code
                     PSZ   *ppMsgError )      //message array
{
  PSZ     pszErrorString;       //return error string

  switch (usRc)
  {
  case LX_PROTECTED_ASD:
  case LX_RENUM_RQD_ASD:
  case LX_OPEN_FLD_ASD:
  case BTREE_FILE_NOTFOUND:
  case BTREE_OPEN_FAILED:
  case BTREE_ACCESS_ERROR:
  case BTREE_NETWORK_ACCESS_DENIED:
  case TMERR_PROP_NOT_FOUND:
  case TMERR_PROP_EXIST:
  case TMERR_PROP_WRITE_ERROR:
  case TMERR_PROP_READ_ERROR:
  case BTREE_IN_USE:
    pszErrorString = ppMsgError[0];     //dictionary name
    break;
  case LX_OTHER_USER_ASD:             //current entry
  case LX_WRD_NT_FND_ASD:             //current entry
  case LX_WRD_EXISTS_ASD:             //current entry
    pszErrorString = ppMsgError[1];
    break;
  case  LX_IDX_FN_ERR:
  case  BTREE_INVALID_DRIVE:
    pszErrorString = ppMsgError[2];     //dictionary drive
    break;
  case TMERR_SERVER_NOT_STARTED:
  case TMERR_SERVERCODE_NOT_STARTED:
  case TMERR_COMMUNICATION_FAILURE:
  case TMERR_SERVER_ABOUT_TO_EXIT:
  case TMERR_TOO_MANY_QUERIES:
    pszErrorString = ppMsgError[3];     //server name
    break;
  case LX_BAD_LANG_CODE:
    pszErrorString = ppMsgError[4];     //source language
    break;
  default:
    pszErrorString = EMPTY_STRING;
    break;
  } // endswitch
  return( pszErrorString );
}  /* endQDAMErrorString */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     DicPropPropertySheetLoad                                 |
//+----------------------------------------------------------------------------+
//|Function call:     DicPropertySheetLoad( hwnd, mp2 );                       |
//+----------------------------------------------------------------------------+
//|Description:       handle changes on the tab page                           |
//+----------------------------------------------------------------------------+
//|Parameters:        HWND hwnd   handle of the dialog                         |
//|                   LPARAM  mp2    message parameter 2                       |
//+----------------------------------------------------------------------------+
//|Returncode type:   MRESULT                                                  |
//+----------------------------------------------------------------------------+
//|Returncodes:       return code from default window proc or FALSE            |
//+----------------------------------------------------------------------------+
//|Function flow:     create any pages,                                        |
//|                   load the tabctrl text                                    |
//|                   load the (modeless) dialog, register it and position into|
//|                     tab area                                               |
//|                   return                                                   |
//+----------------------------------------------------------------------------+
BOOL DicPropPropertySheetLoad
(
HWND hwnd,
PDICCREATEIDA     pIda
)
{
  BOOL      fOk = TRUE;
  TC_ITEM   TabCtrlItem;
  USHORT    nItem = 0;
  HWND      hwndTabCtrl;
  HINSTANCE hInst;
  CHAR      szBuffer[80];

  if ( fOk )
  {
    RECT rect;
	HMODULE hResMod;
	hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
    // remember adress of user area
    hInst = GETINSTANCE( hwnd );
    hwndTabCtrl = GetDlgItem( hwnd, ID_DICT_PROP_TABCTRL );
    pIda->hwndTabCtrl = hwndTabCtrl;
    GetClientRect( hwndTabCtrl, &rect );
    TabCtrl_AdjustRect( hwndTabCtrl, FALSE, &rect );

    // leave some additional space at top
    rect.top += 20;
    MapWindowPoints( hwndTabCtrl, hwnd, (POINT *) &rect, 2 );


    TabCtrlItem.mask = TCIF_TEXT | TCIF_PARAM;

    // -----------------------------------------------------------------
    //
    // create the appropriate TAB control and load the associated dialog
    //
    // -----------------------------------------------------------------

    //
    // IDS_DICTPROP_TAB_GENERAL
    //

    LOADSTRING( hab, hResMod, IDS_DICTPROP_TAB_GENERAL , szBuffer );
    TabCtrlItem.pszText = szBuffer;
    TabCtrlItem.lParam = nItem;
    SendMessage(hwndTabCtrl, TCM_INSERTITEM, nItem, (LPARAM)&TabCtrlItem);
    pIda->hwndPages[nItem] =
    CreateDialogParam( hInst,
                       MAKEINTRESOURCE( ID_DICTPROP_GENERAL_DLG ),
                       hwnd,
                       DICTPROP_GENERAL_DLGPROC,
                       (LPARAM)pIda );

    SetWindowPos( pIda->hwndPages[nItem], HWND_TOP,
                  rect.left, rect.top,
                  rect.right-rect.left, rect.bottom-rect.top, SWP_SHOWWINDOW );
    SetFocus( pIda->hwndPages[nItem] );
    UtlRegisterModelessDlg( pIda->hwndPages[nItem] );
    nItem++;

    //
    // IDS_DICTPROP_TAB_OPTIONS
    //

    LOADSTRING( hab, hResMod, IDS_DICTPROP_TAB_OPTIONS , szBuffer );
    TabCtrlItem.pszText = szBuffer;
    TabCtrlItem.lParam = nItem;
    SendMessage(hwndTabCtrl, TCM_INSERTITEM, nItem, (LPARAM)&TabCtrlItem);
    pIda->hwndPages[nItem] =
    CreateDialogParam( hInst,
                       MAKEINTRESOURCE( ID_DICTPROP_OPTIONS_DLG ),
                       hwnd,
                       DICTPROP_OPTIONS_DLGPROC,
                       (LPARAM)pIda );

    SetWindowPos( pIda->hwndPages[nItem], HWND_TOP,
                  rect.left, rect.top,
                  rect.right-rect.left, rect.bottom-rect.top, SWP_SHOWWINDOW );
    SetFocus( pIda->hwndPages[nItem] );
    UtlRegisterModelessDlg( pIda->hwndPages[nItem] );
    nItem++;




  } /* endif */


  // -----------------------------------------------------------------
  //
  // hide all dialog pages but the first one
  //
  // -----------------------------------------------------------------

  if ( fOk )
  {
    int i = 1;
    while ( pIda->hwndPages[i] )
    {
      ShowWindow( pIda->hwndPages[i], SW_HIDE );
      i++;
    } /* endwhile */
  } /* endif */

  if ( !fOk )
  {
    POSTEQFCLOSE( hwnd, FALSE );
  } /* endif */

  return fOk;
}



//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     DicPropPropertySheetNotification                         |
//+----------------------------------------------------------------------------+
//|Function call:     DicPropPropertySheetNotifiaction( hwnd, mp1, mp2);       |
//+----------------------------------------------------------------------------+
//|Description:       handle changes on the tab page                           |
//+----------------------------------------------------------------------------+
//|Parameters:        HWND hwnd   handle of the dialog                         |
//|                   WPARAM  mp1    message parameter 1                       |
//|                   LPARAM  mp2    message parameter 2                       |
//+----------------------------------------------------------------------------+
//|Returncode type:   MRESULT                                                  |
//+----------------------------------------------------------------------------+
//|Returncodes:       return code from default window proc or FALSE            |
//+----------------------------------------------------------------------------+
//|Function flow:     switch ( pNMHdr->code )                                  |
//|                     case TCN_SELCHANGE:                                    |
//|                       activate new page                                    |
//|                     case TCN_SELCHANGING                                   |
//|                       hide the dialog                                      |
//|                   return                                                   |
//+----------------------------------------------------------------------------+
MRESULT DicPropPropertySheetNotification
(
HWND hwnd,
WPARAM  mp1,
LPARAM  mp2
)
{
  NMHDR * pNMHdr;
  USHORT       usTabCtrl;
  MRESULT      mResult = FALSE;
  PDICCREATEIDA     pIda;
  pNMHdr = (LPNMHDR)mp2;

  mp1;
  switch ( pNMHdr->code )
  {
  case TCN_SELCHANGE:
    pIda = ACCESSDLGIDA(hwnd, PDICCREATEIDA);
    if ( pIda )
    {
      TC_ITEM Item;
      HWND hwndTabCtrl = GetDlgItem( hwnd, ID_DICT_PROP_TABCTRL );
      usTabCtrl = (USHORT)TabCtrl_GetCurSel( hwndTabCtrl );
      memset( &Item, 0, sizeof(Item) );
      Item.mask = TCIF_PARAM;
      TabCtrl_GetItem( hwndTabCtrl, usTabCtrl, &Item );
      usTabCtrl = (USHORT)Item.lParam;
      ShowWindow( pIda->hwndPages[ usTabCtrl ], SW_SHOW );
    } /* endif */
    break;
  case TCN_SELCHANGING:
    pIda = ACCESSDLGIDA( hwnd, PDICCREATEIDA );
    if ( pIda )
    {
      /**************************************************************/
      /* Issue a direct call to the appropriate dialog proc with    */
      /* WM_COMMAND, ID_TB_PROP_SET_PB and the second parameter set */
      /* to 1L to force only consistency checking                   */
      /**************************************************************/
      TC_ITEM Item;
      PFNWP pfnWp;
      HWND hwndTabCtrl = GetDlgItem( hwnd, ID_DICT_PROP_TABCTRL );
      usTabCtrl = (USHORT)TabCtrl_GetCurSel( hwndTabCtrl );
      memset( &Item, 0, sizeof(Item) );
      Item.mask = TCIF_PARAM;
      TabCtrl_GetItem( hwndTabCtrl, usTabCtrl, &Item );
      usTabCtrl = (USHORT)Item.lParam;
      pfnWp = (PFNWP) GetWindowLong( pIda->hwndPages[ usTabCtrl ], DWL_DLGPROC );

      mResult = pfnWp( pIda->hwndPages[usTabCtrl], WM_COMMAND,
                       PID_PB_OK, 1L);
      if ( mResult )
      {
        /************************************************************/
        /* stick on the side                                        */
        /* we have to post the request again since one of the system*/
        /* routines thinks that we still want to change the page..  */
        /************************************************************/
        WinPostMsg( hwnd, TCM_SETCURSEL, usTabCtrl, 0L );
      } /* endif */
      ShowWindow( pIda->hwndPages[ usTabCtrl ], SW_HIDE );
    } /* endif */
    break;
  case TTN_NEEDTEXT:
    {
      TOOLTIPTEXT *pToolTipText = (TOOLTIPTEXT *) mp2;
      if ( pToolTipText )
      {
        TC_ITEM Item;
        HWND hwndTabCtrl = GetDlgItem( hwnd, ID_DICT_PROP_TABCTRL );
		HMODULE hResMod;
		hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
        memset( &Item, 0, sizeof(Item) );
        Item.mask = TCIF_PARAM;
        TabCtrl_GetItem( hwndTabCtrl, pToolTipText->hdr.idFrom, &Item );
        switch ( (SHORT)Item.lParam )
        {
        case 0:      // first page
          LOADSTRING( hab, hResMod, IDS_DICTPROP_TAB_GENERAL,
                      pToolTipText->szText );
          break;
        case 1:      // second page
          LOADSTRING( hab, hResMod, IDS_DICTPROP_TAB_OPTIONS,
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
} /* end of function DicPropPropertySheetNotification */



//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     DicPropCommand                                           |
//+----------------------------------------------------------------------------+
//|Function call:     DicPropCommand( hwndDlg, mp1, mp2);                      |
//+----------------------------------------------------------------------------+
//|Description:       handles commands triggered by majors PBs of the dialog   |
//|                    and help triggered by ESC and Help-PB resp.             |
//+----------------------------------------------------------------------------+
//|Parameters:        HWND hwndDlg   handle of the dialog                      |
//|                   WPARAM  mp1    message parameter 1                       |
//|                   LPARAM  mp2    message parameter 2                       |
//+----------------------------------------------------------------------------+
//|Returncode type:   MRESULT                                                  |
//+----------------------------------------------------------------------------+
//|Returncodes:       return code from default window proc or FALSE            |
//+----------------------------------------------------------------------------+
//|Function flow:     switch ( WMCOMMANDID( mp1, mp2 ))                        |
//|                     case DID_CANCEL:                                       |
//|                     case ID_DICPROP_CANCEL_PB                              |
//|                     case ID_DICPROP_PROTECTED_PB                           |
//|                     case ID_DICPROP_CHANGE_PB                              |
//|                   return                                                   |
//+----------------------------------------------------------------------------+


MRESULT DicPropCommand
(
HWND hwndDlg,
WPARAM mp1,
LPARAM mp2
)
{
  PDICCREATEIDA   pIda;               // ptr to instance data area
//   SHORT           sItem;              // index of listbox items
  MRESULT         mResult = FALSE;    // dialog procedure return value
//   PPROPDICTIONARY pProp;       // pointer to dictionary properties
  EQFINFO         ErrorInfo;          // error code of property handler calls
  BOOL            fOK = TRUE;         // internal ok flag
  PSZ             pError;             // error string pointer
//   PSZ             pszServer;          // server name string pointer
//   CHAR            szDrive[2];         // drive letter
//   USHORT          usRc;               // return codes
  PPUTDICTPROP_IN pPutDictPropIn = NULL; //Ptr to dict prop struct for u-code
  PPUTPROP_OUT    pPutDictPropOut = NULL;//Ptr to dict prop struct for u-code

  HMODULE hResMod;
  hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);

  mp2;
  pIda = ACCESSDLGIDA( hwndDlg, PDICCREATEIDA );

  mResult = MRFROMSHORT( TRUE ); // TRUE is the default return value

  switch (WMCOMMANDID( mp1, mp2 ))
  {
  case ID_DICPROP_HELP_PB:
    UtlInvokeHelp();
    break;
  case DID_CANCEL:
  case ID_DICPROP_CANCEL_PB:
    WinPostMsg( hwndDlg, WM_CLOSE, MP1FROMSHORT(FALSE), 0L );
    break;

  case ID_DICPROP_PROTECTED_PB:
    //dictionary is not protected and user wants to add a
    //password to protect it
    if ( !pIda->Prop.fProtected )
    {
      pIda->ulPassword = 0L;
      //if the dictionary is remote
      if ( pIda->Prop.usLocation == LOC_SHARED )
      {
        //if the user is the owner then allow password
        //protection else issue msg that user isn't authorized
        if ( (strcmp( pIda->Prop.szUserid, pIda->szLanUserID ) == 0)
             || (pIda->usUserPriv == USER_ADMIN) )
        {
          //allow protection
          DIALOGBOX( hwndDlg, DICT2PASSWORDDLG, hResMod,
                     ID_DICT2PASSWORD_DLG, &pIda->ulPassword, fOK );
        } /* endif */
      }
      else
      {
        //local dictionary
        DIALOGBOX( hwndDlg, DICT2PASSWORDDLG, hResMod,
                   ID_DICT2PASSWORD_DLG, &pIda->ulPassword, fOK );
      } /* endif */

      if ( fOK )
      {
        pIda->fProtectionChanged = TRUE;

        pIda->Prop.ulPassWord = pIda->ulPassword;

        //if a valid password has been entered set protection flag
        if ( pIda->Prop.ulPassWord != 0L )
          pIda->Prop.fProtected = TRUE;

        LOADSTRING( NULLHANDLE, hResMod, IDS_DICPROP_UNPROTECT_TEXT,
                    pIda->szBuffer1 );
        SETTEXT( hwndDlg, ID_DICPROP_PROTECTED_PB, pIda->szBuffer1);
      } /* endif */
    }
    else
    {
      DIALOGBOX( hwndDlg, DICT1PASSWORDDLG, hResMod,
                 ID_DICTPASSWORD_DLG, &pIda->Prop, fOK );
      if ( fOK )
      {
        pIda->fProtectionChanged = TRUE;

        if ( !pIda->Prop.fProtected )
        {
          //if the dictionary is remote
          if ( pIda->Prop.szServer[0] != NULC )
          {
            //if the user is the owner then allow password
            //protection else issue msg that user isn't authorized
            if ( (strcmp( pIda->Prop.szUserid, pIda->szLanUserID ) == 0)
                 || (pIda->usUserPriv == USER_ADMIN) )
            {
              //if the dictionary is a remote dictionary and
              //the password has been removed update the remote
              //dictionary properties too
              fOK = UtlAlloc( (PVOID *) &pPutDictPropOut, 0L,
                              (LONG)( sizeof( PUTDICTPROP_IN ) +
                                      sizeof( PUTPROP_OUT )), ERROR_STORAGE );
              if ( fOK )
              {
                //Set pointers
                pPutDictPropIn = (PPUTDICTPROP_IN)
                                 ( pPutDictPropOut + 1 );
                pPutDictPropIn->prefin.usLenIn =
                sizeof( PUTDICTPROP_IN );
                pPutDictPropIn->prefin.idCommand =
                UPDATE_DICT_PROPERTIES;
                pPutDictPropIn->ulPropLength =
                (ULONG)sizeof( PROPDICTIONARY );
                strcpy( pPutDictPropIn->szServer,
                        pIda->Prop.szServer );
                UtlMakeEQFPath( pPutDictPropIn->szPathFileName,
                                pIda->Prop.chRemPrimDrive,
                                COMPROP_PATH, NULL );
                strcat( pPutDictPropIn->szPathFileName,
                        BACKSLASH_STR );
                strcat( pPutDictPropIn->szPathFileName,
                        pIda->Prop.PropHead.szName );

                memcpy( &pPutDictPropIn->DictProp, &pIda->PropCopy,
                        sizeof(PROPDICTIONARY) );

                //update with new protection stuff
                pPutDictPropIn->DictProp.ulPassWord = 0L;
                pPutDictPropIn->DictProp.fProtected = FALSE;
              } /* endif */
            } /* endif */
          } /* endif */

          LOADSTRING( NULLHANDLE, hResMod, IDS_DICPROP_PROTECTED_TEXT,
                      pIda->szBuffer1 );
          SETTEXT( hwndDlg, ID_DICPROP_PROTECTED_PB, pIda->szBuffer1);
        } /* endif */
      } /* endif */
    } /* endif */
    break;


  case ID_DICPROP_CHANGE_PB:
    fOK = TRUE;

    {
      // issue command to all active dialog pages
      int nItem = 0;
      while ( pIda->hwndPages[nItem] && fOK )
      {
        PFNWP pfnWp = (PFNWP) GetWindowLong( pIda->hwndPages[ nItem ],
                                             DWL_DLGPROC );

        switch ( nItem )
        {
        // general  settings
        case 0:
          fOK =  !pfnWp( pIda->hwndPages[nItem], WM_COMMAND,
                         ID_DICPROP_CHANGE_PB, 0L);
          break;

          // options
        case 1:
          fOK =  !pfnWp( pIda->hwndPages[nItem], WM_COMMAND,
                         ID_DICPROP_CHANGE_PB, 0L);
          break;

        } /* endswitch */
        nItem++;
      } /* endwhile */


      if ( pIda->Prop.fCopyRight )
      {
        pError = pIda->szLongName;
        UtlError( ERROR_PROP_CHANGE_DENIED,
                  MB_CANCEL, 1, &pError,
                  EQF_ERROR );
        fOK = FALSE;
      }
      else
      {
//               QUERYTEXT( hwndDlg, ID_DICPROP_DESCR_EF, pIda->Prop.szDescription );
//               ANSITOOEM( pIda->Prop.szDescription );

        //update properties - if dict is remote only the local
        //prop file copy is updated
        // copy own property area to property handler area
        fOK =  ( PutAllProperties( pIda->hDicProp,
                                   &pIda->Prop,
                                   &ErrorInfo ) == 0 );
        // if ok, save properties
        if ( fOK )
          fOK = ( SaveProperties( pIda->hDicProp, &ErrorInfo )
                  == 0);

        if ( !fOK )
        {
          pError = pIda->szLongName;
          UtlError( ERROR_SAVING_PROPS, MB_CANCEL,
                    1, &pError, EQF_ERROR );
          fOK = FALSE;
        } /* endif */
      } /* endif */

      // if O.K and this is a shared dictionary and dictionary
      // protection has been changed, update shared copy of
      // property file
      if ( fOK &&
           (pIda->Prop.usLocation == LOC_SHARED) &&
           pIda->fProtectionChanged )
      {
        CHAR szRemProp[MAX_EQF_PATH]; // name of remote property file
        ULONG    ulRc;               // return value from Utlxxx functions
        PPROPDICTIONARY pRemProp = NULL; // ptr to dictionary properties

        // setup fully qualified file name of remote property file
        Utlstrccpy( szRemProp, pIda->Prop.szDictPath, DOT );
        strcat( szRemProp, EXT_OF_SHARED_DICTPROP );

        // Load remote property file
        fOK = UtlLoadFileL( szRemProp, (PVOID *)&pRemProp, &ulRc,
                           TRUE, FALSE );

        // update protection fields and rewrite remote property file
        if ( fOK )
        {
          pRemProp->ulPassWord = pIda->Prop.ulPassWord;
          pRemProp->fProtected = pIda->Prop.fProtected;
          ulRc =(ULONG)UtlWriteFile( szRemProp, sizeof(pIda->Prop),
                               pRemProp, TRUE );
          fOK = (ulRc == NO_ERROR);
        } /* endif */

        // free memory of remote copy of property file
        if ( pRemProp != NULL )
        {
          UtlAlloc( (PVOID *) &pRemProp, 0L, 0L, NOMSG );
        } /* endif */
      } /* endif */

      //if all okay close dialog
      if ( fOK )
      {
        WinPostMsg( hwndDlg, WM_CLOSE, MP1FROMSHORT(TRUE), 0L );
      } /* endif */


    }

    break;


  } // end switch


  return mResult;

}


//+----------------------------------------------------------------------------+
//|  Dictionary Property Dialog with Property Sheets for WINDOWS only          |
//|                                                                            |
//+----------------------------------------------------------------------------+
INT_PTR CALLBACK DICTIONARYPROPERTYDLGPROC
(
HWND hwnd,
WINMSG message,
WPARAM mp1,
LPARAM mp2
)
{
  PDICCREATEIDA   pIda;               // ptr to instance data area

  MRESULT         mResult = MRFROMSHORT(FALSE);    // dialog procedure return value
//   PPROPDICTIONARY pProp;              // pointer to dictionary properties
  EQFINFO         ErrorInfo;          // error code of property handler calls
  HMODULE hResMod;
  hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);

  switch ( message)
  {
/*--------------------------------------------------------------------------*/

  case WM_EQF_QUERYID: HANDLEQUERYID( ID_DICTPROP_DLG, mp2 ); break;

  case WM_INITDLG:
    pIda = (PDICCREATEIDA) mp2;

    ANCHORDLGIDA( hwnd, pIda );

    //get pointer to system properties
    pIda->pSysProp = (PPROPSYSTEM) MakePropPtrFromHnd( EqfQuerySystemPropHnd() );
    mResult = DicPropPropertySheetLoad( hwnd, pIda );

    //name of second pushbutton
    LOADSTRING( NULLHANDLE, hResMod,
                ( ( pIda->Prop.fProtected ) ? IDS_DICPROP_UNPROTECT_TEXT :
                  IDS_DICPROP_PROTECTED_TEXT),
                pIda->szBuffer1 );
    SETTEXT( hwnd, ID_DICPROP_PROTECTED_PB, pIda->szBuffer1);
//    mResult = DIALOGINITRETURN( mResult );
    mResult = MRFROMSHORT(TRUE);
    break;

  case WM_COMMAND:
    mResult = DicPropCommand( hwnd, mp1, mp2 );
    break;



  case WM_NOTIFY:

    mResult = DicPropPropertySheetNotification( hwnd, mp1, mp2 );
    break;

  case WM_HELP:
    /*************************************************************/
    /* pass on a HELP_WM_HELP request                            */
    /*************************************************************/
    EqfDisplayContextHelp( (HWND)((LPHELPINFO) mp2)->hItemHandle,
                           &hlpsubtblDicPropDlg[0] );
    mResult = TRUE;  // message processed
    break;


/*-------------------------------------------------------------------------*/
  case WM_DESTROY:
    pIda = ACCESSDLGIDA( hwnd, PDICCREATEIDA );
    if ( pIda )
    {
      UtlAlloc( (PVOID *) &pIda, 0L, 0L, NOMSG );
      ANCHORDLGIDA( hwnd, pIda);
    } /* endif */
    break;


/*-------------------------------------------------------------------------*/
  case WM_CLOSE:
    pIda = ACCESSDLGIDA( hwnd, PDICCREATEIDA );
    if ( pIda && pIda->hDicProp )
    {
      CloseProperties( pIda->hDicProp, PROP_QUIT, &ErrorInfo );
      pIda->hDicProp = NULL;
    } /* endif */
    WinDismissDlg( hwnd, SHORT1FROMMP1(mp1) );
    break;

/*--------------------------------------------------------------------------*/
  default:
    mResult = WinDefDlgProc( hwndDlg, message, mp1, mp2 );
/*--------------------------------------------------------------------------*/
  } /* endswitch */
  return( mResult );
} /* end of DICTIONARYPROPERTYDLGPROC */



//+----------------------------------------------------------------------------+
//|Internal function  DICTPROP_GENERAL_DLGPROC                                 |
//+----------------------------------------------------------------------------+
//|Function name:   DICTPROP_GENERAL_DLGPROC                                   |
//+----------------------------------------------------------------------------+
//|Function call:  DICTPROP_GENERAL_DLGPROC( hwnd,msg,mp1,mp2 )                |
//+----------------------------------------------------------------------------+
//|Description:   First Property Tab Page called "General"                     |
//+----------------------------------------------------------------------------+
//|Parameters:  HWND hwnd handle of the dialog window                          |
//|             WINMSG msg                                                     |
//|             WPARAM mp1 message parameter 1                                 |
//|             LPARAM mp2 message parameter 2                                 |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Returncode type:  MRESULT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:  return code from default window proc or FALSE                 |
//+----------------------------------------------------------------------------+
//|Function flow: switch( msg )                                                |
//|                 case WM_INITDLG                                            |
//|                    anchor dialog                                           |
//|                    get copy of dictionary properties                       |                                                                            |
//|                 case WM_COMMAND                                            |
//|                    switch WM_COMMANDID                                     |
//|                      case ID_DICPROP_DESCR_EF                              |
//|                 case WM_HELP                                               |
//|                 case WM_CLOSE                                              |
//+----------------------------------------------------------------------------+

INT_PTR CALLBACK DICTPROP_GENERAL_DLGPROC
(
HWND hwnd,                       /* handle of dialog window             */
WINMSG msg,
WPARAM mp1,
LPARAM mp2
)
{  PDICCREATEIDA   pIda;               // ptr to instance data area
  SHORT           sItem;              // index of listbox items
  MRESULT         mResult = FALSE;    // dialog procedure return value
  PPROPDICTIONARY pProp;              // pointer to dictionary properties
  EQFINFO         ErrorInfo;          // error code of property handler calls
  BOOL            fOK = TRUE;         // internal ok flag
  PSZ             pError;             // error string pointer
  PSZ             pszServer;          // server name string pointer
  CHAR            szDrive[2];         // drive letter
  USHORT          usRc;               // return codes
  BOOL            fPropsChanged = FALSE; // indicates if props have changed

  switch ( msg )
  {
  case WM_INITDLG:
    //-----------------------------------
    {
      pIda = (PDICCREATEIDA) mp2;
      ANCHORDLGIDA( hwnd, pIda );


      UtlSortString( pIda->pSysProp->szDriveList );

      SetCtrlFnt( hwnd, GetCharSet(), ID_DICPROP_DESCR_EF,
                  ID_DICPROP_ENTRYFIELDS_LB );
      ENABLECTRL(hwnd,ID_DICPROP_DESCR_EF,TRUE);

      // get copy of dictionary properties
      PROPNAME( pIda->szBuffer1, pIda->szDicName );
      pIda->hDicProp = OpenProperties( pIda->szBuffer1, NULL,
                                       PROP_ACCESS_WRITE, &ErrorInfo);
      if ( pIda->hDicProp )
      {
        pProp =(PPROPDICTIONARY) MakePropPtrFromHnd( pIda->hDicProp );

        //check if properties fit to selected dictionary
        if ( pProp->usLocation == LOC_SHARED )
        {
          CHAR szLANProps[MAX_EQF_PATH];  // buffer for prop file name
          PPROPDICTIONARY pRemProps = NULL;//buffer for remote props.

          /*****************************************************/
          /* setup path of dictionary properties               */
          /* (the property file is located in the dictionary   */
          /*  directory!)                                      */
          /*****************************************************/
          UtlMakeEQFPath( szLANProps, pProp->szDictPath[0],
                          DIC_PATH, NULL );
          strcat( szLANProps, BACKSLASH_STR );
          Utlstrccpy( szLANProps + strlen(szLANProps),
                      UtlGetFnameFromPath( pProp->szDictPath ),
                      DOT );
          strcat( szLANProps, EXT_OF_SHARED_DICTPROP );

          /*****************************************************/
          /* Allocate buffer for remote properties             */
          /*****************************************************/
          if ( UtlAlloc( (PVOID *) &pRemProps, 0L,
                         (LONG)sizeof(PROPDICTIONARY), ERROR_STORAGE ) )
          {
            /*****************************************************/
            /* Load remote properties                            */
            /*****************************************************/
            usRc = AsdLoadDictProperties( szLANProps, pRemProps );

            /*****************************************************/
            /* Compare properties and update local properties    */
            /* if necessary                                      */
            /*****************************************************/
            if ( usRc == LX_RC_OK_ASD )
            {
              if ( fCompDictProps( pRemProps, pProp ) )
              {
                pRemProps->szDictPath[0] = pProp->szDictPath[0];
                pRemProps->szIndexPath[0] = pProp->szIndexPath[0];
                pRemProps->PropHead.szPath[0] = pProp->PropHead.szPath[0];
                memcpy( pProp, pRemProps, sizeof(PROPDICTIONARY) );
                fPropsChanged = TRUE;
              } /* endif */
            }
            else
            {
              // open of remote properties failed, map return code
              // to allow correct processing in DictRCHandling
              usRc = TMERR_PROP_NOT_FOUND;

              // grey out dictionary
              sItem = QUERYSELECTION( pIda->hwndDictLB, PID_DICTIONARY_LB );
              WinSendDlgItemMsg( pIda->hwndDictLB, PID_DICTIONARY_LB,
                                 LM_EQF_SETITEMSTATE, MP1FROMSHORT (sItem), MP2FROMSHORT(FALSE) );
            } /* endif */
            UtlAlloc( (PVOID *) &pRemProps, 0L, 0L, NOMSG );
          }
          else
          {
            usRc = ERROR_STORAGE;
          } /* endif */
        }
        else
        {
          usRc = NO_ERROR;
          fPropsChanged = FALSE;
        } /* endif */

        if ( usRc != NO_ERROR )
        {
          usRc = DictRcHandling2( usRc, pProp->szDictPath, NULLHANDLE,
                                  pProp->szServer, pIda->szLongName );
          fOK = FALSE;
          CloseProperties( pIda->hDicProp, PROP_QUIT, &ErrorInfo );
          pIda->hDicProp = NULL;
        } /* endif */

        //if props have changed write to file
        if ( fPropsChanged && fOK )
        {
          PutAllProperties( pIda->hDicProp, pProp, &ErrorInfo );
          SaveProperties( pIda->hDicProp, &ErrorInfo);

          //notify handler that props have changed
          //make property path
          PROPNAME( pIda->szBuffer1, pIda->szDicName );

          EqfSend2Handler( DICTIONARYHANDLER,
                           WM_EQFN_PROPERTIESCHANGED,
                           MP1FROMSHORT( PROP_CLASS_DICTIONARY ),
                           MP2FROMP(pIda->szBuffer1) );
        } /* endif */

        if ( fOK )
        {
          memcpy( &pIda->Prop, pProp, sizeof(PROPDICTIONARY) );
          if ( pIda->Prop.szDictPath[0] == NULC )  //no dict path yet
          {
            //build the  fully qualified dictionary path
            DICTNAME( pIda->Prop.szDictPath, NULC, pIda->szDicName );
          } /* endif */
        } /* endif */
      }
      else
      {
        pError = pIda->szLongName;
        UtlError( ERROR_OPENING_PROPS, MB_CANCEL, 1, &pError, EQF_ERROR );
        fOK = FALSE;

        sItem = QUERYSELECTION( pIda->hwndDictLB, PID_DICTIONARY_LB );
        QUERYITEMTEXT( pIda->hwndDictLB, PID_DICTIONARY_LB , sItem,
                       pIda->szBuffer1 );
        pszServer = UtlParseX15( pIda->szBuffer1, DIC_SERVER_IND);
        if ( pszServer[0] == NULC )
        {
          //grey out dictionary as it cannot be accessed
          WinSendDlgItemMsg( pIda->hwndDictLB, PID_DICTIONARY_LB,
                             LM_EQF_SETITEMSTATE, MP1FROMSHORT (sItem), MP2FROMSHORT(FALSE) );
        } /* endif */
      } /* endif */

      if ( fOK )
      {
		HMODULE hResMod;
		hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
        //name of dictionary
        Utlstrccpy( pIda->szBuffer1, pIda->Prop.PropHead.szName, DOT );

        if ( pIda->Prop.szLongName[0] != EOS )
        {
          strcpy( pIda->szLongName, pIda->Prop.szLongName );
          OEMTOANSI( pIda->szLongName );
          strcpy( pIda->szBuffer1, pIda->szLongName );
        }
        else
        {
          strcpy( pIda->szLongName, pIda->szBuffer1 );
        } /* endif */

        SETTEXT( hwnd, ID_DICPROP_NAME_EF, pIda->szBuffer1);

        // dictionary short name
        {
          CHAR szShortName[MAX_FILESPEC];

          Utlstrccpy( szShortName, pIda->Prop.PropHead.szName, DOT );
          SETTEXT( hwnd, ID_DICPROP_SHORTNAME_EF, szShortName );
        }

        //description of dictionary
        SETTEXTLIMIT( hwnd, ID_DICPROP_DESCR_EF,
                      sizeof( pIda->Prop.szLongDesc) - 1);

        if( pIda->Prop.szLongDesc[0] == EOS )
        {
           strcpy( pIda->szBuffer1, pIda->Prop.szDescription );
        }
        else
        {
           strcpy( pIda->szBuffer1, pIda->Prop.szLongDesc );
        }
        OEMTOANSI( pIda->szBuffer1 );
        SETTEXT( hwnd, ID_DICPROP_DESCR_EF, pIda->szBuffer1 );

        //location of dictionary - local or remote
        if ( pIda->Prop.usLocation == LOC_SHARED )
        {
          LOADSTRING( NULLHANDLE, hResMod, IDS_DICPROP_SHARED_TEXT, pIda->szBuffer1 );
        }
        else if ( pIda->Prop.szServer[0] == NULC )
        {
          LOADSTRING( NULLHANDLE, hResMod, IDS_DICPROP_LOCAL_TEXT, pIda->szBuffer1 );
        }
        else
        {
          LOADSTRING( NULLHANDLE, hResMod, IDS_DICPROP_REMOTE_TEXT, pIda->szBuffer1 );
        } /* endif */
        SETTEXT( hwnd, ID_DICPROP_LOC_EF, pIda->szBuffer1 );

        //drive on which the dict is located
        szDrive[0] = pIda->Prop.szDictPath[0];
        szDrive[1] = EOS;
        SETTEXT( hwnd, ID_DICPROP_DRIVE_EF, szDrive );

        //dict source language
        SETTEXT( hwnd, ID_DICPROP_SOURCELANG_EF, pIda->Prop.szSourceLang );


        if ( (pIda->Prop.usLocation == LOC_SHARED) ||
             (pIda->Prop.szServer[0] != NULC) ) //remote dictionary
        {
          //Get LAN user id
          usRc = UtlGetLANUserID(pIda->szLanUserID, &pIda->usUserPriv, FALSE );
          if ( usRc == NO_ERROR )
          {
            if ( strcmp( pIda->szLanUserID, pIda->Prop.szUserid ) )
            {
              if ( pIda->usUserPriv != USER_ADMIN )
              {
                //grey out protected button as only the owner of the
                //remote dict or lan administrator may protect and
                //unprotect the dictionary
                ENABLECTRL( hwnd, ID_DICPROP_PROTECTED_PB, FALSE );
              } /* endif */
            } /* endif */
          }
          else
          {
            //grey out protected button when no lan user connection
            ENABLECTRL( hwnd, ID_DICPROP_PROTECTED_PB, FALSE );
          } /* endif */
        } /* endif */

        //dictionary description
        SETFOCUS( hwnd, ID_DICPROP_DESCR_EF );

        //make second copy for updating remote properties in case a
        //password is entered
        if ( pIda->Prop.szServer[0] != NULC ) //remote dictionary
          memcpy( &pIda->PropCopy, &pIda->Prop, sizeof(PROPDICTIONARY) );

        mResult = MRFROMSHORT(TRUE);
      }
      else
      {
//               WinDismissDlg( hwnd, FALSE );
//               mResult = WinDefDlgProc( hwnd, msg, mp1, mp2 );
        return( MRFROMSHORT(FALSE) );
      } /* endif */
    }
    break;


  case WM_HELP:
    /*************************************************************/
    /* pass on a HELP_WM_HELP request                            */
    /*************************************************************/
    EqfDisplayContextHelp((HWND) ((LPHELPINFO) mp2)->hItemHandle,
                           &hlpsubtblDicPropDlg[0] );
    mResult = TRUE;  // message processed
    break;



  case WM_COMMAND:
    //-----------------------------------

    pIda = ACCESSDLGIDA( hwnd, PDICCREATEIDA );

    mResult = MRFROMSHORT( TRUE ); // TRUE is the default return value

    switch (WMCOMMANDID( mp1, mp2 ))
    {
      case ID_DICPROP_CHANGE_PB:
        pIda->Prop.szDescription[0] = EOS; 
        QUERYTEXT( hwnd, ID_DICPROP_DESCR_EF, pIda->Prop.szLongDesc );
        ANSITOOEM( pIda->Prop.szLongDesc );
        break;

      case ID_DICPROP_DESCR_EF:
        if ( WMCOMMANDCMD( mp1, mp2 ) == EN_KILLFOCUS )
        {
          ClearIME( hwnd );
        } /* endif */
        break;
    }/* endswitch */


    break;


  case WM_CLOSE:
    //-----------------------------------/


    DelCtrlFont( hwnd, ID_DICPROP_DESCR_EF );
//         WinDismissDlg( hwnd, SHORT1FROMMP1(mp1) );


    mResult = WinDefDlgProc( hwnd, msg, mp1, mp2 );
    break;

  default:
    //-----------------------------------
    mResult = WinDefDlgProc( hwnd, msg, mp1, mp2 );
//         WinDismissDlg( hwnd, SHORT1FROMMP1(mp1) );

    break;
  } /* endswitch */


  return mResult;

}

//+----------------------------------------------------------------------------+
//|Internal function  DICTPROP_OPTIONS_DLGPROC                                 |
//+----------------------------------------------------------------------------+
//|Function name:   DICTPROP_OPTIONS_DLGPROC                                   |
//+----------------------------------------------------------------------------+
//|Function call:  DICTPROP_OPTIONS_DLGPROC( hwnd,msg,mp1,mp2 )                |
//+----------------------------------------------------------------------------+
//|Description:  Second Property Tab Page called "Entry structure"             |
//+----------------------------------------------------------------------------+
//|Parameters:  HWND hwnd handle of the dialog window                          |
//|             WINMSG msg                                                     |
//|             WPARAM mp1 message parameter 1                                 |
//|             LPARAM mp2 message parameter 2                                 |                                                                                                               |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Returncode type: MRESULT                                                    |
//+----------------------------------------------------------------------------+
//|Returncodes:  return code from default window proc or FALSE                 |
//+----------------------------------------------------------------------------+
//|Function flow: switch( msg )                                                |
//|                 case WM_INITDLG                                            |
//|                    anchor dialog                                           |
//|                 case WM_COMMAND                                            |
//|                      case ID_DICPROP_CHANGE_PB                             |
//|                 case WM_HELP                                               |
//|                 case WM_CLOSE                                              |
//+----------------------------------------------------------------------------+



INT_PTR CALLBACK DICTPROP_OPTIONS_DLGPROC
(
HWND hwnd,                       /* handle of dialog window             */
WINMSG msg,
WPARAM mp1,
LPARAM mp2
)
{  PDICCREATEIDA   pIda;               // ptr to instance data area
  SHORT           sItem;              // index of listbox items
  MRESULT         mResult = FALSE;    // dialog procedure return value
  PPROPDICTIONARY pProp;              // pointer to dictionary properties
//   EQFINFO         ErrorInfo;          // error code of property handler calls
  BOOL            fOK = TRUE;         // internal ok flag
//   PSZ             pError;             // error string pointer
//   PSZ             pszServer;          // server name string pointer
//   CHAR            szDrive[2];         // drive letter
//   USHORT          usRc;               // return codes

  switch ( msg )
  {

  case WM_INITDLG:
    //-----------------------------------
    {  pIda = (PDICCREATEIDA) mp2;
      ANCHORDLGIDA( hwnd, pIda );

      //get pointer to system properties
      UtlSortString( pIda->pSysProp->szDriveList );

      //set text colour in level group box to black
      SETCOLOR( hwnd, ID_DICPROP_LEV1_TEXT, CLR_BLACK );
      SETCOLOR( hwnd, ID_DICPROP_LEV2_TEXT, CLR_BLACK );
      SETCOLOR( hwnd, ID_DICPROP_LEV3_TEXT, CLR_BLACK );
      SETCOLOR( hwnd, ID_DICPROP_LEV4_TEXT, CLR_BLACK );


      // get copy of dictionary properties
//            PROPNAME( pIda->szBuffer1, pIda->szDicName );
//            pIda->hDicProp = OpenProperties( pIda->szBuffer1, NULL,
//                                             PROP_ACCESS_WRITE, &ErrorInfo);
      if ( pIda->hDicProp )
      {
        pProp =(PPROPDICTIONARY) MakePropPtrFromHnd( pIda->hDicProp );

        //fill entry listbox
        DicFillEntryLB( hwnd, pIda, ID_DICPROP_ENTRYFIELDS_LB );

        /*************************************************************/
        /* Force processing of listbox selection (not required for   */
        /* OS/2)                                                     */
        /*************************************************************/
        DictPropControl( hwnd, ID_DICPROP_ENTRYFIELDS_LB, LN_SELECT );

        //make second copy for updating remote properties in case a
        //password is entered
        if ( pIda->Prop.szServer[0] != NULC ) //remote dictionary
          memcpy( &pIda->PropCopy, &pIda->Prop, sizeof(PROPDICTIONARY) );

        mResult = MRFROMSHORT(TRUE);
      }
      else
      {
//               WinDismissDlg( hwnd, FALSE );
//               mResult = WinDefDlgProc( hwnd, msg, mp1, mp2 );

        return( MRFROMSHORT(FALSE) );
      } /* endif */
    }
    break;

  case WM_HELP:
    //-----------------------------------
    /*************************************************************/
    /* pass on a HELP_WM_HELP request                            */
    /*************************************************************/
    EqfDisplayContextHelp( (HWND)((LPHELPINFO) mp2)->hItemHandle,
                           &hlpsubtblDicPropDlg[0] );
    mResult = TRUE;  // message processed
    break;

  case WM_COMMAND:
    //-----------------------------------

    pIda = ACCESSDLGIDA( hwnd, PDICCREATEIDA );

    mResult = MRFROMSHORT( TRUE ); // TRUE is the default return value

    switch (WMCOMMANDID( mp1, mp2 ))
    {
    case ID_DICPROP_CHANGENAME_PB:
      // check if listbox item is selected
      sItem = QUERYSELECTION( hwnd, ID_DICPROP_ENTRYFIELDS_LB );
      if ( sItem == LIT_NONE )
      {
        UtlError( ERROR_ENTRYFIELD_NOT_SELECTED,
                  MB_CANCEL, 0, NULL, EQF_ERROR );
      }
      else
      {
		HMODULE hResMod;
		hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
        pIda->pEntry = (PPROFENTRY) QUERYITEMHANDLE ( hwnd,
                                                      ID_DICPROP_ENTRYFIELDS_LB,
                                                      sItem );
        pIda->fChangeName = TRUE;
        DIALOGBOX( hwnd, PCH_DLGPROC, hResMod, ID_PCH_DLG,
                   pIda, fOK );
        // if new term entered, refill the entry fields listbox
        // with new value
        if ( fOK == TRUE )
        {
          DicFillEntryLB( hwnd, pIda, ID_DICPROP_ENTRYFIELDS_LB );

          //position on changed name
          SELECTITEM( hwnd, ID_DICPROP_ENTRYFIELDS_LB, sItem );
        } /* endif */
        //focus on list box item
        SETFOCUS( hwnd, ID_DICPROP_ENTRYFIELDS_LB );
      }
      break;

      //           case ID_DICPROP_CHANGE_PB:

      // nothing to do here

//               break;
    case ID_DICPROP_ENTRYFIELDS_LB:
    case ID_DICPROP_OMIT_RB:
    case ID_DICPROP_L1_RB:
    case ID_DICPROP_L2_RB:
    case ID_DICPROP_L3_RB:
    case ID_DICPROP_SMALL_RB:
    case ID_DICPROP_LARGE_RB:
    case ID_DICPROP_AUTLOOKUP_CB:
      mResult = DictPropControl( hwnd, WMCOMMANDID( mp1, mp2 ),
                                 WMCOMMANDCMD( mp1, mp2 ) );
      break;

    case WM_CLOSE:
      //-----------------------------------/
//               WinDefDlgProc( hwnd, msg, mp1, mp2 );

      DelCtrlFont( hwnd, ID_DICPROP_ENTRYFIELDS_LB );
//               WinDismissDlg( hwnd, SHORT1FROMMP1(mp1) );
      mResult = WinDefDlgProc( hwnd, msg, mp1, mp2 );

      break;

    default:
      //-----------------------------------

//               mResult = WinDismissDlg( hwnd, SHORT1FROMMP1(mp1) );
      mResult = WinDefDlgProc( hwnd, msg, mp1, mp2 );

      break;
    } /* endswitch */
  }

  return mResult;
}

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     DicRenameDict                                            |
//+----------------------------------------------------------------------------+
//|Description:       Physically rename a dictionary and all associated files  |
//+----------------------------------------------------------------------------+
//|Input parameter:   BOOL        fMsg,              show-error-messages flag  |
//|                   PSZ         pszOldAndNewName   old and new dict name     |
//|                                                  seperated by 0x15         |
//|                                                  (old name = object name,  |
//|                                                  new name = file name only)|
//|                              e.g. H:\EQF\MEM\SAMPLE1.PRO0x15NEWNAME0x00    |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT              error code or 0 if success           |
//+----------------------------------------------------------------------------+
BOOL DicRenameDict
(
  PSZ         pszOldAndNewName,        // old and new dict name (x15 seperated)
  BOOL        fMsg                     // show-error-messages flag
)
{

  // our private data area
  typedef struct _PRIVATEDATA
  {
   CHAR       szOldObject[MAX_EQF_PATH];// buffer for old object name
   CHAR       szNewObject[MAX_EQF_PATH];// buffer for new object name
   CHAR       szOldProp[MAX_EQF_PATH]; // buffer for old TM property file name
   CHAR       szNewProp[MAX_EQF_PATH]; // buffer for new TM property file name
   CHAR       szOldName[MAX_FNAME];    // buffer for old name
   CHAR       szNewName[MAX_FNAME];    // buffer for new name
   CHAR       szOldPath[MAX_EQF_PATH]; // buffer for old path name
   CHAR       szNewPath[MAX_EQF_PATH]; // buffer for new path name
   CHAR       szOldIndexPath[MAX_EQF_PATH]; // buffer for old index path name
   CHAR       szNewIndexPath[MAX_EQF_PATH]; // buffer for new index path name
   CHAR       szExt[6];                // buffer for TM name extension
   PROPDICTIONARY DicProp;             // buffer for dictionary properties
   CHAR       szLongName[MAX_LONGFILESPEC]; // buffer for new long name
  } PRIVATEDATA, *PPRIVATEDATA;

  BOOL        fOK = TRUE;              // internal O.K. flag and return value
  PPRIVATEDATA pData = NULL;           // ptr to private data area

  // allocate our private data area
  fOK = UtlAlloc( (PVOID *)&pData, 0L, (LONG)sizeof(PRIVATEDATA),
                  (USHORT)(( fMsg ) ? ERROR_STORAGE : NOMSG) );

  // split input data and store old and new dict name
  if ( fOK )
  {
    PSZ pszNewName = strchr( pszOldAndNewName, X15 );
    if ( pszNewName != NULL )
    {
      PSZ pszNewLongName;
      *pszNewName = EOS;
      pszNewName++;
      pszNewLongName = strchr( pszNewName, X15 );
      if ( pszNewLongName != NULL )
      {
        *pszNewLongName = EOS;
        pszNewLongName++;
        strcpy( pData->szLongName, pszNewLongName );
      }
      else
      {
        pData->szLongName[0] = EOS;
      } /* endif */
      strcpy( pData->szNewName, pszNewName );
    } /* endif */
    strcpy( pData->szOldObject, pszOldAndNewName );
    Utlstrccpy( pData->szOldName, UtlGetFnameFromPath( pData->szOldObject ),
                DOT );
  } /* endif */

  // setup dict property file name
  if ( fOK )
  {
    UtlMakeEQFPath( pData->szOldProp, NULC, PROPERTY_PATH, NULL );
    strcat( pData->szOldProp, BACKSLASH_STR );
    strcat( pData->szOldProp, pData->szOldName );
    strcat( pData->szOldProp, EXT_OF_DICTPROP );
  } /* endif */

  // load dictionary property file
  if ( fOK )
  {
    ULONG ulRead;     // number of bytes read from disk
    PVOID pvTemp = (PVOID)&pData->DicProp;
    fOK = UtlLoadFileL( pData->szOldProp, &pvTemp, &ulRead,
                       FALSE, fMsg );
  } /* endif */

  // adjust names in dictionary property file
  if ( fOK )
  {
    // adjust name in property header
    strcpy( pData->DicProp.PropHead.szName, pData->szNewName );
    strcat( pData->DicProp.PropHead.szName, EXT_OF_DICTPROP );

    // adjust long name
    if ( strcmp( pData->szLongName, pData->szNewName ) != EOS )
    {
      strcpy( pData->DicProp.szLongName, pData->szLongName );
    }
    else
    {
      pData->DicProp.szLongName[0] = EOS;
    } /* endif */

    // remember old data and index path names
    strcpy( pData->szOldPath, pData->DicProp.szDictPath );
    strcpy( pData->szOldIndexPath, pData->DicProp.szIndexPath );

    // adjust fully qualified dict name
    {
      PSZ pszExt;
      PSZ pszNamePos = UtlGetFnameFromPath( pData->DicProp.szDictPath );
      pszExt = strchr( pszNamePos, DOT );
      if ( pszExt != NULL )
      {
        strcpy( pData->szExt, pszExt );
      }
      else
      {
        pData->szExt[0] = EOS;
      } /* endif */
      strcpy( pszNamePos, pData->szNewName );
      strcat( pszNamePos, pData->szExt );
    }

    // adjust fully qualified index name
    {
      PSZ pszExt;
      PSZ pszNamePos = UtlGetFnameFromPath( pData->DicProp.szIndexPath );
      pszExt = strchr( pszNamePos, DOT );
      if ( pszExt != NULL )
      {
        strcpy( pData->szExt, pszExt );
      }
      else
      {
        pData->szExt[0] = EOS;
      } /* endif */
      strcpy( pszNamePos, pData->szNewName );
      strcat( pszNamePos, pData->szExt );
    }

    // remember new data and index path names
    strcpy( pData->szNewPath, pData->DicProp.szDictPath );
    strcpy( pData->szNewIndexPath, pData->DicProp.szIndexPath );
  } /* endif */

  // rename dictionary property file
  if ( fOK )
  {
    // setup new property path name
    UtlMakeEQFPath( pData->szNewProp, NULC, PROPERTY_PATH, NULL );
    strcat( pData->szNewProp, BACKSLASH_STR );
    strcat( pData->szNewProp, pData->szNewName );
    strcat( pData->szNewProp, EXT_OF_DICTPROP );

    // actually rename property file using UtlMove...
    fOK = UtlMove( pData->szOldProp, pData->szNewProp, 0L,
                   fMsg ) == NO_ERROR;
  } /* endif */

  // rewrite dictionary property file
  if ( fOK )
  {
    fOK = UtlWriteFile( pData->szNewProp,
                        sizeof(pData->DicProp),
                        &pData->DicProp, fMsg ) == NO_ERROR;
  } /* endif */

  // rename database file
  if ( fOK )
  {
    fOK = UtlMove( pData->szOldPath, pData->szNewPath, 0L,
                   fMsg ) == NO_ERROR;
  } /* endif */

  // rename index file
  if ( fOK )
  {
    fOK = UtlMove( pData->szOldIndexPath, pData->szNewIndexPath, 0L,
                   fMsg ) == NO_ERROR;
  } /* endif */

  // rename any shared property file (delete old one, rewrite new one)
  if ( fOK )
  {
    if ( pData->DicProp.usLocation == LOC_SHARED )
    {
      PSZ pszExt;                      // points to file name extension

      // convert dictioary path name to name of shared property files
      pszExt = strrchr( pData->szOldPath, DOT );
      strcpy( pszExt, EXT_OF_SHARED_DICTPROP );

      pszExt = strrchr( pData->szNewPath, DOT );
      strcpy( pszExt, EXT_OF_SHARED_DICTPROP );

      // delete old shared property file
      UtlDelete( pData->szOldPath, 0L, FALSE );

      // write properties to new shared property file
      fOK = UtlWriteFile( pData->szNewPath,
                          sizeof(pData->DicProp),
                          &pData->DicProp, fMsg ) == NO_ERROR;
    } /* endif */
  } /* endif */

  // broadcast changed dictionary name
  if ( fOK )
  {
    // remove original dictionary object
    EqfSend2AllHandlers( WM_EQFN_DELETED,
                         MP1FROMSHORT(clsDICTIONARY),
                         MP2FROMP(pData->szOldObject) );

    // add new dictionary object
    UtlMakeEQFPath( pData->szNewObject, NULC, SYSTEM_PATH, NULL );
    strcat( pData->szNewObject, BACKSLASH_STR );
    strcat( pData->szNewObject, pData->DicProp.PropHead.szName );
    EqfSend2AllHandlers( WM_EQFN_CREATED, MP1FROMSHORT(clsDICTIONARY),
                         MP2FROMP(pData->szNewObject) );
  } /* endif */

  // cleanup
  if ( pData ) UtlAlloc( (PVOID *)&pData, 0L, 0L, NOMSG );

  return( fOK );
} /* end of function DicRenameDict */

