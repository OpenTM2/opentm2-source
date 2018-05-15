//+----------------------------------------------------------------------------+
//|  EQFLNG00.C - Language update Handler and instance                         |
//+----------------------------------------------------------------------------+
//|  Copyright Notice:                                                         |
//|                                                                            |
//|      Copyright (C) 1990-2015, International Business Machines              |
//|      Corporation and others. All rights reserved                           |

//+----------------------------------------------------------------------------+
//|  Author       : Eberhard Schoeck                                           |
//|                 major rework: QSoft                                        |
//+----------------------------------------------------------------------------+
//|  Description  : Object handler and instance window procedure               |
//|                 for language update.                                       |
//+----------------------------------------------------------------------------+
//|  Entry Points :                                                            |
//+----------------------------------------------------------------------------+
//|  Externals    :                                                            |
//+----------------------------------------------------------------------------+
//|  Internals    :                                                            |
//+----------------------------------------------------------------------------+
//|  Include files:                                                            |
//+----------------------------------------------------------------------------+
//|  To be done / known limitations / caveats:                                 |
//|                                                                            |
//+----------------------------------------------------------------------------+

#define INCL_EQF_MORPH            // morphologic functions
#define INCL_EQF_LIST             // terminology list functions
#include <eqf.h>                  // General Translation Manager include file

#include <eqflng00.h>             // Language update  h-file
#include <eqflng00.id>            // Language Update id-file
// cv already included, see above
//#include <eqf.h>                  // General Translation Manager include file

#include "eqflist.id"             // List Handler IDs
#include "eqflp.h"                // Defines for generic list handlers
#define   EQFLIST_C               // initialize variables in EQFLISTI.H
#include "eqflisti.h"             // Private List Handler defines
#include "eqfcolw.id"             // column width IDs

#include "core\utilities\LanguageFactory.H"


#define QUERYITEMSTATE( hwnd, sItem ) \
   WinSendMsg( hwnd, LM_EQF_QUERYITEMSTATE, MP1FROMSHORT(sItem), 0L )

#define SETITEMSTATE( hwnd,  sItem, fState ) \
   WinSendMsg( hwnd, LM_EQF_SETITEMSTATE,    \
               MP1FROMSHORT(sItem), MP2FROMSHORT(fState) )

MRESULT LngListCallBack( PLISTCOMMAREA, HWND, WINMSG, WPARAM, LPARAM );

static BOOL fLangUpdRunning = TRUE;

