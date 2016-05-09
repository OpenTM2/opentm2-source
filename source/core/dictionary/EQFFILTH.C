//+----------------------------------------------------------------------------+
//|  EQFFILTH.C - EQF Filter Handler                                           |
//+----------------------------------------------------------------------------+
//|  Copyright Notice:                                                         |
//|                                                                            |
//|      Copyright (C) 1990-2012, International Business Machines              |
//|      Corporation and others. All rights reserved                           |

//+----------------------------------------------------------------------------+
//|  Description:                                                              |
//|      The filter handler is the central entry point for all filter related  |
//|      requests from other MAT Tools functions.                              |
//|                                                                            |
//|      The filter handler is implemented as a PM object window which is      |
//|      registered as a handler to the MAT Tools object handler. Unlike       |
//|      other MAT Tools handlers, the list handler has no list window of      |
//|      its own.                                                              |
//+----------------------------------------------------------------------------+
//|  Entry Points:                                                             |
//|                                                                            |
//|  FiltHandlerWP         filter handler window procedure                     |
//|                                                                           |
//|+-- status ("H"=Header,"D"=Design,"C"=Code,"T"=Test, " "=complete,          |
//|            "Q"=Quick-and-dirty )                                           |
//+----------------------------------------------------------------------------+
//|  Externals:                                                                |
//|                                                                            |
//|  FiltLoadFilterHeader  load filter header into memory                      |
//+----------------------------------------------------------------------------+
//|  Internals:                                                                |
//|                                                                            |
//|  FiltRefreshListbox    refresh contents of filter list listbox             |
//|                                                                           |
//|+-- status ("H"=Header,"D"=Design,"C"=Code,"T"=Test, " "=complete,          |
//|            "Q"=Quick-and-dirty )                                           |
//+----------------------------------------------------------------------------+
//|  Include files:                                                            |
//+----------------------------------------------------------------------------+
//|  To be done / known limitations / caveats:                                 |
//|                                                                            |
//+----------------------------------------------------------------------------+
#define INCL_EQF_ASD              // dictionary access functions (Asd...)
#define INCL_EQF_LDB              // dictionary data encoding functions
#define INCL_EQF_FILT             // dictionary filter functions
#include <eqf.h>                  // General Translation Manager include file
#include "eqfstart.id"                 // general IDs (here IDs for text strings)
#include "eqffilt.id"                  // Filter IDs
#include "eqffilti.h"                  // Filter private include file

/**********************************************************************/
/* Filter handler IDA                                                 */
/**********************************************************************/
typedef struct _FILTERHANDLERIDA
{
  IDA_HEAD      IdaHead;               // IDA header data
  HWND          hwndLB;                // handle of filter list listbox
  HWND          hwndTempl;             // handle of template for filter control
  FILTPROP      FiltProp;              // buffer for filter header data
  FILEFINDBUF   ResultBuf;             // result buffer for DosFindFirst calls
  CHAR          szSearchPath[MAX_EQF_PATH]; // path for filter search
  CHAR          szFilterPath[MAX_EQF_PATH]; // path for filter operations
  CHAR          szItemText[MAX_FNAME+  // buffer for item text strings
                           MAX_DESCRIPTION+2];
  FILTTEMPL     FiltTemplate;          // template for the creating of filter
                                       // controls
} FILTERHANDLERIDA, *PFILTERHANDLERIDA;

