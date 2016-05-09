//+----------------------------------------------------------------------------+
//|EQFUTCLB.C    EQF Utilities: Column Listbox Control (CLB)                   |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|          Copyright (C) 1990-2014, International Business Machines          |
//|          Corporation and others. All rights reserved                       |
//|                                                                            |
//|                                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Author:          G. Queck (QSoft)                                           |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//|      The column listbox control (CLB) implements a listbox where the       |
//|      data of the listbox items is aligned in columns.                      |
//|      The columns can be reordered and hidden/displayed without changing    |
//|      the individual listbox items. There is a dialog available which       |
//|      allows the selection of displayed columns by the user. Later on       |
//|      a sort and filter function will be available.                         |
//+----------------------------------------------------------------------------+
//|Entry Points:                                                               |
//|      UtlRegisterCLB      - register the CLB window class                   |
//|      CLBWindowProc       - CLB window procedure                            |
//|                                                                           |
//|+-- status ("H"=Header,"D"=Design,"C"=Code,"T"=Test, " "=complete,          |
//|            "Q"=Quick-and-dirty )                                           |
//+----------------------------------------------------------------------------+
//|Externals:                                                                  |
//|    Utils.UtlAlloc                                                          |
//|    Utils.UtlLongToTimeString                                               |
//|    Utils.UtlLongToDateString                                               |
//|    Utils.UtlFDateToDateString                                              |
//|    Utils.UtlFTimeToDateString                                              |
//|    Utils.UtlQueryULong                                                     |
//|    Utils.UtlError                                                          |
//+----------------------------------------------------------------------------+
//|Internals:                                                                  |
//|      CLBActivateViewList   - Activate a specific view list                 |
//|      CLBCopyViewList       - Copy a view list                              |
//|      CLBCreate             - WM_CREATE processing for CLB                  |
//|      CLBDrawItem           - Draw a CLB listbox item                       |
//|      CLBDrawTitle          - Draw the column titles of the CLB             |
//|      CLBQueryItemWidth     - Compute the width of a CLB item               |
//|      SelectViewDlg         - Window procedure for 'Change View' dialog     |
//|                                                                           |
//|+-- status ("H"=Header,"D"=Design,"C"=Code,"T"=Test, " "=complete,          |
//|            "Q"=Quick-and-dirty )                                           |
//+----------------------------------------------------------------------------+
//|  Include files:                                                            |
//|      EQF.H               - general EQF .h file                             |
//|      TIME.H              - C time/date related functions                   |
//|      EQFUTCLB.ID         - CLB private dialog control IDs                  |
//|      EQFUTCLB.H"         - CLB private defines                             |
//+----------------------------------------------------------------------------+
//|To be done / known limitations / caveats:                                   |
//|  - the column string titles are copied using strdup rather than using      |
//|    UtlAlloc                                                                |
//|  - the sort function is not implemented yet                                |
//|  - the filter function is not implemented yet                              |
//+----------------------------------------------------------------------------+
//| PVCS Section                                                               |
//
// $CMVC
// 
// $Revision: 1.1 $ ----------- 14 Dec 2009
//  -- New Release TM6.2.0!!
// 
// 
// $Revision: 1.1 $ ----------- 1 Oct 2009
//  -- New Release TM6.1.8!!
// 
// 
// $Revision: 1.1 $ ----------- 2 Jun 2009
//  -- New Release TM6.1.7!!
// 
// 
// $Revision: 1.1 $ ----------- 8 Dec 2008
//  -- New Release TM6.1.6!!
// 
// 
// $Revision: 1.1 $ ----------- 23 Sep 2008
//  -- New Release TM6.1.5!!
// 
// 
// $Revision: 1.1 $ ----------- 23 Apr 2008
//  -- New Release TM6.1.4!!
// 
// 
// $Revision: 1.1 $ ----------- 13 Dec 2007
//  -- New Release TM6.1.3!!
// 
// 
// $Revision: 1.1 $ ----------- 29 Aug 2007
//  -- New Release TM6.1.2!!
// 
// 
// $Revision: 1.1 $ ----------- 20 Apr 2007
//  -- New Release TM6.1.1!!
// 
// 
// $Revision: 1.1 $ ----------- 20 Dec 2006
//  -- New Release TM6.1.0!!
// 
// 
// $Revision: 1.1 $ ----------- 9 May 2006
//  -- New Release TM6.0.11!!
// 
// 
// $Revision: 1.1 $ ----------- 20 Dec 2005
//  -- New Release TM6.0.10!!
// 
// 
// $Revision: 1.1 $ ----------- 16 Sep 2005
//  -- New Release TM6.0.9!!
// 
// 
// $Revision: 1.1 $ ----------- 18 May 2005
//  -- New Release TM6.0.8!!
// 
// 
// $Revision: 1.1 $ ----------- 29 Nov 2004
//  -- New Release TM6.0.7!!
// 
// 
// $Revision: 1.1 $ ----------- 30 Aug 2004
//  -- New Release TM6.0.6!!
// 
// 
// $Revision: 1.1 $ ----------- 3 May 2004
//  -- New Release TM6.0.5!!
// 
// 
// $Revision: 1.1 $ ----------- 15 Dec 2003
//  -- New Release TM6.0.4!!
// 
// 
// $Revision: 1.2 $ ----------- 5 Nov 2003
// GQ: - allow definitionof up to MAX_DEFINEDCOLUMNS view columns (viewable columns are
//       restricted to MAX_VIEW)
// 
// 
// $Revision: 1.1 $ ----------- 6 Oct 2003
//  -- New Release TM6.0.3!!
// 
// 
// $Revision: 1.1 $ ----------- 27 Jun 2003
//  -- New Release TM6.0.2!!
// 
// 
// $Revision: 1.3 $ ----------- 17 Mar 2003
// --RJ: removed compiler defines not needed any more and rework code to avoid warnings
// 
//
// $Revision: 1.2 $ ----------- 26 Feb 2003
// --RJ: removed compiler defines not needed any more and rework code to avoid warnings
//
//
// $Revision: 1.1 $ ----------- 20 Feb 2003
//  -- New Release TM6.0.1!!
//
//
// $Revision: 1.3 $ ----------- 4 Sep 2002
// --RJ: R07197: replace isdbcs1 by isdbcs1ex, del. initdbcs()
//
//
// $Revision: 1.2 $ ----------- 29 Jul 2002
// --RJ: R07197: add cp for conversion
//
//
// $Revision: 1.1 $ ----------- 26 Jul 2002
//  -- New Release TM6.0!!
//
//
// $Revision: 1.3 $ ----------- 6 Nov 2001
// RJ: get rid of compiler warnings
//
//
// $Revision: 1.2 $ ----------- 3 Sep 2001
// -- RJ: Unicode enabling
//
//
// $Revision: 1.1 $ ----------- 16 Aug 2001
//  -- New Release TM2.7.2!!
//
// $Revision: 1.8 $ ----------- 17 Jan 2001
// - allow compile of TQM
//
//
// $Revision: 1.7 $ ----------- 16 Oct 2000
// added context help
//
//
// $Revision: 1.6 $ ----------- 24 Aug 2000
// -- KBT0835: fix problem with old GUI and MAX_VIEW reached..
//
//
// $Revision: 1.5 $ ----------- 18 Jul 2000
// - added filter handling
//
//
// $Revision: 1.4 $ ----------- 24 May 2000
// - fixed 'internal error' problem with old GUI and OS/2 version
//
//
// $Revision: 1.3 $ ----------- 22 May 2000
// - added sort list handling
//
//
//
// $Revision: 1.2 $ ----------- 6 Dec 1999
//  -- Initial Revision!!
//
/*
 * $Header:   K:\DATA\EQFUTCLB.CV_   1.26   07 Dec 1998 10:33:42   BUILD  $
 *
 * $Log:   K:\DATA\EQFUTCLB.CV_  $
 *
 *    Rev 1.26   07 Dec 1998 10:33:42   BUILD
 * -- substitute WinSetWindowPtr through WINSETWINDOWPTR
 *
 *    Rev 1.25   23 Nov 1998 10:13:28   BUILD
 * - pass LB_FINDSTRINGEXACT message to listbox control
 *
 *    Rev 1.24   23 Jul 1998 08:53:04   BUILD
 * - fixed PTM KBT0344; Trap in TM/Win when import after delete
 *
 *    Rev 1.23   13 Jul 1998 11:27:10   BUILD
 * - return correct return code for message LM_QUERYITEMTEXT
 *
 *    Rev 1.22   29 Jun 1998 16:38:38   BUILD
 * correct for windows
 *
 *    Rev 1.20   22 Jun 1998 16:58:30   BUILD
 * - added missing semikolon
 *
 *    Rev 1.19   18 May 1998 13:29:20   BUILD
 * - implemneted work item R004360: "Allow more than 300 docs per folder"
 *
 *    Rev 1.18   20 Apr 1998 11:56:38   BUILD
 * - supply additional parameter of UtlPrintOpen
 *
 *    Rev 1.17   07 Apr 1998 15:23:12   BUILD
 * fix for win 32 font problem
 *
 *    Rev 1.16   02 Mar 1998 09:51:42   BUILD
 * - Win32: fixed 'Del'-key-not-working-in-list-windows problem
 *
 *    Rev 1.15   09 Feb 1998 17:10:52   BUILD
 * - Win32: set correct control ID when passing WM_COMMAND message to
 *   CLB owner
 *
 *    Rev 1.14   19 Jan 1998 10:57:44   BUILD
 * - enable double click in column listbox for Win32 environment
 *
 *    Rev 1.13   14 Jan 1998 16:01:52   BUILD
 * - support Win32bit compile
 *
 *    Rev 1.12   12 Nov 1997 13:37:16   BUILD
 * - implemented work item A57m: Printing of column listbox windows
 *
 *    Rev 1.11   04 Jul 1997 14:00:14   BUILD
 * - fixed PTM KBT0035: Warp J Cannot import files with more than 28 characters
 *
 *    Rev 1.10   11 Jun 1997 17:19:46   BUILD
 * - apply AUTOWIDTH style of columns also for the column title
 *
 *    Rev 1.9   21 Mar 1997 12:15:12   BUILD
 * - introduced new column style AUTOWIDTHTEXT_DATA
 *
 *    Rev 1.8   26 Feb 1997 17:03:06   BUILD
 * -- Compiler defines for _POE22, _TKT21, and NEWTCSTUFF eliminated
 *
 *    Rev 1.7   17 Sep 1996 07:47:44   BUILD
 * -- enforce correct scrolling in case of DBCS and 'misadjusted' fonts
 *
 *    Rev 1.6   13 Sep 1996 17:58:32   BUILD
 * - enabled column header filtering for DBCS environemts
 *
 *    Rev 1.5   16 Apr 1996 20:20:24   BUILD
 * - use DosQueryCp to get codepage
 *
 *    Rev 1.4   28 Mar 1996 10:05:34   BUILD
 * - removed unused preprocessor directives
 *
 *    Rev 1.3   18 Mar 1996 16:40:04   BUILD
 * - fixed PTM KWT0251: When scrolling right in list windows, title gets assync
 *
 *    Rev 1.1   05 Feb 1996 13:44:22   BUILD
 * - some minor modification to enhance code readability
 *
 *    Rev 1.0   09 Jan 1996 09:16:28   BUILD
 * Initial revision.
*/
//+----------------------------------------------------------------------------+

#define INCL_EQF_PRINT                 // print functions
#include "eqf.h"                       // general EQF .h file
#include <time.h>                      // C time/date related functions
#include "eqfutclb.id"                 // CLB private dialog control IDs
#include "eqfutclb.h"                  // CLB private defines

static FARPROC lpfnOldListboxProc = NULL;// ptr to original listbox proc
LONG FAR PASCAL EqfListboxProc( register HWND hwnd, UINT msg,
                                register WPARAM mp1, LPARAM mp2);

// flag to mark sort items for descending sort
#define SORT_DESCENDING_FLAG  0x0800

#ifndef _TQM
extern HELPSUBTABLE hlpsubtblFolListChangeView[];

