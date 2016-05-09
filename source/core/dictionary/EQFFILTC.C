//+----------------------------------------------------------------------------+
//|  EQFFILTC.C - EQF Filter Control                                           |
//+----------------------------------------------------------------------------+
//|  Copyright Notice:                                                         |
//|                                                                            |
//|      Copyright (C) 1990-2012, International Business Machines              |
//|      Corporation and others. All rights reserved                           |

//+----------------------------------------------------------------------------+
//|  Description:                                                              |
//|      The filter control is a PM control which contains a filter selection  |
//|      combobox and a description text field. The data in these child        |
//|      controls is controlled by the filter control itself. The filter       |
//|      control also calls the filter dialog once a filter in the filter      |
//|      selection combobox is double-clicked.                                 |
//|                                                                            |
//|      The filter control can be added to dialog templates by adding a       |
//|      CONTROL statement for a control of the class WC_EQF_FILTER.           |
//|      The only prerequisite is that the filter handler is started before    |
//|      any window containing filter controls is created.                     |
//+----------------------------------------------------------------------------+
//|  Entry Points:                                                             |
//|                                                                            |
//|  FiltControlWP         window procedure for filter controls                |
//|                                                                           |
//|+-- status ("H"=Header,"D"=Design,"C"=Code,"T"=Test, " "=complete,          |
//|            "Q"=Quick-and-dirty )                                           |
//+----------------------------------------------------------------------------+
//|  Externals:                                                                |
//+----------------------------------------------------------------------------+
//|  Internals:                                                                |
//|                                                                            |
//|  FiltRefreshCombo    Refresh combobox                                      |
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

#include "OtmDictionaryIF.H"
#include "eqffilti.h"                  // Filter private include file

#include "eqfstart.id"                 // general IDs (here IDs for text strings)
#include "eqffilt.id"                  // filter IDs

typedef struct _FILTCONTROLIDA
{
  HWND      hwnd;                      // handle of filter control
  HWND      hwndDlg;                   // handle of dialog window
  HWND      hwndGB;                    // handle of filter groupbox
  HWND      hwndCheck;                 // handle of filter check box
  HWND      hwndCombo;                 // handle of filter combobox
  HWND      hwndPB;                    // handle of filter pushbutton
  HWND      hwndNameStatic;            // handle of filter name static text
  HWND      hwndDescrStatic;           // handle of filter description static text
  HWND      hwndDescrEF;               // handle of description entry field
  HWND      hwndLB;                    // handle of filter name listbox
  HWND      hwndHandler;               // handle of filter handler object window
  CHAR      szDescription[MAX_DESCRIPTION+1]; // buffer for filter description
  CHAR      szFilter[MAX_FNAME];       // buffer for filter name
  OBJNAME   szObjName;                 // buffer for filter control object name
  USHORT    usCharWidth;               // average character width
  USHORT    usCharHeight;              // character height
  HDCB      hDCB;                      // handle of dictionary currently processed
  CHAR      szDictName[MAX_FNAME];     // name of dictionary
} FILTCONTROLIDA, *PFILTCONTROLIDA;

