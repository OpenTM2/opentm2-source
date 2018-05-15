/*! \file
	Handler for all list-related functions.

	Copyright Notice:

	Copyright (C) 1990-2015, International Business Machines
	Corporation and others. All rights reserved
*/

#define INCL_EQF_LIST             // terminology list functions
#include <eqf.h>                  // General Translation Manager include file

#include "eqflist.id"             // List Handler IDs
#include "eqflp.h"                // Defines for generic list handlers
#define   EQFLIST_C               // initialize variables in EQFLISTI.H
#include "eqflisti.h"             // Private List Handler defines
#include "eqfcolw.id"             // column width IDs

/**********************************************************************/
/* Table containing column definition of the list handler instance    */
/* column listbox. The column heading names (1st parameter in table)  */
/* are loaded from the resource file                                  */
/**********************************************************************/
static CHAR ColHdr[6][80];             // Buffer for column header texts
static CLBCOLDATA ColTable[] =
{
  { "",         1,                   TEXT_DATA, DT_LEFT  }, // object name (=list file path name)
  { ColHdr[1],  (CLBDATATYPE)CLB_MAX_FNAME,       TEXT_DATA, DT_LEFT  }, // list name   (=list file name)
  { ColHdr[2],  (CLBDATATYPE)CLB_MAX_DRIVE,       TEXT_DATA, DT_LEFT  }, // drive letter
  { ColHdr[3],  (CLBDATATYPE)CLB_MAX_SIZE_LENGTH, NUMERIC_DATA, DT_RIGHT }, // size of list
  { ColHdr[4],  (CLBDATATYPE)CLB_MAX_DATE,        FDATE_DATA, DT_LEFT }, // last update (date)
  { ColHdr[5],  (CLBDATATYPE)CLB_MAX_DATE_TIME,   FDATETIME_DATA, DT_LEFT }, // last update (date+time)
  { NULL,       0,                   TEXT_DATA,         0        }
};

/**********************************************************************/
/* Default view lists for the column listbox. These lists are used    */
/* when no last used values are available.                            */
/**********************************************************************/
static SHORT sLastUsedView[MAX_VIEW+1] =
  { LST_NAME_IND, CLBLISTEND };
static SHORT sDefaultView[MAX_VIEW+1]  =
  { LST_NAME_IND, CLBLISTEND };
static SHORT sNameView[MAX_VIEW+1]     =
  { LST_NAME_IND, CLBLISTEND };
static SHORT sDetailsView[MAX_VIEW+1]  =
  { LST_NAME_IND, LST_DRIVE_IND, CLBLISTEND };
static SHORT sSortCriteria[MAX_VIEW+1] =
  { LST_NAME_IND, CLBLISTEND };

/**********************************************************************/
/* Column listbox control structure                                   */
/**********************************************************************/
static CLBCTLDATA LstCLBData =
{  sizeof(CLBCTLDATA),                 // size of control structure
   6,                                  // we have 6 data columns
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

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     ListHandlerWP   window procedure of list handler
//------------------------------------------------------------------------------
// Function call:     MRESULT ListHandlerWP( hwnd, msg, mp1, mp2 )
//------------------------------------------------------------------------------
// Description:       Handles all messages send to the list handler by other
//                    components of MAT Tools.
//------------------------------------------------------------------------------
// Input parameter:   HWND    hwnd    handle of list handler object window
//                    USHORT  msg     message number
//                    MPARAM  mp1     message parameter 1
//                    MPARAM  mp2     message parameter 2
//------------------------------------------------------------------------------
// Returncode type:   MRESULT
//------------------------------------------------------------------------------
// Returncodes:       depends on message type
//                    normal return codes are:
//                    TRUE  = message has been processed
//                    FALSE = message has not been processed
//------------------------------------------------------------------------------
// Function flow:     switch message
//                      case WM_CREATE:
//                        allocate IDA
//                        register handler to object manager
//                        register list handler instance class to PM
//                      case WM_EQF_ABOUTTODELETE:
//                      case WM_EQFN_CREATED:
//                      case WM_EQFN_DELETED:
//                      case WM_EQFN_PROPERTIESCHANGED:
//                      case WM_EQF_ABOUTTOREMOVEDRIVE:
//                        send message to all object of list class
//                      case WM_EQF_OPEN:
//                        create the list handler instance window
//                        post initialization message to instance
//                      case WM_EQF_INSERTNAMES:
//                    endswitch
//------------------------------------------------------------------------------
MRESULT ListHandlerCallBack
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
      pCommArea->pfnCallBack          = ListCallBack;
      strcpy( pCommArea->szHandlerName, LISTHANDLER );
      pCommArea->sBaseClass           = clsLIST;
      pCommArea->sListWindowID        = ID_LIST_WINDOW;
      pCommArea->sListboxID           = ID_LIST_LB;
      pCommArea->asNotifyClassList[0] = clsLIST;
      pCommArea->asNotifyClassList[1] = 0;       // end of list
      break;

    case WM_EQF_OPEN:
      {
        HWND       hwndObj;
        PSZ        pszObj = (PSZ) PVOIDFROMMP2(mp2);

        if( (hwndObj = EqfQueryObject( pszObj, clsLIST, 0) ) != NULLHANDLE )
        {
          ActivateMDIChild( hwndObj );
          mResult = MRFROMSHORT( TRUE );
        }
        else
        {
          mResult = WinSendMsg( hwnd, WM_EQF_CREATELISTWINDOW, mp1, mp2 );
        } /* endif */
      }
      break;

    case WM_EQF_INSERTNAMES:
      {
        PSZ        pszObject;          // ptr to object name
        BOOL       fOK = TRUE;
        LISTTYPES  Type;

        pszObject = (PSZ) PVOIDFROMMP2(mp2);
        memset (&Type, 0, sizeof(Type));
        if ( pszObject == NULL )
        {
          fOK = FALSE;
        }
        else if ( strcmp( pszObject, EXCLUSIONLISTOBJ ) == 0 )
        {
          Type = EXCL_TYPE;
        }
        else if ( strcmp( pszObject, NOISELISTOBJ ) == 0 )
        {
          Type = NOISE_TYPE;
        }
        else if ( strcmp( pszObject, NEWTERMLISTOBJ ) == 0 )
        {
          Type = NTL_TYPE;
        }
        else if ( strcmp( pszObject, FOUNDTERMLISTOBJ ) == 0 )
        {
          Type = FTL_TYPE;
        }
        else
        {
          fOK = FALSE;
        } /* endif */

        if ( fOK )
        {
          mResult = (MRESULT)LstInsertListNames( Type, FALSE,
                                                 HWNDFROMMP1(mp1),
                                                 pCommArea->szBuffer,
                                                 FALSE );
        }
        else
        {
           mResult = MRFROMSHORT( 0 );
        } /* endif */
      }
      break;

    case WM_DESTROY:
      /****************************************************************/
      /* Nothing to do, as nothing has been allocated by the list     */
      /* handler callback function                                    */
      /****************************************************************/
      break;

    default:
      break;
  } /* endswitch */
  return( mResult );
} /* end of function ListHandlerCallBack */