#endif

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     UtlRegisterCLB         Register the CLB window class     |
//+----------------------------------------------------------------------------+
//|Function call:     UtlRegisterCLB( HAB hab );                               |
//+----------------------------------------------------------------------------+
//|Description:       Register the column listbox window class.                |
//+----------------------------------------------------------------------------+
//|Input parameter:   HAB   hab     PM anchor block handle                     |
//+----------------------------------------------------------------------------+
//|Returncode type:   BOOL                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:       TRUE   column listbox class has been registered          |
//|                   FALSE  register call failed                              |
//+----------------------------------------------------------------------------+
//|Samples:           fOK = UtlRegisterCLB( hab );                             |
//+----------------------------------------------------------------------------+
//|Function flow:     call WinRegisterClass to register the CLB class          |
//+----------------------------------------------------------------------------+
BOOL UtlRegisterCLB
(
HAB hab                             // anchor block handle
)
{
  BOOL fResult;                       // function return value
  {
    WNDCLASS   wndclass;

    wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
//   wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC | CS_DBLCLKS;
    wndclass.lpfnWndProc = CLBWindowProc;
    wndclass.cbClsExtra = 0;
    wndclass.cbWndExtra = CLB_EXTRABYTES;
    wndclass.hInstance = (HINSTANCE) hab;
    wndclass.hIcon = NULL;
    wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
    wndclass.hbrBackground = (HBRUSH) GetStockObject (WHITE_BRUSH);
    wndclass.lpszMenuName = NULL;
    wndclass.lpszClassName = WC_EQF_CLBCLASS;

    fResult = (BOOL) RegisterClass(&wndclass);
  }

  /*******************************************************************/
  /* Register listbox super class (only in Windows environment       */
  /*******************************************************************/
  {
    WNDCLASS  wc;

    GetClassInfo( NULL, WC_LISTBOX, &wc );
    lpfnOldListboxProc = (FARPROC)wc.lpfnWndProc;// Save old proc
    wc.lpszClassName = LISTBOX_SUPER_CLASS;      // Change the class name
    wc.hInstance     = (HINSTANCE)UtlQueryULong( QL_HAB ); // Change the instance so that
    // the class gets unregistered
    // when the app terminates
    wc.lpfnWndProc   = EqfListboxProc;           // Our new proc

    // Register the puppy
    fResult = RegisterClass( &wc );
  }
  return( fResult );
} /* UtlRegisterCLB */

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     CLBWindowProc          Window procedure for CLBs         |
//+----------------------------------------------------------------------------+
//|Function call:     CLBWindowProc( HWND hwnd, USHORT msg, WPARAM mp1,        |
//|                                  LPARAM mp2 );                             |
//+----------------------------------------------------------------------------+
//|Description:       Procedure processing all message for the column listbox  |
//|                   control                                                  |
//+----------------------------------------------------------------------------+
//|Input parameter:   HWND   hwnd      handle of CLB                           |
//|                   USHORT msg       message identifier                      |
//|                   WPARAM mp1       first message parameter                 |
//|                   LPARAM mp2       second message parameter                |
//+----------------------------------------------------------------------------+
//|Returncode type:   MRESULT                                                  |
//+----------------------------------------------------------------------------+
//|Returncodes:       result of message processing                             |
//+----------------------------------------------------------------------------+
//|Function flow:     switch msg                                               |
//|                     case WM_CREATE:                                        |
//|                       call CLBCreate to process the message                |
//|                     case WM_SIZE:                                          |
//|                       set new position of list boc control                 |
//|                     case WM_COMMAND:                                       |
//|                     case WM_EQF_COMMAND:                                   |
//|                       switch control ID                                    |
//|                         case PID_VIEW_MI_NAMES:                            |
//|                           activate the names view list                     |
//|                         case PID_VIEW_MI_DETAILS:                          |
//|                           activate the details view list                   |
//|                         case PID_VIEW_MI_DETAILSDLG:                       |
//|                           load MAT resource module                         |
//|                           pop-up details dialog box                        |
//|                           if dialog box was ended normally                 |
//|                             activate details view list                     |
//|                           endif                                            |
//|                       endswitch                                            |
//|                     case WM_SETFOCUS:                                      |
//|                       if control gets the focus then                       |
//|                         set focus to listbox control using WM_EQF_SETFOCUS |
//|                       endif                                                |
//|                     case WM_EQF_SETFOCUS:                                  |
//|                       set focus to listbox control                         |
//|                     case WM_MEASUREITEM:                                   |
//|                       return width and height of list box elements         |
//|                     case WM_DRAWITEM:                                      |
//|                       if item state has changed then                       |
//|                         call CLBDrawItem to draw the listbox item          |
//|                       endif                                                |
//|                     case WM_CONTROL:                                       |
//|                       pass message to owner of CLB control                 |
//|                     case WM_PAINT:                                         |
//|                       set rectangle coordiantes to listbox title           |
//|                       call CLBDrawTitle to draw the column titles          |
//|                     case WM_CLOSE:                                         |
//|                       destroy the CLB                                      |
//|                     case WM_DESTROY:                                       |
//|                       free all resources used by the CLB control           |
//|                     case LM_EQF_SETVIEWLIST:                               |
//|                       copy the given view list to the specified list type  |
//|                        or if type is CURRENT_VIEW, activate the given view |
//|                        list                                                |
//|                     case LM_EQF_QUERYVIEWLIST:                             |
//|                       store view list of the requested type at the given   |
//|                        location                                            |
//|                     case LM_EQF_QUERYSORTLIST:                             |
//|                       store sort list at the given location                |
//|                     case LM_EQF_SETSORTLIST:                               |
//|                       store the give sort list in the sort list buffer     |
//|                     case LM_DELETEITEM:                                    |
//|                       pass message to list box                             |
//|                       decrement number of list box items                   |
//|                     case LM_DELETEALL:                                     |
//|                       pass message to list box                             |
//|                       set number of list box items to zero                 |
//|                     case LM_INSERTITEM:                                    |
//|                       pass message to list box                             |
//|                       increment number of list box items                   |
//|                     case LM_QUERYITEMCOUNT        :                        |
//|                     case LM_QUERYITEMHANDLE       :                        |
//|                     case LM_QUERYITEMTEXT         :                        |
//|                     case LM_QUERYITEMTEXTLENGTH   :                        |
//|                     case LM_QUERYSELECTION        :                        |
//|                     case LM_QUERYTOPINDEX         :                        |
//|                     case LM_SEARCHSTRING          :                        |
//|                     case LM_SELECTITEM            :                        |
//|                     case LM_SETTOPINDEX           :                        |
//|                     case LM_SETITEMHANDLE         :                        |
//|                     case LM_SETITEMHEIGHT         :                        |
//|                     case LM_SETITEMTEXT           :                        |
//|                       pass message to list box                             |
//|                     default                                                |
//|                       pass message to default window procedure             |
//|                   endswitch                                                |
//+----------------------------------------------------------------------------+
MRESULT EXPENTRY CLBWindowProc
(
HWND hwnd,
WINMSG msg,
WPARAM mp1,
LPARAM mp2
)
{
  PCLBIDA     pIda;                   // ptr to listbox IDA
  MRESULT     mResult = FALSE;        // result of message processing
  USHORT      usState;                // state of item (active/inactive)
  SHORT       sItem;                  // number of item
//   SHORT       sItemlength;            // itemlength
  INT_PTR	 iRc;                     // return code from DialogBox
  HMODULE hResMod;

  hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);

  switch ( msg )
  {
    // check for Delete key pressed (i.e. released)
    case WM_VKEYTOITEM:
      if ( LOWORD(mp1) == VK_DELETE )
      {
        /**********************************************************/
        /* inform owner that user wants to delete the requested   */
        /* item...                                                */
        /**********************************************************/
        WinPostMsg ( WinQueryWindow (hwnd, QW_PARENT, FALSE), WM_COMMAND,
                     MP1FROMSHORT (PID_FILE_MI_DELETE), 0L);
      } /* endif */
      mResult = -1;       //  Windows should handle everything
      break;

    case WM_CREATE:
      mResult = CLBCreate( hwnd,
                           (PCLBCTLDATA)((LPCREATESTRUCT)mp2)->lpCreateParams,
                           mp2 );
      break;

    case WM_SIZE:
      //--- set new size/position of our listbox control ---
      // Note: the listbox control fills the entire area occupied by
      //       the CLB control except a small area at the top of the
      //       CLB which is used to display the column titles
      pIda = (PCLBIDA) GETWINDOWPTR( hwnd, IDA_OFFSET );
      if ( pIda != NULL )
      {
        MoveWindow( pIda->hwndLB,
                    0,
                    LOWORD(pIda->lCharHeight) + 5,
                    LOWORD( mp2 ),
                    HIWORD( mp2 ) - (LOWORD(pIda->lCharHeight) + 5) ,
                    TRUE );
        /***********************************************************/
        /* force an update of the title text - we don't care about */
        /* mp1 and mp2 values...                                   */
        /***********************************************************/
        PostMessage( pIda->hwndLB, WM_HSCROLL, 0, 0L );
      } /* endif */
      break;

    case WM_COMMAND:
    case WM_EQF_COMMAND:
      pIda = (PCLBIDA) GETWINDOWPTR( hwnd, IDA_OFFSET );
      switch ( SHORT1FROMMP1(mp1) )
      {
        case PID_VIEW_MI_NAMES:
          CLBActivateViewList( pIda, pIda->pCtlData->psNameViewList,
                               NAME_VIEW );
          mResult = MRFROMSHORT( TRUE );
          break;

        case PID_VIEW_MI_DETAILS:
          CLBActivateViewList( pIda, pIda->pCtlData->psDetailsViewList,
                               NAME_VIEW );
          mResult = MRFROMSHORT( TRUE );
          break;

        case PID_VIEW_MI_DETAILSDLG:
          DIALOGBOX( hwnd, SelectViewDlg, hResMod, ID_SELVIEW_DLG,
                     pIda, iRc);
          if ( iRc )
          {
            CLBActivateViewList( pIda, pIda->pCtlData->psDetailsViewList,
                                 NAME_VIEW );
          } /* endif */
          mResult = MRFROMSHORT( TRUE );
          break;

        case PID_FILE_MI_PRINTLIST:
          CLBPrintList( pIda );
          break;

        default:
          /*******************************************************/
          /* pass all command messages to window owner ---       */
          /* you never know what he wants to do .....            */
          /*******************************************************/
          pIda = (PCLBIDA) GETWINDOWPTR( hwnd, IDA_OFFSET );
          if ( WMCOMMANDCMD(mp1,mp2) == LBN_DBLCLK)
          {
            PostMessage( pIda->hwndOwner, WM_BUTTON1DBLCLK, 0,0L);
          }
          else
          {
            // set correct control ID for message
            SHORT sID = (SHORT)GetWindowLong( hwnd, GWL_ID );
            mp1 = (WPARAM)sID;
            PostMessage( pIda->hwndOwner, msg, mp1, mp2 );
          }
          break;
      } /* endswitch */
      break;

    case WM_SETFOCUS:
      pIda = (PCLBIDA) GETWINDOWPTR( hwnd, IDA_OFFSET );
      WinPostMsg( hwnd, WM_EQF_SETFOCUS, 0L, MP2FROMHWND( pIda->hwndLB ) );
      break;

    case WM_SETREDRAW:
      pIda = (PCLBIDA) GETWINDOWPTR( hwnd, IDA_OFFSET );
      WinSendMsg( pIda->hwndLB, msg, mp1, mp2 );
      mResult = WinDefWindowProc( hwnd, msg, mp1, mp2 );
      break;

    case WM_EQF_SETFOCUS:
      WinSetFocus( HWND_DESKTOP, HWNDFROMMP2( mp2 ) );
      break;

    case WM_MEASUREITEM:
      pIda = (PCLBIDA) GETWINDOWPTR( hwnd, IDA_OFFSET );
      //--- return old width (if exists) otherwise return new width
      // Note: the LS_OWNERDRAW listbox queries of all new listbox items
      //       the heigth and width of the new item. Internally it keeps
      //       the maximum width over all items to set the range of the
      //       horizontal scrollbar correctly. If a listbox item is deleted
      //       the listbox queries the width of this item and, if this
      //       width is equal to the maximum width; queries again the
      //       width of all listbox items to set the new maximum width.
      //       This behaviour is used by the CLB to set a new width when
      //       the column view has been changed (this does not alter the
      //       actually contents of the listbox items). A dummy item is
      //       inserted and deleted again and the ulOldWidth field is set
      //       to the width which was in effect using the old column view.
      //       So the first WM_MEASUREITEM ( for the deleted dummy item)
      //       will return the old width ( which is of course the maximum
      //       width) and will therefore force the listbox to query the
      //       width of the listbox items again (which will when return
      //       the width of the new column view).
      {
        if ( pIda->ulOldWidth )
        {
          ((LPMEASUREITEMSTRUCT)mp2)->itemWidth  = pIda->ulOldWidth;
          pIda->ulOldWidth = 0;
        }
        else
        {
          ((LPMEASUREITEMSTRUCT)mp2)->itemWidth  = pIda->ulDisplayWidth;
        } /* endif */
        ((LPMEASUREITEMSTRUCT)mp2)->itemHeight = (USHORT)pIda->lCharHeight;
        mResult = TRUE;
        /***********************************************************/
        /* force update of scrollbar                               */
        /***********************************************************/
        SendMessage( hwnd, LB_SETHORIZONTALEXTENT, pIda->ulDisplayWidth, 0L);
        PostMessage( GetParent( hwnd ), WM_HSCROLL, 0, 0L );
      }
      break;

    case WM_DRAWITEM:                // draw a listbox item
      {
        /**********************************************************/
        /* draw listbox item ...                                  */
        /**********************************************************/
        LPDRAWITEMSTRUCT lpDisp = (LPDRAWITEMSTRUCT)mp2;

        if ( (lpDisp->itemID != -1) )
        {
          pIda = (PCLBIDA) GETWINDOWPTR( hwnd, IDA_OFFSET );
          if ( lpDisp->itemAction & (ODA_DRAWENTIRE | ODA_SELECT) )
          {
            CLBDrawItem( lpDisp->hDC,
                         &lpDisp->rcItem,
                         pIda,
                         (SHORT)lpDisp->itemID );

            if (  lpDisp->itemState & ODS_SELECTED  )
            {
              InvertRect( lpDisp->hDC, &lpDisp->rcItem );
            } /* endif */
          } /* endif */
        } /* endif */

        /**********************************************************/
        /* let windows do the dirty work of displaying the frame..*/
        /**********************************************************/
        mResult = WinDefWindowProc( hwnd, msg, mp1, mp2 );
      }
      break;


    case WM_CONTROL:
      //--- pass all control messages to window owner ---
      pIda = (PCLBIDA) GETWINDOWPTR( hwnd, IDA_OFFSET );
      mResult = WinSendMsg( pIda->hwndOwner, msg, mp1, mp2 );
      break;

    case WM_HSCROLL:
      {
        /*************************************************************/
        /* get the scroll position of the title text                 */
        /*************************************************************/
        RECT     rcListbox;
        int  iMin, iMax;
        ULONG  ulPos, ulWidth, ulMin, ulMax;
        ULONG   ulListBox;
        LONG    lCurrentXLeft;
        pIda = (PCLBIDA) GETWINDOWPTR( hwnd, IDA_OFFSET );
        ulPos = GetScrollPos( pIda->hwndLB, SB_HORZ );
        GetScrollRange( pIda->hwndLB, SB_HORZ, &iMin, &iMax );
        ulMax = iMax;
        ulMin = iMin;
        GetWindowRect( pIda->hwndLB, &rcListbox );
        /*************************************************************/
        /* get total width of text to be scrolled                    */
        /*************************************************************/
        ulWidth =  SendMessage( pIda->hwndLB,
                                        LB_GETHORIZONTALEXTENT, 0, 0L);
        ulListBox = rcListbox.right - rcListbox.left -
                    WinQuerySysValue(HWND_DESKTOP, SV_CXVSCROLL );
        if ( ulWidth <= ulListBox )
        {
          ulWidth = 0;
        } /* endif */

        lCurrentXLeft = -( (LONG)(ulPos - ulMin) * (LONG)ulWidth /
                           (LONG)(ulMax - ulMin) );

        if ( pIda->lCurrentXLeft != lCurrentXLeft )
        {
          InvalidateRect( pIda->hwndCLB, NULL, FALSE );
        } /* endif */
      }
      break;

    case WM_PAINT:
      //--- paint the column title area rectangle. This rectangle is the
      //    window rectangle minus the rectangle occupied by the listbox.
      //    The rectangle's xLeft value is set to the xLeft value of the
      //    listbox items to allow horizontal scrolling
      pIda = (PCLBIDA) GETWINDOWPTR( hwnd, IDA_OFFSET );
      {
        HDC      hdc;               // device context
        PAINTSTRUCT ps;             // pointer to paint struct
        RECT     rcListbox, rectl;
        LPPOINT  pPt;

        hdc = BeginPaint(hwnd, &ps );
        GetWindowRect( pIda->hwndLB, &rcListbox );
        pPt = (LPPOINT) &rcListbox;
        MapWindowPoints(HWND_DESKTOP, hwnd, pPt, 2 );

        rcListbox.right -= WinQuerySysValue(HWND_DESKTOP, SV_CXVSCROLL );
        SetRect( &rectl, 0, 0, rcListbox.right, rcListbox.top );

        rectl.left = (SHORT)pIda->lCurrentXLeft;
        CLBDrawTitle( hdc, &rectl, pIda );
        EndPaint(hwnd, &ps);
      }
      break;

    case WM_CLOSE:
      //--- destroy the window. Any termination processing will be done
      //    on receiving WM_DESTROY ---
      WinDestroyWindow( hwnd );
      break;

    case WM_DESTROY:
      //--- free all resource used by the CLB ---
      pIda = (PCLBIDA) GETWINDOWPTR( hwnd, IDA_OFFSET );
      if ( pIda )
      {
        // free memory allocated for listbox items
        SHORT sNoOfItems = QUERYITEMCOUNTHWND( pIda->hwndLB );
        SHORT sItem = 0;
        while ( sItem < sNoOfItems )
        {
          // get item text buffer and free it
          PSZ pszData = (PSZ)QUERYITEMHANDLEHWND( pIda->hwndLB, sItem );
          SETITEMHANDLEHWND( pIda->hwndLB, sItem, NULL );
          UtlAlloc( (PVOID *) &pszData, 0L, 0L, NOMSG );
          sItem++;
        } /* endwhile */
        UtlAlloc( (PVOID *)&pIda->psCurrentViewList, 0L, 0L, NOMSG );
        UtlAlloc( (PVOID *)&pIda->pCtlData->pColData, 0L, 0L, NOMSG );
        UtlAlloc( (PVOID *)&pIda->pCtlData, 0L, 0L, NOMSG );
        UtlAlloc( (PVOID *)&pIda->pDataPtr, 0L, 0L, NOMSG );
        UtlAlloc( (PVOID *)&pIda, 0L, 0L, NOMSG );
      } /* endif */
      break;

    case LM_EQF_SETVIEWLIST       :
      pIda = (PCLBIDA) GETWINDOWPTR( hwnd, IDA_OFFSET );
      switch ( SHORT1FROMMP1(mp1) )
      {
        case CURRENT_VIEW :
          CLBActivateViewList( pIda, (PSHORT)mp2, NAME_VIEW );
          break;
        case DETAILS_VIEW :
          CLBCopyViewList( pIda->pCtlData->psDetailsViewList,
                           (PSHORT)mp2,
                           pIda->pCtlData->usNoOfColumns );
          break;
        case DEFAULT_VIEW :
          CLBCopyViewList( pIda->pCtlData->psDefaultViewList,
                           (PSHORT)mp2,
                           pIda->pCtlData->usNoOfColumns );
          break;
        case NAME_VIEW    :
          CLBCopyViewList( pIda->pCtlData->psNameViewList,
                           (PSHORT)mp2,
                           pIda->pCtlData->usNoOfColumns );
          break;
      } /* endswitch */
      break;

    case LM_EQF_QUERYVIEWLIST       :
      pIda = (PCLBIDA) GETWINDOWPTR( hwnd, IDA_OFFSET );
      switch ( SHORT1FROMMP1(mp1) )
      {
        case CURRENT_VIEW :
          CLBCopyViewList( (PSHORT)mp2,
                           pIda->psCurrentViewList,
                           pIda->pCtlData->usNoOfColumns );
          break;
        case DETAILS_VIEW :
          CLBCopyViewList( (PSHORT)mp2,
                           pIda->pCtlData->psDetailsViewList,
                           pIda->pCtlData->usNoOfColumns );
          break;
        case DEFAULT_VIEW :
          CLBCopyViewList( (PSHORT)mp2,
                           pIda->pCtlData->psDefaultViewList,
                           pIda->pCtlData->usNoOfColumns );
          break;
        case NAME_VIEW    :
          CLBCopyViewList( (PSHORT)mp2,
                           pIda->pCtlData->psNameViewList,
                           pIda->pCtlData->usNoOfColumns );
          break;
        case SORT_VIEW    :
          CLBCopyViewList( (PSHORT)mp2,
                           pIda->pCtlData->psSortList,
                           pIda->pCtlData->usNoOfColumns );
          break;
      } /* endswitch */
      break;

    case LM_EQF_QUERYSORTLIST       :
      pIda = (PCLBIDA) GETWINDOWPTR( hwnd, IDA_OFFSET );
      CLBCopyViewList( (PSHORT)mp2,
                       pIda->pCtlData->psSortList,
                       pIda->pCtlData->usNoOfColumns );
      break;

    case LM_EQF_SETSORTLIST       :
      pIda = (PCLBIDA) GETWINDOWPTR( hwnd, IDA_OFFSET );
      CLBCopyViewList( pIda->pCtlData->psSortList,
                       (PSHORT)mp2,
                       pIda->pCtlData->usNoOfColumns );
      break;

    case LM_EQF_QUERYFILTER       :
      pIda = (PCLBIDA) GETWINDOWPTR( hwnd, IDA_OFFSET );
      memcpy( (PVOID)mp2, pIda->pCtlData->pFilter,
              sizeof(CLBFILTER) );
      break;

    case LM_EQF_SETFILTER       :
      pIda = (PCLBIDA) GETWINDOWPTR( hwnd, IDA_OFFSET );
      memcpy( pIda->pCtlData->pFilter, (PVOID)mp2,
              sizeof(CLBFILTER) );
      break;

    case  LM_EQF_SETITEMSTATE    :
      pIda = (PCLBIDA) GETWINDOWPTR( hwnd, IDA_OFFSET );
      sItem   = SHORT1FROMMP1(mp1);
      usState = SHORT1FROMMP2(mp2);
      /*************************************************************/
      /* passed values:                                            */
      /*   0   disable                                             */
      /*   1   enable                                              */
      /* but since a NULL handle is the default, we have to swap   */
      /* the values....                                            */
      /*************************************************************/
      // we use the first 4 bytes of the item buffer as handle
      {
        LONG lHandle = QUERYITEMHANDLEHWND( pIda->hwndLB, mp1 );
        PSZ pszData = (PSZ)lHandle;
        if ( lHandle != LIT_ERROR )
        {
          if ( pszData == NULL )    // no item buffer yet?
          {
            // ... allocate a new one
            UtlAlloc( (PVOID *) &pszData, 0L, 20, ERROR_STORAGE );
            SETITEMHANDLEHWND( pIda->hwndLB, mp1, pszData );
          } /* endif */

          if ( pszData != NULL )
          {
            *((PULONG)pszData) = !usState;
          } /* endif */
        } /* endif */
      }
      {
        RECT rcItem;
        memset(&rcItem, 0, sizeof(RECT));
        SendMessage( pIda->hwndLB, LB_GETITEMRECT, MP1FROMSHORT(sItem),
                     MP2FROMP(&rcItem) );
        InvalidateRect( pIda->hwndLB, &rcItem, FALSE );
      }
      break;

    case  LM_EQF_INSERTITEMSTATE    :
      /*************************************************************/
      /* highest bit of the short in mp1 contains state - either   */
      /* enabled or disabled                                       */
      /*************************************************************/
      pIda = (PCLBIDA) GETWINDOWPTR( hwnd, IDA_OFFSET );
      sItem   = SHORT1FROMMP1(mp1) & ~CLB_INSERTITEMSTATE_ENABLED;
      usState = SHORT1FROMMP1(mp1) & CLB_INSERTITEMSTATE_ENABLED;
      sItem = sItem;
      /*************************************************************/
      /* passed values:                                            */
      /*   0   disable                                             */
      /*   1   enable                                              */
      /* but since a NULL handle is the default, we have to swap   */
      /* the values....                                            */
      /*************************************************************/
      {
        PSZ pszData;

        INSERTITEMHWND( pIda->hwndLB, EMPTY_STRING );

        UtlAlloc( (PVOID *) &pszData, 0L, strlen((PSZ)mp2) + 5, ERROR_STORAGE );
        if ( pszData != NULL )
        {
          strcpy( pszData + 4, (PSZ)mp2 );
          *((PULONG)pszData) = !usState;
          SETITEMHANDLEHWND( pIda->hwndLB, mp1, MP2FROMP(pszData));
          Utlstrccpy( pIda->szBuffer, (PSZ)mp2, X15 );
          SETITEMTEXTHWND( pIda->hwndLB, sItem, pIda->szBuffer );
        } /* endif */
      }
      break;

    case  LM_EQF_QUERYITEMSTATE    :
      /*************************************************************/
      /* return value:                                             */
      /*   0  enabled                                              */
      /*   1  disabled                                             */
      /* but callee wants it the other way round .....             */
      /*************************************************************/
      pIda = (PCLBIDA) GETWINDOWPTR( hwnd, IDA_OFFSET );
      {
        LONG lHandle = QUERYITEMHANDLEHWND( pIda->hwndLB, mp1 );
        PULONG pulData = (PULONG)lHandle;
        if ( (lHandle != LIT_ERROR) && (pulData != NULL) )
        {
          usState = (USHORT)*pulData;
        }
        else
        {
          usState = 0;
        } /* endif */
      }
      mResult = ( MRESULT ) ! usState;
      break;


    case  LM_EQF_REFRESH :
      //--- force listbox to compute a new item width ---
      pIda = (PCLBIDA) GETWINDOWPTR( hwnd, IDA_OFFSET );

      // Note: the listbox control keeps an internal variable
      //       with the maximum width of the listbox items.
      //       If a listbox item is deleted and the width of this item is
      //       the stored maximum item width,  the listbox control reacts
      //       in sending WM_MEASUREITEM messages for all remaining items
      //       to compute the new maximum item width value.
      pIda->ulOldWidth = pIda->ulDisplayWidth;
      pIda->ulDisplayWidth = CLBQueryItemWidth( pIda );
      SendMessage( pIda->hwndLB, LB_SETHORIZONTALEXTENT,
                   pIda->ulDisplayWidth, 0L);
      SendMessage( pIda->hwndLB, WM_HSCROLL, SB_TOP, 0L );
      //--- activate new column titles ---
      INVALIDATERECT( pIda->hwndCLB, NULL, FALSE );
      break;

    case LM_DELETEITEM            :
      pIda = (PCLBIDA) GETWINDOWPTR( hwnd, IDA_OFFSET );
      // free any allocate storage associated with the item
      {
        LONG lHandle = QUERYITEMHANDLEHWND( pIda->hwndLB, mp1 );
        PSZ pszData = (PSZ)lHandle;
        if ( lHandle != LIT_ERROR )
        {
          SETITEMHANDLEHWND( pIda->hwndLB, mp1, NULL );
          UtlAlloc( (PVOID *) &pszData, 0L, 0L, NOMSG );
        } /* endif */
      }
      mResult = DELETEITEMHWND( pIda->hwndLB, mp1 );
      pIda->usNoOfItems = SHORT1FROMMR(mResult);
      break;

    case LM_DELETEALL             :
      pIda = (PCLBIDA) GETWINDOWPTR( hwnd, IDA_OFFSET );
      if ( pIda != NULL )
      {
        // free memory allocated for listbox items
        SHORT sNoOfItems = QUERYITEMCOUNTHWND( pIda->hwndLB );
        SHORT sItem = 0;
        while ( sItem < sNoOfItems )
        {
          // get item text buffer and free it
          PSZ pszData = (PSZ)QUERYITEMHANDLEHWND( pIda->hwndLB, sItem );
          SETITEMHANDLEHWND( pIda->hwndLB, sItem, NULL );
          UtlAlloc( (PVOID *) &pszData, 0L, 0L, NOMSG );
          sItem++;
        } /* endwhile */
      } /* endif */
      mResult = DELETEALLHWND( pIda->hwndLB );
      pIda->usNoOfItems = 0;
      break;

    case LM_INSERTITEM            :
      pIda = (PCLBIDA) GETWINDOWPTR( hwnd, IDA_OFFSET );
      {
        PSZ pszData;

        UtlAlloc( (PVOID *) &pszData, 0L, strlen((PSZ)mp2) + 5, ERROR_STORAGE );
        if ( pszData != NULL )
        {
          Utlstrccpy( pIda->szBuffer, (PSZ)mp2, X15 );
          strcat( pIda->szBuffer, " " ); // to allow a refresh later on ...
           mResult = SendMessage( pIda->hwndLB, LB_ADDSTRING, mp1,
                                 MP2FROMP(pIda->szBuffer) );
          if ( SHORT1FROMMR(mResult) >= 0 )
          {
            strcpy( pszData + 4, (PSZ)mp2 );
            SETITEMHANDLEHWND( pIda->hwndLB, SHORT1FROMMR(mResult),
                               MP2FROMP(pszData));

            // force a refresh of the listbox text
            Utlstrccpy( pIda->szBuffer, (PSZ)mp2, X15 );
            SETITEMTEXTHWND( pIda->hwndLB,
                             SHORT1FROMMR(mResult),
                             (PSZ)MP2FROMP(pIda->szBuffer) );
          }
          else
          {
            UtlAlloc( (PVOID *) &pszData, 0L, 0L, NOMSG );
          } /* endif */
        } /* endif */
      }
      break;

    case LB_INSERTSTRING:
      pIda = (PCLBIDA) GETWINDOWPTR( hwnd, IDA_OFFSET );
      {
        PSZ pszData;

        UtlAlloc( (PVOID *) &pszData, 0L, strlen((PSZ)mp2) + 5, ERROR_STORAGE );
        if ( pszData != NULL )
        {
          Utlstrccpy( pIda->szBuffer, (PSZ)mp2, X15 );
          mResult = SendMessage( pIda->hwndLB, LB_INSERTSTRING, mp1,
                                 MP2FROMP(pIda->szBuffer) );
          if ( SHORT1FROMMR(mResult) >= 0 )
          {
            strcpy( pszData + 4, (PSZ)mp2 );
            SETITEMHANDLEHWND( pIda->hwndLB, SHORT1FROMMR(mResult),
                               MP2FROMP(pszData));
          }
          else
          {
            UtlAlloc( (PVOID *) &pszData, 0L, 0L, NOMSG );
          } /* endif */
        } /* endif */
      }
      break;

    case LM_QUERYITEMHANDLE       :
      pIda = (PCLBIDA) GETWINDOWPTR( hwnd, IDA_OFFSET );
      {
        LONG lHandle = QUERYITEMHANDLEHWND( pIda->hwndLB, mp1 );
        PSZ pszData = (PSZ)lHandle;
        if ( lHandle != LIT_ERROR )
        {
          ULONG  ulState = 0L;

          if ( pszData != NULL )
          {
            ulState = *((PULONG)pszData);
          } /* endif */
          mResult = ulState;
        } /* endif */
      }
      break;

    case LM_QUERYITEMTEXT         :
      pIda = (PCLBIDA) GETWINDOWPTR( hwnd, IDA_OFFSET );
      {
        LONG  lLen = LIT_ERROR;
        LONG lHandle = QUERYITEMHANDLEHWND( pIda->hwndLB, mp1 );
        PSZ pszData = (PSZ)lHandle;
        if ( lHandle != LIT_ERROR )
        {
          if ( pszData != NULL )
          {
            strcpy( (PSZ)mp2, pszData + 4 );
            lLen = strlen( (pszData + 4) );
          } /* endif */
          mResult = lLen;
        } /* endif */
      }
      break;

    case LM_QUERYITEMTEXTLENGTH   :
      pIda = (PCLBIDA) GETWINDOWPTR( hwnd, IDA_OFFSET );
      {
        LONG lHandle = QUERYITEMHANDLEHWND( pIda->hwndLB, mp1 );
        PSZ pszData = (PSZ)lHandle;
        ULONG ulSize = 0L;
        if ( lHandle != LIT_ERROR )
        {
          if ( pszData != NULL )
          {
            ulSize = strlen( pszData + 4 ) + 1;
          } /* endif */
        } /* endif */
        mResult = ulSize;
      }
      break;

    case LM_SETITEMHANDLE         :
      pIda = (PCLBIDA) GETWINDOWPTR( hwnd, IDA_OFFSET );
      {
        LONG lHandle = QUERYITEMHANDLEHWND( pIda->hwndLB, mp1 );
        PSZ pszData = (PSZ)lHandle;
        if ( lHandle != LIT_ERROR )
        {
          ULONG ulState = 0L;
          if ( pszData != NULL )
          {
            ulState = *((PULONG)pszData);
            *((PULONG)pszData) = (ULONG)mp2;
          } /* endif */
          mResult = ulState;
        } /* endif */
      }
      break;

    case LM_QUERYITEMCOUNT        :
    case LM_QUERYSELECTION        :
    case LM_QUERYTOPINDEX         :
    case LM_SEARCHSTRING          :
    case LM_SELECTITEM            :
    case LM_SETTOPINDEX           :
    case LM_SETITEMHEIGHT         :

      /***************************************************************/
      /* the following three messages are for multiple selection LBs */
      /***************************************************************/
    case LB_GETSELCOUNT          :
    case LB_GETSELITEMS          :
    case LB_SETSEL               :
    case LB_FINDSTRINGEXACT      :
      //--- pass all list messages to listbox control ---
      pIda = (PCLBIDA) GETWINDOWPTR( hwnd, IDA_OFFSET );
      mResult = WinSendMsg( pIda->hwndLB, msg, mp1, mp2 );
      break;

    default:
      mResult = WinDefWindowProc( hwnd, msg, mp1, mp2 );
  }
  return( mResult );
}

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     CLBCreate                                                |
//+----------------------------------------------------------------------------+
//|Function call:     CLBCreate( HWND, hwnd,        mp1, LPARAM mp2 )          |
//+----------------------------------------------------------------------------+
//|Description:       Process the WM_CREATE message for a column list box      |
//|                   control: allocate required memory blocks and create the  |
//|                   list box control.                                        |
//+----------------------------------------------------------------------------+
//|Input parameter:   HWND      hwnd       handle of CLB window                |
//|                   WPARAM    mp1        first message parameter: ptr to     |
//|                                        CLB control area                    |
//|                   LPARAM    mp2        second message parameter: ptr to    |
//|                                        CREATESTRUCT structure              |
//+----------------------------------------------------------------------------+
//|Returncode type:   MRESULT                                                  |
//+----------------------------------------------------------------------------+
//|Returncodes:       TRUE     errors occured, CLB can't be created            |
//|                   FALSE    CLB creation was successful                     |
//+----------------------------------------------------------------------------+
//|Function flow:     do consistency check of supplied data in CLB control area|
//|                   if ok get memory for IDA and work buffers                |
//|                   if ok anchor IDA                                         |
//|                   if ok get copy of supplied control area                  |
//|                   if ok resolve pointers in CLB control area               |
//|                   if ok store font characteristics in IDA                  |
//|                   if ok call CLBQueryItemWidth to set initial item width   |
//|                   if ok create inner list box control                      |
//|                   return inverted ok flag                                  |
//+----------------------------------------------------------------------------+
MRESULT CLBCreate( HWND hwnd, PCLBCTLDATA pCtlData, LPARAM mp2 )
{
  PCLBIDA       pIda = NULL;          // ptr to listbox IDA
  LPCREATESTRUCT pCreateData;
  BOOL          fOK = TRUE;           // internal OK flag
  USHORT        usI;                  // general loop index
  PSHORT        psTemp;               // temporary pointer for memory alloc

  pCreateData = (LPCREATESTRUCT) mp2; // address general create window data

  //--- check consistency of CLB control area ---
  if ( (pCtlData == NULL)                   ||  // if no control area or
       ((SHORT)pCtlData->usNoOfColumns < 0) ||  // to few columns or
       (pCtlData->usNoOfColumns > MAX_DEFINEDCOLUMNS) ||  // to many columns or
       (pCtlData->pColData == NULL)         ||  // no column def table or
       (pCtlData->psLastUsedViewList == NULL) ||// no lastused view list or
       (pCtlData->psDefaultViewList == NULL) || // no default view list or
       (pCtlData->psDetailsViewList == NULL) || // no details view listor
       (pCtlData->psNameViewList == NULL)   ||  // no name view list or
//        (pCtlData->pFilter  == NULL)         ||  // no filter area or -- don't care -- not set in case of TagTable
       (pCtlData->psSortList == NULL) )         // no sort criteria list ...
  {
    fOK = FALSE;                               // ... cannot create a CLB!
  } /* endif */

  //--- get memory for listbox IDA and related areas ---
  if ( fOK )
  {
    //--- memory for CLB IDA ---
    fOK = UtlAlloc( (PVOID *)&pIda, 0L, (LONG) sizeof( CLBIDA ), ERROR_STORAGE );
  } /* endif */
  if ( fOK )
  {
    //--- memory for column string pointers ---
    fOK = UtlAlloc( (PVOID *)&pIda->pDataPtr, 0L,
                    (LONG) ( MAX_DEFINEDCOLUMNS * sizeof(PSZ) ),
                    ERROR_STORAGE );
  } /* endif */
  if ( fOK )
  {
    WINSETWINDOWPTR( hwnd, IDA_OFFSET, pIda );
    pIda->hwndOwner = GETPARENT( hwnd );
    pIda->hwndCLB   = hwnd;
    /*******************************************************************/
    /* if okay get fixed fonts...                                      */
    /*******************************************************************/
    SetFixedFont (hwnd, pIda);
  } /* endif */


  //--- get copy of column listbox control data area
  if ( fOK )
  {
    fOK = UtlAlloc( (PVOID *)&pIda->pCtlData, 0L, (LONG) sizeof( CLBCTLDATA ),
                    ERROR_STORAGE );
    if ( fOK )
    {
      memcpy( pIda->pCtlData, pCtlData, sizeof( CLBCTLDATA ) );
    } /* endif */
  } /* endif */

  //--- resolve pointers in column listbox control data area
  if ( fOK )
  {
    //--- get memory for column definition table ---
    fOK = UtlAlloc( (PVOID *)&pIda->pCtlData->pColData, 0L,
                    (LONG) MAX_DEFINEDCOLUMNS * sizeof( CLBCOLDATA ),
                    ERROR_STORAGE );
  } /* endif */
  if ( fOK )
  {
    //--- get one area for all CLB view lists, set view list pointers and
    //    fill in the view lists (there are 6 different view lists) ---
    //                                         |
    fOK = UtlAlloc( (PVOID *)&psTemp, 0L,  //  
                    (LONG) (((MAX_VIEW + 1) * 6 * sizeof( SHORT )) + sizeof(CLBFILTER)),
                    ERROR_STORAGE );
    if ( fOK )
    {
      pIda->psCurrentViewList = psTemp;
      psTemp += MAX_VIEW + 1;
      pIda->pCtlData->psLastUsedViewList = psTemp;
      psTemp += MAX_VIEW + 1;
      pIda->pCtlData->psDefaultViewList = psTemp;
      psTemp += MAX_VIEW + 1;
      pIda->pCtlData->psDetailsViewList = psTemp;
      psTemp += MAX_VIEW + 1;
      pIda->pCtlData->psNameViewList = psTemp;
      psTemp += MAX_VIEW + 1;
      pIda->pCtlData->psSortList = psTemp;
      psTemp += MAX_VIEW + 1;
      pIda->pCtlData->pFilter = (PCLBFILTER)psTemp;
    } /* endif */
    if ( fOK )
    {
      //--- copy column definition data and title strings ---
      memcpy( pIda->pCtlData->pColData,
              pCtlData->pColData,
              pCtlData->usNoOfColumns * sizeof( CLBCOLDATA ) );
      for ( usI = 0; usI < pIda->pCtlData->usNoOfColumns; usI++ )
      {
        (pIda->pCtlData->pColData + usI)->pszTitle =
        strdup( (pIda->pCtlData->pColData + usI)->pszTitle );
      } /* endfor */

      //--- copy view lists ---
      CLBCopyViewList( pIda->psCurrentViewList,
                       pCtlData->psLastUsedViewList,
                       pIda->pCtlData->usNoOfColumns );
      CLBCopyViewList( pIda->pCtlData->psDefaultViewList,
                       pCtlData->psDefaultViewList,
                       pIda->pCtlData->usNoOfColumns );
      CLBCopyViewList( pIda->pCtlData->psDetailsViewList,
                       pCtlData->psDetailsViewList,
                       pIda->pCtlData->usNoOfColumns );
      CLBCopyViewList( pIda->pCtlData->psNameViewList,
                       pCtlData->psNameViewList,
                       pIda->pCtlData->usNoOfColumns );
      CLBCopyViewList( pIda->pCtlData->psSortList,
                       pCtlData->psSortList,
                       pIda->pCtlData->usNoOfColumns );
      if ( pCtlData->pFilter )
      {
        memcpy( pIda->pCtlData->pFilter, pCtlData->pFilter,
                sizeof(CLBFILTER) );
      }
    } /* endif */
  } /* endif */


  //--- set initial display width ---
  if ( fOK )
  {
    pIda->ulDisplayWidth = CLBQueryItemWidth( pIda );
  } /* endif */

  if ( fOK )
  {
    pIda->hwndLB =
    CreateWindow ( LISTBOX_SUPER_CLASS,
                   "",    //window caption
                   pCreateData->style  | LBS_STANDARD |
                   LBS_OWNERDRAWFIXED | LBS_NOTIFY |
                   WS_VSCROLL | WS_HSCROLL | LBS_DISABLENOSCROLL |
                   LBS_HASSTRINGS |      LBS_WANTKEYBOARDINPUT  |
                   LBS_NOINTEGRALHEIGHT,
                   pCreateData->x,
                   pCreateData->y,
                   pCreateData->cx,
                   pCreateData->cy,
                   hwnd,              // parent wnd handle
                   (HMENU) NULL,
                   (HINSTANCE) UtlQueryULong( QL_HAB ),
                   NULL );
    /*************************************************************/
    /* ensure, that the listbox horizontal scrollbar is set      */
    /* correct...                                                */
    /*************************************************************/
    SendMessage( pIda->hwndLB, LB_SETHORIZONTALEXTENT,
                 pIda->ulDisplayWidth, 0L);
    fOK = ( pIda->hwndLB != NULLHANDLE );
  } /* endif */

  return( MRFROMSHORT( !fOK ) );      // create window only if everything is ok
} /* endof CLBCreate */


