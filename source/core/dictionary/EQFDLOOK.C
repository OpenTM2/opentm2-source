//+----------------------------------------------------------------------------+
//|  EQFDLOOK.C - Lookup terms in dictionaries                                 |
//+----------------------------------------------------------------------------+
//|  Copyright Notice:                                                         |
//|                                                                            |
//|      Copyright (C) 1990-2012, International Business Machines              |
//|      Corporation and others. All rights reserved                           |

//+----------------------------------------------------------------------------+
//|  Description: Dialog procedure to set options for lookup and invoke of     |
//|               lookup dialog procedure                                      |
//+----------------------------------------------------------------------------+
#define INCL_EQF_ASD              // dictionary access functions (Asd...)
#define INCL_EQF_LDB              // dictionary data encoding functions
#define INCL_EQF_FILT             // dictionary filter functions
#include <eqf.h>                  // General Translation Manager include file

#include "eqfdasdi.h"
#include "OtmDictionaryIF.H"
#include "eqfdlook.h"             // internal lookup header file
#include "eqfddlg.id"             // dialog IDs

#include "eqfutmdi.h"           // MDI utilities

/**********************************************************************/
/* Macro to set text in combobox and post a notification message to   */
/* dialog (required only in Windows environment)                      */
/**********************************************************************/
#define COMBOSETTEXT( hwnd, id, text ) \
  { \
    SetDlgItemText( hwnd, id, text ); \
    PostMessage( hwnd, WM_EQF_COMMAND, MP1FROMSHORT(id), MP2FROM2SHORT( 0, CBN_EFCHANGE ) ); \
  }

#define COMBOSETTEXTW( hwnd, id, text ) \
  { \
    SetDlgItemTextW( hwnd, id, text ); \
    PostMessage( hwnd, WM_EQF_COMMAND, MP1FROMSHORT(id), MP2FROM2SHORT( 0, CBN_EFCHANGE ) ); \
  }

#define LOOKUP_TASK       USER_TASK + 2
#define SHOWCOMBO_TASK    USER_TASK + 3

MRESULT LookupDlgControl
(
   HWND   hwndDlg,                     // dialog handle
   SHORT  sId,                         // id in action
   SHORT  sNotification                // notification
);
VOID LookHeadword( PLOOKIDA pIda );
VOID LookNeigh( PLOOKIDA pIda );
VOID LookIndex( PLOOKIDA pIda, USHORT usTermType );
VOID LookWildCard( PLOOKIDA pIda, BOOL fCompound );
BOOL  FilterMatch( PLOOKIDA pIda, PSZ_W pszTerm );
MRESULT EXPENTRY ComboEditSubclassProc( HWND, WINMSG, WPARAM, LPARAM );
MRESULT EXPENTRY RadioButtonSubclassProc( HWND, WINMSG, WPARAM, LPARAM );
void LookupCheckForUpdate( HWND hwnd );
SHORT LookupAddToCBIfNew( HWND hwndCombo, PSZ_W pszTerm );



static FARPROC pfnOrgComboEdit;        // original combo edit window procedure
static FARPROC pfnOrgRadioButton;      // original radiobutton window procedure

static CHAR szAnsiTerm[512];

/**********************************************************************/
/* TermLookup                                                         */
/*                                                                    */
/* Start the lookup dialog                                            */
/**********************************************************************/
BOOL TermLookup
(
   PLUPCB  pLUPCB,                     // lookup control block pointer
   PSZ_W   pucTerm,                    // term being looked-up
   HMODULE hmod,                       // resource module handle
   HWND    hwndCall                    // caller's window handle
)
{
  BOOL fFound = FALSE;
  BOOL fOK = TRUE;

  /********************************************************************/
  /* Lookup term in dictionary                                        */
  /********************************************************************/
  if ( pucTerm && (*pucTerm != EOS) )
  {
    HDCB           hEditHandle;        // handle of dictionary containing entry
    ULONG          ulTermNum;          // number of term
    ULONG          ulDataLen;          // length of term data
    USHORT         usASDRC;            // return code from ASD-calls

    usASDRC = AsdFndMatch( pucTerm,
                           pLUPCB->hDCB,
                           pLUPCB->hUCB,
                           pLUPCB->ucTerm,
                           &ulTermNum,
                           &ulDataLen,
                           &hEditHandle );

    if (usASDRC == LX_WRD_NT_FND_ASD)
    {
       usASDRC = AsdFndEquivW( pucTerm,
                              pLUPCB->hDCB,
                              pLUPCB->hUCB,
                              pLUPCB->ucTerm,
                              &ulTermNum,
                              &ulDataLen,
                              &hEditHandle );
    } /* endif */

    if( usASDRC == LX_WRD_NT_FND_ASD )
    {
      AsdGetStemForm( pLUPCB->hUCB,
                      pLUPCB->hDCB,
                      pucTerm,
                      pLUPCB->ucTerm );

       usASDRC = AsdFndMatch( pLUPCB->ucTerm,
                              pLUPCB->hDCB,
                              pLUPCB->hUCB,
                              pLUPCB->ucTerm ,
                              &ulTermNum,
                              &ulDataLen,
                              &hEditHandle );
    } /* endif */

    if( usASDRC == LX_RC_OK_ASD )
    {
      fFound = TRUE;
    } /* endif */
  } /* endif */

  if ( fFound )
  {
	HMODULE hResMod;
	hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
    DisplayEntry( pLUPCB, pLUPCB->ucTerm, hResMod,
                  hwndCall, EMPTY_STRING );
  }
  else
  {
    if ( pucTerm != NULL )
    {
      UTF16strcpy( pLUPCB->ucTerm, pucTerm );
    }
    else
    {
      pLUPCB->ucTerm[0] = EOS;
    } /* endif */

    if ( pLUPCB->ulFlags & LUP_MODALDLG_MODE  )
    {
	  HMODULE hResMod;
	  hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
      DIALOGBOX( hwndCall, LOOKUPDLGPROC,
                 hResMod,
                 LOOKUP_DLG,
                 pLUPCB, fOK );
    }
    else
    {
      if ( pLUPCB->lpfnLookupProc == NULL )
      {
        pLUPCB->lpfnLookupProc = MakeProcInstance( (FARPROC)(LOOKUPDLGPROC),
                                                 (HAB)UtlQueryULong( QL_HAB ) );
      } /* endif */

       pLUPCB->hwndLook = CreateMDIDialogParam( hmod,
                                                MAKEINTRESOURCE(LOOKUP_DLG),
                                                (HWND)UtlQueryULong( QL_TWBCLIENT ),
                                                pLUPCB->lpfnLookupProc,
                                                MP2FROMP(pLUPCB), TRUE,
                                                (HPOINTER) UtlQueryULong(QL_DICTENTRYDISPICO)); //hiconDICTDISP );
        fOK = (pLUPCB->hwndLook != NULLHANDLE );
    } /* endif */
  } /* endif */
  return ( fOK );
}