BOOL FiltRefreshCombo( PFILTCONTROLIDA pIda );
MRESULT FiltCtrlControl( HWND, SHORT, SHORT );
MRESULT FiltCtrlCommand( HWND, SHORT, SHORT );

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     FiltControlWP          Filter control Window Procedure   |
//+----------------------------------------------------------------------------+
//|Function call:     FiltControlWP( HWND hwnd. USHORT msg, MPARAM mp1,        |
//|                                  MPARAM mp2 );                             |
//|                   :note.                                                   |
//|                   The function is called by PM for all messages for        |
//|                   filter control windows                                   |
//|                   :enote.                                                  |
//+----------------------------------------------------------------------------+
//|Description:       Procedure for PM controls with the WC_EQF_FILTER class   |
//|                                                                            |
//|                                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Input parameter:   HWND    hwnd        window handle of filter control      |
//|                   USHORT  msg         message ID                           |
//|                   MPARAM  mp1         first message parameter              |
//|                   MPARAM  mp2         second message parameter             |
//+----------------------------------------------------------------------------+
//|Returncode type:   MRESULT                                                  |
//+----------------------------------------------------------------------------+
//|Returncodes:       result of message processing                             |
//+----------------------------------------------------------------------------+
//|Prerequesits:      filter resource module (hResMod) must have been loaded  |
//|                   the filter handler must have been started before creating|
//|                   windows containing filter controls                       |
//+----------------------------------------------------------------------------+
//|Function flow:     switch msg                                               |
//|                     case WM_CREATE:                                        |
//|                       allocate and anchor IDA                              |
//|                       fill IDA fields                                      |
//|                       create filte groupbox                                |
//|                       create filter selection spinbutton                   |
//|                       create filter pushbutton                             |
//|                       create description groupbox                          |
//|                       create description static test field                 |
//|                       create dummy listbox for filter names                |
//|                       refresh filter spinbutton                            |
//|                       register filter control to object manager            |
//|                       hide filter control window                           |
//|                       set default filter                                   |
//|                     case WM_SETWINDOWPARAMS:                               |
//|                       if text is set                                       |
//|                         search filter in listbox                           |
//|                         if found set current spinbutton value to filter    |
//|                       endif                                                |
//|                       if controldata is set                                |
//|                         check passed dictionary handle                     |
//|                         store handle in IDA                                |
//|                       endif                                                |
//|                     case WM_QUERYWINDOWPARAMS:                             |
//|                       if text is queried                                   |
//|                         copy current spinbutton value to buffer            |
//|                       endif                                                |
//|                       if text length is queried                            |
//|                         return length of spinbutton value                  |
//|                       endif                                                |
//|                     case WM_QUERYDLGCODE:                                  |
//|                       return DLG_STATIC and DLGC_TABONCLICK                |
//|                     case WM_HITTEST:                                       |
//|                       return HT_TRANSPARENT                                |
//|                     case WM_COMMAND:                                       |
//|                       switch command value                                 |
//|                         case filter pushbutton                             |
//|                           get filter name from spinbutton                  |
//|                           call filter dialog                               |
//|                       endswitch                                            |
//|                     case WM_EQFN_PROPERTIESCHANGED:                        |
//|                     case WM_EQFN_DELETED:                                  |
//|                     case WM_EQFN_CREATED:                                  |
//|                       refresh spinbutton data                              |
//|                     case WM_DESTROY:                                       |
//|                       free string buffer                                   |
//|                       free IDA                                             |
//|                     case WM_CONTROL:                                       |
//|                       switch control ID                                    |
//|                         case spinbutton                                    |
//|                           switch control message                           |
//|                             case spinbutton change                         |
//|                               get filter name                              |
//|                               get and display filter description           |
//|                               set enabled state of the filter pushbutton   |
//|                           endswitch                                        |
//|                       endswitch                                            |
//|                   endswitch                                                |
//+----------------------------------------------------------------------------+
MRESULT EXPENTRY FILTCONTROLWP
(
  HWND    hwnd,                        // window handle of filter control window
  WINMSG  msg,                         // message ID
  WPARAM  mp1,                         // first message parameter
  LPARAM  mp2                          // second message parameter
)
{
  PFILTCONTROLIDA pIda;                // pointer to filter control IDA
  MRESULT         mResult = FALSE;     // result value of message processing
  BOOL            fOK;                 // program flow control flag
  CREATESTRUCT   *pCreateData;         // window creation data
  RECTL           rectl;               // buffer for window rectangles
  HWND            hwndDlg;             // handle of dialog window
  SWP             Swp;                 // size/position of window controls
  PFILTTEMPL      pTempl;              // ptr to filter control template

  /********************************************************************/
  /* Do message dependend processing                                  */
  /********************************************************************/
  switch( msg )
  {
    /******************************************************************/
    /* Process WM_CREATE message:                                     */
    /* (is sent during creation of a filter control window)           */
    /*   allocate and anchor IDA,                                     */
    /*   create child windows and fill filter selection combobox,     */
    /*   register filter control to the object manager                */
    /******************************************************************/
    case WM_CREATE:
      /****************************************************************/
      /* Address create data structure                                */
      /****************************************************************/
      pCreateData =(CREATESTRUCT *) PVOIDFROMMP2(mp2);

      hwndDlg = GETPARENT( hwnd );

      /**************************************************************/
      /* Allocate and anchor IDA                                    */
      /**************************************************************/
      fOK = UtlAlloc( (PVOID *) &pIda, 0L, (LONG)sizeof(FILTCONTROLIDA),
                      ERROR_STORAGE );
      if( fOK )
      {
        fOK = (BOOL)ANCHORWNDIDA( hwnd, pIda);
        if ( !fOK )
        {
          UtlError( 0, MB_CANCEL, 0, NULL, SYSTEM_ERROR );
        } /* endif */
      } /* endif */

      /****************************************************************/
      /* Fill IDA fields                                              */
      /****************************************************************/
      if ( fOK )
      {
         pIda->hwnd         = hwnd;
         pIda->hwndDlg      = hwndDlg;
         pIda->usCharWidth  = (USHORT) UtlQueryULong( QL_AVECHARWIDTH );
         pIda->usCharHeight = (USHORT) UtlQueryULong( QL_PELSPERLINE );
         pIda->hwndHandler  = EqfQueryHandler( FILTERHANDLER );
      } /* endif */

      /****************************************************************/
      /* Get filter template pointer                                  */
      /****************************************************************/
      EqfSend2Handler( FILTERHANDLER, WM_EQF_PROCESSTASK,
                       MP1FROMSHORT(GETFILTERTEMPL_TASK),
                       MP2FROMP( &pTempl ) );

      /****************************************************************/
      /* Create filter groupbox                                       */
      /****************************************************************/
      if ( fOK )
      {
        pIda->hwndGB   = WinCreateWindow( pIda->hwndDlg,
                                          WC_BUTTON,
                                          pTempl->szGroupBox,
                                          pTempl->ulGroupBoxStyle | WS_CHILD,
                                          (SHORT)pCreateData->x,
                                          (SHORT)pCreateData->y,
                                          (SHORT)pCreateData->cx,
                                          (SHORT)pCreateData->cy,
                                          hwnd,
                                          HWND_BOTTOM,
                                          ID_FILTER_GB,
                                          NULL, NULL);

        if ( !pIda->hwndGB )
        {
          UtlError( 0, MB_CANCEL, 0, NULL, SYSTEM_ERROR );
          fOK = FALSE;
        } /* endif */
      } /* endif */

      /****************************************************************/
      /* Create filter checkbox                                       */
      /****************************************************************/
      if ( fOK )
      {
        memcpy( &Swp, &(pTempl->swpCheckBox), sizeof(Swp) );
        Swp.x = (SHORT)(Swp.x + pCreateData->x);
        Swp.y = (SHORT)(pCreateData->y + pCreateData->cy - Swp.y);
        pIda->hwndCheck = WinCreateWindow( pIda->hwndDlg, WC_BUTTON,
                                           pTempl->szCheckBox,
                                           pTempl->ulCheckBoxStyle | WS_CHILD,
                                           Swp.x, Swp.y, Swp.cx, Swp.cy,
                                           hwnd,
                                           hwnd,
                                           ID_FILTER_CHECKBOX,
                                           NULL, NULL);

        if ( !pIda->hwndCheck )
        {
          UtlError( 0, MB_CANCEL, 0, NULL, SYSTEM_ERROR );
          fOK = FALSE;
        } /* endif */
      } /* endif */

      /****************************************************************/
      /* Create filter edit pushbutton                                */
      /****************************************************************/
      if ( fOK )
      {
		HMODULE hResMod;
		hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
        memcpy( &Swp, &(pTempl->swpEditPB ), sizeof(Swp) );
        LOADSTRING( NULLHANDLE, hResMod, SID_FILTER_EDIT_PB, pTempl->szEditPB );
        Swp.x = (SHORT)(Swp.x + pCreateData->x);
        Swp.y = (SHORT)(pCreateData->y + pCreateData->cy - Swp.y);
        pIda->hwndPB   = WinCreateWindow( pIda->hwndDlg, WC_BUTTON,
                                          pTempl->szEditPB,
                                          pTempl->ulEditPBStyle | WS_CHILD,
                                          Swp.x, Swp.y, Swp.cx, Swp.cy,
                                          hwnd,
                                          pIda->hwndCheck,
                                          ID_FILTER_EDIT_PB,
                                          NULL, NULL);

        if ( !pIda->hwndPB )
        {
          UtlError( 0, MB_CANCEL, 0, NULL, SYSTEM_ERROR );
          fOK = FALSE;
        } /* endif */
      } /* endif */

      /****************************************************************/
      /* Create filter name static                                    */
      /****************************************************************/
      if ( fOK )
      {
        memcpy( &Swp, &(pTempl->swpName ), sizeof(Swp) );
        Swp.x = (SHORT)(Swp.x + pCreateData->x);
        Swp.y = (SHORT)(pCreateData->y + pCreateData->cy - Swp.y);
        pIda->hwndNameStatic = WinCreateWindow( pIda->hwndDlg, WC_STATIC,
                                          pTempl->szName,
                                          pTempl->ulNameStyle | WS_CHILD,
                                          Swp.x, Swp.y, Swp.cx, Swp.cy,
                                          hwnd,
                                          pIda->hwndPB,
                                          ID_FILTER_NAME_TEXT,
                                          NULL, NULL);

        if ( !pIda->hwndNameStatic )
        {
          UtlError( 0, MB_CANCEL, 0, NULL, SYSTEM_ERROR );
          fOK = FALSE;
        } /* endif */
      } /* endif */

      /****************************************************************/
      /* Create filter combobox                                       */
      /****************************************************************/
      if ( fOK )
      {
        memcpy( &Swp, &(pTempl->swpCombo), sizeof(Swp) );
        Swp.x = (SHORT)(Swp.x + pCreateData->x);
        Swp.y = (SHORT)(pCreateData->y + pCreateData->cy - Swp.y);
        // increase size as window seems to return the size of the entry
        // field of the combobox and not the overall size of the control
        Swp.cy += 2 * (SHORT)UtlQueryULong( QL_PELSPERLINE );
        pIda->hwndCombo = WinCreateWindow( pIda->hwndDlg, WC_COMBOBOX, NULL,
                                           pTempl->ulComboStyle | WS_VSCROLL | WS_CHILD,
                                           Swp.x, Swp.y, Swp.cx, Swp.cy,
                                           hwnd,
                                           pIda->hwndNameStatic,
                                           ID_FILTER_COMBO,
                                           NULL, NULL);

        if ( !pIda->hwndCombo )
        {
          UtlError( 0, MB_CANCEL, 0, NULL, SYSTEM_ERROR );
          fOK = FALSE;
        }
        else
        {
          CBSETTEXTLIMITHWND( pIda->hwndCombo, MAX_FNAME - 1 );
        } /* endif */
      } /* endif */

      /****************************************************************/
      /* Create description static                                    */
      /****************************************************************/
      if ( fOK )
      {
        memcpy( &Swp, &(pTempl->swpDescr), sizeof(Swp) );
        Swp.x = (SHORT)(Swp.x + pCreateData->x);
        Swp.y = (SHORT)(pCreateData->y + pCreateData->cy - Swp.y);
        pIda->hwndDescrStatic = WinCreateWindow( pIda->hwndDlg, WC_STATIC,
                                        pTempl->szDescr,
                                        pTempl->ulDescrStyle | WS_CHILD,
                                        Swp.x, Swp.y, Swp.cx, Swp.cy,
                                        hwnd,
                                        pIda->hwndCombo,
                                        ID_FILTER_DESCR_TEXT,
                                        NULL, NULL);
        if ( !pIda->hwndDescrStatic )
        {
          UtlError( 0, MB_CANCEL, 0, NULL, SYSTEM_ERROR );
          fOK = FALSE;
        } /* endif */
      } /* endif */

      /****************************************************************/
      /* Create description entryfield                                */
      /****************************************************************/
      if ( fOK )
      {
        memcpy( &Swp, &(pTempl->swpDescrEF), sizeof(Swp) );
        Swp.x = (SHORT)(Swp.x + pCreateData->x);
        Swp.y = (SHORT)(pCreateData->y + pCreateData->cy - Swp.y);
        pIda->hwndDescrEF = WinCreateWindow( pIda->hwndDlg, WC_ENTRYFIELD,
                                             "",
                                             pTempl->ulDescrEFStyle | WS_CHILD,
                                             Swp.x, Swp.y, Swp.cx, Swp.cy,
                                             hwnd,
                                             pIda->hwndDescrStatic,
                                             ID_FILTER_DESCR_EF,
                                             NULL, NULL);
        if ( !pIda->hwndDescrEF )
        {
          UtlError( 0, MB_CANCEL, 0, NULL, SYSTEM_ERROR );
          fOK = FALSE;
        }
        else
        {
          SETTEXTLIMITHWND( pIda->hwndDescrEF, (MAX_DESCRIPTION - 1) );
        } /* endif */
      } /* endif */

      /****************************************************************/
      /* Create dummy listbox for filter names                        */
      /****************************************************************/
      if ( fOK )
      {
        pIda->hwndLB    = WinCreateWindow( hwnd, WC_LISTBOX, "",
                                        LBS_STANDARD | WS_CHILD,
                                        0, 0, 0, 0,
                                        hwnd,
                                        HWND_BOTTOM,
                                        6,               // dummy id
                                        NULL, NULL);
        if ( !pIda->hwndLB )
        {
          UtlError( 0, MB_CANCEL, 0, NULL, SYSTEM_ERROR );
          fOK = FALSE;
        } /* endif */
      } /* endif */

      /****************************************************************/
      /* Refresh combobox                                             */
      /****************************************************************/
      if ( fOK )
      {
        fOK = FiltRefreshCombo( pIda );
      } /* endif */

//    /****************************************************************/
//    /* Register filter control to MAT Tools object manager          */
//    /****************************************************************/
//    if ( fOK )
//    {
//      /**************************************************************/
//      /* use the window handle to create an unique object name for  */
//      /* the filter control                                         */
//      /**************************************************************/
//      sprintf( pIda->szObjName, "Filter:%ld", (LONG)hwnd );
//
//      /**************************************************************/
//      /* Register the filter control to the object manager          */
//      /**************************************************************/
//        EqfRegisterObject( pIda->szObjName, hwnd, clsFILTER );
//    } /* endif */

      /****************************************************************/
      /* Hide filter control window                                   */
      /****************************************************************/
      if ( fOK )
      {
        WinShowWindow( hwnd, FALSE );
        WinEnableWindow( hwnd, FALSE );
        WinSetWindowPos( hwnd, HWND_BOTTOM, 0, 0, 0, 0,
                         EQF_SWP_SIZE | EQF_SWP_MOVE | EQF_SWP_ZORDER );
      } /* endif */

      /****************************************************************/
      /* Set default filter (= no filter)                             */
      /****************************************************************/
      if ( fOK  )
      {
        WinSetWindowText( hwnd, EMPTY_STRING );
        SETCHECKHWND( pIda->hwndCheck, FALSE );
        PostMessage( hwndDlg, WM_COMMAND,
                     MP1FROMSHORT( ID_FILTER_CHECKBOX ),
                     MP2FROM2SHORT( 0, BN_CLICKED ) );
      } /* endif */

      if ( !fOK )
      {
        mResult = MRFROMSHORT( DO_NOT_CREATE );  // do not create the window
      } /* endif */
      break;

    case WM_SETFONT :
      pIda = ACCESSWNDIDA( hwnd, PFILTCONTROLIDA );
      SendMessage( pIda->hwndGB,          WM_SETFONT, mp1, mp2 );
      SendMessage( pIda->hwndCheck,       WM_SETFONT, mp1, mp2 );
      SendMessage( pIda->hwndPB,          WM_SETFONT, mp1, mp2 );
      SendMessage( pIda->hwndNameStatic,  WM_SETFONT, mp1, mp2 );
      SendMessage( pIda->hwndCombo,       WM_SETFONT, mp1, mp2 );
      SendMessage( pIda->hwndDescrStatic, WM_SETFONT, mp1, mp2 );
      SendMessage( pIda->hwndDescrEF,     WM_SETFONT, mp1, mp2 );
      break;

    /******************************************************************/
    /* Process the WM_SETTEXT message:                                */
    /******************************************************************/
    case WM_SETTEXT :
      {
        SHORT   sItem;

        pIda = ACCESSWNDIDA( hwnd, PFILTCONTROLIDA );
        sItem = CBSEARCHITEMHWND( pIda->hwndCombo, mp2 );
        if ( sItem != LIT_NONE )
        {
          CBSELECTITEMHWND( pIda->hwndCombo, sItem );
          SETCHECKHWND( pIda->hwndCheck, TRUE );
          PostMessage( hwnd, WM_COMMAND,
                       MP1FROMSHORT( ID_FILTER_CHECKBOX ),
                       MP2FROM2SHORT( 0, BN_CLICKED ) );
        }
        else
        {
          SETTEXTHWND( pIda->hwndCombo, EMPTY_STRING );
          SETTEXTHWND( pIda->hwndDescrEF, EMPTY_STRING );
        } /* endif */
        mResult = MRFROMSHORT( TRUE ); // text has been set
      } /* endif */
    break;

   case WM_EQF_FILTSETDICT :
      pIda = ACCESSWNDIDA( hwnd, PFILTCONTROLIDA );
      if ( AsdIsDcbOK( (HDCB)mp2 ) )
      {
        pIda->hDCB = (HDCB)mp2;
        mResult = MRFROMSHORT( TRUE ); // dictionary handle has been set
      }
      else
      {
        mResult = MRFROMSHORT( FALSE ); // invalid control data
      } /* endif */
      break;

    /******************************************************************/
    /* Process the WM_GETTEXT message:                                */
    /******************************************************************/
    case WM_GETTEXT:
      pIda = ACCESSWNDIDA( hwnd, PFILTCONTROLIDA );

      if ( QUERYCHECKHWND( pIda->hwndCheck ) )
      {
        QUERYTEXTHWND( pIda->hwndCombo, pIda->szFilter );
        UtlStripBlanks( pIda->szFilter );
        strncpy((char*) PVOIDFROMMP2(mp2), pIda->szFilter, SHORTFROMMP1(mp1) - 1 );
        ((PSZ)PVOIDFROMMP2(mp2))[SHORTFROMMP1(mp1) - 1] = EOS;
      }
      else
      {
        ((PSZ)PVOIDFROMMP2(mp2))[0] = EOS;
      } /* endif */
      mResult = MRFROMSHORT( TRUE );         // text has been returned
      break;

    case WM_GETTEXTLENGTH:
      pIda = ACCESSWNDIDA( hwnd, PFILTCONTROLIDA );
      if ( QUERYCHECKHWND( pIda->hwndCheck ) )
      {
        QUERYTEXTHWND( pIda->hwndCombo, pIda->szFilter );
        UtlStripBlanks( pIda->szFilter );
        mResult = MRFROMSHORT( strlen( pIda->szFilter ) );
      }
      else
      {
        mResult = MRFROMSHORT( 0 );
      } /* endif */
      break;

    /******************************************************************/
    /* Process WM_COMMAND messages                                    */
    /******************************************************************/
    case WM_COMMAND:
      mResult = FiltCtrlCommand( hwnd, SHORT1FROMMP1(mp1),
                                 SHORT2FROMMP2( mp2 ) );
      break;

    /******************************************************************/
    /* Process notification messages WM_EQFN_PROPERTIESCHANGED,       */
    /* WM_EQFN_DELETED and WM_EQFN_CREATED:                           */
    /*   update filter selection listbox                              */
    /******************************************************************/
    case WM_EQFN_PROPERTIESCHANGED:
    case WM_EQFN_DELETED:
    case WM_EQFN_CREATED:
      if ( SHORT1FROMMP1(mp1) == clsFILTER )
      {
        pIda = ACCESSWNDIDA( hwnd, PFILTCONTROLIDA );
        fOK = FiltRefreshCombo( pIda );
        if ( msg == WM_EQFN_DELETED )
        {

        } /* endif */
      } /* endif */
      break;

    /******************************************************************/
    /* Process the WM_DESTROY message:                                */
    /* (the message is sent during processing of WinDestroyWindow)    */
    /*   free IDA                                                     */
    /******************************************************************/
    case WM_DESTROY:
      pIda = ACCESSWNDIDA( hwnd, PFILTCONTROLIDA );
      if ( pIda )
      {
        UtlAlloc( (PVOID *) &pIda, 0L, 0L, NOMSG );
      } /* endif */
      break;

    /******************************************************************/
    /* Process the WM_SIZE message:                                   */
    /*   Resize and position child controls                           */
    /******************************************************************/
    case WM_SIZE:
      pIda = ACCESSWNDIDA( hwnd, PFILTCONTROLIDA );
      WinQueryWindowRect( hwnd, &rectl);
      break;

    /******************************************************************/
    /* pass all other messages to the default window procedure        */
    /******************************************************************/
    default:
      mResult = WinDefWindowProc( hwnd, msg, mp1, mp2 );
      break;

  } /* endswitch */

  return( mResult );
} /* end of function FiltControlWP */


