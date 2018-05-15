//+----------------------------------------------------------------------------+
//|  EQFDIC00.C - EQF Dictionary Handler                                       |
//+----------------------------------------------------------------------------+
//|  Copyright Notice:                                                         |
//|                                                                            |
//|      Copyright (C) 1990-2015, International Business Machines              |
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
//|                                                                            |
//+----------------------------------------------------------------------------+
#define INCL_EQF_TM               // general Transl. Memory functions
#define INCL_EQF_TMREMOTE         // remote Transl. Memory functions (U-Code)
#define INCL_EQF_ASD              // dictionary access functions (Asd...)
#define INCL_EQF_LDB              // dictionary data encoding functions
#define INCL_EQF_DICT             // dictionary handler functions
#define INCL_EQF_DICTPRINT        // dictionary print functions
#define INCL_EQF_DLGUTILS         // dialog utilities
#include <eqf.h>                  // General Translation Manager include file

#include <eqfdtag.h>              // include tag definitions
#include "eqfdde.h"
#include "eqfdicti.h"             // Dictionary Handler private defines
#include "OtmDictionaryIF.H"
#include "eqfdic00.id"            // Dictionary Handler window IDs
#include "eqfcolw.id"             // column width IDs
#include "EQFFUNCI.H"

static CHAR ColHdr[7][30];             // Buffer for column header texts
static CLBCOLDATA ColTable[] =
{ { "", 1, TEXT_DATA, DT_LEFT },
  { ColHdr[1], CLB_MAX_FNAME, AUTOWIDTHTEXT_DATA, DT_LEFT },
  { ColHdr[2], CLB_MAX_DESCRIPTION, TEXT_DATA, DT_LEFT },
  { ColHdr[3], CLB_MAX_DRIVE, TEXT_DATA, DT_LEFT },
  { ColHdr[4], CLB_MAX_SERVER_NAME, TEXT_DATA, DT_LEFT },
  { ColHdr[5], CLB_MAX_OWNER_LENGTH, TEXT_DATA, DT_LEFT },
  { ColHdr[6], CLB_MAX_LANG_LENGTH, TEXT_DATA, DT_LEFT },
  { NULL,      0,                   (CLBDATATYPE) 0,         0      } };

static SHORT sLastUsedView[] = { DIC_NAME_IND, CLBLISTEND };
static SHORT sDefaultView[]  = { DIC_NAME_IND, DIC_DESCR_IND, CLBLISTEND };
static SHORT sNameView[]     = { DIC_NAME_IND, CLBLISTEND };
static SHORT sDetailsView[]  = { DIC_NAME_IND, DIC_DESCR_IND, CLBLISTEND };
static SHORT sSortCriteria[] = { DIC_NAME_IND, CLBLISTEND };

static CLBCTLDATA DicCLBData =
{  sizeof(CLBCTLDATA),                 // size of control structure
   7,                                  // we have 5 data columns
   1,                                  // two character space between columns
   SYSCLR_WINDOWSTATICTEXT,            // paint title in color of static text
   SYSCLR_WINDOW,                      // background is normal window background
   SYSCLR_WINDOWTEXT,                  // paint item in color of window text
   SYSCLR_WINDOW,                      // background is normal window background
   X15,                                // use X15 character as data seperator
   sLastUsedView,                      // set current (= last used) view list
   sDefaultView,                       // set default view list
   sDetailsView,                       // set user set details view list
   sNameView,                          // set view list for 'name' view option
   sSortCriteria,                      // set sort criteria list
   ColTable };                         // set address of column definition table

MRESULT DictListCallBack( PLISTCOMMAREA, HWND, WINMSG, WPARAM, LPARAM );
BOOL ProcessDicListCommand( HWND, PLISTCOMMAREA, SHORT );
VOID DictionaryDelete( HWND hwnd, PLISTCOMMAREA pCommArea, SHORT sItem, PSZ pszName, PSZ pszObjName, PUSHORT );


/**********************************************************************/
/* Handler callback function for dictionary list handler              */
/**********************************************************************/
MRESULT DictListHandlerCallBack
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
      pCommArea->pfnCallBack          = DictListCallBack;
      strcpy( pCommArea->szHandlerName, DICTIONARYHANDLER );
      pCommArea->sBaseClass           = clsDICTIONARY;
      pCommArea->sListWindowID        = ID_DICTIONARY_WINDOW;
      pCommArea->sListboxID           = PID_DICTIONARY_LB;

      /****************************************************************/
      /* Define object classes to be notified for EQFN messages       */
      /****************************************************************/
      pCommArea->asNotifyClassList[0] = clsDICTIONARY;
      pCommArea->asNotifyClassList[1] = clsDICTORG;
      pCommArea->asNotifyClassList[2] = clsDICTEXP;
      pCommArea->asNotifyClassList[3] = clsDICTIMP;
      pCommArea->asNotifyClassList[4] = 0;       // end of list

      /****************************************************************/
      /* Define additional messages processed by the callback function*/
      /****************************************************************/
      pCommArea->asMsgsWanted[0]      = WM_EQF_DDE_REQUEST;
      pCommArea->asMsgsWanted[1]      = WM_EQF_NEXTSTEP;
      pCommArea->asMsgsWanted[2]      = 0;       // end of list
      break;

    case WM_EQF_OPEN:
      {
        HWND       hwndObj;
        PSZ        pszObj = (PSZ) PVOIDFROMMP2(mp2);

        hwndObj = EqfQueryObject( pszObj, clsDICTIONARY, 0);
        if( hwndObj )
        {
          SETFOCUSHWND( hwndObj );
          mResult = MRFROMSHORT( TRUE );
        }
        else
        {
          mResult = WinSendMsg( hwnd, WM_EQF_CREATELISTWINDOW, mp1, mp2 );
        } /* endif */
      }
      break;

    case WM_EQF_CREATE:
      mResult = MRFROMSHORT( TRUE );
      break;

    case WM_EQF_INSERTNAMES:
      mResult = (MRESULT)EqfSend2AllObjects( clsDICTIONARY, msg, mp1, mp2);
      break;

    case WM_DESTROY:
      /****************************************************************/
      /* Nothing to do, as nothing has been allocated by the dict     */
      /* handler callback function                                    */
      /****************************************************************/
      break;

    case WM_EQF_PROCESSTASK:
      switch ( SHORT1FROMMP1(mp1) )
      {
        case BATCHIMPORT_TASK:
        case IMPORT_TASK:
          WinPostMsg( EqfQueryObject(NULL, clsDICTIONARY, OBJ_ACTIVE),
                      WM_EQF_PROCESSTASK, mp1, mp2 );
          mResult = (MRESULT)TRUE;
          break;
        case RENAMEOBJECT_TASK:
          mResult = (MRESULT)DicRenameDict( (PSZ)PVOIDFROMMP2(mp2), TRUE );
          break;
       } /* endswitch */
       break;

     case  WM_EQF_DDE_REQUEST:
       /************************************************************/
       /*     mp1:  (DDETASK) Task                                 */
       /*     mp2:  (PVOID) pTaskIda                               */
       /************************************************************/
       switch ( SHORT1FROMMP1( mp1 ) )
       {
         case  TASK_DICIMP:
         case  TASK_DICEXP:
           WinPostMsg( hwnd, WM_EQF_NEXTSTEP, mp1, mp2 );
           break;
         default :
           break;
       } /* endswitch */
       break;

     /************************************************************/
     /*  this message handles the different Batch mode requests  */
     /*    mp1:  task to be accomplished                         */
     /*    mp2: pointer to structure                             */
     /************************************************************/
     case WM_EQF_NEXTSTEP:
       switch ( SHORT1FROMMP1( mp1 ) )
       {
         case  TASK_DICIMP:
           //call the function to be processed ..
           DicBatchImp( hwnd, PVOIDFROMMP2(mp2) );
           break;
         case  TASK_DICEXP:
           //call the function to be processed ..
           DicBatchExp( hwnd, PVOIDFROMMP2(mp2) );
           break;
         case  TASK_END:
           {
             PDICIMPEXP pDicImpExp = (PDICIMPEXP)PVOIDFROMMP2(mp2);
             //either set error message or return to caller....
             WinPostMsg( pDicImpExp->hwndOwner, WM_EQF_DDE_ANSWER,
                         NULL, MP2FROMP( &pDicImpExp->DDEReturn ) );
           }
           break;
         default :
           break;
       } /* endswitch */
       break;

     case WM_EQF_DELETE:
       {
         PSZ       pszParm;            // parameter for error messages
         PDICTDEL  pDictDel = (PDICTDEL) PVOIDFROMMP2(mp2);
         USHORT    usRC;               // function return code
         CHAR      szDictName[MAX_FNAME]; // buffer for dictionary name
         OBJNAME   szObjName;          // buffer for dictionary object name

         /*************************************************************/
         /* Something to remember:                                    */
         /*                                                           */
         /* The dictionary name in the DICDEL structure is the        */
         /* path name of the data file (e.g X:\EQF\DICT\YYY.ASD)      */
         /*                                                           */
         /* For the Asd functions we need the fully qualified name    */
         /* of the dict. property file (e.g X:\EQF\PROPERTY\YYY.PRO)  */
         /*                                                           */
         /* For symbols (locking) and notification message we need    */
         /* the dictionary object name which is the name of the       */
         /* property file without the property directory              */
         /* (e.g. X:\EQF\YYY.PRO)                                     */
         /*************************************************************/

         /*************************************************************/
         /* Isolate dictionary name, build dictionary object name and */
         /* name of dictionary property file                          */
         /*************************************************************/
         Utlstrccpy( szDictName, UtlGetFnameFromPath( pDictDel->pszName ), DOT );

         UtlMakeEQFPath( pCommArea->szBuffer, NULC, PROPERTY_PATH, (PSZ) NULP );
         strcat( pCommArea->szBuffer, BACKSLASH_STR );
         strcat( pCommArea->szBuffer, szDictName );
         strcat( pCommArea->szBuffer, EXT_OF_DICTPROP );

         UtlMakeEQFPath( szObjName, NULC, SYSTEM_PATH,(PSZ) NULP );
         strcat( szObjName, BACKSLASH_STR );
         strcat( szObjName, szDictName );
         strcat( szObjName, EXT_OF_DICTPROP );



         // delete the dictionary
         if( EqfSend2AllHandlers( WM_EQF_ABOUTTODELETE,
             MP1FROMSHORT( clsDICTIONARY ), MP2FROMP(szObjName)  ))
         {
            //check if locked locally
            pszParm = szDictName;
            UtlError( ERROR_DICT_NOT_DELETED, MB_CANCEL, 1, &pszParm, EQF_INFO);
            mResult = MRFROMSHORT(TRUE);
         }
         else
         {
            switch ( pDictDel->usType )
            {
               case LOCAL_DEL: //local delete - 3 files
                 SETCURSOR( SPTR_WAIT );
                 usRC = AsdDelete( pCommArea->szBuffer );
                 SETCURSOR( SPTR_ARROW );
                 break;
               case REM_DEL:   //remote delete - 4 files
                 SETCURSOR( SPTR_WAIT );
                 usRC = AsdDeleteRemote( pCommArea->szBuffer );
                 SETCURSOR( SPTR_ARROW );
                 if ( usRC != LX_RC_OK_ASD )
                 {
                   if ( usRC == LX_PROTECTED_ASD ) //dictionary_locked_flag
                   {
                     //issue warning that dictionary is locked and cannot
                     //be deleted
                     pszParm = szDictName;
                     UtlError( ERROR_DICT_NOT_DELETED, MB_CANCEL,
                               1, &pszParm, EQF_INFO );
                     mResult = MRFROMSHORT(TRUE);
                   } /* endif */
                 } /* endif */
                 break;
               case PROPS :
                 //delete only local copy of property file
                 UtlDelete( pCommArea->szBuffer, 0L, TRUE );
                 break;
            } /* endswitch */

            //send message to remove from dictionary list window
            EqfSend2AllHandlers( WM_EQFN_DELETED,
                                 MP1FROMSHORT( clsDICTIONARY ),
                                 MP2FROMP( szObjName ) );
         } /* endif */
       }
       break;

    default:
      break;
  } /* endswitch */
  return( mResult );
} /* end of function DictListHandlerCallBack */

