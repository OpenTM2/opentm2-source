/*! \file
	Description: EQF Folder Handler dialog procedures - 2 -
	
	Copyright Notice:

	Copyright (C) 1990-2017, International Business Machines
	Corporation and others. All rights reserved
*/

#ifdef _DEBUG
//  #define SESSIONLOG
#endif

#define INCL_EQF_TAGTABLE         // tagtable defines and functions
#define INCL_EQF_PRINT            // print functions
#define INCL_EQF_FOLDER           // folder list and document list functions
#define INCL_EQF_ASD              // dictionary access functions
#define INCL_EQF_EDITORAPI        // editor API
#define INCL_EQF_TM             //
#define INCL_EQF_TP               // public translation processor functions
#define INCL_EQF_TAGTABLE         // tagtable defines and functions
#include <eqf.h>                  // General Translation Manager include file
#include "eqftmi.h"

#include "eqfutpck.h"                  // package utilities
#include "eqfdde.h"                    // batch mode functions
#include "eqffol00.h"                  // our .h stuff
#include "eqffol.id"                   // our ID file
#include "eqfsetup.h"             // our EQF subdirectory structure
#include <time.h>                      // C standard time functions
#include "EQFHLOG.H"                 // defines for history log processing

#include "eqfdoc00.h"
  #include "OTMFUNC.H"            // public defines for function call interface
  #include "EQFFUNCI.H"           // private defines for function call interface
  #include "EQFAPROF.H"           // analysis profile defines and functios

#include <string>
#include <vector>
#include "core\pluginmanager\PluginManager.h"
#include "core\pluginmanager\OtmMarkup.h"
#include "core\pluginmanager\OtmMarkupPlugin.h"
#include "core\pluginmanager\OtmMemory.h"
#include "core\pluginmanager\OtmMemoryPlugin.h"
#include "core\memory\MemoryFactory.h"
#include "MarkupPluginMapper.H"

// name of last used valued fro folder import directory
#define FOLIMPEXPLASTUSEDDIR "FOLDERIMPORTEXPORT"

HWND hwndImportInstance = NULLHANDLE;                  // handle of import instance window

extern HELPSUBTABLE hlpsubtblFolPropImpDlg[];
extern HELPSUBTABLE hlpsubtblFolPropGeneralImpDlg[];

  #include "SHLOBJ.H"                  // folder browse function

#define FOL_EXPORT_TASK USER_TASK + 1  // export-a-folder task
#define FOL_IMPORT_TASK USER_TASK + 2  // import-a-folder task


typedef int (* __cdecl FOLXLIFFIMPORT_DCL)( PFOLIMPEXPXLIFF ) ;
HMODULE hXLIFF2 = NULL ;
FOLXLIFFIMPORT_DCL pfnFOLXLIFFIMPORT = NULL ;

BOOL FolMergeMTLog( PSZ pszNewMTLog, PSZ pszTargetMTLog );
BOOL FolCheckFolderName( PSZ );
USHORT FolRefreshFolderListbox( HWND hwndDlg, PFOLIMPIDA pIda );
MRESULT FolImportCallBack( PPROCESSCOMMAREA, HWND, WINMSG, WPARAM, LPARAM );
BOOL FolImpFolLang
(
PFOLIMPIDA   pIda,                   // pointer to instance area
PPROPFOLDER  pProp,                  // ptr to properties of existing folder
// or NULL if no folder exists
HWND         hwnd,                   // handle of folder import process window
PSZ          pszNewFolder            // fully qualified name of property file
// of imported folder
);
VOID FolImpSetToDriveState
(
PFOLIMPIDA pIda,                    // ptr to folder import IDA
HWND       hwndDlg,                 // handle of import dialog window
PSZ        pszName                  // folder name
);

BOOL FolImpPreProcessDocs
(
PFOLIMPIDA pIda,                     // ptr to folder import IDA
PBOOL      pfUserInput               // ptr to user input flag
);

BOOL FolImpProcessDocs
(
PFOLIMPIDA pIda                      // ptr to folder import IDA
);

USHORT FolImpGetFileDate
(
PSZ         pszPath,                 // path of file being checked
FDATE       *pfileDate,              // ptr to buffer for file date
FTIME       *pfileTime               // ptr to buffer for file time
);

// macro to check if a FDATE value is all zero
#define ISFDATEZERO( fdate ) \
  ( (fdate.day == 0) && (fdate.month == 0) && (fdate.year == 0) )

// macro to check if a FTIME value is all zero
#define ISFTIMEZERO( ftime ) \
  ( (ftime.twosecs == 0) && (ftime.minutes == 0) && (ftime.hours == 0) )

VOID UtlAddImportToPath( PFOLIMPIDA pIda, PSZ pszPath );
VOID UtlRemoveImportFromPath( PFOLIMPIDA pIda, PSZ pszPath );

USHORT FolImpProcessFolder( HWND hwnd, PFOLIMPIDA pIda );
BOOL FolImpProcessTables( HWND hwnd, PFOLIMPIDA pIda );
USHORT FolImpProcessSubFolders( HWND hwnd, PFOLIMPIDA pIda );
BOOL FolImpBuildAddTagTableFilesList( PFOLIMPIDA pIda );
void FolImpImportPathChanged( HWND hwnd, PFOLIMPIDA pIda );

HWND GetFolderImportHandle()
{
  return( hwndImportInstance );
}

USHORT FolFuncPrepImportFolder
(
  PFCTDATA    pData,                   // function I/F session data
  PSZ         pszFolderName,           // name of folder being imported
  PSZ         pszFromPath,             // path containing the imported folder
  CHAR        chToDrive,               // target drive for folder
  PSZ         pszNewFolderName,        // new name for the folder
  PSZ         pszNewMemNames,          // opt: comma separated list of new memory names, the first entry is used to rename the folder memory, the remaining names are used to rename the search memories
  LONG        lOptions                 // folder import options
);

USHORT FolFuncImportFolderProcess
(
  PFCTDATA    pData                    // function I/F session data
);

BOOL FolImpGetFolderDrive
(
  PFOLIMPIDA pIda,
  PSZ pszName,
  CHAR *pchDrive
);

// check if file dates are valid
BOOL FolCheckFileDates
(
  PSZ    pszImportedObject,            // full path name of object being imported
  PSZ    pszExistingObject,            // full path name of existing object
  HWND   hwndErrMsg,                   // parent handle for error messages
  USHORT usMsgExisting,                // number of message to be displayed for existing
  USHORT usMsgImported                 // number of message to be displayed for imported
);

USHORT FolAddRecordToTable( PHISTLOGRECORD *ppTable, PULONG pulTableSize, PULONG pulTableUsed, PHISTLOGRECORD pRecord );
BOOL FolImp_ControlledFolderMergeConfirmation( HWND hwnd, PSZ pszPassword, PSZ pszFolderName, BOOL fMerge );
INT_PTR CALLBACK FolImp_PasswordDlg( HWND hwndDlg, WINMSG message, WPARAM mp1, LPARAM mp2 );

void FolImpBuildUniqueImportPath( PFOLIMPIDA pIda );

// data area for the import of the memory files
typedef struct _MEMIMPORTDATA
{
  OtmMemoryPlugin *pMemPlugin;
  std::string strPluginName;
  std::string strMemFiles;
  std::string strMemoryName;
  PVOID pPrivPluginData;
  BOOL   fNewMemory;
} MEMIMPORTDATA, *PMEMIMPORTDATA;

BOOL FolImpScanMemoryInfoFile( PFOLIMPIDA pIda, PSZ pszInfoFile );
BOOL FolImpContinueMemoryImport( PFOLIMPIDA pIda, PBOOL pfContinue );
BOOL FolImpProcessNextMemory( PFOLIMPIDA pIda, PBOOL pfMoreMemories, PBOOL pfContinue  );
BOOL FolImpStartMemoryImport( PFOLIMPIDA pIda, PBOOL pfContinue );

// compare two characters and return result
static int cCompare( const void *arg1, const void *arg2 )
{
  return( *((PCHAR)arg1) - *((PCHAR)arg2) );
}/* end cCompare */

int ListXLiffFolder( PFOLIMPIDA pIda, HWND hwndDlg, PSZ pszPath, PSZ pszPattern );


MRESULT FolImpControl
(
HWND hwndDlg,
SHORT  sId,                         // id in action
SHORT  sNotification                // notification
)
{
  MRESULT mResult = FALSE;
  PFOLIMPIDA pIda;                    // ptr to folder import IDA
  SHORT   sItem;                      // index of selected listbox item
  ULONG   ulHeaderSize;               // size of package's user header
  ULONG   ulRC;                       // retrun code of UtlPack functions
  PCHAR       pchDrive;               // ptr for drive list processing
  BOOL        fOK;                    // internal OK flag


  pIda = ACCESSDLGIDA( hwndDlg, PFOLIMPIDA );

  switch ( sId )
  {
    case ID_FOLIMP_RENAME_CHK:
      {
        pIda->fImportAs = QUERYCHECK( hwndDlg, ID_FOLIMP_RENAME_CHK );

        if ( pIda->fImportAs )
        {
          ENABLECTRL( hwndDlg, ID_FOLIMP_NEWNAME_CB, TRUE );
          EqfSend2Handler( FOLDERLISTHANDLER, WM_EQF_INSERTNAMES,
                           MP1FROMHWND( GETHANDLEFROMID( hwndDlg, ID_FOLIMP_NEWNAME_CB) ),
                           0L );
          SETTEXT( hwndDlg, ID_FOLIMP_NEWNAME_CB, pIda->szLongFolName );
        }
        else
        {
          ENABLECTRL( hwndDlg, ID_FOLIMP_NEWNAME_CB, FALSE );
          CBDELETEALL( hwndDlg, ID_FOLIMP_NEWNAME_CB );
          SETTEXT( hwndDlg, ID_FOLIMP_NEWNAME_CB, "" );
        } /* endif */
      }
      break;

    case ID_FOLIMP_NEWNAME_CB:
      {

        switch ( sNotification )
        {
          case CBN_SELCHANGE :
          case CBN_EFCHANGE :
            {
              pIda->fImportAs = QUERYCHECK( hwndDlg, ID_FOLIMP_RENAME_CHK);
              if ( pIda->fImportAs )
              {
                QUERYTEXT( hwndDlg, ID_FOLIMP_NEWNAME_CB, pIda->szLongBuffer );
                UtlStripBlanks( pIda->szLongBuffer );
                ANSITOOEM( pIda->szLongBuffer );
                FolImpSetToDriveState( pIda, hwndDlg, pIda->szLongBuffer );
              } /* endif */
            }
            break;
        } /* endswitch */
      }
      break;

    case ID_FOLIMP_FOLDER_LB:
      switch ( sNotification )
      {
        case LN_SELECT:
          sItem = QUERYSELECTION( hwndDlg, ID_FOLIMP_FOLDER_LB );
          if ( sItem != LIT_NONE )
          {
            //--- handle any previously processed packages ---
            if ( pIda->pPackCtrl )            // is there an open package ???
            {
              UtlPackClosePackage( pIda->pPackCtrl ); // ... close it
              pIda->pPackCtrl = NULL;
            } /* endif */
            if ( pIda->pHeader )              // is there a package header???
            {
              UtlAlloc( (PVOID *)&pIda->pHeader, 0L, 0L, NOMSG ); // ... discard it
              pIda->pHeader = NULL;
            } /* endif */

            //--- uncheck rename pushbutton
            SETCHECK_FALSE( hwndDlg, ID_FOLIMP_RENAME_CHK );
            ENABLECTRL( hwndDlg, ID_FOLIMP_NEWNAME_CB, FALSE );
            CBDELETEALL( hwndDlg, ID_FOLIMP_NEWNAME_CB );
            SETTEXT( hwndDlg, ID_FOLIMP_NEWNAME_CB, "" );

            //--- get package name and open the package ---
            fOK = TRUE;             // assume everything is OK

            if ( pIda->usImpMode == FOL_TMGR_FORMAT )
            {
                // get folder short name from our invisible short name listbox
                {
                  SHORT sShortIndex = (SHORT)QUERYITEMHANDLE( hwndDlg,
                                                              ID_FOLIMP_FOLDER_LB,
                                                              sItem );
                  QUERYITEMTEXT( hwndDlg, ID_FOLIMP_FOLDER_LB,
                                sItem, pIda->szLongFolName );
                  QUERYITEMTEXTHWND( pIda->hwndShortNameLB, sShortIndex,
                                    pIda->szSourceFolName );
                }
                UtlUpper( pIda->szSourceFolName );
                strcpy( pIda->szObjPath, pIda->szSourceFolName );
                strcat( pIda->szObjPath, FOLDEREXPORT_EXT );
                pIda->pPackCtrl = UtlPackOpenPackage( pIda->chFromDrive, NULL, NULL,
                                                      pIda->szObjPath,
                                                      hwndDlg );

                //--- read package's user header ---
                if ( pIda->pPackCtrl )            // if the package is open ...
                {
                  fOK = TRUE;
                  ulHeaderSize = UtlPackQueryHeaderSize( pIda->pPackCtrl );
                  if ( ulHeaderSize )
                  {
                    UtlAlloc( (PVOID *)&pIda->pHeader, 0L, (LONG) ulHeaderSize,
                              ERROR_STORAGE );
                    if ( pIda->pHeader )
                    {
                      ulRC = UtlPackReadHeader( pIda->pPackCtrl,
                                                (PBYTE) pIda->pHeader,
                                                ulHeaderSize );
                      if ( ulRC == PACK_OK_RC )
                      {
                        //--- allow usage of INFO button ---
                        ENABLECTRL( hwndDlg, ID_FOLIMP_INFO_PB, TRUE );
                        ENABLECTRL( hwndDlg, ID_FOLIMP_IMPORT_PB, TRUE );
                        SETTABSTOP( hwndDlg, ID_FOLIMP_CANCEL_PB );

                        ENABLECTRL( hwndDlg, ID_FOLIMP_WITHDICT_CHK, pIda->pHeader->fContainsDict );
                        SETCHECK( hwndDlg, ID_FOLIMP_WITHDICT_CHK, pIda->pHeader->fContainsDict );
                        ENABLECTRL( hwndDlg, ID_FOLIMP_WITHMEM_CHK, pIda->pHeader->BitFlags.fContainsMem );
                        SETCHECK( hwndDlg, ID_FOLIMP_WITHMEM_CHK, pIda->pHeader->BitFlags.fContainsMem );
                      }
                      else
                      {
                        // read of header failed
                        ENABLECTRL( hwndDlg, ID_FOLIMP_INFO_PB, FALSE );
                        ENABLECTRL( hwndDlg, ID_FOLIMP_IMPORT_PB, FALSE );
                        fOK = FALSE;
                        UtlAlloc( (PVOID *)&pIda->pHeader, 0L, 0L, NOMSG );
                        UtlPackClosePackage( pIda->pPackCtrl );
                        pIda->pPackCtrl = NULL;
                      } /* endif */
                    }
                    else
                    {
                      // alloc failed
                      fOK = FALSE;
                      UtlPackClosePackage( pIda->pPackCtrl );
                      pIda->pPackCtrl = NULL;
                    } /* endif */
                  }
                  else
                  {
                    // package has no user header
                    ENABLECTRL( hwndDlg, ID_FOLIMP_INFO_PB, FALSE );
                    ENABLECTRL( hwndDlg, ID_FOLIMP_IMPORT_PB, FALSE );
                    CLEARTABSTOP( hwndDlg, ID_FOLIMP_CANCEL_PB );
                    ENABLECTRL( hwndDlg, ID_FOLIMP_WITHDICT_CHK, FALSE );
                    SETCHECK_FALSE( hwndDlg, ID_FOLIMP_WITHDICT_CHK );
                    ENABLECTRL( hwndDlg, ID_FOLIMP_WITHMEM_CHK, FALSE );
                    SETCHECK_FALSE( hwndDlg, ID_FOLIMP_WITHMEM_CHK );
                  } /* endif */
                }
                else
                {
                  fOK = FALSE;         // error: package did not open
                } /* endif */

                //--- check if folder exists somethere and set TO drives ---
                if ( fOK )
                {
                  strcpy( pIda->szSourceFolder, pIda->szSourceFolName );
                  strcat( pIda->szSourceFolder, EXT_FOLDER_MAIN );
                  ANSITOOEM( pIda->szLongFolName );
                  FolImpSetToDriveState( pIda, hwndDlg, pIda->szLongFolName );
                }
                else
                {
                  // something failed, cleanup
                  ENABLECTRL( hwndDlg, ID_FOLIMP_INFO_PB, FALSE );
                  ENABLECTRL( hwndDlg, ID_FOLIMP_IMPORT_PB, FALSE );
                  SETTABSTOP( hwndDlg, ID_FOLIMP_CANCEL_PB );
                  ENABLECTRL( hwndDlg, ID_FOLIMP_WITHDICT_CHK, FALSE );
                  SETCHECK_FALSE( hwndDlg, ID_FOLIMP_WITHDICT_CHK );
                  ENABLECTRL( hwndDlg, ID_FOLIMP_WITHMEM_CHK, FALSE );
                  SETCHECK_FALSE( hwndDlg, ID_FOLIMP_WITHMEM_CHK );
                  DESELECTITEM( hwndDlg, ID_FOLIMP_FOLDER_LB, sItem );
                } /* endif */
              }
              else
              {
                // get XLIFF file name from our invisible short name listbox
                int  iShortIndex = QUERYITEMHANDLE( hwndDlg, ID_FOLIMP_FOLDER_LB, sItem );
                QUERYITEMTEXT( hwndDlg, ID_FOLIMP_FOLDER_LB, sItem, pIda->szLongFolName );
                QUERYITEMTEXTHWND( pIda->hwndShortNameLB, iShortIndex, pIda->szXliffFileName );

                ENABLECTRL( hwndDlg, ID_FOLIMP_INFO_PB, FALSE );
                ENABLECTRL( hwndDlg, ID_FOLIMP_IMPORT_PB, TRUE );
                ENABLECTRL( hwndDlg, ID_FOLIMP_WITHDICT_CHK, FALSE );
                SETCHECK_FALSE( hwndDlg, ID_FOLIMP_WITHDICT_CHK );
                SETCHECK_FALSE( hwndDlg, ID_FOLIMP_RENAME_CHK );
                ENABLECTRL( hwndDlg, ID_FOLIMP_NEWNAME_CB, FALSE );
              } /* endif */
            }
            else
            {
              ENABLECTRL( hwndDlg, ID_FOLIMP_INFO_PB, FALSE );
              ENABLECTRL( hwndDlg, ID_FOLIMP_IMPORT_PB, FALSE );
              SETTABSTOP( hwndDlg, ID_FOLIMP_CANCEL_PB );
              if ( pIda->chToDrive != ' ' )
              {
                SETDRIVE( hwndDlg,
                          IDFROMDRIVE( ID_TODRIVEA_BTN, pIda->chToDrive ),
                          FALSE );
              } /* endif */
              for ( pchDrive = pIda->szToDriveList; *pchDrive; pchDrive++ )
              {
                ENABLECTRL( hwndDlg,
                            IDFROMDRIVE( ID_TODRIVEA_BTN, *pchDrive ),
                            TRUE );
              } /* endfor */
            } /* endif */
          break;
      } /* endswitch */
      break;
  } /* endswitch */
  return( mResult );
} /* end of FolImpControl */





VOID FolImpSetToDriveState
(
PFOLIMPIDA pIda,                    // ptr to folder import IDA
HWND       hwndDlg,                 // handle of import dialog window
PSZ        pszName                  // folder name
)
{
  BOOL        fFolderExists = FALSE;   // assume that no folder exists
  CHAR        chFolderDrive = pIda->chSysDrive;

  fFolderExists = FolImpGetFolderDrive( pIda, pszName, &chFolderDrive );

  if ( fFolderExists )
  {
    // select folder drive
    int i = 0;
    int iMax = CBQUERYITEMCOUNT( hwndDlg, ID_FOLIMP_TODRIVE_PB );
    for ( i = 0; i < iMax; i++ )
    {
      int nDrive = CBQUERYITEMHANDLE( hwndDlg,ID_FOLIMP_TODRIVE_PB, i );
      CHAR chDrive = (CHAR)('A' + nDrive);
      if ( chDrive == chFolderDrive )
      {
        CBSELECTITEM( hwndDlg, ID_FOLIMP_TODRIVE_PB, i );
      } /* endif */
    } /* endfor */

    // disable combobox
    ENABLECTRL( hwndDlg, ID_FOLIMP_TODRIVE_PB, FALSE );
  }
  else
  {
    // enable combobox
    ENABLECTRL( hwndDlg, ID_FOLIMP_TODRIVE_PB, TRUE );
  } /* endif */

// old code for drive buttons
//  for ( pchDrive = pIda->szToDriveList; *pchDrive; pchDrive++ )
//  {
//    hwndDrive = WinWindowFromID( hwndDlg, IDFROMDRIVE( ID_TODRIVEA_BTN, *pchDrive ));
//    ENABLECTRLHWND( hwndDrive, !fFolderExists );
//  } /* endfor */

  pIda->fExists =  fFolderExists;

//  WinPostMsg( hwndDlg, WM_COMMAND, MP1FROMSHORT( IDFROMDRIVE( ID_TODRIVEA_BTN,
//              chFolderDrive ) ), 0L );

  return;
} /* end of function FolImpSetToDriveState */

// FolImpComand is used by properties folder import dialog for drive button
// handling, all other functions have been removed from FolImpCommand
MRESULT FolImpCommand
(
HWND hwndDlg,
SHORT sId,                          // id of button
SHORT sNotification                 // notification type
)
{
  MRESULT mResult = MRFROMSHORT(TRUE);// TRUE = command is processed
  PFOLIMPIDA pIda;                    // ptr to folder import IDA
  PCHAR       pchDrive;               // ptr for drive list processing

  sNotification;

  // --- get IDA pointer ---
  pIda = ACCESSDLGIDA( hwndDlg, PFOLIMPIDA );

  //--- check for drive buttons ---
  if ( (sId >= ID_FROMDRIVEA_BTN) && (sId <= ID_FROMDRIVEZ_BTN)  )
  {
    //--- deselect any previously selected drive button ---
    if ( pIda->chFromDrive != ' ' )
    {
      SETDRIVE( hwndDlg,
                IDFROMDRIVE( ID_FROMDRIVEA_BTN, pIda->chFromDrive ),
                FALSE );
    } /* endif */

    //--- select new drive button ---
    pIda->chFromDrive = DRIVEFROMID( ID_FROMDRIVEA_BTN, sId );
    SETDRIVE( hwndDlg, sId, TRUE );

    //--- refresh packaged folders listbox ---
    FolRefreshFolderListbox( hwndDlg, pIda );
    if ( QUERYITEMCOUNT( hwndDlg, ID_FOLIMP_FOLDER_LB ) == 0 )
    {
      ENABLECTRL( hwndDlg, ID_FOLIMP_INFO_PB, FALSE );
      ENABLECTRL( hwndDlg, ID_FOLIMP_IMPORT_PB, FALSE );
      SETTABSTOP( hwndDlg, ID_FOLIMP_CANCEL_PB );
      if ( pIda->chToDrive != ' ' )
      {
        SETDRIVE( hwndDlg,
                  IDFROMDRIVE( ID_TODRIVEA_BTN, pIda->chToDrive ),
                  FALSE );
      } /* endif */
      for ( pchDrive = pIda->szToDriveList; *pchDrive; pchDrive++ )
      {
        ENABLECTRL( hwndDlg,
                    IDFROMDRIVE( ID_TODRIVEA_BTN, *pchDrive ),
                    TRUE );
      } /* endfor */
    }
    else
    {
      SELECTITEM( hwndDlg, ID_FOLIMP_FOLDER_LB, 0 );
    } /* endif */

  }
  else
    if ( (sId >= ID_TODRIVEA_BTN) && (sId <= ID_TODRIVEZ_BTN) )
  {
    // deselect any previously selected drive button ---
    if ( pIda->chToDrive != ' ' )
    {
      SETDRIVE( hwndDlg,
                IDFROMDRIVE( ID_TODRIVEA_BTN, pIda->chToDrive ),
                FALSE );
    } /* endif */
    pIda->chToDrive = DRIVEFROMID( ID_TODRIVEA_BTN, sId );
    SETDRIVE( hwndDlg, sId, TRUE );
  } /* endif */

  return( mResult );
} /* end of FolImpCommand */

MRESULT FolImpClose
(
HWND hwndDlg,
WPARAM mp1,
LPARAM mp2
)
{
  MRESULT mResult = FALSE;
  PFOLIMPIDA pIda;                    // ptr to folder import IDA
  EQFINFO     ErrorInfo;              // error code of property handler calls

  mp2 = mp2;                          // supress 'unreferenced parameter' msg

  // get IDA pointer
  pIda = ACCESSDLGIDA( hwndDlg, PFOLIMPIDA );

  if ( pIda )
  {
    //--- close folderlist properties ---
    CloseProperties( pIda->hFllProp, PROP_QUIT, &ErrorInfo);

    //--- destroy our short name listbox
    if ( pIda->hwndShortNameLB != NULLHANDLE )
    {
      WinDestroyWindow( pIda->hwndShortNameLB );
    } /* endif */
  } /* endif */

  //--- get rid off folder import dialog ---
  DISMISSDLG( hwndDlg, (BOOL)mp1 );

  return( mResult );
} /* end of FolImpClose */

// fill memory info of info tab
void FolImpInfoFillMemInfo( PFOLIMPIDA  pIda, HWND hwndDlg )
{
  PFILELIST   pFileList;              // ptr to package's file list
//USHORT      usNoOfEntries;          // # of enteries in package's file list
  BOOL        fTranslationManagerFolder = FALSE;

  UtlPackGetFileListPtr( pIda->pPackCtrl, &pFileList );

  if ( pFileList == NULL )
  {
    // can't access file list
    return;
  } /* endif */

  // find and access folder property file to get type of imported folder
  {
    USHORT usNoOfEntries = (USHORT)pFileList->ulListUsed;
    PFILELISTENTRY pFile = pFileList->pEntries;
    while ( usNoOfEntries )
    {
      if ( pFile->usFileType == FOLDER_PROP_FILE )
      {
//      PPROP_NTM pProp = NULL;

        pIda->pszBuffer[0] = EOS;
        UtlPackReadFilePart( pIda->pPackCtrl, pFile->pszName, 0, (PBYTE)pIda->pszBuffer, sizeof(PROPHEAD) );  
        fTranslationManagerFolder = ( stricmp( ((PPROPHEAD)pIda->pszBuffer)->szPath + 2, "\\EQF" ) == 0 );
        break;              // leave while loop
      } /* endif */
      usNoOfEntries--;            // skip to next entry in file list
      pFile++;
    } /* endwhile */
  } 

  // process memory files for TranslationManager folder
  if ( fTranslationManagerFolder ) 
  {
    //--- set folder exported memory static ---
    switch ( pIda->pHeader->BitFlags.fHeaderType )
    {
      case RELEASE0_HEADER:
        // get memory file name from memory name variable in header
        SETTEXT( hwndDlg, ID_FOLINFO_MEM_TEXT, pIda->pHeader->Head.Rel0.szMemory );
        break;
      case RELEASE1_HEADER:
        // get memory file name from list of packaged files
        if ( pFileList )
        {
          PFILELISTENTRY pFile = pFileList->pEntries;
          USHORT usNoOfEntries = (USHORT)pFileList->ulListUsed;
          while ( usNoOfEntries )
          {
            if ( pFile->usFileType == MEMORY_PROP_FILE )
            {
              PPROP_NTM pProp = NULL;

              pIda->pszBuffer[0] = EOS;
              UtlPackReadFilePart( pIda->pPackCtrl, pFile->pszName, (PBYTE)&(pProp->szLongName) - (PBYTE)pProp, (PBYTE)pIda->pszBuffer, MAX_LONGFILESPEC );
              if ( pIda->pszBuffer[0] == EOS )
              {
                Utlstrccpy( pIda->pszBuffer, UtlGetFnameFromPath( pFile->pszName ), DOT );
              } /* endif */
              SETTEXT( hwndDlg, ID_FOLINFO_MEM_TEXT, pIda->pszBuffer );
              break;              // leave while loop
            } /* endif */

            usNoOfEntries--;            // skip to next entry in file list
            pFile++;
          } /* endwhile */
        } /* endif */
        break;
      default:
        break;
    } /* endswitch */

    //--- fill Translation Memory list box
    switch ( pIda->pHeader->BitFlags.fHeaderType )
    {
      case RELEASE0_HEADER:

        DELETEALL(hwndDlg, ID_FOLINFO_TMEMS_LB);
        // get TM name from header
        INSERTITEM( hwndDlg, ID_FOLINFO_TMEMS_LB, pIda->pHeader->Head.Rel0.szMemory );
        break;
      case RELEASE1_HEADER:
        // get TMs from list of packaged files
        if ( pFileList )
        {

          DELETEALL(hwndDlg, ID_FOLINFO_TMEMS_LB);
          USHORT usNoOfEntries = (USHORT)pFileList->ulListUsed;
          PFILELISTENTRY pFile = pFileList->pEntries;
          while ( usNoOfEntries )
          {
            if ( pFile->usFileType == MEMORY_PROP_FILE )
            {
              PPROP_NTM pProp = NULL;

              pIda->pszBuffer[0] = EOS;
              UtlPackReadFilePart( pIda->pPackCtrl, pFile->pszName, (PBYTE)&(pProp->szLongName) - (PBYTE)pProp, (PBYTE)pIda->pszBuffer, MAX_LONGFILESPEC );
              OEMTOANSI(pIda->pszBuffer);
              if ( pIda->pszBuffer[0] == EOS )
              {
                Utlstrccpy( pIda->pszBuffer, UtlGetFnameFromPath( pFile->pszName ), DOT );
              } /* endif */
              INSERTITEM( hwndDlg, ID_FOLINFO_TMEMS_LB, pIda->pszBuffer );
            } /* endif */
            usNoOfEntries--;            // skip to next entry in file list
            pFile++;
          } /* endwhile */
        } /* endif */
        break;
      default:
        break;
    } /* endswitch */
  }
  else
  {
    // loop over all OpenTM2 memory info files and extract the memory names
    BOOL fFirstMemory = TRUE;
    USHORT usNoOfEntries = (USHORT)pFileList->ulListUsed;
    PFILELISTENTRY pFile = pFileList->pEntries;

    DELETEALL(hwndDlg, ID_FOLINFO_TMEMS_LB);

    usNoOfEntries = (USHORT)pFileList->ulListUsed;
    pFile         = pFileList->pEntries;
    while ( usNoOfEntries )
    {
      if ( pFile->usFileType == MEMORY_INFO_FILE )
      {
        // read memory info file into our buffer
        ULONG ulReadLen = ( pFile->ulFileSize > IMPBUFFERSIZE ) ? IMPBUFFERSIZE : pFile->ulFileSize;
        UtlPackReadFilePart( pIda->pPackCtrl, pFile->pszName, 0, (PBYTE)pIda->pszBuffer, ulReadLen );
        pIda->pszBuffer[ulReadLen-1] = 0;
        PSZ pszName = strstr( pIda->pszBuffer, MEMINFOFILE_NAME_KEY );
        if ( pszName != NULL )
        {
           while ( *pszName && (*pszName != '=') ) pszName++;
           if ( *pszName ) pszName++;
           PSZ pszEnd = pszName;
           while ( *pszEnd && (*pszEnd != LF) && (*pszEnd != CR) ) pszEnd++;
           *pszEnd = EOS;
           if ( pszEnd > pszName )
           {
             if ( fFirstMemory )
             { 
               SETTEXT( hwndDlg, ID_FOLINFO_MEM_TEXT, pszName );
               fFirstMemory = FALSE;
             } /* endif */          
             INSERTITEM( hwndDlg, ID_FOLINFO_TMEMS_LB, pszName );
           } /* endwhile */
        } /* end */           
      } /* endif */
      usNoOfEntries--;            // skip to next entry in file list
      pFile++;
    } /* endwhile */
  } /* end */     
  
  
}


MRESULT FolImpInfoInit
(
HWND    hwndDlg,                    // handle of folder export dialog window
WPARAM  mp1,                        // first parameter of WM_INITDLG
LPARAM  mp2                         // second parameter of WM_INITDLG
)
{
  MRESULT     mResult = FALSE;        // result of message processing
  PFOLIMPIDA  pIda;                   // ptr to folder import IDA
  PSZ         pszItem;                // temporary item text pointer
  PSZ         pszTemp;                // temporary work pointer
  PFILELIST   pFileList;              // ptr to package's file list
  USHORT      usNoOfEntries;          // # of enteries in package's file list
  PFILELISTENTRY pFile;               // ptr for file list processing
  LONG        lDateTime = 0;          // buffer for date/time value

  mp1 = mp1;                         // suppress 'unreferenced parameter' msg

  //--- store pointer to IDA  ---

  pIda = (PFOLIMPIDA)PVOIDFROMMP2(mp2);
  if (!pIda)
  {
    pIda = ACCESSDLGIDA( hwndDlg, PFOLIMPIDA );
  }

  ANCHORDLGIDA( hwndDlg, pIda );
  SETWINDOWID( hwndDlg, ID_FOLINFO_DLG );

  //--- set color of our statics containing data --
  SETCOLOR( hwndDlg, ID_FOLINFO_NAME_TEXT,   SYSCLR_WINDOWTEXT );
  SETCOLOR( hwndDlg, ID_FOLINFO_DESCR_TEXT,  SYSCLR_WINDOWTEXT );
  SETCOLOR( hwndDlg, ID_FOLINFO_DATE_TEXT,   SYSCLR_WINDOWTEXT );
  SETCOLOR( hwndDlg, ID_FOLINFO_MEM_TEXT,    SYSCLR_WINDOWTEXT );
  SETCOLOR( hwndDlg, ID_FOLINFO_MARKUP_TEXT, SYSCLR_WINDOWTEXT );
  SETCOLOR( hwndDlg, ID_FOLINFO_OPTION_TEXT, SYSCLR_WINDOWTEXT );

  SetCtrlFnt (hwndDlg, GetCharSet(), ID_FOLINFO_DESCR_TEXT, ID_FOLINFO_NOTE_MLE);

  if ( pIda )
  {
    //--- fill fields from package's user header ---
    UtlPackGetFileListPtr( pIda->pPackCtrl, &pFileList );

    //--- set folder name static ---
    SETTEXT( hwndDlg, ID_FOLINFO_NAME_TEXT, pIda->szLongFolName );

    //--- set folder description static ---
    {
      PSZ  pszDescr;                 // ptr to folder description

      switch ( pIda->pHeader->BitFlags.fHeaderType )
      {
        case RELEASE0_HEADER :
          pszDescr = pIda->pHeader->Head.Rel0.szDescription;
          break;
        case RELEASE1_HEADER:
          pszDescr = pIda->pHeader->Head.Rel1.szDescription;
          break;
        default:
          pszDescr = NULL;
          break;
      } /* endswitch */
      if ( pszDescr != NULL )
      {
        OEMTOANSI( pszDescr );
        SETTEXT( hwndDlg, ID_FOLINFO_DESCR_TEXT, pszDescr );
        ANSITOOEM( pszDescr );
      } /* endif */
    }
    //--- set folder export date static ---
    switch ( pIda->pHeader->BitFlags.fHeaderType )
    {
      case RELEASE0_HEADER:
        lDateTime = pIda->pHeader->Head.Rel0.lDateTime;
        break;
      case RELEASE1_HEADER:
        lDateTime = pIda->pHeader->Head.Rel1.lDateTime;
        break;
      default:
        break;
    } /* endswitch */
    LONGTODATETIMELEN( lDateTime, pIda->pszBuffer, IMPBUFFERSIZE );
    SETTEXT( hwndDlg, ID_FOLINFO_DATE_TEXT, pIda->pszBuffer );


    //--- set folder exported markup table static ---
    switch ( pIda->pHeader->BitFlags.fHeaderType )
    {
      case RELEASE0_HEADER:
        SETTEXT( hwndDlg, ID_FOLINFO_MARKUP_TEXT,
                 pIda->pHeader->Head.Rel0.szFormat );
        break;
      case RELEASE1_HEADER:
        SETTEXT( hwndDlg, ID_FOLINFO_MARKUP_TEXT,
                 pIda->pHeader->Head.Rel1.szFormat );
        break;
      default:
        break;
    } /* endswitch */

    //--- set folder export options static ---
    switch ( pIda->pHeader->BitFlags.fHeaderType )
    {
      case RELEASE0_HEADER:
      case RELEASE1_HEADER:
        if ( pIda->pHeader->BitFlags.fSelectedDocs )
        {
          HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
          SETTEXTFROMRES( hwndDlg, ID_FOLINFO_OPTION_TEXT, pIda->pszBuffer,
                          hResMod, SID_FOLINFO_DOCSELECTED );
        } /* endif */
        break;
      default:
        break;
    } /* endswitch */

    //--- fill documents listbox
    switch ( pIda->pHeader->BitFlags.fHeaderType )
    {
      case RELEASE0_HEADER:
        // get documents from file table using type PROP_CLASS_DOCUMENTS
        // and avoiding duplicate names
        if ( pFileList )
        {

          DELETEALL(hwndDlg, ID_FOLINFO_DOCS_LB);
          usNoOfEntries = (USHORT)pFileList->ulListUsed;
          pFile         = pFileList->pEntries;
          while ( usNoOfEntries )
          {
            if (pFile->usFileType == PROP_CLASS_DOCUMENT)
            {
              strcpy( pIda->pszBuffer,
                      UtlGetFnameFromPath( pFile->pszName ) );
              if ( SEARCHITEM( hwndDlg, ID_FOLINFO_DOCS_LB,
                               pIda->pszBuffer ) == LIT_NONE )
              {
                INSERTITEM( hwndDlg, ID_FOLINFO_DOCS_LB, pIda->pszBuffer );
              } /* endif */
            } /* endif */
            usNoOfEntries--;            // skip to next entry in file list
            pFile++;
          } /* endwhile */
          {
            HWND hwndLB = GetDlgItem(hwndDlg, ID_FOLINFO_DOCS_LB);
            UtlSetHorzScrollingForLB(hwndLB);
          }
        } /* endif */
        break;
      case RELEASE1_HEADER:
        // get documents from file table using type DOCUMENT_SRC_FILE
        if ( pIda->pHeader->BitFlags.fDocNameTable )
        {
          PSZ pszDocName; // ptr to current document

          pszDocName = (PSZ)pIda->pHeader;

          // position to long name table
          if ( pIda->pHeader->Head.Rel1.usNoteSize != 0 )
          {
            // position to end of normal header (FOLEXPHEADER contains
            // a one byte buffer for the note already, so we have to
            // decrease the note size by 1)
            pszDocName += sizeof(FOLEXPHEADER) +
                          pIda->pHeader->Head.Rel1.usNoteSize - 1;
          }
          else
          {
            pszDocName += sizeof(FOLEXPHEADER);
          } /* endif */

          DELETEALL(hwndDlg, ID_FOLINFO_DOCS_LB);
          while ( *pszDocName != EOS )
          {
            OEMTOANSI( pszDocName );
            INSERTITEM( hwndDlg, ID_FOLINFO_DOCS_LB, pszDocName );
            ANSITOOEM( pszDocName );
            pszDocName = pszDocName + strlen(pszDocName) + 1;
          } /* endwhile */

          {

            HWND hwndLB = GetDlgItem(hwndDlg, ID_FOLINFO_DOCS_LB);
            UtlSetHorzScrollingForLB(hwndLB);
          }
        }
        else if ( pFileList )
        {
          DELETEALL(hwndDlg, ID_FOLINFO_DOCS_LB);
          usNoOfEntries = (USHORT)pFileList->ulListUsed;
          pFile         = pFileList->pEntries;
          while ( usNoOfEntries )
          {
            if (pFile->usFileType == DOCUMENT_SRC_FILE)
            {
              strcpy( pIda->pszBuffer,
                      UtlGetFnameFromPath( pFile->pszName ) );
              INSERTITEM( hwndDlg, ID_FOLINFO_DOCS_LB, pIda->pszBuffer );
            } /* endif */
            usNoOfEntries--;            // skip to next entry in file list
            pFile++;
          } /* endwhile */
          {
            HWND hwndLB = GetDlgItem(hwndDlg, ID_FOLINFO_DOCS_LB);
            UtlSetHorzScrollingForLB(hwndLB);
          }
        } /* endif */
        break;
      default:
        break;
    } /* endswitch */

    // fill memory listbox and folder memory static
    FolImpInfoFillMemInfo( pIda, hwndDlg );

    //--- fill folder dictionary LB
    switch ( pIda->pHeader->BitFlags.fHeaderType )
    {
      case RELEASE0_HEADER:
        // get dictionary names from dictionary table in header
        if ( pIda->pHeader->Head.Rel0.DicTbl[0] )
        {

          DELETEALL(hwndDlg, ID_FOLINFO_DICTS_LB);
          strcpy( pIda->pszBuffer, pIda->pHeader->Head.Rel0.DicTbl );
          pszItem = pIda->pszBuffer;
          while ( *pszItem )
          {
            pszTemp = strchr( pszItem, X15 );
            if ( pszTemp )
            {
              *pszTemp = EOS;
            } /* endif */
            INSERTITEMEND( hwndDlg, ID_FOLINFO_DICTS_LB, pszItem );
            pszItem = ( pszTemp ) ? pszTemp + 1 : pszItem + strlen( pszItem );
          } /* endwhile */
        } /* endif */
        break;
      case RELEASE1_HEADER:
        // get dictionary names from list of packaged files
        if ( pFileList )
        {

          DELETEALL(hwndDlg, ID_FOLINFO_DICTS_LB);
          usNoOfEntries = (USHORT)pFileList->ulListUsed;
          pFile         = pFileList->pEntries;
          while ( usNoOfEntries )
          {
            if ( pFile->usFileType == DICTIONARY_PROP_FILE )
            {
              PPROPDICTIONARY pProp = NULL;

              pIda->pszBuffer[0] = EOS;
              UtlPackReadFilePart( pIda->pPackCtrl, pFile->pszName,
                                   (PBYTE)&(pProp->szLongName) - (PBYTE)pProp,
                                   (PBYTE)pIda->pszBuffer,
                                   MAX_LONGFILESPEC );
              OEMTOANSI(pIda->pszBuffer);
              if ( pIda->pszBuffer[0] == EOS )
              {
                Utlstrccpy( pIda->pszBuffer,
                            UtlGetFnameFromPath( pFile->pszName ), DOT );
              } /* endif */
              INSERTITEMEND( hwndDlg, ID_FOLINFO_DICTS_LB, pIda->pszBuffer );
            } /* endif */

            usNoOfEntries--;            // skip to next entry in file list
            pFile++;
          } /* endwhile */
        } /* endif */
        break;
      default:
        break;
    } /* endswitch */

    //--- fill attached note MLE ---
    switch ( pIda->pHeader->BitFlags.fHeaderType )
    {
      case RELEASE0_HEADER:
        SETTEXT( hwndDlg, ID_FOLINFO_NOTE_MLE, "" );
        if ( pIda->pHeader->Head.Rel0.szNoteBuffer[0] )
        {
          PSZ pszNote = pIda->pHeader->Head.Rel0.szNoteBuffer;

          OEMTOANSI( pszNote );
          SETTEXT( hwndDlg, ID_FOLINFO_NOTE_MLE, pszNote );
          ANSITOOEM( pszNote );
        } /* endif */
        break;
      case RELEASE1_HEADER:
        SETTEXT( hwndDlg, ID_FOLINFO_NOTE_MLE, "" );
        if ( pIda->pHeader->Head.Rel1.szNoteBuffer[0] )
        {
          PSZ pszNote = pIda->pHeader->Head.Rel1.szNoteBuffer;

          OEMTOANSI( pszNote );
          SETTEXT( hwndDlg, ID_FOLINFO_NOTE_MLE, pszNote );
          ANSITOOEM( pszNote );
        } /* endif */
        break;
      default:
        break;
    } /* endswitch */
  } /* endif */

  return( mResult );
} /* end of FolImpInfoInit */









MRESULT FolImpInfoCommand
(
HWND hwndDlg,
WPARAM mp1,
LPARAM mp2
)
{
  MRESULT mResult = MRFROMSHORT(TRUE);// TRUE = command is processed
  PFOLIMPIDA pIda;                    // ptr to folder import IDA

  mp2 = mp2;                          // supress 'unreferenced parameter' msg

  // --- get IDA pointer ---
  pIda = ACCESSDLGIDA( hwndDlg, PFOLIMPIDA );

  switch ( WMCOMMANDID( mp1, mp2 ) )
  {
    case ID_FOLINFO_PRINT_PB:
      FolImpInfoPrintDetails( pIda, hwndDlg );
      break;

    case ID_FOLINFO_OK_PB:
    case DID_CANCEL:
      POSTCLOSE( hwndDlg, 0 );
      break;

    default:
      break;
  } /* endswitch */

  return( mResult );
} /* end of FolImpInfoCommand */

MRESULT FolImpInfoClose
(
HWND hwndDlg,
WPARAM mp1,
LPARAM mp2
)
{
  MRESULT mResult = FALSE;
  PFOLIMPIDA pIda;                    // ptr to folder import IDA
// EQFINFO     ErrorInfo;              // error code of property handler calls

  mp1 = mp1;                          // supress 'unreferenced parameter' msg
  mp2 = mp2;                          //               "

  // get IDA pointer
  pIda = ACCESSDLGIDA( hwndDlg, PFOLIMPIDA );

  DelCtrlFont (hwndDlg, ID_FOLINFO_DESCR_TEXT);
  //--- get rid off folder info dialog ---
  DISMISSDLG( hwndDlg, TRUE );

  return( mResult );
} /* end of FolImpInfoClose */


VOID FolderImport( HWND hwnd, PSZ pSelFolderName )
{
  PFOLIMPIDA      pIda;               // folder import dialog IDA
  BOOL            fOK = TRUE;         // return value

  hwnd;
  pSelFolderName = pSelFolderName;    // supress compiler warning

  // create FolImpIda
  fOK = UtlAlloc( (PVOID *)&pIda, 0L, (LONG) sizeof(FOLIMPIDA), ERROR_STORAGE );
  if ( fOK )
  {
    fOK = UtlAlloc( (PVOID *)&pIda->pszBuffer, 0L, IMPBUFFERSIZE, ERROR_STORAGE );
    if ( fOK )
    {
      pIda->ulBufSize = IMPBUFFERSIZE;
    } /* endif */
  } /* endif */


  if ( fOK )
  {
    HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
    pIda->usDialogType = FOLDER_IMPORT;

    DIALOGBOX( EqfQueryTwbClient(), FOLIMPPROPDLGPROC, hResMod, ID_FOLIMPPROP_DLG,
               pIda, fOK );

    if ( fOK )
    {
      UtlPackSetTargetDrive( pIda->pPackCtrl, pIda->chToDrive );
      strcpy( pIda->szObjPath, FOLIMPOBJPREFIX );
      strcat( pIda->szObjPath, pIda->szObjName );
      fOK = CreateProcessWindow( pIda->szObjPath, FolImportCallBack, pIda );
    } /* endif */

    if ( !fOK )
    {
      UtlAlloc( (PVOID *)&(pIda), 0L, 0L, NOMSG) ;
    } /* endif */
  } /* endif */
}

MRESULT FolImportCallBack
(
PPROCESSCOMMAREA pCommArea,          // ptr to commmunication area
HWND             hwnd,               // handle of process window
WINMSG           message,            // message to be processed
WPARAM           mp1,                // first message parameter
LPARAM           mp2                 // second message parameter
)
{
  PFOLIMPIDA      pIda;                // pointer to instance area
  MRESULT         mResult = FALSE;     // return code for handler proc

  switch ( message)
  {
    /******************************************************************/
    /* WM_CREATE:                                                     */
    /*                                                                */
    /* Fill fields in communication area                              */
    /* Initialize data of callback function                           */
    /******************************************************************/
    case WM_CREATE :
      {
        HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);

        /**************************************************************/
        /* Set global export instance window handle and anchor IDA    */
        /**************************************************************/
        hwndImportInstance  = hwnd;
        pIda                = (PFOLIMPIDA)PVOIDFROMMP2(mp2);
        pCommArea->pUserIDA = pIda;

        /****************************************************************/
        /* supply all information required to create the process        */
        /* window                                                       */
        /****************************************************************/
        pCommArea->sProcessWindowID = ID_FOLIMP_WINDOW;
        pCommArea->sProcessObjClass = clsFOLDERIMP;
        if ( pIda->pDDEFolImp == NULL )
        {
          if ( pIda->usImpMode == FOL_TMGR_FORMAT )
          {
            pCommArea->Style            = PROCWIN_TEXTONLY;
          }
          else
          {
            pCommArea->Style            = PROCWIN_SLIDERONLY;
            if ( pIda->pXLIFFData )
            {
              pIda->pXLIFFData->hwndErrMsg = hwnd;
            } /* endif */
          } /* endif */
        }
        else
        {
          pCommArea->Style            = PROCWIN_BATCH;
        } /* endif */
        pCommArea->sSliderID        = ID_FOLIMP_TEXT;
        LOADSTRING( NULLHANDLE, hResMod, SID_FOLIMP_TITLE, pCommArea->szTitle );
        strcat ( pCommArea->szTitle, pIda->szTargetFolName );
        OEMTOANSI(pCommArea->szTitle);
        pCommArea->hIcon            = (HPOINTER) UtlQueryULong(QL_FOLIMPICON); //hiconFOLIMP;
        pCommArea->fNoClose         = FALSE;
        pCommArea->swpSizePos.x     = 100;
        pCommArea->swpSizePos.y     = 100;
        pCommArea->swpSizePos.cx    = (SHORT) UtlQueryULong( QL_AVECHARWIDTH ) * 60;
        pCommArea->swpSizePos.cy    = (SHORT) UtlQueryULong( QL_PELSPERLINE ) * 10;
        pCommArea->asMsgsWanted[0]  = WM_EQF_PROCESSTASK;
        pCommArea->asMsgsWanted[1]  = WM_EQF_ABOUTTODELETE;
        pCommArea->asMsgsWanted[2]  = WM_EQFN_TASKDONE;
        pCommArea->asMsgsWanted[3]  = 0;
        pCommArea->usComplete       = 0;
      }
      break;


      /****************************************************************/
      /* Start processing by posting WM_EQF_PROCESSTASK               */
      /****************************************************************/
    case WM_EQF_INITIALIZE:
      pIda = (PFOLIMPIDA)pCommArea->pUserIDA;

      if ( pIda->usImpMode == FOL_TMGR_FORMAT )
      {
        pIda->NextTask = LOADFILES;

        // build list of additional tag table files contained in folder package
        FolImpBuildAddTagTableFilesList( pIda );


        /**************************************************************/
        /* set owner handle for messages                              */
        /**************************************************************/
        if ( pIda->usImpMode == FOL_TMGR_FORMAT )
        {
          if ( pIda->pDDEFolImp == NULL )
          {
            UtlPackSetOwnerHwnd( pIda->pPackCtrl, hwnd );
            pIda->hwndErrMsg = hwnd;
          }
          else
          {
            UtlPackSetOwnerHwnd( pIda->pPackCtrl, pIda->pDDEFolImp->hwndErrMsg );
            pIda->hwndErrMsg = pIda->pDDEFolImp->hwndErrMsg;
          } /* endif */
        } /* endif */

        WinPostMsg( hwnd, WM_EQFN_TASKDONE, MP1FROMSHORT(TRUE), 0L );
      }
      else
      {
        WinPostMsg( hwnd, WM_EQF_PROCESSTASK, MP1FROMSHORT(FOL_IMPORT_TASK), 0l );
      } /* endif */
      break;

      /******************************************************************/
      /* WM_CLOSE:                                                      */
      /*                                                                */
      /* Prepare/initialize shutdown of process                         */
      /******************************************************************/
    case WM_CLOSE:
      pIda = (PFOLIMPIDA)pCommArea->pUserIDA;
      if ( pIda )
      {
        pIda->fKill = TRUE;
        if ( pIda->objCurProcess[0] != EOS )
        {
          HWND hwndObj = EqfQueryObject( pIda->objCurProcess, clsANY, 0);
          if ( hwndObj != NULLHANDLE )
          {
            // pass close message to current import process
            WinPostMsg( hwndObj, WM_CLOSE, mp1, mp2 );
          } /* endif */
        } /* endif */
        mResult = MRFROMSHORT( TRUE );   // = do not close right now
      }
      else
      {
        mResult = MRFROMSHORT( FALSE );  // = continue with close
      } /* endif */
      break;

      /******************************************************************/
      /* WM_DESTROY:                                                    */
      /*                                                                */
      /* Cleanup all resources used by the process                      */
      /******************************************************************/
    case WM_DESTROY:
      pIda = (PFOLIMPIDA)pCommArea->pUserIDA;
      if ( pIda->usImpMode == FOL_TMGR_FORMAT )
      {
        if ( pIda->pPackCtrl )
        {
          UtlPackCleanup( pIda->pPackCtrl );
          pIda->pPackCtrl = NULL;
        } /* endif */
        //--- discard all files and directories under x:\EQF\IMPORT ---
        if ( pIda->szImportDir[0]!= EOS )
        {
          UtlMakeEQFPath( pIda->szObjPath, pIda->chToDrive, SYSTEM_PATH, NULL );
          strcat( pIda->szObjPath, pIda->szImportDir );
          if ( UtlDirExist( pIda->szObjPath ) )
          {
            UtlRemoveDir( pIda->szObjPath, FALSE );
          } /* endif */
        } /* endif */           

        if ( pIda->pvMemImportData != NULL )
        {
          PMEMIMPORTDATA pMemData = (PMEMIMPORTDATA)pIda->pvMemImportData; 
          free( pMemData );
          pIda->pvMemImportData = NULL;
        } /* endif */           
      } /* endif */

      /**************************************************************/
      /* Tell DDE handler that we are thru ...                      */
      /**************************************************************/
      if ( pIda->pDDEFolImp != NULL )
      {
        WinPostMsg( pIda->pDDEFolImp->hwndOwner, WM_EQF_DDE_ANSWER,
                    NULL, &pIda->pDDEFolImp->DDEReturn );

      } /* endif */

      REMOVESYMBOL( pIda->szTargetFolObj );

      if ( pIda != NULL )
      {
        if ( pIda->pDocData != NULL )
        {
          UtlAlloc( (PVOID *)&pIda->pDocData, 0L, 0L, NOMSG );
        } /* endif */
        if ( pIda->pszBuffer ) UtlAlloc( (PVOID *)&pIda->pszBuffer, 0L, 0L, NOMSG );
      } /* endif */

      if ( pIda->pszFolderList && (pIda->usImpMode == FOL_TMGR_FORMAT) )
      {
        // skip current entry in folder list
        if ( pIda->pszActiveFolder && *(pIda->pszActiveFolder) )
        {
          pIda->pszActiveFolder += strlen(pIda->pszActiveFolder) + 1;
        } /* endif */

        // start next import operation if another folder is to be imported
        if ( pIda->pszActiveFolder && *(pIda->pszActiveFolder) )
        {
          ULONG ulRC = NO_ERROR;
          BOOL fOK = TRUE;

          if ( pIda->pPackCtrl )
          {
            UtlPackClosePackage( pIda->pPackCtrl ); // ... close it
            pIda->pPackCtrl = NULL;
          } /* endif */

          if ( pIda->szImportDir[0] != EOS )
          {
            UtlMakeEQFPath( pIda->szObjPath, pIda->chToDrive, SYSTEM_PATH, NULL );
            strcat( pIda->szObjPath, pIda->szImportDir );
            if ( UtlDirExist( pIda->szObjPath ) )
            {
              UtlRemoveDir( pIda->szObjPath, FALSE );
            } /* endif */
            pIda->szImportDir[0]= EOS;
          } /* endif */             

          if ( pIda->pHeader ) UtlAlloc( (PVOID *)&pIda->pHeader, 0L, 0L, NOMSG );

          strcpy( pIda->szSourceFolName, pIda->pszActiveFolder );
          strcpy( pIda->szSourceFolder,  pIda->pszActiveFolder );
          strcpy( pIda->szLongFolName,   pIda->pszActiveFolder );
          UtlUpper( pIda->szSourceFolName );
          strcpy( pIda->szObjPath, pIda->szSourceFolName );
          strcat( pIda->szObjPath, FOLDEREXPORT_EXT );

          pIda->pPackCtrl = UtlPackOpenPackage( pIda->chFromDrive, NULL, pIda->szImportPath,
                                                pIda->szObjPath,
                                                hwnd );

          //--- read package's user header ---
          if ( pIda->pPackCtrl )            // if the package is open ...
          {
            ULONG ulHeaderSize = UtlPackQueryHeaderSize( pIda->pPackCtrl );
            if ( ulHeaderSize )
            {
              UtlAlloc( (PVOID *)&pIda->pHeader, 0L, (LONG) ulHeaderSize, ERROR_STORAGE );
              if ( pIda->pHeader )
              {
                ulRC = UtlPackReadHeader( pIda->pPackCtrl, (PBYTE) pIda->pHeader, ulHeaderSize );
                if ( ulRC == PACK_OK_RC )
                {
                }
                else
                {
                  // read of header failed
                  fOK = FALSE;
                  UtlAlloc( (PVOID *)&pIda->pHeader, 0L, 0L, NOMSG );
                  UtlPackClosePackage( pIda->pPackCtrl );
                  pIda->pPackCtrl = NULL;
                } /* endif */
              }
              else
              {
                // alloc failed
                fOK = FALSE;
                UtlPackClosePackage( pIda->pPackCtrl );
                pIda->pPackCtrl = NULL;
              } /* endif */
            } /* endif */
          }
          else
          {
            fOK = FALSE;         // error: package did not open
          } /* endif */

          // check if folder exists somethere and set TO drive
          if ( fOK )
          {
            BOOL fFolderExists = FolImpGetFolderDrive( pIda, pIda->pszActiveFolder,
                                                       &(pIda->chToDrive) );
            if ( !fFolderExists ) pIda->chToDrive = pIda->chOrgToDrive;
          } /* endif */

          // prepare folder import
          if ( fOK )
          {
            // set drive of target folder object name
            pIda->szTargetFolObj[0] = pIda->chToDrive;


            // check if the target folder exists but has a different unique ID
            // (i.e. it is another folder with the same name)
            // perform this check only if folder names are equal
            if ( fOK )
            {
              BOOL fIsNew = FALSE;
              BOOL fUniqueIDAvailable = TRUE;
              PPROPFOLDER pFolProp = NULL;
              ULONG ulLen = 0;
              char szSourceID[sizeof(pFolProp->UniqueID)];
              char szTargetID[sizeof(pFolProp->UniqueID)];


              // get unique ID of target folder
              ObjLongToShortName( pIda->szTargetFolName,
                                  pIda->szTargetShortFolName,
                                  FOLDER_OBJECT, &fIsNew );
              strcpy( pIda->szShortFolName, pIda->szTargetShortFolName );

              if ( !fIsNew )       // perform unique ID check for existing folders only
              {
                UtlMakeEQFPath( pIda->szCurFile,
                                pIda->szTargetFolObj[0],
                                PROPERTY_PATH, NULL );
                strcat( pIda->szCurFile, BACKSLASH_STR );
                strcat( pIda->szCurFile, pIda->szTargetShortFolName );
                strcat( pIda->szCurFile, EXT_FOLDER_MAIN );
                if ( UtlLoadFileL( pIda->szCurFile, (PVOID *)&pFolProp, &ulLen, FALSE, FALSE ) )
                {
                  strcpy( szTargetID, pFolProp->UniqueID );
                  UtlAlloc( (PVOID *)&pFolProp, 0L, 0L, NOMSG );
                }
                else
                {
                  fUniqueIDAvailable = FALSE;
                } /* endif */

                // get unique ID of folder being imported
                if ( pIda->pHeader && (pIda->pHeader->BitFlags.fHeaderType == 1) )
                {
                  strcpy( szSourceID, pIda->pHeader->Head.Rel1.UniqueID );
                }
                else if ( pIda->pPackCtrl )
                {
                  UtlAlloc( (PVOID *)&pFolProp, 0L, sizeof(PROPFOLDER), NOMSG );

                  if ( pFolProp )
                  {
                    UtlMakeEQFPath( pIda->szCurFile,
                                    pIda->szTargetFolObj[0],  // don't care for drive letter
                                    PROPERTY_PATH, NULL );
                    strcat( pIda->szCurFile, BACKSLASH_STR );
                    strcat( pIda->szCurFile, pIda->szShortFolName );
                    strcat( pIda->szCurFile, EXT_FOLDER_MAIN );
                    UtlPackReadFilePart( pIda->pPackCtrl, pIda->szCurFile, 0,
                                         (PBYTE)pFolProp, sizeof(PROPFOLDER) );
                    strcpy( szSourceID, pFolProp->UniqueID );
                    UtlAlloc( (PVOID *)&pFolProp, 0L, 0L, NOMSG );
                  }
                  else
                  {
                    fUniqueIDAvailable = FALSE;
                  } /* endif */
                }
                else
                {
                  fUniqueIDAvailable = FALSE;
                } /* endif */


                if ( !fUniqueIDAvailable || (strcmp( szSourceID, szTargetID ) != 0) )
                {
                  // folder's are not the same, get merge confirmation
                  PSZ   pszErrParm = pIda->szTargetFolName;
                  USHORT usMBReturn;
                  OEMTOANSI(pIda->szTargetFolName);
                  usMBReturn = UtlErrorHwnd( ERROR_IMP_FOLDER_EXISTS, MB_OKCANCEL, 1,
                                                    &pszErrParm, EQF_ERROR, pIda->hwndErrMsg );
                  ANSITOOEM(pIda->szTargetFolName);
                  if ( usMBReturn == MBID_CANCEL )
                  {
                    fOK = FALSE;
                  } /* endif */
                } /* endif */
              } /* endif */
            } /* endif */

            //--- check if folder has been locked by another process
            if ( fOK )
            {
              SHORT sRC = QUERYSYMBOL( pIda->szTargetFolObj );
              if ( sRC != -1 )
              {
                PSZ pErrParms[3];
                fOK = FALSE;
                pErrParms[0] = pIda->szTargetFolName;
                UtlError( ERROR_FOLDER_LOCKED, MB_CANCEL, 1, pErrParms, EQF_INFO );
              } /* endif */
            } /* endif */

            // Check if folder is currently open
            if ( fOK )
            {
              if ( EqfQueryObject( pIda->szTargetFolObj, clsFOLDER, 0) )
              {
                PSZ        pszParm;

                pszParm = pIda->szTargetFolName;
                OEMTOANSI(pIda->szTargetFolName);
                UtlError( ERROR_IMPORT_OPEN_FOLDER, MB_CANCEL, 1, &pszParm, EQF_ERROR );
                ANSITOOEM(pIda->szTargetFolName);
                fOK = FALSE;
              } /* endif */
            } /* endif */

            //--- if o.k. set/reset skip flags of package files
            if ( fOK && pIda->pPackCtrl )
            {
              PFILELIST pFileList;           // ptr to package's file list
              ulRC = UtlPackGetFileListPtr( pIda->pPackCtrl, &pFileList );
              fOK = UtlPackHandleError( ulRC, pIda->szSourceFolName, hwnd );
              if ( fOK )
              {
                USHORT usNoOfEntries = (USHORT)pFileList->ulListUsed;
                PFILELISTENTRY pFile = pFileList->pEntries;
                while ( usNoOfEntries )
                {
                  pFile->usProcessFlags &= ~FILE_SKIP;

                  if ( pIda->pHeader->BitFlags.fHeaderType == RELEASE0_HEADER )
                  {
                    if ( ( pFile->usFileType == PROP_CLASS_TAGTABLE ) ||
                         ( pFile->usFileType == PROP_CLASS_MEMORY ) )
                    {
                      if ( !pIda->fWithMem )
                      {
                        pFile->usProcessFlags |= FILE_SKIP;
                      } /* endif */
                    }
                    else if ( pFile->usFileType == PROP_CLASS_DICTIONARY )
                    {
                      if ( !pIda->fWithDict )
                      {
                        pFile->usProcessFlags |= FILE_SKIP;
                      } /* endif */
                    } /* endif */
                  }
                  else
                  {
                    if ( ( pFile->usFileType == MEMORY_PROP_FILE ) ||
                         ( pFile->usFileType == MEMORY_DATA_FILE ) ||
                         ( pFile->usFileType == MEMORY_TABLE_FILE ) ||
                         ( pFile->usFileType == NTMMEMORY_INDEX_FILE ) ||
                         ( pFile->usFileType == NTMMEMORY_DATA_FILE ) )
                    {
                      if ( !pIda->fWithMem )
                      {
                        pFile->usProcessFlags |= FILE_SKIP;
                      } /* endif */
                    }
                    else if ( ( pFile->usFileType == DICTIONARY_PROP_FILE ) ||
                              ( pFile->usFileType == DICTIONARY_DATA_FILE ) ||
                              ( pFile->usFileType == DICTIONARY_INDEX_FILE ) )
                    {
                      if ( !pIda->fWithDict )
                      {
                        pFile->usProcessFlags |= FILE_SKIP;
                      } /* endif */
                    } /* endif */
                  } /* endif */

                  usNoOfEntries--;            // skip to next entry in file list
                  pFile++;
                } /* endwhile */
              } /* endif */
            } /* endif */

            //--- if o.k. check free space on target drive ---
            if ( fOK && pIda->pPackCtrl )
            {
              ULONG ulRequiredSpace;
              ulRC = UtlPackQueryRequiredSpace( pIda->pPackCtrl,
                                                &ulRequiredSpace );
              fOK = UtlPackHandleError( ulRC, pIda->szSourceFolName, hwnd );
              if ( fOK )
              {
                ULONG64 lFreeSpace = UtlQueryFreeSpace( pIda->chToDrive, TRUE );
                if ( lFreeSpace < (LONG)ulRequiredSpace )
                {
                  PSZ pErrParms[2];
                  CHAR szDrive[4];
                  pErrParms[0] = pIda->szTargetFolName;
                  szDrive[0] = pIda->chToDrive;
                  szDrive[1] = EOS;
                  pErrParms[1] = szDrive;
                  UtlError( ERROR_FOLIMP_NOSPACE, MB_CANCEL, 2, pErrParms, EQF_ERROR );
                  fOK = FALSE;
                } /* endif */
              } /* endif */
            } /* endif */
          } /* endif */

          // start import by creating process window
          if ( fOK )
          {
            pIda->fOk = TRUE;
            pIda->fExists = FALSE;
            pIda->fUpdated = FALSE;
            pIda->fKill = FALSE;
            pIda->ulBufSize = 0;
            UtlPackSetTargetDrive( pIda->pPackCtrl, pIda->chToDrive );
            strcpy( pIda->szObjPath, FOLIMPOBJPREFIX );
            strcat( pIda->szObjPath, pIda->szObjName );
            fOK = CreateProcessWindow( pIda->szObjPath, FolImportCallBack, pIda );
          } /* endif */
        }
        else
        {
          CHAR        szErrValue[MAX_DRIVE];// error parameter
          PSZ         pszErrParm[3];        // error parameter table

          // show completion message
          szErrValue[0] = pIda->chToDrive;
          szErrValue[1] = EOS;
          pszErrParm[0] = pIda->pszNameList;
          pszErrParm[1] = szErrValue;
          UtlError( MESSAGE_MULFOLIMP_COMPLETED, MB_OK, 2, pszErrParm, EQF_INFO );

          // cleanup
          UtlAlloc( (PVOID *)&(pIda->pszFolderList), 0L, 0L, NOMSG );
          UtlAlloc( (PVOID *)&(pIda->pszNameList), 0L, 0L, NOMSG );
          UtlAlloc( (PVOID *)&pIda, 0L, 0L, NOMSG );
          hwndImportInstance = NULLHANDLE;
        } /* endif */

      }
      else
      {
        UtlAlloc( (PVOID *)&pIda, 0L, 0L, NOMSG );
        hwndImportInstance = NULLHANDLE;
      } /* endif */

      break;


      /******************************************************************/
      /* WM_EQF_TERMINATE:                                              */
      /*                                                                */
      /* Allow or disable termination of process                        */
      /******************************************************************/
    case WM_EQF_TERMINATE:
      mResult = MRFROMSHORT( FALSE );          // = continue with close
      break;

      /******************************************************************/
      /* WM_INITMENU:                                                   */
      /*                                                                */
      /* Enable/Disable actionbar items                                 */
      /******************************************************************/
    case WM_INITMENU:
      UtlMenuDisableAll( hwnd, SHORT1FROMMP1( mp1) );
      break;

      /******************************************************************/
      /* other messages:                                                */
      /*                                                                */
      /* requested from generic process window procedure using          */
      /* asMsgsWanted array in communication area                       */
      /******************************************************************/
    case WM_EQF_ABOUTTODELETE:
      if ( SHORT1FROMMP1( mp1 ) == clsFOLDER )
      {
        pIda = (PFOLIMPIDA)pCommArea->pUserIDA;
        if ( stricmp( pIda->szTargetFolObj, (PSZ)mp2 ) == 0 )
        {
          mResult = MRFROMSHORT( TRUE ); // don't allow delete of folder
        } /* endif */
      } /* endif */
      return( mResult );
      break;

    case WM_EQF_PROCESSTASK:
      switch ( SHORT1FROMMP1(mp1) )
      {
        case FOL_IMPORT_TASK:
          pIda = (PFOLIMPIDA)pCommArea->pUserIDA;
          pIda->objCurProcess[0] = EOS;
          if ( pIda->fKill )
          {
            USHORT   usMBCode;
            PSZ      pszErrParm = pIda->szLongFolName;
            if ( pIda->pDDEFolImp != NULL )
            {
              // we are in batch mode, continue cancel of export
              usMBCode = MBID_YES;
            }
            else
            {
              usMBCode = UtlError( ERROR_FOLIMP_CANCEL, MB_YESNO,
                                   1, &pszErrParm, EQF_QUERY );
            } /* endif */

            if ( usMBCode == MBID_YES )
            {
              EqfRemoveObject( TWBFORCE, hwnd);
            }
            else
            {
              pIda->fKill = FALSE;
              WinPostMsg( hwnd,
                          WM_EQF_PROCESSTASK,
                          MP1FROMSHORT(FOL_IMPORT_TASK),
                          0l );
            } /* endif */
          }
          else if ( pIda->usImpMode == FOL_XLIFF_FORMAT )
          {
            int iRC = -1;

            if ( hXLIFF2 == NULL ) 
               hXLIFF2 = LoadLibrary("EQFXLIFF.DLL");
            if ( hXLIFF2 != NULL ) {
               if ( pfnFOLXLIFFIMPORT == NULL ) 
                  pfnFOLXLIFFIMPORT = (FOLXLIFFIMPORT_DCL)GetProcAddress(hXLIFF2, "FOLXLIFFIMPORT");
               if ( pfnFOLXLIFFIMPORT != NULL )
                  iRC = (*pfnFOLXLIFFIMPORT)( pIda->pXLIFFData );
            }

            if ( iRC == FOL_EXPORT_IMPORT_COMPLETE )
            {
              CHAR        szErrValue[MAX_DRIVE];// error parameter
              PSZ         pszErrParm[3];        // error parameter table

              pCommArea->usComplete = 100;
              szErrValue[0] = pIda->chToDrive;
              szErrValue[1] = EOS;
              pszErrParm[0] = pIda->szLongFolName;
              pszErrParm[1] = szErrValue;
              OEMTOANSI(pIda->szLongFolName);
              UtlError( MESSAGE_FOLIMP_COMPLETED, MB_OK, 2, pszErrParm, EQF_INFO );
              ANSITOOEM(pIda->szLongFolName);
              EqfRemoveObject( TWBFORCE, hwnd);
              if ( hXLIFF2 ) {
                 FreeLibrary( hXLIFF2 );
                 hXLIFF2 = NULL ;
                 pfnFOLXLIFFIMPORT = NULL ;
              }
            }
            else if ( iRC == 0 )
            {
              UtlDispatch();
              pCommArea->usComplete = pIda->pXLIFFData->usComplete;
              WinPostMsg( hwnd, WM_EQF_PROCESSTASK, MP1FROMSHORT(FOL_IMPORT_TASK), 0l );
            }
            else
            {
              CHAR        szErrValue[MAX_DRIVE];// error parameter
              PSZ         pszErrParm[3];        // error parameter table
              USHORT      usMsg;

              szErrValue[0] = pIda->chToDrive;
              szErrValue[1] = EOS;
              pszErrParm[0] = pIda->szLongFolName;
              pszErrParm[1] = szErrValue;
              OEMTOANSI(pIda->szLongFolName);
              usMsg = ( pIda->fUserExitCopyPending ) ? MESSAGE_FOLIMP_COMPL_USEREXITINUSE : MESSAGE_FOLIMP_TERMINATED;
              UtlError( usMsg, MB_CANCEL, 2, pszErrParm, EQF_ERROR );
              ANSITOOEM(pIda->szLongFolName);
              EqfRemoveObject( TWBFORCE, hwnd);
              if ( hXLIFF2 ) {
                 FreeLibrary( hXLIFF2 );
                 hXLIFF2 = NULL ;
                 pfnFOLXLIFFIMPORT = NULL ;
              }
            } /* endif */
          }
          else if ( pIda->pPackCtrl )
          {
            ULONG ulRC = 0;

            // create unique import directory if not done yet
            if ( pIda->szImportDir[0] == EOS )
            {
              FolImpBuildUniqueImportPath( pIda );
              UtlPackSetImportDir( pIda->pPackCtrl, pIda->szImportDir + 1 );
            } /* endif */               

            ulRC = UtlPackReadPackage( pIda->pPackCtrl );
            switch ( ulRC )
            {
              case PACK_OK_RC:
                //--- continue in reading package ---
                UtlDispatch();
                WinPostMsg( hwnd,
                            WM_EQF_PROCESSTASK,
                            MP1FROMSHORT(FOL_IMPORT_TASK),
                            0l );
                break;
              case PACK_COMPLETE_RC:
                WinPostMsg( hwnd, WM_EQFN_TASKDONE, MP1FROMSHORT(TRUE), 0L );
                break;
              default:
                pIda->NextTask = IMPORTCANCEL;
                WinPostMsg( hwnd, WM_EQFN_TASKDONE, MP1FROMSHORT(TRUE), 0L );
                break;
            } /* endswitch */
          } /* endif */
          break;
      } /* endswitch */
      break;

    case WM_EQFN_TASKDONE:
      pIda = (PFOLIMPIDA)pCommArea->pUserIDA;
      pIda->objCurProcess[0] = EOS;
      if ( SHORT1FROMMP1(mp1) )
      {
        HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);

        switch ( pIda->NextTask )
        {
          case LOADFILES:
            LOADSTRING( NULLHANDLE, hResMod, SID_FOLIMP_FILES, pIda->szAction );
            if ( pIda->pDDEFolImp == NULL )
            {
              WinSendMsg( hwnd, WM_EQF_UPDATESLIDER, MP1FROMSHORT(-1),
                          MP2FROMP(pIda->szAction) );
            } /* endif */

            UtlDispatch();

            pCommArea = (PPROCESSCOMMAREA)AccessGenProcCommArea( hwnd );
            if ( pCommArea == NULL )
            {
              pIda = NULL;
            }
            else
            {
              pIda = (PFOLIMPIDA)pCommArea->pUserIDA;
            } /* endif */

            if ( pIda != NULL )
            {
              WinPostMsg( hwnd, WM_EQF_PROCESSTASK,
                          MP1FROMSHORT(FOL_IMPORT_TASK), 0L );
              pIda->NextTask = PROCESSTABLES;
            } /* endif */
            break;

          case PROCESSTABLES:
            {
              BOOL fOK = FolImpProcessTables( hwnd, pIda );
              pIda->NextTask = ( fOK ) ? PROCESSFOLDER : IMPORTCANCEL ;
              WinPostMsg( hwnd, WM_EQFN_TASKDONE, MP1FROMSHORT(TRUE), 0L );
            }
            break;

          case PROCESSFOLDER:
            FolImpProcessFolder( hwnd, pIda );

            if ( pIda->usProcessedObjects <= pIda->usMaxObjects ) pIda->usProcessedObjects++;

            WinPostMsg( hwnd, WM_EQFN_TASKDONE, MP1FROMSHORT(TRUE), 0L );
            break;

          case PROCESSSUBFOLDERS:
            FolImpProcessSubFolders( hwnd, pIda );
            WinPostMsg( hwnd, WM_EQFN_TASKDONE, MP1FROMSHORT(TRUE), 0L );
            break;

          case PROCESSDOCS:
            {
              BOOL        fOK = TRUE; // internal OK flag
              if ( pIda->pDDEFolImp == NULL )
              {
                LOADSTRING( NULLHANDLE, hResMod, SID_FOLIMP_DOCS, pIda->szAction );
                WinSendMsg( hwnd, WM_EQF_UPDATESLIDER, MP1FROMSHORT(-1),
                            MP2FROMP(pIda->szAction) );
              } /* endif */

              UtlDispatch();
              pCommArea = (PPROCESSCOMMAREA)AccessGenProcCommArea( hwnd );
              if ( pCommArea == NULL )
              {
                pIda = NULL;
              }
              else
              {
                pIda = (PFOLIMPIDA)pCommArea->pUserIDA;
              } /* endif */

              if ( pIda != NULL )
              {
                BOOL fUserInput = FALSE; // user input for doc. settings

                //--- make path for document objects ---
                UtlMakeEQFPath( pIda->szObjPath, pIda->chToDrive,
                                SYSTEM_PATH,
                                pIda->szSourceFolder );
                UtlAddImportToPath( pIda, pIda->szObjPath );

                //--- make path for document properties ---
                UtlMakeEQFPath( pIda->szSearchPath, pIda->chToDrive,
                                PROPERTY_PATH,
                                pIda->szSourceFolder );
                UtlAddImportToPath( pIda, pIda->szSearchPath );
                strcat( pIda->szSearchPath, BACKSLASH_STR );
                strcat( pIda->szSearchPath, DEFAULT_PATTERN );

                //--- build list of imported documents ---
                fOK = FolImpBuildObjList( pIda->szSearchPath,
                                          pIda->szObjPath,
                                          &pIda->pszBuffer,
                                          &pIda->ulBufSize );

                // build document processing and info table
                if ( fOK )
                {
                  fOK = FolImpPreProcessDocs( pIda, &fUserInput );
                  if ( !fOK )
                  {
                    pIda->NextTask = IMPORTCANCEL;
                  }
                } /* endif */

                // get user confirmation for processing steps
                if ( fOK && fUserInput && (pIda->pDDEFolImp == NULL) )
                {
                  HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
                  DIALOGBOX( EqfQueryTwbClient(), FOLIMPDOCDLGPROC,
                             hResMod, ID_FOLIMPDOC_DLG,  pIda, fOK );
                  if ( !fOK )
                  {
                    pIda->fNoMsg = TRUE;
                    pIda->NextTask = IMPORTCANCEL;
                  }
                  else
                  {
                    pIda->NextTask = PROCESSDICTIONARY;
                  } /* endif */
                }
                else
                {
                  pIda->NextTask = PROCESSDICTIONARY;
                } /* endif */

                // process documents in document processing table
                if ( fOK )
                {
                  fOK = FolImpProcessDocs( pIda );
                  if ( !fOK )
                  {
                    pIda->NextTask = IMPORTCANCEL;
                  }
                } /* endif */

                WinPostMsg( hwnd, WM_EQFN_TASKDONE, MP1FROMSHORT(TRUE), 0L );
              } /* endif */
            }
            break;

          case PROCESSNEXTMEMORY:
            {
              BOOL        fOK = TRUE; // internal OK flag

              if ( pIda->fWithMem )
              {
                BOOL fContinue = FALSE;
                BOOL fMoreMemories = FALSE;
                fOK = FolImpProcessNextMemory( pIda, &fMoreMemories, &fContinue );
                if ( !fOK )
                {
                  pIda->NextTask = IMPORTCANCEL;
                }
                else if ( fContinue )
                {
                  pIda->NextTask = PROCESSMEMORY;
                }
                else if ( fMoreMemories )
                {
                  pIda->NextTask = PROCESSNEXTMEMORY;
                }
                else
                {
                  pIda->NextTask = IMPORTOK;
                } /* end */                   
              }
              else
              {
                pIda->NextTask = IMPORTOK;
              } /* endif */
              WinPostMsg( hwnd, WM_EQFN_TASKDONE, MP1FROMSHORT(TRUE), 0L );
            }
            break;


          case PROCESSMEMORY:
            {
              BOOL        fOK = TRUE;            // internal OK flag

              if ( pIda->fWithMem )
              {
                BOOL fContinue = FALSE;
                fOK = FolImpContinueMemoryImport( pIda, &fContinue );
                if ( !fOK )
                {
                  pIda->NextTask = IMPORTCANCEL;
                }
                else if ( fContinue )
                {
                  pIda->NextTask = PROCESSMEMORY;
                }
                else
                {
              //  pIda->NextTask = IMPORTOK;
                  pIda->NextTask = PROCESSNEXTMEMORY;  /* P402080 */
                } /* endif */
              }
              else
              {
                pIda->NextTask = IMPORTOK;
              } /* endif */
              WinPostMsg( hwnd, WM_EQFN_TASKDONE, MP1FROMSHORT(TRUE), 0L );
            }
            break;

          case PROCESSDICTIONARY:
            {
              BOOL        fOK = TRUE; // internal OK flag

              if ( pIda->fWithDict )
              {
                //make path for local dictionary objects
                UtlMakeEQFPath( pIda->szObjPath, pIda->chToDrive,
                                DIC_PATH, pIda->szTargetFolder );
                UtlAddImportToPath( pIda, pIda->szObjPath );

                //make path for dictionaries
                UtlMakeEQFPath( pIda->szSearchPath, pIda->chToDrive,
                                DIC_PATH, pIda->szTargetFolder );
                UtlAddImportToPath( pIda, pIda->szSearchPath );
                strcat( pIda->szSearchPath, BACKSLASH_STR );
                strcat( pIda->szSearchPath, DEFAULT_PATTERN_NAME );
                strcat( pIda->szSearchPath, EXT_OF_DIC );

                //build object for first imported dictionary
                fOK = FolImpBuildObject( pIda->szSearchPath,
                                         pIda->szObjPath,
                                         pIda->pszBuffer,
                                         pIda->ulBufSize,
                                         pIda->szCurFile );


                /****************************************************/
                /* if no local dictionary was found try remote ones */
                /****************************************************/
                if ( !fOK )
                {
                  //make path for local dictionary objects
                  UtlMakeEQFPath( pIda->szObjPath, pIda->chToDrive,
                                  COMDICT_PATH, pIda->szTargetFolder );
                  UtlAddImportToPath( pIda, pIda->szObjPath );

                  //make path for dictionaries
                  UtlMakeEQFPath( pIda->szSearchPath, pIda->chToDrive,
                                  COMDICT_PATH, pIda->szTargetFolder );
                  UtlAddImportToPath( pIda, pIda->szSearchPath );
                  strcat( pIda->szSearchPath, BACKSLASH_STR );
                  strcat( pIda->szSearchPath, DEFAULT_PATTERN_NAME );
                  strcat( pIda->szSearchPath, EXT_OF_DIC );

                  //build object for first imported dictionary
                  fOK = FolImpBuildObject( pIda->szSearchPath,
                                           pIda->szObjPath,
                                           pIda->pszBuffer,
                                           pIda->ulBufSize,
                                           pIda->szCurFile );
                } /* endif */

                /*****************************************************/
                /* if no remote dictionary was found try shared ones */
                /*****************************************************/
                if ( !fOK )
                {
                  //make path for shared dictionary objects
                  UtlMakeEQFPath( pIda->szObjPath, pIda->chToDrive,
                                  DIC_PATH, pIda->szTargetFolder );
                  UtlAddImportToPath( pIda, pIda->szObjPath );

                  //make path for dictionaries
                  UtlMakeEQFPath( pIda->szSearchPath, pIda->chToDrive,
                                  DIC_PATH, pIda->szTargetFolder );
                  UtlAddImportToPath( pIda, pIda->szSearchPath );
                  strcat( pIda->szSearchPath, BACKSLASH_STR );
                  strcat( pIda->szSearchPath, DEFAULT_PATTERN_NAME );
                  strcat( pIda->szSearchPath, EXT_OF_SHARED_DIC );

                  //build object for first imported dictionary
                  fOK = FolImpBuildObject( pIda->szSearchPath,
                                           pIda->szObjPath,
                                           pIda->pszBuffer,
                                           pIda->ulBufSize,
                                           pIda->szCurFile );
                } /* endif */

              }
              else
              {
                fOK = FALSE;         // do not import dictionaries
              } /* endif */

              if ( fOK )
              {
                if ( pIda->pDDEFolImp == NULL )
                {
                  HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
                  LOADSTRING( NULLHANDLE, hResMod, SID_FOLIMP_DICT, pIda->szAction );
                  WinSendMsg( hwnd, WM_EQF_UPDATESLIDER, MP1FROMSHORT(-1),
                              MP2FROMP(pIda->szAction) );
                } /* endif */

                UtlDispatch();
                pCommArea = (PPROCESSCOMMAREA)AccessGenProcCommArea( hwnd );
                if ( pCommArea == NULL )
                {
                  pIda = NULL;
                }
                else
                {
                  pIda = (PFOLIMPIDA)pCommArea->pUserIDA;
                } /* endif */

                if ( pIda != NULL )
                {
                  pIda->NextTask = PROCESSNEXTDICTIONARY;
                  strcpy( pIda->objCurProcess, DICIMP_PROC_OBJPREFIX );
                  strcat( pIda->objCurProcess, pIda->szCurFile );
                  if ( pIda->pDDEFolImp == NULL )
                  {
                    WinSendMsg( EqfQueryHandler(DICTIONARYHANDLER),
                                WM_EQF_PROCESSTASK,
                                MP1FROMSHORT(IMPORT_TASK),
                                MP2FROMP( pIda->pszBuffer ) );
                  }
                  else
                  {
                    PBATCHIMPORTPARMS pBatchImpParms = NULL;
                    if ( UtlAlloc( (PVOID *)&pBatchImpParms, 0L,
                                   min( MIN_ALLOC, sizeof(BATCHIMPORTPARMS) ),
                                   ERROR_STORAGE ) )
                    {
                      // de-allocation of pBatchimpParms will be done in
                      // called function
                      pBatchImpParms->pszObject  = pIda->pszBuffer;
                      pBatchImpParms->hwndErrMsg = pIda->hwndErrMsg;
                      WinSendMsg( EqfQueryHandler(DICTIONARYHANDLER),
                                  WM_EQF_PROCESSTASK,
                                  MP1FROMSHORT(BATCHIMPORT_TASK),
                                  MP2FROMP( pBatchImpParms ) );
                    } /* endif */
                  } /* endif */
                } /* endif */
              }
              else
              {
                //call up next process
                pIda->NextTask = PROCESSNEXTMEMORY;
                WinPostMsg( hwnd, WM_EQFN_TASKDONE, MP1FROMSHORT(TRUE), 0L );
              } /* endif */
            }
            break;

          case PROCESSNEXTDICTIONARY:
            {
              BOOL        fOK = TRUE; // internal OK flag

              if ( pIda->fWithDict )
              {
                /****************************************************/
                /* Delete last processed dictionary if it has not   */
                /* delete by the dictionary handler                 */
                /****************************************************/
                if ( UtlFileExist( pIda->szCurFile ) )
                {
                  UtlDelete( pIda->szCurFile, 0L, FALSE );
                } /* endif */

                if ( pIda->usProcessedObjects <= pIda->usMaxObjects ) pIda->usProcessedObjects++;


                //make path for local dictionary objects
                UtlMakeEQFPath( pIda->szObjPath, pIda->chToDrive,
                                DIC_PATH, pIda->szTargetFolder );
                UtlAddImportToPath( pIda, pIda->szObjPath );

                //make path for dictionaries
                UtlMakeEQFPath( pIda->szSearchPath, pIda->chToDrive,
                                DIC_PATH, pIda->szTargetFolder );
                UtlAddImportToPath( pIda, pIda->szSearchPath );
                strcat( pIda->szSearchPath, BACKSLASH_STR );
                strcat( pIda->szSearchPath, DEFAULT_PATTERN_NAME );
                strcat( pIda->szSearchPath, EXT_OF_DIC );

                //build object for first imported dictionary
                fOK = FolImpBuildObject( pIda->szSearchPath,
                                         pIda->szObjPath,
                                         pIda->pszBuffer,
                                         pIda->ulBufSize,
                                         pIda->szCurFile );


                /****************************************************/
                /* if no local dictionary was found try remote ones */
                /****************************************************/
                if ( !fOK )
                {
                  //make path for local dictionary objects
                  UtlMakeEQFPath( pIda->szObjPath, pIda->chToDrive,
                                  COMDICT_PATH, pIda->szTargetFolder );
                  UtlAddImportToPath( pIda, pIda->szObjPath );

                  //make path for dictionaries
                  UtlMakeEQFPath( pIda->szSearchPath, pIda->chToDrive,
                                  COMDICT_PATH, pIda->szTargetFolder );
                  UtlAddImportToPath( pIda, pIda->szSearchPath );
                  strcat( pIda->szSearchPath, BACKSLASH_STR );
                  strcat( pIda->szSearchPath, DEFAULT_PATTERN_NAME );
                  strcat( pIda->szSearchPath, EXT_OF_DIC );

                  //build object for first imported dictionary
                  fOK = FolImpBuildObject( pIda->szSearchPath,
                                           pIda->szObjPath,
                                           pIda->pszBuffer,
                                           pIda->ulBufSize,
                                           pIda->szCurFile );
                } /* endif */

                /*****************************************************/
                /* if no remote dictionary was found try shared ones */
                /*****************************************************/
                if ( !fOK )
                {
                  //make path for shared dictionary objects
                  UtlMakeEQFPath( pIda->szObjPath, pIda->chToDrive,
                                  DIC_PATH, pIda->szTargetFolder );
                  UtlAddImportToPath( pIda, pIda->szObjPath );

                  //make path for dictionaries
                  UtlMakeEQFPath( pIda->szSearchPath, pIda->chToDrive,
                                  DIC_PATH, pIda->szTargetFolder );
                  UtlAddImportToPath( pIda, pIda->szSearchPath );
                  strcat( pIda->szSearchPath, BACKSLASH_STR );
                  strcat( pIda->szSearchPath, DEFAULT_PATTERN_NAME );
                  strcat( pIda->szSearchPath, EXT_OF_SHARED_DIC );

                  //build object for first imported dictionary
                  fOK = FolImpBuildObject( pIda->szSearchPath,
                                           pIda->szObjPath,
                                           pIda->pszBuffer,
                                           pIda->ulBufSize,
                                           pIda->szCurFile );
                } /* endif */
              }
              else
              {
                fOK = FALSE;         // do not import dictionaries
              } /* endif */

              if ( fOK )
              {
                strcpy( pIda->objCurProcess, DICIMP_PROC_OBJPREFIX );
                strcat( pIda->objCurProcess, pIda->szCurFile );
                if ( pIda->pDDEFolImp == NULL )
                {
                  WinSendMsg( EqfQueryHandler(DICTIONARYHANDLER),
                              WM_EQF_PROCESSTASK,
                              MP1FROMSHORT(IMPORT_TASK),
                              MP2FROMP( pIda->pszBuffer ) );
                }
                else
                {
                  PBATCHIMPORTPARMS pBatchImpParms = NULL;
                  if ( UtlAlloc( (PVOID *)&pBatchImpParms, 0L,
                                 min( MIN_ALLOC, sizeof(BATCHIMPORTPARMS) ),
                                 ERROR_STORAGE ) )
                  {
                    // de-allocation of pBatchimpParms will be done in
                    // called function
                    pBatchImpParms->pszObject  = pIda->pszBuffer;
                    pBatchImpParms->hwndErrMsg = pIda->hwndErrMsg;
                    WinSendMsg( EqfQueryHandler(DICTIONARYHANDLER),
                                WM_EQF_PROCESSTASK,
                                MP1FROMSHORT(BATCHIMPORT_TASK),
                                MP2FROMP( pBatchImpParms ) );
                  } /* endif */
                } /* endif */
              }
              else
              {
                //call up next process
                pIda->NextTask = PROCESSNEXTMEMORY;
                WinPostMsg( hwnd, WM_EQFN_TASKDONE, MP1FROMSHORT(TRUE), 0L );
              } /* endif */
            }
            break;

          case IMPORTOK:
            {
              CHAR        szErrValue[MAX_DRIVE];// error parameter
              PSZ         pszErrParm[3];        // error parameter table
              BOOL fShowPropDialog = FALSE;

              // set the fGlobalMemOptCheckRequired flag in the folder properties if this flag had been set in the folder
              if ( pIda->fGlobalMemOptCheckRequired )
              {
                UtlMakeEQFPath( pIda->szObjPath, NULC, SYSTEM_PATH, NULL );
                strcat( pIda->szObjPath, BACKSLASH_STR );
                strcat( pIda->szObjPath, pIda->szTargetFolder );
                EQFINFO ErrorInfo = 0;
                HPROP hFolProp = OpenProperties( pIda->szObjPath, NULL, PROP_ACCESS_READ, &ErrorInfo);
                if ( hFolProp )
                {
                  if ( SetPropAccess( hFolProp, PROP_ACCESS_WRITE) )
                  {
                    PPROPFOLDER pFolProp = (PPROPFOLDER)MakePropPtrFromHnd( hFolProp );
                    pFolProp->fGlobalMemOptCheckRequired  = (EQF_BOOL)pIda->fGlobalMemOptCheckRequired;
                    fShowPropDialog = TRUE;
                    SaveProperties( hFolProp, &ErrorInfo);
                  } /* endif */
                  CloseProperties( hFolProp, PROP_QUIT, &ErrorInfo );
                } /* endif */
              }

              /*******************************************************/
              /* If a folder has no languages set and a memory       */
              /* has been imported for this folder try to set        */
              /* the folder languages from the memory (indirect      */
              /* by forcing a refresh of the folder listbox for      */
              /* this folder)                                        */
              /*******************************************************/
              if ( pIda->fNoFolderLanguage && pIda->fWithMem )
              {
                UtlMakeEQFPath( pIda->szObjPath, pIda->chToDrive,
                                SYSTEM_PATH,
                                pIda->szTargetFolder );
                EqfSend2AllHandlers( WM_EQFN_PROPERTIESCHANGED,
                                     MP1FROMSHORT( PROP_CLASS_FOLDER ),
                                     MP2FROMP(pIda->szObjPath) );
              } /* endif */
              if ( pIda->szImportDir[0]!= EOS )
              {
                UtlMakeEQFPath( pIda->szObjPath, pIda->chToDrive, SYSTEM_PATH, NULL );
                strcat( pIda->szObjPath, pIda->szImportDir );
                if ( UtlDirExist( pIda->szObjPath ) )
                {
                  UtlRemoveDir( pIda->szObjPath, FALSE );
                } /* endif */
              }
              else
              {
                UtlMakeEQFPath( pIda->szObjPath, pIda->chToDrive,
                                IMPORT_PATH,
                                NULL );
                UtlRemoveDir( pIda->szObjPath, FALSE );
              } /* endif */           

              if ( pIda->pDDEFolImp == NULL )
              {
                szErrValue[0] = pIda->chToDrive;
                szErrValue[1] = EOS;
                pszErrParm[0] = pIda->szLongFolName;
                pszErrParm[1] = szErrValue;
                OEMTOANSI(pIda->szLongFolName);

                if ( pIda->pszFolderList )
                {
                  // add folder name to name list
                  if ( pIda->pszNameList[0] != 0 )
                  {
                    strcat( pIda->pszNameList, ", " );
                  } /* endif */
                  strcat( pIda->pszNameList, "\"" );
                  strcat( pIda->pszNameList, pIda->szLongFolName );
                  strcat( pIda->pszNameList, "\"" );
                }
                else
                {
                  USHORT usMsg;
                  usMsg = ( pIda->fUserExitCopyPending ) ? MESSAGE_FOLIMP_COMPL_USEREXITINUSE : MESSAGE_FOLIMP_COMPLETED;
                  UtlError( usMsg, MB_OK, 2, pszErrParm, EQF_INFO );
                } /* endif */

                ANSITOOEM(pIda->szLongFolName);
              } /* endif */

              if ( fShowPropDialog )
              {
                UtlMakeEQFPath( pIda->szObjPath, NULC, SYSTEM_PATH, NULL  );
                strcat( pIda->szObjPath, BACKSLASH_STR );
                strcat( pIda->szObjPath, DEFAULT_FOLDERLIST_NAME );
                HWND hwndFolderList = EqfQueryObject( pIda->szObjPath, clsFOLDERLIST, 0);
                if ( hwndFolderList != NULLHANDLE )
                {
                  PostMessage( hwndFolderList , WM_COMMAND, MP1FROMSHORT( PID_FILE_MI_PROPERTIES ), 0L);
                } /* endif */         
              }

              pIda->NextTask = CLEANUP;
              WinPostMsg( hwnd, WM_EQFN_TASKDONE, MP1FROMSHORT(TRUE), 0L );
            }
            break;

          case IMPORTCANCEL:
            {
              CHAR        szErrValue[MAX_DRIVE];// error parameter
              PSZ         pszErrParm[3];        // error parameter table

              if ( pIda->szImportDir[0]!= EOS )
              {
                UtlMakeEQFPath( pIda->szObjPath, pIda->chToDrive, SYSTEM_PATH, NULL );
                strcat( pIda->szObjPath, pIda->szImportDir );
                if ( UtlDirExist( pIda->szObjPath ) )
                {
                  UtlRemoveDir( pIda->szObjPath, FALSE );
                } /* endif */
              }
              else
              {
                UtlMakeEQFPath( pIda->szObjPath, pIda->chToDrive,
                                IMPORT_PATH,
                                NULL );
                UtlRemoveDir( pIda->szObjPath, FALSE );
              } /* endif */           
              if ( !pIda->fNoMsg && (pIda->pDDEFolImp == NULL) )
              {
                USHORT usMsg;
                szErrValue[0] = pIda->chToDrive;
                szErrValue[1] = EOS;
                pszErrParm[0] = pIda->szLongFolName;
                pszErrParm[1] = szErrValue;
                OEMTOANSI(pIda->szLongFolName);
                usMsg = ( pIda->fUserExitCopyPending ) ? MESSAGE_FOLIMP_COMPL_USEREXITINUSE : MESSAGE_FOLIMP_TERMINATED;
                UtlError( usMsg, MB_CANCEL, 2, pszErrParm, EQF_ERROR );
                ANSITOOEM(pIda->szLongFolName);
              } /* endif */
              pIda->NextTask = CLEANUP;
              WinPostMsg( hwnd, WM_EQFN_TASKDONE, MP1FROMSHORT(TRUE), 0L );
            }
            break;

          case CLEANUP:
            EqfRemoveObject( TWBFORCE, hwnd);
            break;
        } /* endswitch */
      }
      else
      {
        CHAR        szErrValue[MAX_DRIVE];      // error parameter
        PSZ         pszErrParm[3];              // error parameter table

        if ( pIda->szImportDir[0]!= EOS )
        {
          UtlMakeEQFPath( pIda->szObjPath, pIda->chToDrive, SYSTEM_PATH, NULL );
          strcat( pIda->szObjPath, pIda->szImportDir );
          if ( UtlDirExist( pIda->szObjPath ) )
          {
            UtlRemoveDir( pIda->szObjPath, FALSE );
          } /* endif */
        } /* endif */           
        if ( pIda->pDDEFolImp == NULL )
        {
          USHORT usMsg;
          szErrValue[0] = pIda->chToDrive;
          szErrValue[1] = EOS;
          pszErrParm[0] = pIda->szLongFolName;
          pszErrParm[1] = szErrValue;
          OEMTOANSI(pIda->szLongFolName);
          usMsg = ( pIda->fUserExitCopyPending ) ? MESSAGE_FOLIMP_COMPL_USEREXITINUSE : MESSAGE_FOLIMP_TERMINATED;
          UtlError( usMsg, MB_CANCEL, 2, pszErrParm, EQF_ERROR );
          ANSITOOEM(pIda->szLongFolName);
        }
        else
        {
          pIda->pDDEFolImp->DDEReturn.usRc =
          UtlGetDDEErrorCode( pIda->pDDEFolImp->hwndErrMsg );
        } /* endif */
        pIda->NextTask = CLEANUP;
        WinPostMsg( hwnd, WM_EQFN_TASKDONE, MP1FROMSHORT(TRUE), 0L );
      } /* endif */
      break;
  } /* endswitch */
  return( mResult );
}




//------------------------------------------------------------------------------
// Internal function FolImpProcessFolder                                        
//------------------------------------------------------------------------------
// Function name:  FolImpProcessFolder                                          
//------------------------------------------------------------------------------
// Function call:                                                               
//------------------------------------------------------------------------------
// Description: Process a folder during import                                  
//------------------------------------------------------------------------------
// Parameters:                                                                  
//                                                                              
//------------------------------------------------------------------------------
// Returncode type:                                                             
//------------------------------------------------------------------------------
// Returncodes:                                                                 
//------------------------------------------------------------------------------
// Function flow:                                                               
//                                                                              
//                                                                              
//                                                                              
//                                                                              
//------------------------------------------------------------------------------

// Process a folder during import
USHORT FolImpProcessFolder( HWND hwnd, PFOLIMPIDA pIda )
{
  BOOL        fOK = TRUE; // internal OK flag
  PVOID       hFolProp;   // handle of folder properties
  EQFINFO     ErrorInfo;  // error code of property handler calls
  BOOL        fObjectNew = FALSE; // TRUE if a new object has been created
  PPROPFOLDER pFolProp = NULL;  // ptr to folder properties
  PSZ         pszTemp;    // general purpose temporary pointer
  PPROCESSCOMMAREA pCommArea;    // ptr to commmunication area
  // AFC: Variables for property merge
  EQF_BOOL  fTCMasterFolder = FALSE;         // Mark TC Master folder
  CHAR      szVendor[MAX_DESCRIPTION];       // Vendor Name (Translator)
  CHAR      szVendorEMail[MAX_DESCRIPTION];  // Vendor e-mail
  CHAR      szExpOriginator[MAX_DESCRIPTION];// Originator defined during export
  CHAR      szExpEMail[MAX_DESCRIPTION];     // Originator Name e-mail
  CHAR      szShipment[MAX_DESCRIPTION];     // folder shipment


  // correct source folder name using the name of the imported
  // property file (if any)
  {
    FILEFINDBUF stFile;              // Output buffer of UtlFindFirst
    USHORT usCount;                  // For UtlFindFirst
    HDIR hSearch;                    // Directory handle for UtlFindFirst
    USHORT usRC;

    hSearch = HDIR_CREATE;
    memset( &stFile, 0, sizeof(stFile) );
    usCount = 1;
    UtlMakeEQFPath( pIda->szObjPath, pIda->chToDrive,
                    PROPERTY_PATH, NULL );
    strcat( pIda->szObjPath, BACKSLASH_STR );
    strcat( pIda->szObjPath, DEFAULT_PATTERN_NAME );
    strcat( pIda->szObjPath, EXT_FOLDER_MAIN );
    UtlAddImportToPath( pIda, pIda->szObjPath );
    usRC = UtlFindFirst( pIda->szObjPath, &hSearch, 0, &stFile,
                         sizeof(stFile), &usCount, 0L, FALSE );
    UtlFindClose( hSearch, FALSE );
    if ( usRC == NO_ERROR )
    {
      strcpy( pIda->szSourceFolder, RESBUFNAME(stFile) );
    } /* endif */
  }

  /*****************************************************/
  /* Check if at least folder property file has        */
  /* been exported                                     */
  /*****************************************************/
  UtlMakeEQFPath( pIda->szObjPath, pIda->chToDrive,
                  PROPERTY_PATH, NULL );
  strcat( pIda->szObjPath, BACKSLASH_STR );
  strcat( pIda->szObjPath, pIda->szSourceFolder );
  UtlAddImportToPath( pIda, pIda->szObjPath );
  if ( !UtlFileExist(pIda->szObjPath) )
  {
    UtlErrorHwnd( ERROR_FOLIMP_NOPROPFILE, MB_CANCEL, 0, NULL,
                  EQF_ERROR, pIda->hwndErrMsg );
    fOK = FALSE;
  } /* endif */

  if ( fOK && (hwnd != HWND_FUNCIF) && (pIda->pDDEFolImp == NULL) )
  {
    HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
    LOADSTRING( NULLHANDLE, hResMod, SID_FOLIMP_FOLDER, pIda->szAction );
    WinSendMsg( hwnd, WM_EQF_UPDATESLIDER, MP1FROMSHORT(-1),
                MP2FROMP(pIda->szAction) );
    UtlDispatch();
    pCommArea = (PPROCESSCOMMAREA)AccessGenProcCommArea( hwnd );
    if ( pCommArea == NULL )
    {
      pIda = NULL;
    }
    else
    {
      pIda = (PFOLIMPIDA)pCommArea->pUserIDA;
    } /* endif */
    if ( pIda == NULL )
    {
      fOK = FALSE;
    } /* endif */
  } /* endif */

  // prepare any memory rename request
  if ( fOK && (pIda->szNewMemNames[0] != EOS) )
  {
    // load imported property file and fill memory rename table
    PPROPFOLDER pNewProps = NULL;
    USHORT usBytesRead = 0;

    UtlMakeEQFPath( pIda->szSearchPath, NULC, PROPERTY_PATH, NULL );
    strcat( pIda->szSearchPath, BACKSLASH_STR );
    strcat( pIda->szSearchPath, pIda->szSourceFolder );
    *pIda->szSearchPath = pIda->chToDrive;
    UtlAddImportToPath( pIda, pIda->szSearchPath );
    if ( UtlLoadFile( pIda->szSearchPath, (PVOID *)&pNewProps, &usBytesRead, TRUE, TRUE ) )
    { 
      fOK = (FolImpBuildMemRenameTable( pNewProps, &(pIda->MemRename), pIda->szNewMemNames ) == 0);
      UtlAlloc( (PVOID *)&pNewProps, 0, 0, NOMSG );
    } /* endif */
  } /* endif */

  if ( fOK )
  {
    CHAR chOrgDrive = pIda->szTargetFolObj[0];
    CHAR chSysDrive[MAX_DRIVE];

    UtlQueryString( QST_PRIMARYDRIVE, chSysDrive,
                    sizeof(chSysDrive) );
    pIda->szTargetFolObj[0] = chSysDrive[0];
    if ( !pIda->fReserved )
    {
      hFolProp = OpenProperties( pIda->szTargetFolObj, NULL,
                                 PROP_ACCESS_READ,
                                 &ErrorInfo);
    }
    else
    {
      // ignore short name reserved dummy file
      hFolProp = NULL;
    } /* endif */
    pIda->szTargetFolObj[0] = chOrgDrive;

    if ( hFolProp )          // does folder exist ???
    {
      // -- folder exists already, now check unique IDs  ---
      // (only if folder is not renamed and/or merged into
      // another one)
      fObjectNew = FALSE;
      pFolProp = (PPROPFOLDER)MakePropPtrFromHnd( hFolProp );

      switch ( pIda->pHeader->BitFlags.fHeaderType )
      {
        case RELEASE0_HEADER :
          pszTemp = pIda->pHeader->Head.Rel0.UniqueID;
          break;
        case RELEASE1_HEADER :
          pszTemp = pIda->pHeader->Head.Rel1.UniqueID;
          break;
      } /* endswitch */

      // folder unique ID check removed, as the import to this folder
      // has been confirmed already

      /**************************************************/
      /* Handle folder languages                        */
      /**************************************************/
      if ( fOK )
      {
        UtlMakeEQFPath( pIda->szObjPath, pIda->chToDrive,
                        PROPERTY_PATH, NULL );
        strcat( pIda->szObjPath, BACKSLASH_STR );
        strcat( pIda->szObjPath, pIda->szSourceFolder );
        UtlAddImportToPath( pIda, pIda->szObjPath );
        fOK = FolImpFolLang( pIda, pFolProp, pIda->hwndErrMsg,
                             pIda->szObjPath );
      } /* endif */

      //
      // AFC: save data for merge
      //
      fTCMasterFolder = pFolProp->fTCMasterFolder;
      strcpy(szVendor,pFolProp->szVendor );
      strcpy(szVendorEMail,pFolProp->szVendorEMail) ;
      strcpy(szExpOriginator,pFolProp->szExpOriginator) ;
      strcpy(szExpEMail,pFolProp->szExpEMail) ;
      strcpy(szShipment,pFolProp->szShipment) ;

      // remember original markup and original language settings of folder
      strcpy( pIda->szOrgFormat,     pFolProp->szFormat );
      strcpy( pIda->szOrgSourceLang, pFolProp->szSourceLang );
      strcpy( pIda->szOrgTargetLang, pFolProp->szTargetLang );

      // load new property file to copy some of the new properties to the old properties
      {
        PPROPFOLDER pNewProps = NULL;
        USHORT usBytesRead = 0;

        UtlMakeEQFPath( pIda->szSearchPath, NULC, PROPERTY_PATH, NULL );
        strcat( pIda->szSearchPath, BACKSLASH_STR );
        strcat( pIda->szSearchPath, pIda->szSourceFolder );
        *pIda->szSearchPath = pIda->chToDrive;
        UtlAddImportToPath( pIda, pIda->szSearchPath );
        if ( UtlLoadFile( pIda->szSearchPath, (PVOID *)&pNewProps, &usBytesRead, TRUE, TRUE ) )
        { 
          // check if this is a TranslationManager folder
          pIda->fTranslationManagerFolder = ( stricmp( pNewProps->PropHead.szPath + 2, "\\EQF" ) == 0 );


          if ( SetPropAccess( hFolProp, PROP_ACCESS_WRITE) )
          {
            BOOL fCopyStandardValues = FALSE;
            BOOL fCopyAFCSettings = FALSE;
            BOOL fCopyMTValues = FALSE;

            if ( pIda->pHeader->BitFlags.fMasterFolder )
            {
              // for master folder import we overwrite all settings
              fCopyStandardValues = TRUE;
              fCopyAFCSettings = TRUE;
              fCopyMTValues = TRUE;
            }
            else if ( pFolProp->fTCMasterFolder )
            {
              // for master folders we overwrite no properties
              fCopyStandardValues = FALSE;
              fCopyAFCSettings = FALSE;
              fCopyMTValues = FALSE;
            } 
            else 
            {
              // for normal folders we overwrite all relevant settings
              fCopyStandardValues = TRUE;
              fCopyAFCSettings = TRUE;
              fCopyMTValues = TRUE;
            } /* endif */

            // copy standard settings
            if ( fCopyStandardValues )
            {
              strcpy( pFolProp->szDescription,     pNewProps->szDescription );
              strcpy( pFolProp->szFormat,          pNewProps->szFormat );
              strcpy( pFolProp->szMemory,          pNewProps->szMemory );
              strcpy( pFolProp->szExlist,          pNewProps->szExlist );
              memcpy( pFolProp->DicTbl,            pNewProps->DicTbl, sizeof(pFolProp->DicTbl) ); 
              strcpy( pFolProp->szEditor,          pNewProps->szEditor );
              strcpy( pFolProp->szSourceLang,      pNewProps->szSourceLang ); 
              strcpy( pFolProp->szTargetLang,      pNewProps->szTargetLang );
              strcpy( pFolProp->szOrgName,         pNewProps->szOrgName );
              memcpy( pFolProp->MemTbl,            pNewProps->MemTbl, sizeof(pFolProp->MemTbl) );
              strcpy( pFolProp->szConversion,      pNewProps->szConversion );
              strcpy( pFolProp->szLongName,        pNewProps->szLongName );
              pFolProp->ulParentFolder           = pNewProps->ulParentFolder;
              strcpy( pFolProp->szLongMemory,      pNewProps->szLongMemory );
              memcpy( pFolProp->aLongMemTbl,       pNewProps->aLongMemTbl, sizeof(pFolProp->aLongMemTbl) );  
              memcpy( pFolProp->aLongDicTbl,       pNewProps->aLongDicTbl, sizeof(pFolProp->aLongDicTbl) );
              strcpy( pFolProp->szOrgLongName,     pNewProps->szOrgLongName );
              strcpy( pFolProp->szGlobalMemOptFile, pNewProps->szGlobalMemOptFile );
            } /* endif */


            // copy controlled folder settings
            if ( fCopyAFCSettings )
            {
              pFolProp->fAFCFolder               = pNewProps->fAFCFolder;
              pFolProp->fTCMasterFolder          = pNewProps->fTCMasterFolder;
              pFolProp->fPasswordOK              = pNewProps->fPasswordOK; 
              pFolProp->usChildNr                = pNewProps->usChildNr; 
              strcpy( pFolProp->szAFCPassword,     pNewProps->szAFCPassword );
              strcpy( pFolProp->szCoordinator,     pNewProps->szCoordinator );
              strcpy( pFolProp->szCoordinatorEMail, pNewProps->szCoordinatorEMail );
              strcpy( pFolProp->szParent,          pNewProps->szParent );
              strcpy( pFolProp->szParentEMail,     pNewProps->szParentEMail );
              strcpy( pFolProp->szExpOriginator,   pNewProps->szExpOriginator );
              strcpy( pFolProp->szExpEMail,        pNewProps->szExpEMail );
              strcpy( pFolProp->szVendor,          pNewProps->szVendor );
              strcpy( pFolProp->szVendorEMail,     pNewProps->szVendorEMail );
              pFolProp->usMaxHierachies          = pNewProps->usMaxHierachies;
              pFolProp->usShipment               = pNewProps->usShipment;
              strcpy( pFolProp->szProduct,         pNewProps->szProduct );
              strcpy( pFolProp->szProductFamily,   pNewProps->szProductFamily );
              strcpy( pFolProp->szSimilarProduct,  pNewProps->szSimilarProduct );
              strcpy( pFolProp->szSubjectDict,     pNewProps->szSubjectDict );
              strcpy( pFolProp->szSubjectMem,      pNewProps->szSubjectMem );
              strcpy( pFolProp->szPrevVersion,     pNewProps->szPrevVersion );  
              strcpy( pFolProp->szVersion,         pNewProps->szVersion );
              strcpy( pFolProp->szShipment,        pNewProps->szShipment );
              strcpy( pFolProp->szLogShipment,     pNewProps->szLogShipment );
            } /* endif */

            // copy MT infos
            if ( fCopyMTValues )
            {
              pFolProp->fMTFieldsFilled          = pNewProps->fMTFieldsFilled;
              pFolProp->fMTReceived              = pNewProps->fMTReceived;
              memcpy( pFolProp->ulMTTotalWords,    pNewProps->ulMTTotalWords, sizeof(pFolProp->ulMTTotalWords) );
              memcpy( pFolProp->ulMTTotalSegs,     pNewProps->ulMTTotalSegs, sizeof(pFolProp->ulMTTotalSegs) );
              memcpy( pFolProp->ulMTSendWords,     pNewProps->ulMTSendWords, sizeof(pFolProp->ulMTSendWords) );
              memcpy( pFolProp->ulMTSendSegs,      pNewProps->ulMTSendSegs, sizeof(pFolProp->ulMTSendSegs) );
              memcpy( pFolProp->ulMTReceivedWords, pNewProps->ulMTReceivedWords, sizeof(pFolProp->ulMTReceivedWords) );
              memcpy( pFolProp->ulMTReceivedSegs,  pNewProps->ulMTReceivedSegs, sizeof(pFolProp->ulMTReceivedSegs) );
              pFolProp->fMTLogging               = pNewProps->fMTLogging; 
            } /* endif */
          } /* endif */

          SaveProperties( hFolProp, &ErrorInfo);
          UtlAlloc( (PVOID *)&pNewProps, 0, 0, NOMSG );
        } /* endif */
      }

      // delete new property file
      UtlDelete( pIda->szSearchPath, 0L, FALSE );

      CloseProperties( hFolProp, PROP_QUIT, &ErrorInfo );
    }
    else
    {  // folder is a new one ---

       /**************************************************/
       /* Handle missing languages                       */
       /**************************************************/
      UtlMakeEQFPath( pIda->szObjPath, pIda->chToDrive,
                      PROPERTY_PATH, NULL );
      strcat( pIda->szObjPath, BACKSLASH_STR );
      strcat( pIda->szObjPath, pIda->szSourceFolder );
      UtlAddImportToPath( pIda, pIda->szObjPath );
      fOK = FolImpFolLang( pIda, NULL, pIda->hwndErrMsg,
                           pIda->szObjPath );

      if ( fOK )
      {
        fObjectNew = TRUE;

        //--- move folder property file from import directory to
        //    the property directory ---

        // make target path
        UtlMakeEQFPath( pIda->szObjPath, NULC, PROPERTY_PATH, NULL );
        strcat( pIda->szObjPath, BACKSLASH_STR );
        strcat( pIda->szObjPath, pIda->szTargetFolder );

        // make source path
        UtlMakeEQFPath( pIda->szSearchPath, NULC, PROPERTY_PATH, NULL );
        strcat( pIda->szSearchPath, BACKSLASH_STR );
        strcat( pIda->szSearchPath, pIda->szSourceFolder );
        *pIda->szSearchPath = pIda->chToDrive;
        UtlAddImportToPath( pIda, pIda->szSearchPath );
 
        // check if imported folder is a TranslationManager folder
        {
          PPROPFOLDER pTestProp = NULL;  
          USHORT usBytesRead = 0;
          if ( UtlLoadFile( pIda->szSearchPath, (PVOID *)&pTestProp, &usBytesRead, FALSE, FALSE ) )
          { 
          
            pIda->fTranslationManagerFolder = ( stricmp( pTestProp->PropHead.szPath + 2, "\\EQF" ) == 0 );
            UtlAlloc( (PVOID *)&pTestProp, 0, 0, NOMSG );
          }
        }


        // move property file
        // GQ 2017/04/27: use copy/delete combo as UtlSmartMove may fail when property file exists (e.g. because of a short name reserve dummy file) so we had to use 
        // UtlSmartMoveHwnd( pIda->szSearchPath, pIda->szObjPath, TRUE , pIda->hwndErrMsg );
        {
          PSZ pszParms[2];

          int iFileRC = 0;
          if ( !CopyFile( pIda->szSearchPath, pIda->szObjPath, FALSE ) )
          {
            iFileRC = GetLastError();
            pszParms[0] = pIda->szObjPath;
          } /* endif */

          if ( iFileRC == 0 )
          {
            if ( !DeleteFile( pIda->szSearchPath ) )
            {
              iFileRC = GetLastError();
              pszParms[0] = pIda->szSearchPath;
            } /* endif */
          } /* endif */

          if ( iFileRC != 0 )
          {
            char szCode[10];
            itoa( iFileRC, szCode, 10 );
            pszParms[1] = szCode;
            UtlErrorHwnd( (SHORT)iFileRC, MB_CANCEL, 2, pszParms, EQF_ERROR, pIda->hwndErrMsg );
          }
          pIda->fReserved = FALSE; // property file is not a dummy file anymore
        }

        // correct PROPHEAD part in property file
        fOK = FolCorrectPropHead( pIda->szObjPath,
                                  pIda->szObjPath[0],
                                  NULL,
                                  pIda->szTargetFolder,
                                  pIda->hwndErrMsg );

        //--- create folder directories ---

        // folder property directory
        UtlMakeEQFPath( pIda->szObjPath, pIda->chToDrive,
                        PROPERTY_PATH,
                        pIda->szTargetFolder );
        UtlMkMultDirHwnd( pIda->szObjPath, TRUE , pIda->hwndErrMsg );

        // folder source directory
        UtlMakeEQFPath( pIda->szObjPath, pIda->chToDrive,
                        DIRSOURCEDOC_PATH,
                        pIda->szTargetFolder );
        UtlMkMultDirHwnd( pIda->szObjPath, TRUE  , pIda->hwndErrMsg );

        // folder segmented source directory
        UtlMakeEQFPath( pIda->szObjPath, pIda->chToDrive,
                        DIRSEGSOURCEDOC_PATH,
                        pIda->szTargetFolder );
        UtlMkMultDirHwnd( pIda->szObjPath, TRUE  , pIda->hwndErrMsg );

        // folder target directory
        UtlMakeEQFPath( pIda->szObjPath, pIda->chToDrive,
                        DIRTARGETDOC_PATH,
                        pIda->szTargetFolder );
        UtlMkMultDirHwnd( pIda->szObjPath, TRUE  , pIda->hwndErrMsg );

        // folder segmented target directory
        UtlMakeEQFPath( pIda->szObjPath, pIda->chToDrive,
                        DIRSEGTARGETDOC_PATH,
                        pIda->szTargetFolder );
        UtlMkMultDirHwnd( pIda->szObjPath, TRUE  , pIda->hwndErrMsg );

        // folder RTF source directory
        UtlMakeEQFPath( pIda->szObjPath, pIda->chToDrive,
                        DIRSEGRTF_PATH,
                        pIda->szTargetFolder );
        UtlMkMultDirHwnd( pIda->szObjPath, TRUE  , pIda->hwndErrMsg );

        // folder EA data directory
        UtlMakeEQFPath( pIda->szObjPath, pIda->chToDrive,
                        EADATA_PATH,
                        pIda->szTargetFolder );
        UtlMkMultDirHwnd( pIda->szObjPath, TRUE  , pIda->hwndErrMsg );



        /**************************************************/
        // AFC: new properties
        /**************************************************/
        if ( !pIda->pHeader->BitFlags.fMasterFolder )
        {
          fTCMasterFolder = FALSE;
          strcpy(szVendor,"" );
          strcpy(szVendorEMail,"") ;
          strcpy(szExpOriginator,"") ;
          strcpy(szExpEMail,"") ;
          szShipment[0] = EOS;
        } /* endif */

      } /* endif */
    } /* endif */
  } /* endif */

  if ( fOK  )
  {
    UtlMakeEQFPath( pIda->szObjPath, NULC, SYSTEM_PATH, NULL );
    strcat( pIda->szObjPath, BACKSLASH_STR );
    strcat( pIda->szObjPath, pIda->szTargetFolder );
    hFolProp = OpenProperties( pIda->szObjPath, NULL,
                               PROP_ACCESS_READ,
                               &ErrorInfo);
    if ( hFolProp )
    {
      if ( SetPropAccess( hFolProp, PROP_ACCESS_WRITE) )
      {
        pFolProp = (PPROPFOLDER)MakePropPtrFromHnd( hFolProp );
        if ( fObjectNew )
        {
          if ( pFolProp->szOrgName[0] == EOS )
          {
            CHAR szShortName[MAX_FNAME];
            Utlstrccpy( szShortName, pIda->szSourceFolder, DOT );
            if ( strcmp( szShortName,
                         pIda->szTargetShortFolName ) != 0 )
            {
              strcpy( pFolProp->szOrgName, szShortName);
            } /* endif */
          } /* endif */
          if ( pFolProp->szOrgLongName[0] == EOS )
          {
            if ( strcmp( pIda->szLongFolName,
                         pIda->szTargetFolName ) != 0 )
            {
              strcpy( pFolProp->szOrgLongName, pIda->szLongFolName );
            } /* endif */
          } /* endif */
        } /* endif */

        pFolProp->chDrive = pIda->chToDrive;
        if ( (pFolProp->szSourceLang[0] == EOS) ||
             (pFolProp->szTargetLang[0] == EOS) )
        {
          /*********************************************/
          /* Folder has no languages set, remember     */
          /* this fact and try to update folder        */
          /* languages after the folder translation    */
          /* memory has been imported                  */
          /*********************************************/
          pIda->fNoFolderLanguage = TRUE;
        } /* endif */

        if ( strcmp( pIda->szTargetFolName, pIda->szTargetShortFolName ) != 0 )
        {
          strcpy( pFolProp->szLongName, pIda->szTargetFolName );
        }
        else
        {
          pFolProp->szLongName[0] = EOS;   // no folder long name
        } /* endif */

        pFolProp->PropHead.szPath[0] = pIda->szObjPath[0];

        // AFC: merge saved data into copied properties
        if (fOK)
        {
          // only overwrite settings if not imported as master folder
          if ( !pIda->pHeader->BitFlags.fMasterFolder )
          {
            pFolProp->fTCMasterFolder = fTCMasterFolder;
            strcpy(pFolProp->szVendor, szVendor);
            strcpy(pFolProp->szVendorEMail,szVendorEMail);
            strcpy(pFolProp->szShipment,szShipment );

            // handle originators
            // not in case of backup on the same machine
            if (strcmp(szExpOriginator,pFolProp->szExpOriginator ))
            {
              strcpy(pFolProp->szParent     ,pFolProp->szExpOriginator);
              strcpy(pFolProp->szParentEMail,pFolProp->szExpEMail);

              // merge back
              strcpy(pFolProp->szExpOriginator,szExpOriginator);
              strcpy(pFolProp->szExpEMail     ,szExpEMail);
            }//end if
          }//end if
        }//end if

        // remember language settings of imported folder
        strcpy( pIda->szImpSourceLang, pFolProp->szSourceLang );
        strcpy( pIda->szImpTargetLang, pFolProp->szTargetLang );

        // save imported markup and restore original markup table if
        // folders are merged
        if ( (strcmp( pIda->szSourceFolder, pIda->szTargetFolder ) != 0) &&
             (pIda->szOrgFormat[0] != EOS) )
        {
          strcpy( pIda->szImpFormat, pFolProp->szFormat );
          strcpy( pFolProp->szFormat, pIda->szOrgFormat );
        } /* endif */

        // remember actual markup and language settings of folder
        strcpy( pIda->szFolderFormat,     pFolProp->szFormat );
        strcpy( pIda->szFolderSourceLang, pFolProp->szSourceLang );
        strcpy( pIda->szFolderTargetLang, pFolProp->szTargetLang );

        // reset document export and import paths
        if ( fObjectNew )
        {
          pFolProp->fDocImpLastUsed = FALSE;
          pFolProp->szSavedDlgLoadDrive[0] = EOS; 
          pFolProp->sSavedDocImpDlgMode = 0;
          pFolProp->szSavedStartPath[0] = EOS;
          pFolProp->szSavedDlgLoadPath[0] = EOS;
          pFolProp->fDocExpLastUsed = FALSE;
          pFolProp->sSavedDlgFExpoFormat = 0;
          pFolProp->fSavedDlgFExpoTranslation = FALSE;
          pFolProp->fSavedDlgFExpoWithRevMark = FALSE;
          pFolProp->fSavedDlgFExpoWithTrackID = FALSE;
          pFolProp->fSavedDlgFExpoOriginal = FALSE;   
          pFolProp->fSavedDlgFExpoSNOMATCH = FALSE;   
          pFolProp->cSavedDlgFExpoDrive = EOS;      
          pFolProp->cSavedDlgFExpoDriveTrans = EOS; 
          pFolProp->cSavedDlgFExpoDriveOrg = EOS;   
          pFolProp->cSavedDlgFExpoDriveSno = EOS;   
          pFolProp->szSavedRevMark[0] = EOS;
          pFolProp->szSavedDlgFExpoTPath[0] = EOS; 
          pFolProp->szSavedDlgFExpoSPath[0] = EOS; 
          pFolProp->szSavedDlgFExpoNPath[0] = EOS; 
        } /* endif */

        // copy any global memory filter file name
        strcpy( pIda->szGlobalMemOptFile, pFolProp->szGlobalMemOptFile );

        // remember fGlobalMemOptCheckRequired flag and reset it, set the flag at the end of the import
        pIda->fGlobalMemOptCheckRequired = pFolProp->fGlobalMemOptCheckRequired;
        pFolProp->fGlobalMemOptCheckRequired = FALSE;

        pFolProp->fDisabled_UserExitRefresh = pIda->fUserExitCopyPending;

        // apply any memory renames
        {
          int iMemIndex = -1;

          while ( iMemIndex < MAX_NUM_OF_READONLY_MDB )
          {
            PSZ pszCurMem;

            pszCurMem = (iMemIndex == -1) ? pFolProp->szLongMemory : pFolProp->aLongMemTbl[iMemIndex]; 
            if ( pszCurMem[0] != EOS )
            {
              PSZ pszNewName = FolImpGetNewMemoryName( &(pIda->MemRename), pszCurMem );
              if ( pszNewName != pszCurMem )
              {
                strcpy( pszCurMem, pszNewName );
              } /* endif */
            } /* endif */
            iMemIndex++;
          } /* endwhile */
        }

        SaveProperties( hFolProp, &ErrorInfo);


      } /* endif */
      CloseProperties( hFolProp, PROP_QUIT, &ErrorInfo );
    } /* endif */

    UtlMakeEQFPath( pIda->szObjPath, pIda->chToDrive,
                    SYSTEM_PATH,
                    pIda->szTargetFolder );
    if ( hwnd != HWND_FUNCIF )
    {
      if ( fObjectNew )
      {
        EqfSend2AllHandlers( WM_EQFN_CREATED,
                             MP1FROMSHORT( clsFOLDER ),
                             MP2FROMP(pIda->szObjPath) );
      }
      else
      {
        EqfSend2AllHandlers( WM_EQFN_PROPERTIESCHANGED,
                             MP1FROMSHORT( PROP_CLASS_FOLDER ),
                             MP2FROMP(pIda->szObjPath) );
      } /* endif */
    } /* endif */
  } /* endif */

  // Process imported redundand segment list (if any)
  if ( fOK  )
  {
    // setup target path of redundand segment list 
    UtlMakeEQFPath( pIda->szObjPath, pIda->chToDrive, PROPERTY_PATH, pIda->szTargetFolder );
    strcat( pIda->szObjPath, BACKSLASH_STR  );
    strcat( pIda->szObjPath, REDUNDCOUNTFILE );

    // setup source path of redundand segment list 
    UtlMakeEQFPath( pIda->szSearchPath, pIda->chToDrive,
                    PROPERTY_PATH, pIda->szSourceFolder );
    strcat( pIda->szSearchPath, BACKSLASH_STR  );
    strcat( pIda->szSearchPath, REDUNDCOUNTFILE );
    UtlAddImportToPath( pIda, pIda->szSearchPath );

    // if there is an redundand segment list ...
    if ( UtlFileExist( pIda->szSearchPath ) )
    {
      // copy it to the folder
      UtlDelete( pIda->szObjPath, 0L, FALSE );
      UtlSmartMoveHwnd( pIda->szSearchPath, pIda->szObjPath, TRUE , pIda->hwndErrMsg );
    } /* endif */

    // in newer exported folders the redundant segment file is stored in the root directory of the folder...
    UtlMakeEQFPath( pIda->szSearchPath, pIda->chToDrive, SYSTEM_PATH, pIda->szSourceFolder );
    strcat( pIda->szSearchPath, BACKSLASH_STR  );
    strcat( pIda->szSearchPath, REDUNDCOUNTFILE );
    UtlAddImportToPath( pIda, pIda->szSearchPath );
    if ( UtlFileExist( pIda->szSearchPath ) )
    {
      // copy it to the folder
      UtlDelete( pIda->szObjPath, 0L, FALSE );
      UtlSmartMoveHwnd( pIda->szSearchPath, pIda->szObjPath, TRUE , pIda->hwndErrMsg );
    } /* endif */

  } /* endif */

  /*******************************************************/
  /* Process imported history log file (if any)          */
  /*******************************************************/
  if ( fOK  )
  {
    // setup target path of history log file
    UtlMakeEQFPath( pIda->szObjPath, pIda->chToDrive,
                    PROPERTY_PATH, pIda->szTargetFolder );
    strcat( pIda->szObjPath, BACKSLASH_STR  );
    strcat( pIda->szObjPath, HISTLOGFILE );

    // setup source path of history log file
    UtlMakeEQFPath( pIda->szSearchPath, pIda->chToDrive,
                    PROPERTY_PATH, pIda->szSourceFolder );
    strcat( pIda->szSearchPath, BACKSLASH_STR  );
    strcat( pIda->szSearchPath, HISTLOGFILE );
    UtlAddImportToPath( pIda, pIda->szSearchPath );

    // if there is an imported history log file...
    if ( UtlFileExist( pIda->szSearchPath ) )
    {
      // if there is already a history log file
      if ( UtlFileExist( pIda->szObjPath ) )
      {
        // merge imported history log file into existing one
        FolMergeHistoryLog( pIda->szSearchPath, pIda->szObjPath,
                            TRUE, pIda->hwndErrMsg,
                            NULL, NULL );

        // delete imported history log file
        UtlDelete( pIda->szSearchPath, 0L, FALSE );
      }
      else
      {
        // no history log file in folder, so copy the imported one
        UtlSmartMoveHwnd( pIda->szSearchPath, pIda->szObjPath,
                          TRUE , pIda->hwndErrMsg );
      } /* endif */
    } /* endif */
  } /* endif */

  // Process imported global memory filter (if any)
  if ( fOK  && (pIda->szGlobalMemOptFile[0] != EOS) )
  {
    // setup target path of global memory file file
    UtlMakeEQFPath( pIda->szObjPath, pIda->chToDrive, SYSTEM_PATH, NULL );
    strcat( pIda->szObjPath, BACKSLASH_STR  );
    strcat( pIda->szObjPath, pIda->szTargetFolder  );
    strcat( pIda->szObjPath, BACKSLASH_STR  );
    strcat( pIda->szObjPath, pIda->szGlobalMemOptFile );

    // setup source path of redundand segment list 
    UtlMakeEQFPath( pIda->szSearchPath, pIda->chToDrive, SYSTEM_PATH, NULL );
    strcat( pIda->szSearchPath, BACKSLASH_STR  );
    strcat( pIda->szSearchPath, pIda->szSourceFolder  );
    strcat( pIda->szSearchPath, BACKSLASH_STR  );
    strcat( pIda->szSearchPath, pIda->szGlobalMemOptFile );
    UtlAddImportToPath( pIda, pIda->szSearchPath );

    // if there is an imported global memory filter file ...
    if ( UtlFileExist( pIda->szSearchPath ) )
    {
      // ...copy it to the folder
      UtlDelete( pIda->szObjPath, 0L, FALSE );
      UtlSmartMoveHwnd( pIda->szSearchPath, pIda->szObjPath, TRUE , pIda->hwndErrMsg );
    } /* endif */
  } /* endif */

  /* Process imported binary calculation report file (if any)          */
  if ( fOK  )
  {
    // setup target path of binary calculation report file 
    UtlMakeEQFPath( pIda->szObjPath, pIda->chToDrive, SYSTEM_PATH, pIda->szTargetFolder );
    strcat( pIda->szObjPath, BACKSLASH_STR  );
    strcat( pIda->szObjPath, CALREPORTDATAFILE );

    // setup source path of binary calculation report file 
    UtlMakeEQFPath( pIda->szSearchPath, pIda->chToDrive, SYSTEM_PATH, pIda->szSourceFolder );
    strcat( pIda->szSearchPath, BACKSLASH_STR  );
    strcat( pIda->szSearchPath, CALREPORTDATAFILE );
    UtlAddImportToPath( pIda, pIda->szSearchPath );

    // if there is an binary calculation report file ...
    if ( UtlFileExist( pIda->szSearchPath ) )
    {
      // copy it to the folder
      UtlDelete( pIda->szObjPath, 0L, FALSE );
      UtlSmartMoveHwnd( pIda->szSearchPath, pIda->szObjPath, TRUE , pIda->hwndErrMsg );
    } /* endif */
  } /* endif */

  // adjust markup and language settings of existing documents if settings have been changed by the import
  if ( fOK  )
  {
    BOOL fMarkupChanged     = strcmp( pIda->szOrgFormat, pIda->szFolderFormat ) != 0;
    BOOL fSourceLangChanged = strcmp( pIda->szOrgSourceLang, pIda->szFolderSourceLang ) != 0;
    BOOL fTargetLangChanged = strcmp( pIda->szOrgTargetLang, pIda->szFolderTargetLang ) != 0;

    if ( (pIda->szOrgFormat[0]     && fMarkupChanged) ||
         (pIda->szOrgSourceLang[0] && fSourceLangChanged) ||
         (pIda->szOrgTargetLang[0] && fTargetLangChanged) )
    {
      HWND  hwndDocLB = NULLHANDLE; // listbox for document names
      SHORT sDocItem;               // current item in document listbox
      SHORT sNumOfDocs;             // number of documents
      BOOL  fDocOK;
      BOOL  fPropChanged = FALSE;   // TRUE = properties have been changed
      CHAR  szDocProp[MAX_EQF_PATH];// buffer for document properties name
      CHAR  szDocObj[MAX_EQF_PATH]; // buffer for document object name

      // create invisible listbox for folder documents
      hwndDocLB = WinCreateWindow( hwnd, WC_LISTBOX, "", WS_CHILD | LBS_STANDARD,
                                   0, 0, 0, 0, hwnd, HWND_TOP, 4712, NULL, NULL );

      // get documents of current folder
      EqfSend2Handler( FOLDERHANDLER, WM_EQF_INSERTNAMES, MP1FROMHWND(hwndDocLB), MP2FROMP(pIda->szTargetFolObj) );

      // loop over documents in listbox
      sDocItem = 0;
      sNumOfDocs = QUERYITEMCOUNTHWND( hwndDocLB );
      while ( fOK && (sDocItem < sNumOfDocs) )
      {
        CHAR szDocument[MAX_FILESPEC];
        PPROPDOCUMENT pDocProp = NULL;
        ULONG ulPropLen = 0L;                    // size of loaded doc property file

        fPropChanged = FALSE;
        fDocOK = TRUE;

        // setup document property file name
        QUERYITEMTEXTHWND( hwndDocLB, sDocItem, szDocument );
        UtlMakeEQFPath( szDocProp, pIda->chToDrive, PROPERTY_PATH, pIda->szTargetFolder );
        strcat( szDocProp, BACKSLASH_STR );
        strcat( szDocProp, szDocument );

        // setup document object name
        UtlMakeEQFPath( szDocObj, pIda->chToDrive, SYSTEM_PATH, pIda->szTargetFolder );
        strcat( szDocObj, BACKSLASH_STR );
        strcat( szDocObj, szDocument );

        // load document property file
        {
          fDocOK = UtlLoadFileL( szDocProp, (PVOID *)&pDocProp, &ulPropLen, FALSE, FALSE );
        }

        // adjust references in document property file
        if ( fDocOK )
        {
          if ( fMarkupChanged && (pDocProp->szFormat[0] == EOS) )
          {
            strcpy( pDocProp->szFormat, pIda->szOrgFormat );
            fPropChanged = TRUE;
          } /* endif */

          if ( fSourceLangChanged && (pDocProp->szSourceLang[0] == EOS) )
          {
            strcpy( pDocProp->szSourceLang, pIda->szOrgSourceLang );
            fPropChanged = TRUE;
          } /* endif */

          if ( fTargetLangChanged && (pDocProp->szTargetLang[0] == EOS) )
          {
            strcpy( pDocProp->szTargetLang, pIda->szOrgTargetLang );
            fPropChanged = TRUE;
          } /* endif */

          // rewrite document property file if changed
          if ( fPropChanged )
          {
            fDocOK = UtlWriteFileL( szDocProp, ulPropLen, pDocProp, FALSE ) == NO_ERROR;

            // broadcast props changed message
            if ( fDocOK )
            {
              EqfSend2AllHandlers( WM_EQFN_PROPERTIESCHANGED, MP1FROMSHORT( PROP_CLASS_DOCUMENT ), MP2FROMP(szDocObj) );
            } /* endif */
          } /* endif */

          // free document properties
          UtlAlloc( (PVOID *)&pDocProp, 0L, 0L, NOMSG );
        } /* endif */

        // next document
        sDocItem++;
      } /* endwhile */

      if ( hwndDocLB != NULLHANDLE ) WinDestroyWindow( hwndDocLB );
    } /* endif */
  } /* endif */

  if ( pIda != NULL )
  {
    pIda->NextTask = ( fOK ) ? PROCESSSUBFOLDERS : IMPORTCANCEL;
  } /* endif */

  if ( !fOK && (pIda->pDDEFolImp != NULL ) )
  {
    pIda->pDDEFolImp->DDEReturn.usRc =
    UtlGetDDEErrorCode( pIda->pDDEFolImp->hwndErrMsg );
  } /* endif */
  return( 0 );
} /* end of function FolImpProcessFolder */


// Process subfolders during import
USHORT FolImpProcessSubFolders( HWND hwnd, PFOLIMPIDA pIda )
{
  BOOL        fOK = TRUE; // internal OK flag
  PSUBFOLINFO pSubFolInfo = NULL; // info regarding existing subfolders


  // get subfolder information tables
  if ( fOK )
  {
    // get subfolder info table for imported subfolders
    UtlMakeEQFPath( pIda->szSearchPath, pIda->chToDrive, SYSTEM_PATH, pIda->szSourceFolder );
    UtlAddImportToPath( pIda, pIda->szSearchPath);
    SubFolCreateInfoTable( pIda->szSearchPath, &pIda->pImpSubFolInfo );

    // get subfolder info table for existing subfolders
    UtlMakeEQFPath( pIda->szSearchPath, pIda->chToDrive, SYSTEM_PATH, pIda->szTargetFolder );
    SubFolCreateInfoTable( pIda->szSearchPath, &pSubFolInfo );
  } /* endif */

  // preprocess subfolder information tables and check for ID changes
  if ( fOK )
  {
    int i, j;

    // check if subfolder IDs are in use for different subfolder names
    i = 0;
    while ( pIda->pImpSubFolInfo[i].szName[0] != EOS )
    {
      j = 0;
      while ( pSubFolInfo[j].szName[0] != EOS )
      {
        if ( (pIda->pImpSubFolInfo[i].ulID == pSubFolInfo[j].ulID) &&
             (strcmp( pIda->pImpSubFolInfo[i].szName, pSubFolInfo[j].szName ) != 0 ) )
        {
          // ID change required ...

          // get a free ID for the imported subfolder
          PPROPFOLDER pFolProp = NULL;
          ULONG ulLen = 0;

          UtlMakeEQFPath( pIda->szSearchPath, NULC, PROPERTY_PATH, NULL );
          strcat( pIda->szSearchPath, BACKSLASH_STR );
          strcat( pIda->szSearchPath, pIda->szTargetFolder );
          fOK = UtlLoadFileL( pIda->szSearchPath, (PVOID *)&pFolProp, &ulLen, FALSE, TRUE );

          if ( fOK )
          {
            ULONG ulID = pFolProp->ulNextSubFolderID;
            if ( ulID == 0 ) ulID = 1;
            do
            {
              sprintf( pIda->szBuffer, "%c%s\\%s\\%8.8lu%s", pFolProp->chDrive,
                       pIda->szSearchPath+1, PROPDIR, ulID++, EXT_OF_SUBFOLDER );
            }
            while ( UtlFileExist( pIda->szBuffer ) );

            // update next subfolder ID in folder properties
            pFolProp->ulNextSubFolderID = ulID;
            UtlWriteFileL( pIda->szSearchPath, ulLen, pFolProp, FALSE );

            // store new subfolder ID
            pIda->pImpSubFolInfo[i].ulValue = ulID - 1; // has been post-incremented!
          } /* endif */
          if ( pFolProp ) UtlAlloc( (PVOID *)&pFolProp, 0L, 0L, NOMSG );
        } /* endif */
        j++;
      } /* endwhile */
      i++;
    } /* endwhile */

    // discard subfolder info table for existing folder
    if ( pSubFolInfo ) UtlAlloc( (PVOID *)&pSubFolInfo, 0L, 0L, NOMSG );
  } /* endif */


  // loop over all imported subfolders and process them
  if ( fOK )
  {
    FILEFINDBUF stFile;              // Output buffer of UtlFindFirst
    USHORT usCount;                  // For UtlFindFirst
    HDIR hSearch = HDIR_CREATE;      // Directory handle for UtlFindFirst
    USHORT usRC;

    memset( &stFile, 0, sizeof(stFile) );
    usCount = 1;
    UtlMakeEQFPath( pIda->szSearchPath, pIda->chToDrive, PROPERTY_PATH, pIda->szSourceFolder );
    strcat( pIda->szSearchPath, BACKSLASH_STR );
    strcat( pIda->szSearchPath, DEFAULT_PATTERN_NAME );
    strcat( pIda->szSearchPath, EXT_OF_SUBFOLDER );
    UtlAddImportToPath( pIda, pIda->szSearchPath);
    usRC = UtlFindFirst( pIda->szSearchPath, &hSearch, 0, &stFile,
                         sizeof(stFile), &usCount, 0L, FALSE );
    while ( (usRC == NO_ERROR) && usCount )
    {
      // process found property file but ignore any history log files
      if ( strcmp( RESBUFNAME(stFile), HISTLOGFILE ) != 0 )
      {
        PPROPDOCUMENT pProp = NULL;
        ULONG ulLen = 0;

        // build property file path
        UtlSplitFnameFromPath( pIda->szSearchPath );
        strcat( pIda->szSearchPath, BACKSLASH_STR );
        strcat( pIda->szSearchPath, RESBUFNAME(stFile) );

        // handle subfolder or document
        // (the property head part of an subfolder property file is intentionally
        // left empty)
        if ( UtlLoadFileL( pIda->szSearchPath, (PVOID *)&pProp, &ulLen, FALSE, FALSE ) )
        {
          if ( pProp->PropHead.szName[0] == EOS )
          {
            BOOL fSubFolderExists = FALSE;
            ULONG ulNewID = 0L;

            // check if name change or parent ID is required because ID had
            // to be changed
            {
              PPROPFOLDER pPropFol = (PPROPFOLDER)pProp;
              BOOL fChangeRequired = FALSE;
              ULONG ulNewParentID = 0L;

              int i = 0;
              while ( pIda->pImpSubFolInfo[i].szName[0] != EOS )
              {
                if ( pPropFol->ulSubFolderID == pIda->pImpSubFolInfo[i].ulID )
                {
                  if ( pIda->pImpSubFolInfo[i].ulValue != 0L )
                  {
                    ulNewID = pIda->pImpSubFolInfo[i].ulValue;
                    fChangeRequired = TRUE;
                  } /* endif */
                } /* endif */

                if ( pPropFol->ulParentFolder == pIda->pImpSubFolInfo[i].ulID )
                {
                  if ( pIda->pImpSubFolInfo[i].ulValue != 0L )
                  {
                    ulNewParentID = pIda->pImpSubFolInfo[i].ulValue;
                    fChangeRequired = TRUE;
                  } /* endif */
                } /* endif */
                i++;
              } /* endwhile */

              if ( fChangeRequired )
              {
                if ( ulNewID != 0L )       pPropFol->ulSubFolderID = ulNewID;
                if ( ulNewParentID != 0L ) pPropFol->ulParentFolder = ulNewParentID;

                UtlWriteFileL( pIda->szSearchPath, ulLen, pPropFol, FALSE );
              } /* endif */
            }

            // setup target subfolder name
            UtlMakeEQFPath( pIda->szObjPath, pIda->chToDrive,
                            PROPERTY_PATH, pIda->szTargetFolder );
            strcat( pIda->szObjPath, BACKSLASH_STR  );
            if ( ulNewID != 0L )
            {
              sprintf( pIda->szObjPath+strlen(pIda->szObjPath), "%8.8d%s",
                       ulNewID, EXT_OF_SUBFOLDER );
            }
            else
            {
              strcat( pIda->szObjPath, RESBUFNAME(stFile) );
            } /* endif */
            if ( UtlFileExist( pIda->szObjPath ) )
            {
              fSubFolderExists = TRUE;
              UtlDelete( pIda->szObjPath, 0L, FALSE );
            } /* endif */

            // move subfolder property file to target folder
            UtlSmartMoveHwnd( pIda->szSearchPath, pIda->szObjPath,
                              TRUE, pIda->hwndErrMsg );

            // broadcast new or changed message
            if ( hwnd != HWND_FUNCIF )
            {
              if ( !fSubFolderExists )
              {
                EqfSend2AllHandlers( WM_EQFN_CREATED, MP1FROMSHORT( clsDOCUMENT ),
                                     MP2FROMP(pIda->szObjPath) );
              }
              else
              {
                EqfSend2AllHandlers( WM_EQFN_PROPERTIESCHANGED, MP1FROMSHORT( PROP_CLASS_DOCUMENT ),
                                     MP2FROMP(pIda->szObjPath) );
              } /* endif */
            } /* endif */
          } /* endif */
          UtlAlloc( (PVOID *)&pProp, 0L, 0L, NOMSG );
        } /* endif */
      } /* endif */

      // get next object
      usRC = UtlFindNext( hSearch, &stFile, sizeof(stFile), &usCount, FALSE );
    } /* endwhile */
   if ( hSearch != HDIR_CREATE ) UtlFindClose( hSearch, FALSE );
  } /* endif */

  if ( pIda != NULL )
  {
    pIda->NextTask = ( fOK ) ? PROCESSDOCS : IMPORTCANCEL;
  } /* endif */

  if ( !fOK && (pIda->pDDEFolImp != NULL ) )
  {
    pIda->pDDEFolImp->DDEReturn.usRc =
    UtlGetDDEErrorCode( pIda->pDDEFolImp->hwndErrMsg );
  } /* endif */
  return( 0 );
} /* end of function FolImpProcessSubFolders */



// helper function to correct path names
void FolImpCorrectPathName( PFOLIMPIDA pIda, std::string &strPath )
{
  // correct path names
  size_t iPos = strPath.find( '\\' ); 
  if ( iPos != std::string::npos ) iPos = strPath.find( '\\', iPos + 1 );
  if ( iPos != std::string::npos ) strPath.insert( iPos, pIda->szImportDir );
}

// process a single markup table info file
BOOL FolImpProcessMarkupTableInfoFile( HWND hwnd, PFOLIMPIDA pIda, PSZ pszInfoFile )
{
  BOOL fOK = TRUE;
  std::string strPlugin = "";
  std::string strMarkupName = "";
  std::string strMarkupTable = "";
  std::string strUserExit = "";
  std::string strVersion = "";
  std::string strDescription = "";

  std::vector<std::string> vAdditionalFiles;

  OtmMarkupPlugin *pPlugin = NULL;

  hwnd; 

  // read and process info file
  FILE *hfInfoFile = fopen( pszInfoFile, "r" );
  if ( hfInfoFile == NULL )
  {
    fOK = FALSE;
  }
  else
  {
    while ( !feof( hfInfoFile ) )
    {
      memset( pIda->szInfoFileLine, 0, sizeof(pIda->szInfoFileLine) );
      fgets( pIda->szInfoFileLine, sizeof(pIda->szInfoFileLine), hfInfoFile );
      if ( pIda->szInfoFileLine[0] != EOS )
      {
        pIda->szInfoFileLine[strlen(pIda->szInfoFileLine)-1] = EOS;

        PSZ pszValue = strchr( pIda->szInfoFileLine, '=' );
        if ( pszValue != NULL )
        {
          *pszValue = EOS;
          pszValue += 1;

          if ( strcmp( pIda->szInfoFileLine, MUTINFOFILE_PLUGIN_KEY ) == 0 )
          {
            strPlugin = pszValue;
          }
          else if ( strcmp( pIda->szInfoFileLine, MUTINFOFILE_TABLE_KEY ) == 0 )
          {
            *pszValue = pIda->chToDrive;                 // adjust drive letter
            strMarkupTable = pszValue;
          }
          else if ( strcmp( pIda->szInfoFileLine,MUTINFOFILE_USEREXIT_KEY ) == 0 )
          {
            *pszValue = pIda->chToDrive;                 // adjust drive letter
            strUserExit = pszValue;
          }
          else if ( strcmp( pIda->szInfoFileLine, MUTINFOFILE_ADDFILE_KEY ) == 0 )
          {
            *pszValue = pIda->chToDrive;                 // adjust drive letter
            vAdditionalFiles.push_back( pszValue );
          }
          else if ( strcmp( pIda->szInfoFileLine,MUTINFOFILE_NAME_KEY ) == 0 )
          {
            strMarkupName = pszValue;
          }
          else if ( strcmp( pIda->szInfoFileLine,MUTINFOFILE_VERSION_KEY ) == 0 )
          {
            strVersion = pszValue;
          }
          else if ( strcmp( pIda->szInfoFileLine,MUTINFOFILE_DESCRIPTION_KEY ) == 0 )
          {
            strDescription = pszValue;
          }
          else
          {
            // ignore unknown keys

          } /* endif */
        } /* endif */
      }
    } /* endwhile */
    fclose( hfInfoFile );
  } /* endif */

  // check if required infos have been found
  if ( fOK )
  {
    if ( (strPlugin.length() == 0) || (strMarkupTable.length() == 0) )
    {
      // insuffient data in markup table info file
      PSZ   pszErrParm = UtlGetFnameFromPath( pszInfoFile );
      UtlErrorHwnd( ERROR_MARKUPTABLEINFO, MB_CANCEL, 1, &pszErrParm, EQF_ERROR, pIda->hwndErrMsg );
      fOK = FALSE;
    } /* endif */
  } /* endif */

  // check if required markup table plugin is available
  if ( fOK )
  {
    PluginManager* pPluginManager = PluginManager::getInstance();
    pPlugin = (OtmMarkupPlugin *) pPluginManager->getPlugin( strPlugin.c_str() );
    if ( pPlugin == NULL )
    {
      pPlugin = GetMarkupPlugin( (PSZ)strMarkupName.c_str() ); 
    }
    if ( pPlugin == NULL )
    {
      PSZ   pszErrParm = (PSZ)strMarkupName.c_str();
      UtlErrorHwnd( ERROR_PLUGINREQUIRED, MB_CANCEL, 1, &pszErrParm, EQF_ERROR, pIda->hwndErrMsg );
      fOK = FALSE;
    } /* endif */
  } /* endif */

 

  // call markup table plugin to handle the import of the markup table files
  if ( fOK )
  {
    std::string strAddFiles = "";

    // correct path names
    FolImpCorrectPathName( pIda, strMarkupTable ); 
    if ( strUserExit.length() != 0 ) FolImpCorrectPathName( pIda, strUserExit ); 
    for( size_t i = 0; i < vAdditionalFiles.size(); i++ )
    {
      FolImpCorrectPathName( pIda, vAdditionalFiles[i] );
      if ( i != 0 ) strAddFiles.append( "," );
      strAddFiles.append( vAdditionalFiles[i] );
    } /* endfor */

    // call markup table plugin
    int iRC = pPlugin->updateFiles( (char *)strMarkupName.c_str(), (char *)strDescription.c_str(), (char *)strDescription.c_str(), 
      (char *)strVersion.c_str(), (char *)strMarkupTable.c_str(), (char *)strUserExit.c_str(), (char *)strAddFiles.c_str() );
    if ( iRC == UPDATE_MARKUP_INUSE ) 
    {
       pIda->fUserExitCopyPending = TRUE ;
    }

  } /* endif */

  return( fOK );
} /* end of function FolImpProcessMarkupTableInfoFile */


// process the markup tables imported with the folder
BOOL FolImpProcessTables( HWND hwnd, PFOLIMPIDA pIda )
{
  BOOL        fOK = TRUE; // internal OK flag
  PSZ         pszObject;  // ptr to object currently processed
  PSZ         pszTemp;    // general purpose temporary pointer
  BOOL        fMoved;     // TRUE if file had to be moved
  CHAR        szSysDrive[MAX_DRIVE+1];  // buffer for system drive
  PPROCESSCOMMAREA pCommArea;    // ptr to commmunication area
  BOOL        fFiles = FALSE;

  if ( (hwnd != HWND_FUNCIF) && (pIda->pDDEFolImp == NULL) )
  {
    HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
    LOADSTRING( NULLHANDLE, hResMod, SID_FOLIMP_TABLES, pIda->szAction );
    WinSendMsg( hwnd, WM_EQF_UPDATESLIDER, MP1FROMSHORT(-1),
                MP2FROMP(pIda->szAction) );
    UtlDispatch();
    pCommArea = (PPROCESSCOMMAREA)AccessGenProcCommArea( hwnd );
    if ( pCommArea == NULL )
    {
      pIda = NULL;
    }
    else
    {
      pIda = (PFOLIMPIDA)pCommArea->pUserIDA;
    } /* endif */
  } /* endif */

  if ( pIda != NULL )
  {
    //--- make path for markup table info objects ---
    UtlMakeEQFPath( pIda->szObjPath, pIda->chToDrive, TABLE_PATH, pIda->szTargetFolder );
    UtlAddImportToPath( pIda, pIda->szObjPath );

    //--- make search path for tables ---
    strcpy( pIda->szSearchPath, pIda->szObjPath );
    strcat( pIda->szSearchPath, BACKSLASH_STR );
    strcat( pIda->szSearchPath, DEFAULT_PATTERN_NAME );
    strcat( pIda->szSearchPath, EXT_OF_TAGTABLEINFO );

    //--- build and process list of imported markup table info files ---
    fFiles = FolImpBuildObjList( pIda->szSearchPath, pIda->szObjPath, &pIda->pszBuffer, &pIda->ulBufSize );
    if ( fFiles )
    {

      // skip handler name
      pszObject = strchr( pIda->pszBuffer, X15 ) + 1;
      // get system drive
      UtlQueryString( QST_PRIMARYDRIVE, szSysDrive,
                      sizeof(szSysDrive) );

      while ( fOK && *pszObject )
      {
        // isolate markup table info file name
        pszTemp = strchr( pszObject, X15 );
        *pszTemp = EOS;

        // process markup table info file
        fOK = FolImpProcessMarkupTableInfoFile( hwnd, pIda, pszObject );

        // continue with next object
        if ( pIda->usProcessedObjects <= pIda->usMaxObjects ) pIda->usProcessedObjects++;
        pszObject = pszTemp + 1;
      } /* endwhile */
    } /* endif */


    /*****************************************************/
    /* Build list of analysis profiles                   */
    /*****************************************************/
    if ( fOK )
    {
      //--- make path for table user exit objects ---
      UtlMakeEQFPath( pIda->szObjPath, pIda->chToDrive, PROPERTY_PATH, NULL );
      UtlAddImportToPath( pIda, pIda->szObjPath );

      //--- make search path for table user exits ---
      strcpy( pIda->szSearchPath, pIda->szObjPath );
      strcat( pIda->szSearchPath, BACKSLASH_STR );
      strcat( pIda->szSearchPath, DEFAULT_PATTERN_NAME );
      strcat( pIda->szSearchPath, EXT_OF_ANAPROFILE );

      //--- build list of imported analysis profiles ---
      fFiles = FolImpBuildObjList( pIda->szSearchPath, pIda->szObjPath, &pIda->pszBuffer, &pIda->ulBufSize );
    } /* endif */

    /*****************************************************/
    /* Process list of analysis profiles                 */
    /*****************************************************/
    UtlMakeEQFPath( pIda->szObjPath, pIda->chToDrive, PROPERTY_PATH, NULL );
    if ( fOK && fFiles )
    {
      // skip handler name
      pszObject = strchr( pIda->pszBuffer, X15 ) + 1;
      // get system drive
      UtlQueryString( QST_PRIMARYDRIVE, szSysDrive, sizeof(szSysDrive) );

      while ( *pszObject )
      {
        // isolate analysis profile name
        pszTemp = strchr( pszObject, X15 );
        *pszTemp = EOS;

        strcpy( pIda->szObjPath, pszObject );
        UtlRemoveImportFromPath( pIda, pIda->szObjPath );
        pIda->szObjPath[0] = szSysDrive[0];
        fMoved = UtlMoveIfNewerOrNotExistHwnd( pszObject, pIda->szObjPath, pIda->hwndErrMsg );
        // continue with next object
        pszObject = pszTemp + 1;
      } /* endwhile */
    } /* endif */


  } /* endif */


  return( fOK );
} /* end of function FolImpProcessTables */

/**********************************************************************/
/* Create and fill document processing disposition table              */
/**********************************************************************/
BOOL FolImpPreProcessDocs
(
PFOLIMPIDA        pIda,              // ptr to folder import IDA
PBOOL      pfUserInput               // ptr to user input flag
)
{

  BOOL        fOK = TRUE;              // function return code
  USHORT      usNumOfDocs = 0;         // number of documents in list

  // initialize user input flag
  *pfUserInput = FALSE;

  // create string pool for document names
  if ( fOK )
  {
    pIda->pPool = PoolCreate( 16000 );
    fOK = (pIda->pPool != NULL);
  } /* end if */

  // count number of documents in object list in pszBuffer
  if ( fOK )
  {
    PSZ pszObject = strchr( pIda->pszBuffer, X15 ) + 1;
    while ( pszObject && *pszObject )
    {
      pszObject = strchr( pszObject, X15 );
      if ( pszObject )
      {
        pszObject++;
        usNumOfDocs++;
      } /* endif */
    } /* endwhile */
    usNumOfDocs += 2;                   // some extra space ...
  } /* endif */

  // allocate array for documents
  if ( fOK )
  {
    ULONG ulSize = (ULONG)sizeof(FOLIMPDOCDATA) * (ULONG)usNumOfDocs;

    fOK = UtlAlloc( (PVOID *)&(pIda->pDocData), 0L, ulSize, ERROR_STORAGE );
  } /* endif */

  // fill array with document object names
  if ( fOK )
  {
    PSZ     pszTemp;
    PSZ     pszObject = strchr( pIda->pszBuffer, X15 ) + 1;
    SHORT   i = 0;                  // array index

    while ( fOK && (pszObject && *pszObject) )
    {
      // isolate and copy document object name
      pszTemp = strchr( pszObject, X15 );
      *pszTemp = EOS;
      pIda->pDocData[i].pszDocObjName = PoolAddString( pIda->pPool, pszObject );
      fOK = (pIda->pDocData[i].pszDocObjName != NULL);
      i++;

      // continue with next object
      pszObject = pszTemp + 1;
    } /* endwhile */
  } /* endif */

  // collect document information and set default processing disposition
  if ( fOK )
  {
    PFOLIMPDOCDATA pData = pIda->pDocData;

    while ( fOK && (pData->pszDocObjName != NULL) )
    {
      PSZ    pszDocName;              // ptr to document name

      // get position of document name within document object name
      pszDocName = UtlGetFnameFromPath( pData->pszDocObjName );

      // use source short name as default target short name
      strcpy( pData->szTargetShortName, pszDocName );

      // preset "document is new" flag
      pData->fIsNew = FALSE;

      // get document long name from imported document property file
      {
        PPROPDOCUMENT pProp = NULL;
        ULONG ulPropSize = 0;

        UtlMakeEQFPath( pIda->szObjPath, pIda->chToDrive,
                        PROPERTY_PATH, pIda->szSourceFolder );
        strcat( pIda->szObjPath, BACKSLASH_STR );
        strcat( pIda->szObjPath, pszDocName );
        UtlAddImportToPath( pIda, pIda->szObjPath );
        fOK = UtlLoadFileL( pIda->szObjPath, (PVOID *)&pProp, &ulPropSize,
                           FALSE, TRUE );

        if ( fOK && (pProp->szLongName[0] != EOS) )
        {
          // store document long name
          pData->pszLongName = PoolAddString( pIda->pPool, pProp->szLongName );
          fOK = (pData->pszLongName != NULL);

          // get document short name as used in existing folder
          FolLongToShortDocName( pIda->szTargetFolObj, pProp->szLongName,
                                 pData->szTargetShortName, &(pData->fIsNew) );

        } /* endif */

        // free any memory allocated for document properties
        if ( pProp != NULL ) UtlAlloc( (PVOID *)&pProp, 0L, 0L, NOMSG );
      }

      // get values for source document
      UtlMakeEQFPath( pIda->szObjPath, pIda->chToDrive,
                      DIRSOURCEDOC_PATH, pIda->szTargetFolder );
      strcat( pIda->szObjPath, BACKSLASH_STR );
      strcat( pIda->szObjPath, pData->szTargetShortName );
      FolImpGetFileDate( pIda->szObjPath, &pData->fdateSource,
                         &pData->ftimeSource );

      // get values for imported source document
      UtlMakeEQFPath( pIda->szObjPath, pIda->chToDrive,
                      DIRSOURCEDOC_PATH, pIda->szSourceFolder );
      strcat( pIda->szObjPath, BACKSLASH_STR );
      strcat( pIda->szObjPath, pszDocName );
      UtlAddImportToPath( pIda, pIda->szObjPath );
      FolImpGetFileDate( pIda->szObjPath, &pData->fdateImpSource,
                         &pData->ftimeImpSource );

      // get values for target document
      UtlMakeEQFPath( pIda->szObjPath, pIda->chToDrive,
                      DIRSEGTARGETDOC_PATH, pIda->szTargetFolder );
      strcat( pIda->szObjPath, BACKSLASH_STR );
      strcat( pIda->szObjPath, pData->szTargetShortName );
      FolImpGetFileDate( pIda->szObjPath, &pData->fdateTarget,
                         &pData->ftimeTarget );

      // get values for imported target document
      UtlMakeEQFPath( pIda->szObjPath, pIda->chToDrive,
                      DIRSEGTARGETDOC_PATH, pIda->szSourceFolder );
      strcat( pIda->szObjPath, BACKSLASH_STR );
      strcat( pIda->szObjPath, pszDocName );
      UtlAddImportToPath( pIda, pIda->szObjPath );
      FolImpGetFileDate( pIda->szObjPath, &pData->fdateImpTarget,
                         &pData->ftimeImpTarget );

      // set status of source document
      if ( pData->fIsNew )
      {
        // document is new
        pData->bSourceState = NEW_DOCSTAT;
      }
      else if ( ISFDATEZERO(pData->fdateSource) && ISFTIMEZERO( pData->ftimeSource ) )
      {
        // document is new
        pData->bSourceState = NEW_DOCSTAT;
        pData->fIsNew;
      }
      else
      {
        LONG lCompare;

        lCompare = UtlCompareDate( &(pData->fdateSource),
                                   &(pData->fdateImpSource) );
        if ( lCompare == 0L )
        {
          lCompare = UtlCompareTime( &(pData->ftimeSource),
                                     &(pData->ftimeImpSource) );
        } /* endif */

        if ( lCompare < 0L )
        {
          pData->bSourceState = NEWER_DOCSTAT;
        }
        else if ( lCompare > 0L )
        {
          pData->bSourceState = OLDER_DOCSTAT;
        }
        else
        {
          pData->bSourceState = EQUAL_DOCSTAT;
        } /* endif */
      } /* endif */

      // set status of target document
      if ( pData->fIsNew ||
           ( ISFDATEZERO(pData->fdateTarget) &&
             ISFTIMEZERO( pData->ftimeTarget ) ) )
      {
        if ( ISFDATEZERO(pData->fdateImpTarget) &&
             ISFTIMEZERO( pData->ftimeImpTarget ) )
        {
          pData->bTargetState = NONE_DOCSTAT;
        }
        else
        {
          pData->bTargetState = NEW_DOCSTAT;
        } /* endif */
      }
      else if ( ISFDATEZERO(pData->fdateImpTarget) &&
                ISFTIMEZERO( pData->ftimeImpTarget ) )
      {
        pData->bTargetState = OLDEXIST_DOCSTAT;
      }
      else
      {
        LONG lCompare;

        lCompare = UtlCompareDate( &(pData->fdateTarget),
                                   &(pData->fdateImpTarget) );
        if ( lCompare == 0L )
        {
          lCompare = UtlCompareTime( &(pData->ftimeTarget),
                                     &(pData->ftimeImpTarget) );
        } /* endif */

        if ( lCompare < 0L )
        {
          pData->bTargetState = NEWER_DOCSTAT;
        }
        else if ( lCompare > 0L )
        {
          pData->bTargetState = OLDER_DOCSTAT;
        }
        else
        {
          pData->bTargetState = EQUAL_DOCSTAT;
        } /* endif */
      } /* endif */

      // set document disposition
      if ( (pData->bSourceState == NEW_DOCSTAT) ||
           (pData->bSourceState == NEWER_DOCSTAT) )
      {
        if ( pData->bTargetState == NONE_DOCSTAT )
        {
          pData->bDisposition = COPYSRC_DOCDISP;
        }
        else if ( pData->bTargetState == OLDEXIST_DOCSTAT )
        {
          pData->bDisposition = COPYSRCDELTGT_DOCDISP;
        }
        else
        {
          pData->bDisposition = COPYSRCTGT_DOCDISP;
        } /* endif */
      }
      else if ( pData->bSourceState == EQUAL_DOCSTAT )
      {
        if ( (pData->bTargetState == NEW_DOCSTAT) ||
             (pData->bTargetState == NEWER_DOCSTAT) )
        {
          pData->bDisposition = COPYTGT_DOCDISP;
        }
        else
        {
          pData->bDisposition = IGNORE_DOCDISP;
        } /* endif */
      }
      else
      {
        pData->bDisposition = IGNORE_DOCDISP;
      } /* endif */

      // check if user input is required
      if ( (pData->bSourceState == NEW_DOCSTAT) &&
           ( (pData->bTargetState == NEW_DOCSTAT) ||
             (pData->bTargetState == NONE_DOCSTAT) ) )
      {
        // no user input required, document is new
      }
      else if ( (pData->bSourceState == EQUAL_DOCSTAT) &&
                ( (pData->bTargetState == EQUAL_DOCSTAT) ||
                  (pData->bTargetState == NONE_DOCSTAT) ) )
      {
        // no user input required, documents are equal
      }
      else
      {
        // user input may be necessary
        *pfUserInput = TRUE;
      } /* endif */

      // next document
      pData++;
    } /* endwhile */
  } /* endif */

  return( fOK );
} /* end of function FolImpPreProcessDocs */


/**********************************************************************/
/* Get file date and time for the given file                          */
/**********************************************************************/
USHORT FolImpGetFileDate
(
PSZ         pszPath,                 // path of file being checked
FDATE       *pfileDate,              // ptr to buffer for file date
FTIME       *pfileTime               // ptr to buffer for file time
)
{
  FILEFINDBUF stFile;              // Output buffer of UtlFindFirst
  USHORT usCount;                  // For UtlFindFirst
  HDIR hSearch;                    // Directory handle for UtlFindFirst
  USHORT usRC;

  hSearch = HDIR_CREATE;
  memset( &stFile, 0, sizeof(stFile) );
  usCount = 1;
  usRC = UtlFindFirst( pszPath, &hSearch, 0, &stFile, sizeof(stFile),
                       &usCount, 0L, FALSE );
  UtlFindClose( hSearch, FALSE );
  if ( usRC == NO_ERROR )
  {
    FileTimeToDosDateTime( &stFile.ftLastWriteTime, (LPWORD)pfileDate, (LPWORD)pfileTime );
  }
  else
  {
    memset( pfileDate, 0, sizeof(FDATE) );
    memset( pfileTime, 0, sizeof(FTIME) );
  } /* endif */

  return( usRC );
} /* end of function FolImpGetFileDate */

//------------------------------------------------------------------------------
// Internal function FolImpProcessDocs                                          
//------------------------------------------------------------------------------
// Function name:  FolImpProcessDocs                                            
//------------------------------------------------------------------------------
// Function call:                                                               
//------------------------------------------------------------------------------
// Description:  Process documents in document processing disposition table     
//------------------------------------------------------------------------------
// Parameters:                                                                  
//                                                                              
//------------------------------------------------------------------------------
// Returncode type:                                                             
//------------------------------------------------------------------------------
// Returncodes:                                                                 
//------------------------------------------------------------------------------
// Function flow:                                                               
//                                                                              
//                                                                              
//                                                                              
//                                                                              
//------------------------------------------------------------------------------

/**********************************************************************/
/* Process documents in document processing disposition table         */
/**********************************************************************/
BOOL FolImpProcessDocs
(
PFOLIMPIDA  pIda                     // ptr to folder import IDA
)
{
  typedef struct _PROCESSDOCDATA       // structure for private data
  {
    CHAR      szDocObj[MAX_LONGFILESPEC];  // document object name
    CHAR      szProp[MAX_LONGFILESPEC];    // property path of existing document
    CHAR      szSource[MAX_LONGFILESPEC];  // source path of existing document
    CHAR      szTarget[MAX_LONGFILESPEC];  // target path of existing document
    CHAR      szRTF[MAX_LONGFILESPEC];     // RTF source path of existing document
    CHAR      szSegSource[MAX_LONGFILESPEC];// seg. source path of existing document
    CHAR      szSegTarget[MAX_LONGFILESPEC];// seg. target path of existing document
    CHAR      szImpProp[MAX_LONGFILESPEC];    // property path of imported document
    CHAR      szImpSource[MAX_LONGFILESPEC];  // source path of imported document
    CHAR      szImpTarget[MAX_LONGFILESPEC];  // target path of imported document
    CHAR      szImpRTF[MAX_LONGFILESPEC];  // RTF source path of imported document
    CHAR      szImpSegSource[MAX_LONGFILESPEC];// seg. source path of imported document
    CHAR      szImpSegTarget[MAX_LONGFILESPEC];// seg. target path of imported document
    CHAR      szTempPath[MAX_LONGFILESPEC];// buffer for path names
    CHAR      szSize[40];              // size of existing document
    CHAR      szImpSize[40];           // size of imported document
    CHAR      szDate[80];              // date/time of existing document
    CHAR      szImpDate[80];           // date/time of imported document
    FDATE     filDate;                 // date of existing document
    FTIME     filTime;                 // time of existing document
    FDATE     filImpDate;              // date of imported document
    FTIME     filImpTime;              // time of imported document
    DOCIMPORTHIST DocImpHist;          // history record for document import
    CHAR      szEAData[MAX_LONGFILESPEC];  // EA data of existing document
    CHAR      szImpEAData[MAX_LONGFILESPEC];// EA data of imported document
    CHAR      szMTLog[MAX_LONGFILESPEC];    // MT log file of existing document
    CHAR      szImpMTLog[MAX_LONGFILESPEC]; // MT log file of imported document
    CHAR      szDirBuffer[MAX_LONGFILESPEC]; // buffer for directory names
    CHAR      szMisc[MAX_LONGFILESPEC];    // Misc file of existing document
    CHAR      szImpMisc[MAX_LONGFILESPEC]; // Misc file of imported document
    CHAR      szXLIFF[MAX_LONGFILESPEC];    // XLIFF file of existing document
    CHAR      szImpXLIFF[MAX_LONGFILESPEC]; // XLIFF file of imported document
    CHAR      szMetaData[MAX_LONGFILESPEC];    // metadata file of existing document
    CHAR      szImpMetaData[MAX_LONGFILESPEC]; // metadata file of imported document
    CHAR      szEntity[MAX_LONGFILESPEC];    // entity file of existing document
    CHAR      szImpEntity[MAX_LONGFILESPEC]; // entity file of imported document
  } PROCESSDOCDATA, *PPROCESSDOCDATA;
  PPROCESSDOCDATA pData = NULL;        // ptr to dynamic private data of function
  BOOL        fOK = TRUE;              // internal O.K. flag and function RC
  BOOL        fSourceRep = FALSE;      // TRUE if source document is replaced
  BOOL        fTargetRep = FALSE;      // TRUE if target document is replaced
  BOOL        fDocExists = FALSE;      // TRUE if document exists in target folder
  BOOL        fCopyProps = FALSE;      // TRUE if copy of property file required
  BOOL        fObjectNew = FALSE;      // TRUE if new document has been created
  BOOL        fImpSourceIsNewer = FALSE; // TRUE if imported source is newer
  BOOL        fOldVendorInfo;          // TRUE = vendor info of existing doc available
  PFOLIMPDOCDATA pDoc = pIda->pDocData;


  // AFC: save vendor information
  CHAR      szVendor[MAX_DESCRIPTION];       // Vendor Name (Translator)
  CHAR      szVendorEMail[MAX_DESCRIPTION];  // Vendor e-mail

  HPROP  hpropDocument;
  PPROPDOCUMENT ppropDocument;
  PSZ    pszReplace;
  ULONG  ErrorInfo = 0;
  ULONG  ulErrorInfo = 0;

  /********************************************************************/
  /* Allocate dynamic private data area                               */
  /********************************************************************/
  fOK = UtlAllocHwnd( (PVOID *)&pData, 0L, (ULONG)sizeof(PROCESSDOCDATA),
                      ERROR_STORAGE, pIda->hwndErrMsg );

  // process documents
  while ( fOK && (pDoc->pszDocObjName != NULL) )
  {
    PSZ    pszDocName;              // ptr to document name

    // initiliaze vendor fields
    szVendor[0] = EOS;
    szVendorEMail[0] = EOS;
    fOldVendorInfo = FALSE;

    // get position of document name within document object name
    pszDocName = UtlGetFnameFromPath( pDoc->pszDocObjName );


    // for new documents with long names: get an unique short name or use short name from imported
    // folder if not in use already
    if ( pDoc->fIsNew && (pDoc->pszLongName != NULL) )
    {
      // setup document property file name using old short name
      UtlMakeEQFPath( pData->szProp, pIda->chToDrive,
                      PROPERTY_PATH, pIda->szTargetFolder );
      strcat( pData->szProp, BACKSLASH_STR );
      strcat( pData->szProp, pszDocName );

      // if property file exists ...
      if ( UtlFileExist( pData->szProp ) )
      {
        // .. create new unique short name
        FolLongToShortDocName( pIda->szTargetFolObj, pDoc->pszLongName,
                               pDoc->szTargetShortName, &(pDoc->fIsNew) );
      }
      else
      {
        // ...use short name from imported document
        strcpy( pDoc->szTargetShortName, pszDocName );
      } /* endif */
    } /* endif */

    // initialize process flags
    fSourceRep = FALSE;
    fTargetRep = FALSE;
    fDocExists = FALSE;
    fCopyProps = FALSE;
    fObjectNew = FALSE;
    fImpSourceIsNewer = FALSE;

    // Setup path names for imported and existing document

    // document object name
    UtlMakeEQFPath( pData->szDocObj, pIda->chToDrive,
                    SYSTEM_PATH, pIda->szTargetFolder );
    strcat( pData->szDocObj, BACKSLASH_STR );
    strcat( pData->szDocObj, pDoc->szTargetShortName );


    // existing document property file
    UtlMakeEQFPath( pData->szProp, pIda->chToDrive,
                    PROPERTY_PATH, pIda->szTargetFolder );
    strcat( pData->szProp, BACKSLASH_STR );
    strcat( pData->szProp, pDoc->szTargetShortName );


    //
    // AFC: Open document properties
    //      save vendor information

    if (UtlFileExist( pData->szProp ))
    {

      if ((hpropDocument =
            OpenProperties (pData->szDocObj, NULL,
                            PROP_ACCESS_READ, &ulErrorInfo))== NULL)
      {

        strcpy(szVendor, "");
        strcpy(szVendorEMail, "");

        // display error
        pszReplace = pData->szDocObj;
        UtlError( ERROR_OPEN_PROPERTIES, MB_CANCEL,
                  1, &pszReplace, EQF_ERROR);
      }
      else
      {

        ppropDocument = (PPROPDOCUMENT)MakePropPtrFromHnd( hpropDocument );

        strcpy(szVendor, ppropDocument->szVendor);
        strcpy(szVendorEMail, ppropDocument->szVendorEMail);
        fOldVendorInfo = TRUE;

        CloseProperties(hpropDocument , PROP_QUIT, &ErrorInfo );

      }//end if

    }
    else
    {
      strcpy(szVendor, "");
      strcpy(szVendorEMail, "");
    }//end if


    // existing source document
    UtlMakeEQFPath( pData->szSource, pIda->chToDrive,
                    DIRSOURCEDOC_PATH, pIda->szTargetFolder );
    strcat( pData->szSource, BACKSLASH_STR );
    strcat( pData->szSource, pDoc->szTargetShortName );

    // existing segmented source document
    UtlMakeEQFPath( pData->szSegSource, pIda->chToDrive,
                    DIRSEGSOURCEDOC_PATH, pIda->szTargetFolder );
    strcat( pData->szSegSource, BACKSLASH_STR );
    strcat( pData->szSegSource, pDoc->szTargetShortName );

    // existing segmented target document
    UtlMakeEQFPath( pData->szSegTarget, pIda->chToDrive,
                    DIRSEGTARGETDOC_PATH, pIda->szTargetFolder );
    strcat( pData->szSegTarget, BACKSLASH_STR );
    strcat( pData->szSegTarget, pDoc->szTargetShortName );

    // existing target document
    UtlMakeEQFPath( pData->szTarget, pIda->chToDrive,
                    DIRTARGETDOC_PATH, pIda->szTargetFolder );
    strcat( pData->szTarget, BACKSLASH_STR );
    strcat( pData->szTarget, pDoc->szTargetShortName );

    // existing RTF source document
    UtlMakeEQFPath( pData->szRTF, pIda->chToDrive,
                    DIRSEGRTF_PATH, pIda->szTargetFolder );
    strcat( pData->szRTF, BACKSLASH_STR );
    strcat( pData->szRTF, pDoc->szTargetShortName );

    // existing EA data
    UtlMakeEQFPath( pData->szEAData, pIda->chToDrive,
                    EADATA_PATH, pIda->szTargetFolder );
    strcat( pData->szEAData, BACKSLASH_STR );
    strcat( pData->szEAData, pDoc->szTargetShortName );

    // existing MT Log file
    UtlMakeEQFPath( pData->szMTLog, pIda->chToDrive,
                    MTLOG_PATH, pIda->szTargetFolder );
    strcat( pData->szMTLog, BACKSLASH_STR );
    strcat( pData->szMTLog, pDoc->szTargetShortName );

    // existing MISC file
    UtlMakeEQFPath( pData->szMisc, pIda->chToDrive,
                    MISC_PATH, pIda->szTargetFolder );
    strcat( pData->szMisc, BACKSLASH_STR );
    strcat( pData->szMisc, pDoc->szTargetShortName );

    // existing ENTITY file
    UtlMakeEQFPath( pData->szEntity, pIda->chToDrive,
                    ENTITY_PATH, pIda->szTargetFolder );
    strcat( pData->szEntity, BACKSLASH_STR );
    strcat( pData->szEntity, pDoc->szTargetShortName );

    // existing XLIFF document
    UtlMakeEQFPath( pData->szXLIFF, pIda->chToDrive, XLIFF_PATH, pIda->szTargetFolder );
    strcat( pData->szXLIFF, BACKSLASH_STR );
    strcat( pData->szXLIFF, pDoc->szTargetShortName );

    // existing METADATA
    UtlMakeEQFPath( pData->szMetaData, pIda->chToDrive,
                    METADATA_PATH, pIda->szTargetFolder );
    strcat( pData->szMetaData, BACKSLASH_STR );
    strcat( pData->szMetaData, pDoc->szTargetShortName );


    // imported document property file
    UtlMakeEQFPath( pData->szImpProp, pIda->chToDrive,
                    PROPERTY_PATH, pIda->szSourceFolder );
    strcat( pData->szImpProp, BACKSLASH_STR );
    strcat( pData->szImpProp, pszDocName );
    UtlAddImportToPath( pIda, pData->szImpProp );

    // imported source document
    UtlMakeEQFPath( pData->szImpSource, pIda->chToDrive,
                    DIRSOURCEDOC_PATH, pIda->szSourceFolder );
    strcat( pData->szImpSource, BACKSLASH_STR );
    strcat( pData->szImpSource, pszDocName );
    UtlAddImportToPath( pIda, pData->szImpSource );

#ifdef SESSIONLOG
    UtlLogWriteString2( "ImportFolder: Importing document %s to %s", pData->szImpSource, pData->szSource );
#endif


    // imported segmented source document
    UtlMakeEQFPath( pData->szImpSegSource, pIda->chToDrive,
                    DIRSEGSOURCEDOC_PATH, pIda->szSourceFolder );
    strcat( pData->szImpSegSource, BACKSLASH_STR );
    strcat( pData->szImpSegSource, pszDocName );
    UtlAddImportToPath( pIda, pData->szImpSegSource );

    // imported segmented target document
    UtlMakeEQFPath( pData->szImpSegTarget, pIda->chToDrive,
                    DIRSEGTARGETDOC_PATH, pIda->szSourceFolder );
    strcat( pData->szImpSegTarget, BACKSLASH_STR );
    strcat( pData->szImpSegTarget, pszDocName );
    UtlAddImportToPath( pIda, pData->szImpSegTarget );



    // imported target document
    UtlMakeEQFPath( pData->szImpTarget, pIda->chToDrive,
                    DIRTARGETDOC_PATH, pIda->szSourceFolder );
    strcat( pData->szImpTarget, BACKSLASH_STR );
    strcat( pData->szImpTarget, pszDocName );
    UtlAddImportToPath( pIda, pData->szImpTarget );

    // imported RTF source document
    UtlMakeEQFPath( pData->szImpRTF, pIda->chToDrive,
                    DIRSEGRTF_PATH, pIda->szSourceFolder );
    strcat( pData->szImpRTF, BACKSLASH_STR );
    strcat( pData->szImpRTF, pszDocName );
    UtlAddImportToPath( pIda, pData->szImpRTF );

    // imported EA data
    UtlMakeEQFPath( pData->szImpEAData, pIda->chToDrive,
                    EADATA_PATH, pIda->szSourceFolder );
    strcat( pData->szImpEAData, BACKSLASH_STR );
    strcat( pData->szImpEAData, pszDocName );
    UtlAddImportToPath( pIda, pData->szImpEAData );

    // imported MT log file
    UtlMakeEQFPath( pData->szImpMTLog, pIda->chToDrive,
                    MTLOG_PATH, pIda->szSourceFolder );
    strcat( pData->szImpMTLog, BACKSLASH_STR );
    strcat( pData->szImpMTLog, pszDocName );
    UtlAddImportToPath( pIda, pData->szImpMTLog );

    // imported Misc file
    UtlMakeEQFPath( pData->szImpMisc, pIda->chToDrive, MISC_PATH, pIda->szSourceFolder );
    strcat( pData->szImpMisc, BACKSLASH_STR );
    strcat( pData->szImpMisc, pszDocName );
    UtlAddImportToPath( pIda, pData->szImpMisc );

    // imported entity file
    UtlMakeEQFPath( pData->szImpEntity, pIda->chToDrive, ENTITY_PATH, pIda->szSourceFolder );
    strcat( pData->szImpEntity, BACKSLASH_STR );
    strcat( pData->szImpEntity, pszDocName );
    UtlAddImportToPath( pIda, pData->szImpEntity );

    // imported XLIFF file
    UtlMakeEQFPath( pData->szImpXLIFF, pIda->chToDrive, XLIFF_PATH, pIda->szSourceFolder );
    strcat( pData->szImpXLIFF, BACKSLASH_STR );
    strcat( pData->szImpXLIFF, pszDocName );
    UtlAddImportToPath( pIda, pData->szImpXLIFF );

    // imported METADATA file
    UtlMakeEQFPath( pData->szImpMetaData, pIda->chToDrive, METADATA_PATH, pIda->szSourceFolder );
    strcat( pData->szImpMetaData, BACKSLASH_STR );
    strcat( pData->szImpMetaData, pszDocName );
    UtlAddImportToPath( pIda, pData->szImpMetaData );

    // Check if the document exists already in the target folder
    fDocExists = UtlFileExist( pData->szProp );

    // process document using the selected disposition
    switch ( pDoc->bDisposition )
    {
      case COPYSRCTGT_DOCDISP :
      case COPYSRC_DOCDISP :
      case COPYSRCDELTGT_DOCDISP :
        // delete all files of existing document
        UtlDelete( pData->szProp, 0L, FALSE );
        UtlDelete( pData->szSource, 0L, FALSE );
        UtlDelete( pData->szSegSource, 0L, FALSE );
        UtlDelete( pData->szSegTarget, 0L, FALSE );
        UtlDelete( pData->szTarget, 0L, FALSE );
        UtlDelete( pData->szRTF, 0L, FALSE );
        UtlDelete( pData->szEAData, 0L, FALSE );
        UtlDelete( pData->szMisc, 0L, FALSE );
        UtlDelete( pData->szXLIFF, 0L, FALSE );
        UtlDelete( pData->szMetaData, 0L, FALSE );
        UtlDelete( pData->szEntity, 0L, FALSE );


       // MT logs are not deleted anymore but a "new shipment" identifier is appended to the MTLOG file
        if ( UtlFileExist( pData->szMTLog ) )
        {
          FILE *hMTLog = fopen( pData->szMTLog, "ab" );
          if ( hMTLog != NULL  )
          {
            ACTSEGLOGENH2 SegLog;
            memset( &SegLog, 0, sizeof(SegLog) );
            SegLog.ulEyeCatcher = ACTSEGLOG2_EYECATCHER;
            SegLog.AddFlags.ShipmentRec = TRUE;
            WriteToMTLog( hMTLog, &SegLog, NULL, NULL, NULL, NULL );
            fclose( hMTLog );          
          } /* endif */       
        } /* endif */

		
        if ( fDocExists )
        {
          fObjectNew = FALSE;
        } /* endif */

        // move property file
        if ( UtlFileExist( pData->szImpProp ) )
        {
          UtlSmartMoveHwnd( pData->szImpProp, pData->szProp, TRUE , pIda->hwndErrMsg );
        } /* endif */
        //
        // AFC: merge document property file
        //

        // Note: Access to property file using the property handler will fail as the data
        // in the property head has not been adjusted to the new location, so we use
        // UtlLoadFile / UtlWriteFile instead
        {
          PPROPDOCUMENT pProp = NULL;
          ULONG ulLen;

          if ( UtlLoadFileL( pData->szProp, (PVOID *)&pProp, &ulLen, FALSE, FALSE ) )
          {
            if ( fOldVendorInfo )
            {
              strcpy( pProp->szVendor, szVendor);
              strcpy( pProp->szVendorEMail, szVendorEMail);
            } /* endif */

            // change parent folder ID if subfolder IDs had to be changed
            {
              int i = 0;
              while ( pIda->pImpSubFolInfo[i].szName[0] != EOS )
              {
                if ( pProp->ulParentFolder == pIda->pImpSubFolInfo[i].ulID )
                {
                  if ( pIda->pImpSubFolInfo[i].ulValue != 0L )
                  {
                    pProp->ulParentFolder = pIda->pImpSubFolInfo[i].ulValue;
                  } /* endif */
                  break;
                } /* endif */
                i++;
              } /* endwhile */
            }

            // adjust markup if markup of target folder differs from imported folder
            // and no markup has been specified on document level
            if ( (pProp->szFormat[0] == EOS) &&
                 (strcmp( pIda->szImpFormat, pIda->szFolderFormat ) != 0) )
            {
              strcpy( pProp->szFormat, pIda->szImpFormat );
            } /* endif */

            // adjust source language if source language of target folder differs from imported folder
            // and no source language has been specified on document level
            if ( (pProp->szSourceLang[0] == EOS) &&
                 (strcmp( pIda->szImpSourceLang, pIda->szFolderSourceLang ) != 0) )
            {
              strcpy( pProp->szSourceLang, pIda->szImpSourceLang );
            } /* endif */

            // adjust target language if target language of target folder differs from imported folder
            // and no target language has been specified on document level
            if ( (pProp->szTargetLang[0] == EOS) &&
                 (strcmp( pIda->szImpTargetLang, pIda->szFolderTargetLang ) != 0) )
            {
              strcpy( pProp->szTargetLang, pIda->szImpTargetLang );
            } /* endif */

            UtlWriteFileL( pData->szProp, ulLen, pProp, FALSE );

            UtlAlloc( (PVOID *)&pProp, 0L, 0L, NOMSG );
          }//end if
        }


        // move source
        if ( UtlFileExist( pData->szImpSource ) )
        {
          fSourceRep = TRUE;
          UtlSmartMoveHwnd( pData->szImpSource, pData->szSource, TRUE , pIda->hwndErrMsg );
        } /* endif */

        // move segmented source
        if ( UtlFileExist( pData->szImpSegSource ) )
        {
          fSourceRep = TRUE;
          UtlSmartMoveHwnd( pData->szImpSegSource, pData->szSegSource, TRUE , pIda->hwndErrMsg );
        } /* endif */

        // move segmented target
        if ( UtlFileExist( pData->szImpSegTarget ) )
        {
          fTargetRep = TRUE;
          UtlSmartMoveHwnd( pData->szImpSegTarget, pData->szSegTarget, TRUE , pIda->hwndErrMsg );
        } /* endif */

        // move target
        if ( UtlFileExist( pData->szImpTarget ) )
        {
          UtlSmartMoveHwnd( pData->szImpTarget, pData->szTarget, TRUE , pIda->hwndErrMsg );
        } /* endif */

        // move RTF source
        if ( UtlFileExist( pData->szImpRTF ) )
        {
          UtlSmartMoveHwnd( pData->szImpRTF, pData->szRTF, TRUE , pIda->hwndErrMsg );
        } /* endif */

        // move EA data
        if ( UtlFileExist( pData->szImpEAData ) )
        {
          // create EA data directory if necessary
          strcpy( pData->szDirBuffer, pData->szEAData );
          UtlSplitFnameFromPath( pData->szDirBuffer );
          if ( !UtlDirExist( pData->szDirBuffer ) )
          {
            UtlMkDir( pData->szDirBuffer, 0, FALSE );
          } /* endif */

          UtlSmartMoveHwnd( pData->szImpEAData, pData->szEAData,
                            TRUE , pIda->hwndErrMsg );
        } /* endif */

        // copy/merge MT log file(s)
        if ( UtlFileExist( pData->szImpMTLog ) )
        {
          // create MTLOG directory if necessary
          strcpy( pData->szDirBuffer, pData->szMTLog );
          UtlSplitFnameFromPath( pData->szDirBuffer );
          if ( !UtlDirExist( pData->szDirBuffer ) )
          {
            UtlMkDir( pData->szDirBuffer, 0, FALSE );
          } /* endif */

          if ( UtlFileExist( pData->szMTLog ) )
          {
            FolMergeMTLog( pData->szImpMTLog, pData->szMTLog  );
          }
          else
          {
            UtlSmartMoveHwnd( pData->szImpMTLog, pData->szMTLog, TRUE , pIda->hwndErrMsg );
          } /* endif */             
        } /* endif */

        // move Misc file
        if ( UtlFileExist( pData->szImpMisc ) )
        {
          // create MISC directory if necessary
          strcpy( pData->szDirBuffer, pData->szMisc );
          UtlSplitFnameFromPath( pData->szDirBuffer );
          if ( !UtlDirExist( pData->szDirBuffer ) )
          {
            UtlMkDir( pData->szDirBuffer, 0, FALSE );
          } /* endif */

          UtlSmartMoveHwnd( pData->szImpMisc, pData->szMisc, TRUE , pIda->hwndErrMsg );
        } /* endif */

        // move entity file
        if ( UtlFileExist( pData->szImpEntity ) )
        {
          // create entity directory if necessary
          strcpy( pData->szDirBuffer, pData->szEntity );
          UtlSplitFnameFromPath( pData->szDirBuffer );
          if ( !UtlDirExist( pData->szDirBuffer ) )
          {
            UtlMkDir( pData->szDirBuffer, 0, FALSE );
          } /* endif */

          UtlSmartMoveHwnd( pData->szImpEntity, pData->szEntity, TRUE , pIda->hwndErrMsg );
        } /* endif */

        // move XLIFF file
        if ( UtlFileExist( pData->szImpXLIFF ) )
        {
          // create MISC directory if necessary
          strcpy( pData->szDirBuffer, pData->szXLIFF );
          UtlSplitFnameFromPath( pData->szDirBuffer );
          if ( !UtlDirExist( pData->szDirBuffer ) )
          {
            UtlMkDir( pData->szDirBuffer, 0, FALSE );
          } /* endif */

          UtlSmartMoveHwnd( pData->szImpXLIFF, pData->szXLIFF, TRUE , pIda->hwndErrMsg );
        } /* endif */

        // move Metadata file
        if ( UtlFileExist( pData->szImpMetaData ) )
        {
          // create MISC directory if necessary
          strcpy( pData->szDirBuffer, pData->szMetaData );
          UtlSplitFnameFromPath( pData->szDirBuffer );
          if ( !UtlDirExist( pData->szDirBuffer ) )
          {
            UtlMkDir( pData->szDirBuffer, 0, FALSE );
          } /* endif */

          UtlSmartMoveHwnd( pData->szImpMetaData, pData->szMetaData, TRUE , pIda->hwndErrMsg );
        } /* endif */


        // correct PROPHEAD part in property file
        strcpy( pData->szTempPath, pData->szProp );
        UtlSplitFnameFromPath( pData->szTempPath ); // get rid off document name
        UtlSplitFnameFromPath( pData->szTempPath ); // get rid off subdirectory
        fOK = FolCorrectPropHead( pData->szProp,
                                  NULC,
                                  pData->szTempPath,
                                  pDoc->szTargetShortName,
                                  pIda->hwndErrMsg );
        break;

      case COPYTGT_DOCDISP :
        // delete all files of existing document
        UtlDelete( pData->szSegSource, 0L, FALSE );
        UtlDelete( pData->szSegTarget, 0L, FALSE );
        UtlDelete( pData->szTarget, 0L, FALSE );
        UtlDelete( pData->szRTF, 0L, FALSE );
        UtlDelete( pData->szProp, 0L, FALSE );
        UtlDelete( pData->szMTLog, 0L, FALSE );
        UtlDelete( pData->szMisc, 0L, FALSE );
        UtlDelete( pData->szXLIFF, 0L, FALSE );
        UtlDelete( pData->szMetaData, 0L, FALSE );

        fObjectNew = FALSE;

        // move segmented source (in order to keep segmented source and target
        // in sync)
        if ( UtlFileExist( pData->szImpSegSource ) )
        {
          fSourceRep = TRUE;
          UtlSmartMoveHwnd( pData->szImpSegSource, pData->szSegSource,
                            TRUE , pIda->hwndErrMsg );
        } /* endif */

        // move segmented target
        if ( UtlFileExist( pData->szImpSegTarget ) )
        {
          fTargetRep = TRUE;
          UtlSmartMoveHwnd( pData->szImpSegTarget, pData->szSegTarget,
                            TRUE , pIda->hwndErrMsg );
        } /* endif */

        // move target
        if ( UtlFileExist( pData->szImpTarget ) )
        {
          UtlSmartMoveHwnd( pData->szImpTarget, pData->szTarget,
                            TRUE , pIda->hwndErrMsg );
        } /* endif */

        // move RTF source
        if ( UtlFileExist( pData->szImpRTF ) )
        {
          UtlSmartMoveHwnd( pData->szImpRTF, pData->szRTF,
                            TRUE , pIda->hwndErrMsg );
        } /* endif */

        // move EA data
        if ( UtlFileExist( pData->szImpEAData ) )
        {
          UtlSmartMoveHwnd( pData->szImpEAData, pData->szEAData,
                            TRUE , pIda->hwndErrMsg );
        } /* endif */

        // move or merge MT log file
        if ( UtlFileExist( pData->szImpMTLog ) )
        {
          // create MTLOG directory if necessary
          strcpy( pData->szDirBuffer, pData->szMTLog );
          UtlSplitFnameFromPath( pData->szDirBuffer );
          if ( !UtlDirExist( pData->szDirBuffer ) )
          {
            UtlMkDir( pData->szDirBuffer, 0, FALSE );
          } /* endif */

          if ( UtlFileExist( pData->szMTLog ) )
          {
            FolMergeMTLog( pData->szImpMTLog, pData->szMTLog  );
          }
          else
          {
            UtlSmartMoveHwnd( pData->szImpMTLog, pData->szMTLog, TRUE , pIda->hwndErrMsg );
          } /* endif */             

        } /* endif */

        // move Misc file
        if ( UtlFileExist( pData->szImpMisc ) )
        {
          // create MISC directory if necessary
          strcpy( pData->szDirBuffer, pData->szMisc );
          UtlSplitFnameFromPath( pData->szDirBuffer );
          if ( !UtlDirExist( pData->szDirBuffer ) )
          {
            UtlMkDir( pData->szDirBuffer, 0, FALSE );
          } /* endif */

          UtlSmartMoveHwnd( pData->szImpMisc, pData->szMisc,
                            TRUE , pIda->hwndErrMsg );
        } /* endif */

        // move XLIFF file
        if ( UtlFileExist( pData->szImpXLIFF ) )
        {
          // create MISC directory if necessary
          strcpy( pData->szDirBuffer, pData->szXLIFF );
          UtlSplitFnameFromPath( pData->szDirBuffer );
          if ( !UtlDirExist( pData->szDirBuffer ) )
          {
            UtlMkDir( pData->szDirBuffer, 0, FALSE );
          } /* endif */

          UtlSmartMoveHwnd( pData->szImpXLIFF, pData->szXLIFF,
                            TRUE , pIda->hwndErrMsg );
        } /* endif */

        // move Metadata file
        if ( UtlFileExist( pData->szImpMetaData ) )
        {
          // create MISC directory if necessary
          strcpy( pData->szDirBuffer, pData->szMetaData );
          UtlSplitFnameFromPath( pData->szDirBuffer );
          if ( !UtlDirExist( pData->szDirBuffer ) )
          {
            UtlMkDir( pData->szDirBuffer, 0, FALSE );
          } /* endif */

          UtlSmartMoveHwnd( pData->szImpMetaData, pData->szMetaData, TRUE , pIda->hwndErrMsg );
        } /* endif */

        // move property file
        if ( UtlFileExist( pData->szImpProp ) )
        {
          UtlSmartMoveHwnd( pData->szImpProp, pData->szProp,
                            TRUE , pIda->hwndErrMsg );
        } /* endif */


        //
        // AFC: merge document property file
        //
        {
          PPROPDOCUMENT pProp = NULL;
          ULONG ulLen;

          if ( UtlLoadFileL( pData->szProp, (PVOID *)&pProp, &ulLen, FALSE, FALSE ) )
          {
            if ( fOldVendorInfo )
            {
              strcpy( pProp->szVendor, szVendor);
              strcpy( pProp->szVendorEMail, szVendorEMail);
            } /* endif */

            // change parent folder ID if subfolder IDs had to be changed
            {
              int i = 0;
              while ( pIda->pImpSubFolInfo[i].szName[0] != EOS )
              {
                if ( pProp->ulParentFolder == pIda->pImpSubFolInfo[i].ulID )
                {
                  if ( pIda->pImpSubFolInfo[i].ulValue != 0L )
                  {
                    pProp->ulParentFolder = pIda->pImpSubFolInfo[i].ulValue;
                  } /* endif */
                  break;
                } /* endif */
                i++;
              } /* endwhile */
            }

            // adjust markup if markup of target folder differs from imported folder
            // and no markup has been specified on document level
            if ( (pProp->szFormat[0] == EOS) &&
                 (strcmp( pIda->szImpFormat, pIda->szOrgFormat ) != 0) )
            {
              strcpy( pProp->szFormat, pIda->szImpFormat );
            } /* endif */

            // adjust source language if source language of target folder differs from imported folder
            // and no source language has been specified on document level
            if ( (pProp->szSourceLang[0] == EOS) &&
                 (strcmp( pIda->szImpSourceLang, pIda->szFolderSourceLang ) != 0) )
            {
              strcpy( pProp->szSourceLang, pIda->szImpSourceLang );
            } /* endif */

            // adjust target language if target language of target folder differs from imported folder
            // and no target language has been specified on document level
            if ( (pProp->szTargetLang[0] == EOS) &&
                 (strcmp( pIda->szImpTargetLang, pIda->szFolderTargetLang ) != 0) )
            {
              strcpy( pProp->szTargetLang, pIda->szImpTargetLang );
            } /* endif */

            UtlWriteFileL( pData->szProp, ulLen, pProp, FALSE );

            UtlAlloc( (PVOID *)&pProp, 0L, 0L, NOMSG );
          }//end if
        }

        // correct PROPHEAD part in property file
        strcpy( pData->szTempPath, pData->szProp );
        UtlSplitFnameFromPath( pData->szTempPath ); // get rid off document name
        UtlSplitFnameFromPath( pData->szTempPath ); // get rid off subdirectory
        fOK = FolCorrectPropHead( pData->szProp,
                                  NULC,
                                  pData->szTempPath,
                                  pDoc->szTargetShortName,
                                  pIda->hwndErrMsg );
        break;

      case IGNORE_DOCDISP :
        // delete imported files
        UtlDelete( pData->szImpProp, 0L, FALSE );
        UtlDelete( pData->szImpSource, 0L, FALSE );
        UtlDelete( pData->szImpSegSource, 0L, FALSE );
        UtlDelete( pData->szImpSegTarget, 0L, FALSE );
        UtlDelete( pData->szImpTarget, 0L, FALSE );
        UtlDelete( pData->szImpRTF, 0L, FALSE );
        UtlDelete( pData->szImpEAData, 0L, FALSE );
        UtlDelete( pData->szImpMTLog, 0L, FALSE );
        UtlDelete( pData->szImpMisc, 0L, FALSE );
        break;
    } /* endswitch */

    /******************************************************************/
    /* Post new or change notification message                        */
    /******************************************************************/
    if ( fOK && (pDoc->bDisposition != IGNORE_DOCDISP) )
    {
      if ( !fDocExists )
      {
        EqfSend2AllHandlers( WM_EQFN_CREATED,
                             MP1FROMSHORT( clsDOCUMENT ),
                             MP2FROMP( pData->szDocObj ) );
      }
      else
      {
        EqfSend2AllHandlers( WM_EQFN_PROPERTIESCHANGED,
                             MP1FROMSHORT( PROP_CLASS_DOCUMENT ),
                             MP2FROMP( pData->szDocObj ) );
      } /* endif */
    } /* endif */

    if ( pDoc->bDisposition == IGNORE_DOCDISP )
    {
      if ( (pDoc->bSourceState == NEW_DOCSTAT) ||
           (pDoc->bSourceState == NEWER_DOCSTAT) )
      {
        pData->DocImpHist.sType              = NEWSOURCE_NOTIMPORTED_SUBTYPE;
        Utlstrccpy( pData->DocImpHist.szFolder, pIda->szTargetFolder,
                    DOT );
        UtlMakeEQFPath( pData->szTempPath, pIda->chToDrive,
                        SYSTEM_PATH, pIda->szTargetFolder );
        EQFBWriteHistLog2( pData->szTempPath,
                           pszDocName,
                           DOCIMPORT_LOGTASK,
                           sizeof(DOCIMPORTHIST),
                           (PVOID)&(pData->DocImpHist),
                           TRUE, pIda->hwndErrMsg, pDoc->pszLongName );
      } /* endif */
    }
    else
    {
      /*********************************************************/
      /* Add import record to history log                      */
      /*********************************************************/
      {
        if ( fOK )
        {
          pData->DocImpHist.sType              = FOLDER_SUBTYPE;
          pData->DocImpHist.fSourceDocReplaced = fSourceRep;
          pData->DocImpHist.fTargetDocReplaced = fTargetRep;
          Utlstrccpy( pData->DocImpHist.szFolder, pIda->szTargetFolder,
                      DOT );

          UtlMakeEQFPath( pData->szTempPath, pIda->chToDrive,
                          SYSTEM_PATH, pIda->szTargetFolder );
          EQFBWriteHistLog2( pData->szTempPath,
                             pszDocName,
                             DOCIMPORT_LOGTASK,
                             sizeof(DOCIMPORTHIST),
                             (PVOID)&(pData->DocImpHist),
                             TRUE, pIda->hwndErrMsg, pDoc->pszLongName );
        } /* endif */
      }

    } /* endif */

    // Create new target history log record if necessary
    if ( fTargetRep )
    {
      UtlMakeEQFPath( pData->szTempPath, pIda->chToDrive,
                      SYSTEM_PATH, pIda->szTargetFolder );
      FolWriteNewTargetRec( pData->szTempPath, pszDocName,
                            TRUE, pIda->hwndErrMsg );
    } /* endif */

    // continue with next document
    pDoc++;
    if ( pIda->usProcessedObjects <= pIda->usMaxObjects ) pIda->usProcessedObjects++;
  } /* endwhile */

  /********************************************************************/
  /* Cleanup                                                          */
  /********************************************************************/
  if (  pData )          UtlAlloc( (PVOID *)&pData, 0L, 0L, NOMSG );
  if (  pIda->pDocData ) UtlAlloc( (PVOID *)&(pIda->pDocData), 0L, 0L, NOMSG );
  if (  pIda->pPool )    PoolDestroy( pIda->pPool );

  /********************************************************************/
  /* Return to caller                                                 */
  /********************************************************************/
  return( fOK );
} /* end of function FolImpProcessDoc */


BOOL FolImpBuildObjList
(
PSZ       pszSearchPath,            // path to search for
PSZ       pszObjPath,               // path of created objects
PSZ       *ppszBuffer,              // buffer for resulting object list
PULONG    pulBufSize                // size of buffer in bytes
)
{
  BOOL      fOK = FALSE;              // TRUE = objects added to buffer
  FILEFINDBUF ResultBuf;              // DOS file find struct
  USHORT  usCount = 1;
  HDIR    hDirHandle = INVALID_HANDLE_VALUE; // DosFind routine handle
  USHORT  usRC = NO_ERROR;            // return code of Dos... alias Utl...
  PSZ     pszName = ResultBuf.cFileName;   // ptr to name in result buffer
  ULONG   ulBufUsed = 0L;             // used bytes in buffer
  ULONG ulLen;                        // length of data being added

  //--- fill-in handler name ---
  ulLen = strlen(FOLDERHANDLER) + 1;
  if ( (ulBufUsed + ulLen) >= *pulBufSize )
  {
    if ( !UtlAlloc( (PVOID *)ppszBuffer, *pulBufSize,
                    *pulBufSize + IMPBUFFERSIZE, NOMSG ) )
    {
      usRC = ERROR_NOT_ENOUGH_MEMORY;
    }
    else
    {
      *pulBufSize += IMPBUFFERSIZE;
    } /* endif */
  } /* endif */

  if ( usRC == NO_ERROR )
  {
    strcpy( *ppszBuffer, FOLDERHANDLER );
    strcat( *ppszBuffer, X15_STR );
    ulBufUsed += ulLen;
  } /* endif */

  //--- search objects ---
  if ( usRC == NO_ERROR )
  {
    hDirHandle = FindFirstFile( pszSearchPath, &ResultBuf );
    if ( hDirHandle == INVALID_HANDLE_VALUE )
    {
      usRC = (USHORT)GetLastError();
      usCount = 0;
    }
    else
    {
      usCount = 1;
    }
  } /* endif */

  while ( usCount && !usRC )
  {

    fOK = TRUE;

    if ( ResultBuf.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
    {
      // ignore directories
    }
    else
    {
      // enlarge buffer if necessary
      ulLen = strlen(pszObjPath) + 1 + strlen(pszName) + 1;
      if ( (ulBufUsed + ulLen) >= *pulBufSize )
      {
        if ( !UtlAlloc( (PVOID *)ppszBuffer, *pulBufSize,
                        *pulBufSize + IMPBUFFERSIZE, NOMSG ) )
        {
          usRC = ERROR_NOT_ENOUGH_MEMORY;
        }
        else
        {
          *pulBufSize += IMPBUFFERSIZE;
        } /* endif */
      } /* endif */

      // add object to buffer
      if ( usRC == NO_ERROR )
      {
        strcpy( *ppszBuffer + ulBufUsed, pszObjPath );
        strcat( *ppszBuffer + ulBufUsed, BACKSLASH_STR );
        strcat( *ppszBuffer + ulBufUsed, pszName );
        strcat( *ppszBuffer + ulBufUsed, X15_STR );
        ulBufUsed += ulLen;
      } /* endif */
    }
    // continue with next object
    if ( usRC == NO_ERROR )
    {
      if ( FindNextFile( hDirHandle, &ResultBuf ) )
      {
        usCount = 1;
      }
      else
      {
        usRC = (USHORT)GetLastError();
        usCount = 0;
      }
    } /* endif */
  } /* endwhile */

  // close search file handle
  if ( hDirHandle != INVALID_HANDLE_VALUE ) FindClose( hDirHandle );

  return( fOK );
} /* end of FolImpBuildObjList */

BOOL FolImpBuildObject
(
PSZ       pszSearchPath,            // path to search for
PSZ       pszObjPath,               // path of created objects
PSZ       pszBuffer,                // buffer for resulting object list
ULONG     ulBufSize,                // size of buffer in bytes
PSZ       pszObjName                // buffer for object name only
)
{
  BOOL      fOK = FALSE;              // TRUE = objects added to buffer
  FILEFINDBUF ResultBuf;              // DOS file find struct
  USHORT  usCount = 1;
  HDIR    hDirHandle = INVALID_HANDLE_VALUE;   // DosFind routine handle
  USHORT  usRC = 0;                    // return code of Dos... alias Utl...

  ulBufSize = ulBufSize;        // supress compiler warning

  //--- fill-in handler name ---
  strcpy( pszBuffer, FOLDERHANDLER );
  strcat( pszBuffer, X15_STR );

  //--- search first object of given type ---
  hDirHandle = FindFirstFile( pszSearchPath, &ResultBuf );
  if ( hDirHandle == INVALID_HANDLE_VALUE )
  {
    usRC = (USHORT)GetLastError();
    usCount = 0;
  }
  else
  {
    usCount = 1;
  }

  // skip any directory
  if ( usCount )
  {
    while ( (ResultBuf.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && (usCount == 1) )
    {
      if ( FindNextFile( hDirHandle, &ResultBuf ) )
      {
        usCount = 1;
      }
      else
      {
        usRC = (USHORT)GetLastError();
        usCount = 0;
      }
    }
  }

  if ( usCount && !usRC )
  {
    fOK = TRUE;
    strcpy( pszObjName, pszObjPath );
    strcat( pszObjName, BACKSLASH_STR );
    strcat( pszObjName, ResultBuf.cFileName );
    strcat( pszBuffer, pszObjName );
    strcat( pszBuffer, X15_STR );
  } /* endif */

  // close search file handle
  if ( hDirHandle != INVALID_HANDLE_VALUE ) FindClose( hDirHandle );


  return( fOK );
} /* end of FolImpBuildObject */




VOID FolMakeImportPath
(
PSZ      pszPath,                   // pointer to path buffer
CHAR     chDrive,                   // drive letter
USHORT   usPathID,                  // path identifier
PSZ      pszFolder                  // name of folder
)
{
  CHAR     szTempPath[MAX_EQF_PATH];  // temporary path name
  PSZ      pszTemp;

  UtlMakeEQFPath( pszPath, chDrive, IMPORT_PATH, NULL );

  UtlMakeEQFPath( szTempPath, chDrive, usPathID, pszFolder );

  pszTemp = strchr( szTempPath, BACKSLASH );    // skip drive letter
  if ( pszTemp )
  {
    pszTemp = strchr( pszTemp, BACKSLASH ); // skip EQF directory
  } /* endif */
  if ( pszTemp )
  {
    strcat( pszPath, pszTemp );              // append remaining path
  } /* endif */
} /* end of FolMakeImportPath  */

VOID UtlAddImportToPath( PFOLIMPIDA pIda, PSZ pszPath )
{
  PSZ pszImport;                      // ptr to position where IMPORT is inserted

  pszImport = strchr( pszPath, BACKSLASH );     // go to 1st backslash

  if ( pszImport )
  {
    pszImport = strchr( pszImport + 1, BACKSLASH ); // go to 2nd backslash
  } /* endif */

  if ( pszImport )
  {
    memmove( pszImport + pIda->ulImportDirLength, pszImport, strlen( pszImport ) + 1 );
    memcpy( pszImport, pIda->szImportDir, pIda->ulImportDirLength );
  } /* endif */
} /* end of UtlAddImportToPath */

VOID UtlRemoveImportFromPath( PFOLIMPIDA pIda, PSZ pszPath )
{
  PSZ pszImport;                      // ptr to position of IMPORTPATH

  pszImport = strstr( pszPath, pIda->szImportDir );
  if (pszImport )
  {
    memmove( pszImport, pszImport + pIda->ulImportDirLength, strlen(pszImport) - pIda->ulImportDirLength + 1 );
  } /* endif */
} /* endof UtlRemovemportFromPath */


//------------------------------------------------------------------------------
// Internal function                                                            
//------------------------------------------------------------------------------
// Function name:     FolImpInfoPrintDetails                                    
//------------------------------------------------------------------------------
// Function call:     FolImpInfoPrintDetails( PFOLIMPIDA  pIda );               
//------------------------------------------------------------------------------
// Description:       Prints the folder details using UtlPrint functions.       
//------------------------------------------------------------------------------
// Input parameter:   PFOLIMPIDA   pIda     pointer to folder import IDA        
//------------------------------------------------------------------------------
// Returncode type:   VOID                                                      
//------------------------------------------------------------------------------
// Prerequesits:      IDA must have been filled in one of the previous dialogs. 
//------------------------------------------------------------------------------
// Side effects:      none                                                      
//------------------------------------------------------------------------------
// Function flow:     switch to hourglass pointer                               
//                    get pinter to file list of package                        
//                    open print device                                         
//                    print heading                                             
//                    print folder name                                         
//                    print folder description                                  
//                    print date and time of export                             
//                    print translation memory name                             
//                    print markup name                                         
//                    print list of documents                                   
//                    print list of dictionaries                                
//                    print attached note                                       
//                    close print device                                        
//                    switch to norma (arrow) pointer                           
//------------------------------------------------------------------------------
VOID FolImpInfoPrintDetails
(
PFOLIMPIDA  pIda,                   // ptr to folder import IDA
HWND        hwndDlg                 // handle of dialog
)
{
  HPRINT        hPrint = NULLHANDLE;   // print handle
  BOOL            fOK;                 // internal OK flag
  PFILELIST       pFileList;           // ptr to package's file list
  USHORT          usNoOfEntries;       // # of enteries in package's file list
  PFILELISTENTRY  pFile;               // ptr for file list processing
  LONG            lDateTime= 0;        // buffer for date/time value
  ULONG           ulInsertPos;         // insertion position in buffer
  CHAR            szName[MAX_FILESPEC];// buffer for document and dictionary name
  PSZ             pszItem = NULL;      // points to currently printed item
  CHAR            szLF[2];             // string containing linefeed character
  HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);

  /********************************************************************/
  /* Prepare linefeed string                                          */
  /********************************************************************/
  szLF[0] = LF;
  szLF[1] = EOS;

  /********************************************************************/
  /* Switch to hourglass pointer                                      */
  /********************************************************************/
  SETCURSOR( SPTR_WAIT );

  /********************************************************************/
  /* Get pointer to package file list                                 */
  /********************************************************************/
  UtlPackGetFileListPtr( pIda->pPackCtrl, &pFileList );

  /********************************************************************/
  /* open print device                                                */
  /********************************************************************/
  {
    ULONG  ulLen;                  // length of generated line
    PSZ    pszName;                  // ptr to folder name
    CHAR   chPrintLine[80];          // buffer for print job title

    LOADSTRINGLEN( (HAB)UtlQueryULong( QL_HAB ), hResMod,
                   SID_FOLDETAILS_PRINT_PRTJOB, pIda->pszBuffer,
                   pIda->ulBufSize );
    pszName  = pIda->szSourceFolName;
    DosInsMessage( &pszName, 1, pIda->pszBuffer, strlen(pIda->pszBuffer),
                   chPrintLine, (SHORT)(sizeof(chPrintLine) - 1),
                   &ulLen );
    chPrintLine[ulLen] = EOS;
    fOK = UtlPrintOpen( &hPrint, chPrintLine, hwndDlg );
  }

  /********************************************************************/
  /* print heading                                                    */
  /********************************************************************/
  if ( fOK )
  {
    LOADSTRINGLEN( NULLHANDLE, hResMod, SID_FOLDETAILS_PRINT_TITLE,
                   pIda->pszBuffer, pIda->ulBufSize );
    ANSITOOEM( pIda->pszBuffer );
    strcat( pIda->pszBuffer, szLF );
    fOK = UtlPrintLine( hPrint, pIda->pszBuffer );
    if ( fOK )
    {
      fOK = UtlPrintLine( hPrint, szLF );
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* print folder name                                                */
  /********************************************************************/
  if ( fOK )
  {
    LOADSTRINGLEN( NULLHANDLE, hResMod, SID_FOLDETAILS_PRINT_NAME,
                   pIda->pszBuffer, pIda->ulBufSize );
    ANSITOOEM( pIda->pszBuffer );
    strcat( pIda->pszBuffer, pIda->szSourceFolName );
    strcat( pIda->pszBuffer, szLF );
    fOK = UtlPrintLine( hPrint, pIda->pszBuffer );
  } /* endif */

  /********************************************************************/
  /* print folder description                                         */
  /********************************************************************/
  if ( fOK )
  {
    LOADSTRINGLEN( NULLHANDLE, hResMod, SID_FOLDETAILS_PRINT_DESCR,
                   pIda->pszBuffer, pIda->ulBufSize );
    ANSITOOEM( pIda->pszBuffer );
    switch ( pIda->pHeader->BitFlags.fHeaderType )
    {
      case RELEASE0_HEADER :
        strcat( pIda->pszBuffer,
                pIda->pHeader->Head.Rel0.szDescription );
        break;
      case RELEASE1_HEADER:
        strcat( pIda->pszBuffer,
                pIda->pHeader->Head.Rel1.szDescription );
        break;
    } /* endswitch */
    strcat( pIda->pszBuffer, szLF );
    fOK = UtlPrintLine( hPrint, pIda->pszBuffer );
  } /* endif */

  /********************************************************************/
  /* print date and time of export                                    */
  /********************************************************************/
  if ( fOK )
  {
    LOADSTRINGLEN( NULLHANDLE, hResMod, SID_FOLDETAILS_PRINT_DATE,
                   pIda->pszBuffer, pIda->ulBufSize );
    ANSITOOEM( pIda->pszBuffer );
    switch ( pIda->pHeader->BitFlags.fHeaderType )
    {
      case RELEASE0_HEADER:
        lDateTime = pIda->pHeader->Head.Rel0.lDateTime;
        break;
      case RELEASE1_HEADER:
        lDateTime = pIda->pHeader->Head.Rel1.lDateTime;
        break;
    } /* endswitch */

    LONGTODATETIMELEN( lDateTime,
                       (pIda->pszBuffer + strlen(pIda->pszBuffer)),
                       ((pIda->ulBufSize - strlen(pIda->pszBuffer))) );
    strcat( pIda->pszBuffer, szLF );
    fOK = UtlPrintLine( hPrint, pIda->pszBuffer );
  } /* endif */

  /********************************************************************/
  /* print translation memory names                                   */
  /********************************************************************/
  if ( fOK )
  {
    LOADSTRINGLEN( NULLHANDLE, hResMod, SID_FOLDETAILS_PRINT_TRANSLMEM,
                   pIda->pszBuffer, pIda->ulBufSize );
    ANSITOOEM( pIda->pszBuffer );
    ulInsertPos = strlen(pIda->pszBuffer);
    switch ( pIda->pHeader->BitFlags.fHeaderType )
    {
      case RELEASE0_HEADER:
        // get memory file name from memory name variable in header
        strcat( pIda->pszBuffer,
                pIda->pHeader->Head.Rel0.szMemory );
        strcat( pIda->pszBuffer, szLF );
        fOK = UtlPrintLine( hPrint, pIda->pszBuffer );
        break;
      case RELEASE1_HEADER:
        // get memory file name from list of packaged files
        if ( pFileList )
        {
          usNoOfEntries = (USHORT)pFileList->ulListUsed;
          pFile         = pFileList->pEntries;
          while ( usNoOfEntries )
          {
            if ((pFile->usFileType == MEMORY_DATA_FILE) ||
                (pFile->usFileType == NTMMEMORY_DATA_FILE) )
            {
              Utlstrccpy( pIda->pszBuffer + ulInsertPos,
                          UtlGetFnameFromPath( pFile->pszName ), DOT );
              strcat( pIda->pszBuffer, szLF );
              fOK = UtlPrintLine( hPrint, pIda->pszBuffer );
              memset( pIda->pszBuffer, BLANK, ulInsertPos );
              pIda->pszBuffer[ulInsertPos] = EOS;
            } /* endif */
            usNoOfEntries--;            // skip to next entry in file list
            pFile++;
          } /* endwhile */
        } /* endif */
        break;
    } /* endswitch */
  } /* endif */

  /********************************************************************/
  /* print markup name                                                */
  /********************************************************************/
  if ( fOK )
  {
    LOADSTRINGLEN( NULLHANDLE, hResMod, SID_FOLDETAILS_PRINT_MARKUP,
                   pIda->pszBuffer, pIda->ulBufSize );
    ANSITOOEM( pIda->pszBuffer );
    switch ( pIda->pHeader->BitFlags.fHeaderType )
    {
      case RELEASE0_HEADER:
        strcat( pIda->pszBuffer, pIda->pHeader->Head.Rel0.szFormat );
        break;
      case RELEASE1_HEADER:
        strcat( pIda->pszBuffer, pIda->pHeader->Head.Rel1.szFormat );
        break;
    } /* endswitch */
    strcat( pIda->pszBuffer, szLF );
    fOK = UtlPrintLine( hPrint, pIda->pszBuffer );
  } /* endif */

  /********************************************************************/
  /* print list of documents                                          */
  /********************************************************************/
  if ( fOK )
  {
    LOADSTRINGLEN( NULLHANDLE, hResMod, SID_FOLDETAILS_PRINT_DOCS,
                   pIda->pszBuffer, pIda->ulBufSize );
    ANSITOOEM( pIda->pszBuffer );
    ulInsertPos = strlen(pIda->pszBuffer);
    szName[0] = EOS;

    //--- fill documents listbox
    switch ( pIda->pHeader->BitFlags.fHeaderType )
    {
      case RELEASE0_HEADER:
        // get documents from file table using type PROP_CLASS_DOCUMENTS
        // and avoiding duplicate names
        if ( pFileList )
        {
          usNoOfEntries = (USHORT)pFileList->ulListUsed;
          pFile         = pFileList->pEntries;
          while ( fOK && usNoOfEntries )
          {
            if (pFile->usFileType == PROP_CLASS_DOCUMENT)
            {
              if ( strcmp( szName,
                           UtlGetFnameFromPath( pFile->pszName ) ) != 0 )
              {
                strcpy( szName,
                        UtlGetFnameFromPath( pFile->pszName ) );
                strcat( pIda->pszBuffer, szName );
                strcat( pIda->pszBuffer, szLF );
                fOK = UtlPrintLine( hPrint, pIda->pszBuffer );
                memset( pIda->pszBuffer, BLANK, ulInsertPos );
                pIda->pszBuffer[ulInsertPos] = EOS;
              } /* endif */
            } /* endif */
            usNoOfEntries--;            // skip to next entry in file list
            pFile++;
          } /* endwhile */
        } /* endif */
        break;
      case RELEASE1_HEADER:
        if ( pIda->pHeader->BitFlags.fDocNameTable )
        {
          PSZ pszDocName; // ptr to current document

          pszDocName = (PSZ)pIda->pHeader;

          // position to long name table
          if ( pIda->pHeader->Head.Rel1.usNoteSize != 0 )
          {
            // position to end of normal header (FOLEXPHEADER contains
            // a one byte buffer for the note already, so we have to
            // decrease the note size by 1)
            pszDocName += sizeof(FOLEXPHEADER) +
                          pIda->pHeader->Head.Rel1.usNoteSize - 1;
          }
          else
          {
            pszDocName += sizeof(FOLEXPHEADER);
          } /* endif */

          while ( *pszDocName != EOS )
          {
            OEMTOANSI( pszDocName );
            strcat( pIda->pszBuffer, pszDocName );
            strcat( pIda->pszBuffer, szLF );
            fOK = UtlPrintLine( hPrint, pIda->pszBuffer );
            memset( pIda->pszBuffer, BLANK, ulInsertPos );
            pIda->pszBuffer[ulInsertPos] = EOS;
            ANSITOOEM( pszDocName );
            pszDocName = pszDocName + strlen(pszDocName) + 1;
          } /* endwhile */
        }
        else
          // get documents from file table using type DOCUMENT_SRC_FILE
          if ( pFileList )
        {
          usNoOfEntries = (USHORT)pFileList->ulListUsed;
          pFile         = pFileList->pEntries;
          while ( fOK && usNoOfEntries )
          {
            if (pFile->usFileType == DOCUMENT_SRC_FILE)
            {
              strcpy( szName,
                      UtlGetFnameFromPath( pFile->pszName ) );
              strcat( pIda->pszBuffer, szName );
              strcat( pIda->pszBuffer, szLF );
              fOK = UtlPrintLine( hPrint, pIda->pszBuffer );
              memset( pIda->pszBuffer, BLANK, ulInsertPos );
              pIda->pszBuffer[ulInsertPos] = EOS;
            } /* endif */
            usNoOfEntries--;            // skip to next entry in file list
            pFile++;
          } /* endwhile */
        } /* endif */
        break;
    } /* endswitch */
  } /* endif */

  /********************************************************************/
  /* print list of dictionaries                                       */
  /********************************************************************/
  if ( fOK )
  {
    LOADSTRINGLEN( NULLHANDLE, hResMod, SID_FOLDETAILS_PRINT_DICTS,
                   pIda->pszBuffer, pIda->ulBufSize );
    ANSITOOEM( pIda->pszBuffer );
    ulInsertPos = strlen(pIda->pszBuffer);

    switch ( pIda->pHeader->BitFlags.fHeaderType )
    {
      case RELEASE0_HEADER:
        // get dictionary names from dictionary table in header
        pszItem = pIda->pHeader->Head.Rel0.DicTbl;
        if ( *pszItem )
        {
          while ( fOK && pszItem )
          {
            Utlstrccpy( szName, pszItem, X15 );
            strcat( pIda->pszBuffer, szName );
            strcat( pIda->pszBuffer, szLF );
            fOK = UtlPrintLine( hPrint, pIda->pszBuffer );
            memset( pIda->pszBuffer, BLANK, ulInsertPos );
            pIda->pszBuffer[ulInsertPos] = EOS;

            pszItem = strchr( pszItem, X15 );
            if ( pszItem )
            {
              pszItem++;
            } /* endif */
          } /* endwhile */
        } /* endif */
        break;
      case RELEASE1_HEADER:
        // get dictionary names from list of packaged files
        if ( pFileList )
        {
          usNoOfEntries = (USHORT)pFileList->ulListUsed;
          pFile         = pFileList->pEntries;
          while ( fOK && usNoOfEntries )
          {
            if (pFile->usFileType == DICTIONARY_DATA_FILE)
            {
              Utlstrccpy( szName,
                          UtlGetFnameFromPath( pFile->pszName ),
                          DOT );
              strcat( pIda->pszBuffer, szName );
              strcat( pIda->pszBuffer, szLF );
              fOK = UtlPrintLine( hPrint, pIda->pszBuffer );
              memset( pIda->pszBuffer, BLANK, ulInsertPos );
              pIda->pszBuffer[ulInsertPos] = EOS;
            } /* endif */
            usNoOfEntries--;            // skip to next entry in file list
            pFile++;
          } /* endwhile */
        } /* endif */
        break;
    } /* endswitch */

  } /* endif */

  /********************************************************************/
  /* print attached note                                              */
  /********************************************************************/
  if ( fOK )
  {
    LOADSTRINGLEN( NULLHANDLE, hResMod, SID_FOLDETAILS_PRINT_NOTE,
                   pIda->pszBuffer, pIda->ulBufSize );
    ANSITOOEM( pIda->pszBuffer );
    ulInsertPos = strlen(pIda->pszBuffer);

    //--- fill attached note MLE ---
    switch ( pIda->pHeader->BitFlags.fHeaderType )
    {
      case RELEASE0_HEADER:
        pszItem = pIda->pHeader->Head.Rel0.szNoteBuffer;
        break;
      case RELEASE1_HEADER:
        pszItem = pIda->pHeader->Head.Rel1.szNoteBuffer;
        break;
    } /* endswitch */

    if ( *pszItem )
    {
      PSZ  pszCRLF;
      CHAR chTemp;

      while ( fOK && (*pszItem != EOS) )
      {
        pszCRLF = pszItem;
        while ( (*pszCRLF != EOS) && (*pszCRLF != LF) && (*pszCRLF != CR) )
        {
          pszCRLF++;
        } /* endwhile */

        chTemp = *pszCRLF;
        *pszCRLF = EOS;
        if ( strlen( pszItem ) > 50 )
        {
          PSZ pszSplit;

          /***********************************************************/
          /* Truncate current line                                   */
          /***********************************************************/
          *pszCRLF = chTemp;
          pszSplit = pszItem + 50;
          while ( (*pszSplit != ' ') && (pszSplit > pszItem) )
          {
            pszSplit--;
          } /* endwhile */

          if ( pszSplit > pszItem )
          {
            /*********************************************************/
            /* Truncate at given blank                               */
            /*********************************************************/
            chTemp = *pszSplit;
            *pszSplit = EOS;
            strcpy( pIda->pszBuffer + ulInsertPos, pszItem );
            *pszSplit = chTemp;

            pszItem = pszSplit + 1;
          }
          else
          {
            /*********************************************************/
            /* no blank found, split at column 50                    */
            /*********************************************************/
            pszSplit = pszItem + 50;
            chTemp = *pszSplit;
            *pszSplit = EOS;
            strcpy( pIda->pszBuffer + ulInsertPos, pszItem );
            *pszSplit = chTemp;

            pszItem = pszSplit;
          } /* endif */
        }
        else
        {
          strcpy( pIda->pszBuffer + ulInsertPos, pszItem );
          *pszCRLF = chTemp;

          pszItem = pszCRLF;
          if ( *pszItem != EOS )
          {
            if ( *pszItem == CR )
            {
              pszItem++;
              if ( *pszItem == LF )
              {
                pszItem++;
              } /* endif */
            }
            else
            {
              if ( *pszItem == LF )
              {
                pszItem++;
              } /* endif */
            } /* endif */
          } /* endif */
        } /* endif */
        strcat( pIda->pszBuffer, szLF );
        fOK = UtlPrintLine( hPrint, pIda->pszBuffer );
        memset( pIda->pszBuffer, BLANK, ulInsertPos );
        pIda->pszBuffer[ulInsertPos] = EOS;
      } /* endwhile */
    } /* endif */
  } /* endif */
  /********************************************************************/
  /* Close print device                                               */
  /********************************************************************/
  if ( hPrint )         UtlPrintClose( hPrint );
  /********************************************************************/
  /* Restore arrow pointer                                            */
  /********************************************************************/
  SETCURSOR( SPTR_ARROW );
} /* end of function FolImpInfoPrintDetails */

//------------------------------------------------------------------------------
// Internal function                                                            
//------------------------------------------------------------------------------
// Function name:     FolImpFolLang                                             
//------------------------------------------------------------------------------
// Description:       Handles folder languages during folder import             
//                    and pops-up folder language dialog if required            
//------------------------------------------------------------------------------
// Input parameter:   PFOLIMPIDA   pIda,  pointer to instance area              
//                    PPROPFOLDER  pProp  ptr to properties of existing folder  
//                                        or NULL if no folder exists           
//                    HWND         hwnd   handle of folder import process       
//                                        window                                
//                    PSZ          pszNewFolder fully qualified name of         
//                                              property file of imported       
//                                              folder                          
//------------------------------------------------------------------------------
// Returncode type:   BOOL                                                      
//------------------------------------------------------------------------------
BOOL FolImpFolLang
(
PFOLIMPIDA   pIda,                   // pointer to instance area
PPROPFOLDER  pProp,                  // ptr to properties of existing folder
// or NULL if no folder exists
HWND         hwnd,                   // handle of folder import process window
PSZ          pszNewFolder            // fully qualified name of property file
// of imported folder
)
{
  PPROPFOLDER      pNewFolder = NULL;  // props of new folder
  ULONG            ulSize;             // prop size
  BOOL             fOK = TRUE;         // function return code
  BOOL             fUpdated = FALSE;   // props-have-been-updated flag

  hwnd;                                // get rid of compiler warnings
  /********************************************************************/
  /* Load properties of imported folder                               */
  /********************************************************************/
  fOK = UtlLoadFileHwnd( pszNewFolder, (PVOID *)&pNewFolder, &ulSize, FALSE, TRUE, hwnd );

  /********************************************************************/
  /* Update with languages from existing folder if none contained     */
  /* in imported folder                                               */
  /********************************************************************/
  if ( fOK && pProp )
  {
    if ( (pNewFolder->szSourceLang[0] == EOS) &&
         (pProp->szSourceLang[0] != EOS) )
    {
      strcpy( pNewFolder->szSourceLang, pProp->szSourceLang );
      fUpdated = TRUE;
    } /* endif */

    if ( (pNewFolder->szTargetLang[0] == EOS) &&
         (pProp->szTargetLang[0] != EOS) )
    {
      strcpy( pNewFolder->szTargetLang, pProp->szTargetLang );
      fUpdated = TRUE;
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* Pop-up folder language dialog if folder languages are missing    */
  /********************************************************************/
  if ( fOK && (pIda->pDDEFolImp == NULL) )
  {
    HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);

    if ( (pNewFolder->szSourceLang[0] == EOS) ||
         (pNewFolder->szTargetLang[0] == EOS) )
    {
      BOOL  fDlgOK;                    // dialog return code
      BOOL  fDone = FALSE;             // we-are-through flag

      do
      {
        DIALOGBOX( EqfQueryTwbClient(), FOLLANGDLGPROC, hResMod, ID_FOLLANG_DLG,
                   pNewFolder, fDlgOK );

        if ( !fDlgOK  )
        {
          PSZ    pszErrParm = pIda->szLongFolName;
          USHORT usMBCode = UtlError( ERROR_FOLIMP_CANCEL, MB_YESNO,
                                      1, &pszErrParm, EQF_QUERY );
          if ( usMBCode == MBID_YES )
          {
            fOK = FALSE;
            pIda->fNoMsg = TRUE;
            fDone = TRUE;
          } /* endif */
        }
        else
        {
          fOK = (fDlgOK == TRUE);      // handle DID_CANCEL return code!
          fDone = TRUE;
          fUpdated = TRUE;             // props may have been changed
        } /* endif */
      } while ( !fDone ); /* enddo */
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* Rewrite properties if languages have been changed                */
  /********************************************************************/
  if ( fOK && fUpdated )
  {
    fOK = ( UtlWriteFileHwnd( pszNewFolder, ulSize, pNewFolder,
                              TRUE, hwnd ) == NO_ERROR);
  } /* endif */

  /********************************************************************/
  /* Cleanup                                                          */
  /********************************************************************/
  if ( pNewFolder )   UtlAlloc( (PVOID *)&pNewFolder, 0L, 0L, NOMSG );

  return( fOK );

} /* end of function FolImpFolLang */

USHORT FolBatchFolderImport
(
HWND             hwndParent,         // handle of folder handler window
PDDEFOLIMP       pFolImp             // folder import data structure
)
{
  PFOLIMPIDA      pIda;               // folder import dialog IDA
  BOOL            fOK = TRUE;         // return value
  HWND            hwndLB = NULLHANDLE;// listbox for folder and document names
  PSZ             pszParm;            // error parameter pointer

  CHAR            szImportPath[MAX_EQF_PATH];
  PSZ             pszImportPath = NULL;      // pointer to that string

  /********************************************************************/
  /* Create invisible listbox for names of folder/memories/...        */
  /********************************************************************/
  hwndLB = WinCreateWindow( hwndParent, WC_LISTBOX, "",
                            LBS_MULTIPLESEL,
                            0, 0, 0, 0,
                            hwndParent, HWND_TOP, 1, NULL, NULL );

  /*******************************************************************/
  /* Create FolImpIda                                                */
  /*******************************************************************/
  fOK = UtlAlloc( (PVOID *)&pIda, 0L, (LONG) sizeof(FOLIMPIDA), NOMSG );
  if ( fOK )
  {
    fOK = UtlAlloc( (PVOID *)&pIda->pszBuffer, 0L, IMPBUFFERSIZE, NOMSG );
    if ( fOK )
    {
      pIda->ulBufSize = IMPBUFFERSIZE;
    } /* endif */
  } /* endif */

  if ( !fOK )
  {
    pFolImp->DDEReturn.usRc = ERROR_STORAGE;
    UtlErrorHwnd( pFolImp->DDEReturn.usRc, MB_CANCEL, 0,
                  NULL, EQF_ERROR, pFolImp->hwndErrMsg );
  } /* endif */

  /*******************************************************************/
  /* Check source drive                                              */
  /*******************************************************************/
  if ( fOK )
  {
    CHAR szDrives[MAX_DRIVELIST];     // buffer for drive letters

    /******************************************************************/
    /* Get valid drives                                               */
    /******************************************************************/
    UtlGetDriveList( (PBYTE)szDrives );

    /******************************************************************/
    /* Check if specified drive is in list of valid drives            */
    /******************************************************************/
    if ( strchr( szDrives, toupper(pFolImp->chFromDrive) ) == NULL )
    {
      CHAR szDrive[MAX_DRIVE];

      fOK = FALSE;
      szDrive[0] = pFolImp->chFromDrive;
      szDrive[1] = COLON;
      szDrive[2] = EOS;
      pszParm = szDrive;
      pFolImp->DDEReturn.usRc = ERROR_EQF_DRIVE_NOT_VALID;
      UtlErrorHwnd( pFolImp->DDEReturn.usRc, MB_CANCEL, 1,
                    &pszParm, EQF_ERROR, pFolImp->hwndErrMsg );
    } /* endif */
  } /* endif */

  /*******************************************************************/
  /* Check target drive                                              */
  /*******************************************************************/
  if ( fOK )
  {
    if ( pFolImp->chToDrive != EOS )
    {
      CHAR szDrives[MAX_DRIVELIST];     // buffer for drive letters

      /******************************************************************/
      /* Get valid drives                                               */
      /******************************************************************/
      UtlGetCheckedEqfDrives( szDrives );

      /******************************************************************/
      /* Check if specified drive is in list of valid drives            */
      /******************************************************************/
      if ( strchr( szDrives, toupper(pFolImp->chToDrive) ) == NULL )
      {
        CHAR szDrive[MAX_DRIVE];

        fOK = FALSE;
        szDrive[0] = pFolImp->chToDrive;
        szDrive[1] = COLON;
        szDrive[2] = EOS;
        pszParm = szDrive;
        pFolImp->DDEReturn.usRc = ERROR_EQF_DRIVE_NOT_VALID;
        UtlErrorHwnd( pFolImp->DDEReturn.usRc, MB_CANCEL, 1,
                      &pszParm, EQF_ERROR, pFolImp->hwndErrMsg );
      } /* endif */
    }
    else
    {
      /***************************************************************/
      /* No target drive specified, use system drive as default      */
      /***************************************************************/
      CHAR szDrive[MAX_DRIVE];
      UtlQueryString( QST_PRIMARYDRIVE, szDrive, MAX_DRIVE );
      pFolImp->chToDrive = szDrive[0];
    } /* endif */
  } /* endif */

  /*******************************************************************/
  /* Check existence of package file                                 */
  /*******************************************************************/
  if ( fOK )
  {
    BOOL        fFound = FALSE;
    CHAR        szTempPath[MAX_EQF_PATH];
    CHAR        chPackName[MAX_FILESPEC];         // buffer for package names

    // first try: access folder export package directly
    {
      static LONGFILEFIND LongResultBuf;      // file find struct
      HDIR hDirHandle = HDIR_CREATE;        // use system file find handle

      pszImportPath = szImportPath;

      if (pFolImp->fFromPath)
      {
        // build complete path

        sprintf(pszImportPath, "%c:\\%s", pFolImp->chFromDrive, pFolImp->chFromPath);
      }
      else
      {
        // do it the conventional way

        UtlMakeEQFPath( szTempPath, pFolImp->chFromDrive, EXPORT_PATH,
                        NULL );
        pszImportPath = NULL;
      }

      strcat( szTempPath, BACKSLASH_STR );
      strcat( szTempPath, pFolImp->szFolder );
      strcat( szTempPath, FOLDEREXPORT_EXT );
      if ( UtlFindFirstLong( szTempPath, &hDirHandle, FILE_NORMAL,
                             &LongResultBuf, FALSE) == NO_ERROR )
      {
        fFound = TRUE;
        // use actual package file name to correct uppercase folder name
        Utlstrccpy( pFolImp->szFolder, LongResultBuf.achName, DOT );
        strcpy( pIda->szLongBuffer, pFolImp->szFolder );
        strcpy( pIda->szBuffer, pFolImp->szFolder );
      } /* endif */
      if ( hDirHandle != HDIR_CREATE ) UtlFindCloseLong( hDirHandle, FALSE );
    }

    // second try: scan all folder packages for package short name
    DELETEALLHWND( hwndLB );
    strcpy( chPackName, DEFAULT_PATTERN_NAME );
    strcat( chPackName, FOLDEREXPORT_EXT );

    UtlPackListPackages( hwndLB, pFolImp->chFromDrive,
                         NULL, pszImportPath, chPackName, 0 );

    // get the long names for the folders from the folder package header
    if ( !fFound )
    {
      SHORT sItem = QUERYITEMCOUNTHWND( hwndLB );
      while ( !fFound && (sItem-- > 0) )
      {
        PVOID       pPackCtrl;

        // get the folder short name (or long name)
        QUERYITEMTEXTHWND( hwndLB, sItem, pIda->szLongBuffer );
        strcpy( pIda->szBuffer, pIda->szLongBuffer );
        UtlUpper( pIda->szBuffer );
        strcat( pIda->szBuffer, FOLDEREXPORT_EXT );
        pPackCtrl = UtlPackOpenPackage( pFolImp->chFromDrive, NULL, pszImportPath,
                                        pIda->szBuffer,
                                        pFolImp->hwndErrMsg );
        if ( pPackCtrl )
        {
          PPROPFOLDER pProp = NULL;

          // setup name of folder property file
          UtlMakeEQFPath(  pIda->szBuffer, NULC, PROPERTY_PATH, NULL );
          strcat( pIda->szBuffer, BACKSLASH_STR );
          strcat( pIda->szBuffer, pIda->szLongBuffer );
          strcat( pIda->szBuffer, EXT_FOLDER_MAIN );

          pIda->szBuffer[0] = EOS;

          UtlPackReadFilePart( pPackCtrl, pIda->szBuffer,
                               (PBYTE)&(pProp->szLongName) - (PBYTE)pProp,
                               (PBYTE)pIda->szBuffer,
                               sizeof(pIda->szBuffer) );
          if ( pIda->szBuffer[0] == EOS )
          {
            // no long name in properties, use folder file name instead
            strcpy( pIda->szBuffer, pIda->szLongBuffer );
          } /* endif */

          // close package
          UtlPackClosePackage( pPackCtrl ); // ... close it
          pPackCtrl = NULL;
        } /* endif */

        // compare long name with specified one
        if ( stricmp( pIda->szBuffer, pFolImp->szFolder ) == 0 )
        {
          // found the requested folder
          fFound = TRUE;                        // force end of loop
        } /* endif */
      } /* endwhile */

      if ( !fFound )
      {
        fOK = FALSE;
        pszParm = pFolImp->szFolder;
        pFolImp->DDEReturn.usRc = ERROR_FILE_ACCESS_ERROR;
        UtlErrorHwnd( pFolImp->DDEReturn.usRc, MB_CANCEL, 1,
                      &pszParm, EQF_ERROR, pFolImp->hwndErrMsg );
      } /* endif */
    } /* endif */
  } /* endif */

  /*******************************************************************/
  /* Fill IDA fields and prepare the package for import              */
  /*******************************************************************/
  if ( fOK )
  {
    pIda->pDDEFolImp = pFolImp;       // anchor batch folder import struct

    pIda->chFromDrive = pFolImp->chFromDrive;
    {
      BOOL        fIsNew;

      strcpy( pIda->szSourceFolName, pIda->szLongBuffer );
      strcpy( pIda->szTargetFolName, pIda->szBuffer );
      ObjLongToShortName( pIda->szBuffer,
                          pIda->szTargetShortFolName,
                          FOLDER_OBJECT, &fIsNew );
    }
    strcpy( pIda->szSourceFolder, pIda->szSourceFolName );
    strcat( pIda->szSourceFolder, EXT_FOLDER_MAIN );
    strcpy( pIda->szObjPath, pIda->szSourceFolName );
    strcat( pIda->szObjPath, FOLDEREXPORT_EXT );
    pIda->pPackCtrl = UtlPackOpenPackage( pIda->chFromDrive, NULL, pszImportPath,
                                          pIda->szObjPath,
                                          pIda->pDDEFolImp->hwndErrMsg );

    /*****************************************************************/
    /* Read package's user header                                    */
    /*****************************************************************/
    if ( pIda->pPackCtrl )            // if the package is open ...
    {
      ULONG ulHeaderSize = UtlPackQueryHeaderSize( pIda->pPackCtrl );
      if ( ulHeaderSize )
      {
        fOK = UtlAlloc( (PVOID *)&pIda->pHeader, 0L, (LONG) ulHeaderSize, NOMSG );
        if ( fOK )
        {
          ULONG ulRC = UtlPackReadHeader( pIda->pPackCtrl,
                                           (PBYTE) pIda->pHeader,
                                           ulHeaderSize );
          if ( ulRC != PACK_OK_RC )
          {
            // read of header failed
            fOK = FALSE;
            UtlAlloc( (PVOID *)&pIda->pHeader, 0L, 0L, NOMSG );
            UtlPackClosePackage( pIda->pPackCtrl );
            pIda->pPackCtrl = NULL;
          } /* endif */
        }
        else
        {
          // alloc failed
          pFolImp->DDEReturn.usRc = ERROR_STORAGE;
          UtlErrorHwnd( pFolImp->DDEReturn.usRc, MB_CANCEL, 0,
                        NULL, EQF_ERROR, pFolImp->hwndErrMsg );
          UtlPackClosePackage( pIda->pPackCtrl );
          pIda->pPackCtrl = NULL;
        } /* endif */
      }
      else
      {
        // file is no valid folder package
        fOK = FALSE;
        pszParm = pIda->szObjPath;
        pFolImp->DDEReturn.usRc = ERROR_FILE_INVALID_DATA;
        UtlErrorHwnd( pFolImp->DDEReturn.usRc, MB_CANCEL, 1,
                      &pszParm, EQF_ERROR, pFolImp->hwndErrMsg );
      } /* endif */
    }
    else
    {
      fOK = FALSE;         // error: package did not open
      pszParm = pIda->szObjPath;
      pFolImp->DDEReturn.usRc = ERROR_FILE_INVALID_DATA;
      UtlErrorHwnd( pFolImp->DDEReturn.usRc, MB_CANCEL, 1,
                    &pszParm, EQF_ERROR, pFolImp->hwndErrMsg );
    } /* endif */

    /*****************************************************************/
    /* Check if folder exists somethere and adjust TO drive          */
    /*****************************************************************/
    if ( fOK )
    {
      PPROPFOLDER pFolProp;               // ptr to folder properties
      PVOID       hFolProp;               // handle of folder properties
      EQFINFO     ErrorInfo;              // error code of property handler calls

      UtlMakeEQFPath( pIda->szTargetFolObj, NULC, SYSTEM_PATH, NULL );
      strcat( pIda->szTargetFolObj, BACKSLASH_STR );
      strcpy( pIda->szTargetFolder, pIda->szTargetShortFolName );
      strcat( pIda->szTargetFolder, EXT_FOLDER_MAIN );
      strcat( pIda->szTargetFolObj, pIda->szTargetFolder );
      hFolProp = OpenProperties( pIda->szTargetFolObj, NULL,
                                 PROP_ACCESS_READ,
                                 &ErrorInfo);
      if ( hFolProp )
      {
        pFolProp = (PPROPFOLDER)MakePropPtrFromHnd( hFolProp );
        if ( pFolProp->chDrive == NULC )
        {
          pFolImp->chToDrive = pIda->szTargetFolObj[0];
        }
        else
        {
          pFolImp->chToDrive = pFolProp->chDrive;
        } /* endif */
        CloseProperties( hFolProp, PROP_QUIT, &ErrorInfo );
      } /* endif */
    } /* endif */

    /*****************************************************************/
    /* Check if folder has been locked by another process            */
    /*****************************************************************/
    if ( fOK )
    {
      SHORT sRC = QUERYSYMBOL( pIda->szTargetFolObj );
      if ( sRC != -1 )
      {
        PSZ    pszParm = pIda->szTargetFolObj;
        fOK = FALSE;
        pFolImp->DDEReturn.usRc = ERROR_FOLDER_LOCKED;
        UtlErrorHwnd( ERROR_FOLDER_LOCKED, MB_CANCEL, 1,
                      &pszParm, EQF_INFO, pFolImp->hwndErrMsg );
      } /* endif */
    } /* endif */

    /*****************************************************************/
    /* If o.k. remember settings                                     */
    /*****************************************************************/
    if ( fOK )
    {
      pIda->fWithDict = pFolImp->fWithDict;
      pIda->fWithMem  = pFolImp->fWithMem;
      pIda->chToDrive = pFolImp->chToDrive;;
    } /* endif */

    /*****************************************************************/
    /* If o.k. set/reset skip flags of package files                 */
    /*****************************************************************/
    if ( fOK )
    {
      PFILELIST   pFileList;         // ptr to package's file list
      USHORT      usNoOfEntries;     // # of enteries in package's file list
      PFILELISTENTRY pFile;          // ptr for file list processing

      ULONG ulRC = UtlPackGetFileListPtr( pIda->pPackCtrl, &pFileList );
      fOK = UtlPackHandleError( ulRC, pIda->szSourceFolName, pIda->pDDEFolImp->hwndErrMsg  );
      if ( fOK )
      {
        usNoOfEntries = (USHORT)pFileList->ulListUsed;
        pFile         = pFileList->pEntries;
        while ( usNoOfEntries )
        {
          pFile->usProcessFlags &= ~FILE_SKIP;

          if ( pIda->pHeader->BitFlags.fHeaderType == RELEASE0_HEADER )
          {
            if ( ( pFile->usFileType == PROP_CLASS_TAGTABLE ) ||
                 ( pFile->usFileType == PROP_CLASS_MEMORY ) )
            {
              if ( !pIda->fWithMem )
              {
                pFile->usProcessFlags |= FILE_SKIP;
              } /* endif */
            }
            else if ( pFile->usFileType == PROP_CLASS_DICTIONARY )
            {
              if ( !pIda->fWithDict )
              {
                pFile->usProcessFlags |= FILE_SKIP;
              } /* endif */
            } /* endif */
          }
          else
          {
            if ( ( pFile->usFileType == MEMORY_PROP_FILE ) ||
                 ( pFile->usFileType == MEMORY_DATA_FILE ) ||
                 ( pFile->usFileType == MEMORY_TABLE_FILE ) )
            {
              if ( !pIda->fWithMem )
              {
                pFile->usProcessFlags |= FILE_SKIP;
              } /* endif */
            }
            else if ( ( pFile->usFileType == DICTIONARY_PROP_FILE ) ||
                      ( pFile->usFileType == DICTIONARY_DATA_FILE ) ||
                      ( pFile->usFileType == DICTIONARY_INDEX_FILE ) )
            {
              if ( !pIda->fWithDict )
              {
                pFile->usProcessFlags |= FILE_SKIP;
              } /* endif */
            } /* endif */
          } /* endif */

          usNoOfEntries--;            // skip to next entry in file list
          pFile++;
        } /* endwhile */
      } /* endif */
      if ( !fOK ) pFolImp->DDEReturn.usRc = UtlGetDDEErrorCode( pFolImp->hwndErrMsg );
    } /* endif */

    /*****************************************************************/
    /* If o.k. check free space on target drive                      */
    /*****************************************************************/
    if ( fOK )
    {
      ULONG ulRequiredSpace;         // required space for package

      ULONG ulRC = UtlPackQueryRequiredSpace( pIda->pPackCtrl,
                                               &ulRequiredSpace );
      fOK = UtlPackHandleError( ulRC, pIda->szSourceFolName, pIda->pDDEFolImp->hwndErrMsg );
      if ( fOK )
      {
        ULONG64 lFreeSpace = UtlQueryFreeSpace( pIda->chToDrive, FALSE );
        if ( lFreeSpace < (LONG)ulRequiredSpace )
        {
          CHAR   szDrive[3];       // buffer for drive string
          PSZ    pErrParms[2];

          pErrParms[0] = pIda->szSourceFolName;
          szDrive[0] = pIda->chToDrive;
          szDrive[1] = EOS;
          pErrParms[1] = szDrive;
          pFolImp->DDEReturn.usRc = ERROR_FOLIMP_NOSPACE;
          UtlErrorHwnd( pFolImp->DDEReturn.usRc, MB_CANCEL, 2,
                        pErrParms, EQF_ERROR, pIda->pDDEFolImp->hwndErrMsg );
          fOK = FALSE;
        } /* endif */
      } /* endif */
      if ( !fOK ) pFolImp->DDEReturn.usRc = UtlGetDDEErrorCode( pFolImp->hwndErrMsg );
    } /* endif */


    /*****************************************************************/
    /* If o.k. lock folder                                           */
    /*****************************************************************/
    if ( fOK )
    {
      SETSYMBOL( pIda->szTargetFolObj );
    } /* endif */

    UtlPackSetTargetDrive( pIda->pPackCtrl, pIda->chToDrive );
  } /* endif */

  /*******************************************************************/
  /* Start folder import                                             */
  /*******************************************************************/
  if ( fOK )
  {
    strcpy( pIda->szObjPath, FOLIMPOBJPREFIX );
    strcat( pIda->szObjPath, pIda->szObjName );
    fOK = CreateProcessWindow( pIda->szObjPath, FolImportCallBack, pIda );
  } /* endif */

  /*******************************************************************/
  /* Cleanup                                                         */
  /*******************************************************************/
  if ( hwndLB )  WinDestroyWindow( hwndLB );
  if ( !fOK )
  {
    /****************************************************************/
    /* Free allocated memory                                        */
    /****************************************************************/
    if ( pIda )
    {
      if ( pIda->pszBuffer ) UtlAlloc( (PVOID *)&(pIda->pszBuffer), 0L, 0L, NOMSG) ;
      UtlAlloc( (PVOID *)&(pIda), 0L, 0L, NOMSG) ;
    } /* endif */

    /****************************************************************/
    /* report end of task to DDE handler                            */
    /****************************************************************/
    WinPostMsg( pFolImp->hwndOwner, WM_EQF_DDE_ANSWER,
                NULL, &pFolImp->DDEReturn );
  } /* endif */

  return( pFolImp->DDEReturn.usRc );

} /* end of function FolbatchFolderImport */



/**********************************************************************/
/* FolMergeHistoryLog                                                 */
/*                                                                    */
/* Merge an imported history log file into an existing one            */
/* returns NO_ERROR or any DOS error return code encountered          */
/* Optionally rename a document during merge operation                */
/**********************************************************************/
USHORT FolMergeHistoryLog
(
PSZ         pszMergeFrom,            // fully qualified name of input log
PSZ         pszMergeInto,            // fully qualified name of existing log
BOOL        fMsg,                    // message handling flag
HWND        hwnd,                    // window handle for error messages
PSZ         pszRenameFrom,           // rename from document name or NULL
PSZ         pszRenameTo              // rename to document name or NULL
)
{
  USHORT      usRC = NO_ERROR;         // function return code
  CHAR        szTempLog[MAX_EQF_PATH]; // name of temporary (= output) logfile
  BOOL        fEOFInFile1 = FALSE;     // TRUE = EOF for hInFile1 reached
  BOOL        fEOFInFile2 = FALSE;     // TRUE = EOF for hInFile2 reached
  HFILE       hInFile1 = NULLHANDLE;   // handle of merge-into log file
  HFILE       hInFile2 = NULLHANDLE;   // handle of merge-from log file
  HFILE       hOutFile = NULLHANDLE;   // handle of output log file
  PHISTLOGRECORD pRecord1 = NULL;      // ptr to buffer for log records
  PHISTLOGRECORD pRecord2 = NULL;      // ptr to buffer for log records
  PHISTLOGRECORD pTable = NULL;        // ptr to table of histlog records
  ULONG       ulTableSize = 0;         // size of table
  ULONG       ulTableUsed = 0;         // number of used entries in table

  // allocate history log record buffers
  if ( usRC == NO_ERROR )
  {
    if ( !UtlAlloc( (PVOID *)&pRecord1, 0, MAXHISTLOGRECORDSIZE, NOMSG ) )
    {
      usRC = ERROR_NOT_ENOUGH_MEMORY;
    } /* endif */
  } /* endif */
  if ( usRC == NO_ERROR )
  {
    if ( !UtlAlloc( (PVOID *)&pRecord2, 0, MAXHISTLOGRECORDSIZE, NOMSG ) )
    {
      usRC = ERROR_NOT_ENOUGH_MEMORY;
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* Open merge-into log file                                         */
  /********************************************************************/
  if ( usRC == NO_ERROR )
  {
    USHORT usOpenAction;

    usRC = UtlOpenHwnd( pszMergeInto, &hInFile1, &usOpenAction, 0L,
                        FILE_NORMAL, FILE_OPEN,
                        OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYWRITE,
                        0L, fMsg, hwnd );
  } /* endif */

  /********************************************************************/
  /* Open merge-from log file                                         */
  /********************************************************************/
  if ( usRC == NO_ERROR )
  {
    USHORT usOpenAction;

    usRC = UtlOpenHwnd( pszMergeFrom, &hInFile2, &usOpenAction, 0L,
                        FILE_NORMAL, FILE_OPEN,
                        OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYWRITE,
                        0L, fMsg, hwnd );
  } /* endif */

  /********************************************************************/
  /* Setup name of temporary output file                              */
  /********************************************************************/
  if ( usRC == NO_ERROR )
  {
    PSZ pszExt;

    strcpy( szTempLog, pszMergeInto );
    pszExt = strrchr( szTempLog, DOT );
    if ( pszExt != NULL )
    {
      strcpy( pszExt, ".$$$" );
    }
    else
    {
      strcat( szTempLog, ".$$$" );
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* Create temporary log file                                        */
  /********************************************************************/
  if ( usRC == NO_ERROR )
  {
    USHORT usOpenAction;

    usRC = UtlOpenHwnd( szTempLog, &hOutFile, &usOpenAction, 0L,
                        FILE_NORMAL,
                        FILE_TRUNCATE | FILE_CREATE,
                        OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYWRITE,
                        0L, fMsg, hwnd );
  } /* endif */

  /********************************************************************/
  /* Read first record of merge-from log file                         */
  /********************************************************************/
  if ( usRC == NO_ERROR )
  {
    usRC = FolReadLogRecord( hInFile1, pRecord1, &fEOFInFile1, fMsg, hwnd );
  } /* endif */

  /********************************************************************/
  /* Read first record from merge-into log file                       */
  /********************************************************************/
  if ( usRC == NO_ERROR )
  {
    usRC = FolReadLogRecord( hInFile2, pRecord2, &fEOFInFile2, fMsg, hwnd );
  } /* endif */

  /********************************************************************/
  /* Process records until EOF for BOTH input log files               */
  /********************************************************************/
  while ( (usRC == NO_ERROR) && !(fEOFInFile1 && fEOFInFile2) )
  {
    /******************************************************************/
    /* Apply document rename if requested                             */
    /******************************************************************/
    if ( (pszRenameFrom != NULL) && (pszRenameTo != NULL) )
    {
      if ( strcmp( pszRenameFrom, pRecord2->szDocName ) == 0 )
      {
        strcpy( pRecord2->szDocName, pszRenameTo );
      } /* endif */
    } /* endif */

    /******************************************************************/
    /* Write older record to output log file, ignore identical        */
    /* records, re-read records as approbriate                        */
    /******************************************************************/
    if ( pRecord1->lTime > pRecord2->lTime )
    {
      /****************************************************************/
      /* Record 2 is older than record 1 so write this one to the     */
      /* output log file and read next record from InFile2            */
      /****************************************************************/
      usRC = FolWriteLogRecord( hOutFile, pRecord2, fMsg, hwnd );
      if ( usRC == NO_ERROR && !fEOFInFile2 )
      {
        usRC = FolReadLogRecord( hInFile2, pRecord2, &fEOFInFile2, fMsg, hwnd );
      } /* endif */
    }
    else if ( pRecord1->lTime < pRecord2->lTime )
    {
      /****************************************************************/
      /* Record 1 is older than record 2 so write this one to the     */
      /* output log file and read next record from InFile1            */
      /****************************************************************/
      usRC = FolWriteLogRecord( hOutFile, pRecord1, fMsg, hwnd );
      if ( usRC == NO_ERROR && !fEOFInFile1 )
      {
        usRC = FolReadLogRecord( hInFile1, pRecord1, &fEOFInFile1, fMsg, hwnd );
      } /* endif */
    }
    else
    {
      // we have records which have the same date/time stamp
      // to ensure that duplicates are properly removed we have to read all
      // records having the same date/time stamp into an array and ignore
      // the duplicate ones
      LONG lRecTime = pRecord1->lTime;

      ulTableUsed = 0;

      // get all records from histlog1
      while ( (usRC == NO_ERROR) && (pRecord1->lTime == lRecTime) )
      {
        usRC = FolAddRecordToTable( &pTable, &ulTableSize, &ulTableUsed, pRecord1 );
        if ( usRC == NO_ERROR && !fEOFInFile1 )
        {
          usRC = FolReadLogRecord( hInFile1, pRecord1, &fEOFInFile1, fMsg, hwnd );
        } /* endif */
      } /* endwhile */

      // get all records from histlog2
      while ( (usRC == NO_ERROR) && (pRecord2->lTime == lRecTime) )
      {
        usRC = FolAddRecordToTable( &pTable, &ulTableSize, &ulTableUsed, pRecord2 );
        if ( usRC == NO_ERROR && !fEOFInFile2 )
        {
          usRC = FolReadLogRecord( hInFile2, pRecord2, &fEOFInFile2, fMsg, hwnd );
        } /* endif */
      } /* endwhile */

      // write consolidated records from table to output histlog
      {
        PHISTLOGRECORD pRecord = pTable;
        ULONG ulIndex = 0;
        while ( (ulIndex < ulTableUsed) && (usRC == NO_ERROR) )
        {
          usRC = FolWriteLogRecord( hOutFile, pRecord, fMsg, hwnd );
          pRecord = (PHISTLOGRECORD)((PBYTE)pRecord + MAXHISTLOGRECORDSIZE);
          ulIndex++;
        } /* endwhile */
      }
    } /* endif */
  } /* endwhile */

  /********************************************************************/
  /* Close all logfiles                                               */
  /********************************************************************/
  if ( hInFile1 != NULLHANDLE ) UtlClose( hInFile1, FALSE );
  if ( hInFile2 != NULLHANDLE ) UtlClose( hInFile2, FALSE );
  if ( hOutFile != NULLHANDLE ) UtlClose( hOutFile, FALSE );

  /********************************************************************/
  /* Delete merge-into log file and rename tempory log file to        */
  /* merge-into log file                                              */
  /********************************************************************/
  if ( usRC == NO_ERROR )
  {
    UtlDeleteHwnd( pszMergeInto, 0L, fMsg, hwnd );
    usRC = UtlMoveHwnd( szTempLog, pszMergeInto, 0L, fMsg, hwnd );
  } /* endif */

  /********************************************************************/
  /* Cleanup                                                          */
  /********************************************************************/
  if ( pRecord1 != NULL ) UtlAlloc( (PVOID *)&pRecord1, 0L, 0L, NOMSG );
  if ( pRecord2 != NULL ) UtlAlloc( (PVOID *)&pRecord2, 0L, 0L, NOMSG );
  if ( pTable != NULL ) UtlAlloc( (PVOID *)&pTable, 0L, 0L, NOMSG );
  return( usRC );
} /* end of function FolMergeHistoryLog */

// adds a history log entry to the table if the record is not yet part of the table
USHORT FolAddRecordToTable( PHISTLOGRECORD *ppTable, PULONG pulTableSize, PULONG pulTableUsed, PHISTLOGRECORD pRecord )
{
  USHORT      usRC = NO_ERROR;         // function return code
  BOOL        fFound = FALSE;          // TRUE = record already contained in table
  ULONG       ulIndex = 0;             // index for table access
  PHISTLOGRECORD pTemp = *ppTable;

  // search record in our table
  while ( !fFound && (ulIndex < *pulTableUsed) )
  {
    if ( memcmp( pTemp, pRecord, sizeof(HISTLOGRECORD) ) == 0 )
    {
      fFound = TRUE;                   // record already contained in our ttable
    }
    else
    {
      pTemp = (PHISTLOGRECORD)((PBYTE)pTemp + MAXHISTLOGRECORDSIZE);
      ulIndex++;
    } /* endif */
  } /* endwhile */

  // if not found add record to table
  if ( !fFound )
  {
    // enlarge tabe if necessary
    if ( *pulTableUsed >= *pulTableSize )
    {
      ULONG ulOldLen = *pulTableSize * MAXHISTLOGRECORDSIZE;
      ULONG ulNewLen = (*pulTableSize + 5) * MAXHISTLOGRECORDSIZE;

      if ( !UtlAlloc( (PVOID *)ppTable, ulOldLen, ulNewLen, NOMSG ) )
      {
        usRC = ERROR_NOT_ENOUGH_MEMORY;
      }
      else
      {
        *pulTableSize += 5;
      } /* endif */
    } /* endif */

    // add record to table
    if ( usRC == NO_ERROR )
    {
      PHISTLOGRECORD pNewRecord;
      pNewRecord = (PHISTLOGRECORD)((PBYTE)*ppTable + (*pulTableUsed * MAXHISTLOGRECORDSIZE));
      memcpy( pNewRecord, pRecord, MAXHISTLOGRECORDSIZE );
      *pulTableUsed += 1;
    } /* endif */

  } /* endif */

  return( usRC );
} /* end of function FolAddRecordToTable */


/**********************************************************************/
/* FolFilterHistoryLog                                                */
/*                                                                    */
/* Creates a copy of the history log file containing only the entries */
/* for the selected document                                          */
/**********************************************************************/
USHORT FolFilterHistoryLog
(
PSZ         pszFromLog,              // fully qualified name of input log
PSZ         pszToLog,                // fully qualified name of new log
PSZ         pszDoc,                  // name of selected document
BOOL        fMsg,                    // message handling flag
HWND        hwnd                     // window handle for error messages
)
{
  USHORT      usRC = NO_ERROR;         // function return code
  BOOL        fEOF = FALSE;            // TRUE = EOF for hInFile reached
  HFILE       hInFile  = NULLHANDLE;   // handle of input log file
  HFILE       hOutFile = NULLHANDLE;   // handle of output log file
  PHISTLOGRECORD pRecord  = NULL;      // ptr to buffer for log records

  if ( !UtlAlloc( (PVOID *)&pRecord, 0, MAXHISTLOGRECORDSIZE, NOMSG ) )
  {
    usRC = ERROR_NOT_ENOUGH_MEMORY;
  } /* endif */

  /********************************************************************/
  /* Open input log file                                              */
  /********************************************************************/
  if ( usRC == NO_ERROR )
  {
    USHORT usOpenAction;

    usRC = UtlOpenHwnd( pszFromLog, &hInFile, &usOpenAction, 0L,
                        FILE_NORMAL, FILE_OPEN,
                        OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYWRITE,
                        0L, fMsg, hwnd );
  } /* endif */

  /********************************************************************/
  /* Create output log file                                           */
  /********************************************************************/
  if ( usRC == NO_ERROR )
  {
    USHORT usOpenAction;

    usRC = UtlOpenHwnd( pszToLog, &hOutFile, &usOpenAction, 0L,
                        FILE_NORMAL,
                        FILE_TRUNCATE | FILE_CREATE,
                        OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYWRITE,
                        0L, fMsg, hwnd );
  } /* endif */

  /********************************************************************/
  /* Read first record of input log file                              */
  /********************************************************************/
  if ( usRC == NO_ERROR )
  {
    usRC = FolReadLogRecord( hInFile, pRecord, &fEOF, fMsg, hwnd );
  } /* endif */

  /********************************************************************/
  /* Process records until EOF                                        */
  /********************************************************************/
  while ( (usRC == NO_ERROR) && !fEOF )
  {
    /******************************************************************/
    /* Write records for the selected document to the output log file */
    /******************************************************************/
    if ( stricmp( pszDoc, pRecord->szDocName) == 0 )
    {
      usRC = FolWriteLogRecord( hOutFile, pRecord, fMsg, hwnd );
    } /* endif */

    /******************************************************************/
    /* Read next record                                               */
    /******************************************************************/
    if ( usRC == NO_ERROR )
    {
      usRC = FolReadLogRecord( hInFile, pRecord, &fEOF, fMsg, hwnd );
    } /* endif */
  } /* endwhile */

  /********************************************************************/
  /* Close all logfiles                                               */
  /********************************************************************/
  if ( hInFile != NULLHANDLE )  UtlClose( hInFile, FALSE );
  if ( hOutFile != NULLHANDLE ) UtlClose( hOutFile, FALSE );

  /********************************************************************/
  /* Cleanup                                                          */
  /********************************************************************/
  if ( pRecord != NULL ) UtlAlloc( (PVOID *)&pRecord, 0L, 0L, NOMSG );

  return( usRC );
} /* end of function FolFilterHistoryLog */

USHORT FolFuncImportFolder
(
  PFCTDATA    pData,                   // function I/F session data
  PSZ         pszFolderName,           // name of folder being imported
  PSZ         pszFromPath,             // path containing the imported folder
  CHAR        chToDrive,               // target drive for folder
  PSZ         pszNewFolderName,        // new name for the folder
  PSZ         pszNewMemNames,          // opt: comma separated list of new memory names, the first entry is used to rename the folder memory, the remaining names are used to rename the search memories
  LONG        lOptions                 // folder import options
)
{
  USHORT      usRC = NO_ERROR;         // function return code

  // prepare a new analysis run or continue current one
  if ( pData->fComplete )              // has last run been completed
  {
    // prepare a new analysis run
    pData->usProgress = 0;
    usRC = FolFuncPrepImportFolder( pData, pszFolderName, pszFromPath, chToDrive, pszNewFolderName, pszNewMemNames, lOptions );
  }
  else
  {
    // continue current analysis process
    usRC = FolFuncImportFolderProcess( pData );
  } /* endif */
  return( usRC );
} /* end of function FolFuncImportFolder */

/**********************************************************************/
/* FolWriteNewTargetRec                                               */
/*                                                                    */
/* Writes a new target record to the history log file                 */
/**********************************************************************/
USHORT FolWriteNewTargetRec
(
PSZ         pszFolObjName,           // object name of folder
PSZ         pszDocument,             // name of document
BOOL        fMsg,                    // TRUE = show errors directly
HWND        hwnd                     // window to be used as parent for messages
)
{
  USHORT      usRC = NO_ERROR;         // function return code
  PTBDOCUMENT pDoc = NULL;             // ptr to document structure
  CHAR  szFullDocName[MAX_EQF_PATH];   // fully qualified document name
  fMsg;
  // setup fully qualified document path name
  UtlMakeEQFPath( szFullDocName, pszFolObjName[0],
                  DIRSEGTARGETDOC_PATH,
                  UtlGetFnameFromPath(pszFolObjName) );
  strcat( szFullDocName, BACKSLASH_STR );
  strcat( szFullDocName, pszDocument );

  // return immediately if there is no target document
  if ( !UtlFileExist( szFullDocName ) )
  {
    return( NO_ERROR );
  } /* endif */

  /********************************************************************/
  /* Allocate TBDOCUMENT structure                                    */
  /********************************************************************/
  if ( usRC == NO_ERROR )
  {
    BOOL fOK = UtlAllocHwnd( (PVOID *)&pDoc, 0L,
                             (LONG)sizeof(TBDOCUMENT), ERROR_STORAGE, hwnd );
    if ( !fOK )
    {
      usRC = ERROR_NOT_ENOUGH_MEMORY;
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* Load the QF tag table                                            */
  /********************************************************************/
  if ( usRC == NO_ERROR )
  {
    usRC = TALoadTagTable( QFTAG_TABLE, (PLOADEDTABLE *)&(pDoc->pQFTagTable),
                           TRUE,                  // load internal table
                           TRUE );                // do message handling
  } /* endif */

  // set TBDOCUMENT ulOEMCodePage/ulAnsiCodePage acc. to System Preferences language
  pDoc->ulOemCodePage = GetLangCodePage( OEM_CP, NULL );
  pDoc->ulAnsiCodePage = GetLangCodePage( ANSI_CP, NULL );

  /********************************************************************/
  /* Load the segmented target file into the document structure       */
  /********************************************************************/
  if ( usRC == NO_ERROR )
  {
    SHORT sRc;
    strcpy( pDoc->szDocName, szFullDocName );
    sRc = EQFBFileReadExW( szFullDocName, pDoc, FILEREAD_FPOS );
  } /* endif */

  /********************************************************************/
  /* Count words and write history log record                         */
  /********************************************************************/
  if ( usRC == NO_ERROR )
  {
    EQFBHistDocSave( szFullDocName, pDoc, DOCIMPNEWTARGET_LOGTASK2 );
  } /* endif */

  /********************************************************************/
  /* Free the document structure                                      */
  /********************************************************************/
  if ( pDoc != NULL)
  {
    PTBSEGMENTTABLE pSegTable;          // ptr for segment table deleting
    ULONG           ulI, ulJ;           // loop counter
    PTBSEGMENT      pSegment;           // ptr for segment deleting

    UtlAlloc( (PVOID *) &pDoc->pInBuf,  0L, 0L, NOMSG );
    UtlAlloc( (PVOID *) &pDoc->pTokBuf, 0L, 0L, NOMSG );

    pSegTable = pDoc->pSegTables;
    for ( ulI = 0; ulI < pDoc->ulSegTables; ulI++ )
    {
      pSegment = pSegTable->pSegments;
      for ( ulJ = 0; ulJ < pSegTable->ulSegments; ulJ++ )
      {
        if ( pSegment->pData )
        {
          UtlAlloc( (PVOID *) &pSegment->pData, 0L, 0L, NOMSG );
          if (pSegment->pContext) UtlAlloc((PVOID *)&(pSegment->pContext),0L,0L,NOMSG);
        } /* endif */
        pSegment++;
      } /* endfor */
      UtlAlloc( (PVOID *) &pSegTable->pSegments, 0L, 0L, NOMSG );
      pSegTable++;
    } /* endfor */
    pDoc->ulSegTables = 0;
    UtlAlloc( (PVOID *) &pDoc->pSegTables, 0L, 0L, NOMSG );
    UtlAlloc( (PVOID *) &pDoc->pUndoSeg, 0L, 0L, NOMSG );  //free storage of Undo
    UtlAlloc( (PVOID *) &pDoc->pUndoSegW, 0L, 0L, NOMSG );
    if ( pDoc->pContext )  UtlAlloc((PVOID *) &pDoc->pContext, 0L, 0L, NOMSG );
    if (pDoc->pWSList) UtlAlloc((PVOID *)&(pDoc->pWSList), 0L, 0, NOMSG);
    UtlAlloc( (PVOID *) &pDoc, 0L, 0L, NOMSG );
  } /* endif */

  /********************************************************************/
  /* Return to caller                                                 */
  /********************************************************************/
  return( usRC );
} /* end of function FolWriteNewTargetRec */



USHORT FolFuncPrepImportFolder
(
  PFCTDATA    pData,                   // function I/F session data
  PSZ         pszFolderName,           // name of folder being imported
  PSZ         pszFromPath,             // path containing the imported folder
  CHAR        chToDrive,               // target drive for folder
  PSZ         pszNewFolderName,        // new name for the folder
  PSZ         pszNewMemNames,          // opt: comma separated list of new memory names, the first entry is used to rename the folder memory, the remaining names are used to rename the search memories
  LONG        lOptions                 // folder import options
)
{
  PFOLIMPIDA      pIda = NULL;        // folder import dialog IDA
  BOOL            fOK = TRUE;         // return value
  PSZ             pszParm;            // error parameter pointer
  ULONG           ulRC = NO_ERROR;    // function return code

  pData->usProgress = 0;              // nothing done yet

  // check if folder name is specified
  if ( fOK )
  {
    if ( (pszFolderName == NULL) || (*pszFolderName == EOS) )
    {
      fOK = FALSE;
      ulRC = TA_MANDFOLDER;
      UtlErrorHwnd( TA_MANDFOLDER, MB_CANCEL, 0, NULL, EQF_ERROR, HWND_FUNCIF );
    } /* endif */
  } /* endif */


  // Create FolImpIda
  if ( fOK )
  {
    fOK = UtlAlloc( (PVOID *)&pIda, 0L, (LONG) sizeof(FOLIMPIDA), NOMSG );
    if ( fOK )
    {
      fOK = UtlAlloc( (PVOID *)&pIda->pszBuffer, 0L, IMPBUFFERSIZE, NOMSG  );
      if ( fOK )
      {
        pIda->ulBufSize = IMPBUFFERSIZE;
      } /* endif */
    } /* endif */
    if ( !fOK )
    {
      ulRC = ERROR_STORAGE;
      UtlErrorHwnd( ERROR_STORAGE, MB_CANCEL, 0,
                    NULL, EQF_ERROR, HWND_FUNCIF );
    } /* endif */
  } /* endif */

  // check source path
  if ( fOK )
  {
    if (!UtlDirExist(pszFromPath))
    {
      ulRC = ERROR_FILENAME_NOT_VALID;
      UtlErrorHwnd( ERROR_FILENAME_NOT_VALID, MB_CANCEL, 1, &pszFromPath, EQF_ERROR, HWND_FUNCIF );
    } /* endif */
  } /* endif */

  // check import mode
  if ( fOK )
  {
    pIda->usImpMode = ((lOptions & XLIFF_OPT) != 0L) ? FOL_XLIFF_FORMAT : FOL_TMGR_FORMAT;
  } /* endif */

  // check target drive
  if ( fOK )
  {
    if ( chToDrive != EOS )
    {
      CHAR szDrives[MAX_DRIVELIST];     // buffer for drive letters

      // get valid drives
      UtlGetCheckedEqfDrives( szDrives );

      /* check if specified drive is in list of valid drives            */
      if ( strchr( szDrives, toupper(chToDrive) ) == NULL )
      {
        CHAR szDrive[MAX_DRIVE];

        fOK = FALSE;
        szDrive[0] = chToDrive;
        szDrive[1] = COLON;
        szDrive[2] = EOS;
        pszParm = szDrive;
        ulRC = ERROR_EQF_DRIVE_NOT_VALID;
        UtlErrorHwnd( ERROR_EQF_DRIVE_NOT_VALID, MB_CANCEL, 1,
                      &pszParm, EQF_ERROR, HWND_FUNCIF );
      } /* endif */
    }
    else
    {
      // No target drive specified, use system drive as default
      CHAR szDrive[MAX_DRIVE];
      UtlQueryString( QST_PRIMARYDRIVE, szDrive, MAX_DRIVE );
      chToDrive = szDrive[0];
    } /* endif */
  } /* endif */

  // check new folder name
  if ( (pszNewFolderName != NULL) && (*pszNewFolderName != EOS) )
  {
    if ( !UtlCheckLongName( pszNewFolderName ) )
    {
      fOK = FALSE;
      pszParm = pszNewFolderName;
      UtlErrorHwnd( ERROR_INV_LONGNAME, MB_CANCEL, 1, &pszParm, EQF_ERROR, HWND_FUNCIF );
    } /* endif */
  } /* endif */

  // check existence of package file                                 */
  if ( fOK )
  {
    CHAR   szTempPath[MAX_LONGPATH];

    //UtlMakeEQFPath( szTempPath, chFromDrive, EXPORT_PATH,
    //                NULL );

    strcpy( szTempPath, pszFromPath );
    strcat( szTempPath, BACKSLASH_STR );
    strcat( szTempPath, pszFolderName );

    if ( pIda->usImpMode == FOL_TMGR_FORMAT )
    {
      strcat( szTempPath, FOLDEREXPORT_EXT );
    }
    else
    {
      strcat( szTempPath, FOLDEREXPORTXLIFF_EXT );
      strcpy( pIda->szXliffFileName, szTempPath );
    } /* endif */

    if ( !UtlFileExist( szTempPath ) )
    {
      fOK = FALSE;
      pszParm = szTempPath;
      ulRC = ERROR_FILE_ACCESS_ERROR;
      UtlErrorHwnd( ERROR_FILE_ACCESS_ERROR, MB_CANCEL, 1, &pszParm, EQF_ERROR, HWND_FUNCIF );
    } /* endif */
  } /* endif */

  // Fill IDA fields and prepare the package for import
  if ( fOK )
  {
    BOOL        fIsNew = FALSE;

    pData->pvFolderImportIda = pIda;

    pIda->chFromDrive = pszFromPath[0];
    {
      ObjLongToShortName( pszFolderName, pIda->szSourceFolName, FOLDER_OBJECT, &fIsNew );
      strcpy( pIda->szLongFolName, pszFolderName );
    }
    strcpy( pIda->szSourceFolder, pIda->szSourceFolName);
    strcat( pIda->szSourceFolder, EXT_FOLDER_MAIN );
    strcpy( pIda->szObjPath, pszFolderName );
    strcat( pIda->szObjPath, FOLDEREXPORT_EXT );
    if ( (pszNewFolderName != NULL) && (*pszNewFolderName != EOS) )
    {
      strcpy( pIda->szTargetFolName, pszNewFolderName );
    }
    else
    {
      strcpy( pIda->szTargetFolName, pszFolderName );
    } /* endif */
    pIda->szNewMemNames[0] = EOS;
    if ( pszNewMemNames != NULL )  
    {
      strcpy( pIda->szNewMemNames, pszNewMemNames );
    } /* endif */

    ANSITOOEM(pIda->szTargetFolName);
    OBJLONGTOSHORTSTATE ObjState;

    // create target short name and ensure that the new short name is reserved and may not be reused by other processes
    ObjLongToShortNameEx2( pIda->szTargetFolName, EOS, pIda->szTargetShortFolName, FOLDER_OBJECT, &ObjState, TRUE, &pIda->fReserved );

    if ( pIda->usImpMode == FOL_TMGR_FORMAT )
    {
#ifdef SESSIONLOG
      UtlLogWriteString2( "ImportFolder: Opening package %s\\%s", pszFromPath, pIda->szObjPath );
#endif
      pIda->pPackCtrl = UtlPackOpenPackage( pIda->chFromDrive, NULL, pszFromPath,
                                            pIda->szObjPath,
                                            HWND_FUNCIF );

      // read package's user header
      if ( pIda->pPackCtrl )            // if the package is open ...
      {
        ULONG ulHeaderSize = UtlPackQueryHeaderSize( pIda->pPackCtrl );
        if ( ulHeaderSize )
        {
          fOK = UtlAlloc( (PVOID *)&pIda->pHeader, 0L, (LONG) ulHeaderSize, NOMSG );
          if ( fOK )
          {
            ULONG ulRC = UtlPackReadHeader( pIda->pPackCtrl,
                                            (PBYTE) pIda->pHeader,
                                            ulHeaderSize );
            if ( ulRC != PACK_OK_RC )
            {
              // read of header failed
              fOK = FALSE;
              UtlAlloc( (PVOID *)&pIda->pHeader, 0L, 0L, NOMSG );
              UtlPackClosePackage( pIda->pPackCtrl );
              pIda->pPackCtrl = NULL;
            } /* endif */
          }
          else
          {
            // alloc failed
            ulRC = ERROR_STORAGE;
            UtlErrorHwnd( ERROR_STORAGE, MB_CANCEL, 0, NULL, EQF_ERROR, HWND_FUNCIF );
            UtlPackClosePackage( pIda->pPackCtrl );
            pIda->pPackCtrl = NULL;
          } /* endif */
        }
        else
        {
          // file is no valid folder package
          fOK = FALSE;
          pszParm = pIda->szObjPath;
          ulRC = ERROR_FILE_INVALID_DATA;
          UtlErrorHwnd( ERROR_FILE_INVALID_DATA, MB_CANCEL, 1, &pszParm, EQF_ERROR, HWND_FUNCIF );
        } /* endif */
      }
      else
      {
        fOK = FALSE;         // error: package did not open
        pszParm = pIda->szObjPath;
        ulRC = ERROR_FILE_INVALID_DATA;
        UtlErrorHwnd( ERROR_FILE_INVALID_DATA, MB_CANCEL, 1, &pszParm, EQF_ERROR, HWND_FUNCIF );
      } /* endif */

      // build list of additional tag table files contained in folder package
      if ( fOK )
      {
        fOK = FolImpBuildAddTagTableFilesList( pIda );
      } /* endif */
    } /* endif */

    // check if folder exists somethere and adjust TO drive
    {
      PPROPFOLDER pFolProp;               // ptr to folder properties
      PVOID       hFolProp;               // handle of folder properties
      EQFINFO     ErrorInfo;              // error code of property handler calls

      //UtlMakeEQFPath( pIda->szTargetFolObj, NULC, SYSTEM_PATH, NULL );
      UtlMakeEQFPath( pIda->szTargetFolObj, (CHAR)toupper(chToDrive), SYSTEM_PATH, NULL ); // changed 20032001 bt
      strcat( pIda->szTargetFolObj, BACKSLASH_STR );
      strcpy( pIda->szTargetFolder, pIda->szTargetShortFolName );
      strcat( pIda->szTargetFolder, EXT_FOLDER_MAIN );
      strcat( pIda->szTargetFolObj, pIda->szTargetFolder );

      // ignore reserved short name dummy property file 
      if ( !pIda->fReserved )
      {
        hFolProp = OpenProperties( pIda->szTargetFolObj, NULL,
                                   PROP_ACCESS_READ,
                                   &ErrorInfo);
        if ( hFolProp )
        {
          pFolProp = (PPROPFOLDER)MakePropPtrFromHnd( hFolProp );
          if ( pFolProp->chDrive == NULC )
          {
            chToDrive = pIda->szTargetFolObj[0];
          }
          else
          {
            chToDrive = pFolProp->chDrive;
          } /* endif */
          CloseProperties( hFolProp, PROP_QUIT, &ErrorInfo );
        } /* endif */
      } /* endif */
    } /* endif */

    // check if folder has been locked by another process
    if ( fOK )
    {
      SHORT sRC = QUERYSYMBOL( pIda->szTargetFolObj );
      if ( sRC != -1 )
      {
        fOK = FALSE;
        pszParm = pIda->szSourceFolName;
        ulRC = ERROR_FOLDER_LOCKED;
        UtlErrorHwnd( ERROR_FOLDER_LOCKED, MB_CANCEL, 1,
                      &pszParm, EQF_INFO, HWND_FUNCIF );
      } /* endif */
    } /* endif */

    // if o.k. remember settings
    if ( fOK )
    {
      pIda->fWithMarkup= (lOptions & NOMARKUP_UPDATE_OPT) == 0L;
      pIda->fWithDict = (lOptions & WITHDICT_OPT) != 0L;
      pIda->fWithMem  = (lOptions & WITHMEM_OPT) != 0L;
      pIda->chToDrive = (CHAR)toupper(chToDrive);
      pIda->NextTask = LOADFILES;
      if ( pIda->usImpMode == FOL_TMGR_FORMAT )
      {
        UtlPackSetOwnerHwnd( pIda->pPackCtrl, HWND_FUNCIF );
      }
      else
      {
        fOK = UtlAllocHwnd( (PVOID *)&(pIda->pXLIFFData), 0, sizeof(FOLIMPEXPXLIFF), ERROR_STORAGE, HWND_FUNCIF );
        if ( fOK )
        {
          strcat( pIda->pXLIFFData->szExportName, pIda->szXliffFileName );
          strcpy( pIda->pXLIFFData->szTargetFolObj, pIda->szTargetFolObj );
          strcpy( pIda->pXLIFFData->szTargetFolder, pIda->szTargetFolName );
          pIda->pXLIFFData->fWithFolderMem = pIda->fWithMem;
          pIda->pXLIFFData->hwndErrMsg = HWND_FUNCIF;
        } /* endif */
      } /* endif */
      pIda->hwndErrMsg = HWND_FUNCIF;
    } /* endif */

    // If o.k. set/reset skip flags of package files
    if ( fOK && (pIda->usImpMode == FOL_TMGR_FORMAT) )
    {
      PFILELIST   pFileList;         // ptr to package's file list
      USHORT      usNoOfEntries;     // # of enteries in package's file list
      PFILELISTENTRY pFile;          // ptr for file list processing

      ULONG  ulRC = UtlPackGetFileListPtr( pIda->pPackCtrl, &pFileList );
      fOK = UtlPackHandleError( ulRC, pIda->szSourceFolName, HWND_FUNCIF );
      if ( fOK )
      {
        usNoOfEntries = (USHORT)pFileList->ulListUsed;
        pFile         = pFileList->pEntries;
        while ( usNoOfEntries )
        {
          pFile->usProcessFlags &= ~FILE_SKIP;

          if ( pIda->pHeader->BitFlags.fHeaderType == RELEASE0_HEADER )
          {
            if ( ( pFile->usFileType == PROP_CLASS_TAGTABLE ) ||
                 ( pFile->usFileType == PROP_CLASS_MEMORY ) )
            {
              if ( !pIda->fWithMem )
              {
                pFile->usProcessFlags |= FILE_SKIP;
              } /* endif */
            }
            else if ( pFile->usFileType == PROP_CLASS_DICTIONARY )
            {
              if ( !pIda->fWithDict )
              {
                pFile->usProcessFlags |= FILE_SKIP;
              } /* endif */
            } /* endif */
          }
          else
          {
            if ( ( pFile->usFileType == MEMORY_PROP_FILE ) ||
                 ( pFile->usFileType == MEMORY_DATA_FILE ) ||
                 ( pFile->usFileType == MEMORY_TABLE_FILE ) )
            {
              if ( !pIda->fWithMem )
              {
                pFile->usProcessFlags |= FILE_SKIP;
              } /* endif */
            }
            else if ( ( pFile->usFileType == DICTIONARY_PROP_FILE ) ||
                      ( pFile->usFileType == DICTIONARY_DATA_FILE ) ||
                      ( pFile->usFileType == DICTIONARY_INDEX_FILE ) )
            {
              if ( !pIda->fWithDict )
              {
                pFile->usProcessFlags |= FILE_SKIP;
              } /* endif */
            } /* endif */
          } /* endif */

          usNoOfEntries--;            // skip to next entry in file list
          pFile++;
        } /* endwhile */


        // count number of objects being imported
        usNoOfEntries = (USHORT)pFileList->ulListUsed;
        pFile         = pFileList->pEntries;
        pIda->usMaxObjects = 0;
        while ( usNoOfEntries )
        {
          if ( !(pFile->usProcessFlags & FILE_SKIP) )
          {
            switch( pFile->usFileType )
            {

              case DOCUMENT_PROP_FILE:
              case TAGTABLE_DATA_FILE:
//              case NTMMEMORY_OLD_PROP:
              case MEMORY_PROP_FILE:
              case DICTIONARY_PROP_FILE:
              case FOLDER_PROP_FILE:
                pIda->usMaxObjects++;
                break;
              default:
                break;
            } /* endswitch */
          } /* endif */
          usNoOfEntries--;            // skip to next entry in file list
          pFile++;
        } /* endwhile */

      } /* endif */
      if ( !fOK ) ulRC = UtlQueryUShort( QS_LASTERRORMSGID );
    } /* endif */

    // if o.k. check free space on target drive
    if ( fOK && (pIda->usImpMode == FOL_TMGR_FORMAT) )
    {
      ULONG ulRequiredSpace;         // required space for package

      ulRC = UtlPackQueryRequiredSpace( pIda->pPackCtrl, &ulRequiredSpace );
      fOK = UtlPackHandleError( ulRC, pIda->szSourceFolName, HWND_FUNCIF );
      if ( fOK )
      {
        ULONG64 lFreeSpace = UtlQueryFreeSpace( pIda->chToDrive, FALSE );
        if ( lFreeSpace < (ULONG64)ulRequiredSpace )
        {
          CHAR   szDrive[3];       // buffer for drive string
          PSZ    pErrParms[2];

          pErrParms[0] = pIda->szSourceFolName;
          szDrive[0] = pIda->chToDrive;
          szDrive[1] = EOS;
          pErrParms[1] = szDrive;
          UtlErrorHwnd( ERROR_FOLIMP_NOSPACE, MB_CANCEL, 2, pErrParms, EQF_ERROR, HWND_FUNCIF );
          ulRC = UtlQueryUShort( QS_LASTERRORMSGID );
          fOK = FALSE;
        } /* endif */
      } /* endif */
      if ( !fOK ) ulRC = UtlQueryUShort( QS_LASTERRORMSGID );
    } /* endif */


    // if o.k. lock folder
    if ( fOK )
    {
      SETSYMBOL( pIda->szTargetFolObj );
    } /* endif */

    if ( fOK && (pIda->usImpMode == FOL_TMGR_FORMAT) )
    {
      UtlPackSetTargetDrive( pIda->pPackCtrl, pIda->chToDrive );
    } /* endif */
  } /* endif */

  // cleanup
  if ( !fOK )
  {
    if ( pIda )
    {
      // delete any reserved short name placeholder
      if ( pIda->fReserved )
      {
        UtlMakeEQFPath( pIda->szTargetFolObj, NULC, SYSTEM_PATH, NULL ); 
        strcat( pIda->szTargetFolObj, BACKSLASH_STR );
        strcat( pIda->szTargetFolObj, pIda->szTargetShortFolName );
        strcat( pIda->szTargetFolObj, EXT_FOLDER_MAIN );
        DeleteFile( pIda->szTargetFolObj );
        pIda->fReserved = FALSE;
      }

      if ( pIda->pPackCtrl )            // is there an open package ???
      {
        UtlPackClosePackage( pIda->pPackCtrl ); // ... close it
        pIda->pPackCtrl = NULL;
      } /* endif */
      if ( pIda->pszBuffer ) UtlAlloc( (PVOID *)&(pIda->pszBuffer), 0L, 0L, NOMSG) ;
      UtlAlloc( (PVOID *)&(pIda), 0L, 0L, NOMSG) ;
    } /* endif */
  }
  else
  {
    pData->fComplete = FALSE;         // import is not complete yet
  } /* endif */

  return( (USHORT)ulRC );
} /* end of function FolFuncPrepImportFolder */

// perform the actual folder import in non-DDE function call mode
USHORT FolFuncImportFolderProcess
(
PFCTDATA    pData                    // function I/F session data
)
{
  PFOLIMPIDA      pIda;               // folder import dialog IDA
  USHORT          usRC = NO_ERROR;    // function return code

  pIda = (PFOLIMPIDA)pData->pvFolderImportIda;
  pIda->objCurProcess[0] = EOS;

  if ( pIda->usImpMode == FOL_TMGR_FORMAT )
  {
    switch ( pIda->NextTask )
    {
      case LOADFILES:
        {
          ULONG ulPackRC = 0;

          // build unique import directory if not done yet
          if ( pIda->szImportDir[0] == EOS )
          {
            FolImpBuildUniqueImportPath( pIda );
            UtlPackSetImportDir( pIda->pPackCtrl, pIda->szImportDir + 1 );
          } /* endif */               

          ulPackRC = UtlPackReadPackage( pIda->pPackCtrl );
          switch ( ulPackRC )
          {
            case PACK_OK_RC:
              //--- continue in reading package ---
              break;
            case PACK_COMPLETE_RC:
#ifdef SESSIONLOG
              UtlLogWrite( "ImportFolder: Package file loaded" );
#endif
              pIda->NextTask = PROCESSTABLES;
              break;
            default:
              pIda->NextTask = IMPORTCANCEL;
              break;
          } /* endswitch */
        }
        break;

      case PROCESSTABLES:
        if ( pIda->fWithMarkup )
        {
          BOOL fOK = FolImpProcessTables( HWND_FUNCIF, pIda );
          pIda->NextTask = ( fOK ) ? PROCESSFOLDER : IMPORTCANCEL ;
        }
        else
        {
          pIda->NextTask = PROCESSFOLDER;
        } /* endif */           
        break;

      case PROCESSFOLDER:
        FolImpProcessFolder( HWND_FUNCIF, pIda );
        if ( pIda->usProcessedObjects <= pIda->usMaxObjects ) pIda->usProcessedObjects++;
        break;

      case PROCESSSUBFOLDERS:
        FolImpProcessSubFolders( HWND_FUNCIF, pIda );
        break;


      case PROCESSDOCS:
        {
          BOOL        fOK = TRUE; // internal OK flag

          //--- make path for document objects ---
          UtlMakeEQFPath( pIda->szObjPath, pIda->chToDrive,
                          SYSTEM_PATH,
                          pIda->szSourceFolder );
          UtlAddImportToPath( pIda, pIda->szObjPath );

          //--- make path for document properties ---
          UtlMakeEQFPath( pIda->szSearchPath, pIda->chToDrive,
                          PROPERTY_PATH,
                          pIda->szSourceFolder );
          UtlAddImportToPath( pIda, pIda->szSearchPath );
          strcat( pIda->szSearchPath, BACKSLASH_STR );
          strcat( pIda->szSearchPath, DEFAULT_PATTERN );

          //--- build list of imported documents ---
          fOK = FolImpBuildObjList( pIda->szSearchPath,
                                    pIda->szObjPath,
                                    &pIda->pszBuffer,
                                    &pIda->ulBufSize );

#ifdef SESSIONLOG
          UtlLogWriteString( "ImportFolder: Processing documents in %s", pIda->szSearchPath );
#endif
          if ( fOK )
          {
            // build document processing and info table
            if ( fOK )
            {
              BOOL fUserInput = FALSE;
              fOK = FolImpPreProcessDocs( pIda, &fUserInput );
              if ( !fOK )
              {
                pIda->NextTask = IMPORTCANCEL;
              }
            } /* endif */


            // process documents in document processing table
            if ( fOK )
            {
              fOK = FolImpProcessDocs( pIda );
            } /* endif */
            pIda->NextTask = ( fOK ) ? PROCESSDICTIONARY : IMPORTCANCEL;
          }
          else
          {
            // no documents to process, continue with tag tables
            pIda->NextTask = PROCESSDICTIONARY;
          } /* endif */
        }
        break;

      case PROCESSNEXTMEMORY:
        {
          BOOL        fOK = TRUE; // internal OK flag

          if ( pIda->fWithMem )
          {
            BOOL fContinue = FALSE;
            BOOL fMoreMemories = FALSE;
            fOK = FolImpProcessNextMemory( pIda, &fMoreMemories, &fContinue );
            if ( !fOK )
            {
              pIda->NextTask = IMPORTCANCEL;
            }
            else if ( fContinue )
            {
              pIda->NextTask = PROCESSMEMORY;
            }
            else if ( fMoreMemories )
            {
              pIda->NextTask = PROCESSNEXTMEMORY;

              // notify OpenTM2 GUI to add memory information
               HWND hwnd = FindWindow( TWBMAIN ,NULL);
               if(hwnd != NULL)
               {
                 PMEMIMPORTDATA pMemData = (PMEMIMPORTDATA)pIda->pvMemImportData; 
                 std::string fullname = pMemData->pMemPlugin->getName();
                 fullname = fullname+":";
                 fullname = fullname+pMemData->strMemoryName;
                 
                 COPYDATASTRUCT cds;
                 cds.dwData = 1;
                 cds.cbData = (fullname.size()+1)*sizeof(char);
                 cds.lpData =  (char*)fullname.c_str();
                 SendMessage(hwnd,WM_COPYDATA, MP1FROMCHAR(pIda->chToDrive),(LPARAM)(LPVOID)&cds);
               }
               //notify end
            }
            else
            {
              pIda->NextTask = IMPORTOK;
            } /* end */                   
          }
          else
          {
            pIda->NextTask = IMPORTOK;
          } /* endif */
        }
        break;

      case PROCESSMEMORY:
        {
          BOOL        fOK = TRUE;            // internal OK flag

          if ( pIda->fWithMem )
          {
            BOOL fContinue = FALSE;
            fOK = FolImpContinueMemoryImport( pIda, &fContinue );
            if ( !fOK )
            {
              pIda->NextTask = IMPORTCANCEL;
            }
            else if ( fContinue )
            {
              pIda->NextTask = PROCESSMEMORY;
            }
            else
            {
              pIda->NextTask = PROCESSNEXTMEMORY;
            } /* endif */
          }
          else
          {
            pIda->NextTask = IMPORTOK;
          } /* endif */
        }
        break;

      case PROCESSDICTIONARY:
        {
          BOOL        fOK = TRUE; // internal OK flag

          if ( pIda->fWithDict )
          {
            //make path for local dictionary objects
            UtlMakeEQFPath( pIda->szObjPath, pIda->chToDrive,
                            DIC_PATH, pIda->szTargetFolder );
            UtlAddImportToPath( pIda, pIda->szObjPath );

            //make path for dictionaries
            UtlMakeEQFPath( pIda->szSearchPath, pIda->chToDrive,
                            DIC_PATH, pIda->szTargetFolder );
            UtlAddImportToPath( pIda, pIda->szSearchPath );
            strcat( pIda->szSearchPath, BACKSLASH_STR );
            strcat( pIda->szSearchPath, DEFAULT_PATTERN_NAME );
            strcat( pIda->szSearchPath, EXT_OF_DIC );

            //build object for first imported dictionary
            fOK = FolImpBuildObject( pIda->szSearchPath,
                                    pIda->szObjPath,
                                    pIda->pszBuffer,
                                    pIda->ulBufSize,
                                    pIda->szCurFile );


            /****************************************************/
            /* if no local dictionary was found try remote ones */
            /****************************************************/
            if ( !fOK )
            {
              //make path for local dictionary objects
              UtlMakeEQFPath( pIda->szObjPath, pIda->chToDrive,
                              COMDICT_PATH, pIda->szTargetFolder );
              UtlAddImportToPath( pIda, pIda->szObjPath );

              //make path for dictionaries
              UtlMakeEQFPath( pIda->szSearchPath, pIda->chToDrive,
                              COMDICT_PATH, pIda->szTargetFolder );
              UtlAddImportToPath( pIda, pIda->szSearchPath );
              strcat( pIda->szSearchPath, BACKSLASH_STR );
              strcat( pIda->szSearchPath, DEFAULT_PATTERN_NAME );
              strcat( pIda->szSearchPath, EXT_OF_DIC );

              //build object for first imported dictionary
              fOK = FolImpBuildObject( pIda->szSearchPath,
                                      pIda->szObjPath,
                                      pIda->pszBuffer,
                                      pIda->ulBufSize,
                                      pIda->szCurFile );
            } /* endif */

            /*****************************************************/
            /* if no remote dictionary was found try shared ones */
            /*****************************************************/
            if ( !fOK )
            {
              //make path for shared dictionary objects
              UtlMakeEQFPath( pIda->szObjPath, pIda->chToDrive,
                              DIC_PATH, pIda->szTargetFolder );
              UtlAddImportToPath( pIda, pIda->szObjPath );

              //make path for dictionaries
              UtlMakeEQFPath( pIda->szSearchPath, pIda->chToDrive,
                              DIC_PATH, pIda->szTargetFolder );
              UtlAddImportToPath( pIda, pIda->szSearchPath );
              strcat( pIda->szSearchPath, BACKSLASH_STR );
              strcat( pIda->szSearchPath, DEFAULT_PATTERN_NAME );
              strcat( pIda->szSearchPath, EXT_OF_SHARED_DIC );

              //build object for first imported dictionary
              fOK = FolImpBuildObject( pIda->szSearchPath,
                                      pIda->szObjPath,
                                      pIda->pszBuffer,
                                      pIda->ulBufSize,
                                      pIda->szCurFile );
            } /* endif */

          }
          else
          {
            fOK = FALSE;         // do not import dictionaries
          } /* endif */

          if ( fOK )
          {
            pIda->NextTask = PROCESSNEXTDICTIONARY;
            strcpy( pIda->objCurProcess, DICIMP_PROC_OBJPREFIX );
            strcat( pIda->objCurProcess, pIda->szCurFile );
            DictionaryImport( HWND_FUNCIF, pIda->pszBuffer, TRUE,
                              HWND_FUNCIF );
          }
          else
          {
            //call up next process
            pIda->NextTask = PROCESSNEXTMEMORY;
          } /* endif */
        }
        break;

      case PROCESSNEXTDICTIONARY:
        {
          BOOL        fOK = TRUE; // internal OK flag

          if ( pIda->fWithDict )
          {
            /****************************************************/
            /* Delete last processed dictionary if it has not   */
            /* delete by the dictionary handler                 */
            /****************************************************/
            if ( UtlFileExist( pIda->szCurFile ) )
            {
              UtlDelete( pIda->szCurFile, 0L, FALSE );
            } /* endif */

            if ( pIda->usProcessedObjects <= pIda->usMaxObjects ) pIda->usProcessedObjects++;

            //make path for local dictionary objects
            UtlMakeEQFPath( pIda->szObjPath, pIda->chToDrive,
                            DIC_PATH, pIda->szTargetFolder );
            UtlAddImportToPath( pIda, pIda->szObjPath );

            //make path for dictionaries
            UtlMakeEQFPath( pIda->szSearchPath, pIda->chToDrive,
                            DIC_PATH, pIda->szTargetFolder );
            UtlAddImportToPath( pIda, pIda->szSearchPath );
            strcat( pIda->szSearchPath, BACKSLASH_STR );
            strcat( pIda->szSearchPath, DEFAULT_PATTERN_NAME );
            strcat( pIda->szSearchPath, EXT_OF_DIC );

            //build object for first imported dictionary
            fOK = FolImpBuildObject( pIda->szSearchPath,
                                    pIda->szObjPath,
                                    pIda->pszBuffer,
                                    pIda->ulBufSize,
                                    pIda->szCurFile );


            /****************************************************/
            /* if no local dictionary was found try remote ones */
            /****************************************************/
            if ( !fOK )
            {
              //make path for local dictionary objects
              UtlMakeEQFPath( pIda->szObjPath, pIda->chToDrive,
                              COMDICT_PATH, pIda->szTargetFolder );
              UtlAddImportToPath( pIda, pIda->szObjPath );

              //make path for dictionaries
              UtlMakeEQFPath( pIda->szSearchPath, pIda->chToDrive,
                              COMDICT_PATH, pIda->szTargetFolder );
              UtlAddImportToPath( pIda, pIda->szSearchPath );
              strcat( pIda->szSearchPath, BACKSLASH_STR );
              strcat( pIda->szSearchPath, DEFAULT_PATTERN_NAME );
              strcat( pIda->szSearchPath, EXT_OF_DIC );

              //build object for first imported dictionary
              fOK = FolImpBuildObject( pIda->szSearchPath,
                                      pIda->szObjPath,
                                      pIda->pszBuffer,
                                      pIda->ulBufSize,
                                      pIda->szCurFile );
            } /* endif */

            /*****************************************************/
            /* if no remote dictionary was found try shared ones */
            /*****************************************************/
            if ( !fOK )
            {
              //make path for shared dictionary objects
              UtlMakeEQFPath( pIda->szObjPath, pIda->chToDrive,
                              DIC_PATH, pIda->szTargetFolder );
              UtlAddImportToPath( pIda, pIda->szObjPath );

              //make path for dictionaries
              UtlMakeEQFPath( pIda->szSearchPath, pIda->chToDrive,
                              DIC_PATH, pIda->szTargetFolder );
              UtlAddImportToPath( pIda, pIda->szSearchPath );
              strcat( pIda->szSearchPath, BACKSLASH_STR );
              strcat( pIda->szSearchPath, DEFAULT_PATTERN_NAME );
              strcat( pIda->szSearchPath, EXT_OF_SHARED_DIC );

              //build object for first imported dictionary
              fOK = FolImpBuildObject( pIda->szSearchPath,
                                      pIda->szObjPath,
                                      pIda->pszBuffer,
                                      pIda->ulBufSize,
                                      pIda->szCurFile );
            } /* endif */
          }
          else
          {
            fOK = FALSE;         // do not import dictionaries
          } /* endif */

          if ( fOK )
          {
            strcpy( pIda->objCurProcess, DICIMP_PROC_OBJPREFIX );
            strcat( pIda->objCurProcess, pIda->szCurFile );
            DictionaryImport( HWND_FUNCIF, pIda->pszBuffer, TRUE,
                              HWND_FUNCIF );
          }
          else
          {
            //call up next process
            pIda->NextTask = PROCESSNEXTMEMORY;
          } /* endif */
        }
        break;

      case IMPORTOK:
        {
          /*******************************************************/
          /* If a folder has no languages set and a memory       */
          /* has been imported for this folder try to set        */
          /* the folder languages from the memory (indirect      */
          /* by forcing a refresh of the folder listbox for      */
          /* this folder)                                        */
          /*******************************************************/
          if ( pIda->fNoFolderLanguage && pIda->fWithMem )
          {
            UtlMakeEQFPath( pIda->szObjPath, pIda->chToDrive,
                            SYSTEM_PATH,
                            pIda->szTargetFolder );
            ObjBroadcast( WM_EQFN_PROPERTIESCHANGED,
                          PROP_CLASS_FOLDER,
                          pIda->szObjPath );
          } /* endif */
          if ( pIda->szImportDir[0]!= EOS )
          {
            UtlMakeEQFPath( pIda->szObjPath, pIda->chToDrive, SYSTEM_PATH, NULL );
            strcat( pIda->szObjPath, pIda->szImportDir );
            if ( UtlDirExist( pIda->szObjPath ) )
            {
              UtlRemoveDir( pIda->szObjPath, FALSE );
            } /* endif */
          }
          else
          {
            UtlMakeEQFPath( pIda->szObjPath, pIda->chToDrive,
                            IMPORT_PATH,
                            NULL );
            UtlRemoveDir( pIda->szObjPath, FALSE );
          } /* endif */           
          pIda->usImportError = 0;
          pIda->NextTask = CLEANUP;
        }
        break;

      case IMPORTCANCEL:
        {
          pIda->usImportError = UtlQueryUShort( QS_LASTERRORMSGID );
          if ( pIda->szImportDir[0]!= EOS )
          {
            UtlMakeEQFPath( pIda->szObjPath, pIda->chToDrive, SYSTEM_PATH, NULL );
            strcat( pIda->szObjPath, pIda->szImportDir );
            if ( UtlDirExist( pIda->szObjPath ) )
            {
              UtlRemoveDir( pIda->szObjPath, FALSE );
            } /* endif */
          }
          else
          {
            UtlMakeEQFPath( pIda->szObjPath, pIda->chToDrive,
                            IMPORT_PATH,
                            NULL );
            UtlRemoveDir( pIda->szObjPath, FALSE );
          } /* endif */           
          pIda->NextTask = CLEANUP;
        }
        break;

      case CLEANUP:
        // cleanup
        if ( pIda->pPackCtrl )
        {
          UtlPackCleanup( pIda->pPackCtrl );
          pIda->pPackCtrl = NULL;
        } /* endif */
        //--- discard all files and directories under x:\EQF\IMPORT ---
        if ( pIda->szImportDir[0]!= EOS )
        {
          UtlMakeEQFPath( pIda->szObjPath, pIda->chToDrive, SYSTEM_PATH, NULL );
          strcat( pIda->szObjPath, pIda->szImportDir );
          if ( UtlDirExist( pIda->szObjPath ) )
          {
            UtlRemoveDir( pIda->szObjPath, FALSE );
          } /* endif */
        }
        else
        {
          UtlMakeEQFPath( pIda->szObjPath, pIda->chToDrive,
                          IMPORT_PATH,
                          NULL );
          UtlRemoveDir( pIda->szObjPath, FALSE );
        } /* endif */           
        REMOVESYMBOL( pIda->szTargetFolObj );

        if ( pIda != NULL )
        {
          usRC = pIda->usImportError;
          if ( pIda->pDocData != NULL )
          {
            UtlAlloc( (PVOID *)&pIda->pDocData, 0L, 0L, NOMSG );
          } /* endif */
          if ( pIda->pszBuffer ) UtlAlloc( (PVOID *)&pIda->pszBuffer, 0L, 0L, NOMSG );
          if ( pIda->pImpSubFolInfo ) UtlAlloc( (PVOID *)&pIda->pImpSubFolInfo, 0L, 0L, NOMSG );
        } /* endif */
        UtlAlloc( (PVOID *)&pIda, 0L, 0L, NOMSG );
        pData->fComplete = TRUE;
#ifdef SESSIONLOG
        UtlLogWrite( "ImportFolder: Processing complete" );
#endif
        break;
    } /* endswitch */
    if ( pData->fComplete )
    {
      pData->usProgress = 100;
    }
    else if ( pIda )
    {
      pData->usProgress = (USHORT)(pIda->usProcessedObjects * 100 / pIda->usMaxObjects);
    } /* endif */
  }
  else
  {
     //  Import the XLIFF folder.
    int iRC = -1;
    typedef int (* __cdecl FOLXLIFFIMPORT_DCL)( PFOLIMPEXPXLIFF ) ;

    HMODULE hXLIFF = LoadLibrary("EQFXLIFF.DLL");
    if (hXLIFF != NULL) {
      FOLXLIFFIMPORT_DCL pfnFOLXLIFFIMPORT = (FOLXLIFFIMPORT_DCL)GetProcAddress(hXLIFF, "FOLXLIFFIMPORT");
      if ( pfnFOLXLIFFIMPORT != NULL )
         iRC = (*pfnFOLXLIFFIMPORT)( pIda->pXLIFFData );
      if (hXLIFF)
          FreeLibrary(hXLIFF);
    }

    if ( iRC == FOL_EXPORT_IMPORT_COMPLETE )
    {
      pData->fComplete = TRUE;
      pData->usProgress = 100;
      REMOVESYMBOL( pIda->szTargetFolObj );
      if ( pIda != NULL )
      {
        if ( pIda->pXLIFFData != NULL )
        {
          UtlAlloc( (PVOID *)&pIda->pXLIFFData, 0L, 0L, NOMSG );
        } /* endif */
        UtlAlloc( (PVOID *)&pIda, 0L, 0L, NOMSG );
      } /* endif */
    }
    else if ( iRC == 0 )
    {
      pData->usProgress = pIda->pXLIFFData->usComplete;
    }
    else
    {
      pData->fComplete = TRUE;
      pData->usProgress = 100;
      REMOVESYMBOL( pIda->szTargetFolObj );
      usRC = UtlQueryUShort( QS_LASTERRORMSGID );
      if ( pIda != NULL )
      {
        if ( pIda->pXLIFFData != NULL )
        {
          UtlAlloc( (PVOID *)&pIda->pXLIFFData, 0L, 0L, NOMSG );
        } /* endif */
        UtlAlloc( (PVOID *)&pIda, 0L, 0L, NOMSG );
      } /* endif */
    } /* endif */
  } /* endif */

  return( usRC );
} /* end of function FolFuncImportFolderProcess */



USHORT FolRefreshFolderListbox
(
HWND hwndDlg,
PFOLIMPIDA pIda                      // ptr to folder import IDA
)
{
  PVOID       pPackCtrl;
  CHAR        chPackName[MAX_FILESPEC];         // buffer for package names
  PSZ         pszTemp = NULL;
  ULONG       ulRC = 0;

  DELETEALL( hwndDlg, ID_FOLIMP_FOLDER_LB );
  strcpy( chPackName, DEFAULT_PATTERN_NAME );
  strcat( chPackName, FOLDEREXPORT_EXT );
  ENABLEUPDATE_FALSE( hwndDlg, ID_FOLIMP_FOLDER_LB );
  SETCURSOR( SPTR_WAIT );

  // clear any open packages
  {
      //--- handle any previously processed packages ---
    if ( pIda->pPackCtrl )            // is there an open package ???
    {
      UtlPackClosePackage( pIda->pPackCtrl ); // ... close it
      pIda->pPackCtrl = NULL;
    } /* endif */
    if ( pIda->pHeader )              // is there a package header???
    {
      UtlAlloc( (PVOID *)&pIda->pHeader, 0L, 0L, NOMSG ); // ... discard it
      pIda->pHeader = NULL;
    } /* endif */
  }

  // get package short names or list XLIFF files
  DELETEALLHWND( pIda->hwndShortNameLB );
  if ( pIda->usImpMode == FOL_TMGR_FORMAT )
  {
    switch ( pIda->usDialogType )
    {
        case FOLDER_IMPORT:
          ulRC = UtlPackListPackages( pIda->hwndShortNameLB,
                                      pIda->chFromDrive,
                                      NULL, pIda->szImportPath, chPackName, 0 );
          SETTEXT( hwndDlg, ID_FOLIMP_FILENAME_CB, pIda->szImportPath );
          pszTemp = pIda->szImportPath;
          break;
        case FOLDER_DELETE:
          ulRC = UtlPackListPackages( pIda->hwndShortNameLB,
                                      pIda->chFromDrive,
                                      NULL, pIda->szString, chPackName, PACKFLAG_LISTALL );
          pszTemp = pIda->szString;
          break;
        default:
          break;
      } /* endswitch */
  }
  else
  {
    SETTEXT( hwndDlg, ID_FOLIMP_FILENAME_CB, pIda->szImportPath );
    ListXLiffFolder( pIda, hwndDlg, pIda->szImportPath, "*.XLF" );
    ListXLiffFolder( pIda, hwndDlg, pIda->szImportPath, "*.XLIFF" );
  } /* endif */

  //@bt
  if (ulRC != 0 && ulRC != ERROR_FILE_NOT_FOUND)
  {
    // invalid path

    UtlErrorHwnd( ERROR_FILENAME_NOT_VALID, MB_CANCEL, 1, &pszTemp, EQF_ERROR, pIda->hwndErrMsg );

    SETFOCUS( hwndDlg, ID_FOLIMP_FILENAME_CB );
  }

  // get the long names for the folders from the folder package header
  if ( pIda->usImpMode == FOL_TMGR_FORMAT )
  {
    SHORT sItem = QUERYITEMCOUNTHWND( pIda->hwndShortNameLB );
    while ( sItem-- )
    {
      SHORT sInsertItem;           // index of inserted item

      // get the folder short name
      QUERYITEMTEXTHWND( pIda->hwndShortNameLB, sItem, pIda->szShortFolName );
      strcpy( pIda->szObjPath, pIda->szShortFolName );
      strcat( pIda->szObjPath, FOLDEREXPORT_EXT );
      pIda->szLongFolName[0] = EOS;

      pPackCtrl = UtlPackOpenPackage( pIda->chFromDrive, NULL, pszTemp, pIda->szObjPath, hwndDlg );

      if ( pPackCtrl )
      {
        PPROPFOLDER pProp = NULL;

        // setup name of folder property file
        UtlMakeEQFPath(  pIda->szObjPath, NULC, PROPERTY_PATH, NULL );
        strcat( pIda->szObjPath, BACKSLASH_STR );
        strcat( pIda->szObjPath, pIda->szShortFolName );
        strcat( pIda->szObjPath, EXT_FOLDER_MAIN );

        UtlPackReadFilePart( pPackCtrl, pIda->szObjPath,
                             (PBYTE)&(pProp->szLongName) - (PBYTE)pProp,
                             (PBYTE)pIda->szLongFolName,
                             sizeof(pIda->szLongFolName) );

        // close package
        UtlPackClosePackage( pPackCtrl ); // ... close it
        pPackCtrl = NULL;
      } /* endif */

      // add long name or short folder name to listbox
      {
        PSZ pszName = (pIda->szLongFolName[0] != EOS) ? pIda->szLongFolName : pIda->szShortFolName;
        sInsertItem = INSERTITEM( hwndDlg, ID_FOLIMP_FOLDER_LB, pszName );
      }

      // set item handle to index of folder in invisible folder lb
      if ( sInsertItem >= 0 )
      {
        SETITEMHANDLE( hwndDlg, ID_FOLIMP_FOLDER_LB, sInsertItem, sItem );
      } /* endif */
    } /* endwhile */
  } /* endif */
  SETCURSOR( SPTR_ARROW );
  ENABLEUPDATE_TRUE( hwndDlg, ID_FOLIMP_FOLDER_LB );

  // set correct width
  {
    HWND hwndLB;

    hwndLB = GetDlgItem(hwndDlg, ID_FOLIMP_FOLDER_LB);
    UtlSetHorzScrollingForLB(hwndLB);
  }
  return( NO_ERROR );
} /* end of function FolRefreshFolderListbox */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// PROPERTY SHEET///////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//------------------------------------------------------------------------------
// Internal function                                                            
//------------------------------------------------------------------------------
// Function name:     FolImpPropInit                                            
//------------------------------------------------------------------------------
// Function call:     FlxpPropertySheetNotifiaction( hwnd, mp1, mp2);           
//------------------------------------------------------------------------------
// Description:       handle changes on the tab page                            
//------------------------------------------------------------------------------
// Parameters:        HWND hwnd   handle of the dialog                          
//                    WPARAM  mp1    message parameter 1                        
//                    LPARAM  mp2    message parameter 2                        
//------------------------------------------------------------------------------
// Returncode type:   MRESULT                                                   
//------------------------------------------------------------------------------
// Returncodes:       return code from default window proc or FALSE             
//------------------------------------------------------------------------------
// Function flow:     switch ( pNMHdr->code )                                   
//                      case TCN_SELCHANGE:                                     
//                        activate new page                                     
//                      case TCN_SELCHANGING                                    
//                        hide the dialog                                       
//                    return                                                    
//------------------------------------------------------------------------------



MRESULT FolImpPropInit
(
HWND    hwndDlg,                    // handle of folder export dialog window
WPARAM  mp1,                        // first parameter of WM_INITDLG
LPARAM  mp2                         // second parameter of WM_INITDLG
)
{
  MRESULT     mResult = MRFROMSHORT(TRUE); // result of message processing
  CHAR        szDrives[MAX_DRIVELIST]; // list of available drives
  PFOLIMPIDA  pIda;                   // ptr to folder import IDA
  EQFINFO     ErrorInfo;              // error code of property handler calls
  PPROPFOLDERLIST pFllProp;           // ptr to folder list properties
  PPROPSYSTEM pSysp;                  // ptr to system properties



  mp1 = mp1;                         // suppress 'unreferenced parameter' msg

  //--- create IDA and store pointer to it ---
  pIda = (PFOLIMPIDA) mp2;
  ANCHORDLGIDA( hwndDlg, pIda );

  if ( pIda )
  {
    // create listbox for folder short names
    pIda->hwndShortNameLB = WinCreateWindow( hwndDlg, WC_LISTBOX, "", 0,
                                             0, 0, 0, 0,
                                             hwndDlg, HWND_TOP, 1, NULL, NULL );
    //--- fill in IDA fields ---
    pIda->chToDrive = ' ';
    pIda->chFromDrive = ' ';

    //--- do import option specific processing
    switch ( pIda->usDialogType )
    {
      case FOLDER_IMPORT:
        // nothing to do: title bar text and button text are correct
        break;
      case FOLDER_DELETE:


        break;
      default:
        break;
    } /* endswitch */

    //--- access folder list properties (for last used values) ---

    // GQ: code de-activated as sometimes the wrong object name was
    //     returned, use default folder list name instead
    // pIda->hwndFll = EqfQueryActiveFolderlistHwnd();
    // EqfQueryObjectName( pIda->hwndFll, pIda->szFllObjName );
    UtlMakeEQFPath( pIda->szFllObjName, NULC, SYSTEM_PATH, NULL );
    strcat( pIda->szFllObjName, BACKSLASH_STR );
    strcat( pIda->szFllObjName, DEFAULT_FOLDERLIST_NAME );
    pIda->hFllProp = OpenProperties( pIda->szFllObjName,
                                     NULL,
                                     PROP_ACCESS_READ,
                                     &ErrorInfo );

    UtlGetDriveList( (PBYTE)szDrives );
    //GS TODO
//    ENABLECTRL( hwndDlg, ID_FOLIMP_INFO_PB, FALSE );
//    ENABLECTRL( hwndDlg, ID_FOLIMP_IMPORT_PB, FALSE );
//    ENABLECTRL( hwndDlg, ID_FOLIMP_NEWNAME_CB, FALSE );
//    SETTABSTOP( hwndDlg, ID_FOLIMP_CANCEL_PB );

    //--- create To drive buttons if required
    pFllProp = (PPROPFOLDERLIST)MakePropPtrFromHnd( pIda->hFllProp );
    switch ( pIda->usDialogType )
    {
      case FOLDER_IMPORT:
        //--- create to drive buttons for 'To' drives ---
        pSysp = GetSystemPropPtr();
        UtlGetCheckedEqfDrives( pIda->szToDriveList );
        //--- sort drive letters alphabetically ---
        qsort( (void *)pIda->szToDriveList,
               strlen(pIda->szToDriveList),
               sizeof(CHAR), cCompare );
        pIda->chSysDrive = pSysp->szPrimaryDrive[0];


        break;
      case FOLDER_DELETE:
        // do nothing as no 'To' drives are required
        break;
    } /* endswitch */


    // fill format combobox and select last used format
    if ( pIda->usDialogType == FOLDER_IMPORT )
    {
      FolFillFolderFormatCombobox( pIda->hwndDlg, ID_FOLIMP_FORMAT_CB, pFllProp->usFolImpFormat );
      pIda->usImpMode = pFllProp->usFolImpFormat;
    }
    else
    {
      HIDECONTROL( pIda->hwndDlg, ID_FOLIMP_FORMAT_TEXT );
      HIDECONTROL( pIda->hwndDlg, ID_FOLIMP_FORMAT_CB );
    } /* endif */

    //-----------------------------------------
    //--- apply last used for 'From' drives ---
    //-----------------------------------------
    pFllProp = (PPROPFOLDERLIST)MakePropPtrFromHnd( pIda->hFllProp );

    if ( pFllProp->fImportPath && strchr( szDrives, toupper(pFllProp->szImportPath[0]) ) )
    {
      // check if last-used drive is ready
      ULONG64 lFreeSpace = UtlQueryFreeSpace( pFllProp->szImportPath[0], FALSE );
      if ( lFreeSpace != (ULONG64)-1L )
      {

        pIda->chFromDrive = pFllProp->szImportPath[0];
        strcpy(pIda->szImportPath, pFllProp->szImportPath);

        FolImpCommand( hwndDlg,
                       (SHORT)(IDFROMDRIVE( ID_FROMDRIVEA_BTN, pIda->chFromDrive )),
                       BN_CLICKED );

      }
      else
      {
        pIda->chFromDrive = ' ';      // drive is not avialable
      } /* endif */
    }
    else
    {
      pIda->chFromDrive = ' ';
    } /* endif */

    FolRefreshFolderListbox( hwndDlg, pIda );
    if ( QUERYITEMCOUNT( hwndDlg, ID_FOLIMP_FOLDER_LB ) == 0 )
    {
      // gs TODO (these buttons can not be disabled here)
//      ENABLECTRL( hwndDlg, ID_FOLIMP_INFO_PB, FALSE );
//      ENABLECTRL( hwndDlg, ID_FOLIMP_IMPORT_PB, FALSE );
//      SETTABSTOP( hwndDlg, ID_FOLIMP_CANCEL_PB );
    }
    else
    {
      SELECTITEMMS( hwndDlg, ID_FOLIMP_FOLDER_LB, 0 );
    } /* endif */


  } /* endif */

  //SETFOCUS( hwndDlg, ID_FROMDRIVEA_BTN );
  SETFOCUS( hwndDlg, ID_FOLIMP_FILENAME_CB  );
  return( mResult );
} /* end of FolImpPropInit */

//------------------------------------------------------------------------------
// Internal function                                                            
//------------------------------------------------------------------------------
// Function name:     FolImpPropertySheetNotification                           
//------------------------------------------------------------------------------
// Function call:     FlxpPropertySheetNotifiaction( hwnd, mp1, mp2);           
//------------------------------------------------------------------------------
// Description:       handle changes on the tab page                            
//------------------------------------------------------------------------------
// Parameters:        HWND hwnd   handle of the dialog                          
//                    WPARAM  mp1    message parameter 1                        
//                    LPARAM  mp2    message parameter 2                        
//------------------------------------------------------------------------------
// Returncode type:   MRESULT                                                   
//------------------------------------------------------------------------------
// Returncodes:       return code from default window proc or FALSE             
//------------------------------------------------------------------------------
// Function flow:     switch ( pNMHdr->code )                                   
//                      case TCN_SELCHANGE:                                     
//                        activate new page                                     
//                      case TCN_SELCHANGING                                    
//                        hide the dialog                                       
//                    return                                                    
//------------------------------------------------------------------------------
MRESULT FolImpPropertySheetNotification
(
HWND hwnd,
WPARAM  mp1,
LPARAM  mp2
)
{
  NMHDR * pNMHdr;
  ULONG       ulTabCtrl;
  MRESULT      mResult = FALSE;
  PFOLIMPIDA     pIda;
  pNMHdr = (LPNMHDR)mp2;

  mp1;
  switch ( pNMHdr->code )
  {
    case TCN_SELCHANGE:
      pIda = ACCESSDLGIDA(hwnd, PFOLIMPIDA);
      if ( pIda )
      {
        TC_ITEM Item;
        HWND hwndTabCtrl = GetDlgItem( hwnd, ID_FOLIMP_PROP_TABCTRL );
        ulTabCtrl = TabCtrl_GetCurSel( hwndTabCtrl );
        memset( &Item, 0, sizeof(Item) );
        Item.mask = TCIF_PARAM;
        TabCtrl_GetItem( hwndTabCtrl, ulTabCtrl, &Item );
        ulTabCtrl = Item.lParam;
        ShowWindow( pIda->hwndPages[ ulTabCtrl ], SW_SHOW );
      } /* endif */
      break;
    case TCN_SELCHANGING:
      pIda = ACCESSDLGIDA( hwnd, PFOLIMPIDA );
      if ( pIda )
      {
        /**************************************************************/
        /* Issue a direct call to the appropriate dialog proc with    */
        /* WM_COMMAND, ID_TB_PROP_SET_PB and the second parameter set */
        /* to 1L to force only consistency checking                   */
        /**************************************************************/
        TC_ITEM Item;
        PFNWP pfnWp;
        HWND hwndTabCtrl = GetDlgItem( hwnd, ID_FOLIMP_PROP_TABCTRL );
        ulTabCtrl = TabCtrl_GetCurSel( hwndTabCtrl );
        memset( &Item, 0, sizeof(Item) );
        Item.mask = TCIF_PARAM;
        TabCtrl_GetItem( hwndTabCtrl, ulTabCtrl, &Item );
        ulTabCtrl = Item.lParam;
        pfnWp = (PFNWP) GetWindowLong( pIda->hwndPages[ ulTabCtrl ], DWL_DLGPROC );

        mResult = pfnWp( pIda->hwndPages[ulTabCtrl], WM_COMMAND,
                         PID_PB_OK, 1L);
        if ( mResult )
        {
          /************************************************************/
          /* stick on the side                                        */
          /* we have to post the request again since one of the system*/
          /* routines thinks that we still want to change the page..  */
          /************************************************************/
          WinPostMsg( hwnd, TCM_SETCURSEL, ulTabCtrl, 0L );
        } /* endif */
        ShowWindow( pIda->hwndPages[ ulTabCtrl ], SW_HIDE );
      } /* endif */
      break;
    case TTN_NEEDTEXT:
      {
        TOOLTIPTEXT *pToolTipText = (TOOLTIPTEXT *) mp2;
        if ( pToolTipText )
        {
          HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
          TC_ITEM Item;
          HWND hwndTabCtrl = GetDlgItem( hwnd, ID_FOLIMP_PROP_TABCTRL );
          memset( &Item, 0, sizeof(Item) );
          Item.mask = TCIF_PARAM;
          TabCtrl_GetItem( hwndTabCtrl, pToolTipText->hdr.idFrom, &Item );
          switch ( (SHORT)Item.lParam )
          {
            case 0:      // first page
              LOADSTRING( hab, hResMod, IDS_FOLIMPPROP_TAB_GENERAL,
                          pToolTipText->szText );

            case 1:      // first page
              LOADSTRING( hab, hResMod, IDS_FOLIMPPROP_TAB_DETAILS,
                          pToolTipText->szText );

              break;

          } /* endswitch */
        } /* endif */
      }
      break;
    default:
      break;
  } /* endswitch */
  return mResult;
} /* end of function FolImpPropertySheetNotification */






//------------------------------------------------------------------------------
// Internal function                                                            
//------------------------------------------------------------------------------
// Function name:     FolImpPropertySheetLoad                                   
//------------------------------------------------------------------------------
// Function call:     CntPropertySheetLoad( hwnd, mp2 );                        
//------------------------------------------------------------------------------
// Description:       handle changes on the tab page                            
//------------------------------------------------------------------------------
// Parameters:        HWND hwnd   handle of the dialog                          
//                    LPARAM  mp2    message parameter 2                        
//------------------------------------------------------------------------------
// Returncode type:   MRESULT                                                   
//------------------------------------------------------------------------------
// Returncodes:       return code from default window proc or FALSE             
//------------------------------------------------------------------------------
// Function flow:     create any pages,                                         
//                    load the tabctrl text                                     
//                    load the (modeless) dialog, register it and position into 
//                      tab area                                                
//                    return                                                    
//------------------------------------------------------------------------------
BOOL FolImpPropertySheetLoad
(
HWND hwnd,
PFOLIMPIDA     pIda
)
{
  BOOL      fOk = TRUE;
  TC_ITEM   TabCtrlItem;
  USHORT    nItem = 0;
  HWND      hwndTabCtrl;
  HINSTANCE hInst;
  CHAR      szBuffer[80];
  HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);

  if ( fOk )
  {
    RECT rect;
    // remember adress of user area
    hInst = GETINSTANCE( hwnd );
    hwndTabCtrl = GetDlgItem( hwnd, ID_FOLIMP_PROP_TABCTRL );
    pIda->hwndTabCtrl = hwndTabCtrl;
    GetClientRect( hwndTabCtrl, &rect );
    TabCtrl_AdjustRect( hwndTabCtrl, FALSE, &rect );

    // leave some additional space at top
    rect.top += 20;
    MapWindowPoints( hwndTabCtrl, hwnd, (POINT *) &rect, 2 );


    TabCtrlItem.mask = TCIF_TEXT | TCIF_PARAM;

    // -----------------------------------------------------------------
    //
    // create the appropriate TAB control and load the associated dialog
    //
    // -----------------------------------------------------------------

    //
    // IDS_FOLIMPPROP_TAB_GENERAL
    //

    LOADSTRING( hab, hResMod, IDS_FOLIMPPROP_TAB_GENERAL , szBuffer );
    TabCtrlItem.pszText = szBuffer;
    TabCtrlItem.lParam = nItem;
    SendMessage(hwndTabCtrl, TCM_INSERTITEM, nItem, (LPARAM)&TabCtrlItem);
    pIda->hwndPages[nItem] =
    CreateDialogParam( hInst,
                       MAKEINTRESOURCE( ID_FOLIMPPROP_GENERAL_DLG ),
                       hwnd,
                       FOLIMPPROP_GENERAL_DLGPROC,
                       (LPARAM)pIda );

    SetWindowPos( pIda->hwndPages[nItem], HWND_TOP,
                  rect.left, rect.top,
                  rect.right-rect.left, rect.bottom-rect.top, SWP_SHOWWINDOW );
    SetFocus( pIda->hwndPages[nItem] );
    UtlRegisterModelessDlg( pIda->hwndPages[nItem] );
    nItem++;


    //
    // IDS_FOLIMPPROP_TAB_DETAILS
    //

    LOADSTRING( hab, hResMod, IDS_FOLIMPPROP_TAB_DETAILS , szBuffer );
    TabCtrlItem.pszText = szBuffer;
    TabCtrlItem.lParam = nItem;
    SendMessage(hwndTabCtrl, TCM_INSERTITEM, nItem, (LPARAM)&TabCtrlItem);
    pIda->hwndPages[nItem] =
    CreateDialogParam( hInst,
                       MAKEINTRESOURCE( ID_FOLIMPPROP_DETAILS_DLG ),
                       hwnd,
                       FOLIMPPROP_DETAILS_DLGPROC,
                       (LPARAM)pIda );

    SetWindowPos( pIda->hwndPages[nItem], HWND_TOP,
                  rect.left, rect.top,
                  rect.right-rect.left, rect.bottom-rect.top, SWP_SHOWWINDOW );
    SetFocus( pIda->hwndPages[nItem] );
    UtlRegisterModelessDlg( pIda->hwndPages[nItem] );
    nItem++;



  } /* endif */


  // -----------------------------------------------------------------
  //
  // hide all dialog pages but the first one
  //
  // -----------------------------------------------------------------

  if ( fOk )
  {
    int i = 1;
    while ( pIda->hwndPages[i] )
    {
      ShowWindow( pIda->hwndPages[i], SW_HIDE );
      i++;
    } /* endwhile */
  } /* endif */

  if ( !fOk )
  {
    POSTEQFCLOSE( hwnd, FALSE );
  } /* endif */

  return fOk;
}




//------------------------------------------------------------------------------
// Internal function                                                            
//------------------------------------------------------------------------------
// Function name:     FolImpPropCommand                                         
//------------------------------------------------------------------------------
// Function call:                                                               
//------------------------------------------------------------------------------
// Description:       Handle WM_COMMAND message of property sheet dialog        
//------------------------------------------------------------------------------
// Parameters:        HWND hwnd   handle of the dialog                          
//                    WPARAM  mp1    message parameter 1                        
//                    LPARAM  mp2    message parameter 2                        
//------------------------------------------------------------------------------
// Returncode type:   MRESULT                                                   
//------------------------------------------------------------------------------
// Returncodes:       return code from default window proc or FALSE             
//------------------------------------------------------------------------------
MRESULT FolImpPropCommand
(
HWND hwnd,
WPARAM mp1,
LPARAM mp2
)
{
  BOOL fOk = TRUE;
  INT     nItem;
  MRESULT mResult = MRFROMSHORT(TRUE);
  PFOLIMPIDA pIda;                    // ptr to folder export IDA

  SHORT sNotification= WMCOMMANDCMD( mp1,mp2 );

// --- get IDA pointer ---
  pIda = ACCESSDLGIDA( hwnd, PFOLIMPIDA );
  mp2;

  switch ( WMCOMMANDID( mp1, mp2 ) )
  {
    case ID_FOLIMP_FILENAME_CB:
      if ( sNotification == CBN_SELCHANGE )
      {
      }
      break;

    case ID_FOLIMP_HELP_PB:
      UtlInvokeHelp();
      break;

    case ID_FOLINFO_PRINT_PB:
      if ( pIda && pIda->pPackCtrl )
      {
        FolImpInfoPrintDetails( pIda, hwnd );
      } /* endif */
      break;



    case ID_FOLIMP_IMPORT_PB:
      {


        pIda = ACCESSDLGIDA( hwnd, PFOLIMPIDA );


        // issue command to all active dialog pages
        nItem = 0;
        while ( pIda->hwndPages[nItem] && fOk )
        {
          PFNWP pfnWp = (PFNWP) GetWindowLong( pIda->hwndPages[ nItem ],
                                               DWL_DLGPROC );

          switch ( nItem )
          {
            // general  settings
            case 0:
              fOk =  !pfnWp( pIda->hwndPages[nItem], WM_COMMAND,
                             PID_PB_OK, 0L);
              break;

          } /* endswitch */
          nItem++;
        } /* endwhile */

        if ( pIda->fOk && pIda->szTargetFolObj[0] )
        {
          // prepare data for folder import in XLIFF format
          if ( pIda->usImpMode == FOL_XLIFF_FORMAT )
          {
            pIda->fOk = UtlAlloc( (PVOID *)&(pIda->pXLIFFData), 0, sizeof(FOLIMPEXPXLIFF), ERROR_STORAGE );
            if ( pIda->fOk )
            {
              strcpy( pIda->pXLIFFData->szExportName, pIda->szXliffFileName );
              strcpy( pIda->pXLIFFData->szTargetFolObj, pIda->szTargetFolObj );
              strcpy( pIda->pXLIFFData->szTargetFolder, pIda->szTargetFolName );
              pIda->pXLIFFData->fWithFolderMem = pIda->fWithMem;
            } /* endif */
          } /* endif */
        } /* endif */

        //--- if o.k. lock folder and remove dialog ---
        if (pIda->fOk )
        {
          SETSYMBOL( pIda->szTargetFolObj );
          POSTCLOSE( hwnd, TRUE );
        } /* endif */
        break;
      }
      break;
    case ID_FOLIMP_CANCEL_PB:
    case DID_CANCEL:

      UtlPackClosePackage( pIda->pPackCtrl );
      POSTEQFCLOSE( hwnd, FALSE );
      break;

    case ID_FOLIMP_FORMAT_CB:
      if ( sNotification == CBN_SELCHANGE )
      {
        int item = CBQUERYSELECTION( pIda->hwndDlg, ID_FOLIMP_FORMAT_CB );
        if ( item != -1 )
        {
          pIda->usImpMode = (USHORT)CBQUERYITEMHANDLE( pIda->hwndDlg, ID_FOLIMP_FORMAT_CB, item );
          if ( pIda->usImpMode == FOL_TMGR_FORMAT)
          {
            ENABLECTRL( pIda->hwndPages[0], ID_FOLIMP_RENAME_CHK, TRUE );
          }
          else
          {
            ENABLECTRL( pIda->hwndPages[0], ID_FOLIMP_WITHDICT_CHK, FALSE );
            SETCHECK_FALSE( pIda->hwndPages[0], ID_FOLIMP_WITHDICT_CHK );
            ENABLECTRL( pIda->hwndPages[0], ID_FOLIMP_WITHMEM_CHK, TRUE );
            SETCHECK_FALSE( pIda->hwndPages[0], ID_FOLIMP_WITHMEM_CHK );
            ENABLECTRL( pIda->hwndPages[0], ID_FOLIMP_RENAME_CHK, FALSE  );
            SETCHECK_FALSE( pIda->hwndPages[0], ID_FOLIMP_RENAME_CHK );
            ENABLECTRL( pIda->hwndPages[0], ID_FOLIMP_NEWNAME_CB, FALSE );
            CBDELETEALL( pIda->hwndPages[0], ID_FOLIMP_NEWNAME_CB );
            SETTEXT( pIda->hwndPages[0], ID_FOLIMP_NEWNAME_CB, "" );
          } /* endif */
          FolRefreshFolderListbox( pIda->hwndPages[0], pIda );

        } /* endif */
      } /* endif */
      break;

    default:
      mResult = WinDefDlgProc( hwnd, WM_COMMAND, mp1, mp2 );
      break;
  } /* endswitch */

  return( mResult );
} /* end of FolImpPropCommand */




//------------------------------------------------------------------------------
// Internal function FOLIMPPROPDLGPROC                                          
//------------------------------------------------------------------------------
// Function name:                                                               
//------------------------------------------------------------------------------
// Function call:                                                               
//------------------------------------------------------------------------------
// Description:                                                                 
//------------------------------------------------------------------------------
// Parameters:                                                                  
//                                                                              
//------------------------------------------------------------------------------
// Returncode type:                                                             
//------------------------------------------------------------------------------
// Returncodes:                                                                 
//------------------------------------------------------------------------------
// Function flow:                                                               
//                                                                              
//                                                                              
//                                                                              
//                                                                              
//------------------------------------------------------------------------------


INT_PTR CALLBACK FOLIMPPROPDLGPROC
(
HWND hwnd,                       /* handle of dialog window             */
WINMSG msg,
WPARAM mp1,
LPARAM mp2
)
{
  MRESULT  mResult = MRFROMSHORT( TRUE );                // result value of procedure
  PFOLIMPIDA pIda;                      // dialog instance data area
  BOOL     fOk = TRUE;                       // internal O.K. flag

  switch ( msg )
  {
    case WM_EQF_QUERYID: HANDLEQUERYID( ID_FOLIMPPROP_DLG, mp2 ); break;

    case WM_INITDLG:
      // get the export IDA
      pIda = (PFOLIMPIDA) mp2;

      if (!ANCHORDLGIDA (hwnd, pIda))
      {
        fOk = FALSE; // set fOk to FALSE to stop dialog

        // display error message system error
        UtlError (0, MB_CANCEL, 0, NULL, SYSTEM_ERROR);
        // post message to close dialog, do not start report instance
        WinPostMsg (hwnd, WM_EQF_CLOSE, MP1FROMSHORT(FALSE), NULL);
      }


      if ( !fOk )
      {
        //--- close analysis dialog, FALSE means: - do not start analysis instance
        //Close_proc( hwnd, FALSE );
      }
      else
      {
        pIda->hwndDlg = hwnd;
        FolImpPropertySheetLoad( hwnd, pIda );
      } /* endif */

      mResult = DIALOGINITRETURN( mResult );


      break;


    case DM_GETDEFID:
/************************************************************/
/* check if user pressed the ENTER key, but wants only to   */
/* select/deselect an item of the listbox via a simulated   */
/* (keystroke) double click.                                */
/************************************************************/
      {

        pIda = ACCESSDLGIDA (hwnd, PFOLIMPIDA);  // get access to IDA

        if ( GetKeyState(VK_RETURN) & 0x8000 )
        {
          int nItem = 0;
          PFNWP pfnWp = (PFNWP) GetWindowLong( pIda->hwndPages[ nItem ],
                                               DWL_DLGPROC );

          fOk =  !pfnWp( pIda->hwndPages[nItem], WM_COMMAND, DM_GETDEFID
                         , 0L);

          mResult = pIda->fUpdated;


        } /* end if*/


      }
      break;


    case WM_COMMAND:
      mResult = FolImpPropCommand( hwnd, mp1, mp2 );
      break;

    case WM_NOTIFY:

      mResult = FolImpPropertySheetNotification( hwnd, mp1, mp2 );
      break;

    case WM_HELP:
      /*************************************************************/
      /* pass on a HELP_WM_HELP request                            */
      /*************************************************************/
      EqfDisplayContextHelp( (HWND)((LPHELPINFO) mp2)->hItemHandle,
                             &hlpsubtblFolPropImpDlg[0] );

      mResult = TRUE;  // message processed
      break;

    case WM_CLOSE:
//-----------------------------------/
      mResult = FolImpClose( hwnd, mp1, mp2 );

      //mResult = FolExpClose( hwnd, mp1, mp2 );
      break;



    case WM_EQF_CLOSE :
      // mp1 : TRUE -> start Counting Report window
      // mp2 : not used

      pIda = ACCESSDLGIDA (hwnd, PFOLIMPIDA);  // get access to IDA


      mResult = FolImpClose( hwnd, mp1, mp2 );

      //mResult = FolExpClose( hwnd, mp1, mp2 );

      if (pIda)
      {
        ANCHORDLGIDA (hwnd, NULL);
      } // end if

      // destroy dialog pass flag in mp1, to tell the count handler if
      // count instance should be started or not
      // mp1 == FALSE : do not start count instance
      // mp1 == TRUE  : start count instance

      DelCtrlFont(hwnd, ID_FOLIMP_FILENAME_CB );

      WinDismissDlg (hwnd, SHORT1FROMMP1(mp1));
      break;


    default:
      mResult = WinDefDlgProc( hwnd, msg, mp1, mp2 );
      break;
  } /* endswitch */

  return mResult;
} /* end of FOLIMPPROPDLGPROC */




//------------------------------------------------------------------------------
// Internal function  FOLIMPPROP_GENERAL_DLGPROC                                
//------------------------------------------------------------------------------
// Function name:                                                               
//------------------------------------------------------------------------------
// Function call:                                                               
//------------------------------------------------------------------------------
// Description:                                                                 
//------------------------------------------------------------------------------
// Parameters:                                                                  
//                                                                              
//------------------------------------------------------------------------------
// Returncode type:                                                             
//------------------------------------------------------------------------------
// Returncodes:                                                                 
//------------------------------------------------------------------------------
// Function flow:                                                               
//                                                                              
//                                                                              
//                                                                              
//                                                                              
//------------------------------------------------------------------------------

INT_PTR CALLBACK FOLIMPPROP_GENERAL_DLGPROC
(
HWND hwnd,                       /* handle of dialog window             */
WINMSG msg,
WPARAM mp1,
LPARAM mp2
)
{
  MRESULT  mResult = MRFROMSHORT( TRUE );                // result value of procedure
  PFOLIMPIDA  pIda;
  BOOL    fOk=TRUE;
  PCHAR   pchDrive;
  SHORT   sItem;
  ULONG  ulHeaderSize;               // size of package's user header
  ULONG   ulRC;                       // retrun code of UtlPack functions

  BOOL        fOK = TRUE;                    // internal OK flag
  PSZ         pErrParms[4];           // error message parameter table
  CHAR        szDrive[3];             // buffer for drive string
  ULONG64     ulFreeSpace;             // # of bytes free on target disk
  ULONG       ulRequiredSpace;        // # of bytes required for import files
  SHORT       sRC;                    // short signed return code
  PFILELIST   pFileList;              // ptr to package's file list
  USHORT      usNoOfEntries;          // # of enteries in package's file list
  PFILELISTENTRY pFile;               // ptr for file list processing

  PPROPFOLDERLIST pFllProp;           // ptr to folder list properties
  PPROPSYSTEM pSysp;



  SHORT sNotification= WMCOMMANDCMD( mp1, mp2 );
  SHORT sId = WMCOMMANDID( mp1, mp2 );
  switch ( msg )
  {
    case WM_INITDLG:
      //-----------------------------------
      {
        mResult = FolImpPropInit( hwnd, mp1, mp2 );

        pIda = ACCESSDLGIDA (hwnd, PFOLIMPIDA);  // get access to IDA

        if ( QUERYITEMCOUNT( hwnd, ID_FOLIMP_FOLDER_LB ) != 0 )
        {
          SELECTITEMMS( hwnd, ID_FOLIMP_FOLDER_LB, 0 );
          SETFOCUS( hwnd, ID_FOLIMP_FILENAME_CB  );
        }
        else
        {
           FolImpImportPathChanged( hwnd, pIda );              /* 4-13-17 */
        }


        SetCtrlFnt(hwnd, GetCharSet(), ID_FOLIMP_FILENAME_CB, 0 );

        CBSETTEXTLIMIT( hwnd, ID_FOLIMP_FILENAME_CB, (MAX_LONGFILESPEC-1) );
        CBSETTEXTLIMIT( hwnd, ID_FOLIMP_NEWNAME_CB, (MAX_LONGFILESPEC-1) );

        UtlLoadLastUsedStrings( hwnd, ID_FOLIMP_FILENAME_CB, FOLIMPEXPLASTUSEDDIR );

          //--- create to drive buttons for 'To' drives ---
          pSysp = GetSystemPropPtr();
          UtlGetCheckedEqfDrives( pIda->szToDriveList );
          //--- sort drive letters alphabetically ---
          qsort( (void *)pIda->szToDriveList, strlen(pIda->szToDriveList), sizeof(CHAR), cCompare );
          pIda->chSysDrive = pSysp->szPrimaryDrive[0];
          pFllProp = (PPROPFOLDERLIST)MakePropPtrFromHnd( pIda->hFllProp );
          pIda->chOrgToDrive = pIda->chToDrive = DRIVEFROMID( ID_TODRIVEA_BTN, sId );

          {
            CHAR szBuffer[MAX_PATH+100];
            UINT nDrive=0;
            DWORD dwLogicalDrives=GetLogicalDrives();
            SHFILEINFO FileInfo;
            DWORD r ;
            USHORT ipos=0;
            USHORT npos=0;
            USHORT ifound=0;
            COMBOBOXEXITEM CBEItem;

            memset(&CBEItem, 0, sizeof(CBEItem));
            CBEItem.mask        = CBEIF_IMAGE | CBEIF_SELECTEDIMAGE | CBEIF_TEXT | CBEIF_LPARAM;
            CBEItem.cchTextMax  = 0;  // is ignored
            CBEItem.iItem       = -1; // insert at end

            CBDELETEALL(hwnd, ID_FOLIMP_TODRIVE_PB);
            if (dwLogicalDrives)
            {
              for (nDrive=0;nDrive <32;nDrive++)
              {
                if (dwLogicalDrives &(1 << nDrive))
                {
                  wsprintf(szBuffer,"%c:\\",nDrive+'A');
                  SetErrorMode( SEM_FAILCRITICALERRORS | SEM_NOOPENFILEERRORBOX );
                  r = SHGetFileInfo( szBuffer, 0, &FileInfo, sizeof( FileInfo ),
                                     SHGFI_DISPLAYNAME | SHGFI_SYSICONINDEX | SHGFI_SMALLICON );
                  SetErrorMode(0);
                  if ( r )
                  {
                    CHAR chDrive = (CHAR)(nDrive+'A');

                    // insert icon and string into list box
                    CBEItem.pszText = FileInfo.szDisplayName;
                    CBEItem.lParam  = ( LPARAM )szBuffer;
                    CBEItem.iSelectedImage  =
                    CBEItem.iImage          = FileInfo.iIcon; // index into system image list

                    if (strchr( pIda->szToDriveList, chDrive) )
                    {
                      if ( !pIda->fExists )
                      {
                        int iItem = CBINSERTITEMEND( hwnd, ID_FOLIMP_TODRIVE_PB, CBEItem.pszText);
                        CBSETITEMHANDLE( hwnd, ID_FOLIMP_TODRIVE_PB, iItem, nDrive );
                        npos++;
                      }
                      else if (chDrive == pIda->chToDrive)
                      {
                        int iItem = CBINSERTITEMEND( hwnd, ID_FOLIMP_TODRIVE_PB, CBEItem.pszText);
                        CBSETITEMHANDLE( hwnd, ID_FOLIMP_TODRIVE_PB, iItem, nDrive );
                        npos++;
                      }
                    }// end if
                  }// end if
                } // end for
              } // end construction

              // Set system drive
              for (ipos=0; ipos<npos;ipos=ipos+1)
              {
                int nDrive = CBQUERYITEMHANDLE( hwnd,ID_FOLIMP_TODRIVE_PB ,ipos );
                char chDrive = (CHAR)('A' + nDrive);
                if ( chDrive == pIda->chSysDrive)
                {
                  ifound=ipos;
                  pIda->chOrgToDrive = pIda->chToDrive = pIda->chSysDrive;
                }// end if
              } // end for

              CBSELECTITEM (hwnd,ID_FOLIMP_TODRIVE_PB ,ifound );
            }  // end if logical
          } // end construction
      }
      break;


    case WM_COMMAND:


      pIda = ACCESSDLGIDA (hwnd, PFOLIMPIDA);  // get access to IDA

      switch ( WMCOMMANDID( mp1, mp2 ) )
      {

        case DM_GETDEFID:
          /************************************************************/
          /* check if user pressed the ENTER key, but wants only to   */
          /* select/deselect an item of the listbox via a simulated   */
          /* (keystroke) double click.                                */
          /************************************************************/

          // Send clear information to Details folder
          //------------------------------------------
          {
            int nItem = 1;
            PFNWP pfnWp = (PFNWP) GetWindowLong( pIda->hwndPages[ nItem ],
                                                 DWL_DLGPROC );

            fOk =  !pfnWp( pIda->hwndPages[nItem], WM_COMMAND, ID_FOLIMP_CANCEL_PB
                           , 0L);
          } /* end */


          {
            if ( GetKeyState(VK_RETURN) & 0x8000 )
            {
              HWND hwndFocus = GetFocus();
              HWND hwndParent = GetParent( hwndFocus );
              HWND hwndCombo = GetDlgItem( hwnd, ID_FOLIMP_FILENAME_CB );
              if ( (hwndFocus == hwndCombo) || (hwndParent == hwndCombo) )
              {
                QUERYTEXT( hwnd, ID_FOLIMP_FILENAME_CB, pIda->szImportPath );
                FolImpImportPathChanged( hwnd, pIda );
                pIda->fUpdated = TRUE;
                mResult = TRUE;
              }
              else
              {
                mResult = TRUE;
                pIda->fUpdated = FALSE;
              }
            }
            else
            {
              mResult = TRUE;
              pIda->fUpdated = FALSE;
            } /* endif */

          }
          break;


        case ID_FOLIMP_FROMDRIVE_PB:
          {
            BROWSEINFO bi;
            LPITEMIDLIST pidlBrowse;    // PIDL selected by user

            // Fill in the BROWSEINFO structure.
            bi.hwndOwner = hwnd;
            bi.pidlRoot = NULL;

            // work on copy of actual import path as Win2K destroys our current buffer data
            strcpy( pIda->szBuffer, pIda->szImportPath );
            bi.pszDisplayName = pIda->szBuffer;

            //bi.pszDisplayName[0] = pIda->chFromDrive;
            bi.lpszTitle = "Select Import Directory";
            bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_USENEWUI;

            // if UtlBrowseForFolderCallbackProc is specified as callback the lParam
            // parameter must contain the initial folder directory!!!
            bi.lpfn = UtlBrowseForFolderCallbackProc;
            bi.lParam = (LPARAM)pIda->szImportPath;

            // Browse for a folder and return its PIDL.
            pidlBrowse = SHBrowseForFolder(&bi);

            if (pidlBrowse != NULL)
            {
              // get the selected directory path
              SHGetPathFromIDList( pidlBrowse, pIda->szImportPath );

              // Free the PIDL returned by SHBrowseForFolder.
              {
                LPMALLOC pMalloc;
                SHGetMalloc( &pMalloc );
                pMalloc->Free(pidlBrowse );
              }

              // activate selected path
              SETTEXT( hwnd, ID_FOLIMP_FILENAME_CB, pIda->szImportPath );
              //DocImpHandlePathInput( hwnd );

              FolImpImportPathChanged( hwnd, pIda );
            } /* endif */
          }
          break;

        case ID_FOLIMP_FILENAME_CB:
          if ( sNotification == CBN_SELCHANGE )
          {
            int ipos;
            CBQUERYSELECTEDITEMTEXT ( ipos, hwnd, ID_FOLIMP_FILENAME_CB, pIda->szImportPath  );
            FolImpImportPathChanged( hwnd, pIda );
            pIda->fUpdated = TRUE;
          }
          break;

        case ID_FOLIMP_RENAME_CHK:
          {
            pIda->fImportAs = QUERYCHECK( hwnd, ID_FOLIMP_RENAME_CHK );

            if ( pIda->fImportAs )
            {
              ENABLECTRL( hwnd, ID_FOLIMP_NEWNAME_CB, TRUE );
              EqfSend2Handler( FOLDERLISTHANDLER, WM_EQF_INSERTNAMES,
                               MP1FROMHWND( GETHANDLEFROMID( hwnd, ID_FOLIMP_NEWNAME_CB) ),
                               0L );
              SETTEXT( hwnd, ID_FOLIMP_NEWNAME_CB, pIda->szLongFolName );
             }
            else
            {
              ENABLECTRL( hwnd, ID_FOLIMP_NEWNAME_CB, FALSE );
              CBDELETEALL( hwnd, ID_FOLIMP_NEWNAME_CB );
              SETTEXT( hwnd, ID_FOLIMP_NEWNAME_CB, "" );

              // restore original to drive state for not renamed folder
              FolImpSetToDriveState( pIda, hwnd, pIda->szLongFolName );
            } /* endif */
          }
          break;

        case ID_FOLIMP_NEWNAME_CB:
          {

            switch ( sNotification )
            {
              case CBN_SELCHANGE :
              case CBN_EFCHANGE :
                {
                  pIda->fImportAs = QUERYCHECK( hwnd, ID_FOLIMP_RENAME_CHK);
                  if ( pIda->fImportAs )
                  {
                    //QUERYTEXT( hwnd, ID_FOLIMP_NEWNAME_CB, pIda->szLongBuffer );
                    //selection
                    {
                      int ipos;

                      CBQUERYSELECTEDITEMTEXT (ipos ,hwnd,
                                               ID_FOLIMP_NEWNAME_CB, pIda->szLongBuffer);
                    }
                    // manual input
                    if (pIda->szLongBuffer[0] == EOS)
                    {
                      QUERYTEXT( hwnd, ID_FOLIMP_NEWNAME_CB, pIda->szLongBuffer );
                    } // end if
                    UtlStripBlanks( pIda->szLongBuffer );
                    ANSITOOEM( pIda->szLongBuffer );
                    if ( pIda->usImpMode == FOL_XLIFF_FORMAT )
                    {
                      // cut-off extension from folder name
                      PSZ pszTemp = strrchr( pIda->szLongBuffer, '.' );
                      if ( pszTemp ) *pszTemp = EOS;
                    } /* endif */
                    FolImpSetToDriveState( pIda, hwnd, pIda->szLongBuffer );
                  } /* endif */
                }
                break;
            } /* endswitch */
          }
          break;

        case ID_FOLIMP_FOLDER_LB:
          switch ( sNotification )
          {
            case LN_SELECT:
              sItem = QUERYSELECTION( hwnd, ID_FOLIMP_FOLDER_LB );
              if ( pIda->usImpMode == FOL_TMGR_FORMAT )
              {
                if ( SendDlgItemMessage( hwnd, ID_FOLIMP_FOLDER_LB, LB_GETSELCOUNT, 0, 0 ) > 1)
                {
                  // more than one folder is selected ...

                  if ( pIda->pPackCtrl )            // is there an open package ???
                  {
                    UtlPackClosePackage( pIda->pPackCtrl ); // ... close it
                    pIda->pPackCtrl = NULL;
                  } /* endif */
                  if ( pIda->pHeader )              // is there a package header???
                  {
                    UtlAlloc( (PVOID *)&pIda->pHeader, 0L, 0L, NOMSG ); // ... discard it
                    pIda->pHeader = NULL;
                  } /* endif */

                  ENABLECTRL( pIda->hwndDlg, ID_FOLIMP_INFO_PB, FALSE );
                  ENABLECTRL( pIda->hwndDlg, ID_FOLIMP_IMPORT_PB, TRUE );
                  ENABLECTRL( pIda->hwndDlg, ID_FOLINFO_PRINT_PB, FALSE );
                  SETTABSTOP( hwnd, ID_FOLIMP_CANCEL_PB );
                  ENABLECTRL( hwnd, ID_FOLIMP_RENAME_CHK, FALSE );
                  ENABLECTRL( hwnd, ID_FOLIMP_NEWNAME_CB, FALSE );
                  CBDELETEALL( hwnd, ID_FOLIMP_NEWNAME_CB );
                  SETTEXT( hwnd, ID_FOLIMP_NEWNAME_CB, "" );
                  ENABLECTRL( hwnd, ID_FOLIMP_WITHDICT_CHK, TRUE );
                  SETCHECK_FALSE( hwnd, ID_FOLIMP_WITHDICT_CHK );
                  ENABLECTRL( hwnd, ID_FOLIMP_WITHMEM_CHK, TRUE );
                  SETCHECK_FALSE( hwnd, ID_FOLIMP_WITHMEM_CHK );
                }
                else
                if ( sItem != LIT_NONE )
                {
                  //--- handle any previously processed packages ---
                  if ( pIda->pPackCtrl )            // is there an open package ???
                  {
                    UtlPackClosePackage( pIda->pPackCtrl ); // ... close it
                    pIda->pPackCtrl = NULL;
                  } /* endif */
                  if ( pIda->pHeader )              // is there a package header???
                  {
                    UtlAlloc( (PVOID *)&pIda->pHeader, 0L, 0L, NOMSG ); // ... discard it
                    pIda->pHeader = NULL;
                  } /* endif */

                  //--- uncheck rename pushbutton
                  ENABLECTRL( hwnd, ID_FOLIMP_RENAME_CHK, TRUE );
                  SETCHECK_FALSE( hwnd, ID_FOLIMP_RENAME_CHK );
                  ENABLECTRL( hwnd, ID_FOLIMP_NEWNAME_CB, FALSE );
                  CBDELETEALL( hwnd, ID_FOLIMP_NEWNAME_CB );
                  SETTEXT( hwnd, ID_FOLIMP_NEWNAME_CB, "" );

                  //--- get package name and open the package ---
                  fOk = TRUE;             // assume everything is OK
                  // get folder short name from our invisible short name listbox
                  {
                    SHORT sShortIndex = (SHORT)QUERYITEMHANDLE( hwnd,
                                                                ID_FOLIMP_FOLDER_LB,
                                                                sItem );
                    QUERYITEMTEXT( hwnd, ID_FOLIMP_FOLDER_LB,
                                  sItem, pIda->szLongFolName );
                    QUERYITEMTEXTHWND( pIda->hwndShortNameLB, sShortIndex,
                                      pIda->szSourceFolName );
                  }
                  // UtlUpper( pIda->szSourceFolName );
                  strcpy( pIda->szObjPath, pIda->szSourceFolName );
                  strcat( pIda->szObjPath, FOLDEREXPORT_EXT );

                  //@bt

                  if ( pIda->usImpMode == FOL_TMGR_FORMAT )
                  {
                    pIda->pPackCtrl = UtlPackOpenPackage( pIda->chFromDrive, NULL, pIda->szImportPath,
                                                          pIda->szObjPath,
                                                          hwnd );

                    //--- read package's user header ---
                    if ( pIda->pPackCtrl )            // if the package is open ...
                    {
                      fOk = TRUE;
                      ulHeaderSize = UtlPackQueryHeaderSize( pIda->pPackCtrl );
                      if ( ulHeaderSize )
                      {
                        UtlAlloc( (PVOID *)&pIda->pHeader, 0L, (LONG) ulHeaderSize,
                                  ERROR_STORAGE );
                        if ( pIda->pHeader )
                        {
                          ulRC = UtlPackReadHeader( pIda->pPackCtrl,
                                                    (PBYTE) pIda->pHeader,
                                                    ulHeaderSize );
                          if ( ulRC == PACK_OK_RC )
                          {
                            //--- allow usage of INFO button ---

                            // Send update information to Details folder
                            //------------------------------------------


                            {
                              int nItem = 1;
                              PFNWP pfnWp = (PFNWP) GetWindowLong( pIda->hwndPages[ nItem ],
                                                                  DWL_DLGPROC );

                              fOk =  !pfnWp( pIda->hwndPages[nItem], WM_COMMAND, ID_FOLIMP_INFO_PB
                                            , 0L);
                            } /* end */


                            //----------
                            ENABLECTRL( pIda->hwndDlg, ID_FOLIMP_INFO_PB, TRUE );
                            ENABLECTRL( pIda->hwndDlg, ID_FOLIMP_IMPORT_PB, TRUE );
                            ENABLECTRL( pIda->hwndDlg, ID_FOLINFO_PRINT_PB, TRUE );
                            SETTABSTOP( pIda->hwndDlg, ID_FOLIMP_CANCEL_PB );

                            ENABLECTRL( hwnd, ID_FOLIMP_WITHDICT_CHK,
                                        pIda->pHeader->fContainsDict );
                            SETCHECK( hwnd, ID_FOLIMP_WITHDICT_CHK,
                                      pIda->pHeader->fContainsDict );
                            ENABLECTRL( hwnd, ID_FOLIMP_WITHMEM_CHK,
                                        pIda->pHeader->BitFlags.fContainsMem );
                            SETCHECK( hwnd, ID_FOLIMP_WITHMEM_CHK,
                                      pIda->pHeader->BitFlags.fContainsMem );
                          }
                          else
                          {
                            // read of header failed

                            //GS TODO
                            ENABLECTRL( pIda->hwndDlg, ID_FOLIMP_INFO_PB, FALSE );
                            ENABLECTRL( pIda->hwndDlg, ID_FOLIMP_IMPORT_PB, FALSE );
                            ENABLECTRL( pIda->hwndDlg, ID_FOLINFO_PRINT_PB, FALSE );
                            fOk = FALSE;
                            UtlAlloc( (PVOID *)&pIda->pHeader, 0L, 0L, NOMSG );
                            UtlPackClosePackage( pIda->pPackCtrl );
                            pIda->pPackCtrl = NULL;
                          } /* endif */
                        }
                        else
                        {
                          // alloc failed
                          fOk = FALSE;
                          UtlPackClosePackage( pIda->pPackCtrl );
                          pIda->pPackCtrl = NULL;
                        } /* endif */
                      }
                      else
                      {
                        // package has no user header
                        ENABLECTRL( pIda->hwndDlg, ID_FOLIMP_INFO_PB, FALSE );
                        ENABLECTRL( pIda->hwndDlg, ID_FOLIMP_IMPORT_PB, FALSE );
                        ENABLECTRL( pIda->hwndDlg, ID_FOLINFO_PRINT_PB, FALSE );
                        CLEARTABSTOP( pIda->hwndDlg, ID_FOLIMP_CANCEL_PB );
                        ENABLECTRL( hwnd, ID_FOLIMP_WITHDICT_CHK, FALSE );
                        SETCHECK_FALSE( hwnd, ID_FOLIMP_WITHDICT_CHK );
                        ENABLECTRL( hwnd, ID_FOLIMP_WITHMEM_CHK, FALSE );
                        SETCHECK_FALSE( hwnd, ID_FOLIMP_WITHMEM_CHK );
                      } /* endif */
                    }
                    else
                    {
                      fOk = FALSE;         // error: package did not open
                    } /* endif */
                  } /* endif */

                  //--- check if folder exists somethere and set TO drives ---
                  if ( fOk )
                  {
                    strcpy( pIda->szSourceFolder, pIda->szSourceFolName );
                    strcat( pIda->szSourceFolder, EXT_FOLDER_MAIN );
                    ANSITOOEM( pIda->szLongFolName );
                    FolImpSetToDriveState( pIda, hwnd, pIda->szLongFolName );
                  }
                  else
                  {
                    // something failed, cleanup
                    ENABLECTRL( pIda->hwndDlg, ID_FOLIMP_INFO_PB, FALSE );
                    ENABLECTRL( pIda->hwndDlg, ID_FOLIMP_IMPORT_PB, FALSE );
                    ENABLECTRL( pIda->hwndDlg, ID_FOLINFO_PRINT_PB, FALSE );
                    SETTABSTOP( hwnd, ID_FOLIMP_CANCEL_PB );
                    ENABLECTRL( hwnd, ID_FOLIMP_WITHDICT_CHK, FALSE );
                    SETCHECK_FALSE( hwnd, ID_FOLIMP_WITHDICT_CHK );
                    ENABLECTRL( hwnd, ID_FOLIMP_WITHMEM_CHK, FALSE );
                    SETCHECK_FALSE( hwnd, ID_FOLIMP_WITHMEM_CHK );
                    DESELECTITEMMS( hwnd, ID_FOLIMP_FOLDER_LB, sItem );
                  } /* endif */
                }
                else
                {
                  ENABLECTRL( pIda->hwndDlg, ID_FOLIMP_INFO_PB, FALSE );
                  ENABLECTRL( pIda->hwndDlg, ID_FOLIMP_IMPORT_PB, FALSE );
                  ENABLECTRL( pIda->hwndDlg, ID_FOLINFO_PRINT_PB, FALSE );
                  SETTABSTOP( hwnd, ID_FOLIMP_CANCEL_PB );
                  if ( pIda->chToDrive != ' ' )
                  {
                    SETDRIVE( hwnd,
                              IDFROMDRIVE( ID_TODRIVEA_BTN, pIda->chToDrive ),
                              FALSE );
                  } /* endif */
                  for ( pchDrive = pIda->szToDriveList; *pchDrive; pchDrive++ )
                  {
                    ENABLECTRL( hwnd,
                                IDFROMDRIVE( ID_TODRIVEA_BTN, *pchDrive ),
                                TRUE );
                  } /* endfor */
                } /* endif */
              }
              else
              {
                if ( sItem != LIT_NONE )
                {
                  // get XLIFF file name from our invisible short name listbox
                  int  iShortIndex = QUERYITEMHANDLE( hwnd, ID_FOLIMP_FOLDER_LB, sItem );
                  QUERYITEMTEXT( hwnd, ID_FOLIMP_FOLDER_LB, sItem, pIda->szLongFolName );
                  QUERYITEMTEXTHWND( pIda->hwndShortNameLB, iShortIndex, pIda->szXliffFileName );
                  ENABLECTRL( pIda->hwndDlg, ID_FOLINFO_PRINT_PB, FALSE );
                  ENABLECTRL( pIda->hwndDlg, ID_FOLIMP_INFO_PB, FALSE );
                  ENABLECTRL( pIda->hwndDlg, ID_FOLIMP_IMPORT_PB, TRUE );
                  ENABLECTRL( hwnd, ID_FOLIMP_WITHDICT_CHK, FALSE );
                  SETCHECK_FALSE( hwnd, ID_FOLIMP_WITHMEM_CHK );
                  SETCHECK_FALSE( hwnd, ID_FOLIMP_WITHDICT_CHK );
                  ENABLECTRL( hwnd, ID_FOLIMP_WITHDICT_CHK, FALSE );
                  ENABLECTRL( hwnd, ID_FOLIMP_RENAME_CHK, FALSE );
                  SETCHECK_FALSE( hwnd, ID_FOLIMP_RENAME_CHK );
                  ENABLECTRL( hwnd, ID_FOLIMP_NEWNAME_CB, FALSE );

                  FolImpSetToDriveState( pIda, hwnd, pIda->szLongFolName  );
                } /* endif */
              } /* endif */
              break;
          } /* endswitch */
          break;

        case PID_PB_OK:
          {
            fOK = TRUE;                   // assume everything is o.k.

            switch ( pIda->usDialogType )
            {
              case FOLDER_IMPORT:
               //--- check input data ---
               // get list of selected folders
                if ( fOK )
                {
                  int iFolders = SendDlgItemMessage( hwnd, ID_FOLIMP_FOLDER_LB, LB_GETSELCOUNT, 0L, 0L );

                  // free any selected folder list
                  if ( pIda->pszFolderList ) UtlAlloc( (PVOID *)&(pIda->pszFolderList), 0L, 0L, NOMSG );
                  if ( pIda->pszNameList )   UtlAlloc( (PVOID *)&(pIda->pszNameList), 0L, 0L, NOMSG );

                  if ( iFolders > 1 )
                  {
                    // allocate buffer for the folder names
                    fOK = UtlAlloc( (PVOID *)&(pIda->pszFolderList), 0, (MAX_LONGFILESPEC*iFolders)+10,
                                    ERROR_STORAGE );

                    // allocate buffer for folder names
                    if ( fOK ) fOK = UtlAlloc( (PVOID *)&(pIda->pszNameList), 0, ((MAX_LONGFILESPEC+5)*iFolders)+10,
                                    ERROR_STORAGE );

                    // fill-in names of selected import folders
                    if ( fOK )
                    {
                      PSZ pszCurPos = pIda->pszFolderList;
                      SHORT sItem = LIT_FIRST;
                      BOOL fFirstFolder = TRUE;

                      while ( iFolders )
                      {
                        // get folder short name from our invisible short name listbox
                        sItem = QUERYNEXTSELECTION( hwnd, ID_FOLIMP_FOLDER_LB, sItem );
                        if ( sItem >= 0 )
                        {
                          SHORT sShortIndex = (SHORT)QUERYITEMHANDLE( hwnd, ID_FOLIMP_FOLDER_LB,
                                                                      sItem );
                          if ( fFirstFolder )
                          {
                            QUERYITEMTEXT( hwnd, ID_FOLIMP_FOLDER_LB, sItem, pIda->szLongFolName );
                            fFirstFolder = FALSE;
                          } /* endif */
                          QUERYITEMTEXTHWNDL( pIda->hwndShortNameLB, sShortIndex,
                                              pszCurPos, MAX_LONGFILESPEC);
                          pszCurPos += strlen( pszCurPos ) + 1;
                        } /* endif */
                        iFolders--;
                      } /* endwhile */
                      *pszCurPos = EOS;          // terminate list
                    } /* endif */
                  } /* endif */
                } /* endif */

                // get the selected ImportToDrive
                if ( fOK )
                {
                  int iItem = CBQUERYSELECTION( hwnd, ID_FOLIMP_TODRIVE_PB );
                  int nDrive = CBQUERYITEMHANDLE( hwnd,ID_FOLIMP_TODRIVE_PB, iItem );
                  pIda->chOrgToDrive = pIda->chToDrive = (CHAR)('A' + nDrive);
                }

                // make first folder in list the active one
                if ( fOK && pIda->pszFolderList && (pIda->usImpMode == FOL_TMGR_FORMAT) )
                {
                  pIda->pszActiveFolder = pIda->pszFolderList;

                  //--- handle any previously processed packages ---
                  if ( pIda->pPackCtrl )            // is there an open package ???
                  {
                    UtlPackClosePackage( pIda->pPackCtrl ); // ... close it
                    pIda->pPackCtrl = NULL;
                  } /* endif */
                  if ( pIda->pHeader )              // is there a package header???
                  {
                    UtlAlloc( (PVOID *)&pIda->pHeader, 0L, 0L, NOMSG ); // ... discard it
                    pIda->pHeader = NULL;
                  } /* endif */

                  //--- get package name and open the package ---
                  strcpy( pIda->szSourceFolName, pIda->pszFolderList );
                  strcpy( pIda->szTargetFolName, pIda->pszFolderList );
                  UtlUpper( pIda->szSourceFolName );
                  strcpy( pIda->szObjPath, pIda->szSourceFolName );
                  strcat( pIda->szObjPath, FOLDEREXPORT_EXT );

                  pIda->pPackCtrl = UtlPackOpenPackage( pIda->chFromDrive, NULL, pIda->szImportPath,
                                                        pIda->szObjPath, hwnd );

                  //--- read package's user header ---
                  if ( pIda->pPackCtrl )            // if the package is open ...
                  {
                    ULONG ulHeaderSize = UtlPackQueryHeaderSize( pIda->pPackCtrl );
                    if ( ulHeaderSize )
                    {
                      UtlAlloc( (PVOID *)&pIda->pHeader, 0L, (LONG) ulHeaderSize,
                                ERROR_STORAGE );
                      if ( pIda->pHeader )
                      {
                        ulRC = UtlPackReadHeader( pIda->pPackCtrl,
                                                  (PBYTE) pIda->pHeader,
                                                  ulHeaderSize );
                      }
                      else
                      {
                        // read of header failed
                        UtlAlloc( (PVOID *)&pIda->pHeader, 0L, 0L, NOMSG );
                        UtlPackClosePackage( pIda->pPackCtrl );
                        pIda->pPackCtrl = NULL;
                      } /* endif */
                    }
                    else
                    {
                      // alloc failed
                      fOk = FALSE;
                      UtlPackClosePackage( pIda->pPackCtrl );
                      pIda->pPackCtrl = NULL;
                    } /* endif */
                  } /* endif */

                  //--- check if folder exists somethere and set TO drives ---
                  if ( fOk )
                  {
                    strcpy( pIda->szSourceFolder, pIda->szSourceFolName );
                    strcat( pIda->szSourceFolder, EXT_FOLDER_MAIN );
                    ANSITOOEM( pIda->szLongFolName ); //????? was ist mit long fol name????
                    FolImpSetToDriveState( pIda, hwnd, pIda->szLongFolName );

                    // get the new selected ImportToDrive (in case of existing folder)
                    if ( fOK )
                    {
                      int iItem = CBQUERYSELECTION( hwnd, ID_FOLIMP_TODRIVE_PB  );
                      int nDrive = CBQUERYITEMHANDLE( hwnd,ID_FOLIMP_TODRIVE_PB , iItem );
                      pIda->chToDrive  = (CHAR)('A' + nDrive);
                    } /* endif */
                  } /* endif */
                } /* endif */

                if ( pIda->chToDrive == ' ') // has a drive been selected ???
                {
                  UtlError( ERROR_NO_TARGETDRIVE_SELECTED, MB_CANCEL, 0, (PSZ *)NULP,
                            EQF_ERROR );
                  SETFOCUS( hwnd, ID_FOLIMP_TODRIVE_PB );
                  fOK = FALSE;
                }
                else
                {
                  // set drive of target folder object name
                  pIda->szTargetFolObj[0] = pIda->chToDrive;
                } /* endif */

                // check "import to" name if specified
                if ( fOK && pIda->fImportAs )
                {
                  // Note: specified name has been stored in szTargetFolName
                  //       field of import IDA by function FolSetToDriveSate

                  // check if a name has been specified
                  if ( pIda->szTargetFolName[0] == EOS )
                  {
                    fOK = FALSE;
                    UtlError( ERROR_MISSING_FOLDER_NAME, MB_CANCEL, 0,
                              NULL, EQF_ERROR );
                    SETFOCUS( hwnd, ID_FOLIMP_NEWNAME_CB );
                  } /* endif */

                  // check if the specified name is a valid one
                  if ( fOK )
                  {
                    if ( !UtlCheckLongName( pIda->szTargetFolName ) )
                    {
                      fOK = FALSE;
                      pErrParms[0] = pIda->szTargetFolName;
                      OEMTOANSI( pIda->szTargetFolName );
                      UtlError( ERROR_INV_LONGNAME, MB_CANCEL, 1, pErrParms, EQF_ERROR );
                      ANSITOOEM( pIda->szTargetFolName );
                      SETFOCUS( hwnd, ID_FOLIMP_NEWNAME_CB );
                    } /* endif */
                  } /* endif */

                  // check if a folder with the given name exists already
                  if ( fOK )
                  {
                    // controlled folder fields of target folder
                    PPROPFOLDER pFolProp = NULL;
                    BOOL fTargetAFCFolder = FALSE;
                    char szAFCPassword[sizeof(pFolProp->szAFCPassword)];
                    BOOL fIsNew = FALSE;         // folder-is-new flag

                    ObjLongToShortName( pIda->szTargetFolName, pIda->szTargetShortFolName, FOLDER_OBJECT, &fIsNew );

                    UtlMakeEQFPath( pIda->szTargetFolObj, pIda->szTargetFolObj[0], SYSTEM_PATH, NULL );
                    strcat( pIda->szTargetFolObj, BACKSLASH_STR );
                    strcat( pIda->szTargetFolObj, pIda->szTargetShortFolName );
                    strcat( pIda->szTargetFolObj, EXT_FOLDER_MAIN );

                    // get controlled folder fields from existing folder
                    if ( !fIsNew )
                    { 
                      ULONG ulLen = 0;
                      UtlMakeEQFPath( pIda->szCurFile, pIda->szTargetFolObj[0], PROPERTY_PATH, NULL );
                      strcat( pIda->szCurFile, BACKSLASH_STR );
                      strcat( pIda->szCurFile, pIda->szTargetShortFolName );
                      strcat( pIda->szCurFile, EXT_FOLDER_MAIN );
                      if ( UtlLoadFileL( pIda->szCurFile, (PVOID *)&pFolProp, &ulLen, FALSE, FALSE ) )
                      {
                        fTargetAFCFolder = pFolProp->fAFCFolder;
                        strcpy( szAFCPassword, pFolProp->szAFCPassword );
                        UtlAlloc( (PVOID *)&pFolProp, 0L, 0L, NOMSG );
                      } /* endif */
                    } /* endif */

                    if ( stricmp( pIda->szLongFolName, pIda->szTargetFolName) != 0 )
                    {
                      if ( !fIsNew )
                      {
                        if ( fTargetAFCFolder )
                        {
                          fOK = FolImp_ControlledFolderMergeConfirmation( hwnd, szAFCPassword, 
                                                                          pIda->szTargetFolName, TRUE );
                          if ( !fOK ) SETFOCUS( hwnd, ID_FOLIMP_NEWNAME_CB );
                        }
                        else
                        {
                          USHORT usMBReturn;          // return code of message box

                          pErrParms[0] = pIda->szTargetFolName;
                          pErrParms[1] = pIda->szLongFolName;
                          usMBReturn = UtlError( FOLDER_MERGE_CONFIRMATION, MB_YESNO,
                                                2, pErrParms, EQF_QUERY );
                          if ( usMBReturn != MBID_YES )
                          {
                            SETFOCUS( hwnd, ID_FOLIMP_NEWNAME_CB );
                            fOK = FALSE;
                          } /* endif */
                        } /* endif */
                      } /* endif */
                    } /* endif */
                  } /* endif */
                } /* endif */

                // check if the target folder exists but has a different unique ID
                // (i.e. it is another folder with the same name)
                // perform this check only if folder names are equal
                if ( fOK && !mp2 && (pIda->usImpMode == FOL_TMGR_FORMAT) && (stricmp( pIda->szLongFolName, pIda->szTargetFolName) == 0) )
                {
                  BOOL fIsNew = FALSE;
                  BOOL fUniqueIDAvailable = TRUE;
                  PPROPFOLDER pFolProp = NULL;
                  ULONG ulLen = 0;
                  char szSourceID[sizeof(pFolProp->UniqueID)];
                  char szTargetID[sizeof(pFolProp->UniqueID)];

                  // controlled folder fields of target folder
                  BOOL fTargetAFCFolder = FALSE;
                  BOOL fTargetTCMasterFolder = FALSE;
                  char szAFCPassword[sizeof(pFolProp->szAFCPassword)];


                  // get unique ID of target folder
                  ObjLongToShortName( pIda->szTargetFolName,
                                      pIda->szTargetShortFolName,
                                      FOLDER_OBJECT, &fIsNew );

                  if ( !fIsNew )       // perform unique ID check for existing folders only
                  {
                    UtlMakeEQFPath( pIda->szCurFile,
                                    pIda->szTargetFolObj[0],
                                    PROPERTY_PATH, NULL );
                    strcat( pIda->szCurFile, BACKSLASH_STR );
                    strcat( pIda->szCurFile, pIda->szTargetShortFolName );
                    strcat( pIda->szCurFile, EXT_FOLDER_MAIN );
                    if ( UtlLoadFileL( pIda->szCurFile, (PVOID *)&pFolProp, &ulLen, FALSE, FALSE ) )
                    {
                      strcpy( szTargetID, pFolProp->UniqueID );
                      fTargetAFCFolder = pFolProp->fAFCFolder;
                      fTargetTCMasterFolder = pFolProp->fTCMasterFolder;
                      strcpy( szAFCPassword, pFolProp->szAFCPassword );
                      UtlAlloc( (PVOID *)&pFolProp, 0L, 0L, NOMSG );
                    }
                    else
                    {
                      fUniqueIDAvailable = FALSE;
                    } /* endif */

                    // get unique ID of folder being imported
                    if ( pIda->pHeader && (pIda->pHeader->BitFlags.fHeaderType == 1) )
                    {
                      strcpy( szSourceID, pIda->pHeader->Head.Rel1.UniqueID );
                    }
                    else if ( pIda->pPackCtrl )
                    {
                      UtlAlloc( (PVOID *)&pFolProp, 0L, sizeof(PROPFOLDER), NOMSG );

                      if ( pFolProp )
                      {
                        UtlMakeEQFPath( pIda->szCurFile,
                                        pIda->szTargetFolObj[0],  // don't care for drive letter
                                        PROPERTY_PATH, NULL );
                        strcat( pIda->szCurFile, BACKSLASH_STR );
                        strcat( pIda->szCurFile, pIda->szShortFolName );
                        strcat( pIda->szCurFile, EXT_FOLDER_MAIN );
                        UtlPackReadFilePart( pIda->pPackCtrl, pIda->szCurFile, 0,
                                             (PBYTE)pFolProp, sizeof(PROPFOLDER) );
                        strcpy( szSourceID, pFolProp->UniqueID );
                        UtlAlloc( (PVOID *)&pFolProp, 0L, 0L, NOMSG );
                      }
                      else
                      {
                        fUniqueIDAvailable = FALSE;
                      } /* endif */
                    }
                    else
                    {
                      fUniqueIDAvailable = FALSE;
                    } /* endif */

                    if ( !fUniqueIDAvailable || (strcmp( szSourceID, szTargetID ) != 0) )
                    {
                      if ( fTargetAFCFolder )
                      {
                        fOK = FolImp_ControlledFolderMergeConfirmation( hwnd, szAFCPassword, pIda->szTargetFolName, FALSE );
                      }
                      else
                      {
                        // folder's are not the same, get merge confirmation
                        PSZ   pszErrParm = pIda->szTargetFolName;
                        USHORT usMBReturn;
                        OEMTOANSI(pIda->szTargetFolName);
                        usMBReturn = UtlErrorHwnd( ERROR_IMP_FOLDER_EXISTS, MB_OKCANCEL, 1,
                                                          &pszErrParm, EQF_ERROR, pIda->hwndErrMsg );
                        ANSITOOEM(pIda->szTargetFolName);
                        if ( usMBReturn == MBID_CANCEL )
                        {
                          fOK = FALSE;
                        } /* endif */
                      } /* endif */
                    } /* endif */
                  } /* endif */
                } /* endif */
                //--- check if folder has been locked by another process
                if ( fOK )
                {
                  sRC = QUERYSYMBOL( pIda->szTargetFolObj );
                  if ( sRC != -1 )
                  {
                    fOK = FALSE;
                    pErrParms[0] = pIda->szTargetFolName;
                    UtlError( ERROR_FOLDER_LOCKED, MB_CANCEL, 1,
                              pErrParms, EQF_INFO );
                  } /* endif */
                } /* endif */

                /*******************************************************/
                /* Check if folder is currently open                   */
                /*******************************************************/
                if ( fOK )
                {
                  if ( EqfQueryObject( pIda->szTargetFolObj, clsFOLDER, 0) )
                  {
                    PSZ        pszParm;

                    pszParm = pIda->szTargetFolName;
                    OEMTOANSI(pIda->szTargetFolName);
                    UtlError( ERROR_IMPORT_OPEN_FOLDER, MB_CANCEL, 1, &pszParm, EQF_ERROR );
                    ANSITOOEM(pIda->szTargetFolName);
                    fOK = FALSE;
                  } /* endif */
                } /* endif */

                //--- if o.k. remember checkbox settings ---
                if ( fOK )
                {
                  pIda->fWithDict = QUERYCHECK(hwnd, ID_FOLIMP_WITHDICT_CHK);
                  pIda->fWithMem  = QUERYCHECK(hwnd, ID_FOLIMP_WITHMEM_CHK );
                } /* endif */

                //--- if o.k. set/reset skip flags of package files
                if ( fOK && pIda->pPackCtrl )
                {
                  ulRC = UtlPackGetFileListPtr( pIda->pPackCtrl, &pFileList );
                  fOK = UtlPackHandleError( ulRC, pIda->szSourceFolName, hwnd );
                  if ( fOK )
                  {
                    usNoOfEntries = (USHORT)pFileList->ulListUsed;
                    pFile         = pFileList->pEntries;
                    while ( usNoOfEntries )
                    {
                      pFile->usProcessFlags &= ~FILE_SKIP;

                      if ( pIda->pHeader->BitFlags.fHeaderType == RELEASE0_HEADER )
                      {
                        if ( ( pFile->usFileType == PROP_CLASS_TAGTABLE ) ||
                             ( pFile->usFileType == PROP_CLASS_MEMORY ) )
                        {
                          if ( !pIda->fWithMem )
                          {
                            pFile->usProcessFlags |= FILE_SKIP;
                          } /* endif */
                        }
                        else if ( pFile->usFileType == PROP_CLASS_DICTIONARY )
                        {
                          if ( !pIda->fWithDict )
                          {
                            pFile->usProcessFlags |= FILE_SKIP;
                          } /* endif */
                        } /* endif */
                      }
                      else
                      {
                        if ( ( pFile->usFileType == MEMORY_PROP_FILE ) ||
                             ( pFile->usFileType == MEMORY_DATA_FILE ) ||
                             ( pFile->usFileType == MEMORY_TABLE_FILE ) ||
                             ( pFile->usFileType == NTMMEMORY_INDEX_FILE ) ||
                             ( pFile->usFileType == NTMMEMORY_DATA_FILE ) )
                        {
                          if ( !pIda->fWithMem )
                          {
                            pFile->usProcessFlags |= FILE_SKIP;
                          } /* endif */
                        }
                        else if ( ( pFile->usFileType == DICTIONARY_PROP_FILE ) ||
                                  ( pFile->usFileType == DICTIONARY_DATA_FILE ) ||
                                  ( pFile->usFileType == DICTIONARY_INDEX_FILE ) )
                        {
                          if ( !pIda->fWithDict )
                          {
                            pFile->usProcessFlags |= FILE_SKIP;
                          } /* endif */
                        } /* endif */
                      } /* endif */

                      usNoOfEntries--;            // skip to next entry in file list
                      pFile++;
                    } /* endwhile */
                  } /* endif */
                } /* endif */

                //--- if o.k. check free space on target drive ---
                if ( fOK && pIda->pPackCtrl )
                {
                  ulRC = UtlPackQueryRequiredSpace( pIda->pPackCtrl,
                                                    &ulRequiredSpace );
                  fOK = UtlPackHandleError( ulRC, pIda->szSourceFolName, hwnd );
                  if ( fOK )
                  {
                    ulFreeSpace = UtlQueryFreeSpace( pIda->chToDrive, TRUE );
                    if ( ulFreeSpace < ulRequiredSpace )
                    {
                      pErrParms[0] = pIda->szLongFolName;
                      szDrive[0] = pIda->chToDrive;
                      szDrive[1] = EOS;
                      pErrParms[1] = szDrive;
                      UtlError( ERROR_FOLIMP_NOSPACE, MB_CANCEL, 2, pErrParms,
                                EQF_ERROR );
                      fOK = FALSE;
                    } /* endif */
                  } /* endif */
                } /* endif */


                /*******************************************************/
                /* save last used values                               */
                /*******************************************************/
                if ( fOK )
                {
                  UtlSaveLastUsedString( hwnd, ID_FOLIMP_FILENAME_CB, FOLIMPEXPLASTUSEDDIR, 10 );

                  if ( SetPropAccess( pIda->hFllProp, PROP_ACCESS_WRITE) )
                  {
                    PPROPFOLDERLIST pFllProp;     // ptr to folder list properties
                    EQFINFO         ErrorInfo;    // error code of property handler calls

                    pFllProp = (PPROPFOLDERLIST)MakePropPtrFromHnd( pIda->hFllProp);
                    pFllProp->chFolImpFromDrive = pIda->chFromDrive;
                    pFllProp->fImportPath = TRUE;
                    strcpy(pFllProp->szImportPath, pIda->szImportPath);
                    pFllProp->usFolImpFormat = pIda->usImpMode;
                    SaveProperties( pIda->hFllProp, &ErrorInfo );
                  } /* endif */
                } /* endif */
                //--- if o.k. lock folder and remove dialog ---
//          if ( fOK )
                {

                  pIda->fOk = fOK;
                  //SETSYMBOL( pIda->szTargetFolObj );
                  // POSTCLOSE( hwnd, TRUE );
                } /* endif */
            } /* endswitch */

            // avoid switch to details if more than one folder package is selected
            if ( mp2 && (SendDlgItemMessage( hwnd, ID_FOLIMP_FOLDER_LB, LB_GETSELITEMS, 0L, 0L ) > 1) )
            {
              mResult = MRFROMSHORT(FALSE);
            } /* endif */
          }
          break;

        default:
            break;
       } /* endswitch */
       break;

    case WM_HELP:
      //-----------------------------------
      /*************************************************************/
      /* pass on a HELP_WM_HELP request                            */
      /*************************************************************/
      EqfDisplayContextHelp( (HWND)((LPHELPINFO) mp2)->hItemHandle,
                             &hlpsubtblFolPropGeneralImpDlg[0] );
      mResult = TRUE;  // message processed
      break;

    case WM_CLOSE:
      //-----------------------------------/
      mResult = WinDefDlgProc( hwnd, msg, mp1, mp2 );
      break;

    case WM_DRAWITEM:                // draw a listbox/combobox item
      {
        LPDRAWITEMSTRUCT lpDisp = (LPDRAWITEMSTRUCT)mp2;

        if ( (lpDisp->itemID != -1) )
        {
          HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);

          if ( lpDisp->itemAction & (ODA_DRAWENTIRE | ODA_SELECT) )
          {
            BITMAP     bm;               // bitmap structure
            HBITMAP    hbm = (HBITMAP)NULL; // bitmap
            RECT       rectl;            // rectl to be drawn
            POINTL     pt;               // for WM_DRAWITEM
            CHAR       chDrive;          // buffer for drive being painted
            SHORT      sDriveType;       // type of drive being painted
            int nDrive;

            pIda = ACCESSDLGIDA (hwnd, PFOLIMPIDA);  // get access to IDA

            // get test and drive letter of drive
            CBQUERYITEMTEXT( hwnd, ID_FOLIMP_TODRIVE_PB, lpDisp->itemID,
                             pIda->szBuffer );
            nDrive = CBQUERYITEMHANDLE( hwnd,ID_FOLIMP_TODRIVE_PB, lpDisp->itemID );
            chDrive  = (CHAR)('A' + nDrive);

            // get bitmap for this drive letter
            sDriveType = UtlDriveType( chDrive );
            switch ( sDriveType )
            {
              case  DRIVE_REMOVABLE:
                hbm = LoadBitmap( hResMod, MAKEINTRESOURCE( EQFFD_BITMAP ));
                break;
              case  DRIVE_FIXED:
                hbm = LoadBitmap( hResMod, MAKEINTRESOURCE( EQFHD_BITMAP ));
                break;
              case  DRIVE_REMOTE:
                hbm = LoadBitmap( hResMod, MAKEINTRESOURCE( EQFNET_BITMAP ));
                break;
              case DRIVE_CDROM:
                hbm = LoadBitmap( hResMod, MAKEINTRESOURCE( EQFCD_BITMAP ));
                break;
              case DRIVE_RAM:
                hbm = LoadBitmap( hResMod, MAKEINTRESOURCE( EQFRAM_BITMAP ));
                break;
              default :
                hbm = LoadBitmap( hResMod, MAKEINTRESOURCE( EQFFD_BITMAP ));
                break;
            } /* endswitch */

            // Get a copy of the item rectangle
            memcpy( &rectl, &lpDisp->rcItem, sizeof(rectl) );

            // Draw item
            FILLRECT( lpDisp->hDC, rectl, SYSCLR_WINDOW );
            pt.x = rectl.left + 2;
            pt.y = rectl.top  + 2;
            if ( hbm )
            {
              GetObject( hbm, sizeof( BITMAP ), (LPSTR) &bm );
              WinDrawBitmap( lpDisp->hDC, hbm, NULL, &pt,
                             GetSysColor(COLOR_WINDOWTEXT),
                             GetSysColor(COLOR_WINDOW),
                             DBM_NORMAL );
              rectl.left += bm.bmWidth + 6;
              rectl.top  += 2l;
              DeleteObject( hbm );
            } /* endif */

            {
              HFONT hOldFont = (HFONT)SelectObject( lpDisp->hDC, GetStockObject(DEFAULT_GUI_FONT) );
              DRAWTEXT( lpDisp->hDC, pIda->szBuffer, rectl,
                      GetSysColor(COLOR_WINDOWTEXT), GetSysColor(COLOR_WINDOW),
                      DT_LEFT | DT_VCENTER );
              SelectObject( lpDisp->hDC, hOldFont );
            }

            // let Windows do the selection ...
            if (  lpDisp->itemState & ODS_SELECTED  )
            {
              InvertRect( lpDisp->hDC, &lpDisp->rcItem );
            } /* endif */
          } /* endif */

          // item gets or loses the focus ...
          if (  lpDisp->itemAction & ODA_FOCUS  )
          {
            DrawFocusRect( lpDisp->hDC, &lpDisp->rcItem );
          } /* endif */
        } /* endif */
        mResult = MRFROMSHORT( TRUE );
      }
      break;


    default:
      //-----------------------------------
      mResult = WinDefDlgProc( hwnd, msg, mp1, mp2 );
      break;
  } /* endswitch */

  return mResult;
};






//------------------------------------------------------------------------------
// Internal function  FOLIMPPROP_DETAILS_DLGPROC                                
//------------------------------------------------------------------------------
// Function name:                                                               
//------------------------------------------------------------------------------
// Function call:                                                               
//------------------------------------------------------------------------------
// Description:                                                                 
//------------------------------------------------------------------------------
// Parameters:                                                                  
//                                                                              
//------------------------------------------------------------------------------
// Returncode type:                                                             
//------------------------------------------------------------------------------
// Returncodes:                                                                 
//------------------------------------------------------------------------------
// Function flow:                                                               
//                                                                              
//                                                                              
//                                                                              
//                                                                              
//------------------------------------------------------------------------------

INT_PTR CALLBACK FOLIMPPROP_DETAILS_DLGPROC
(
HWND hwnd,                       /* handle of dialog window             */
WINMSG msg,
WPARAM mp1,
LPARAM mp2
)
{
  MRESULT  mResult = MRFROMSHORT( TRUE );                // result value of procedure
  PFOLIMPIDA  pIda;
  switch ( msg )
  {
    case WM_INITDLG:
      //-----------------------------------
      {
        //--- create IDA and store pointer to it ---
        pIda = (PFOLIMPIDA) mp2;
        ANCHORDLGIDA( hwnd, pIda );
      }
      break;

    case WM_COMMAND:
      pIda = ACCESSDLGIDA (hwnd, PFOLIMPIDA);  // get access to IDA
      //-----------------------------------
      switch ( WMCOMMANDID( mp1, mp2 ) )
      {
        case ID_FOLIMP_INFO_PB:

          mResult = FolImpInfoInit( hwnd, mp1, mp2 );

          break;

        case ID_FOLIMP_CANCEL_PB:

          DELETEALL(hwnd, ID_FOLINFO_DOCS_LB);
          DELETEALL(hwnd, ID_FOLINFO_TMEMS_LB);
          DELETEALL(hwnd, ID_FOLINFO_DICTS_LB);
          SETTEXT( hwnd, ID_FOLINFO_NAME_TEXT, "" );
          SETTEXT( hwnd, ID_FOLINFO_DESCR_TEXT, "" );
          SETTEXT( hwnd, ID_FOLINFO_DATE_TEXT, "" );
          SETTEXT( hwnd, ID_FOLINFO_MEM_TEXT, "" );
          SETTEXT( hwnd, ID_FOLINFO_MEM_TEXT, "");
          SETTEXT( hwnd, ID_FOLINFO_MARKUP_TEXT,"");
          SETTEXT( hwnd, ID_FOLINFO_OPTION_TEXT,"");


          break;


        default:

          break;


      } /* endswitch */
      break;




    case WM_HELP:
      //-----------------------------------
      /*************************************************************/
      /* pass on a HELP_WM_HELP request                            */
      /*************************************************************/
//          EqfDisplayContextHelp( ((LPHELPINFO) mp2)->hItemHandle,
//                                 &hlpsubtblRptPropGeneral[0] );
      mResult = TRUE;  // message processed
      break;

    case WM_CLOSE:
      //-----------------------------------/
      mResult = FolImpInfoClose( hwnd, mp1, mp2 );
//    UtlPackClosePackage( pIda->pPackCtrl );

      //mResult = WinDefDlgProc( hwnd, msg, mp1, mp2 );
      break;

    default:
      //-----------------------------------
      mResult = WinDefDlgProc( hwnd, msg, mp1, mp2 );
      break;
  } /* endswitch */

  return mResult;

}; // DETAILS


BOOL FolImpGetFolderDrive
(
  PFOLIMPIDA pIda,
  PSZ pszName,
  CHAR *pchDrive
)
{
  EQFINFO     ErrorInfo;               // error code of property handler calls
  PPROPFOLDER pFolProp;                // ptr to folder properties
  PVOID       hFolProp;                // handle of folder properties
  BOOL        fIsNew;
  BOOL        fFolderExists = FALSE;

  ObjLongToShortName( pszName, pIda->szTargetShortFolName, FOLDER_OBJECT, &fIsNew );
  UtlMakeEQFPath( pIda->szTargetFolObj, NULC, SYSTEM_PATH, NULL );
  strcat( pIda->szTargetFolObj, BACKSLASH_STR );
  strcpy( pIda->szTargetFolName, pszName );
  strcpy( pIda->szTargetFolder, pIda->szTargetShortFolName );
  strcat( pIda->szTargetFolder, EXT_FOLDER_MAIN );
  strcat( pIda->szTargetFolObj,  pIda->szTargetFolder );

  hFolProp = OpenProperties( pIda->szTargetFolObj, NULL, PROP_ACCESS_READ, &ErrorInfo);
  if ( hFolProp )
  {
    pFolProp = (PPROPFOLDER)MakePropPtrFromHnd( hFolProp );
    if ( pFolProp->chDrive == NULC )
    {
      *pchDrive = pIda->szTargetFolObj[0];
    }
    else
    {
      *pchDrive = pFolProp->chDrive;
    } /* endif */
    CloseProperties( hFolProp, PROP_QUIT, &ErrorInfo );

    if ( strchr( pIda->szToDriveList, toupper(*pchDrive)  ) != NULL )
    {
      fFolderExists = TRUE;
    }
    else
    {
      *pchDrive  = pIda->chSysDrive;
    } /* endif */
  } /* endif */

  return( fFolderExists );

} /* end of function FolImpGetFolderDrive */

// check if file dates of imported and existing objects are correct
BOOL FolCheckFileDates
(
  PSZ    pszImportedObject,            // full path name of object being imported
  PSZ    pszExistingObject,            // full path name of existing object
  HWND   hwndErrMsg,                   // parent handle for error messages
  USHORT usMsgExisting,                // number of message to be displayed for existing
  USHORT usMsgImported                 // number of message to be displayed for imported
)
{
  WIN32_FIND_DATA FindData;
  FILETIME  ftImpFile;
  FILETIME  ftExistFile;
  FILETIME  ftSysTime;
  SYSTEMTIME SystemTime;
  BOOL       fOK = TRUE;
  HANDLE      hDir;
  BOOL        fExists = FALSE;
  BOOL        fImpExists = FALSE;
  CHAR        szDate[30];
  CHAR        szName[30];

  // get system time and apply acceptance factor
  {
    LARGE_INTEGER liTime;
    LARGE_INTEGER liAdd;

    GetSystemTimeAsFileTime( &ftSysTime );
    memcpy( &liTime, &ftSysTime, sizeof(liTime ) );

    // FILETIME is number of 100-nanosecond intervals
    // so we add 3 days here ...
    liAdd.QuadPart = 10000;             // miliseconds
    liAdd.QuadPart *= 1000;             // seconds
    liAdd.QuadPart *= (60 * 60);        // hours
    liAdd.QuadPart *= (24 * 3);         // 3 days
    liTime.QuadPart += liAdd.QuadPart;
    memcpy( &ftSysTime, &liTime, sizeof(liTime ) );
  }

  hDir = FindFirstFile( pszImportedObject, &FindData );
  if ( hDir != INVALID_HANDLE_VALUE )
  {
    memcpy( &ftImpFile, &(FindData.ftLastWriteTime), sizeof(ftImpFile) );
    fImpExists = TRUE;
    FindClose( hDir );
  } /* endif */

  hDir = FindFirstFile( pszExistingObject, &FindData );
  if ( hDir != INVALID_HANDLE_VALUE )
  {
    memcpy( &ftExistFile, &(FindData.ftLastWriteTime), sizeof(ftExistFile) );
    FindClose( hDir );
    fExists = TRUE;
  } /* endif */

  // check existing object
  if ( fExists )
  {
    if ( CompareFileTime( &ftSysTime, &ftExistFile ) < 0 )
    {
      PSZ pszParms[2];
      pszParms[0] = szName;
      pszParms[1] = szDate;
      FileTimeToSystemTime( &ftExistFile, &SystemTime );
      Utlstrccpy( szName, UtlGetFnameFromPath( pszExistingObject ), DOT );
      sprintf( szDate, "%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d",
               SystemTime.wYear, SystemTime.wMonth, SystemTime.wDay,
               SystemTime.wHour, SystemTime.wMinute, SystemTime.wSecond );
      UtlErrorHwnd( usMsgExisting, MB_CANCEL, 2, pszParms, EQF_ERROR, hwndErrMsg );
      fOK = FALSE;
    } /* endif */
  } /* endif */

  // check imported object
  if ( fImpExists )
  {
    if ( CompareFileTime( &ftSysTime, &ftImpFile ) < 0 )
    {
      PSZ pszParms[2];
      pszParms[0] = szName;
      pszParms[1] = szDate;
      FileTimeToSystemTime( &ftImpFile, &SystemTime );
      Utlstrccpy( szName, UtlGetFnameFromPath( pszImportedObject ), DOT );
      sprintf( szDate, "%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d",
               SystemTime.wYear, SystemTime.wMonth, SystemTime.wDay,
               SystemTime.wHour, SystemTime.wMinute, SystemTime.wSecond );
      UtlErrorHwnd( usMsgImported, MB_CANCEL, 2, pszParms, EQF_ERROR, hwndErrMsg );
      fOK = FALSE;
    } /* endif */
  } /* endif */

  return( fOK );
} /* end of function FolCheckFileDates */


// build list of additional tag table files contained in folder package
BOOL FolImpBuildAddTagTableFilesList( PFOLIMPIDA pIda )
{
  BOOL fOK = TRUE;
  PFILELIST   pFileList;              // ptr to package's file list
  USHORT      usNoOfEntries;          // # of enteries in package's file list
  PFILELISTENTRY pFile;               // ptr for file list processing
  ULONG ulRC = 0;
  ULONG ulListSize = 0;               // size of list in number of bytes

  // free any existing list
  if ( pIda->pAddTagTableFiles != NULL ) UtlAlloc( (PVOID *)&pIda->pAddTagTableFiles, 0L, 0L, NOMSG );
  pIda->pAddTagTableFiles = NULL;

  // get required size of list buffer
  ulRC = UtlPackGetFileListPtr( pIda->pPackCtrl, &pFileList );
  fOK = (ulRC == 0);
  if ( fOK )
  {
    usNoOfEntries = (USHORT)pFileList->ulListUsed;
    pFile         = pFileList->pEntries;
    while ( usNoOfEntries )
    {
      if ( pFile->usFileType == TAGTABLE_ADDITIONAL_FILE )
      {
        ulListSize += strlen(pFile->pszName) + 1;
      } /* endif */
      usNoOfEntries--;            // skip to next entry in file list
      pFile++;
    } /* endwhile */
  } /* endif */

  // allocate new list
  if ( fOK && (ulListSize != 0) )
  {
    fOK = UtlAlloc( (PVOID *)&pIda->pAddTagTableFiles, 0L, ulListSize + 10, ERROR_STORAGE );
  } /* endif */

  // fill buffer with file names
  if ( fOK && (pIda->pAddTagTableFiles!= NULL))
  {
    PSZ pszCurFile = pIda->pAddTagTableFiles;
    usNoOfEntries = (USHORT)pFileList->ulListUsed;
    pFile         = pFileList->pEntries;
    while ( usNoOfEntries )
    {
      if ( pFile->usFileType == TAGTABLE_ADDITIONAL_FILE )
      {
        strcpy( pszCurFile, pFile->pszName );
        pszCurFile += strlen( pFile->pszName ) + 1;
      } /* endif */
      usNoOfEntries--;            // skip to next entry in file list
      pFile++;
    } /* endwhile */
    *pszCurFile = EOS;           // add end-of-list indicator
  } /* endif */

  return( fOK );
} /* end of function FolImpBuildAddTagTableFilesList */


// get folder merge confirmation for controlled folder with password check
BOOL FolImp_ControlledFolderMergeConfirmation( HWND hwnd, PSZ pszPassword, PSZ pszFolderName, BOOL fMerge )
{
  BOOL fOK = FALSE;
  PSZ   pszErrParm = pszFolderName;
  USHORT usMBReturn;

  OEMTOANSI(pszFolderName);
  usMBReturn = UtlErrorHwnd( fMerge ? ERROR_FOLIMP_CONTROLLEDFOLDERMERGE : ERROR_FOLIMP_CONTROLLEDFOLDEREXISTS, 
                             MB_OKCANCEL, 1, &pszErrParm, EQF_ERROR, hwnd );
  ANSITOOEM(pszFolderName);
  if ( usMBReturn == MBID_OK )
  {
    HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
    // password check before
    DIALOGBOX( hwnd, FolImp_PasswordDlg, hResMod, ID_FOLIMPPW_DLG, pszPassword, fOK ) ;
  } /* endif */
  return( fOK );
}  /* end of function FolImp_ControlledFolderMergeConfirmation */


INT_PTR CALLBACK FolImp_PasswordDlg
(
  HWND   hwndDlg,        //handle of dialog window
  WINMSG message,        //message id
  WPARAM mp1,            //message parameter
  LPARAM mp2             //message parameter
)
{
  MRESULT      mResult = FALSE;             //return code

  switch ( message)
  {
    case ( WM_INITDLG ):
      //anchor password
      {
        PSZ pszPassword = (PSZ) mp2;
        ANCHORDLGIDA( hwndDlg, pszPassword );
      } 
      //length of password entry field is 8
      SETTEXTLIMIT( hwndDlg, ID_FOLIMPPW_PSW_EF, MAX_FNAME - 1 );
      break;

    case WM_HELP:
//TODO      EqfDisplayContextHelp( ((LPHELPINFO) mp2)->hItemHandle, &hlpsubtblDictPasswordDlg[0] );
      mResult = TRUE;  // message processed
      break;

    case ( WM_CLOSE ):
      WinDismissDlg( hwndDlg, SHORT1FROMMP1( mp1 ) );
      break;

    case WM_CONTROL:
      break;

    case ( WM_COMMAND ):
      mResult = MRFROMSHORT( TRUE);

      switch (WMCOMMANDID( mp1, mp2 ))
      {
		    case ID_FOLIMPPW_HELP_PB:
		      mResult = UtlInvokeHelp();
  	      break;

        case DID_CANCEL:
        case ID_FOLIMPPW_CANCEL_PB:
          WinPostMsg( hwndDlg, WM_CLOSE, NULL, NULL );
          break;

        case ID_FOLIMPPW_ENTER_PB:
          {
            BOOL fOK = FALSE;

            CHAR szPassword[20];

            // get folder password
            PSZ pszPassword = ACCESSDLGIDA( hwndDlg, PSZ );

            // get specified password
            QUERYTEXT( hwndDlg, ID_FOLIMPPW_PSW_EF, szPassword );
            UtlStripBlanks( szPassword );

            // check password
            if ( szPassword[0] != EOS )
            {
              if ( strcmp( pszPassword, szPassword ) != 0 )
              {
                UtlErrorHwnd( ERRMSG_WRONG_PASSWORD, MB_CANCEL, 0, NULL, EQF_ERROR, hwndDlg );
                SETFOCUS( hwndDlg, ID_FOLIMPPW_PSW_EF );
              }
              else
              {
                // password correct
                fOK = TRUE;
              } /* endif */
            }
            else
            {
              UtlErrorHwnd( ERROR_NO_PASSWORD_ENTERED, MB_CANCEL, 0, NULL, EQF_ERROR, hwndDlg );
              SETFOCUS( hwndDlg, ID_FOLIMPPW_PSW_EF );
            } /* endif */

            if ( fOK )
            {
              WinPostMsg( hwndDlg, WM_CLOSE, MP1FROMSHORT( fOK ), NULL );
            } /* endif */
          }
          break;
      } /* endswitch */
      break;

    default:
      mResult = WinDefDlgProc ( hwndDlg, message, mp1, mp2 );
      break;
  } /* endswitch */
  return( mResult );
}

// list XLIFF folders 
int ListXLiffFolder
(
  PFOLIMPIDA       pIda,               // ptr to folder import IDA
  HWND             hwndDlg,            // handle of dialog window
  PSZ              pszPath,            // path bein searched thru
  PSZ              pszPattern          // file pattern
)
{
  // loop over all matching files and add any folders founf to our listbox
  FILEFINDBUF stFile;              // Output buffer of UtlFindFirst
  USHORT usCount;                  // For UtlFindFirst
  HDIR hSearch;                    // Directory handle for UtlFindFirst
  USHORT usRC;
  int    iFolders = 0;

  typedef int (* __cdecl FOLXLIFFCHECKFILE_DCL)( PSZ, PSZ, HWND );
  FOLXLIFFCHECKFILE_DCL pfnFOLXLIFFCHECKFILE = NULL ;
  HMODULE hXLIFF = NULL;

  hSearch = HDIR_CREATE;
  memset( &stFile, 0, sizeof(stFile) );
  usCount = 1;
  strcpy( pIda->szObjPath, pszPath );
  strcat( pIda->szObjPath, BACKSLASH_STR );
  strcat( pIda->szObjPath, pszPattern );
  usRC = UtlFindFirst( pIda->szObjPath, &hSearch, 0, &stFile, sizeof(stFile), &usCount, 0L, FALSE );
  if ( ! usRC ) {
     hXLIFF = LoadLibrary( "EQFXLIFF.DLL" );
     if (hXLIFF != NULL) {
       pfnFOLXLIFFCHECKFILE = (FOLXLIFFCHECKFILE_DCL)GetProcAddress(hXLIFF, "FOLXLIFFCHECKFILE");
       if ( pfnFOLXLIFFCHECKFILE == NULL ) 
          usRC=1 ;
     } else {
        usRC=1 ;
     }
  }

  while ( !usRC && usCount )
  {
    strcpy( pIda->szObjPath, pszPath );
    strcat( pIda->szObjPath, BACKSLASH_STR );
    strcat( pIda->szObjPath, stFile.cFileName );

    if ( (*pfnFOLXLIFFCHECKFILE)( pIda->szObjPath, pIda->szBuffer, pIda->hwndErrMsg ) == 0 )
    {
      int item = INSERTITEMHWND( pIda->hwndShortNameLB, pIda->szObjPath );
      if ( item != LIT_NONE )
      {
        int iLongNameItem = INSERTITEM( hwndDlg, ID_FOLIMP_FOLDER_LB, pIda->szBuffer );
        if ( iLongNameItem >= 0 )
        {
          SETITEMHANDLE( hwndDlg, ID_FOLIMP_FOLDER_LB, iLongNameItem, item );
        } /* endif */
        iFolders++;
      } /* endif */
    } /* endif */

    usRC = UtlFindNext( hSearch, &stFile, sizeof(stFile), &usCount, 0);
  } /*endwhile */

  if ( hXLIFF )
     FreeLibrary( hXLIFF );

  return( iFolders );
}


// build a unique folder import path
void FolImpBuildUniqueImportPath( PFOLIMPIDA pIda )
{
  LONG lTime = 0;
  struct tm   *pTimeDate;    // time/date structure

  time( &lTime );
  pTimeDate = localtime( &lTime );
  UtlQueryString( QST_IMPORTDIR, pIda->szImportDir + 1, sizeof(pIda->szImportDir) - 1);
  pIda->szImportDir[0] = BACKSLASH;
  if ( pTimeDate != NULL )
  {
    sprintf( pIda->szImportDir + strlen(pIda->szImportDir), "%4.4ld%2.2ld%2.2ld-", 
        (LONG)(pTimeDate->tm_year + 1900), (LONG)(pTimeDate->tm_mon + 1), (LONG)pTimeDate->tm_mday );
    Utlstrccpy( pIda->szImportDir + strlen(pIda->szImportDir), pIda->szTargetFolder, DOT );
  } /* endif */     
  pIda->ulImportDirLength = strlen(pIda->szImportDir);
} /* end of FolImpBuildUniqueImportPath */


// MTLOG data area for a single input file
typedef struct _MTLOGDATA
{
  FILE *hfMTLog;                       // handle of MTLOG file
  BOOL fRecordAvailable;               // a record is available               
  BOOL fOldFormatRecord;               // record is in old format               
  BOOL fDone;                          // no more records to follow
  ACTSEGLOGOLD OldEntry;               // old format MT log entry
  ACTSEGLOGENH OldEnhEntry;            // fixed part of MT log entry
  ACTSEGLOGENH2 Entry;                 // fixed part of MT log entry
  CHAR EntryBuffer[MAX_SEGMENT_SIZE+MAX_SEGMENT_SIZE+MAX_LONGFILESPEC+20+MAX_SEGMENT_SIZE+MAX_SEGMENT_SIZE+(5 * MAX_SEGMENT_SIZE * sizeof(CHAR_W)) ]; // buffer for variable part of MT log entry
} MTLOGDATA, *PMTLOGDATA;

// MTLOG merge data area
typedef struct _MTLOGMERGEDATA
{
  MTLOGDATA MTLog1;                    // data from input MTLOG1
  MTLOGDATA MTLog2;                    // data from input MTLOG2
  FILE *hfMTLogOut;                    // handle of output MTLOG file
  CHAR szTempMTLog[MAX_LONGFILESPEC];  // temporary MTLOG file name
} MTLOGMERGEDATA, *PMTLOGMERGEDATA;

// MTLOG merge helper function FolMTLogReadNextRecord
BOOL FolMTLogReadNextRecord( PMTLOGDATA pMTLog )
{
  BOOL fOK = TRUE;

  // stop reading at end of file
  if ( feof( pMTLog->hfMTLog ) )
  {
    pMTLog->fDone = TRUE;
    return( TRUE );
  } /* endif */    

  // read the record
  if ( fOK )
  {
    // read and check record eye catcher
    ULONG ulEyeCatcher = 0;
    int iRead = 0;

    iRead = fread( &ulEyeCatcher, 1, sizeof(ULONG), pMTLog->hfMTLog );
    if ( iRead != sizeof(ULONG) )
    {
      pMTLog->fDone = TRUE;
      return( TRUE );
    }

    // read old format record
    else if ( (ulEyeCatcher != ACTSEGLOG_EYECATCHER) && (ulEyeCatcher != ACTSEGLOG2_EYECATCHER)  )
    {
      int iBytesToRead;

      pMTLog->fOldFormatRecord = TRUE;
      memcpy( &pMTLog->OldEntry, &ulEyeCatcher, sizeof(ULONG) ); 
      iBytesToRead = sizeof(pMTLog->OldEntry) - sizeof(ULONG);
      iRead = fread( ((PBYTE)&(pMTLog->OldEntry)) + sizeof(ULONG), 1, iBytesToRead, pMTLog->hfMTLog );
      if ( iRead != iBytesToRead )
      {
        pMTLog->fDone = TRUE;
        return( TRUE );
      }
    }
    else if ( ulEyeCatcher == ACTSEGLOG_EYECATCHER )
    // read older enhanced format record
    {
      int iBytesToRead;

      // read fixed part
      pMTLog->fOldFormatRecord = FALSE;
      memcpy( &pMTLog->OldEnhEntry, &ulEyeCatcher, sizeof(ULONG) ); 
      iBytesToRead = sizeof(pMTLog->OldEnhEntry) - sizeof(ULONG);
      iRead = fread( ((PBYTE)&(pMTLog->OldEnhEntry)) + sizeof(ULONG), 1, iBytesToRead, pMTLog->hfMTLog );
      if ( iRead != iBytesToRead )
      {
        pMTLog->fDone = TRUE;
        return( TRUE );
      }

      // copy data to ACTSEGLOGENH2 structure
      memset( &(pMTLog->Entry), 0, sizeof(pMTLog->Entry) );
      pMTLog->Entry.ulEyeCatcher       = ACTSEGLOG2_EYECATCHER;
      pMTLog->Entry.ulCheckSum         = 0;                // will be recomputed later
      pMTLog->Entry.ulRecordLen        = pMTLog->OldEnhEntry.ulRecordLen + sizeof(pMTLog->Entry) - sizeof(pMTLog->OldEnhEntry);
      pMTLog->Entry.ulSegLen           = pMTLog->OldEnhEntry.ulSegLen;
      pMTLog->Entry.ulPropSourceLen    = pMTLog->OldEnhEntry.ulPropSourceLen;
      pMTLog->Entry.ulMetaDataLen    = pMTLog->OldEnhEntry.ulPropSourceLen;
      pMTLog->Entry.ulPropSourceSegNum = pMTLog->OldEnhEntry.ulPropSourceSegNum;
      pMTLog->Entry.ulSegNum           = pMTLog->OldEnhEntry.ulSegNum;
      pMTLog->Entry.ulTimeStamp        = pMTLog->OldEnhEntry.ulTimeStamp;
      pMTLog->Entry.ulTime             = pMTLog->OldEnhEntry.ulTime;
      pMTLog->Entry.usFuzzyness        = pMTLog->OldEnhEntry.usFuzzyness;
      memcpy( &(pMTLog->Entry.VersionInfo), &(pMTLog->OldEnhEntry.VersionInfo), sizeof(pMTLog->Entry.VersionInfo) );
      memcpy( &(pMTLog->Entry.PropTypeExists), &(pMTLog->OldEnhEntry.PropTypeExists), sizeof(pMTLog->Entry.PropTypeExists) );
      memcpy( &(pMTLog->Entry.PropTypeCopied), &(pMTLog->OldEnhEntry.PropTypeCopied), sizeof(pMTLog->Entry.PropTypeCopied) );
      pMTLog->Entry.ucNumDictCopied    = pMTLog->OldEnhEntry.ucNumDictCopied;
      pMTLog->Entry.usNumTyped         = pMTLog->OldEnhEntry.usNumTyped;
      pMTLog->Entry.usWordCnt          = pMTLog->OldEnhEntry.usWordCnt;
      pMTLog->Entry.AddFlags.AutoSubst = pMTLog->OldEnhEntry.AddFlags.AutoSubst;
      pMTLog->Entry.AddFlags.ShipmentRec = pMTLog->OldEnhEntry.AddFlags.ShipmentRec;

      // read remaining data
      if ( fOK )
      {
        iBytesToRead = pMTLog->OldEnhEntry.ulRecordLen - sizeof(pMTLog->OldEnhEntry);
        iRead = fread( ((PBYTE)&(pMTLog->Entry)) + sizeof(pMTLog->Entry), 1, iBytesToRead, pMTLog->hfMTLog );
        if ( iRead != iBytesToRead )
        {
          fOK = FALSE;
          pMTLog->fDone = TRUE;
        }
      } /* endif */         

      // re-compute checksum
      {
        int i = 0;
        int iLen = (int)pMTLog->Entry.ulRecordLen - sizeof(pMTLog->Entry.ulEyeCatcher) - sizeof(pMTLog->Entry.ulCheckSum);
        PBYTE pbTest = (PBYTE)&(pMTLog->Entry.ulRecordLen);
        pMTLog->Entry.ulCheckSum = 0;
        while ( i < iLen ) pMTLog->Entry.ulCheckSum = pMTLog->Entry.ulCheckSum + (ULONG)pbTest[i++];
      }
    }
    else // read record in enhanced version 2 format
    {
      int iBytesToRead;

      // read fixed part
      pMTLog->fOldFormatRecord = FALSE;
      memcpy( &pMTLog->Entry, &ulEyeCatcher, sizeof(ULONG) ); 
      iBytesToRead = sizeof(pMTLog->Entry) - sizeof(ULONG);
      iRead = fread( ((PBYTE)&(pMTLog->Entry)) + sizeof(ULONG), 1, iBytesToRead, pMTLog->hfMTLog );
      if ( iRead != iBytesToRead )
      {
        pMTLog->fDone = TRUE;
        return( TRUE );
      }

      // read remaining data
      if ( fOK )
      {
        iBytesToRead = pMTLog->Entry.ulRecordLen - sizeof(pMTLog->Entry);
        iRead = fread( ((PBYTE)&(pMTLog->Entry)) + sizeof(pMTLog->Entry), 1, iBytesToRead, pMTLog->hfMTLog );
        if ( iRead != iBytesToRead )
        {
          fOK = FALSE;
          pMTLog->fDone = TRUE;
        }
      } /* endif */         
    } /* endif */       
  } /* endif */     
  return( fOK );
}

// MTLOG merge helper function FolMTLogWriteRecord
BOOL FolMTLogWriteRecord( FILE *hfMTLogOut, PMTLOGDATA pMTLog )
{
  if ( pMTLog->fOldFormatRecord )
  {
    fwrite( &(pMTLog->OldEntry), 1, sizeof(pMTLog->OldEntry), hfMTLogOut );
  }
  else
  {
    fwrite( &(pMTLog->Entry), 1, pMTLog->Entry.ulRecordLen, hfMTLogOut );
  } /* endif */     
  return( TRUE );
}

// MTLOG merge helper function FolMTLogCompareRecord
int FolMTLogCompareRecord( PMTLOGDATA pMTLog1, PMTLOGDATA pMTLog2 )
{
  // at end of file always use other MTLOG
  if ( pMTLog1->fDone ) return( 1 );
  if ( pMTLog2->fDone ) return( -1 );

  // old records are always smaller than new records
  if ( pMTLog1->fOldFormatRecord != pMTLog2->fOldFormatRecord )
  {
    if ( pMTLog1->fOldFormatRecord ) return( -1 );
    if ( pMTLog2->fOldFormatRecord ) return( 1 );
  }
  else if ( pMTLog1->fOldFormatRecord && pMTLog2->fOldFormatRecord )
  {
    // as old records only should occur for the first dummy records we can assume that the records are equal
    return( 0 );
  }
  else
  {
    // use record time stamp for the compare
    return( (int)(pMTLog1->Entry.ulTimeStamp - (int)(pMTLog2->Entry.ulTimeStamp) ) );
  } /* endif */     

  return( 0 );
}


// merge two MTLOGs
BOOL FolMergeMTLog( PSZ pszNewMTLog, PSZ pszTargetMTLog )
{
  PMTLOGMERGEDATA pData = NULL;
  BOOL fOK = TRUE;

  // allocate our data area
  fOK = UtlAlloc( (PVOID *)&pData, 0L, sizeof(MTLOGMERGEDATA), ERROR_STORAGE );
  if (!fOK ) return( fOK );

  // open input MTLOG files
  if ( fOK  )
  {
    pData->MTLog1.hfMTLog = fopen( pszNewMTLog, "rb" );
    pData->MTLog2.hfMTLog = fopen( pszTargetMTLog, "rb" );
    fOK = ((pData->MTLog1.hfMTLog != NULL ) && (pData->MTLog2.hfMTLog != NULL )); 
  } /* endif */     

  // open temporary output MTLOG
  if ( fOK  )
  {
    strcpy( pData->szTempMTLog, pszTargetMTLog );
    strcat( pData->szTempMTLog, ".temp" );
    pData->hfMTLogOut = fopen( pData->szTempMTLog, "wb" );
    fOK = (pData->hfMTLogOut != NULL ); 
  } /* endif */     

  // read first (=markup dummy record) from both input MTLOGs
  if ( fOK )
  {
    fOK = FolMTLogReadNextRecord( &(pData->MTLog1) );
    if ( fOK ) fOK = FolMTLogReadNextRecord( &(pData->MTLog2) );
  } /* endif */     

  // process first dummy record
  if ( fOK  )
  {
     fOK = FolMTLogWriteRecord( pData->hfMTLogOut, &(pData->MTLog1) );
  } /* endif */     

  // read first real record from both input MTLOGs
  if ( fOK )
  {
    fOK = FolMTLogReadNextRecord( &(pData->MTLog1) );
    if ( fOK ) fOK = FolMTLogReadNextRecord( &(pData->MTLog2) );
  } /* endif */     

  // process records until done
  if ( fOK )
  {
    while ( fOK && ( !(pData->MTLog1.fDone && pData->MTLog2.fDone ) ) )
    {
      int iCompare = FolMTLogCompareRecord( &(pData->MTLog1), &(pData->MTLog2) );
      if ( iCompare < 0  )
      {
        fOK = FolMTLogWriteRecord( pData->hfMTLogOut, &(pData->MTLog1) );
        if ( fOK ) fOK = FolMTLogReadNextRecord( &(pData->MTLog1) );
      }
      else if ( iCompare > 0  )
      {
        fOK = FolMTLogWriteRecord( pData->hfMTLogOut, &(pData->MTLog2) );
        if ( fOK ) fOK = FolMTLogReadNextRecord( &(pData->MTLog2) );
      }
      else
      {
        // identical recors, ignore the second one
        fOK = FolMTLogWriteRecord( pData->hfMTLogOut, &(pData->MTLog1) );
        if ( fOK ) fOK = FolMTLogReadNextRecord( &(pData->MTLog1) );
        if ( fOK ) fOK = FolMTLogReadNextRecord( &(pData->MTLog2) );
      } /* endif */         
    } /* endwhile */       
  } /* endif */     

  // close files and replace target MT log with merged one
  if ( fOK  )
  {
    // close all files
    fclose( pData->MTLog1.hfMTLog );
    pData->MTLog1.hfMTLog = NULL;
    fclose( pData->MTLog2.hfMTLog );
    pData->MTLog2.hfMTLog = NULL;
    fclose( pData->hfMTLogOut );
    pData->hfMTLogOut  = NULL;

    // delete old MTlog file
    UtlDelete( pszTargetMTLog, 0L, FALSE );

    // replace with merged MTLOG file
    UtlMove( pData->szTempMTLog, pszTargetMTLog, 0L, FALSE );
 } /* endif */     

  // cleanup 
  if ( pData )
  {
    if ( pData->MTLog1.hfMTLog ) fclose( pData->MTLog1.hfMTLog );
    if ( pData->MTLog2.hfMTLog ) fclose( pData->MTLog2.hfMTLog );
    if ( pData->hfMTLogOut ) fclose( pData->hfMTLogOut );
    UtlAlloc( (PVOID *)&pData, 0L, 0L, NOMSG );
  } /* endif */     

  return( fOK );
}


BOOL FolImpProcessNextMemory( PFOLIMPIDA pIda, PBOOL pfMoreMemories, PBOOL pfContinue  )
{
  BOOL fOK = TRUE;
  *pfMoreMemories = FALSE;

  // alocate or data area if not done yet
  if ( pIda->pvMemImportData == NULL )
  {
    PMEMIMPORTDATA pMemData = new(MEMIMPORTDATA); 
    pIda->pvMemImportData = (PVOID)pMemData;
  } /* endif */           


  if ( pIda->fTranslationManagerFolder )
  {
    // for TranslationManager folder we look for memory property files
    UtlMakeEQFPath( pIda->szObjPath, pIda->chToDrive, PROPERTY_PATH, NULL );
    UtlAddImportToPath( pIda, pIda->szObjPath );

    UtlMakeEQFPath( pIda->szSearchPath, pIda->chToDrive, PROPERTY_PATH, NULL );
    UtlAddImportToPath( pIda, pIda->szSearchPath );
    strcat( pIda->szSearchPath, "\\*" );
    strcat( pIda->szSearchPath, EXT_OF_MEM );
  }
  else
  {
    // for OpenTM2 folder we look for memory info files
    UtlMakeEQFPath( pIda->szObjPath, pIda->chToDrive, MEM_PATH, NULL );
    UtlAddImportToPath( pIda, pIda->szObjPath );

    UtlMakeEQFPath( pIda->szSearchPath, pIda->chToDrive, MEM_PATH, NULL );
    UtlAddImportToPath( pIda, pIda->szSearchPath );
    strcat( pIda->szSearchPath, "\\*" );
    strcat( pIda->szSearchPath, EXT_OF_MEMORYINFO );
  } /* endif */               

  //--- build list of imported memories  ---
  fOK = FolImpBuildObject( pIda->szSearchPath, pIda->szObjPath, pIda->pszBuffer, pIda->ulBufSize, pIda->szCurFile );

  // process first memory from object list
  if ( fOK )
  {
    *pfMoreMemories = TRUE;

    if ( pIda->fTranslationManagerFolder )
    {
       // process TranslationManager memory info file
      PMEMIMPORTDATA pMemData = (PMEMIMPORTDATA)pIda->pvMemImportData; 
      BOOL fOK = TRUE;

      pMemData->strPluginName = "";
      pMemData->strMemFiles = "";
      pMemData->strMemoryName = "";

      // get memory long name from property file
      PPROP_NTM pstMemProp = NULL;
      ULONG ulRead = 0;
      if ( UtlLoadFileL( pIda->szCurFile, (PVOID *)&pstMemProp, &ulRead, FALSE, FALSE ) )
      {
        pMemData->strPluginName = "EqfMemoryPlugin";
        pMemData->strMemoryName = (pstMemProp->szLongName[0] != EOS) ? pstMemProp->szLongName : pstMemProp->stTMSignature.szName;
        pMemData->strMemFiles = pIda->szCurFile;
        pMemData->strMemFiles.append( "," );

        UtlMakeEQFPath( pIda->szInfoFileLine, pIda->chToDrive, MEM_PATH, NULL );
        strcat( pIda->szInfoFileLine, "\\" );
        Utlstrccpy( pIda->szInfoFileLine+strlen(pIda->szInfoFileLine), pstMemProp->stPropHead.szName, '.' );
        strcat( pIda->szInfoFileLine, EXT_OF_TMDATA );
        UtlAddImportToPath( pIda, pIda->szInfoFileLine );
        pMemData->strMemFiles.append( pIda->szInfoFileLine );
        pMemData->strMemFiles.append( "," );

        UtlMakeEQFPath( pIda->szInfoFileLine, pIda->chToDrive, MEM_PATH, NULL );
        strcat( pIda->szInfoFileLine, "\\" );
        Utlstrccpy( pIda->szInfoFileLine+strlen(pIda->szInfoFileLine), pstMemProp->stPropHead.szName, '.' );
        strcat( pIda->szInfoFileLine, EXT_OF_TMINDEX );
        UtlAddImportToPath( pIda, pIda->szInfoFileLine );
        pMemData->strMemFiles.append( pIda->szInfoFileLine );

        UtlAlloc( (PVOID *)&pstMemProp, 0, 0, NOMSG );
      }
      else
      {
        fOK = FALSE;
      }
    }
    else
    {
      // process OpenTM2 memory info file
      fOK = FolImpScanMemoryInfoFile( pIda, pIda->szCurFile  );
    } /* endif */

    // start the actual import
    if ( fOK )
    {
      fOK = FolImpStartMemoryImport( pIda, pfContinue );
    } /* endif */         
  }
  else
  {
    // no more memories to import
    fOK = TRUE;
    *pfMoreMemories = FALSE;
    *pfContinue = FALSE;
  } /* endif */
  return( fOK );
}

// scan a memory info file
BOOL FolImpScanMemoryInfoFile( PFOLIMPIDA pIda, PSZ pszInfoFile )
{
  PMEMIMPORTDATA pMemData = (PMEMIMPORTDATA)pIda->pvMemImportData; 
  BOOL fOK = TRUE;

  pMemData->strPluginName = "";
  pMemData->strMemFiles = "";
  pMemData->strMemoryName = "";


  // read and process info file
  FILE *hfInfoFile = fopen( pszInfoFile, "r" );
  if ( hfInfoFile == NULL )
  {
    fOK = FALSE;
  }
  else
  {
    while ( !feof( hfInfoFile ) )
    {
      memset( pIda->szInfoFileLine, 0, sizeof(pIda->szInfoFileLine) );
      fgets( pIda->szInfoFileLine, sizeof(pIda->szInfoFileLine), hfInfoFile );
      if ( pIda->szInfoFileLine[0] != EOS )
      {
        pIda->szInfoFileLine[strlen(pIda->szInfoFileLine)-1] = EOS;

        PSZ pszValue = strchr( pIda->szInfoFileLine, '=' );
        if ( pszValue != NULL )
        {
          *pszValue = EOS;
          pszValue += 1;

          if ( strcmp( pIda->szInfoFileLine, MEMINFOFILE_PLUGIN_KEY ) == 0 )
          {
            pMemData->strPluginName = pszValue;
          }
          else if ( strcmp( pIda->szInfoFileLine, MEMINFOFILE_FILES_KEY ) == 0 )
          {
            PSZ pszCurPos = pszValue;

            // use import-to drive for all names in the list
            while ( *pszCurPos != EOS )
            {
              *pszCurPos = pIda->chToDrive;
              PSZ pszNext = strchr( pszCurPos, ',' );
              if ( pszNext != NULL )
              {
                pszCurPos = pszNext + 1;
              }
              else
              {
                pszCurPos += strlen(pszCurPos);
              } /* end */                 
            } /* end */               

            pMemData->strMemFiles = pszValue;
          }
          else if ( strcmp( pIda->szInfoFileLine, MEMINFOFILE_NAME_KEY ) == 0 )
          {
            PSZ pszNewName = FolImpGetNewMemoryName( &(pIda->MemRename), pszValue );
            pMemData->strMemoryName = pszNewName;
          }
          else
          {
            // ignore unknown keys

          } /* endif */
        } /* endif */
      }
    } /* endwhile */
    fclose( hfInfoFile );
  } /* endif */

  // check if required infos have been found
  if ( fOK )
  {
    if ( (pMemData->strPluginName.length() == 0) || (pMemData->strMemoryName.length() == 0) || (pMemData->strMemFiles.length() == 0))
    {
      // insuffient data in memory info file
      PSZ   pszErrParm = UtlGetFnameFromPath( pszInfoFile );
      UtlErrorHwnd( ERROR_MEMORYINFO, MB_CANCEL, 1, &pszErrParm, EQF_ERROR, pIda->hwndErrMsg );
      fOK = FALSE;
    } /* endif */
  } /* endif */

  // adjust memory file names
  if ( fOK )
  {
     std::string strCorrectedFiles = "";
     size_t Pos = 0;
     BOOL fComplete = FALSE;
     while ( !fComplete )
     {
       // extract next file name from list
       std::string strCurFile;
       size_t End = pMemData->strMemFiles.find_first_of( ',', Pos );
       if ( End == std::string::npos )
       {
         strCurFile = pMemData->strMemFiles.substr( Pos, std::string::npos );
         fComplete = TRUE;
       }
       else
       {
         strCurFile = pMemData->strMemFiles.substr( Pos, End - Pos );
         Pos = End + 1;
       } /* endif */     

       FolImpCorrectPathName( pIda, strCurFile ); 

       strCorrectedFiles.append( strCurFile );
       if ( !fComplete ) strCorrectedFiles.append( "," ); 
     } /* endwhile */        
     pMemData->strMemFiles = strCorrectedFiles;
  }


  return( fOK );
} /* end of function FolImpScanMemoryInfoFile */

// start import of memory files
BOOL FolImpStartMemoryImport( PFOLIMPIDA pIda, PBOOL pfContinue )
{
  PMEMIMPORTDATA pMemData = (PMEMIMPORTDATA)pIda->pvMemImportData; 
  BOOL fOK = TRUE;
  BOOL fSkip = FALSE;
  *pfContinue = FALSE;

  pMemData->pMemPlugin = NULL;

  // check if required memory plugin is available
  if ( fOK )
  {
    PluginManager* pPluginManager = PluginManager::getInstance();
    pMemData->pMemPlugin = (OtmMemoryPlugin *) pPluginManager->getPlugin( pMemData->strPluginName.c_str() );
    if ( pMemData->pMemPlugin == NULL )
    {
      PSZ   pszErrParm = (PSZ)pMemData->strPluginName.c_str();
      UtlErrorHwnd( ERROR_MEMPLUGINREQUIRED, MB_CANCEL, 1, &pszErrParm, EQF_ERROR, pIda->hwndErrMsg );
      fOK = FALSE;
    } /* endif */
  } /* endif */

  // check if the memory is a new one
  if ( fOK )
  {
    MemoryFactory *pFactory = MemoryFactory::getInstance();
    pMemData->fNewMemory = !pFactory->exists( (PSZ)pMemData->strPluginName.c_str(), (PSZ)pMemData->strMemoryName.c_str() );

    // If memory exists, ask user if should merge it or not.    5-2-14
    if ( ! pMemData->fNewMemory ) {
         PSZ   pszErrParm = (PSZ)pMemData->strMemoryName.c_str();
         USHORT usMBReturn;
         usMBReturn = UtlErrorHwnd( QUERY_MEM_FOLIMP_MERGE, MB_YESNO|MB_DEFBUTTON2, 1,
                                           &pszErrParm, EQF_QUERY, pIda->hwndErrMsg );
         if ( usMBReturn == MBID_NO )
         {
           fSkip = TRUE;
           UtlDelete( pIda->szCurFile, 0L, FALSE );
         } /* endif */
    }
  } /* endif */     

  // call memory files import function
  if ( fOK && ! fSkip )
  {
    pMemData->pPrivPluginData= NULL;
    fOK = FolImpContinueMemoryImport( pIda, pfContinue );
  } /* endif */

  return( fOK );
} /* end of function FolImpStartMemoryImport */

// continue import of memory files
BOOL FolImpContinueMemoryImport( PFOLIMPIDA pIda, PBOOL pfContinue )
{
  PMEMIMPORTDATA pMemData = (PMEMIMPORTDATA)pIda->pvMemImportData; 

  int iRC = 0;
  if ( pMemData->pMemPlugin->getType() == OtmPlugin::eTranslationMemoryType )
  {
    iRC = pMemData->pMemPlugin->importFromMemoryFiles( (PSZ)pMemData->strMemoryName.c_str(), (PSZ)pMemData->strMemFiles.c_str(), 0, &(pMemData->pPrivPluginData) );
  }
  else if ( pMemData->pMemPlugin->getType() == OtmPlugin::eSharedTranslationMemoryType )
  {
    iRC = ((OtmSharedMemoryPlugin *)pMemData->pMemPlugin)->importFromMemoryFiles( (PSZ)pMemData->strMemoryName.c_str(), (PSZ)pMemData->strMemFiles.c_str(), 0, &(pMemData->pPrivPluginData) );
  }

  *pfContinue = (iRC == OtmMemoryPlugin::eRepeat) ? TRUE : FALSE;

  // delete currently imported file if import does not continue 
  if ( !*pfContinue )
  {
    UtlDelete( pIda->szCurFile, 0L, FALSE );
  } /* end */    

  // for new memories send "created" notification
  if ( !*pfContinue && pMemData->fNewMemory )
  {
    PSZ pszObjName = NULL;
    UtlAlloc( (PVOID *)&pszObjName, 0L, MAX_LONGFILESPEC + MAX_LONGFILESPEC + 2, NOMSG );
    if ( pszObjName != NULL )
    {
      strcpy( pszObjName, pMemData->strPluginName.c_str() );
      strcat( pszObjName, ":" );
      strcat( pszObjName, pMemData->strMemoryName.c_str() );
      EqfSend2Handler( MEMORYHANDLER, WM_EQFN_CREATED, MP1FROMSHORT( clsMEMORYDB ), MP2FROMP( pszObjName ));
      if ( pszObjName != NULL ) UtlAlloc( (PVOID *)&pszObjName, 0L, 0L, NOMSG );
    } /* end */       
  } /* end */    


  return( (iRC == OtmMemoryPlugin::eSuccess) || (iRC == OtmMemoryPlugin::eRepeat) );
} /* end of function FolImpProcessMemoryInfoFile */


void FolImpImportPathChanged( HWND hwnd, PFOLIMPIDA pIda )
{
  SETTEXT( hwnd, ID_FOLIMP_FILENAME_CB, pIda->szImportPath );

  //--- refresh packaged folders listbox ---
  if (pIda->szImportPath[0] != EOS)
  {
    pIda->chFromDrive =  pIda->szImportPath[0];
  }

  // check if selected directory exists
  if ( !UtlDirExist( pIda->szImportPath ) )
  {
    PSZ pszErrParm = pIda->szImportPath;
    UtlError( ERROR_PATH_NOT_FOUND_MSG, MB_CANCEL, 1, &pszErrParm,
              EQF_ERROR );
  } /* endif */

  FolRefreshFolderListbox( hwnd, pIda );
  if ( QUERYITEMCOUNT( hwnd, ID_FOLIMP_FOLDER_LB ) == 0 )
  {
    ENABLECTRL( pIda->hwndDlg, ID_FOLIMP_INFO_PB, FALSE );
    ENABLECTRL( pIda->hwndDlg, ID_FOLIMP_IMPORT_PB, FALSE );
    ENABLECTRL( pIda->hwndDlg, ID_FOLINFO_PRINT_PB, FALSE );
    SETTABSTOP( pIda->hwndDlg, ID_FOLIMP_CANCEL_PB );
  }
  else
  {
    SELECTITEMMS( hwnd, ID_FOLIMP_FOLDER_LB, 0 );
  } /* endif */
}

// get the index of the entry for the specified old memory name
int FolImpGetRenameMemoryIndex( PFOLMEMRENAMEDATA pRenameData, PSZ pszMemName )
{
  int i = 0;
  while ( i < pRenameData->iMemRenames ) 
  {
    if ( strcmp( pszMemName, pRenameData->aszOldMemNames[i] ) == 0 )
    {
      return( i );
    } /* endif */
    i++;
  } /* endwhile */
  return( -1 );
} /* end of FolImpGetRenameMemoryIndex */

// get the index of the entry for the specified old memory short name
int FolImpGetRenameMemoryShortNameIndex( PFOLMEMRENAMEDATA pRenameData, PSZ pszMemShortName )
{
  int i = 0;
  while ( i < pRenameData->iMemRenames ) 
  {
    if ( strcmp( pszMemShortName, pRenameData->aszOldMemShortNames[i] ) == 0 )
    {
      return( i );
    } /* endif */
    i++;
  } /* endwhile */
  return( -1 );
} /* end of FolImpGetRenameMemoryShortNameIndex */

// apply any rename request on the memory name
PSZ FolImpGetNewMemoryName( PFOLMEMRENAMEDATA pRenameData, PSZ pszMemName )
{
  int i = FolImpGetRenameMemoryIndex( pRenameData, pszMemName );
  if ( i >= 0 )
  {
    return( pRenameData->aszNewMemNames[i] );
  }
  else
  {
    return( pszMemName );
  } /* endif */
} /* end of FolImpGetNewMemoryName */

// setup memory rename table
USHORT FolImpBuildMemRenameTable( PPROPFOLDER pFolProp, PFOLMEMRENAMEDATA pRenameData, PSZ pszNewMemNames )
{
  CHAR szMemName[MAX_LONGFILESPEC+1];
  int iMemIndex = -1;
  USHORT usRC = 0;

  memset( pRenameData, 0, sizeof(FOLMEMRENAMEDATA) );
  pRenameData->iMemRenames = 0;
  PSZ pszCurrentName = pszNewMemNames;
  while ( (*pszCurrentName != EOS) && (iMemIndex < MAX_NUM_OF_READONLY_MDB) && (usRC == 0) )
  {
    // extract next new memory name
    PSZ pszCurPos = szMemName;
    while ( isspace( *pszCurrentName ) ) pszCurrentName++;
    while ( (*pszCurrentName != ',') && (*pszCurrentName != EOS) )
    {
      *pszCurPos++ = *pszCurrentName++;
    } /* endwhile */
    *pszCurPos = EOS;
    if (*pszCurrentName == ',') pszCurrentName++;

    // add corresponding old memory name and new memory name to rename table
    if ( szMemName[0] != EOS )
    {
      if (!UtlCheckLongName( szMemName) )
      {
        PSZ pszParm = szMemName;
        usRC = ERROR_INV_LONGNAME;
        UtlErrorHwnd( ERROR_INV_LONGNAME, MB_CANCEL, 1, &pszParm, EQF_ERROR, HWND_FUNCIF );
      } 
      else if ( iMemIndex == -1 )
      {
        strcpy( pRenameData->aszOldMemNames[pRenameData->iMemRenames], pFolProp->szLongMemory ); // use folder memory 
      }
      else
      {
        strcpy( pRenameData->aszOldMemNames[pRenameData->iMemRenames], pFolProp->aLongMemTbl[iMemIndex] ); // use search memory 
      } /* endif */

      if ( (usRC == 0) && (pRenameData->aszOldMemNames[pRenameData->iMemRenames][0] != EOS) )
      {
        BOOL fIsNew = FALSE;

        strcpy( pRenameData->aszNewMemNames[pRenameData->iMemRenames], szMemName );

        // get short name of original memory
        ObjLongToShortName( pRenameData->aszOldMemNames[pRenameData->iMemRenames], pRenameData->aszOldMemShortNames[pRenameData->iMemRenames], TM_OBJECT, &fIsNew );

        // get new short name for new memory 
        ObjLongToShortName( szMemName, pRenameData->aszNewMemShortNames[pRenameData->iMemRenames], TM_OBJECT, &fIsNew );

        // ensure that the new name is not in use by one of the other memories being renamed
        BOOL fNameIsInUse = FALSE;
        int iCounter = 1;
        do
        {
          fNameIsInUse = FALSE;
          int i = 0;
          while ( !fNameIsInUse && (i < pRenameData->iMemRenames) )
          {
            if ( strcmp( pRenameData->aszNewMemShortNames[pRenameData->iMemRenames], pRenameData->aszNewMemShortNames[i]) == 0 )
            {
              fNameIsInUse = TRUE;
            } /* endif */
            i++;
          } /* endwhile */

          // build new name when name is in use
          if ( fNameIsInUse )
          {
            int iNameLen = strlen( pRenameData->aszNewMemShortNames[pRenameData->iMemRenames] );
            if ( iNameLen > 5 )
            {
              iNameLen = 5;
            } /* endif */
            sprintf( pRenameData->aszNewMemShortNames[pRenameData->iMemRenames] + iNameLen, "%3.3ld", iCounter++ );
          } /* endif */
        } while ( fNameIsInUse );


        pRenameData->iMemRenames += 1;
      } /* endif */
    } /* endif */
    iMemIndex++;
  } /* endwhile */
  return( usRC );
} /* end of FolImpBuildMemRenameTable */

//   End of EQFFOL02.C






