/*! \brief EQFTAG00.C - EQF Tag Table Handler
         Copyright (C) 1990-2016, International Business Machines          |
         Corporation and others. All rights reserved                       |
*/

#define INCL_EQF_TAGTABLE         // tagtable defines and functions
#include "eqf.h"                  // General .H for EQF
#include "eqftag00.h"             // tag table handler defines
#include "eqftag00.id"            // tag table handler window IDs
#include "eqfcolw.id"             // column width IDs
#include "MarkupPluginMapper.H"


  static SHORT sLastUsedView[MAX_VIEW+1] = { TAG_NAME_IND, CLBLISTEND };
  static SHORT sDefaultView[MAX_VIEW+1]  = { TAG_NAME_IND, TAG_SHORTDESCR_IND, CLBLISTEND };
  static SHORT sNameView[MAX_VIEW+1]     = { TAG_NAME_IND, CLBLISTEND };
  static SHORT sDetailsView[MAX_VIEW+1]  = { TAG_NAME_IND, TAG_SHORTDESCR_IND, CLBLISTEND };
  static SHORT sSortCriteria[MAX_VIEW+1] = { TAG_NAME_IND, CLBLISTEND };

  static CHAR  ColHdr[7][30];

  static CLBCOLDATA ColTagt[] =
  { { "", 1, TEXT_DATA, DT_LEFT },
    { ColHdr[0] , CLB_MAX_FNAME,       AUTOWIDTHTEXT_DATA, DT_LEFT },
    { ColHdr[1] , CLB_MAX_DESCRIPTION, TEXT_DATA,          DT_LEFT },
    { ColHdr[2] , CLB_MAX_DESCRIPTION, TEXT_DATA,          DT_LEFT },
    { ColHdr[3] , CLB_MAX_VERSION,     AUTOWIDTHTEXT_DATA, DT_LEFT },
    { ColHdr[4] , CLB_MAX_FNAME,       AUTOWIDTHTEXT_DATA, DT_LEFT },
    { ColHdr[5] , CLB_MAX_DESCRIPTION, TEXT_DATA,          DT_LEFT },
    { NULL,       0,                   (CLBDATATYPE) 0,    0      } };   //

  static CLBCTLDATA TagtCLBData =
  {  sizeof(CLBCTLDATA),                 // size of control area
     4,                                  // we have 3 data columns
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
     ColTagt };                          // set address of column definition table

 #include "eqfstart.id"                // format name IDs


// size of dynamic markup table info table (in elements)
USHORT usMarkupInfoElements = 0;

static PSZ pszFormatFilters = NULL;          // ready-to-use filter strings

USHORT MULoadDescrNames();



MRESULT TagListCallBack( PLISTCOMMAREA, HWND, WINMSG, WPARAM, LPARAM );

