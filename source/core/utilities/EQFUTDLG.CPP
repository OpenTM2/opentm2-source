//------------------------------------------------------------------------------
// EQFUTDLG.CPP                                                                   
//------------------------------------------------------------------------------
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|      Copyright (C) 1990-2015, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+

#define INCL_EQF_DLGUTILS         // dialog utilities
#define INCL_EQF_ANALYSIS         // analysis functions
#define INCL_EQF_TAGTABLE         // tagtable defines and functions
#define INCL_EQF_FOLDER           // folder list and document list functions
#define INCL_EQF_DLGUTILS         // dialog utilities
#define INCL_EQF_TP               // public translation processor functions

#include <eqf.h>                  // General Translation Manager include file

#undef _WPTMIF                         // we don't care about WP I/F
#include "eqfhelp.id"                  // help resource IDs
#include "eqfhlp1.h"                   // first part of help tables
#include "eqfmsg.htb"                          // message help table

#include <eqfutils.id>            // IDs used by dialog utilities
#include <eqftwb.id>              // IDs of other TWB dialogs
#include <eqfutpri.h>             // private defines
#include <eqfdoc01.id>             // private defines
#include <eqfdoc01.h>             // private defines
#include <eqfhlog.h>              // history log defines
#include <OTMFUNC.H>              // API calls

#include <time.h>
#include <io.h>
#include <process.h>
#include <string>
#include <vector>
#include <core\utilities\LogWriter.h>
#include "core\memory\memoryfactory.h"
#include "core\pluginmanager\PluginManager.h"
#include "core\pluginmanager\OtmMarkup.h"
#include "core\pluginmanager\OtmMarkupPlugin.h"
#include "core\morph\MorphFactory.h"
#include "markuppluginmapper.h"
#include "OtmDictionaryIF.H"


/**********************************************************************/
/* IDA for print destination dialog                                   */
/**********************************************************************/
typedef struct _UTL_DEST_DLG_IDA
{
  CHAR                  szDriveList[MAX_DRIVELIST+1];      // Valid drive letters
  CONTROLSIDA           ControlsIDA;                       // Set file dialog
                                                           // control IDA
  PSZ                   pszCallersPrintDest;               // ptr to caller's
                                                           // print destination
  CHAR                  szPrintDest[CCHMAXPATH];           // Print destination
} UTL_DEST_DLG_IDA, *PUTL_DEST_DLG_IDA;

INT_PTR CALLBACK UTLSETFILEDLG
(
HWND   hwnd,                        // handle of window
WINMSG msg,                         // type of message
WPARAM mp1,                         // first message parameter
LPARAM mp2                          // second message parameter
);

#ifndef _TQM

extern HELPSUBTABLE hlpsubtblRenameDictDlg[];
extern HELPSUBTABLE hlpsubtblRenameTMDlg[];
extern HELPSUBTABLE hlpsubtblRenameFolDlg[];

#endif

// rename functions located in other mdoules linked into EQFDLL.DLL
BOOL FolRenameFolder( PSZ pszOldAndNewName, BOOL fMsg );

//+----------------------------------------------------------------------------+
// Function name:  UtlControlsInit                                              
//+----------------------------------------------------------------------------+
// Description:  Initialize controls in import and export dialogs, e.g.         
//               dictionaries, markup tables, translation memories.             
//               Handles the setting of drive buttons, radio buttons,           
//               entry fields (also statics) and list boxes.                    
//               Either defaults or last used values.                           
//+----------------------------------------------------------------------------+
// Parameters:   HWND           hwndDlg        IN - dialog window handle        
//               PCONTROLSIDA   pControlsIda   IN - ida with dialog control     
//                                                  information                 
//+----------------------------------------------------------------------------+
// Returncode type: VOID                                                        
//+----------------------------------------------------------------------------+
// Returncodes: -                                                               
//+----------------------------------------------------------------------------+
// Prerequesites:                                                               
//     The following fields in CONTROLSIDA need to be filled before the         
//     function may be called:                                                  
//     IDs for import dialog:                                                   
//     (Set the following ids if the utility is used for an import function)    
//       idPathEF              with the id of path entry field                  
//       idDirLB               with the id of directories listbox               
//       idFilesLB             with the id of files listbox                     
//       idCurrentDirectoryEF  with id of current path static                   
//       idDriveBTN            with id of dummy drive button                    
//       idControlsGB          with id of group box around drives, EFs,         
//                             listboxes                                        
//       idInternalRB          with id of radio button for internal processing  
//       idExternalRB          with id of radio button for external processing  
//       idToLB                with id of combo box with filenames              
//       idOkPB                with the id of btn that triggers process         
//     IDs for export dialog:                                                   
//     (Set the following ids if the utility is used for an export function)    
//       idPathEF              with the id of path entry field                  
//       idDirLB               with the id of directories listbox               
//       idCurrentDirectoryEF  with id of current path static                   
//       idDriveBTN            with id of dummy drive button                    
//       idControlsGB          with id of group box around drives, EFs,         
//                             listboxes                                        
//       idInternalRB          with id of radio button for internal processing  
//       idExternalRB          with id of radio button for external processing  
//       idExportTEXT          with id of file for export                       
//       idOkPB                with the id of btn that triggers process         
//     Other types:                                                             
//     (Set the following if the utility is used for an export function         
//       szExt                 with the extension of the export file (including 
//                             the dot e.g. .SGM for exported dictionary files  
//     (Set the following if the utility is used for an import function         
//       szHandler             with name of processing handler e.g.             
//                             DICTIONARYHANDLER for dictionary processing.     
//     (Set the following if the utility is used for an import or export funct. 
//       szSelectedName        with name of file to import to or NULL,          
//                             or the name of the file to export (only filename 
//                             without extension)                               
//       szSavedPath           with fully qualified saved path (drive, path and 
//                             filename plus extension) (last used value).      
//                             If NULL then default handling                    
//       chSavedDrive          with saved drive (only drive letter)             
//                             (last used value).                               
//                             If NULL then default handling                    
//       usSavedFormat         with saved format (last used value)              
//                             E.g. INTERNAL (MAT processing) or EXTERNAL       
//                             (processing outside of MAT).                     
//                             If NULL then default handling                    
//       fImport               flag to indicate import/export dialog            
//                             TRUE if import, FALSE if export function         
//+----------------------------------------------------------------------------+
// Side effects:                                                                
//     IDA fields may be changed to reflect the new dialog status.              
//     szPathContent         contains the fully qualified path name             
//     szPath                contains drive and directory                       
//     szPatternName         contains file name plus extension                  
//     szDrive               contains drive with colon                          
//     The rest of the fields in the ida are used for control processing        
//+----------------------------------------------------------------------------+
// Function call: UltControlsInit( hwnd, &pIda.ControlsIda                      
//+----------------------------------------------------------------------------+
//+----------------------------------------------------------------------------+
// Samples:                                                                     
//   UtlControlsInit is called in WM_INITDLG of an export or import dialog      
//   after the parameters specified in "Prerequesites" have been set. E.g.      
//                                                                              
//   case ( WM_INITDLG ) :             //initialize and display dialogbox       
//     //fill CONTROLS structure with values needed                             
//     //call up utility to initialize dialog controls                          
//     WinPostMsg( hwndDlg,                                                     
//                 WM_EQF_INITIALIZE,                                           
//                 NULL,                                                        
//                 &pDimpIda->Controls );                                       
//                                                                              
//     mResult = (MRESULT)FALSE;                                                
//     break;                                                                   
//                                                                              
//   case WM_EQF_INITIALIZE:                                                    
//     //mp1 = NULL                                                             
//     //mp2 = Import dictionary ida                                            
//     //get access to ida                                                      
//     pDimpIda = (PDIMPIDA) WinQueryWindowULong( hwndDlg, QWL_USER );          
//                                                                              
//     UtlControlsInit( hwndDlg, &pDimpIda->Controls );                         
//     break;                                                                   
//+----------------------------------------------------------------------------+
// Function flow:                                                               
//   if usSavedFormat = NULL then                                               
//     set default = EXTERNAL                                                   
//                                                                              
//   if usSavedFormat = INTERNAL then                                           
//     set InternalRB active                                                    
//   else                                                                       
//     set ExternalRB active (endif)                                            
//                                                                              
//   Get available drives and put value in szDriveList                          
//                                                                              
//   Get position of dummy drive button and group box in which the drive        
//     button are positioned                                                    
//                                                                              
//   Create the drive buttons based on contents of szDriveList                  
//                                                                              
//   if the InternalRB exists                                                   
//     position drive buttons based on the id of the InternalRB                 
//   else                                                                       
//     position drive buttons with respect to the DESKTOP handle (endif)        
//                                                                              
//   set szDrive to system drive                                                
//                                                                              
//   if chSavedDrive has a last used drive value then                           
//     select this drive button                                                 
//     set szDrive to this value                                                
//   else                                                                       
//     set szSavedPath to NULL so that path gets reset to system default(endif) 
//                                                                              
//   Display selected drive                                                     
//                                                                              
//   if szSavedPath has no last used path set then                              
//     set CurrentDirectory entry field to system path                          
//                                                                              
//     if this is an import dialog then                                         
//       set the Name entry field to *.*                                        
//       set szPathContent to with file path information                        
//     else                                                                     
//       set the Name entry field to the export file name plus given            
//         extension                                                            
//       set szPathContent to with file path information (endif)                
//   else if szSavedPath has last used path                                     
//     split up path to obtain path and filename information                    
//                                                                              
//     if this is an export dialog then                                         
//       filename and extension is file for export and given export file        
//       extension                                                              
//     else                                                                     
//       filename and extension = *.* (endif)                                   
//                                                                              
//     set CurrentDirectory entry field to saved path                           
//                                                                              
//     set Name entry field to filename and extension as set in the if          
//       condition immidiately above this statement (endif)                     
//                                                                              
//   if this is an import dialog then                                           
//     set drop-down combo-box entry field length limit to 8 characters         
//                                                                              
//     fill combo-box with file names delivered by specified handler            
//                                                                              
//     if szSelectedName is set to a specified file then                        
//       select it in combo-box                                                 
//   else                                                                       
//     write file for export in static field (endif)                            
//                                                                              
//   fill the directories and files directories                                 
//                                                                              
//   if return code = error because drive not ready                             
//     set drive to system drive                                                
//     set path to system path and fill CurrentDirectory entry field            
//     fill directories and files listboxes (endif)                             
//                                                                              
//+----------------------------------------------------------------------------+

VOID UtlControlsInit( HWND hwndDlg,              //window handle
                      PCONTROLSIDA pControlsIda )  //pointer to controls ida
{
  USHORT      usId = 0;               //id of drive
  SHORT       sRc;                    //FillFileDir return code
  USHORT      usDrive;                //drive btn id

  OEMTOANSI( pControlsIda->szSelectedName );
  //select last used format button or default which is the external btn
  if ( !pControlsIda->usSavedFormat )  //no saved format
    pControlsIda->usSavedFormat = EXTERNAL;

  if ( pControlsIda->usSavedFormat == INTERNAL )
  {
    ENABLECTRL( hwndDlg, pControlsIda->idInternalRB, TRUE );
  }
  else
  {
    ENABLECTRL( hwndDlg, pControlsIda->idExternalRB, TRUE );
  } /* endif */

  //call utility to get available drives and save them
  UtlGetDriveList( (PBYTE)pControlsIda->szDriveList );

  //display available drives as icons
  if ( pControlsIda->idDriveBTN )
  {
    UtlDriveButtons( hwndDlg, pControlsIda->szDriveList, PID_DRIVEBUTTON_A,
                    WS_GROUP | WS_TABSTOP, WS_VISIBLE,
                    WinWindowFromID( hwndDlg, pControlsIda->idControlsGB ),
                    WinWindowFromID( hwndDlg, pControlsIda->idDriveBTN ),
                    WinWindowFromID( hwndDlg, pControlsIda->idPathEF ) );

    //set szDrive to default system drive
    if ( UtlQueryString(QST_PRIMARYDRIVE, pControlsIda->szDrive, MAX_DRIVE) )
      usId = IDFROMDRIVE( PID_DRIVEBUTTON_A, pControlsIda->szDrive[0] );
  } /* endif */

  if ( pControlsIda->szDefPattern[0] == EOS )
  {
    strcpy( pControlsIda->szDefPattern, DEFAULT_PATTERN );
  } /* endif */

  if ( pControlsIda->chSavedDrive )
  {
    //is saved drive still in list of available drives?
    if ( strchr( pControlsIda->szDriveList, pControlsIda->chSavedDrive ) )
    {
      //saved drive available so set drive btn
      usId = IDFROMDRIVE( PID_DRIVEBUTTON_A, pControlsIda->chSavedDrive );
      pControlsIda->szDrive[0] = pControlsIda->chSavedDrive;
      pControlsIda->szDrive[1] = COLON;
      pControlsIda->szDrive[2] = EOS;
    }
    else
    {
      //set szSavedPath to null so that it gets reset to root
      pControlsIda->szSavedPath[0] = NULC;

      // use primary drive as default drive
      UtlQueryString( QST_PRIMARYDRIVE, pControlsIda->szDrive,  MAX_DRIVE );
      pControlsIda->szDrive[1] = COLON;
      pControlsIda->szDrive[2] = EOS;
    } /* endif */
  }
  else
  {
    //set szSavedPath to null so that it gets reset to root
    pControlsIda->szSavedPath[0] = NULC;

    // use primary drive as default drive
    UtlQueryString( QST_PRIMARYDRIVE, pControlsIda->szDrive,  MAX_DRIVE );
    pControlsIda->szDrive[1] = COLON;
    pControlsIda->szDrive[2] = EOS;
  } /* endif */

  //display selected drive
  SETDRIVE( hwndDlg, usId, TRUE );

  //if no last used values use defaults - system drive and *.* as
  //pattern name
  if ( pControlsIda->szSavedPath[0] == NULC ) //nothing saved
  {
    //fill current directory static, default is system drive
    strcpy( pControlsIda->szPath, pControlsIda->szDrive );
    strcat( pControlsIda->szPath, BACKSLASH_STR );
    SETTEXT( hwndDlg, pControlsIda->idCurrentDirectoryEF, pControlsIda->szPath );

    //begin filling szPathContent with path info
    strcpy( pControlsIda->szPathContent, pControlsIda->szPath );

    if ( pControlsIda->fImport )
    {
      //fill name sle
      SETTEXT( hwndDlg, pControlsIda->idPathEF, pControlsIda->szDefPattern );
      strcat( pControlsIda->szPathContent, pControlsIda->szDefPattern );
    }
    else
    {
      //fill name sle with selected file plus given extension
      sprintf( pControlsIda->szPatternName, "%s%s",
               pControlsIda->szSelectedName, pControlsIda->szExt );
      SETTEXT( hwndDlg, pControlsIda->idPathEF, pControlsIda->szPatternName );

      //finish filling szPathContent with path info
      strcat( pControlsIda->szPathContent, pControlsIda->szPatternName );
    } /* endif */
  }
  else  //if last used values set
  {
    //obtain path and pattern name from fully qualified saved path
    //to be able to fill both entry fields on dialog panel
    UtlMakeFNameAndPath( pControlsIda->szSavedPath,
                         pControlsIda->szPath,         //path without fname
                         pControlsIda->szPatternName); //fname + ext
    if ( !pControlsIda->fImport )
    {
      //export dialog with selected filename plus extension
      strcpy( pControlsIda->szPatternName, pControlsIda->szSelectedName );
      strcat( pControlsIda->szPatternName, pControlsIda->szExt );
    }
    else
    {
      strcpy( pControlsIda->szPatternName, pControlsIda->szDefPattern );
    } /* endif */

    //fill current dir static
    SETTEXT( hwndDlg, pControlsIda->idCurrentDirectoryEF, pControlsIda->szPath );
    //fill path sle
    SETTEXT( hwndDlg, pControlsIda->idPathEF, pControlsIda->szPatternName );

    //fill szPathContent before FillFileDir is called
    strcpy( pControlsIda->szPathContent, pControlsIda->szPath );
    strcat( pControlsIda->szPathContent, pControlsIda->szPatternName );
  } /* endif */

  if ( pControlsIda->fImport )
  {
    //set drop-down combo box entry field length limit to eight chars
    if ( pControlsIda->fLongFileNames )
    {
      CBSETTEXTLIMIT( hwndDlg, pControlsIda->idToLB, (MAX_LONGFILESPEC - 1) );
    }
    else
    {
      CBSETTEXTLIMIT( hwndDlg, pControlsIda->idToLB, (MAX_FNAME - 1) );
    } /* endif */

    //fill drop-down combo-box with dicts or memories for e.g.
    EqfSend2Handler( pControlsIda->szHandler, WM_EQF_INSERTNAMES,
                     MP1FROMHWND( WinWindowFromID( hwndDlg, pControlsIda->idToLB )), 0L );

    //if dialog was called up with a file selected in the respective listbox
    //e.g. dictionary list or memory list, then activate selected file
    //name in entry field of drop-down combo box if szSelectedName is NULL
    //then entry field is empty
    if ( pControlsIda->szSelectedName[0] != NULC )
    {
      SETTEXT( hwndDlg, pControlsIda->idToLB, pControlsIda->szSelectedName );
    } /* endif */
  }
  else
  {
    //Display selected file for export in static field
    SETTEXT( hwndDlg, pControlsIda->idExportTEXT, pControlsIda->szSelectedName );

  } /* endif */

  //update files/directories listbox
  sRc = UtlFillFileDir( hwndDlg, pControlsIda, FALSE );

  //if listboxes could not be filled, e.g. drive not ready msg issued
  if ( sRc == UTLERROR )
  {
    //select system drive and refill directories and files listboxes
    //and name and current directory entry field

    //deselect drive that lead to error msg issued
    usDrive = IDFROMDRIVE( PID_DRIVEBUTTON_A, pControlsIda->szDrive[0]);
    SETDRIVE( hwndDlg, usDrive, FALSE );

    //select system drive as default
    if ( UtlQueryString(QST_PRIMARYDRIVE, pControlsIda->szDrive, MAX_DRIVE) )
      usId = IDFROMDRIVE( PID_DRIVEBUTTON_A, pControlsIda->szDrive[0] );

    //display selected drive
    SETDRIVE( hwndDlg, usId, TRUE );

    //fill current directory static, default is system drive
    strcpy( pControlsIda->szPath, pControlsIda->szDrive );
    strcat( pControlsIda->szPath, BACKSLASH_STR );
    SETTEXT( hwndDlg, pControlsIda->idCurrentDirectoryEF, pControlsIda->szPath );

    //fill szPathContent before FillFileDir is called
    strcpy( pControlsIda->szPathContent, pControlsIda->szPath );
    strcat( pControlsIda->szPathContent, pControlsIda->szPatternName );

    //update files/directories listbox
    UtlFillFileDir( hwndDlg, pControlsIda, TRUE );
  } /* endif */

  ANSITOOEM( pControlsIda->szSelectedName );

}  /* end UtlControlsInit */



MRESULT UtlDMGETDEFID
(
HWND             hwndDlg,            // dialog window handle
WPARAM           mp1,                // first message parameter
LPARAM           mp2,                // second message parameter
PCONTROLSIDA pControlsIda            // pointer to ida
)
{
  MRESULT     mResult = (MRESULT)FALSE; //return code of this function
  HWND        hwndFocus;                //handle for input focus

  mp1; mp2;
  /************************************************************/
  /* check if user pressed the ENTER key, but wants only to   */
  /* select/deselect an item of the listbox via a simulated   */
  /* (keystroke) double click or if the user is in the        */
  /* file name entry field and tries to activate a new        */
  /* search pattern                                           */
  /************************************************************/
  if ( GetKeyState(VK_RETURN) & 0x8000  )
  {
    hwndFocus = GetFocus();

    //UCD_ONE_PROPERTY
    if ( hwndFocus == GetDlgItem( hwndDlg, pControlsIda->idPathEF )  )
    {

      UtlEFValidityTest( pControlsIda, hwndDlg );

      mResult = TRUE;                  // do not process as default pushbutton
    }
    else if ( hwndFocus == GetDlgItem( hwndDlg, pControlsIda->idDirLB ) )
    {
	  UtlWMControls( hwndDlg, WM_CONTROL,
	                 pControlsIda->idDirLB,/*sId,*/
	                 LN_ENTER, /*sNotification, */
                     pControlsIda );
  //    PostMessage( hwndDlg, WM_COMMAND,
  //                 MP1FROMSHORT(pControlsIda->idDirLB),
  //                 MP2FROM2SHORT( 0, LN_ENTER ) );
      mResult = TRUE;                  // do not process as default pushbutton
    } /* endif */
  } /* endif */
  return( mResult );
} /* end of function UtlDMGETDEFID */

