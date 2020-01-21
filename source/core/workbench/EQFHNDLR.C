//+----------------------------------------------------------------------------+
//|  EQFHNDLR.C - General definitions for EQF handlers                         |
//+----------------------------------------------------------------------------+
//|  Copyright Notice:                                                         |
//|                                                                            |
//|      Copyright (C) 1990-2017, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+
  #define NOEXTRESMOD
  // #define NOEXTICONS
#define INCL_EQF_FOLDER           // folder list and document list functions
#include "eqf.h"                       // General .H for EQF
#include "eqfprogr.h"                // progress indicator defines

#include "eqfstart.id"            // IDs for EQFSTARR resource
#include "eqfdde.h"               // batch mode definitions
#include "eqffol00.h"             // Document List Handler defines
#include <eqf.id>   

/**********************************************************************/
/* Handles of preloaded icons                                         */
/**********************************************************************/
HPOINTER hiconDICTIMP;                 // dictionary import icon
HPOINTER hiconDICTEXP;                 // dictionary export icon
HPOINTER hiconFOLIMP;                  // folder import icon
HPOINTER hiconFOLEXP;                  // folder export icon
HPOINTER hiconTWB;                     // workbench icon
HPOINTER hiconFLL;                     // folder list icon
HPOINTER hiconFOL;                     // folder icon
HPOINTER hiconDOC;                     // document icon
HPOINTER hiconANA;                     // analysis icon
HPOINTER hiconMEM;                     // memory list icon
HPOINTER hiconTAG;                     // tagtable list icon
HPOINTER hiconCOUNT;                   // count icon
HPOINTER hiconTMEMIMP;                 // memory import icon
HPOINTER hiconTMEMEXP;                 // memory export icon
HPOINTER hiconTMEMMERGE;               // memory merge icon
HPOINTER hiconTMEMORG;                 // memory organize icon
HPOINTER hiconMARKUPIMP;               // tagtable import icon
HPOINTER hiconMARKUPEXP;               // tagtable export icon
HPOINTER hiconDICTORG;                 // dictionary organize icon
HPOINTER hiconDICTLIST;                // dicionary list icon
HPOINTER hiconLIST;                    // terminology list icon
HPOINTER hiconEXCLLIST;                    // terminology list icon
HPOINTER hiconNEWLIST;                    // terminology list icon
HPOINTER hiconFOUNDLIST;                    // terminology list icon
HPOINTER hiconDICTPRINT;               // dictionary print icon
HPOINTER hiconDICTDISP;                // dictionary display entry icon
HPOINTER hiconTMM;                     // translation memory maintenance icon

/**********************************************************************/
/* Layout (line spacing) for processwindows (the numbers listed are   */
/* the number of lines to be used for each part of the process window)*/
/**********************************************************************/
typedef struct _PROCWINLAYOUT
{
  LONG            lPreTextLines;      // lines before text control
  LONG            lTextLines;         // lines to be used for text control
  LONG            lPostTextLines;     // lines following text control
  LONG            lPreSliderLines;    // lines before slider control
  LONG            lSliderLines;       // lines to be used for slider control
  LONG            lPostSliderLines;   // lines following slider control
  LONG            lPreEntryLines;     // lines before entry groupbox
  LONG            lEntryLines;        // lines to be used for entry groupbox
  LONG            lPostEntryLines;    // lines following entry groupbox
  LONG            lPreLBHeaderLines;  // lines before listbox header
  LONG            lLBHeaderLines;     // lines to be used for listbox header
  LONG            lPostLBHeaderLines; // lines following listbox header
  LONG            lPreListboxLines;   // lines before listbox control
  LONG            lListboxLines;      // lines to be used for listbox control
  LONG            lPostListboxLines;  // lines following listbox control
} PROCWINLAYOUT, *PPROCWINLAYOUT;

/**********************************************************************/
/* Different layouts for Windows and OS/2 because the Windows slider  */
/* is much smaller as the OS/2 slider ...                             */
/**********************************************************************/
PROCWINLAYOUT ProcWinLayout[] = {
/* Style                  --TEXT--   -SLIDER-   -ENTRY-    LBHEADER   LISTBOX */
/* TEXTONLY         */ {  3, 1, 2,   0, 0, 0,   0, 0, 0,   0, 0, 0,   0, 0, 0 },
/* SLIDERONLY       */ {  0, 0, 0,   2, 2, 2,   0, 0, 0,   0, 0, 0,   0, 0, 0 },
/* TEXTSLIDER       */ {  2, 1, 1,   0, 2, 2,   0, 0, 0,   0, 0, 0,   0, 0, 0 },
/* SLIDERENTRY      */ {  0, 0, 0,   1, 2, 1,   1, 4, 1,   0, 0, 0,   0, 0, 0 },
/* TEXTSLIDERENTRY  */ {  1, 1, 1,   1, 2, 1,   1, 4, 1,   0, 0, 0,   0, 0, 0 },
/* TEXTSLIDERLISTBOX*/ {  1, 1, 1,   0, 2, 1,   0, 0, 0,   1, 1, 0,   0, 4, 0 }
};

static HWND hwndPopupMenu = NULLHANDLE;

/**********************************************************************/
/* Prototypes for internal functions                                  */
/**********************************************************************/
void CheckForUpdate( HWND hwnd, PGENPROCESSINSTIDA  pIda );
void CheckForRefresh();

/*
Messages to be processed by the handler callback function:

  WM_CREATE:        allocate IDA (if any) and store pointer to it at
                    pUserIDA variable in communication area
                    fill the required fields of the communication area

  WM_EQF_SHUTDOWN:  return FALSE is shutdown can be continued,
                    return TRUE is shutdown is not possible
                    if handler callback function returns FALSE,
                    the message ist send to all objects of the classes
                    listed in the asNotifyClassList

  WM_DESTROY:       free any resources allocated by the callback function

  WM_EQFN_TASKDONE: do whatever has to be done for the completed task
  WM_EQF_DELETE:    delete an object ...
  WM_EQF_OPEN:      open an object ...
  WM_EQF_CREATE:    create an object ...
  WM_EQF_INSERTNAMES: fill-in object names ...
  WM_EQF_QUERYSELECTEDNAMES: fill-in selected objects
  WM_EQF_PROCESSTASK: process the task ...
  WM_EQF_DDE_REQUEST: handle DDE (batch mode) tasks

Messages to be processed by the list instance callback function:

  WM_CREATE:        allocate IDA if necessary and store ptr to it in
                    field pUserIda of communication area
                    supply column table for column listbox

  WM_EQF_INITIALIZE: fill column listbox

  WM_EQF_BUILDITEMTEXT: build the item text for the object with the name
                        given in mp2, store the item text in the buffer area
                        of the communication area
                        return TRUE if item text could be build
  WM_CLOSE:         do any handling for close of instance; e.g save current
                    view lists, ...
  WM_EQF_TERMINATE: do any handling for termination of instance; return
                    TRUE if termination is currently not possible

  WM_DESTROY:       free any resources allocated by the callback function

  WM_EQF_ABOUTTOREMOVEDRIVE: return TRUE if drive may not be removed

  WM_INITMENU:      set the actionbar item states

  WM_EQFN_DRIVEREMOVED update listbox accordingly
  WM_EQFN_DRIVEADDED   update listbox accordingly

      case WM_EQFN_DRIVEREMOVED:
      case WM_EQFN_DRIVEADDED:
        pIda = ACCESSWNDIDA( hwnd, PFOLDERLIST_IDA );
        FllLoadFolderNames( pIda->hLBox ); // refresh folder listbox
        return( 0L );
        break;


  WM_EQF_COMMAND:
  WM_COMMAND:       handle commands
                    Note: The command values PID_VIEW_MI_NAMES,
                          PID_VIEW_MI_DETAILS, PID_VIEW_MI_DETAILSDLG,
                          PID_FILE_MI_SELECTALL, and PID_FILE_MI_DESELECTALL
                          are processes by the list instance window and are
                          NOT passed to the callback function
*/


MRESULT EXPENTRY GENERICLISTFRAMEWP( HWND, WINMSG, WPARAM, LPARAM );

#define  SLIDER_INCR             101   // slider increment
/***************************
* Private global variables *
***************************/
static PFNWP pfnwpOldFrameProc;              // address of old frame proc
static PLISTCREATEPARMS pListCreateParms;    // ptr to current list create parms
static PPROCESSCREATEPARMS pProcCreateParms; // ptr to current process create parms
PFNWP            pfnOldListboxProc = NULL;// ptr to original listbox proc
MRESULT APIENTRY ProcessListboxProc( register HWND hwnd, UINT msg,
                                      register WPARAM mp1, LPARAM mp2);