/**********************************************************************/
/* Handler callback function for tag table list handler               */
/**********************************************************************/
MRESULT TagListHandlerCallBack
(
  PHANDLERCOMMAREA pCommArea,
  HWND             hwnd,
  WINMSG           msg,
  WPARAM           mp1,
  LPARAM           mp2
)
{
  MRESULT          mResult = MRFROMSHORT(FALSE);

  switch ( msg )
  {
    /******************************************************************/
    /* WM_CREATE: fill variables of communication area                */
    /******************************************************************/
    case WM_CREATE :
      pCommArea->pfnCallBack          = TagListCallBack;
      strcpy( pCommArea->szHandlerName, TAGTABLEHANDLER );
      pCommArea->sBaseClass           = clsTAGTABLE;
      pCommArea->sListWindowID        = ID_TAGTABLE_WINDOW;
      pCommArea->sListboxID           = ID_TAGTABLE_LB;

      /****************************************************************/
      /* Define object classes to be notified for EQFN messages       */
      /****************************************************************/
      pCommArea->asNotifyClassList[0] = clsTAGTABLE;
      pCommArea->asNotifyClassList[1] = 0;       // end of list

      /****************************************************************/
      /* Define additional messages processed by the callback function*/
      /****************************************************************/
      pCommArea->asMsgsWanted[0]      = 0;       // end of list

      break;

    case WM_EQF_OPEN:
      {
        HWND       hwndObj;
        PSZ        pszObj = (PSZ) PVOIDFROMMP2(mp2);

        hwndObj = EqfQueryObject( pszObj, clsTAGTABLE, 0);
        if( hwndObj )
        {
          SETFOCUSHWND( hwndObj );
          mResult = MRFROMSHORT( TRUE );
        }
        else
        {
          mResult = WinSendMsg( hwnd, WM_EQF_CREATELISTWINDOW, NULL, mp2 );
        } /* endif */
      }
      break;

    case WM_EQF_CREATE:
      mResult = MRFROMSHORT( TRUE );
      break;

    case WM_EQF_INSERTNAMES:
      {
        HWND       hwndLB = HWNDFROMMP1(mp1);
        MUFillLBWithMarkups( hwndLB, NULL, 0, MUFILL_NAMES );
      }
      break;

    case WM_DESTROY:
      /****************************************************************/
      /* Nothing to do, as nothing has been allocated by the tag table*/
      /* handler callback function                                    */
      /****************************************************************/
      break;

    case WM_EQF_DELETE:
      {
        PLISTCOMMAREA  pCommArea2 =(PLISTCOMMAREA) PVOIDFROMMP2(mp2);
        SHORT          sItem = QUERYSELECTIONHWND( pCommArea2->hwndLB );
        PSZ            pszMarkup ;
        PSZ            pszPlugin ;        
        PSZ            pszFilePath ;        

        SendMessage( pCommArea2->hwndLB, LM_EQF_QUERYITEMTEXT, (WPARAM)sItem, (LPARAM)pCommArea2->szBuffer );
        pszMarkup = UtlParseX15( pCommArea2->szBuffer, TAG_NAME_IND );
        pszPlugin = UtlParseX15( pCommArea2->szBuffer, TAG_PLUGIN_IND );
        UtlAlloc( (PVOID *)&pszFilePath, 0L, 512, ERROR_STORAGE );
        MUGetMarkupTableFilePath( pszMarkup, pszPlugin, pszFilePath, 512 ) ;

        if ( isMarkupDeletable( pszMarkup, pszPlugin ) ) {

           //--- delete the tagtable ---
           if( EqfSend2AllHandlers( WM_EQF_ABOUTTODELETE,
                                    MP1FROMSHORT( clsTAGTABLE ),
                                    MP2FROMP(pszFilePath) ) ) {
              UtlError( ERROR_TABLE_NOT_DELETED, MB_CANCEL,
                        1, &pszMarkup, EQF_INFO);
              mResult = MRFROMSHORT(TRUE);
           } else 
           if( UtlDelete( pszFilePath, 0L, FALSE ) == 0 ) {
              MUDeleteMarkupTable( pszMarkup, pszPlugin ) ;
              DELETEITEMHWND( pCommArea2->hwndLB, sItem ) ;
              EqfSend2AllHandlers( WM_EQFN_DELETED,
                                   MP1FROMSHORT( clsTAGTABLE ),
                                   MP2FROMP(pszFilePath) );
              /***********************************************************/
              /* invalidate this particular tag table..                  */
              /***********************************************************/
              TAInvalidateTagTable( pszFilePath );
           } else {
              UtlError( ERROR_TABLE_NOT_DELETED, MB_CANCEL,
                        1, &pszMarkup, EQF_INFO);
              mResult = MRFROMSHORT(TRUE);
           } /* endif */
        }
        UtlAlloc( (PVOID *)&pszFilePath, 0L, 0L, NOMSG );
      }

      break;

    default:
      break;
  } /* endswitch */
  return( mResult );
} /* end of function TagListHandlerCallBack */