//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     CLBDrawTitle           Draw CLB column titles            |
//+----------------------------------------------------------------------------+
//|Function call:     CLBDrawTitle( HPS hps, PRECTL prcl, PCLBIDA pIda );      |
//+----------------------------------------------------------------------------+
//|Description:       Draw the column list box column titles into the          |
//|                   supplied presentation space using the passed rectangle   |
//+----------------------------------------------------------------------------+
//|Input parameter:   HPS      hps    presentation space handle                |
//|                   PRECTL   prcl   pointer to bounding rectangle            |
//|                   PCLBIDA  pIda   pointer to column list box IDA           |
//+----------------------------------------------------------------------------+
//|Returncode type:   VOID                                                     |
//+----------------------------------------------------------------------------+
//|Function flow:     get copy of bounding rectangle                           |
//|                   erase rectangle                                          |
//|                   while not end of active view list                        |
//|                     address column data for column                         |
//|                     truncate rectangle to column width of current column   |
//|                     cut off any comments from column title                 |
//|                     draw the column title using WinDrawText                |
//|                     update the left side of the bounding rectangle         |
//|                     go to next column in view list                         |
//|                   endwhile                                                 |
//+----------------------------------------------------------------------------+
VOID CLBDrawTitle( HDC hdc, PRECT prcl, PCLBIDA pIda )
{
  LONG        lLeft, lMaxRight;
  PCLBCTLDATA pCLB;
  PCLBCOLDATA pCol;
  PSHORT      psViewColumn;  // current column in view column list
  RECT        rcl;
  PSZ         pszSource, pszTarget;
  HFONT       hFont, hOldFont;
  PPROPSYSTEM  pPropSys = GetSystemPropPtr();

  memcpy( &rcl, prcl, sizeof(RECT) );// get copy of rectangle structure
  rcl.top += 2;                      // leave two pels space at top...
  pCLB         = pIda->pCtlData;
  pCol         = pCLB->pColData;
  if ( rcl.left )
  {
    rcl.left-- ;                     //paint one step more
  } /* endif */

  FILLRECT( hdc, rcl, pCLB->lItemBackColor );

  rcl.left     = pIda->lCurrentXLeft;
  lMaxRight   = rcl.right;
  lLeft       = rcl.left + ( LOWORD(pIda->lAveCharWidth *
                                    CLB_WIDTH_FACTOR * pCLB->usColDistance));
  psViewColumn = pIda->psCurrentViewList;


  hFont = CreateFontIndirect( &pIda->lf );
  hOldFont = (HFONT) SelectObject( hdc, hFont );

  while ( *psViewColumn != CLBLISTEND )
  {
    pCol = pCLB->pColData + *psViewColumn;
    rcl.left  = lLeft;
    rcl.right = rcl.left +  (SHORT)(pIda->lAveCharWidth * CLB_WIDTH_FACTOR *
                                    pCol->usWidth);
    rcl.right = min( rcl.right, lMaxRight);
    if ( rcl.right > rcl.left)
    {
      /*************************************************************/
      /* Filter column title                                       */
      /*************************************************************/
      pszSource = pCol->pszTitle;
      pszTarget = pIda->szBuffer;
      while ( *pszSource &&
              ((pszTarget - pIda->szBuffer) < (sizeof(pIda->szBuffer) - 1)))
      {
        if ( *pszSource == CLB_START_COMMENT )
        {
          while ( *pszSource && (*pszSource != CLB_END_COMMENT ) )
          {
            pszSource++;
          } /* endwhile */
          if ( *pszSource == CLB_END_COMMENT )
          {
            pszSource++;
          } /* endif */
        }
        else
        {
          UCHAR c = *pszSource;
          *pszTarget++ = *pszSource++;
          /********************************************************/
          /* enable for DBCS text ...                             */
          /********************************************************/
          if (IsDBCS_CP(pPropSys->ulSystemPrefCP)  &&
             (isdbcs1ex((USHORT)pPropSys->ulSystemPrefCP, c) == DBCS_1ST ))
          {
            *pszTarget++ = *pszSource++;
          } /* endif */
        } /* endif */
      } /* endwhile */
      *pszTarget = EOS;

      // check width of data in column for AUTOWIDTHTEXT_DATA columns
      if ( pCol->DataType == AUTOWIDTHTEXT_DATA )
      {
        LONG  lColWidth;               // actual width of column data
        LONG  lOldWidth;               // current width of column
        LONG  lColHeight;

        // get actual width of column data
        TEXTSIZE( hdc, pIda->szBuffer, lColWidth, lColHeight );

        // enlarge column width if it is too small
        lOldWidth = (pIda->lAveCharWidth * (LONG)CLB_WIDTH_FACTOR
                     * (LONG) pCol->usWidth);
        if ( lOldWidth < lColWidth )
        {
          // adjust column width
          pCol->usWidth = (USHORT)(((lColWidth * 100L /
                                     ((LONG)CLB_WIDTH_FACTOR * pIda->lAveCharWidth)) + 99L)
                                   / 100L);

          // adjust current draw rectangle
          rcl.left  = lLeft;
          rcl.right = rcl.left + (LOWORD(pIda->lAveCharWidth * CLB_WIDTH_FACTOR
                                         * pCol->usWidth));
          rcl.right = min( rcl.right, lMaxRight);

          // force a repaint of column listbox
          WinPostMsg( pIda->hwndCLB, LM_EQF_REFRESH, 0L, 0L );
        } /* endif */
      } /* endif */
      /*************************************************************/
      /* Draw Column Title                                         */
      /*************************************************************/
      DRAWTEXT( hdc, pIda->szBuffer, rcl,
                pCLB->lTitleColor,
                pCLB->lTitleBackColor,
                pCol->usFormat );
    } /* endif */
    lLeft += LOWORD(pIda->lAveCharWidth * CLB_WIDTH_FACTOR *
                    (pCol->usWidth + pCLB->usColDistance)) ;
    psViewColumn++;
  } /* endwhile */
  SelectObject( hdc, hOldFont );
  DeleteObject( hFont );

} /* end of CLBDrawTitle */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     CLBDrawItem           Draw a CLB item                    |
//+----------------------------------------------------------------------------+
//|Function call:     CLBDrawItem( HPS hps, PRECTL prcl, PCLBIDA pIda,         |
//|                                SHORT sItem );                              |
//+----------------------------------------------------------------------------+
//|Description:       Draw the column list box item into the                   |
//|                   supplied presentation space using the passed rectangle   |
//+----------------------------------------------------------------------------+
//|Input parameter:   HPS      hps    presentation space handle                |
//|                   PRECTL   prcl   pointer to bounding rectangle            |
//|                   PCLBIDA  pIda   pointer to column list box IDA           |
//|                   SHORT    sItem  index of item in list box                |
//+----------------------------------------------------------------------------+
//|Returncode type:   VOID                                                     |
//+----------------------------------------------------------------------------+
//|Function flow:     if left hand side of rectangle has been changed then     |
//|                     invalidate title text region to force a repaint        |
//|                   endif                                                    |
//|                   get item text                                            |
//|                   convert column text strings to zero terminated strings   |
//|                   erase rectangle                                          |
//|                   while not end of current view list                       |
//|                     adjust right hand border of rectangle                  |
//|                     switch data type of column                             |
//|                       case DATA_DATA:                                      |
//|                         convert column text to LONG                        |
//|                         call UtlLongToDateString to convert to string      |
//|                       case TIME_DATA:                                      |
//|                         convert column text to LONG                        |
//|                         call UtlLongToTimeString to convert to string      |
//|                       case DATATIME_DATA:                                  |
//|                         convert column text to LONG                        |
//|                         call UtlLongToDataString to convert to string      |
//|                         add result of UtlLongToTimeString to string        |
//|                       case FDATE_DATA:                                     |
//|                         convert column text to FDATE                       |
//|                         call UtlFDateToDateString to convert to string     |
//|                       case FTIME_DATA:                                     |
//|                         convert column text to FTIME                       |
//|                         call UtlFTimeToTimeString to convert to string     |
//|                       case FDATETIME_DATA:                                 |
//|                         convert column text to FDATE and FTIME             |
//|                         call UtlFDateToDataString to convert to string     |
//|                         add result of UtlFTimeToTimeString to string       |
//|                       case AUTOWIDTHTEXT_DATA:                             |
//|                       case TEXT_DATA:                                      |
//|                         use column text as-is                              |
//|                     endswitch                                              |
//|                     if not outside bounding rectangle then                 |
//|                       draw test using WinDrawText                          |
//|                     endif                                                  |
//|                     set new left hand side of rectangle                    |
//|                     go to next column in view list                         |
//|                   endwhile                                                 |
//+----------------------------------------------------------------------------+
VOID CLBDrawItem( HDC hps, PRECT prcl, PCLBIDA pIda, SHORT sItem )
{
  register USHORT      usI;
  LONG        lxLeft, lxMaxRight;
  PCLBCTLDATA pCLB;
  PCLBCOLDATA pCol;
  PSZ         pEndOfColumn;  // ptr to end of column data
  PSZ         pColumnStart;  // ptr to start of column data
  PSZ         *pColData;     // current position in data ptr table
  PSZ         pszTemp;       // general work pointer
  PSHORT      psViewColumn;  // current column in view column list
  PSZ         pszColText;    // ptr to text for column
  LONG        lDate;         // long date of DATE_DATA values
  CHAR        chDate[50];             // date converted to a character string
  RECT        rcl;                  // rectangle
  HFONT       hFont, hOldFont;      // created font
  FDATE       FDate;                  // date in FDATE format
  FTIME       FTime;                  // time in FTIME format
  ULONG       ulLen;                  // buffer for string lengths
  USHORT      usDrawstate ;
  BOOL        fDisabled;

  pCLB         = pIda->pCtlData;
  memcpy( &rcl, prcl, sizeof(rcl) );        // get copy of rectangle structure


  hFont = CreateFontIndirect( &pIda->lf );
  hOldFont = (HFONT) SelectObject( hps, hFont );

  //--- paint title if first visible position has changed ---
  {
    POINT pt;
    GetWindowOrgEx( hps, &pt   );
    if ( pIda->lCurrentXLeft != -pt.x )
    {
      /***********************************************************/
      /* force an update of the title text                       */
      /***********************************************************/
      pIda->lCurrentXLeft = -pt.x;
      INVALIDATEREGION( pIda->hwndCLB, NULLHANDLE, FALSE );
    } /* endif */
  }

  //--- get text of item ---
  {
    PSZ pszData = (PSZ)QUERYITEMHANDLEHWND( pIda->hwndLB, sItem );
    if ( pszData != NULL )
    {
      strcpy( pIda->szWork, pszData + 4 );
      fDisabled = (BOOL)*((PULONG)pszData);
    }
    else
    {
      pIda->szWork[0] = EOS;
      fDisabled = FALSE;
    } /* endif */
  }
  pColData = pIda->pDataPtr;          // start with first string ptr
  pColumnStart = pIda->szWork;        // set column data start pointer
  for ( usI = 0; usI < pCLB->usNoOfColumns; usI++ )
  {
    pEndOfColumn = strchr( pColumnStart, pCLB->chDataSeperator );
    if ( pEndOfColumn )
    {
      *pEndOfColumn = '\0';
    } /* endif */
    *pColData++ = pColumnStart;
    if ( pEndOfColumn )
    {
      pColumnStart = pEndOfColumn + 1;
    }
    else
    {
      pColumnStart += strlen( pColumnStart );
    } /* endif */
  } /* endfor */

  lxMaxRight   = rcl.right;
  lxLeft       = rcl.left +
                 (pIda->lAveCharWidth * (LONG)CLB_WIDTH_FACTOR *
                  (LONG)pCLB->usColDistance);

  FILLRECT( hps, rcl, pCLB->lItemBackColor );

  psViewColumn = pIda->psCurrentViewList;
  while ( *psViewColumn != CLBLISTEND )
  {
    pCol = pCLB->pColData + *psViewColumn;
    rcl.left  = (SHORT) lxLeft;
    rcl.right = rcl.left + (LOWORD(pIda->lAveCharWidth * CLB_WIDTH_FACTOR
                                   * pCol->usWidth));
    rcl.right = min( rcl.right, (SHORT)lxMaxRight);
    switch ( pCol->DataType )
    {
      case DATE_DATA:
        lDate = atol(*(pIda->pDataPtr + *psViewColumn));
        UtlLongToDateString( lDate, chDate, sizeof(chDate) );
        pszColText = chDate;
        break;

      case TIME_DATA:
        lDate = atol(*(pIda->pDataPtr + *psViewColumn));
        UtlLongToTimeString( lDate, chDate, sizeof(chDate) );
        pszColText = chDate;
        break;

      case DATETIME_DATA:
        lDate = atol(*(pIda->pDataPtr + *psViewColumn));
        UtlLongToDateString( lDate, chDate, sizeof(chDate) );
        ulLen = strlen(chDate);
        chDate[ulLen] = BLANK;
        UtlLongToTimeString( lDate, chDate + ulLen + 1,
                             (USHORT)(sizeof(chDate) - ulLen - 1) );
        pszColText = chDate;
        break;

      case FDATE_DATA:
        *((PUSHORT)&FDate) = (USHORT)atoi(*(pIda->pDataPtr + *psViewColumn));
        UtlFDateToDateString( &FDate, chDate, sizeof(chDate) );
        pszColText = chDate;
        break;

      case FTIME_DATA:
        *((PUSHORT)&FTime) = (USHORT)atoi(*(pIda->pDataPtr + *psViewColumn));
        UtlFTimeToTimeString( &FTime, chDate, sizeof(chDate) );
        pszColText = chDate;
        break;

      case FDATETIME_DATA:
        *((PUSHORT)&FDate) = (USHORT)atoi(*(pIda->pDataPtr + *psViewColumn));
        pszTemp = strchr( *(pIda->pDataPtr + *psViewColumn), '.' );
        if ( pszTemp )
        {
          *((PUSHORT)&FTime) = (USHORT)atoi( pszTemp + 1 );
        }
        else
        {
          *((PUSHORT)&FTime) = 0;
        } /* endif */
        UtlFDateToDateString( &FDate, chDate, sizeof(chDate) );
        ulLen = strlen(chDate);
        chDate[ulLen] = BLANK;
        UtlFTimeToTimeString( &FTime, chDate + ulLen + 1,
                              (sizeof(chDate) - ulLen - 1) );
        pszColText = chDate;
        break;

      case TEXT_DATA:
      case AUTOWIDTHTEXT_DATA:
      default:
        pszColText = *(pIda->pDataPtr + *psViewColumn);
        break;
    } /* endswitch */

    usDrawstate = fDisabled ? DT_HALFTONE : 0;

    // check width of data in column for AUTOWIDTHTEXT_DATA columns
    if ( pCol->DataType == AUTOWIDTHTEXT_DATA )
    {
      LONG  lColWidth;               // actual width of column data
      LONG  lOldWidth;               // current width of column

      // get actual width of column data
      LONG cx, cy;

      TEXTSIZE( hps, pszColText, cx, cy );
      lColWidth = cx;

      // enlarge column width if it is too small
      lOldWidth = (pIda->lAveCharWidth * (LONG)CLB_WIDTH_FACTOR
                   * (LONG) pCol->usWidth);
      if ( lOldWidth < lColWidth )
      {
        // adjust column width
        pCol->usWidth = (USHORT)(((lColWidth * 100L /
                                   ((LONG)CLB_WIDTH_FACTOR * pIda->lAveCharWidth)) + 99L)
                                 / 100L);

        // adjust current draw rectangle
        rcl.left  = (SHORT) lxLeft;
        rcl.right = rcl.left + (LOWORD(pIda->lAveCharWidth * CLB_WIDTH_FACTOR
                                       * pCol->usWidth));
        rcl.right = min( rcl.right, (SHORT)lxMaxRight);
        // force a repaint of column listbox
        WinPostMsg( pIda->hwndCLB, LM_EQF_REFRESH, 0L, 0L );
      } /* endif */
    } /* endif */

    if ( rcl.right > rcl.left)
    {
      if ( fDisabled )
      {
        DRAWTEXT( hps, pszColText, rcl,
                  CLR_PALEGRAY, pCLB->lItemBackColor,
                  pCol->usFormat | usDrawstate);
      }
      else
      {
        DRAWTEXT( hps, pszColText, rcl,
                  pCLB->lItemColor, pCLB->lItemBackColor,
                  pCol->usFormat | usDrawstate);
      } /* endif */
    } /* endif */
    lxLeft += pIda->lAveCharWidth * (LONG)CLB_WIDTH_FACTOR *
              (LONG) (pCol->usWidth + pCLB->usColDistance) ;
    psViewColumn++;
  } /* endwhile */

  /*******************************************************************/
  /* reset to default font ...                                       */
  /*******************************************************************/
  SelectObject( hps, hOldFont );
  DeleteObject( hFont );

} /* end of CLBDrawItem */


//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     CLBQueryItemWidth      Compute the width of a CLB item   |
//+----------------------------------------------------------------------------+
//|Function call:     CLBQueryItemWidth( PCLBIDA pIda );                       |
//+----------------------------------------------------------------------------+
//|Description:       Compute the display width of all columns in the current  |
//|                   view list                                                |
//+----------------------------------------------------------------------------+
//|Input parameter:   PCLBIDA    pIda     pointer to CLB instance data area    |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       width of current view list                               |
//+----------------------------------------------------------------------------+
//|Function flow:     while not end of current view list                       |
//|                     add width of current column to total width             |
//|                     add column distance width to total width               |
//|                     go to next column in current view list                 |
//|                   endwhile                                                 |
//|                   return computed width                                    |
//+----------------------------------------------------------------------------+
ULONG CLBQueryItemWidth( PCLBIDA pIda )
{
  PCLBCTLDATA pCLB;
  PCLBCOLDATA pCol;
  PSHORT      psViewColumn;           // current column in view column list
  ULONG        ulWidth;                // width of item

  pCLB         = pIda->pCtlData;
  ulWidth      = (pIda->lAveCharWidth * (LONG)pCLB->usColDistance);
  psViewColumn = pIda->psCurrentViewList;
  while ( *psViewColumn != CLBLISTEND )
  {
    pCol = pCLB->pColData + *psViewColumn;
    ulWidth = (ULONG)(ulWidth + (pIda->lAveCharWidth * CLB_WIDTH_FACTOR *
                         (pCol->usWidth + pCLB->usColDistance) ));
    psViewColumn++;
  } /* endwhile */
  return( ulWidth );
} /* end of CLBQueryItemWidth */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     SelectViewDlg          Dialog procedure for 'Select View'|
//+----------------------------------------------------------------------------+
//|Function call:     SelectViewDlg( HWND hwnd, USHORT msg, WPARAM mp1,        |
//|                                  LPARAM mp2 )                              |
//+----------------------------------------------------------------------------+
//|Description:       Dialog procedure for the 'Select View' dialog which is   |
//|                   currently called 'Change view details' dialog.           |
//+----------------------------------------------------------------------------+
//|Input parameter:   HWND    hwnd   Dialog window handle                      |
//|                   USHORT  msg    message identifier                        |
//|                   WPARAM  mp1    first message parameter                   |
//|                   LPARAM  mp2    second message parameter                  |
//+----------------------------------------------------------------------------+
//|Returncode type:   MRESULT                                                  |
//+----------------------------------------------------------------------------+
//|Returncodes:      depends on message type                                   |
//|                  normal return codes are:                                  |
//|                  TRUE  = message has been processed                        |
//|                  FALSE = message has not been processed                    |
//+----------------------------------------------------------------------------+
//|Function flow:     switch msg                                               |
//|                     case WM_INITDLG:                                       |
//|                       anchor supplied IDA                                  |
//|                       fill available column listbox with all column titles |
//|                       fill selected details cloumn listbox with values in  |
//|                         current details view list                          |
//|                     case WM_CHAR                                           |
//|                       if key is VK_ENTER or VK_NEWLINE and                 |
//|                          focus is on one of the list boxes then            |
//|                          return TRUE                                       |
//|                       else                                                 |
//|                          pass message to default dialog procedure          |
//|                       endif                                                |
//|                     case WM_CONTROL:                                       |
//|                       switch control ID                                    |
//|                         case available columns list box                    |
//|                           remove selected column from selected list box    |
//|                           add selected column at end of selected list box  |
//|                         case selected columns list box:                    |
//|                           remove selected column from selected list box    |
//|                       endswitch                                            |
//|                     case WM_COMMAND:                                       |
//|                       switch command ID                                    |
//|                         case VIEW pushbutton                               |
//|                           copy items in selected columns list box to       |
//|                            to details view list                            |
//|                           remove the dialog using WinDismiss dialog        |
//|                         case CLEAR pushbutton                              |
//|                           delete all items in selected columns listbox     |
//|                         case DID_CANCEL:                                   |
//|                         case CANCEL pushbutton:                            |
//|                           remove the dialog using WinDismiss dialog        |
//|                       endswitch                                            |
//|                     default:                                               |
//|                       pass message to default dialog procedure             |
//|                   endswitch                                                |
//|                   return result to calling function                        |
//+----------------------------------------------------------------------------+
INT_PTR CALLBACK SelectViewDlg
(
HWND hwnd,
WINMSG msg,
WPARAM mp1,
LPARAM mp2
)
{
  PCLBIDA       pIda;                // ptr to column listbox ida
  MRESULT       mResult = FALSE;     // result of message processing;
  PCLBCOLDATA   pColumn;             // ptr to control data for a single column
  SHORT         sItem;               // item number for listbox items
  SHORT         sColumn;             // number of currently processed column
  SHORT         sNoOfItems;          // no of items in a listbox
  PSHORT        psViewColumn;        // ptr into current view list
  USHORT        usI;                 // index for loops

  switch ( msg )
  {


    case WM_HELP:
      /*************************************************************/
      /* pass on a HELP_WM_HELP request                            */
      /*************************************************************/
#ifndef _TQM
      EqfDisplayContextHelp( (HWND) ((LPHELPINFO) mp2)->hItemHandle,
                             &hlpsubtblFolListChangeView[0] );
#endif
      mResult = TRUE;  // message processed
      break;




    case WM_INITDLG:
      //--- get and save pointer to IDA ------
      pIda = (PCLBIDA) mp2;
      ANCHORDLGIDA( hwnd, pIda);

      //----- fill available details listbox -----
      pColumn = pIda->pCtlData->pColData;
      for ( usI = 0; usI < pIda->pCtlData->usNoOfColumns; usI++ )
      {
        //--- insert all non-empty column titles ---
        if ( (pColumn->pszTitle != NULL) && (*pColumn->pszTitle != '\0') )
        {
          sItem = (SHORT) INSERTITEMEND(hwnd,ID_SELVIEW_AVAIL_LB,
                                        pColumn->pszTitle);

          if ( sItem != LIT_NONE )
          {
            SETITEMHANDLE( hwnd, ID_SELVIEW_AVAIL_LB, sItem, usI );
          } /* endif */
        } /* endif */
        pColumn++;
      } /* endwhile */

      //----- fill selected details listbox -----
      psViewColumn = pIda->pCtlData->psDetailsViewList;
      while ( *psViewColumn != CLBLISTEND )
      {
        pColumn = pIda->pCtlData->pColData + *psViewColumn;

        //--- insert selected non-empty column titles ---
        if ( (pColumn->pszTitle != NULL) && (*pColumn->pszTitle != '\0') )
        {
          sItem = (SHORT) INSERTITEMEND( hwnd, ID_SELVIEW_SELECT_LB,
                                         pColumn->pszTitle );

          if ( sItem != LIT_NONE )
          {
            SETITEMHANDLE( hwnd, ID_SELVIEW_SELECT_LB, sItem,
                           *psViewColumn );
          } /* endif */
        } /* endif */
        psViewColumn++;
      } /* endwhile */
      break;

    case DM_GETDEFID:
      /************************************************************/
      /* check if user pressed the ENTER key, but wants only to   */
      /* select/deselect an item of the listbox via a simulated   */
      /* (keystroke) double click.                                */
      /************************************************************/
      {
        HWND   hwndFocus;          // handle of focus window

        if ( GetKeyState(VK_RETURN) & 0x8000 )
        {
          hwndFocus = GetFocus();
          if ( hwndFocus == GetDlgItem( hwnd, ID_SELVIEW_AVAIL_LB ) )
          {
            SelectViewDlgControl( hwnd, ID_SELVIEW_AVAIL_LB, LN_ENTER );
            mResult = TRUE;
          }
          else if ( hwndFocus == GetDlgItem( hwnd, ID_SELVIEW_SELECT_LB ) )
          {
            SelectViewDlgControl( hwnd, ID_SELVIEW_SELECT_LB, LN_ENTER );
            mResult = TRUE;
          } /* endif */
        } /* endif */
      }
      break;
    case WM_DESTROY:
      break;

      /****************************************************************/
      /* under Windows NO WM_Control message is sent - everything is  */
      /* handled via the WM_COMMAND ....                              */
      /****************************************************************/
    case WM_COMMAND:
      pIda = ACCESSDLGIDA( hwnd, PCLBIDA );
      switch ( WMCOMMANDID( mp1, mp2 ) )
      {
        case ID_SELVIEW_VIEW_PB:
          sNoOfItems = QUERYITEMCOUNT( hwnd, ID_SELVIEW_SELECT_LB);
          psViewColumn = pIda->pCtlData->psDetailsViewList;
          sItem = 0;               // start with first item
          while ( sNoOfItems > 0 )
          {
            sColumn = GETITEMHANDLE( hwnd, ID_SELVIEW_SELECT_LB,
                                     sItem, SHORT );
            *psViewColumn++ = sColumn;
            sItem++;
            sNoOfItems--;
          } /* endwhile */
          *psViewColumn = CLBLISTEND;
          WinDismissDlg( hwnd, TRUE );
          break;

        case ID_SELVIEW_CLEAR_PB:
          DELETEALL( hwnd, ID_SELVIEW_SELECT_LB );
          ENABLECTRL( hwnd, ID_SELVIEW_VIEW_PB, FALSE );
          break;

        case DID_CANCEL:
        case ID_SELVIEW_CANCEL_PB:
          DISMISSDLG( hwnd, FALSE );
          break;
          /************************************************************/
          /* here our Notification messages for Windows will be       */
          /* handled...                                               */
          /************************************************************/
        case ID_SELVIEW_AVAIL_LB:
        case ID_SELVIEW_SELECT_LB:
          SelectViewDlgControl( hwnd, WMCOMMANDID(mp1,mp2),
                                WMCOMMANDCMD(mp1,mp2));
          break;
      } /* endswitch */
      break;

    default:
      mResult = WinDefDlgProc( hwnd, msg, mp1, mp2 );
      break;
  } /* endswitch */

  return( mResult );
}


/**********************************************************************/
/* handle the WM_Control messages for the SelectViewDlg ...           */
/**********************************************************************/
VOID
SelectViewDlgControl
(
HWND hwnd,                           // handle of listbox
SHORT  sId,                          // id of control in action
SHORT  sNotification                 // notification
)
{
  PCLBIDA       pIda;                // ptr to column listbox ida
  SHORT         sItem;               // item number for listbox items
  SHORT         sColumn;             // number of currently processed column
  SHORT         sNoOfItems;          // no of items in a listbox

  switch ( sId )
  {
    case ID_SELVIEW_AVAIL_LB:
      switch ( sNotification )
      {
        case LN_ENTER :
          //--- address dialog IDA ---
          pIda = ACCESSDLGIDA( hwnd, PCLBIDA );

          //--- get selected column ---
          sColumn = CLBLISTEND;        //--- no column yet
          sItem = QUERYSELECTION( hwnd, ID_SELVIEW_AVAIL_LB );
          if ( sItem != LIT_NONE )
          {
            sColumn = GETITEMHANDLE( hwnd, ID_SELVIEW_AVAIL_LB,
                                     sItem, SHORT );
          } /* endif */

          //--- remove column from selected items listbox ----
          if ( sColumn != CLBLISTEND )
          {
            sNoOfItems = QUERYITEMCOUNT( hwnd, ID_SELVIEW_SELECT_LB);
            while ( sNoOfItems > 0 )
            {
              sNoOfItems--;
              if ( sColumn == GETITEMHANDLE( hwnd,
                                             ID_SELVIEW_SELECT_LB,
                                             sNoOfItems, SHORT ))
              {
                DELETEITEM( hwnd,ID_SELVIEW_SELECT_LB,sNoOfItems );
                sNoOfItems = -1;    // trigger while condition
              } /* endif */
            } /* endwhile */
          } /* endif */

          //--- add column to end of selected items listbox ---
          if ( sColumn != CLBLISTEND )
          {
            sItem = (SHORT)INSERTITEMEND( hwnd, ID_SELVIEW_SELECT_LB,
                                          (pIda->pCtlData->pColData+sColumn)->pszTitle);
            SETITEMHANDLE( hwnd, ID_SELVIEW_SELECT_LB,
                           sItem, sColumn );

            ENABLECTRL( hwnd, ID_SELVIEW_VIEW_PB, TRUE );
          } /* endif */
          break;
      } /* endswitch */
      break;
    case ID_SELVIEW_SELECT_LB:
      switch ( sNotification )
      {
        case LN_ENTER :
          //--- remove selected item from listbox ---
          sItem = QUERYSELECTION( hwnd, ID_SELVIEW_SELECT_LB );
          if ( sItem != LIT_NONE )
          {
            DELETEITEM( hwnd, ID_SELVIEW_SELECT_LB, sItem );
            sNoOfItems = QUERYITEMCOUNT( hwnd, ID_SELVIEW_SELECT_LB);
            if ( sNoOfItems == 0 )
            {
              ENABLECTRL( hwnd, ID_SELVIEW_VIEW_PB, FALSE );
            } /* endif */
          } /* endif */
          break;
      } /* endswitch */
      break;
  } /* endswitch */
  return;
}
//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     CLBCopyViewList        Copy a view list                  |
//+----------------------------------------------------------------------------+
//|Function call:     CLBCopyViewList( PSHORT psTarget, PSHORT psSource,       |
//|                                    USHORT usNoOfColumns );                 |
//+----------------------------------------------------------------------------+
//|Description:       Copies a view list up to the CLBLISTEND delimiter or     |
//|                   until the max. number of columns has been reached.       |
//+----------------------------------------------------------------------------+
//|Input parameter:   PSHORT    psTarget      pointer to target view list      |
//|                   PSHORT    psSource      pointer to sopurce view list     |
//|                   USHORT    usNoOfColumns number of columns defined        |
//+----------------------------------------------------------------------------+
//|Returncode type:   VOID                                                     |
//+----------------------------------------------------------------------------+
//|Side effects:      View list pointed to by psTarget is filled with data     |
//|                   from psSource view list                                  |
//+----------------------------------------------------------------------------+
//|Function flow:     while not end of view list and                           |
//|                         not usNoOfColumns copied and                       |
//|                         not MAX_VIEW columns copied                        |
//|                     copy current view column from source to target         |
//|                     increment source and target pointer                    |
//|                   endwhile                                                 |
//|                   if last entry processed is not CLBLISTEND then           |
//|                     issue internal error message                           |
//|                   endif                                                    |
//|                   terminate target view list with CLBLISTEND               |
//+----------------------------------------------------------------------------+
VOID CLBCopyViewList( PSHORT psTarget, PSHORT psSource, USHORT usNoOfColumns )
{
  USHORT usMaxView = MAX_VIEW;        // counter for MAX_VIEW columns processed
  USHORT usMaxCol;                    // counter for max columns processed

  usMaxCol = usNoOfColumns;

  if ( psSource )
  {
    while ( usMaxCol                    &&
            usMaxView                   &&
            ( *psSource != CLBLISTEND )   &&
            ( (*psSource & ~SORT_DESCENDING_FLAG) < (SHORT)usNoOfColumns ) )
    {
      *psTarget++ = *psSource++;
      usMaxCol--;
      usMaxView--;
    } /* endwhile */
  } /* endif */

  //--- was view list corrupted ???? ---
  if ( *psSource != CLBLISTEND )      // if source list not terminated ...
  {
    if ((usMaxView >0 ) || (usMaxCol > 0 ))
    {
      UtlError( ERROR_INTERNAL,
                MB_CANCEL,
                0,
                NULL,
                INTERNAL_ERROR );
    }
  } /* endif */

  *psTarget = CLBLISTEND;
}

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     CLBActivateViewList    Activate a specific view list     |
//+----------------------------------------------------------------------------+
//|Function call:     _                                                        |
//+----------------------------------------------------------------------------+
//|Description:       _                                                        |
//+----------------------------------------------------------------------------+
//|Input parameter:   PCLBIDA       pIda          ptr to column listbox ida    |
//|                   PSHORT        psViewList    new view list                |
//|                   USHORT        usListType    type of new list             |
//+----------------------------------------------------------------------------+
//|Returncode type:   BOOL                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:       TRUE (always)                                            |
//+----------------------------------------------------------------------------+
//|Function flow:     Check if view list has changed                           |
//|                   remember new view list type                              |
//|                   if view list has changed                                 |
//|                    copy supplied view list to current view list            |
//|                    force a recomputation of the list box width by          |
//|                     inserting and deleting of a dummy item                 |
//|                    activate new view list by invalidating the CLB region   |
//|                   endif                                                    |
//+----------------------------------------------------------------------------+
BOOL CLBActivateViewList
(
PCLBIDA       pIda,                // ptr to column listbox ida
PSHORT        psViewList,          // new view list
USHORT        usListType           // type of new list; e.g. NAME_VIEW
)
{
  PSHORT    psOld;                    // ptr into old view list
  PSHORT    psNew;                    // ptr into new view list

  //--- check if new viewlist is already active ---
  psOld = pIda->psCurrentViewList;
  psNew = psViewList;
  while ( (*psOld != CLBLISTEND) &&
          (*psNew != CLBLISTEND) &&
          (*psNew == *psOld) )
  {
    psNew++;
    psOld++;
  } /* endwhile */

  //--- set new view list type ---
  pIda->usViewListType = usListType;

  //--- do the remaining actions only if view has been changed ! ---
  if ( (*psNew != *psOld) )
  {
    //--- make the new view list the active one ---
    CLBCopyViewList( pIda->psCurrentViewList, psViewList,  MAX_VIEW+1 );

    // refresh the listbox
    WinSendMsg( pIda->hwndCLB, LM_EQF_REFRESH, 0L, 0L );
  } /* endif */

  return( TRUE );
}
/* end of CLBActivateViewList */


//+----------------------------------------------------------------------------+
//| SetFixedFont                                                               |
//+----------------------------------------------------------------------------+
//| Description:                                                               |
//|    Set the font of the MLE to a font with the same character size as the   |
//|    default system font, but change the typeface of the font to             |
//|    'System Monospaced'.                                                    |
//+----------------------------------------------------------------------------+
//| Arguments:                                                                 |
//+----------------------------------------------------------------------------+
//| Returns:                                                                   |
//|    BOOL                                                                    |
//+----------------------------------------------------------------------------+
//| Prereqs:                                                                   |
//|    None                                                                    |
//+----------------------------------------------------------------------------+
//| SideEffects:                                                               |
//|    None                                                                    |
//+----------------------------------------------------------------------------+
BOOL SetFixedFont
(
HWND          hwnd,
PCLBIDA       pIda                // ptr to column listbox ida
)
{
  BOOL          fOK = TRUE;           // internal OK flag
  LOGFONT  lf;                      // logical font structure
  HFONT    hFont, hOldFont;         // handle of created font
  TEXTMETRIC tm;                    // filled text metrics
  HDC      hdc;                     // device context
  PPROPSYSTEM pPropSys = GetSystemPropPtr();



  memset(&lf, 0, sizeof(lf));
  hdc = GetDC( hwnd );
  GetTextMetrics( hdc, &tm );

  lf.lfHeight = tm.tmHeight; // GetSystemMetrics(SM_CYSCREEN) / 30;
  lf.lfWidth  = tm.tmAveCharWidth; //GetSystemMetrics(SM_CXSCREEN) / 80;
  lf.lfCharSet = (BYTE)GetCharSet();      // get SBCS or DBCS characterset

  if ( !IsDBCS_CP(pPropSys->ulSystemPrefCP) )
  {
    SelectObject( hdc, GetStockObject(SYSTEM_FIXED_FONT) );
    lf.lfPitchAndFamily = FIXED_PITCH;
    GetTextFace( hdc, sizeof( lf.lfFaceName ), lf.lfFaceName );
  }
  else
  {
    // lf.lfHeight = GetSystemMetrics(SM_CYSCREEN) / 30;
    lf.lfWeight = FW_NORMAL;
    lf.lfOutPrecision = OUT_TT_PRECIS;
    lf.lfQuality = DEFAULT_QUALITY;
    lf.lfPitchAndFamily = FIXED_PITCH | FF_DONTCARE;
    strcpy (lf.lfFaceName, "‚l‚r ƒSƒVƒbƒN");
  } /* endif */

  hFont = CreateFontIndirect( &lf );
  hOldFont = (HFONT) SelectObject( hdc, hFont );

  GetTextMetrics( hdc, &tm );

  SelectObject( hdc, hOldFont );
  DeleteObject( hFont );
  ReleaseDC (hwnd, hdc);
  pIda->lf = lf;
  pIda->lCharHeight    = tm.tmHeight;
  pIda->lLineHeight    = tm.tmExternalLeading;
  pIda->lAveCharWidth  = tm.tmAveCharWidth;
  return( fOK );

} /* end of SetFixedFont */

static int asSelItems[400];          // local buffer for selected items

LONG FAR PASCAL EqfListboxProc( register HWND hwnd, UINT msg,
                                register WPARAM mp1, LPARAM mp2)
{
  switch (msg)
  {
    case WM_RBUTTONDOWN:
      /* Draw the "floating" popup in the app's client area */
      {
        RECT rc;
         POINT pt;

        GetClientRect( hwnd, (LPRECT)&rc);
        pt.x = LOWORD(mp2);
        pt.y = HIWORD(mp2);
        if (PtInRect ((LPRECT)&rc, pt))
        {
          HWND       hwndCLB, hwndList;
          BOOL       fItemAlreadySelected = FALSE;

          /************************************************************/
          /* Get handle of listbox owner window                       */
          /************************************************************/
          hwndCLB  = GetParent( hwnd );
          hwndList = GetParent( hwndCLB );

          /************************************************************/
          /* Get all selected items of the listbox and check if one   */
          /* these items is under the cursor                          */
          /************************************************************/
          {
            SHORT sSelItems;          // number of selected items
            SHORT sNextItem;          // index of next selected item
            int   i;                  // loop index

            /********************************************************************/
            /* Get number of selected items, LB_ERR indicates that the listbox  */
            /* is a single selection listbox!                                   */
            /********************************************************************/
            sSelItems = (SHORT)SendMessage( hwnd, LB_GETSELCOUNT, 0, 0L );

            /********************************************************************/
            /* Retrieve selected items                                          */
            /********************************************************************/
            if ( sSelItems == LB_ERR )
            {
              /******************************************************************/
              /* That's a single selection listbox!                             */
              /* Get current selection and check if it is in front of the       */
              /* position requested                                             */
              /******************************************************************/
              sNextItem = (SHORT)SendMessage( hwnd, LB_GETCURSEL, 0, 0L );
              if ( sNextItem >= 0 )
              {
                asSelItems[0] = sNextItem;
                sSelItems = 1;
              }
              else
              {
                sSelItems = 0;
              } /* endif */
            }
            else
            {
              /******************************************************************/
              /* Get selected items into our local buffer                       */
              /******************************************************************/
              SendMessage( hwnd, LB_GETSELITEMS, 400, MP2FROMP(asSelItems) );
            } /* endif */

            /************************************************************/
            /* Check if one of the selected items is currently under    */
            /* the cursor                                               */
            /************************************************************/
            for ( i = 0; i < sSelItems; i++ )
            {
              RECT rcItem;
              POINT pt;
              memset( &rcItem, 0, sizeof(RECT) );
              SendMessage( hwnd, LB_GETITEMRECT, MP1FROMSHORT(asSelItems[i]),
                           MP2FROMP(&rcItem) );
              pt.x = LOWORD(mp2);
              pt.y = HIWORD(mp2);
              if (PtInRect ( &rcItem, pt))
              {
                fItemAlreadySelected = TRUE;
                break;
              } /* endif */
            } /* endfor */
          }


          /************************************************************/
          /* Select current item                                      */
          /************************************************************/
          if ( !fItemAlreadySelected )
          {
            CallWindowProc( (WNDPROC)lpfnOldListboxProc, hwnd, WM_LBUTTONDOWN, mp1, mp2 );
            CallWindowProc( (WNDPROC)lpfnOldListboxProc, hwnd, WM_LBUTTONUP, mp1, mp2 );
          } /* endif */

          /************************************************************/
          /* Leave it to owner to handle the request appribriately    */
          /************************************************************/
          SendMessage( hwndList, WM_EQF_SHOWPOPUP, MP1FROMHWND(hwnd), mp2 );
          return 0;
        } /* endif */
      }
      break;

    case WM_KEYDOWN:
      if ( mp1 == VK_F1 )
      {
        /**************************************************************/
        /* Post message to the list window (the owner of the listbox  */
        /* is the column listbox, the owner of the column listbox is  */
        /* the list window)                                           */
        /**************************************************************/
        HWND       hwndCLB, hwndList;

        hwndCLB  = GetParent( hwnd );
        hwndList = GetParent( hwndCLB );

        PostMessage( hwndList, WM_KEYDOWN, mp1, mp2 );
      } /* endif */
      break;

    case WM_HSCROLL:
      PostMessage( GetParent( hwnd ), WM_HSCROLL, mp1, mp2 );
      break;

    default:
      break;
  } /* endswitch */

  // call old window procedure
  return CallWindowProc( (WNDPROC)lpfnOldListboxProc, hwnd, msg, mp1, mp2 );
}


//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     CLBPrintList                                             |
//+----------------------------------------------------------------------------+
//|Function call:     _                                                        |
//+----------------------------------------------------------------------------+
//|Description:       Print the contents of the column listbox using the active|
//|                   view list                                                |
//+----------------------------------------------------------------------------+
//|Input parameter:   PCLBIDA       pIda          ptr to column listbox ida    |
//+----------------------------------------------------------------------------+
//|Returncode type:   BOOL                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:       TRUE       function completed successfully               |
//|                   FALSE      an error occured                              |
//+----------------------------------------------------------------------------+
// size of print line buffer
  #define PRINTBUFSIZE        8096

BOOL CLBPrintList
(
PCLBIDA       pIda                 // ptr to column listbox ida
)
{
  BOOL        fOK = TRUE;              // internal O.K. flag and return code
  PSHORT      psViewColumn;            // current column in view column list
  PSZ         pszLine = NULL;          // ptr to buffer for print line
  PCLBCTLDATA pCLB;                    // ptr to column listbox control data
  PCLBCOLDATA pCol;                    // ptr to column listbox column table
  HPRINT      hPrint = NULLHANDLE;     // print handle
  PSZ_W       pszLineW = NULL;         // Unicode line

  PPROPSYSTEM  pPropSys = GetSystemPropPtr();

  // initialize some pointers
  pCLB = pIda->pCtlData;

  // allocate print line buffer
  fOK = UtlAlloc( (PVOID *)&pszLine, 0L, (LONG)PRINTBUFSIZE * (sizeof(CHAR_W)+1), ERROR_STORAGE );

  // open print context
  if ( fOK )
  {
    pszLineW = (PSZ_W) (pszLine+PRINTBUFSIZE);
    QUERYTEXTHWNDLEN( GETPARENT(pIda->hwndCLB), pszLine, PRINTBUFSIZE );
    ASCII2Unicode( pszLine, pszLineW, 0L);
    fOK = UtlPrintOpen( &hPrint, pszLine, GETPARENT(pIda->hwndCLB) );
  } /* endif */

  // print header
  if ( fOK )
  {
    strcat( pszLine, "\n" );
    ASCII2Unicode( pszLine, pszLineW, 0L);
    fOK = UtlPrintLineW( hPrint, pszLineW );
    strcpy( pszLine, "\n" );
    ASCII2Unicode( pszLine, pszLineW, 0L);
    fOK = UtlPrintLineW( hPrint, pszLineW );
  } /* endif */

  // print column title line
  if ( fOK )
  {
    PCLBCTLDATA pCLB = pIda->pCtlData;
    PCLBCOLDATA pCol = pCLB->pColData;
    USHORT usCurColStart;              // start of current column in output line
    USHORT usActColStart;              // actual start of column text (may be

    psViewColumn = pIda->psCurrentViewList;
    usCurColStart = 0;                 // we start at begin of pszLine
    usActColStart = 0;                 // we start at begin of pszLine

    while ( *psViewColumn != CLBLISTEND )
    {
      pCol = pCLB->pColData + *psViewColumn;

      // Filter column title
      {
        PSZ pszSource = pCol->pszTitle;
        PSZ pszTarget = pIda->szBuffer;
        while ( *pszSource &&
                ((pszTarget - pIda->szBuffer) < (sizeof(pIda->szBuffer) - 1)))
        {
          if ( *pszSource == CLB_START_COMMENT )
          {
            while ( *pszSource && (*pszSource != CLB_END_COMMENT ) )
            {
              if ( IsDBCS_CP(pPropSys->ulSystemPrefCP) &&
                   (isdbcs1ex((USHORT)pPropSys->ulSystemPrefCP, *pszSource) == DBCS_1ST) )
              {
                // skip both bytes of DBCS character
                pszSource++;
                if ( *pszSource ) pszSource++;
              }
              else
              {
                pszSource++;
              } /* endif */
            } /* endwhile */
            if ( *pszSource == CLB_END_COMMENT )
            {
              pszSource++;
            } /* endif */
          }
          else
          {
            if ( IsDBCS_CP(pPropSys->ulSystemPrefCP) &&
                 (isdbcs1ex((USHORT)pPropSys->ulSystemPrefCP, *pszSource) == DBCS_1ST) )
            {
              // copy both DBCS bytes
              *pszTarget++ = *pszSource++;
              if ( *pszSource ) *pszTarget++ = *pszSource++;
            }
            else
            {
              *pszTarget++ = *pszSource++;
            } /* endif */
          } /* endif */
        } /* endwhile */
        *pszTarget = EOS;
      }

      // fill any gaps in front of column with blanks
      while ( usActColStart < usCurColStart )
      {
        pszLine[usActColStart++] = BLANK;
      } /* endif */

      // right adjust data if required
      if ( pCol->usFormat & DT_RIGHT )
      {
        ULONG ulDataLen = strlen(pIda->szBuffer);
        if ( (usActColStart + ulDataLen) < (ULONG)(usCurColStart + pCol->usWidth) )
        {
          ULONG ulPadChars = usCurColStart + pCol->usWidth -
                              usActColStart - ulDataLen;
          while ( ulPadChars )
          {
            pszLine[usActColStart++] = BLANK;
            ulPadChars--;
          } /* endwhile */
        } /* endif */
      } /* endif */

      // insert title of current column
      {
        PSZ pszTextTemp = pIda->szBuffer;
        while ( *pszTextTemp )
        {
          pszLine[usActColStart++] = *pszTextTemp++;
        } /* endwhile */
        pszLine[usActColStart++] = BLANK;
      }

      usCurColStart += pCol->usWidth + 1;   // add with of current column
      psViewColumn++;
    } /* endwhile */

    pszLine[usActColStart++] = LF;
    pszLine[usActColStart++] = EOS;

    ANSITOOEM( pszLine );
    ASCII2Unicode( pszLine, pszLineW, 0L);
    fOK = UtlPrintLineW( hPrint, pszLineW );
  } /* endif */

  // print all items of column listbox
  if ( fOK )
  {
    SHORT sNoOfItems = QUERYITEMCOUNTHWND( pIda->hwndLB );
    SHORT sItem = 0;                   // start with first item
    PSZ   pszColText;                  // ptr to text for a specific column
    CHAR  chDate[50];                  // buffer for date in ASCII format
    USHORT usCurColStart;              // start of current column in output line
    USHORT usActColStart;              // actual start of column text (may be
                                       // larger than usCurColPos if the data for
                                       // a column exceeded the width of the
                                       // column)
    // loop over all listbox items
    while ( sItem < sNoOfItems )
    {
      // get item text
      {
        PSZ pszData = (PSZ)QUERYITEMHANDLEHWND( pIda->hwndLB, sItem );
        if ( pszData != NULL )
        {
          strcpy( pIda->szWork, pszData + 4 );
        }
        else
        {
          pIda->szWork[0] = EOS;
        } /* endif */
      }
       // setup column data pointers
      {
        PSZ   pEndOfColumn;            // ptr to end of column data
        PSZ   pColumnStart;            // ptr to start of column data
        PSZ   *pColData;               // current position in data ptr table
        USHORT usI;                    // loop index

        pColData = pIda->pDataPtr;     // start with first string ptr
        pColumnStart = pIda->szWork;   // set column data start pointer
        for ( usI = 0; usI < pCLB->usNoOfColumns; usI++ )
        {
          pEndOfColumn = strchr( pColumnStart, pCLB->chDataSeperator );
          if ( pEndOfColumn ) *pEndOfColumn = '\0';

          *pColData++ = pColumnStart;
          pColumnStart = ( pEndOfColumn ) ? (pEndOfColumn + 1) :
                         (pColumnStart + strlen( pColumnStart ) );
        } /* endfor */
      }

      // add data of selected columns to output buffer
      psViewColumn = pIda->psCurrentViewList;
      usCurColStart = 0;               // we start at begin of pszLine
      usActColStart = 0;               // we start at begin of pszLine
      while ( *psViewColumn != CLBLISTEND )
      {
        pCol = pCLB->pColData + *psViewColumn;

        switch ( pCol->DataType )
        {
          case DATE_DATA:
            {
              LONG lDate = atol(*(pIda->pDataPtr + *psViewColumn));
              UtlLongToDateString( lDate, chDate, sizeof(chDate) );
              pszColText = chDate;
            }
            break;

          case TIME_DATA:
            {
              LONG lDate = atol(*(pIda->pDataPtr + *psViewColumn));
              UtlLongToTimeString( lDate, chDate, sizeof(chDate) );
              pszColText = chDate;
            }
            break;

          case DATETIME_DATA:
            {
              ULONG ulLen;
              LONG lDate = atol(*(pIda->pDataPtr + *psViewColumn));
              UtlLongToDateString( lDate, chDate, sizeof(chDate) );
              ulLen = strlen(chDate);
              chDate[ulLen] = BLANK;
              UtlLongToTimeString( lDate, chDate + ulLen + 1,
                                   (USHORT)(sizeof(chDate) - ulLen - 1) );
              pszColText = chDate;
            }
            break;

          case FDATE_DATA:
            {
              FDATE FDate;          // date in FDATE format

              *((PUSHORT)&FDate) = (USHORT)atoi(*(pIda->pDataPtr + *psViewColumn));
              UtlFDateToDateString( &FDate, chDate, sizeof(chDate) );
              pszColText = chDate;
            }
            break;

          case FTIME_DATA:
            {
              FTIME FTime;          // time in FTIME format

              *((PUSHORT)&FTime) = (USHORT)atoi(*(pIda->pDataPtr + *psViewColumn));
              UtlFTimeToTimeString( &FTime, chDate, sizeof(chDate) );
              pszColText = chDate;
            }
            break;

          case FDATETIME_DATA:
            {
              ULONG ulLen;
              FDATE FDate;          // date in FDATE format
              FTIME FTime;          // time in FTIME format
              PSZ pszTemp;          // temporary data pointer

              *((PUSHORT)&FDate) = (USHORT)atoi(*(pIda->pDataPtr + *psViewColumn));
              pszTemp = strchr( *(pIda->pDataPtr + *psViewColumn), '.' );
              if ( pszTemp )
              {
                *((PUSHORT)&FTime) = (USHORT)atoi( pszTemp + 1 );
              }
              else
              {
                *((PUSHORT)&FTime) = 0;
              } /* endif */
              UtlFDateToDateString( &FDate, chDate, sizeof(chDate) );
              ulLen = strlen(chDate);
              chDate[ulLen] = BLANK;
              UtlFTimeToTimeString( &FTime, chDate + ulLen + 1,
                                    (sizeof(chDate) - ulLen - 1) );
              pszColText = chDate;
            }
            break;

          case TEXT_DATA:
          case AUTOWIDTHTEXT_DATA:
          default:
            pszColText = *(pIda->pDataPtr + *psViewColumn);
            break;
        } /* endswitch */

        // add data of column to output line
        {
          // fill any gaps in front of column with blanks
          while ( usActColStart < usCurColStart )
          {
            pszLine[usActColStart++] = BLANK;
          } /* endif */

          // right adjust data if required
          if ( pCol->usFormat & DT_RIGHT )
          {
            ULONG ulDataLen = strlen(pszColText);
            if ( (usActColStart + ulDataLen) < (ULONG)(usCurColStart + pCol->usWidth) )
            {
              ULONG ulPadChars = usCurColStart + pCol->usWidth -
                                  usActColStart - ulDataLen;
              while ( ulPadChars )
              {
                pszLine[usActColStart++] = BLANK;
                ulPadChars--;
              } /* endwhile */
            } /* endif */
          } /* endif */

          // insert data of current column
          {
            PSZ pszTextTemp = pszColText;
            while ( *pszTextTemp )
            {
              pszLine[usActColStart++] = *pszTextTemp++;
            } /* endwhile */
            pszLine[usActColStart++] = BLANK;
          }
          usCurColStart += pCol->usWidth + 1; // add width of current column
        }

        // next column
        psViewColumn++;
      } /* endwhile */

      // print current item line
      pszLine[usActColStart++] = LF;
      pszLine[usActColStart++] = EOS;

      ANSITOOEM( pszLine );
      ASCII2Unicode( pszLine, pszLineW, 0L);
      fOK = UtlPrintLineW( hPrint, pszLineW );
      // continue with next item
      sItem++;
    } /* endwhile */
  } /* endif */

  // close print context and cleanup
  if ( hPrint )          UtlPrintClose( hPrint );
  if ( pszLine != NULL ) UtlAlloc( (PVOID *)&pszLine, 0L, 0L, NOMSG );
  return( fOK );
}
/* end of CLBPrintList */