/**********************************************************************/
/**********************************************************************/
/**********************************************************************/
/***                                                                ***/
/***                 Generic handler window procedure               ***/
/***                                                                ***/
/**********************************************************************/
/**********************************************************************/
/**********************************************************************/
MRESULT APIENTRY GENERICHANDLERWP
(
  HWND hwnd,
  WINMSG message,
  WPARAM mp1,
  LPARAM mp2
)
{
  PGENHANDLERIDA   pIda;               // pointer to handler IDA
  MRESULT         mResult = 0;         // result of message processing

  switch( message)
  {
    case WM_CREATE:
      UtlAlloc( (PVOID *)&pIda, 0L, (LONG) sizeof(GENHANDLERIDA), ERROR_STORAGE );
      if ( pIda == NULL )
      {
        mResult = MRFROMSHORT( DO_NOT_CREATE );  // do not create window
      }
      else
      {
        /**************************************************************/
        /* Anchor IDA, remember handler callback function             */
        /**************************************************************/
        ANCHORWNDIDA( hwnd, pIda );
        pIda->hFrame = hwnd;
        {
          LPCREATESTRUCT   pCreateStruct = (LPCREATESTRUCT) PVOIDFROMMP2(mp2);
          PGENHANDLERCREATEPARMS pCreateParms = (PGENHANDLERCREATEPARMS)
                                       pCreateStruct->lpCreateParams;
          pIda->pfnCallBack = pCreateParms->pfnCallBack;
        }
        /**************************************************************/
        /* Call handler callback function for initialisation          */
        /**************************************************************/
        (*pIda->pfnCallBack)( &(pIda->CommArea), hwnd, message, mp1, mp2 );

        /**************************************************************/
        /* Install handler                                            */
        /**************************************************************/
        EqfInstallHandler( pIda->CommArea.szHandlerName, hwnd,
                           pIda->CommArea.sBaseClass );
      } /* endif */
      break;

      case WM_EQF_ABOUTTODELETE:
         pIda = ACCESSWNDIDA( hwnd, PGENHANDLERIDA );
         if ( SHORTFROMMP1( mp1 ) == pIda->CommArea.sBaseClass )
         {
           PSHORT    psNotifyClass;    // ptr for notify class list processing

           mResult = FALSE;
           psNotifyClass = pIda->CommArea.asNotifyClassList;
           while ( !mResult && *psNotifyClass )
           {
             mResult = (MRESULT)EqfSend2AllObjects( *psNotifyClass,
                                                    (USHORT)message, mp1, mp2);
             psNotifyClass++;
           } /* endwhile */
         } /* endif */
         break;

      case WM_EQF_SHUTDOWN:
        pIda = ACCESSWNDIDA( hwnd, PGENHANDLERIDA );
        mResult = (*pIda->pfnCallBack)( &(pIda->CommArea),
                                      hwnd, message, mp1, mp2 );
        if ( !mResult )
        {
          PSHORT    psNotifyClass;    // ptr for notify class list processing

          mResult = FALSE;
          psNotifyClass = pIda->CommArea.asNotifyClassList;
          while ( !mResult && *psNotifyClass )
          {
            mResult = (MRESULT)EqfSend2AllObjects( *psNotifyClass,
                                                   (USHORT)message, mp1, mp2);
            psNotifyClass++;
          } /* endwhile */
        } /* endif */
        break;

      /****************************************************************/
      /* Messages passed directly to the handler callback function    */
      /****************************************************************/
      case WM_EQFN_TASKDONE:
      case WM_EQF_DELETE:
      case WM_EQF_OPEN:
      case WM_EQF_CREATE:
      case WM_EQF_INSERTNAMES:
      case WM_EQF_QUERYSELECTEDNAMES:
      case WM_EQF_PROCESSTASK:
      case WM_EQF_INITIALIZE:
      case WM_EQF_DDE_REQUEST:
        pIda = ACCESSWNDIDA( hwnd, PGENHANDLERIDA );
        mResult = (*pIda->pfnCallBack)( &(pIda->CommArea),
                                      hwnd, message, mp1, mp2 );
        break;


      /****************************************************************/
      /* Messages passed to all objects of the classes listed in the  */
      /* notification class list                                      */
      /****************************************************************/
      case WM_EQFN_CREATED:
      case WM_EQFN_DELETED:
      case WM_EQFN_DELETEDNAME:
      case WM_EQFN_PROPERTIESCHANGED:
      case WM_EQF_ABOUTTOREMOVEDRIVE:
      case WM_EQFN_DRIVEREMOVED:
      case WM_EQFN_DRIVEADDED:
        pIda = ACCESSWNDIDA( hwnd, PGENHANDLERIDA );
        {
          PSHORT    psNotifyClass;    // ptr for notify class list processing

          mResult = FALSE;
          psNotifyClass = pIda->CommArea.asNotifyClassList;
          while ( *psNotifyClass )
          {
            mResult = (MRESULT)EqfSend2AllObjects( *psNotifyClass,
                                                    (USHORT)message, mp1, mp2);
            psNotifyClass++;
          } /* endwhile */
        }
        break;

      case WM_DESTROY:
        pIda = ACCESSWNDIDA( hwnd, PGENHANDLERIDA );
        if ( pIda != NULL )
        {
          (*pIda->pfnCallBack)( &(pIda->CommArea), hwnd, message, mp1, mp2 );
          UtlAlloc( (PVOID *)&pIda, 0L, 0L, NOMSG );
          ANCHORWNDIDA( hwnd, NULL );
        } /* endif */
        break;

      case WM_EQF_TERMINATE:
        pIda = ACCESSWNDIDA( hwnd, PGENHANDLERIDA );
        WinDestroyWindow( pIda->hFrame );
        break;

      case WM_EQF_CREATELISTWINDOW :
        {
          PSZ             pszObj = (PSZ) PVOIDFROMMP2(mp2);
          BOOL            fOK;                 // success flag

          pIda = ACCESSWNDIDA( hwnd, PGENHANDLERIDA );
          fOK = CreateListWindow( pszObj, pIda->CommArea.pfnCallBack, NULL,
                                  (BOOL)SHORTFROMMP1(mp1)  );
          if( fOK )
          {
            mResult = MRFROMSHORT( TRUE );
          }
          else
          {
            mResult = MRFROMSHORT( FALSE );
          } /* endif */
        }
        break;

      //case WM_EQF_READINQUE:
      case WM_EQF_ADDMEMTOLIST:
      case WM_EQF_REMOVEMEMFROMLIST:
          pIda = ACCESSWNDIDA( hwnd, PGENHANDLERIDA );
          if ( pIda != NULL )
          {
            mResult = (*pIda->pfnCallBack)( &(pIda->CommArea), hwnd, message, mp1, mp2 );
          }
          break;

      default:
        /**************************************************************/
        /* Check for additional messages wanted by the callback       */
        /* function                                                   */
        /**************************************************************/
        CheckForRefresh();
        pIda = ACCESSWNDIDA( hwnd, PGENHANDLERIDA );
        if ( pIda != NULL )
        {
          PSHORT   psMsg = pIda->CommArea.asMsgsWanted;

          while ( (*psMsg != 0) && (*psMsg != (SHORT)message) )
          {
            psMsg++;
          } /* endwhile */
          if ( *psMsg != 0 )
          {
            mResult = (*pIda->pfnCallBack)( &(pIda->CommArea), hwnd, message,
                                            mp1, mp2 );
          }
          else
          {
            mResult = WinDefWindowProc( hwnd, message, mp1, mp2 );
          } /* endif */
        }
        else
        {
          mResult = WinDefWindowProc( hwnd, message, mp1, mp2 );
        } /* endif */
        break;

    } /* endswitch */
    return( mResult );
}