/**********************************************************************/
/* List instance callback function for dictionary list window         */
/**********************************************************************/
MRESULT DictListCallBack
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
        BOOL       fOK = TRUE;         // initialisation is O.K. flag
        EQFINFO    ErrorInfo;          // error info of property handler
        HPROP      hProp = NULL;       // dict. list properties handle
        PPROPDICTLIST   pProp = NULL;  // ptr to dictlist properties
        PSZ        pszParm;            // ptr to error message parameters
		HMODULE hResMod;
		hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);

        /**************************************************************/
        /* Allocate callback function IDA                             */
        /**************************************************************/
        fOK = UtlAlloc( (PVOID *) &(pCommArea->pUserIDA), 0L,
                        (LONG)sizeof(DICTIONARYIDA), ERROR_STORAGE );

        /**************************************************************/
        /* Open dictionary list properties                            */
        /**************************************************************/
        if ( fOK )
        {
          hProp = OpenProperties( pCommArea->szObjName, NULL,
                                  PROP_ACCESS_READ, &ErrorInfo);
          if( hProp )
          {
            pProp = (PPROPDICTLIST)MakePropPtrFromHnd( hProp );
          }
          else
          {
            pszParm = pCommArea->szObjName;
            UtlError( ERROR_OPEN_PROPERTIES, MB_CANCEL, 1, &pszParm, EQF_ERROR );
            fOK = FALSE;
          } /* endif */
        } /* endif */

        /**************************************************************/
        /* Load column listbox title strings                          */
        /**************************************************************/
        if ( fOK  )
        {
          LOADSTRING( NULLHANDLE, hResMod, IDS_DICT_NAME_COLTITLE, ColHdr[1]);
          UtlLoadWidth( NULLHANDLE, hResMod, IDS_DICT_NAME_COLWIDTH,
                        &(ColTable[1].usWidth) );
          LOADSTRING( NULLHANDLE, hResMod, IDS_DICT_DESCR_COLTITLE, ColHdr[2]);
          UtlLoadWidth( NULLHANDLE, hResMod, IDS_DICT_DESCR_COLWIDTH,
                        &(ColTable[2].usWidth) );
          LOADSTRING( NULLHANDLE, hResMod, IDS_DICT_DRIVE_COLTITLE, ColHdr[3]);
          UtlLoadWidth( NULLHANDLE, hResMod, IDS_DICT_DRIVE_COLWIDTH,
                        &(ColTable[3].usWidth) );
          LOADSTRING( NULLHANDLE, hResMod, IDS_DICT_OWNER_COLTITLE, ColHdr[5]);
          UtlLoadWidth( NULLHANDLE, hResMod, IDS_DICT_OWNER_COLWIDTH,
                        &(ColTable[5].usWidth) );
          /************************************************************/
          /* Suppress server column under Windows                     */
          /************************************************************/
          LOADSTRING( NULLHANDLE, hResMod, IDS_DICT_SOURCELANG_COLTITLE, ColHdr[6]);
          UtlLoadWidth( NULLHANDLE, hResMod, IDS_DICT_SOURCELANG_COLWIDTH,
                        &(ColTable[6].usWidth) );
        } /* endif */

        /**************************************************************/
        /* Set column listbox view lists                              */
        /**************************************************************/
        if ( fOK  )
        {
            int i;
            memcpy( pCommArea->asCurView,
                    (pProp->sLastUsedViewList[0] != 0) ? pProp->sLastUsedViewList :
                    sLastUsedView,
                    sizeof(pCommArea->asCurView) );
            DicCLBData.psLastUsedViewList = pCommArea->asCurView;

            memcpy( pCommArea->asDetailsView,
                    (pProp->sDetailsViewList[0] != 0) ? pProp->sDetailsViewList :
                    sDetailsView,
                    sizeof(pCommArea->asDetailsView) );
            DicCLBData.psDetailsViewList = pCommArea->asDetailsView;

            memcpy( pCommArea->asSortList,
                    (pProp->sSortList[0] != 0) ? pProp->sSortList :
                    sSortCriteria,
                    sizeof(pCommArea->asSortList) );
            DicCLBData.psSortList = pCommArea->asSortList;

            memcpy( &(pCommArea->Filter), &(pProp->Filter),
                    sizeof(pCommArea->Filter) );
            DicCLBData.pFilter    = &(pCommArea->Filter);

            DicCLBData.psSortList = pCommArea->asSortList;

            for (i=0;pCommArea->asCurView[i]>0;i++)
            {
                int index = pCommArea->asCurView[i];
                if (pProp->sLastUsedViewWidth[i] > 0)
                {
                    ColTable[index].usWidth = pProp->sLastUsedViewWidth[i];
                } else
                {
                    pProp->sLastUsedViewWidth[i] = ColTable[index].usWidth;
                }
            } // end for
            memcpy( pCommArea->asCurViewWidth, (pProp->sLastUsedViewWidth),
                    sizeof(pCommArea->asCurViewWidth) );

        } /* endif */

        /****************************************************************/
        /* supply all information required to create a dictionary list  */
        /****************************************************************/
        if ( fOK )
        {
          pCommArea->sListObjClass  = clsDICTIONARY;
          LOADSTRING( NULLHANDLE, hResMod, SID_DICT_TITLE, pCommArea->szTitle );
          pCommArea->hIcon          = (HPOINTER) UtlQueryULong(QL_DICTLISTICON); //hiconDICTLIST;
          pCommArea->fNoClose       = TRUE;
          pCommArea->sObjNameIndex  = DIC_OBJECT_IND;
          pCommArea->sNameIndex     = DIC_NAME_IND;
          pCommArea->sListWindowID  = ID_DICTIONARY_WINDOW;
          pCommArea->sListboxID     = PID_DICTIONARY_LB;
          pCommArea->sPopupMenuID   = ID_DIC_POPUP;
          pCommArea->sGreyedPopupMenuID   = ID_DIC_POPUP;
          pCommArea->sNoSelPopupMenuID = ID_DIC_POPUP_NOSEL;
          pCommArea->pColData       = &DicCLBData;
          pCommArea->fMultipleSel   = TRUE;
          pCommArea->sMultPopupMenuID = ID_DIC_POPUP_MULTSEL;
          pCommArea->sDefaultAction = PID_FILE_MI_OPEN;
          memcpy( &(pCommArea->swpSizePos), &(pProp->Swp), sizeof(EQF_SWP) );
          pCommArea->sItemClass     = clsDICTIONARY;
          pCommArea->sItemPropClass = PROP_CLASS_DICTIONARY;
          pCommArea->asMsgsWanted[0] = WM_EQF_WD_MAIN_NOTIFY;
          pCommArea->asMsgsWanted[1] = WM_EQF_PROCESSTASK;
          pCommArea->asMsgsWanted[2] = 0;        // ends list of messages
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
      /* Save view lists for WM_EQF_TERMINATE only if save flag is  */
      /* on                                                         */
      /**************************************************************/
      if ( (msg == WM_CLOSE) || (SHORT1FROMMP1(mp1) & TWBSAVE) )
      {
        PPROPDICTLIST pProp;
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
          /* Save current view lists and window position              */
          /************************************************************/
          if( SetPropAccess( hProp, PROP_ACCESS_WRITE ) )
          {
            pProp =(PPROPDICTLIST) MakePropPtrFromHnd( hProp);
            memcpy( &pProp->Swp, &(pCommArea->swpSizePos), sizeof(EQF_SWP) );
            memcpy( pProp->sLastUsedViewList, pCommArea->asCurView,
                    sizeof(pProp->sLastUsedViewList) );

            memcpy( (pProp->sLastUsedViewWidth), (pCommArea->asCurViewWidth),
                     sizeof( pProp->sLastUsedViewWidth ));

            memcpy( pProp->sDetailsViewList, pCommArea->asDetailsView,
                    sizeof(pProp->sDetailsViewList) );
            memcpy( pProp->sSortList, pCommArea->asSortList,
                    sizeof(pProp->sSortList) );
            memcpy( &(pProp->Filter), &(pCommArea->Filter),
                    sizeof(pProp->Filter) );
            SaveProperties( hProp, &ErrorInfo );
          } /* endif */
          CloseProperties( hProp, PROP_QUIT, &ErrorInfo );
        } /* endif */


      } /* endif */

      /**************************************************************/
      /* Close all open dictionaries (only for WM_EQF_TERMINATE msg)*/
      /**************************************************************/
      if ( msg == WM_EQF_TERMINATE )
      {
        USHORT          usI;         // loop index
        PDICTIONARYIDA  pIda = (PDICTIONARYIDA)pCommArea->pUserIDA;

        for ( usI = 0; usI < pIda->usDictNum; usI++ )
        {
           LupEnd( pIda->hLookup[usI] );
           AsdClose( pIda->hUser[usI], pIda->hDict[usI] );
           AsdEnd( pIda->hUser[usI] );
        } /* endfor */

      } /* endif */
      break;

    case WM_DESTROY:
      /****************************************************************/
      /* Free all resource allocated by list instance callback        */
      /* function                                                     */
      /****************************************************************/
      if ( pCommArea->pUserIDA != NULL )
      {
        UtlAlloc( (PVOID *) &(pCommArea->pUserIDA), 0L, 0L, NOMSG );
      } /* endif */
      break;

    case WM_EQF_INITIALIZE:
      /****************************************************************/
      /* Fill column listbox                                          */
      /****************************************************************/
      DicFillDictLB( hwnd, pCommArea->hwndLB, pCommArea->szBuffer, TRUE );
      break;

    case WM_EQF_BUILDITEMTEXT :
      /****************************************************************/
      /* Setup item text for the object passed in mp2 parameter       */
      /****************************************************************/
      {
        PSZ        pszObj = (PSZ) PVOIDFROMMP2(mp2);   // ptr to object name
        HPROP      hDicProp;                     // dictionary properties handle
        EQFINFO    ErrorInfo;                    // return code from prop. handler

        /*************************************************************/
        /* Access dictionary properties                              */
        /*************************************************************/
        hDicProp = OpenProperties( pszObj, NULL, PROP_ACCESS_READ, &ErrorInfo);

        /**************************************************************/
        /* Build item string                                          */
        /**************************************************************/
        if ( hDicProp )
        {
          DicMakeDictListItem( hwnd, pszObj, hDicProp, pCommArea->szBuffer );
          CloseProperties( hDicProp, PROP_QUIT, &ErrorInfo );
          mResult = MRFROMSHORT( TRUE );
        } /* endif */
      }
      break;

    case WM_EQFN_DRIVEREMOVED:
    case WM_EQFN_DRIVEADDED:
      /****************************************************************/
      /* Refresh column listbox                                       */
      /****************************************************************/
      DicFillDictLB( hwnd, pCommArea->hwndLB, pCommArea->szBuffer, TRUE );
      break;

    case WM_EQF_INITMENU:
    case WM_INITMENU:
      {
        SHORT         sItem;                // selected listbox item
        BOOL          fOK = TRUE;           // internal O.K. flag

        UtlMenuDisableAll( hwnd, SHORT1FROMMP1( mp1) );
        UtlMenuEnableItem( PID_FILE_MI_NEW );
        UtlMenuEnableItem( PID_VIEW_MI_NAMES );
        UtlMenuEnableItem( PID_VIEW_MI_DETAILSDLG );
        UtlMenuEnableItem( PID_VIEW_MI_DETAILS );
        UtlMenuEnableItem( PID_UTILS_MI_CONNECT );
        UtlMenuEnableItem( PID_FILE_MI_SELECTALL );
        UtlMenuEnableItem( PID_FILE_MI_DESELECTALL );

        UtlMenuEnableItem( PID_VIEW_MI_SORT );
        UtlMenuEnableItem( PID_VIEW_MI_SOME );
        UtlMenuEnableItem( PID_VIEW_MI_ALL );
        UtlMenuEnableItem( PID_FILE_MI_PRINTLIST );

        sItem = QUERYNEXTSELECTIONHWND( pCommArea->hwndLB, LIT_FIRST );
        if ( sItem != LIT_NONE )
        {
          fOK = (BOOL)WinSendMsg( pCommArea->hwndLB,
                 LM_EQF_QUERYITEMSTATE, MP1FROMSHORT (sItem), NULL );

          //if list box item not greyed out
          if ( fOK )
          {
            UtlMenuEnableItem( PID_FILE_MI_IMPORT );
            UtlMenuEnableItem( PID_FILE_MI_EXPORT );
            UtlMenuEnableItem( PID_FILE_MI_PROPERTIES );
            UtlMenuEnableItem( PID_FILE_MI_HTMLPROPS );
            UtlMenuEnableItem( PID_FILE_MI_OPEN );
            UtlMenuEnableItem( PID_FILE_MI_ORGANIZE );
            UtlMenuEnableItem( PID_FILE_MI_PRINT );
            UtlMenuEnableItem( PID_FILE_MI_RENAME );
          } /* endif */
          UtlMenuEnableItem( PID_FILE_MI_DELETE );
        }
        else
        {
          UtlMenuEnableItem( PID_FILE_MI_IMPORT );
        } /* endif */
      }
      break;

    case WM_EQF_TOOLBAR_ENABLED:
      switch ( mp1 )
      {
        /**************************************************************/
        /* check for items to be enabled ..                           */
        /**************************************************************/
        case PID_FILE_MI_NEW:
        case PID_VIEW_MI_NAMES:
        case PID_VIEW_MI_DETAILSDLG:
        case PID_VIEW_MI_DETAILS:
        case PID_UTILS_MI_CONNECT:
        case PID_FILE_MI_PRINTLIST:
        case PID_FILE_MI_DELETE:
        case PID_VIEW_MI_SORT:
        case PID_VIEW_MI_SOME:
        case PID_VIEW_MI_ALL:
        case PID_FILE_MI_SELECTALL:
        case PID_FILE_MI_DESELECTALL:
          mResult = MRFROMSHORT(TRUE);
          break;

        case PID_FILE_MI_IMPORT:
          {
            SHORT sItem = QUERYNEXTSELECTIONHWND( pCommArea->hwndLB, LIT_FIRST );
            if ( (sItem < 0) ||
                  WinSendMsg( pCommArea->hwndLB, LM_EQF_QUERYITEMSTATE,
                                                 MP1FROMSHORT(sItem), 0L ))
            {
              mResult = MRFROMSHORT(TRUE);
            } /* endif */
          }
          break;

        case PID_FILE_MI_EXPORT:
        case PID_FILE_MI_PROPERTIES:
        case PID_FILE_MI_HTMLPROPS:
        case PID_FILE_MI_OPEN:
        case PID_FILE_MI_ORGANIZE:
        case PID_FILE_MI_PRINT:
        case PID_FILE_MI_RENAME:
          {
            SHORT sItem = QUERYNEXTSELECTIONHWND( pCommArea->hwndLB, LIT_FIRST );
            if ( (sItem != LIT_NONE) &&
                  WinSendMsg( pCommArea->hwndLB, LM_EQF_QUERYITEMSTATE,
                                                 MP1FROMSHORT(sItem), 0L ))
            {
              mResult = MRFROMSHORT(TRUE);
            } /* endif */
          }
          break;
      } /* endswitch */
      break;


    case WM_EQF_COMMAND:
    case WM_COMMAND:
      if ( SHORT1FROMMP1(mp1) == PID_FILE_MI_PRINTLIST )
      {
        // pass message to column listbox control
        WinSendMsg( pCommArea->hwndLB, msg, mp1, mp2 );
      }
      else
      if ( ProcessDicListCommand( hwnd, pCommArea, SHORT1FROMMP1(mp1) ) )
      {
        mResult = MRFROMSHORT( TRUE  ); // command has been processed
      }
      else if( msg == WM_EQF_COMMAND)
      {
        mResult = MRFROMSHORT( FALSE ); // tell twbmain that we rejected
      }
      else
      {
        mResult = MRFROMSHORT( TRUE  ); // command not processed, but no EQF msg
      } /* endif */
      break;

    case WM_EQF_WD_MAIN_NOTIFY:
      {
        USHORT     usI, usJ;           // loop indices
        SWP        swp;                // window position
        EQFINFO    ErrorInfo;          // error return code
        PDICTIONARYIDA pIda =(PDICTIONARYIDA) pCommArea->pUserIDA;

        if ( (SHORT1FROMMP2(mp2) == DLG_HIDDEN) &&
             (SHORT2FROMMP2(mp2) == LOOKUP_DLG) )
        {
          for ( usI = 0; usI < pIda->usDictNum; usI++ )
          {
            if ( pIda->usLUPID[usI] == USHORT1FROMMP1(mp1) )
            {
               LupEnd( pIda->hLookup[usI] );
               AsdClose( pIda->hUser[usI], pIda->hDict[usI] );
               AsdEnd( pIda->hUser[usI] );

               //remove symbol so dict free for further use
               REMOVESYMBOL( pIda->szLookupDictName[usI] );

               //update array to avoid empty array elements
               for ( usJ = usI; usJ < (pIda->usDictNum-1); usJ++ )
               {
                  pIda->hDict[usJ] = pIda->hDict[usJ+1];
                  pIda->hUser[usJ] = pIda->hUser[usJ+1];
                  pIda->hLookup[usJ] = pIda->hLookup[usJ+1];
                  pIda->usLUPID[usJ] = pIda->usLUPID[usJ+1];
                  strcpy( pIda->szLookupDictName[usJ],
                          pIda->szLookupDictName[usJ+1] );
               } /* endfor */
               pIda->usDictNum--;
               break;   //leave for loop
            } /* endif */
          } /* endfor */
        }
        else
        {
          /*-----------------------------------------------------------------------
          * Store the position of DISPLAY dialog.
          *----------------------------------------------------------------------*/
          if ((SHORT2FROMMP2(mp2) == DISP_ENTRY_DLG) &&
              (SHORT1FROMMP2(mp2) == DLG_POSITIONED))
          {
             HPROP          hProp;     // handle of dictionary list properties
             PPROPDICTLIST  pProp;     // ptr to dictlist properties

             if ( !WinQueryWindowPos ((HWND)mp1, &swp) )
               memset (&swp, 0, sizeof (swp));
             hProp = OpenProperties( pCommArea->szObjName,
                                     NULL,
                                     PROP_ACCESS_READ,
                                     &ErrorInfo);
             if( hProp )
             {
                //always save current display window size
                if( SetPropAccess( hProp, PROP_ACCESS_WRITE))
                {
                   pProp =(PPROPDICTLIST) MakePropPtrFromHnd( hProp );
                   RECTL_XLEFT(pProp->rclDisp) = swp.x;
                   RECTL_XRIGHT(pProp->rclDisp) = swp.x + swp.cx;
                   RECTL_YBOTTOM(pProp->rclDisp) = swp.y;
                   RECTL_YTOP(pProp->rclDisp) = swp.y + swp.cy;
                   SaveProperties( hProp, &ErrorInfo);
                   CloseProperties( hProp, PROP_QUIT, &ErrorInfo );
                } /* endif */
             } /* endif */
          } /* endif */
        } /* endif */
      }
      break;

    case WM_EQF_PROCESSTASK:
      switch ( SHORT1FROMMP1(mp1) )
      {
        case IMPORT_TASK:
          DictionaryImport( hwnd, (PSZ)PVOIDFROMMP2(mp2), TRUE, NULLHANDLE );
          break;
        case BATCHIMPORT_TASK:
          {
            PBATCHIMPORTPARMS pBatchImpParms = (PBATCHIMPORTPARMS) PVOIDFROMMP2(mp2);
            DictionaryImport( hwnd, pBatchImpParms->pszObject, TRUE,
                              pBatchImpParms->hwndErrMsg);
            UtlAlloc( (PVOID *)&pBatchImpParms, 0L, 0L, NOMSG );
          }
          break;
       } /* endswitch */
       break;

    default:
      break;
  } /* endswitch */
  return( mResult );
} /* end of function DictListCallBack */