/*******************************************************************************
*
*       function:       LOOKUPDLGPROC
*
*******************************************************************************/
INT_PTR CALLBACK LOOKUPDLGPROC( HWND hwnd, WINMSG msg, WPARAM mp1, LPARAM mp2)
{
  PLOOKIDA pIda;                       // ptr to lookup dialog instance data
  BOOL           fOK;                  // internal OK flag
  USHORT         usRC;                 // function return code
  PPROPDICTIONARY pDictProp;           // ptr to dictionary properties
  HDCB           ahDCB[MAX_DICTS+1];   // list of associated dictionaries
  USHORT         usNumOfDicts;         // number of dictionaries in association
  USHORT         usI;                  // loop index
  BOOL           fAnsiConv = TRUE;
//  static HWND hwndCLBox;               // handle of combobox listbox

   switch (msg)
   {
      case WM_EQF_QUERYID: HANDLEQUERYID( LOOKUP_DLG, mp2 ); break;


//    case WM_CTLCOLORLISTBOX:
//      /**************************************************************/
//      /* take care of the odd behaviour of the drop down of a combo */
//      /* box -- it is not child of a combobox                       */
//      /**************************************************************/
//      if ( !hwndCLBox )
//      {
//        USHORT usI, usNum;
//        CHAR   chText[256];
//        hwndCLBox = (HWND) mp2;
//        SetCtrlFntHwnd( hwndCLBox, GetCharSet() );
//        usNum = QUERYITEMCOUNTHWND( hwndCLBox );
//        for ( usI = 0; usI < usNum; usI++  )
//        {
//          SendMessage( hwndCLBox, LB_GETTEXT, 0, chText );
//          SendMessage( hwndCLBox, LB_DELETESTRING, 0, 0 );
//          SendMessage( hwndCLBox, LB_INSERTSTRING, -1, chText );
//        } /* endfor */
//      } /* endif */
//      break;

      case WM_INITDLG:
         pIda = NULL;
         fOK = UtlAlloc( (PVOID *)&pIda, 0L, (LONG) sizeof(LOOKIDA), ERROR_STORAGE );
         if ( fOK )
         {
           fOK = UtlAlloc( (PVOID *)&pIda->pucTermListBuffer, 0L,
                           (LONG)DLUP_TERMBUF_SIZE * sizeof(CHAR_W), ERROR_STORAGE );
         } /* endif */

         if ( fOK )
         {
			HMODULE hResMod;
			hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
            ANCHORDLGIDA( hwnd, pIda );
            pIda->pLUPCB = (PLUPCB) mp2;
            pIda->hwnd = hwnd;
            pIda->fLookupDlgShown = FALSE;
            pIda->hwndPrevFocus = GETFOCUS();

            // build unique dialog object name
            if ( !(pIda->pLUPCB->ulFlags & LUP_MODALDLG_MODE) )
            {
              sprintf( pIda->szObjName, "LOOKUP: %ld", (LONG)mp2 );
              EqfRegisterObject( pIda->szObjName, pIda->hwnd, clsDICTLOOKUP );
            } /* endif */

            LOADSTRINGLEN( NULLHANDLE, hResMod, SID_LOOKUP_TITLE,
                   (PCHAR)pIda->ucbTermBuf, sizeof(pIda->ucbTermBuf) );

            SETTEXTHWND( hwnd, (PCHAR)pIda->ucbTermBuf );
            pIda->usLookAction = LOOK_HW;
            /****************************************************************/
            /* determine if we are dealing with extended WP combo boxes or  */
            /* normal ones using a more or less primitive approach          */
            /* (try to create the combobox with WCWP_WPCOMBOBOX class       */
            /*  and check if we are successful)                             */
            /****************************************************************/

            SETTEXTLIMIT( hwnd, LOOKUP_WORD_COMBO, MAX_TERM_LEN  );

         SetCtrlFnt (hwnd, GetCharSet(), LOOKUP_WORD_COMBO, 0 );

         ENABLECTRL( hwnd, LOOKUP_PUBO_OK, FALSE );
            ENABLECTRL( hwnd, LOOKUP_PUBO_EDIT, FALSE );
            SETCHECK_TRUE( hwnd, LOOKUP_ARBUT_HEADWORD );
            SETCHECK_FALSE( hwnd, LOOKUP_ARBUT_NEIGHB );
            SETCHECK_FALSE( hwnd, LOOKUP_ARBUT_COMPWORD );
            SETCHECK_FALSE( hwnd, LOOKUP_ARBUT_SYN );
            SETCHECK_FALSE( hwnd, LOOKUP_ARBUT_RELTERM);
            SETCHECK_FALSE( hwnd, LOOKUP_ARBUT_ABBREV );

            WinSendMsg( WinWindowFromID( hwnd, LOOKUP_ARBUT_FILT_CHG ),
                        WM_EQF_FILTSETDICT,
                        0,
                        MP2FROMP(pIda->pLUPCB->hDCB) );

            if ( !(pIda->pLUPCB->ulFlags & LUP_MODALDLG_MODE) )
            {
              UtlRegisterModelessDlg( hwnd );
            } /* endif */

           /**********************************************************/
           /* Under Windows only:                                    */
           /* Subclass entry field of term combobox and radiobuttons */
           /* in order to trap Enter key.                            */
           /**********************************************************/

           {
             HWND  hwndCombo;          // handle of combobox
             HWND  hwndEdit;           // handle of entryfield
             HWND  hwndRB;             // handle of radiobutton

             hwndCombo = GetDlgItem( hwnd, LOOKUP_WORD_COMBO );
             hwndEdit  = GetWindow( hwndCombo, GW_CHILD );
             if ( hwndEdit != NULL )
             {
               pfnOrgComboEdit = (FARPROC)SetWindowLong( hwndEdit, GWL_WNDPROC,
                                   (LONG)ComboEditSubclassProc );
             } /* endif */


             hwndRB = GetDlgItem( hwnd, LOOKUP_ARBUT_HEADWORD );
             pfnOrgRadioButton = (FARPROC)SetWindowLong( hwndRB, GWL_WNDPROC,
                                   (LONG)RadioButtonSubclassProc );
             hwndRB = GetDlgItem( hwnd, LOOKUP_ARBUT_NEIGHB );
             pfnOrgRadioButton = (FARPROC)SetWindowLong( hwndRB, GWL_WNDPROC,
                                   (LONG)RadioButtonSubclassProc );
             hwndRB = GetDlgItem( hwnd, LOOKUP_ARBUT_COMPWORD );
             pfnOrgRadioButton = (FARPROC)SetWindowLong( hwndRB, GWL_WNDPROC,
                                   (LONG)RadioButtonSubclassProc );
             hwndRB = GetDlgItem( hwnd, LOOKUP_ARBUT_SYN );
             pfnOrgRadioButton = (FARPROC)SetWindowLong( hwndRB, GWL_WNDPROC,
                                   (LONG)RadioButtonSubclassProc );
             hwndRB = GetDlgItem( hwnd, LOOKUP_ARBUT_RELTERM );
             pfnOrgRadioButton = (FARPROC)SetWindowLong( hwndRB, GWL_WNDPROC,
                                   (LONG)RadioButtonSubclassProc );
             hwndRB = GetDlgItem( hwnd, LOOKUP_ARBUT_ABBREV );
             pfnOrgRadioButton = (FARPROC)SetWindowLong( hwndRB, GWL_WNDPROC,
                                   (LONG)(RadioButtonSubclassProc) );
           }

            /*************************************************************/
            /* fill dictionary listbox                                   */
            /*************************************************************/
            AsdRetDictList( pIda->pLUPCB->hDCB, ahDCB, &usNumOfDicts );
            fAnsiConv = TRUE;
                        if (GetCharSet() == THAI_CHARSET )
                        {
              fAnsiConv = FALSE;
                    }
            for ( usI=0; usI < usNumOfDicts; usI++)
            {
              AsdQueryDictName( ahDCB[usI], (PSZ)pIda->ucbTermBuf );
              CONDOEMTOANSI( fAnsiConv, (PSZ) pIda->ucbTermBuf );
              switch ( usI )
              {
                case 0 : SETTEXT( hwnd, LOOKUP_DICT01_TEXT, (PUCHAR)pIda->ucbTermBuf ); break;
                case 1 : SETTEXT( hwnd, LOOKUP_DICT02_TEXT, (PUCHAR)pIda->ucbTermBuf ); break;
                case 2 : SETTEXT( hwnd, LOOKUP_DICT03_TEXT, (PUCHAR)pIda->ucbTermBuf ); break;
                case 3 : SETTEXT( hwnd, LOOKUP_DICT04_TEXT, (PUCHAR)pIda->ucbTermBuf ); break;
                case 4 : SETTEXT( hwnd, LOOKUP_DICT05_TEXT, (PUCHAR)pIda->ucbTermBuf ); break;
                case 5 : SETTEXT( hwnd, LOOKUP_DICT06_TEXT, (PUCHAR)pIda->ucbTermBuf ); break;
                case 6 : SETTEXT( hwnd, LOOKUP_DICT07_TEXT, (PUCHAR)pIda->ucbTermBuf ); break;
                case 7 : SETTEXT( hwnd, LOOKUP_DICT08_TEXT, (PUCHAR)pIda->ucbTermBuf ); break;
                case 8 : SETTEXT( hwnd, LOOKUP_DICT09_TEXT, (PUCHAR)pIda->ucbTermBuf ); break;
                case 9 : SETTEXT( hwnd, LOOKUP_DICT10_TEXT, (PUCHAR)pIda->ucbTermBuf ); break;
              } /* endswitch */
            } /* endfor */

            /*************************************************************/
            /* Check if dictionary/ies has/have system fields using the  */
            /* index (Abbreviation, Synonym, Related). If not            */
            /* disable the approbriate search type radiobutton           */
            /*************************************************************/
            {
              PDCB        pDCB;           // dictionary control block pointer
              BOOL        fAbbrev  = FALSE;
              BOOL        fSynonym = FALSE;
              BOOL        fRelated = FALSE;

              // loop over all dictionaries in association or over single one
              for ( usI=0; usI < usNumOfDicts; usI++)
              {
                // convert handle to pointer to allow access to the 'inner circle'
                pDCB = (PDCB)ahDCB[usI];

                // set search type flag if field is contained in dictionary
                fAbbrev  |= pDCB->AbbrField.fInDict;
                fSynonym |= pDCB->SynField.fInDict;
                fRelated |= pDCB->RelField.fInDict;
              } /* endfor */

              // set state of search type radiobuttons accordingly
              ENABLECTRL( hwnd, LOOKUP_ARBUT_SYN, fSynonym );
              ENABLECTRL( hwnd, LOOKUP_ARBUT_RELTERM, fRelated );
              ENABLECTRL( hwnd, LOOKUP_ARBUT_ABBREV, fAbbrev );
            }

            /**********************************************************/
            /* Do a neighborhood search if called with a term         */
            /**********************************************************/
            UtlStripBlanksW( pIda->pLUPCB->ucTerm );
            if ( pIda->pLUPCB->ucTerm[0] != EOS )
            {
              UTF16strcpy( pIda->ucbTerm, pIda->pLUPCB->ucTerm );
              LookNeigh( pIda );

                //strcpy( szAnsiTerm, pIda->ucbTerm );
                //CONDOEMTOANSI( fAnsiConv, szAnsiTerm );
                COMBOSETTEXTW( pIda->hwnd, LOOKUP_WORD_COMBO, pIda->ucbTerm );
            } /* endif */
            WinPostMsg (pIda->pLUPCB->hwndParent,
                        pIda->pLUPCB->usNotifyMsg,
                        (WPARAM)pIda->hwnd,
                        MP2FROM2SHORT (DLG_SHOWN, LOOKUP_DLG) );

            /**********************************************************/
            /* Keep dialog within TWB                                 */
            /**********************************************************/
            if ( !(pIda->pLUPCB->ulFlags & LUP_MODALDLG_MODE)  )
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
            } /* endif */

         }
         else // no allocmem IDA or termbuffer
         {
            if ( pIda ) UtlAlloc( (PVOID *)&pIda, 0L, 0L, NOMSG );
            if ( !(pIda->pLUPCB->ulFlags & LUP_MODALDLG_MODE) )
            {
              SendMessage( (HWND)UtlQueryULong( QL_TWBCLIENT ), WM_MDIDESTROY,
                           MP1FROMHWND(hwnd), 0L ) ;
            }
            else
            {
              WinDestroyWindow (hwnd);
            } /* endif */
            return 0;
         }
         SETFOCUS( hwnd, LOOKUP_WORD_COMBO );
         return MRFROMSHORT(TRUE);

    case WM_EQF_PROCESSTASK:
         pIda = ACCESSDLGIDA( hwnd, PLOOKIDA );
         fAnsiConv = TRUE;
                 if (GetCharSet() == THAI_CHARSET )
                 {
              fAnsiConv = FALSE;
                 }
         switch ( SHORT1FROMMP1(mp1) )
         {
            case SHOWCOMBO_TASK:
               if ( !pIda->fDisabled )
               {
                 SETCHECK_TRUE( hwnd, LOOKUP_ARBUT_HEADWORD);
                 SETCHECK_FALSE( hwnd, LOOKUP_ARBUT_NEIGHB);
                 SETCHECK_FALSE( hwnd, LOOKUP_ARBUT_COMPWORD);
                 SETCHECK_FALSE( hwnd, LOOKUP_ARBUT_SYN );
                 SETCHECK_FALSE( hwnd, LOOKUP_ARBUT_RELTERM );
                 SETCHECK_FALSE( hwnd, LOOKUP_ARBUT_ABBREV );
                 SETFOCUS( hwnd, LOOKUP_WORD_COMBO );
                 if ( pIda->usLookAction != LOOK_COMP_WORD )
                 {
                     //strcpy( szAnsiTerm, pIda->ucbTerm );
                     //CONDOEMTOANSI( fAnsiConv, szAnsiTerm );
                   COMBOSETTEXTW( pIda->hwnd, LOOKUP_WORD_COMBO, pIda->ucbTerm );
                   UtlDispatch();
                   pIda = ACCESSDLGIDA( hwnd, PLOOKIDA );
                 } /* endif */

                 if ( pIda != NULL )
                 {
                   if ( !pIda->fDisabled )
                   {
                     CBSHOWLIST( hwnd, LOOKUP_WORD_COMBO, FALSE );
                     CBSHOWLIST( hwnd, LOOKUP_WORD_COMBO, TRUE );
                   } /* endif */
                   pIda->usLookAction = LOOK_HW;
                 } /* endif */
               } /* endif */
               break;

            case LOOKUP_TASK:
               fAnsiConv = TRUE;
                                if (GetCharSet() == THAI_CHARSET )
                                {
                  fAnsiConv = FALSE;
                        }
               if ( !pIda->fDisabled )
               {
                 fOK = TRUE;
                 pIda->hFilter = NULL;
                 QUERYTEXT( hwnd, LOOKUP_ARBUT_FILT_CHG, pIda->szFilter );
                 if ( pIda->szFilter[0] )
                 {
                   usRC = FiltOpen( pIda->szFilter, pIda->pLUPCB->hDCB,
                                    &pIda->hFilter );
                   if ( usRC == NO_ERROR )
                   {
                     AsdRetDictList( pIda->pLUPCB->hDCB, ahDCB, &usNumOfDicts );
                     for ( usI = 0; (usI < usNumOfDicts) && !usRC; usI++ )
                     {
                       if ( AsdRetPropPtr( NULL, ahDCB[usI],
                                           &pDictProp ) == LX_RC_OK_ASD )
                       {
                         usRC = FiltCheckFields( pIda->hFilter,
                                                 pDictProp,
                                                 TRUE );
                       }
                       else
                       {
                         usRC = ERROR_INVALID_DATA;
                       } /* endif */
                     } /* endfor */
                   }
                   else
                   {
                     SETFOCUS( hwnd, ID_FILTER_COMBO );
                   } /* endif */
                   fOK = ( usRC == NO_ERROR );
                 } /* endif */

                 if ( fOK )
                 {
                   if ( !pIda->ucbTerm[0] )
                   {
                      QUERYTEXTW( hwnd, LOOKUP_WORD_COMBO, pIda->ucbTerm );
                      pIda->ucbTerm[sizeof(pIda->ucbTerm)/sizeof(CHAR_W) - 1] = EOS;
                      //CONDANSITOOEM( fAnsiConv, pIda->ucbTerm );
                      UtlStripBlanksW(pIda->ucbTerm);
                   } /* endif */

                   if (!memcmp (pIda->ucbTerm, L"*", 4))
                   {
                      pIda->usLookAction = LOOK_NEIGHB;
                      pIda->ucbTerm[0] = EOS;
                   } /* endif */

                   switch ( pIda->usLookAction )
                   {
                      case LOOK_COMP_WORD:
                         LookWildCard( pIda, TRUE );
                         break;
                      case LOOK_SYNONYMS:
                         LookIndex( pIda, SYNONYM_TYPE );
                         break;
                      case LOOK_REL_TERM:
                         LookIndex( pIda, RELATED_TYPE );
                         break;
                      case LOOK_ABBREV:
                         LookIndex( pIda, ABBREV_TYPE );
                         break;
                      case LOOK_HW:
                         LookHeadword( pIda );
                         break;
                      case LOOK_NEIGHB:
                         LookNeigh( pIda );
                         break;
                   } /* endswitch */
                 } /* endif */

                 if ( pIda->hFilter != NULL )
                 {
                   FiltClose( pIda->hFilter );
                 } /* endif */
               } /* endif */

               break;
         } /* endswitch */
         return 0;


    case WM_EQF_COMMAND:
      pIda = ACCESSDLGIDA( hwnd, PLOOKIDA );
    case WM_COMMAND:
      pIda = ACCESSDLGIDA( hwnd, PLOOKIDA );
      if ( pIda != NULL )
      {
        fAnsiConv = TRUE;
                if (GetCharSet() == THAI_CHARSET )
                {
             fAnsiConv = FALSE;
                }
        if ( !pIda->fWaitCursor )
        {
          switch ( WMCOMMANDID( mp1, mp2 ) )
          {
             case LOOKUP_PUBO_HELP:
                UtlInvokeHelp();
                break;
             case LOOKUP_PUBO_CANCEL:
             case DID_CANCEL:
                WinPostMsg (hwnd, WM_CLOSE, NULL, NULL);
                break;
             case LOOKUP_PUBO_OK:
                pIda->ucbTerm[0] = EOS;
                QUERYTEXTW( hwnd, LOOKUP_WORD_COMBO, pIda->ucbTerm );
                pIda->ucbTerm[ sizeof(pIda->ucbTerm)/sizeof(CHAR_W) - 1] = EOS;
                //CONDANSITOOEM( fAnsiConv, pIda->ucbTerm );
                UtlStripBlanksW( pIda->ucbTerm );
                if ( !pIda->fDisabled )
                {
                  WinPostMsg( hwnd, WM_EQF_PROCESSTASK,
                              MP1FROMSHORT(LOOKUP_TASK), NULL );
                } /* endif */
                break;
             case LOOKUP_PUBO_EDIT:
                pIda->ucbTerm[0] = EOS;
                QUERYTEXTW( hwnd, LOOKUP_WORD_COMBO, pIda->ucbTerm );
                pIda->ucbTerm[ sizeof(pIda->ucbTerm)/sizeof(CHAR_W) - 1] = EOS;
                //CONDANSITOOEM( fAnsiConv, pIda->ucbTerm );
                UtlStripBlanksW( pIda->ucbTerm );
                if ( (pIda->ucbTerm[0] == EOS) || pIda->fDisabled )
                {
                  WinAlarm( HWND_DESKTOP, WA_ERROR );
                }
                else
                {
                  HDCB      hdcbEdit = NULL;   // handle of dictionary containing term
                  ULONG     ulTermNum,  // term number
                            ulDataLen;  // size of term data in numb of char_w's
				  HMODULE hResMod;
				  hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);

                  if ( pIda->ucbTerm[0] != EOS )
                  {
                    USHORT usRC;

                    usRC = AsdFndMatch( pIda->ucbTerm,
                                       pIda->pLUPCB->hDCB,
                                       pIda->pLUPCB->hUCB,
                                       pIda->pLUPCB->ucTerm,
                                       &ulTermNum,
                                       &ulDataLen,
                                       &hdcbEdit );
                 //   usRC = AsdFndMatch( pIda->ucbTerm,
                 //                       pIda->pLUPCB->hDCB,
                 //                       pIda->pLUPCB->hUCB,
                 //                       pIda->pLUPCB->ucTerm,
                 //                       &ulTermNum,
                 //                       &ulDataLen,
                 //                       &hdcbEdit );

                    if (usRC == LX_WRD_NT_FND_ASD)
                    {
                       usRC = AsdFndEquivW( pIda->ucbTerm,
                                           pIda->pLUPCB->hDCB,
                                           pIda->pLUPCB->hUCB,
                                           pIda->pLUPCB->ucTerm,
                                           &ulTermNum,
                                           &ulDataLen,
                                           &hdcbEdit );
                    } /* endif */

                    if( usRC == LX_WRD_NT_FND_ASD )
                    {
                      AsdGetStemForm( pIda->pLUPCB->hUCB,
                                      pIda->pLUPCB->hDCB,
                                      pIda->ucbTerm,
                                      pIda->ucbTermBuf );

                       usRC = AsdFndMatch( pIda->ucbTermBuf,
                                           pIda->pLUPCB->hDCB,
                                           pIda->pLUPCB->hUCB,
                                           pIda->pLUPCB->ucTerm,
                                           &ulTermNum,
                                           &ulDataLen,
                                           &hdcbEdit );
                    } /* endif */

                    if ( usRC != LX_RC_OK_ASD )
                    {
                      hdcbEdit = NULL;
                      UTF16strcpy( pIda->pLUPCB->ucTerm, pIda->ucbTerm );
                    } /* endif */
                  }
                  else
                  {
                     pIda->pLUPCB->ucTerm[0] = EOS;
                  } /* endif */

                  pIda->fDisabled = TRUE;
                  if ( pIda->pLUPCB->ulFlags & LUP_MODALDLG_MODE  )
                  {
                    SearchAndEdit( hwnd, pIda->pLUPCB,
                                   pIda->pLUPCB->ucTerm, hResMod, hdcbEdit );
                  }
                  else
                  {
                    SearchAndEdit( pIda->pLUPCB->hwndParent, pIda->pLUPCB,
                                   pIda->pLUPCB->ucTerm, hResMod, hdcbEdit );
                  } /* endif */
                  pIda->fDisabled = FALSE;

                } /* endif */
                break;

                   case LOOKUP_WORD_COMBO:
                   case LOOKUP_ARBUT_HEADWORD:
                   case LOOKUP_ARBUT_NEIGHB:
                   case LOOKUP_ARBUT_COMPWORD:
                   case LOOKUP_ARBUT_SYN:
                   case LOOKUP_ARBUT_RELTERM:
                   case LOOKUP_ARBUT_ABBREV:
                       LookupDlgControl( hwnd, WMCOMMANDID( mp1, mp2 ),
                                         WMCOMMANDCMD( mp1, mp2 ));
                       break;
                   default:
                     IsFilterMessage( WinWindowFromID( hwnd, LOOKUP_ARBUT_FILT_CHG ),
                                      msg, mp1, mp2 );
              } /* endswitch */
        }
       else
       {
         WinAlarm( HWND_DESKTOP, WA_ERROR );
       } /* endif */
      } /* endif */
      return 0;

    case WM_CLOSE:
         pIda = ACCESSDLGIDA( hwnd, PLOOKIDA );
         /*************************************************************/
         /* Post Dialog hidden notification only if no more           */
         /* display dialog is active for this lookup session          */
         /*************************************************************/
         if ( pIda->pLUPCB->usDisplayDlgs == 0 )
         {
           WinPostMsg( pIda->pLUPCB->hwndParent,
                       pIda->pLUPCB->usNotifyMsg,
                       MP1FROMSHORT( pIda->pLUPCB->usID ),
                       MP2FROM2SHORT (DLG_HIDDEN, LOOKUP_DLG) );
         } /* endif */


         DelCtrlFont (hwnd, LOOKUP_WORD_COMBO);