/**********************************************************************/
/**********************************************************************/
/**********************************************************************/
/***                                                                ***/
/***           Generic window procedure for list windows            ***/
/***                                                                ***/
/**********************************************************************/
/**********************************************************************/
/**********************************************************************/
MRESULT APIENTRY GENERICLISTWP
(
  HWND hwnd,
  WINMSG message,
  WPARAM mp1,
  LPARAM mp2
)
{
  PGENLISTINSTIDA  pIda;               // pointer to generic list instance area
  MRESULT          mResult = 0L;      // result value of message processing
  BOOL             fOK;
  USHORT usWidth[MAX_VIEW+1];

  switch( message)
  {
    case WM_CREATE:
      fOK = UtlAlloc( (PVOID *)&pIda, 0L, (LONG) sizeof(GENLISTINSTIDA), ERROR_STORAGE );

      if( !fOK )
      {
        mResult = MRFROMSHORT( DO_NOT_CREATE );    // do not create the window
      } /* endif */

      /****************************************************************/
      /* Get list instance callback functions and store it in IDA     */
      /****************************************************************/
      if ( fOK )
      {
        PLISTCREATEPARMS pCreateParms;
        {
          LPCREATESTRUCT    pCreateStruct;
          LPMDICREATESTRUCT pMDICreateStruct;

          pCreateStruct = (LPCREATESTRUCT) PVOIDFROMMP2(mp2);
          pMDICreateStruct = (LPMDICREATESTRUCT) pCreateStruct->lpCreateParams;
          pCreateParms = (PLISTCREATEPARMS) PVOIDFROMMP2(pMDICreateStruct->lParam);
        }
        pIda->pfnCallBack = pCreateParms->pfnCallBack;
        pIda->pvUserData  = pCreateParms->pvUserData;
        pIda->fRestart    = pCreateParms->fRestart;
      } /* endif */

      if ( fOK )
      {
        /**************************************************************/
        /* Under OS/2 PM this message procedure is for the client     */
        /* window of a list window. Under Windows there is not        */
        /* client window and the message procedure processes the      */
        /* frame messages                                             */
        /**************************************************************/
        pIda->hFrame = hwnd;
        QUERYTEXTHWND( pIda->hFrame, pIda->CommArea.szObjName);
      } /* endif */

      /**************************************************************/
      /* Call instance callback function for initialisation         */
      /**************************************************************/
      if ( fOK )
      {
        mResult = (*pIda->pfnCallBack)( &pIda->CommArea, hwnd, message,
                                        mp1, MP2FROMP(pIda->pvUserData) );
        if ( mResult == MRFROMSHORT(DO_NOT_CREATE) )
        {
          fOK = FALSE;
        } /* endif */
      } /* endif */

      if ( fOK )
      {
        ANCHORWNDIDA( hwnd, pIda );
/**********************************************************************/
/* the following sequence is moved to the WM_EQF_INITIALZE case,      */
/* due to the nature of the internal MDI creation process.            */
/* (otherwise we will always get wrong system menus)    KWT0014       */
/**********************************************************************/
//        /**************************************************************/
//        /* disable close option in system menu if requested by        */
//        /* callback function                                          */
//        /**************************************************************/
//        if ( pIda->CommArea.fNoClose )
//        {
//#if defined(_WINDOWS)
//          HMENU hSysMenu;         // handle of system menu
//
//          hSysMenu = GetSystemMenu( pIda->hFrame, FALSE );
//          if ( hSysMenu != NULL )
//          {
//            EnableMenuItem( hSysMenu, SC_CLOSE, MF_GRAYED );
//          } /* endif */
//#else
//          WinSendMsg( WinWindowFromID( pIda->hFrame, FID_SYSMENU), MM_SETITEMATTR,
//                      MPFROM2SHORT( SC_CLOSE, TRUE),
//                      MPFROM2SHORT( MIA_DISABLED, MIA_DISABLED));
//#endif
//        } /* endif */

        /****************************************************************/
        /* Windows: no child system menu and setting of icon required!  */
        /****************************************************************/
        SendMessage( pIda->hFrame, WM_SETICON, (WPARAM)ICON_SMALL,
                     (LPARAM)pIda->CommArea.hIcon );
        EqfRegisterObject( pIda->CommArea.szObjName, hwnd,
                            pIda->CommArea.sListObjClass );
      } /* endif */

      /****************************************************************/
      /* Set flags for handling of WM_EQFN_ messages                  */
      /****************************************************************/
      if ( fOK )
      {
        PSHORT   psMsg = pIda->CommArea.asMsgsWanted;

        while ( *psMsg != 0 )
        {
          switch ( *psMsg )
          {
            case WM_EQFN_PROPERTIESCHANGED :
              pIda->fWantsPropMsg = TRUE;
              break;
            case WM_EQF_QUERYSELECTEDNAMES :
              pIda->fWantsSelectedNamesMsg = TRUE;
              break;
            case WM_EQFN_DELETED :
              pIda->fWantsDelMsg = TRUE;
              break;
            case WM_EQFN_CREATED :
              pIda->fWantsCreateMsg = TRUE;
              break;
          } /* endswitch */
          psMsg++;
        } /* endwhile */
        if ( *psMsg != 0 )
        {
          mResult = (*pIda->pfnCallBack)( &(pIda->CommArea), hwnd, message,
                                          mp1, mp2 );
          return( mResult );
        } /* endif */
      } /* endif */
      return( mResult );               // do not pass to default procedure
      break;

    case WM_CLOSE:
      pIda = ACCESSWNDIDA( hwnd, PGENLISTINSTIDA );
      if ( pIda != NULL )
      {
        if ( !pIda->CommArea.fNoClose )
        {
          /**************************************************************/
          /* Get current view lists and list window position            */
          /**************************************************************/
          WinSendMsg( pIda->CommArea.hwndLB, LM_EQF_QUERYVIEWLIST,
                      MP1FROMSHORT( CURRENT_VIEW ),
                      MP2FROMP( pIda->CommArea.asCurView ) );

          WinSendMsg( pIda->CommArea.hwndLB,
                      WM_EQF_GETCOLUMNWIDTH,
                      (WPARAM)usWidth,
                      NULL );

          memcpy( (pIda->CommArea.asCurViewWidth), ( usWidth),
                   sizeof(pIda->CommArea.asCurViewWidth));


          WinSendMsg( pIda->CommArea.hwndLB, LM_EQF_QUERYVIEWLIST,
                     MP1FROMSHORT( DETAILS_VIEW ),
                     MP2FROMP( pIda->CommArea.asDetailsView ) );
          WinSendMsg( pIda->CommArea.hwndLB, LM_EQF_QUERYVIEWLIST,
                     MP1FROMSHORT( SORT_VIEW ),
                     MP2FROMP( pIda->CommArea.asSortList ) );
          WinSendMsg( pIda->CommArea.hwndLB, LM_EQF_QUERYFILTER,
                     MP1FROMSHORT( 0 ),
                     MP2FROMP( &pIda->CommArea.Filter ) );
          UtlSaveWindowPos( pIda->hFrame, &(pIda->CommArea.swpSizePos) );

          /**************************************************************/
          /* Call instance callback function                            */
          /**************************************************************/
          mResult = (*pIda->pfnCallBack)( &pIda->CommArea, hwnd, message, mp1, mp2 );

          /************************************************************/
          /* End list window                                          */
          /************************************************************/
          if ( mResult != TRUE )
          {
            EqfRemoveObject( TWBFORCE, hwnd);
          } /* endif */
        } /* endif */
      }
      else
      {
        EqfRemoveObject( TWBFORCE, hwnd);
      } /* endif */
      return( mResult );               // do not pass to default procedure

    case WM_DESTROY:
      pIda = ACCESSWNDIDA( hwnd, PGENLISTINSTIDA );
      if ( pIda != NULL )
      {
        /**************************************************************/
        /* Call instance callback function                            */
        /**************************************************************/
        (*pIda->pfnCallBack)( &(pIda->CommArea), hwnd, message, mp1, mp2 );
        UtlAlloc( (PVOID *)&pIda, 0L, 0L, NOMSG );
        ANCHORWNDIDA( hwnd, NULL );
      } /* endif */
      return( mResult );               // do not pass to default procedure
      break;

    case WM_EQF_TERMINATE:

      pIda = ACCESSWNDIDA( hwnd, PGENLISTINSTIDA );

      /**************************************************************/
      /* Get current view lists and list window position            */
      /**************************************************************/
      WinSendMsg( pIda->CommArea.hwndLB, LM_EQF_QUERYVIEWLIST,
                  MP1FROMSHORT( CURRENT_VIEW ),
                  MP2FROMP( pIda->CommArea.asCurView ) );
      WinSendMsg( pIda->CommArea.hwndLB, LM_EQF_QUERYVIEWLIST,
                 MP1FROMSHORT( DETAILS_VIEW ),
                 MP2FROMP( pIda->CommArea.asDetailsView ) );
      WinSendMsg( pIda->CommArea.hwndLB, LM_EQF_QUERYVIEWLIST,
                 MP1FROMSHORT( SORT_VIEW ),
                 MP2FROMP( pIda->CommArea.asSortList ) );


      (BOOL)WinSendMsg( pIda->CommArea.hwndLB,
                                   WM_EQF_GETCOLUMNWIDTH,
                                   (WPARAM)usWidth,
                                   NULL );

      memcpy( (pIda->CommArea.asCurViewWidth), ( usWidth),
              sizeof(pIda->CommArea.asCurViewWidth));

      WinSendMsg( pIda->CommArea.hwndLB, LM_EQF_QUERYFILTER,
                 MP1FROMSHORT( 0 ),
                 MP2FROMP( &pIda->CommArea.Filter ) );
      UtlSaveWindowPos( pIda->hFrame, &(pIda->CommArea.swpSizePos) );

      /**************************************************************/
      /* Call instance callback function                            */
      /**************************************************************/
      mResult = (*pIda->pfnCallBack)( &(pIda->CommArea), hwnd,
                                      message, mp1, mp2 );
      /****************************************************************/
      /* Terminate only if callback function has returned FALSE       */
      /* (= continue termination)                                     */
      /****************************************************************/
      if ( mResult == FALSE )
      {
        EqfActivateInstance( hwnd, FALSE );
        SendMessage( (HWND)UtlQueryULong( QL_TWBCLIENT), WM_MDIDESTROY, MP1FROMHWND(hwnd), 0L );
      } /* endif */
      return( mResult );               // do not pass to default procedure
      break;

    case WM_EQF_INITIALIZE:
      pIda = ACCESSWNDIDA( hwnd, PGENLISTINSTIDA );

/**********************************************************************/
/* the following sequence was moved from the WM_CREATE case,          */
/* due to the nature of the internal MDI creation process.            */
/* (otherwise we will always get wrong system menus)    KWT0014       */
/**********************************************************************/
        /**************************************************************/
        /* disable close option in system menu if requested by        */
        /* callback function                                          */
        /**************************************************************/
        if ( pIda->CommArea.fNoClose )
        {
          HMENU hSysMenu;         // handle of system menu

          hSysMenu = GetSystemMenu( pIda->hFrame, FALSE );
          if ( hSysMenu != NULL )
          {
            EnableMenuItem( hSysMenu, SC_CLOSE, MF_GRAYED );
          } /* endif */
        } /* endif */

      /****************************************************************/
      /* Show window title text                                       */
      /****************************************************************/
      {
        strcpy( pIda->CommArea.szBuffer, pIda->CommArea.szTitle );
        if ( (pIda->CommArea.Filter.asColumn[0] != 0) &&
             (pIda->CommArea.Filter.asOperator[0] != 0) )
        {
          strcat( pIda->CommArea.szBuffer, " [Some]" );
        } /* endif */
        SETTEXTHWND( pIda->hFrame, pIda->CommArea.szBuffer );
      }

      /****************************************************************/
      /* Create column listbox                                        */
      /****************************************************************/
      {
        ULONG  ulStyle;                // style flags for listbox

        if ( pIda->CommArea.fMultipleSel )
        {
          ulStyle = WS_CHILDWINDOW | WS_VISIBLE | LBS_MULTIPLESEL |
                    LBS_OWNERDRAWFIXED |LBS_NOTIFY | LBS_EXTENDEDSEL;
        }
        else
        {
          ulStyle = WS_CHILDWINDOW | WS_VISIBLE | LBS_OWNERDRAWFIXED |
                    LBS_NOTIFY;
        } /* endif */

        pIda->CommArea.hwndLB = WinCreateWindow( hwnd, WC_EQF_CLBCLASS, "",
                          ulStyle, 0,0,0,0,
                          hwnd, HWND_TOP,
                          pIda->CommArea.sListboxID,
                          pIda->CommArea.pColData,
                          NULL);
      }

      /**************************************************************/
      /* Call instance callback function to fill the column         */
      /* listbox                                                    */
      /**************************************************************/
      (*pIda->pfnCallBack)( &pIda->CommArea, hwnd, message, mp1, mp2 );

      if ( QUERYITEMCOUNTHWND( pIda->CommArea.hwndLB) )
      {
        if ( pIda->CommArea.fMultipleSel )
        {
          SELECTITEMMSHWND( pIda->CommArea.hwndLB, 0 );
          DESELECTITEMMSHWND( pIda->CommArea.hwndLB, 0 );
        }
        else
        {
          SELECTITEMHWND( pIda->CommArea.hwndLB, 0 );
        } /* endif */
        SETTOPINDEXHWND( pIda->CommArea.hwndLB, 0 );
      } /* endif */

      {
        /***********************************************************/
        /* set focus to our listbox -- in case we're active        */
        /***********************************************************/
        if ( hwnd == (HWND) SendMessage((HWND) UtlQueryULong(QL_TWBCLIENT),
                                        WM_MDIGETACTIVE, 0, 0L) )
        {
          SetFocus( pIda->CommArea.hwndLB );
        } /* endif */
      }
      /*******************************************************************/
      /* adjust window position if translation workbench window is in    */
      /* maximized state                                                 */
      /*******************************************************************/
      /*******************************************************************/
      /* Keep window inside TWB                                          */
      /*******************************************************************/
      {
        SWP        swpTWB;             // size+pos of translation workbench

        memset( &swpTWB, 0, sizeof(swpTWB) );
        WinQueryWindowPos( (HWND)UtlQueryULong( QL_TWBCLIENT ), &swpTWB );
        if (  swpTWB.cy != 0 )         // only if TWB size is available
        {
          SWP swp;
          EQFSWP2SWP( pIda->CommArea.swpSizePos, swp );
          UtlKeepInTWB( &swp  );
          SWP2EQFSWP( swp, pIda->CommArea.swpSizePos );
        } /* endif */
      }

      /****************************************************************/
      /* Mask minimize flag if not restarted by TWB                   */
      /****************************************************************/
      if ( !pIda->fRestart )
      {
        pIda->CommArea.swpSizePos.fs &= ~EQF_SWP_MINIMIZE;
      } /* endif */
      //--- set document list window postion and size ---
      if ( pIda->CommArea.fInvisible )
      {
        WinSetWindowPos( pIda->hFrame, HWND_TOP,
                pIda->CommArea.swpSizePos.x,  pIda->CommArea.swpSizePos.y,
                pIda->CommArea.swpSizePos.cx, pIda->CommArea.swpSizePos.cy,
                (USHORT)(pIda->CommArea.swpSizePos.fs |
                EQF_SWP_SIZE | EQF_SWP_MOVE));
      }
      else
      {
        WinSetWindowPos( pIda->hFrame, HWND_TOP,
                pIda->CommArea.swpSizePos.x,  pIda->CommArea.swpSizePos.y,
                pIda->CommArea.swpSizePos.cx, pIda->CommArea.swpSizePos.cy,
                (USHORT)(pIda->CommArea.swpSizePos.fs | EQF_SWP_ACTIVATE |
                EQF_SWP_SHOW | EQF_SWP_SIZE | EQF_SWP_MOVE));
      }
      return( mResult );               // do not pass to default procedure
      break;

    case WM_EQFN_PROPERTIESCHANGED:
      pIda = ACCESSWNDIDA( hwnd, PGENLISTINSTIDA );
      if ( pIda->fWantsPropMsg )
      {
        mResult = (*pIda->pfnCallBack)( &pIda->CommArea, hwnd, message, mp1, mp2 );
      }
      else if ( SHORT1FROMMP1( mp1 ) == pIda->CommArea.sItemPropClass )
      {
        SHORT      sItem;
        PSZ        pszObjName = (PSZ) PVOIDFROMMP2( mp2 );
        BOOL       fUpdate = FALSE;


        /**************************************************************/
        /* Search object name of item in our listbox                  */
        /**************************************************************/
        strcpy( pIda->CommArea.szBuffer, pszObjName );
        strcat( pIda->CommArea.szBuffer, X15_STR );
        sItem = SEARCHITEMHWND( pIda->CommArea.hwndLB, pIda->CommArea.szBuffer );

        /**************************************************************/
        /* If found, leave it to callback function to supply the new  */
        /* item text for the item                                     */
        /**************************************************************/
        if ( sItem != LIT_NONE )
        {
          fUpdate = (BOOL)(*pIda->pfnCallBack)( &pIda->CommArea, hwnd,
                                           WM_EQF_BUILDITEMTEXT,
                                           MP1FROMSHORT(0),
                                           MP2FROMP( pszObjName ) );
        } /* endif */

        /**************************************************************/
        /* Update item with new item text                             */
        /**************************************************************/
        if ( fUpdate )
        {
           BOOL fDisabled = (fUpdate & 4) != 0;
          // using SETITEMTEXT on list view corrupts sometimes the displayed
          // elements, use own setitemtext message instead
          SendMessage( pIda->CommArea.hwndLB, LM_EQF_SETITEMTEXT, MP1FROMSHORT(sItem),
                       MP2FROMP(pIda->CommArea.szBuffer) );
          if ( fDisabled )
          {
            WinSendMsg( pIda->CommArea.hwndLB, LM_EQF_SETITEMSTATE, MP1FROMSHORT( sItem ), MP2FROMSHORT( FALSE ) );
          } /* endif */  
          //SETITEMTEXTHWND( pIda->CommArea.hwndLB, sItem, pIda->CommArea.szBuffer );
        } /* endif */
      } /* endif */
      return( mResult );               // do not pass to default procedure
      break;

    case WM_EQFN_CREATED:
      pIda = ACCESSWNDIDA( hwnd, PGENLISTINSTIDA );
      if ( pIda->fWantsCreateMsg )
      {
        mResult = (*pIda->pfnCallBack)( &pIda->CommArea, hwnd, message, mp1, mp2 );
      }
      else if ( SHORT1FROMMP1( mp1 ) == pIda->CommArea.sItemClass )
      {
        PSZ        pszObjName = (PSZ) PVOIDFROMMP2( mp2 );
        BOOL       fInsert;

        /**************************************************************/
        /* Leave it to callback function to supply the text for the   */
        /* new item                                                   */
        /**************************************************************/
        fInsert = (BOOL)(*pIda->pfnCallBack)( &pIda->CommArea, hwnd,
                                         WM_EQF_BUILDITEMTEXT,
                                         MP1FROMSHORT(0),
                                         MP2FROMP( pszObjName ) );

        /**************************************************************/
        /* Insert new item                                            */
        /**************************************************************/
        if ( fInsert )
        {
          INSERTITEMHWND( pIda->CommArea.hwndLB, pIda->CommArea.szBuffer );
          if ( QUERYITEMCOUNTHWND( pIda->CommArea.hwndLB ) == 1 )
          {
            /*****************************************************/
            /* Object is first item in listbox, select and       */
            /* deselect item for sake of keyboard navigation ... */
            /*****************************************************/
            SELECTITEMHWND( pIda->CommArea.hwndLB, 0 );
            DESELECTITEMHWND( pIda->CommArea.hwndLB, 0 );
          } /* endif */
        } /* endif */
      } /* endif */
      return( mResult );               // do not pass to default procedure
      break;

    case WM_EQF_ABOUTTOREMOVEDRIVE:
    case WM_EQFN_DRIVEREMOVED:
    case WM_EQFN_DRIVEADDED:
      pIda = ACCESSWNDIDA( hwnd, PGENLISTINSTIDA );
      mResult = (*pIda->pfnCallBack)( &pIda->CommArea, hwnd, message,
                                           mp1, mp2 );
      return( mResult );               // do not pass to default procedure
      break;

    case WM_EQFN_DELETED:
      pIda = ACCESSWNDIDA( hwnd, PGENLISTINSTIDA );
      if ( pIda->fWantsDelMsg )
      {
        mResult = (*pIda->pfnCallBack)( &pIda->CommArea, hwnd, message, mp1, mp2 );
      }
      else if ( SHORT1FROMMP1( mp1 ) == pIda->CommArea.sItemClass )
      {
        SHORT      sItem;
        PSZ        pszObjName = (PSZ) PVOIDFROMMP2( mp2 );

        /**************************************************************/
        /* Search object name of item in our listbox                  */
        /**************************************************************/
        strcpy( pIda->CommArea.szBuffer, pszObjName );
        strcat( pIda->CommArea.szBuffer, X15_STR );
        sItem = SEARCHITEMHWND( pIda->CommArea.hwndLB, pIda->CommArea.szBuffer );

        /**************************************************************/
        /* If found, remove item from listbox                         */
        /**************************************************************/
        if ( sItem != LIT_NONE )
        {
          DELETEITEMHWND( pIda->CommArea.hwndLB, sItem );
        } /* endif */
      } /* endif */
      return( mResult );               // do not pass to default procedure
      break;

    case WM_EQFN_DELETEDNAME:
      pIda = ACCESSWNDIDA( hwnd, PGENLISTINSTIDA );
      if ( pIda->fWantsDelMsg )
      {
        mResult = (*pIda->pfnCallBack)( &pIda->CommArea, hwnd, message, mp1, mp2 );
      } /* endif */
      return( mResult );               // do not pass to default procedure
      break;

    case WM_EQF_QUERYSELECTEDNAMES:
      pIda = ACCESSWNDIDA( hwnd, PGENLISTINSTIDA );
      if ( pIda )
      {
        if ( pIda->fWantsSelectedNamesMsg )
        {
          mResult = (*pIda->pfnCallBack)( &pIda->CommArea, hwnd, message, mp1, mp2 );
        }
        else
        {
          HWND       hwndTargetLB = HWNDFROMMP1(mp1);
          SHORT      sItem = LIT_FIRST;

          sItem = QUERYNEXTSELECTIONHWND( pIda->CommArea.hwndLB, sItem );
          while ( sItem != LIT_NONE )
          {
            // Win2000 corrupts our data if we use LB_GETTEXT so use own message instead
//          QUERYITEMTEXTHWN( pIda->CommArea.hwndLB, sItem, pIda->CommArea.szBuffer  );
            SendMessage( pIda->CommArea.hwndLB, LM_EQF_QUERYITEMTEXT, (WPARAM)sItem, (LPARAM)pIda->CommArea.szBuffer );
            INSERTITEMHWND( hwndTargetLB, UtlParseX15( pIda->CommArea.szBuffer,
                                                     pIda->CommArea.sNameIndex ));
            sItem = QUERYNEXTSELECTIONHWND( pIda->CommArea.hwndLB, sItem );
          } /* endwhile */
          mResult = MRFROMSHORT( QUERYITEMCOUNTHWND( hwndTargetLB ) );
        } /* endif */
      } /* endif */
      return( mResult );               // do not pass to default procedure
      break;

    case WM_EQF_INSERTNAMES:
      pIda = ACCESSWNDIDA( hwnd, PGENLISTINSTIDA );
      if ( pIda )
      {
        HWND       hwndTargetLB = HWNDFROMMP1(mp1);
        SHORT      sItemCount;
        SHORT      sIndex;
        BOOL       fCombo;


        ISCOMBOBOX( hwndTargetLB, fCombo );

        sItemCount = QUERYITEMCOUNTHWND( pIda->CommArea.hwndLB );

        for ( sIndex = 0; sIndex < sItemCount; sIndex++ )
        {
          if ( (BOOL)WinSendMsg( pIda->CommArea.hwndLB,
                                 LM_EQF_QUERYITEMSTATE,
                                 MP1FROMSHORT( sIndex ),
                                 MP2FROMP(NULL) ) )
          {
            // Win2000 corrupts our data if we use LB_GETTEXT so use own message instead
//          QUERYITEMTEXTHWN( pIda->CommArea.hwndLB, sIndex, pIda->CommArea.szBuffer );
            SendMessage( pIda->CommArea.hwndLB, LM_EQF_QUERYITEMTEXT, (WPARAM)sIndex, (LPARAM)pIda->CommArea.szBuffer  );
            if ( fCombo )
            {
              CBINSERTITEMHWND( hwndTargetLB,
                                UtlParseX15( pIda->CommArea.szBuffer,
                                             pIda->CommArea.sNameIndex ) );
            }
            else
            {
              INSERTITEMHWND( hwndTargetLB,
                              UtlParseX15( pIda->CommArea.szBuffer,
                                           pIda->CommArea.sNameIndex ) );
            } /* endif */
          } /* endif */
        } /* endfor */
        if ( fCombo )
        {
          mResult = MRFROMSHORT( CBQUERYITEMCOUNTHWND( hwndTargetLB ) );
        }
        else
        {
          mResult = MRFROMSHORT( QUERYITEMCOUNTHWND( hwndTargetLB ) );
        } /* endif */
      } /* endif */
      return( mResult );               // do not pass to default procedure
      break;

    case WM_EQFN_OBJECTREMOVED:
      pIda = ACCESSWNDIDA( hwnd, PGENLISTINSTIDA );
      return( mResult );               // do not pass to default procedure
      break;

    case WM_EQF_INITMENU:
    case WM_INITMENU:
    case WM_INITMENUPOPUP:
      pIda = ACCESSWNDIDA( hwnd, PGENLISTINSTIDA );
      /**************************************************************/
      /* Call instance callback function to set the enable state    */
      /* of actionbar items                                         */
      /**************************************************************/
      mResult = (*pIda->pfnCallBack)( &pIda->CommArea,
                                           hwnd, message, mp1, mp2 );
      return( mResult );               // do not pass to default procedure
      break;

    case WM_EQF_COMMAND:
    case WM_COMMAND:
      pIda = ACCESSWNDIDA( hwnd, PGENLISTINSTIDA );
      mResult = MRFROMSHORT( TRUE );   // default for mResult is TRUE
      switch (SHORT1FROMMP1(mp1))
      {
        case PID_VIEW_MI_NAMES:
        case PID_VIEW_MI_DETAILS:
        case PID_VIEW_MI_DETAILSDLG:
        case PID_VIEW_MI_SORT:
        case PID_VIEW_MI_SOME:
        case PID_VIEW_MI_ALL:
          WinSendMsg( pIda->CommArea.hwndLB, message, mp1, mp2 );
          break;
        case PID_FILE_MI_SELECTALL:
          SELECTALLHWND( pIda->CommArea.hwndLB );
          break;
        case PID_FILE_MI_DESELECTALL:
          DESELECTALLHWND( pIda->CommArea.hwndLB );
          break;
        default:
          mResult = (*pIda->pfnCallBack)( &pIda->CommArea,
                                               hwnd, message, mp1, mp2 );
          break;
      } /* endswitch */

      if( message == WM_COMMAND)
      {
        mResult = DefMDIChildProc( hwnd, message, mp1, mp2 );
      } /* endif */
      return( mResult );               // do not pass to default procedure
      break;

      /*****************************************************************/
      /* allow for a doubleclick for default action on object          */
      /* this function is available due to the extended selection lb.  */
      /* Implementation: Simulate a selection of default action from   */
      /* within the file pulldown...                                   */
      /*****************************************************************/
    case WM_BUTTON1DBLCLK:
      pIda = ACCESSWNDIDA( hwnd, PGENLISTINSTIDA );

      /****************************************************************/
      /* If currently selected item is not disabled and a default     */
      /* action has been set call callback function to handle         */
      /* default action                                               */
      /****************************************************************/
      {
        SHORT      sItem = QUERYSELECTIONHWND( pIda->CommArea.hwndLB );

        if ( (sItem != LIT_NONE) && (pIda->CommArea.sDefaultAction != 0) )
        {
          if ( WinSendMsg( pIda->CommArea.hwndLB, LM_EQF_QUERYITEMSTATE,
                           MP1FROMSHORT(sItem), 0L ) )
          {
            mResult = (*pIda->pfnCallBack)( &pIda->CommArea,
                             hwnd,
                             WM_EQF_COMMAND,
                             MP1FROMSHORT(pIda->CommArea.sDefaultAction),
                             MP2FROMP(NULL) );
            return( mResult );         // do not pass to default procedure
          } /* endif */
        } /* endif */
      }
      break;

    case WM_SIZE:
      pIda = ACCESSWNDIDA( hwnd, PGENLISTINSTIDA );
      // resize inner window only if normal sizing request...
      if ( (pIda->CommArea.hwndLB != NULLHANDLE) &&
           ((mp1 == SIZENORMAL) || (mp1 == SIZEFULLSCREEN)) )
      {
        MoveWindow( pIda->CommArea.hwndLB,
                    0, 0,
                    LOWORD( mp2 ), HIWORD( mp2 ) ,
                    TRUE );
      }
      break;


    case WM_SETFOCUS:
      pIda = ACCESSWNDIDA( hwnd, PGENLISTINSTIDA );
      if ( pIda != NULL )
      {
        WinPostMsg( hwnd, WM_EQF_SETFOCUS, 0L, MP2FROMHWND( pIda->CommArea.hwndLB ) );
      } /* endif */
      break;


    case WM_MDIACTIVATE :
      EqfActivateInstance( hwnd, (hwnd == (HWND)mp2) );
      if ( hwnd == (HWND)mp2 )
      {
        pIda = ACCESSWNDIDA( hwnd, PGENLISTINSTIDA );
        if ( pIda != NULL )
        {
          WinPostMsg( hwnd, WM_EQF_SETFOCUS, 0L, MP2FROMHWND( pIda->CommArea.hwndLB ) );
        } /* endif */
      } /* endif */
      /*************************************************************/
      /* update the Toolbar if available....                       */
      /*************************************************************/
      return( MRFROMSHORT(0) );
      break;

    case WM_ACTIVATE:
      EqfActivateInstance( hwnd, (mp1 != WA_INACTIVE) );
      return( MRFROMSHORT(0) );
      break;

    case WM_EQF_SETFOCUS:
      if ( mp2 )
      {
        SETFOCUSHWND( HWNDFROMMP2(mp2) );
      } /* endif */
      return( 0L );
      break;

    case WM_QUERYDRAGICON:
      pIda = ACCESSWNDIDA( hwnd, PGENLISTINSTIDA );
      if ( pIda != NULL )
      {
        return( (MRESULT)pIda->CommArea.hIcon  );
      }
      else
      {
        return( 0L );
      } /* endif */
      break;

    case WM_PAINT:
      {
        PAINTSTRUCT ps;

        BeginPaint( hwnd, &ps );
        if ( IsIconic( hwnd ) )
        {
          pIda = ACCESSWNDIDA( hwnd, PGENLISTINSTIDA );
          if ( pIda != NULL )
          {
            SendMessage( hwnd, WM_ICONERASEBKGND, (WPARAM)ps.hdc, 0L );
            DrawIcon( ps.hdc, 0, 0, pIda->CommArea.hIcon );
          } /* endif */
        } /* endif */
        EndPaint( hwnd, &ps );
      }
      break;

    case WM_ERASEBKGND:
      if ( IsIconic(hwnd) )
        return TRUE;
      else
        break;

    case WM_ICONERASEBKGND:
      {
        POINT pt = { 0, 0 };
        LONG lResult;
        HWND hwndClient = (HWND)UtlQueryULong( QL_TWBCLIENT );

        // adjust the hdc for the MDI client window and
        // have the MDI client erase the icon background
        // with the background bitmap or color
        ClientToScreen( hwnd, &pt );
        ScreenToClient( hwndClient, &pt );
        OffsetViewportOrgEx( (HDC)mp1, -pt.x, -pt.y, NULL );
        OffsetClipRgn( (HDC)mp1, pt.x, pt.y );

        // note reverse polarity in return values from WM_ERASEBKGND
        // and WM_ICONERASEBKGND
        lResult = !SendMessage( hwndClient, WM_ERASEBKGND, mp1, mp2 );

        // restore viewport extents and clipping offset
        OffsetViewportOrgEx( (HDC)mp1, pt.x, pt.y, NULL );
        OffsetClipRgn( (HDC)mp1, -pt.x, -pt.y );

        return lResult;
      }
      break;

    /******************************************************************/
    /* The following message is posted to the list window by the      */
    /* superclassed listbox inside the column listbox                 */
    /*                                                                */
    /* Attention: this NOT a message send by Windows                  */
    /******************************************************************/
    case WM_KEYDOWN:
      if ( mp1 == VK_F1 )
      {
        /**************************************************************/
        /* Trigger help by posting HM_HELPSUBITEM_NOT_FOUND to TWB    */
        /**************************************************************/
        pIda = ACCESSWNDIDA( hwnd, PGENLISTINSTIDA );
        PostMessage( (HWND)UtlQueryULong( QL_TWBFRAME ),
                     HM_HELPSUBITEM_NOT_FOUND,
                     0,
                     MP2FROM2SHORT( pIda->CommArea.sListWindowID,
                                    pIda->CommArea.sListboxID ) );
      } /* endif */
      break;

      case WM_EQF_SHOWPOPUP:
        /**************************************************************/
        /* Ensure that the list window is active                      */
        /**************************************************************/
        {
          HWND     hwndActive;         // handle of active MDI window
          MRESULT  mReturn;            // return value of WM_MDIGETACTIVE

          mReturn = SendMessage( (HWND)UtlQueryULong( QL_TWBCLIENT ),
                                 WM_MDIGETACTIVE, MP1FROMSHORT(0), MP2FROMSHORT(0) );
          hwndActive = (HWND)mReturn;
          if ( hwndActive != hwnd )
          {
            SendMessage( (HWND)UtlQueryULong( QL_TWBCLIENT ),
                         WM_MDIACTIVATE, MP1FROMHWND(hwnd), MP2FROMSHORT(0) );
          } /* endif */
        }

        /**************************************************************/
        /* Show the popup menu                                        */
        /**************************************************************/
        pIda = ACCESSWNDIDA( hwnd, PGENLISTINSTIDA );
        {
          SHORT    sPopupID;           // ID of popup menu
          SHORT    sItem;              // listbox item index

          int iSelItems = SendMessage( pIda->CommArea.hwndLB, LB_GETSELCOUNT, 0, 0 );
          sItem = QUERYNEXTSELECTIONHWND( pIda->CommArea.hwndLB, LIT_FIRST );

          if ( iSelItems > 1 )
          {
            if ( pIda->CommArea.sMultPopupMenuID )
            {
              sPopupID = pIda->CommArea.sMultPopupMenuID;
            }
            else
            {
              sPopupID = pIda->CommArea.sPopupMenuID;
            } /* endif */
          }
          else
          if ( sItem != LIT_NONE )
          {
            if ( CLBQUERYITEMSTATEHWND( pIda->CommArea.hwndLB, sItem) )
            {
              sPopupID = pIda->CommArea.sPopupMenuID;
            }
            else
            {
              sPopupID = pIda->CommArea.sGreyedPopupMenuID;
            } /* endif */
          }
          else
          {
            sPopupID = pIda->CommArea.sNoSelPopupMenuID;
          } /* endif */

          if ( sPopupID != -1 )
          {
            POINT pt;
            pt.x = LOWORD(mp2);
            pt.y = HIWORD(mp2);
            HandlePopupMenu( HWNDFROMMP1(mp1), pt, sPopupID );
          } /* endif */
        }
        break;

      default:
        /**************************************************************/
        /* Check for additional messages wanted by the callback       */
        /* function                                                   */
        /**************************************************************/
        pIda = ACCESSWNDIDA( hwnd, PGENLISTINSTIDA );
        if ( pIda != NULL )
        {
          PSHORT   psMsg = pIda->CommArea.asMsgsWanted;

          while ( (*psMsg != 0) && (*psMsg != (SHORT)message) )
          {
            psMsg++;
          } /* endwhile */
          if ( *psMsg != 0 )
          {
            mResult = (*pIda->pfnCallBack)( &(pIda->CommArea), hwnd, message,
                                            mp1, mp2 );
            return( mResult );
          } /* endif */
        } /* endif */
        break;
  } /* end switch( message ) */
  // pass all messages to default procedure
  mResult = DefMDIChildProc( hwnd, message, mp1, mp2 );
  return( mResult );
}