//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     ListInstanceWP  Window procedure for lists list window
//------------------------------------------------------------------------------
// Function call:     MRESULT ListInstanceWP( hwnd, msg, mp1, mp2 )
//------------------------------------------------------------------------------
// Description:       Procedure controlling the list handler list window.
//------------------------------------------------------------------------------
// Input parameter:   HWND    hwnd    handle of list handler object window
//                    USHORT  msg     message number
//                    MPARAM  mp1     message parameter 1
//                    MPARAM  mp2     message parameter 2
//------------------------------------------------------------------------------
// Returncode type:   MRESULT
//------------------------------------------------------------------------------
// Returncodes:       depends on message type
//                    normal return codes are:
//                    TRUE  = message has been processed
//                    FALSE = message has not been processed
//------------------------------------------------------------------------------
// Function flow:     switch message
//                      case WM_CREATE:
//                        call LstInstCREATE to process the message
//                      case WM_CLOSE:
//                        remove object using EqfRemoveObject
//                      case WM_DESTROY:
//                        free window IDA
//                      case WM_EQF_TERMINATE:
//                        call LstInstTERMINATE to process the message
//                      case WM_EQF_INITIALIZE:
//                        call LstInstINITIALIZE to process the message
//                      case WM_EQFN_CREATED:
//                      case WM_EQFN_DELETED:
//                      case WM_EQFN_PROPERTIESCHANGED:
//                        refresh contents of lists list box
//                      case WM_EQF_INSERTNAMES:
//                        call list processor of requested list type to fill
//                          the caller's list
//                      case WM_INITMENU:
//                        disable all menu items which are not supported by
//                          the currently active list processer
//                      case WM_CONTROL:
//                        if message was sent by list type combo box and
//                           is CBN_EFCHANGE
//                          activate list processor for selected list type
//                          call active list processor to fill lists list box
//                        endif
//                      case WM_EQF_COMMAND:
//                      case WM_COMMAND:
//                        switch command value
//                          case PID_VIEW_MI_NAMES:
//                          case PID_VIEW_MI_DETAILS:
//                             pass message to column list box
//                          case PID_VIEW_MI_DETAILSDLG:
//                             pass message to column list box
//                             save new details view list to list properties
//                          case PID_MI_FILE_OPEN:
//                             call active listprocessor to open the list
//                          case PID_MI_FILE_DELETE:
//                             call active listprocessor to delete the list
//                          case PID_MI_FILE_NEW:
//                             call active listprocessor to create the list
//                          case PID_MI_FILE_EXPORT:
//                             call active listprocessor to export the list
//                          case PID_MI_FILE_EXPORT:
//                             call active listprocessor to import the list
//                        endswitch
//                      case WM_SIZE:
//                        set size and position of window controls
//                      case WM_SETFOCUS:
//                        pass focus to lists list box
//                      case WM_ACTIVATE:
//                        call EqfActivateInstance with first message parameter
//                    endswitch
//------------------------------------------------------------------------------
/**********************************************************************/
/* List instance callback function for terminology list window        */
/**********************************************************************/
MRESULT ListCallBack
(
  PLISTCOMMAREA    pCommArea,
  HWND             hwnd,
  WINMSG           msg,
  WPARAM           mp1,
  LPARAM           mp2
)
{
  MRESULT          mResult = MRFROMSHORT(FALSE);
  PLSTIDA          pIda;               // pointer to instance data area
  PLISTLASTUSED    pLU = NULL;         // ptr to last used values for list


  switch ( msg )
  {
    case WM_CREATE :
      {
        BOOL       fOK = TRUE;         // initialisation is O.K. flag
        EQFINFO    ErrorInfo;          // error info of property handler
        HPROP      hProp = NULL;       // ptr to property handle
        PPROPLIST  pProp = NULL;       // ptr to list properties
        CHAR       szParm[20];         // buffer for error parameters
        PSZ        pParm;              // ptr to error parameter(s)
        HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);

        /*******************************************************************/
        /* Allocate list instance data area (IDA)                          */
        /*******************************************************************/
        UtlAlloc( (PVOID *) &pIda, 0L, (LONG) sizeof( LSTIDA), ERROR_STORAGE );
        if( !pIda )
        {
          fOK = FALSE;
        } /* endif */

        /**************************************************************/
        /* Check object name (only 5 types of lists are supported)    */
        /**************************************************************/
        if ( fOK )
        {
          if ( strcmp( pCommArea->szObjName, EXCLUSIONLISTOBJ ) == 0 )
          {
            pIda->Type = EXCL_TYPE;
          }
          else if ( strcmp( pCommArea->szObjName, NOISELISTOBJ ) == 0 )
          {
            pIda->Type = NOISE_TYPE;
          }
          else if ( strcmp( pCommArea->szObjName, NEWTERMLISTOBJ ) == 0 )
          {
            pIda->Type = NTL_TYPE;
          }
          else if ( strcmp( pCommArea->szObjName, FOUNDTERMLISTOBJ ) == 0 )
          {
            pIda->Type = FTL_TYPE;
          }
          else
          {
            fOK = FALSE;
          } /* endif */
        } /* endif */

        /*******************************************************************/
        /* Fill and anchor IDA                                             */
        /*******************************************************************/
        if ( fOK )                     // if everything is OK so far ...
        {
           pIda->fFirstActivation = TRUE;
           pCommArea->pUserIDA = pIda;
        } /* endif */

        /*******************************************************************/
        /* Open or create list properties                                  */
        /*******************************************************************/
        if ( fOK )                          // if everything is OK so far ...
        {
          /************************************************************/
          /* Setup properties name                                    */
          /************************************************************/
          UtlMakeEQFPath( pCommArea->szBuffer, NULC, SYSTEM_PATH, NULL );
          strcat( pCommArea->szBuffer, BACKSLASH_STR );
          strcat( pCommArea->szBuffer, LISTHANDLER_PROPERTIES_NAME );

          /*****************************************************************/
          /* Try to open property file                                     */
          /*****************************************************************/
          hProp = OpenProperties( pCommArea->szBuffer, NULL,
                                  PROP_ACCESS_READ, &ErrorInfo);
          if( hProp )
          {
            /***************************************************************/
            /* If open was successful get pointer to property file         */
            /***************************************************************/
             pProp = (PPROPLIST) MakePropPtrFromHnd( hProp );
          }
          else
          {
            /**************************************************************/
            /* Try to create a new properties file                        */
            /**************************************************************/
            hProp= CreateProperties( pCommArea->szBuffer, (PSZ)NULP,
                                     PROP_CLASS_LIST, &ErrorInfo);
            if ( !hProp )
            {
              /*************************************************************/
              /* Handle errors returned by CreateProperties                */
              /*************************************************************/
              switch ( (USHORT)ErrorInfo )
              {
                case Err_NoDiskSpace :
                  strncpy( szParm, pIda->IdaHead.szObjName, 2 );
                  szParm[2] = '\0';
                  pParm = szParm;
                  UtlError( ERROR_DISK_IS_FULL, MB_CANCEL, 1, &pParm, EQF_ERROR );
                  break;
                default :
                  /*********************************************************/
                  /* If ErrorInfo is Err_NoStorage then a message has      */
                  /* been issued by CreateProperties already.              */
                  /*********************************************************/
                  if ( ErrorInfo != Err_NoStorage )
                  {
                    pParm = pCommArea->szBuffer;
                    UtlError( ERROR_CREATE_PROPERTIES, MB_CANCEL, 1, &pParm, EQF_ERROR );
                  } /* endif */
                  break;
              } /* endswitch */
              mResult = MRFROMSHORT(DO_NOT_CREATE);        // do not create the window
              fOK = FALSE;
            }
            else
            {
              /*************************************************************/
              /* Fill-in default values                                    */
              /*************************************************************/
              pProp = (PPROPLIST)MakePropPtrFromHnd( hProp );
              pProp->LU.Swp.cx = 40 * (SHORT)UtlQueryULong( QL_AVECHARWIDTH );
              pProp->LU.Swp.cy = 14 * (SHORT)UtlQueryULong( QL_CHARHEIGHT );
              pProp->LU.Swp.x =  70;
              pProp->LU.Swp.y =  100;
              pProp->LU.Swp.fs = EQF_SWP_SHOW | EQF_SWP_MOVE | EQF_SWP_SIZE;


              /*************************************************************/
              /* Write the properties and reset property access            */
              /*************************************************************/
              if ( SaveProperties( hProp, &ErrorInfo) )
              {
                pParm = pCommArea->szBuffer;
                UtlError( ERROR_WRITE_PROPERTIES, MB_CANCEL, 1, &pParm, EQF_ERROR);
                mResult = MRFROMSHORT(DO_NOT_CREATE);        // do not create the window
                fOK = FALSE;
              } /* endif */
              ResetPropAccess( hProp, PROP_ACCESS_WRITE );
            } /* endif */
          } /* endif */
        } /* endif */

        /**************************************************************/
        /* Load column listbox title strings                          */
        /**************************************************************/
        if ( fOK  )
        {
          LOADSTRING( NULLHANDLE, hResMod, SID_LST_NAME_HDR, ColHdr[1] );
          UtlLoadWidth( NULLHANDLE, hResMod, SID_LST_NAME_COLWIDTH,
                        &(ColTable[1] .usWidth) );
          LOADSTRING( NULLHANDLE, hResMod, SID_LST_DRIVE_HDR, ColHdr[2] );
          UtlLoadWidth( NULLHANDLE, hResMod, SID_LST_DRIVE_COLWIDTH,
                        &(ColTable[2] .usWidth) );
          LOADSTRING( NULLHANDLE, hResMod, SID_LST_SIZE_HDR, ColHdr[3] );
          UtlLoadWidth( NULLHANDLE, hResMod, SID_LST_SIZE_COLWIDTH,
                        &(ColTable[3] .usWidth) );
          LOADSTRING( NULLHANDLE, hResMod, SID_LST_UPD1_HDR, ColHdr[4] );
          UtlLoadWidth( NULLHANDLE, hResMod, SID_LST_UPD1_COLWIDTH,
                        &(ColTable[4] .usWidth) );
          LOADSTRING( NULLHANDLE, hResMod, SID_LST_UPD2_HDR, ColHdr[5] );
          UtlLoadWidth( NULLHANDLE, hResMod, SID_LST_UPD2_COLWIDTH,
                        &(ColTable[5] .usWidth) );
        } /* endif */

        /**************************************************************/
        /* Get pointer to last used values for given list type        */
        /**************************************************************/
        if ( fOK )
        {
          LISTLASTUSED Empty;

          switch ( pIda->Type )
          {
            case EXCL_TYPE :   pLU = &(pProp->EXCLU);  break;
            case NOISE_TYPE :  pLU = &(pProp->NOILU);  break;
            case FTL_TYPE :    pLU = &(pProp->FTLLU);  break;
            case NTL_TYPE :    pLU = &(pProp->NTLLU);  break;
          } /* endswitch */
          memset( &Empty, 0, sizeof(Empty) );
          if ( memcmp( pLU, &Empty, sizeof(LISTLASTUSED) ) == 0 )
          {
            HWND hwndObj = EqfQueryObject( NULL, clsLIST, OBJ_BUSY);
            memcpy( pLU, &(pProp->LU), sizeof(LISTLASTUSED) );
            if ( hwndObj != NULLHANDLE )
            {
              SWP swp;
              WinQueryWindowPos( hwndObj, &swp );
              SWP2EQFSWP( swp, pLU->Swp );
              pLU->Swp.x += 3 * (SHORT)UtlQueryULong( QL_AVECHARWIDTH );
              pLU->Swp.y -= (SHORT)WinQuerySysValue( HWND_DESKTOP, SV_CYTITLEBAR ) * 3 / 2;
            } /* endif */
          } /* endif */
        } /* endif */

        /**************************************************************/
        /* Set column listbox view lists                              */
        /**************************************************************/
        if ( fOK  )
        {
          memcpy( pCommArea->asCurView,
                  (pLU->sLastUsedViewList[0] != 0) ? pLU->sLastUsedViewList :
                                                       sLastUsedView,
                  sizeof(pCommArea->asCurView) );
          LstCLBData.psLastUsedViewList = pCommArea->asCurView;

          memcpy( pCommArea->asDetailsView,
                  (pLU->sDetailsViewList[0] != 0) ? pLU->sDetailsViewList :
                                                      sDetailsView,
                  sizeof(pCommArea->asDetailsView) );
          LstCLBData.psDetailsViewList = pCommArea->asDetailsView;

          memcpy( pCommArea->asSortList,
                  (pLU->sSortList[0] != 0) ? pLU->sSortList :
                                             sSortCriteria,
                  sizeof(pCommArea->asSortList) );
          LstCLBData.psSortList = pCommArea->asSortList;

          memcpy( &pCommArea->Filter, &pProp->Filter,
                  sizeof(pCommArea->Filter) );
          LstCLBData.pFilter = &pCommArea->Filter;

          /*
          {
	         int i=0;
	         for (i=0;pCommArea->asCurView[i]>0;i++)
	         {
                  int index = pCommArea->asCurView[i];
                  if (pLU->sLastUsedViewWidth[i] > 0)
                  {
                      ColTable[index].usWidth = pLU->sLastUsedViewWidth[i];
                  }
                  else
                  {
                      pLU->sLastUsedViewWidth[i] = ColTable[index].usWidth;
                  }
		     }

	      }
          */
        } /* endif */

        /****************************************************************/
        /* supply all information required to create a the list window  */
        /****************************************************************/
        if ( fOK )
        {
          pCommArea->sListObjClass = clsLIST;
          switch( pIda->Type )
          {
            case EXCL_TYPE:
              LOADSTRING( NULLHANDLE, hResMod, SID_LST_EXCL_TITLE, pCommArea->szTitle );
              pCommArea->hIcon          = (HPOINTER) UtlQueryULong(QL_EXCLLISTICON); //hiconEXCLLIST;
              break;
            case NOISE_TYPE:
              LOADSTRING( NULLHANDLE, hResMod, SID_LST_NOISE_TITLE, pCommArea->szTitle );
              pCommArea->hIcon          = (HPOINTER) UtlQueryULong(QL_LISTICON); //hiconLIST;
              break;
            case NTL_TYPE:
              LOADSTRING( NULLHANDLE, hResMod, SID_LST_NTL_TITLE, pCommArea->szTitle );
              pCommArea->hIcon          = (HPOINTER) UtlQueryULong(QL_NEWLISTICON); //hiconNEWLIST;
              break;
            case FTL_TYPE:
              LOADSTRING( NULLHANDLE, hResMod, SID_LST_FTL_TITLE, pCommArea->szTitle );
              pCommArea->hIcon          = (HPOINTER) UtlQueryULong(QL_FOUNDLISTICON); //hiconFOUNDLIST;
              break;
          } /* endswitch */
          pCommArea->sObjNameIndex  = LST_OBJECT_IND;
          pCommArea->sNameIndex     = LST_NAME_IND;
          pCommArea->sListWindowID  = ID_LIST_WINDOW;
          pCommArea->sListboxID     = ID_LIST_LB;
          switch( pIda->Type )
          {
            case EXCL_TYPE:
            case NOISE_TYPE:
              pCommArea->sPopupMenuID      = ID_LST_POPUP;
              pCommArea->sGreyedPopupMenuID = ID_LST_POPUP;
              pCommArea->sNoSelPopupMenuID = ID_LST_POPUP_NOSEL;
              break;
            case NTL_TYPE:
            case FTL_TYPE:
              pCommArea->sPopupMenuID      = ID_LST_WO_NEW_POPUP;
              pCommArea->sGreyedPopupMenuID = ID_LST_POPUP;
              pCommArea->sNoSelPopupMenuID = ID_LST_WO_NEW_POPUP_NOSEL;
              break;
          } /* endswitch */
          pCommArea->pColData       = &LstCLBData;
          pCommArea->fMultipleSel   = FALSE;
          pCommArea->sDefaultAction = PID_FILE_MI_OPEN;
          memcpy( &(pCommArea->swpSizePos), &(pLU->Swp), sizeof(EQF_SWP) );
          pCommArea->sItemClass     = clsLIST;
          pCommArea->sItemPropClass = PROP_CLASS_LIST;
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

    case WM_EQF_INITIALIZE:
      /****************************************************************/
      /* Fill column listbox                                          */
      /****************************************************************/
      pIda = (PLSTIDA) pCommArea->pUserIDA;
      LstInsertListNames( pIda->Type, TRUE, pCommArea->hwndLB,
                          pCommArea->szBuffer, FALSE );

      if (QUERYITEMCOUNTHWND(pCommArea->hwndLB) )
      {
        SELECTITEMHWND(pCommArea->hwndLB, 0);
      } /* endif */
      break;


    case WM_CLOSE :
    case WM_EQF_TERMINATE :
      /**************************************************************/
      /* Save view lists for WM_EQF_TERMINATE only if save flag is  */
      /* on                                                         */
      /**************************************************************/
      if ( (msg == WM_CLOSE) || (SHORT1FROMMP1(mp1) & TWBSAVE) )
      {
        PPROPLIST     pProp;
        EQFINFO       ErrorInfo;
        HPROP         hProp;

        /************************************************************/
        /* Setup properties name                                    */
        /************************************************************/
        pIda = (PLSTIDA)pCommArea->pUserIDA;
        UtlMakeEQFPath( pCommArea->szBuffer, NULC, SYSTEM_PATH, NULL );
        strcat( pCommArea->szBuffer, BACKSLASH_STR );
        strcat( pCommArea->szBuffer, LISTHANDLER_PROPERTIES_NAME );

        /**************************************************************/
        /* Open properties                                            */
        /**************************************************************/
        hProp = OpenProperties( pCommArea->szBuffer, NULL,
                                PROP_ACCESS_READ, &ErrorInfo );
        if( hProp )
        {
          /************************************************************/
          /* Save current view lists and window position              */
          /************************************************************/
          if( SetPropAccess( hProp, PROP_ACCESS_WRITE ) )
          {
            USHORT usWidth[MAX_DEFINEDCOLUMNS];

           (BOOL)WinSendMsg( pCommArea->hwndLB,
                             WM_EQF_GETCOLUMNWIDTH,
                             (WPARAM)usWidth,
                              NULL );

            pProp = (PPROPLIST)MakePropPtrFromHnd( hProp);
            switch ( pIda->Type )
            {
              case EXCL_TYPE :   pLU = &(pProp->EXCLU);  break;
              case NOISE_TYPE :  pLU = &(pProp->NOILU);  break;
              case FTL_TYPE :    pLU = &(pProp->FTLLU);  break;
              case NTL_TYPE :    pLU = &(pProp->NTLLU);  break;
            } /* endswitch */
            memcpy( &pLU->Swp, &(pCommArea->swpSizePos), sizeof(EQF_SWP) );
            memcpy( pLU->sLastUsedViewList, pCommArea->asCurView,
                    sizeof(pLU->sLastUsedViewList) );

            //memcpy( (pLU->sLastUsedViewWidth), (usWidth),
            //         sizeof( pLU->sLastUsedViewWidth ));

            memcpy( pLU->sDetailsViewList, pCommArea->asDetailsView,
                    sizeof(pLU->sDetailsViewList) );
            memcpy( pLU->sSortList, pCommArea->asSortList,
                    sizeof(pLU->sSortList) );
            memcpy( &pProp->Filter, &pCommArea->Filter,
                    sizeof(pProp->Filter) );
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
      /* nothing to do for folder                                     */
      /****************************************************************/
      pIda = (PLSTIDA)pCommArea->pUserIDA;
      UtlAlloc( (PVOID *) &pIda, 0L, 0L, NOMSG );
      pCommArea->pUserIDA = NULL;
      break;

    case WM_EQF_BUILDITEMTEXT :
      pIda = (PLSTIDA)pCommArea->pUserIDA;
      /****************************************************************/
      /* Setup item text for the object passed in mp2 parameter       */
      /****************************************************************/

      /****************************************************************/
      /* First of all check if item belongs to our list ...           */
      /****************************************************************/
      if ( mp2 != 0L )               // continue only if a object name is given
      {
        PSZ        pszExtension = NULL;
        PSZ        pszObject = (PSZ) PVOIDFROMMP2(mp2);
        USHORT     usPathID = 0;

        switch ( pIda->Type )
        {
          case NTL_TYPE :
            usPathID = LIST_PATH;
            pszExtension = EXT_OF_NEWTERMS_LIST;
            break;

          case FTL_TYPE :
            usPathID = LIST_PATH;
            pszExtension = EXT_OF_FOUNDTERMS_LIST;
            break;

          case NOISE_TYPE :
            usPathID = TABLE_PATH;
            pszExtension = EXT_OF_EXCLUSION;
            break;

          case EXCL_TYPE :
            usPathID = LIST_PATH;
            pszExtension = EXT_OF_EXCLUSION;
            break;
        } /* endswitch */
        UtlMakeEQFPath( pCommArea->szBuffer, *pszObject, usPathID, NULL );
        strcat( pCommArea->szBuffer, BACKSLASH_STR );
        Utlstrccpy( pCommArea->szBuffer + strlen(pCommArea->szBuffer),
                    UtlGetFnameFromPath( pszObject ),
                    DOT );
        strcat( pCommArea->szBuffer, pszExtension );
        if( strcmp( (PSZ)PVOIDFROMMP2(mp2), pCommArea->szBuffer ) == 0 )
        {
          HDIR     hDir = HDIR_CREATE;
          USHORT   usCount = 1;
          FILEFINDBUF   stResultBuf;

          if ( UtlFindFirst( pCommArea->szBuffer, &hDir, FILE_NORMAL,
                        &stResultBuf, sizeof( stResultBuf), &usCount, 0L, 0) )
          {
            usCount = 0;  // no files as return code is set
          } /* endif */

          // close search file handle
          if ( hDir != HDIR_CREATE ) UtlFindClose( hDir, FALSE );


          if ( usCount == 1)
          {
            /************************************************************/
            /* Build listbox item string                                */
            /************************************************************/
            FDATE fDate;
            FTIME fTime;
            FileTimeToDosDateTime( &stResultBuf.ftLastWriteTime,
                                   (LPWORD)(PVOID)&fDate, (LPWORD)(PVOID)&fTime );
            LstBuildListboxItem( pCommArea->szBuffer, pszObject, &fDate,
                                 &fTime, RESBUFSIZE(stResultBuf) );
            mResult = MRFROMSHORT( TRUE );
          } /* endif */
        } /* endif */
      }
      else
      {
        /**************************************************************/
        /* No object name given, so return FALSE to indicate an error */
        /**************************************************************/
        mResult = MRFROMSHORT( TRUE );
      } /* endif */
      break;

    case WM_EQF_INITMENU:
    case WM_INITMENU:
      {
        SHORT         sItem;                // selected listbox item

        pIda = (PLSTIDA)pCommArea->pUserIDA;
        sItem = QUERYNEXTSELECTIONHWND( pCommArea->hwndLB, LIT_FIRST );
        UtlMenuDisableAll( hwnd, SHORT1FROMMP1( mp1) );
        UtlMenuEnableItem( PID_FILE_MI_PRINTLIST );
        switch ( pIda->Type )
        {
          case NTL_TYPE :
            UtlMenuEnableItem( PID_FILE_MI_IMPORT );
            UtlMenuEnableItem( PID_VIEW_MI_NAMES );
            UtlMenuEnableItem( PID_VIEW_MI_DETAILSDLG );
            UtlMenuEnableItem( PID_VIEW_MI_DETAILS );
            UtlMenuEnableItem( PID_VIEW_MI_SORT );
            UtlMenuEnableItem( PID_VIEW_MI_SOME );
            UtlMenuEnableItem( PID_VIEW_MI_ALL );
            if ( sItem != LIT_NONE )
            {
              UtlMenuEnableItem( PID_FILE_MI_OPEN );
              UtlMenuEnableItem( PID_FILE_MI_EXPORT );
              UtlMenuEnableItem( PID_FILE_MI_PRINT );
              UtlMenuEnableItem( PID_FILE_MI_DELETE );
            } /* endif */
            break;

          case FTL_TYPE :
            UtlMenuEnableItem( PID_FILE_MI_IMPORT );
            UtlMenuEnableItem( PID_VIEW_MI_NAMES );
            UtlMenuEnableItem( PID_VIEW_MI_DETAILSDLG );
            UtlMenuEnableItem( PID_VIEW_MI_DETAILS );
            UtlMenuEnableItem( PID_VIEW_MI_SORT );
            UtlMenuEnableItem( PID_VIEW_MI_SOME );
            UtlMenuEnableItem( PID_VIEW_MI_ALL );
            if ( sItem != LIT_NONE )
            {
              UtlMenuEnableItem( PID_FILE_MI_OPEN );
              UtlMenuEnableItem( PID_FILE_MI_EXPORT );
              UtlMenuEnableItem( PID_FILE_MI_PRINT );
              UtlMenuEnableItem( PID_FILE_MI_DELETE );
            } /* endif */
            break;

          case NOISE_TYPE :
            UtlMenuEnableItem( PID_VIEW_MI_NAMES );
            UtlMenuEnableItem( PID_VIEW_MI_DETAILSDLG );
            UtlMenuEnableItem( PID_VIEW_MI_DETAILS );
            UtlMenuEnableItem( PID_VIEW_MI_SORT );
            UtlMenuEnableItem( PID_VIEW_MI_SOME );
            UtlMenuEnableItem( PID_VIEW_MI_ALL );
            if ( sItem != LIT_NONE )
            {
              UtlMenuEnableItem( PID_FILE_MI_OPEN );
              UtlMenuEnableItem( PID_FILE_MI_PRINT );
            } /* endif */
            break;

          case EXCL_TYPE :
            UtlMenuEnableItem( PID_FILE_MI_NEW );
            UtlMenuEnableItem( PID_FILE_MI_IMPORT );
            UtlMenuEnableItem( PID_VIEW_MI_NAMES );
            UtlMenuEnableItem( PID_VIEW_MI_DETAILSDLG );
            UtlMenuEnableItem( PID_VIEW_MI_DETAILS );
            UtlMenuEnableItem( PID_VIEW_MI_SORT );
            UtlMenuEnableItem( PID_VIEW_MI_SOME );
            UtlMenuEnableItem( PID_VIEW_MI_ALL );
            if ( sItem != LIT_NONE )
            {
              UtlMenuEnableItem( PID_FILE_MI_OPEN );
              UtlMenuEnableItem( PID_FILE_MI_EXPORT );
              UtlMenuEnableItem( PID_FILE_MI_PRINT );
              UtlMenuEnableItem( PID_FILE_MI_DELETE );
            } /* endif */
            break;
        } /* endswitch */

      }
      break;

    case WM_EQF_TOOLBAR_ENABLED:
      {
        /**************************************************************/
        /* check for toolbar items to be enabled...                   */
        /**************************************************************/
        SHORT         sItem;                // selected listbox item

        pIda = (PLSTIDA)pCommArea->pUserIDA;
        sItem = QUERYNEXTSELECTIONHWND( pCommArea->hwndLB, LIT_FIRST );

        switch ( pIda->Type )
        {
          case NTL_TYPE :
          case FTL_TYPE :
            switch ( mp1 )
            {
              case PID_FILE_MI_IMPORT:
              case PID_VIEW_MI_NAMES:
              case PID_VIEW_MI_DETAILSDLG:
              case PID_VIEW_MI_DETAILS:
              case PID_VIEW_MI_SORT:
              case PID_VIEW_MI_SOME:
              case PID_VIEW_MI_ALL:
                mResult = TRUE;
                break;
              case PID_FILE_MI_OPEN:
              case PID_FILE_MI_EXPORT:
              case PID_FILE_MI_PRINT:
              case PID_FILE_MI_DELETE:
                mResult = (sItem != LIT_NONE);
                break;
            } /* endswitch */
            break;

          case NOISE_TYPE :
            switch ( mp1 )
            {
              case PID_VIEW_MI_NAMES:
              case PID_VIEW_MI_DETAILSDLG:
              case PID_VIEW_MI_DETAILS:
              case PID_VIEW_MI_SORT:
              case PID_VIEW_MI_SOME:
              case PID_VIEW_MI_ALL:
                mResult = TRUE;
                break;
              case PID_FILE_MI_OPEN:
              case PID_FILE_MI_PRINT:
                mResult = (sItem != LIT_NONE);
                break;
            } /* endswitch */
            break;

          case EXCL_TYPE :
            switch ( mp1 )
            {
              case PID_FILE_MI_NEW:
              case PID_FILE_MI_IMPORT:
              case PID_VIEW_MI_NAMES:
              case PID_VIEW_MI_DETAILSDLG:
              case PID_VIEW_MI_DETAILS:
              case PID_VIEW_MI_SORT:
              case PID_VIEW_MI_SOME:
              case PID_VIEW_MI_ALL:
                mResult = TRUE;
                break;
              case PID_FILE_MI_OPEN:
              case PID_FILE_MI_EXPORT:
              case PID_FILE_MI_PRINT:
              case PID_FILE_MI_DELETE:
                mResult = (sItem != LIT_NONE);
                break;
            } /* endswitch */
            break;
        } /* endswitch */
        break;
      }
      break;

    case WM_EQF_COMMAND:
    case WM_COMMAND:
      {
        PSZ   pszObjName;
        PSZ   pszListName;
        mResult = MRFROMSHORT( TRUE );   // default return code for COMMAND msgs
        pIda = (PLSTIDA)pCommArea->pUserIDA;


        /****************************************************************/
        /* extract currently selected item                              */
        /****************************************************************/
        {
          SHORT sItem;

          sItem = QUERYSELECTIONHWND( pCommArea->hwndLB );
          if ( sItem != LIT_NONE )
          {
            // Win2000 corrupts our data if we use LB_GETTEXT so use own message instead
            //  QUERYITEMTEXTHWND( pCommArea->hwndLB, sItem, pCommArea->szBuffer );
            SendMessage( pCommArea->hwndLB, LM_EQF_QUERYITEMTEXT, (WPARAM)sItem, (LPARAM)pCommArea->szBuffer );
            pszObjName = UtlParseX15( pCommArea->szBuffer, LST_OBJECT_IND );
            pszListName = UtlParseX15( pCommArea->szBuffer, LST_NAME_IND );
          }
          else
          {
            pszObjName = NULL;
          } /* endif */
        }

        /**************************************************************/
        /* Process the command                                        */
        /**************************************************************/
        switch ( SHORT1FROMMP1(mp1) )
        {
          case PID_FILE_MI_NEW:
            switch ( pIda->Type )
            {
              case EXCL_TYPE:
              case NOISE_TYPE:
                {
                  PLISTEDITIDA   pDlgIda;        // pointer to dialog IDA
                  BOOL           fOK;

                  UtlAlloc( (PVOID *) &pDlgIda, 0L, (LONG)sizeof(LISTEDITIDA), ERROR_STORAGE );
                  if ( pDlgIda )
                  {
                    HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
                    pDlgIda->usListType = (USHORT)pIda->Type;
                    pDlgIda->szListPath[0] = EOS;
                    pDlgIda->szListName[0] = EOS;
                    DIALOGBOX( hwnd, LSTEDITLISTSDLG, hResMod, ID_LISTEDIT_NEW_EXCL_DLG,
                               pDlgIda, fOK );
                    UtlAlloc( (PVOID *) &pDlgIda, 0L, 0L, NOMSG );
                  } /* endif */
                }
                break;
            } /* endswitch */
            break;

          case PID_FILE_MI_OPEN:
            if ( pszObjName )
            {
              /********************************************************/
              /* Check if list is currently in use                    */
              /********************************************************/
              if ( QUERYSYMBOL( pszObjName ) != -1 )
              {
                UtlError( ERROR_LST_IN_USE, MB_CANCEL, 1, &pszListName, EQF_INFO );
              }
              else
              {
                switch ( pIda->Type )
                {
                  case NTL_TYPE :
                  case FTL_TYPE :
                    {
                      PWWLDLGIDA    pDlgIda;                 // pointer to dialog IDA

                      UtlAlloc( (PVOID *) &pDlgIda, 0L, (LONG)sizeof(WWLDLGIDA), ERROR_STORAGE );
                      if ( pDlgIda )
                      {
                        BOOL   fOK;
                        HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);

                        pDlgIda->usListType = (USHORT)pIda->Type;
                        strcpy( pDlgIda->szListPath, pszObjName );
                        strcpy( pDlgIda->szListName,
                                UtlParseX15( pCommArea->szBuffer, LST_NAME_IND ) );
                        DIALOGBOX( hwnd, LSTWORKWITHLISTSDLG, hResMod, ID_LISTWORK_NTL_DLG,
                                   pDlgIda, fOK );
                        UtlAlloc( (PVOID *) &pDlgIda, 0L, 0L, NOMSG );
                      } /* endif */
                    }
                    break;

                  case EXCL_TYPE:
                  case NOISE_TYPE:
                    {
                      PLISTEDITIDA   pDlgIda;        // pointer to dialog IDA
                      BOOL           fOK;

                      UtlAlloc( (PVOID *) &pDlgIda, 0L, (LONG)sizeof(LISTEDITIDA), ERROR_STORAGE );
                      if ( pDlgIda )
                      {
                        HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
                        pDlgIda->usListType = (USHORT)pIda->Type;
                        strcpy( pDlgIda->szListPath, pszObjName );
                        strcpy( pDlgIda->szListName,
                                UtlParseX15( pCommArea->szBuffer, LST_NAME_IND ) );
                        DIALOGBOX( hwnd, LSTEDITLISTSDLG, hResMod, ID_LISTEDIT_NEW_EXCL_DLG,
                                   pDlgIda, fOK );
                        UtlAlloc( (PVOID *) &pDlgIda, 0L, 0L, NOMSG );
                      } /* endif */
                    }
                    break;
                } /* endswitch */
              } /* endif */

            } /* endif */
            break;

          case PID_FILE_MI_DELETE:
            if ( pszObjName )
            {
              LstDeleteList( pszObjName );
            } /* endif */
            break;

          case PID_FILE_MI_IMPORT:
            {
              BOOL   fOK;
              PSZ    pszList = NULL;

              switch ( pIda->Type )
              {
                case NTL_TYPE :  pszList = NTL_LIST_NAME;       break;
                case FTL_TYPE:   pszList = FTL_LIST_NAME;       break;
                case EXCL_TYPE:  pszList = EXCLUSION_LIST_NAME; break;
              } /* endswitch */
              if ( pszList != NULL )
              {
                HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
                DIALOGBOX( hwnd, LSTIMPORTDLG, hResMod, ID_LISTIMP_NTL_DLG,
                           pszList, fOK  );
              } /* endif */
            }
            break;

          case PID_FILE_MI_EXPORT:
            if ( pszObjName )
            {
              BOOL   fOK;

              /********************************************************/
              /* Check if list is currently in use                    */
              /********************************************************/
              if ( QUERYSYMBOL( pszObjName ) != -1 )
              {
                UtlError( ERROR_LST_IN_USE, MB_CANCEL, 1, &pszListName, EQF_INFO );
                fOK = FALSE;
              }
              else
              {
                HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
                DIALOGBOX( hwnd, LSTEXPORTDLG, hResMod, ID_LISTEXP_NTL_DLG,
                           pszObjName, fOK );
              } /* endif */
            } /* endif */
            break;

          case PID_FILE_MI_PRINT:
            if ( pszObjName )
            {
              /********************************************************/
              /* Check if list is currently in use                    */
              /********************************************************/
              if ( QUERYSYMBOL( pszObjName ) != -1 )
              {
                UtlError( ERROR_LST_IN_USE, MB_CANCEL, 1, &pszListName, EQF_INFO );
              }
              else
              {
                LstPrintList( pszObjName, (USHORT)pIda->Type );
              } /* endif */
            } /* endif */
            break;

          case PID_FILE_MI_PRINTLIST:
            // pass message to column listbox control
            WinSendMsg( pCommArea->hwndLB, msg, mp1, mp2 );
            break;

          default:
            if( msg == WM_EQF_COMMAND)
            {
              mResult = MRFROMSHORT( FALSE ); // tell twbmain that we rejected
            } /* endif */
        } /* endswitch */
      }
      break;

    default:
      break;
  } /* endswitch */
  return( mResult );
} /* end of function ListCallBack */

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     LstEditAddenda  Edit addenda dictionary
//------------------------------------------------------------------------------
// Description:       Calls the edit list dialog for the addenda dictionary
//------------------------------------------------------------------------------
// Input parameter:   PSZ     pszLanguage   name of language
//------------------------------------------------------------------------------
// Returncode type:   BOOL    (FALSE in case of errors)
//------------------------------------------------------------------------------
BOOL LstEditAddenda
(
  PSZ pszLanguage                      // language to be processed
)
{
  PLISTEDITIDA   pDlgIda;        // pointer to dialog IDA
  BOOL           fOK;
  HWND           hwnd;

  fOK = UtlAlloc( (PVOID *)&pDlgIda, 0L, (LONG)sizeof(LISTEDITIDA),
                  ERROR_STORAGE );
  if ( fOK )
  {
    HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
    hwnd = (HWND)UtlQueryULong( QL_TWBCLIENT );
    pDlgIda->usListType = ADD_TYPE;
    strcpy( pDlgIda->szListPath, pszLanguage );
    strcpy( pDlgIda->szListName, pszLanguage );
    DIALOGBOX( hwnd, LSTEDITLISTSDLG, hResMod, ID_LISTEDIT_NEW_EXCL_DLG,
               pDlgIda, fOK );
    UtlAlloc( (PVOID *)&pDlgIda, 0L, 0L, NOMSG );
  } /* endif */
  return( fOK );
} /* end of function LstEditAddenda */

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     LstEditAbbrev   Edit abbreviation dictionary
//------------------------------------------------------------------------------
// Description:       Calls the edit list dialog for the abbreviation dict.
//------------------------------------------------------------------------------
// Input parameter:   PSZ     pszLanguage   name of language
//------------------------------------------------------------------------------
// Returncode type:   BOOL    (FALSE in case of errors)
//------------------------------------------------------------------------------
BOOL LstEditAbbrev
(
  PSZ pszLanguage                      // language to be processed
)
{
  PLISTEDITIDA   pDlgIda;        // pointer to dialog IDA
  BOOL           fOK;
  HWND           hwnd;

  fOK = UtlAlloc( (PVOID *)&pDlgIda, 0L, (LONG)sizeof(LISTEDITIDA),
                  ERROR_STORAGE );
  if ( fOK )
  {
    HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
    hwnd = (HWND)UtlQueryULong( QL_TWBCLIENT );
    pDlgIda->usListType = ABR_TYPE;
    strcpy( pDlgIda->szListPath, pszLanguage );
    strcpy( pDlgIda->szListName, pszLanguage );
    DIALOGBOX( hwnd, LSTEDITLISTSDLG, hResMod, ID_LISTEDIT_NEW_EXCL_DLG,
               pDlgIda, fOK );
    UtlAlloc( (PVOID *)&pDlgIda, 0L, 0L, NOMSG );
  } /* endif */
  return( fOK );
} /* end of function LstEditAbbrev */

//------------------------------------------------------------------------------
//                            End of EQFLIST.C
//------------------------------------------------------------------------------