BOOL ProcessDicListCommand( HWND hwnd, PLISTCOMMAREA pCommArea, SHORT sCommand )
{
  BOOL             fProcessed = TRUE;

  switch ( sCommand )
  {
    case PID_FILE_MI_IMPORT:
      {
        SHORT sItem = QUERYSELECTIONHWND( pCommArea->hwndLB );

        if ( (sItem != LIT_NONE ) &&
             CLBQUERYITEMSTATEHWND( pCommArea->hwndLB, sItem ) )
        {
           //if dictionary name is not greyed out
           // Win2000 corrupts our data if we use LB_GETTEXT so use own message instead
           //  QUERYITEMTEXTHWND( pCommArea->hwndLB, sItem, pCommArea->szBuffer );
           SendMessage( pCommArea->hwndLB, LM_EQF_QUERYITEMTEXT, (WPARAM)sItem, (LPARAM)pCommArea->szBuffer );
           DictionaryImport( hwnd,
                             UtlParseX15( pCommArea->szBuffer, DIC_NAME_IND),
                             FALSE, NULLHANDLE );
        }
        else
        {
           DictionaryImport( hwnd, NULL, FALSE, NULLHANDLE );
        } /* endif */
      }
      break;

    case PID_FILE_MI_NEW:
      DictionaryNew( hwnd, NULL );
      break;

    /******************************************************************/
    /* Commands which require check for locked dictionary and allow   */
    /* only single selection                                          */
    /******************************************************************/
    case PID_FILE_MI_PRINT:
    case PID_FILE_MI_OPEN:
    case PID_FILE_MI_PROPERTIES:
    case PID_FILE_MI_HTMLPROPS:
    case PID_FILE_MI_RENAME:
      {
        OBJNAME  szObjName;
        PSZ      pszName;
        SHORT    sRC;
        SHORT    sItem;
        BOOL     fOK = TRUE;

        sItem = QUERYSELECTIONHWND( pCommArea->hwndLB );

        if ( fOK && (sItem != LIT_NONE) )
        {
           // Win2000 corrupts our data if we use LB_GETTEXT so use own message instead
//         QUERYITEMTEXTHWND( pCommArea->hwndLB, sItem, pCommArea->szBuffer );
           SendMessage( pCommArea->hwndLB, LM_EQF_QUERYITEMTEXT, (WPARAM)sItem, (LPARAM)pCommArea->szBuffer );

           strcpy( szObjName, UtlParseX15( pCommArea->szBuffer, DIC_OBJECT_IND) );
           pszName = UtlParseX15( pCommArea->szBuffer, DIC_NAME_IND );

           /***********************************************************/
           /* For PID_FILE_MI_OPEN only: check if dictionary is       */
           /* already open; activate the lookup dialog for this       */
           /* dictionary if found                                     */
           /***********************************************************/
           if ( sCommand == PID_FILE_MI_OPEN )
           {
             USHORT usI;               // loop index
             PDICTIONARYIDA pIda =(PDICTIONARYIDA) pCommArea->pUserIDA;

             for ( usI = 0; usI < pIda->usDictNum; usI++ )
             {
               if ( strcmp( szObjName, pIda->szLookupDictName[usI] )  == 0 )
               {
                 /*****************************************************/
                 /* Dictionary is already open, activate lookup dialog*/
                 /* and leave command processing                      */
                 /*****************************************************/
                 LupActivate( pIda->hLookup[usI] );
                 fProcessed = TRUE;
                 return( fProcessed );
               } /* endif */
             } /* endfor */
           } /* endif */

           sRC = QUERYSYMBOL( szObjName );

           if ( sRC != -1 )
           {
             UtlError( ERROR_DICT_LOCKED, MB_CANCEL, 1, &pszName, EQF_ERROR );
           }
           else
           {
             ANSITOOEM( pszName );
             switch ( sCommand )
             {
               case PID_FILE_MI_PRINT:
                 QDPRDictionaryPrint( pszName );
                 break;

               case PID_FILE_MI_PROPERTIES:
                 SETSYMBOL( szObjName );
                 DictionaryProp( hwnd, pszName );
                 REMOVESYMBOL( szObjName );
                 break;
               case PID_FILE_MI_HTMLPROPS:
                 {
                   BOOL fOk;
                   fOk = EqfDictPropsToHtml( szObjName, hwnd );
                 }
                 break;
               case PID_FILE_MI_RENAME:
                 UtlRenameObjectDlg( szObjName, clsDICTIONARY );
                 break;

               case PID_FILE_MI_OPEN:
                 {
                   PDICTIONARYIDA  pIda =(PDICTIONARYIDA) pCommArea->pUserIDA;
                   DictionaryOpen( hwnd, pszName, pIda );
                 }
                 break;

             } /* endswitch */
           } /* endif */
         } /* endif */
      }
      break;
    /******************************************************************/
    /* Commands which require check for locked dictionary and allow   */
    /* multiple selections                                            */
    /******************************************************************/
    case PID_FILE_MI_EXPORT:
    case PID_FILE_MI_ORGANIZE:
    case PID_FILE_MI_DELETE:
      {
        PSELDICTINFO pSelDicts = NULL;    // list of selected objects
        OBJNAME  szObjName;
        PSZ      pszName;
        SHORT    sRC;
        SHORT    sItem;
        BOOL     fOK = TRUE;

        // get number of selected dictionaries and build list
        {
          SHORT sItem = LIT_FIRST;
          int iSelItems = SendMessage( pCommArea->hwndLB, LB_GETSELCOUNT, 0, 0 );

          if ( iSelItems > 1 )
          {
            // allocate buffer for the selected dictionaries list
            fOK = UtlAlloc( (PVOID *)&pSelDicts, 0, sizeof(SELDICTINFO)*(iSelItems+1),
                            ERROR_STORAGE );

            // fill list of selected dictionaries and check if dictionaries are
            // locked
            if ( fOK )
            {
              PSELDICTINFO pCurPos = pSelDicts;
              sItem = LIT_FIRST;

              while ( fOK && iSelItems )
              {
                sItem = QUERYNEXTSELECTIONHWND( pCommArea->hwndLB, sItem );
                if ( sItem >= 0 )
                {
                  SendMessage( pCommArea->hwndLB, LM_EQF_QUERYITEMTEXT, (WPARAM)sItem, (LPARAM)pCommArea->szBuffer );
                  strcpy( pCurPos->szObjName, UtlParseX15( pCommArea->szBuffer, DIC_OBJECT_IND ) );
                  strcpy( pCurPos->szName, UtlParseX15( pCommArea->szBuffer, DIC_NAME_IND ) );
                  pCurPos->sItem = sItem;

                  // check if dictionary is locked
                  sRC = QUERYSYMBOL( pCurPos->szObjName );
                  if ( sRC != -1 )
                  {
                    PSZ pszParm = pCurPos->szName;
                    UtlError( ERROR_DICT_LOCKED, MB_CANCEL, 1, &pszParm, EQF_ERROR );
                    fOK = FALSE;
                  }
                  pCurPos++;
                } /* endif */
                iSelItems--;
              } /* endwhile */
              // terminate list
              pCurPos->szObjName[0] = EOS;
              pCurPos->szName[0] = EOS;
              pCurPos->sItem = -1;
            } /* endif */
          } /* endif */
        }
        sItem = QUERYSELECTIONHWND( pCommArea->hwndLB );

        if ( fOK && (sItem != LIT_NONE) )
        {
           // Win2000 corrupts our data if we use LB_GETTEXT so use own message instead
           SendMessage( pCommArea->hwndLB, LM_EQF_QUERYITEMTEXT, (WPARAM)sItem, (LPARAM)pCommArea->szBuffer );

           strcpy( szObjName, UtlParseX15( pCommArea->szBuffer, DIC_OBJECT_IND) );
           pszName = UtlParseX15( pCommArea->szBuffer, DIC_NAME_IND );

           sRC = QUERYSYMBOL( szObjName );

           if ( sRC != -1 )
           {
             UtlError( ERROR_DICT_LOCKED, MB_CANCEL, 1, &pszName, EQF_ERROR );
           }
           else
           {
             ANSITOOEM( pszName );
             switch ( sCommand )
             {
               case PID_FILE_MI_EXPORT:
                 DictionaryExport( hwnd, pszName, (PVOID)pSelDicts );
                 pSelDicts = NULL;    // list is now owned by DictionaryExport
                 break;


               case PID_FILE_MI_ORGANIZE:
                 DictionaryOrganize( hwnd, pszName, (PVOID)pSelDicts );
                 pSelDicts = NULL;    // list is now owned by DictionaryOrganize
                 break;


               case PID_FILE_MI_DELETE:
                 {
                   USHORT          usYesToAllMode = 0;
                   PSELDICTINFO pCurPos = pSelDicts;
                   if ( pCurPos ) usYesToAllMode = MB_EQF_YESTOALL;
                   do
                   {
                     DictionaryDelete( hwnd, pCommArea, sItem, pszName, szObjName, &usYesToAllMode );
                     if ( pCurPos )
                     {
                       pCurPos++;
                       strcpy( szObjName, pCurPos->szObjName );
                       pszName = pCurPos->szName;
                       ANSITOOEM( pszName );
                       sItem = SEARCHITEMHWND( pCommArea->hwndLB, szObjName );
                     } /* endif */
                   } while ( pCurPos && (pCurPos->szName[0] != EOS) );
                 }
                 break;
             } /* endswitch */
           } /* endif */
         } /* endif */
        if ( pSelDicts ) UtlAlloc( (PVOID *)&pSelDicts, 0, 0, NOMSG );
      }
      break;

    default:
      fProcessed = FALSE;
  } /* endswitch */
  return( fProcessed );
} /* end of function ProcessDicListCommand */