/**********************************************************************/
/* Subclass procedure for frame of list instance windows              */
/* Windows: Subclassing of frame not necessary ... we are             */
/* dealing with real supported MDI                                    */
/**********************************************************************/

/**********************************************************************/
/**********************************************************************/
/**********************************************************************/
/***                                                                ***/
/***       Generic window procedure for process windows             ***/
/***                                                                ***/
/**********************************************************************/
/**********************************************************************/
/**********************************************************************/
MRESULT APIENTRY GENERICPROCESSWP
(
  HWND hwnd,
  WINMSG message,
  WPARAM mp1,
  LPARAM mp2
)
{
  PGENPROCESSINSTIDA  pIda;            // pointer to generic process instance area
  MRESULT          mResult = 0L;       // result value of message processing

  BOOL             fOK;

  switch( message)
  {
    case WM_CREATE:
      fOK = UtlAlloc( (PVOID *)&pIda, 0L, (LONG) sizeof(GENPROCESSINSTIDA), ERROR_STORAGE );

      if( !fOK )
      {
        mResult = MRFROMSHORT( DO_NOT_CREATE );    // do not create the window
      } /* endif */

      /****************************************************************/
      /* Get process instance callback functions and store it in IDA  */
      /****************************************************************/
      if ( fOK )
      {
        PPROCESSCREATEPARMS pCreateParms;
        {
          LPCREATESTRUCT    pCreateStruct;
          LPMDICREATESTRUCT pMDICreateStruct;

          pCreateStruct = (LPCREATESTRUCT) PVOIDFROMMP2(mp2);
          pMDICreateStruct = (LPMDICREATESTRUCT) pCreateStruct->lpCreateParams;
          pCreateParms = (PPROCESSCREATEPARMS) PVOIDFROMMP2(pMDICreateStruct->lParam);
        }
        pIda->pfnCallBack = pCreateParms->pfnCallBack;
        pIda->pvUserData  = pCreateParms->pvUserData;
        pIda->fVisible    = pCreateParms->fVisible;
      } /* endif */

      if ( fOK )
      {
        fOK = CreateProcessControls( hwnd, pIda, mp1 );
        /*************************************************************/
        /* Register the process window                               */
        /*************************************************************/
        if ( fOK && !pIda->CommArea.fDoNotRegisterObject )
        {
          EqfRegisterObject( pIda->CommArea.szObjName, hwnd,
                             pIda->CommArea.sProcessObjClass );
        } /* endif */

      } /* endif */

      return( mResult );               // do not pass to default procedure
      break;
    case WM_EQF_SETFOCUS:
      pIda = ACCESSWNDIDA( hwnd, PGENPROCESSINSTIDA );
      if ( pIda->hwndSlider )
      {
        SetFocus( pIda->hwndSlider );
      } /* endif */
      UtlDispatch();
      return ( 0L );

    /******************************************************************/
    /* The following message is posted to the process window by the   */
    /* control having the input foxus inside the process window       */
    /*                                                                */
    /* Attention: this is NOT a message send by Windows               */
    /******************************************************************/
    case WM_KEYDOWN:
      if ( mp1 == VK_F1 )
      {
        /**************************************************************/
        /* Trigger help by posting HM_HELPSUBITEM_NOT_FOUND to TWB    */
        /**************************************************************/
        pIda = ACCESSWNDIDA( hwnd, PGENPROCESSINSTIDA );
        PostMessage( (HWND)UtlQueryULong( QL_TWBFRAME ),
                     HM_HELPSUBITEM_NOT_FOUND,
                     0,
                     MP2FROM2SHORT( pIda->CommArea.sProcessWindowID, 0 ) );
      } /* endif */
      break;

    case WM_CLOSE:
      pIda = ACCESSWNDIDA( hwnd, PGENPROCESSINSTIDA );
      if ( pIda != NULL )
      {
        BOOL   fNoClose;

        /**************************************************************/
        /* Call instance callback function                            */
        /**************************************************************/
        fNoClose = (BOOL)(*pIda->pfnCallBack)( &pIda->CommArea,
                                               hwnd, message, mp1, mp2 );
        if ( !fNoClose )
        {
          EqfRemoveObject( TWBFORCE, hwnd);
        } /* endif */
      }
      else
      {
        EqfRemoveObject( TWBFORCE, hwnd);
      } /* endif */
      return( 0L );               // = do not pass to default procedure
      break;

    case WM_DESTROY:
      pIda = ACCESSWNDIDA( hwnd, PGENPROCESSINSTIDA );
      if ( pIda != NULL )
      {
        /**************************************************************/
        /* Call instance callback function                            */
        /**************************************************************/
        (*pIda->pfnCallBack)( &(pIda->CommArea), hwnd, message, mp1, mp2 );
        UtlAlloc( (PVOID *)&pIda, 0L, 0L, NOMSG );
        ANCHORWNDIDA( hwnd, NULL );
      } /* endif */
      return( mResult );               // do not pass to default procedure
      break;

    case WM_EQF_TERMINATE:
      pIda = ACCESSWNDIDA( hwnd, PGENPROCESSINSTIDA );

      /**************************************************************/
      /* Call instance callback function                            */
      /**************************************************************/
      mResult = (*pIda->pfnCallBack)( &(pIda->CommArea), hwnd,
                                      message, mp1, mp2 );
      /****************************************************************/
      /* Terminate only if callback function has returned FALSE       */
      /* (= continue termination)                                     */
      /****************************************************************/
      if ( mResult == FALSE )
      {
        if ( pIda->CommArea.Style != PROCWIN_BATCH )
        {
          EqfActivateInstance( hwnd, FALSE );
        } /* endif */
        SendMessage( (HWND)UtlQueryULong( QL_TWBCLIENT), WM_MDIDESTROY, MP1FROMHWND(hwnd), 0L );
      } /* endif */
      return( mResult );               // do not pass to default procedure
      break;

    case WM_EQF_INITIALIZE:
      pIda = ACCESSWNDIDA( hwnd, PGENPROCESSINSTIDA );

      /****************************************************************/
      /* Show window title text                                       */
      /****************************************************************/
      strcpy( pIda->szCurTitle, pIda->CommArea.szTitle );
      SETTEXTHWND( pIda->hFrame, pIda->CommArea.szTitle );
      /*******************************************************************/
      /* adjust window position if translation workbench window is in    */
      /* maximized state                                                 */
      /*******************************************************************/
      {
        /***********************************************************/
        /* set focus to our control -- in case we're active        */
        /***********************************************************/
        if ( hwnd == (HWND) SendMessage((HWND) UtlQueryULong(QL_TWBCLIENT),
                                        WM_MDIGETACTIVE, 0, 0L) )
        {
          if ( pIda->hwndSlider )
          {
            SetFocus( pIda->hwndSlider );
          }
          else
          {
            SetFocus( hwnd );
          } /* endif */
        } /* endif */
      }
      /*******************************************************************/
      /* Keep window titlebar within client area                         */
      /*******************************************************************/
      if ( pIda->CommArea.Style != PROCWIN_BATCH )
      {
        SWP        swpWindow;

        WinQueryWindowPos( EqfQueryTwbClient(), &swpWindow );
        if ( (pIda->CommArea.swpSizePos.y +
             pIda->CommArea.swpSizePos.cy) >= swpWindow.cy )
        {
          pIda->CommArea.swpSizePos.y -= pIda->CommArea.swpSizePos.y +
                                         pIda->CommArea.swpSizePos.cy -
                                         swpWindow.cy;
        } /* endif */
      }

      //--- set process window position and size ---
      if ( pIda->fVisible )
      {
        WinSetWindowPos( pIda->hFrame, HWND_TOP,
                pIda->CommArea.swpSizePos.x,  pIda->CommArea.swpSizePos.y,
                pIda->CommArea.swpSizePos.cx, pIda->CommArea.swpSizePos.cy,
                (USHORT)(SWP_FLAG(pIda->CommArea.swpSizePos) | EQF_SWP_ACTIVATE |
                EQF_SWP_SHOW | EQF_SWP_SIZE | EQF_SWP_MOVE));
      }
      else
      {
        WinShowWindow( pIda->hFrame, FALSE );
      } /* endif */

      /****************************************************************/
      /* Give controls some time (some messages) to complete          */
      /* re-painting ...                                              */
      /****************************************************************/
      UtlDispatch();
      pIda = ACCESSWNDIDA( hwnd, PGENPROCESSINSTIDA );

      /**************************************************************/
      /* Call instance callback function                            */
      /**************************************************************/
      if ( pIda != NULL )
      {
        mResult = (*pIda->pfnCallBack)( &(pIda->CommArea), hwnd,
                                        message, mp1, mp2 );
      } /* endif */


      return( mResult );               // do not pass to default procedure
      break;

    case WM_EQF_INITMENU:
    case WM_INITMENU:
    case WM_INITMENUPOPUP:
      pIda = ACCESSWNDIDA( hwnd, PGENPROCESSINSTIDA );
      /**************************************************************/
      /* Call instance callback function to set the enable state    */
      /* of actionbar items                                         */
      /**************************************************************/
      mResult = (*pIda->pfnCallBack)( &pIda->CommArea,
                                           hwnd, WM_INITMENU, mp1, mp2 );
      return( mResult );               // do not pass to default procedure
      break;

    case WM_EQF_COMMAND:
    case WM_COMMAND:
      mResult = (MRESULT) (message != WM_EQF_COMMAND);
      if( message == WM_COMMAND)
      {
        mResult = DefMDIChildProc( hwnd, message, mp1, mp2 );
      } /* endif */
      return( mResult );               // do not pass to default procedure
      break;
    case WM_SIZE:
      {
        SHORT sHeigth = HIWORD(mp2);   // new height of window
        SHORT sSize   = LOWORD(mp2);   // new width size of window

        /**************************************************************/
        /* Windows only: resize/rearrange controls only for normal    */
        /* size requests                                              */
        /**************************************************************/
        if ( (mp1 != SIZENORMAL) && (mp1 != SIZEFULLSCREEN) )
        {
          return( DefMDIChildProc( hwnd, message, mp1, mp2 ) );
        } /* endif */

        pIda = ACCESSWNDIDA( hwnd, PGENPROCESSINSTIDA );
        PositionProcessControls( pIda, sHeigth, sSize );
      }
      break;

    case WM_MDIACTIVATE :
      pIda = ACCESSWNDIDA( hwnd, PGENPROCESSINSTIDA );
      if ( (pIda != NULL) && (pIda->CommArea.Style != PROCWIN_BATCH) )
      {
        EqfActivateInstance( hwnd, mp1 );
      } /* endif */
      if ( hwnd == (HWND)mp2 )
      {
        if ( (pIda != NULL) && (pIda->CommArea.Style != PROCWIN_BATCH) )
        {

          WinPostMsg( hwnd, WM_EQF_SETFOCUS, 0, 0L );
        } /* endif */
      } /* endif */
      return( 0L );
      break;

    case WM_SETFOCUS:
        pIda = ACCESSWNDIDA( hwnd, PGENPROCESSINSTIDA );
        if ( pIda != NULL )
        {
          WinPostMsg( hwnd, WM_EQF_SETFOCUS, 0, 0L );
        } /* endif */
      break;
    case WM_ACTIVATE:
      pIda = ACCESSWNDIDA( hwnd, PGENPROCESSINSTIDA );
      if ( !pIda || (pIda->CommArea.Style != PROCWIN_BATCH) )
      {
        EqfActivateInstance( hwnd, (mp1 != WA_INACTIVE) );
      } /* endif */
      return( 0L );
      break;

    case WM_PAINT:
      {
        PAINTSTRUCT ps;

        BeginPaint( hwnd, &ps );
        if ( IsIconic( hwnd ) )
        {
          pIda = ACCESSWNDIDA( hwnd, PGENPROCESSINSTIDA );
          if ( pIda != NULL )
          {
            SendMessage( hwnd, WM_ICONERASEBKGND, (WPARAM)ps.hdc, 0L );
            DrawIcon( ps.hdc, 0, 0, pIda->CommArea.hIcon );
          } /* endif */
        } /* endif */
        EndPaint( hwnd, &ps );
      }
      break;

    case WM_ERASEBKGND:
      if ( IsIconic(hwnd) )
        return TRUE;
      else
        break;

    case WM_ICONERASEBKGND:
      {
        POINT pt = { 0, 0 };
        LONG lResult;
        HWND hwndClient = (HWND)UtlQueryULong( QL_TWBCLIENT );

        // adjust the hdc for the MDI client window and
        // have the MDI client erase the icon background
        // with the background bitmap or color
        ClientToScreen( hwnd, &pt );
        ScreenToClient( hwndClient, &pt );
        OffsetViewportOrgEx( (HDC)mp1, -pt.x, -pt.y, NULL );
        OffsetClipRgn( (HDC)mp1, pt.x, pt.y );

        // note reverse polarity in return values from WM_ERASEBKGND
        // and WM_ICONERASEBKGND
        lResult = !SendMessage( hwndClient, WM_ERASEBKGND, mp1, mp2 );
        // restore viewport extents and clipping offset
        OffsetViewportOrgEx( (HDC)mp1, pt.x, pt.y, NULL );
        OffsetClipRgn( (HDC)mp1, -pt.x, -pt.y );
        return lResult;
      }
      break;
    case WM_QUERYDRAGICON:
      pIda = ACCESSWNDIDA( hwnd, PGENPROCESSINSTIDA );
      if ( pIda != NULL )
      {
        return( (MRESULT)pIda->CommArea.hIcon );
      }
      else
      {
        return( 0L );
      } /* endif */
      break;

    case WM_EQF_UPDATESLIDER:
      {
        pIda = ACCESSWNDIDA( hwnd, PGENPROCESSINSTIDA );

        if ( pIda && pIda->fVisible && (pIda->CommArea.Style != PROCWIN_BATCH) )
        {
          /**************************************************************/
          /* Update commarea fields using supplied data                 */
          /**************************************************************/
          if ( SHORTFROMMP1(mp1) != -1 )// new slider arm position specified ??
          {
            pIda->CommArea.usComplete = USHORTFROMMP1(mp1);
          } /* endif */

          if ( PVOIDFROMMP2(mp2) != NULL )   // new text string specified ???
          {
            strcpy( pIda->CommArea.szText, (char *) PVOIDFROMMP2(mp2) );
          } /* endif */

          CheckForUpdate( hwnd, pIda );  // handle any updates
        } /* endif */
      }
      break;

    case WM_SETTEXT:
       DefWindowProc( hwnd, message, mp1, mp2 );
       return ( DefMDIChildProc( hwnd, message, mp1, mp2 ) );

    default:
      /**************************************************************/
      /* Check for additional messages wanted by the callback       */
      /* function                                                   */
      /**************************************************************/
      pIda = ACCESSWNDIDA( hwnd, PGENPROCESSINSTIDA );
      if ( pIda != NULL )
      {
        PSHORT   psMsg = pIda->CommArea.asMsgsWanted;

        while ( (*psMsg != 0) && (*psMsg != (SHORT)message) )
        {
          psMsg++;
        } /* endwhile */
        if ( *psMsg != 0 )
        {
          mResult = (*pIda->pfnCallBack)( &(pIda->CommArea), hwnd, message,
                                          mp1, mp2 );
          pIda = ACCESSWNDIDA( hwnd, PGENPROCESSINSTIDA );
          if ( pIda != NULL )
          {
            CheckForUpdate( hwnd, pIda );        // handle any updates
          } /* endif */
        } /* endif */
      } /* endif */
      break;
  } /* end switch( message ) */

  // pass all messages to default procedure
  mResult = DefMDIChildProc( hwnd, message, mp1, mp2 );
  return( mResult );
}