VOID             FiltRefreshListbox( PFILTERHANDLERIDA, HWND, PSZ, WINMSG );
INT_PTR CALLBACK  FILTCONTROLDUMMYDLGPROC( HWND, WINMSG, WPARAM, LPARAM );

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     FiltHandlerWP          Filter Handler Window Procedure   |
//+----------------------------------------------------------------------------+
//|Function call:     FiltHandlerWP( HWND hwnd. USHORT msg, MPARAM mp1,        |
//|                                  MPARAM mp2 );                             |
//+----------------------------------------------------------------------------+
//|Description:       Procedure for the filter handler object window.          |
//+----------------------------------------------------------------------------+
//|Input parameter:   HWND    hwnd        window handle of filter object window|
//|                   USHORT  msg         message ID                           |
//|                   MPARAM  mp1         first message parameter              |
//|                   MPARAM  mp2         second message parameter             |
//+----------------------------------------------------------------------------+
//|Returncode type:   MRESULT                                                  |
//+----------------------------------------------------------------------------+
//|Returncodes:       result of message processing                             |
//+----------------------------------------------------------------------------+
//|Prerequesits:      filter resource module (hResMod) must have been loaded  |
//+----------------------------------------------------------------------------+
//|Function flow:     switch msg                                               |
//|                     case WM_CREATE:                                        |
//|                       allocate and anchor IDA                              |
//|                       register filter handler                              |
//|                       register filter control window class                 |
//|                       create invisible filter list box                     |
//|                     case WM_EQF_INITIALIZE:                                |
//|                       call FiltRefreshListbox to refresh filter listbox    |
//|                     case WM_EQF_INSERTNAMES:                               |
//|                       loop over all items of the listbox                   |
//|                         copy listbox item to target listbox                |
//|                       endloop                                              |
//|                     case WM_EQFN_PROPERTIESCHANGED:                        |
//|                     case WM_EQFN_DELETED:                                  |
//|                     case WM_EQFN_CREATED:                                  |
//|                       if object class is filter                            |
//|                         call FiltRefreshListbox to refresh filter listbox  |
//|                       endif                                                |
//|                     case WM_EQF_PROCESSTASK:                               |
//|                       switch task                                          |
//|                         case GETFILTERDESCR_TASK:                          |
//|                           if filter name is a pseudo filter name           |
//|                             return 'no filter selected' description        |
//|                           else                                             |
//|                             search filter in filter listbox                |
//|                             return description of filter                   |
//|                           endif                                            |
//|                       endswitch                                            |
//|                     case WM_EQF_TERMINATE:                                 |
//|                       destroy filter handler object window                 |
//|                     case WM_DESTROY:                                       |
//|                       destroy filter listbox                               |
//|                       free IDA                                             |
//|                   endswitch                                                |
//+----------------------------------------------------------------------------+
MRESULT APIENTRY FILTHANDLERWP
(
  HWND    hwnd,                        // window handle of filter object window
  WINMSG  msg,                         // message ID
  WPARAM  mp1,                         // first message parameter
  LPARAM  mp2                          // second message parameter
)
{
  PFILTERHANDLERIDA pIda;              // pointer to instance area
  MRESULT         mResult = FALSE;     // result value of message processing
  BOOL            fOK;                 // program flow control flag
  SHORT           sItems;              // number of listbox items
  SHORT           sItem;               // listbox item index
  int             i;                   // loop index
  PSZ             pszTemp;             // work pointer


  /********************************************************************/
  /* Do message dependend processing                                  */
  /********************************************************************/
  switch( msg )
  {
    /******************************************************************/
    /* Process WM_CREATE message:                                     */
    /* (is sent during processing of WinCreateStdWindow)              */
    /*   allocate and anchor IDA, create filter list listbox          */
    /******************************************************************/
    case WM_CREATE:
      /**************************************************************/
      /* Allocate and anchor IDA                                    */
      /**************************************************************/
      fOK = UtlAlloc( (PVOID *) &pIda, 0L, (LONG)sizeof(FILTERHANDLERIDA),
                      ERROR_STORAGE );
      if( fOK )
      {
        ANCHORWNDIDA( hwnd, pIda );
      } /* endif */

      /****************************************************************/
      /* Register filter handler                                      */
      /****************************************************************/
      if ( fOK )
      {
         pIda->IdaHead.hFrame = hwnd;
         strcpy( pIda->IdaHead.szObjName, FILTERHANDLER );
         pIda->IdaHead.pszObjName = pIda->IdaHead.szObjName;
         EqfInstallHandler( FILTERHANDLER, hwnd, clsFILTER );
      } /* endif */

      /****************************************************************/
      /* Register filter control window class                         */
      /****************************************************************/
      if ( fOK )
      {
        WNDCLASS  wc;

        wc.style         = CS_GLOBALCLASS;          /* !!!! */
        wc.lpfnWndProc   = FILTCONTROLWP;
        wc.cbClsExtra    = 0;
        wc.cbWndExtra    = sizeof(PVOID);
        wc.hInstance     = (HINSTANCE)UtlQueryULong( QL_HAB );
        wc.hIcon         = LoadIcon((HINSTANCE) NULL, IDI_APPLICATION);
        wc.hCursor       = LoadCursor((HINSTANCE) NULL, IDC_ARROW);
        wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
        wc.lpszMenuName  = WC_EQF_FILTER;
        wc.lpszClassName = WC_EQF_FILTER;

        RegisterClass( &wc );
        if ( !fOK )
        {
          UtlError( 0, MB_CANCEL, 0, NULL, SYSTEM_ERROR );
        } /* endif */
      } /* endif */

      /****************************************************************/
      /* Create invisible filter list listbox                         */
      /****************************************************************/
      if ( fOK )
      {
        pIda->hwndLB = CreateWindow( "LISTBOX", "",
                                     WS_CHILD | LBS_STANDARD,
                                     0,0,10,10,
                                     hwnd,
                                     NULL,
                                     (HINSTANCE)UtlQueryULong( QL_HAB ),
                                     NULL);

        if ( !pIda->hwndLB )
        {
          UtlError( 0, MB_CANCEL, 0, NULL, SYSTEM_ERROR );
          fOK = FALSE;
        } /* endif */
      } /* endif */

      /****************************************************************/
      /* Call dummy filter dialog in order to get size and position   */
      /* of filter controls                                           */
      /****************************************************************/
      if ( fOK )
      {
		HMODULE hResMod;
		hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
        DIALOGBOX( hwnd, FILTCONTROLDUMMYDLGPROC, hResMod, ID_FILT_CONTROL,
                   &(pIda->FiltTemplate), fOK );
        LOADSTRING( NULLHANDLE, hResMod, SID_FILTER_EDIT_PB,
                    pIda->FiltTemplate.szEditPB );
        LOADSTRING( NULLHANDLE, hResMod, 1822,
                    pIda->FiltTemplate.szEditPB );
        LOADSTRING( NULLHANDLE, hResMod, 1822,
                    pIda->FiltTemplate.szEditPB );
      } /* endif */

      if ( !fOK )
      {
        mResult = MRFROMSHORT( TRUE );         // do not create the window
      } /* endif */
      break;

    /******************************************************************/
    /* Process WM_EQF_INITIALIZE message:                             */
    /*   (is sent by EQFSTART once the workbench window is created)   */
    /*   fill filter list listbox                                     */
    /******************************************************************/
    case  WM_EQF_INITIALIZE:
      /****************************************************************/
      /* Fill filter list listbox                                     */
      /****************************************************************/
      pIda = ACCESSWNDIDA( hwnd, PFILTERHANDLERIDA );
      if ( (pIda != NULL) && (pIda->hwndLB != NULLHANDLE) )
      {
        FiltRefreshListbox( pIda, pIda->hwndLB, NULL, 0 );
      } /* endif */
      break;


    /******************************************************************/
    /* Process WM_EQF_INSERTNAMES request:                            */
    /*   Fill given listbox with filter names                         */
    /******************************************************************/
    case WM_EQF_INSERTNAMES:
      {
        BOOL       fCombo;             // type of target listbox

        pIda = ACCESSWNDIDA( hwnd, PFILTERHANDLERIDA );
        sItems = (SHORT)WinSendMsg( pIda->hwndLB, LM_QUERYITEMCOUNT, 0L, 0L );
        ISCOMBOBOX( HWNDFROMMP1(mp1), fCombo );
        for ( i = 0; i < sItems; i++ )
        {
          QUERYITEMTEXTHWND( pIda->hwndLB, i, pIda->szItemText );
          pszTemp = strchr( pIda->szItemText, X15 );
          if ( pszTemp )
          {
            *pszTemp = EOS;
          } /* endif */
          if ( fCombo )
          {
            CBINSERTITEMHWND( HWNDFROMMP1(mp1), pIda->szItemText );
          }
          else
          {
            INSERTITEMHWND( HWNDFROMMP1(mp1), pIda->szItemText );
          } /* endif */
        } /* endfor */
      }
      break;

    /******************************************************************/
    /* Process notification messages WM_EQFN_PROPERTIESCHANGED,       */
    /* WM_EQFN_DELETED and WM_EQFN_CREATED:                           */
    /*   update internal filter list listbox and                      */
    /*   broadcast message to all filter controls                     */
    /******************************************************************/
    case WM_EQFN_PROPERTIESCHANGED:
    case WM_EQFN_DELETED:
    case WM_EQFN_CREATED:
      if ( SHORT1FROMMP1(mp1) == clsFILTER )
      {
        pIda = ACCESSWNDIDA( hwnd, PFILTERHANDLERIDA );
        FiltRefreshListbox( pIda, pIda->hwndLB,(PSZ) PVOIDFROMMP2(mp2), msg );
        EqfSend2AllObjects( clsFILTER, msg, mp1, mp2 );
      } /* endif */
      break;

    /******************************************************************/
    /* Process WM_EQF_PROCESSTASK request:                            */
    /*   pop-up filter dialog (for testing purposes only)             */
    /******************************************************************/
    case WM_EQF_PROCESSTASK:
      switch ( SHORT1FROMMP1(mp1) )
      {
        /**************************************************************/
        /* return filter description text                             */
        /**************************************************************/
        case GETFILTERDESCR_TASK :
          pIda = ACCESSWNDIDA( hwnd, PFILTERHANDLERIDA );
          pszTemp = (PSZ)mp2;
          strcpy( pIda->szItemText, pszTemp );
          strcat( pIda->szItemText, X15_STR );
          sItem = SEARCHITEMPREFIXHWND( pIda->hwndLB, pIda->szItemText );
          if ( sItem == LIT_NONE )
          {
            *pszTemp = EOS;
          }
          else
          {
            QUERYITEMTEXTHWND( pIda->hwndLB, sItem, pIda->szItemText );
            strcpy( pszTemp, strchr( pIda->szItemText, X15 ) + 1 );
          } /* endif */
          break;

        /**************************************************************/
        /* set callers filter control template pointer                */
        /**************************************************************/
        case GETFILTERTEMPL_TASK :
          {
            PFILTTEMPL  *ppTempl;

            pIda = ACCESSWNDIDA( hwnd, PFILTERHANDLERIDA );
            ppTempl =(PFILTTEMPL *) PVOIDFROMMP2(mp2);
            *ppTempl = &pIda->FiltTemplate;
            return( MRFROMSHORT(TRUE) );
          }

        default :
          break;
      } /* endswitch */
      break;

    /******************************************************************/
    /* Process the WM_EQF_TERMINATE message:                          */
    /* (the message is sent by the MAT Tools object handler during    */
    /*  shutdown of the workbench)                                    */
    /*   destroy the filter handler frame window                      */
    /******************************************************************/
    case WM_EQF_TERMINATE:
      pIda = ACCESSWNDIDA( hwnd, PFILTERHANDLERIDA );
      WinDestroyWindow( pIda->IdaHead.hFrame);
      break;

    /******************************************************************/
    /* Process the WM_DESTROY message:                                */
    /* (the message is sent during processing of WinDestroyWindow)    */
    /*   destroy filter list listbox                                  */
    /*   free IDA                                                     */
    /******************************************************************/
    case WM_DESTROY:
      pIda = ACCESSWNDIDA( hwnd, PFILTERHANDLERIDA );
      if ( pIda )
      {
        if ( pIda->hwndLB )
        {
          WinDestroyWindow( pIda->hwndLB );
        } /* endif */
        if ( pIda->hwndTempl )
        {
          WinDestroyWindow( pIda->hwndTempl );
        } /* endif */
        UtlAlloc( (PVOID *) &pIda, 0L, 0L, NOMSG );
      } /* endif */
      break;

    /******************************************************************/
    /* pass all other messages to the default window procedure        */
    /******************************************************************/
    default:
      mResult = WinDefWindowProc( hwnd, msg, mp1, mp2 );
      break;

  } /* endswitch */

  return( mResult );
} /* end of function FiltHandlerWP */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     FiltRefreshListbox  Refresh filter listbox               |
//+----------------------------------------------------------------------------+
//|Function call:     FiltRefreshListbox( HWND hwndLB, PSZ pszFilter,          |
//|                                       USHORT usMsg );                      |
//+----------------------------------------------------------------------------+
//|Description:       Fills or updates a listbox containing filter names and   |
//|                   descriptions.                                            |
//|                                                                            |
//|                   If no filter name is given, the listbox is filled with   |
//|                   the names of all available filters.                      |
//|                                                                            |
//|                   If a filter name is given the referenced filter is       |
//|                   either deleted, refreshed or added to the listbox        |
//|                   dependent on the value in usMsg.                         |
//+----------------------------------------------------------------------------+
//|Input parameter:   PFILTERHANDLERIDA pIda ptr to filter handler IDA         |
//|                   HWND        hwndLB    handle of the listbox control      |
//|                   PSZ         pszFilter name of filter or NULL             |
//|                   USHORT      usMsg     message forcing the refresh of the |
//|                                         listbox or 0                       |
//+----------------------------------------------------------------------------+
//|Returncode type:   VOID                                                     |
//+----------------------------------------------------------------------------+
//|Prerequesits:      the filter handler IDA must exists                       |
//|                   the listbox being filled/refreshed must exist            |
//+----------------------------------------------------------------------------+
//|Side effects:      items in the listbox are updated                         |
//+----------------------------------------------------------------------------+
//|Function flow:     if a filter name is given                                |
//|                     switch msg                                             |
//|                       case WM_EQFN_CREATED:                                |
//|                       case WM_EQFN_PROPERTIESCHANGED:                      |
//|                         load filter header                                 |
//|                         add or update filte in filter listbox              |
//|                       case WM_EQFN_DELETED:                                |
//|                         delete filter from listbox                         |
//|                     endswitch                                              |
//|                   else                                                     |
//|                     clear listbox                                          |
//|                     build filter search path                               |
//|                     loop over all filter property files                    |
//|                       get filter description                               |
//|                       insert filter into listbox                           |
//|                     endloop                                                |
//|                   endif                                                    |
//+----------------------------------------------------------------------------+
VOID FiltRefreshListbox
(
  PFILTERHANDLERIDA pIda,              // ptr to filter handler IDA
  HWND          hwndLB,                // handle of the listbox control
  PSZ           pszFilter,             // name of filter or NULL
  WINMSG        usMsg                  // message for type of refresh
)
{
  USHORT        usRC = NO_ERROR;       // return code buffer
  USHORT        usCount;               // number of files requested/found
  SHORT         sItem;                 // listbox item index
  HDIR          hDirHandle = HDIR_CREATE; // DosFind routine handle

  /********************************************************************/
  /* Fill listbox or update listbox dependend on pszFilter            */
  /********************************************************************/
  if ( pszFilter )
  {
    /******************************************************************/
    /* Update listbox with supplied data dependent on usMsg           */
    /******************************************************************/
    switch ( usMsg )
    {
      case WM_EQFN_CREATED :
      case WM_EQFN_PROPERTIESCHANGED:
        /**************************************************************/
        /* Add or update filter in listbox                            */
        /**************************************************************/
        usRC = FiltLoadFilterHeader( pszFilter, &pIda->FiltProp );
        if ( usRC == NO_ERROR )
        {
          strcpy( pIda->szItemText, pszFilter );
          strcat( pIda->szItemText, X15_STR );
          sItem = SEARCHITEMPREFIXHWND( hwndLB, pIda->szItemText );
          strcat( pIda->szItemText, pIda->FiltProp.szDescription );
          if ( sItem != LIT_NONE )
          {
            DELETEITEMHWND( hwndLB, sItem );
          } /* endif */

          INSERTITEMHWND( hwndLB, pIda->szItemText );

        } /* endif */
        break;

      case WM_EQFN_DELETED :
        /**************************************************************/
        /* Delete filter from listbox                                 */
        /**************************************************************/
        strcpy( pIda->szItemText, pszFilter );
        strcat( pIda->szItemText, X15_STR );
        sItem = SEARCHITEMPREFIXHWND( hwndLB, pIda->szItemText );
        if ( sItem != LIT_NONE )
        {
          DELETEITEMHWND( hwndLB, sItem );
        } /* endif */
        break;

      default :
        /**************************************************************/
        /* ignore unknown message                                     */
        /**************************************************************/
        break;
    } /* endswitch */
  }
  else
  {
    /******************************************************************/
    /* (Re)fill listbox with names and descriptions of available      */
    /* filters                                                        */
    /******************************************************************/

    /******************************************************************/
    /* clear listbox                                                  */
    /******************************************************************/
    WinSendMsg( hwndLB, LM_DELETEALL, 0L, 0L );

    /******************************************************************/
    /* Setup search path                                              */
    /******************************************************************/
    UtlMakeEQFPath( pIda->szSearchPath, NULC, PROPERTY_PATH, NULL );
    strcat( pIda->szSearchPath, BACKSLASH_STR );
    strcat( pIda->szSearchPath, DEFAULT_PATTERN_NAME );
    strcat( pIda->szSearchPath, EXT_OF_FILTPROP );

    /******************************************************************/
    /* Loop over all filter property files and add filters to listbox */
    /******************************************************************/
    usCount = 1;
    usRC = UtlFindFirst( pIda->szSearchPath, &hDirHandle, FILE_NORMAL,
                         &pIda->ResultBuf, sizeof(FILEFINDBUF),
                         &usCount, 0L, TRUE );
    usCount = ( usRC ) ? 0 : usCount;
    while( usCount)
    {
      /****************************************************************/
      /* Get filter description                                       */
      /****************************************************************/
      Utlstrccpy( pIda->szItemText, RESBUFNAME(pIda->ResultBuf), DOT );
        usRC = FiltLoadFilterHeader( pIda->szItemText, &pIda->FiltProp );
      if ( usRC == NO_ERROR )
      {
        strcat( pIda->szItemText, X15_STR );
        strcat( pIda->szItemText, pIda->FiltProp.szDescription );
        INSERTITEMHWND( hwndLB, pIda->szItemText );
      } /* endif */

      usRC = UtlFindNext( hDirHandle, &pIda->ResultBuf, sizeof(FILEFINDBUF),
                          &usCount, TRUE );
      usCount = ( usRC ) ? 0 : usCount;
    } /* endwhile */
    // close search file handle
    if ( hDirHandle != HDIR_CREATE ) UtlFindClose( hDirHandle, FALSE );

  } /* endif */
} /* end of function FiltRefreshListbox */