USHORT DicFillDictLB( HWND hwndDlg, HWND hwndLBox, PSZ pszBuffer,
                      BOOL fCompleteEntry)
{
   HPROP           hProp;              // handle of dictionary properties
   PPROPDICTIONARY pProp;              // pointer to dictionary properties
   EQFINFO         ErrorInfo;          // error code of property handler calls
   FILEFINDBUF     ResultBuf;          // DOS file find struct
   USHORT          usCount;
   USHORT          usRC;               // return value of Utl/Dos calls
   HDIR            hPropHandle;        // DosFind routine handle
   PSZ             pszObjName;         // ptr to buffer for object name
   PSZ             pszObjPath;         // ptr to buffer for object paths
   PSZ             pszSearchPath;      // ptr to buffer for search path
   PPROPSYSTEM     pSysProp;           // ptr to EQF system properties
   SHORT           sItem = 0;          // list box item
   BOOL            fGreyed;
   PSZ             pszName = RESBUFNAME(ResultBuf);

   SETCURSOR( SPTR_WAIT );
   ENABLEUPDATEHWND_FALSE( hwndLBox );
   DELETEALLHWND( hwndLBox );

   pSysProp = (PPROPSYSTEM) MakePropPtrFromHnd( EqfQuerySystemPropHnd());

   //--- allocate buffer for paths ---
   UtlAlloc( (PVOID *) &pszObjName, 0L, (LONG) (3 * MAX_EQF_PATH), ERROR_STORAGE );
   pszSearchPath = pszObjName + MAX_EQF_PATH;
   pszObjPath    = pszSearchPath + MAX_EQF_PATH;

   if ( pszObjName )
   {
      //--- get path to dictionary properties ---
      UtlMakeEQFPath( pszObjPath, NULC,  SYSTEM_PATH, NULL );
      UtlMakeEQFPath( pszSearchPath, NULC, PROPERTY_PATH, NULL );
      sprintf( pszSearchPath + strlen(pszSearchPath), "\\*%s", EXT_OF_DICPROP );

      //--- loop through all dictionary properties ---
      usCount = 1;
      hPropHandle = HDIR_CREATE;
      usRC = UtlFindFirst( pszSearchPath, &hPropHandle, 0,
                           &ResultBuf, sizeof( ResultBuf),
                           &usCount, 0L, 0 );
      usCount = ( usRC ) ? 0 : usCount;
      while( usCount)
      {
        //initialize fGreyed
        fGreyed = FALSE;

        //check properties of dictionary and dictionary file
        hProp = OpenProperties( pszName, pszObjPath,
                                PROP_ACCESS_READ, &ErrorInfo);
        if ( hProp )
        {
           // access properties
           pProp = (PPROPDICTIONARY) MakePropPtrFromHnd( hProp );

           // build dictionary object name
           DICTNAME( pszObjName, NULC, pszName );

           PROPNAME(pszObjName, UtlGetFnameFromPath(pProp->szDictPath));
           DicMakeDictListItem( hwndDlg, pszObjName, hProp, pszBuffer );
           /***********************************************************/
           /* determine if we need complete object name, or only      */
           /* the dictionary name (8 char)                            */
           /***********************************************************/
           if ( fCompleteEntry )
           {
             sItem = INSERTITEMHWND( hwndLBox, pszBuffer );
           }
           else
           {
             PSZ ps = UtlParseX15( pszBuffer, DIC_NAME_IND );
             sItem = INSERTITEMHWND( hwndLBox, ps );
           } /* endif */

           if ( pProp->usLocation == LOC_SHARED )
           {
             /*********************************************************/
             /* For shared dictionaries: check remote property file   */
             /* and data and index dictionary                         */
             /*********************************************************/
             CHAR szRemProp[MAX_EQF_PATH];       // name of remote property file

             Utlstrccpy( szRemProp, pProp->szDictPath, DOT );
             strcat( szRemProp, EXT_OF_SHARED_DICTPROP );

             if ( !UtlFileExist( pProp->szDictPath ) ||
                  !UtlFileExist( pProp->szIndexPath ) ||
                  !UtlFileExist( szRemProp ) )
             {
                fGreyed = TRUE;
                WinSendMsg( hwndLBox, LM_EQF_SETITEMSTATE,
                            MP1FROMSHORT( sItem ), MP2FROMSHORT( FALSE ) );
             } /* endif */
           }
           else if ( pProp->szServer[0] == EOS )
           {
             //if the dict is local check if it drive configured else
             //grey out
              // check dictionary file drive and configured drive
              if ( strchr(pSysProp->szDriveList, pProp->szDictPath[0]) )
              {
                //find asd file
                if ( !UtlFileExist( pProp->szDictPath ) ||
                     !UtlFileExist( pProp->szIndexPath ) )
                {
                  fGreyed = TRUE;
                } /* endif */
              }
              else
              {
                //grey out dictionary as drive not in sys props
                fGreyed = TRUE;
              } /* endif */

              if ( fGreyed )
               //grey out file name in dict list window
               WinSendMsg( hwndLBox, LM_EQF_SETITEMSTATE,
                           MP1FROMSHORT( sItem ), MP2FROMSHORT( FALSE ) );
           }
           else
           {
              fGreyed = TRUE;
              WinSendMsg( hwndLBox, LM_EQF_SETITEMSTATE,
                          MP1FROMSHORT( sItem ), MP2FROMSHORT( FALSE ) );
           } /* endif */
           /***********************************************************/
           /* if we only want the name of a dictionary and it is      */
           /* greyed out, we have to delete it ...                    */
           /***********************************************************/
           if ( fGreyed && !fCompleteEntry )
           {
              DELETEITEMHWND( hwndLBox, sItem );
           } /* endif */
           CloseProperties( hProp, PROP_QUIT, &ErrorInfo );
        } /* endif */
        usRC = UtlFindNext( hPropHandle, &ResultBuf, sizeof( ResultBuf),
               &usCount, FALSE );
        usCount = ( usRC ) ? 0 : usCount;
      } /* endwhile */
   } /* endif */

   if ( pszObjName )
   {
      UtlAlloc( (PVOID *) &pszObjName, 0L, 0L, NOMSG );
   } /* endif */

   SETCURSOR( SPTR_ARROW );

   usCount = QUERYITEMCOUNTHWND( hwndLBox );

   if ( usCount )
   {
     SELECTITEMHWND( hwndLBox, 0 );
   } /* endif */

   ENABLEUPDATEHWND_TRUE( hwndLBox );

   return( usCount );
}