//+----------------------------------------------------------------------------+
// Function name:  UtlWMControls                                                
//+----------------------------------------------------------------------------+
// Description: Processes WM_CONTROL msg in import and export dialogs e.g.      
//              dictionaries, markup tables and translation memories.           
//              Drive button selection, handling of the name entry field namely 
//              adding path info to current directory static field and leaving  
//              file name plus extension in the name entry field. Filling list  
//              boxes with either directories and/or files.                     
//+----------------------------------------------------------------------------+
// Parameters:                                                                  
//     HWND           hwndDlg             IN      dialog window handle          
//     USHORT         msg                 IN      message id                    
//     MPARAM         mp1                 IN      first message parameter       
//     MPARAM         mp2                 IN      second message parameter      
//     PCONTROLSIDA   pControlsIda        IN      ida with dialog control info  
//+----------------------------------------------------------------------------+
// Returncode type: MRESULT                                                     
//+----------------------------------------------------------------------------+
// Returncodes: -                                                               
//+----------------------------------------------------------------------------+
// Prerequesites:                                                               
//     The following fields in CONTROLSIDA need to be filled before the         
//     function may be called:                                                  
//     idCurrentDirectoryEF  with the id of current directory entry field       
//     idPathEF              with the id of path entry field                    
//     idDirLB               with the id of directories listbox                 
//     idFilesLB             with the id of files listbox only for import       
//     idOkPB                with the id of btn that triggers process           
//     szDrive[_MAX_DRIVE]   with the drive letter and colon - either last      
//                           used value or default system drive                 
//+----------------------------------------------------------------------------+
// Side effects:                                                                
//     IDA fields may be changed to reflect the new dialog status.              
//     szPathContent         contains the fully qualified path name             
//     szPath                contains drive and directory                       
//     szPatternName         contains file name plus extension                  
//     szDrive               contains drive with colon                          
//     The rest of the fields in the ida are used for control processing        
//+----------------------------------------------------------------------------+
// Function call:  UtlWMControls( hwndDlg, msg, mp1, mp2, &pda->ControlsIda )   
//+----------------------------------------------------------------------------+
// Samples:                                                                     
//   UtlWMControls is called in WM_CONTROL of an export or import dialog.       
//   For example:                                                               
//   case ( WM_CONTROL ) :                                                      
//     //get access to ida                                                      
//     pDimpIda = (PDIMPIDA) WinQueryWindowULong( hwndDlg, QWL_USER );          
//                                                                              
//     UtlWMControls( hwndDlg, msg, mp1, mp2, &pDimpIda->Controls );            
//     mResult = (MRESULT)FALSE;                                                
//     break;                                                                   
//+----------------------------------------------------------------------------+
// Function flow:                                                               
//   if one of the drive buttons is selected then                               
//     deselect previously selected drive button if one was selected            
//     save newly selected drive to szDrive                                     
//     select newly selected drive button                                       
//                                                                              
//     if new drive then                                                        
//       add new drive to path                                                  
//       update CurrentDirectory entry field                                    
//                                                                              
//       if this is an import dialog then                                       
//         filename = *.*                                                       
//       else                                                                   
//         filename is composed of selected file for exported with given        
//           extension (endif)                                                  
//                                                                              
//       update Name entry field                                                
//       update szPathContent                                                   
//       fill the directories and files directories                             
//                                                                              
//       if return code = error because drive not ready then                    
//         set drive to system drive                                            
//         set path to system path and fill CurrentDirectory entry field        
//         fill directories and files listboxes (endif)                         
//     (endif)                                                                  
//   else                                                                       
//     if selection in directories listbox then                                 
//       if enter or double click then                                          
//         get index of selected item in listbox                                
//         if index != none then                                                
//           remember selected directory in szString                            
//           get path from CurrentDirectory entry field and put in szPath       
//                                                                              
//           if selected item in listbox is .. then                             
//             go back one directory level                                      
//             update CurrentDirectory entry field                              
//                                                                              
//             if this is an import dialog then                                 
//               if the filename does not contain either ? or * then            
//                 set szPatternName to *.*  (endif)                            
//             update Name entry field                                          
//             update szPathContent with fully qualified file path              
//           else                                                               
//             a directory has been selected                                    
//             update szPath with new directory                                 
//                                                                              
//             if this is an import dialog then                                 
//               if the filename does not contain either ? or * then            
//                 set szPatternName to *.*  (endif)                            
//                                                                              
//             update CurrentDirectory entry field                              
//             update szPathContent with fully qualified file path              
//           else                                                               
//             if files listbox is a single selection listbox and a file has    
//             been selected                                                    
//               update szPatternName and add new value to Name entry field     
//               update szPathContent                                           
//             else                                                             
//               enter or double click on selected file in files listbox        
//               continue processing (endif)                                    
//           else                                                               
//             default handling (endif)                                         
//   (endif)                                                                    
//+----------------------------------------------------------------------------+

MRESULT UtlWMControls
(
HWND hwndDlg,                        //dialog handle
WINMSG msg,                          //message id
SHORT  sID,                          // id in action
SHORT  sNotification,                // notification
PCONTROLSIDA pControlsIda            //ida with dialog ctl info
)
{
  SHORT       sIndexItem;                  //index for list box items
  PSZ         pszPointer;                  //pointer to string with dir info
  USHORT      usRc = TRUE;                 //process return code
  SHORT       sDrive = 0;                  //selected drive
  MRESULT     mResult = (MRESULT)FALSE;    //function return value
  SHORT       sRc;                         //FillFileDir return code
  USHORT      usAttribute;                       //Dos return info
  BOOL        fOK = TRUE;
  static CHAR        szCurPath[1024];

  msg;                                     // get rid of compiler warnings

  if ( sNotification == BN_CLICKED &&
       sID >= ( PID_DRIVEBUTTON_A ) &&  //one of the drive buttons is
       sID <= PID_DRIVEBUTTON_Z)        //clicked
  {
    //deselect drive button if one was selected
    //szDrive is passed into funct and is set to last used
    //drive value or default system drive
    if ( pControlsIda->szDrive[0] != EOS )
    {
      sDrive = IDFROMDRIVE( PID_DRIVEBUTTON_A, pControlsIda->szDrive[0]);
      SETDRIVE( hwndDlg, sDrive, FALSE );
    }

    //save newly selected drive character in ida and select it
    pControlsIda->szDrive[0] = DRIVEFROMID( PID_DRIVEBUTTON_A, sID );
    SETDRIVE( hwndDlg, sID, TRUE );

    if ( sID != sDrive )             //drive changed - reset path
    {
      //set szPath to to drive value in szDrive passed into function and
      //add terminating backslash
      strcpy( pControlsIda->szPath, pControlsIda->szDrive );
      strcat( pControlsIda->szPath, BACKSLASH_STR );

      //update current directory entry field with new drive
      SETTEXT( hwndDlg, pControlsIda->idCurrentDirectoryEF, pControlsIda->szPath );
      // cv
      SETTEXT( hwndDlg, pControlsIda->idCurrentDirectoryEF+1, pControlsIda->szPath );

      if ( pControlsIda->fImport )
      {
        //fill path entry field, default is *.*
        strcpy( pControlsIda->szPatternName, pControlsIda->szDefPattern );
      }
      else
      {
        //check what's been entered in name sle and update accordingly

        //load content of name entryfield into szPathContent
        QUERYTEXT( hwndDlg, pControlsIda->idPathEF, pControlsIda->szString );

        if ( pControlsIda->szString[0] == NULC )
        {
          //check if file name with extension is a valid OS/2 name
          strcpy( pControlsIda->szPatternName, DEFAULT_PATTERN );
        }
        else
        {
          //get path and pattern from pControlsIda->szPathContent
          UtlMakeFNameAndPath( pControlsIda->szString,
                               pControlsIda->szDummy, //path without fname
                               pControlsIda->szPatternName); //fname + ext

          //check if file name with extension is a valid OS/2 name
          strcpy( pControlsIda->szPathContent, pControlsIda->szPath );
          strcat( pControlsIda->szPathContent, pControlsIda->szPatternName );

          usRc = UtlQFileMode( pControlsIda->szPathContent, &usAttribute,
                               0L, FALSE );

          if ( usRc == ERROR_INVALID_NAME ||
               usRc == ERROR_FILENAME_EXCED_RANGE )
          {
            UtlError( INVALID_OS2_FILENAME, MB_CANCEL, 0, NULL, EQF_ERROR );
            fOK = FALSE;

            //deselect newly selected drive
            SETDRIVE( hwndDlg, sID, FALSE );

            //set drive btn back to what it was
            SETDRIVE( hwndDlg, sDrive, TRUE );

            //remember selected drive
            pControlsIda->szDrive[0] = DRIVEFROMID( PID_DRIVEBUTTON_A,
                                                    sDrive );

            strcpy( pControlsIda->szPath, pControlsIda->szDrive );
            strcat( pControlsIda->szPath, BACKSLASH_STR );

            //update current directory entry field with old drive
            SETTEXT( hwndDlg, pControlsIda->idCurrentDirectoryEF,
                     pControlsIda->szPath );

            SETFOCUS( hwndDlg, pControlsIda->idPathEF );
          } /* endif */
        } /* endif */
      } /* endif */

      if ( fOK )
      {
        //update name entry field with name plus extension
        SETTEXT( hwndDlg,pControlsIda->idPathEF,
                 pControlsIda->szPatternName );

        //build szFindPath for FillFileDir
        strcpy( pControlsIda->szPathContent, pControlsIda->szPath );
        strcat( pControlsIda->szPathContent, pControlsIda->szPatternName );

        //update the directory/files listboxes
        sRc = UtlFillFileDir( hwndDlg, pControlsIda, TRUE );

        //if listboxes could not be filled, e.g. drive not ready msg issued
        if ( sRc == UTLERROR )
        {
          //select system drive and refill directories and files listboxes
          //and name and current directory entry field

          //deselect drive that issues error msg
          sDrive = IDFROMDRIVE( PID_DRIVEBUTTON_A, pControlsIda->szDrive[0]);
          SETDRIVE( hwndDlg, sDrive, FALSE );

          //select system drive as default
          if ( UtlQueryString( QST_PRIMARYDRIVE, pControlsIda->szDrive,
                               MAX_DRIVE) )
            sID = IDFROMDRIVE( PID_DRIVEBUTTON_A, pControlsIda->szDrive[0] );

          //display selected drive
          SETDRIVE( hwndDlg, sID, TRUE );

          //fill current directory static, default is system drive
          strcpy( pControlsIda->szPath, pControlsIda->szDrive );
          strcat( pControlsIda->szPath, BACKSLASH_STR );
          SETTEXT( hwndDlg, pControlsIda->idCurrentDirectoryEF,
                   pControlsIda->szPath );
          // cv
          SETTEXT( hwndDlg, pControlsIda->idCurrentDirectoryEF+1,
                   pControlsIda->szPath );

          //fill szPathContent before FillFileDir is called
          strcpy( pControlsIda->szPathContent, pControlsIda->szPath );
          strcat( pControlsIda->szPathContent, pControlsIda->szPatternName );

          //update files/directories listbox
          UtlFillFileDir( hwndDlg, pControlsIda, TRUE );
        } /* endif */
      } /* endif */
    } /* endif */
  }
  else if (sID == (SHORT)pControlsIda->idDirLB )
  //selection in Directories listbox
  {
    if ( sNotification == LN_ENTER )
    {
      //check if pathef is empty and if so enter *.*
      QUERYTEXT( hwndDlg, pControlsIda->idPathEF, pControlsIda->szString );

      if ( pControlsIda->szString[0] == NULC )
      {
        //id empty add *.* and enter intp pathEF
        if ( pControlsIda->fImport )
        {
          strcpy( pControlsIda->szPatternName, pControlsIda->szDefPattern );
        }
        else
        {
          strcpy( pControlsIda->szPatternName, DEFAULT_PATTERN );
        } /* endif */

        strcpy( pControlsIda->szPathContent, pControlsIda->szPath );
        strcat( pControlsIda->szPathContent, pControlsIda->szPatternName );
        SETTEXT( hwndDlg,pControlsIda->idPathEF, pControlsIda->szPatternName );
      } /* endif */

      //get selected item index of selected item in Directories listbox
      sIndexItem = QUERYSELECTION( hwndDlg, pControlsIda->idDirLB );
      if ( sIndexItem != LIT_NONE )
      {
        //load name of selected item in Directories listbox into szString
        QUERYITEMTEXT( hwndDlg, pControlsIda->idDirLB,
                       sIndexItem, pControlsIda->szString );

        //get current directory netry field setting
        QUERYTEXT( hwndDlg, pControlsIda->idCurrentDirectoryEF,
                   pControlsIda->szPath );

        if ( strcmp( pControlsIda->szString, PARENT_DIR_NAME ) == 0)
        {
          //Go back one directory level
          pszPointer = strrchr( pControlsIda->szPath, BACKSLASH);
          if (pszPointer)
          {
            *pszPointer = EOS;
            pszPointer = strrchr( pControlsIda->szPath, BACKSLASH);
            if ( pszPointer )
            {
              pszPointer++;
              *pszPointer = EOS; //add end of string indication
            }
            else
            {
              pszPointer = EMPTY_STRING;
            } /* endif */
          }
          else
          {
            pszPointer = EMPTY_STRING;
          } /* endif */

          if ( pControlsIda->fImport )
          {
            //if szpatternname does not contain either ? or *
            if ( !strchr( pControlsIda->szPatternName,
                          SINGLE_SUBSTITUTION )  &&
                 !strchr( pControlsIda->szPatternName,
                          MULTIPLE_SUBSTITUTION ) )
            //set szPatternName to *.*
            {
              strcpy( pControlsIda->szPatternName, pControlsIda->szDefPattern );
              SETTEXT( hwndDlg,pControlsIda->idPathEF, pControlsIda->szDefPattern );
            } /* endif */
          } /* endif */

          //update current directory entry field
          SETTEXT( hwndDlg, pControlsIda->idCurrentDirectoryEF,
                   pControlsIda->szPath );
          // cv
          SETTEXT( hwndDlg, pControlsIda->idCurrentDirectoryEF+1,
                   pControlsIda->szPath );

          //fill szPatternName correctly for FillFileDir
          strcpy( pControlsIda->szPathContent, pControlsIda->szPath );
          strcat( pControlsIda->szPathContent,
                  pControlsIda->szPatternName );

        }
        else
        {
          //Selected item text is not ".." which means that a
          //directory is selected. Save directory in szPath and
          //add terminating backslash. Set szPatternName to *.*
          //and add the values of szPath and szPatternName to
          //szpathContent
          strcat( pControlsIda->szPath, pControlsIda->szString );
          strcat( pControlsIda->szPath, BACKSLASH_STR );

          if ( pControlsIda->fImport )
          {
            //if szpatternname does not contain either ? or *
            if ( !strchr( pControlsIda->szPatternName,
                          SINGLE_SUBSTITUTION )  &&
                 !strchr( pControlsIda->szPatternName,
                          MULTIPLE_SUBSTITUTION ) )
              strcpy( pControlsIda->szPatternName, pControlsIda->szDefPattern );

            //update path entry field with value in szPatternName
            SETTEXT( hwndDlg, pControlsIda->idPathEF,
                     pControlsIda->szPatternName );
          } /* endif */

          strcpy( pControlsIda->szPathContent, pControlsIda->szPath );
          strcat( pControlsIda->szPathContent,
                  pControlsIda->szPatternName );

          //update current dir static
          // cv
          // shorten path to avoid truncation
          if (strlen(pControlsIda->szPath) > 30)
          {
            strcpy( szCurPath, pControlsIda->szDrive );
            strcat( szCurPath, BACKSLASH_STR );
            strcat( szCurPath, "...");
            strcat( szCurPath, BACKSLASH_STR );
            strcat( szCurPath, pControlsIda->szString);
            strcat( szCurPath, BACKSLASH_STR );

            SETTEXT( hwndDlg, pControlsIda->idCurrentDirectoryEF+1,
                     szCurPath );
          }
          else
          {
            SETTEXT( hwndDlg, pControlsIda->idCurrentDirectoryEF+1,
                     pControlsIda->szPath );
          }
          SETTEXT( hwndDlg, pControlsIda->idCurrentDirectoryEF,
                   pControlsIda->szPath );
        }/*end if*/

        //fill dir listbox
        UtlFillFileDir( hwndDlg, pControlsIda, TRUE );
      } /* endif */
    } /*endif*/
  }
  else if (sID == (SHORT)pControlsIda->idFilesLB )
  {
    //a file has been selected in a single selection list box
    if ( sNotification == LN_SELECT )
    {
      if ( pControlsIda->fMultiSelectionLB )
      {
        //get selected item index of selected item in files listbox
        int iSelItems = SendDlgItemMessage( hwndDlg, pControlsIda->idFilesLB,
                                            LB_GETSELCOUNT, 0L, 0L );
        if ( iSelItems > 1 )
        {
          // hide TO combobox
          if ( pControlsIda->fImport ) HIDECONTROL( hwndDlg, pControlsIda->idToLB );
        }
        else
        {
          // show TO combobox
          if ( pControlsIda->fImport ) SHOWCONTROL( hwndDlg, pControlsIda->idToLB );

          sIndexItem = QUERYNEXTSELECTION( hwndDlg,
                                           pControlsIda->idFilesLB, LIT_FIRST );
          if ( sIndexItem != LIT_NONE )
          {
            //load name of selected item in files listbox into
            //szPatternname
            QUERYITEMTEXT( hwndDlg, pControlsIda->idFilesLB,
                           sIndexItem, pControlsIda->szPatternName );

            //update path entry field to contain the selected file
            SETTEXT( hwndDlg, pControlsIda->idPathEF,
                     pControlsIda->szPatternName );

            //update szPathContent to include newly selected filename
            strcpy( pControlsIda->szPathContent, pControlsIda->szPath );
            strcat( pControlsIda->szPathContent,
                    pControlsIda->szPatternName );

            if ( pControlsIda->fImport )
            {
              PSZ pszExt = strrchr( pControlsIda->szPatternName, DOT );
              if ( pszExt != NULL )
              {
                *pszExt = EOS;
              } /* endif */
              SETTEXT( hwndDlg, pControlsIda->idToLB,
                       pControlsIda->szPatternName );
            } /* endif */
          } /* endif */
        } /* endif */
      }
      else
      {
        //get selected item index of selected item in files listbox
        sIndexItem = QUERYSELECTION( hwndDlg,
                                     pControlsIda->idFilesLB );
        if ( sIndexItem != LIT_NONE )
        {
          //load name of selected item in files listbox into
          //szPatternname
          QUERYITEMTEXT( hwndDlg, pControlsIda->idFilesLB,
                         sIndexItem, pControlsIda->szPatternName );

          //update path entry field to contain the selected file
          SETTEXT( hwndDlg, pControlsIda->idPathEF,
                   pControlsIda->szPatternName );

          //update szPathContent to include newly selected filename
          strcpy( pControlsIda->szPathContent, pControlsIda->szPath );
          strcat( pControlsIda->szPathContent,
                  pControlsIda->szPatternName );

          if ( pControlsIda->fImport )
          {
            PSZ pszExt = strrchr( pControlsIda->szPatternName, DOT );
            if ( pszExt != NULL )
            {
              *pszExt = EOS;
            } /* endif */
            SETTEXT( hwndDlg, pControlsIda->idToLB,
                     pControlsIda->szPatternName );
          } /* endif */
        } /* endif */
      } /* endif */
    }
    else
    {
      //double click on selected file - continue processing
      if ( sNotification == LN_ENTER )
      {
        WinPostMsg( hwndDlg,
                    WM_COMMAND,
                    MP1FROMSHORT( pControlsIda->idOkPB ),
                    NULL );
      } /* endif */
    } /* endif */
  }
  else if ( (sID == ID_UTL_SETFILE_NAME_EF) &&
            (sNotification == EN_KILLFOCUS) )
  {
    ClearIME( hwndDlg );
  } /* endif */
  return( mResult );
} /* end of function DialogControls */