//       if ( hwndCLBox )
//       {
//         DelCtrlFontHwnd( hwndCLBox );
//       } /* endif */
         if ( pIda->pLUPCB->ulFlags & LUP_MODALDLG_MODE )
         {
           WinDismissDlg( hwnd, FALSE );
         }
         else
         {
           SendMessage( (HWND)UtlQueryULong( QL_TWBCLIENT ), WM_MDIDESTROY,
                        MP1FROMHWND(hwnd), 0L ) ;
         } /* endif */

         return 0;

       case WM_EQF_INITMENU:
       case WM_INITMENU:
       case WM_INITMENUPOPUP:

         UtlMenuDisableAll( hwnd, SHORT1FROMMP1( mp1) );
         break;

       case WM_EQF_TOOLBAR_ENABLED:
         /*************************************************************/
         /* disable all toolbar items                                 */
         /*************************************************************/
         break;

     case DM_GETDEFID:
        /************************************************************/
        /* Handle ENTER key -> simulate press of LOOKUP button      */
        /* Windows seems to have problems with the disabled         */
        /* pushbuttons ...                                          */
        /************************************************************/
        {
          if ( GetKeyState(VK_RETURN) & 0x8000 )
          {
            if ( IsWindowEnabled( GetDlgItem( hwnd, LOOKUP_PUBO_OK ) ) )
            {
              PostMessage( hwnd, WM_EQF_COMMAND,
                           MP1FROMSHORT( LOOKUP_PUBO_OK ), 0L );
              return( MRFROMSHORT(TRUE) );
            } /* endif */
          } /* endif */
        }
        break;

    case WM_DESTROY:
         {
           ULONG ulFlags;              // lookup control flags

           pIda = ACCESSDLGIDA( hwnd, PLOOKIDA );
           ulFlags = pIda->pLUPCB->ulFlags;
           if ( !(ulFlags & LUP_MODALDLG_MODE) )
           {
             EqfRemoveObject(TWBFORCE, hwnd);
           } /* endif */
           if (pIda)
           {
              if ( WinIsWindow( (HAB)UtlQueryULong( QL_HAB ),
                             pIda->pLUPCB->hwndParent ) )
              {
                 WinPostMsg( pIda->pLUPCB->hwndParent,
                             pIda->pLUPCB->usNotifyMsg,
                             hwnd,
                             MP2FROM2SHORT( DLG_TERM_NORM, LOOKUP_DLG ) );
              } /* endif */
              pIda->pLUPCB->hwndLook = NULLHANDLE;

              if ( pIda->pucData ) UtlAlloc( (PVOID *)&pIda->pucData, 0L, 0L, NOMSG);
              if ( pIda->pucTermListBuffer )
                   UtlAlloc( (PVOID *)&pIda->pucTermListBuffer, 0L, 0L, NOMSG);

              UtlAlloc ((PVOID *)&pIda, 0L, 0L, NOMSG);
           }

           if ( !(ulFlags & LUP_MODALDLG_MODE) )
           {
             UtlUnregisterModelessDlg( hwnd );
           } /* endif */
         }
         break;
    } /* endswitch */

    return WinDefDlgProc (hwnd, msg, mp1, mp2);

} /* end LookupDlgProc */