BOOL DicMakeDictListItem
(
   HWND      hwnd,                     // dialog handle
   PSZ       pszObjName,               // dictionary object name
   PVOID     hProp,                    // property handle
   PSZ       pszBuffer                 // buffer to fill with dictionarylist item
)
{
   PPROPDICTIONARY pProp;              // ptr to dictionary properties
   CHAR      chTempName[_MAX_FNAME];   // buffer for dictionary name
   CHAR      chDrive[3];               // buffer for drive letter string
   CHAR      chOwner[MAX_USERID];      // buffer for owner string
   PSZ       pszDescription;

   hwnd;
   pProp =(PPROPDICTIONARY) MakePropPtrFromHnd( hProp );

   //get filename up to extension delimiter
   Utlstrnccpy( chTempName,
                pProp->PropHead.szName,
                sizeof(chTempName) - 1,
                DOT );

   //build drive letter string
   sprintf( chDrive, "%c:", *pProp->szDictPath );

   //build owner string
   if ( (pProp->szUserid[0] != NULC) &&
        ((pProp->usLocation == LOC_SHARED) || (pProp->szServer[0] != NULC)) )
   {
     strcpy( chOwner, pProp->szUserid );
   }
   else
   {
	 HMODULE hResMod;
	 hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
     WinLoadString( WinQueryAnchorBlock(hwnd), hResMod,
                    IDS_DICT_NO_OWNER_STR,
                    sizeof( chOwner ), chOwner);
   } /* endif */

   //build complete item string
   if(pProp->szLongDesc[0] != EOS)
   {
     pszDescription = ( pProp->szLongDesc );
   }
   else
   {
     pszDescription = ( pProp->szDescription );
   }
   OEMTOANSI( pszDescription );
   OEMTOANSI( pProp->szLongName );

   sprintf( pszBuffer,
            "%s%s%s%s%s%s%s%s%s%s%s%s%s%s",
            pszObjName,                // DIC_OBJ_IND
            X15_STR,
            (pProp->szLongName[0] != EOS) ? pProp->szLongName : chTempName,
            X15_STR,
            pszDescription,            // DIC_DESCR_IND
            X15_STR,
            chDrive,                   // DIC_DRIVE_IND
            X15_STR,
            pProp->szServer,           // DIC_SERVER_IND
            X15_STR,
            chOwner,                   // DIC_USERID_IND
            X15_STR,
            pProp->szSourceLang,       // DIC_SOURCELANG_IND
            X15_STR
            );
   ANSITOOEM( pszDescription );
   ANSITOOEM( pProp->szLongName );

   return( TRUE );
}