//+----------------------------------------------------------------------------+
// Function name:  UtlEFValidityTest                                            
//+----------------------------------------------------------------------------+
// Description:  Process double click or enter on the name entry field          
//               that can contain a fully or partly qualified pathname.         
//               Separate the contents of the EF out into path and pattern      
//               name. Check validity of both and possibly add defaults         
//               (e.g. *.*) or missing backslashes or drive indication.         
//+----------------------------------------------------------------------------+
// Parameters:                                                                  
//     HWND           hwndDlg             IN      dialog window handle          
//     PCONTROLSIDA   pControlsIda        IN      ida with dialog control info  
//+----------------------------------------------------------------------------+
// Returncode type:  BOOLEAN                                                    
//+----------------------------------------------------------------------------+
// Returncodes:  -  TRUE                                                        
//                  FALSE                                                       
//+----------------------------------------------------------------------------+
//  Prerequesites:                                                              
//     The following fields in CONTROLSIDA need to be filled before the         
//     function may be called:                                                  
//     IDs:                                                                     
//     idCurrentDirectoryEF  with the id of current directory entry field       
//     idPathEF              with the id of path entry field                    
//     idDirLB               with the id of directories listbox                 
//     idFilesLB             with the id of files listbox (only import)         
//     STRINGs:                                                                 
//     szDrive[_MAX_DRIVE]   with the drive letter and colon - set in ida       
//                           automatically through controls processing          
//+----------------------------------------------------------------------------+
// Side effects:                                                                
//     IDA fields may be changed to reflect the new dialog status.              
//     szPathContent         contains the fully qualified path name             
//     szPath                contains drive and directory                       
//     szPatternName         contains file name plus extension                  
//     szDrive               contains drive with colon                          
//     The rest of the fields in the ida are used for control processing        
//+----------------------------------------------------------------------------+
// Function call:  UtlEFValidityTest( pControlsIda, hwndDlg );                  
//+----------------------------------------------------------------------------+
// Samples:                                                                     
//   UtlEFValidityTest is called in UtlWMChar as follows:                       
//                                                                              
//   if ( hwndFocus )                                                           
//   {                                                                          
//      usFocusID = WinQueryWindowUShort( hwndFocus, QWS_ID );                  
//                                                                              
//      if ( usFocusID == pControlsIda->idPathEF ) //Path entryfield has        
//      {                                          //input focus                
//         UtlEFValidityTest( pControlsIda, hwndDlg );                          
//      }                                                                       
//      else                                                                    
//      {                                                                       
//         if ( ( usFocusID != pControlsIda->idDirLB ) ||                       
//              ( usFocusID != pControlsIda->idFilesLB ) )                      
//         {                                                                    
//            WinPostMsg( hwndDlg,                                              
//                        WM_COMMAND,                                           
//                        MPFROMSHORT( pControlsIda->idOkPB ),                  
//                        NULL );                                               
//         } /* endif */                                                        
//         else                                                                 
//         {                                                                    
//            mResult = UTLDEFDIALOGPROC( hwndDlg, msg, mp1, mp2 );             
//         }  /* endif */                                                       
//      } /* endif */                                                           
//   } /* endif */                                                              
//                                                                              
//+----------------------------------------------------------------------------+
// Function flow:                                                               
//   load contents of Name entry field into szPathContent                       
//   load contents of CurrentDirectory entry field into szPath                  
//   split contents ofNmae entry field into path and filename plus extension    
//                                                                              
//   if there is a filename then                                                
//     split into filename and extension                                        
//     if there is an extension then                                            
//       save this to szExt                                                     
//     else                                                                     
//       there is no file extension so check if path includes a drive           
//       specification                                                          
//       if there is no drive in path then                                      
//         add drive from CurrentDirectory entry field                          
//         update szpathContent with fully qualified path (endif)               
//                                                                              
//       test if contents of szPathContent is valid                             
//       if name after the last backslash is a valid directory then             
//         if this is an import dialog then                                     
//           update szPathContent by adding a backslash behind valid directory  
//           and *.* as filename specification                                  
//         else export dialog                                                   
//           update szPathContent by adding a backslash behind valid directory  
//           and file for export plus given extension as filename               
//           specification (endif)                                              
//                                                                              
//         split szPathContent into path and filename information               
//       else                                                                   
//         not a valid directory so query user if he wants to add directory     
//         (endif)                                                              
//     (endif)                                                                  
//   else no filename                                                           
//     if this is an import dialog                                              
//       update filename (szPatternName) to *.*                                 
//     else                                                                     
//       filename is file for export plus given extension (endif)               
//                                                                              
//     check if terminating backslash needs to be added to path information     
//     and whether drive specifications have been added to path                 
//                                                                              
//     update szPathContent with fully qualified file path (endif)              
//                                                                              
//   if file path exists (szPathContent is valid) then                          
//     update drive button setting on dialog                                    
//     update CurrentDirectory entry field on dialog                            
//     update Name entry field on dialog                                        
//     update files and directories list boxes (which ever apply depending      
//                                              on dialog)                      
//     if update successfully then                                              
//       continue processing (importing or exporting) (endif)                   
//   (endif)                                                                    
//+----------------------------------------------------------------------------+

BOOL UtlEFValidityTest ( PCONTROLSIDA pControlsIda, HWND hwndDlg )
{
  USHORT      usRc = TRUE;                 //process return code
  USHORT      usId = 0;                    //selected drive id
  USHORT      usDrive;                     //selected drive
  USHORT      usAttribute;                 //return value of DosQFileMode
  FILEFINDBUF ResultBuf;                   //DOS file find struct
  HDIR        hDirHandle = HDIR_CREATE;    //DosFind routine handle
  PSZ         pszExt;                      //pointer to file extension
  SHORT       sRc;                         //FillFileDir return code
  PSZ         pszError;                    //pointer to error string
  PSZ         pszPath;                     //pointer to path string
  BOOL        fOK = TRUE;                  //success indicator
  SHORT       sItem;                       //selected listbox item
  USHORT      usAction;
  HFILE       hFile;


  //load content of name entryfield into szPathContent
  QUERYTEXT( hwndDlg, pControlsIda->idPathEF, pControlsIda->szPathContent );

  //load content of current dir entryfield into szPath
  QUERYTEXT( hwndDlg, pControlsIda->idCurrentDirectoryEF, pControlsIda->szPath );

  if (pControlsIda->szPath[strlen(pControlsIda->szPath)-1] != '\\')
  {
    strcat(pControlsIda->szPath,"\\");
  }


  //get path and pattern from pControlsIda->szPathContent
  UtlMakeFNameAndPath( pControlsIda->szPathContent,
                       pControlsIda->szDummy, //path without fname
                       pControlsIda->szPatternName); //fname + ext

  //is szPatterName empty?
  if ( pControlsIda->szPatternName[0] != NULC )
  {
    //szPatterName is not an empty string
    //get file name(szName) and extension(szExt)
    Utlstrccpy( pControlsIda->szName,
                pControlsIda->szPatternName,
                DOT );
    pszExt = strrchr( pControlsIda->szPatternName , DOT );

    if ( pszExt )
    {
      //there is a file extension so set szExt to this value
      strcpy( pControlsIda->szExt, pszExt );
    }
    else
    {
      if ( pControlsIda->szPathContent[2] != EOS )
      {
        //no * or ?
        if ( !strchr( pControlsIda->szPathContent,
                      SINGLE_SUBSTITUTION )  &&
             !strchr( pControlsIda->szPathContent,
                      MULTIPLE_SUBSTITUTION ) )
        {
          //there is no file extension
          //check if Path includes a drive specification
          if ( pControlsIda->szPathContent[1] != COLON )
          {
            //no drive in szPathContent so add drive set
            //in CurrentDirectoryEF
            //that is currently stored in szPath
            strcpy( pControlsIda->szPathContent, pControlsIda->szPath );
            strcat( pControlsIda->szPathContent, pControlsIda->szPatternName );
          } /* endif */

          //test validity of contents of szPathContent
          usRc = UtlQFileMode( pControlsIda->szPathContent,
                               &usAttribute,
                               0L,
                               FALSE );
          if ( !usRc )
          {
            //if name after last backslash in szPathContent is a
            //valid directory name then add missing backslash after
            //directory name and add *.*
            if ( usAttribute == FILE_DIRECTORY )
            {
              if ( pControlsIda->fImport )
              {
                sprintf( pControlsIda->szString, "%s\\%s",
                         pControlsIda->szPathContent,
                         pControlsIda->szDefPattern );
              }
              else
              {
                sprintf( pControlsIda->szString, "%s\\%s%s",
                         pControlsIda->szPathContent,
                         pControlsIda->szSelectedName,
                         pControlsIda->szExt );
              } /* endif */

              //set szPathContent to newly specified path
              strcpy ( pControlsIda->szPathContent,
                       pControlsIda->szString );

              //get path and pattern from szPathContent
              UtlMakeFNameAndPath( pControlsIda->szPathContent,
                                   pControlsIda->szPath,   //path without fname
                                   pControlsIda->szPatternName);        //fname + ext
            } /* endif */
          }
          else
          {
            if ( pControlsIda->fImport )
            {
              fOK = FALSE;
              pszError = pControlsIda->szPathContent;
              UtlErrorHwnd( ERROR_FILENAME_NOT_VALID,
                            MB_CANCEL,
                            1,
                            &pszError,
                            EQF_ERROR,
                            hwndDlg );
            } /* endif */
          } /* endif */
        } /* endif */
      } /* endif */
    }  /* endif */
  }
  else
  {
    //szPatternName is an empty string so set to *.*
    if ( pControlsIda->fImport )
    {
      strcpy( pControlsIda->szPatternName, pControlsIda->szDefPattern );
    }
    else
    {
      strcpy( pControlsIda->szPatternName, pControlsIda->szSelectedName );
      strcat( pControlsIda->szPatternName, pControlsIda->szExt );
    } /* endif */

    //if szDummy is not an empty string, i.e. there was
    //a path specification in idPathEF, check if end
    //backslash needs to be added
    if ( pControlsIda->szDummy[0] != NULC )
    {
      //If no terminating backslash add to szDummy
      if ( pControlsIda->szDummy[strlen(pControlsIda->szDummy)-1]
           != BACKSLASH )
        pControlsIda->szDummy[strlen(pControlsIda->szDummy)-1] =
        BACKSLASH;
    } /* endif */
  } /* endif */

  //check if path is valid, if not issue msg otherwise
  //update fields on dialog panel and trigger import function

  //if szDummy empty then update pathcontent with contents
  //of szPath from static dir entry field
  if ( pControlsIda->szDummy[0] == NULC && fOK )
  {
    //if patternname contains a colon then don't add path as patternname
    //is something like c:matdict.sgm
    if ( pControlsIda->szPatternName[1] != COLON )
    {
      strcpy( pControlsIda->szPathContent, pControlsIda->szPath );
      strcat( pControlsIda->szPathContent, pControlsIda->szPatternName );
    }
    else //update entry fields correctly
    {
      pControlsIda->szPath[0] = pControlsIda->szPatternName[0];
      pControlsIda->szPath[1] = COLON;
      pControlsIda->szPath[2] = BACKSLASH;
      pControlsIda->szPath[3] = EOS;
      pszPath = strrchr( pControlsIda->szPatternName, COLON );
      if ( *(++pszPath) != EOS )
      {
        strcpy( pControlsIda->szPatternName, pszPath );
      }
      else
      {
        if ( pControlsIda->fImport )
        {
          strcpy( pControlsIda->szPatternName, pControlsIda->szDefPattern );
        }
        else
        {
          strcpy( pControlsIda->szPatternName, pControlsIda->szSelectedName );
          strcat( pControlsIda->szPatternName, pControlsIda->szExt );
        } /* endif */
      } /* endif */
      strcpy( pControlsIda->szPathContent, pControlsIda->szPath );
      strcat( pControlsIda->szPathContent, pControlsIda->szPatternName );
    } /* endif */
  }
  else
  {
    //if szDummy begins with a backslash drop it
    if ( pControlsIda->szDummy[0] == BACKSLASH )
    {
      pszPath = pControlsIda->szDummy;
      pszPath++;
      strcpy( pControlsIda->szDummy, pszPath );
    } /* endif */

    //szdummy doesn't include a drive indication so necessary
    //to udate szPath to include drive specifivation to
    //later update idCurrentDirectoryEF with this value
    if ( pControlsIda->szDummy[1] != COLON )
    {
      strcat( pControlsIda->szPath, pControlsIda->szDummy );

      strcpy( pControlsIda->szPathContent, pControlsIda->szPath );
      strcat( pControlsIda->szPathContent, pControlsIda->szPatternName );
    }
    else
    {
      //set szpath to value of szdummy
      strcpy( pControlsIda->szPath, pControlsIda->szDummy );

      strcpy( pControlsIda->szPathContent, pControlsIda->szPath );
      strcat( pControlsIda->szPathContent, pControlsIda->szPatternName );
    } /* endif */
  } /* endif */

  if ( fOK )
  {
    if ( !UtlCheckPath( pControlsIda->szPathContent, CHKPATH_WILDCARDS, NULL ) )
    {
      fOK = FALSE;
      pszError = pControlsIda->szPathContent;
      UtlErrorHwnd( ERROR_FILENAME_NOT_VALID,
                    MB_CANCEL,
                    1,
                    &pszError,
                    EQF_ERROR,
                    hwndDlg );
    } /* endif */
  } /* endif */

  if ( fOK )
  {
    //if export, check that file path isn't eqf
    //system drive and directory to make sure that
    //that the sgml file is not export onto the system directories
    UtlMakeEQFPath( pControlsIda->szDummy, NULC, SYSTEM_PATH, (PSZ) NULP );
    strcat( pControlsIda->szDummy, BACKSLASH_STR );

    // Allow export/import from/to EQF directories .. so the following code has
    // been disabled

#ifdef DO_NOT_ALLOW_EQF_DIRECTORIES
    //check that nothing is copied onto system directories
    if ( !strnicmp( pControlsIda->szDummy+2, pControlsIda->szPathContent+2,
                    strlen( pControlsIda->szDummy )-2 ) ) //omit drive button
    {
      pszError = pControlsIda->szPathContent;
      UtlError( ERROR_EQF_PATH_INVALID, MB_CANCEL,
                1, &pszError, EQF_ERROR );
      fOK = FALSE;

      //if drive new, change drive btn setting
      if ( toupper(pControlsIda->szDrive[0]) != toupper(pControlsIda->szPath[0]) )
      {
        //deselect drive button if one selected
        usDrive = IDFROMDRIVE( PID_DRIVEBUTTON_A,
                               pControlsIda->szDrive[0]);
        SETDRIVE( hwndDlg, usDrive, FALSE );

        //a new drive has been entered so set drive btn
        //to new drive in path entry field
        usId = IDFROMDRIVE( PID_DRIVEBUTTON_A,
                            pControlsIda->szPath[0] );
        SETDRIVE( hwndDlg, usId, TRUE );

        //set szDrive to new drive
        pControlsIda->szDrive[0] = pControlsIda->szPath[0];
      } /* endif */

      strcpy( pControlsIda->szPath, pControlsIda->szDrive );
      strcat( pControlsIda->szPath, BACKSLASH_STR );
      strcpy( pControlsIda->szPathContent, pControlsIda->szPath );
      strcat( pControlsIda->szPathContent, pControlsIda->szPatternName );

      //update current directory entry field with old drive
      SETTEXT( hwndDlg, pControlsIda->idCurrentDirectoryEF,
               pControlsIda->szPath );

      //update current directory entry field with old drive
      SETTEXT( hwndDlg, pControlsIda->idPathEF,
               pControlsIda->szPatternName );

      //update files and dir listboxes
      sRc = UtlFillFileDir( hwndDlg, pControlsIda, TRUE );

      SETFOCUS( hwndDlg, pControlsIda->idPathEF );
    }/* endif */
#endif

    if ( fOK )
    {
      //check whether device or pipe and if so disallow
      usRc = UtlOpen( pControlsIda->szPathContent,
                      &(hFile), &usAction,
                      0L, FILE_NORMAL,
                      FILE_OPEN,
                      OPEN_ACCESS_READWRITE |
                      OPEN_SHARE_DENYWRITE,
                      0L, FALSE );

      if ( hFile )
        UtlClose( hFile, FALSE );
    } /* endif */


    if ( fOK )
    {
      //does path exist, i.e. is the path plus filename valid
//       usRc = UtlQFileMode( pControlsIda->szPathContent,
//                            &usAttribute, 0L, FALSE );
      usRc = UtlFindFirst( pControlsIda->szPathContent,
                           &hDirHandle, FILE_NORMAL,
                           &(ResultBuf), sizeof( ResultBuf),
                           &usAttribute, 0L, FALSE);
      if ( usRc )
      {
        if ( ( usRc == ERROR_PATH_NOT_FOUND ) &&
             !pControlsIda->fImport )
        {
          //path could not be found to export file to, so query if directory
          //should be added if name does not contain either * or ? and
          //actually contains a directory(isn't only root)
          if ( !strchr( pControlsIda->szPathContent,
                        SINGLE_SUBSTITUTION )  &&
               !strchr( pControlsIda->szPathContent,
                        MULTIPLE_SUBSTITUTION ) )
          {
            pszError = pControlsIda->szPath;
            usRc = UtlErrorHwnd( PATH_NOTEXIST_CREATEIT,
                                 MB_YESNO, 1, &pszError,
                                 EQF_ERROR, hwndDlg );
            fOK = ( usRc != MBID_NO );
            if ( fOK )
            {
              //remove terminating \ from path
              strcpy( pControlsIda->szString, pControlsIda->szPath );
              pszPath = strrchr( pControlsIda->szString, BACKSLASH );
              *(pszPath) = EOS;

              usRc = UtlMkMultDir( pControlsIda->szString,
                                   FALSE );
              if ( usRc )
              {
                fOK = FALSE;
                pszError = pControlsIda->szPathContent;
                UtlErrorHwnd( ERROR_FILENAME_NOT_VALID,
                              MB_CANCEL, 1, &pszError, EQF_ERROR,
                              hwndDlg );
                SETFOCUS( hwndDlg, pControlsIda->idPathEF );
              } /* endif */
            } /* endif */
          }
          else
          {
            pszError = pControlsIda->szPathContent;
            UtlErrorHwnd( ERROR_FILENAME_NOT_VALID,
                          MB_CANCEL, 1, &pszError, EQF_ERROR,
                          hwndDlg );
            fOK = FALSE;
            SETFOCUS( hwndDlg, pControlsIda->idPathEF );
          } /* endif */
        }
        else
        {
          if ( ( usRc == ERROR_FILE_NOT_FOUND ||
                 usRc == ERROR_PATH_NOT_FOUND ) &&
               pControlsIda->fImport )
          {
            if ( strchr( pControlsIda->szPathContent,
                         SINGLE_SUBSTITUTION )  ||
                 strchr( pControlsIda->szPathContent,
                         MULTIPLE_SUBSTITUTION ) )
            {
              // if (strcmp( pControlsIda->szPatternName,
              //             DEFAULT_PATTERN) == 0)
              // {
              //if in the wm_command part then issue a warning as
              //the filename is obviously not correct
              //don't set fOK false though as the listboxes should be
              //refilled correctly
              if ( pControlsIda->fCommand )
              {
                //get selected item index of selected file in listbox
                sItem = QUERYSELECTION( hwndDlg,
                                        pControlsIda->idFilesLB );

                //deselect any old file selection
                DESELECTITEM( hwndDlg, pControlsIda->idFilesLB, sItem );

                UtlErrorHwnd( NO_FILE_SELECTED,
                              MB_CANCEL, 0, NULL, EQF_ERROR,
                              hwndDlg );

                SETFOCUS( hwndDlg, pControlsIda->idPathEF );
              } /* endif*/
              // } /* endif */
            }
            else
            {
              pszError = pControlsIda->szPathContent;
              UtlErrorHwnd( ERROR_FILENAME_NOT_VALID,
                            MB_CANCEL, 1, &pszError, EQF_ERROR,
                            hwndDlg );
              fOK = FALSE;
              SETFOCUS( hwndDlg, pControlsIda->idPathEF );
            } /* endif */
          }
          else
          {
            if ( usRc == ERROR_INVALID_NAME ||
                 usRc == ERROR_FILENAME_EXCED_RANGE )
            {
              UtlErrorHwnd( INVALID_OS2_FILENAME,
                            MB_CANCEL, 0, NULL, EQF_ERROR,
                            hwndDlg );
              fOK = FALSE;
              SETFOCUS( hwndDlg, pControlsIda->idPathEF );
            } /* endif */
          } /* endif */
        } /* endif */
      }
      else
      {
        // close file search handle
        if ( hDirHandle != HDIR_CREATE ) UtlFindClose( hDirHandle, FALSE );

        fOK = TRUE;              //update dialog controls as path is valid
      } /* endif */
    } /* endif */

    if ( fOK )
    {
      //the path and filename are valid, i.e. exist

      //if drive new, change drive btn setting
      if ( toupper(pControlsIda->szDrive[0]) != toupper(pControlsIda->szPath[0]) )
      {
        //deselect drive button if one selected
        usDrive = IDFROMDRIVE( PID_DRIVEBUTTON_A,
                               pControlsIda->szDrive[0]);
        SETDRIVE( hwndDlg, usDrive, FALSE );

        //a new drive has been entered so set drive btn
        //to new drive in path entry field
        usId = IDFROMDRIVE( PID_DRIVEBUTTON_A,
                            pControlsIda->szPath[0] );
        SETDRIVE( hwndDlg, usId, TRUE );

        //set szDrive to new drive
        pControlsIda->szDrive[0] = pControlsIda->szPath[0];
      } /* endif */

      //update current directory entry field with path
      //excluding filename and file extension
      SETTEXT( hwndDlg, pControlsIda->idCurrentDirectoryEF,
               pControlsIda->szPath );

      //update path entry field to contain only
      //szPatternName i.e. filename plus extension
      SETTEXT( hwndDlg, pControlsIda->idPathEF, pControlsIda->szPatternName );

      //update files and dir listboxes
      sRc = UtlFillFileDir( hwndDlg, pControlsIda, TRUE );

      //error msg issued in FillFileDir
      if ( sRc == UTLERROR )
      {
        //select system drive and refill directories and files listboxes
        //and name and current directory entry field

        //deselect drive that lead to error msg issued
        usDrive = IDFROMDRIVE( PID_DRIVEBUTTON_A, pControlsIda->szDrive[0]);
        SETDRIVE( hwndDlg, usDrive, FALSE );

        //select system drive as default
        if ( UtlQueryString(QST_PRIMARYDRIVE, pControlsIda->szDrive, MAX_DRIVE) )
          usId = IDFROMDRIVE( PID_DRIVEBUTTON_A, pControlsIda->szDrive[0] );

        //display selected drive
        SETDRIVE( hwndDlg, usId, TRUE );

        //fill current directory static, default is system drive
        strcpy( pControlsIda->szPath, pControlsIda->szDrive );
        strcat( pControlsIda->szPath, BACKSLASH_STR );
        SETTEXT( hwndDlg, pControlsIda->idCurrentDirectoryEF,
                 pControlsIda->szPath );

        //fill szPathContent before FillFileDir is called
        strcpy( pControlsIda->szPathContent, pControlsIda->szPath );
        strcat( pControlsIda->szPathContent, pControlsIda->szPatternName );

        //update files/directories listbox
        UtlFillFileDir( hwndDlg, pControlsIda, TRUE );
        //--- error occured set functions return code to FALSE   /*@1337A*/
        fOK = FALSE;                                             /*@1337A*/
      }
      else
      {
        if ( !pControlsIda->fCommand )
        {
          //if szpathcontent contains no asterisk, i.e. is complete
          if ( !strchr( pControlsIda->szPathContent,
                        SINGLE_SUBSTITUTION )  &&
               !strchr( pControlsIda->szPathContent,
                        MULTIPLE_SUBSTITUTION ) )
          {
            //invoke command function
            WinPostMsg( hwndDlg, WM_COMMAND,
                        MP1FROMSHORT( pControlsIda->idOkPB ), NULL );
          }
        }
        else
        {
          //if szpathcontent contains either * or ?
          if ( strchr( pControlsIda->szPathContent,
                       SINGLE_SUBSTITUTION )  ||
               strchr( pControlsIda->szPathContent,
                       MULTIPLE_SUBSTITUTION ) )
          {
            if ( pControlsIda->fImport )
            {
              sItem = QUERYSELECTION( hwndDlg, pControlsIda->idFilesLB );
              if ( sItem == LIT_NONE )
              {
                UtlErrorHwnd( NO_FILE_SELECTED,
                              MB_CANCEL, 0, NULL, EQF_ERROR,
                              hwndDlg );
                fOK = FALSE;
                SETFOCUS( hwndDlg, pControlsIda->idPathEF );
              }
            } /* endif */

            //for import & export cases with *  when ok button is pressed
            if ( fOK )
            {
              UtlErrorHwnd( INVALID_OS2_FILENAME,
                            MB_CANCEL, 0, NULL, EQF_ERROR,
                            hwndDlg );
              fOK = FALSE;
              SETFOCUS( hwndDlg, pControlsIda->idPathEF );
            } /* endif */
          } /* endif */
        } /* endif */
      } /* endif */
    } /* endif */
  } /* endif */
  return(fOK);
} /* end UtlEFValidityTest */