MRESULT LookupDlgControl
(
   HWND   hwnd,                        // dialog handle
   SHORT  sId,                         // id in action
   SHORT  sNotification                // notification
)
{
  PLOOKIDA pIda;                       // ptr to structure with static information
  MRESULT mResult = MRFROMSHORT(FALSE);
  BOOL    fAnsiConv = TRUE;

  pIda = ACCESSDLGIDA( hwnd, PLOOKIDA );
  if ( !pIda->fWaitCursor )
  {
    fAnsiConv = TRUE;
        if (GetCharSet() == THAI_CHARSET )
        {
        fAnsiConv = FALSE;
        }
                  switch ( sId )
                  {
                    case LOOKUP_WORD_COMBO:
                       if ( sNotification == CBN_EFCHANGE )
                       {
                         QUERYTEXTW( hwnd, LOOKUP_WORD_COMBO, pIda->ucbTermBuf );
                         pIda->ucbTermBuf[sizeof(pIda->ucbTermBuf)/sizeof(CHAR_W)-1] = EOS;
                         //CONDANSITOOEM( fAnsiConv, pIda->ucbTermBuf );
                         UtlStripBlanksW( pIda->ucbTermBuf );
                         if ( pIda->ucbTermBuf[0] != EOS )
                         {
                           ENABLECTRL( hwnd, LOOKUP_PUBO_OK, TRUE );
                           ENABLECTRL( hwnd, LOOKUP_PUBO_EDIT, QUERYCHECK( hwnd, LOOKUP_ARBUT_HEADWORD ) );
                         }
                         else
                         {
                           ENABLECTRL( hwnd, LOOKUP_PUBO_OK, FALSE );
                           ENABLECTRL( hwnd, LOOKUP_PUBO_EDIT, FALSE );
                         } /* endif */
                       }
                       else if ( sNotification == CBN_SELCHANGE )
                       {
                         SHORT  sItem = CBQUERYSELECTION( hwnd, LOOKUP_WORD_COMBO );
                         if ( sItem != LIT_NONE )
                         {
                           CBQUERYITEMTEXTW( hwnd, LOOKUP_WORD_COMBO, sItem, pIda->ucbTermBuf );
                          // CONDANSITOOEM( fAnsiConv, pIda->ucbTermBuf );
                           UtlStripBlanksW( pIda->ucbTermBuf );
                           if ( pIda->ucbTermBuf[0] != EOS )
                           {
                             ENABLECTRL( hwnd, LOOKUP_PUBO_OK, TRUE );
                             ENABLECTRL( hwnd, LOOKUP_PUBO_EDIT, QUERYCHECK( hwnd, LOOKUP_ARBUT_HEADWORD ) );
                           }
                           else
                           {
                             ENABLECTRL( hwnd, LOOKUP_PUBO_OK, FALSE );
                             ENABLECTRL( hwnd, LOOKUP_PUBO_EDIT, FALSE );
                           } /* endif */
                         } /* endif */
                       }
                       else if ( sNotification == CBN_KILLFOCUS )
                       {
                         ClearIME( hwnd );
                       } /* endif */
                       break;

                    case LOOKUP_ARBUT_HEADWORD:
                       pIda->usLookAction = LOOK_HW;
                       if ( QUERYCHECK( hwnd, LOOKUP_ARBUT_HEADWORD ) )
                       {
                         QUERYTEXTW( hwnd, LOOKUP_WORD_COMBO, pIda->ucbTermBuf );
                         pIda->ucbTermBuf[sizeof(pIda->ucbTermBuf)/sizeof(CHAR_W)-1] = EOS;
                         //CONDANSITOOEM(fAnsiConv, pIda->ucbTermBuf );
                         UtlStripBlanksW( pIda->ucbTermBuf );
                         ENABLECTRL( hwnd, LOOKUP_PUBO_EDIT, (pIda->ucbTermBuf[0] != EOS) );
                       } /* endif */
                       if ( sNotification == BN_DBLCLICKED )
                       {
                         WinPostMsg( hwnd, WM_EQF_PROCESSTASK, MP1FROMSHORT(LOOKUP_TASK), NULL );
                       }
                       break;

                    case LOOKUP_ARBUT_NEIGHB:
                       pIda->usLookAction = LOOK_NEIGHB;
                       if ( !QUERYCHECK( hwnd, LOOKUP_ARBUT_HEADWORD ) )
                       {
                         ENABLECTRL( hwnd, LOOKUP_PUBO_EDIT, FALSE );
                       } /* endif */
                       if ( sNotification == BN_DBLCLICKED )
                       {
                         WinPostMsg( hwnd, WM_EQF_PROCESSTASK, MP1FROMSHORT(LOOKUP_TASK), NULL );
                       }
                       break;

                    case LOOKUP_ARBUT_COMPWORD:
                       pIda->usLookAction = LOOK_COMP_WORD;
                       if ( !QUERYCHECK( hwnd, LOOKUP_ARBUT_HEADWORD ) )
                       {
                         ENABLECTRL( hwnd, LOOKUP_PUBO_EDIT, FALSE );
                       } /* endif */
                       if ( sNotification == BN_DBLCLICKED )
                       {
                         WinPostMsg( hwnd, WM_EQF_PROCESSTASK, MP1FROMSHORT(LOOKUP_TASK), NULL );
                       }
                       break;

                    case LOOKUP_ARBUT_SYN:
                       pIda->usLookAction = LOOK_SYNONYMS;
                       if ( !QUERYCHECK( hwnd, LOOKUP_ARBUT_HEADWORD ) )
                       {
                         ENABLECTRL( hwnd, LOOKUP_PUBO_EDIT, FALSE );
                       } /* endif */
                       if ( sNotification == BN_DBLCLICKED )
                       {
                         WinPostMsg( hwnd, WM_EQF_PROCESSTASK, MP1FROMSHORT(LOOKUP_TASK), NULL );
                       }
                       break;

                    case LOOKUP_ARBUT_RELTERM:
                       pIda->usLookAction = LOOK_REL_TERM;
                       if ( !QUERYCHECK( hwnd, LOOKUP_ARBUT_HEADWORD ) )
                       {
                         ENABLECTRL( hwnd, LOOKUP_PUBO_EDIT, FALSE );
                       } /* endif */
                       if ( sNotification == BN_DBLCLICKED )
                       {
                         WinPostMsg( hwnd, WM_EQF_PROCESSTASK, MP1FROMSHORT(LOOKUP_TASK), NULL );
                       }
                       break;

                    case LOOKUP_ARBUT_ABBREV:
                       pIda->usLookAction = LOOK_ABBREV;
                       if ( !QUERYCHECK( hwnd, LOOKUP_ARBUT_HEADWORD ) )
                       {
                         ENABLECTRL( hwnd, LOOKUP_PUBO_EDIT, FALSE );
                       } /* endif */
                       if ( sNotification == BN_DBLCLICKED )
                       {
                         WinPostMsg( hwnd, WM_EQF_PROCESSTASK, MP1FROMSHORT(LOOKUP_TASK), NULL );
                       }
                       break;
                  } /* endswitch usControlID */
  } /* endif */
  return ( mResult );
} /* end of function LookupDlgControl */