VOID PositionProcessControls( PGENPROCESSINSTIDA  pIda,
                              SHORT sHeigth, SHORT sSize  )
{

        PROCWINLAYOUT   Layout;        // layout to be used for process window
        LONG lTextY, lSliderY, lBoxY, lCurY;
        LONG  lPelsPerLine = UtlQueryULong( QL_PELSPERLINE );
        lCurY = 0;
        /**************************************************************/
        /* Get copy of layout to be used for process window and       */
        /* convert all values to physical units                       */
        /**************************************************************/
        memcpy( &Layout, &(ProcWinLayout[pIda->CommArea.Style]), sizeof(Layout) );
        Layout.lPreTextLines      *= lPelsPerLine;
        Layout.lTextLines         *= lPelsPerLine;
        Layout.lPostTextLines     *= lPelsPerLine;
        Layout.lPreSliderLines    *= lPelsPerLine;
        Layout.lSliderLines       *= lPelsPerLine;
        Layout.lPostSliderLines   *= lPelsPerLine;
        Layout.lPreEntryLines     *= lPelsPerLine;
        Layout.lEntryLines        *= lPelsPerLine;
        Layout.lPostEntryLines    *= lPelsPerLine;
        Layout.lPreLBHeaderLines  *= lPelsPerLine;
        Layout.lLBHeaderLines     *= lPelsPerLine;
        Layout.lPostLBHeaderLines *= lPelsPerLine;
        Layout.lPreListboxLines   *= lPelsPerLine;
        Layout.lListboxLines      *= lPelsPerLine;
        Layout.lPostListboxLines  *= lPelsPerLine;

        /**************************************************************/
        /* Position text line (if any)                                */
        /**************************************************************/
        if ( (pIda != NULL) &&
             (pIda->CommArea.Style != PROCWIN_SLIDERONLY) &&
             (pIda->CommArea.Style != PROCWIN_SLIDERENTRY) &&
             (pIda->hwndText != NULLHANDLE ) )
        {
          lTextY = lCurY + Layout.lPreTextLines;
          lCurY += Layout.lPreTextLines +
                   Layout.lTextLines +
                   Layout.lPostTextLines;
          if ( pIda->hwndText2 != NULLHANDLE  )
          {
            lCurY += Layout.lTextLines;
          } /* endif */
          MoveWindow( pIda->hwndText, 0, lTextY, sSize, Layout.lTextLines, TRUE );
          if ( pIda->hwndText2 != NULL  )
          {
            lCurY += Layout.lTextLines;
            MoveWindow( pIda->hwndText2, 0, lTextY + Layout.lTextLines,
                        sSize, Layout.lTextLines, TRUE );
          } /* endif */
        } /* endif */
        /**************************************************************/
        /* Position slider control (if any)                           */
        /**************************************************************/
        if ( (pIda != NULL) &&
             (pIda->CommArea.Style != PROCWIN_TEXTONLY) &&
             (pIda->hwndSlider != NULLHANDLE ) )
        {
          lSliderY = lCurY + Layout.lPreSliderLines;
          lCurY += Layout.lPreSliderLines +
                   Layout.lSliderLines +
                   Layout.lPostSliderLines;
          MoveWindow( pIda->hwndSlider, (sSize - 254) / 2, lSliderY,
                      254, Layout.lSliderLines, TRUE );
        } /* endif */

        /**************************************************************/
        /* Position entry groupbox and entry text (if any)            */
        /**************************************************************/
        if ( (pIda != NULL) &&
             ( (pIda->CommArea.Style == PROCWIN_TEXTSLIDERENTRY) ||
               (pIda->CommArea.Style == PROCWIN_SLIDERENTRY) ) &&
             (pIda->hwndGB != NULLHANDLE ) &&
             (pIda->hwndEntry != NULLHANDLE) )
        {
          LONG lHalfLine = (lPelsPerLine / 2 );

          lBoxY = lCurY + Layout.lPreEntryLines;
          lCurY += Layout.lPreEntryLines +
                   Layout.lEntryLines +
                   Layout.lPostEntryLines;
          MoveWindow( pIda->hwndGB, 5, lBoxY + lHalfLine,
                      max( 0, (sSize - 10) ),
                      Layout.lEntryLines - lHalfLine, TRUE );
          MoveWindow( pIda->hwndGBText, 15, lBoxY,
                      max( 0, pIda->sGBTextWidth ),
                      lPelsPerLine, TRUE );
          MoveWindow( pIda->hwndRect, 6, lBoxY + lHalfLine + 1,
                      max( 0, (sSize - 12) ),
                      Layout.lEntryLines - lHalfLine - 2, TRUE );
          MoveWindow( pIda->hwndEntry, 10,
                      lBoxY + (Layout.lEntryLines / 2),
                      max( 0, (sSize - 20) ),
                      lPelsPerLine, TRUE );
        } /* endif */

        /**************************************************************/
        /* Position listboxes and listbox headers (if any)            */
        /**************************************************************/
        if ( (pIda != NULL) &&
             (pIda->CommArea.Style == PROCWIN_TEXTSLIDERLISTBOX) &&
             (pIda->CommArea.hwndLB1 != NULLHANDLE ) &&
             (pIda->hwndLB1Text != NULLHANDLE ) &&
             (pIda->CommArea.hwndLB2 != NULLHANDLE ) &&
             (pIda->hwndLB2Text != NULLHANDLE ) )
        {
          lBoxY = lCurY + Layout.lPreLBHeaderLines;
          lCurY += Layout.lPreLBHeaderLines +
                   Layout.lLBHeaderLines +
                   Layout.lPostLBHeaderLines;

          MoveWindow( pIda->hwndLB1Text, 0, lBoxY,
                      sSize / 2, Layout.lLBHeaderLines, TRUE );
          MoveWindow( pIda->hwndLB2Text, sSize / 2, lBoxY,
                      sSize / 2, Layout.lLBHeaderLines, TRUE );

          lBoxY = lCurY + Layout.lPreListboxLines;

          MoveWindow( pIda->CommArea.hwndLB1, 0,
                      lBoxY,
                      sSize / 2,
                      max( 0, (sHeigth - lBoxY)),
                      TRUE );
          MoveWindow( pIda->CommArea.hwndLB2, sSize / 2,
                      lBoxY,
                      sSize / 2,
                      max( 0, (sHeigth - lBoxY)),
                      TRUE );
        } /* endif */
}