//+----------------------------------------------------------------------------+
// Function name:  UtlFillFileDir                                               
//+----------------------------------------------------------------------------+
// Description:  Fills directories and file listboxes depending on the          
//               filepath passed to it.                                         
//+----------------------------------------------------------------------------+
// Parameters:   HWND          hwndDlg      IN     dialog window handle         
//               PCONTROLSIDA  pControlsIda IN     dialog control ida           
//               BOOL          fMsg         IN     show messages flag           
//+----------------------------------------------------------------------------+
// Returncode type:  SHORT                                                      
//+----------------------------------------------------------------------------+
// Returncodes:  -  UTLERROR  in case of an error                               
//                  number of files                                             
//+----------------------------------------------------------------------------+
//  Prerequesites:                                                              
//     The following fields in CONTROLSIDA need to be filled before the         
//     function may be called:                                                  
//     IDs:                                                                     
//     idPathEF        with the id of path entry field                          
//     idDirLB         with the id of directories listbox                       
//     idFilesLB       with the id of files listbox (only import)               
//     STRINGs:                                                                 
//     szPathContent   with input path name                                     
//+----------------------------------------------------------------------------+
// Side effects:                                                                
//     If UTLERROR is returned, then a message was issues saying that           
//     the list boxes could not be filled because for e.g. the drive            
//     was not ready.                                                           
//+----------------------------------------------------------------------------+
// Function call:  UtlFillFileDir( hwndDlg, pControlsIda );                     
//+----------------------------------------------------------------------------+
// Samples:                                                                     
//   UtlFillFileDir is called as follows with a fully qualified path:           
//                                                                              
//    //build szFindPath for FillFileDir                                        
//    strcpy( pControlsIda->szPathContent, pControlsIda->szPath );              
//    strcat( pControlsIda->szPathContent, pControlsIda->szPatternName );       
//                                                                              
//    //update the directory/files listboxes                                    
//    sRc = UtlFillFileDir( hwndDlg, pControlsIda );                            
//                                                                              
//    //if listboxes could not be filled, e.g. drive not ready msg issued       
//    if ( sRc == UTLERROR )                                                    
//    {                                                                         
//       ...                                                                    
//                                                                              
// ----------------------------------------------------------------------------+
// Function flow:                                                               
//    disable updating in both the files and directories listboxes              
//    delete contents of directories listbox                                    
//    split szPathContent into path and filename information                    
//                                                                              
//    fill directories listbox                                                  
//    if no error and there is a files listbox then                             
//      check that system directory has not been added to directories listbox   
//      delete contents of files listbox                                        
//      file files listbox (endif)                                              
//                                                                              
//    enable upadeting in both listboxes (if available)                         
//+----------------------------------------------------------------------------+

SHORT UtlFillFileDir
(
HWND             hwndDlg,            // window handle
PCONTROLSIDA     pControlsIda,       // pointer to ida
BOOL             fMsg                // show messages flag
)
{

  SHORT  sRc;                      //return code
  SHORT  sFlags;                   // flags to be used for UtlLoadFileNames
  BOOL   fLongFileNames = FALSE;   // use-long-file-name-functions flag

  // enable/disable long file name functions
  fLongFileNames = pControlsIda->fLongFileNames;

  // under Windows we can use long file name functions for all
  // Windows Version higher than or equal to 3.95
  if ( fLongFileNames )
  {
    DWORD dwVersion = GetVersion();
    BYTE  bWinMajVersion = LOBYTE( LOWORD( dwVersion ) );
    BYTE  bWinMinVersion = HIBYTE( LOWORD( dwVersion ) );
    if ( bWinMajVersion <= 2 )
    {
      fLongFileNames = FALSE;
    }
    else if ( (bWinMajVersion == 3) && (bWinMinVersion < 95) )
    {
      fLongFileNames = FALSE;
    } /* endif */
  } /* endif */


  ENABLEUPDATE_FALSE( hwndDlg, pControlsIda->idDirLB);
  if ( pControlsIda->idFilesLB )
  {
    ENABLEUPDATE_FALSE( hwndDlg, pControlsIda->idFilesLB);
  } /* endif */

  DELETEALL( hwndDlg, pControlsIda->idDirLB );

  //get path and pattern from szPathContent
  UtlMakeFNameAndPath( pControlsIda->szPathContent,
                       pControlsIda->szPath,
                       pControlsIda->szPatternName );

  //set patternname to *.* so dir listbox is filled correctly. Store
  //temporarily in szString
  strcpy( pControlsIda->szString, pControlsIda->szPath );
  strcat( pControlsIda->szString, DEFAULT_PATTERN );

  //switch cursor to hour-glass
  SETCURSOR( SPTR_WAIT );

  //fill directories listbox
  sFlags = NAMFMT_NODRV;
  if ( !fMsg )
  {
    sFlags |= NAMFMT_NOERROR;
  } /* endif */
  if ( fLongFileNames )
  {
    sRc = UtlLoadLongFileNamesHwnd( pControlsIda->szString,
                                    FILE_DIRECTORY,
                                    WinWindowFromID( hwndDlg, pControlsIda->idDirLB ),
                                    sFlags, hwndDlg );
  }
  else
  {
    sRc = UtlLoadFileNamesHwnd( pControlsIda->szString,
                                FILE_DIRECTORY,
                                WinWindowFromID( hwndDlg, pControlsIda->idDirLB ),
                                sFlags, hwndDlg );
  } /* endif */

  if ( sRc != UTLERROR )
  {
    // the following code has been disabled to allow access to EQF directories

    //Search for the directory name EQF and discard it if present
//    UtlMakeEQFPath( pControlsIda->szDummy, NULC,
//                    SYSTEM_PATH, NULP );            // system drive
//    pSystemPath = pControlsIda->szDummy + 3;          // skip drive
//    sSelectItem = SEARCHITEM( hwndDlg, pControlsIda->idDirLB,
//                              pSystemPath );
//    if ( sSelectItem != LIT_NONE && sSelectItem != LIT_ERROR )
//      DELETEITEM( hwndDlg, pControlsIda->idDirLB, sSelectItem );

    SELECTITEM( hwndDlg, pControlsIda->idDirLB, 0 );

    //refill  files listbox
    if ( pControlsIda->idFilesLB )
    {
      DELETEALL( hwndDlg, pControlsIda->idFilesLB );
      sFlags = NAMFMT_NODRV | NAMFMT_NODIR | NAMFMT_NOROOT;
      if ( !fMsg )
      {
        sFlags |= NAMFMT_NOERROR;
      } /* endif */

      if ( fLongFileNames )
      {
        UtlLoadLongFileNamesHwnd( pControlsIda->szPathContent,
                                  FILE_NORMAL,
                                  WinWindowFromID( hwndDlg, pControlsIda->idFilesLB ),
                                  sFlags, hwndDlg );
      }
      else
      {
        UtlLoadFileNamesHwnd( pControlsIda->szPathContent,
                              FILE_NORMAL,
                              WinWindowFromID( hwndDlg, pControlsIda->idFilesLB ),
                              sFlags, hwndDlg );
      } /* endif */


      {

        HWND hwndLB = GetDlgItem(hwndDlg, pControlsIda->idFilesLB);
        UtlSetHorzScrollingForLB(hwndLB);
        hwndLB = GetDlgItem(hwndDlg, pControlsIda->idDirLB);
        UtlSetHorzScrollingForLB(hwndLB);


      }




      if ( pControlsIda->fMultiSelectionLB )
      {
        //select and then deselect first listbox item
        SELECTITEM( hwndDlg, pControlsIda->idFilesLB, 0 );
        DESELECTITEM( hwndDlg, pControlsIda->idFilesLB, 0 );
      } /* endif */

      // automatically select first listbox item if there is only one
      // item in the listbox
      if ( pControlsIda->fMultiSelectionLB &&
           (QUERYITEMCOUNT( hwndDlg, pControlsIda->idFilesLB ) == 1) )
      {
        SELECTITEM( hwndDlg, pControlsIda->idFilesLB, 0 );
      } /* endif */
    } /* endif */
  }
  else
  {
    if ( pControlsIda->idFilesLB )
    {
      DELETEALL( hwndDlg, pControlsIda->idFilesLB );
    } /* endif */
  } /* endif */

  ENABLEUPDATE_TRUE( hwndDlg, pControlsIda->idDirLB );
  if ( pControlsIda->idFilesLB )
  {
    ENABLEUPDATE_TRUE( hwndDlg, pControlsIda->idFilesLB );
  } /* endif */

  SETCURSOR( SPTR_ARROW );

  return( sRc );
} /* end of function UtlFillFileDir */

//+----------------------------------------------------------------------------+
// Function name:  UtlMakeFNameAndPath                                          
//+----------------------------------------------------------------------------+
// Description: Parses a string and returns the drive and directories           
//              in second parameter and filename plus extension in the          
//              third parameter                                                 
//+----------------------------------------------------------------------------+
// Parameters: PSZ file path                 IN                                 
//             PSZ drive and directories     OUT                                
//             PSZ filename plus extension   OUT                                
//+----------------------------------------------------------------------------+
// Returncode type:  VOID                                                       
//+----------------------------------------------------------------------------+
// Returncodes:  -                                                              
//+----------------------------------------------------------------------------+
// Prerequesites: none                                                          
//+----------------------------------------------------------------------------+
// Side effects: none                                                           
//+----------------------------------------------------------------------------+
// Function call:  UtlMakeFNameAndPath( szFullPath, szPath, szPatternName );    
//+----------------------------------------------------------------------------+
// Samples:                                                                     
//   UtlMakeFNameAndPath is called as follows:                                  
//                                                                              
//   //get path and pattern from pControlsIda->szPathContent                    
//   UtlMakeFNameAndPath( pControlsIda->szPathContent,   //input string         
//                        pControlsIda->szDummy,         //path without fname   
//                        pControlsIda->szPatternName);  //fname + ext          
//+----------------------------------------------------------------------------+
// Function flow:                                                               
//   separate out path from fully qualified file path                           
//   if there is path information then                                          
//     add backslash to path                                                    
//     save path and filename information                                       
//   else                                                                       
//     set path string to NULL                                                  
//     save filename information (endif)                                        
//+----------------------------------------------------------------------------+

VOID UtlMakeFNameAndPath( PSZ pszFullPath,    //in- fully qualified pathname
                          PSZ pszPath,        //out- drive and directories
                          PSZ pszFileName )   //out- filename plus extension
{
  PSZ   pszPointer;
  CHAR  szPath[MAX_LONGPATH];

  strcpy( szPath, pszFullPath );
  pszPointer = strrchr( szPath, BACKSLASH );
  if ( pszPointer++ )
  {
    strcpy( pszFileName, pszPointer);
    *(pszPointer) = EOS;
    strcpy( pszPath, szPath );
  }
  else
  {
    strcpy( pszFileName, szPath );
    strcpy( pszPath, EMPTY_STRING );
  } /* endif */
}  /* end UtlMakeFNameAndPath */

//+----------------------------------------------------------------------------+
// Function name:  UtlCheckFileName                                             
//+----------------------------------------------------------------------------+
// Description: Checks if a string is alphanumeric or contains either a         
//              @, $, #, or _. If it contains any other special characters      
//              FALSE is returned.                                              
//+----------------------------------------------------------------------------+
// Parameters: PSZ file name                 IN                                 
//             BOOL TRUE or FALSE            OUT                                
//+----------------------------------------------------------------------------+
// Returncode type:  BOOL                                                       
//+----------------------------------------------------------------------------+
// Returncodes: TRUE if file name is valid, else FALSE                          
//+----------------------------------------------------------------------------+
// Prerequesites: none                                                          
//+----------------------------------------------------------------------------+
// Side effects: none                                                           
//+----------------------------------------------------------------------------+
// Function call:  UtlCheckFileName( pszName );                                 
//+----------------------------------------------------------------------------+
// Samples:                                                                     
//   UtlCheckFileName is called as follows:                                     
//                                                                              
//   fOK = UtlCheckFileName( pIda->szDictFilePath ); //path is file name with   
//          or without extension - path checked in dialog utility               
//          called UtlEFValidityTest.                                           
//+----------------------------------------------------------------------------+
// Function flow:                                                               
//   remove leading and trailing blanks                                         
//   check if alphanumeric or one of the special characters                     
//   if not issue error message and return FALSE else if valid return TRUE      
//+----------------------------------------------------------------------------+

BOOL UtlCheckFileName( PSZ pszName )
{
  BOOL   fOK = TRUE;                   //success indicator
  PSZ    pszTemp;                      //ptr to string
  PSZ    pszString;                    //ptr to string
  PSZ    pszExt;                       //ptr to file extension
  CHAR   szName[MAX_EQF_PATH];         //name string
  CHAR   szString[MAX_FNAME];          //name string

  //initialize string pointers
  pszString = pszTemp = pszName;
  strcpy( szName, pszName );

  //remove leading and trailing blanks
  UtlStripBlanks( pszString );

  //check if filename has an extension
  pszExt = strrchr( pszString, DOT );

  if ( pszExt )
  {
    //get ext without dot
    pszString = pszExt+1;
    //get fname without extension
    Utlstrccpy( szString, szName, DOT );
    pszTemp = szString;

    //check extension and then fname
    if ( UtlCheckString( pszString ) )
      fOK = UtlCheckString( pszTemp );
    else
      fOK = FALSE;
  }
  else
  {
    //check fname
    fOK = UtlCheckString( pszString );
  }

  if ( !fOK )
  {
    UtlError( ERROR_FILENAME_NOT_VALID, MB_CANCEL, 1, &pszName, EQF_ERROR );
  } /* endif */

  return( fOK );
} /* endUtlCheckFileName */

BOOL UtlCheckString( PSZ pszName )
{
  BOOL   fOK = TRUE;                   //success indicator

  //check that pszname is alphanumeric or contains any of the special
  //characters below
  while ( *pszName && fOK )
  {
    switch ( *pszName )
    {
      case '@':
      case '#':
      case '$':
      case '_':
        fOK = TRUE;
        break;
      default:
        fOK = isalnum( *pszName );
        break;
    } /* endswitch */
    pszName++;
  } /* endwhile */
  return( fOK );
} /* endUtlCheckString */

BOOL UtlSetOutputFileNameDlg
(
HWND             hwndParent,         // parent window of dialog
HWND             hwndOwner,          // owner window of dialog
PSZ              pszFileName         // ptr to filename buffer
)
{
  BOOL             fSet;               // return code of dialog
  HMODULE hResMod;
  hwndParent;                          // get rid of compiler warnings

  hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
  DIALOGBOX( hwndOwner, UTLSETFILEDLG, hResMod, ID_UTL_SETFILE_DLG,
             pszFileName, fSet );
  return( fSet );
} /* end of function UtlSetOutputFileNameDlg */