VOID LookHeadword( PLOOKIDA pIda )
{
  HDCB          hEditHandle; // handle of dictionary containing entry
  PUCHAR        pucDictData; // Dictionary Data
  USHORT        usTermLen;   // length of term to be looked up and found
  USHORT        usASDRC;     // return code from ASD-calls
  BOOL          fWildCard;   // wildcard-mode search flag
  BOOL          fAnsiConv;

  fAnsiConv = TRUE;
  if (GetCharSet() == THAI_CHARSET )
  {
      fAnsiConv = FALSE;
  }
   if ( pIda->ucbTerm[0] != 0)                // check empty string
   {
     usTermLen = (USHORT)UTF16strlenCHAR( pIda->ucbTerm );
     pucDictData = NULL;

     /*****************************************************************/
     /* Check for wildcard characters in term being searched          */
     /*****************************************************************/
     {
       PSZ_W pszTemp = pIda->ucbTerm;

       while ( *pszTemp )
       {
         if ( (*pszTemp == MULTIPLE_SUBSTITUTION) ||
              (*pszTemp == SINGLE_SUBSTITUTION) )
         {
           break; // leave loop as substitution chars have been spotted
         }
         else
         {
           pszTemp++;                  // try next character
         } /* endif */
       } /* endwhile */
       fWildCard = (*pszTemp != EOS);
     }

     if ( fWildCard )
     {
        LookWildCard( pIda, FALSE );
     }
     else
     {
        usASDRC = AsdFndMatch( pIda->ucbTerm,
                               pIda->pLUPCB->hDCB,
                               pIda->pLUPCB->hUCB,
                               pIda->ucbTermBuf,
                               &pIda->ulTermNum,
                               &pIda->ulDataLen,
                               &hEditHandle );

        if (usASDRC == LX_WRD_NT_FND_ASD)
        {
           usASDRC = AsdFndEquivW( pIda->ucbTerm,
                                  pIda->pLUPCB->hDCB,
                                  pIda->pLUPCB->hUCB,
                                  pIda->ucbTermBuf,
                                  &pIda->ulTermNum,
                                  &pIda->ulDataLen,
                                  &hEditHandle );
        } /* endif */

        if( usASDRC == LX_WRD_NT_FND_ASD )
        {
          AsdGetStemForm( pIda->pLUPCB->hUCB,
                          pIda->pLUPCB->hDCB,
                          pIda->ucbTerm,
                          pIda->ucbTermBuf );

           usASDRC = AsdFndMatch( pIda->ucbTermBuf,
                                  pIda->pLUPCB->hDCB,
                                  pIda->pLUPCB->hUCB,
                                  pIda->ucbTermBuf,
                                  &pIda->ulTermNum,
                                  &pIda->ulDataLen,
                                  &hEditHandle );
        } /* endif */

        if( usASDRC == LX_WRD_NT_FND_ASD )
        {
           // allow edit of not existing term
           LupAskForEdit( pIda->hwnd, pIda->pLUPCB, pIda->ucbTerm);
        }
        else if (usASDRC != LX_RC_OK_ASD)
        {
          UtlError( usASDRC, MB_CANCEL, 0, NULL, QDAM_ERROR );
        }
        else if ( FilterMatch( pIda, pIda->ucbTermBuf ) )
        {
		   HMODULE hResMod;
		   hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
           if ( pIda->pLUPCB->ulFlags & LUP_MODALDLG_MODE  )
           {
             DisplayEntry( pIda->pLUPCB, pIda->ucbTermBuf, hResMod,
                           pIda->hwnd, pIda->szFilter );
           }
           else
           {
             DisplayEntry( pIda->pLUPCB, pIda->ucbTermBuf, hResMod,
                           pIda->hwndPrevFocus, pIda->szFilter );
           } /* endif */
           pIda->usLookAction = LOOK_HW;
           SETCHECK_TRUE( pIda->hwnd, LOOKUP_ARBUT_HEADWORD );
           SETCHECK_FALSE( pIda->hwnd, LOOKUP_ARBUT_NEIGHB );
           SETCHECK_FALSE( pIda->hwnd, LOOKUP_ARBUT_COMPWORD );
           SETCHECK_FALSE( pIda->hwnd, LOOKUP_ARBUT_SYN );
           SETCHECK_FALSE( pIda->hwnd, LOOKUP_ARBUT_RELTERM );
           SETCHECK_FALSE( pIda->hwnd, LOOKUP_ARBUT_ABBREV );
        }
        else
        {
            //UTF16strcpy( szAnsiTerm, pIda->ucbTerm );
            //CONDOEMTOANSI( fAnsiConv, szAnsiTerm );
           COMBOSETTEXTW( pIda->hwnd, LOOKUP_WORD_COMBO, pIda->ucbTerm );
           UtlError( ERROR_DLUP_NO_FILTER_MATCH, MB_OK, 0, NULL, EQF_INFO );
        } /* endif usASDRC */
     }  /* endif headword search */
   } // endif pIda->ucbTerm == ""
}