INT_PTR CALLBACK FILTCONTROLDUMMYDLGPROC
(
   HWND            hwndDlg,            // handle of dialog window
   WINMSG          msg,                // message id
   WPARAM          mp1,                // message parameter or NULL
   LPARAM          mp2                 // message parameter or NULL
)
{
  PFILTTEMPL       pTempl;             // ptr to filter control template
  HWND             hwndControl;        // handle of current control

  mp1;
  if ( msg == WM_INITDLG )
  {
	HMODULE hResMod;
	hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
    /******************************************************************/
    /* Copy size and position of controls to filter control template  */
    /******************************************************************/
    pTempl =(PFILTTEMPL) PVOIDFROMMP2(mp2);

    /******************************************************************/
    /* ... groupbox                                                   */
    /******************************************************************/
    hwndControl = WinWindowFromID( hwndDlg, ID_FILTER_GB );
    QUERYTEXTHWND( hwndControl, pTempl->szGroupBox );
    pTempl->ulGroupBoxStyle = WinQueryWindowULong( hwndControl, QWL_STYLE );

    /******************************************************************/
    /* ... checkbox                                                   */
    /******************************************************************/
    hwndControl = WinWindowFromID( hwndDlg, ID_FILTER_CHECKBOX );
    QUERYTEXTHWND( hwndControl, pTempl->szCheckBox );
    pTempl->ulCheckBoxStyle = WinQueryWindowULong( hwndControl, QWL_STYLE );
    WinQueryWindowPos( hwndControl, &(pTempl->swpCheckBox) );

    /******************************************************************/
    /* ... edit pushbutton                                            */
    /******************************************************************/
    hwndControl = WinWindowFromID( hwndDlg, ID_FILTER_EDIT_PB );
    LOADSTRING( NULLHANDLE, hResMod, SID_FILTER_EDIT_PB, pTempl->szEditPB );
    pTempl->ulEditPBStyle = WinQueryWindowULong( hwndControl, QWL_STYLE );
    WinQueryWindowPos( hwndControl, &(pTempl->swpEditPB) );

    /******************************************************************/
    /* ... filter name static                                         */
    /******************************************************************/
    hwndControl = WinWindowFromID( hwndDlg, ID_FILTER_NAME_TEXT );
    QUERYTEXTHWND( hwndControl, pTempl->szName );
    pTempl->ulNameStyle = WinQueryWindowULong( hwndControl, QWL_STYLE );
    WinQueryWindowPos( hwndControl, &(pTempl->swpName) );

    /******************************************************************/
    /* ... filter combobox                                            */
    /******************************************************************/
    hwndControl = WinWindowFromID( hwndDlg, ID_FILTER_COMBO );
    pTempl->ulComboStyle = WinQueryWindowULong( hwndControl, QWL_STYLE );
    WinQueryWindowPos( hwndControl, &(pTempl->swpCombo) );

    /******************************************************************/
    /* ... filter description static                                  */
    /******************************************************************/
    hwndControl = WinWindowFromID( hwndDlg, ID_FILTER_DESCR_TEXT );
    QUERYTEXTHWND( hwndControl, pTempl->szDescr );
    pTempl->ulDescrStyle = WinQueryWindowULong( hwndControl, QWL_STYLE );
    WinQueryWindowPos( hwndControl, &(pTempl->swpDescr) );

    /******************************************************************/
    /* ... filter description entryfield                              */
    /******************************************************************/
    hwndControl = WinWindowFromID( hwndDlg, ID_FILTER_DESCR_EF );
    pTempl->ulDescrEFStyle = WinQueryWindowULong( hwndControl, QWL_STYLE );
    WinQueryWindowPos( hwndControl, &(pTempl->swpDescrEF) );

    /******************************************************************/
    /* Removed the dummy dialog                                       */
    /******************************************************************/
    WinDismissDlg( hwndDlg, TRUE );
    return( MRFROMSHORT( TRUE ) );
  }
  else
  {
    return ( WinDefDlgProc( hwndDlg, msg, mp1, mp2 ) );
  };
}