//+----------------------------------------------------------------------------+
// Internal function                                                            
//+----------------------------------------------------------------------------+
// Function name: UtlSetFileDlg                                                 
//+----------------------------------------------------------------------------+
// Function call:                                                               
//   MRESULT EXPENTRY UtlSetFileDlg( hwnd, msg, mp1, mp2 )                      
//+----------------------------------------------------------------------------+
// Description:                                                                 
//   This function is the dialog procedure for the "Set File" dialog.           
//                                                                              
//   The function expects a pointer to the QDPR IDA in the                      
//   second "message parameter" (mp2).                                          
//                                                                              
//   UtlSetFileDlg handles the processing of the different messages that        
//   are sent if a button is pressed or an item from the listboxes is           
//   selected.                                                                  
//+----------------------------------------------------------------------------+
// Input parameter:                                                             
//   HWND    hwnd     handle of window                                          
//   USHORT  msg      type of message                                           
//   MPARAM  mp1      first message parameter                                   
//   MPARAM  mp2      second message parameter                                  
//+----------------------------------------------------------------------------+
// Output parameter:                                                            
//   none                                                                       
//+----------------------------------------------------------------------------+
// Returncode type: MRESULT                                                     
//+----------------------------------------------------------------------------+
// Returncodes:                                                                 
//   TRUE                                                                       
//   FALSE                                                                      
//+----------------------------------------------------------------------------+
// Prerequesits:                                                                
//   - the function expects to receive in the WM_INITDLG message the            
//     pointer to the dialog IDA                                                
//+----------------------------------------------------------------------------+
// Side effects:                                                                
//   - sets the filename in the input/output structure                          
//+----------------------------------------------------------------------------+
// Samples:                                                                     
//                                                                              
//+----------------------------------------------------------------------------+
// Function flow:                                                               
//                                                                              
//   SWITCH on the message received                                             
//     CASE WM_INITDLG                                                          
//       Get pointer to IDA from the second message parameter                   
//       Store the pointer to the IDA in dialog window word                     
//       Post a WM_EQF_INITIALIZE message                                       
//                                                                              
//     CASE WM_EQF_INITIALIZE                                                   
//       Set last-used-values in the dialog                                     
//                                                                              
//     CASE WM_COMMAND                                                          
//       SWITCH on the control sending the message                              
//         CASE "Set" pushbutton                                                
//           Leave the dialog                                                   
//                                                                              
//         CASE "Cancel" pushbutton                                             
//           Leave the dialog                                                   
//                                                                              
//     CASE WM_CONTROL                                                          
//       pass message to dialog utilities                                       
//+----------------------------------------------------------------------------+
INT_PTR CALLBACK UTLSETFILEDLG
(
HWND   hwnd,                        // handle of window
WINMSG msg,                         // type of message
WPARAM mp1,                         // first message parameter
LPARAM mp2                          // second message parameter
)
{
  /********************************************************************/
  /*                         local variables                          */
  /********************************************************************/
  MRESULT              mResult = FALSE; // dlg procedure return value
  PUTL_DEST_DLG_IDA    psctIDA;         // dialog IDA

  switch ( msg )
  {
    case WM_EQF_QUERYID: HANDLEQUERYID( ID_UTL_SETFILE_DLG, mp2 ); break;

    case WM_INITDLG :
      {
        /**************************************************************/
        /*                     Create dialog ida                      */
        /**************************************************************/
        if ( UtlAlloc( (PVOID *)&psctIDA, 0L, (LONG)sizeof( UTL_DEST_DLG_IDA ),
                       ERROR_STORAGE ) )
        {
          /************************************************************/
          /* save the pointer to the IDA in the dialog reserved memory*/
          /************************************************************/
          ANCHORDLGIDA( hwnd, psctIDA );

          /************************************************************/
          /* Hide dummy controls of dialog                            */
          /************************************************************/
          HIDECONTROL( hwnd, ID_UTL_SETFILE_DUMMY1 );
          HIDECONTROL( hwnd, ID_UTL_SETFILE_DUMMY2 );
          HIDECONTROL( hwnd, ID_UTL_SETFILE_DUMMY3 );

          /**************************************************************/
          /*  Get print destination from second message parameter    */
          /**************************************************************/
          if ( PVOIDFROMMP2( mp2 ) != NULL )
          {
            strcpy( psctIDA->szPrintDest, (char *) PVOIDFROMMP2( mp2 ) );
          }
          else
          {
            strcpy( psctIDA->szPrintDest, EMPTY_STRING );
          } /* endif */
          psctIDA->pszCallersPrintDest = (PSZ) PVOIDFROMMP2( mp2 );

          /************************************************************/
          /*                   get the drives list                    */
          /************************************************************/
          UtlGetDriveList( (PBYTE)psctIDA->szDriveList );

          /************************************************************/
          /*    post a WM_EQF_INITIALIZE to continue initialization   */
          /************************************************************/
          WinPostMsg( hwnd, WM_EQF_INITIALIZE, NULL, NULL );
        }
        else
        {
          /************************************************************/
          /*         allocation failed, so dismiss the dialog         */
          /************************************************************/
          WinDismissDlg( hwnd, FALSE );
        } /* endif */

      }
      break;

    case WM_EQF_INITIALIZE :
      {
        /**************************************************************/
        /*                     get pointer to IDA                     */
        /**************************************************************/
        psctIDA = ACCESSDLGIDA( hwnd, PUTL_DEST_DLG_IDA );

        if ( psctIDA != NULL )
        {
          /************************************************************/
          /*             set mouse pointer to hour glass              */
          /************************************************************/
          SETCURSOR( SPTR_WAIT );

          /************************************************************/
          /*       set the variables in the dialog controls IDA       */
          /************************************************************/
          memset( &(psctIDA->ControlsIDA), NULC, sizeof( CONTROLSIDA ) );
          psctIDA->ControlsIDA.idCurrentDirectoryEF = ID_UTL_CURDIR_NAME_TXT;
          psctIDA->ControlsIDA.idPathEF             = ID_UTL_SETFILE_NAME_EF;
          psctIDA->ControlsIDA.idDirLB              = ID_UTL_SETFILE_DIR_LB;
          psctIDA->ControlsIDA.idInternalRB         = ID_UTL_SETFILE_DUMMY1;
          psctIDA->ControlsIDA.idExternalRB         = ID_UTL_SETFILE_DUMMY2;
          psctIDA->ControlsIDA.idDriveBTN           = ID_UTL_SETFILE_DRIVEA_PB;
          psctIDA->ControlsIDA.idControlsGB         = ID_UTL_SETFILE_GP;
          psctIDA->ControlsIDA.idOkPB               = ID_UTL_SETFILE_SET_PB;
          psctIDA->ControlsIDA.idToLB               = ID_UTL_SETFILE_DUMMY3;
          psctIDA->ControlsIDA.fImport              = FALSE;
          UtlQueryString( QST_PRIMARYDRIVE, psctIDA->ControlsIDA.szDrive,
                          sizeof(psctIDA->ControlsIDA.szDrive) );
          strcpy( psctIDA->ControlsIDA.szHandler, DICTIONARYHANDLER );

          /************************************************************/
          /* Set last used values                                     */
          /************************************************************/
          UtlStripBlanks( psctIDA->szPrintDest );
          if ( psctIDA->szPrintDest[0] != EOS )
          {
            /**********************************************************/
            /* check if drive letter of last-used filename is in the  */
            /*                list of available drives                */
            /*            if it is not do not set anything            */
            /**********************************************************/
            if ( ( psctIDA->szPrintDest[1] == COLON)  &&
                 strchr( psctIDA->szDriveList, psctIDA->szPrintDest[0] ) )
            {
              psctIDA->ControlsIDA.chSavedDrive = psctIDA->szPrintDest[0];
              strncpy( psctIDA->ControlsIDA.szSavedPath,
                       psctIDA->szPrintDest, MAX_PATH144 - 1 );
              strcpy( psctIDA->ControlsIDA.szSelectedName,
                      UtlGetFnameFromPath( psctIDA->szPrintDest ) );
            } /* endif */
          } /* endif */

          /************************************************************/
          /*                 initialize the controls                  */
          /************************************************************/
          UtlControlsInit( hwnd, &(psctIDA->ControlsIDA) );
          SetCtrlFnt(hwnd, GetCharSet(), ID_UTL_SETFILE_NAME_EF,
                     ID_UTL_CURDIR_NAME_TXT );
          SetCtrlFnt(hwnd, GetCharSet(), ID_UTL_SETFILE_DIR_LB , 0);

          /************************************************************/
          /*                   reset mouse pointer                    */
          /************************************************************/
          SETCURSOR( SPTR_ARROW );
        } /* endif */
      }
      break;

    case WM_COMMAND :
      {
        /**************************************************************/
        /*                     get pointer to IDA                     */
        /**************************************************************/
        psctIDA = ACCESSDLGIDA( hwnd, PUTL_DEST_DLG_IDA );

        if ( psctIDA != NULL )
        {
          switch ( WMCOMMANDID( mp1, mp2 ))
          {
            case ID_UTL_SETFILE_SET_PB :
              /**********************************************************/
              /*     check if a valid print destination is selected     */
              /**********************************************************/
              if ( UtlEFValidityTest( &(psctIDA->ControlsIDA), hwnd ) )
              {
                strncpy( psctIDA->szPrintDest,
                         psctIDA->ControlsIDA.szPathContent,
                         CCHMAXPATH - 1 );
                _strupr( psctIDA->szPrintDest );
                if ( psctIDA->pszCallersPrintDest )
                {
                  strcpy( psctIDA->pszCallersPrintDest, psctIDA->szPrintDest );
                } /* endif */
                DelCtrlFont(hwnd, ID_UTL_SETFILE_NAME_EF );
                DelCtrlFont(hwnd, ID_UTL_SETFILE_DIR_LB );
                WinDismissDlg( hwnd, TRUE );
              }
              else
              {
                SETFOCUS( hwnd, ID_UTL_SETFILE_NAME_EF );
              } /* endif */
              break;

            case ID_UTL_SETFILE_CANCEL_PB :
            case DID_CANCEL :
              DelCtrlFont(hwnd, ID_UTL_SETFILE_NAME_EF );
              DelCtrlFont(hwnd, ID_UTL_SETFILE_DIR_LB );
              WinDismissDlg( hwnd, FALSE );
              break;
            default:
              mResult = UtlWMControls( hwnd, msg, WMCOMMANDID( mp1, mp2 ),
                                       WMCOMMANDCMD( mp1, mp2 ),
                                       &(psctIDA->ControlsIDA) );
          } /* endswitch */
        } /* endif */
      }
      break;

    default :
      {
        mResult = WinDefDlgProc( hwnd, msg, mp1, mp2 );
      }
      break;
  } /* endswitch */

  return( mResult );

} /* end of function UtlSetFileDlg */


#ifndef _TQM

#ifdef TOP97_A57K_RENAME
//+----------------------------------------------------------------------------+
// Internal function                                                            
//+----------------------------------------------------------------------------+
// Function name: UTLRenameObjjectDlg                                           
//+----------------------------------------------------------------------------+
// Function call:                                                               
//   fOK = UtlRenameObjectDlg( pszObjName, ObjClass );                          
//+----------------------------------------------------------------------------+
// Description:                                                                 
//   This procedure is the front-end to the rename object dialog. The name      
//   of the object being rename and the class of the object (e.g. clsFOLDER)    
//   has to be supplied.                                                        
//+----------------------------------------------------------------------------+
// Input parameter:                                                             
//   PSZ     pszObjName   ptr to object name (e.g. C:\EQF\SAMPLE1.F00)          
//   CLASSES ObjectClass  class of object (e.g. clsFOLDER)                      
//+----------------------------------------------------------------------------+
// Output parameter:                                                            
//   none                                                                       
//+----------------------------------------------------------------------------+
// Returncode type: USHORT                                                      
//+----------------------------------------------------------------------------+
// Returncodes:                                                                 
//   NO_ERROR (0)          function completed successfully                      
//   ERROR_INVALID_DATA    object name is invalid or not supplied               
//   ERROR_NOT_SUPPORTED   no rename option for the given class                 
//   ERROR_SHARING_VIOLATION when object is currently in use                    
//   other                 other error codes from called functions              
//+----------------------------------------------------------------------------+
USHORT UtlRenameObjectDlg
(
PSZ         pszObjName,              // name of object
CLASSES     ObjectClass              // class of object (e.g. clsFolder)                      
)
{
  USHORT      usRC = NO_ERROR;         // function return code
  PRENAMEDLGIDA pIDA = NULL;           // rename dialog IDA
  BOOL        fLocked = FALSE;         // object-has-been-locked flag

  // check input parameters
  if ( (pszObjName == NULL) || (*pszObjName == EOS) )
  {
    usRC = ERROR_INVALID_DATA;
  }
  else if ( (strlen(pszObjName) + 1) >= sizeof(OBJNAME) )
  {
    usRC = ERROR_INVALID_DATA;
  }
  else if ( usRC == NO_ERROR )
  {
    switch ( ObjectClass )
    {
      case clsFOLDER :
      case clsMEMORY :
      case clsDICTIONARY :
        // O.K. we support the rename of these types of objects
        break;
      default :
        // rename of these objects is not supported
        usRC = ERROR_NOT_SUPPORTED;
        break;
    } /* endswitch */
  } /* endif */

  // allocate dialog IDA
  if ( usRC == NO_ERROR )
  {
    if ( !UtlAlloc( (PVOID *)&pIDA, 0L, sizeof(RENAMEDLGIDA), ERROR_STORAGE ) )
    {
      usRC = ERROR_NOT_ENOUGH_MEMORY;
    } /* endif */
  } /* endif */

  // store input parameters in dialog IDA
  if ( usRC == NO_ERROR )
  {
    strcpy( pIDA->szObject, pszObjName );
    pIDA->ObjectClass = ObjectClass;
    switch ( ObjectClass )
    {
      case clsFOLDER    : 
        Utlstrccpy( pIDA->szOldName, UtlGetFnameFromPath( pszObjName ), DOT );
        pIDA->ObjectType = FOLDER_OBJECT; 
        break;
      case clsMEMORY    : 
        strcpy( pIDA->szOldName, pszObjName );
        pIDA->ObjectType = TM_OBJECT;     
        break;
      case clsDICTIONARY: 
        Utlstrccpy( pIDA->szOldName, UtlGetFnameFromPath( pszObjName ), DOT );
        pIDA->ObjectType = DICT_OBJECT;   
        break;
      default :                                             
        break;
    } /* endswitch */
    strcpy( pIDA->szOldShortName, pIDA->szOldName );
    ObjShortToLongName( pIDA->szOldName, pIDA->szOldName, pIDA->ObjectType );
  } /* endif */

  // check if object is in-use and lock object
  if ( usRC == NO_ERROR )
  {
    SHORT sRC = QUERYSYMBOL( pIDA->szObject );
    if ( sRC != -1 )
    {
      USHORT usMsgNo = 0;              // number of message to be displayed
      PSZ    pszParm;                  // points to error msg parameter

      switch ( pIDA->ObjectClass )
      {
        case clsFOLDER:
          usMsgNo = ERROR_FOLDER_LOCKED;
          break;
        case clsMEMORY:
          usMsgNo = ERROR_DICT_LOCKED;
          break;
        case clsDICTIONARY:
          usMsgNo = ERROR_DICT_LOCKED;
          break;
      } /* endswitch */
      pszParm = pIDA->szOldName;
      OEMTOANSI( pszParm );
      UtlError( usMsgNo, MB_CANCEL, 1, &pszParm, EQF_ERROR );
      ANSITOOEM( pszParm );
      usRC = ERROR_SHARING_VIOLATION;
    }
    else
    {
      SETSYMBOL(pIDA->szObject);
      fLocked = TRUE;
    } /* endif */
  } /* endif */

  // pop-up dialog
  if ( usRC == NO_ERROR )
  {
    BOOL fOK;
	HMODULE hResMod;

	hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
    DIALOGBOX( QUERYACTIVEWINDOW(), UTLRENAMEDLG, hResMod, ID_RENAME_DLG, pIDA, fOK );
  } /* endif */

  // cleanup
  if ( fLocked ) REMOVESYMBOL(pIDA->szObject);
  if ( pIDA != NULL ) UtlAlloc( (PVOID *)&pIDA, 0L, 0L, NOMSG );

  // return to caller
  return( usRC );

} /* end of function UtlRenameObjectDlg */

// data area for rename function
typedef struct _RENAMEDATA
{
  // info on renamed object
  CLASSES          ObjectClass;                            // class of object being renamed
  UTLCHECKOBJTYPES ObjType;                                // type of object being rename
  CHAR             szOldShortName[MAX_FILESPEC];           // old short name of renamed object 
  CHAR             szNewShortName[MAX_FILESPEC];           // new short name of renamed object
  CHAR             szOldName[MAX_LONGFILESPEC];            // old name of renamed dobject
  CHAR             szNewName[MAX_LONGFILESPEC];            // new name of renamed dobject
  HWND             hwndErrMsg;                             // window handle for error messages
  CHAR             szOldAndNewName[MAX_EQF_PATH + MAX_LONGFILESPEC + MAX_LONGFILESPEC];
  CHAR             szOldObjName[MAX_EQF_PATH];             // object name of old object 

  // temporary buffers for folder searching and reference adjusting
  CHAR             szPropDir[MAX_EQF_PATH];                // folder property file search path
  CHAR             szSubFolObj[MAX_EQF_PATH];              // object name of subfolders
  CHAR             szSubFolSearchPath[MAX_EQF_PATH];       // subfolder search path
  CHAR             szFolSearchPath[MAX_EQF_PATH];          // folder search path
  CHAR             szFolProp[MAX_EQF_PATH];                // folder property file name
  CHAR             szFolObj[MAX_EQF_PATH];                 // folder object name
  PROPFOLDER       stFolProp;                              // folder properties
  CHAR             MemTbl[MAX_NUM_OF_READONLY_MDB*MAX_FILESPEC];  // buffer for TM list
  CHAR             DicTbl[NUM_OF_FOLDER_DICS*MAX_FILESPEC];   // buffer for dict list
  
} RENAMEDATA, *PRENAMEDATA;

// adjust references in a single folder
USHORT UtlAdjustFolderReferences
(
  PRENAMEDATA      pData,
  PSZ              pszFolder
)
{
  ULONG ulRead;
  PVOID pvTemp = &pData->stFolProp;
  BOOL fOK = TRUE;
  BOOL fPropChanged = FALSE;
  USHORT  usRC= 0;

  // setup folder property file name
  UtlMakeEQFPath( pData->szFolProp, NULC, PROPERTY_PATH, (PSZ) NULP );
  strcat( pData->szFolProp, BACKSLASH_STR );
  strcat( pData->szFolProp, pszFolder );

  // setup folder object name
  UtlMakeEQFPath( pData->szFolObj, NULC, SYSTEM_PATH, (PSZ) NULP );
  strcat( pData->szFolObj, BACKSLASH_STR );
  strcat( pData->szFolObj, pszFolder );

  // load folder property file
  fOK = UtlLoadFileL( pData->szFolProp, &pvTemp, &ulRead, FALSE, TRUE );

  // adjust references in folder property file
  if ( fOK )
  {
    // correct folder object name (use correct drive letter)
    pData->szFolObj[0] = pData->stFolProp.chDrive;

    // check property data
    fPropChanged = FALSE;
    switch ( pData->ObjectClass )
    {
      case clsMEMORY :
        // check folder TM
        if ( strcmp( pData->stFolProp.szMemory, pData->szOldShortName ) == 0 )
        {
          strcpy( pData->stFolProp.szMemory, pData->szNewShortName );
          fPropChanged = TRUE;
        } /* endif */
        if ( stricmp( pData->stFolProp.szLongMemory, pData->szOldName ) == 0 )
        {
          strcpy( pData->stFolProp.szLongMemory, pData->szNewName );
          fPropChanged = TRUE;
        } /* endif */

        if ( pData->stFolProp.aLongMemTbl[0][0] != EOS )
        {
          int i = 0;
          while ( (i < MAX_NUM_OF_READONLY_MDB) && (pData->stFolProp.aLongMemTbl[i][0] != EOS) )
          {
            if ( stricmp( pData->szOldName, pData->stFolProp.aLongMemTbl[i] ) == 0 )
            {
              strcpy( pData->stFolProp.aLongMemTbl[i], pData->szNewName );
              fPropChanged = TRUE;
            } /* endif */
            i++;
          } /* endwhile */
        } /* endif */

        // get copy of read-only TM list
        strcpy( pData->MemTbl, pData->stFolProp.MemTbl );

        // parse copy of TM list and copy TMs back to properties
        {
          PSZ p;         // ptr to TM in list

          p = strtok( pData->MemTbl, "\x15");
          pData->stFolProp.MemTbl[0] = EOS; // clear list

          for (; p; p=strtok( NULL, "\x15"))
          {
            if ( strcmp( pData->szOldShortName, p ) == 0 )
            {
              strcat( pData->stFolProp.MemTbl, pData->szNewShortName );
              fPropChanged = TRUE;
            }
            else
            {
              strcat( pData->stFolProp.MemTbl, p );
            } /* endif */
            strcat( pData->stFolProp.MemTbl, X15_STR );
          }
        }
        break;

      case clsDICTIONARY :
        if ( pData->stFolProp.aLongDicTbl[0][0] != EOS )
        {
          int i = 0;
          while ( (i < MAX_NUM_OF_FOLDER_DICS) && (pData->stFolProp.aLongDicTbl[i][0] != EOS) )
          {
            if ( strcmp( pData->szOldName, pData->stFolProp.aLongDicTbl[i] ) == 0 )
            {
              strcpy( pData->stFolProp.aLongDicTbl[i], pData->szNewName );
              fPropChanged = TRUE;
            } /* endif */
            i++;
          } /* endwhile */
        } /* endif */
        // get copy of dict list
        strcpy( pData->DicTbl, pData->stFolProp.DicTbl );

        // parse copy of dict list and copy dicts back to properties
        {
          PSZ p;         // ptr to dict in list

          p = strtok( pData->DicTbl, "\x15");
          pData->stFolProp.DicTbl[0] = EOS; // clear list

          for (; p; p=strtok( NULL, "\x15"))
          {
            if ( strcmp( pData->szOldShortName, p ) == 0 )
            {
              strcat( pData->stFolProp.DicTbl, pData->szNewShortName );
              fPropChanged = TRUE;
            }
            else
            {
              strcat( pData->stFolProp.DicTbl, p );
            } /* endif */
            strcat( pData->stFolProp.DicTbl, X15_STR );
          }
        }
        break;
    } /* endswitch */
  } /* endif */

  // check if folder is currently in use
  if ( fOK )
  {
    SHORT sRC = QUERYSYMBOL( pData->szFolObj );
    if ( sRC != -1 )
    {
      PSZ pszErrParm = pData->stFolProp.szLongName;
      fOK = FALSE;
      usRC = ERROR_FOLDER_LOCKED;
      UtlErrorHwnd( usRC, MB_CANCEL, 1, &pszErrParm, EQF_INFO, pData->hwndErrMsg );
    } /* endif */
  } /* endif */

  // rewrite folder property file if changed
  if ( fOK && fPropChanged )
  {
    fOK = UtlWriteFile( pData->szFolProp, sizeof(PROPFOLDER), &(pData->stFolProp), TRUE ) == NO_ERROR;

    // broadcast props changed message
    if ( fOK && (pData->hwndErrMsg != HWND_FUNCIF) )
    {
      EqfSend2AllHandlers( WM_EQFN_PROPERTIESCHANGED, MP1FROMSHORT( PROP_CLASS_FOLDER ), MP2FROMP(pData->szFolObj) );
    } /* endif */
  } /* endif */

  // loop over all subfolders and documents (required only for TM!)
  if ( fOK && (pData->ObjectClass == clsMEMORY) )
  {
    FILEFINDBUF stResultBuf;             // DOS file find struct
    USHORT      usCount = 1;             // number of files requested
    USHORT      usRC;                    // return code of called functions
    HDIR        hDirHandle = HDIR_CREATE;// DosFind routine handle

    UtlQueryString( QST_PROPDIR, pData->szPropDir, sizeof(pData->szPropDir) );

    // build search path
    {
      strcpy( pData->szSubFolSearchPath, pData->szFolObj );
      strcat( pData->szSubFolSearchPath, BACKSLASH_STR );
      strcat( pData->szSubFolSearchPath, pData->szPropDir );
      strcat( pData->szSubFolSearchPath, "\\*.*" );
    }

    usRC = UtlFindFirst( pData->szSubFolSearchPath, &hDirHandle, FILE_NORMAL, &stResultBuf, sizeof( stResultBuf), &usCount, 0L, 0);
    while ( (usRC == NO_ERROR) && usCount )
    {
      ULONG ulLen;
      PSZ pszExt = strrchr( RESBUFNAME(stResultBuf), '.' );

      // handle subfolde ror document property file
      if ( (pszExt != NULL) && (strcmp( pszExt, EXT_OF_SUBFOLDER ) == 0) )
      {
        PPROPFOLDER  pProp = NULL;           // subfolder properties

        // load subfolder properties and change TM name if required
        sprintf( pData->szSubFolObj, "%s\\%s\\%s", pData->szFolObj, pData->szPropDir, RESBUFNAME(stResultBuf) );
        if ( UtlLoadFileL( pData->szSubFolObj, (PVOID *) &pProp, &ulLen, FALSE, FALSE ) )
        {
          BOOL fPropChanged = FALSE;
          if ( strcmp( pProp->szMemory, pData->szOldShortName ) == 0 )
          {
            strcpy( pProp->szMemory, pData->szNewShortName );
            fPropChanged = TRUE;
          } /* endif */
          if ( strcmp( pProp->szLongMemory, pData->szOldName ) == 0 )
          {
            strcpy( pProp->szLongMemory, pData->szNewName );
            fPropChanged = TRUE;
          } /* endif */

          if ( fPropChanged )
          {
            UtlWriteFileL( pData->szSubFolObj, ulLen, pProp, TRUE );
          } /* endif */

          UtlAlloc( (PVOID *) &pProp, 0L, 0L, NOMSG );
          pProp = NULL;
        } /* endif if UtlLoadFile */
      }
      else if ( strcmp( RESBUFNAME(stResultBuf), HISTLOGFILE ) != 0 )
      {
        PPROPDOCUMENT  pProp = NULL;           // subfolder properties

        // load document properties and change TM name if required
        sprintf( pData->szSubFolObj, "%s\\%s\\%s", pData->szFolObj, pData->szPropDir, RESBUFNAME(stResultBuf) );
        if ( UtlLoadFileL( pData->szSubFolObj, (PVOID *) &pProp, &ulLen, FALSE, FALSE ) )
        {
          BOOL fPropChanged = FALSE;

          if ( strcmp( pProp->szLongMemory, pData->szOldName ) == 0 )
          {
            strcpy( pProp->szLongMemory, pData->szNewName );
            fPropChanged = TRUE;
          } /* endif */

          if ( strcmp( pProp->szMemory, pData->szOldShortName ) == 0 )
          {
            strcpy( pProp->szMemory, pData->szNewShortName );
            fPropChanged = TRUE;
          } /* endif */

          if ( fPropChanged )
          {
            UtlWriteFileL( pData->szSubFolObj, ulLen, pProp, TRUE );
          } /* endif */

          UtlAlloc( (PVOID *) &pProp, 0L, 0L, NOMSG );
          pProp = NULL;
        } /* endif if UtlLoadFile */
      } /* endif */

      usRC = UtlFindNext( hDirHandle, &stResultBuf, sizeof( stResultBuf), &usCount, 0);
    } /* endwhile */
    if ( hDirHandle != HDIR_CREATE ) UtlFindClose( hDirHandle, FALSE );
  } /* endif */
  return( 0 );
}