//+----------------------------------------------------------------------------+
//| LookWildCard            - Term Wildcard Search (pattern or compound search)|
//+----------------------------------------------------------------------------+
//| Description:                                                               |
//|    Lookup terms following the given pattern or terms having the given      |
//|    compound                                                                |
//+----------------------------------------------------------------------------+
//| Flow:                                                                      |
//+----------------------------------------------------------------------------+
//| Arguments:                                                                 |
//|    PLOOKIDA  pIda                IN      ptr to lookup dialog IDA          |
//+----------------------------------------------------------------------------+
//| Returns:                                                                   |
//|    Nothing                                                                 |
//+----------------------------------------------------------------------------+
//| Prereqs:                                                                   |
//|    None                                                                    |
//+----------------------------------------------------------------------------+
//| SideEffects:                                                               |
//|    None                                                                    |
//+----------------------------------------------------------------------------+
VOID LookWildCard( PLOOKIDA pIda, BOOL fCompound )
{
  USHORT        usTermLen;   // length of term to be looked up and found
  USHORT        usTermCount; // count terms for wildcard search and neighbh. up
                             // to MAX_TERMS
  USHORT        usASDRC;     // return code from ASD-calls
  PSZ_W         pszTerm;     // pointer to current term in term list
  USHORT        usTerms;     // number of terms in term list
  HWND          hwndCombo;            // handle of combo listbox
  BOOL          fAnsiConv;

  fAnsiConv = TRUE;
  if (GetCharSet() == THAI_CHARSET )
  {
      fAnsiConv = FALSE;
  }
  SETCURSOR( SPTR_WAIT );
  pIda->fWaitCursor = TRUE;
  WinEnableWindow( pIda->hwnd, FALSE );

  usTermCount = 0;
  usTerms     = 0;
  usTermLen   = (USHORT)UTF16strlenCHAR( pIda->ucbTerm );

  hwndCombo = WinWindowFromID( pIda->hwnd, LOOKUP_WORD_COMBO );

  CBSHOWLIST( pIda->hwnd, LOOKUP_WORD_COMBO, FALSE );
  CBDELETEALL( pIda->hwnd, LOOKUP_WORD_COMBO );

  UTF16strcpy( pIda->ucbTermBuf, pIda->ucbTerm );
  do
  {
    usTerms     = 0;
    usASDRC = AsdWildCardList( pIda->pLUPCB->hUCB,        // lookup handle
                               pIda->pLUPCB->hDCB,        // dictioanry handle
                               L"",                        // start key
                               pIda->ucbTermBuf,          // pattern/compound(s)
                               MAX_TERMS_WILD,            // max terms to return
                               fCompound,                 // Compound search???
                               pIda->pucTermListBuffer,   // buffer for terms
                               DLUP_TERMBUF_SIZE );       // size of buffer
    pszTerm = pIda->pucTermListBuffer;

    /******************************************************************/
    /* skip first term if this is not the first time we go through    */
    /* the outer loop                                                 */
    /******************************************************************/
    if ( usTermCount && *pszTerm )
    {
      pszTerm += UTF16strlenCHAR(pszTerm) + 1;
    } /* endif */

    while ( ((usASDRC == LX_RC_OK_ASD) || (usASDRC == LX_EOF_ASD)) &&
            (*pszTerm != EOS)          &&
            (usTermCount < MAX_TERMS_WILD ) )
    {
       usTerms++;
       UTF16strcpy( pIda->ucbTermBuf, pszTerm );

       if ( FilterMatch( pIda, pszTerm ) )
       {
           SHORT sItem;
           //CONDOEMTOANSI( fAnsiConv, pszTerm );
           sItem = LookupAddToCBIfNew( hwndCombo, pszTerm );
           if( sItem != LIT_NONE)
           {
             usTermCount++;
           } /* endif */
       } /* endif */
       pszTerm += UTF16strlenCHAR(pszTerm) + 1;
    } /* endwhile */

    if ( usASDRC != LX_RC_OK_ASD )
    {
      /****************************************************************/
      /* An error occured and will be handled later on                */
      /****************************************************************/
    }
    else if ( usTerms < MAX_TERMS_WILD )
    {
      /****************************************************************/
      /* End of dictionary has been reached                           */
      /****************************************************************/
      usASDRC = LX_EOF_ASD;
    }
    else if ( usTermCount == MAX_TERMS_WILD )
    {
      /****************************************************************/
      /* Maximum of terms has been processed ...                      */
      /* ... nothing to do here                                       */
      /****************************************************************/
    }
    else
    {
      /****************************************************************/
      /* Get next list of terms from dictionary                       */
      /****************************************************************/
    } /* endif */
  } while ( (usASDRC == LX_RC_OK_ASD)  &&
            (usTermCount < MAX_TERMS_WILD) ); /* enddo */

  SETCURSOR( SPTR_ARROW );
  pIda->fWaitCursor = FALSE;
  WinEnableWindow( pIda->hwnd, TRUE );

  if ( (usASDRC != LX_RC_OK_ASD) &&
       (usASDRC != LX_WRD_NT_FND_ASD) &&
       (usASDRC != LX_EOF_ASD) )
  {
    UtlError( usASDRC, MB_CANCEL, 0, NULL, QDAM_ERROR );
  }
  else if ( usTermCount == 0 )
  {
     UtlError( WD_ASD_BEG_NT_FND, MB_CANCEL, 0, NULL, EQF_INFO );
     SETFOCUS( pIda->hwnd, LOOKUP_WORD_COMBO );
  }
  else     // wildcard search and terms found
  {
     WinPostMsg( pIda->hwnd, WM_EQF_PROCESSTASK, MP1FROMSHORT(SHOWCOMBO_TASK), 0L );
  } /* endif */

   //strcpy( szAnsiTerm, pIda->ucbTerm );
   //CONDOEMTOANSI( fAnsiConv, szAnsiTerm );
   COMBOSETTEXTW( pIda->hwnd, LOOKUP_WORD_COMBO, pIda->ucbTerm );
 } /* end of LookWildCard */

VOID  LookNeigh( PLOOKIDA pIda )
{
   USHORT        usTermCount;          // count of terms
   USHORT        usASDRC;              // return code from ASD-calls
   BOOL          fItemSelected = FALSE;// a-listbox-item-has-been-selected flag
   BOOL          fExactMatch   = FALSE;// an exact match has been found
   SHORT         sItem;                // index of listbox items
   PSZ_W         pszTerm;              // pointer to current term in term list
   USHORT        usTerms;              // number of terms in term list
   HWND          hwndCombo;            // handle of combo listbox
   BOOL          fAnsiConv;

   fAnsiConv = TRUE;
   if (GetCharSet() == THAI_CHARSET )
   {
      fAnsiConv = FALSE;
   }
   SETCURSOR( SPTR_WAIT );
   pIda->fWaitCursor = TRUE;

   hwndCombo = WinWindowFromID( pIda->hwnd, LOOKUP_WORD_COMBO );

   CBSHOWLISTHWND( hwndCombo, FALSE );
   CBDELETEALLHWND( hwndCombo );

   //strcpy( szAnsiTerm, pIda->ucbTerm );
   //CONDOEMTOANSI( fAnsiConv, szAnsiTerm );
   COMBOSETTEXTW( pIda->hwnd, LOOKUP_WORD_COMBO, pIda->ucbTerm );
   usTermCount = 0;

  /********************************************************************/
  /* Search backward from current term                                */
  /********************************************************************/
  UTF16strcpy( pIda->ucbTermBuf, pIda->ucbTerm );
  do
  {
    usTerms     = 0;
    usASDRC = AsdTermList( pIda->pLUPCB->hUCB,
                           pIda->pLUPCB->hDCB,
                           pIda->ucbTermBuf,
                           MAX_TERMS,
                           QDAM_LIST_BACKWARD,
                           pIda->pucTermListBuffer,
                           DLUP_TERMBUF_SIZE );
    pszTerm = pIda->pucTermListBuffer;

    while ( (usASDRC == LX_RC_OK_ASD)  &&
            (*pszTerm != EOS)          &&
            (usTermCount <= (MAX_TERMS / 2)) &&
            (*pszTerm != EOS) )
    {
       usTerms++;
       UTF16strcpy( pIda->ucbTermBuf, pszTerm );

       if ( FilterMatch( pIda, pszTerm ) )
       {
         //CONDOEMTOANSI( fAnsiConv, pszTerm );
         sItem = LookupAddToCBIfNew( hwndCombo, pszTerm );
         if( sItem != LIT_NONE)
         {
           usTermCount++;
           if ( !fItemSelected )
           {
             CBSELECTITEMHWND( hwndCombo, sItem );
             fItemSelected = TRUE;
             if ( UTF16stricmp( pszTerm, pIda->ucbTerm ) == 0 )
             {
               fExactMatch = TRUE;
             } /* endif */
           } /* endif */
         } /* endif */
       } /* endif */
       pszTerm += UTF16strlenCHAR(pszTerm) + 1;
    } /* endwhile */

    if ( usASDRC != LX_RC_OK_ASD )
    {
      /****************************************************************/
      /* An error occured and will be handled later on                */
      /****************************************************************/
    }
    else if ( usTermCount == (MAX_TERMS / 2) )
    {
      /****************************************************************/
      /* Maximum of terms has been processed ...                      */
      /* ... nothing to do here                                       */
      /****************************************************************/
    }
    else if ( usTerms < MAX_TERMS )
    {
      /****************************************************************/
      /* End of dictionary has been reached                           */
      /****************************************************************/
      usASDRC = LX_EOF_ASD;
    }
    else
    {
      /****************************************************************/
      /* Get next list of terms from dictionary                       */
      /****************************************************************/
    } /* endif */
  } while ( (usASDRC == LX_RC_OK_ASD)  &&
            (usTermCount < (MAX_TERMS / 2)) ); /* enddo */

  fItemSelected = FALSE;

  /********************************************************************/
  /* Search forward from starting  term                               */
  /********************************************************************/
  if ( (usASDRC == LX_RC_OK_ASD) || (usASDRC == LX_EOF_ASD) )
  {
    UTF16strcpy( pIda->ucbTermBuf, pIda->ucbTerm );
    usASDRC = LX_RC_OK_ASD;
    do
    {
      usTerms     = 0;
      usASDRC = AsdTermList( pIda->pLUPCB->hUCB,
                             pIda->pLUPCB->hDCB,
                             pIda->ucbTermBuf,
                             MAX_TERMS,
                             QDAM_LIST_FORWARD,
                             pIda->pucTermListBuffer,
                             DLUP_TERMBUF_SIZE );
      pszTerm = pIda->pucTermListBuffer;

      while ( (usASDRC == LX_RC_OK_ASD)  &&
              (usTermCount <= MAX_TERMS) &&
              (*pszTerm != EOS) )
      {
         usTerms++;
         UTF16strcpy( pIda->ucbTermBuf, pszTerm );

         if ( FilterMatch( pIda, pszTerm ) )
         {
           //CONDOEMTOANSI( fAnsiConv, pszTerm );
           sItem = LookupAddToCBIfNew( hwndCombo, pszTerm );
           if( sItem != LIT_NONE)
           {
             usTermCount++;
             if ( !fItemSelected )
             {
               CBSELECTITEMHWND( hwndCombo, sItem );
               fItemSelected = TRUE;
               if ( UTF16stricmp( pszTerm, pIda->ucbTerm ) == 0 )
               {
                 fExactMatch = TRUE;
               } /* endif */
             } /* endif */
           } /* endif */
         } /* endif */
         pszTerm += UTF16strlenCHAR(pszTerm) + 1;
      } /* endwhile */

      if ( usASDRC != LX_RC_OK_ASD )
      {
        /****************************************************************/
        /* An error occured and will be handled later on                */
        /****************************************************************/
      }
      else if ( usTermCount == MAX_TERMS )
      {
        /****************************************************************/
        /* Maximum of terms has been processed ...                      */
        /* ... nothing to do here                                       */
        /****************************************************************/
      }
      else if ( usTerms < MAX_TERMS )
      {
        /****************************************************************/
        /* End of dictionary has been reached                           */
        /****************************************************************/
        usASDRC = LX_EOF_ASD;
      }
      else
      {
        /****************************************************************/
        /* Get next list of terms from dictionary                       */
        /****************************************************************/
      } /* endif */
    } while ( (usASDRC == LX_RC_OK_ASD)  &&
              (usTermCount < MAX_TERMS) ); /* enddo */
  } /* endif */

   if ( (usASDRC != LX_RC_OK_ASD) &&
        (usASDRC != LX_WRD_NT_FND_ASD) &&
        (usASDRC != LX_EOF_ASD) )
   {
     UtlError( usASDRC, MB_CANCEL, 0, NULL, QDAM_ERROR );
   }
   else if ( usTermCount == 0 )
   {
      UtlError( WD_ASD_BEG_NT_FND, MB_CANCEL, 0, NULL, EQF_INFO );
      SETFOCUS( pIda->hwnd, LOOKUP_WORD_COMBO );
   }
   else
   {
     /*****************************************************************/
     /* Shift selected item to the middle of the listbox              */
     /*****************************************************************/
     sItem = CBQUERYSELECTION( pIda->hwnd, LOOKUP_WORD_COMBO );
     if ( sItem != LIT_NONE )
     {
       /*****************************************************************/
       /* Restore term in entry field of combobox                       */
       /*****************************************************************/
       //strcpy( szAnsiTerm, pIda->ucbTerm );
       //CONDOEMTOANSI( fAnsiConv, szAnsiTerm );
       COMBOSETTEXTW( pIda->hwnd, LOOKUP_WORD_COMBO, pIda->ucbTerm );

       if ( !fExactMatch )
       {
         CBDESELECTITEMHWND( hwndCombo, sItem );
       } /* endif */

       CBSETTOPINDEXHWND( hwndCombo, max( (sItem - 4), 0 ) );
     } /* endif */

     WinPostMsg( pIda->hwnd, WM_EQF_PROCESSTASK, MP1FROMSHORT(SHOWCOMBO_TASK), 0L );
   } /* endif */

   SETCURSOR( SPTR_ARROW );
   pIda->fWaitCursor = FALSE;
}