/**********************************************************************/
/* Function CreateProcessControls                                     */
/*   Create the slider and all associated windows                     */
/**********************************************************************/
BOOL CreateProcessControls( HWND hwnd, PGENPROCESSINSTIDA  pIda, WPARAM mp1 )
{
  HWND             hwndPrevCtrl;       // buffer for handle of previous control
  BOOL fOK = TRUE;
      if ( fOK )
      {
        /**************************************************************/
        /* Under OS/2 PM this message procedure is for the client     */
        /* window of a list window. Under Windows there is not        */
        /* client window and the message procedure processes the      */
        /* frame messages                                             */
        /**************************************************************/
        pIda->hFrame = hwnd;
        QUERYTEXTHWND( pIda->hFrame, pIda->CommArea.szObjName);
      } /* endif */

      /**************************************************************/
      /* Call instance callback function for initialisation         */
      /**************************************************************/
      if ( fOK )
      {
        MRESULT   mresultCallBack;     // return code of callback function

        mresultCallBack = (*pIda->pfnCallBack)( &pIda->CommArea,
                                                hwnd, WM_CREATE, mp1,
                                                MP2FROMP(pIda->pvUserData) );
      } /* endif */

      if ( fOK )
      {
        ANCHORWNDIDA( hwnd, pIda );
        /**************************************************************/
        /* Clear visible flag for batch mode process windows          */
        /**************************************************************/
        if ( pIda->CommArea.Style == PROCWIN_BATCH )
        {
          pIda->fVisible    = FALSE;
        } /* endif */

        /**************************************************************/
        /* disable close option in system menu if requested by        */
        /* callback function                                          */
        /**************************************************************/
        if ( pIda->CommArea.fNoClose )
        {
          HMENU hSysMenu;         // handle of system menu

          hSysMenu = GetSystemMenu( pIda->hFrame, FALSE );
          if ( hSysMenu != NULL )
          {
            EnableMenuItem( hSysMenu, SC_CLOSE, MF_GRAYED );
          } /* endif */
        } /* endif */

        /****************************************************************/
        /* Windows: no child system menu and setting of icon required!  */
        /****************************************************************/
         hwndPrevCtrl = HWND_TOP;      // set Z-order start handle

         /*************************************************************/
         /* Create text control (if required)                         */
         /*************************************************************/
         if ( fOK && (pIda->CommArea.Style != PROCWIN_SLIDERONLY) &&
                     (pIda->CommArea.Style != PROCWIN_BATCH)      &&
                     (pIda->CommArea.Style != PROCWIN_SLIDERENTRY) )
         {
            pIda->hwndText = WinCreateWindow( hwnd,
                                              WC_STATIC,
                                              pIda->CommArea.szText,
                                              WS_VISIBLE | WS_CHILDWINDOW |
                                              SS_CENTER,
                                              0,0,      // dummy size and pos
                                              100,100,
                                              NULLHANDLE,
                                              hwndPrevCtrl,
                                              pIda->CommArea.sTextID,
                                              NULL,
                                              NULL);
            fOK = ( pIda->hwndText != NULLHANDLE );
            hwndPrevCtrl = pIda->hwndText;
            strcpy( pIda->szCurText, pIda->CommArea.szText );
         } /* endif */

         if ( fOK && (pIda->CommArea.Style != PROCWIN_SLIDERONLY) &&
                     (pIda->CommArea.Style != PROCWIN_SLIDERENTRY) &&
                     (pIda->CommArea.Style != PROCWIN_BATCH)      &&
                     (pIda->CommArea.szText2[0] != EOS) )
         {
            pIda->hwndText2 = WinCreateWindow( hwnd,
                                              WC_STATIC,
                                              pIda->CommArea.szText2,
                                              WS_VISIBLE | WS_CHILDWINDOW |
                                              SS_CENTER,
                                              0,0,      // dummy size and pos
                                              100,100,
                                              NULLHANDLE,
                                              hwndPrevCtrl,
                                              (SHORT)(pIda->CommArea.sTextID + 1),
                                              NULL,
                                              NULL);
            fOK = ( pIda->hwndText2 != NULLHANDLE );
            hwndPrevCtrl = pIda->hwndText2;
            strcpy( pIda->szCurText2, pIda->CommArea.szText2 );
         } /* endif */

         /*************************************************************/
         /* Create slider control (if required)                       */
         /*************************************************************/
         if ( fOK && (pIda->CommArea.Style != PROCWIN_TEXTONLY) &&
                     (pIda->CommArea.Style != PROCWIN_BATCH) )
         {
            LONG        lWindowStyle;

            lWindowStyle = WS_VISIBLE | WS_CHILDWINDOW | PBS_HORIZONTAL |
                           PBS_PERCENTAGE | PBS_CHISELED;
            pIda->hwndSlider = WinCreateWindow( hwnd,
                                          PROGRESSCONTROL,
                                          NULL,
                                          lWindowStyle,
                                          0,0, // Dummy size of window see
                                          254,
                                          (SHORT)(UtlQueryULong( QL_PELSPERLINE ) * 5),
                                          hwnd,
                                          hwndPrevCtrl,
                                          pIda->CommArea.sSliderID,
                                          NULL,
                                          NULL);
            if (pIda->hwndSlider)
            {
              SendMessage( pIda->hwndSlider, PB_SETRANGE, 100, 0 );
              SendMessage( pIda->hwndSlider, PB_SETPOS, 0, 0 );
              SendMessage( pIda->hwndSlider, PB_SETCOLOR, 0, RGB(0,0,0) );
            } /* endif */
            fOK = ( pIda->hwndSlider != NULLHANDLE );
            hwndPrevCtrl = pIda->hwndSlider;
         } /* endif */

         /*************************************************************/
         /* Create entry groupbox (if required)                       */
         /*************************************************************/
         if ( fOK && ( (pIda->CommArea.Style == PROCWIN_TEXTSLIDERENTRY) ||
                       (pIda->CommArea.Style == PROCWIN_SLIDERENTRY) ) )
         {
            /**********************************************************/
            /* As the Windows groupbox in MDI dialogs has enormous    */
            /* repaint problems we construct here a pseudo groupbox   */
            /* consisting of a blackframe, a inner white rectangle    */
            /* and a static text control for the groupbox title       */
            /**********************************************************/
            pIda->hwndGB   = WinCreateWindow( hwnd,
                                              WC_STATIC,
                                              "",
                                              WS_VISIBLE | WS_CHILDWINDOW |
                                              SS_BLACKFRAME,
                                              0,0,      // dummy size and pos
                                              100,100,
                                              NULL,
                                              hwndPrevCtrl,
                                              pIda->CommArea.sEntryGBID,
                                              NULL,
                                              NULL);
            fOK = ( pIda->hwndGB != NULLHANDLE );
            hwndPrevCtrl = pIda->hwndGB;

            if ( fOK )
            {
              pIda->hwndRect = WinCreateWindow( hwnd,
                                                WC_STATIC,
                                                "",
                                                WS_VISIBLE | WS_CHILDWINDOW |
                                                SS_LEFT,
                                                0,0,      // dummy size and pos
                                                100,100,
                                                NULL,
                                                hwndPrevCtrl,
                                                5,
                                                NULL,
                                                NULL);
              fOK = ( pIda->hwndRect != NULLHANDLE );
              hwndPrevCtrl = pIda->hwndRect;
            } /* endif */

            if ( fOK )
            {
              HPS   hpsWindow;
              LONG lCY, lCX;

              hpsWindow = GETPS( hwnd );
              TEXTSIZE( hpsWindow, pIda->CommArea.szGroupBoxTitle, lCX, lCY );
              pIda->sGBTextWidth = (SHORT)lCX;
              RELEASEPS( hwnd, hpsWindow );

              pIda->hwndGBText = WinCreateWindow( hwnd,
                                                WC_STATIC,
                                                pIda->CommArea.szGroupBoxTitle,
                                                WS_VISIBLE | WS_CHILDWINDOW |
                                                SS_LEFTNOWORDWRAP,
                                                0,0,      // dummy size and pos
                                                100,100,
                                                NULL,
                                                hwndPrevCtrl,
                                                4,
                                                NULL,
                                                NULL);
              fOK = ( pIda->hwndGBText != NULLHANDLE );
              hwndPrevCtrl = pIda->hwndGBText;
            } /* endif */
         } /* endif */

         /*************************************************************/
         /* Create entry text control (if required)                   */
         /*************************************************************/
         if ( fOK && ( (pIda->CommArea.Style == PROCWIN_TEXTSLIDERENTRY) ||
                       (pIda->CommArea.Style == PROCWIN_SLIDERENTRY) ) )
         {
            pIda->hwndEntry = WinCreateWindow( hwnd,
                                              WC_STATIC,
                                              pIda->CommArea.szEntry,
                                              WS_VISIBLE | WS_CHILDWINDOW |
                                              SS_LEFT,
                                              0,0,      // dummy size and pos
                                              100,100,
                                              NULLHANDLE,
                                              hwndPrevCtrl,
                                              pIda->CommArea.sEntryID,
                                              NULL,
                                              NULL);
            fOK = ( pIda->hwndEntry != NULLHANDLE );
            hwndPrevCtrl = pIda->hwndEntry;
            strcpy( pIda->szCurEntry, pIda->CommArea.szEntry );
         } /* endif */

         /*************************************************************/
         /* Create listbox 1 label (if required)                      */
         /*************************************************************/
         if ( fOK && (pIda->CommArea.Style == PROCWIN_TEXTSLIDERLISTBOX) )
         {
            pIda->hwndLB1Text = WinCreateWindow( hwnd,
                                              WC_STATIC,
                                              pIda->CommArea.szLB1Text,
                                              WS_VISIBLE | WS_CHILDWINDOW |
                                              SS_LEFT,
                                              0,0,      // dummy size and pos
                                              100,100,
                                              NULLHANDLE,
                                              hwndPrevCtrl,
                                              pIda->CommArea.sLB1TextID,
                                              NULL,
                                              NULL);
            fOK = ( pIda->hwndLB1Text != NULLHANDLE );
            hwndPrevCtrl = pIda->hwndLB1Text;
         } /* endif */

         /*************************************************************/
         /* Create listbox 2 label (if required)                      */
         /*************************************************************/
         if ( fOK && (pIda->CommArea.Style == PROCWIN_TEXTSLIDERLISTBOX) )
         {
            pIda->hwndLB2Text = WinCreateWindow( hwnd,
                                              WC_STATIC,
                                              pIda->CommArea.szLB2Text,
                                              WS_VISIBLE | WS_CHILDWINDOW |
                                              SS_LEFT,
                                              0,0,      // dummy size and pos
                                              100,100,
                                              NULLHANDLE,
                                              hwndPrevCtrl,
                                              pIda->CommArea.sLB2TextID,
                                              NULL,
                                              NULL);
            fOK = ( pIda->hwndLB2Text != NULLHANDLE );
            hwndPrevCtrl = pIda->hwndLB2Text;
         } /* endif */

         /*************************************************************/
         /* Create listbox 1 (if required)                            */
         /*************************************************************/
         if ( fOK && (pIda->CommArea.Style == PROCWIN_TEXTSLIDERLISTBOX) )
         {
            pIda->CommArea.hwndLB1 = WinCreateWindow( hwnd,
                                              WC_LISTBOX,
                                              "",
                                              WS_VISIBLE | WS_CHILDWINDOW |
                                              LBS_STANDARD,
                                              0,0,      // dummy size and pos
                                              100,100,
                                              NULLHANDLE,
                                              hwndPrevCtrl,
                                              pIda->CommArea.sLB1ID,
                                              NULL,
                                              NULL);
            fOK = ( pIda->CommArea.hwndLB1 != NULLHANDLE );
            hwndPrevCtrl = pIda->CommArea.hwndLB1;
            if ( fOK )
            {
			  pfnOldListboxProc = SUBCLASSWND(pIda->CommArea.hwndLB1, ProcessListboxProc);
            } /* endif */
         } /* endif */

         /*************************************************************/
         /* Create listbox 2 (if required)                            */
         /*************************************************************/
         if ( fOK && (pIda->CommArea.Style == PROCWIN_TEXTSLIDERLISTBOX) )
         {
            pIda->CommArea.hwndLB2 = WinCreateWindow( hwnd,
                                              WC_LISTBOX,
                                              "",
                                              WS_VISIBLE | WS_CHILDWINDOW |
                                              LBS_STANDARD,
                                              0,0,      // dummy size and pos
                                              100,100,
                                              NULLHANDLE,
                                              hwndPrevCtrl,
                                              pIda->CommArea.sLB2ID,
                                              NULL,
                                              NULL);
            fOK = ( pIda->CommArea.hwndLB2 != NULLHANDLE );
            hwndPrevCtrl = pIda->CommArea.hwndLB2;
            if ( fOK )
            {
			  pfnOldListboxProc = SUBCLASSWND(pIda->CommArea.hwndLB2, ProcessListboxProc);

            } /* endif */
         } /* endif */
      } /* endif */
  return fOK;
}