// adjust all references of a renamed object
USHORT UtlAdjustReferences
(
  PRENAMEDATA      pData
)
{
  USHORT usRC = 0;

  CHAR  szFolName[MAX_FILESPEC]; // buffer for name of current folder

  // loop over all folders properties files and process them
  WIN32_FIND_DATA FileFindData;
  HANDLE hFolFindDir;
  BOOL fMoreFilesExist = FALSE;

  UtlMakeEQFPath( pData->szFolSearchPath, NULC, PROPERTY_PATH, NULL );
  strcat( pData->szFolSearchPath, "\\*" );
  strcat( pData->szFolSearchPath, EXT_FOLDER_MAIN );

  hFolFindDir = FindFirstFile( pData->szFolSearchPath, &FileFindData );
  if ( hFolFindDir != INVALID_HANDLE_VALUE )
  {
    BOOL fMoreFiles = TRUE;
    do
    {
      if ( (FileFindData.nFileSizeHigh != 0) || (FileFindData.nFileSizeLow != 0) )
      {
        // check if we deal with a real file or the dummy file containing the folder long name
        if ( _stricmp( FileFindData.cFileName, szFolName ) != 0 )
        {
          fMoreFilesExist = TRUE;
        } /* endif */

        usRC = UtlAdjustFolderReferences( pData, FileFindData.cFileName );
      } /* endif */

      fMoreFiles = FindNextFile( hFolFindDir, &FileFindData );
    } while ( fMoreFiles );
    FindClose( hFolFindDir );
  } /* endif */

  return( 0 );
} /* end of function UtlAdjustReferences */


//+----------------------------------------------------------------------------+
// Internal function                                                            
//+----------------------------------------------------------------------------+
// Function name: UTLRENAMEDLG                                                  
//+----------------------------------------------------------------------------+
// Function call:                                                               
//   MRESULT EXPENTRY UTLRENAMEDLG( hwnd, msg, mp1, mp2 )                       
//+----------------------------------------------------------------------------+
// Description:                                                                 
//   This dialog procedure is the general "rename-an-object" procedure and can  
//   rename dictionaries, Translation Memory databases and Folders.             
//+----------------------------------------------------------------------------+
// Input parameter:                                                             
//   HWND    hwnd     handle of window                                          
//   USHORT  msg      type of message                                           
//   MPARAM  mp1      first message parameter                                   
//   MPARAM  mp2      second message parameter                                  
//+----------------------------------------------------------------------------+
// Output parameter:                                                            
//   none                                                                       
//+----------------------------------------------------------------------------+
// Returncode type: MRESULT                                                     
//+----------------------------------------------------------------------------+
// Returncodes:                                                                 
//   TRUE                                                                       
//   FALSE                                                                      
//+----------------------------------------------------------------------------+
// Prerequesits:                                                                
//   - the function expects to receive in the WM_INITDLG message the            
//     pointer to the dialog IDA                                                
//+----------------------------------------------------------------------------+
INT_PTR CALLBACK UTLRENAMEDLG
(
HWND   hwnd,                        // handle of window
WINMSG msg,                         // type of message
WPARAM mp1,                         // first message parameter
LPARAM mp2                          // second message parameter
)
{
  /********************************************************************/
  /*                         local variables                          */
  /********************************************************************/
  MRESULT              mResult = FALSE;// dlg procedure return value
  PRENAMEDLGIDA        pIDA;           // dialog IDA
  BOOL                 fOK;            // internal O.K. flag

  switch ( msg )
  {
    case WM_EQF_QUERYID:
      pIDA = ACCESSDLGIDA( hwnd, PRENAMEDLGIDA );
      HANDLEQUERYID( pIDA->sID, mp2 );
      break;

    case WM_HELP:


      pIDA = ACCESSDLGIDA( hwnd, PRENAMEDLGIDA );

      switch ( pIDA->ObjectClass )
      {
        case clsFOLDER :

          /*************************************************************/
          /* pass on a HELP_WM_HELP request                            */
          /*************************************************************/
          EqfDisplayContextHelp( (HWND) ((LPHELPINFO) mp2)->hItemHandle,
                                 &hlpsubtblRenameFolDlg[0] );
          mResult = TRUE;  // message processed
          break;

        case clsDICTIONARY :
          /*************************************************************/
          /* pass on a HELP_WM_HELP request                            */
          /*************************************************************/
          EqfDisplayContextHelp( (HWND) ((LPHELPINFO) mp2)->hItemHandle,
                                 &hlpsubtblRenameDictDlg[0] );
          mResult = TRUE;  // message processed
          break;

        case clsMEMORY :
          /*************************************************************/
          /* pass on a HELP_WM_HELP request                            */
          /*************************************************************/
          EqfDisplayContextHelp( (HWND) ((LPHELPINFO) mp2)->hItemHandle,
                                 &hlpsubtblRenameTMDlg[0] );
          mResult = TRUE;  // message processed
          break;

        default :
          break;
      } /* endswitch */
      break;

    case WM_INITDLG :
      {
        // save the pointer to the IDA in the dialog reserved memory
        pIDA = (PRENAMEDLGIDA)PVOIDFROMMP2( mp2 );
        ANCHORDLGIDA( hwnd, pIDA );

        // Keep dialog within TWB
        {
          SWP  swpDlg, swpTWB;

          // Get dialog size/position
          WinQueryWindowPos( hwnd, &swpDlg );

          // Center dialog within TWB
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
        }

        // do object type specific handling
        {
          SHORT sDlgTextID = 0;        // string ID for dialog title text
          SHORT sObjTextID = 0;        // string ID for object type name text

          switch ( pIDA->ObjectClass )
          {
            case clsFOLDER :
              pIDA->sID  = ID_RENAME_FOL_DLG;
              sDlgTextID = SID_RENAME_FOL_DLG;
              sObjTextID = SID_RENAME_FOL_OBJ_TEXT;
              // no referene adjusting for folders ...
              HIDECONTROL( hwnd, ID_RENAME_ADJUST_CHK );
              break;

            case clsDICTIONARY :
              pIDA->sID  = ID_RENAME_DICT_DLG;
              sDlgTextID = SID_RENAME_DICT_DLG;
              sObjTextID = SID_RENAME_DICT_OBJ_TEXT;
              break;

            case clsMEMORY :
              pIDA->sID  = ID_RENAME_TM_DLG;
              sDlgTextID = SID_RENAME_TM_DLG;
              sObjTextID = SID_RENAME_TM_OBJ_TEXT;
              break;

            default :
              break;
          } /* endswitch */

          if ( sDlgTextID )
          {
			HMODULE hResMod;
			hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
            LOADSTRING( NULLHANDLE, hResMod, sDlgTextID, pIDA->szBuffer );
            SETTEXTHWND( hwnd, pIDA->szBuffer );
          } /* endif */
          SETWINDOWID( hwnd, pIDA->sID );
          if ( sObjTextID )
          {
			HMODULE hResMod;
			hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
            SETTEXTFROMRES( hwnd, ID_RENAME_OBJ_TEXT, pIDA->szBuffer,
                            hResMod, sObjTextID );
          } /* endif */
        }

        // display old name
        OEMTOANSI( pIDA->szOldName );
        SETTEXT( hwnd, ID_RENAME_NAME_TEXT, pIDA->szOldName );
        ANSITOOEM( pIDA->szOldName );

        // set text limit for new name field
        SETTEXTLIMIT( hwnd, ID_RENAME_NEWNAME_EF, MAX_LONGFILESPEC - 1 );

        // preset new name field with old name
        OEMTOANSI( pIDA->szOldName );
        SETTEXT( hwnd, ID_RENAME_NEWNAME_EF, pIDA->szOldName );
        ANSITOOEM( pIDA->szOldName );

        // set focus to name entry field
        SETFOCUS( hwnd, ID_RENAME_NEWNAME_EF );

        // select all text in the edit control
        SendDlgItemMessage( hwnd, ID_RENAME_NEWNAME_EF, EM_SETSEL, (WPARAM)0, (LPARAM)-1 );

        // leave the focus where we put it
        mResult = MRFROMSHORT(DIALOGINITRETURN(TRUE));
      }
      break;

    case WM_COMMAND :
      {
        switch ( WMCOMMANDID( mp1, mp2 ) )
        {
          case ID_RENAME_RENAME_PB :
            // access dialog IDA
            fOK = TRUE;
            pIDA = ACCESSDLGIDA( hwnd, PRENAMEDLGIDA );

            // get new object name
            QUERYTEXT( hwnd, ID_RENAME_NEWNAME_EF, pIDA->szBuffer );
            ANSITOOEM( pIDA->szBuffer );
            UtlStripBlanks( pIDA->szBuffer );
            strcpy ( pIDA->szNewName, pIDA->szBuffer );

            // check if name has been specified
            if ( strlen(pIDA->szBuffer) == 0 )
            {
              UtlErrorHwnd( ERROR_NEW_NAME_MISSING, MB_CANCEL, 0, NULL, EQF_ERROR, hwnd );
              SETFOCUS( hwnd, ID_RENAME_NEWNAME_EF );
              fOK = FALSE;
            } /* endif */

            // check object name syntax
            if ( fOK )
            {
              if ( !UtlCheckLongName( pIDA->szBuffer ) )
              {
                PSZ    pszParm;          // points to error msg parameter
                pszParm = pIDA->szBuffer;
                OEMTOANSI( pIDA->szBuffer );
                UtlErrorHwnd( ERROR_INV_LONGNAME, MB_CANCEL, 1, &pszParm, EQF_ERROR, hwnd );
                SETFOCUS( hwnd, ID_RENAME_NEWNAME_EF );
                fOK = FALSE;
              } /* endif */
            } /* endif */

            // existence check
            if ( fOK )
            {
              if ( UtlCheckIfExist( pIDA->szNewName, (SHORT)pIDA->ObjectType )  )
              {
                USHORT usMsgNo = 0;      // number of message to be displayed
                PSZ    pszParm;          // points to error msg parameter

                // set message number
                switch ( pIDA->ObjectClass )
                {
                  case clsFOLDER:     usMsgNo = ERROR_NEWFOLDER_EXISTS; break;
                  case clsMEMORY:     usMsgNo = ERROR_MEM_NAME_INVALID; break;
                  case clsDICTIONARY: usMsgNo = ERROR_DICTIONARY_EXISTS; break;
                } /* endswitch */

                pszParm = pIDA->szNewName;
                OEMTOANSI( pIDA->szNewName );
                UtlErrorHwnd( usMsgNo, MB_CANCEL, 1, &pszParm, EQF_ERROR, hwnd );
                SETFOCUS( hwnd, ID_RENAME_NEWNAME_EF );
                fOK = FALSE;
              } /* endif */
            } /* endif */

            // get object short name (only for folder and dictioanry)
            if ( fOK )
            {
              switch ( pIDA->ObjectClass )
              {
                case clsFOLDER:     
                case clsDICTIONARY: 
                  {
                    BOOL fIsNew = FALSE;
                    strcpy( pIDA->szNewName, pIDA->szBuffer );
                    ObjLongToShortName( pIDA->szNewName, pIDA->szNewShortName,  pIDA->ObjectType, &fIsNew );

                  }
                  break;
                case clsMEMORY:     break;
              } /* endswitch */
            } /* endif */

            // let the handler for the object class do the actual renaming ...
            if ( fOK )
            {
              switch ( pIDA->ObjectClass )
              {
                case clsFOLDER :
                  // setup old and new name parameter
                  strcpy( pIDA->szBuffer, pIDA->szObject );
                  strcat( pIDA->szBuffer, X15_STR );
                  strcat( pIDA->szBuffer, pIDA->szNewShortName );
                  strcat( pIDA->szBuffer, X15_STR );
                  strcat( pIDA->szBuffer, pIDA->szNewName );

                  fOK = (BOOL)EqfSend2Handler( FOLDERHANDLER,
                                               WM_EQF_PROCESSTASK,
                                               MP1FROMSHORT(RENAMEOBJECT_TASK),
                                               MP2FROMP(pIDA->szBuffer) );
                  break;

                case clsMEMORY :
                  {
                    MemoryFactory *pFactory = MemoryFactory::getInstance();
                    if ( pFactory->renameMemory( NULL, pIDA->szOldName, pIDA->szNewName ) != 0 )
                    {
                      fOK = FALSE;
                      pFactory->showLastError( NULL, pIDA->szOldName, NULL, hwnd );
                    }
                  }
                  break;

                case clsDICTIONARY :
                  // setup old and new name parameter
                  strcpy( pIDA->szBuffer, pIDA->szObject );
                  strcat( pIDA->szBuffer, X15_STR );
                  strcat( pIDA->szBuffer, pIDA->szNewShortName );
                  strcat( pIDA->szBuffer, X15_STR );
                  strcat( pIDA->szBuffer, pIDA->szNewName );

                  fOK = (BOOL)EqfSend2Handler( DICTIONARYHANDLER,
                                               WM_EQF_PROCESSTASK,
                                               MP1FROMSHORT(RENAMEOBJECT_TASK),
                                               MP2FROMP(pIDA->szBuffer) );
                  break;
              } /* endswitch */
            } /* endif */

            // adjust references to object
            if ( fOK && QUERYCHECK( hwnd, ID_RENAME_ADJUST_CHK ) )
            {
              HWND  hwndLB = NULLHANDLE; // listbox for folder names
              HWND  hwndDocLB = NULLHANDLE; // listbox for document names
              SHORT sItem = 0;         // current listbox item
              SHORT sNumOfItems;       // number of listbox items
              BOOL  fPropChanged = FALSE;    // TRUE = properties have been changed
              CHAR  szFolName[MAX_FILESPEC]; // buffer for name of current folder
              CHAR  szFolder[MAX_FILESPEC];  // buffer for folder + extension

              // create invisible listbox for folder names
              hwndLB = WinCreateWindow( hwnd, WC_LISTBOX, "",
                                        WS_CHILD | LBS_STANDARD,
                                        0, 0, 0, 0, hwnd, HWND_TOP, 4711,
                                        NULL, NULL );

              hwndDocLB = WinCreateWindow( hwnd, WC_LISTBOX, "",
                                           WS_CHILD | LBS_STANDARD,
                                           0, 0, 0, 0, hwnd, HWND_TOP, 4712,
                                           NULL, NULL );

              // get names of all folders
              EqfSend2Handler( FOLDERLISTHANDLER, WM_EQF_INSERTNAMES,
                               MP1FROMHWND(hwndLB), 0L );

              // loop over all folders in listbox
              sNumOfItems = QUERYITEMCOUNTHWND( hwndLB );
              sItem = 0;
              while ( sItem < sNumOfItems )
              {
                fOK = TRUE;            // ignore any errors ...

                // get folder property file
                {
                  ULONG ulRead;
                  PVOID pvTemp = &pIDA->stFolProp;

                  // get folder name from folder listbox
                  QUERYITEMTEXTHWND( hwndLB, sItem, pIDA->szLongName );
                  ANSITOOEM( pIDA->szLongName );
                  {
                    BOOL fIsNew = FALSE;
                    ObjLongToShortName( pIDA->szLongName, szFolName,
                                        FOLDER_OBJECT, &fIsNew );
                  }

                  strcpy( szFolder, szFolName );
                  strcat( szFolder, EXT_FOLDER_MAIN );

                  // setup folder property file name
                  UtlMakeEQFPath( pIDA->szFolProp, NULC, PROPERTY_PATH, (PSZ) NULP );
                  strcat( pIDA->szFolProp, BACKSLASH_STR );
                  strcat( pIDA->szFolProp, szFolder );

                  // setup folder object name
                  UtlMakeEQFPath( pIDA->szFolObj, NULC, SYSTEM_PATH, (PSZ) NULP );
                  strcat( pIDA->szFolObj, BACKSLASH_STR );
                  strcat( pIDA->szFolObj, szFolder );

                  // load folder property file
                  fOK = UtlLoadFileL( pIDA->szFolProp, &pvTemp, &ulRead,
                                     FALSE, TRUE );
                }

                // adjust references in folder property file
                if ( fOK )
                {
                  // correct folder object name (use correct drive letter)
                  pIDA->szFolObj[0] = pIDA->stFolProp.chDrive;

                  // check property data
                  fPropChanged = FALSE;
                  switch ( pIDA->ObjectClass )
                  {
                    case clsMEMORY :
                      // check folder TM
                      if ( strcmp( pIDA->stFolProp.szMemory,
                                   pIDA->szOldShortName ) == 0 )
                      {
                        strcpy( pIDA->stFolProp.szMemory, pIDA->szNewShortName );
                        fPropChanged = TRUE;
                      } /* endif */
                      if ( strcmp( pIDA->stFolProp.szLongMemory,
                                   pIDA->szOldName ) == 0 )
                      {
                        strcpy( pIDA->stFolProp.szLongMemory, pIDA->szNewName );
                        fPropChanged = TRUE;
                      } /* endif */

                      if ( pIDA->stFolProp.aLongMemTbl[0][0] != EOS )
                      {
                        int i = 0;
                        while ( (i < MAX_NUM_OF_READONLY_MDB) &&
                                (pIDA->stFolProp.aLongMemTbl[i][0] != EOS) )
                        {
                          if ( strcmp( pIDA->szOldName, pIDA->stFolProp.aLongMemTbl[i] ) == 0 )
                          {
                            strcpy( pIDA->stFolProp.aLongMemTbl[i], pIDA->szNewName );
                            fPropChanged = TRUE;
                          } /* endif */
                          i++;
                        } /* endwhile */
                      } /* endif */

                      // get copy of read-only TM list
                      strcpy( pIDA->MemTbl, pIDA->stFolProp.MemTbl );

                      // parse copy of TM list and copy TMs back to properties
                      {
                        PSZ p;         // ptr to TM in list

                        p = strtok( pIDA->MemTbl, "\x15");
                        pIDA->stFolProp.MemTbl[0] = EOS; // clear list

                        for (; p; p=strtok( NULL, "\x15"))
                        {
                          if ( strcmp( pIDA->szOldShortName, p ) == 0 )
                          {
                            strcat( pIDA->stFolProp.MemTbl, pIDA->szNewShortName );
                            fPropChanged = TRUE;
                          }
                          else
                          {
                            strcat( pIDA->stFolProp.MemTbl, p );
                          } /* endif */
                          strcat( pIDA->stFolProp.MemTbl, X15_STR );
                        }
                      }
                      break;

                    case clsDICTIONARY :
                      if ( pIDA->stFolProp.aLongDicTbl[0][0] != EOS )
                      {
                        int i = 0;
                        while ( (i < MAX_NUM_OF_FOLDER_DICS) &&
                                (pIDA->stFolProp.aLongDicTbl[i][0] != EOS) )
                        {
                          if ( strcmp( pIDA->szOldName, pIDA->stFolProp.aLongDicTbl[i] ) == 0 )
                          {
                            strcpy( pIDA->stFolProp.aLongDicTbl[i], pIDA->szNewName );
                            fPropChanged = TRUE;
                          } /* endif */
                          i++;
                        } /* endwhile */
                      } /* endif */
                      // get copy of dict list
                      strcpy( pIDA->DicTbl, pIDA->stFolProp.DicTbl );

                      // parse copy of dict list and copy dicts back to properties
                      {
                        PSZ p;         // ptr to dict in list

                        p = strtok( pIDA->DicTbl, "\x15");
                        pIDA->stFolProp.DicTbl[0] = EOS; // clear list

                        for (; p; p=strtok( NULL, "\x15"))
                        {
                          if ( strcmp( pIDA->szOldShortName, p ) == 0 )
                          {
                            strcat( pIDA->stFolProp.DicTbl, pIDA->szNewShortName );
                            fPropChanged = TRUE;
                          }
                          else
                          {
                            strcat( pIDA->stFolProp.DicTbl, p );
                          } /* endif */
                          strcat( pIDA->stFolProp.DicTbl, X15_STR );
                        }
                      }
                      break;
                  } /* endswitch */
                } /* endif */

                // check if folder is currently in use
                if ( fOK )
                {
                  SHORT sRC = QUERYSYMBOL( pIDA->szFolObj );
                  if ( sRC != -1 )
                  {
                    PSZ pszErrParm = szFolName;
                    fOK = FALSE;
                    UtlError( ERROR_FOLDER_LOCKED, MB_CANCEL, 1,
                              &pszErrParm, EQF_INFO );
                  } /* endif */
                } /* endif */

                // rewrite folder property file if changed
                if ( fOK && fPropChanged )
                {
                  fOK = UtlWriteFile( pIDA->szFolProp, sizeof(pIDA->stFolProp),
                                      &pIDA->stFolProp, TRUE ) == NO_ERROR;

                  // broadcast props changed message
                  if ( fOK )
                  {
                    EqfSend2AllHandlers( WM_EQFN_PROPERTIESCHANGED,
                                         MP1FROMSHORT( PROP_CLASS_FOLDER ),
                                         MP2FROMP(pIDA->szFolObj) );
                  } /* endif */
                } /* endif */

                // loop over all folder documents (required only for TM!)
                if ( fOK && (pIDA->ObjectClass == clsMEMORY) )
                {
                  SHORT sDocItem;      // current item in document listbox
                  SHORT sNumOfDocs;    // number of documents
                  BOOL fDocOK;

                  // clear document listbox
                  DELETEALLHWND( hwndDocLB );

                  // get document names of current folder
                  EqfSend2Handler( FOLDERHANDLER, WM_EQF_INSERTNAMES,
                                   MP1FROMHWND(hwndDocLB),
                                   MP2FROMP(pIDA->szFolObj) );

                  // loop over documents in listbox
                  sDocItem = 0;
                  sNumOfDocs = QUERYITEMCOUNTHWND( hwndDocLB );
                  while ( fOK && (sDocItem < sNumOfDocs) )
                  {
                    CHAR szDocument[MAX_FILESPEC];

                    fPropChanged = FALSE;
                    fDocOK = TRUE;

                    // setup document property file name
                    QUERYITEMTEXTHWND( hwndDocLB, sDocItem, szDocument );
                    UtlMakeEQFPath( pIDA->szDocProp, pIDA->szFolObj[0],
                                    PROPERTY_PATH, szFolder );
                    strcat( pIDA->szDocProp, BACKSLASH_STR );
                    strcat( pIDA->szDocProp, szDocument );

                    // setup document object name
                    UtlMakeEQFPath( pIDA->szDocObj, pIDA->szFolObj[0],
                                    SYSTEM_PATH, szFolder );
                    strcat( pIDA->szDocObj, BACKSLASH_STR );
                    strcat( pIDA->szDocObj, szDocument );

                    // load document property file
                    {
                      PVOID pvDocProp = &pIDA->stDocProp;
                      ULONG ulRead;   // number of bytes read
                      fDocOK = UtlLoadFileL( pIDA->szDocProp, &pvDocProp, &ulRead,
                                            FALSE, FALSE );
                    }

                    // adjust references in document property file
                    if ( fDocOK )
                    {
                      if ( strcmp( pIDA->stDocProp.szLongMemory,
                                   pIDA->szOldName ) == 0 )
                      {
                        strcpy( pIDA->stDocProp.szLongMemory, pIDA->szNewName );
                        fPropChanged = TRUE;
                      } /* endif */
                      if ( strcmp( pIDA->stDocProp.szMemory,
                                   pIDA->szOldShortName ) == 0 )
                      {
                        strcpy( pIDA->stDocProp.szMemory, pIDA->szNewShortName );
                        fPropChanged = TRUE;
                      } /* endif */
                     } /* endif */

                    // check if document is currently in use
                    if ( fDocOK )
                    {
                      SHORT sRC = QUERYSYMBOL( pIDA->szDocObj );
                      if ( sRC != -1 )
                      {
                        PSZ pszErrParm = (pIDA->stDocProp.szLongName[0]) ?
                                         pIDA->stDocProp.szLongName :
                                         szDocument;
                        fDocOK = FALSE;
                        UtlError( ERROR_DOC_LOCKED, MB_CANCEL, 1,
                                  &pszErrParm, EQF_INFO );
                      } /* endif */
                    } /* endif */

                    // rewrite document property file if changed
                    if ( fDocOK && fPropChanged )
                    {
                      fDocOK = UtlWriteFile( pIDA->szDocProp, sizeof(pIDA->stDocProp),
                                             &pIDA->stDocProp, TRUE ) == NO_ERROR;

                      // broadcast props changed message
                      if ( fDocOK )
                      {
                        EqfSend2AllHandlers( WM_EQFN_PROPERTIESCHANGED,
                                             MP1FROMSHORT( PROP_CLASS_DOCUMENT ),
                                             MP2FROMP(pIDA->szDocObj) );
                      } /* endif */
                    } /* endif */

                    // next document
                    sDocItem++;
                  } /* endwhile */
                } /* endif */

                // loop over all subfolders (required only for TM!)
                if ( fOK && (pIDA->ObjectClass == clsMEMORY) )
                {
                  FILEFINDBUF stResultBuf;             // DOS file find struct
                  USHORT      usCount = 1;             // number of files requested
                  USHORT      usRC;                    // return code of called functions
                  HDIR        hDirHandle = HDIR_CREATE;// DosFind routine handle
                  PPROPFOLDER  pProp = NULL;           // subfolder properties

                  UtlQueryString( QST_PROPDIR, pIDA->szPropDir, sizeof(pIDA->szPropDir) );

                  // build subfolder search path
                  {
                    strcpy( pIDA->szSubFolSearchPath, pIDA->szFolObj );
                    strcat( pIDA->szSubFolSearchPath, BACKSLASH_STR );
                    strcat( pIDA->szSubFolSearchPath, pIDA->szPropDir );
                    strcat( pIDA->szSubFolSearchPath, "\\*" );
                    strcat( pIDA->szSubFolSearchPath, EXT_OF_SUBFOLDER );
                  }

                  usRC = UtlFindFirst( pIDA->szSubFolSearchPath, &hDirHandle, FILE_NORMAL, &stResultBuf, sizeof( stResultBuf), &usCount, 0L, 0);
                  while ( (usRC == NO_ERROR) && usCount )
                  {
                    ULONG ulLen;

                    // load subfolder properties and change TM name if required
                    sprintf( pIDA->szSubFolObj, "%s\\%s\\%s", pIDA->szFolObj, pIDA->szPropDir, RESBUFNAME(stResultBuf) );
                    if ( UtlLoadFileL( pIDA->szSubFolObj, (PVOID *) &pProp, &ulLen, FALSE, FALSE ) )
                    {
                      BOOL fPropChanged = FALSE;
                      if ( strcmp( pProp->szMemory, pIDA->szOldShortName ) == 0 )
                      {
                        strcpy( pProp->szMemory, pIDA->szNewShortName );
                        fPropChanged = TRUE;
                      } /* endif */
                      if ( strcmp( pProp->szLongMemory, pIDA->szOldName ) == 0 )
                      {
                        strcpy( pProp->szLongMemory, pIDA->szNewName );
                        fPropChanged = TRUE;
                      } /* endif */

                      if ( fPropChanged )
                      {
                        UtlWriteFileL( pIDA->szSubFolObj, ulLen, pProp, TRUE );
                      } /* endif */

                      UtlAlloc( (PVOID *) &pProp, 0L, 0L, NOMSG );
                      pProp = NULL;
                    } /* endif if UtlLoadFile */

                    usRC = UtlFindNext( hDirHandle, &stResultBuf, sizeof( stResultBuf), &usCount, 0);
                  } /* endwhile */

                  // close search file handle
                  if ( hDirHandle != HDIR_CREATE ) UtlFindClose( hDirHandle, FALSE );
                } /* endif */

                // next folder
                sItem++;
              } /* endwhile */

              if ( hwndLB != NULLHANDLE ) WinDestroyWindow( hwndLB );
              if ( hwndDocLB != NULLHANDLE ) WinDestroyWindow( hwndDocLB );
              fOK = TRUE;            // ignore any errors ...
            } /* endif */

            // cleanup

            // leave dialog
            if ( fOK )
            {
              WinDismissDlg( hwnd, TRUE );
            } /* endif */
            break;

          case ID_RENAME_CANCEL_PB :
          case DID_CANCEL :
            WinDismissDlg( hwnd, FALSE );
            break;

          case ID_RENAME_HELP_PB:
            UtlInvokeHelp();
            break;
        } /* endswitch */
      }
      break;

    default :
      mResult = WinDefDlgProc( hwnd, msg, mp1, mp2 );
      break;
  } /* endswitch */

  return( mResult );

} /* end of function UTLRENAMEDLG */