VOID LookIndex( PLOOKIDA pIda, USHORT usTermType )
{
   USHORT        usTermCount;          // count of terms
   USHORT        usASDRC;              // return code from ASD-calls
   PSZ_W         pucTermBuf = NULL;    // buffer for returned terms
   PSZ_W         pucTerm;              // ptr to a term in pucTermBuf
   LONG          lUsed     = 0;        // used size of buffer
   LONG          lSize     = 0;        // actual size of buffer
   HDCB          hEditHandle;          // handle of dictionary containing entry
   SHORT         sItem = 0;            // index of listbox items
   BOOL          fAnsiConv;

   SETCURSOR( SPTR_WAIT );
   pIda->fWaitCursor = TRUE;
   fAnsiConv = TRUE;
   if (GetCharSet() == THAI_CHARSET )
   {
       fAnsiConv = FALSE;
   }
   CBSHOWLIST( pIda->hwnd, LOOKUP_WORD_COMBO, FALSE );
   CBDELETEALL( pIda->hwnd, LOOKUP_WORD_COMBO );

   usASDRC = AsdListIndex( pIda->pLUPCB->hUCB,
                           pIda->pLUPCB->hDCB,
                           pIda->ucbTerm,
                           usTermType,
                           &pucTermBuf,
                           &lUsed,
                           &lSize,
                           &usTermCount,
                           FALSE );

   pucTerm = pucTermBuf;                         // start at begin of term buf

   while ( (usASDRC == LX_RC_OK_ASD) && usTermCount )
   {
     /*****************************************************************/
     /* get entry and check for filter match                          */
     /*****************************************************************/
     if ( pIda->szFilter[0] != EOS )
     {
       usASDRC = AsdFndMatch( pucTerm,
                              pIda->pLUPCB->hDCB,
                              pIda->pLUPCB->hUCB,
                              pIda->ucbTermBuf,
                              &pIda->ulTermNum,
                              &pIda->ulDataLen,            //in numb of char_w's
                              &hEditHandle );

       if (usASDRC == LX_RC_OK_ASD)
       {
         if ( FilterMatch( pIda, pIda->ucbTermBuf ) )
         {
           //CONDOEMTOANSI( fAnsiConv, pucTerm );
           sItem = CBINSERTITEMW( pIda->hwnd, LOOKUP_WORD_COMBO, pucTerm );
         } /* endif */
       } /* endif */
     }
     else
     {
       //CONDOEMTOANSI( fAnsiConv, pucTerm );
       sItem = CBINSERTITEMW( pIda->hwnd, LOOKUP_WORD_COMBO, pucTerm );
     } /* endif */

     if ( sItem < 0 )
     {
       usASDRC = LX_EOF_ASD;
     } /* endif */

     if ( *pucTerm == EOS )
     {
       usASDRC = LX_EOF_ASD;
     } /* endif */

     pucTerm = pucTerm + UTF16strlenCHAR(pucTerm) + 1; // go to next entry in list
     usTermCount--;
   } /* endwhile */

   if ( pucTermBuf ) UtlAlloc( (PVOID *)&pucTermBuf, 0L, 0L, NOMSG );

   WinPostMsg( pIda->hwnd, WM_EQF_PROCESSTASK, MP1FROMSHORT(SHOWCOMBO_TASK), 0L );

   SETCURSOR( SPTR_ARROW );
   pIda->fWaitCursor = FALSE;
   //strcpy( szAnsiTerm, pIda->ucbTerm );
   //CONDOEMTOANSI( fAnsiConv, szAnsiTerm );
   COMBOSETTEXTW( pIda->hwnd, LOOKUP_WORD_COMBO, pIda->ucbTerm );
}

BOOL  FilterMatch( PLOOKIDA pIda, PSZ_W pszTerm )                 /* 1@KIT0967C */
{
  BOOL fMatch = FALSE;
  BOOL fOK = TRUE;
  USHORT      usAsdRC;               // return code of Asd calls
  USHORT      usLdbRC;               // return code of Ldb calls
  USHORT      usFltRC;               // return code of Filter calls
  HDCB        hEditHandle;           // handle of dictionary containing term
  PDCB        pDCB;                  // dictionary control block pointer
  PVOID       hLDBTree = NULL;       // LDB tree handle

  if ( pIda->szFilter[0] )
  {
    /******************************************************************/
    /* Position to term in dictionary                                 */
    /******************************************************************/
    usAsdRC = AsdFndMatch( pszTerm,
                           pIda->pLUPCB->hDCB,
                           pIda->pLUPCB->hUCB,
                           pIda->ucbTermBuf,
                           &pIda->ulTermNum,
                           &pIda->ulDataLen,         // in numb of char_w's
                           &hEditHandle );
    fOK = ( usAsdRC == LX_RC_OK_ASD );

    /******************************************************************/
    /* get buffer for entry data                                      */
    /******************************************************************/
    if ( pIda->ulDataLen > (ULONG)pIda->usDataSize )
    {
      fOK = UtlAlloc( (PVOID *)&pIda->pucData, (ULONG)pIda->usDataSize * sizeof(CHAR_W),
                      pIda->ulDataLen * sizeof(CHAR_W), ERROR_STORAGE );
      if ( fOK )
      {
        pIda->usDataSize = (USHORT)pIda->ulDataLen;
      } /* endif */
    } /* endif */

    /******************************************************************/
    /* get entry                                                      */
    /******************************************************************/
    if ( fOK )
    {
      usAsdRC = AsdRetEntryW( hEditHandle,
                             pIda->pLUPCB->hUCB,
                             pIda->ucbTermBuf,
                             &pIda->ulTermNum,
                             pIda->pucData,
                             &pIda->ulDataLen,
                             &hEditHandle );
      fOK = ( usAsdRC == LX_RC_OK_ASD );
    } /* endif */

    /****************************************************************/
    /* convert entry to LDB tree                                    */
    /****************************************************************/
    if ( fOK )
    {
      pDCB = (PDCB)hEditHandle;
      usLdbRC = QLDBRecordToTree( pDCB->ausNoOfFields,
                                  pIda->pucData,
                                  pIda->ulDataLen,
                                  &hLDBTree );
      fOK = ( usLdbRC == QLDB_NO_ERROR );
    } /* endif */

    /****************************************************************/
    /* Apply filter on LDB tree                                     */
    /****************************************************************/
    if ( fOK )
    {
      usFltRC = FiltWork2( pIda->hFilter, hLDBTree, NULL,
                hEditHandle );
      if ( usFltRC == NO_ERROR )
      {
        fMatch = TRUE;
      }
      else if ( usFltRC == ERROR_NO_MORE_FILES )
      {
        /************************************************************/
        /* processing was OK but entry did not match                */
        /************************************************************/
      }
      else
      {
        fOK = FALSE;
      } /* endif */
    } /* endif */

    /****************************************************************/
    /* cleanup                                                      */
    /****************************************************************/
    if ( hLDBTree )  QLDBDestroyTree( &hLDBTree );
  }
  else
  {
    fMatch = TRUE;
  } /* endif */
  return( fMatch );
} /* end of function FilterMatch */