MRESULT FiltCtrlCommand
(
   HWND  hwnd,                         // dialog handle
   SHORT sId,                          // id of button
   SHORT sNotification                 // notification type
)
{
  BOOL             fOK;                // internal OK flag
  CHAR            szFilter[MAX_FNAME]; // buffer for filter names
  SHORT           sItem;               // listbox item index
  PFILTCONTROLIDA pIda;                // pointer to filter control IDA

  switch ( sId )
  {
    case ID_FILTER_EDIT_PB :
      pIda = ACCESSWNDIDA( hwnd, PFILTCONTROLIDA );

      /************************************************************/
      /* Get filter name                                          */
      /************************************************************/
      QUERYTEXTHWND( pIda->hwndCombo, pIda->szFilter );
      UtlStripBlanks( pIda->szFilter );

      /************************************************************/
      /* Call filter dialog                                       */
      /************************************************************/
      strcpy( szFilter, pIda->szFilter );
      DictionaryFilter( pIda->hDCB, szFilter, pIda->hwndDlg );
      fOK = FiltRefreshCombo( pIda );
      if ( szFilter[0] != EOS )
      {
        SETTEXTHWND( pIda->hwnd, szFilter );
      }
      else if ( pIda->szFilter[0] != EOS )
      {
        /**********************************************************/
        /* Check if current filter is still in filter combobox    */
        /**********************************************************/
        sItem = SEARCHITEMHWND( pIda->hwndCombo, pIda->szFilter );
        if ( (sItem == LIT_NONE) || (sItem == LIT_ERROR) )
        {
          SETTEXTHWND( pIda->hwnd, EMPTY_STRING );
        } /* endif */
      } /* endif */
      break;
    case ID_FILTER_COMBO:
    case ID_FILTER_CHECKBOX:
      FiltCtrlControl( hwnd, sId, sNotification );
      break;
    default :
      break;
  } /* endswitch */
  return( MRFROMSHORT( TRUE ) );
} /* end of function FiltCtrlCommand */