/**********************************************************************/
/* List instance callback function for tag table list window          */
/**********************************************************************/
MRESULT TagListCallBack
(
  PLISTCOMMAREA    pCommArea,
  HWND             hwnd,
  WINMSG           msg,
  WPARAM           mp1,
  LPARAM           mp2
)
{
  MRESULT          mResult = MRFROMSHORT(FALSE);
  SHORT sItem;

  switch ( msg )
  {
    case WM_CREATE :
      {
        HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
        BOOL       fOK = TRUE;         // initialisation is O.K. flag
        EQFINFO    ErrorInfo;          // error info of property handler
        HPROP      hProp = NULL;       // tag table list properties handle
        PPROPTAGTABLE   pProp = NULL;         // ptr to TagTable properties

        //--- access or create properties ---
        hProp = OpenProperties( pCommArea->szObjName, NULL,
                                PROP_ACCESS_READ, &ErrorInfo);
        if( !hProp )
        { //--- properties access error, let's try to create new ones ... ---
           UtlAlloc( (PVOID *)&pProp, 0L, (LONG) sizeof( *pProp), ERROR_STORAGE );
           hProp = NULL;
           UtlMakeEQFPath( pProp->PropHead.szPath, NULC, SYSTEM_PATH, NULL );
           pProp->PropHead.usClass = PROP_CLASS_TAGTABLE;
           strcpy( pProp->PropHead.szName,
                   UtlGetFnameFromPath( pCommArea->szObjName) );
           pProp->PropHead.chType  = PROP_TYPE_INSTANCE;
           pProp->Swp.x  = 200;                  // ... set default pos
           pProp->Swp.cx = 350;
           pProp->Swp.y  = 200;
           pProp->Swp.cy = 200;
           hProp = CreateProperties( pCommArea->szObjName,(PSZ) NULP,
                                     PROP_CLASS_TAGTABLE,
                                     &ErrorInfo );
           PutAllProperties( hProp, pProp, &ErrorInfo );
           SaveProperties( hProp, &ErrorInfo );
           UtlAlloc( (PVOID *) &pProp, 0L, 0L, NOMSG );
        } /* endif */

        /**************************************************************/
        /* Load column listbox title strings                          */
        /**************************************************************/
        if ( fOK  )
        {
          LOADSTRING( NULLHANDLE, hResMod, IDS_TAGT_NAME_COLTITLE, ColHdr[0]);
          UtlLoadWidth( NULLHANDLE, hResMod, IDS_TAGT_NAME_COLWIDTH,
                        &(ColTagt[1].usWidth) );
          LOADSTRING( NULLHANDLE, hResMod, IDS_TAGT_DESCRNAME_COLTITLE, ColHdr[1]);
          UtlLoadWidth( NULLHANDLE, hResMod, IDS_TAGT_DESCRNAME_COLWIDTH,
                        &(ColTagt[2].usWidth) );
          LOADSTRING( NULLHANDLE, hResMod, IDS_TAGT_DESCR_COLTITLE, ColHdr[2]);
          UtlLoadWidth( NULLHANDLE, hResMod, IDS_TAGT_DESCR_COLWIDTH,
                        &(ColTagt[3].usWidth) );
          LOADSTRING( NULLHANDLE, hResMod, IDS_TAGT_VERSION_COLTITLE, ColHdr[3]);
          UtlLoadWidth( NULLHANDLE, hResMod, IDS_TAGT_VERSION_COLWIDTH,
                        &(ColTagt[3].usWidth) );
          LOADSTRING( NULLHANDLE, hResMod, IDS_TAGT_PLUGIN_COLTITLE, ColHdr[4]);
          UtlLoadWidth( NULLHANDLE, hResMod, IDS_TAGT_PLUGIN_COLWIDTH,
                        &(ColTagt[4].usWidth) );
          LOADSTRING( NULLHANDLE, hResMod, IDS_TAGT_SUPPLIER_COLTITLE, ColHdr[5]);
          UtlLoadWidth( NULLHANDLE, hResMod, IDS_TAGT_SUPPLIER_COLWIDTH,
                        &(ColTagt[5].usWidth) );
        } /* endif */
        /**************************************************************/
        /* Set column listbox view lists                              */
        /**************************************************************/
        if (fOK)
        {
          pProp =(PPROPTAGTABLE) MakePropPtrFromHnd( hProp);
        }
        if ( fOK  )
        {
            int i;
            memcpy( pCommArea->asCurView,
                    (pProp->sLastUsedViewList[0] != 0) ? pProp->sLastUsedViewList :
                    sLastUsedView,
                    sizeof(pCommArea->asCurView) );
            TagtCLBData.psLastUsedViewList = pCommArea->asCurView;

            memcpy( pCommArea->asDetailsView,
                    (pProp->sDetailsViewList[0] != 0) ? pProp->sDetailsViewList :
                    sDetailsView,
                    sizeof(pCommArea->asDetailsView) );
            TagtCLBData.psDetailsViewList = pCommArea->asDetailsView;

            memcpy( pCommArea->asSortList,
                    (pProp->sSortList[0] != 0) ? pProp->sSortList :
                    sSortCriteria,
                    sizeof(pCommArea->asSortList) );
            TagtCLBData.psSortList = pCommArea->asSortList;

            for (i=0;pCommArea->asCurView[i]>0;i++)
            {
                int index = pCommArea->asCurView[i];
                if (pProp->sLastUsedViewWidth[i] > 0)
                {
                    ColTagt[index].usWidth = pProp->sLastUsedViewWidth[i];
                } else
                {
                    pProp->sLastUsedViewWidth[i] = ColTagt[index].usWidth;
                }
            } // end for
            memcpy( pCommArea->asCurViewWidth, (pProp->sLastUsedViewWidth),
                    sizeof(pCommArea->asCurViewWidth) );

        } /* endif */

        /****************************************************************/
        /* supply all information required to create a tag table list   */
        /****************************************************************/
        if ( fOK )
        {
          pCommArea->sListObjClass  = clsTAGTABLE;
          LOADSTRING( NULLHANDLE, hResMod, SID_TAGTABLE_TITLE, pCommArea->szTitle );
          pCommArea->hIcon          = (HPOINTER) UtlQueryULong(QL_TAGICON); //hiconTAG;
          pCommArea->fNoClose       = FALSE;
          pCommArea->sObjNameIndex  = TAG_OBJECT_IND;
          pCommArea->sNameIndex     = TAG_NAME_IND;
          pCommArea->sListWindowID  = ID_TAGTABLE_WINDOW;
          pCommArea->sListboxID     = ID_TAGTABLE_LB;
          pCommArea->sPopupMenuID   = ID_TAG_POPUP;
          pCommArea->sGreyedPopupMenuID   = ID_TAG_POPUP;
          pCommArea->sNoSelPopupMenuID = ID_TAG_POPUP_NOSEL;
          pCommArea->pColData       = &TagtCLBData;
          pCommArea->fMultipleSel   = FALSE;
          pCommArea->sDefaultAction = 0;
          memcpy( &(pCommArea->swpSizePos), &(pProp->Swp), sizeof(EQF_SWP) );
          pCommArea->sItemClass     = clsTAGTABLE;
          pCommArea->sItemPropClass = PROP_CLASS_TAGTABLE;
          pCommArea->asMsgsWanted[0] = 0;        // ends list of messages
        } /* endif */

        /**************************************************************/
        /* Close properties                                           */
        /**************************************************************/
        if ( hProp )
        {
          CloseProperties( hProp, PROP_QUIT, &ErrorInfo );
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

    case WM_CLOSE :
    case WM_EQF_TERMINATE :
      /**************************************************************/
      /* Save window pos for WM_EQF_TERMINATE only if save flag is  */
      /* on                                                         */
      /**************************************************************/
      if ( (msg == WM_CLOSE) || (SHORT1FROMMP1(mp1) & TWBSAVE) )
      {
        PPROPTAGTABLE   pProp;         // ptr to TagTable properties
        EQFINFO       ErrorInfo;
        HPROP         hProp;

        /**************************************************************/
        /* Open properties                                            */
        /**************************************************************/
        hProp = OpenProperties( pCommArea->szObjName, NULL,
                                PROP_ACCESS_READ, &ErrorInfo );
        if( hProp )
        {
          /************************************************************/
          /* Save current window position                             */
          /************************************************************/
          if( SetPropAccess( hProp, PROP_ACCESS_WRITE ) )
          {
            pProp =(PPROPTAGTABLE) MakePropPtrFromHnd( hProp);
            memcpy( &pProp->Swp, &(pCommArea->swpSizePos), sizeof(EQF_SWP) );
            memcpy( pProp->sLastUsedViewList, pCommArea->asCurView,
                    sizeof(pProp->sLastUsedViewList) );

            memcpy( (pProp->sLastUsedViewWidth), (pCommArea->asCurViewWidth),
                     sizeof( pProp->sLastUsedViewWidth ));

            memcpy( pProp->sDetailsViewList, pCommArea->asDetailsView,
                    sizeof(pProp->sDetailsViewList) );
            memcpy( pProp->sSortList, pCommArea->asSortList,
                     sizeof(pProp->sSortList) );
            SaveProperties( hProp, &ErrorInfo );
          } /* endif */
          CloseProperties( hProp, PROP_QUIT, &ErrorInfo );
        } /* endif */
      } /* endif */
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
      MUFillLBWithMarkups( pCommArea->hwndLB, pCommArea->szBuffer, sizeof(pCommArea->szBuffer), MUFILL_CLBITEMS );
      break;

    case WM_EQF_BUILDITEMTEXT :
      /****************************************************************/
      /* Setup item text for the object passed in mp2 parameter       */
      /****************************************************************/
      {
        PSZ        pszObj = (PSZ)PVOIDFROMMP2(mp2);   // ptr to object name
        PSZ        pszTemp;
        PSZ        pszMarkup;
        PSZ        pszPlugin;

        pszTemp = strrchr( pszObj, '.' ) ;
        pszMarkup = strrchr( pszObj, '\\' ) ;
        if ( pszTemp && pszMarkup ) {
           *pszTemp = NULL ;
           ++pszMarkup ; 
        } else {
           pszMarkup = pszObj ;
        }
        pszPlugin = strstr( pszObj, "\\PLUGINS\\" ) ;
        if ( pszPlugin ) {
           pszPlugin += 9 ;
           pszTemp = strchr( pszPlugin, '\\' ) ;
           if ( pszTemp ) 
              *pszTemp = NULL ;
        } else {
           pszPlugin = NULL ;
        }
        if ( ! pszPlugin ) {
           OtmMarkupPlugin *plugin = GetMarkupPlugin( pszMarkup );
           if ( plugin ) 
              pszPlugin = (PSZ) plugin->getName();
        }
        TagMakeListItem( pszMarkup, pszPlugin, pCommArea->szBuffer, sizeof(pCommArea->szBuffer) );
        mResult = MRFROMSHORT( TRUE );
      }
      break;

    case WM_EQF_INITMENU:
    case WM_INITMENU:
      UtlMenuDisableAll( hwnd, SHORT1FROMMP1( mp1) );
      sItem = QUERYSELECTIONHWND( pCommArea->hwndLB );
      if ( sItem  != LIT_NONE )
      {
        PSZ   pszTemp;
        PSZ   pszPlugin;

        SendMessage( pCommArea->hwndLB, LM_EQF_QUERYITEMTEXT, (WPARAM)sItem, (LPARAM)pCommArea->szBuffer );
        pszTemp = UtlParseX15( pCommArea->szBuffer, TAG_NAME_IND);
        pszPlugin = UtlParseX15( pCommArea->szBuffer, TAG_PLUGIN_IND);

        UtlMenuEnableItem( PID_FILE_MI_IMPORT );
        if ( isMarkupExportable( pszTemp, pszPlugin ) ) UtlMenuEnableItem( PID_FILE_MI_EXPORT  );
        if ( isMarkupDeletable( pszTemp, pszPlugin ) ) UtlMenuEnableItem( PID_FILE_MI_DELETE );

        UtlMenuEnableItem( PID_FILE_MI_PROPERTIES );
      }
      else
      {
        UtlMenuEnableItem( PID_FILE_MI_IMPORT );
      } /* endif */
      UtlMenuEnableItem( PID_FILE_MI_NEW );
      UtlMenuEnableItem( PID_FILE_MI_PRINTLIST );

      UtlMenuEnableItem( PID_VIEW_MI_NAMES );
      UtlMenuEnableItem( PID_VIEW_MI_DETAILSDLG );
      UtlMenuEnableItem( PID_VIEW_MI_DETAILS );
      UtlMenuEnableItem( PID_VIEW_MI_SORT );
      UtlMenuEnableItem( PID_VIEW_MI_SOME );
      UtlMenuEnableItem( PID_VIEW_MI_ALL );
      break;

    case WM_EQF_TOOLBAR_ENABLED:
      switch ( mp1 )
      {
        /**************************************************************/
        /* check for items to be enabled ..                           */
        /**************************************************************/
        case PID_FILE_MI_PRINTLIST:
        case PID_FILE_MI_NEW:
        case PID_VIEW_MI_NAMES:
        case PID_VIEW_MI_DETAILSDLG:
        case PID_VIEW_MI_DETAILS:
        case PID_VIEW_MI_SORT:
        case PID_VIEW_MI_SOME:
        case PID_VIEW_MI_ALL:
          mResult = MRFROMSHORT(TRUE);
          break;

        case PID_FILE_MI_IMPORT:
        case PID_FILE_MI_DELETE:
        case PID_FILE_MI_EXPORT:
        case PID_FILE_MI_PROPERTIES:
          {
            SHORT sItem = QUERYSELECTIONHWND( pCommArea->hwndLB );
            sItem = QUERYSELECTIONHWND( pCommArea->hwndLB );
            if ( sItem  != LIT_NONE )
            {
              PSZ   pszTemp;
              PSZ   pszPlugin;
              SendMessage( pCommArea->hwndLB, LM_EQF_QUERYITEMTEXT, (WPARAM)sItem, (LPARAM)pCommArea->szBuffer );
              pszTemp = UtlParseX15( pCommArea->szBuffer, TAG_NAME_IND);
              pszPlugin = UtlParseX15( pCommArea->szBuffer, TAG_PLUGIN_IND);

              switch ( mp1 )
              {
                case PID_FILE_MI_DELETE:     mResult = MRFROMSHORT( isMarkupDeletable( pszTemp, pszPlugin ) ); break;
                case PID_FILE_MI_EXPORT:     mResult = MRFROMSHORT( isMarkupExportable( pszTemp, pszPlugin ) ); break;
                case PID_FILE_MI_IMPORT:     mResult = MRFROMSHORT( TRUE ); break;

                case PID_FILE_MI_PROPERTIES: mResult = MRFROMSHORT( TRUE ); break;
              }
            }
            else if ( mp1 == PID_FILE_MI_IMPORT ) 
            {
              mResult = MRFROMSHORT( TRUE );
            }
            else
            {
              mResult = MRFROMSHORT( FALSE );
            }
          }
          break;
      } /* endswitch */
      break;

    case WM_EQF_COMMAND:
    case WM_COMMAND:
      mResult = MRFROMSHORT( TRUE ); // default return code for COMMAND msgs
      switch ( SHORT1FROMMP1(mp1) )
      {
        case PID_FILE_MI_IMPORT:
          {
            SHORT sItem = QUERYSELECTIONHWND( pCommArea->hwndLB );
            if ( sItem != LIT_NONE )
            {
              PSZ   pszTemp;

               SendMessage( pCommArea->hwndLB, LM_EQF_QUERYITEMTEXT, (WPARAM)sItem, (LPARAM)pCommArea->szBuffer );
               pszTemp = UtlParseX15( pCommArea->szBuffer, TAG_NAME_IND);
               TagTableImport( hwnd, pszTemp );
            }
            else
            {
              TagTableImport(hwnd, EMPTY_STRING );
            } /* endif */
          }
          break;

        case PID_FILE_MI_EXPORT:
          {
            SHORT sItem = QUERYSELECTIONHWND( pCommArea->hwndLB );
            if ( sItem != LIT_NONE )
            {
               PSZ   pszTemp;
               PSZ   pszPlugin;
               // Win2000 corrupts our data if we use LB_GETTEXT so use own message instead
//             QUERYITEMTEXTHWND( pCommArea->hwndLB, sItem, pCommArea->szBuffer );
               SendMessage( pCommArea->hwndLB, LM_EQF_QUERYITEMTEXT, (WPARAM)sItem, (LPARAM)pCommArea->szBuffer );
               pszTemp = UtlParseX15( pCommArea->szBuffer, TAG_NAME_IND);
               pszPlugin = UtlParseX15( pCommArea->szBuffer, TAG_PLUGIN_IND);
               if ( isMarkupExportable( pszTemp, pszPlugin ) ) {
                 TagTableExport( hwnd, pszTemp );
               }
            }
            else
            {
              UtlError( ERROR_NO_TABLE_SELECTED, MB_CANCEL, 0, NULL, EQF_WARNING);
            } /* endif */
          }
          break;

        case PID_FILE_MI_DELETE:
          {
            PSZ   pszTemp;
            PSZ   pszPlugin ;
            SHORT sItem = QUERYSELECTIONHWND( pCommArea->hwndLB );

            if ( sItem != LIT_NONE )
            {
               USHORT usMBCode;

               SendMessage( pCommArea->hwndLB, LM_EQF_QUERYITEMTEXT, (WPARAM)sItem, (LPARAM)pCommArea->szBuffer );
               pszTemp = UtlParseX15( pCommArea->szBuffer, TAG_NAME_IND );
               pszPlugin = UtlParseX15( pCommArea->szBuffer, TAG_PLUGIN_IND );
               if ( isMarkupDeletable( pszTemp, pszPlugin ) ) {
                 usMBCode = UtlError( WARNING_DELETE_TAGTABLE, MB_YESNO | MB_DEFBUTTON2, 1, &pszTemp, EQF_QUERY );
                 DESELECTITEMHWND( pCommArea->hwndLB, sItem );
                 if ( usMBCode == MBID_YES ) {
                    EqfSend2Handler( TAGTABLEHANDLER, WM_EQF_DELETE, NULL, MP2FROMP(pCommArea) );
                 } /* endif */
               }
            } /* endif */
          }
          break;

        case PID_FILE_MI_PRINTLIST:
          // pass message to column listbox control
          WinSendMsg( pCommArea->hwndLB, msg, mp1, mp2 );
          break;

        case PID_FILE_MI_PROPERTIES:

          {
            SHORT sItem = QUERYSELECTIONHWND( pCommArea->hwndLB );

            if ( sItem != LIT_NONE )
            {
              SendMessage( pCommArea->hwndLB, LM_EQF_QUERYITEMTEXT, (WPARAM)sItem, (LPARAM)pCommArea->szBuffer );

              // send request for markup table properties dialog to our window
              // as the C++ code for the properties dialog is located in EQFD.EXE it
              // cannot directly be called from here
              SendMessage( hwnd, WM_EQF_TAGTABLEPROPS, 0, MP2FROMP(UtlParseX15( pCommArea->szBuffer, 0) ) );

            } /* endif */
          }
          break;

        case PID_FILE_MI_NEW:
          {
              // send request for markup table properties dialog to our window
              // as the C++ code for the properties dialog is located in EQFD.EXE it
              // cannot directly be called from here
            SendMessage( hwnd, WM_EQF_TAGTABLEPROPS, 0, 0 );
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
} /* end of function TagListCallBack */

// GQ: moved function TagFillLB to MarkupPluginMapper.CPP

// GQ: moved function TagMakeListItem to MarkupPluginMapper.CPP



// load an external list containing markup table names into memory
// the list in memory consists of null terminated markup table names
// and is terminated with an additional null character
BOOL TALoadExternalMarkupTableList
(
  PSZ         pszListName,             // name of list file (with fully qualified path )
  PSZ         *ppListArea,             // caller's list area pointer
  BOOL        fMsg                     // TRUE = show error messages
)
{
  BOOL fOK = TRUE;
  PSZ  pszMarkupList = NULL;
  ULONG ulLength = 0;

  // load list with supported markups into memory
  fOK = UtlLoadFileL( pszListName, (PVOID *)&pszMarkupList, &ulLength, FALSE, fMsg );

  // get buffer for preprocessed list
  if ( fOK )
  {
    fOK = UtlAlloc( (PVOID *)ppListArea, 0L, ulLength + 10, ERROR_STORAGE );
  } /* endif */

  // preprocess list
  if ( fOK )
  {
    PSZ pszSource = pszMarkupList;
    PSZ pszTarget = *ppListArea;

    if ( ulLength && (pszSource[ulLength-1] == EOFCHAR) )
    {
      ulLength--;                      // remove EOF character
    } /* endif */

    while ( ulLength )
    {
      // skip leading whitespace
      while ( ulLength && ((*pszSource == LF) || (*pszSource == CR) || (*pszSource == SPACE) ) )
      {
        ulLength--;
        pszSource++;
      } /* endwhile */

      if ( *pszSource == '*' )
      {
        // skip comment line
        while ( ulLength && (*pszSource != LF) && (*pszSource != CR) )
        {
          ulLength--;
          pszSource++;
        } /* endwhile */
      }
      else
      {
        // copy markup name to buffer up to first whitespace character
        while ( ulLength && (*pszSource != LF) && (*pszSource != CR) && (*pszSource != SPACE) )
        {
          ulLength--;
          *pszTarget++ = *pszSource++;
        } /* endwhile */
        *pszTarget++ = EOS;
      } /* endif */
    } /* endwhile */

    // terminate markup table list
    *pszTarget++ = EOS;
    *pszTarget++ = EOS;
  } /* endif */

  // cleanup
  if ( pszMarkupList ) UtlAlloc( (PVOID *)&pszMarkupList, 0L, 0L, NOMSG );

  return( fOK );
} /* end of fuction TALoadExternalMarkupTableList */

//   End of EQFTAG00.C
//