VOID DictionaryDelete
(
  HWND hwnd,
  PLISTCOMMAREA pCommArea,
  SHORT sItem,
  PSZ pszName,
  PSZ pszObjName,
  PUSHORT pusYesToAllMode
)
{
  PSZ    pszServer;
  USHORT usMBCode;           // buffer for UtlError return value
  BOOL   fOK = TRUE;         // internal O.K. flag
  CHAR   chTempBuf1[MAX_EQF_PATH];     // temp string buffer
  CHAR    szUserID[MAX_USERID];         // buffer for user ID
  HPROP   hDicProp;                     // handle of dict properties
  BOOL    fDelFree = FALSE;             // dict free for open or not flag
  PPROPDICTIONARY pDicProp = NULL;      // ptr to dict props
  DICTDEL         DictDel;              // delete parameter structure
  EQFINFO         ErrorInfo;            // error return code
  USHORT          usRc = NO_ERROR;      // return code
  BOOL            fWait = FALSE;        // TRUE = wait cursor (hourglass) is shown
  USHORT          usUserLevel;
  BOOL fIsNew = FALSE;                  // is-new flag
  CHAR szShortName[MAX_FNAME];

  // get user ID
  if ( UtlGetLANUserID( szUserID, &usUserLevel, FALSE ) != NO_ERROR )
  {
    usUserLevel = 0;
    szUserID[0] = NULC;
  } /* endif */

  ObjLongToShortName( pszName, szShortName, DICT_OBJECT, &fIsNew );
  // convert long name to ANSI for display in error messages
  OEMTOANSI( pszName );

  // check if prop file exists
  UtlMakeEQFPath( chTempBuf1, NULC, PROPERTY_PATH, NULL );
  strcat( chTempBuf1, BACKSLASH_STR );
  strcat( chTempBuf1, szShortName );
  strcat( chTempBuf1, EXT_OF_DICTPROP );

  // if greyed out issue msg that an attempt will be made
  // to delete as much as possible
  if ( !CLBQUERYITEMSTATEHWND( pCommArea->hwndLB, sItem ) )
  {
    // issue msg
    if ( *pusYesToAllMode == MB_EQF_YESTOALL )
    {
      usMBCode = UtlErrorHwnd( ERROR_DEL_GREYEDOUT_DICT,
                               MB_EQF_YESTOALL | MB_DEFBUTTON2,
                               1, &pszName, EQF_QUERY, hwnd );
      if ( usMBCode == MBID_EQF_YESTOALL )
      {
        *pusYesToAllMode = usMBCode = MBID_YES;
      } /* endif */
    }
    else if ( *pusYesToAllMode == MBID_YES )
    {
      usMBCode = MBID_YES;
    }
    else
    {
     usMBCode = UtlErrorHwnd( ERROR_DEL_GREYEDOUT_DICT,
                             MB_YESNO | MB_DEFBUTTON2,
                             1, &pszName, EQF_QUERY, hwnd );
    } /* endif */

    if ( usMBCode == MBID_YES )
    {
      SETCURSOR( SPTR_WAIT );
      fWait = TRUE;

      // delete depending if remote or local
      pszServer = UtlParseX15( pCommArea->szBuffer, DIC_SERVER_IND);
      if ( pszServer[0] != NULC )
        AsdDeleteRemote( chTempBuf1 );
      else
        AsdDelete( chTempBuf1 );

      SETCURSOR( SPTR_ARROW );
      fWait = FALSE;

      // remove from dict list window
      EqfSend2AllHandlers( WM_EQFN_DELETED,
         MP1FROMSHORT( clsDICTIONARY ), MP2FROMP(pszObjName) );
    } /* endif */

    fOK = FALSE;                       // prevent further processing
  } /* endif */

  if ( fOK )
  {
    SETCURSOR( SPTR_WAIT );
    fWait = TRUE;

    if ( !UtlFileExist( chTempBuf1 ) )
    {
      fOK = FALSE;
      pszServer = UtlParseX15( pCommArea->szBuffer, DIC_SERVER_IND);

      if ( pszServer[0] == NULC ) //local dict
      {
        //property file does not exist and so cannot be opened
        UtlError( ERROR_OPENING_PROPS,
                  MB_CANCEL, 1, &pszName, EQF_ERROR );

        // grey out dictionary as it cannot be accessed
        CLBSETITEMSTATEHWND( pCommArea->hwndLB, sItem, FALSE );
      }
      else  // remote dict - local prop file deleted
      {
        SETCURSOR( SPTR_ARROW );
        fWait = FALSE;

        // property file does not exist and so cannot be opened
        UtlError( ERROR_DEL_PROPS,
                  MB_CANCEL, 1, &pszName, EQF_ERROR );
        // delete prop file
        UtlDelete( chTempBuf1, 0L, FALSE );

        //remove entry from listbox
        EqfSend2AllHandlers( WM_EQFN_DELETED,
            MP1FROMSHORT( clsDICTIONARY ), MP2FROMP(pszObjName) );
      } /* endif */
    }
    else
    {
      SETSYMBOL( pszObjName );
      fDelFree = TRUE;

      if ( fOK )
      {
        hDicProp = OpenProperties( pszObjName, NULL,
                       PROP_ACCESS_READ, &ErrorInfo);
        if ( hDicProp )
        {
          pDicProp = (PPROPDICTIONARY) MakePropPtrFromHnd( hDicProp );

          if ( pDicProp->fCopyRight && fOK )
          {
            //dict is copyrighted
            SETCURSOR( SPTR_ARROW );
            fWait = FALSE;
            if ( *pusYesToAllMode == MB_EQF_YESTOALL )
            {
              usMBCode = UtlErrorHwnd( ERROR_DEL_COPYRIGHTED,
                                       MB_EQF_YESTOALL | MB_DEFBUTTON2,
                                       1, &pszName, EQF_QUERY, hwnd );
              if ( usMBCode == MBID_EQF_YESTOALL )
              {
                *pusYesToAllMode = usMBCode = MBID_YES;
              } /* endif */
            }
            else if ( *pusYesToAllMode == MBID_YES )
            {
              usMBCode = MBID_YES;
            }
            else
            {
              usMBCode = UtlError( ERROR_DEL_COPYRIGHTED,
                                   MB_YESNO | MB_DEFBUTTON2,
                                  1, &pszName, EQF_ERROR );
            } /* endif */
            if ( usMBCode != MBID_YES )
              fOK = FALSE;  //no delete
          } /* endif */
        }
        else
        {
          SETCURSOR( SPTR_ARROW );
          fWait = FALSE;

          //error opening prop file
          UtlError( ERROR_OPENING_PROPS, MB_CANCEL, 1, &pszName,
                    EQF_ERROR );
          fOK = FALSE;

          pszServer = UtlParseX15( pCommArea->szBuffer, DIC_SERVER_IND);

          if ( pszServer[0] == NULC )
          {
            CLBSETITEMSTATEHWND( pCommArea->hwndLB, sItem, FALSE );
          } /* endif */
        } /* endif */

        /*************************************************************/
        /* Try to open dictionary data file to ensure that no        */
        /* other process (local or remote) has openend the dictionary*/
        /* (only if not a shared dictionary access is deleted)       */
        /*************************************************************/
        if ( fOK && (pDicProp->szServer[0] == EOS) &&
             ( (pDicProp->szUserid[0] == NULC) ||
               (strcmp( pDicProp->szUserid, szUserID ) == 0) ||
               (usUserLevel == USER_ADMIN) ) )
        {
          HFILE  hFile;
          USHORT usOpenAction;

          usRc = UtlOpen( pDicProp->szDictPath, &hFile, &usOpenAction, 0L,
                          FILE_NORMAL, FILE_OPEN,
                          OPEN_ACCESS_READONLY | OPEN_SHARE_DENYREADWRITE,
                          0L, FALSE );
          if ( usRc == NO_ERROR )
          {
            UtlClose( hFile, FALSE );
          }
          else if ( (usRc == ERROR_SHARING_VIOLATION) ||
                    (usRc == ERROR_LOCK_VIOLATION) )
          {
            SETCURSOR( SPTR_ARROW );
            fWait = FALSE;

            UtlError( ERROR_DICT_LOCKED, MB_CANCEL, 1, &pszName, EQF_ERROR );
          fOK = FALSE;
          } /* endif */
        } /* endif */

        // usMBCode is still set correctly in the case of a
        // copyrighted dict and so applies below too; only
        // the error message has to be suppressed as otherwise
        // two messages would be issued
        if ( fOK )
        {
          if ( pDicProp->usLocation == LOC_SHARED )
          {
            if ( usRc != NO_ERROR )
            {
              usUserLevel = 0;
              chTempBuf1[0] = NULC;
            } /* endif */
            {
              if ( (pDicProp->szUserid[0] == NULC) ||
                   (strcmp( pDicProp->szUserid, szUserID ) == 0) ||
                   (usUserLevel == USER_ADMIN) )
              {
                //delete warning for a remote dictionary
                //if you are the owner of the dictionary
                //or the LAN administrator

                SETCURSOR( SPTR_ARROW );
                fWait = FALSE;
                if ( *pusYesToAllMode == MB_EQF_YESTOALL )
                {
                  usMBCode = UtlErrorHwnd( WARNING_DEL_OWN_REM_DICT,
                                           MB_EQF_YESTOALL | MB_DEFBUTTON2,
                                           1, &pszName, EQF_QUERY, hwnd );
                  if ( usMBCode == MBID_EQF_YESTOALL )
                  {
                    *pusYesToAllMode = usMBCode = MBID_YES;
                  } /* endif */
                }
                else if ( *pusYesToAllMode == MBID_YES )
                {
                  usMBCode = MBID_YES;
                }
                else
                {
                  usMBCode = UtlError(
                               WARNING_DEL_OWN_REM_DICT,
                               MB_YESNO | MB_DEFBUTTON2,
                               1, &pszName, EQF_QUERY );
                } /* endif */
                if ( usMBCode == MBID_YES )
                {
                  DictDel.usType = LOCAL_DEL;
                  DictDel.pszName = pDicProp->szDictPath;
                  EqfSend2Handler( DICTIONARYHANDLER,
                     WM_EQF_DELETE, 0, MP2FROMP(&DictDel) );
                  fOK = FALSE;
                }
                else
                {
                  fOK = FALSE;             //no delete
                } /* endif */
              }
              else
              {
                //delete warning for a remote dictionary
                //if you are the not owner of the
                //dictionary
                SETCURSOR( SPTR_ARROW );
                fWait = FALSE;
                if ( *pusYesToAllMode == MB_EQF_YESTOALL )
                {
                  usMBCode = UtlErrorHwnd( WARNING_DEL_REM_DICT,
                                           MB_EQF_YESTOALL | MB_DEFBUTTON2,
                                           1, &pszName, EQF_QUERY, hwnd );
                  if ( usMBCode == MBID_EQF_YESTOALL )
                  {
                    *pusYesToAllMode = usMBCode = MBID_YES;
                  } /* endif */
                }
                else if ( *pusYesToAllMode == MBID_YES )
                {
                  usMBCode = MBID_YES;
                }
                else
                {
                  usMBCode = UtlError(
                                 WARNING_DEL_REM_DICT,
                                 MB_YESNO | MB_DEFBUTTON2,
                                 1, &pszName, EQF_QUERY );
                } /* endif */

                if ( usMBCode == MBID_YES )
                {
                  DictDel.usType = PROPS;
                  DictDel.pszName = pDicProp->szDictPath;
                  EqfSend2Handler( DICTIONARYHANDLER,
                               WM_EQF_DELETE,
                               0,
                               MP2FROMP(&DictDel) );
                } /* endif */
                fOK = FALSE; // no delete
              } /* endif */
            } /* endif */
          }
          else if ( pDicProp->szServer[0] == NULC )
          {
            //delete warning for a local dictionary
            SETCURSOR( SPTR_ARROW );
            fWait = FALSE;
            if ( UtlFileExist( pDicProp->szDictPath ) )
            {
              usMBCode = MBID_CANCEL;
//            Allow to delete copyrighted dictionaries, too -
//            you have asked before if user wants
//              if ( !pDicProp->fCopyRight )
              {
                if ( *pusYesToAllMode == MB_EQF_YESTOALL )
                {
                  usMBCode = UtlErrorHwnd( WARNING_DELETE_DICTIONARY,
                                           MB_EQF_YESTOALL | MB_DEFBUTTON2,
                                           1, &pszName, EQF_QUERY, hwnd );
                  if ( usMBCode == MBID_EQF_YESTOALL )
                  {
                    *pusYesToAllMode = usMBCode = MBID_YES;
                  } /* endif */
                }
                else if ( *pusYesToAllMode == MBID_YES )
                {
                  usMBCode = MBID_YES;
                }
                else
                {
                  usMBCode = UtlError( WARNING_DELETE_DICTIONARY,
                                     MB_YESNO | MB_DEFBUTTON2,
                                     1, &pszName, EQF_QUERY );
                } /* endif */
              } /* endif */
              if ( usMBCode == MBID_YES )
              {
                DictDel.usType = LOCAL_DEL;
                DictDel.pszName = pDicProp->szDictPath;
                EqfSend2Handler( DICTIONARYHANDLER,
                                 WM_EQF_DELETE,
                                 0, MP2FROMP(&DictDel) );
                fOK = FALSE;
              }
              else
               fOK = FALSE;   //no delete
            }
            else
            {
              //file cannot be accessed, delete rest of dict
              if ( *pusYesToAllMode == MB_EQF_YESTOALL )
              {
                usMBCode = UtlErrorHwnd( ERROR_DEL_GREYEDOUT_DICT,
                                         MB_EQF_YESTOALL | MB_DEFBUTTON2,
                                         1, &pszName, EQF_QUERY, hwnd );
                if ( usMBCode == MBID_EQF_YESTOALL )
                {
                  *pusYesToAllMode = usMBCode = MBID_YES;
                } /* endif */
              }
              else if ( *pusYesToAllMode == MBID_YES )
              {
                usMBCode = MBID_YES;
              }
              else
              {
                usMBCode = UtlErrorHwnd( ERROR_DEL_GREYEDOUT_DICT,
                                         MB_YESNO | MB_DEFBUTTON2,
                                         1, &pszName, EQF_QUERY, hwnd );
              } /* endif */
              if ( usMBCode == MBID_YES )
              {
                AsdDelete( chTempBuf1 );

                // remove from dict list window
                EqfSend2AllHandlers( WM_EQFN_DELETED,
                   MP1FROMSHORT( clsDICTIONARY ), MP2FROMP(pszObjName) );
              }
              else
              {
                CLBSETITEMSTATEHWND( pCommArea->hwndLB, sItem, FALSE );
              } /* endif */
            } /* endif */
          }
        } /* endif */

        //remove symbol after deletion
        if ( fDelFree )
        {
          REMOVESYMBOL( pszObjName );
          fDelFree = FALSE;
        } /* endif */

        //close dict properties
        if ( hDicProp )
          CloseProperties( hDicProp, PROP_QUIT, &ErrorInfo);
      } /* endif */
    } /* endif */
  } /* endif */

  if ( fWait )
  {
    SETCURSOR( SPTR_ARROW );
  } /* endif */
  // convert long name back to OEM  characterset
  ANSITOOEM( pszName );
} /* end of function DictionaryDelete */