#else
USHORT UtlRenameObjectDlg( PSZ pszObjName, CLASSES ObjectClass )
{ return( NO_ERROR );}
#endif
#endif


// rename API call
USHORT UtlFuncRenameObject( USHORT usMode, PSZ pszName, PSZ pszNewName,LONG lOptions )
{
  USHORT      usRC = 0;
  PRENAMEDATA pData = NULL;

  // allocate our data area
  if ( !UtlAllocHwnd( (PVOID *)&pData, 0, sizeof(RENAMEDATA), ERROR_STORAGE, HWND_FUNCIF ) )
  {
    usRC = ERROR_NOT_ENOUGH_MEMORY;
  } /* endif */

  // check input data
  if ( !usRC )
  {
    if ( (pszName == NULL) || (*pszName == EOS) || (pszNewName == NULL) || (*pszNewName == EOS) )
    {
      usRC = WRONG_OPTIONS_RC;
    } /* endif */
  } /* endif */


  // check mode
  if ( !usRC )
  {
    if ( usMode == RENAME_FOLDER )
    {
      pData->ObjectClass = clsFOLDER;
      pData->ObjType     = FOLDER_OBJECT;
    }
    else if ( usMode == RENAME_MEMORY )    
    {
      pData->ObjectClass = clsMEMORY;
      pData->ObjType     = TM_OBJECT;
    }
    else if ( usMode == RENAME_DICTIONARY )    
    {
      pData->ObjectClass = clsDICTIONARY;
      pData->ObjType     = DICT_OBJECT;
    }
    else
    {
      usRC = WRONG_OPTIONS_RC;
    } /* endif */
  } /* endif */

  // check specified options
  if ( !usRC )
  {
    if ( (usMode == RENAME_MEMORY) || (usMode == RENAME_DICTIONARY) ) 
    {
      if ( (lOptions != 0) && (lOptions != ADJUSTREFERENCES_OPT) )
      {
        usRC = WRONG_OPTIONS_RC;
      } /* endif */
    }
    else
    {
      if ( lOptions != 0 )
      {
        usRC = WRONG_OPTIONS_RC;
      } /* endif */
    } /* endif */
  } /* endif */

  // check new name
  if ( !usRC)
  {
    if ( UtlCheckIfExist( pszNewName, (SHORT)pData->ObjType )  )
    {
      // set message number
      switch ( pData->ObjectClass )
      {
        case clsFOLDER:     usRC = ERROR_NEWFOLDER_EXISTS; break;
        case clsMEMORY:     usRC = ERROR_MEM_NAME_INVALID; break;
        case clsDICTIONARY: usRC = ERROR_NEWFOLDER_EXISTS; break;
      } /* endswitch */

      UtlErrorHwnd( usRC, MB_CANCEL, 1, &pszNewName, EQF_ERROR, HWND_FUNCIF );
    } /* endif */
  } /* endif */

  // use specific rename function depending on object type
  if ( !usRC )
  {
    // check existence and get object short name
    if ( !UtlCheckIfExist( pszName, (SHORT)pData->ObjType )  )
    {
      switch ( pData->ObjType )
      {
        case FOLDER_OBJECT: usRC = ERROR_XLATE_FOLDER_NOT_EXIST; break;
        case TM_OBJECT:     usRC = ERROR_MEMORY_NOTFOUND; break;
        case DICT_OBJECT:   usRC = ERROR_DIC_NOTFOUND; break;
      } /*endswitch */
      UtlErrorHwnd( usRC, MB_CANCEL, 1, &pszName, EQF_ERROR, HWND_FUNCIF );
    }
    else
    {
      switch ( pData->ObjType )
      {
        case FOLDER_OBJECT: 
          {
            BOOL fOK = TRUE;
            BOOL fIsNew = TRUE;

            // build object name for old object
            strcpy( pData->szOldName, pszName );
            strcpy( pData->szNewName, pszNewName );
            ObjLongToShortName( pData->szOldName, pData->szOldShortName, pData->ObjType, &fIsNew );
            UtlMakeEQFPath( pData->szOldObjName, NULC, SYSTEM_PATH, NULL );
            strcat( pData->szOldObjName, BACKSLASH_STR );
            strcat( pData->szOldObjName, pData->szOldShortName );
            strcat( pData->szOldObjName, EXT_FOLDER_MAIN ); 

            // build rename function input string
            ObjLongToShortName( pData->szNewName, pData->szNewShortName, pData->ObjType, &fIsNew );
            strcpy( pData->szOldAndNewName, pData->szOldObjName );
            strcat( pData->szOldAndNewName, X15_STR );
            strcat( pData->szOldAndNewName, pData->szNewShortName );
            strcat( pData->szOldAndNewName, X15_STR );
            strcat( pData->szOldAndNewName, pszNewName );

            // rename the folder
            fOK = FolRenameFolder( pData->szOldAndNewName, TRUE );
            if ( !fOK ) usRC = UtlQueryUShort( QS_LASTERRORMSGID );
          }
          break;

        case TM_OBJECT:
          {
            MemoryFactory *pFactory = MemoryFactory::getInstance();
            strcpy( pData->szOldName, pszName );
            strcpy( pData->szNewName, pszNewName );
            usRC = (USHORT)pFactory->renameMemory( NULL, pszName, pszNewName );

            if ( usRC != 0 ) 
                pFactory->showLastError( NULL, pszName, NULL, HWND_FUNCIF );
            else
            {
                // send notification to OpenTM2 GUI to update memory list
                HWND hwnd = FindWindow( TWBMAIN ,NULL);
                if(hwnd != NULL)
                {
                    OtmMemoryPlugin::MEMORYINFO info;
                    char chToDrive='\0';
                    if(pFactory->getMemoryInfo(NULL,pszNewName, &info)==0)
                       chToDrive= info.szFullPath[0];

                    if(chToDrive != '\0')
                    {
                        COPYDATASTRUCT cdsDel;
                        std::string strOldName(info.szPlugin);
                        strOldName = strOldName+":";
                        strOldName = strOldName+pszName;
                        cdsDel.dwData = 2;
                        cdsDel.cbData = (strOldName.size()+1)*sizeof(char);
                        cdsDel.lpData = (char*)strOldName.c_str();
                        SendMessage(hwnd,WM_COPYDATA, 0,(LPARAM)(LPVOID)&cdsDel);

                        COPYDATASTRUCT cdsAdd;
                        std::string strNewName(info.szPlugin);
                        strNewName = strNewName + ":";
                        strNewName = strNewName + pszNewName;
                        cdsAdd.dwData = 1;
                        cdsAdd.cbData = (strNewName.size()+1)*sizeof(char);
                        cdsAdd.lpData =  (char*)strNewName.c_str();
                        SendMessage(hwnd,WM_COPYDATA, MP1FROMCHAR(chToDrive),(LPARAM)(LPVOID)&cdsAdd);
                    }
                }//end if
            }//end else
          }
          break;

        case DICT_OBJECT:   
          {
            BOOL fOK = TRUE;
            BOOL fIsNew = TRUE;

            // build object name for old object
            strcpy( pData->szOldName, pszName );
            strcpy( pData->szNewName, pszNewName );
            ObjLongToShortName( pData->szOldName, pData->szOldShortName, pData->ObjType, &fIsNew );
            UtlMakeEQFPath( pData->szOldObjName, NULC, SYSTEM_PATH, NULL );
            strcat( pData->szOldObjName, BACKSLASH_STR );
            strcat( pData->szOldObjName, pData->szOldShortName );
            strcat( pData->szOldObjName, EXT_OF_DICTPROP ); 

            // build rename function input string
            strcpy( pData->szOldAndNewName, pData->szOldObjName );
            strcat( pData->szOldAndNewName, X15_STR );
            strcat( pData->szOldAndNewName, pData->szNewShortName );
            strcat( pData->szOldAndNewName, X15_STR );
            strcat( pData->szOldAndNewName, pszNewName );

            // rename the dictionary
            fOK = DicRenameDict( pData->szOldAndNewName, TRUE );
            if ( !fOK ) usRC = UtlQueryUShort( QS_LASTERRORMSGID );
          }
          break;
      } /*endswitch */
    } /* endif */

    if ( !usRC )
    {
      // adjust references
      if ( lOptions == ADJUSTREFERENCES_OPT )
      {
        usRC = UtlAdjustReferences( pData );
      } /* endif */
    } /* endif */
  } /* endif */

  // cleanup
  if ( pData ) UtlAllocHwnd( (PVOID *)&pData, 0, 0, NOMSG, HWND_FUNCIF  );

  return( usRC );
} /* end of function UtlFuncRenameObject */