MRESULT FiltCtrlControl
(
   HWND   hwnd,                        // dialog handle
   SHORT  sId,                         // id in action
   SHORT  sNotification                // notification
)
{
  PFILTCONTROLIDA pIda;                // pointer to filter control IDA

  switch ( sId )
  {
    case ID_FILTER_COMBO:
      switch ( sNotification )
      {
        case CBN_EFCHANGE:
          pIda = ACCESSWNDIDA( hwnd, PFILTCONTROLIDA );

          /********************************************************/
          /* Get filter name                                      */
          /********************************************************/
          QUERYTEXTHWND( pIda->hwndCombo, pIda->szFilter );
          UtlStripBlanks( pIda->szFilter );

          /********************************************************/
          /* Get and display filter description                   */
          /********************************************************/
          strcpy( pIda->szDescription, pIda->szFilter );
          WinSendMsg( pIda->hwndHandler, WM_EQF_PROCESSTASK,
                      MP1FROMSHORT( GETFILTERDESCR_TASK ),
                      MP2FROMP( pIda->szDescription ) );
          OEMTOANSI( pIda->szDescription );
          SETTEXTHWND( pIda->hwndDescrEF, pIda->szDescription );

          /********************************************************/
          /* Set enabled state of filter pushbutton               */
          /********************************************************/
          ENABLECTRL( pIda->hwndDlg, ID_FILTER_EDIT_PB,
                      pIda->szFilter[0] != EOS );
          break;

        case CBN_SELCHANGE:
          pIda = ACCESSWNDIDA( hwnd, PFILTCONTROLIDA );

          /********************************************************/
          /* Get filter name                                      */
          /********************************************************/
          {
            SHORT sItem;

            sItem = CBQUERYSELECTIONHWND( pIda->hwndCombo );
            if ( sItem != LIT_NONE )
            {
              SendMessage( pIda->hwndCombo, CB_GETLBTEXT, sItem, (LPARAM)pIda->szFilter );
            }
            else
            {
              pIda->szFilter[0] = EOS;         // no name is selected
            } /* endif */
          }
          UtlStripBlanks( pIda->szFilter );

          /********************************************************/
          /* Get and display filter description                   */
          /********************************************************/
          strcpy( pIda->szDescription, pIda->szFilter );
          WinSendMsg( pIda->hwndHandler, WM_EQF_PROCESSTASK,
                      MP1FROMSHORT( GETFILTERDESCR_TASK ),
                      MP2FROMP( pIda->szDescription ) );
          OEMTOANSI( pIda->szDescription );
          SETTEXTHWND( pIda->hwndDescrEF, pIda->szDescription );

          /********************************************************/
          /* Set enabled state of filter pushbutton               */
          /********************************************************/
          ENABLECTRL( pIda->hwndDlg, ID_FILTER_EDIT_PB,
                      pIda->szFilter[0] != EOS );
          break;
      } /* endswitch */
      break;


    case ID_FILTER_CHECKBOX:
      switch ( sNotification )
      {
        case BN_CLICKED:
          {
            BOOL   fChecked;

            pIda = ACCESSWNDIDA( hwnd, PFILTCONTROLIDA );
            fChecked = QUERYCHECK( pIda->hwndDlg, ID_FILTER_CHECKBOX );
            ENABLECTRL( pIda->hwndDlg, ID_FILTER_EDIT_PB, fChecked );
            ENABLECTRL( pIda->hwndDlg, ID_FILTER_COMBO, fChecked );
            ENABLECTRL( pIda->hwndDlg, ID_FILTER_DESCR_EF, fChecked );
            ENABLECTRL( pIda->hwndDlg, ID_FILTER_NAME_TEXT, fChecked );
            ENABLECTRL( pIda->hwndDlg, ID_FILTER_DESCR_TEXT, fChecked );
          }
          break;
      } /* endswitch */
      break;
  } /* endswitch */
  return( MRFROMSHORT(FALSE) );
} /* end of function FiltCtrlControl */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     FiltRefreshCombo    Refresh combobox contents            |
//+----------------------------------------------------------------------------+
//|Function call:     FiltRefreshCombo( pIda );                                |
//+----------------------------------------------------------------------------+
//|Description:       Refresh contents of filter combobox                      |
//+----------------------------------------------------------------------------+
//|Input parameter:   PFILTCONTROLIDA     pIda    pointer to filter control IDA|
//+----------------------------------------------------------------------------+
//|Returncode type:   BOOL                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:       TRUE    combobox has been refreshed                      |
//|                   FALSE   refresh failed due to memory shortage            |
//+----------------------------------------------------------------------------+
//|Function flow:     disable window update of combobox                        |
//|                   fill combobox with filter names                          |
//|                   enable window update of combobox                         |
//+----------------------------------------------------------------------------+
BOOL FiltRefreshCombo
(
  PFILTCONTROLIDA pIda                 // pointer to filter control IDA
)
{
  BOOL            fOK = TRUE;          // internal OK flag
  CHAR            szFilter[MAX_FNAME];

  /********************************************************************/
  /* Disable window update of combobox                                */
  /********************************************************************/
  ENABLEUPDATEHWND_FALSE( pIda->hwndCombo );
  QUERYTEXTHWND( pIda->hwndCombo, szFilter );

  /****************************************************************/
  /* Fill combobox with filter names                              */
  /****************************************************************/
  if ( fOK )
  {
    CBDELETEALLHWND( pIda->hwndCombo );
    EqfSend2Handler( FILTERHANDLER, WM_EQF_INSERTNAMES,
                     MP1FROMHWND( pIda->hwndCombo ), 0L );
  } /* endif */

  /********************************************************************/
  /* Enable window update of combobox                                 */
  /********************************************************************/
  SETTEXTHWND( pIda->hwndCombo, szFilter );
  ENABLEUPDATEHWND_TRUE( pIda->hwndCombo);

  return( fOK );
} /* end of function FiltRefreshCombo */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     IsFilterMessage     Check and handle filter messages     |
//+----------------------------------------------------------------------------+
//|Function call:     IsFilterMessage( hwnd, msg, mp1, mp2 )                   |
//+----------------------------------------------------------------------------+
//|Description:       Checks if the given message is to be processed by the    |
//|                   filter control and process the message approbriate       |
//+----------------------------------------------------------------------------+
//|Input parameter:   HWND             hwnd  handle of filter control          |
//|                   USHORT           usMsg message to be checked for         |
//|                   WPARAM           mp1   first message parameter           |
//|                   LPARAM           mp2   second message parameter          |
//+----------------------------------------------------------------------------+
//|Returncode type:   BOOL                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:       TRUE    message has been processed by the filter control |
//|                   FALSE   message has not been processed                   |
//+----------------------------------------------------------------------------+
BOOL IsFilterMessage
(
  HWND             hwnd,               // handle of filter control
  WINMSG           msg,                // message to be checked for
  WPARAM           mp1,                // first message parameter
  LPARAM           mp2                 // second message parameter
)
{
  BOOL             fProcessed = FALSE; // function return code
  mp2;
  switch ( msg )
  {
    case WM_COMMAND :
      switch ( SHORT1FROMMP1(mp1) )
      {
        case ID_FILTER_COMBO:
        case ID_FILTER_CHECKBOX:
          /************************************************************/
          /* Process control notification message                     */
          /************************************************************/
          FiltCtrlControl( hwnd, WMCOMMANDID( mp1, mp2 ),
                                 WMCOMMANDCMD( mp1, mp2 ) );
          fProcessed = TRUE;
          break;

        case ID_FILTER_EDIT_PB :
          /************************************************************/
          /* Process command message                                  */
          /************************************************************/
          FiltCtrlCommand( hwnd, WMCOMMANDID( mp1, mp2 ),
                                 WMCOMMANDCMD( mp1, mp2 ) );
          fProcessed = TRUE;
          break;

      } /* endswitch */
      break;
  } /* endswitch */

  return ( fProcessed );
} /* end of function IsFilterMessage */