/**********************************************************************/
/* Function CheckForUpdate                                            */
/*                                                                    */
/* The function checks if one of the elements of a process window     */
/* requires an update.                                                */
/*                                                                    */
/* If an update is required the process window controls are updated   */
/* and (to allow a repaint of the controls) messages are dispatched   */
/**********************************************************************/
void CheckForUpdate( HWND hwnd, PGENPROCESSINSTIDA  pIda )
{
  BOOL             fChanged = FALSE;   // controls have been changed flag
  hwnd;
  /**************************************************************/
  /* Check if slider arm position requires update               */
  /**************************************************************/
  if ( pIda->usComplete != pIda->CommArea.usComplete )
  {
    SendMessage( pIda->hwndSlider, PB_SETPOS, pIda->CommArea.usComplete, 0 );
    UpdateWindow( pIda->hwndSlider );
    pIda->usComplete = pIda->CommArea.usComplete;
    fChanged = TRUE;
  } /* endif */

  /**************************************************************/
  /* Check for change in slider texts                           */
  /**************************************************************/
  if ( (pIda->CommArea.Style != PROCWIN_SLIDERONLY ) &&
       (pIda->CommArea.Style != PROCWIN_SLIDERENTRY) &&
       (strcmp( pIda->CommArea.szText, pIda->szCurText ) != 0 ) )
  {
    SETTEXTHWND( pIda->hwndText, pIda->CommArea.szText );
    UpdateWindow( pIda->hwndText );
    strcpy( pIda->szCurText, pIda->CommArea.szText );
    fChanged = TRUE;
  } /* endif */

  if ( (pIda->CommArea.Style != PROCWIN_SLIDERONLY ) &&
       (pIda->CommArea.Style != PROCWIN_SLIDERENTRY) &&
       (strcmp( pIda->CommArea.szText2, pIda->szCurText2 ) != 0 ) )
  {
    SETTEXTHWND( pIda->hwndText2, pIda->CommArea.szText2 );
    UpdateWindow( pIda->hwndText2 );
    strcpy( pIda->szCurText2, pIda->CommArea.szText2 );
    fChanged = TRUE;
  } /* endif */

  if ( (pIda->CommArea.Style != PROCWIN_SLIDERONLY ) &&
       (pIda->CommArea.Style != PROCWIN_SLIDERENTRY) &&
       (strcmp( pIda->CommArea.szTitle, pIda->szCurTitle ) != 0 ) )
  {
    SETTEXTHWND( pIda->hFrame, pIda->CommArea.szTitle );
    UpdateWindow( pIda->hFrame );
    strcpy( pIda->szCurTitle, pIda->CommArea.szTitle );
    fChanged = TRUE;
  } /* endif */

  /**************************************************************/
  /* Check for change in entry text                             */
  /**************************************************************/
  if ( ( (pIda->CommArea.Style == PROCWIN_TEXTSLIDERENTRY ) ||
         (pIda->CommArea.Style == PROCWIN_SLIDERENTRY) ) &&
       (strcmp( pIda->CommArea.szEntry, pIda->szCurEntry ) != 0 ) )
  {
    if ( !pIda->fWPStatic )
    {
      SETTEXTHWND( pIda->hwndEntry, pIda->CommArea.szEntry );
      UpdateWindow( pIda->hwndEntry );
    } /* endif */
    strcpy( pIda->szCurEntry, pIda->CommArea.szEntry );
    fChanged = TRUE;
  } /* endif */

   /**************************************************************/
   /* Dispatch messages to allow repaint of window               */
   /**************************************************************/
   if ( fChanged )
   {
//   UtlDispatch();
   } /* endif */

   return;
} /* end of function CheckForUpdate */