// get short name API call
USHORT UtlFuncGetShortName( USHORT ObjectType, PSZ pszLongName, PSZ pszShortName )
{
  USHORT      usRC = 0;
  UTLCHECKOBJTYPES ObjType;                                // type of object 
  BOOL fIsNew = TRUE;

  // check input data
  if ( !usRC )
  {
    if ( (pszLongName == NULL) || (*pszLongName == EOS) || (pszShortName == NULL) )
    {
      return( WRONG_OPTIONS_RC );
    } /* endif */
  } /* endif */

  // check mode
  if ( ObjectType == FOLDER_OBJ )
  {
    ObjType     = FOLDER_OBJECT;
  }
  else if ( ObjectType == MEMORY_OBJ )    
  {
    ObjType     = TM_OBJECT;
  }
  else if ( ObjectType == DICT_OBJ )    
  {
    ObjType     = DICT_OBJECT;
  }
  else if ( ObjectType == DOCUMENT_OBJ )    
  {
    ObjType     = DOCUMENT_OBJECT;
  }
  else
  {
    return( WRONG_OPTIONS_RC );
  } /* endif */

  // get short name
  if ( ObjType != DOCUMENT_OBJECT )
  {
    // no pre-processing required for folders/dicts/memories

    // get short name of object
    ObjLongToShortName( pszLongName, pszShortName, ObjType, &fIsNew );

    if ( fIsNew )
    {
      // set message number
      switch ( ObjType )
      {
        case FOLDER_OBJECT: usRC = ERROR_XLATE_FOLDER_NOT_EXIST; break;
        case TM_OBJECT:     usRC = ERROR_MEMORY_NOTFOUND; break;
        case DICT_OBJECT:   usRC = ERROR_DIC_NOTFOUND; break;
      } /* endswitch */

      UtlErrorHwnd( usRC, MB_CANCEL, 1, &pszLongName, EQF_ERROR, HWND_FUNCIF );
      return( usRC );
    } /* endif */
  }
  else
  {
    OBJNAME szFolderObj;

    // split folder and document name
    PSZ pszDocName = strchr( pszLongName, ':' );
    if ( pszDocName == NULL )
    {
      usRC = INVALID_OS2_FILENAME;
      UtlErrorHwnd( usRC, MB_CANCEL, 1, &pszLongName, EQF_ERROR, HWND_FUNCIF );
      return( usRC );
    } /* endif */

    // split folder and doucment names
    *pszDocName = EOS;
    pszDocName++;

    // setup folder object name
    UtlMakeEQFPath( szFolderObj, NULC, SYSTEM_PATH, NULL );
    strcat( szFolderObj, BACKSLASH_STR );
    ObjLongToShortName( pszLongName, szFolderObj+strlen(szFolderObj), FOLDER_OBJECT, &fIsNew );
    strcat( szFolderObj, EXT_FOLDER_MAIN );

    // handle errors in folder name
    if ( fIsNew )
    {
      usRC = ERROR_XLATE_FOLDER_NOT_EXIST;
      UtlErrorHwnd( usRC, MB_CANCEL, 1, &pszLongName, EQF_ERROR, HWND_FUNCIF );
      pszDocName--;
      *pszDocName = ':';
      return( usRC );
    } /* end */         

    // get document short name
    FolLongToShortDocName( szFolderObj, pszDocName, pszShortName, &fIsNew );
    if ( fIsNew )
    {
      usRC = ERROR_TA_SOURCEFILE;
      UtlErrorHwnd( usRC, MB_CANCEL, 1, &pszDocName, EQF_ERROR, HWND_FUNCIF );
    } /* end */         

    pszDocName--;
    *pszDocName = ':';
  } /* end */       

  return( usRC );
} /* end of function UtlFuncGetShortName */

                                                                                               
USHORT UtlAddUserDlg
(
PSZ         pszObjName,              // name of object
CLASSES     ObjectClass              // class of object (e.g. clsFolder)                      
)
{
  USHORT      usRC = NO_ERROR;         // function return code
  PRENAMEDLGIDA pIDA = NULL;           // rename dialog IDA
  BOOL        fLocked = FALSE;         // object-has-been-locked flag

  ObjectClass;

  // check input parameters
  if ( (pszObjName == NULL) || (*pszObjName == EOS) )
  {
    usRC = ERROR_INVALID_DATA;
  }
  else if ( (strlen(pszObjName) + 1) >= sizeof(OBJNAME) )
  {
    usRC = ERROR_INVALID_DATA;
  }

  // allocate dialog IDA
  if ( usRC == NO_ERROR )
  {
    if ( !UtlAlloc( (PVOID *)&pIDA, 0L, sizeof(RENAMEDLGIDA), ERROR_STORAGE ) )
    {
        usRC = ERROR_NOT_ENOUGH_MEMORY;
    }
  }

  // check if object is in-use and lock object
  if ( usRC == NO_ERROR )
  {
	strcpy( pIDA->szObject, pszObjName );
    SHORT sRC = QUERYSYMBOL( pIDA->szObject );
    if ( sRC != -1 )
    {
      USHORT usMsgNo = 0;              // number of message to be displayed
      PSZ    pszParm;                  // points to error msg parameter
      pszParm = pIDA->szOldName;
      OEMTOANSI( pszParm );
      UtlError( usMsgNo, MB_CANCEL, 1, &pszParm, EQF_ERROR );
      ANSITOOEM( pszParm );
      usRC = ERROR_SHARING_VIOLATION;
    }
    else
    {
      SETSYMBOL(pIDA->szObject);
      fLocked = TRUE;
    }
  }

  // pop-up dialog
  if ( usRC == NO_ERROR )
  {
    BOOL fOK;
	HMODULE hResMod;

	hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
    DIALOGBOX( QUERYACTIVEWINDOW(), UTLADDUSERDLG, hResMod, ID_ADDUSER_DLG, pIDA, fOK );
  }

  // cleanup
  if ( fLocked ) REMOVESYMBOL(pIDA->szObject);
  if ( pIDA != NULL ) UtlAlloc( (PVOID *)&pIDA, 0L, 0L, NOMSG );
  
  // return to caller
  return( usRC );

}

INT_PTR CALLBACK UTLADDUSERDLG
(
HWND   hwnd,                        // handle of window
WINMSG msg,                         // type of message
WPARAM mp1,                         // first message parameter
LPARAM mp2                          // second message parameter
)
{
  /********************************************************************/
  /*                         local variables                          */
  /********************************************************************/
  MRESULT              mResult = FALSE;// dlg procedure return value
  PADDUSERDLGIDA        pIDA;           // dialog IDA

  switch ( msg )
  {
    case WM_EQF_QUERYID:
      pIDA = ACCESSDLGIDA( hwnd, PADDUSERDLGIDA );
      HANDLEQUERYID( pIDA->sID, mp2 );
      break;

    case WM_INITDLG :
      {
        // save the pointer to the IDA in the dialog reserved memory
        pIDA = (PADDUSERDLGIDA)PVOIDFROMMP2( mp2 );
        ANCHORDLGIDA( hwnd, pIDA );

        // Keep dialog within TWB
        {
          SWP  swpDlg, swpTWB;

          // Get dialog size/position
          WinQueryWindowPos( hwnd, &swpDlg );

          // Center dialog within TWB
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
        }

        // leave the focus where we put it
        mResult = MRFROMSHORT(DIALOGINITRETURN(TRUE));
      }
      break;

    case WM_COMMAND :
      {
        switch ( WMCOMMANDID( mp1, mp2 ) )
        {
          case ID_ADDUSER_ADD_PB :
            {
              // access dialog IDA
              pIDA = ACCESSDLGIDA( hwnd, PADDUSERDLGIDA );

              // get new object name
              QUERYTEXT( hwnd, ID_ADDUSER_DLG_USERID_EF, pIDA->szUserObject );
              ANSITOOEM( pIDA->szUserObject );

              // remove any leading and trailing blanks
              UtlStripBlanks( pIDA->szUserObject );

              // check if name has been specified
              if ( strlen(pIDA->szUserObject) == 0 )
              {
                UtlErrorHwnd( ERROR_NEW_NAME_MISSING, MB_CANCEL, 0, NULL, EQF_ERROR, hwnd );
                SETFOCUS( hwnd, ID_ADDUSER_DLG_USERID_EF );
                return FALSE;
              } 

              MemoryFactory *pFactory = MemoryFactory::getInstance();

              // existence check
              std::vector<std::string> users;
              std::string strError;
              int iRC = pFactory->listSharedMemoryUsers( "", pIDA->szObject,users,strError);
              if(iRC != 0)
              {
                 MessageBox(hwnd, strError.c_str(),NULL,MB_OK|MB_ICONWARNING);
                 return FALSE;
              }

              for ( std::size_t i = 0; i < users.size(); i++ )
              {
                if(users[i] == pIDA->szUserObject)
                {
                  MessageBox(hwnd, "User have existed",NULL,MB_OK|MB_ICONWARNING);
                  return FALSE;
                }
              }

              // send to handler to process it
              strError = "";
              iRC = pFactory->addSharedMemoryUser( "", pIDA->szObject, pIDA->szUserObject,strError);
              if(iRC == 0)
              {
                MessageBox(hwnd, "Add user successfully",NULL,MB_OK|MB_ICONINFORMATION);
                // leave the dialog
                WinDismissDlg( hwnd, TRUE );
              }
              else
              {
                MessageBox(hwnd, strError.c_str(),NULL,MB_OK|MB_ICONWARNING);
                return FALSE;
              }
              
            }
            break;

          case ID_ADDUSER_CANCEL_PB :
          case DID_CANCEL :
            WinDismissDlg( hwnd, FALSE );
            break;

        } /* endswitch */
      }
      break;

    default :
      mResult = WinDefDlgProc( hwnd, msg, mp1, mp2 );
      break;
  } /* endswitch */

  return( mResult );

} /* end of function UTLRENAMEDLG */

USHORT UtlRemoveUserDlg
(
PSZ         pszObjName,              // name of object
CLASSES     ObjectClass              // class of object (e.g. clsFolder)                      
)
{
  USHORT      usRC = NO_ERROR;         // function return code
  PRENAMEDLGIDA pIDA = NULL;           // rename dialog IDA

  ObjectClass;

  // check input parameters
  if ( (pszObjName == NULL) || (*pszObjName == EOS) )
  {
    usRC = ERROR_INVALID_DATA;
  }
  else if ( (strlen(pszObjName) + 1) >= sizeof(OBJNAME) )
  {
    usRC = ERROR_INVALID_DATA;
  }

  // allocate dialog IDA
  if ( usRC == NO_ERROR )
  {
    if ( !UtlAlloc( (PVOID *)&pIDA, 0L, sizeof(RENAMEDLGIDA), ERROR_STORAGE ) )
    {
        usRC = ERROR_NOT_ENOUGH_MEMORY;
    }
  }

  // pop-up dialog
  if ( usRC == NO_ERROR )
  {
    BOOL fOK;
	HMODULE hResMod;

	strcpy(pIDA->szObject, pszObjName);

	hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
    DIALOGBOX( QUERYACTIVEWINDOW(), UTLREMOVEUSERDLG, hResMod, ID_REMOVEUSER_DLG, pIDA, fOK );
  }

  if ( pIDA != NULL ) UtlAlloc( (PVOID *)&pIDA, 0L, 0L, NOMSG );
  
  // return to caller
  return( usRC );

}

INT_PTR CALLBACK UTLREMOVEUSERDLG
(
HWND   hwnd,                        // handle of window
WINMSG msg,                         // type of message
WPARAM mp1,                         // first message parameter
LPARAM mp2                          // second message parameter
)
{
  /********************************************************************/
  /*                         local variables                          */
  /********************************************************************/
  MRESULT              mResult = FALSE;// dlg procedure return value
  PREMOVEUSERIDA        pIDA;           // dialog IDA

  switch ( msg )
  {
    case WM_EQF_QUERYID:
      pIDA = ACCESSDLGIDA( hwnd, PREMOVEUSERIDA );
      HANDLEQUERYID( pIDA->sID, mp2 );
      break;

    case WM_INITDLG :
      {
        // save the pointer to the IDA in the dialog reserved memory
        pIDA = (PREMOVEUSERIDA)PVOIDFROMMP2( mp2 );
        ANCHORDLGIDA( hwnd, pIDA );

        // Keep dialog within TWB
        {
          SWP  swpDlg, swpTWB;

          // Get dialog size/position
          WinQueryWindowPos( hwnd, &swpDlg );

          // Center dialog within TWB
          WinQueryWindowPos( (HWND)UtlQueryULong( QL_TWBCLIENT ), &swpTWB );
          if ( (swpDlg.x > 0) && ((swpDlg.x + swpDlg.cx) < swpTWB.cx) )
          {
            swpDlg.x = (swpTWB.cx - swpDlg.cx) / 2;
          } /* endif */
          if ( (swpDlg.y > 0) && ((swpDlg.y + swpDlg.cy) < swpTWB.cy) )
          {
            swpDlg.y = (swpTWB.cy - swpDlg.cy) / 2;
          } /* endif */

          // list users
          MemoryFactory *pFactory = MemoryFactory::getInstance();
          std::vector<std::string> users;
          std::string strError;
          int iRC = pFactory->listSharedMemoryUsers( "", pIDA->szObject,users,strError);
          if(iRC != 0)
          {
             MessageBox(hwnd, strError.c_str(),NULL,MB_OK|MB_ICONWARNING);
             return FALSE;
          }

          for ( std::size_t i = 0; i < users.size(); i++ )
          {
             INSERTITEM( hwnd, ID_REMOVEUSER_USERS_LB, users[i].c_str() );
          }

          UtlCopyListBox( pIDA->hwndLB,WinWindowFromID( hwnd, ID_REMOVEUSER_USERS_LB ) );
          // adjust horizontal scrollbar
          UtlSetHorzScrollingForLB( GetDlgItem( hwnd, ID_REMOVEUSER_USERS_LB) );
          // set pos
          WinSetWindowPos( hwnd, HWND_TOP,
                           swpDlg.x, swpDlg.y, swpDlg.cx, swpDlg.cy,
                           EQF_SWP_MOVE |
                           EQF_SWP_SHOW | EQF_SWP_ACTIVATE );
        }

        // leave the focus where we put it
        mResult = MRFROMSHORT(DIALOGINITRETURN(TRUE));
      }
      break;

    case WM_COMMAND :
      {
        switch ( WMCOMMANDID( mp1, mp2 ) )
        {
          case ID_REMOVEUSER_REMOVE_PB :
           { 
               SHORT sItem = 0;
               std::string strError;
               pIDA = ACCESSDLGIDA( hwnd, PREMOVEUSERIDA );
               sItem = QUERYNEXTSELECTION( hwnd, ID_REMOVEUSER_USERS_LB,LIT_FIRST );
               pIDA->hwndLB = GetDlgItem( hwnd, ID_REMOVEUSER_USERS_LB);
               // At least select one user
               if(sItem == LIT_NONE)
               {
                   MessageBox(hwnd, "Please select user to move",NULL,MB_OK|MB_ICONWARNING);
               }
               // to remove from user list
               while ( sItem != LIT_NONE )
               {
                   MemoryFactory *pFactory = MemoryFactory::getInstance();
                   QUERYITEMTEXT( hwnd, ID_REMOVEUSER_USERS_LB, sItem, pIDA->szUserObject );
                   // call method to remove user
                   strError = "";
                   int iRC = pFactory->removeSharedMemoryUser( "", pIDA->szObject, pIDA->szUserObject,strError);
                   if(iRC == 0)
                   {
                       // remove it from list box
                       DELETEITEMHWND(pIDA->hwndLB, sItem );
                       // set srcollbar
                       UtlSetHorzScrollingForLB(pIDA->hwndLB);

                       MessageBox(hwnd, "Remove user successfully",NULL,MB_OK|MB_ICONINFORMATION);
                       // leave the dialog
                       WinDismissDlg( hwnd, TRUE );
                   }
                   else
                   {
                       MessageBox(hwnd, strError.c_str(),NULL,MB_OK|MB_ICONWARNING);
                       return FALSE;
                   }
                   // get next item
                   sItem = QUERYNEXTSELECTION( hwnd, ID_REMOVEUSER_USERS_LB,LIT_FIRST );
               }
              // leave the dialog
              //WinDismissDlg( hwnd, TRUE );
          }
            break;

          case ID_REMOVEUSER_CANCEL_PB :
          case DID_CANCEL :
            WinDismissDlg( hwnd, FALSE );
            break;

        } /* endswitch */
      }
      break;

    default :
      mResult = WinDefDlgProc( hwnd, msg, mp1, mp2 );
      break;
  } /* endswitch */

  return( mResult );

}


USHORT UtlListUserDlg
(
PSZ         pszObjName,              // name of object
CLASSES     ObjectClass              // class of object (e.g. clsFolder)                      
)
{
  USHORT      usRC = NO_ERROR;         // function return code
  PLISTUSERIDA pIDA = NULL;           // rename dialog IDA

  ObjectClass;

  // check input parameters
  if ( (pszObjName == NULL) || (*pszObjName == EOS) )
  {
    usRC = ERROR_INVALID_DATA;
  }
  else if ( (strlen(pszObjName) + 1) >= sizeof(OBJNAME) )
  {
    usRC = ERROR_INVALID_DATA;
  }

  // allocate dialog IDA
  if ( usRC == NO_ERROR )
  {
    if ( !UtlAlloc( (PVOID *)&pIDA, 0L, sizeof(LISTUSERIDA), ERROR_STORAGE ) )
    {
        usRC = ERROR_NOT_ENOUGH_MEMORY;
    }
  }

  // pop-up dialog
  if ( usRC == NO_ERROR )
  {
    BOOL fOK;
	HMODULE hResMod;

	strcpy(pIDA->szObject, pszObjName);

	hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
    DIALOGBOX( QUERYACTIVEWINDOW(), UTLLISTUSERDLG, hResMod, ID_LISTUSER_DLG, pIDA, fOK );
  }

  if ( pIDA != NULL ) UtlAlloc( (PVOID *)&pIDA, 0L, 0L, NOMSG );
  
  // return to caller
  return( usRC );

}


INT_PTR CALLBACK UTLLISTUSERDLG
(
HWND   hwnd,                        // handle of window
WINMSG msg,                         // type of message
WPARAM mp1,                         // first message parameter
LPARAM mp2                          // second message parameter
)
{
  /********************************************************************/
  /*                         local variables                          */
  /********************************************************************/
  MRESULT              mResult = FALSE;// dlg procedure return value
  PLISTUSERIDA        pIDA;           // dialog IDA

  switch ( msg )
  {
    case WM_EQF_QUERYID:
      pIDA = ACCESSDLGIDA( hwnd, PLISTUSERIDA );
      HANDLEQUERYID( pIDA->sID, mp2 );
      break;

    case WM_INITDLG :
      {
        // save the pointer to the IDA in the dialog reserved memory
        pIDA = (PLISTUSERIDA)PVOIDFROMMP2( mp2 );
        ANCHORDLGIDA( hwnd, pIDA );

        // Keep dialog within TWB
        {
          SWP  swpDlg, swpTWB;

          // Get dialog size/position
          WinQueryWindowPos( hwnd, &swpDlg );

          // Center dialog within TWB
          WinQueryWindowPos( (HWND)UtlQueryULong( QL_TWBCLIENT ), &swpTWB );
          if ( (swpDlg.x > 0) && ((swpDlg.x + swpDlg.cx) < swpTWB.cx) )
          {
            swpDlg.x = (swpTWB.cx - swpDlg.cx) / 2;
          } /* endif */
          if ( (swpDlg.y > 0) && ((swpDlg.y + swpDlg.cy) < swpTWB.cy) )
          {
            swpDlg.y = (swpTWB.cy - swpDlg.cy) / 2;
          } /* endif */

           // list users
          MemoryFactory *pFactory = MemoryFactory::getInstance();
          std::vector<std::string> users;
          std::string strError;
          int iRC = pFactory->listSharedMemoryUsers( "EqfSharedMemoryPlugin", pIDA->szObject,users,strError);
          if(iRC != 0)
          {
             MessageBox(hwnd, strError.c_str(),NULL,MB_OK|MB_ICONWARNING);
             return FALSE;
          }

          pIDA->hwndLB = GetDlgItem( hwnd, ID_LISTUSER_USERS_LB);
          for ( std::size_t i = 0; i < users.size(); i++ )
          {
             INSERTITEM( hwnd, ID_LISTUSER_USERS_LB, users[i].c_str() );
             // adjust horizontal scrollbar
             UtlSetHorzScrollingForLB(pIDA->hwndLB );
          }

          // set pos
          WinSetWindowPos( hwnd, HWND_TOP,
                           swpDlg.x, swpDlg.y, swpDlg.cx, swpDlg.cy,
                           EQF_SWP_MOVE |
                           EQF_SWP_SHOW | EQF_SWP_ACTIVATE );
        }

        // leave the focus where we put it
        mResult = MRFROMSHORT(DIALOGINITRETURN(TRUE));
      }
      break;

    case WM_COMMAND :
      {
        switch ( WMCOMMANDID( mp1, mp2 ) )
        {
          case ID_LISTUSER_CANCEL_PB :
          case DID_CANCEL :
            WinDismissDlg( hwnd, FALSE );
            break;
        } 
      }
      break;

    default :
      mResult = WinDefDlgProc( hwnd, msg, mp1, mp2 );
      break;
  } /* endswitch */

  return( mResult );

}

// check if there is an object of the given type
BOOL UtlCheckIfExist
(
  PSZ pszName,
  SHORT sObjType
)
{
  CHAR szPath[MAX_EQF_PATH];
  BOOL fExist = FALSE;

  switch ( sObjType )
  {
    case FOLDER_OBJECT :
      {
        CHAR szShortName[MAX_FILESPEC];
        BOOL fIsNew = FALSE;
        ObjLongToShortName( pszName, szShortName, (UTLCHECKOBJTYPES) sObjType, &fIsNew );
        fExist = !fIsNew;
      }
      break;

    case TM_OBJECT :
      {
        MemoryFactory *pFactory = MemoryFactory::getInstance();
        fExist = pFactory->exists( NULL, pszName );
      }
      break;

    case DICT_OBJECT :
      {
        CHAR szShortName[MAX_FILESPEC];
        BOOL fIsNew = FALSE;
        ObjLongToShortName( pszName, szShortName, (UTLCHECKOBJTYPES) sObjType, &fIsNew );
        fExist = !fIsNew;
      }
      break;

    case EDITOR_OBJECT :
      UtlMakeEQFPath( szPath, NULC, PROPERTY_PATH, NULL );
      strcat( szPath, BACKSLASH_STR );
      strcat( szPath, pszName );
      strcat( szPath, EXT_OF_EDITOR );
      fExist = UtlFileExist( szPath );
      break;

    case MARKUP_OBJECT :
      {
        OtmMarkupPlugin *pPlugin = GetMarkupPlugin( pszName ); 
        fExist = pPlugin != NULL;
      }
      break;

    case TARGET_LANGUAGE_OBJECT :
    case SOURCE_LANGUAGE_OBJECT :
      {
		    MorphFactory* pMorphFactory = MorphFactory::getInstance();
		    if ( pMorphFactory != NULL )
        {
          fExist = pMorphFactory->isSupported( pszName );
        } /* endif */
      }
      break;
  }

  return( fExist );
}