// callback function to add languages to the column listbox
int AddLangToColumnLB( PVOID pvData, char *pszName, LanguageFactory::PLANGUAGEINFO pInfo  )
{ 
  static char szTextBuffer[512];
  char szAsciiCP[10];
  char szAnsiCP[10];

  ltoa( pInfo->lAsciiCP, szAsciiCP, 10 );
  ltoa( pInfo->lAnsiCP, szAnsiCP, 10 );

  sprintf( szTextBuffer, LNG_ITEM_FORMAT, pInfo->szName, szAsciiCP, szAnsiCP, pInfo->szIsoCPName, pInfo->szIsoID, pInfo->szIcuID, pInfo->szLangGroup, 
    pInfo->isSourceLanguage ? "yes" : "no",
    pInfo->isTargetLanguage ? "yes" : "no",
    "?", "?" );

  INSERTITEMHWND( (HWND)pvData, szTextBuffer ); 

  return( 0 ); 
}

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     LngListHandlerCallBack                                   |
//+----------------------------------------------------------------------------+
//|Description  : Object handler and instance window procedure                 |
//|               for language update.                                         |
//+----------------------------------------------------------------------------+
//|Returncode type:   MRESULT                                                  |
//+----------------------------------------------------------------------------+
//|Function flow: see documentation for Generic Handler layout                 |
//+----------------------------------------------------------------------------+
MRESULT LngListHandlerCallBack
(
  PHANDLERCOMMAREA pCommArea,
  HWND             hwnd,
  WINMSG           msg,
  WPARAM           mp1,
  LPARAM           mp2
)
{
  MRESULT          mResult = MRFROMSHORT(FALSE);

  mp1;                                 // avoid compiler warnings

  switch ( msg )
  {
    /******************************************************************/
    /* WM_CREATE: fill variables of communication area                */
    /******************************************************************/
    case WM_CREATE :
      pCommArea->pfnCallBack          = LngListCallBack;
      strcpy( pCommArea->szHandlerName, LNGUPDATEHANDLER );
      pCommArea->sBaseClass           = clsLNGUPDATE;
      pCommArea->sListWindowID        = ID_LNGUPDATE_WINDOW;
      pCommArea->sListboxID           = ID_LNGUPDATE_LB;

      /****************************************************************/
      /* Define object classes to be notified for EQFN messages       */
      /****************************************************************/
      pCommArea->asNotifyClassList[0] = clsLNGUPDATE;
      pCommArea->asNotifyClassList[1] = 0;       // end of list

      /****************************************************************/
      /* Define additional messages processed by the callback function*/
      /****************************************************************/
      pCommArea->asMsgsWanted[0]      = 0;       // end of list
      break;


    case WM_EQF_CREATE:
      mResult = WinSendMsg( hwnd, WM_EQF_CREATELISTWINDOW, NULL, mp2 );
      break;


    case WM_DESTROY:
      /****************************************************************/
      /* Nothing to do, as nothing has been allocated by the language */
      /* handler callback function                                    */
      /****************************************************************/
      break;


    default:
      break;
  } /* endswitch */
  return( mResult );
}

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     LngListCallBack                                          |
//+----------------------------------------------------------------------------+
//|Description:       Callback function for Language update handler            |
//+----------------------------------------------------------------------------+
//|Returncode type:   MRESULT                                                  |
//+----------------------------------------------------------------------------+
//|Function flow:  see Generic Handler documentation                           |
//+----------------------------------------------------------------------------+
MRESULT LngListCallBack
(
  PLISTCOMMAREA    pCommArea,
  HWND             hwnd,
  WINMSG           msg,
  WPARAM           mp1,
  LPARAM           mp2
)
{
  MRESULT          mResult = MRFROMSHORT(FALSE);

  switch ( msg )
  {
    case WM_CREATE :
      {
        HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
        BOOL       fOK = TRUE;         // initialisation is O.K. flag

        /**************************************************************/
        /* Set column listbox view lists                              */
        /* (nothing to do: lngupdate list only has one view)          */
        /**************************************************************/
        if ( fOK  )
        {
        } /* endif */

        /****************************************************************/
        /* supply all information required to create a lng update list  */
        /****************************************************************/
        if ( fOK )
        {
          pCommArea->sListObjClass  = clsLNGUPDATE;
          LOADSTRING( NULLHANDLE, hResMod, SID_LNGUP_TITLE, pCommArea->szTitle );
          pCommArea->hIcon          = (HPOINTER) UtlQueryULong(QL_LISTICON); //hiconLIST;
          pCommArea->fNoClose       = FALSE;
          pCommArea->sObjNameIndex  = LNG_LANGUAGE_IND;
          pCommArea->sNameIndex     = LNG_LANGUAGE_IND;
          pCommArea->sListWindowID  = ID_LNGUPDATE_WINDOW;
          pCommArea->sListboxID     = ID_LNGUPDATE_LB;
          pCommArea->sPopupMenuID   = ID_LNG_POPUP;
          pCommArea->sGreyedPopupMenuID   = -1; // no popup for greyed items
          // no pop up any more in case of NO selection, earlier: ID_LNG_POPUP_NOSEL;
          pCommArea->sNoSelPopupMenuID = -1;
          pCommArea->pColData       = &LngCLBData;
          pCommArea->fMultipleSel   = FALSE;
          pCommArea->sDefaultAction = PID_FILE_MI_OPEN;

          pCommArea->swpSizePos.x = 100;
          pCommArea->swpSizePos.y = 100;
          pCommArea->swpSizePos.cx = LNG_WINDOW_WIDTH;
          pCommArea->swpSizePos.cy = LNG_WINDOW_HEIGHT;
//        memcpy( &(pCommArea->swpSizePos), &(pProp->Swp), sizeof(EQF_SWP) );
          pCommArea->sItemClass     = clsLNGUPDATE;
          pCommArea->sItemPropClass = 0;
          pCommArea->asMsgsWanted[0] = WM_EQF_PROCESSTASK;
          pCommArea->asMsgsWanted[1] = 0;        // ends list of messages
        } /* endif */

        /**************************************************************/
        /* In case of errors set error return code                    */
        /**************************************************************/
        if ( !fOK )
        {
          mResult = MRFROMSHORT(DO_NOT_CREATE);
        } /* endif */
      }
      break;


    case WM_DESTROY:
      /****************************************************************/
      /* Free all resource allocated by list instance callback        */
      /* function                                                     */
      /****************************************************************/
      break;

    case WM_EQF_INITIALIZE:
      /****************************************************************/
      /* Fill column listbox                                          */
      /****************************************************************/
      {
        LanguageFactory *pLangFactory = LanguageFactory::getInstance();
        pLangFactory->listLanguages( LanguageFactory::ALL_LANGUAGES_TYPE, AddLangToColumnLB, (PVOID)pCommArea->hwndLB, TRUE );
      }

      /****************************************************************/
      /* start checking with first item ...                           */
      /****************************************************************/
      InvalidateRect( hwnd, NULL, TRUE );
      UpdateWindow( hwnd );
      UtlDispatch();
      fLangUpdRunning = TRUE;
      WinPostMsg( hwnd, WM_EQF_PROCESSTASK, MP1FROMSHORT(LNG_CHECKLNG), 0 );
      break;


    case WM_EQF_BUILDITEMTEXT :
      /****************************************************************/
      /* Setup item text for the object passed in mp2 parameter       */
      /****************************************************************/
      strcpy( pCommArea->szBuffer, (char *) PVOIDFROMMP2(mp2));
      mResult = MRFROMSHORT( TRUE );
      break;

    case WM_EQF_INITMENU:
    case WM_INITMENU:
      {
        SHORT sItem = QUERYNEXTSELECTIONHWND( pCommArea->hwndLB, LIT_FIRST );
        UtlMenuDisableAll( hwnd, SHORT1FROMMP1( mp1) );
        UtlMenuEnableItem( PID_VIEW_MI_NAMES );
        UtlMenuEnableItem( PID_VIEW_MI_DETAILS );
        if ( (sItem != LIT_NONE) && QUERYITEMSTATE( pCommArea->hwndLB, sItem))
        {
           UtlMenuEnableItem( PID_FILE_MI_OPEN );
           //UtlMenuEnableItem( PID_FILE_MI_DELETE );
        } /* endif */
      }
      break;

    case WM_EQF_TOOLBAR_ENABLED:
      switch ( mp1 )
      {
        case PID_VIEW_MI_NAMES:
        case PID_VIEW_MI_DETAILS:
          mResult = MRFROMSHORT(TRUE);
          break;
        case PID_FILE_MI_OPEN:
          {
            SHORT sItem = QUERYSELECTIONHWND( pCommArea->hwndLB );

            if ( (sItem != LIT_NONE) && QUERYITEMSTATE( pCommArea->hwndLB, sItem))
            {
              mResult = MRFROMSHORT(TRUE);
            } /* endif */
          }
          break;
        default:
          break;
      } /* endswitch */
      break;

    case WM_EQF_PROCESSTASK:
      /****************************************************************/
      /* get language and check if active or not                      */
      /****************************************************************/
      {
        SHORT sItem = SHORT1FROMMP2(mp2);
        BOOL  fInstalled;
        SHORT sID;
        CHAR  szLanguageInfo[512];
        CHAR  szWork[ STATE_LENGTH + 1 ];
        PSZ   pszTemp;
        BOOL  fDone = FALSE;           // success indicator

        if ( sItem >= QUERYITEMCOUNTHWND( pCommArea->hwndLB ) )
        {
          fDone = TRUE;
          SELECTITEMHWND( pCommArea->hwndLB, QUERYITEMCOUNTHWND (pCommArea->hwndLB)-1 );
        } /* endif */

        if ( !fDone )
        {
          SELECTITEMHWND( pCommArea->hwndLB, sItem );
          // Win2000 corrupts our data if we use LB_GETTEXT so use own message instead of QUERYITEMTEXTHWND( pCommArea->hwndLB, sItem, szLanguage );
          sID = (SHORT)SendMessage( pCommArea->hwndLB, LM_EQF_QUERYITEMTEXT, (WPARAM)sItem, (LPARAM)szLanguageInfo );

          if (sID == LIT_ERROR)
          {
            fDone = TRUE;
          } /* endif */
        } /* endif */

        if ( !fDone )
        {
          BOOL fSpell = FALSE;
          BOOL fMorph = FALSE;

          PSZ pszLanguage = UtlParseX15( szLanguageInfo, LNG_LANGUAGE_IND );
          PSZ pszAsciiCP = UtlParseX15( szLanguageInfo, LNG_ASCIICP_IND );
          PSZ pszAnsiCP = UtlParseX15( szLanguageInfo, LNG_ANSICP_IND );
          PSZ pszCPName = UtlParseX15( szLanguageInfo, LNG_CPNAME_IND );
          PSZ pszIsoName = UtlParseX15( szLanguageInfo, LNG_ISONAME_IND );
          PSZ pszIcuName = UtlParseX15( szLanguageInfo, LNG_ICUNAME_IND );
          PSZ pszGroup = UtlParseX15( szLanguageInfo, LNG_GROUP_IND );
          PSZ pszSource = UtlParseX15( szLanguageInfo, LNG_ISSOURCE_IND );
          PSZ pszTarget = UtlParseX15( szLanguageInfo, LNG_ISTARGET_IND );
          PSZ pszMorph = UtlParseX15( szLanguageInfo, LNG_MORPHSUPPORT_IND );
          PSZ pszSpell = UtlParseX15( szLanguageInfo, LNG_SPELLCHECK_IND );

          fSpell = isSpellCheckerAvailable( pszLanguage );
          fMorph = isMorphSupportAvailable( pszLanguage );

          pszMorph = fMorph ? "yes" : "no";
          pszSpell = fSpell ? "yes" : "no";

          sprintf( pCommArea->szBuffer, LNG_ITEM_FORMAT, pszLanguage, pszAsciiCP, pszAnsiCP, pszCPName, pszIsoName, pszIcuName, pszGroup, pszSource, pszTarget, pszMorph, pszSpell );

          SETITEMTEXTHWND( pCommArea->hwndLB, sItem, pCommArea->szBuffer );

          SETITEMSTATE( pCommArea->hwndLB,  sItem, fMorph );

          /**************************************************************/
          /* work on next one...                                        */
          /**************************************************************/
          {
           /***********************************************************/
           /* due to timing we have to enforce an invalidate all to   */
           /* get a correct repaint (at least for the first item...   */
           /***********************************************************/
           if ( sItem < 1 )
           {
             InvalidateRect( hwnd, NULL, TRUE);
             InvalidateRect( pCommArea->hwndLB, NULL, TRUE);
             UpdateWindow( pCommArea->hwndLB );
             UpdateWindow( hwnd );
           } /* endif */
            UtlDispatch();
            fLangUpdRunning = TRUE;
            WinPostMsg( hwnd, WM_EQF_PROCESSTASK,
                        MP1FROMSHORT(LNG_CHECKLNG), MP2FROMSHORT(sItem + 1));
          } 
        }
        else
        {
          InvalidateRect( hwnd, NULL, FALSE);
//        InvalidateRect( pCommArea->hwndLB, NULL, FALSE);
//        UpdateWindow( pCommArea->hwndLB );
          UpdateWindow( hwnd );
          fLangUpdRunning = FALSE;
        } /* endif */
      }
      break;
    case WM_EQF_COMMAND:
    case WM_COMMAND:
      mResult = MRFROMSHORT( TRUE ); // default return code for COMMAND msgs
      switch ( SHORT1FROMMP1(mp1) )
      {
        case PID_SYS_SIZE:
           SendMessage( hwnd, WM_SYSCOMMAND, SC_SIZE, 0L );
           break;
        case PID_SYS_MOVE:
           SendMessage( hwnd, WM_SYSCOMMAND, SC_MOVE, 0L );
           break;
        case PID_SYS_CLOSE:
          SendMessage( hwnd, WM_SYSCOMMAND, SC_CLOSE, 0L );
          break;
        case PID_FILE_MI_DELETE:
         /***********************************************************/
         /* get item to be deleted and do action what user requests.*/
         /***********************************************************/
         //{
         // PSZ   pszTemp;
         // SHORT sItem = QUERYSELECTIONHWND( pCommArea->hwndLB );

         // if (fLangUpdRunning)
         // {
         //   UtlError( ERROR_DIALOG_NOT_FINISHED, MB_CANCEL, 0, NULL, EQF_ERROR);
         // }
         // else
         // {
         //   if ( (sItem != LIT_NONE) && QUERYITEMSTATE( pCommArea->hwndLB, sItem))
         //   {
         //      USHORT usMBCode;

         //      // Win2000 corrupts our data if we use LB_GETTEXT so use own message instead
         //      //  QUERYITEMTEXTHWND( pCommArea->hwndLB, sItem, pCommArea->szBuffer );
         //      SendMessage( pCommArea->hwndLB, LM_EQF_QUERYITEMTEXT, (WPARAM)sItem, (LPARAM)pCommArea->szBuffer );
         //      pszTemp = UtlParseX15( pCommArea->szBuffer, LNG_LANGUAGE_IND );
         //      usMBCode = UtlError( WARNING_DELETE_LANGUAGE,
         //                           MB_YESNO | MB_DEFBUTTON2, 1,
         //                           &pszTemp, EQF_QUERY );
         //      DESELECTITEMHWND( pCommArea->hwndLB, sItem );
         //      if ( usMBCode == MBID_YES )
         //      {
         //        /*******************************************************/
         //        /* check if language is in use at the moment           */
         //        /*******************************************************/
         //        SHORT sID;
         //        MorphCheckLanguage( pszTemp, &sID );
         //        if ( sID > -1 )
         //        {
         //          /*****************************************************/
         //          /* error message -- language currently in use ...    */
         //          /*****************************************************/
         //          UtlError( ERROR_SUPPORTFILE_NOT_DELETED, MB_CANCEL,
         //                    1, &pszTemp, EQF_INFO);
         //        }
         //        else
         //        {
         //          /*****************************************************/
         //          /* delete everything associated with this language   */
         //          /*****************************************************/
         //          if (!UtlDeleteLanguage( pszTemp, TRUE ))
         //          {
         //            CHAR  szWork[ STATE_LENGTH + 1 ];
         //            LOADSTRING( NULLHANDLE, hResMod, SID_LNG_NOT_ACTIVE, szWork );
         //            sprintf(pCommArea->szBuffer, LNG_ITEM_FORMAT,
         //                    pszTemp, szWork );
         //            SETITEMTEXTHWND( pCommArea->hwndLB, sItem,
         //                             pCommArea->szBuffer );
         //            SETITEMSTATE( pCommArea->hwndLB,  sItem, FALSE );
         //          } /* endif */
         //        } /* endif */
         //      } /* endif */
         //   }
         //   else
         //   {
         //     WinAlarm( HWND_DESKTOP, WA_WARNING );
         //   } /* endif */
         // } /* endif */
         //}
          break;

        case PID_FILE_MI_OPEN:
         /***********************************************************/
         /* get item to be opened and do requested action           */
         /***********************************************************/
         {
          PSZ   pszTemp;
          SHORT sItem = QUERYSELECTIONHWND( pCommArea->hwndLB );

          if (fLangUpdRunning)
          {
            UtlError( ERROR_DIALOG_NOT_FINISHED, MB_CANCEL, 0, NULL, EQF_ERROR);
          }
          else
          {
            if ( (sItem != LIT_NONE) && QUERYITEMSTATE( pCommArea->hwndLB, sItem))
            {
               // Win2000 corrupts our data if we use LB_GETTEXT so use own message instead
               //  QUERYITEMTEXTHWND( pCommArea->hwndLB, sItem, pCommArea->szBuffer );
               SendMessage( pCommArea->hwndLB, LM_EQF_QUERYITEMTEXT, (WPARAM)sItem, (LPARAM)pCommArea->szBuffer );
               pszTemp = UtlParseX15( pCommArea->szBuffer, LNG_LANGUAGE_IND );

               LstEditAbbrev( pszTemp );
            }
            else
            {
              WinAlarm( HWND_DESKTOP, WA_WARNING );
            } /* endif */
          } /* endif */
         }
          break;

        default:
          if( msg == WM_EQF_COMMAND)
          {
            mResult = MRFROMSHORT( FALSE ); // tell twbmain that we rejected
          } /* endif */
      }
      break;

    default:
      break;
  } /* endswitch */
  return( mResult );
} /* end of function LanguageListCallBack */