void CheckForRefresh()
{
    HANDLE        hMapObject = NULL;

    hMapObject = OpenFileMapping (FILE_MAP_WRITE, TRUE, EQFNDDE_SHFLAG );
    if (!hMapObject)
    {
        hMapObject = CreateFileMapping(
                                      (HANDLE)0xFFFFFFFF,   // use page file
                                      NULL,                 // no security attrib
                                      PAGE_READWRITE,       // read/write access
                                      0,                    // size: high 32bits
                                      sizeof(ULONG),        // size: low 32bit
                                      EQFNDDE_SHFLAG );     // name of file mapping
        if (!hMapObject)
        {
            DWORD Error;
            Error = GetLastError();
        }
    }
    if (hMapObject)
    {
        ULONG *ulActFlag = (ULONG *)MapViewOfFile (hMapObject,
                                                   FILE_MAP_WRITE,
                                                   0, 0, 0);
        if ( hMapObject )
        {
            if (*ulActFlag & EQF_REFR_FOLLIST)
            {
                EqfPost2Handler(FOLDERLISTHANDLER, WM_EQFN_DRIVEADDED,MP1FROMHWND( NULL ),MP2FROMP(NULL));
                *ulActFlag ^= EQF_REFR_FOLLIST;
            }
            if (*ulActFlag & EQF_REFR_DICLIST)
            {
                EqfPost2Handler(DICTIONARYHANDLER, WM_EQFN_DRIVEADDED,MP1FROMHWND(NULL),MP2FROMP(NULL));
                *ulActFlag ^= EQF_REFR_DICLIST;
            }
            if (*ulActFlag & EQF_REFR_MEMLIST)
            {
                EqfPost2Handler(MEMORYHANDLER, WM_EQFN_DRIVEADDED,MP1FROMHWND(NULL),MP2FROMP(NULL));
                *ulActFlag ^= EQF_REFR_MEMLIST;
            }
            UnmapViewOfFile(ulActFlag);
        }
    }
}


/**********************************************************************/
/* Function for popup menus                                           */
/**********************************************************************/
void HandlePopupMenu( HWND hwnd, POINT point, SHORT sMenuID )
{
  HMENU hMenu;
  HMENU hMenuTrackPopup;
  CHAR  szEngineString[200];
  UINT uiFormat;
  PGENLISTINSTIDA  pIda;
  int    iSelItems; 
  /* Get the menu for the popup from the resource file. */
  HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
  hMenu = LoadMenu( hResMod, MAKEINTRESOURCE( ID_POPUP_MENU ) );
  if ( !hMenu )
      return;

  hMenuTrackPopup = GetSubMenu( hMenu, sMenuID );


  GetStringFromRegistry( APPL_Name, KEY_MTEngine, szEngineString, sizeof(szEngineString ), "" );

  if (!szEngineString[0] )
  {

    DeleteMenu( hMenuTrackPopup, PID_UTILS_MI_MT, MF_BYCOMMAND );

  } /* endif */

  uiFormat = UtlQueryUShort(QS_CLIPBOARDFORMAT);
  if (!IsClipboardFormatAvailable(uiFormat))
  {
       //DeleteMenu(hMenuTrackPopup, PID_FILE_MI_PASTE, MF_BYCOMMAND);
       EnableMenuItem(hMenuTrackPopup, PID_FILE_MI_PASTE, MF_BYCOMMAND|MF_GRAYED);
  }

  // if memory list window focused, decide whether need to show rename menu item
  pIda = ACCESSWNDIDA( hwnd, PGENLISTINSTIDA );
  iSelItems = SendMessage(  (pIda->CommArea).hwndLB, LB_GETSELCOUNT, 0, 0 );
  if((pIda->CommArea).sListWindowID == ID_MEMORY_WINDOW && iSelItems>0)
  {
     char szBuf[MAX_EQF_PATH*2+1] = {0};
     char szMem[MAX_EQF_PATH*2+1] = {0};  
     SHORT sItem = QUERYSELECTIONHWND( (pIda->CommArea).hwndLB );
     SendMessage( (pIda->CommArea).hwndLB , LM_EQF_QUERYITEMTEXT, (WPARAM)sItem, (LPARAM)szBuf);
     strncpy( szMem, UtlParseX15( szBuf, 1),MAX_EQF_PATH*2);
     // get memory property path
     memset(szBuf,0,sizeof(szBuf));
     UtlMakeEQFPath( szBuf, NULC, PROPERTY_PATH, NULL );
     strcat( szBuf, BACKSLASH_STR );
     strcat( szBuf, szMem );
     strcat( szBuf, ".SHP" );
     // if it exists,means it's share memory
     if(UtlFileExist(szBuf))
     {
       EnableMenuItem(hMenuTrackPopup, PID_FILE_MI_RENAME, MF_BYCOMMAND|MF_GRAYED);

       // add menu items, add/remove/list users
       AppendMenu(hMenuTrackPopup,MF_ENABLED, PID_FILE_MI_ADDUSER, "Add User");
       AppendMenu(hMenuTrackPopup,MF_ENABLED, PID_FILE_MI_REMOVEUSER, "Remove User");
       AppendMenu(hMenuTrackPopup,MF_ENABLED, PID_FILE_MI_LISTUSER, "List User");
	   //AppendMenu(hMenuTrackPopup,MF_ENABLED, PID_FILE_MI_UPLOADTOSERVER, "Batch upload");
	   //AppendMenu(hMenuTrackPopup,MF_ENABLED, PID_FILE_MI_DOWNFROMSERVER, "Batch download");
     } 
	
  }  

  /* Convert the mouse point to screen coordinates since that is what
   * TrackPopup expects.
   */
  ClientToScreen( hwnd, (LPPOINT)&point );

  /* Draw and track the "floating" popup */
  TrackPopupMenu( hMenuTrackPopup, 0, point.x, point.y, 0,
                  (HWND)UtlQueryULong( QL_TWBFRAME ), NULL );

  /* Destroy the menu since were are done with it. */
  DestroyMenu( hMenu );
}

MRESULT APIENTRY ProcessListboxProc( register HWND hwnd, UINT msg,
                                    register WPARAM mp1, LPARAM mp2)
{
  switch (msg)
  {
    case WM_KEYDOWN:
      if ( mp1 == VK_F1 )
      {
        /**************************************************************/
        /* Post message to the process window                         */
        /**************************************************************/
        HWND       hwndProcess;

        hwndProcess = GetParent( hwnd );
        PostMessage( hwndProcess, WM_KEYDOWN, mp1, mp2 );
      } /* endif */
      break;

    default:
      break;
  } /* endswitch */

  // call old window procedure
  return CallWindowProc( (WNDPROC)pfnOldListboxProc, hwnd, msg, mp1, mp2 );
}

/**********************************************************************/
/* Procedure to get or refresh the commarea pointer for a generic     */
/* list window (e.g. adfter a call to UtlDispatch)                    */
/*                                                                    */
/* If NULL is returned either the list window has been destroyed or   */
/* it's data areas have been freed                                    */
/**********************************************************************/
PVOID AccessGenListCommArea( HWND hwnd )
{
  PLISTCOMMAREA pCommArea = NULL;
  PGENLISTINSTIDA pIda;

  pIda = ACCESSWNDIDA( hwnd, PGENLISTINSTIDA );
  if ( pIda != NULL )
  {
    pCommArea = &(pIda->CommArea);
  } /* endif */
  return( pCommArea );
} /* end of function AccessGenListCommArea */

//   End of EQFHNDLR.C
//ßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßß
