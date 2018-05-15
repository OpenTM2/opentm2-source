/******************************************************************************/
//+----------------------------------------------------------------------------+
//|EQFLSTIE.C                                                                  |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|          Copyright (C) 1990-2012, International Business Machines          |
//|          Corporation and others. All rights reserved                       |
//|                                                                            |
//|                                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Author: Gerhard Queck (QSoft)                                               |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//|  Import and export functions of the list handler                           |
//+----------------------------------------------------------------------------+
//|Entry Points:                                                               |
//|                                                                           |
//|+-- status ("H"=Header,"D"=Design,"C"=Code,"T"=Test, " "=complete,          |
//|            "Q"=Quick-and-dirty )                                           |
//+----------------------------------------------------------------------------+
//|Externals:                                                                  |
//+----------------------------------------------------------------------------+
//|Internals:                                                                  |
//|                                                                           |
//|+-- status ("H"=Header,"D"=Design,"C"=Code,"T"=Test, " "=complete,          |
//|            "Q"=Quick-and-dirty )                                           |
//+----------------------------------------------------------------------------+
//|To be done / known limitations / caveats:                                   |
//|                                                                            |
//|  - in export in user format only the first context of the term is          |
//|    exported.                                                               |
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
// $Revision: 1.2 $ ----------- 25 Jul 2005
// GQ: - fixed P022466: cannot export found terms list
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
// $Revision: 1.2 $ ----------- 29 Sep 2004
// --RJ: accessibility: support Enter on Help PB
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
// $Revision: 1.2 $ ----------- 15 Jan 2004
// --RJ: Write Noise-Excl lists: UtlBufWriteConv: add ulAnsiCP
//
//
// $Revision: 1.1 $ ----------- 15 Dec 2003
//  -- New Release TM6.0.4!!
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
// $Revision: 1.2 $ ----------- 24 Feb 2003
// --RJ: delete obsolete code and remove (if possible)compiler warnings
//
//
// $Revision: 1.1 $ ----------- 20 Feb 2003
//  -- New Release TM6.0.1!!
//
//
// $Revision: 1.3 $ ----------- 16 Sep 2002
// --RJ: P015855: use OEM-CP for Unicode2Ansi too
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
// $Revision: 1.4 $ ----------- 17 Dec 2001
// RJ: Export Exclusion list: use LstReadNoiseExclList to read list correctly; allow Unicode
//
//
// $Revision: 1.3 $ ----------- 11 Dec 2001
// --RJ: use EQFGetCPOem() instead of CP_OEMCP
//
//
// $Revision: 1.2 $ ----------- 25 Sep 2001
// -- RJ: change LstBufWrite into UtlBufWriteConv
//
//
// $Revision: 1.1 $ ----------- 16 Aug 2001
//  -- New Release TM2.7.2!!
//
//
// $Revision: 1.4 $ ----------- 10 Apr 2001
// GQ: Adjusted to new type of size parameter of term lists
//
//
// $Revision: 1.3 $ ----------- 14 Feb 2001
// - added handling for export/import in ANSI format
//
//
//
// $Revision: 1.2 $ ----------- 4 Dec 1999
//  -- Initial Revision!!
//
/*
 * $Header:   J:\DATA\EQFLSTIE.CV_   1.4   09 Feb 1998 17:15:08   BUILD  $
 *
 * $Log:   J:\DATA\EQFLSTIE.CV_  $
 *
 *    Rev 1.4   09 Feb 1998 17:15:08   BUILD
 * - Win32: use UtlDirExist to check for the existence of directories instead of
 *   UtlFileExist which does not work for directories anymore
 *
 *    Rev 1.3   14 Jan 1998 16:07:24   BUILD
 * - migrated to Win32 environment
 *
 *    Rev 1.2   18 Mar 1996 16:29:30   BUILD
 * - fixed PTM KWT0419: All import dialogs should have the correct file extension
 *
 *    Rev 1.0   09 Jan 1996 09:08:54   BUILD
 * Initial revision.
*/
//+----------------------------------------------------------------------------+

#define INCL_EQF_LIST             // terminology list functions
#define INCL_EQF_DLGUTILS         // dialog utilities
#include <eqf.h>                  // General Translation Manager include file

#include "eqflp.h"                // Defines for generic list handlers
#include "eqflisti.h"             // Private List Handler defines
#include "eqflist.id"             // List Handler IDs
#include "eqflstie.h"             // List export/import private defines

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     LSTEXPORTDLG   Dialog procedure for the list export dlg  |
//+----------------------------------------------------------------------------+
//|Function call:     LSTEXPORTDLG ( HWND hwnd, USHORT msg, MPARAM mp1,        |
//|                                  MPARAM mp2 )                              |
//+----------------------------------------------------------------------------+
//|Description:      Dialog procedure for the export of lists dialog.          |
//+----------------------------------------------------------------------------+
//|Input parameter:  HWND    hwnd     handle of window                         |
//|                  USHORT  msg      type of message                          |
//|                  MPARAM  mp1      first message parameter                  |
//|                  MPARAM  mp2      second message parameter                 |
//+----------------------------------------------------------------------------+
//|Returncode type:  MRESULT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:      depends on message type                                   |
//|                  normal return codes are:                                  |
//|                  TRUE  = message has been processed                        |
//|                  FALSE = message has not been processed                    |
//+----------------------------------------------------------------------------+
//|Function flow:     switch msg;                                              |
//|                     case WM_INITDLG:                                       |
//|                       call LstExpInit to initialize the dialog             |
//|                     case WM_EQF_CLOSE                                          |
//|                       dismiss the dialog                                   |
//|                     case WM_DESTROY                                        |
//|                       free the memory allocated for the IDA                |
//|                     case WM_CONTROL                                        |
//|                       switch control id                                    |
//|                         case MAT format radiobutton                        |
//|                           hide name static, name entry field               |
//|                           hide current directory static                    |
//|                           hide directory static and listbox                |
//|                           simulate selecting of the current drive button   |
//|                           set mode to export in MAT format                 |
//|                         case external radiobutton                          |
//|                         case user format radiobutton                       |
//|                           show name static, name entry field               |
//|                           show current directory static                    |
//|                           show directory static and listbox                |
//|                           simulate selecting of the current drive button   |
//|                           set mode to export in the selected format        |
//|                         case drive button                                  |
//|                           if dialog is in export in MAT format then        |
//|                             deselect the old drive button                  |
//|                             select the new one                             |
//|                           else                                             |
//|                             call UtlWMControls to process the message      |
//|                           endif                                            |
//|                         default:                                           |
//|                           call UtlWMControls to process the message        |
//|                       endswitch                                            |
//|                     case WM_CHAR:                                          |
//|                       if VK_ENTER or VK_NEWLINE then                       |
//|                         call UtlWMChar                                     |
//|                       else                                                 |
//|                         call default dialog procedure                      |
//|                       endif                                                |
//|                     case WM_COMMAND:                                       |
//|                       call LstExpCommand to process the message            |
//|                     default:                                               |
//|                       call default dialog procedure                        |
//|                   endswitch;                                               |
//+----------------------------------------------------------------------------+
INT_PTR CALLBACK LSTEXPORTDLG
(
   HWND   hwnd,
   WINMSG msg,
   WPARAM mp1,
   LPARAM mp2
)
{
  MRESULT      mResult = TRUE;         // message processing return code
  PLSTEXPIDA   pIda;                   // pointer to export instance data area

  switch ( msg )
  {
    case WM_EQF_QUERYID:
      pIda = ACCESSDLGIDA( hwnd, PLSTEXPIDA );
      switch ( pIda->usListType )
      {
        case NTL_TYPE:
          HANDLEQUERYID( ID_LISTEXP_NTL_DLG, mp2 );
          break;
        case FTL_TYPE:
          HANDLEQUERYID( ID_LISTEXP_FTL_DLG, mp2 );
          break;
        case EXCL_TYPE:
          HANDLEQUERYID( ID_LISTEXP_EXCL_DLG, mp2 );
          break;
        case NOISE_TYPE:
          HANDLEQUERYID( ID_LISTEXP_NOIS_DLG, mp2 );
          break;
        default:
          break;
      } /* endswitch */
      break;

    case WM_INITDLG:
      mResult = LstExpInit( hwnd, mp1, mp2 );
      break;

    case WM_EQF_CLOSE :
      WinDismissDlg( hwnd, 0 );
      break;

    case WM_DESTROY:
      pIda = ACCESSDLGIDA( hwnd, PLSTEXPIDA );
      UtlAlloc( (PVOID *) &pIda, 0L, 0L, NOMSG );
      mResult = FALSE;
      break;

    case DM_GETDEFID:
        pIda = ACCESSDLGIDA( hwnd, PLSTEXPIDA );
       mResult = UtlDMGETDEFID( hwnd, mp1, mp2, &pIda->Controls );
       break;

    case WM_COMMAND :
      mResult = LstExpCommand( hwnd, WMCOMMANDID( mp1, mp2 ),
                               WMCOMMANDCMD( mp1, mp2 ));
      break;

    default :
      mResult = WinDefDlgProc( hwnd, msg, mp1, mp2 );
  } /* endswitch */

  return( mResult );
} /* end of function LSTEXPORTDLG */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     LstExpInit                                               |
//+----------------------------------------------------------------------------+
//|Function call:     LstExpInit( HWND hwnd, MPARAM mp2, MPARAM mp2 );         |
//+----------------------------------------------------------------------------+
//|Description:       Initialization code for the list export dialog.          |
//+----------------------------------------------------------------------------+
//|Input parameter:   HWND   hwnd      Dialog window handle                    |
//|                   MPARAM mp1       First message parameter of WM_INITDLG   |
//|                                    message: dialog window handle           |
//|                   MPARAM mp2       Second message parameter of WM_INITDLG  |
//|                                    message: ptr to object name of list     |
//|                                    being exported                          |
//+----------------------------------------------------------------------------+
//|Returncode type:   MRESULT                                                  |
//+----------------------------------------------------------------------------+
//|Returncodes:       FALSE  (always)                                          |
//+----------------------------------------------------------------------------+
//|Side effects:      The dialog is removed using WinDismissDlg if severe      |
//|                   errors occur during initialization                       |
//+----------------------------------------------------------------------------+
//|Function flow:     allocate and anchor IDA                                  |
//|                   look for userexit DLL and check entry points             |
//|                   if found then                                            |
//|                     enable user format radio button                        |
//|                   else                                                     |
//|                     disable user format radio button                       |
//|                   end                                                      |
//|                   store list name and type in dialog IDA                   |
//|                   get and apply last used values                           |
//|                   store control IDs and initial values in Controls IDA     |
//|                   call dialog utility to do the rest of the initialization |
//|                   if initialization failed then                            |
//|                     dismiss the dialog                                     |
//|                   else                                                     |
//|                     set focus to currently active format radio button      |
//|                   endif                                                    |
//+----------------------------------------------------------------------------+
static MRESULT LstExpInit( HWND hwnd, WPARAM mp1, LPARAM mp2 )
{
   PLSTEXPIDA  pIda;                   // instance data area of list export
   BOOL        fOK = TRUE;             // internal OK flag
   HPROP       hProp = NULL;           // properties handler
   EQFINFO     ErrorInfo;              // error info from property handler calls
   PSZ         pszTemp;                // temporary pointer
   MRESULT     mResult = MRFROMSHORT(TRUE); // result of message processing
   PPROPLIST   pProp;                  // pointer to list properties
   USHORT      usID = 0;               // general buffer for ID values

   mp1;                                // get rid of compiler warnings
   /*******************************************************************/
   /* Allocate and anchor IDA                                         */
   /*******************************************************************/
   fOK = UtlAlloc( (PVOID *) &pIda, 0L, (LONG) sizeof(LSTEXPIDA), ERROR_STORAGE );
   if ( fOK )
   {
      ANCHORDLGIDA( hwnd, pIda );
   } /* endif */

   /*******************************************************************/
   /* Store list object name and list type in IDA (list type is taken */
   /* from the extension of the list object name)                     */
   /*******************************************************************/
   if ( fOK )
   {
     strcpy( pIda->szListName, (PSZ) mp2 );
     pIda->usListType = LstGetListTypeFromName( pIda->szListName );
     if ( pIda->usListType == UNKNOWN_TYPE )
     {
       /***************************************************************/
       /* Internal Error: List object name is corrupt or list is no   */
       /* list processed by the built-in list processors.             */
       /***************************************************************/
       UtlError( ERROR_INTERNAL, MB_CANCEL, 0, NULL, INTERNAL_ERROR );
       fOK = FALSE;
     } /* endif */
   } /* endif */

   /*******************************************************************/
   /* Set title bar text and dialog ID                                */
   /*******************************************************************/
   if ( fOK )
   {
     HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
     switch ( pIda->usListType )
     {
       case NTL_TYPE :
         usID = SID_LSTEXP_NTL_TITLE_TEXT;
         SETWINDOWID( hwnd, ID_LISTEXP_NTL_DLG );
         break;
       case FTL_TYPE :
         usID = SID_LSTEXP_FTL_TITLE_TEXT;
         SETWINDOWID( hwnd, ID_LISTEXP_FTL_DLG );
         break;
       case EXCL_TYPE :
         usID = SID_LSTEXP_EXCL_TITLE_TEXT;
         SETWINDOWID( hwnd, ID_LISTEXP_EXCL_DLG );
         break;
       case NOISE_TYPE :
         usID = SID_LSTEXP_NOISE_TITLE_TEXT;
         SETWINDOWID( hwnd, ID_LISTEXP_NOIS_DLG );
         break;
     } /* endswitch */
     LOADSTRING( NULLHANDLE, hResMod, usID, pIda->szString );
     SETTEXTHWND( hwnd, pIda->szString );
   } /* endif */

   /*******************************************************************/
   /* Set list name field                                             */
   /*******************************************************************/
   if ( fOK )
   {
     Utlstrccpy( pIda->szString, UtlGetFnameFromPath( pIda->szListName ), DOT );
     SETTEXT( hwnd, ID_LISTEXP_EXPORT_EF, pIda->szString );
   } /* endif */

   if ( fOK )
   {
     ENABLECTRL( hwnd, ID_LISTEXP_USER_RB, FALSE );
   } /* endif */

   /*******************************************************************/
   /* Get/apply last used values                                      */
   /*******************************************************************/
   if ( fOK )
   {
      pIda->usFormat = SGMLFORMAT_ASCII; // set default value

     /*****************************************************************/
     /* Open list handler properties                                  */
     /*****************************************************************/
     UtlMakeEQFPath( pIda->szPropName, NULC, SYSTEM_PATH, NULL );
     strcat( pIda->szPropName, BACKSLASH_STR );
     strcat( pIda->szPropName, LISTHANDLER_PROPERTIES_NAME );
     hProp = OpenProperties( pIda->szPropName, NULL, PROP_ACCESS_READ,
                             &ErrorInfo);
     if ( !hProp )
     {
       pszTemp = pIda->szPropName;
       UtlError( ERROR_OPEN_PROPERTIES, MB_CANCEL, 1, &pszTemp, EQF_ERROR );
       fOK = FALSE;
     } /* endif */

     /*****************************************************************/
     /* Get last used values or use defaults if none set              */
     /*****************************************************************/
     if ( fOK )
     {
       pProp = (PPROPLIST)MakePropPtrFromHnd( hProp );
       if ( pProp->usExpFormat ) pIda->usFormat = pProp->usExpFormat;
       pIda->Controls.chSavedDrive = pProp->chListExpDrive;
       strcpy( pIda->Controls.szSavedPath, pProp->szListExpPath );
       if ( pProp->usListExpFormatID )
       {
         pIda->usFormatID = pProp->usListExpFormatID;
       }
       else
       {
         pIda->usFormatID = ID_LISTEXP_EXTERNAL_RB;
       } /* endif */
       pIda->Controls.usSavedFormat = 0;
     } /* endif */

     /*****************************************************************/
     /* Close list properties                                         */
     /*****************************************************************/
     if ( hProp )
     {
       CloseProperties( hProp, PROP_QUIT, &ErrorInfo );
     } /* endif */
   } /* endif */

   /*******************************************************************/
   /* Store IDs of dialog controls in Controls IDA                    */
   /*******************************************************************/
   if ( fOK )
   {
     {
       PSZ pszFormatList = SGML_FORMAT_FILTERS;
       USHORT usIndex = 1;
       while ( pszFormatList[0] != EOS )
       {
           SHORT sItem = CBINSERTITEM( hwnd, ID_LISTEXP_FORMAT_CB, pszFormatList );
           if ( sItem != LIT_NONE )
           {
             CBSETITEMHANDLE( hwnd, ID_LISTEXP_FORMAT_CB, sItem, usIndex );
             if ( usIndex == pIda->usFormat )
             {
               CBSELECTITEM( hwnd, ID_LISTEXP_FORMAT_CB, sItem );
             } /* endif */
           } /* endif */
         pszFormatList += strlen(pszFormatList) + 1;   // skip filter name
         pszFormatList += strlen(pszFormatList) + 1;   // skip filter extention
         usIndex++;
       } /* endwhile */
     }

     pIda->Controls.idDirLB      = ID_LISTEXP_DIR_LB;
     pIda->Controls.idPathEF     = ID_LISTEXP_NAME_EF;
     pIda->Controls.idCurrentDirectoryEF = ID_LISTEXP_CURDIR_EF;
     pIda->Controls.idInternalRB = ID_LISTEXP_USER_RB;
     pIda->Controls.idExternalRB = ID_LISTEXP_EXTERNAL_RB;
     pIda->Controls.idDriveBTN   = ID_LISTEXP_DUMMY;
     pIda->Controls.idControlsGB = ID_LISTEXP_TO_GB;
     pIda->Controls.idOkPB       = ID_LISTEXP_EXPORT_PB;
     pIda->Controls.idExportTEXT = ID_LISTEXP_EXPORT_EF;   //dict exported
     strcpy( pIda->Controls.szExt, EXT_OF_EXTERNAL_LIST );
     switch ( pIda->usListType )
     {
       case NTL_TYPE :
         strcpy( pIda->Controls.szExt, EXT_OF_NEWTERMS_LIST );
         break;
       case FTL_TYPE :
         strcpy( pIda->Controls.szExt, EXT_OF_FOUNDTERMS_LIST );
         break;
       default:
         strcpy( pIda->Controls.szExt, EXT_OF_EXTERNAL_LIST );
         break;
     } /* endswitch */
     Utlstrccpy( pIda->Controls.szSelectedName,
                 UtlGetFnameFromPath( pIda->szListName ), DOT );
     pIda->Controls.fImport      = FALSE;
   } /* endif */

   /*******************************************************************/
   /* Call dialog utility to do the rest of the initialization        */
   /*******************************************************************/
   if ( fOK )
   {
     UtlControlsInit( hwnd, &pIda->Controls );
   } /* endif */

   /*******************************************************************/
   /* Remove dialog if initialization failed or set focus to          */
   /* currently active format radio button and 'click' it             */
   /*******************************************************************/
   if ( !fOK )
   {
     WinDismissDlg( hwnd, FALSE );
   }
   else
   {
     pIda->fInitInProgress = TRUE;
     CLICK( hwnd, pIda->usFormatID );
     pIda->fInitInProgress = FALSE;
   } /* endif */

   return( mResult );
} /* end of function LstExpInit */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     LstExpCommand                                            |
//+----------------------------------------------------------------------------+
//|Function call:     LstExpCommand( HWND hwnd, MPARAM mp2, MPARAM mp2 );      |
//+----------------------------------------------------------------------------+
//|Description:       Process the WM_COMMAND message of the list export dialog |
//|                   and do the actual exporting of the list.                 |
//+----------------------------------------------------------------------------+
//|Input parameter:   HWND   hwnd      Dialog window handle                    |
//|                   MPARAM mp1       First message parameter of WM_INITDLG   |
//|                                    message: dialog window handle           |
//|                   MPARAM mp2       Second message parameter of WM_INITDLG  |
//|                                    message: ptr to object name of list     |
//|                                    being exported                          |
//+----------------------------------------------------------------------------+
//|Returncode type:   MRESULT                                                  |
//+----------------------------------------------------------------------------+
//|Returncodes:       FALSE  (always)                                          |
//+----------------------------------------------------------------------------+
//|Side effects:      The list is exported if 'export' has been pressed. The   |
//|                   dialog is ended on Cancel or successful export.          |
//+----------------------------------------------------------------------------+
//|Function flow:     switch control ID                                        |
//|                     case CANCEL button:                                    |
//|                     case escape key:                                       |
//|                       post a WM_EQF_CLOSE message to the dialog                |
//|                     case export button:                                    |
//|                       address IDA                                          |
//|                       if not in MAT format export then                     |
//|                         get and check specified file name                  |
//|                       endif                                                |
//|                       if ok and not in MAT format export                   |
//|                         build and check name of export file against EQF    |
//|                          directories, syntax and handle file exists        |
//|                          condition                                         |
//|                       endif                                                |
//|                       if ok then                                           |
//|                         check if target drive has enough free space to     |
//|                          receive the exported file                         |
//|                       endif                                                |
//|                       if ok then                                           |
//|                         call LstExportList to do the actual export         |
//|                       endif                                                |
//|                       if ok then                                           |
//|                         close the dialog and save last used values         |
//|                       endif                                                |
//|                   endswitch                                                |
//+----------------------------------------------------------------------------+
static MRESULT LstExpCommand
(
   HWND hwnd,
   SHORT sId,                          // id of button
   SHORT sNotification                 // notification type
)
{
   PLSTEXPIDA  pIda = NULL;            // list export IDA
   USHORT      usRC;                   // Return code from Dos functions
   MRESULT     mResult = (MRESULT)FALSE; // function return value
   BOOL        fOK = TRUE;             // internal OK flag
   PSZ         pszError[2];            // parameter for UtlError calls
   ULONG       ulSpace;                // free space on target disk
   EQFINFO     ErrorInfo;              // error info of property handler calls
   PPROPLIST   pProp;                  // pointer to list handler properties
   HPROP       hProp;                  // handle of list handler properties
   CHAR        szList[MAX_FNAME];      // buffer for list name

   sNotification;                      // get rid of compiler warnings

   switch ( sId )
   {
	 case ID_LISTEXP_HELP_PB:
	   mResult = UtlInvokeHelp();
	   break;
     case ID_LISTEXP_CANCEL_PB :       // CANCEL button selected
     case DID_CANCEL :                 // ESC key pressed
       WinPostMsg( hwnd, WM_EQF_CLOSE, NULL, NULL );
       break;

     case ID_LISTEXP_EXPORT_PB :
       pIda = ACCESSDLGIDA( hwnd, PLSTEXPIDA );

       // validate entry field contents
       pIda->Controls.fCommand = TRUE;
       fOK = UtlEFValidityTest( &(pIda->Controls), hwnd );
       pIda->Controls.fCommand = FALSE;

       if ( fOK )
       {
        /***************************************************************/
        /* Get current path and drive settings from controls IDA       */
        /***************************************************************/
        strcpy( pIda->szExportPath,  pIda->Controls.szPath );
        strcpy( pIda->szExportDrive, pIda->Controls.szDrive );

        /***************************************************************/
        /* Build name of target file for export                        */
        /***************************************************************/

        pIda->usFormatID = ID_LISTEXP_EXTERNAL_RB;
        {
          SHORT sItem = CBQUERYSELECTION( hwnd, ID_LISTEXP_FORMAT_CB );
          if ( sItem >= 0 )
          {
            pIda->usFormat = (USHORT)CBQUERYITEMHANDLE( hwnd, ID_LISTEXP_FORMAT_CB, sItem );
          }
          else
          {
            // use default mode
            pIda->usFormat = SGMLFORMAT_ASCII;
          } /* endif */
        }


        if ( pIda->usFormatID != ID_LISTEXP_MAT_RB )
        {
          /*************************************************************/
          /* For non-MAT format exports use specified name as          */
          /* export file                                               */
          /*************************************************************/
          QUERYTEXT( hwnd, ID_LISTEXP_NAME_EF, pIda->szString );

          if ( pIda->szString[0] == NULC && fOK )
          {
              /**********************************************************/
              /* Display error message that no filename given           */
              /**********************************************************/
              UtlError( ERROR_NO_FILE_NAME, MB_CANCEL,
                        0, NULL, EQF_WARNING );
              SETFOCUS( hwnd, ID_LISTEXP_NAME_EF );
              fOK = FALSE;
          }
          else
          {
              /**********************************************************/
              /* Add path to filename to create fully qualified path    */
              /**********************************************************/
              strupr( pIda->szString );
              sprintf( pIda->szExportName, "%s%s",
                      pIda->szExportPath,
                      pIda->szString );
          } /* endif */
        }
        else
        {
          /*************************************************************/
          /* Build name for MAT format exports = export path + list    */
          /* name + export extension of list                           */
          /*************************************************************/
          UtlMakeEQFPath( pIda->szExportName, pIda->Controls.szDrive[0],
                          EXPORT_PATH, NULL );
          strcat( pIda->szExportName, BACKSLASH_STR );
          Utlstrccpy( pIda->szExportName + strlen(pIda->szExportName),
                      UtlGetFnameFromPath( pIda->szListName ), DOT );
          switch ( pIda->usListType )
          {
            case NTL_TYPE :
              strcat( pIda->szExportName, EXPORT_NTL_EXT );
              break;
            case FTL_TYPE :
              strcat( pIda->szExportName, EXPORT_FTL_EXT );
              break;
            case EXCL_TYPE :
              strcat( pIda->szExportName, EXPORT_EXCL_EXT );
              break;
            case NOISE_TYPE :
              strcat( pIda->szExportName, EXPORT_NOISE_EXT);
              break;
          } /* endswitch */
        } /* endif */

        /***************************************************************/
        /* Check that file does not collidate with MAT files =         */
        /* check that file is not exported to MAT system directories   */
        /***************************************************************/
        if ( pIda->usFormatID != ID_LISTEXP_MAT_RB )
        {
          if ( fOK )
          {

              /**********************************************************/
              /* Check that nothing is copied onto system directories   */
              /**********************************************************/
              UtlMakeEQFPath( pIda->szString, NULC, SYSTEM_PATH, (PSZ)NULP );
              strcat( pIda->szString, BACKSLASH_STR );
              if ( !strncmp( pIda->szString+2, pIda->szExportName+2,
                            strlen( pIda->szString ) - 2 ) )
              {
                pszError[0] = pIda->szExportName;
                UtlError( ERROR_EQF_PATH_INVALID, MB_CANCEL,
                          1, pszError, EQF_ERROR );
                fOK = FALSE;
                SETFOCUS( hwnd, ID_LISTEXP_NAME_EF );
              } /* endif */
          } /* endif */
        } /* endif */
       } /* endif */

       /***************************************************************/
       /* Check if targte file exists and get user confirmation for   */
       /* overwrite of existing files                                 */
       /***************************************************************/
       if ( fOK )
       {
         if ( UtlFileExist( pIda->szExportName) )
         {
           /*******************************************************/
           /* Display message saying file will be overwritten     */
           /*******************************************************/
           pszError[0] = pIda->szExportName;
           usRC = UtlError( FILE_EXISTS, MB_YESNO, 1, pszError,
                            EQF_WARNING );
           if ( usRC == MBID_NO )
           {
              fOK = FALSE;
              SETFOCUS( hwnd, ID_LISTEXP_NAME_EF );
           }
           else
           {
             usRC = UtlDelete( pIda->szExportName, 0L, TRUE );
             if ( usRC != NO_ERROR )
             {
               fOK = FALSE;
             } /* endif */
           } /* endif */
         } /* endif */
       } /* endif */

    /******************************************************************/
    /* Check if there is enough room left on the target drive.        */
    /*   The check is performed on the size of the source file * 0.8. */
    /*   This check is not accurate as the target file will be        */
    /*   smaller in most cases. But as it is not possible to compute  */
    /*   the size of the target file without exporting it, the check  */
    /*   seems to be accurate enough.                                 */
    /******************************************************************/
    if ( fOK )
    {
      pszError[0] = pIda->szExportDrive;
      fOK = UtlCheckSpaceForFile( pIda->szListName,
                                  80,                  // factor is 80%
                                  0L,                  // no explicit size
                                  pszError,            // target drive
                                  (PLONG)&ulSpace,     // missing space
                                  TRUE );              // message handle flag

      if ( fOK && ( ulSpace != 0L ) )        // more space needed???
      {
        CHAR szListName[MAX_FILESPEC];

        Utlstrccpy( szListName, UtlGetFnameFromPath( pIda->szListName ), DOT );
        pszError[1] = szListName;
        UtlError( ERROR_LSTEXP_NOSPACE, MB_CANCEL, 2, pszError,
                  EQF_ERROR );
        fOK = FALSE;
      } /* endif */
    } /* endif */

    /******************************************************************/
    /* Do the actual export of the list                               */
    /******************************************************************/
    if ( fOK )
    {
      if ( LstExportList( pIda->szListName, pIda->usListType,
                          pIda->usFormatID, pIda->szExportName,
                          pIda->usFormat ) == 0 )

      {

        Utlstrccpy( szList, UtlGetFnameFromPath( pIda->szListName ), DOT );
        pszError[0] = szList;
        if ( pIda->usFormatID == ID_LISTEXP_MAT_RB )
        {
          pszError[1] = pIda->szExportName;
          pIda->szExportName[2] = EOS;
          UtlError( INFO_LST_EXPORTED_INT, MB_OK, 2, pszError, EQF_INFO );
        }
        else
        {
          pszError[1] = pIda->szExportName;
          UtlError( INFO_LST_EXPORTED_EXT, MB_OK, 2, pszError, EQF_INFO );
        } /* endif */
      }
      else
      {
        fOK = FALSE;
      } /* endif */
    } /* endif */

    // Close dialog and save last used values
    if ( fOK )
    {
      hProp = OpenProperties( pIda->szPropName, NULL, PROP_ACCESS_READ,
                                 &ErrorInfo);
      if ( !hProp )
      {
        pszError[0] = pIda->szPropName;
        UtlError( ERROR_OPEN_PROPERTIES, MB_CANCEL, 1, pszError, EQF_ERROR );
        fOK = FALSE;
      } /* endif */

      if ( fOK )
      {
        if( SetPropAccess( hProp, PROP_ACCESS_WRITE))
        {
          pProp = (PPROPLIST)MakePropPtrFromHnd( hProp);
          if ( pIda->usFormatID != ID_LISTEXP_MAT_RB )
          {
            strcpy( pProp->szListExpPath, pIda->szExportName );
          } /* endif */
          pProp->chListExpDrive = pIda->szExportDrive[0];
          pProp->usListExpFormatID = pIda->usFormatID;
          pProp->usExpFormat = pIda->usFormat;
          SaveProperties( hProp, &ErrorInfo);
          CloseProperties( hProp, PROP_QUIT, &ErrorInfo );
        } /* endif */
      } /* endif */

      WinPostMsg( hwnd, WM_EQF_CLOSE, MP1FROMSHORT( fOK ), NULL );
    } /* endif */
    break;
  default :
     mResult = LstExpControl( hwnd, sId, sNotification );
   } /* endswitch */

   return mResult;
}

MRESULT LstExpControl
(
   HWND   hwnd,                        // dialog handle
   SHORT  sId,                         // id in action
   SHORT  sNotification                // notification
)
{
   PLSTEXPIDA  pIda = NULL;            // list export IDA
  USHORT       usDrive;                // drive ID

  /****************************************************************/
  /* Get access to ida                                            */
  /****************************************************************/
  pIda = ACCESSDLGIDA( hwnd, PLSTEXPIDA );

  /****************************************************************/
  /* Filter messages from format radio buttons as they are        */
  /* currently not handled correctly in the dialog utility        */
  /****************************************************************/
  switch ( sId )
  {
    case ID_LISTEXP_EXTERNAL_RB :
    case ID_LISTEXP_USER_RB :
      if ( QUERYCHECK( hwnd, sId ) )
      {
        if ( (pIda->usFormatID == ID_LISTEXP_MAT_RB) ||
             pIda->fInitInProgress )
        {
          /********************************************************/
          /* Show name, current directory, directory              */
          /********************************************************/
          SHOWCONTROL( hwnd, ID_LISTEXP_NAME_TEXT );
          SHOWCONTROL( hwnd, ID_LISTEXP_NAME_EF );
          SHOWCONTROL( hwnd, ID_LISTEXP_CURDIR_EF );
          SHOWCONTROL( hwnd, ID_LISTEXP_CURDIR_TEXT );
          SHOWCONTROL( hwnd, ID_LISTEXP_DIR_TEXT );
          SHOWCONTROL( hwnd, ID_LISTEXP_DIR_LB );

          /********************************************************/
          /* Force a refresh of the list boxes                    */
          /********************************************************/
          if ( (pIda->Controls.szDrive[0] != EOS) &&
               !pIda->fInitInProgress )
          {
             usDrive = IDFROMDRIVE( PID_DRIVEBUTTON_A,
                                    pIda->Controls.szDrive[0]);
             pIda->Controls.szDrive[0] += 1;
             SENDNOTIFICATION( hwnd, usDrive, BN_CLICKED );
          } /* endif */
        } /* endif */
        pIda->usFormatID = sId;
      } /* endif */
      break;

    case ID_LISTEXP_MAT_RB :
      if ( QUERYCHECK( hwnd, sId ) )
      {
        if ( (pIda->usFormatID != ID_LISTEXP_MAT_RB) ||
             pIda->fInitInProgress )
        {

          /********************************************************/
          /* Hide name, current directory, directory and file     */
          /* control                                              */
          /********************************************************/
          HIDECONTROL( hwnd, ID_LISTEXP_NAME_TEXT );
          HIDECONTROL( hwnd, ID_LISTEXP_NAME_EF );
          HIDECONTROL( hwnd, ID_LISTEXP_CURDIR_EF );
          HIDECONTROL( hwnd, ID_LISTEXP_CURDIR_TEXT );
          HIDECONTROL( hwnd, ID_LISTEXP_DIR_TEXT );
          HIDECONTROL( hwnd, ID_LISTEXP_DIR_LB );

          /********************************************************/
          /* Force a refresh of the listboxes                     */
          /********************************************************/
          if ( pIda->Controls.szDrive[0] != EOS )
          {
             usDrive = IDFROMDRIVE( PID_DRIVEBUTTON_A,
                                    pIda->Controls.szDrive[0]);
             pIda->Controls.szDrive[0] += 1;
             SENDNOTIFICATION( hwnd, usDrive, BN_CLICKED );
          } /* endif */

        } /* endif */
        pIda->usFormatID = sId;
      } /* endif */
      break;

    default :
      if ( pIda->usFormatID == ID_LISTEXP_MAT_RB )
      {
        /**********************************************************/
        /* Handle control message within dialog as dialog utility */
        /* can not handle internal formats                        */
        /**********************************************************/
        if ( ( sNotification == BN_CLICKED )          &&
             ( sId >= PID_DRIVEBUTTON_A ) && ( sId <= PID_DRIVEBUTTON_Z ) )
        {
          if ( pIda->Controls.szDrive[0] != EOS )
          {
             usDrive = IDFROMDRIVE( PID_DRIVEBUTTON_A,
                                       pIda->Controls.szDrive[0]);
             SETDRIVE( hwnd, usDrive, FALSE );
          }
          else
          {
            usDrive = 0;
          } /* endif */

          /**********************************************************/
          /* Save newly selected drive character in IDA and select  */
          /* it                                                     */
          /**********************************************************/
          pIda->Controls.szDrive[0] = DRIVEFROMID( PID_DRIVEBUTTON_A, sId );
          SETDRIVE( hwnd, sId, TRUE );

        } /* endif */
      }
      else
      {
        UtlWMControls( hwnd, WM_CONTROL, sId, sNotification, &pIda->Controls );
      } /* endif */

      break;
  } /* endswitch */
  return( MRFROMSHORT(FALSE) );
} /* end of function LstExpControl */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     LstExportList       Export a list                        |
//+----------------------------------------------------------------------------+
//|Function call:     LstExportList( PSZ pszListName, USHORT usListType,       |
//|                                  USHORT usFormatID, PSZ pszExportFile );   |
//+----------------------------------------------------------------------------+
//|Description:       Exports a list in the selected format                    |
//+----------------------------------------------------------------------------+
//|Input parameter:   PSZ    pszListName    fully qualified list name          |
//|                   USHORT usListType     identifier for type of list        |
//|                   USHORT usFormaID      identifier for format of export    |
//|                   PSZ    pszExportFile  file name of export file           |
//|                   PUSEREXIT pUserExit   ptr to structure containing user   |
//|                                         exit entry points or NULL if not   |
//|                                         exporting in user format           |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       0       function completed successfully                  |
//|                   other   return code of called DOS functions              |
//+----------------------------------------------------------------------------+
//|Function flow:     switch type of list                                      |
//|                     case new terms list                                    |
//|                     case found terms list                                  |
//|                       switch format of export                              |
//|                         case MAT format:                                   |
//|                           create export directory if required              |
//|                           copy the list file to the export file spec.      |
//|                         case external format:                              |
//|                           read the list using LstReadSGMLList              |
//|                           write the list using LstWriteSGMLList with the   |
//|                            external flag set                               |
//|                           free list areas                                  |
//|                         case user format:                                  |
//|                           read the list using LstReadSGMLList              |
//|                           call the export init entry point of user exit    |
//|                           while ok and not end of terms list               |
//|                             pass term and term data to term export entry   |
//|                              point of user exit                            |
//|                           endwhile                                         |
//|                           call the export end entry point of user exit     |
//|                           free list areas                                  |
//|                     case noise list                                        |
//|                     case exclusion list                                    |
//|                       switch format of export                              |
//|                         case MAT format:                                   |
//|                           create export directory if required              |
//|                           copy the list file to the export file spec.      |
//|                         case external format:                              |
//|                           read the list using UtlLoadFile                  |
//|                           open the output file                             |
//|                           while not end of list                            |
//|                             write term to flat ASCII file                  |
//|                           endwhile                                         |
//|                           close the output file                            |
//|                           free list buffer                                 |
//|                         case user format:                                  |
//|                           read the list using UtlLoadFile                  |
//|                           call the export init entry point of user exit    |
//|                           while ok and not end of terms                    |
//|                             pass term and term data to term export entry   |
//|                              point of user exit                            |
//|                           endwhile                                         |
//|                           call the export end entry point of user exit     |
//|                           free list buffer                                 |
//|                   endswitch                                                |
//+----------------------------------------------------------------------------+
USHORT LstExportList
(
  PSZ        pszListName,              // fully qualified list name
  USHORT     usListType,               // identifier for type of list
  USHORT     usFormatID,               // identifier for format of export
  PSZ        pszExportFile,            // file name of export file
  USHORT     usFormat                  // format of export (ANSI,ASCII,...)
)
{
  USHORT        usRC = 0;              // function return code
  CHAR          szEQFPath[MAX_EQF_PATH]; // buffer for EQF path names
  LISTHEADER    ListHead;              // header for NTL+FTL lists
  PCONTEXTTABLE pContextTable= NULL;   // context table for NTL+FTL lists
  PTERMTABLE    pTermTable   = NULL;   // term table for NTL+FTL lists
  PPOOL         pPool        = NULL;   // string pool for NTL+FTL lists
  PBUFCB        pBufCB       = NULL;   // ptr to CB for buffered IO
  PEXCLUSIONLIST pExclList   = NULL;   // pointer to loaded exclusion lists
  PUSHORT       pusTermInd;            // pointer to term index of excl. lists
  CHAR_W        szCRLF[3];             // buffer for carriage return/linefeed
  USHORT        usI;                   // general loop index
  PSZ_W         pszTemp;               // temporary term text pointer

  switch ( usListType )
  {
    case NTL_TYPE :
    case FTL_TYPE :
      switch ( usFormatID )
      {
        case ID_LISTEXP_MAT_RB :
          /************************************************************/
          /* Create export directory if required                      */
          /************************************************************/
          UtlMakeEQFPath( szEQFPath, pszExportFile[0], EXPORT_PATH, NULL );
          usRC = UtlMkMultDir( szEQFPath, TRUE );

          /************************************************************/
          /* Copy the list file to the target file                    */
          /************************************************************/
          if ( !usRC )
          {
            UtlCopy( pszListName, pszExportFile, 1, 0L, TRUE );
          } /* endif */
          break;

        case ID_LISTEXP_EXTERNAL_RB :
          memset( &ListHead, 0, sizeof(ListHead) );
          usRC = LstReadSGMLList( pszListName, &ListHead,
                                  &pTermTable, &pContextTable,
                                  &pPool, FALSE, (LISTTYPES)usListType, SGMLFORMAT_UNICODE );
          if ( !usRC )
          {
            usRC = LstWriteSGMLList( pszExportFile,
                                     &ListHead,
                                     pTermTable,
                                     pContextTable,
                                     TRUE,
                                     (LISTTYPES)usListType, usFormat );
          } /* endif */

          if ( pTermTable )    LstDestroyTermTable( pTermTable );
          if ( pContextTable ) LstDestroyContextTable( pContextTable );
          if ( pPool )         PoolDestroy( pPool );

          break;
      } /* endswitch */
      break;

    case  EXCL_TYPE :
    case  NOISE_TYPE :
      switch ( usFormatID )
      {
        case ID_LISTEXP_MAT_RB :
          /************************************************************/
          /* Create export directory if required                      */
          /************************************************************/
          UtlMakeEQFPath( szEQFPath, pszExportFile[0], EXPORT_PATH, NULL );
          usRC = UtlMkMultDir( szEQFPath, TRUE );

          /************************************************************/
          /* Copy the list file to the target file                    */
          /************************************************************/
          if ( !usRC )
          {
            UtlCopy( pszListName, pszExportFile, 1, 0L, TRUE );
          } /* endif */
          break;

        case ID_LISTEXP_EXTERNAL_RB :
          { ULONG  ulLength = 0L;
            ULONG  ulFormatCP = 0L;
            ULONG  ulAnsiCP = 0L;
            pExclList = NULL;
            szCRLF[0] = CR;
            szCRLF[1] = LF;
            szCRLF[2] = EOS;

            usRC = LstReadNoiseExclList( pszListName, &ulLength, &pExclList );

  //          if ( !UtlLoadFile( pszListName, (PVOID *)&pExclList, &usI, FALSE, TRUE ) )
  //          {
  //            usRC = ERROR_INVALID_FUNCTION;  // set return code to non-zero value
  //          } /* endif */

            if ( !usRC )
            {
              usRC = UtlBufOpen( &pBufCB, pszExportFile, IO_BUFFER_SIZE,
                                 FILE_CREATE, TRUE  );
            } /* endif */

            //if usFormat is Unicode, write UNICODEFILEPREFIX first
            if (usFormat == SGMLFORMAT_UNICODE )
            {

              usRC = UtlBufWrite( pBufCB, UNICODEFILEPREFIX,
                                 (SHORT)strlen(UNICODEFILEPREFIX), TRUE );
            }
            if ((usFormat == SGMLFORMAT_ASCII) || (usFormat == SGMLFORMAT_ANSI) )
            {
              ulFormatCP = GetLangOEMCP(NULL);
              ulAnsiCP = GetLangAnsiCP(NULL);
            }

            if ( !usRC )
            {
              pusTermInd = (PUSHORT) (((PSZ)pExclList) + pExclList->uFirstEntry);
              for ( usI = 0; !usRC, usI < pExclList->usNumEntries; usI++ )
              {
                 pszTemp = (PSZ_W)((PBYTE)pExclList + pExclList->uStrings) + *pusTermInd;
                 usRC = UtlBufWriteConv( pBufCB, pszTemp, (SHORT)UTF16strlenBYTE(pszTemp), TRUE, usFormat,
                 						ulFormatCP, ulAnsiCP);
                 if ( !usRC )
                 {
                   usRC = UtlBufWriteConv( pBufCB, szCRLF, (SHORT)UTF16strlenBYTE(szCRLF), TRUE, usFormat,
                                            ulFormatCP, ulAnsiCP );
                 } /* endif */
                 pusTermInd++;
              } /* endfor */
            } /* endif */

            /************************************************************/
            /* cleanup                                                  */
            /************************************************************/
            if ( pExclList ) UtlAlloc( (PVOID *) &pExclList, 0L, 0L, NOMSG );
            if ( pBufCB )
            {
              if ( usRC )
              {
                UtlBufClose( pBufCB, FALSE );      // close file w/o error handling
              }
              else
              {
                usRC = UtlBufClose( pBufCB, TRUE );
              } /* endif */
            } /* endif */
          }
          break;

      } /* endswitch */
      break;

  } /* endswitch */

  return( usRC );

} /* end of function LstExportList */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     LSTIMPORTDLG   Dialog procedure for the list import dlg  |
//+----------------------------------------------------------------------------+
//|Function call:     LSTIMPORTDLG ( HWND hwnd, USHORT msg, MPARAM mp1,        |
//|                                  MPARAM mp2 )                              |
//+----------------------------------------------------------------------------+
//|Description:      Dialog procedure for the import of lists dialog.          |
//+----------------------------------------------------------------------------+
//|Input parameter:  HWND    hwnd     handle of window                         |
//|                  USHORT  msg      type of message                          |
//|                  MPARAM  mp1      first message parameter                  |
//|                  MPARAM  mp2      second message parameter                 |
//+----------------------------------------------------------------------------+
//|Returncode type:  MRESULT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:      depends on message type                                   |
//|                  normal return codes are:                                  |
//|                  TRUE  = message has been processed                        |
//|                  FALSE = message has not been processed                    |
//+----------------------------------------------------------------------------+
//|Function flow:     switch msg;                                              |
//|                     case WM_INITDLG:                                       |
//|                       call LstImpInit to initialize the dialog             |
//|                     case WM_EQF_CLOSE                                          |
//|                       dismiss the dialog                                   |
//|                     case WM_DESTROY                                        |
//|                       free the memory allocated for the IDA                |
//|                     case WM_CONTROL                                        |
//|                       switch control id                                    |
//|                         case MAT format radiobutton                        |
//|                           hide name static, name entry field               |
//|                           hide current directory static                    |
//|                           hide directory static and listbox                |
//|                           hide file name static and listbox                |
//|                           simulate selecting of the current drive button   |
//|                           set mode to import in MAT format                 |
//|                         case external radiobutton                          |
//|                         case user format radiobutton                       |
//|                           show name static, name entry field               |
//|                           show current directory static                    |
//|                           show directory static and listbox                |
//|                           show file name static and listbox                |
//|                           simulate selecting of the current drive button   |
//|                           set mode to import in the selected format        |
//|                         case drive button                                  |
//|                           if dialog is in import in MAT format then        |
//|                             deselect the old drive button                  |
//|                             select the new one                             |
//|                             refresh the directory listbox with the names   |
//|                              of exported lists on the selected drive       |
//|                           else                                             |
//|                             call UtlWMControls to process the message      |
//|                           endif                                            |
//|                         default:                                           |
//|                           call UtlWMControls to process the message        |
//|                       endswitch                                            |
//|                     case WM_CHAR:                                          |
//|                       if VK_ENTER or VK_NEWLINE then                       |
//|                         call UtlWMChar                                     |
//|                       else                                                 |
//|                         call default dialog procedure                      |
//|                       endif                                                |
//|                     case WM_COMMAND:                                       |
//|                       call LstImpCommand to process the message            |
//|                     default:                                               |
//|                       call default dialog procedure                        |
//|                   endswitch;                                               |
//+----------------------------------------------------------------------------+
INT_PTR CALLBACK LSTIMPORTDLG
(
   HWND   hwnd,
   WINMSG msg,
   WPARAM mp1,
   LPARAM mp2
)
{
  MRESULT   mResult = FALSE;
  PLSTIMPIDA pIda;                     // pointer to list import IDA

  switch ( msg )
  {
    case WM_EQF_QUERYID:
      pIda = ACCESSDLGIDA( hwnd, PLSTIMPIDA );
      switch ( pIda->usListType )
      {
        case NTL_TYPE:
          HANDLEQUERYID( ID_LISTIMP_NTL_DLG, mp2 );
          break;
        case FTL_TYPE:
          HANDLEQUERYID( ID_LISTIMP_FTL_DLG, mp2 );
          break;
        case EXCL_TYPE:
          HANDLEQUERYID( ID_LISTIMP_EXCL_DLG, mp2 );
          break;
        case NOISE_TYPE:
          HANDLEQUERYID( ID_LISTIMP_NOIS_DLG, mp2 );
          break;
        default:
          break;
      } /* endswitch */
      break;

    case WM_INITDLG :             //initialize and display dialogbox
      mResult = LstImpInit( hwnd, mp1, mp2 );
      break;

    case WM_COMMAND :
      mResult = LstImpCommand( hwnd, WMCOMMANDID( mp1, mp2 ),
                               WMCOMMANDCMD( mp1, mp2 ));
      break;

    case WM_EQF_CLOSE :
      WinDismissDlg( hwnd, SHORT1FROMMP1( mp1 ) );
      break;

   case DM_GETDEFID:
        pIda = ACCESSDLGIDA( hwnd, PLSTIMPIDA );
       mResult = UtlDMGETDEFID( hwnd, mp1, mp2, &pIda->Controls );
       break;

    default :
       //return to default dialog procedure
       mResult = UTLDEFDIALOGPROC( hwnd, msg, mp1, mp2 );
      break;
  } /* endswitch */

  return( mResult );

} /* end of function LSTIMPORTDLG */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     LstImpInit                                               |
//+----------------------------------------------------------------------------+
//|Function call:     LstImpInit( HWND hwnd, MPARAM mp2, MPARAM mp2 );         |
//+----------------------------------------------------------------------------+
//|Description:       Initialization code for the list import dialog.          |
//+----------------------------------------------------------------------------+
//|Input parameter:   HWND   hwnd      Dialog window handle                    |
//|                   MPARAM mp1       First message parameter of WM_INITDLG   |
//|                                    message: dialog window handle           |
//|                   MPARAM mp2       Second message parameter of WM_INITDLG  |
//|                                    message: ptr to object name of list     |
//|                                    being imported                          |
//+----------------------------------------------------------------------------+
//|Returncode type:   MRESULT                                                  |
//+----------------------------------------------------------------------------+
//|Returncodes:       FALSE  (always)                                          |
//+----------------------------------------------------------------------------+
//|Side effects:      The dialog is removed using WInDismissDlg if severe      |
//|                   errors occur during initialization                       |
//+----------------------------------------------------------------------------+
//|Function flow:     allocate and anchor IDA                                  |
//|                   look for userexit DLL and check entry points             |
//|                   if found then                                            |
//|                     enable user format radio button                        |
//|                   else                                                     |
//|                     disable user format radio button                       |
//|                   end                                                      |
//|                   store list name and type in dialog IDA                   |
//|                   get and apply last used values                           |
//|                   store control IDs and initial values in Controls IDA     |
//|                   call dialog utility to do the rest of the initialization |
//|                   if initialization failed then                            |
//|                     dismiss the dialog                                     |
//|                   else                                                     |
//|                     set focus to currently active format radio button      |
//|                   endif                                                    |
//+----------------------------------------------------------------------------+
static MRESULT LstImpInit
(
  HWND  hwnd,                          // handle of dialog window
  WPARAM mp1,                          // message parameter or NULL
  LPARAM mp2                           // message parameter or NULL
)
{
  PLSTIMPIDA pIda;                     // pointer to list import IDA
  EQFINFO    ErrorInfo;                // return code of property handler
  HPROP      hProp = NULL;             // handle of list properties
  PPROPLIST  pProp;                    // pointer to list properties
  PSZ        pszError;                 // pointer to error message parameter
  BOOL       fOK = TRUE;               // internal OK flag
  USHORT     usID = 0;                 // ID of titlebar text string

  mp1;                                // get rid of compiler warnings
  /*******************************************************************/
  /* Allocate and anchor IDA                                         */
  /*******************************************************************/
  fOK = UtlAlloc( (PVOID *) &pIda, 0L, (LONG) sizeof(LSTIMPIDA), ERROR_STORAGE );
  if ( fOK )
  {
     ANCHORDLGIDA( hwnd, pIda );
  } /* endif */

  /*******************************************************************/
  /* Store list object name and list type in IDA (list type is taken */
  /* from the extension of the list object name)                     */
  /*******************************************************************/
  if ( fOK )
  {
    strcpy( pIda->szListType, (PSZ) mp2 );
    if ( stricmp( pIda->szListType, NTL_LIST_NAME ) == 0 )
    {
      pIda->usListType = NTL_TYPE;
    }
    else if ( stricmp( pIda->szListType, FTL_LIST_NAME ) == 0 )
    {
      pIda->usListType = FTL_TYPE;
    }
    else if ( stricmp( pIda->szListType, EXCLUSION_LIST_NAME ) == 0 )
    {
      pIda->usListType = EXCL_TYPE;
    }
    else if ( stricmp( pIda->szListType, NOISE_LIST_NAME ) == 0 )
    {
      pIda->usListType = NOISE_TYPE;
    }
    else
    {
      /***************************************************************/
      /* Internal Error: List is no list processed by the built-in   */
      /* list processors.                                            */
      /***************************************************************/
      UtlError( ERROR_INTERNAL, MB_CANCEL, 0, NULL, INTERNAL_ERROR );
      fOK = FALSE;
    } /* endif */
  } /* endif */

   /*******************************************************************/
   /* Set title bar text                                              */
   /*******************************************************************/
   if ( fOK )
   {
     HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
     switch ( pIda->usListType )
     {
       case NTL_TYPE :
         usID = SID_LSTIMP_NTL_TITLE_TEXT;
         SETWINDOWID( hwnd, ID_LISTIMP_NTL_DLG );
         break;
       case FTL_TYPE :
         usID = SID_LSTIMP_FTL_TITLE_TEXT;
         SETWINDOWID( hwnd, ID_LISTIMP_FTL_DLG );
         break;
       case EXCL_TYPE :
         usID = SID_LSTIMP_EXCL_TITLE_TEXT;
         SETWINDOWID( hwnd, ID_LISTIMP_EXCL_DLG );
         break;
       case NOISE_TYPE :
         usID = SID_LSTIMP_NOISE_TITLE_TEXT;
         SETWINDOWID( hwnd, ID_LISTIMP_NOIS_DLG );
         break;
     } /* endswitch */
     LOADSTRING( NULLHANDLE, hResMod, usID, pIda->szString );
     SETTEXTHWND( hwnd, pIda->szString );
   } /* endif */

  /********************************************************************/
  /* Open list properties                                             */
  /********************************************************************/
  if ( fOK )
  {
    UtlMakeEQFPath( pIda->szPropName, NULC, SYSTEM_PATH, NULL );
    strcat( pIda->szPropName, BACKSLASH_STR );
    strcat( pIda->szPropName, LISTHANDLER_PROPERTIES_NAME );
    hProp = OpenProperties( pIda->szPropName, NULL,
                            PROP_ACCESS_READ, &ErrorInfo );
    if( !hProp )
    {
      /****************************************************************/
      /* error opening properties                                     */
      /****************************************************************/
      pszError = LISTHANDLER_PROPERTIES_NAME;
      UtlError( ERROR_OPEN_PROPERTIES,
                MB_CANCEL,
                1,
                &pszError,
                EQF_ERROR);
      fOK = FALSE;
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* Setup controls IDA needed for dialog control utility             */
  /********************************************************************/
  if ( fOK )
  {
    pIda->usFormat = SGMLFORMAT_ASCII;          // set default value

    pIda->Controls.idFilesLB            = ID_LISTIMP_FILES_LB;
    pIda->Controls.idDirLB              = ID_LISTIMP_DIR_LB;
    pIda->Controls.idPathEF             = ID_LISTIMP_PATH_EF;
    pIda->Controls.idCurrentDirectoryEF = ID_LISTIMP_NEWCURDIR_TEXT;
    pIda->Controls.idToLB               = ID_LISTIMP_TO_LB;
    pIda->Controls.idInternalRB         = ID_LISTIMP_USER_RB;
    pIda->Controls.idExternalRB         = ID_LISTIMP_EXTERNAL_RB;
    pIda->Controls.idDriveBTN           = ID_LISTIMP_DUMMY;
    pIda->Controls.idControlsGB         = ID_LISTIMP_IMPORT_GB;
    pIda->Controls.idOkPB               = ID_LISTIMP_OK_PB;
    strcpy( pIda->Controls.szHandler, LISTHANDLER );
    pIda->Controls.fImport = TRUE;
    ENABLECTRL( hwnd, ID_LISTIMP_OK_PB, FALSE );
    strcpy( pIda->Controls.szDefPattern, DEFAULT_PATTERN_NAME );
    switch ( pIda->usListType )
    {
      case NTL_TYPE :
        strcat( pIda->Controls.szDefPattern, EXT_OF_NEWTERMS_LIST );
        break;
      case FTL_TYPE :
        strcat( pIda->Controls.szDefPattern,EXT_OF_FOUNDTERMS_LIST );
        break;
      default:
        strcat( pIda->Controls.szDefPattern, EXT_OF_EXTERNAL_LIST );
        break;
    } /* endswitch */
  } /* endif */

  /********************************************************************/
  /* Set text limits                                                  */
  /********************************************************************/
  if ( fOK )
  {
    CBSETTEXTLIMIT( hwnd, ID_LISTIMP_TO_LB, MAX_FNAME - 1 );
  } /* endif */

  /********************************************************************/
  /* Load last used values from properties into controls IDA          */
  /********************************************************************/
  if ( fOK )
  {
    PSZ pszFName;
    pProp = (PPROPLIST)MakePropPtrFromHnd( hProp );
    if ( pProp->usImpFormat ) pIda->usFormat = pProp->usImpFormat;
    pIda->Controls.chSavedDrive = pProp->chListImpDrive;
    strcpy( pIda->Controls.szSavedPath, pProp->szListImpPath );
    pszFName = UtlGetFnameFromPath( pIda->Controls.szSavedPath );
    if ( pszFName != NULL )
    {
      *pszFName = EOS;
    }
    else
    {
      pIda->Controls.szSavedPath[0] = EOS;
    } /* endif */
    strcat( pIda->Controls.szSavedPath, DEFAULT_PATTERN_NAME );
    strcat( pIda->Controls.szSavedPath, pIda->Controls.szExt );
    pIda->Controls.usSavedFormat = 0;
    pIda->usFormatID = ID_LISTIMP_EXTERNAL_RB;
    CloseProperties( hProp, PROP_QUIT, &ErrorInfo );
  } /* endif */

  /********************************************************************/
  /* Call up utility to initialize dialog controls                    */
  /********************************************************************/
  if ( fOK )
  {
    {
      PSZ pszFormatList = SGML_FORMAT_FILTERS;
      USHORT usIndex = 1;
      while ( pszFormatList[0] != EOS )
      {
          SHORT sItem = CBINSERTITEM( hwnd, ID_LISTIMP_FORMAT_CB, pszFormatList );
          if ( sItem != LIT_NONE )
          {
            CBSETITEMHANDLE( hwnd, ID_LISTIMP_FORMAT_CB, sItem, usIndex );
            if ( usIndex == pIda->usFormat )
            {
              CBSELECTITEM( hwnd, ID_LISTIMP_FORMAT_CB, sItem );
            } /* endif */
          } /* endif */
        pszFormatList += strlen(pszFormatList) + 1;   // skip filter name
        pszFormatList += strlen(pszFormatList) + 1;   // skip filter extention
        usIndex++;
      } /* endwhile */
    }
    UtlControlsInit( hwnd, &pIda->Controls );
  } /* endif */

  /********************************************************************/
  /* Fill import to combobox                                          */
  /********************************************************************/
  if ( fOK )
  {
    LstInsertListNames( (LISTTYPES)pIda->usListType, FALSE,
                        WinWindowFromID( hwnd, ID_LISTIMP_TO_LB ),
                        pIda->szString, FALSE );
  } /* endif */

  /*******************************************************************/
  /* Remove dialog if initialization failed or set focus to          */
  /* currently active format radio button and set radiobutton        */
  /* selection state                                                  */
  /*******************************************************************/
  if ( !fOK )
  {
    WinDismissDlg( hwnd, FALSE );
  }
  else
  {
    pIda->fInitInProgress = TRUE;
    CLICK( hwnd, pIda->usFormatID );
    pIda->fInitInProgress = FALSE;
  } /* endif */

  return( MRFROMSHORT(TRUE) );
} /* end of function LstImpInit */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     LstImpCommand                                            |
//+----------------------------------------------------------------------------+
//|Function call:     LstImpCommand( HWND hwnd, MPARAM mp2, MPARAM mp2 );      |
//+----------------------------------------------------------------------------+
//|Description:       Process the WM_COMMAND message of the list import dialog |
//|                   and do the actual importing of the list.                 |
//+----------------------------------------------------------------------------+
//|Input parameter:   HWND   hwnd      Dialog window handle                    |
//|                   MPARAM mp1       First message parameter of WM_INITDLG   |
//|                                    message: dialog window handle           |
//|                   MPARAM mp2       Second message parameter of WM_INITDLG  |
//|                                    message: ptr to object name of list     |
//|                                    being imported                          |
//+----------------------------------------------------------------------------+
//|Returncode type:   MRESULT                                                  |
//+----------------------------------------------------------------------------+
//|Returncodes:       FALSE  (always)                                          |
//+----------------------------------------------------------------------------+
//|Side effects:      The list is imported if 'import' has been pressed. The   |
//|                   dialog is ended on Cancel or successful import.          |
//+----------------------------------------------------------------------------+
//|Function flow:     switch control ID                                        |
//|                     case CANCEL button:                                    |
//|                     case escape key:                                       |
//|                       post a WM_EQF_CLOSE message to the dialog                |
//|                     case import button:                                    |
//|                       address IDA                                          |
//|                       if not in MAT format import then                     |
//|                         get and check specified file name                  |
//|                       endif                                                |
//|                       if ok and not in MAT format import                   |
//|                         build and check name of import file                |
//|                       endif                                                |
//|                       if ok then                                           |
//|                         check if system drive has enough free space to     |
//|                          receive the imported list                         |
//|                       endif                                                |
//|                       if ok then                                           |
//|                         call LstImportList to do the actual import         |
//|                       endif                                                |
//|                       if ok then                                           |
//|                         close the dialog and save last used values         |
//|                       endif                                                |
//|                   endswitch                                                |
//+----------------------------------------------------------------------------+
MRESULT LstImpCommand
(
  HWND  hwnd,                          // handle of dialog window
   SHORT sId,                          // id of button
   SHORT sNotification                 // notification type
)
{
   PLSTIMPIDA    pIda = NULL;          // Load dialog IDA
   USHORT      usRC;                   // Return code from Dos functions
   SHORT       sIndexItem;             // index of selected item in listb.
   MRESULT     mResult = (MRESULT)FALSE;  // function return value
   BOOL        fOK = TRUE;             // success indicator
   HDIR        hDirHandle = HDIR_CREATE;    // DosFind routine handle
   USHORT      usCount = 1;            // number of files requested
   PSZ         pszTemp;                // temp. pointer to dict name
   PSZ         pszErrParm[2];          // error parameter array
   ULONG       ulSpace;                // number of bytes missing on disk
   CHAR        szDrive[MAX_DRIVE];     // buffer for drive name
   PPROPLIST   pProp;                  // ptr to dictionary properties
   EQFINFO     ErrorInfo;              // error info from property handler
   HPROP       hProp;                  // property handle
   FILEFINDBUF stResultBuf;            // buffer for result of UtlFindFirst

   sNotification;                      // get rid of compiler warning

   memset(&stResultBuf, 0, sizeof(stResultBuf));
   switch ( sId )
   { case ID_LISTIMP_HELP_PB:
      mResult = UtlInvokeHelp();
      break;
     case ID_LISTIMP_CANCEL_PB :       // CANCEL button selected
     case DID_CANCEL :                 // ESCape key pressed
       WinPostMsg( hwnd, WM_EQF_CLOSE, NULL, NULL );
       break;

     case ID_LISTIMP_OK_PB :
       pIda = ACCESSDLGIDA( hwnd, PLSTIMPIDA );

       /***************************************************************/
       /* Get values from controls ida                                */
       /***************************************************************/
       pIda->usFormatID = ID_LISTIMP_EXTERNAL_RB;
       {
         SHORT sItem = CBQUERYSELECTION( hwnd, ID_LISTIMP_FORMAT_CB );
         if ( sItem >= 0 )
         {
           pIda->usFormat = (USHORT)CBQUERYITEMHANDLE( hwnd, ID_LISTIMP_FORMAT_CB, sItem );
         }
         else
         {
           // use default mode
           pIda->usFormat = SGMLFORMAT_ASCII;
         } /* endif */
       }

       if ( pIda->usFormatID != ID_LISTIMP_MAT_RB )
       {
         strcpy( pIda->szImportPath,  pIda->Controls.szPathContent );
         strcpy( pIda->szImportName,  pIda->Controls.szPatternName );
         strcpy( pIda->szImportDrive, pIda->Controls.szDrive );
       }
       else
       {
         sIndexItem = QUERYSELECTION( hwnd, ID_LISTIMP_DIR_LB );
         if ( sIndexItem == LIT_NONE )
         {
           UtlError( NO_FILE_SELECTED, MB_CANCEL, 0, NULL, EQF_ERROR );
           SETFOCUS( hwnd, ID_LISTIMP_DIR_LB );
           fOK = FALSE;
         }
         else
         {
           strcpy( pIda->szImportDrive, pIda->Controls.szDrive );
           QUERYITEMTEXT( hwnd, ID_LISTIMP_DIR_LB, sIndexItem, pIda->szImportName );
           UtlMakeEQFPath( pIda->szImportPath, pIda->szImportDrive[0],
                           EXPORT_PATH, NULL );
           strcat( pIda->szImportPath, BACKSLASH_STR );
           strcat( pIda->szImportPath, pIda->szImportName );
           switch ( pIda->usListType )
           {
             case NTL_TYPE :
               strcat( pIda->szImportPath, EXPORT_NTL_EXT );
               break;
             case FTL_TYPE :
               strcat( pIda->szImportPath, EXPORT_FTL_EXT );
               break;
             case EXCL_TYPE :
               strcat( pIda->szImportPath, EXPORT_EXCL_EXT );
               break;
             case NOISE_TYPE :
               strcat( pIda->szImportPath, EXPORT_NOISE_EXT);
               break;
           } /* endswitch */
         } /* endif */
       } /* endif */

       /***************************************************************/
       /* Get new list name from drop-down combo-box                  */
       /***************************************************************/
       if ( fOK )
       {
         QUERYTEXT( hwnd, ID_LISTIMP_TO_LB, pIda->szListName );
       } /* endif */

       /***************************************************************/
       /* If no list selected issue warning                           */
       /***************************************************************/
       if ( fOK )
       {
         UtlStripBlanks( pIda->szListName );
         strupr( pIda->szListName );
         if ( pIda->szListName[0] == NULC )
         {
            UtlError( ERROR_LST_NO_IMPORT_LIST, MB_CANCEL, 0, NULL, EQF_WARNING );
            fOK = FALSE;
            SETFOCUS( hwnd, ID_LISTIMP_TO_LB );
         } /* endif */
       } /* endif */

       /***************************************************************/
       /* Check if list name is valid                                 */
       /***************************************************************/
       if ( fOK )
       {
         pszTemp = pIda->szListName;
         while ( *pszTemp && isalnum(*pszTemp) )
         {
            pszTemp++;
         } /* endwhile */

         if ( *pszTemp != NULC )
         {
            /**********************************************************/
            /* Filename as specified in drop-down combo-box invalid   */
            /**********************************************************/
            pszTemp = pIda->szListName;
            UtlError( ERROR_FILENAME_NOT_VALID, MB_CANCEL,
                      1, &pszTemp, EQF_ERROR );
            fOK = FALSE;
            SETFOCUS( hwnd, ID_LISTIMP_TO_LB );
         } /* endif */
       } /* endif */

       /***************************************************************/
       /* Build fully qualified target list name                      */
       /***************************************************************/
       if ( fOK )
       {
          UtlMakeEQFPath( pIda->szListPath, NULC,
                          (SHORT)(( pIda->usListType == NOISE_TYPE ) ? TABLE_PATH :
                                                               LIST_PATH),
                          NULL );
          strcat( pIda->szListPath, BACKSLASH_STR );
          strcat( pIda->szListPath, pIda->szListName );
          switch ( pIda->usListType )
          {
            case NTL_TYPE :
              strcat( pIda->szListPath, EXT_OF_NEWTERMS_LIST );
              break;
            case FTL_TYPE :
              strcat( pIda->szListPath, EXT_OF_FOUNDTERMS_LIST );
              break;
            case EXCL_TYPE :
            case NOISE_TYPE :
              strcat( pIda->szListPath, EXT_OF_EXCLUSION );
              break;
          } /* endswitch */
       } /* endif */

       /***************************************************************/
       /* Check if input file exists                                  */
       /***************************************************************/
       if ( fOK )
       {
         usRC = UtlFindFirst( pIda->szImportPath,
                              &hDirHandle,
                              FILE_NORMAL,
                              &stResultBuf,
                              sizeof( stResultBuf),
                              &usCount, 0L, FALSE);
         if ( usRC )
         {
            /**********************************************************/
            /* Import file does not exist or file name is invalid     */
            /**********************************************************/
            Utlstrccpy( pIda->szString, pIda->szImportName, DOT );
            pszTemp = pIda->szString;
            UtlError( ERROR_FILENAME_NOT_VALID, MB_CANCEL,
                      1, &pszTemp, EQF_ERROR );
            SETFOCUS( hwnd, ID_LISTIMP_PATH_EF );
            fOK = FALSE;
         } /* endif */

         // close file search handle
         if ( hDirHandle != HDIR_CREATE ) UtlFindClose( hDirHandle, FALSE );
       } /* endif */

       /***************************************************************/
       /* Check if input file contains data                           */
       /***************************************************************/
       if ( fOK )
       {
         if ( RESBUFSIZE(stResultBuf) == 0 )
         {
            Utlstrccpy( pIda->szString, pIda->szImportName, DOT );
            pszTemp = pIda->szString;
            UtlError( ERROR_FILE_IMP_SIZE, MB_CANCEL,
                      1, &pszTemp, EQF_ERROR );
            fOK = FALSE;
         } /* endif */
       } /* endif */

       /******************************************************************/
       /* Check if there is enough room left on the target drive.        */
       /*   The check is performed on the size of the source file * 1.5. */
       /*   This check is not accurate as the exact enlargement factor   */
       /*   may be different. But as it is not possible to compute       */
       /*   the size of the target file without importing it, the check  */
       /*   seems to be accurate enough.                                 */
       /******************************************************************/
       if ( fOK )
       {
         UtlQueryString( QST_PRIMARYDRIVE, szDrive, sizeof(szDrive) );
         pszTemp = szDrive;
         fOK = UtlCheckSpaceForFile( pIda->szImportPath,
                                     150,                 // factor is 150%
                                     0L,                  // no explicit size
                                     &pszTemp,            // target drive
                                     (PLONG)&ulSpace,     // missing space
                                     TRUE );              // message handle flag

         if ( fOK && (ulSpace != 0L) )
         {
           pszErrParm[0] = szDrive;
           pszErrParm[1] = UtlGetFnameFromPath( pIda->szImportPath );
           UtlError( ERROR_LSTIMP_NOSPACE, MB_CANCEL, 2, pszErrParm, EQF_ERROR );
           fOK = FALSE;
         } /* endif */
       } /* endif */

       /***************************************************************/
       /* Check if target list exists and get user confirmation for   */
       /* overwrite of list.                                          */
       /***************************************************************/
       if ( fOK )
       {
         sIndexItem = CBSEARCHITEM( hwnd, ID_LISTIMP_TO_LB, pIda->szListName );

         if ( sIndexItem != LIT_NONE )
         {
             pszTemp = pIda->szListName;
             if ( UtlError( WARNING_LST_FILE_EXISTS,
                            MB_YESNO,
                            1, &pszTemp, EQF_WARNING ) != MBID_YES )
             {
                SETTEXT( hwnd, ID_LISTIMP_TO_LB, EMPTY_STRING );
                SETFOCUS( hwnd, ID_LISTIMP_TO_LB );
                fOK = FALSE;
             } /* endif */
         } /* endif */
       } /* endif */

       /******************************************************************/
       /* Do the actual import of the list                               */
       /******************************************************************/
       if ( fOK )
       {
         if ( LstImportList( pIda->szListPath, pIda->usListType,
                             pIda->usFormatID, pIda->szImportPath,
                             pIda->usFormat ) == 0 )
         {
           pszTemp = pIda->szListName;
           UtlError( INFO_LST_IMPORTED, MB_OK, 1, &pszTemp, EQF_INFO );
         }
         else
         {
           fOK = FALSE;
         } /* endif */
       } /* endif */

       /***************************************************************/
       /* Close dialog and save last used values                      */
       /***************************************************************/
       if ( fOK )
       {
         hProp = OpenProperties( pIda->szPropName, NULL, PROP_ACCESS_READ,
                                 &ErrorInfo);
         if ( !hProp )
         {
           pszTemp = pIda->szPropName;
           UtlError( ERROR_OPEN_PROPERTIES, MB_CANCEL, 1, &pszTemp, EQF_ERROR );
           fOK = FALSE;
         } /* endif */

         if ( fOK )
         {
           if( SetPropAccess( hProp, PROP_ACCESS_WRITE))
           {
             pProp = (PPROPLIST) MakePropPtrFromHnd( hProp);
             pProp->usListImpFormatID = pIda->usFormatID;
             if ( pIda->usFormatID != ID_LISTIMP_MAT_RB )
             {
               strcpy( pProp->szListImpPath, pIda->szImportPath );
             } /* endif */
             pProp->chListImpDrive = pIda->szImportDrive[0];
             pProp->usImpFormat = pIda->usFormat;
             SaveProperties( hProp, &ErrorInfo);
             CloseProperties( hProp, PROP_QUIT, &ErrorInfo );
           } /* endif */
         } /* endif */

         WinPostMsg( hwnd, WM_EQF_CLOSE, MP1FROMSHORT( fOK ), NULL );
       } /* endif */
       break;
     default :
        mResult = LstImpControl( hwnd, sId, sNotification );
        break;
   } /* endswitch */

   return mResult;

} /* end of function LstImpCommand */


MRESULT LstImpControl
(
   HWND   hwnd,                        // dialog handle
   SHORT  sId,                         // id in action
   SHORT  sNotification                // notification
)
{
  PLSTIMPIDA    pIda = NULL;           // Load dialog IDA
  USHORT    usDrive;                   // ID of new drive button
  USHORT    usOldDrive;                // ID of old drive button
  SHORT     sRC;                       // short return code
  SHORT     sItem;                     // index of listbox items

  /****************************************************************/
  /* Get access to ida                                            */
  /****************************************************************/
  pIda = ACCESSDLGIDA( hwnd, PLSTIMPIDA );

  /****************************************************************/
  /* Filter messages from format radio buttons as they are        */
  /* currently not handled correctly in the dialog utility        */
  /****************************************************************/
  switch ( sId )
  {
    case ID_LISTIMP_EXTERNAL_RB :
    case ID_LISTIMP_USER_RB :
      if ( QUERYCHECK( hwnd, sId ) )
      {
        if ( (pIda->usFormatID == ID_LISTIMP_MAT_RB) ||
             pIda->fInitInProgress )
        {
          HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
          /********************************************************/
          /* Show name, current directory, directory and file     */
          /* control                                              */
          /********************************************************/
          SHOWCONTROL( hwnd, ID_LISTIMP_FILENAME_TEXT );
          SHOWCONTROL( hwnd, ID_LISTIMP_PATH_EF );
          SHOWCONTROL( hwnd, ID_LISTIMP_FILES_TEXT );
          SHOWCONTROL( hwnd, ID_LISTIMP_FILES_LB );
          SHOWCONTROL( hwnd, ID_LISTIMP_CURDIR_TEXT );
          SHOWCONTROL( hwnd, ID_LISTIMP_NEWCURDIR_TEXT );
          LOADSTRING( NULLHANDLE, hResMod, SID_LISTIMP_DIR_LB_TITLE, pIda->szString );
          SETTEXT( hwnd, ID_LISTIMP_DIR_TEXT, pIda->szString );

          /********************************************************/
          /* Force a refresh of the listboxes                     */
          /********************************************************/
          if ( pIda->Controls.szDrive[0] != EOS )
          {
             usDrive = IDFROMDRIVE( PID_DRIVEBUTTON_A,
                                    pIda->Controls.szDrive[0]);
             pIda->Controls.szDrive[0] += 1;
             SENDNOTIFICATION( hwnd, usDrive, BN_CLICKED );
          } /* endif */
        } /* endif */
        pIda->usFormatID = sId;

        /**********************************************************/
        /* force refresh of Import pushbutton state               */
        /**********************************************************/
        SENDNOTIFICATION( hwnd, ID_LISTIMP_TO_LB, CBN_EFCHANGE );
      } /* endif */
      break;

    case ID_LISTIMP_MAT_RB :
      if ( QUERYCHECK( hwnd, sId ) )
      {
        if ( (pIda->usFormatID != ID_LISTIMP_MAT_RB) ||
             pIda->fInitInProgress )
        {
          HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
          /********************************************************/
          /* Hide name, current directory, directory and file     */
          /* control                                              */
          /********************************************************/
          HIDECONTROL( hwnd, ID_LISTIMP_FILENAME_TEXT );
          HIDECONTROL( hwnd, ID_LISTIMP_PATH_EF );
          HIDECONTROL( hwnd, ID_LISTIMP_FILES_TEXT );
          HIDECONTROL( hwnd, ID_LISTIMP_FILES_LB );
          HIDECONTROL( hwnd, ID_LISTIMP_CURDIR_TEXT );
          HIDECONTROL( hwnd, ID_LISTIMP_NEWCURDIR_TEXT );
          LOADSTRING( NULLHANDLE, hResMod, SID_LISTIMP_LIST_LB_TITLE, pIda->szString );
          SETTEXT( hwnd, ID_LISTIMP_DIR_TEXT, pIda->szString );

          /********************************************************/
          /* Force a refresh of the listboxes                     */
          /********************************************************/
          if ( pIda->Controls.szDrive[0] != EOS )
          {
             usDrive = IDFROMDRIVE( PID_DRIVEBUTTON_A,
                                    pIda->Controls.szDrive[0]);
             pIda->Controls.szDrive[0] += 1;
             SENDNOTIFICATION( hwnd, usDrive, BN_CLICKED );
          } /* endif */

        } /* endif */
        pIda->usFormatID = sId;

        /**********************************************************/
        /* force refresh of Import pushbutton state               */
        /**********************************************************/
        SENDNOTIFICATION( hwnd, ID_LISTIMP_TO_LB, CBN_EFCHANGE );
      } /* endif */
      break;

    case ID_LISTIMP_TO_LB:
      if ( sNotification == CBN_EFCHANGE )
      {
        QUERYTEXT( hwnd, ID_LISTIMP_TO_LB, pIda->szListName );
        UtlStripBlanks( pIda->szListName );
        if ( pIda->usFormatID == ID_LISTIMP_MAT_RB )
        {
          sItem = QUERYNEXTSELECTION( hwnd, ID_LISTIMP_DIR_LB, LIT_FIRST );
        }
        else
        {
          sItem = QUERYNEXTSELECTION( hwnd, ID_LISTIMP_FILES_LB, LIT_FIRST );
        } /* endif */
        ENABLECTRL( hwnd, ID_LISTIMP_OK_PB,
                    ((pIda->szListName[0] != EOS) && (sItem != LIT_NONE)) );
      }
      else if ( sNotification == CBN_SELCHANGE )
      {
        if ( pIda->usFormatID == ID_LISTIMP_MAT_RB )
        {
          sItem = QUERYNEXTSELECTION( hwnd, ID_LISTIMP_DIR_LB, LIT_FIRST );
        }
        else
        {
          sItem = QUERYNEXTSELECTION( hwnd, ID_LISTIMP_FILES_LB, LIT_FIRST );
        } /* endif */
        ENABLECTRL( hwnd, ID_LISTIMP_OK_PB, (sItem != LIT_NONE) );
      } /* endif */
      break;

    case ID_LISTIMP_DIR_LB:
      if ( pIda->usFormatID == ID_LISTIMP_MAT_RB )
      {
        if ( sNotification == LN_SELECT )
        {
          QUERYTEXT( hwnd, ID_LISTIMP_TO_LB, pIda->szListName );
          UtlStripBlanks( pIda->szListName );
          sItem = QUERYNEXTSELECTION( hwnd, ID_LISTIMP_DIR_LB, LIT_FIRST );
          ENABLECTRL( hwnd, ID_LISTIMP_OK_PB,
                    ((pIda->szListName[0] != EOS) && (sItem != LIT_NONE)) );
        } /* endif */
      }
      else
      {
        UtlWMControls( hwnd, WM_CONTROL, sId, sNotification, &pIda->Controls );
      } /* endif */
      break;

    case ID_LISTIMP_FILES_LB:
      if ( pIda->usFormatID != ID_LISTIMP_MAT_RB )
      {
        UtlWMControls( hwnd, WM_CONTROL, sId, sNotification, &pIda->Controls );
      } /* endif */
      if ( sNotification == LN_SELECT )
      {
        QUERYTEXT( hwnd, ID_LISTIMP_TO_LB, pIda->szListName );
        UtlStripBlanks( pIda->szListName );
        sItem = QUERYNEXTSELECTION( hwnd, ID_LISTIMP_FILES_LB, LIT_FIRST );
        ENABLECTRL( hwnd, ID_LISTIMP_OK_PB,
                  ((pIda->szListName[0] != EOS) && (sItem != LIT_NONE)) );
      } /* endif */
      break;

    default :
      if ( pIda->usFormatID == ID_LISTIMP_MAT_RB )
      {
        /**********************************************************/
        /* Handle control message within dialog as dialog utility */
        /* can not handle internal formats                        */
        /**********************************************************/
        if ( ( sNotification == BN_CLICKED )          &&
             ( sId >= PID_DRIVEBUTTON_A ) && ( sId <= PID_DRIVEBUTTON_Z ) )
        {
          if ( pIda->Controls.szDrive[0] != EOS )
          {
             usOldDrive = IDFROMDRIVE( PID_DRIVEBUTTON_A,
                                       pIda->Controls.szDrive[0]);
             SETDRIVE( hwnd, usOldDrive, FALSE );
          }
          else
          {
            usOldDrive = 0;
          } /* endif */

          /**********************************************************/
          /* Save newly selected drive character in IDA and select  */
          /* it                                                     */
          /**********************************************************/
          pIda->Controls.szDrive[0] = DRIVEFROMID( PID_DRIVEBUTTON_A, sId );
          SETDRIVE( hwnd, sId, TRUE );

          /**********************************************************/
          /* Update the filectory listbox which is here used as     */
          /* list name listbox                                      */
          /**********************************************************/
          UtlMakeEQFPath( pIda->szString, pIda->Controls.szDrive[0],
                          EXPORT_PATH, NULL );
          if ( UtlDirExist( pIda->szString ) )
          {
            strcat( pIda->szString, BACKSLASH_STR );
            strcat( pIda->szString, DEFAULT_PATTERN_NAME );
            switch ( pIda->usListType )
            {
              case  NTL_TYPE:
                strcat( pIda->szString, EXPORT_NTL_EXT );
                break;
              case  FTL_TYPE:
                strcat( pIda->szString, EXPORT_FTL_EXT );
                break;
              case  EXCL_TYPE:
                strcat( pIda->szString, EXPORT_EXCL_EXT );
                break;
              case  NOISE_TYPE:
                strcat( pIda->szString, EXPORT_NOISE_EXT );
                break;
              default :
                break;
            } /* endswitch */
            DELETEALL( hwnd, ID_LISTIMP_DIR_LB );
            sRC = UtlLoadFileNames( pIda->szString, FILE_NORMAL,
                                    WinWindowFromID( hwnd, ID_LISTIMP_DIR_LB ),
                                    NAMFMT_NOEXT );

            /********************************************************/
            /* if listboxes could not be filled,                    */
            /* e.g. drive not ready msg issued restore old drive    */
            /********************************************************/
            if ( sRC == UTLERROR )
            {
               SETDRIVE( hwnd, sId, FALSE );
               pIda->Controls.szDrive[0] = EOS;
               SENDNOTIFICATION( hwnd, usOldDrive, BN_CLICKED );
            } /* endif */
          }
          else
          {
            /******************************************************/
            /* Drive has no EQF export directory, delete the      */
            /* list box items                                     */
            /******************************************************/
            DELETEALL( hwnd, ID_LISTIMP_DIR_LB );
          } /* endif */
        } /* endif */
      }
      else
      {
        UtlWMControls( hwnd, WM_CONTROL, sId, sNotification, &pIda->Controls );
      } /* endif */
      if ( ( sNotification == BN_CLICKED )          &&
           ( sId >= PID_DRIVEBUTTON_A ) && ( sId <= PID_DRIVEBUTTON_Z ) )
      {
        /**********************************************************/
        /* force refresh of Import pushbutton state               */
        /**********************************************************/
        SENDNOTIFICATION( hwnd, ID_LISTIMP_TO_LB, CBN_EFCHANGE );
      } /* endif */

      break;
  } /* endswitch */
  return( (MRESULT)FALSE );
} /* end of function LstImpControl */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     LstImportList       Import a list                        |
//+----------------------------------------------------------------------------+
//|Function call:     LstImportList( PSZ pszListName, USHORT usListType,       |
//|                                  USHORT usFormatID, PSZ pszImportFile );   |
//+----------------------------------------------------------------------------+
//|Description:       Imports a list in the selected format                    |
//+----------------------------------------------------------------------------+
//|Input parameter:   PSZ    pszListName    fully qualified list name          |
//|                   USHORT usListType     identifier for type of list        |
//|                   USHORT usFormaID      identifier for format of import    |
//|                   PSZ    pszImportFile  file name of import file           |
//|                   PUSEREXIT pUserExit   ptr to structure containing user   |
//|                                         exit entry points or NULL if not   |
//|                                         importing in user format           |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       0       function completed successfully                  |
//|                   other   return code of calle DOS functions               |
//+----------------------------------------------------------------------------+
//|Function flow:     switch type of list                                      |
//|                     case new terms list                                    |
//|                     case found terms list                                  |
//|                       switch format of import                              |
//|                         case MAT format:                                   |
//|                           copy the import file to the list directory       |
//|                         case external format:                              |
//|                           read the import file using LstReadSGMLList with  |
//|                            the external flag set                           |
//|                           write the list using LstWriteSGMLList            |
//|                           free list areas                                  |
//|                         case user format:                                  |
//|                           call the import init entry point of user exit    |
//|                           while ok and not end of list                     |
//|                             request the next term from the import entry    |
//|                              point of the user exit                        |
//|                             add the term to the term list                  |
//|                           endwhile                                         |
//|                           call the import end entry point of user exit     |
//|                           write list using LstWriteSGMLList                |
//|                           free list areas                                  |
//|                     case noise list                                        |
//|                     case exclusion list                                    |
//|                       switch format of import                              |
//|                         case MAT format:                                   |
//|                           copy the import file to the list directory       |
//|                         case external format:                              |
//|                           read the list using UtlLoadFile                  |
//|                           convert loaded file into term list               |
//|                           write term list using LstWriteExclList           |
//|                           free list buffer                                 |
//|                         case user format:                                  |
//|                           while ok and not end of list                     |
//|                             request the next term from the import entry    |
//|                              point of the user exit                        |
//|                             add the term to the term buffer                |
//|                           endwhile                                         |
//|                           call the import end entry point of user exit     |
//|                           write list using LstWriteExclList                |
//|                           free list areas                                  |
//|                   endswitch                                                |
//+----------------------------------------------------------------------------+
USHORT LstImportList
(
  PSZ        pszListName,              // fully qualified list name
  USHORT     usListType,               // identifier for type of list
  USHORT     usFormatID,               // identifier for format of import
  PSZ        pszImportFile,            // file name of import file
  USHORT     usFormat                  // format of import (ASCII,ANSI,...)
)
{
  USHORT usRC = 0;                     // function return code
  LISTHEADER    ListHead;              // header for NTL+FTL lists
  PCONTEXTTABLE pContextTable= NULL;   // context table for NTL+FTL lists
  PTERMTABLE    pTermTable   = NULL;   // term table for NTL+FTL lists
  PPOOL         pPool        = NULL;   // string pool for NTL+FTL lists
  PBYTE         pTermBuffer  = NULL;   // pointer to terms for an exclusion lists
  ULONG         ulI;                   // general loop index
  PSZ_W         pszSource;             // pointer for copy operations: source
  PSZ_W         pszTarget;             // pointer for copy operations: target
  PSZ_W         pszTermStart;          // start position of current term
  BOOL          fExists = FALSE;       // target-list-exists flag
  PEXCLUSIONLIST pExclList;            // pointer to loaded exclusion lists


  switch ( usListType )
  {
    case NTL_TYPE :
    case FTL_TYPE :
      usRC = NO_ERROR;
      switch ( usFormatID )
      {
        case ID_LISTIMP_MAT_RB :
          /************************************************************/
          /* Read list to ensure that data is correct                 */
          /************************************************************/
          usRC = LstReadSGMLList( pszImportFile, &ListHead,
                                  &pTermTable, &pContextTable,
                                  &pPool, FALSE, (LISTTYPES)usListType, SGMLFORMAT_ASCII );
          if ( pTermTable )    LstDestroyTermTable( pTermTable );
          if ( pContextTable ) LstDestroyContextTable( pContextTable );
          if ( pPool )         PoolDestroy( pPool );

          /************************************************************/
          /* Copy the list file to the list directory                 */
          /************************************************************/
          if ( !usRC )
          {
            fExists = UtlFileExist( pszListName );
            UtlCopy( pszImportFile, pszListName, 1, 0L, TRUE );
          } /* endif */

          if ( !usRC )
          {
            if ( fExists )
            {
              EqfSend2AllHandlers( WM_EQFN_PROPERTIESCHANGED,
                                   MP1FROMSHORT( PROP_CLASS_LIST ),
                                   MP2FROMP(pszListName) );
            }
            else
            {
              EqfSend2AllHandlers( WM_EQFN_CREATED,
                                   MP1FROMSHORT( clsLIST ),
                                   MP2FROMP(pszListName) );
            } /* endif */
          } /* endif */
          break;

        case ID_LISTIMP_EXTERNAL_RB :
          memset( &ListHead, 0, sizeof(ListHead) );
          usRC = LstReadSGMLList( pszImportFile, &ListHead,
                                  &pTermTable, &pContextTable,
                                  &pPool, TRUE, (LISTTYPES)usListType, usFormat );
          if ( !usRC )
          {
            usRC = LstWriteSGMLList( pszListName,
                                     &ListHead,
                                     pTermTable,
                                     pContextTable,
                                     FALSE,
                                     (LISTTYPES)usListType, SGMLFORMAT_UNICODE );
          } /* endif */

          if ( pTermTable )    LstDestroyTermTable( pTermTable );
          if ( pContextTable ) LstDestroyContextTable( pContextTable );
          if ( pPool )         PoolDestroy( pPool );
          break;
        default:
          usRC = ERROR_INVALID_DATA;
      } /* endswitch */

      break;

    case  EXCL_TYPE :
    case  NOISE_TYPE :
      usRC = NO_ERROR;
      fExists = UtlFileExist( pszListName );
      switch ( usFormatID )
      {
        case ID_LISTIMP_MAT_RB :
          /************************************************************/
          /* Read list to ensure that data is correct                 */
          /************************************************************/
          pExclList = NULL;
          {
            ULONG ulLength;
            usRC = LstReadNoiseExclList( pszImportFile, &ulLength, &pExclList );
          }
          if ( pExclList ) UtlAlloc( (PVOID *) &pExclList, 0L, 0L, NOMSG );

          /************************************************************/
          /* Copy the import file to the list directory               */
          /************************************************************/
          if ( !usRC )
          {
            UtlCopy( pszImportFile, pszListName, 1, 0L, TRUE );
          } /* endif */
          break;

        case ID_LISTIMP_EXTERNAL_RB :
           { PPROPSYSTEM  pPropSys = GetSystemPropPtr();
             ULONG ulConvCP = pPropSys->ulSystemPrefCP;
             PSZ_W  pUnicodeBuffer = NULL;
             ULONG  ulNumOfWides = 0;

             if (usFormat == SGMLFORMAT_ANSI )
             {
                ulConvCP = GetLangAnsiCP(pPropSys->szSystemPrefLang);
             }

             pTermBuffer = NULL;
             if ( !UtlLoadFileL( pszImportFile, (PVOID *)&pTermBuffer, &ulI, FALSE, TRUE ) )
             {
               usRC = ERROR_INVALID_FUNCTION;  // set return code to non-zero value
             } /* endif */

             /**********************************************************/
             /* enlarge term buffer to contain term end characters     */
             /* (3 additional EOS characters may be needed)            */
             /**********************************************************/
             if ( !usRC  )
             {
               if ( !UtlAlloc( (PVOID *) &pUnicodeBuffer, 0L, (ulI + 3) * sizeof(CHAR_W),
                               ERROR_STORAGE ) )
               {
                 usRC = ERROR_NOT_ENOUGH_MEMORY;
               } /* endif */
             } /* endif */
            switch(usFormat)
            {
              case SGMLFORMAT_UNICODE:
              {  ULONG ulLen;
              // skip UNICODEFILEPREFIX, then copy
                ulLen = strlen(UNICODEFILEPREFIX);

                if (memcmp(pTermBuffer, UNICODEFILEPREFIX, ulLen) != 0)
                {
                   usRC = UtlError( NO_VALID_UNICODEFORMAT, MB_YESNO, 1, &pszImportFile,
                                    EQF_WARNING );

                   if ( usRC == MBID_NO )
                   {
                       usRC = ERROR_INVALID_DATA;
                   } /* endif */
                   else
                   {
                     memcpy( pUnicodeBuffer, pTermBuffer,  ulI);
                     ulNumOfWides = ulI;
                   }
                }
                else
                {
                  memcpy( pUnicodeBuffer, pTermBuffer + ulLen, ulI - ulLen);
                  ulNumOfWides = ulI - ulLen;
                }
              }
              break;
              case SGMLFORMAT_ANSI:
                     ulNumOfWides = MultiByteToWideChar( ulConvCP, 0,
                                          (LPCSTR)pTermBuffer, ulI,
                                          (LPWSTR)(pUnicodeBuffer),
                                          (ulI + 3));


               break;
               case SGMLFORMAT_ASCII:

               default:
                  ulNumOfWides = MultiByteToWideChar(ulConvCP, 0,
                                                 (LPCSTR)pTermBuffer, ulI,
                                                 (LPWSTR)(pUnicodeBuffer),
                                                 (ulI + 3));

              break;
            } /* endswitch */
          /************************************************************/
          /* Convert terms to NULL terminated strings                 */
          /************************************************************/
          if ( !usRC  )
          {

            pszSource = pszTarget = pUnicodeBuffer;

            while ( !usRC && (pszSource < (pUnicodeBuffer + ulNumOfWides)) )
            {
              pszTermStart = pszTarget;

              /********************************************************/
              /* skip whitespace at beginning of word                 */
              /********************************************************/
              while ( (pszSource < (pUnicodeBuffer + ulNumOfWides)) &&
                      ( ( *pszSource == BLANK ) ||
                        ( *pszSource == LF )    ||
                        ( *pszSource == CR ) ) )
              {
                pszSource++;
              } /* endwhile */

              /********************************************************/
              /* copy characters up to new line or end of text        */
              /********************************************************/
              while ( (pszSource < (pUnicodeBuffer + ulNumOfWides)) &&
                      (*pszSource != LF)  &&
                      (*pszSource != CR) )
              {
                *pszTarget++ = *pszSource++;
              } /* endwhile */

              /********************************************************/
              /* Remove trailing blanks from term                     */
              /********************************************************/
              while ( ( pszTarget > pszTermStart ) &&
                      ( pszTarget[-1] == BLANK ) )
              {
                pszTarget--;
              } /* endwhile */

              /********************************************************/
              /* Terminate current term (if any) and check length of  */
              /* term                                                 */
              /********************************************************/
              if ( pszTarget > pszTermStart )
              {
                *pszTarget++ = EOS;
                pszSource++;           // position to next term

                if (UTF16strlenCHAR( pszTermStart ) > MAX_TERM_LEN )
                {
                  UtlError( NO_VALID_FORMAT, MB_CANCEL, 1, &pszImportFile,
                            EQF_ERROR );
                  usRC = ERROR_INVALID_DATA;
                } /* endif */
              } /* endif */
            } /* endwhile */

            /**********************************************************/
            /* Terminate term buffer                                  */
            /**********************************************************/
            *pszTarget++ = EOS;
            *pszTarget++ = EOS;
          } /* endif */

          /************************************************************/
          /* Write list to disk                                       */
          /************************************************************/
          if ( !usRC  )
          {
            if ( !LstWriteExclList( pszListName, pUnicodeBuffer ) )
            {
              usRC = ERROR_INVALID_FUNCTION;
            } /* endif */
          } /* endif */

          if ( pUnicodeBuffer ) UtlAlloc( (PVOID *) &pUnicodeBuffer, 0L, 0L, NOMSG );
          if ( pTermBuffer ) UtlAlloc( (PVOID *) &pTermBuffer, 0L, 0L, NOMSG );



        }
          break;

        default:
          usRC = ERROR_INVALID_DATA;
      } /* endswitch */

      if ( !usRC )
      {
        if ( fExists )
        {
          EqfSend2AllHandlers( WM_EQFN_PROPERTIESCHANGED,
                               MP1FROMSHORT( PROP_CLASS_LIST ),
                               MP2FROMP(pszListName) );
        }
        else
        {
          EqfSend2AllHandlers( WM_EQFN_CREATED,
                               MP1FROMSHORT( clsLIST ),
                               MP2FROMP(pszListName) );
        } /* endif */
      } /* endif */
      break;

  } /* endswitch */

  return( usRC );

} /* end of function LstImportList */

//+----------------------------------------------------------------------------+
//|                          End of EQFLSTIE.C                                 |
//+----------------------------------------------------------------------------+