BOOL LupAskForEdit
(
  HWND             hwnd,
  PLUPCB           pLUPCB,
  PSZ_W            pucTerm
)
{
  HDCB           ahDCB[MAX_DICTS+1];   // list of associated dictionaries
  USHORT         usNumOfDicts;         // number of dictionaries in association
  USHORT         usI;                  // loop index
  PPROPDICTIONARY pDictProp;           // ptr to dictionary properties

  /*******************************************************/
  /* Check if there are unprotected dictionaries before  */
  /* asking our add-to-dictionary question               */
  /*******************************************************/
  AsdRetDictList( pLUPCB->hDCB, ahDCB, &usNumOfDicts );
  for ( usI=0; usI < usNumOfDicts; usI++)
  {
    if ( AsdRetPropPtr( NULL, ahDCB[usI],
                        &pDictProp ) == LX_RC_OK_ASD )
    {
      if ( !pDictProp->fProtected && !pDictProp->fCopyRight )
      {
        break;
      } /* endif */
    } /* endif */
  } /* endfor */
  if ( usI >= usNumOfDicts )
  {
    /*****************************************************/
    /* no unprotected dictionaries in list               */
    /*****************************************************/
    //CONDOEMTOANSI( fAnsiConv, pucTerm );
    UtlErrorHwndW( PROTECTED_DICT_AND_NO_TERM, MB_OK, 1,
                  &pucTerm, EQF_INFO, hwnd, TRUE );
    //CONDANSITOOEM( fAnsiConv, pucTerm );
  }
  else
  {
    /*****************************************************/
    /* unprotected dictionaries in list                  */
    /*****************************************************/
    USHORT usMBCode;

    //CONDOEMTOANSI( fAnsiConv, pucTerm );
    usMBCode = UtlErrorHwndW( WD_ASD_WRD_NT_FND, MB_YESNO, 1,
                             &pucTerm, EQF_INFO, hwnd, TRUE );
    //CONDANSITOOEM( fAnsiConv, pucTerm );
    if ( usMBCode == MBID_YES)
    {
	  HMODULE hResMod;
	  hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
      SearchAndEdit( hwnd, pLUPCB, pucTerm, hResMod, NULL );
    } /* endif */
  } /* endif */
  return( TRUE );
}


LONG FAR PASCAL ComboEditSubclassProc
(
  HWND hwnd,
  WINMSG msg,
  WPARAM mp1,
  LPARAM mp2
)
{
  MRESULT mResult;
  HWND    hwndDlg;

  switch ( msg )
  {
    case WM_GETDLGCODE:
      mResult = CallWindowProc( (WNDPROC)pfnOrgComboEdit, hwnd, msg, mp1, mp2 );
      if ( mp1 == VK_RETURN )
      {
        mResult |= DLGC_WANTALLKEYS;
      } /* endif */
      return ( mResult );


    /******************************************************************/
    /* Generate a command message if the Enter key has been pressed   */
    /* and the lookup button is enabled                               */
    /******************************************************************/
    case WM_KEYDOWN:
      /****************************************************************/
      /* check if we have to handle it ...                            */
      /****************************************************************/
      switch ( mp1 )
      {
        case VK_RETURN:
          hwndDlg = GetParent( hwnd );        // get combobox handle
          hwndDlg = GetParent( hwndDlg );          // get dialog handle
          if ( IsWindowEnabled( GetDlgItem( hwndDlg, LOOKUP_PUBO_OK ) ) )
          {
            PostMessage( hwndDlg, WM_EQF_COMMAND, MP1FROMSHORT(LOOKUP_PUBO_OK), 0L );
          } /* endif */
          return ( 0L );
          break;
        case VK_TAB:
          {
            HWND hwndCombo = GetParent( hwnd );      // get combobox handle
            hwndDlg = GetParent( hwndCombo );        // get dialog handle
            SetFocus( GetNextDlgTabItem( hwndDlg, hwndCombo, FALSE ));
            return ( 0L );
          }
          break;
        case VK_BACKTAB:
          {
            hwndDlg = GetParent( hwnd );      // get combobox handle
            SetFocus( GetNextDlgTabItem( GetParent(hwndDlg), hwndDlg, TRUE ));
            return ( 0L );
          }
          break;

      } /* endswitch */
      break;

    case WM_KEYUP:
    case WM_CHAR:
      switch ( mp1 )
      {
        case VK_RETURN:
        case VK_TAB:
        case VK_BACKTAB:
          /************************************************************/
          /* already handled                                          */
          /************************************************************/
          return ( 0L );
          break;
      } /* endswitch */
      break;

  } /* endswitch */
  return( CallWindowProc( (WNDPROC)pfnOrgComboEdit, hwnd, msg, mp1, mp2 ) );
} /* end of function ComboEditSubclassProc */


LONG FAR PASCAL RadioButtonSubclassProc
(
  HWND hwnd,
  WINMSG msg,
  WPARAM mp1,
  LPARAM mp2
)
{
  MRESULT mResult;
  HWND    hwndDlg;

  switch ( msg )
  {
    case WM_GETDLGCODE:
      mResult = CallWindowProc( (WNDPROC)pfnOrgRadioButton, hwnd, msg, mp1, mp2 );
      if ( mp1 == VK_RETURN )
      {
        mResult |= DLGC_WANTALLKEYS;
      } /* endif */
      return ( mResult );


    /******************************************************************/
    /* Generate a command message if the Enter key has been pressed   */
    /* and the lookup button is enabled                               */
    /******************************************************************/
    case WM_KEYDOWN:
      switch ( mp1 )
      {
        case VK_RETURN:
          hwndDlg = GetParent( hwnd );        // get dialog handle
          if ( IsWindowEnabled( GetDlgItem( hwndDlg, LOOKUP_PUBO_OK ) ) )
          {
            PostMessage( hwndDlg, WM_EQF_COMMAND, MP1FROMSHORT(LOOKUP_PUBO_OK), 0L );
          } /* endif */
          break;
        case VK_TAB:
          SetFocus( GetNextDlgTabItem( GetParent( hwnd ), hwnd, FALSE ));
          return ( 0L );
          break;
        case VK_BACKTAB:
          SetFocus( GetNextDlgTabItem( GetParent( hwnd ), hwnd, TRUE ));
          return ( 0L );
          break;
      } /* endswitch */
      break;

  } /* endswitch */
  return( CallWindowProc( (WNDPROC)pfnOrgRadioButton, hwnd, msg, mp1, mp2 ) );
} /* end of function ComboEditSubclassProc */

/**********************************************************************/
/* Add given term to combobox if term is not in combobox yet          */
/* and return index of term or LIT_NONE if term is in combobox        */
/* already                                                            */
/**********************************************************************/
SHORT LookupAddToCBIfNew( HWND hwndCombo, PSZ_W pszTerm )
{
  /********************************************************************/
  /* Windows has no function to search case-sensitive, so we have     */
  /* to check all matches against our term being inserted             */
  /********************************************************************/
  BOOL    fFound = FALSE;
  CHAR_W  szTerm[MAX_TERM_LEN+1];
  SHORT   sItem = LIT_FIRST;             // combobox item index
  SHORT   sOldItem = LIT_FIRST;          // combobox item index

  do {
     sItem = (SHORT) CBSEARCHITEMPOSW( hwndCombo, sOldItem, pszTerm );

     if ( sItem <= sOldItem )
     {
       // wrap around!
       sItem = LIT_NONE;
     }
     else if ( sItem != LIT_NONE )
     {
       CBQUERYITEMTEXTHWNDW( hwndCombo, sItem, szTerm );
       if ( UTF16strcmp( szTerm, pszTerm ) == 0 )
       {
         fFound = TRUE;
       } /* endif */
       sOldItem = sItem;
     } /* endif */
  } while ( (sItem != LIT_NONE) && !fFound ); /* enddo */

  if( !fFound )
  {
    sItem = CBINSERTITEMHWNDW( hwndCombo, pszTerm );
  }
  else
  {
    sItem = LIT_NONE;
  } /* endif */

  return( sItem );
} /* end of function LookupAddToCBIfNew */