/*! \brief Deletes the given dictionary.
  \param pszDict name of the dictionary being deleted
	\returns 0 if successful or an error code in case of failures
*/
USHORT DicFuncDeleteDict
(
  PSZ         pszDictName              // name of dictionary
)
{
  USHORT usRC = 0;                      // function return code
  CHAR   szPropName[MAX_EQF_PATH];      // dictionary property name
  CHAR   szShortName[MAX_FNAME]; 

  if ( (pszDictName == NULL) || (*pszDictName == EOS) )
  {
    usRC = TMT_MANDCMDLINE;
    UtlErrorHwnd( usRC, MB_CANCEL, 0, NULL, EQF_ERROR, HWND_FUNCIF );
  } /* endif */

  // check if there is a dictionary with the given name
  if ( usRC == 0 )
  {
    BOOL fIsNew = FALSE;                  // is-new flag

    ObjLongToShortName( pszDictName, szShortName, DICT_OBJECT, &fIsNew );
    if ( fIsNew )
    {
      usRC = ERROR_DICT_NOT_FOUND;
	    UtlErrorHwnd( usRC, MB_CANCEL, 1, &pszDictName, EQF_ERROR, HWND_FUNCIF );
    }
  }

  // check property file
  if ( usRC == 0 )
  {
    USHORT usLen = 0;

    UtlMakeEQFPath( szPropName, NULC, PROPERTY_PATH, NULL );
    strcat( szPropName, BACKSLASH_STR );
    strcat( szPropName, szShortName );
    strcat( szPropName, EXT_OF_DICTPROP );

    if ( !UtlFileExist( szPropName ) )
    {
      usRC = ERROR_OPENING_PROPS;
      UtlErrorHwnd( usRC, MB_CANCEL, 1, &pszDictName, EQF_ERROR, HWND_FUNCIF );
    }
  }

  // delete the dictionary
  if ( usRC == 0 )
  {

    AsdDelete( szPropName );

    // notify OpenTM2 GUI to update
    //HWND hwnd = FindWindow( TWBMAIN ,NULL);
    //if(hwnd != NULL)
    //{
    //  COPYDATASTRUCT cds;
    //  std::string strMemName(Info.szPlugin);
    //  strMemName = strMemName+":";
    //  strMemName = strMemName +pszMemName;
    //  cds.dwData = 2;
    //  cds.cbData = (strMemName.size()+1)*sizeof(char);
    //  cds.lpData = (char*)strMemName.c_str();
    //  SendMessage(hwnd,WM_COPYDATA, 0,(LPARAM)(LPVOID)&cds);
    //}// end if

  }
  return( usRC );
}
