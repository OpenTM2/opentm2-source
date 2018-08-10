//------------------------------------------------------------------------------
//  EQFFOL06.C - EQF Folder Handler - Folder Export Related Stuff
//------------------------------------------------------------------------------
//  Copyright Notice:
//
//      Copyright (C) 1990-2016, International Business Machines
//      Corporation and others. All rights reserved
//------------------------------------------------------------------------------

// activate tree control under Windows
#define USETREECONTROL


#ifndef CPPTEST
extern "C"
{
#endif#define INCL_EQF_TAGTABLE         // tagtable defines and functions
#define INCL_EQF_PRINT            // print functions
#define INCL_EQF_FOLDER           // folder list and document list functions
#define INCL_EQF_ASD              // dictionary access functions
#define INCL_EQF_TM             //
#define INCL_EQF_ANALYSIS         // analysis functions
#define INCL_EQF_TP               // public translation processor functions
#define INCL_EQF_TAGTABLE         // tagtable defines and functions
#include <eqf.h>                  // General Translation Manager include file
#include "eqftmi.h"
#include <eqftai.h>
#include <otmapi.h>

#include "eqfutpck.h"                  // package utilities
#include "eqfdde.h"                    // batch mode functions
#include "eqffol00.h"                  // our .h stuff
#include "eqffol.id"                   // our ID file
#include <time.h>                      // C standard time functions
#include "EQFHLOG.H"                 // defines for history log processing
  #include "OTMFUNC.H"            // public defines for function call interface
  #include "EQFFUNCI.H"           // private defines for function call interface

  #include "SHLOBJ.H"                  // folder browse function
#include "eqfxliff.h"                  // XLIFF mode export/import
#include "eqfaprof.h"                  // analysis profile functions
#ifndef CPPTEST
}
#endif

#include "vector"
#include "core\pluginmanager\PluginManager.h"
#include "core\pluginmanager\OtmMarkup.h"
#include "core\pluginmanager\OtmMemory.h"
#include "core\pluginmanager\OtmMemoryPlugin.h"
#include "core\memory\MemoryFactory.h"
#include "MarkupPluginMapper.h"

// last used values for export to directory
#define FOLIMPEXPLASTUSEDDIR "FOLDERIMPORTEXPORT"

// buffer size used when calling user exit function EqfQueryExitInfo
#define QUERYEXITINFOBUFSIZE 2048

#define FOL_EXPORT_TASK USER_TASK + 1  // export-a-folder task

MRESULT FolExportCallBack( PPROCESSCOMMAREA, HWND, WINMSG, WPARAM, LPARAM );

USHORT FolPrepFolderExport
(
PFOLEXPIDA       pIda,               // folder import dialog IDA
PSZ              pszDocuments,       // list of documents or NULL
HWND             hwndOwner           // owner window handle for error msgs
);

BOOL FolExpAddLBDocs( HWND, HWND, PFOLEXPIDA, BOOL, BOOL, HWND );
int FolExpCleanup( PFOLEXPIDA pIda );

BOOL FolExpAddDoc( PFOLEXPIDA, PSZ, BOOL, BOOL, HWND );
BOOL FolExpAddDocToList( PFOLEXPIDA pIda, PFOLIMPEXPXLIFF pXLIFFData, PSZ pszDocShortName );
void FolPropSetEnableState
(
  HWND        hwndDlg,
  PFOLEXPIDA  pIda
);

BOOL FolExpAddTagTable
(
  PFOLEXPIDA       pIda,               // ptr to folder export isntance data area
  PSZ              pszFormat,          // name of tag table / format table
  HATOMTBL         hAtomTbl,           // atom table for in-error objects
  HWND             hwnd,               // owner handle for error messages
  BOOL             fOldVers            // export in old format flag
);

USHORT FolFuncPrepExportFolder
(
PFCTDATA    pData,                   // function I/F session data
PSZ         pszFolderName,           // name of folder
PSZ         pszTargetPath,           // folder target path
LONG        lOptions,                // options for the folder export or 0L
PSZ         pszDocuments,            // list of documents or NULL
PSZ         pszDescription,           // export description or NULL
PSZ         pszExportAs,
PSZ         pszMemoryExportAs
);
ULONG FolFuncExportFolderProcess
(
PFCTDATA    pData                    // function I/F session data
);
  // flag to distinguish documents and subfolders in tree view control
  #define SUBFOLDERFLAG 0x8000000

  int FolExpSetTreeCheck( HWND hwndTV, HTREEITEM hItem, BOOL fCheck );
  int FolExpSelCheckedTreeItems( HWND hwndTV, HTREEITEM hItem, HWND hwndLB, PSUBFOLINFO pInfo, PSZ pszBuffer, BOOL fIncludeAll );
  int FolExpSetSubFolInfoFlag( ULONG ulID, PSUBFOLINFO pInfo );

MRESULT FolExpPropCommand(HWND hwnd, WPARAM mp1, LPARAM mp2);


HWND hwndExportInstance = NULLHANDLE;                  // handle of export instance window

HWND GetFolderExportHandle()
{
  return( hwndExportInstance );
}

VOID FolderExport( HWND hwnd, PSZ pSelFolderName, PSZ pszObjList )
{
  PFOLEXPIDA      pIda;               // folder import dialog IDA
  SHORT           sRC;                // buffer for return codes
  BOOL            fOK = TRUE;         // return value
  PSZ             pszErrParm;         // error parameter pointer
  BOOL            fFolderIsLocked = FALSE; // 'folder has been locked' flag

  hwnd;                               // suppress compiler warnings

  // create FolExpIda
  fOK = UtlAlloc( (PVOID *)&pIda, 0L, (LONG) sizeof(FOLEXPIDA), ERROR_STORAGE );

  // check if folder is locked
  if ( fOK )
  {
    sRC = QUERYSYMBOL( pSelFolderName );
    if ( sRC == -1 )
    {
      // folder is not in use ==> lock this folder
      SETSYMBOL( pSelFolderName );
      fFolderIsLocked = TRUE;
    }
    else
    {
      // folder is in use ...
      fOK = FALSE;
      strcpy( pIda->szObjName, pSelFolderName );
      Utlstrccpy( pIda->szName, UtlGetFnameFromPath( pIda->szObjName), DOT );
      ObjShortToLongName( pIda->szName, pIda->szFolLongName, FOLDER_OBJECT );
      OEMTOANSI(pIda->szFolLongName);
      pszErrParm = pIda->szFolLongName;

      UtlError( ERROR_FOLDER_LOCKED, MB_CANCEL, 1, &pszErrParm, EQF_INFO );
    } /* endif */
  } /* endif */

  if ( fOK )
  {
    HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);

    strcpy( pIda->szObjName, pSelFolderName );
    strcpy( pIda->szFolderObjName, pSelFolderName );
    pIda->pszObjList = pIda->pszActiveObj = pszObjList;

    DIALOGBOX( EqfQueryTwbClient(), FOLEXPPROPDLGPROC, hResMod, ID_FOLEXPPROP_DLG,
               pIda, fOK );

    if ( fOK )
    {
      /**************************************************************/
      /* Create unique object name                                  */
      /**************************************************************/
    } /* endif */

    if ( fOK )
    {
      sprintf( pIda->szObjName, "FOLEXP:%s", pIda->szName );
      fOK = CreateProcessWindow( pIda->szObjName, FolExportCallBack, pIda );
    } /* endif */
  } /* endif */

  if ( !fOK )
  {
    if ( fFolderIsLocked )
    {
      REMOVESYMBOL( pSelFolderName );
    } /* endif */

    if ( pIda )
    {
      if ( pszObjList ) UtlAlloc( (PVOID *)&pszObjList, 0L, 0L, NOMSG) ;
      UtlAlloc( (PVOID *)&(pIda), 0L, 0L, NOMSG) ;
    } /* endif */
  } /* endif */
} /* end of FolderExport */

MRESULT FolExportCallBack
(
PPROCESSCOMMAREA pCommArea,          // ptr to commmunication area
HWND             hwnd,               // handle of process window
WINMSG           message,            // message to be processed
WPARAM           mp1,                // first message parameter
LPARAM           mp2                 // second message parameter
)
{
  PFOLEXPIDA      pIda;                // pointer to instance area
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
        hwndExportInstance  = hwnd;
        pIda                = (PFOLEXPIDA)PVOIDFROMMP2(mp2);
        pCommArea->pUserIDA = pIda;

        /****************************************************************/
        /* supply all information required to create the process        */
        /* window                                                       */
        /****************************************************************/
        pCommArea->sProcessWindowID = ID_FOLEXP_WINDOW;
        pCommArea->sProcessObjClass = clsFOLDEREXP;
        if ( pIda->pDDEFolExp == NULL )
        {
          pCommArea->Style            = PROCWIN_SLIDERONLY;
        }
        else
        {
          pCommArea->Style            = PROCWIN_BATCH;
        } /* endif */
        pCommArea->sSliderID        = ID_FOLEXP_SLIDER;
        LOADSTRING( NULLHANDLE, hResMod, SID_FOLEXP_TITLE, pCommArea->szTitle );

        strcat ( pCommArea->szTitle, pIda->szFolLongName );

        pCommArea->hIcon            = (HPOINTER) UtlQueryULong(QL_FOLEXPICON); //hiconFOLEXP;
        pCommArea->fNoClose         = FALSE;
        pCommArea->swpSizePos.x     = 100;
        pCommArea->swpSizePos.y     = 100;
        pCommArea->swpSizePos.cx    = (SHORT) UtlQueryULong( QL_AVECHARWIDTH ) * 60;
        pCommArea->swpSizePos.cy    = (SHORT) UtlQueryULong( QL_PELSPERLINE ) * 10;
        pCommArea->asMsgsWanted[0]  = WM_EQF_PROCESSTASK;
        pCommArea->asMsgsWanted[1]  = WM_EQF_ABOUTTODELETE;
        pCommArea->asMsgsWanted[3]  = 0;
        pCommArea->usComplete       = 0;
      }
      break;

      /****************************************************************/
      /* Start processing by posting WM_EQF_PROCESSTASK               */
      /****************************************************************/
    case WM_EQF_INITIALIZE:
      /**************************************************************/
      /* set owner handle for messages                              */
      /**************************************************************/
      pIda = (PFOLEXPIDA)pCommArea->pUserIDA;
      if ( pIda->pDDEFolExp == NULL )
      {
        UtlPackSetOwnerHwnd( pIda->pPackCtrl, hwnd );
        pIda->hwndErrMsg = hwnd;
      }
      else
      {
        UtlPackSetOwnerHwnd( pIda->pPackCtrl, pIda->pDDEFolExp->hwndErrMsg );
        pIda->hwndErrMsg = pIda->pDDEFolExp->hwndErrMsg;
      } /* endif */

      WinPostMsg( hwnd, WM_EQF_PROCESSTASK, MP1FROMSHORT(FOL_EXPORT_TASK), 0L );
      break;

      /******************************************************************/
      /* WM_CLOSE:                                                      */
      /*                                                                */
      /* Prepare/initialize shutdown of process                         */
      /******************************************************************/
    case WM_CLOSE:
      pIda = (PFOLEXPIDA)pCommArea->pUserIDA;
      if ( pIda )
      {
        pIda->fKill = TRUE;
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
      pIda = (PFOLEXPIDA)pCommArea->pUserIDA;
      if ( pIda->pPackCtrl )
      {
        UtlPackCleanup( pIda->pPackCtrl );
        pIda->pPackCtrl = NULL;
      } /* endif */

      // call general folder export cleanup
      FolExpCleanup( pIda );


      /**************************************************************/
      /* Tell DDE handler that we are thru ...                      */
      /**************************************************************/
      if ( pIda->pDDEFolExp != NULL )
      {
        WinPostMsg( pIda->pDDEFolExp->hwndOwner, WM_EQF_DDE_ANSWER,
                    NULL, &pIda->pDDEFolExp->DDEReturn );

      } /* endif */

      REMOVESYMBOL( pIda->szFolderObjName );
      if ( pIda )
      {
        if ( pIda->pszNameList ) UtlAlloc( (PVOID *)&(pIda->pszNameList), 0L, 0L, NOMSG );
        if ( pIda->pszObjList ) UtlAlloc( (PVOID *)&(pIda->pszObjList), 0L, 0L, NOMSG );
        UtlAlloc( (PVOID *)&pIda, 0L, 0L, NOMSG );
      } /* endf */
      pCommArea->pUserIDA = NULL;
      hwndExportInstance = NULLHANDLE;
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
        pIda = (PFOLEXPIDA)pCommArea->pUserIDA;
        if ( stricmp( pIda->szObjName, (PSZ)mp2 ) == 0 )
        {
          if ( !pIda->fDeleteInProgress )    // if the folder is not deleted
          {
            // by folder export ...
            mResult = MRFROMSHORT( TRUE );  // don't allow delete of folder
          } /* endif */
        } /* endif */
      } /* endif */
      return( mResult );
      break;

    case WM_EQF_PROCESSTASK:
      switch ( SHORT1FROMMP1(mp1) )
      {
        case FOL_EXPORT_TASK:
          pIda = (PFOLEXPIDA)pCommArea->pUserIDA;
          if ( pIda->fKill )
          {
            USHORT  usMBCode;        // return code of message box call
            PSZ     pszErrParm;      // error parameter table

            if ( pIda->pDDEFolExp != NULL )
            {
              // we are in batch mode, continue cancel of export
              usMBCode = MBID_YES;
            }
            else
            {
              pszErrParm = pIda->szFolLongName;
              usMBCode = UtlError( ERROR_FOLEXP_CANCEL, MB_YESNO,
                                   1, &pszErrParm, EQF_QUERY );
            } /* endif */
            if ( usMBCode == MBID_YES )
            {
              if ( (pIda->usExportFormat == FOL_TMGR_FORMAT) && pIda->pPackCtrl )
              {
                UtlPackCleanup( pIda->pPackCtrl );
                pIda->pPackCtrl = NULL;
              } /* endif */

              // call general folder export cleanup
              FolExpCleanup( pIda );


              //--- get rid off exported file ---
              UtlMakeEQFPath( pIda->szObjPath, pIda->chDrive,
                              EXPORT_PATH, NULL );
              strcat( pIda->szObjPath, BACKSLASH_STR );
              strcat( pIda->szObjPath, pIda->szExportName );
              switch ( pIda->usExportFormat )
              {
                case FOL_TMGR_FORMAT : strcat( pIda->szObjPath, FOLDEREXPORT_EXT ); break;
                case FOL_XLIFF_FORMAT : strcat( pIda->szObjPath, FOLDEREXPORTXLIFF_EXT ); break;
                default:
                    break;
              } /*endswitch */
              UtlDelete( pIda->szObjPath, 0L, FALSE );

              EqfRemoveObject( TWBFORCE, hwnd);
            }
            else
            {
              pIda->fKill = FALSE;
              WinPostMsg( hwnd, WM_EQF_PROCESSTASK,
                          MP1FROMSHORT(FOL_EXPORT_TASK), 0l );
            } /* endif */
          }
          else
          {
            ULONG   ulRC;            // return code of called functions
            CHAR    szErrValue[10];  // buffer for error parameters
            PSZ     pszErrParm[3];   // error parameter table

            switch ( pIda->usExportFormat )
            {
              case FOL_TMGR_FORMAT :
                ulRC = UtlPackWritePackage( pIda->pPackCtrl, &pCommArea->usComplete );
                if ( pIda->pDDEFolExp == NULL )
                {
                  WinSendMsg( hwnd, WM_EQF_UPDATESLIDER,
                              MP1FROMSHORT(pCommArea->usComplete), NULL );
                } /* endif */
                switch ( ulRC )
                {
                  case PACK_OK_RC:
                    //--- continue in writing package ---
                    UtlDispatch();
                    WinPostMsg( hwnd, WM_EQF_PROCESSTASK,
                                MP1FROMSHORT(FOL_EXPORT_TASK), 0l );
                    break;

                  case PACK_COMPLETE_RC:
                    //--- ensure slider shows 100% complete ---
                    pCommArea->usComplete = 100;
                    if ( pIda->pDDEFolExp == NULL )
                    {
                      WinSendMsg( hwnd, WM_EQF_UPDATESLIDER,
                                  MP1FROMSHORT(pCommArea->usComplete), NULL );
                    } /* endif */

                    // process folder delete flag
                    if ( pIda->fDelete )
                    {
                      /*************************************************/
                      /* Delete folder and remove symbol temporarly    */
                      /* (or delete will issue 'in use' message)       */
                      /*************************************************/
                      pIda->fDeleteInProgress = TRUE;
                      REMOVESYMBOL( pIda->szFolderObjName );
                      if ( pIda->pDDEFolExp == NULL )
                      {
                        EqfSend2Handler( FOLDERHANDLER, WM_EQF_DELETE,
                                        MP1FROMHWND(hwnd),
                                        MP2FROMP(pIda->szFolderObjName) );
                      }
                      else
                      {
                        EqfSend2Handler( FOLDERHANDLER, WM_EQF_DELETE,
                                        MP1FROMHWND(pIda->pDDEFolExp->hwndErrMsg),
                                        MP2FROMP(pIda->szFolderObjName) );
                      } /* endif */
                      SETSYMBOL( pIda->szFolderObjName );
                    } /* endif */

                    // continue with next folder if any
                    if ( pIda->pszObjList )
                    {
                      BOOL fOK = TRUE;

                      pIda->pszActiveObj += strlen(pIda->pszActiveObj) + 1;

                      if ( *pIda->pszActiveObj )
                      {
                        // termante current export
                        if ( pIda->pPackCtrl )
                        {
                          UtlPackCleanup( pIda->pPackCtrl );
                          pIda->pPackCtrl = NULL;
                        } /* endif */
                        REMOVESYMBOL( pIda->szFolderObjName );

                        // call general folder export cleanup
                        FolExpCleanup( pIda );

                        // check in-use condition, lock folder
                        {
                          SHORT sRC;

                          strcpy( pIda->szObjName, pIda->pszActiveObj );
                          strcpy( pIda->szFolderObjName, pIda->pszActiveObj );
                          strcpy( pIda->szFolName, UtlGetFnameFromPath( pIda->szObjName) );
                          Utlstrccpy( pIda->szName, pIda->szFolName, DOT );
                          ObjShortToLongName( pIda->szName, pIda->szFolLongName, FOLDER_OBJECT );
                          sRC = QUERYSYMBOL( pIda->szFolderObjName );
                          if ( sRC == -1 )
                          {
                            // folder is not in use ==> lock this folder
                            SETSYMBOL( pIda->szFolderObjName );
                          }
                          else
                          {
                            // folder is in use ...
                            PSZ pszParm;
                            fOK = FALSE;
                            OEMTOANSI(pIda->szFolLongName);
                            pszParm = pIda->szFolLongName;
                            UtlError( ERROR_FOLDER_LOCKED, MB_CANCEL, 1, &pszParm, EQF_INFO );
                          } /* endif */
                        }

                        // show new folder name in export window title bar
                        if ( fOK )
                        {
                          HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
                          LOADSTRING( NULLHANDLE, hResMod, SID_FOLEXP_TITLE, pCommArea->szTitle );
                          strcat ( pCommArea->szTitle, pIda->szFolLongName );
                          SendMessage( hwnd, WM_SETTEXT, 0, (LPARAM)pCommArea->szTitle );
                        } /* endif */

                        // create new package header
                        if ( fOK )
                        {
                          FOLEXPBITFLAGS SavedFlags = pIda->pHeader->BitFlags;
                          UtlAlloc( (PVOID *)&pIda->pHeader, 0L, 0L, NOMSG );
                          fOK = UtlAlloc( (PVOID *)&pIda->pHeader, 0L, (LONG) sizeof(FOLEXPHEADER), ERROR_STORAGE );
                          if ( pIda->pHeader )
                          {
                            pIda->pHeader->BitFlags = SavedFlags;
                            pIda->ulHeaderSize = sizeof(FOLEXPHEADER);
                            pIda->pHeader->BitFlags.fHeaderType  = RELEASE1_HEADER;
                            pIda->pHeader->Head.Rel1.usNoteSize = sizeof(pIda->pHeader->Head.Rel1.szNoteBuffer);
                          } /* endif */
                        } /* endif */

                        // load document names to be exported
                        if ( fOK )
                        {
                          // create both document name listboxes
                          // (they are destroyed in the WM_CLOSE handling of the
                          //  export dialog)
                          pIda->hwndDocLB = WinCreateWindow( hwnd, WC_LISTBOX, "", 0,
                                                            0, 0, 0, 0,
                                                            hwnd, HWND_TOP, 1, NULL, NULL );
                          pIda->hwndLongNameLB = WinCreateWindow( hwnd, WC_LISTBOX, "", 0,
                                                                  0, 0, 0, 0,
                                                                  hwnd, HWND_TOP, 1, NULL, NULL );

                          // get folder documents
                          LoadDocumentNames( pIda->szObjName, pIda->hwndDocLB,
                                            LOADDOCNAMES_NAME | LOADDOCNAMES_INCLSUBFOLDERS,
                                            NULL );

                          // fill long name document listbox and check for document TMs
                          {
                            SHORT sItem = QUERYITEMCOUNTHWND( pIda->hwndDocLB );

                            while ( sItem-- )
                            {
                              SHORT sInsertItem;           // index of inserted item

                              // get the document name
                              QUERYITEMTEXTHWND( pIda->hwndDocLB, sItem, pIda->szDocName );

                              // build document object name
                              UtlMakeEQFPath( pIda->szDocObjName,
                                              pIda->szObjName[0], SYSTEM_PATH,
                                              pIda->szFolName );
                              strcat( pIda->szDocObjName, BACKSLASH_STR );
                              strcat( pIda->szDocObjName, pIda->szDocName );

                              // get/set the long name and the TM for this document
                              pIda->szLongName[0] = EOS;
                              DocQueryInfo2( pIda->szDocObjName, pIda->szDocMem, NULL, NULL, NULL,
                                            pIda->szLongName, NULL, NULL, TRUE );

                              // add long name to long name listbox
                              if ( pIda->szLongName[0] != EOS )
                              {
                                OEMTOANSI( pIda->szLongName );
                                sInsertItem = INSERTITEMHWND( pIda->hwndLongNameLB, pIda->szLongName );
                              }
                              else
                              {
                                sInsertItem = INSERTITEMHWND( pIda->hwndLongNameLB, pIda->szDocName );
                              } /* endif */

                              // set item handle to index of document in invisible document lb
                              if ( sInsertItem >= 0 )
                              {
                                SETITEMHANDLEHWND( pIda->hwndLongNameLB, sInsertItem, sItem );
                              } /* endif */
                            } /* endwhile */
                          } /* endif */
                        } /* endif */

                        // prepare folder export
                        if ( fOK )
                        {
                          USHORT usRC;
                          SubFolCreateInfoTable( pIda->szObjName, &pIda->pSubFolInfo );
                          usRC = FolPrepFolderExport( pIda, NULL, hwnd );
                          if ( pIda->pSubFolInfo ) UtlAlloc( (PVOID *)&pIda->pSubFolInfo, 0L, 0L, NOMSG );
                          fOK = (usRC == NO_ERROR);
                          // get rid off our listboxes
                          if ( pIda->hwndDocLB != NULLHANDLE ) WinDestroyWindow( pIda->hwndDocLB );
                              pIda->hwndDocLB = NULLHANDLE;
                          if ( pIda->hwndLongNameLB != NULLHANDLE ) WinDestroyWindow( pIda->hwndLongNameLB );
                              pIda->hwndLongNameLB = NULLHANDLE;
                        } /* endif */

                        // update process window title and restart export process
                        if ( fOK )
                        {
                          HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
                          LOADSTRING( NULLHANDLE, hResMod, SID_FOLEXP_TITLE, pCommArea->szTitle );
                          strcat ( pCommArea->szTitle, pIda->szFolLongName );
                          WinPostMsg( hwnd, WM_EQF_PROCESSTASK, MP1FROMSHORT(FOL_EXPORT_TASK), 0l );
                        } /* endif */

                        // end processing in case of errors
                        if ( !fOK )
                        {
                          EqfRemoveObject( TWBFORCE, hwnd);
                        } /* endif */
                      }
                      else
                      {
                        // issue completed message
                        if ( pIda->pDDEFolExp == NULL )
                        {
                          szErrValue[0] = pIda->chDrive;
                          szErrValue[1] = EOS;
                          pszErrParm[0] = pIda->pszNameList;
                          pszErrParm[1] = szErrValue;
                          UtlError( MESSAGE_FOLEXP_COMPLETED_MUL, MB_OK, 2,
                                    pszErrParm, EQF_INFO );
                        } /* endif */

                        // end the process
                        EqfRemoveObject( TWBFORCE, hwnd);
                      } /* endif */
                    }
                    else
                    {
                      // issue completed message
                      if ( pIda->pDDEFolExp == NULL )
                      {
                        szErrValue[0] = pIda->chDrive;
                        szErrValue[1] = EOS;
                        pszErrParm[0] = pIda->szFolLongName;
                        pszErrParm[1] = szErrValue;
                        UtlError( MESSAGE_FOLEXP_COMPLETED, MB_OK, 2,
                                  pszErrParm, EQF_INFO );
                      } /* endif */

                      //--- end the window ---
                      EqfRemoveObject( TWBFORCE, hwnd);
                    }
                    break;
                  default:
                    szErrValue[0] = pIda->chDrive;
                    szErrValue[1] = EOS;
                    pszErrParm[0] = pIda->szFolLongName;
                    pszErrParm[1] = szErrValue;
                    if ( pIda->pDDEFolExp != NULL )
                    {
                      pIda->pDDEFolExp->DDEReturn.usRc =
                      UtlGetDDEErrorCode( pIda->pDDEFolExp->hwndErrMsg );
                      UtlErrorHwnd( MESSAGE_FOLEXP_TERMINATED, MB_CANCEL, 2,
                                    pszErrParm,
                                    EQF_ERROR, pIda->pDDEFolExp->hwndErrMsg );
                    }
                    else
                    {
                      UtlError( MESSAGE_FOLEXP_TERMINATED, MB_CANCEL, 2,
                                pszErrParm,
                                EQF_ERROR );
                    } /* endif */

                    if ( pIda->pPackCtrl )
                    {
                      UtlPackCleanup( pIda->pPackCtrl );
                      pIda->pPackCtrl = NULL;
                    } /* endif */

                    // call general folder export cleanup
                    FolExpCleanup( pIda );

                    //--- get rid off exported file ---
                    UtlMakeEQFPath( pIda->szObjPath,
                                    pIda->chDrive,
                                    EXPORT_PATH, NULL );
                    strcat( pIda->szObjPath, BACKSLASH_STR );
                    strcat( pIda->szObjPath, pIda->szName );
                    strcat( pIda->szObjPath, FOLDEREXPORT_EXT );
                    UtlDelete( pIda->szObjPath, 0L, FALSE );

                    //--- end the window ---
                    EqfRemoveObject( TWBFORCE, hwnd);
                    break;
                } /* endswitch */
                break;

              case FOL_XLIFF_FORMAT : 
                {
                  int iRC = -1;
                  typedef int (* __cdecl FOLXLIFFEXPORT_DCL)( PFOLIMPEXPXLIFF ) ;

                  HMODULE hXLIFF = LoadLibrary("EQFXLIFF.DLL");
                  if (hXLIFF != NULL) {
                    FOLXLIFFEXPORT_DCL pfnFOLXLIFFEXPORT = (FOLXLIFFEXPORT_DCL)GetProcAddress(hXLIFF, "FOLXLIFFEXPORT");
                    if ( pfnFOLXLIFFEXPORT != NULL )
                       iRC = (*pfnFOLXLIFFEXPORT)( pIda->pXLIFFData );
                    if (hXLIFF)
                        FreeLibrary(hXLIFF);
                  }

                  if ( pCommArea->usComplete != pIda->pXLIFFData->usComplete )
                  {
                    pCommArea->usComplete = pIda->pXLIFFData->usComplete;
                    WinSendMsg( hwnd, WM_EQF_UPDATESLIDER, MP1FROMSHORT(pCommArea->usComplete), NULL );
                  } /* endif */

                  switch ( iRC )
                  {
                    case 0:
                      UtlDispatch();
                      WinPostMsg( hwnd, WM_EQF_PROCESSTASK, MP1FROMSHORT(FOL_EXPORT_TASK), 0l );
                      break;

                    case FOL_EXPORT_IMPORT_COMPLETE:
                      //--- ensure slider shows 100% complete ---
                      pCommArea->usComplete = 100;
                      WinSendMsg( hwnd, WM_EQF_UPDATESLIDER, MP1FROMSHORT(pCommArea->usComplete), NULL );

                      // issue completed message
                      szErrValue[0] = pIda->chDrive;
                      szErrValue[1] = EOS;
                      pszErrParm[0] = pIda->szFolLongName;
                      pszErrParm[1] = szErrValue;
                      UtlError( MESSAGE_FOLEXP_COMPLETED, MB_OK, 2, pszErrParm, EQF_INFO );
                      EqfRemoveObject( TWBFORCE, hwnd);
                      break;
                    default:
                      szErrValue[0] = pIda->chDrive;
                      szErrValue[1] = EOS;
                      pszErrParm[0] = pIda->szFolLongName;
                      pszErrParm[1] = szErrValue;
                      UtlError( MESSAGE_FOLEXP_TERMINATED, MB_CANCEL, 2, pszErrParm, EQF_ERROR );

                      // call general folder export cleanup
                      FolExpCleanup( pIda );

                      //--- get rid off exported file ---
                      UtlMakeEQFPath( pIda->szObjPath, pIda->chDrive, EXPORT_PATH, NULL );
                      strcat( pIda->szObjPath, BACKSLASH_STR );
                      strcat( pIda->szObjPath, pIda->szName );
                      strcat( pIda->szObjPath, FOLDEREXPORT_EXT );
                      UtlDelete( pIda->szObjPath, 0L, FALSE );

                      //--- end the window ---
                      EqfRemoveObject( TWBFORCE, hwnd);
                    break;
                  } /* endswitch */
                } 
                break;


              default:
                  break;
            } /*endswitch */
          } /* endif */
          break;
      } /* endswitch */
      break;
  } /* endswitch */
  return( mResult );
}


MRESULT FolExpClose
(
HWND hwndDlg,
WPARAM mp1,
LPARAM mp2
)
{
  MRESULT mResult = FALSE;
  PFOLEXPIDA pIda;                    // ptr to folder export IDA
  EQFINFO     ErrorInfo;              // error code of property handler calls

  mp2 = mp2;                          // supress compiler warning

  // get IDA pointer
  pIda = ACCESSDLGIDA( hwndDlg, PFOLEXPIDA );

  if ( pIda )
  {
    //--- close folderlist properties ---
    CloseProperties( pIda->hFllProp, PROP_QUIT, &ErrorInfo);

  } /* endif */

  DelCtrlFont (hwndDlg, ID_FOLEXP_DESCR_EF );

  // destroy our listboxes
  if ( pIda->hwndDocLB != NULLHANDLE ) WinDestroyWindow( pIda->hwndDocLB );
  if ( pIda->hwndLongNameLB != NULLHANDLE ) WinDestroyWindow( pIda->hwndLongNameLB );
  if ( pIda->pSubFolInfo ) UtlAlloc( (PVOID *)&pIda->pSubFolInfo, 0L, 0L, NOMSG );

  //--- get rid off folder export dialog ---
  WinDismissDlg( hwndDlg, SHORT1FROMMP1(mp1) );

  return( mResult );
} /* end of FolExpClose */

INT_PTR CALLBACK FolExpNoteDlgProc
(
HWND hwndDlg,                       /* handle of dialog window             */
WINMSG msg,
WPARAM mp1,
LPARAM mp2
)
{
  MRESULT     mResult = MRFROMSHORT( FALSE );      // result value of procedure
  PFOLEXPIDA  pIda;                    // ptr to export IDA
  ULONG       ulTextLength;            // length of note text
  ULONG       ulNewLength;             // new overall length of header area

  switch ( msg )
  {
    case WM_INITDLG:
      pIda = (PFOLEXPIDA) mp2;
      ANCHORDLGIDA( hwndDlg, pIda );
      OEMSETTEXT( hwndDlg, ID_FOLNOTE_FOLDER_TEXT, pIda->szFolLongName );

      SETWINDOWID( hwndDlg, ID_FOLNOTE_DLG );

      SetCtrlFnt(hwndDlg, GetCharSet(),ID_FOLNOTE_NOTE_MLE, 0 );

      if ( pIda->pHeader->Head.Rel1.usNoteSize != 0 )
      {
        SETTEXT( hwndDlg, ID_FOLNOTE_NOTE_MLE,
                 pIda->pHeader->Head.Rel1.szNoteBuffer );
      } /* endif */
      break;

    case WM_COMMAND:
      pIda = ACCESSDLGIDA( hwndDlg, PFOLEXPIDA );
      switch ( WMCOMMANDID( mp1, mp2 ) )
      {
        case ID_FOLNOTE_HELP_PB:
          UtlInvokeHelp();
          break;

        case ID_FOLNOTE_NOTE_PB:
          MLEQUERYTEXTLENGTH( hwndDlg, ID_FOLNOTE_NOTE_MLE, ulTextLength );
          ulNewLength = pIda->ulHeaderSize -
                        pIda->pHeader->Head.Rel1.usNoteSize   +
                        ulTextLength + 10;
          UtlAlloc( (PVOID *)&pIda->pHeader,
                    (LONG) pIda->ulHeaderSize,
                    (LONG) ulNewLength, ERROR_STORAGE );
          pIda->pHeader->Head.Rel1.usNoteSize   = (USHORT)ulTextLength + 10;
          pIda->ulHeaderSize = ulNewLength;
          {
            LONG lTextLength = (LONG)ulTextLength;

            MLEEXPORT( hwndDlg, ID_FOLNOTE_NOTE_MLE,
                       pIda->pHeader->Head.Rel1.szNoteBuffer,
                       lTextLength );

            ulTextLength = (USHORT)lTextLength;
          }
          ANSITOOEM( pIda->pHeader->Head.Rel1.szNoteBuffer );
          POSTCLOSE( hwndDlg, TRUE );
          break;
        case ID_FOLNOTE_CANCEL_PB:
        case DID_CANCEL:
          POSTCLOSE( hwndDlg, FALSE );
          break;
        case ID_FOLNOTE_NOTE_MLE:
          if ( WMCOMMANDCMD( mp1, mp2 ) == EN_KILLFOCUS )
          {
            ClearIME( hwndDlg );
          } /* endif */
          break;
        default:
          break;
      } /* endswitch */
      break;
    case WM_CLOSE:
      DelCtrlFont(hwndDlg, ID_FOLNOTE_NOTE_MLE );
      DISMISSDLG( hwndDlg, SHORT1FROMMP1(mp1) );
      break;
    default:
      mResult = WinDefDlgProc( hwndDlg, msg, mp1, mp2 );
      break;
  } /* endswitch */

  return mResult;
} /* end of FolExpNoteDlgProc */


////////////////////////////////////////////////////////////////////////////////
//  Utility functions used in above dialogs                                   //
////////////////////////////////////////////////////////////////////////////////

// FolAddToFileList - add a file entry to a filelist
PFILELISTENTRY FolAddToFileList
(
PFILELIST pFileList,                // ptr to filelist structure
PSZ       pszName                   // name of new entry
)
{
  BOOL     fOK = TRUE;                // OK flag; is returned to caller
  ULONG    ulNameLength = 0;          // length of file name in bytes
  USHORT   usNoOfEntries;             // no of file list entries to process
  PSZ      pOldBuffer;                // adress of old file name buffer
  PFILELISTENTRY pFileEntry = NULL;   // ptr to an entry in a file list
  LONG     lOldSize;                  // old table or buffer size
  LONG     lNewSize;                  // new table or buffer size

  //--- enlarge file list table if required
  if ( pFileList->ulListUsed >= pFileList->ulListSize )
  {
    lOldSize =  (LONG) sizeof(FILELISTENTRY) * pFileList->ulListSize;
    lNewSize =  (LONG) sizeof(FILELISTENTRY) *
                (LONG) (pFileList->ulListSize + FILEENTRIES_PER_ALLOC);
    fOK = UtlAlloc( (PVOID *)&pFileList->pEntries, lOldSize, lNewSize, ERROR_STORAGE );
    if ( fOK )
    {
      pFileList->ulListSize += FILEENTRIES_PER_ALLOC;
    } /* endif */
  } /* endif */

  //--- enlarge file name buffer if required ---
  if ( fOK )
  {
    ulNameLength = strlen( pszName ) + 1;
    if ( pFileList->ulBufferUsed + ulNameLength >= pFileList->ulBufferSize )
    {
      pOldBuffer = pFileList->pBuffer;        // remember old buffer location
      fOK = UtlAlloc( (PVOID *)&pFileList->pBuffer,
                      (LONG) pFileList->ulBufferSize,
                      (LONG) (pFileList->ulBufferSize +
                              FILENAMEBUFSIZE_PER_ALLOC),
                      ERROR_STORAGE );
      if ( fOK )
      {
        //--- set new buffer size ---
        pFileList->ulBufferSize += FILENAMEBUFSIZE_PER_ALLOC;

        //--- correct file name pointers in filelist table ---
        pFileEntry = pFileList->pEntries;
        usNoOfEntries = (USHORT)pFileList->ulListUsed;
        while ( usNoOfEntries )
        {
          pFileEntry->pszName = pFileList->pBuffer +
                                (pFileEntry->pszName - pOldBuffer);
          pFileEntry++;
          usNoOfEntries--;
        } /* endwhile */
      } /* endif */
    } /* endif */
  } /* endif */

  //--- add new entry to file list ---
  if ( fOK )
  {
    pFileEntry = pFileList->pEntries + pFileList->ulListUsed;
    pFileEntry->pszName = pFileList->pBuffer + pFileList->ulBufferUsed;
    memcpy( pFileList->pBuffer + pFileList->ulBufferUsed,
            pszName, ulNameLength );
    pFileList->ulListUsed++;
    pFileList->ulBufferUsed += ulNameLength;
  } /* endif */

  return( (fOK) ? pFileEntry : NULL );
} /* end of FolAddToFileList */

//+----------------------------------------------------------------------------+
//  Set the enable state of the controls in the export dialog panel
//    but leave the HELP and CANCEL button untouched
//+----------------------------------------------------------------------------+
USHORT FolExpSetEnableState
(
HWND hwndDlg,                       // handle of dialog window
BOOL fEnable                        // TRUE = enable, FALSE = disable
)
{
  USHORT usI;                         // loop index for drive processing
  HWND   hwndDrive;                   // handle of drive buttons

  ENABLECTRL( hwndDlg, ID_FOLEXP_EXPORT_PB, fEnable );
  if ( fEnable )
  {
    CLEARTABSTOP( hwndDlg, ID_FOLEXP_CANCEL_PB );
  }
  else
  {
    SETTABSTOP( hwndDlg, ID_FOLEXP_CANCEL_PB );
  } /* endif */
  ENABLECTRL( hwndDlg, ID_FOLEXP_NOTE_CHK, fEnable );
// ENABLECTRL( hwndDlg, ID_FOLEXP_DELETE_CHK, fEnable );
  ENABLECTRL( hwndDlg, ID_FOLEXP_WITHMEM_CHK, fEnable );
  ENABLECTRL( hwndDlg, ID_FOLEXP_WITHDICT_CHK, fEnable );
  for ( usI = PID_DRIVEBUTTON_A; usI <= PID_DRIVEBUTTON_Z; usI++ )
  {
    hwndDrive = WinWindowFromID( hwndDlg, usI );
    if ( hwndDrive )
    {
      ENABLECTRLHWND( hwndDrive, fEnable );
    } /* endif */
  } /* endfor */
  return( TRUE );
}




//+----------------------------------------------------------------------------+
//Internal function FolExpAddLBDocs
//+----------------------------------------------------------------------------+
//Function name:
//+----------------------------------------------------------------------------+
//Function call:
//+----------------------------------------------------------------------------+
//Description:
//+----------------------------------------------------------------------------+
//Parameters:
//
//+----------------------------------------------------------------------------+
//Returncode type:
//+----------------------------------------------------------------------------+
//Returncodes:
//+----------------------------------------------------------------------------+
//Function flow:
//
//
//
//
//+----------------------------------------------------------------------------+



BOOL FolExpAddLBDocs
(
HWND        hwndLB,                 // LB w/ selected long names of documents
HWND        hwndDocLB,              // LB w/ short names of all documents
PFOLEXPIDA  pIda,                   // folder export dialog IDA
BOOL        fWithMem,               // include-memory-files flag
BOOL        fSelectedDocs,          // include-only-selected-docs flag
HWND        hwnd                    // owner handle for error messages
)
{
  SHORT   sItem;                       // listbox item index
  CHAR    szDocName[MAX_FILESPEC+1];   // buffer for document name
  CHAR    szTempName[MAX_EQF_PATH];    // buffer for file names
  CHAR    szFormat[MAX_FNAME];         // buffer for format name
  BOOL    fOK = TRUE;                  // OK flag (is returned to caller)
  ULONG   ulRC;                        // package utilities return code
  USHORT  usNoOfDocs = 0;              // number of selected documents
  SHORT   sItemCount = 0;              // number of items in listbox
  SHORT   sDocNameIndex;               // index of document in short document LB
  ULONG   ulLongNameLen = 0L;          // length of long name array
  EQFINFO     ErrorInfo;              // error code of property handler calls
  PPROPFOLDER pFolProp;               // ptr to folder properties
  PVOID       hFolProp;               // handle of folder properties
  HPROP  hpropDocument;
  PPROPDOCUMENT ppropDocument;
  PSZ    pszReplace;
  ULONG   ulErrorInfo = 0;                  // error number
  CHAR   szFolderVendor[MAX_DESCRIPTION];
  CHAR   szDocumentVendor[MAX_DESCRIPTION];

  if ( fSelectedDocs )
  {
    sItem = LIT_FIRST;                 // start with first selected item
  }
  else
  {
    sItem = 0;                         // start with first item index
    sItemCount = QUERYITEMCOUNTHWND( hwndLB );
  } /* endif */

  // open folder properties, get Vendor name

  {
    PPROPSYSTEM   pSysProp;

    strcpy( pIda->szObjPath, pIda->szObjName );
    pSysProp = (PPROPSYSTEM)MakePropPtrFromHnd( EqfQuerySystemPropHnd());
    pIda->szObjPath[0] = pSysProp->szPrimaryDrive[0];
  }

  hFolProp = OpenProperties( pIda->szObjPath,
                             NULL,
                             PROP_ACCESS_READ,
                             &ErrorInfo);

  if ( hFolProp )
  {
    pFolProp = (PPROPFOLDER)MakePropPtrFromHnd( hFolProp );

    strcpy(szFolderVendor,pFolProp->szVendor);

    CloseProperties( hFolProp, PROP_QUIT, &ErrorInfo );
  }
  else
  {
    pszReplace = pIda->szObjPath;
    UtlErrorHwnd( ERROR_OPEN_PROPERTIES, MB_CANCEL,
                  1, &pszReplace, EQF_ERROR, hwnd);
  }//end if

  do
  {
    ULONG     ulParentID = 0;           // ID of document parent subfolder

    if ( fSelectedDocs )
    {
      sItem = QUERYNEXTSELECTIONHWND( hwndLB, sItem );
    }
    else if ( sItem >= sItemCount )   // at end of listbox items ???
    {
      sItem = LIT_NONE;               // no more items to follow
    } /* endif */


    // if translator selected compare translator name
    if ( sItem != LIT_NONE )
    {
      // add length of document long name to document long name array length
      QUERYITEMTEXTHWND( hwndLB, sItem, pIda->szLongName );
      ulLongNameLen += strlen( pIda->szLongName ) + 1;

      // get index of document in short document name listbox
      sDocNameIndex = (SHORT)QUERYITEMHANDLEHWND( hwndLB, sItem );

      // get document name
      QUERYITEMTEXTHWND( hwndDocLB, sDocNameIndex, szDocName );
      //
      // open document properties, get Vendor name
      //

      UtlMakeEQFPath( pIda->szDocObjName,
                      pIda->szObjName[0], SYSTEM_PATH,
                      pIda->szFolName );
      strcat( pIda->szDocObjName, BACKSLASH_STR );
      strcat( pIda->szDocObjName, szDocName );

      if ((hpropDocument =
            OpenProperties (pIda->szDocObjName, NULL,
                            PROP_ACCESS_READ, &ulErrorInfo))== NULL)
      {
        // display error message
        pszReplace = pIda->szDocObjName;
        UtlErrorHwnd( ERROR_OPEN_PROPERTIES, MB_CANCEL,
                  1, &pszReplace, EQF_ERROR, hwnd);
      }
      else
      {

        ppropDocument = (PPROPDOCUMENT)MakePropPtrFromHnd( hpropDocument );
        strcpy(szDocumentVendor,ppropDocument->szVendor);
        strcpy( pIda->szLongName, ppropDocument->szLongName );
        ulParentID = ppropDocument->ulParentFolder;

        CloseProperties(hpropDocument , PROP_QUIT, &ErrorInfo );

      }//end if

      // Check if correct vendor is selected

      if (!pIda->fTranslatorSelected ||
//          pIda->szVendor[0]!=EOS ||
          !strcmp(pIda->szVendor,szDocumentVendor) ||
          (szDocumentVendor[0]==EOS && !strcmp(pIda->szVendor,szFolderVendor))  )
      {

        usNoOfDocs++;                  // increment document number


        // mark all parent subfolders for export
        if ( ulParentID != 0 )
        {
          FolExpSetSubFolInfoFlag( ulParentID, pIda->pSubFolInfo );
        } /* endif */

        // add property file to list of package files
        //
        UtlMakeEQFPath( szTempName,
                        pIda->szObjName[0],
                        PROPERTY_PATH,
                        pIda->szFolName );
        strcat( szTempName, BACKSLASH_STR );
        strcat( szTempName, szDocName );
        ulRC = UtlPackAddFile( pIda->pPackCtrl, szTempName,
                               DOCUMENT_PROP_FILE,
                               FILE_COMPRESS );
        fOK = UtlPackHandleError( ulRC, szTempName, hwnd );

        // add source file to list of package files
        //
        if ( fOK )
        {
          UtlMakeEQFPath( szTempName,
                          pIda->szObjName[0],
                          DIRSOURCEDOC_PATH,
                          pIda->szFolName );
          strcat( szTempName, BACKSLASH_STR );
          strcat( szTempName, szDocName );
          ulRC = UtlPackAddFile( pIda->pPackCtrl, szTempName,
                                 DOCUMENT_SRC_FILE,
                                 FILE_COMPRESS );
          fOK = UtlPackHandleError( ulRC, szTempName, hwnd );
        } /* endif */

        // add segmented source file (if any) to list of package files
        //
        if ( fOK )
        {
          UtlMakeEQFPath( szTempName,
                          pIda->szObjName[0],
                          DIRSEGSOURCEDOC_PATH,
                          pIda->szFolName );
          strcat( szTempName, BACKSLASH_STR );
          strcat( szTempName, szDocName );
          if ( UtlFileExist( szTempName ) )
          {
            ulRC = UtlPackAddFile( pIda->pPackCtrl, szTempName,
                                   DOCUMENT_SEGSRC_FILE,
                                   FILE_COMPRESS );
            fOK = UtlPackHandleError( ulRC, szTempName, hwnd );
          } /* endif */
        } /* endif */

        // add EA data file (if any) to list of package files
        //
        if ( fOK )
        {
          UtlMakeEQFPath( szTempName,
                          pIda->szObjName[0],
                          EADATA_PATH,
                          pIda->szFolName );
          strcat( szTempName, BACKSLASH_STR );
          strcat( szTempName, szDocName );
          if ( UtlFileExist( szTempName ) )
          {
            ulRC = UtlPackAddFile( pIda->pPackCtrl, szTempName,
                                   DOCUMENT_EADATA_FILE,
                                   FILE_COMPRESS );
            fOK = UtlPackHandleError( ulRC, szTempName, hwnd );
          } /* endif */
        } /* endif */

        //
        // add segmented target file (if any) to list of package files
        //
        if ( fOK )
        {
          UtlMakeEQFPath( szTempName,
                          pIda->szObjName[0],
                          DIRSEGTARGETDOC_PATH,
                          pIda->szFolName );
          strcat( szTempName, BACKSLASH_STR );
          strcat( szTempName, szDocName );
          if ( UtlFileExist( szTempName ) )
          {
            ulRC = UtlPackAddFile( pIda->pPackCtrl, szTempName,
                                   DOCUMENT_SEGTGT_FILE,
                                   FILE_COMPRESS );
            fOK = UtlPackHandleError( ulRC, szTempName, hwnd );
          } /* endif */
        } /* endif */

        // add RTF source list of package files
        //
        if ( fOK )
        {
          UtlMakeEQFPath( szTempName,
                          pIda->szObjName[0],
                          DIRSEGRTF_PATH,
                          pIda->szFolName );
          strcat( szTempName, BACKSLASH_STR );
          strcat( szTempName, szDocName );
          if ( UtlFileExist( szTempName ) )
          {
            ulRC = UtlPackAddFile( pIda->pPackCtrl, szTempName,
                                   DOCUMENT_SRC_FILE,
                                   FILE_COMPRESS );
            fOK = UtlPackHandleError( ulRC, szTempName, hwnd );
          } /* endif */
        } /* endif */

        // add MT Log file (if any) to list of package files
        //
        if ( fOK )
        {
          UtlMakeEQFPath( szTempName,
                          pIda->szObjName[0],
                          MTLOG_PATH,
                          pIda->szFolName );
          strcat( szTempName, BACKSLASH_STR );
          strcat( szTempName, szDocName );
          if ( UtlFileExist( szTempName ) )
          {
            ulRC = UtlPackAddFile( pIda->pPackCtrl, szTempName,
                                   DOCUMENT_MTLOG_FILE,
                                   FILE_COMPRESS );
            fOK = UtlPackHandleError( ulRC, szTempName, hwnd );
          } /* endif */
        } /* endif */

        // add MISC file (if any) to list of package files
        //
        if ( fOK )
        {
          UtlMakeEQFPath( szTempName, pIda->szObjName[0], MISC_PATH, pIda->szFolName );
          strcat( szTempName, BACKSLASH_STR );
          strcat( szTempName, szDocName );
          if ( UtlFileExist( szTempName ) )
          {
            ulRC = UtlPackAddFile( pIda->pPackCtrl, szTempName,
                                   DOCUMENT_MISC_FILE,
                                   FILE_COMPRESS );
            fOK = UtlPackHandleError( ulRC, szTempName, hwnd );
          } /* endif */
        } /* endif */

        // add Entity file (if any) to list of package files
        //
        if ( fOK )
        {
          UtlMakeEQFPath( szTempName, pIda->szObjName[0], ENTITY_PATH, pIda->szFolName );
          strcat( szTempName, BACKSLASH_STR );
          strcat( szTempName, szDocName );
          if ( UtlFileExist( szTempName ) )
          {
            ulRC = UtlPackAddFile( pIda->pPackCtrl, szTempName,
                                   DOCUMENT_ENTITY_FILE,
                                   FILE_COMPRESS );
            fOK = UtlPackHandleError( ulRC, szTempName, hwnd );
          } /* endif */
        } /* endif */

        // add XLIFF file (if any) to list of package files
        //
        if ( fOK )
        {
          UtlMakeEQFPath( szTempName, pIda->szObjName[0], XLIFF_PATH, pIda->szFolName );
          strcat( szTempName, BACKSLASH_STR );
          strcat( szTempName, szDocName );
          if ( UtlFileExist( szTempName ) )
          {
            ulRC = UtlPackAddFile( pIda->pPackCtrl, szTempName,
                                   DOCUMENT_XLIFF_FILE,
                                   FILE_COMPRESS );
            fOK = UtlPackHandleError( ulRC, szTempName, hwnd );
          } /* endif */
        } /* endif */

        // add Metadata file (if any) to list of package files
        //
        if ( fOK )
        {
          UtlMakeEQFPath( szTempName, pIda->szObjName[0], METADATA_PATH, pIda->szFolName );
          strcat( szTempName, BACKSLASH_STR );
          strcat( szTempName, szDocName );
          if ( UtlFileExist( szTempName ) )
          {
            ulRC = UtlPackAddFile( pIda->pPackCtrl, szTempName,
                                   DOCUMENT_METADATA_FILE,
                                   FILE_COMPRESS );
            fOK = UtlPackHandleError( ulRC, szTempName, hwnd );
          } /* endif */
        } /* endif */

        /************************************************************/
        /* Get format and translation memory of document            */
        /************************************************************/
        if ( fOK )
        {
          UtlMakeEQFPath( szTempName, pIda->szObjName[0], SYSTEM_PATH, NULL );
          strcat( szTempName, BACKSLASH_STR );
          strcat( szTempName, pIda->szFolName );
          strcat( szTempName, BACKSLASH_STR );
          strcat( szTempName, szDocName );
          if ( DocQueryInfoHwnd( szTempName,       // document object name
                                 pIda->szDocMem,         // document translation memory
                                 szFormat,         // format of document
                                 NULL,             // no source language
                                 NULL,             // no target language
                                 TRUE, hwnd ) != NO_ERROR )        // handle errors in function
          {
            fOK = FALSE;
          } /* endif */
        } /* endif */

        /************************************************************/
        /* Add document tag table to list of package files          */
        /************************************************************/
        if ( fOK )
        {
          fOK = FolExpAddTagTable( pIda, szFormat,
                                   pIda->hatomInError, hwnd,
                                   pIda->fOldVers );
        } /* endif */

        /************************************************************/
        /* Add document translation memory to list of package files */
        /* (only for new version export files)                      */
        /************************************************************/
        if ( fOK && fWithMem && !pIda->fOldVers && pIda->fWithDocMem )
        {
          BYTE b = 0;
          if (pIda->fNonUnicode)
          {
             b = EXP_IN_NON_UNICODE;
          }
          fOK = FolExpAddMemory( pIda, pIda->szDocMem,
                                 pIda->hatomInError, b, hwnd );
          pIda->pHeader->BitFlags.fContainsMem = TRUE;
        } /* endif */

        /************************************************************/
        /* Add export record to history log                         */
        /************************************************************/
        if ( fOK )
        {
          DOCEXPORTHIST DocExpHist;  // history record for document export

          memset( (PVOID)&DocExpHist, 0, sizeof(DocExpHist) );
          DocExpHist.sType = FOLDER_SUBTYPE;
          EQFBWriteHistLog2( pIda->szFolderObjName,
                             szDocName, DOCEXPORT_LOGTASK,
                             sizeof(DocExpHist), (PVOID)&DocExpHist,
                             TRUE, hwnd, pIda->szLongName );
        } /* endif */
      } /* endif */
    } /* endif */

    if ( !fSelectedDocs && (sItem != LIT_NONE) )
    {
      sItem++;                        // continue with next item
    } /* endif */
  } while ( fOK && (sItem != LIT_NONE) );


  // add long name array to end of header area
  if ( fOK )
  {
    // make room for long document name array
    ULONG ulNewLength = pIda->ulHeaderSize + ulLongNameLen + 10;

    fOK = UtlAlloc( (PVOID *)&pIda->pHeader,
                    (LONG) pIda->ulHeaderSize,
                    (LONG) ulNewLength, ERROR_STORAGE );
    if ( fOK )
    {
      PSZ pszCurPos;                   // ptr to current pos in document name array

      // position to begin of document array
      pszCurPos = (PSZ)pIda->pHeader + pIda->ulHeaderSize;

      // add all or all-selected document long names to long name array
      if ( fSelectedDocs )
      {
        sItem = LIT_FIRST;                 // start with first selected item
      }
      else
      {
        sItem = 0;                         // start with first item index
        sItemCount = QUERYITEMCOUNTHWND( hwndLB );
      } /* endif */

      do
      {
        if ( fSelectedDocs )
        {
          sItem = QUERYNEXTSELECTIONHWND( hwndLB, sItem );
        }
        else if ( sItem >= sItemCount )   // at end of listbox items ???
        {
          sItem = LIT_NONE;               // no more items to follow
        } /* endif */

        if ( sItem != LIT_NONE )
        {

          // get index of document in short document name listbox
          sDocNameIndex = (SHORT)QUERYITEMHANDLEHWND( hwndLB, sItem );

          // get document name
          QUERYITEMTEXTHWND( hwndDocLB, sDocNameIndex, szDocName );

          //
          // open document properties, get Vendor name
          //

          UtlMakeEQFPath( pIda->szDocObjName,
                          pIda->szObjName[0], SYSTEM_PATH,
                          pIda->szFolName );
          strcat( pIda->szDocObjName, BACKSLASH_STR );
          strcat( pIda->szDocObjName, szDocName );

          if ((hpropDocument =
                OpenProperties (pIda->szDocObjName, NULL,
                                PROP_ACCESS_READ, &ulErrorInfo))== NULL)
          {
            // display error message
            pszReplace = pIda->szDocObjName;
            UtlErrorHwnd( ERROR_OPEN_PROPERTIES, MB_CANCEL,
                      1, &pszReplace, EQF_ERROR, hwnd);
          }
          else
          {

            ppropDocument = (PPROPDOCUMENT)MakePropPtrFromHnd( hpropDocument );
            strcpy(szDocumentVendor,ppropDocument->szVendor);

            CloseProperties(hpropDocument , PROP_QUIT, &ErrorInfo );

          }//end if

          //
          // Check if correct vendor is selected
          // insert the name

          if (!pIda->fTranslatorSelected ||
//          pIda->szVendor[0]!=EOS ||
              !strcmp(pIda->szVendor,szDocumentVendor) ||
              (szDocumentVendor[0]==EOS && !strcmp(pIda->szVendor,szFolderVendor))  )
          {

            QUERYITEMTEXTHWND( hwndLB, sItem, pIda->szLongName );
            ANSITOOEM( pIda->szLongName );
            strcpy( pszCurPos, pIda->szLongName );
            pszCurPos += strlen( pIda->szLongName ) + 1;
          } /* endif */
        } /* endif */

        if ( !fSelectedDocs && (sItem != LIT_NONE) )
        {
          sItem++;                        // continue with next item
        } /* endif */
      } while ( fOK && (sItem != LIT_NONE) );

      // terminate long document name array
      *pszCurPos = EOS;

      // remember new header size
      pIda->ulHeaderSize = ulNewLength;

      // set document name array flag
      pIda->pHeader->BitFlags.fDocNameTable = TRUE;
    } /* endif */
  } /* endif */

  //
  // set 'selected documents' flag in package header
  //
  pIda->pHeader->BitFlags.fSelectedDocs = (USHORT)fSelectedDocs;

  return( fOK );
} /* end of FolExpAddSelectedDocs */



// add a single document to the files being packaged
BOOL FolExpAddDoc
(
PFOLEXPIDA  pIda,                   // folder export dialog IDA
PSZ         pszDocShortName,        // document short name
BOOL        fWithMem,               // include-memory-files flag
BOOL        fSelectedDocs,          // include-only-selected-docs flag
HWND        hwnd                    // owner handle for error messages
)
{
  CHAR    szTempName[MAX_EQF_PATH];    // buffer for file names
  CHAR    szFormat[MAX_FNAME];         // buffer for format name
  BOOL    fOK = TRUE;                  // OK flag (is returned to caller)
  ULONG   ulRC;                        // package utilities return code
  static  CHAR szLongName[MAX_LONGFILESPEC]; // document long name

  fSelectedDocs;
  // add property file to list of package files
  if ( fOK )
  {
    UtlMakeEQFPath( szTempName,
                    pIda->szObjName[0],
                    PROPERTY_PATH,
                    pIda->szFolName );
    strcat( szTempName, BACKSLASH_STR );
    strcat( szTempName, pszDocShortName );
    ulRC = UtlPackAddFile( pIda->pPackCtrl, szTempName,
                           DOCUMENT_PROP_FILE,
                           FILE_COMPRESS );
    fOK = UtlPackHandleError( ulRC, szTempName, hwnd );
  } /* endif */

  // add source file to list of package files
  if ( fOK )
  {
    UtlMakeEQFPath( szTempName,
                    pIda->szObjName[0],
                    DIRSOURCEDOC_PATH,
                    pIda->szFolName );
    strcat( szTempName, BACKSLASH_STR );
    strcat( szTempName, pszDocShortName );
    ulRC = UtlPackAddFile( pIda->pPackCtrl, szTempName,
                           DOCUMENT_SRC_FILE,
                           FILE_COMPRESS );
    fOK = UtlPackHandleError( ulRC, szTempName, hwnd );
  } /* endif */

  // add segmented source file (if any) to list of package files
  if ( fOK )
  {
    UtlMakeEQFPath( szTempName,
                    pIda->szObjName[0],
                    DIRSEGSOURCEDOC_PATH,
                    pIda->szFolName );
    strcat( szTempName, BACKSLASH_STR );
    strcat( szTempName, pszDocShortName );
    if ( UtlFileExist( szTempName ) )
    {
      ulRC = UtlPackAddFile( pIda->pPackCtrl, szTempName,
                             DOCUMENT_SEGSRC_FILE,
                             FILE_COMPRESS );
      fOK = UtlPackHandleError( ulRC, szTempName, hwnd );
    } /* endif */
  } /* endif */


  //
  // add EA data file (if any) to list of package files
  //
  if ( fOK )
  {
    UtlMakeEQFPath( szTempName,
                    pIda->szObjName[0],
                    EADATA_PATH,
                    pIda->szFolName );
    strcat( szTempName, BACKSLASH_STR );
    strcat( szTempName, pszDocShortName );
    if ( UtlFileExist( szTempName ) )
    {
      ulRC = UtlPackAddFile( pIda->pPackCtrl, szTempName,
                             DOCUMENT_EADATA_FILE,
                             FILE_COMPRESS );
      fOK = UtlPackHandleError( ulRC, szTempName, hwnd );
    } /* endif */
  } /* endif */

  //
  // add segmented target file (if any) to list of package files
  //
  if ( fOK )
  {
    UtlMakeEQFPath( szTempName,
                    pIda->szObjName[0],
                    DIRSEGTARGETDOC_PATH,
                    pIda->szFolName );
    strcat( szTempName, BACKSLASH_STR );
    strcat( szTempName, pszDocShortName );
    if ( UtlFileExist( szTempName ) )
    {
      ulRC = UtlPackAddFile( pIda->pPackCtrl, szTempName,
                             DOCUMENT_SEGTGT_FILE,
                             FILE_COMPRESS );
      fOK = UtlPackHandleError( ulRC, szTempName, hwnd );
    } /* endif */
  } /* endif */

  // DIRSEGRTF_PATH
  //
  // add RTF source file (if any) to list of package files
  // used for the eqfmswrd filter
  //
  if ( fOK )
  {
    UtlMakeEQFPath( szTempName,
                    pIda->szObjName[0],
                    DIRSEGRTF_PATH,
                    pIda->szFolName );
    strcat( szTempName, BACKSLASH_STR );
    strcat( szTempName, pszDocShortName );
    if ( UtlFileExist( szTempName ) )
    {
      ulRC = UtlPackAddFile( pIda->pPackCtrl, szTempName,
                             DOCUMENT_SRC_FILE,
                             FILE_COMPRESS );
      fOK = UtlPackHandleError( ulRC, szTempName, hwnd );
    } /* endif */
  } /* endif */

  //
  // add MT log file (if any) to list of package files
  //
  if ( fOK )
  {
    UtlMakeEQFPath( szTempName,
                    pIda->szObjName[0],
                    MTLOG_PATH,
                    pIda->szFolName );
    strcat( szTempName, BACKSLASH_STR );
    strcat( szTempName, pszDocShortName );
    if ( UtlFileExist( szTempName ) )
    {
      ulRC = UtlPackAddFile( pIda->pPackCtrl, szTempName,
                             DOCUMENT_MTLOG_FILE,
                             FILE_COMPRESS );
      fOK = UtlPackHandleError( ulRC, szTempName, hwnd );
    } /* endif */
  } /* endif */

  //
  // add MISC file (if any) to list of package files
  //
  if ( fOK )
  {
    UtlMakeEQFPath( szTempName,
                    pIda->szObjName[0],
                    MISC_PATH,
                    pIda->szFolName );
    strcat( szTempName, BACKSLASH_STR );
    strcat( szTempName, pszDocShortName );
    if ( UtlFileExist( szTempName ) )
    {
      ulRC = UtlPackAddFile( pIda->pPackCtrl, szTempName,
                             DOCUMENT_MISC_FILE,
                             FILE_COMPRESS );
      fOK = UtlPackHandleError( ulRC, szTempName, hwnd );
    } /* endif */
  } /* endif */

  //
  // add METADATA file (if any) to list of package files
  //
  if ( fOK )
  {
    UtlMakeEQFPath( szTempName,
                    pIda->szObjName[0],
                    METADATA_PATH,
                    pIda->szFolName );
    strcat( szTempName, BACKSLASH_STR );
    strcat( szTempName, pszDocShortName );
    if ( UtlFileExist( szTempName ) )
    {
      ulRC = UtlPackAddFile( pIda->pPackCtrl, szTempName,
                             DOCUMENT_METADATA_FILE,
                             FILE_COMPRESS );
      fOK = UtlPackHandleError( ulRC, szTempName, hwnd );
    } /* endif */
  } /* endif */

  //
  // add XLIFF file (if any) to list of package files
  //
  if ( fOK )
  {
    UtlMakeEQFPath( szTempName,
                    pIda->szObjName[0],
                    XLIFF_PATH,
                    pIda->szFolName );
    strcat( szTempName, BACKSLASH_STR );
    strcat( szTempName, pszDocShortName );
    if ( UtlFileExist( szTempName ) )
    {
      ulRC = UtlPackAddFile( pIda->pPackCtrl, szTempName,
                             DOCUMENT_XLIFF_FILE,
                             FILE_COMPRESS );
      fOK = UtlPackHandleError( ulRC, szTempName, hwnd );
    } /* endif */
  } /* endif */

  //
  // add Entity file (if any) to list of package files
  //
  if ( fOK )
  {
    UtlMakeEQFPath( szTempName,
                    pIda->szObjName[0],
                    ENTITY_PATH,
                    pIda->szFolName );
    strcat( szTempName, BACKSLASH_STR );
    strcat( szTempName, pszDocShortName );
    if ( UtlFileExist( szTempName ) )
    {
      ulRC = UtlPackAddFile( pIda->pPackCtrl, szTempName,
                             DOCUMENT_ENTITY_FILE,
                             FILE_COMPRESS );
      fOK = UtlPackHandleError( ulRC, szTempName, hwnd );
    } /* endif */
  } /* endif */


  /************************************************************/
  /* Get format and translation memory of document            */
  /************************************************************/
  if ( fOK )
  {
    UtlMakeEQFPath( szTempName, pIda->szObjName[0], SYSTEM_PATH, NULL );
    strcat( szTempName, BACKSLASH_STR );
    strcat( szTempName, pIda->szFolName );
    strcat( szTempName, BACKSLASH_STR );
    strcat( szTempName, pszDocShortName );
    if ( DocQueryInfo2Hwnd( szTempName,       // document object name
                            pIda->szDocMem,   // document translation memory
                            szFormat,         // format of document
                            NULL,             // no source language
                            NULL,             // no target language
                            szLongName,       // document long name
                            NULL, NULL,       // no alias, no editor
                            TRUE, hwnd ) != NO_ERROR )        // handle errors in function
    {
      fOK = FALSE;
    } /* endif */
  } /* endif */


  /************************************************************/
  /* Add document tag table to list of package files          */
  /************************************************************/
  if ( fOK )
  {
    fOK = FolExpAddTagTable( pIda, szFormat,
                             pIda->hatomInError, hwnd,
                             pIda->fOldVers );
  } /* endif */

  /************************************************************/
  /* Add document translation memory to list of package files */
  /* (only for new version export files)                      */
  /************************************************************/
  if ( fOK && fWithMem && !pIda->fOldVers && pIda->fWithDocMem )
  {
    BYTE b = 0;
    if (pIda->fNonUnicode)
    {
      b = EXP_IN_NON_UNICODE;
    }
    fOK = FolExpAddMemory( pIda, pIda->szDocMem,
                           pIda->hatomInError, b, hwnd );
    pIda->pHeader->BitFlags.fContainsMem = TRUE;
  } /* endif */

  /************************************************************/
  /* Add export record to history log                         */
  /************************************************************/
  if ( fOK )
  {
    DOCEXPORTHIST DocExpHist;  // history record for document export

    memset( (PVOID)&DocExpHist, 0, sizeof(DocExpHist) );
    DocExpHist.sType = FOLDER_SUBTYPE;
    EQFBWriteHistLog2( pIda->szFolderObjName,
                       pszDocShortName, DOCEXPORT_LOGTASK,
                       sizeof(DocExpHist), (PVOID)&DocExpHist,
                       TRUE, hwnd, szLongName );
  } /* endif */
  return( fOK );
} /* end of function FolExpAddDoc */

// add a single document to the document list of the XLIFF import/export area
BOOL FolExpAddDocToList
(
  PFOLEXPIDA  pIda,                   // folder export dialog IDA
  PFOLIMPEXPXLIFF pXLIFFData,         // ptr to xliff data area
  PSZ         pszDocShortName         // document short name
)
{
  BOOL    fOK = TRUE;                  // OK flag (is returned to caller)

  // enlarge document list if necessary
  int iDocNameLen = strlen( pszDocShortName ) + 1;
  if ( (pXLIFFData->iDocListUsed + iDocNameLen) >= pXLIFFData->iDocListSize )
  {
    fOK = UtlAllocHwnd( (PVOID *)&(pXLIFFData->pszDocuments), pXLIFFData->iDocListSize, pXLIFFData->iDocListSize + 1024,
                        ERROR_STORAGE, pIda->hwndErrMsg );
    pXLIFFData->iDocListSize += 1024;
  } /* endif */

  // add property file to list of package files
  if ( fOK )
  {
    strcpy( pXLIFFData->pszDocuments + pXLIFFData->iDocListUsed, pszDocShortName );
    pXLIFFData->iDocListUsed += iDocNameLen;
    pXLIFFData->iDocuments++;
  } /* endif */
  return( fOK );
} /* end of function FolExpAddDocToList */




//+----------------------------------------------------------------------------+
//Internal function
//+----------------------------------------------------------------------------+
//Function name:     FolExpAddTagTable
//+----------------------------------------------------------------------------+
//Description:       Adds the given tag table and any segmentation user exit
//                   of this tag table to the list of package files.
//+----------------------------------------------------------------------------+
//Input parameter:   PVOID    pPackCtrl  ptr to package control structure
//                   PSZ      pszFormat  name of tag table / format table
//+----------------------------------------------------------------------------+
//Returncode type:   BOOL
//+----------------------------------------------------------------------------+
BOOL FolExpAddTagTable
(
PFOLEXPIDA       pIda,               // ptr to folder export isntance data area
PSZ              pszFormat,          // name of tag table / format table
HATOMTBL         hAtomTbl,           // atom table for in-error objects
HWND             hwnd,               // owner handle for error messages
BOOL             fOldVers            // TRUE = export in old format
)
{
  BOOL             fOK = TRUE;         // internal OK flag
  BOOL             fExists = FALSE;    // TRUE = Tag table exists
  ULONG            ulRC;               // return values from UtlPack calls
  PSZ              pszErrParm[2];      // ptr to error parameters
  
  /********************************************************************/
  /* Build fully qualified tag table info file name                   */
  /********************************************************************/
  UtlMakeEQFPath( pIda->szMUTBuffer, NULC, TABLE_PATH, NULL );
  strcat( pIda->szMUTBuffer, BACKSLASH_STR );
  strcat( pIda->szMUTBuffer, pIda->szFolName );
  strcat( pIda->szMUTBuffer, "-" );
  strcat( pIda->szMUTBuffer, pszFormat );
  strcat( pIda->szMUTBuffer, EXT_OF_TAGTABLEINFO );

  /********************************************************************/
  /* Add tag table only if it is not in package file list already     */
  /********************************************************************/
  if (  !UtlPackIsInList( pIda->pPackCtrl, pIda->szMUTBuffer ) )
  {
    OtmMarkup *pMarkup = NULL;
    OtmMarkupPlugin *pPlugin = NULL;

    // get markup table object
    if ( fOK )
    {
      pPlugin = GetMarkupPlugin( pszFormat );
      if ( pPlugin == NULL )
      {
        if ( WinFindAtom( hAtomTbl, pIda->szMUTBuffer ) == NULLHANDLE )
        {
          pszErrParm[0] = pszFormat;
          USHORT usMBRC = UtlErrorHwnd( ERROR_FOLEXP_ACC_TAGTABLE, MB_YESNO, 1, pszErrParm, EQF_ERROR, hwnd );
          fOK = (usMBRC != MBID_NO );
          WinAddAtom( hAtomTbl, pIda->szMUTBuffer );
        } /* endif */
        return( fOK ); // ignore this markup table
      }
      else
      {
        pMarkup = GetMarkupObject( pszFormat, (PSZ)pPlugin->getName() );
        if ( pMarkup == NULL )
        {
          if ( WinFindAtom( hAtomTbl, pIda->szMUTBuffer ) == NULLHANDLE )
          {
            pszErrParm[0] = pszFormat;
            USHORT usMBRC = UtlErrorHwnd( ERROR_FOLEXP_ACC_TAGTABLE, MB_YESNO, 1, pszErrParm, EQF_ERROR, hwnd );
            fOK = (usMBRC != MBID_NO );
            WinAddAtom( hAtomTbl, pIda->szMUTBuffer );
          } /* endif */
          return( fOK ); // ignore this markup table
        } /* endif */
      } /* endif */
    } /* endif */ 

    // open markup table info file for output
    FILE *hfInfoFile = fopen( pIda->szMUTBuffer, "w" );
    if ( hfInfoFile == NULL )
    {
      pszErrParm[0] = pIda->szMUTBuffer;
      USHORT usMBRC = UtlErrorHwnd( ERROR_FILE_OPEN_FAILED, MB_OK, 1, pszErrParm, EQF_ERROR, hwnd );
      usMBRC;
      fOK = FALSE;
    } /* endif */       


    // Add tag table file name to list of package files                           
    if ( fOK )
    {
      pMarkup->getTableFileName( pIda->szMUTBuffer, sizeof(pIda->szMUTBuffer) );
      fExists = UtlFileExist( pIda->szMUTBuffer );
      if ( fExists )
      {
        ulRC = UtlPackAddFile( pIda->pPackCtrl, pIda->szMUTBuffer, TAGTABLE_DATA_FILE, FILE_STORE );

        if ( ulRC == PACK_FILENOTFOUND_RC )
        {
          USHORT usMBRC;

          if ( WinFindAtom( hAtomTbl, pIda->szMUTBuffer ) == NULLHANDLE )
          {
            pszErrParm[0] = pszFormat;
            fExists = FALSE;
            usMBRC = UtlErrorHwnd( ERROR_FOLEXP_ACC_TAGTABLE,
                                   MB_YESNO, 1, pszErrParm, EQF_ERROR, hwnd );
            fOK = (usMBRC != MBID_NO );
            WinAddAtom( hAtomTbl, pIda->szMUTBuffer );
          } /* endif */
        }
        else
        {
          fOK = UtlPackHandleError( ulRC, pszFormat, hwnd );
        } /* endif */

        if ( hfInfoFile != NULL ) fprintf( hfInfoFile, "%s=%s\n", MUTINFOFILE_PLUGIN_KEY, pPlugin->getName() );
        if ( hfInfoFile != NULL ) fprintf( hfInfoFile, "%s=%s\n", MUTINFOFILE_TABLE_KEY, pIda->szMUTBuffer );

        pMarkup->getName( pIda->szMUTBuffer, sizeof(pIda->szMUTBuffer) );
        if ( hfInfoFile != NULL ) fprintf( hfInfoFile, "%s=%s\n", MUTINFOFILE_NAME_KEY, pIda->szMUTBuffer );

        pMarkup->getVersion( pIda->szMUTBuffer, sizeof(pIda->szMUTBuffer) );
        if ( hfInfoFile != NULL ) fprintf( hfInfoFile, "%s=%s\n", MUTINFOFILE_VERSION_KEY, pIda->szMUTBuffer );

        pMarkup->getLongDescription( pIda->szMUTBuffer, sizeof(pIda->szMUTBuffer) );
        if ( hfInfoFile != NULL ) fprintf( hfInfoFile, "%s=%s\n", MUTINFOFILE_DESCRIPTION_KEY, pIda->szMUTBuffer );

      }
      else
      {
        /****************************************************************/
        /* Tag table not found ==> issue error message                  */
        /****************************************************************/
        USHORT usMBRC;

        if ( WinFindAtom( hAtomTbl, pIda->szMUTBuffer ) == NULLHANDLE )
        {
          pszErrParm[0] = pszFormat;
          usMBRC = UtlErrorHwnd( ERROR_FOLEXP_ACC_TAGTABLE,
                                 MB_YESNO, 1, pszErrParm, EQF_ERROR, hwnd );
          fOK = (usMBRC != MBID_NO );
          WinAddAtom( hAtomTbl, pIda->szMUTBuffer );
        } /* endif */
      } /* endif */  
    } /* endif */


    /********************************************************************/
    /* Add any tag table user exit and other files                      */
    /********************************************************************/
    if ( fOK && fExists && !fOldVers)
    {
      if ( pMarkup->getUserExitFileName( pIda->szMUTBuffer, sizeof(pIda->szMUTBuffer) ) != 0 )
      {
        BOOL  fUserExitAdded = FALSE;

        ulRC = UtlPackAddFile( pIda->pPackCtrl, pIda->szMUTBuffer, TAGTABLE_USEREXITWIN_FILE, FILE_STORE );
        if ( hfInfoFile != NULL ) fprintf( hfInfoFile, "%s=%s\n", MUTINFOFILE_USEREXIT_KEY, pIda->szMUTBuffer );
        if ( ulRC == PACK_OK_RC )
        {
            fUserExitAdded = TRUE;
            fOK = TRUE;
        }
        else
        {
          fOK = ( ulRC != PACK_NOMEMORY_RC );
        } /* endif */

        // Report any missing user exit  
        if ( !fUserExitAdded )
        {
          pszErrParm[0] = pIda->szMUTBuffer;
          pszErrParm[1] = pszFormat;
          UtlErrorHwnd( ERROR_EXIT_NOTFOUND,
                        MB_OK, 2, pszErrParm, EQF_WARNING, hwnd );
        } /* endif */
      } /* endif */

      // handle any additional files required by markup table
      if ( fOK )
      {
        if ( pMarkup->getAllFiles( pIda->szMUTBuffer, sizeof(pIda->szMUTBuffer) ) != 0 )
        {
            PSZ pszCurFile, pszTemp;

            pszCurFile = pIda->szMUTBuffer;
            while ( fOK && (*pszCurFile != EOS) )
            {
              // isolate current file
              pszTemp = strchr( pszCurFile, ',' );
              if ( pszTemp ) *pszTemp = EOS;

              // add current file to exported folder
              if ( UtlFileExist( pszCurFile ) && !UtlPackIsInList( pIda->pPackCtrl, pszCurFile ) )
              {
                UtlPackAddFile( pIda->pPackCtrl, pszCurFile, TAGTABLE_ADDITIONAL_FILE, FILE_STORE );
                if ( hfInfoFile != NULL ) fprintf( hfInfoFile, "%s=%s\n", MUTINFOFILE_ADDFILE_KEY, pszCurFile );
              } /* endif */

              // continue with next file
              if ( pszTemp )
              {
                pszCurFile = pszTemp + 1;
              }
              else
              {
                pszCurFile = pszCurFile + strlen(pszCurFile);
              } /* endif */
            } /* endwhile */
        } /* endif */
      } /* endif */
    } /* endif */

    // close markup table info file and add it to the package
    if ( hfInfoFile != NULL ) fclose( hfInfoFile );
    UtlMakeEQFPath( pIda->szMUTBuffer, NULC, TABLE_PATH, NULL );
    strcat( pIda->szMUTBuffer, BACKSLASH_STR );
    strcat( pIda->szMUTBuffer, pIda->szFolName );
    strcat( pIda->szMUTBuffer, "-" );
    strcat( pIda->szMUTBuffer, pszFormat );
    strcat( pIda->szMUTBuffer, EXT_OF_TAGTABLEINFO );
    UtlPackAddFile( pIda->pPackCtrl, pIda->szMUTBuffer, TAGTABLE_INFO_FILE, FILE_STORE );

  } /* endif */

  return( fOK );
} /* end of function FolExpAddTagTable */

//+----------------------------------------------------------------------------+
//Internal function
//+----------------------------------------------------------------------------+
//Function name:     FolExpAddMemory
//+----------------------------------------------------------------------------+
//Description:       Adds the given memory and its related files to the list
//                   of package files
//+----------------------------------------------------------------------------+
//Input parameter:   PVOID    pPackCtrl  ptr to package control structure
//                   PSZ      pszMemory  name of translation memory
//                   HATOMTBL hAtomTbl   atom table for in-error objects
//                   BYTE     bOldVers   export in Older TM format or not
//+----------------------------------------------------------------------------+
//Returncode type:   BOOL
//+----------------------------------------------------------------------------+
BOOL FolExpAddMemory
(
PFOLEXPIDA       pIda,               // ptr to folder export instance data area
PSZ              pszMemory,          // name of translation memory
HATOMTBL         hAtomTbl,           // atom table for in-error objects
BYTE             bFormat,            // export in older TM format or not
HWND             hwnd                // owner handle for error messages
)
{
//CHAR             chTempName[MAX_EQF_PATH];     // buffer for file names
  BOOL             fOK = TRUE;         // internal OK flag
  ULONG            ulRC;               // return values from UtlPack calls
//PSZ              pszErrParm[5];      // ptr to error parameters
//EQFINFO          ErrorInfo;          // error code of property handler calls
//PPROP_NTM        pMemProp = NULL;    // ptr to memory properties
//PVOID            hMemProp = NULL;    // handle of memory properties
  CHAR             szMemory[MAX_FILESPEC]; // buffer for short TM name

  hAtomTbl;bFormat;        ;               // get rid of compiler warnings

  {
    BOOL fIsNew = FALSE;         // is-new flag
    ObjLongToShortName( pszMemory, szMemory, TM_OBJECT, &fIsNew );
  }

  MemoryFactory *pFactory = MemoryFactory::getInstance();
//OtmPlugin *pPlugin;

  // get list of files required to export this memory

  // Build fully qualified memory info file name                   
  UtlMakeEQFPath( pIda->szInfoFileName, NULC, MEM_PATH, NULL );
  strcat( pIda->szInfoFileName, BACKSLASH_STR );
  strcat( pIda->szInfoFileName, pIda->szFolName );
  strcat( pIda->szInfoFileName, "-" );
  strcat( pIda->szInfoFileName, szMemory );
  strcat( pIda->szInfoFileName, EXT_OF_MEMORYINFO );


  /********************************************************************/
  /* Add memory only if it is not in package file list already             */
  /********************************************************************/
  if (  !UtlPackIsInList( pIda->pPackCtrl, pIda->szInfoFileName ) )
  {
    // get list of memory files for this memory and create memory info file
    if ( fOK )
    {
      if ( pFactory->getMemoryFiles( NULL, pszMemory, pIda->szMUTBuffer, sizeof(pIda->szMUTBuffer), pIda->szBuffer ) == 0 )
      {
        FILE *hfInfoFile = fopen( pIda->szInfoFileName, "w" );
        if ( hfInfoFile != NULL )
        {
          fprintf( hfInfoFile, "%s=%s\n", MEMINFOFILE_NAME_KEY, pszMemory );
          fprintf( hfInfoFile, "%s=%s\n", MEMINFOFILE_PLUGIN_KEY, pIda->szBuffer );
          fprintf( hfInfoFile, "%s=%s\n", MEMINFOFILE_FILES_KEY, pIda->szMUTBuffer );
          fclose( hfInfoFile );
        }
        else
        {
          fOK = FALSE;
        } /* endif */       
      }
      else
      {
        fOK = FALSE;
      } /* endif */       

      // show error
      if(!fOK)
      {
          pFactory->showLastError(NULL, pszMemory, NULL,hwnd);
      }
    } /* endif */ 

    // Add memory info file to list of package files                           
    if ( fOK )
    {
      ulRC = UtlPackAddFile( pIda->pPackCtrl, pIda->szInfoFileName, MEMORY_INFO_FILE, FILE_STORE );

      fOK = UtlPackHandleError( ulRC, pIda->szInfoFileName, hwnd );
    } /* endif */ 

    // Add memory data files to list of package files                           
    if ( fOK )
    {
      PSZ pszCurrentFile = pIda->szMUTBuffer;
      while ( fOK && (*pszCurrentFile != EOS) )
      {
        PSZ pszEnd = strchr( pszCurrentFile, ',' );
        if ( pszEnd != NULL )
        {
          *pszEnd = EOS;
        } /* end */           

        ulRC = UtlPackAddFile( pIda->pPackCtrl, pszCurrentFile, PLUGINMEMORY_DATA_FILE, FILE_STORE );

        fOK = UtlPackHandleError( ulRC, pszCurrentFile, hwnd );

        if ( pszEnd != NULL )
        {
          pszCurrentFile = pszEnd + 1;
        }
        else
        {
          *pszCurrentFile = EOS; // force end of loop
        } /* end */           
      } /* endif */  
    } /* endif */
  } /* endif */

  return( fOK );
} /* end of function FolExpAddMemory */


//+----------------------------------------------------------------------------+
//Internal function
//+----------------------------------------------------------------------------+
//Function name:     FolExpAddDict
//+----------------------------------------------------------------------------+
//Description:       Adds the given dictionar and its related files to the
//                   list of package files
//+----------------------------------------------------------------------------+
//Input parameter:   PVOID    pPackCtrl  ptr to package control structure
//                   PSZ      pszDict    name of dictionary
//                   BYTE     bFormat    format of dictionary (non-unicode)
//                   HWND     hwnd       owner window for error messages
//+----------------------------------------------------------------------------+
//Returncode type:   BOOL
//+----------------------------------------------------------------------------+
BOOL FolExpAddDict
(
    PVOID            pPackCtrl,          // ptr to package control structure
    PSZ              pszDict,            // name of dictionary
    BYTE             bFormat,            // format of dict
    HWND             hwnd                // owner handle for error messages
)
{
  CHAR             chTempName[MAX_EQF_PATH];     // buffer for file names
  BOOL             fOK = TRUE;         // internal OK flag
  ULONG            ulRC;               // return values from UtlPack calls
  PSZ              pszErrParm[5];      // ptr to error parameters
  EQFINFO          ErrorInfo;          // error code of property handler calls
  PPROPDICTIONARY  pDicProp = NULL;    // ptr to dictionary properties
  HPROP            hDicProp;           // dictionary property handle
  CHAR             szDict[MAX_FILESPEC]; // buffer for short dict name

  {
    BOOL fIsNew = FALSE;         // is-new flag
    ObjLongToShortName( pszDict, szDict, DICT_OBJECT, &fIsNew );
  }

  //--- get access to dictionary properties ---
  UtlMakeEQFPath( chTempName, NULC, SYSTEM_PATH, NULL );
  strcat( chTempName, BACKSLASH_STR );
  strcat( chTempName, szDict );
  strcat( chTempName, EXT_OF_DICTPROP );
  hDicProp = OpenProperties( chTempName, NULL,
                             PROP_ACCESS_READ,
                             &ErrorInfo);
  if ( hDicProp )
  {
    pDicProp = (PPROPDICTIONARY)MakePropPtrFromHnd( hDicProp );
  }
  else
  {
    fOK = FALSE;
    pszErrParm[0] = pszDict;
    UtlErrorHwnd( ERROR_DIC_NOTFOUND, MB_CANCEL, 1, pszErrParm, EQF_ERROR, hwnd );
  } /* endif */

  //--- dictionary property file ---
  if ( fOK )
  {
    UtlMakeEQFPath( chTempName, NULC, PROPERTY_PATH, NULL );
    strcat( chTempName, BACKSLASH_STR );
    strcat( chTempName, szDict );
    strcat( chTempName, EXT_OF_DICTPROP );
    ulRC = UtlPackAddFile( pPackCtrl,
                           chTempName,
                           DICTIONARY_PROP_FILE,
                           FILE_STORE );
    if ( ulRC == PACK_FILENOTFOUND_RC )
    {
      fOK = FALSE;
      pszErrParm[0] = pszDict;
      UtlErrorHwnd( ERROR_DIC_NOTFOUND, MB_CANCEL, 1, pszErrParm, EQF_ERROR, hwnd );
    }
    else
    {
      fOK = UtlPackHandleError( ulRC, chTempName, hwnd );
    } /* endif */
  } /* endif */

  //--- dictionary data file ---
  if ( fOK )
  {
    switch (bFormat)
    {
      case EXP_IN_NON_UNICODE:
        ulRC = UtlPackAddFile( pPackCtrl,
                               pDicProp->szDictPath,
                               DICTIONARY_NON_UNICODE,
                               FILE_STORE );
        break;
      default:
        ulRC = UtlPackAddFile( pPackCtrl,
                               pDicProp->szDictPath,
                               DICTIONARY_DATA_FILE,
                               FILE_STORE );
        break;
    }
    if ( ulRC == PACK_FILENOTFOUND_RC )
    {
      fOK = FALSE;
      pszErrParm[0] = pszDict;
      UtlErrorHwnd( ERROR_DIC_NOTFOUND, MB_CANCEL, 1, pszErrParm,
                    EQF_ERROR, hwnd );
    }
    else
    {
      fOK = UtlPackHandleError( ulRC, chTempName, hwnd );
    } /* endif */
  } /* endif */

  //--- index data file ---
  if ( fOK && (pDicProp->szIndexPath[0] != EOS) )
  {
    switch (bFormat)
    {
      case EXP_IN_NON_UNICODE:
        ulRC = UtlPackAddFile( pPackCtrl,
                               pDicProp->szIndexPath,
                               DICTIONARY_NON_UNICODE_INDEX,
                               FILE_STORE );
        break;
      default:
        ulRC = UtlPackAddFile( pPackCtrl,
                               pDicProp->szIndexPath,
                               DICTIONARY_INDEX_FILE,
                               FILE_STORE );
        break;
    }
    if ( ulRC == PACK_FILENOTFOUND_RC )
    {
      fOK = FALSE;
      pszErrParm[0] = pszDict;
      UtlErrorHwnd( ERROR_DIC_NOTFOUND, MB_CANCEL, 1, pszErrParm,
                    EQF_ERROR, hwnd );
    }
    else
    {
      fOK = UtlPackHandleError( ulRC, chTempName, hwnd );
    } /* endif */
  } /* endif */

  //--- close dictionary properties ---
  if ( hDicProp )
  {
    CloseProperties( hDicProp, PROP_QUIT, &ErrorInfo );
  } /* endif */

  return( fOK );
} /* end of function FolExpAddDict */

//+----------------------------------------------------------------------------+
//Internal function
//+----------------------------------------------------------------------------+
//Function name:     FolPrepFolderExport
//+----------------------------------------------------------------------------+
//Function call:     _
//+----------------------------------------------------------------------------+
//Description:       Function to prepare the export of folder both for normal
//                   export via folder export process window or for batch
//                   mode folder export.
//+----------------------------------------------------------------------------+
//Input parameter:   _
//Parameters:        _
//+----------------------------------------------------------------------------+
//Output parameter:  _
//+----------------------------------------------------------------------------+
//Returncode type:   _
//+----------------------------------------------------------------------------+
//Returncodes:       _
//+----------------------------------------------------------------------------+
//Prerequesits:      - chDrive field of IDA must contain the target drive
//                   - szName field of IDA must contain folder name
//                   - the following bit flags in the package header must be
//                     set: fOverWrite, fDeleted, fSelectedDocs, fContainsMem,
//                          fContainsDict, fWithNote
//                   - the note is stored at the end of the packager header
//                     area and the note size is set in the header area
//+----------------------------------------------------------------------------+
//Side effects:      _
//+----------------------------------------------------------------------------+
//Samples:           _
//+----------------------------------------------------------------------------+
//Function flow:     _
//+----------------------------------------------------------------------------+
USHORT FolPrepFolderExport
(
PFOLEXPIDA       pIda,               // folder import dialog IDA
PSZ              pszDocuments,       // list of documents or NULL
HWND             hwndOwner           // owner window handle for error msgs
)
{
  BOOL             fOK = TRUE;         // internal O.K. flag
  USHORT           usErrMsg = 0;       // number of displayed error message
  EQFINFO          ErrorInfo;          // error code of property handler calls
  PPROPFOLDER      pFolProp;           // ptr to folder properties
  PVOID            hFolProp = NULL;    // handle of folder properties
  ULONG            ulRC;               // return values from UtlPack calls
  PSZ              pszErrParm[5];      // ptr to error parameters

  pIda->hatomInError = NULLHANDLE;     // no atom table yet

  /********************************************************************/
  /* Check input data                                                 */
  /********************************************************************/
  if ( (pIda->chDrive == ' ') || // has a drive been selected ???
       (pIda->chDrive == EOS) )
  {
    usErrMsg = ERROR_NO_TARGETDRIVE_SELECTED;
    UtlErrorHwnd( usErrMsg, MB_CANCEL, 0, (PSZ *)NULP, EQF_ERROR, hwndOwner );
    fOK = FALSE;
  } /* endif */

  //--- delete folder and select documents may not be used together
  if ( fOK &&
       pIda->pHeader->BitFlags.fDeleted &&
       pIda->pHeader->BitFlags.fSelectedDocs )
  {
    usErrMsg = ERROR_FOLEXP_DELSEL_OPTION;
    UtlErrorHwnd( usErrMsg, MB_CANCEL, 0, (PSZ *)NULP, EQF_ERROR, hwndOwner );
    fOK = FALSE;
  } /* endif */

  //--- check if an export name has been specified
  if ( fOK && pIda->fExportAs && (pIda->szExportAs[0] == EOS) )
  {
    usErrMsg = ERROR_MISSING_EXPORTFOLDER_NAME;
    UtlErrorHwnd( usErrMsg, MB_CANCEL, 0, (PSZ *)NULP, EQF_ERROR, hwndOwner );
    fOK = FALSE;
  } /* endif */

  //--- check if the export name is valid
  if ( fOK && pIda->fExportAs && !UtlCheckLongName(pIda->szExportAs) )
  {
    PSZ pszParm = pIda->szExportAs;
    usErrMsg = ERROR_INV_LONGNAME;
    UtlErrorHwnd( usErrMsg, MB_CANCEL, 1, &pszParm, EQF_ERROR, hwndOwner );
    fOK = FALSE;
  } /* endif */

  //--- check if there is already an exported folder on the target drive
  if ( fOK )
  {
    if ( pIda->fExportAs )
    {
      ANSITOOEM( pIda->szExportAs );
    } /* endif */

    //110900@bt

    //--- check if path is valid

    if (!strlen(pIda->szExportPath))
      UtlMakeEQFPath( pIda->szFullExportName, pIda->chDrive, EXPORT_PATH, NULL );
    else
    {
      pIda->chDrive = pIda->szExportPath[0];
      strcpy( pIda->szFullExportName, pIda->szExportPath);
      if (!UtlDirExist(pIda->szExportPath))
      {
        PSZ pszPath = pIda->szExportPath;
        USHORT usMBCode = UtlErrorHwnd( PATH_NOTEXIST_CREATEIT,
                                        MB_YESNO, 1,
                                        &pszPath, EQF_QUERY, hwndOwner );
        if ( usMBCode == MBID_YES )
        {
          USHORT usRC = UtlMkMultDirHwnd( pIda->szExportPath, TRUE, hwndOwner );
          if ( usRC != NO_ERROR )      //directory not created
          {
            fOK = FALSE;
          } /* endif */
        }
        else
        {
          fOK = FALSE;
        } /* endif */

        if ( !fOK )
        {
          usErrMsg = ERROR_PATH_NOT_EXIST;
          SETTEXT( hwndOwner, ID_FOLEXP_FILENAME_CB, "" );
        } /* endif */
      }
    }

    strcat( pIda->szFullExportName, BACKSLASH_STR );

    if ( pIda->fExportAs )
    {
      strcpy( pIda->szExportName, pIda->szExportAs );
    }
    else if ( pIda->fUseShortFolderName )
    {
      strcpy( pIda->szExportName, pIda->szName );
    }
    else
    {
      strcpy( pIda->szExportName, pIda->szFolLongName );
    } /* endif */


    // get confirmation to use short name if target drive does not support
    // long names
    if ( UtlIsLongFileName(pIda->szExportName) &&
         !UtlSupportsLongNames(pIda->szFullExportName[0]) )
    {
      CHAR    szDrive[5];
      USHORT  usConfirmation;
      PSZ     apszParm[2];

      sprintf( szDrive, "%c:", pIda->szFullExportName[0] );
      apszParm[0] = szDrive;
      apszParm[1] = pIda->szName;
      usErrMsg = ERROR_NOLONGNAMESUPPORT;
      usConfirmation = UtlErrorHwnd( usErrMsg, MB_YESNO, 2, apszParm,
                                     EQF_ERROR, hwndOwner );
      if ( usConfirmation == MBID_YES )
      {
        usErrMsg = 0;                  // continue folder export
        strcpy( pIda->szExportName, pIda->szName );
      }
      else
      {
        fOK = FALSE;
      } /* endif */
    } /* endif */

    strcat( pIda->szFullExportName, pIda->szExportName );
    if ( pIda->usExportFormat == FOL_TMGR_FORMAT )
    {
      strcat( pIda->szFullExportName, FOLDEREXPORT_EXT );
    }
    else if ( pIda->usExportFormat == FOL_XLIFF_FORMAT )
    {
      strcat( pIda->szFullExportName, FOLDEREXPORTXLIFF_EXT );
    }
    else
    {
      // undefined format...
    } /* endif */

    if ( fOK && !pIda->pHeader->BitFlags.fOverWrite && UtlFileExist( pIda->szFullExportName ) )
    {
      CHAR    szDrive[MAX_DRIVE];      // buffer for drive letters

      pszErrParm[0] = pIda->szExportName;
      szDrive[0] = pIda->chDrive;
      szDrive[1] = COLON;
      szDrive[2] = EOS;
      pszErrParm[1] = szDrive;
      if ( (hwndOwner == HWND_FUNCIF) || ISBATCHHWND(hwndOwner) )
      {
        usErrMsg = ERROR_FOLEXP_FOLDER_EXISTS;
        UtlErrorHwnd( usErrMsg, MB_CANCEL, 2, pszErrParm, EQF_WARNING, hwndOwner );
        fOK = FALSE;
      }
      else
      {
        BOOL fMoreToFollow = FALSE;

        if ( pIda->pszObjList )
        {
          PSZ pszNext = pIda->pszActiveObj + strlen(pIda->pszActiveObj) + 1;
          if ( *pszNext )
          {
            fMoreToFollow = TRUE;
          } /* endif */
        } /* endif */

        usErrMsg = ERROR_FOLEXP_FOLDER_EXISTS;
        if ( pIda->usFolOverwriteMode == MBID_EQF_YESTOALL )
        {
          usErrMsg = 0;
        }
        else if ( fMoreToFollow )
        {
          // export of multiple folders
          USHORT usMBReturn = UtlErrorHwnd( usErrMsg, MB_EQF_YESTOALL,
                                            2, pszErrParm,
                                            EQF_WARNING, hwndOwner );
          if ( (usMBReturn == MBID_YES) ||
               (usMBReturn == MBID_EQF_YESTOALL) )
          {
            usErrMsg = 0;
            pIda->usFolOverwriteMode = usMBReturn;
          }
          else
          {
            fOK = FALSE;
          } /* endif */
        }
        else
        {
          if ( UtlErrorHwnd( usErrMsg, MB_OKCANCEL | MB_DEFBUTTON2,
                             2, pszErrParm, EQF_WARNING, hwndOwner ) == MBID_CANCEL )
          {
            fOK = FALSE;
          }
          else
          {
            usErrMsg = 0;
          } /* endif */
        } /* endif */
      } /* endif */
    } /* endif */
  } /* endif */

  // if o.k. and note checkbox is checked, popup note dialog
  if ( fOK && pIda->pHeader->BitFlags.fWithNote )
  {
    if ( (hwndOwner == HWND_FUNCIF) || ISBATCHHWND(hwndOwner)  )
    {
      /****************************************************************/
      /* Nothing to do, note is already stored at the end of the      */
      /* header area                                                  */
      /****************************************************************/
    }
    else
    {
      HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
      DIALOGBOX( hwndOwner, FolExpNoteDlgProc, hResMod, ID_FOLNOTE_DLG,
                 pIda, fOK );
    } /* endif */
  } /* endif */

  // the rest of the preparation depends on the selected export format...
  switch ( pIda->usExportFormat )
  {
    case FOL_TMGR_FORMAT:
      //--- if o.k. build list of files to be exported (only for export in Tmgr format ---
      if ( fOK && (pIda->usExportFormat == FOL_TMGR_FORMAT) )
      {
        CHAR          chTempName[MAX_EQF_PATH];     // buffer for folder path name
        BYTE b = 0;
        if (pIda->fNonUnicode)
        {
          b = EXP_IN_NON_UNICODE;
        }
//        else if (pIda->fOldVers)
//        {
//          b = EXP_IN_VERSION1;
//        }


        //--- initialize package control structure ---
        pIda->hatomInError = WinCreateAtomTable( 100, 100 );
        strcpy( chTempName, pIda->szExportName );
        strcat( chTempName, FOLDEREXPORT_EXT );
        pIda->pPackCtrl = UtlPackInit( pIda->chDrive, NULL, pIda->szExportPath, chTempName,
                                      b, pIda->szObjName, hwndOwner );

        fOK = pIda->pPackCtrl != NULL;
        if (fOK && pIda->fMemoryExportAs )
        {
          ulRC = UtlPackRenameMemoryDuringExport( pIda->pPackCtrl, (PVOID)&(pIda->MemRename) );
        }

        //--- fill fields in package header ---
        if ( fOK )
        {
          UtlTime( &pIda->pHeader->Head.Rel1.lDateTime );

          //--- add folder property file to list ---
          UtlMakeEQFPath( chTempName, NULC, PROPERTY_PATH, NULL );
          strcat( chTempName, BACKSLASH_STR );
          strcat( chTempName, pIda->szFolName );
          ulRC = UtlPackAddFile( pIda->pPackCtrl,
                                chTempName,
                                FOLDER_PROP_FILE, FILE_STORE );
          fOK = UtlPackHandleError( ulRC, chTempName, hwndOwner );
          if ( !fOK ) usErrMsg = UtlQueryUShort( QS_LASTERRORMSGID );
        } /* endif */

        //--- add folder history log to list of files for export
        if ( fOK )
        {
          CHAR           chTempName[MAX_EQF_PATH];

          UtlMakeEQFPath( chTempName, pIda->szObjName[0],
                          PROPERTY_PATH, pIda->szFolName );
          strcat( chTempName, BACKSLASH_STR  );
          strcat( chTempName, HISTLOGFILE );
          if ( UtlFileExist( chTempName ) )
          {
            ulRC = UtlPackAddFile( pIda->pPackCtrl,
                                  chTempName,
                                  HISTLOG_DATA_FILE, FILE_STORE );
            fOK = UtlPackHandleError( ulRC, chTempName, hwndOwner );
            if ( !fOK ) usErrMsg = UtlQueryUShort( QS_LASTERRORMSGID );
          } /* endif */
        } /* endif */

        //--- add redundant segment list of files for export
        if ( fOK && pIda->fWithRedundancyData )
        {
          CHAR           chTempName[MAX_EQF_PATH];

          UtlMakeEQFPath( chTempName, pIda->szObjName[0], PROPERTY_PATH, pIda->szFolName );
          strcat( chTempName, BACKSLASH_STR  );
          strcat( chTempName, REDUNDCOUNTFILE );
          if ( UtlFileExist( chTempName ) )
          {
            // older Tmgr versions fail to import folders if there is a non-document property file in the
            // property directory of the folder, so we have to copy the redundant segment log file
            // to the folder directory temporarely...
            CHAR szTempSegLog[MAX_EQF_PATH];

            UtlMakeEQFPath( szTempSegLog, pIda->szObjName[0], SYSTEM_PATH, pIda->szFolName );
            strcat( szTempSegLog, BACKSLASH_STR  );
            strcat( szTempSegLog, REDUNDCOUNTFILE );
            UtlCopy( chTempName, szTempSegLog, 1, 0L, FALSE );

            ulRC = UtlPackAddFile( pIda->pPackCtrl, szTempSegLog, REDSEGMENT_DATA_FILE, FILE_STORE );
            fOK = UtlPackHandleError( ulRC, chTempName, hwndOwner );
            if ( !fOK ) usErrMsg = UtlQueryUShort( QS_LASTERRORMSGID );
          } /* endif */
        } /* endif */

        // add binary calculation report file to list of files for export
        if ( fOK )
        {
          CHAR           chTempName[MAX_EQF_PATH];

          UtlMakeEQFPath( chTempName, pIda->szObjName[0], SYSTEM_PATH, pIda->szFolName );
          strcat( chTempName, BACKSLASH_STR  );
          strcat( chTempName, CALREPORTDATAFILE );
          if ( UtlFileExist( chTempName ) )
          {
            ulRC = UtlPackAddFile( pIda->pPackCtrl, chTempName, BINCALCREPORT_FILE, FILE_STORE );
            fOK = UtlPackHandleError( ulRC, chTempName, hwndOwner );
            if ( !fOK ) usErrMsg = UtlQueryUShort( QS_LASTERRORMSGID );
          } /* endif */
        } /* endif */

        //--- access folder properties for memory and dictionaries  ---
        if ( fOK )
        {
          PPROPSYSTEM      pSysProp;     // ptr to EQF system properties

          strcpy( chTempName, pIda->szObjName );
          pSysProp = (PPROPSYSTEM)MakePropPtrFromHnd( EqfQuerySystemPropHnd());
          chTempName[0] = pSysProp->szPrimaryDrive[0];
          hFolProp = OpenProperties( chTempName, NULL,
                                    PROP_ACCESS_READ,
                                    &ErrorInfo);
        } /* endif */

        if ( fOK && hFolProp )
        {
          pFolProp = (PPROPFOLDER)MakePropPtrFromHnd( hFolProp );


          // save export as master folder flag to allow restore of this flag for the next folder being exported
          if ( pIda->pHeader->BitFlags.fMasterFolder )
          {
            pIda->fExportAsMasterFolder = TRUE;                          // save export asmaster folder flag
          }
          else if ( pIda->fExportAsMasterFolder )
          {
            pIda->pHeader->BitFlags.fMasterFolder = TRUE;                // restore export as master folder flag
          } /* endif */

          // reset master folder export flag if folder is no master folder
          if ( !(pFolProp->fAFCFolder && pFolProp->fTCMasterFolder) && pIda->pHeader->BitFlags.fMasterFolder )
          {
            pIda->pHeader->BitFlags.fMasterFolder = FALSE;
          } /* endif */

          //--- check/create folder unique ID ---
          if ( pFolProp->UniqueID[0] == EOS )
          {
            if ( SetPropAccess( hFolProp, PROP_ACCESS_WRITE) )
            {
              LONG lTime;              // current time value

              UtlTime( &lTime );
              sprintf( pFolProp->UniqueID, "%ld", lTime );
              SaveProperties( hFolProp, &ErrorInfo);
            } /* endif */
          } /* endif */

          //--- fill header fields from folder properties ---
          strcpy( pIda->pHeader->Head.Rel1.szDescription, pFolProp->szDescription );
          strcpy( pIda->pHeader->Head.Rel1.UniqueID, pFolProp->UniqueID );
          strcpy( pIda->pHeader->Head.Rel1.szFormat, pFolProp->szFormat );

          //--- process format table of folder ---
          if ( fOK )
          {
            fOK = FolExpAddTagTable( pIda, pFolProp->szFormat,
                                    pIda->hatomInError, hwndOwner,
                                    pIda->fOldVers );
          } /* endif */

          // process analysis profile of folder
          if ( fOK && (pFolProp->szSavedDlgIanaProfile[0] != EOS) )
          {
            CHAR             chTempName[MAX_LONGPATH];     // buffer for file names

            // Build fully qualified analysis progile name
            AnaProfBuildFullProfileName( pFolProp->szSavedDlgIanaProfile, chTempName );
            UtlPackAddFile( pIda->pPackCtrl, chTempName, ANALYSIS_PROFILE_FILE, FILE_STORE );
          } /* endif */

          //--- process translation memory if requested ---
          if ( fOK && pIda->fWithFolderMem )
          {
            if ( pFolProp->szLongMemory[0] != EOS )
            {
              BYTE b = 0;
              if (pIda->fNonUnicode)
              {
                b = EXP_IN_NON_UNICODE;
              }

              fOK = FolExpAddMemory( pIda, pFolProp->szLongMemory,
                                    pIda->hatomInError, b,
                                    hwndOwner );
              pIda->pHeader->BitFlags.fContainsMem = TRUE;
            }
            else
            {
              BYTE b = 0;
              if (pIda->fNonUnicode)
              {
                b = EXP_IN_NON_UNICODE;
              }

              fOK = FolExpAddMemory( pIda, pFolProp->szMemory,
                                    pIda->hatomInError, b,
                                    hwndOwner );
               pIda->pHeader->BitFlags.fContainsMem = TRUE;
            } /* endif */
          } /* endif */

          //--- process read-only TMs if requested ---
          if ( fOK && pIda->fWithROMem )
          {
            if ( pFolProp->aLongMemTbl[0][0] != EOS )
            {
              int i = 0;
              BYTE b = 0;
              if (pIda->fNonUnicode)
              {
                b = EXP_IN_NON_UNICODE;
              }

              while ( fOK &&
                      (i < MAX_NUM_OF_FOLDER_MDB) &&
                      (pFolProp->aLongMemTbl[i][0] != EOS) )
              {
                fOK = FolExpAddMemory( pIda, pFolProp->aLongMemTbl[i],
                                      pIda->hatomInError, b,
                                      hwndOwner );
                pIda->pHeader->BitFlags.fContainsMem = TRUE;
                i++;
              } /* endwhile */
            }
            else
            {
              PSZ  pTMs = NULL;           // ptr to copy of TM list
              PSZ  pTM = NULL;            // ptr to single TM from list

              //--- get work copy of folder TM list ---
              fOK = UtlAllocHwnd( (PVOID *)&pTMs, 0L,
                                  (LONG) (strlen( pFolProp->MemTbl ) + 20),
                                  ERROR_STORAGE, hwndOwner );
              if ( fOK )
              {
                strcpy( pTMs, pFolProp->MemTbl );
                pTM = strtok( pTMs, X15_STR );
              } /* endif */

              //--- process TMs in the list ---
              while ( fOK && pTM )
              {
                BYTE b = 0;
                if (pIda->fNonUnicode)
                {
                  b = EXP_IN_NON_UNICODE;
                }
                fOK = FolExpAddMemory( pIda, pTM,
                                      pIda->hatomInError, b,
                                      hwndOwner );
                pIda->pHeader->BitFlags.fContainsMem = TRUE;
                pTM = strtok( NULL, X15_STR );
              } /* endwhile */

              //--- get rid off TM list copy ---
              if ( pTMs )
              {
                UtlAlloc( (PVOID *)&pTMs, 0L, 0L, NOMSG );
              } /* endif */
            }
            if ( !fOK ) usErrMsg = UtlQueryUShort( QS_LASTERRORMSGID );
          } /* endif */

          //--- process dictionaries if requested ---
          if ( fOK && pIda->fWithDict )
          {
            if ( pFolProp->aLongDicTbl[0][0] != EOS )
            {
              int i = 0;
              while ( fOK &&
                      (i < MAX_NUM_OF_FOLDER_DICS) &&
                      (pFolProp->aLongDicTbl[i][0] != EOS) )
              {
                fOK = FolExpAddDict( pIda->pPackCtrl, pFolProp->aLongDicTbl[i],
                                    (BYTE)((pIda->fNonUnicode) ? EXP_IN_NON_UNICODE : 0), hwndOwner );
                pIda->pHeader->fContainsDict = TRUE;
                i++;
              } /* endwhile */
            }
            else
            {
              PSZ  pDicts = NULL;         // ptr to copy of dictionary list
              PSZ  pDictionary = NULL;    // ptr to single dict from list

              //--- get work copy of folder dictionary list ---
              fOK = UtlAllocHwnd( (PVOID *)&pDicts, 0L,
                                  (LONG) (strlen( pFolProp->DicTbl ) + 20),
                                  ERROR_STORAGE, hwndOwner );
              if ( fOK )
              {
                strcpy( pDicts, pFolProp->DicTbl );
                pDictionary = strtok( pDicts, X15_STR );
              } /* endif */

              //--- process dictionaries in the list ---
              while ( fOK && pDictionary )
              {
                fOK = FolExpAddDict( pIda->pPackCtrl, pDictionary,
                                    (BYTE)((pIda->fNonUnicode) ? EXP_IN_NON_UNICODE : 0), hwndOwner );
                pIda->pHeader->fContainsDict = TRUE;
                pDictionary = strtok( NULL, X15_STR );
              } /* endwhile */

              //--- get rid off dictionary list copy ---
              if ( pDicts )
              {
                UtlAlloc( (PVOID *)&pDicts, 0L, 0L, NOMSG );
              } /* endif */
            }
            if ( !fOK ) usErrMsg = UtlQueryUShort( QS_LASTERRORMSGID );
          } /* endif */

          //--- add global memory filter file to list of files for export
          if ( fOK && (pFolProp->szGlobalMemOptFile[0] != EOS) )
          {
            CHAR           chTempName[MAX_EQF_PATH];

            UtlMakeEQFPath( chTempName, pIda->szObjName[0], SYSTEM_PATH, pIda->szFolName );
            strcat( chTempName, BACKSLASH_STR  );
            strcat( chTempName, pFolProp->szGlobalMemOptFile );
            if ( UtlFileExist( chTempName ) )
            {
              ulRC = UtlPackAddFile( pIda->pPackCtrl, chTempName, GLOBALMEMORYFILTER_FILE, FILE_STORE );
              fOK = UtlPackHandleError( ulRC, chTempName, hwndOwner );
              if ( !fOK ) usErrMsg = UtlQueryUShort( QS_LASTERRORMSGID );
            } /* endif */
          } /* endif */

          CloseProperties( hFolProp, PROP_QUIT, &ErrorInfo );
        } /* endif */
      } /* endif */

      // add documents to list
      if ( fOK )
      {
        HWND hwndLB;
        if ( pszDocuments )
        {
          // add all documents in document list
          while ( *pszDocuments )
          {
            fOK = FolExpAddDoc( pIda, pszDocuments,
                                pIda->fWithDocMem,
                                pIda->pHeader->BitFlags.fSelectedDocs,
                                hwndOwner );
            if ( fOK )
            {
              pszDocuments += strlen(pszDocuments) + 1;
            } /* endif */
          } /* endwhile */
        }
        else
        {
    // always use hwndLongNameLB, the checked documents in the tree view control
    // have been selected in hwndLongNameLB
          hwndLB = pIda->hwndLongNameLB;
          fOK = FolExpAddLBDocs( hwndLB, pIda->hwndDocLB, pIda,
                                pIda->fWithDocMem,
                                pIda->pHeader->BitFlags.fSelectedDocs,
                                hwndOwner );
        } /* endif */
        if ( !fOK ) usErrMsg = UtlQueryUShort( QS_LASTERRORMSGID );
      } /* endif */

      // add all or all flagged subfolders of folder to list
      if ( fOK )
      {
        if ( (pIda->pSubFolInfo != NULL) && (pIda->pHeader->BitFlags.fSelectedDocs || pIda->fTranslatorSelected) )
        {
          // add marked subfolders of folder to package
          int i = 0;
          while ( fOK && (pIda->pSubFolInfo[i].szName[0] != EOS) )
          {
            if ( pIda->pSubFolInfo[i].fFlag )
            {
              PPROPFOLDER pFolderProps = NULL;
              ULONG ulLen = 0;


              UtlMakeEQFPath( pIda->szBuffer, pIda->szObjName[0],
                              PROPERTY_PATH, pIda->szFolName );
              sprintf( pIda->szBuffer + strlen(pIda->szBuffer),
                      "\\%8.8ld%s", pIda->pSubFolInfo[i].ulID, EXT_OF_SUBFOLDER );

              ulRC = UtlPackAddFile( pIda->pPackCtrl, pIda->szBuffer,
                                    SUBFOLDER_PROP_FILE, FILE_STORE );
              fOK = UtlPackHandleError( ulRC, pIda->szBuffer, hwndOwner );
              if ( !fOK ) usErrMsg = UtlQueryUShort( QS_LASTERRORMSGID );

              // add subfolder memory
              if ( UtlLoadFileL( pIda->szBuffer, (PVOID *)&pFolderProps, &ulLen, FALSE, FALSE ) )
              {
                if ( pFolderProps->szLongMemory && pFolderProps->szLongMemory[0] != EOS )
                {
                  fOK = FolExpAddMemory( pIda, pFolderProps->szLongMemory,
                                        pIda->hatomInError, (BYTE)pIda->fOldVers,
                                        hwndOwner );
                   pIda->pHeader->BitFlags.fContainsMem = TRUE;
              if ( !fOK ) usErrMsg = UtlQueryUShort( QS_LASTERRORMSGID );
            } /* endif */
                UtlAlloc( (PVOID *)&pFolderProps, 0L, 0L, NOMSG );
              } /* endif */

            } /* endif */
            i++;                           // try next one
          } /* endwhile */
        }
        else
        {
          // add all subfolders of folder to package
          HDIR hdir = HDIR_CREATE;
          ULONG ulRC = NO_ERROR;
          USHORT usCount = 1;
          FILEFINDBUF stResultBuf;

          UtlMakeEQFPath( pIda->szSearchPath, pIda->szObjName[0],
                          PROPERTY_PATH, pIda->szFolName );
          strcat( pIda->szSearchPath, BACKSLASH_STR );
          strcat( pIda->szSearchPath, DEFAULT_PATTERN_NAME );
          strcat( pIda->szSearchPath, EXT_OF_SUBFOLDER );

          ulRC = UtlFindFirst( pIda->szSearchPath, &hdir, FILE_NORMAL,
                              &stResultBuf, sizeof(stResultBuf), &usCount, 0L, 0);
          while ( (ulRC == NO_ERROR) && usCount )
          {
            // process found property file but ignore any history log files
            if ( strcmp( RESBUFNAME(stResultBuf), HISTLOGFILE ) != 0 )
            {
              PPROPDOCUMENT pProp = NULL;
              PPROPFOLDER  pFolderProps = NULL;
              ULONG ulLen = 0;

              // build property file path
              strcpy( pIda->szBuffer, pIda->szSearchPath );
              UtlSplitFnameFromPath( pIda->szBuffer );
              strcat( pIda->szBuffer, BACKSLASH_STR );
              strcat( pIda->szBuffer, RESBUFNAME(stResultBuf) );

              // handle subfolder or document
              // (the property head part of an subfolder property file is intentionally
              // left empty)
              if ( UtlLoadFileL( pIda->szBuffer, (PVOID *)&pProp, &ulLen, FALSE, FALSE ) )
              {
                if ( pProp->PropHead.szName[0] == EOS )
                {
                  // add subfolder to file list
                  ulRC = UtlPackAddFile( pIda->pPackCtrl, pIda->szBuffer,
                                        SUBFOLDER_PROP_FILE, FILE_STORE );
                  fOK = UtlPackHandleError( ulRC, pIda->szBuffer, hwndOwner );
                  if ( !fOK ) usErrMsg = UtlQueryUShort( QS_LASTERRORMSGID );
                } /* endif */
                UtlAlloc( (PVOID *)&pProp, 0L, 0L, NOMSG );
              } /* endif */

              // add subfolder memory
              if ( UtlLoadFileL( pIda->szBuffer, (PVOID *)&pFolderProps, &ulLen, FALSE, FALSE ) )
              {
                if ( pFolderProps->szLongMemory && pFolderProps->szLongMemory[0] != EOS )
                {
                  fOK = FolExpAddMemory( pIda, pFolderProps->szLongMemory,
                                        pIda->hatomInError, (BYTE)pIda->fOldVers,
                                        hwndOwner );
                   pIda->pHeader->BitFlags.fContainsMem = TRUE;
                  if ( !fOK ) usErrMsg = UtlQueryUShort( QS_LASTERRORMSGID );
              } /* endif */
                UtlAlloc( (PVOID *)&pFolderProps, 0L, 0L, NOMSG );
            } /* endif */
            } /* endif */

            // get next object
            ulRC = UtlFindNext( hdir, &stResultBuf, sizeof(stResultBuf), &usCount, FALSE );
          } /* endwhile */
          if ( hdir != HDIR_CREATE ) UtlFindClose( hdir, FALSE );
        } /* endif */
      } /* endif */


      if ( fOK )
      {
        //--- add user header area
        ulRC = UtlPackAddHeader( pIda->pPackCtrl,
                                (PBYTE) pIda->pHeader,
                                pIda->ulHeaderSize );
        fOK = UtlPackHandleError( ulRC, NULL, hwndOwner  );
        if ( !fOK ) usErrMsg = UtlQueryUShort( QS_LASTERRORMSGID );
      } /* endif */
        break;

    case FOL_XLIFF_FORMAT:
        if ( fOK )
        {
           CHAR    szFolderVendor[MAX_DESCRIPTION];
           CHAR    szDocumentVendor[MAX_DESCRIPTION];

          // allocate XLIFF import/export area
          fOK = UtlAllocHwnd( (PVOID *)&(pIda->pXLIFFData), 0, sizeof(FOLIMPEXPXLIFF), ERROR_STORAGE, pIda->hwndErrMsg );

          if ( fOK )
          {
            // output name
            strcpy( pIda->pXLIFFData->szExportName, pIda->szFullExportName );

            // set folder object name
            strcpy( pIda->pXLIFFData->szFolObjName, pIda->szObjName );

            // set export options
            pIda->pXLIFFData->fWithFolderMem = pIda->fWithFolderMem;
            pIda->pXLIFFData->fWithROMem     = pIda->fWithROMem;
            pIda->pXLIFFData->fWithDict = pIda->pHeader->fContainsDict;

            // add list of documents
            if ( pszDocuments )
            {
              // add all documents in document list
              while ( fOK && *pszDocuments )
              {
                fOK = FolExpAddDocToList( pIda, pIda->pXLIFFData, pszDocuments );
                pszDocuments += strlen(pszDocuments) + 1;
              } /* endwhile */
            }
            else
            {
              SHORT   sItem = 0;                   // listbox item index
              SHORT   sItemCount = 0;

              if ( pIda->pHeader->BitFlags.fSelectedDocs )
              {
                sItem = LIT_FIRST;                 // start with first selected item
              }
              else
              {
                sItem = 0;                         // start with first item index
                sItemCount = QUERYITEMCOUNTHWND( pIda->hwndLongNameLB );
              } /* endif */

              // open folder properties, get Vendor name
              if ( pIda->fTranslatorSelected )
              {
                PPROPSYSTEM   pSysProp;
                szFolderVendor[0] = EOS ;

                strcpy( pIda->szObjPath, pIda->szObjName );
                pSysProp = (PPROPSYSTEM)MakePropPtrFromHnd( EqfQuerySystemPropHnd());
                pIda->szObjPath[0] = pSysProp->szPrimaryDrive[0];

                hFolProp = OpenProperties( pIda->szObjPath,
                                           NULL,
                                           PROP_ACCESS_READ,
                                           &ErrorInfo);
                if ( hFolProp )
                {
                  pFolProp = (PPROPFOLDER)MakePropPtrFromHnd( hFolProp );
                  strcpy(szFolderVendor,pFolProp->szVendor);
                  CloseProperties( hFolProp, PROP_QUIT, &ErrorInfo );
                }
              }

              do
              {
                if ( pIda->pHeader->BitFlags.fSelectedDocs  )
                {
                  sItem = QUERYNEXTSELECTIONHWND( pIda->hwndLongNameLB, sItem );
                }
                else if ( sItem >= sItemCount )   // at end of listbox items ???
                {
                  sItem = LIT_NONE;               // no more items to follow
                } /* endif */

                if ( sItem != LIT_NONE )
                {
                  CHAR    szDocName[MAX_FILESPEC+1];   // buffer for document name
                  ULONG   ulLongNameLen = 0L;          // length of long name array
                  SHORT   sDocNameIndex;               // index of document in short document LB
                  BOOL    fProcessFile = TRUE;

                  // add length of document long name to document long name array length
                  QUERYITEMTEXTHWND( pIda->hwndLongNameLB, sItem, pIda->szLongName );
                  ulLongNameLen += strlen( pIda->szLongName ) + 1;
                  sDocNameIndex = (SHORT)QUERYITEMHANDLEHWND( pIda->hwndLongNameLB, sItem );
                  QUERYITEMTEXTHWND( pIda->hwndDocLB, sDocNameIndex, szDocName );

                  // open document properties, get Vendor name
                  if ( pIda->fTranslatorSelected )
                  {
                     HPROP            hpropDocument;
                     PPROPDOCUMENT    ppropDocument;
                     ULONG            ulErrorInfo;
                     PSZ              pszErrorMsg;

                     // build document object name
                     UtlMakeEQFPath( pIda->szDocObjName,
                                     pIda->szObjName[0], SYSTEM_PATH,
                                     pIda->szFolName );
                     strcat( pIda->szDocObjName, BACKSLASH_STR );
                     strcat( pIda->szDocObjName, szDocName );
                     szDocumentVendor[0] = EOS ;
                     if ((hpropDocument =
                           OpenProperties (pIda->szDocObjName, NULL,
                                           PROP_ACCESS_READ, &ulErrorInfo))== NULL)
                     {
                       // display error message
                       pszErrorMsg = pIda->szDocObjName;
                       UtlErrorHwnd( ERROR_OPEN_PROPERTIES, MB_CANCEL,
                                 1, &pszErrorMsg, EQF_ERROR, hwndOwner);
                     }
                     else
                     {
                       ppropDocument = (PPROPDOCUMENT)MakePropPtrFromHnd( hpropDocument );
                       strcpy(szDocumentVendor,ppropDocument->szVendor);
                       CloseProperties(hpropDocument , PROP_QUIT, &ErrorInfo );

                     }//end if

                     // If selecting by vendor name and name does not match, skip this file.
                     if ( ( ( szDocumentVendor[0] != EOS ) && 
                            ( strcmp(pIda->szVendor,szDocumentVendor) ) ) ||
                          ( ( szDocumentVendor[0] == EOS ) && 
                            ( strcmp(pIda->szVendor,szFolderVendor) ) ) ) {
                        fProcessFile = FALSE ;
                     }
                  }

                  if ( fProcessFile ) {
                     fOK = FolExpAddDocToList( pIda, pIda->pXLIFFData, szDocName );
                  }
                } /* endif */

                if ( !pIda->pHeader->BitFlags.fSelectedDocs && (sItem != LIT_NONE) )
                {
                  sItem++;                        // continue with next item
                } /* endif */
              } while ( fOK && (sItem != LIT_NONE) );
            } /* endif */
            if ( !fOK ) usErrMsg = UtlQueryUShort( QS_LASTERRORMSGID );
          } /* endif */
        } /* endif */
        break;


    default:
        // undefined format
        fOK = FALSE;
        break;
  } /*endswitch */

  //--- get rid off atom table
  if ( pIda->hatomInError )
  {
    WinDestroyAtomTable( pIda->hatomInError );
    pIda->hatomInError = NULLHANDLE;
  } /* endif */

  if ( !fOK && (usErrMsg == 0) )
  {
    usErrMsg = UtlQueryUShort( QS_LASTERRORMSGID );
    if ( usErrMsg == 0 )
    {
      usErrMsg = ERROR_INTERNAL;
    } /* endif */
  } /* endif */
  return( usErrMsg );
} /* end of function FolPrepFolderExport */


USHORT FolBatchFolderExport
(
HWND             hwndParent,         // handle of folder handler window
PDDEFOLEXP       pFolExp             // folder export data structure
)
{
  PFOLEXPIDA      pIda;               // folder export dialog IDA
  SHORT           sRC;                // buffer for return codes
  BOOL            fOK = TRUE;         // return value
  PSZ             pszParm;            // error parameter pointer
  HWND            hwndLB = NULLHANDLE;// listbox for folder and document names
  BOOL            fFolderIsLocked = FALSE; // 'folder has been locked' flag
  CHAR            szShortName[MAX_FILESPEC]; // buffer for folder short name

  /*******************************************************************/
  /* Create FolExpIda                                                */
  /*******************************************************************/
  fOK = UtlAlloc( (PVOID *)&pIda, 0L, (LONG) sizeof(FOLEXPIDA), NOMSG );
  if ( !fOK )
  {
    pFolExp->DDEReturn.usRc = ERROR_STORAGE;
    UtlErrorHwnd( pFolExp->DDEReturn.usRc, MB_CANCEL, 0,
                  NULL, EQF_ERROR, pFolExp->hwndErrMsg );
  } /* endif */

  /********************************************************************/
  /* Create invisible listbox for names of folder/memories/...        */
  /********************************************************************/
  if ( fOK )
  {
    // create invisible listbox for the selected documents short names
    // and for the document long names
    hwndLB = pIda->hwndDocLB = WinCreateWindow( hwndParent, WC_LISTBOX, "", 0,
                                                0, 0, 0, 0,
                                                hwndParent, HWND_TOP, 1, NULL, NULL );
    pIda->hwndLongNameLB = WinCreateWindow( hwndParent, WC_LISTBOX, "",
                                            LBS_MULTIPLESEL,
                                            0, 0, 0, 0,
                                            hwndParent, HWND_TOP, 1, NULL, NULL );
  } /* endif */

  /*******************************************************************/
  /* Check if folder exists                                          */
  /*******************************************************************/
  if ( fOK )
  {
    BOOL fIsNew;
    ObjLongToShortName( pFolExp->szFolder, szShortName, FOLDER_OBJECT,
                        &fIsNew  );
    if ( fIsNew )
    {
      fOK = FALSE;
      pszParm = pFolExp->szFolder;
      pFolExp->DDEReturn.usRc = ERROR_XLATE_FOLDER_NOT_EXIST;
      UtlErrorHwnd( pFolExp->DDEReturn.usRc, MB_CANCEL, 1,
                    &pszParm, EQF_ERROR, pFolExp->hwndErrMsg );
    } /* endif */

  } /* endif */

  /*******************************************************************/
  /* Build folder object name and fill IDA fields                    */
  /*******************************************************************/
  if ( fOK )
  {
    UtlMakeEQFPath( pIda->szObjName, NULC, SYSTEM_PATH, NULL );
    strcat( pIda->szObjName, BACKSLASH_STR );

    strcat( pIda->szObjName, szShortName );
    strcpy( pIda->szName, szShortName );
    strcpy( pIda->szFolLongName, pFolExp->szFolder );

    strcat( pIda->szObjName, EXT_FOLDER_MAIN );
    strcpy( pIda->szFolderObjName, pIda->szObjName );
    strcpy( pIda->szFolName, UtlGetFnameFromPath( pIda->szObjName ) );
  } /* endif */

  /*******************************************************************/
  /* Get folder drive to complete the folder object name             */
  /*******************************************************************/
  if ( fOK )
  {
    EQFINFO     ErrorInfo;           // error code of property handler calls
    PPROPFOLDER pFolProp;            // ptr to folder properties
    PVOID       hFolProp;            // handle of folder properties

    hFolProp = OpenProperties( pIda->szFolderObjName, NULL, PROP_ACCESS_READ,
                               &ErrorInfo);
    if ( hFolProp )
    {
      pFolProp = (PPROPFOLDER)MakePropPtrFromHnd( hFolProp );
      pIda->szObjName[0] = pFolProp->chDrive;
      pIda->szFolderObjName[0] = pFolProp->chDrive;
      CloseProperties( hFolProp, PROP_QUIT, &ErrorInfo );
    } /* endif */
  } /* endif */

  /*******************************************************************/
  /* Check if folder is locked                                       */
  /*******************************************************************/
  if ( fOK )
  {
    sRC = QUERYSYMBOL( pIda->szFolderObjName );
    if ( sRC == -1 )
    {
      // folder is not in use ==> lock this folder
      SETSYMBOL( pIda->szFolderObjName );
      fFolderIsLocked = TRUE;
    }
    else
    {
      // folder is in use ...
      fOK = FALSE;
      pszParm = pFolExp->szFolder;
      pFolExp->DDEReturn.usRc = ERROR_FOLDER_LOCKED;
      UtlErrorHwnd( ERROR_FOLDER_LOCKED, MB_CANCEL, 1,
                    &pszParm, EQF_INFO, pFolExp->hwndErrMsg );
    } /* endif */
  } /* endif */

  /*******************************************************************/
  /* Check if delete is specified and the folder is open             */
  /*******************************************************************/
  if ( fOK )
  {
    if ( pFolExp->fDelete && EqfQueryObject( pIda->szObjName, clsFOLDER, 0) )
    {
      fOK = FALSE;
      pszParm = pFolExp->szFolder;
      pFolExp->DDEReturn.usRc = ERROR_DELETE_FOLDER;
      UtlErrorHwnd( ERROR_FOLDER_LOCKED, MB_CANCEL, 1,
                    &pszParm, EQF_ERROR, pFolExp->hwndErrMsg );
    } /* endif */
  } /* endif */

  /*******************************************************************/
  /* Check if target drive is valid                                  */
  /*******************************************************************/
  if ( fOK )
  {
    CHAR szDrives[MAX_DRIVELIST];     // buffer for drive letters

    /******************************************************************/
    /* Get valid drives                                               */
    /******************************************************************/
    UtlGetDriveList( (PBYTE)szDrives );

    /******************************************************************/
    /* Check if specified target drive is in list of valid drives     */
    /******************************************************************/
    if ( strchr( szDrives, toupper(pFolExp->chToDrive) ) == NULL )
    {
      CHAR szDrive[MAX_DRIVE];

      fOK = FALSE;
      szDrive[0] = pFolExp->chToDrive;
      szDrive[1] = COLON;
      szDrive[2] = EOS;
      pszParm = szDrive;
      pFolExp->DDEReturn.usRc = ERROR_EQF_DRIVE_NOT_VALID;
      UtlErrorHwnd( pFolExp->DDEReturn.usRc, MB_CANCEL, 1,
                    &pszParm, EQF_ERROR, pFolExp->hwndErrMsg );
    } /* endif */
  } /* endif */

  /*******************************************************************/
  /* Check if specified document names are valid                     */
  /*******************************************************************/
  if ( fOK )
  {
    // get folder documents
    DELETEALLHWND( pIda->hwndDocLB );
//    EqfSend2Handler( FOLDERHANDLER, WM_EQF_INSERTNAMES,
//                     MP1FROMHWND( pIda->hwndDocLB ),
//                     MP2FROMP(pIda->szFolderObjName) );
    LoadDocumentNames( pIda->szFolderObjName, pIda->hwndDocLB,
                       LOADDOCNAMES_NAME | LOADDOCNAMES_INCLSUBFOLDERS,
                       pIda->szBuffer );


    // fill long name document listbox
    {
      SHORT sItem = QUERYITEMCOUNTHWND( pIda->hwndDocLB );
      while ( sItem-- )
      {
        SHORT sInsertItem;           // index of inserted item

        // get the document name
        QUERYITEMTEXTHWND( pIda->hwndDocLB, sItem, pIda->szDocName );

        // build document object name
        UtlMakeEQFPath( pIda->szDocObjName,
                        pIda->szObjName[0], SYSTEM_PATH,
                        pIda->szFolName );
        strcat( pIda->szDocObjName, BACKSLASH_STR );
        strcat( pIda->szDocObjName, pIda->szDocName );

        // get the long name for this document
        pIda->szLongName[0] = EOS;
        DocQueryInfo2( pIda->szDocObjName, NULL, NULL, NULL, NULL,
                       pIda->szLongName, NULL, NULL, TRUE );

        // add long name to long name listbox
        if ( pIda->szLongName[0] != EOS )
        {
          sInsertItem = INSERTITEMHWND( pIda->hwndLongNameLB, pIda->szLongName );
        }
        else
        {
          sInsertItem = INSERTITEMHWND( pIda->hwndLongNameLB, pIda->szDocName );
        } /* endif */

        // set item handle to index of document in invisible document lb
        if ( sInsertItem >= 0 )
        {
          SETITEMHANDLEHWND( pIda->hwndLongNameLB, sInsertItem, sItem );
        } /* endif */
      } /* endwhile */

    } /* endif */

    // check document names
    if ( pFolExp->usFileNums )
    {
      SHORT i;                       // loop index
      SHORT sItem;                   // listbox item index

      // Search names in listbox
      i = 0;
      while ( fOK && (i < (SHORT)pFolExp->usFileNums) )
      {
        sItem = ISEARCHITEMHWND( pIda->hwndLongNameLB, pFolExp->ppFileArray[i] );

        if ( fOK && (sItem < 0) )
        {
          PSZ pszParms[2];

          fOK = FALSE;
          pszParms[0] = pFolExp->ppFileArray[i];
          pszParms[1] = pFolExp->szFolder;
          pFolExp->DDEReturn.usRc = DDE_DOC_NOT_IN_FOLDR;
          UtlErrorHwnd( pFolExp->DDEReturn.usRc, MB_CANCEL, 2,
                        pszParms, EQF_ERROR, pFolExp->hwndErrMsg );
        } /* endif */
        i++;
      } /* endwhile */
    } /* endif */
  } /* endif */

  /*******************************************************************/
  /* Prepare folder export and fill IDA with values supplied by      */
  /* batch interface                                                 */
  /*******************************************************************/
  if ( fOK )
  {
    /*****************************************************************/
    /* Create empty folder package header                            */
    /*****************************************************************/
    ULONG ulSize;

    ulSize = sizeof(FOLEXPHEADER);
    if ( pFolExp->pszNote )
    {
      ulSize += strlen(pFolExp->pszNote) + 1;
    } /* endif */

    fOK = UtlAlloc( (PVOID *)&pIda->pHeader, 0L, (LONG)ulSize, NOMSG );
    if ( !fOK )
    {
      pFolExp->DDEReturn.usRc = ERROR_STORAGE;
      UtlErrorHwnd( pFolExp->DDEReturn.usRc, MB_CANCEL, 0,
                    NULL, EQF_ERROR, pFolExp->hwndErrMsg );
    }
    else
    {
      pIda->ulHeaderSize = ulSize;
      pIda->pHeader->BitFlags.fHeaderType  = RELEASE1_HEADER;
      if ( pFolExp->pszNote != NULL )
      {
        // sizeof Notebuffer is already 1 byte (as defined in structure)
        // plus length of string plus one byte for the string delimiter
        pIda->pHeader->Head.Rel1.usNoteSize = (USHORT)(strlen(pFolExp->pszNote) + 2);
        strcpy( pIda->pHeader->Head.Rel1.szNoteBuffer, pFolExp->pszNote );
      }
      else
      {
        pIda->pHeader->Head.Rel1.usNoteSize = 0;
      } /* endif */
    } /* endif */

    /*****************************************************************/
    /* Fill IDA and header fields                                    */
    /*****************************************************************/
    if ( fOK )
    {
      strcpy( pIda->szFolName, szShortName );
      strcat( pIda->szFolName, EXT_FOLDER_MAIN );
      strcpy( pIda->szName, szShortName );

      pIda->chDrive = pFolExp->chToDrive;

      if (pFolExp->fToPath)   // bt 050101
      {
        sprintf (pIda->szExportPath, "%c:\\%s", pFolExp->chToDrive, pFolExp->chToPath);
      }
      else
      {
        pIda->szExportPath[0] = EOS;
      }

      pIda->pHeader->BitFlags.fWithNote = (USHORT)(pFolExp->pszNote != NULL );
      pIda->fDelete =
      pIda->pHeader->BitFlags.fDeleted = (USHORT)pFolExp->fDelete;
      pIda->pHeader->BitFlags.fSelectedDocs = (pFolExp->usFileNums != 0);
      pIda->pHeader->fContainsDict = (EQF_BOOL) pFolExp->fWithDict;
      pIda->fWithFolderMem = pFolExp->fWithMem;
      pIda->pHeader->BitFlags.fContainsMem = (USHORT)pFolExp->fWithMem;
      pIda->pHeader->BitFlags.fOverWrite = (USHORT)pFolExp->fOverWrite;
      pIda->fWithROMem = pFolExp->fWithROMem;
      pIda->fWithDocMem = pFolExp->fWithDocMem;
      pIda->pHeader->BitFlags.fMasterFolder = (unsigned short)pFolExp->fAsMasterFolder;

      // select listed documents in listbox
      if ( pFolExp->usFileNums )
      {
        SHORT i;                     // loop index
        SHORT sItem;                 // listbox item index

        for ( i = 0; i < (SHORT)pFolExp->usFileNums; i++ )
        {
          sItem = ISEARCHITEMHWND( pIda->hwndLongNameLB, pFolExp->ppFileArray[i] );
          if ( sItem >= 0 )
          {
            SELECTITEMMSHWND( pIda->hwndLongNameLB, sItem );
          } /* endif */
        } /* endfor */

      } /* endif */
    } /* endif */

    /*****************************************************************/
    /* Prepare folder export                                         */
    /*****************************************************************/
    pFolExp->DDEReturn.usRc = FolPrepFolderExport( pIda, NULL, pFolExp->hwndErrMsg );
    if ( pFolExp->DDEReturn.usRc != 0 )
    {
      fOK = FALSE;
    } /* endif */
  } /* endif */

  /*******************************************************************/
  /* Start folder export process                                     */
  /*******************************************************************/
  if ( fOK )
  {
    pIda->pDDEFolExp = pFolExp;      // anchor DDE folder export area

    sprintf( pIda->szObjName, "FOLEXP:%s", pIda->szName );
    fOK = CreateProcessWindow( pIda->szObjName, FolExportCallBack, pIda );
  } /* endif */

  /*******************************************************************/
  /* Cleanup                                                         */
  /*******************************************************************/
  if ( pIda->hwndLongNameLB )  WinDestroyWindow( pIda->hwndLongNameLB );
  if ( pIda->hwndDocLB )       WinDestroyWindow( pIda->hwndDocLB );
  if ( pIda->pSubFolInfo ) UtlAlloc( (PVOID *)&pIda->pSubFolInfo, 0L, 0L, NOMSG );
  if ( !fOK )
  {
    /****************************************************************/
    /* Unlock folder                                                */
    /****************************************************************/
    if ( fFolderIsLocked )
    {
      REMOVESYMBOL( pIda->szFolderObjName );
    } /* endif */

    /****************************************************************/
    /* Free allocated memory                                        */
    /****************************************************************/
    if ( pIda )
    {
      if ( pIda->pXLIFFData ) UtlAlloc( (PVOID *)&(pIda->pXLIFFData), 0L, 0L, NOMSG) ;
      UtlAlloc( (PVOID *)&(pIda), 0L, 0L, NOMSG) ;
    } /* endif */

    /****************************************************************/
    /* report end of task to DDE handler                            */
    /****************************************************************/
    WinPostMsg( pFolExp->hwndOwner, WM_EQF_DDE_ANSWER,
                NULL, &pFolExp->DDEReturn );
  } /* endif */

  return( pFolExp->DDEReturn.usRc );
} /* end of function FolBatchFolderExport */


USHORT FolFuncExportFolder
(
PFCTDATA    pData,                   // function I/F session data
PSZ         pszFolderName,           // name of folder
PSZ         pszTargetPath,           // folder target path
LONG        lOptions,                // options for the folder export or 0L
PSZ         pszDocuments,            // list of documents or NULL
PSZ         pszDescription,           // export description or NULL
PSZ         pszExportAs,
PSZ         pszMemoryExportAs		  // export folder memory with this name
)
{
  ULONG      ulRC = NO_ERROR;         // function return code

  // prepare a new analysis run or continue current one
  if ( pData->fComplete )              // has last run been completed
  {
    // prepare a new analysis run
    pData->usProgress = 0;
    ulRC = FolFuncPrepExportFolder( pData, pszFolderName, pszTargetPath,
                                    lOptions, pszDocuments, pszDescription, pszExportAs,
                                    pszMemoryExportAs);
  }
  else
  {
    // continue current analysis process
    ulRC = FolFuncExportFolderProcess( pData );
  } /* endif */
  return( (USHORT)ulRC );
} /* end of function FolFuncExportFolder */

USHORT FolFuncPrepExportFolder
(
PFCTDATA    pData,                   // function I/F session data
PSZ         pszFolderName,           // name of folder
PSZ         pszTargetPath,           // folder target path
LONG        lOptions,                // options for the folder export or 0L
PSZ         pszDocuments,            // list of documents or NULL
PSZ         pszDescription,           // export description or NULL
PSZ         pszExportAs,
PSZ         pszMemoryExportAs
)
{
  USHORT      usRC = NO_ERROR;         // function return code
  PFOLEXPIDA  pIda = NULL;             // folder export dialog IDA
  SHORT       sRC;                     // buffer for return codes
  BOOL        fOK = TRUE;              // return value
  PSZ         pszParm;                 // error parameter pointer
  BOOL        fFolderIsLocked = FALSE; // 'folder has been locked' flag
  PSZ         pDocNameBuffer = NULL;   // document name buffer
  LONG        lBufferSize = 0L;        // size of document buffer
  LONG        lBufferUsed = 0L;        // used bytes in document buffer

  // create FolExpIda
  fOK = UtlAlloc( (PVOID *)&pIda, 0L, (LONG) sizeof(FOLEXPIDA), NOMSG );
  if ( !fOK )
  {
    usRC = ERROR_STORAGE;
    UtlErrorHwnd( usRC, MB_CANCEL, 0, NULL, EQF_ERROR, HWND_FUNCIF );
  } /* endif */


  // check if folder exists
  if ( fOK )
  {
    if ( (pszFolderName == NULL) || (*pszFolderName == EOS) )
    {
      fOK = FALSE;
      usRC = TA_MANDFOLDER;
      UtlErrorHwnd( usRC, MB_CANCEL, 0, NULL, EQF_ERROR, HWND_FUNCIF );
    }
    else
    {
      BOOL fIsNew = FALSE;
      ObjLongToShortName( pszFolderName, pIda->szName, FOLDER_OBJECT, &fIsNew);
      if ( fIsNew )
      {
        fOK = FALSE;
        pszParm = pszFolderName;
        usRC = ERROR_XLATE_FOLDER_NOT_EXIST;
        UtlErrorHwnd( usRC, MB_CANCEL, 1, &pszParm, EQF_ERROR, HWND_FUNCIF );
      } /* endif */
    } /* endif */
  } /* endif */

  // check if documents exist
  if ( fOK && (pszDocuments != NULL) && (*pszDocuments != EOS))
  {
    PSZ    pszTemp = pszDocuments;    // ptr for document list processing
    PSZ    pszDocNameStart;           // ptr for document list processing
    CHAR   chTemp;                    // buffer for current character

    // build folder object name (access to folder properties is
    // required to correct folder drive letter)
    {
      PPROPFOLDER  ppropFolder;        // pointer to folder properties
      HPROP        hpropFolder;        // folder properties handle
      ULONG        ulErrorInfo;        // error indicator from property handler

      UtlMakeEQFPath( pData->szObjName, NULC, SYSTEM_PATH, NULL );
      strcat( pData->szObjName, BACKSLASH_STR );
      strcat( pData->szObjName, pIda->szName );

      strcat( pData->szObjName, EXT_FOLDER_MAIN );
      hpropFolder = OpenProperties( pData->szObjName, NULL,
                                    PROP_ACCESS_READ, &ulErrorInfo);
      if ( hpropFolder )
      {
        ppropFolder = (PPROPFOLDER)MakePropPtrFromHnd( hpropFolder );
        if ( (ppropFolder->chDrive != EOS) && (ppropFolder->chDrive != ' ') )
        {
          pData->szObjName[0] = ppropFolder->chDrive;
        } /* endif */
        CloseProperties( hpropFolder, PROP_QUIT, &ulErrorInfo );
      } /* endif */
    }

    // isolate current document name
    pszDocNameStart = pszDocuments;
    while ( fOK && (*pszDocNameStart != EOS) )
    {
      BOOL fIsNew = FALSE;

      // isolate current document name
      {
        // skip leading whitespace and seperators
        while ( (*pszDocNameStart == ' ') || (*pszDocNameStart == COMMA) )
        {
          pszDocNameStart++;
        } /* endwhile */

        // find end of document name
        if ( *pszDocNameStart == DOUBLEQUOTE)
        {
          pszDocNameStart += 1;
          pszTemp = pszDocNameStart;
          while ( *pszTemp && (*pszTemp != DOUBLEQUOTE) )
          {
            pszTemp++;
          } /* endwhile */
          chTemp = *pszTemp;
          *pszTemp = EOS;
        }
        else
        {
          pszTemp = pszDocNameStart;
          while ( *pszTemp && (*pszTemp != COMMA) )
          {
            pszTemp++;
          } /* endwhile */
          chTemp = *pszTemp;
          *pszTemp = EOS;
        } /* endif */
      }

      if ( *pszDocNameStart != EOS)
      {
        CHAR szDocShortName[MAX_FILESPEC];

        FolLongToShortDocName( pData->szObjName, pszDocNameStart,
                               szDocShortName, &fIsNew );

        // TODO: Check if FolLongToShortDocName really works for all types
        //       of document names
        if ( fIsNew )
        {
          fOK = FALSE;
          pszParm = pszDocNameStart;
          usRC = ERROR_TA_SOURCEFILE;
          UtlErrorHwnd( usRC, MB_CANCEL, 1, &pszParm, EQF_ERROR, HWND_FUNCIF );
        }
        else
        {
          // add document short name to document name buffer
          LONG lAddLen = strlen(szDocShortName) + 1;
          if ( lBufferSize < (lBufferUsed + lAddLen + 1) )
          {
            UtlAllocHwnd( (PVOID *)&pDocNameBuffer, lBufferSize,
                          lBufferSize + 8096L, ERROR_STORAGE, HWND_FUNCIF );
            lBufferSize += 8096L;
          } /* endif */

          if ( pDocNameBuffer != NULL )
          {
            strcpy( pDocNameBuffer + lBufferUsed, szDocShortName );
            lBufferUsed += lAddLen;
          } /* endif */
        } /* endif */
      } /* endif */

      // next document name
      if ( chTemp == DOUBLEQUOTE )
      {
        *pszTemp = chTemp;
        pszDocNameStart = pszTemp + 1;
      }
      else
      {
        *pszTemp = chTemp;
        pszDocNameStart = pszTemp;
      } /* endif */
    } /* endwhile */
    if ( pDocNameBuffer != NULL )
    {
      pDocNameBuffer[lBufferUsed] = EOS; // terminate document name buffer
    } /* endif */
  } /* endif */

  //--- check if the export name is valid
  if ( fOK && (pszExportAs != NULL) && (*pszExportAs != EOS))
  {
    if (!UtlCheckLongName(pszExportAs) )
    {
      PSZ pszParm = pszExportAs;
      usRC = ERROR_INV_LONGNAME;
      UtlErrorHwnd( usRC, MB_CANCEL, 1, &pszParm, EQF_ERROR, HWND_FUNCIF );
      fOK = FALSE;
    } /* endif */
  } /* endif */

  // build folder object name and fill IDA fields
  if ( fOK )
  {
    UtlMakeEQFPath( pIda->szObjName, NULC, SYSTEM_PATH, NULL );
    strcat( pIda->szObjName, BACKSLASH_STR );
    strcat( pIda->szObjName, pIda->szName );

    strcat( pIda->szObjName, EXT_FOLDER_MAIN );
    strcpy( pIda->szFolderObjName, pIda->szObjName );
    strcpy( pIda->szFolName, UtlGetFnameFromPath( pIda->szObjName ) );
  } /* endif */

  // get folder drive to complete the folder object name
  if ( fOK )
  {
    EQFINFO     ErrorInfo;           // error code of property handler calls
    PPROPFOLDER pFolProp;            // ptr to folder properties
    PVOID       hFolProp;            // handle of folder properties

    hFolProp = OpenProperties( pIda->szFolderObjName, NULL, PROP_ACCESS_READ,
                               &ErrorInfo);
    if ( hFolProp )
    {
      pFolProp = (PPROPFOLDER)MakePropPtrFromHnd( hFolProp );
      pIda->szObjName[0] = pFolProp->chDrive;
      pIda->szFolderObjName[0] = pFolProp->chDrive;
      //R008591: fill Folder Memory name
      strcpy( pIda->szFolMem, pFolProp->szMemory );
      strcpy( pIda->szLongFolMem, pFolProp->szLongMemory );

      // check rename memory names and prepare rename memory table
      if ( (pszMemoryExportAs != NULL) && (*pszMemoryExportAs != EOS))
      {
        usRC = FolImpBuildMemRenameTable( pFolProp, &(pIda->MemRename), pszMemoryExportAs );
        if (usRC == 0 )
        {
          pIda->fMemoryExportAs = TRUE;
        }
        else
        {
          pIda->fMemoryExportAs = FALSE;
          fOK = FALSE;
        } /* endif */
      } /* endif */

      CloseProperties( hFolProp, PROP_QUIT, &ErrorInfo );
    } /* endif */
  } /* endif */

  // check if folder is locked
  if ( fOK )
  {
    sRC = QUERYSYMBOL( pIda->szFolderObjName );
    if ( sRC == -1 )
    {
      // folder is not in use ==> lock this folder
      SETSYMBOL( pIda->szFolderObjName );
      fFolderIsLocked = TRUE;
    }
    else
    {
      // folder is in use ...
      fOK = FALSE;
      pszParm = pszFolderName;
      usRC = ERROR_FOLDER_LOCKED;
      UtlErrorHwnd( ERROR_FOLDER_LOCKED, MB_CANCEL, 1,
                    &pszParm, EQF_INFO, HWND_FUNCIF );
    } /* endif */
  } /* endif */

  // check if delete is specified and the folder is open
  if ( fOK )
  {
    if ( (lOptions & DELETE_OPT) &&
         EqfQueryObject( pIda->szObjName, clsFOLDER, 0) )
    {
      fOK = FALSE;
      pszParm = pszFolderName;
      usRC = ERROR_DELETE_FOLDER;
      UtlErrorHwnd( ERROR_FOLDER_LOCKED, MB_CANCEL, 1,
                    &pszParm, EQF_ERROR, HWND_FUNCIF );
    } /* endif */
  } /* endif */

  // check if target drive is valid
  if ( fOK )
  {
    if (!UtlDirExist(pszTargetPath))
    {
      usRC = ERROR_FILENAME_NOT_VALID;
      UtlErrorHwnd( usRC, MB_CANCEL, 1, &pszTargetPath, EQF_ERROR, HWND_FUNCIF );
      fOK = FALSE;
    } /* endif */
  } /* endif */

  // prepare folder export and fill IDA fields
  if ( fOK )
  {
    // create empty folder package header
    ULONG ulSize;

    ulSize = sizeof(FOLEXPHEADER);
    if ( (pszDescription != NULL) && (*pszDescription != EOS) )
    {
      ulSize += strlen(pszDescription) + 1;
    } /* endif */

    fOK = UtlAlloc( (PVOID *)&pIda->pHeader, 0L, (LONG)ulSize, NOMSG );
    if ( !fOK )
    {
      usRC = ERROR_STORAGE;
      UtlErrorHwnd( usRC, MB_CANCEL, 0, NULL, EQF_ERROR, HWND_FUNCIF );
    }
    else
    {
      pIda->ulHeaderSize = ulSize;
      pIda->pHeader->BitFlags.fHeaderType  = RELEASE1_HEADER;
      if ( (pszDescription != NULL) && (*pszDescription != EOS) )
      {
        // sizeof Notebuffer is already 1 byte (as defined in structure)
        // plus length of string plus one byte for the string delimiter
        pIda->pHeader->Head.Rel1.usNoteSize = (USHORT)(strlen(pszDescription) + 2);
        strcpy( pIda->pHeader->Head.Rel1.szNoteBuffer, pszDescription );
      }
      else
      {
        pIda->pHeader->Head.Rel1.usNoteSize = 0;
      } /* endif */
    } /* endif */

    // fill IDA and header fields
    if ( fOK )
    {
      pData->pvFolderExportIda = pIda;
      strcpy( pIda->szFolLongName, pszFolderName );
      strcpy( pIda->szFolName, pIda->szName );

      strcat( pIda->szFolName, EXT_FOLDER_MAIN );
      pIda->chDrive = pszTargetPath[0];
      pIda->pHeader->BitFlags.fWithNote = (USHORT)
      ((pszDescription != NULL) && (*pszDescription != EOS));
      pIda->fDelete =
      pIda->pHeader->BitFlags.fDeleted = (USHORT)(lOptions & DELETE_OPT) != 0L;
      pIda->pHeader->BitFlags.fSelectedDocs = (USHORT)(lBufferUsed != 0);
	    pIda->fWithDict = (EQF_BOOL)((lOptions & WITHDICT_OPT) != 0L);
      pIda->pHeader->fContainsDict = (EQF_BOOL)((lOptions & WITHDICT_OPT) != 0L);
      pIda->pHeader->BitFlags.fContainsMem = (USHORT)((lOptions & (WITHMEM_OPT | WITHREADONLYMEM_OPT | WITHDOCMEM_OPT)) != 0L);
      pIda->pHeader->BitFlags.fOverWrite = (USHORT)((lOptions & OVERWRITE_OPT) != 0L);
      pIda->fWithDict = (EQF_BOOL)((lOptions & WITHDICT_OPT) != 0L);
      pIda->fWithFolderMem = ((lOptions & WITHMEM_OPT) != 0L);
      pIda->fWithROMem = ((lOptions & WITHREADONLYMEM_OPT) != 0L);
      pIda->fWithDocMem = ((lOptions & WITHDOCMEM_OPT) != 0L);
      pIda->pHeader->BitFlags.fMasterFolder = (USHORT)((lOptions & MASTERFOLDER_OPT) != 0L);
      pIda->usExportFormat = ((lOptions & XLIFF_OPT) != 0L) ? FOL_XLIFF_FORMAT : FOL_TMGR_FORMAT;
      if ( (pszExportAs != NULL) && (*pszExportAs != EOS) )
	    {
	       pIda->fExportAs = 1;
	       strcpy( pIda->szExportAs, pszExportAs );
	    }
	    else
	    {
  	     pIda->fExportAs = 0;
      } /* endif */
      pIda->fWithRedundancyData = ((lOptions & WO_REDUNDANCY_DATA_OPT) == 0L);
      pIda->pHeader->BitFlags.fWithRedundancyData = pIda->fWithRedundancyData;
    } /* endif */

    // load names of all folder documents if none specified
    if ( fOK )
    {
      if ( (pszDocuments == NULL) || (*pszDocuments == EOS) )
      {
        LoadDocumentNames( pIda->szObjName, HWND_FUNCIF,
                           LOADDOCNAMES_INCLSUBFOLDERS, (PSZ)&pDocNameBuffer );
      } /* endif */
    } /* endif */

    strcpy( pIda->szExportPath, pszTargetPath) ;

    // prepare folder export
    usRC = FolPrepFolderExport( pIda, pDocNameBuffer, HWND_FUNCIF );
    if ( usRC != 0 )
    {
      fOK = FALSE;
    }
    else
    {
      if ( pIda->usExportFormat == FOL_TMGR_FORMAT )
      {
        UtlPackSetOwnerHwnd( pIda->pPackCtrl, HWND_FUNCIF );
      } /* endif */
      pIda->hwndErrMsg = HWND_FUNCIF;
      pData->fComplete = FALSE;
    } /* endif */
  } /* endif */

  // cleanup
  if ( pDocNameBuffer ) UtlAlloc( (PVOID *)&(pDocNameBuffer), 0L, 0L, NOMSG) ;

  if ( !fOK )
  {
    // unlock folder
    if ( fFolderIsLocked )
    {
      REMOVESYMBOL( pIda->szFolderObjName );
    } /* endif */

    // free allocated memory
    if ( pIda ) UtlAlloc( (PVOID *)&(pIda), 0L, 0L, NOMSG) ;
  } /* endif */


  return( usRC );
} /* end of function FolFuncPrepExportFolder */

ULONG FolFuncExportFolderProcess
(
PFCTDATA    pData                    // function I/F session data
)
{
  ULONG     ulRC = NO_ERROR;         // function return code
  PFOLEXPIDA      pIda;                // pointer to instance area

  pIda = (PFOLEXPIDA)pData->pvFolderExportIda;

  if ( pIda->usExportFormat == FOL_TMGR_FORMAT )
  {
    ulRC = UtlPackWritePackage( pIda->pPackCtrl, &pData->usProgress );

    switch ( ulRC )
    {
      case PACK_OK_RC:
        //--- continue in writing package ---
        ulRC = NO_ERROR;
        break;

      case PACK_COMPLETE_RC:
        pData->fComplete = TRUE;
        pData->usProgress = 100;
        ulRC = NO_ERROR;

        // process folder delete flag
        if ( pIda->fDelete )
        {
          REMOVESYMBOL( pIda->szFolderObjName );
          FolFuncDeleteFolder( pIda->szFolLongName );
          SETSYMBOL( pIda->szFolderObjName );
        } /* endif */

        if ( pIda->pPackCtrl )
        {
          UtlPackCleanup( pIda->pPackCtrl );
          pIda->pPackCtrl = NULL;
        } /* endif */

        // call general folder export cleanup
        FolExpCleanup( pIda );

        // remove folder symbol
        REMOVESYMBOL( pIda->szFolderObjName );

        break;
      default:
        ulRC = UtlQueryUShort( QS_LASTERRORMSGID );

        if ( pIda->pPackCtrl )
        {
          UtlPackCleanup( pIda->pPackCtrl );
          pIda->pPackCtrl = NULL;
        } /* endif */

        // call general folder export cleanup
        FolExpCleanup( pIda );

        // remove folder symbol
        REMOVESYMBOL( pIda->szFolderObjName );

        // get rid off exported file
        UtlMakeEQFPath( pIda->szObjPath, pIda->chDrive, EXPORT_PATH, NULL );
        strcat( pIda->szObjPath, BACKSLASH_STR );
        strcat( pIda->szObjPath, pIda->szName );
        strcat( pIda->szObjPath, FOLDEREXPORT_EXT );
        UtlDelete( pIda->szObjPath, 0L, FALSE );
        break;
    } /* enswitch */
  }
  else
  {
    int iRC = -1;
    typedef int (* __cdecl FOLXLIFFEXPORT_DCL)( PFOLIMPEXPXLIFF ) ;

    HMODULE hXLIFF = LoadLibrary("EQFXLIFF.DLL");
    if (hXLIFF != NULL) {
      FOLXLIFFEXPORT_DCL pfnFOLXLIFFEXPORT = (FOLXLIFFEXPORT_DCL)GetProcAddress(hXLIFF, "FOLXLIFFEXPORT");
      if ( pfnFOLXLIFFEXPORT != NULL )
         iRC = (*pfnFOLXLIFFEXPORT)( pIda->pXLIFFData );
      if (hXLIFF)
          FreeLibrary(hXLIFF);
    }

    pData->usProgress = pIda->pXLIFFData->usComplete;

    switch ( iRC )
    {
      case 0:
        ulRC = 0;
        break;

      case FOL_EXPORT_IMPORT_COMPLETE:
        pData->usProgress  = 100;
        pData->fComplete = TRUE;
        ulRC = 0;

        // call general folder export cleanup
        FolExpCleanup( pIda );

        // remove folder symbol
        REMOVESYMBOL( pIda->szFolderObjName );
        break;

      default:
        ulRC = UtlQueryUShort( QS_LASTERRORMSGID );

        // call general folder export cleanup
        FolExpCleanup( pIda );

        // remove folder symbol
        REMOVESYMBOL( pIda->szFolderObjName );

        // get rid off exported file
        UtlMakeEQFPath( pIda->szObjPath, pIda->chDrive, EXPORT_PATH, NULL );
        strcat( pIda->szObjPath, BACKSLASH_STR );
        strcat( pIda->szObjPath, pIda->szName );
        strcat( pIda->szObjPath, FOLDEREXPORT_EXT );
        UtlDelete( pIda->szObjPath, 0L, FALSE );
        break;
    } /*endswitch */
  } /* endif */
  return( ulRC );
} /* end of function FolFuncExportFolderProcess */

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// PROPERTY SHEET///////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//+----------------------------------------------------------------------------+
//Internal function
//+----------------------------------------------------------------------------+
//Function name:     FolPropExpInit
//+----------------------------------------------------------------------------+
//Function call:
//+----------------------------------------------------------------------------+
//Description:
//+----------------------------------------------------------------------------+
//Parameters:
//
//
//+----------------------------------------------------------------------------+
//Returncode type:   MRESULT
//+----------------------------------------------------------------------------+
//Returncodes:       return code from default window proc or FALSE
//+----------------------------------------------------------------------------+
//Function flow:
//
//
//
//
//
//+----------------------------------------------------------------------------+

MRESULT FolPropExpInit
(
HWND    hwndDlg,                    // handle of folder export dialog window
WPARAM  mp1,                        // first parameter of WM_INITDLG
LPARAM  mp2                         // second parameter of WM_INITDLG
)
{
  MRESULT     mResult = MRFROMSHORT(TRUE); // result of message processing
  CHAR        szDrives[26];           // list of available drives
  PFOLEXPIDA  pIda;                   // ptr to folder export IDA
  EQFINFO     ErrorInfo;              // error code of property handler calls
  PPROPFOLDER pFolProp;               // ptr to folder properties
  PPROPFOLDERLIST pFllProp;           // ptr to folder list properties
  PVOID       hFolProp;               // handle of folder properties
  PPROPSYSTEM   pSysProp;             // ptr to EQF system properties
//   BOOL          fDict = FALSE;        // folder-has-associated-dicts flag

  mp1 = mp1;                         // suppress 'unreferenced parameter' msg

  //--- adress IDA and store pointer to it ---
  pIda = (PFOLEXPIDA) mp2;
  if ( pIda )
  {
    ANCHORDLGIDA( hwndDlg, pIda );
  } /* endif */


  pIda -> fHistlog = FALSE;


  SetCtrlFnt (hwndDlg, GetCharSet(), ID_FOLEXP_FILENAME_CB, 0 );

  //--- create empty folder package header ---
  if ( pIda )
  {
    UtlAlloc( (PVOID *)&pIda->pHeader, 0L, (LONG) sizeof(FOLEXPHEADER), ERROR_STORAGE );
    if ( pIda->pHeader )
    {
      pIda->ulHeaderSize = sizeof(FOLEXPHEADER);
      pIda->pHeader->BitFlags.fHeaderType  = RELEASE1_HEADER;
      pIda->pHeader->Head.Rel1.usNoteSize =
      sizeof(pIda->pHeader->Head.Rel1.szNoteBuffer);
    } /* endif */

  } /* endif */

  if ( pIda )
  {
    //--- fill in IDA fields ---
    strcpy( pIda->szFolName, UtlGetFnameFromPath( pIda->szObjName ) );
    Utlstrccpy( pIda->szName, pIda->szFolName, DOT );
    pIda->chDrive = ' ';

    //--- access folder list properties (for last used values) ---
    pIda->hwndFll = EqfQueryActiveFolderlistHwnd();
    if ( pIda->hwndFll != NULLHANDLE )
    {
      EqfQueryObjectName( pIda->hwndFll, pIda->szFllObjName );
    }
    else
    {
      // build standard folder list object name
      UtlMakeEQFPath( pIda->szFllObjName, NULC, SYSTEM_PATH, NULL );
      strcat( pIda->szFllObjName, BACKSLASH_STR );
      strcat( pIda->szFllObjName, DEFAULT_FOLDERLIST_NAME );
    } /* endif */
    pIda->hFllProp = OpenProperties( pIda->szFllObjName,
                                     NULL,
                                     PROP_ACCESS_READ,
                                     &ErrorInfo );

    //--- access folder properties and display folder description ---
    strcpy( pIda->szObjPath, pIda->szObjName );
    pSysProp = (PPROPSYSTEM)MakePropPtrFromHnd( EqfQuerySystemPropHnd());
    pIda->szObjPath[0] = pSysProp->szPrimaryDrive[0];
    hFolProp = OpenProperties( pIda->szObjPath,
                               NULL,
                               PROP_ACCESS_READ,
                               &ErrorInfo);
    if ( hFolProp )
    {
      pFolProp = (PPROPFOLDER)MakePropPtrFromHnd( hFolProp );
      SETTEXTLIMIT( hwndDlg, ID_FOLEXP_DESCR_EF,
                    sizeof(pFolProp->szDescription) - 1);
      OEMTOANSI( pFolProp->szDescription );
      SETTEXT( hwndDlg, ID_FOLEXP_DESCR_EF, pFolProp->szDescription );
      ANSITOOEM( pFolProp->szDescription );
      pIda->fDict = FALSE;
      if ( pFolProp->aLongDicTbl[0][0] != EOS )
      {
        pIda->fDict = TRUE;
      }
      else if ( pFolProp->DicTbl[0] != EOS )
      {
        pIda->fDict = TRUE;
      } /* endif */

      pIda->fFolderHasROMem = FALSE;

      if ( pFolProp->aLongMemTbl[0][0] != EOS )
      {
        pIda->fFolderHasROMem = TRUE;
      }
      else if ( pFolProp->MemTbl[0] != EOS )
      {
        pIda->fFolderHasROMem = TRUE;
      } /* endif */

      strcpy( pIda->szFolMem, pFolProp->szMemory );

      if ( pFolProp->szLongName[0] != EOS)
      {
        strcpy( pIda->szFolLongName, pFolProp->szLongName );
      }
      else
      {
        strcpy( pIda->szFolLongName, pIda->szName );
      } /* endif */

      // get master folder flag
      pIda->fIsMasterFolder = (pFolProp->fAFCFolder && pFolProp->fTCMasterFolder);

      CloseProperties( hFolProp, PROP_QUIT, &ErrorInfo );
    } /* endif */

    //--- display folder name in dialog
    if ( pIda->pszObjList )
    {
      // build list of folder long names for display in the name field
      {
        // count number of folders in list
        int iNumberOfFolder = 0;
        PSZ pszCurFol = pIda->pszObjList;
        PSZ pszCurPos;
        while ( *pszCurFol )
        {
          iNumberOfFolder++ ;
          pszCurFol += strlen(pszCurFol) + 1;
        } /* endwhile */

        // allocate a buffer large enough for the folder names
        UtlAlloc( (PVOID *)&(pIda->pszNameList), 0L, iNumberOfFolder * MAX_LONGFILESPEC, NOMSG );

        // add folder long names to the buffer
        if ( pIda->pszNameList )
        {
          pszCurFol = pIda->pszObjList;
          pszCurPos = pIda->pszNameList;
          while ( *pszCurFol )
          {
            *pszCurPos++ = '\"';
            SubFolObjectNameToName( pszCurFol, pszCurPos );
            pszCurPos += strlen(pszCurPos);
            *pszCurPos++ = '\"';
            pszCurFol += strlen(pszCurFol) + 1;
            if ( *pszCurFol )
            {
              // add delimiters for following folder name
              *pszCurPos++ = ',';
              *pszCurPos++ = ' ';
            } /* endif */
          } /* endwhile */
          *pszCurPos = EOS;
          OEMTOANSI( pIda->pszNameList );
        } /* endif */
      }

      // display name list completion message
      if ( pIda->pszNameList )
      {
        SETTEXT( hwndDlg, ID_FOLEXP_NAME_EF, pIda->pszNameList );
      } /* endif */

      // disable/hide controls which are inactive for export of multiple folders
      ENABLECTRL( hwndDlg, ID_FOLEXP_SELDOCS_CHK, FALSE );
      ENABLECTRL( hwndDlg, ID_FOLEXP_SELALL_PB, FALSE );
      ENABLECTRL( hwndDlg, ID_FOLEXP_DESELALL_PB, FALSE );
      ENABLECTRL( hwndDlg, ID_FOLEXP_DOCS_LB, FALSE );
      HIDECONTROL( hwndDlg, ID_FOLEXP_DESCR_EF );
    }
    else
    {
      OEMSETTEXT( hwndDlg, ID_FOLEXP_NAME_EF, pIda->szFolLongName );

      //--- set initial state of 'Select doc ... ' checkbox

      SETCHECK_FALSE( hwndDlg, ID_FOLEXP_SELDOCS_CHK );
      ENABLECTRL( hwndDlg, ID_FOLEXP_SELALL_PB, FALSE );
      ENABLECTRL( hwndDlg, ID_FOLEXP_DESELALL_PB, FALSE );
      ENABLECTRL( hwndDlg, ID_FOLEXP_DOCS_LB, FALSE );
    }

    ENABLECTRL( hwndDlg, ID_FOLEXP_TRANSLATOR_CB, FALSE );


    SetCtrlFnt(hwndDlg, GetCharSet(), ID_FOLEXP_DESCR_EF, 0 );
    FolExpPropCommand( hwndDlg, ID_FOLEXP_SELDOCS_CHK, 0 );


    // get subfolder info table
    {
      SubFolCreateInfoTable( pIda->szObjName, &pIda->pSubFolInfo );
    }

    // create invisible listbox for the selected documents short names
    // and for the document long names
    // and for translator list

    pIda->hwndDocLB = WinCreateWindow( hwndDlg, WC_LISTBOX, "", 0,
                                       0, 0, 0, 0,
                                       hwndDlg, HWND_TOP, 1, NULL, NULL );
    pIda->hwndLongNameLB = WinCreateWindow( hwndDlg, WC_LISTBOX, "", LBS_MULTIPLESEL,
                                            0, 0, 0, 0,
                                            hwndDlg, HWND_TOP, 1, NULL, NULL );

    pIda->hwndTranslatorLB = WinCreateWindow( hwndDlg, WC_LISTBOX, "", 0,
                                              0, 0, 0, 0,
                                              hwndDlg, HWND_TOP, 1, NULL, NULL );


    // get folder documents
//    EqfSend2Handler( FOLDERHANDLER, WM_EQF_INSERTNAMES,
//                     MP1FROMHWND( pIda->hwndDocLB ),
//                     MP2FROMP(pIda->szObjName) );
    LoadDocumentNames( pIda->szObjName, pIda->hwndDocLB,
                       LOADDOCNAMES_NAME | LOADDOCNAMES_INCLSUBFOLDERS,
                       pIda->szBuffer );


    //
    // open folder properties, get translator information
    //

    {
      EQFINFO    ErrorInfo;          // error info of property handler
      HPROP      hProp = NULL;       // folder list properties handle
      PPROPFOLDER pProp;             // ptr to folder properties
      PSZ        pszParm;            // ptr to error message parameters

      /**************************************************************/
      /* Open folder list properties                                */
      /**************************************************************/
      hProp = OpenProperties( pIda->szObjPath, NULL,
                              PROP_ACCESS_READ, &ErrorInfo);
      if ( hProp )
      {
        pProp = (PPROPFOLDER)MakePropPtrFromHnd( hProp );

        if ( pProp->szVendor[0] != EOS )
        {
          OEMTOANSI( pProp->szVendor );   // szVendor is in ASCII!!!
          INSERTITEMHWND( pIda->hwndTranslatorLB, pProp->szVendor );
          ANSITOOEM( pProp->szVendor );
        } /* endif */
        CloseProperties( hProp, PROP_QUIT, &ErrorInfo );

      }
      else
      {
        pszParm = pIda->szObjName;
        UtlError( ERROR_OPEN_PROPERTIES, MB_CANCEL, 1, &pszParm, EQF_ERROR );
      } /* endif */

    }


    // fill long name document listbox and check for document TMs
    {
      SHORT sItem = QUERYITEMCOUNTHWND( pIda->hwndDocLB );

      while ( sItem-- )
      {
        SHORT sInsertItem;           // index of inserted item
        SHORT sInsertItemTranslator;           // index of inserted item

        // get the document name
        QUERYITEMTEXTHWND( pIda->hwndDocLB, sItem, pIda->szDocName );

        // build document object name
        UtlMakeEQFPath( pIda->szDocObjName,
                        pIda->szObjName[0], SYSTEM_PATH,
                        pIda->szFolName );
        strcat( pIda->szDocObjName, BACKSLASH_STR );
        strcat( pIda->szDocObjName, pIda->szDocName );


        //
        // open docment properties, get translator information
        //

        {
          HPROP  hpropDocument;
          PPROPDOCUMENT ppropDocument;
          ULONG   ulErrorInfo = 0;                  // error number
          PSZ    pszReplace;

          hpropDocument = OpenProperties( pIda->szDocObjName, NULL,
                                          PROP_ACCESS_READ, &ulErrorInfo );

          if ( !hpropDocument )
          {
            // display error message
            pszReplace = pIda->szDocObjName;
            UtlError( ERROR_OPEN_PROPERTIES, MB_CANCEL,
                      1, &pszReplace, EQF_ERROR);
          }
          else
          {

            ppropDocument = (PPROPDOCUMENT)MakePropPtrFromHnd( hpropDocument );

            if ( ppropDocument->szVendor[0] != EOS )
            {
              OEMTOANSI(  ppropDocument->szVendor );  // szVendor is in ASCII!!!
              if (SEARCHITEMHWND( pIda->hwndTranslatorLB, ppropDocument->szVendor )== LIT_NONE)
              {
                sInsertItemTranslator = INSERTITEMHWND( pIda->hwndTranslatorLB, ppropDocument->szVendor );
              } // end if
              ANSITOOEM(  ppropDocument->szVendor );
            } /* endif */
            CloseProperties( hpropDocument, PROP_QUIT, &ErrorInfo );
          } // end if

        }

        //
        // get the long name and the TM for this document
        //
        pIda->szLongName[0] = EOS;
        DocQueryInfo2( pIda->szDocObjName, pIda->szDocMem, NULL, NULL, NULL,
                       pIda->szLongName, NULL, NULL, TRUE );

        // add long name to long name listbox
        if ( pIda->szLongName[0] != EOS )
        {
          OEMTOANSI( pIda->szLongName );
          sInsertItem = INSERTITEMHWND( pIda->hwndLongNameLB, pIda->szLongName );
        }
        else
        {
          sInsertItem = INSERTITEMHWND( pIda->hwndLongNameLB, pIda->szDocName );
        } /* endif */

        // set item handle to index of document in invisible document lb
        if ( sInsertItem >= 0 )
        {
          SETITEMHANDLEHWND( pIda->hwndLongNameLB, sInsertItem, sItem );
        } /* endif */

        // set fDocTM flag if document TM is different from folder TM
        if ( strcmp( pIda->szFolMem, pIda->szDocMem ) != 0 )
        {
          pIda->fDocMem = TRUE;
        } /* endif */
      } /* endwhile */

    } /* endif */

    //--- create drive buttons for available drives ---
    UtlGetDriveList( (PBYTE)szDrives );

    //--- apply last used values ---
    pFllProp = (PPROPFOLDERLIST)MakePropPtrFromHnd( pIda->hFllProp );
    if ( pFllProp->fExportPath && strchr( szDrives, toupper(pFllProp->chFolExpDrive) ) )
    {
      pIda->chDrive = pFllProp->chFolExpDrive;
      strcpy(pIda->szExportPath, pFllProp->szExportPath);
    }
    else
    {
      UtlQueryString( QST_PRIMARYDRIVE, pIda->szExportPath, sizeof(pIda->szExportPath) );
      pIda->chDrive = pIda->szExportPath[0];
      sprintf( pIda->szExportPath, "%c:\\", pIda->chDrive );
    } /* endif */

    UtlLoadLastUsedStrings( hwndDlg, ID_FOLEXP_FILENAME_CB, FOLIMPEXPLASTUSEDDIR );

    SETTEXT( hwndDlg, ID_FOLEXP_FILENAME_CB, pIda->szExportPath );

  } /* endif */


  return( mResult );
} /* end of FolExpPropInit */



// enable state of controls depending on current export mode
void FolPropSetEnableState
(
  HWND        hwndDlg,
  PFOLEXPIDA  pIda
)
{
  if ( pIda->usExportFormat == FOL_TMGR_FORMAT)
  {
    PPROPFOLDERLIST pFllProp = NULL;           // ptr to folder list properties
    pFllProp = (PPROPFOLDERLIST)MakePropPtrFromHnd( pIda->hFllProp );

    ENABLECTRL( hwndDlg, ID_FOLEXP_HISTLOG_CHK, TRUE );
    SETCHECK_FALSE( hwndDlg, ID_FOLEXP_HISTLOG_CHK );

    if ( pIda->fDict )
    {
      ENABLECTRL( hwndDlg, ID_FOLEXP_WITHDICT_CHK, TRUE );
      SETCHECK( hwndDlg, ID_FOLEXP_WITHDICT_CHK, pFllProp->fFolExpWithDict );
    }
    else
    {
      ENABLECTRL( hwndDlg, ID_FOLEXP_WITHDICT_CHK, FALSE );
    } /* endif */

    SETCHECK_FALSE( hwndDlg, ID_FOLEXP_EXPORTAS_CHK );
    ENABLECTRL( hwndDlg, ID_FOLEXP_EXPORTAS_CHK, TRUE );
    ENABLECTRL( hwndDlg, ID_FOLEXP_EXPORTAS_EF, FALSE );

    SETCHECK_FALSE( hwndDlg, ID_FOLEXP_EXPORTSHORT_CHK );
    ENABLECTRL( hwndDlg, ID_FOLEXP_EXPORTSHORT_CHK, TRUE );

    ENABLECTRL( hwndDlg, ID_FOLEXP_WITHMEM_CHK, TRUE );
    SETCHECK( hwndDlg, ID_FOLEXP_WITHMEM_CHK,  pFllProp->fFolExpWithMem );
    if ( pIda->fFolderHasROMem )
    {
      ENABLECTRL( hwndDlg, ID_FOLEXP_WITHROMEM_CHK, TRUE );
      SETCHECK( hwndDlg, ID_FOLEXP_WITHROMEM_CHK, pFllProp->fFolExpWithROMem );
    }
    else
    {
      ENABLECTRL( hwndDlg, ID_FOLEXP_WITHROMEM_CHK, FALSE );
    } /* endif */

    if ( pIda->fDocMem )
    {
      ENABLECTRL( hwndDlg, ID_FOLEXP_WITHDOCMEM_CHK, TRUE );
      SETCHECK( hwndDlg, ID_FOLEXP_WITHDOCMEM_CHK, pFllProp->fFolExpWithDocMem );
    }
    else
    {
      ENABLECTRL( hwndDlg, ID_FOLEXP_WITHDOCMEM_CHK, FALSE );
    } /* endif */

    if ( pIda->pszObjList )
    {
      // enable mem and dict related controls, as which of the folders has TMs and
      // dicts
      ENABLECTRL( hwndDlg, ID_FOLEXP_WITHDICT_CHK, TRUE );
      ENABLECTRL( hwndDlg, ID_FOLEXP_WITHDOCMEM_CHK, TRUE );
      ENABLECTRL( hwndDlg, ID_FOLEXP_WITHROMEM_CHK, TRUE );
      ENABLECTRL( hwndDlg, ID_FOLEXP_WITHMEM_CHK, TRUE );

      ENABLECTRL( hwndDlg, ID_FOLEXP_EXPORTAS_CHK, FALSE );
      ENABLECTRL( hwndDlg, ID_FOLEXP_EXPORTAS_EF, FALSE );
    } /* endif */

    ENABLECTRL( hwndDlg, ID_FOLEXP_NOTE_CHK, TRUE );
    SETCHECK( hwndDlg, ID_FOLEXP_NOTE_CHK,     pFllProp->fFolExpAddNote );
    ENABLECTRL( hwndDlg, ID_FOLEXP_OLDVERS_CHK, TRUE );
    SETCHECK( hwndDlg, ID_FOLEXP_OLDVERS_CHK,  pFllProp->fFolNonUnicode );
    {
      CHAR           chTempName[MAX_EQF_PATH];

      UtlMakeEQFPath( chTempName, pIda->szObjName[0], PROPERTY_PATH, pIda->szFolName );
      strcat( chTempName, BACKSLASH_STR  );
      strcat( chTempName, REDUNDCOUNTFILE );
      BOOL fEnable = UtlFileExist( chTempName ); 
      ENABLECTRL( hwndDlg, ID_FOLEXP_WITH_REDUNDANCY_DATA_CHK, fEnable  );
    }
    //--- check if folder is open, set delete flag accordingly ---
    if ( EqfQueryObject( pIda->szObjName, clsFOLDER, 0) )
    {
      ENABLECTRL( hwndDlg, ID_FOLEXP_DELETE_CHK, FALSE );
    }
    else
    {
      ENABLECTRL( hwndDlg, ID_FOLEXP_DELETE_CHK, TRUE );
      SETCHECK( hwndDlg, ID_FOLEXP_DELETE_CHK,   pFllProp->fFolExpDelFolder );
    } /* endif */

    SETCHECK( hwndDlg, ID_FOLEXP_HISTLOG_CHK,   pFllProp->fFolExpHistLog );
    FolExpPropCommand( hwndDlg, ID_FOLEXP_HISTLOG_CHK, BN_CLICKED );

    ENABLECTRL( hwndDlg, ID_FOLEXP_MASTER_CHK, pIda->fIsMasterFolder  );
  }
  else
  {
    PPROPFOLDERLIST pFllProp = NULL;           // ptr to folder list properties
    pFllProp = (PPROPFOLDERLIST)MakePropPtrFromHnd( pIda->hFllProp );

    SETCHECK_FALSE( hwndDlg, ID_FOLEXP_WITHDICT_CHK );
    ENABLECTRL( hwndDlg, ID_FOLEXP_WITHDICT_CHK, FALSE );
    ENABLECTRL( hwndDlg, ID_FOLEXP_EXPORTAS_EF, FALSE );
    ENABLECTRL( hwndDlg, ID_FOLEXP_EXPORTAS_CHK, FALSE );
    SETCHECK_FALSE( hwndDlg, ID_FOLEXP_EXPORTAS_CHK );
    SETCHECK_FALSE( hwndDlg, ID_FOLEXP_EXPORTSHORT_CHK );
    ENABLECTRL( hwndDlg, ID_FOLEXP_EXPORTSHORT_CHK, FALSE );
    ENABLECTRL( hwndDlg, ID_FOLEXP_WITHMEM_CHK, TRUE );
//  SETCHECK_FALSE( hwndDlg, ID_FOLEXP_WITHMEM_CHK );
    SETCHECK( hwndDlg, ID_FOLEXP_WITHMEM_CHK,  pFllProp->fFolExpWithMem );
//  ENABLECTRL( hwndDlg, ID_FOLEXP_WITHROMEM_CHK, FALSE );
    if ( pIda->fFolderHasROMem )
    {
      ENABLECTRL( hwndDlg, ID_FOLEXP_WITHROMEM_CHK, TRUE );
      SETCHECK( hwndDlg, ID_FOLEXP_WITHROMEM_CHK, pFllProp->fFolExpWithROMem );
    }
    else
    {
      ENABLECTRL( hwndDlg, ID_FOLEXP_WITHROMEM_CHK, FALSE );
    } /* endif */
    ENABLECTRL( hwndDlg, ID_FOLEXP_WITHDOCMEM_CHK, FALSE );
    SETCHECK_FALSE( hwndDlg, ID_FOLEXP_WITHDOCMEM_CHK );
    SETCHECK_FALSE( hwndDlg, ID_FOLEXP_NOTE_CHK );
    ENABLECTRL( hwndDlg, ID_FOLEXP_NOTE_CHK, FALSE );
    ENABLECTRL( hwndDlg, ID_FOLEXP_OLDVERS_CHK, FALSE );
    SETCHECK_FALSE( hwndDlg, ID_FOLEXP_OLDVERS_CHK );
    ENABLECTRL( hwndDlg, ID_FOLEXP_DELETE_CHK, FALSE );
    SETCHECK_FALSE( hwndDlg, ID_FOLEXP_HISTLOG_CHK );
    ENABLECTRL( hwndDlg, ID_FOLEXP_HISTLOG_CHK, FALSE );
    ENABLECTRL( hwndDlg, ID_FOLEXP_MASTER_CHK, FALSE );
    ENABLECTRL( hwndDlg, ID_FOLEXP_WITH_REDUNDANCY_DATA_CHK, FALSE );

  } /* endif */
}



//+----------------------------------------------------------------------------+
//Internal function
//+----------------------------------------------------------------------------+
//Function name:     FolPropExpInitOptions
//+----------------------------------------------------------------------------+



MRESULT FolPropExpInitOptions
(
HWND    hwndDlg,                    // handle of folder export dialog window
WPARAM  mp1,                        // first parameter of WM_INITDLG
LPARAM  mp2                         // second parameter of WM_INITDLG
)
{
  MRESULT     mResult = MRFROMSHORT(TRUE); // result of message processing
  PFOLEXPIDA  pIda;                   // ptr to folder export IDA
  PPROPFOLDERLIST pFllProp = NULL;           // ptr to folder list properties

  mp1 = mp1;                         // suppress 'unreferenced parameter' msg

  //--- adress IDA and store pointer to it ---
  pIda = (PFOLEXPIDA) mp2;
  if ( pIda )
  {
    ANCHORDLGIDA( hwndDlg, pIda );
  } /* endif */


  if ( pIda )
  {
    pIda->fHistlog = FALSE;
    pFllProp = (PPROPFOLDERLIST)MakePropPtrFromHnd( pIda->hFllProp );
    pIda->usExportFormat = pFllProp->usFolExpFormat;
    FolPropSetEnableState( hwndDlg, pIda );

    //Originator: name and e-mail
    SETTEXT( hwndDlg, ID_FOLEXP_ORIGINATOR_EF, pFllProp->szExpOriginator );
    SETTEXT( hwndDlg, ID_FOLEXP_ORIGMAIL_EF, pFllProp->szExpEMail );
    SETFOCUS( hwndDlg, ID_FOLEXP_HISTLOG_CHK );
  } /* endif */

  UtlCheckDlgPos( hwndDlg, FALSE );
  return( mResult );
} /* end of FolExpInit */

//+----------------------------------------------------------------------------+
//Internal function
//+----------------------------------------------------------------------------+
//Function name:     FolExpPropertySheetNotification
//+----------------------------------------------------------------------------+
//Function call:     FlxpPropertySheetNotifiaction( hwnd, mp1, mp2);
//+----------------------------------------------------------------------------+
//Description:       handle changes on the tab page
//+----------------------------------------------------------------------------+
//Parameters:        HWND hwnd   handle of the dialog
//                   WPARAM  mp1    message parameter 1
//                   LPARAM  mp2    message parameter 2
//+----------------------------------------------------------------------------+
//Returncode type:   MRESULT
//+----------------------------------------------------------------------------+
//Returncodes:       return code from default window proc or FALSE
//+----------------------------------------------------------------------------+
//Function flow:     switch ( pNMHdr->code )
//                     case TCN_SELCHANGE:
//                       activate new page
//                     case TCN_SELCHANGING
//                       hide the dialog
//                   return
//+----------------------------------------------------------------------------+
MRESULT FolExpPropertySheetNotification
(
HWND hwnd,
WPARAM  mp1,
LPARAM  mp2
)
{
  NMHDR * pNMHdr;
  ULONG        ulTabCtrl;
  MRESULT      mResult = FALSE;
  PFOLEXPIDA     pIda;
  pNMHdr = (LPNMHDR)mp2;

  mp1;
  switch ( pNMHdr->code )
  {
    case TCN_SELCHANGE:
      pIda = ACCESSDLGIDA(hwnd, PFOLEXPIDA);
      if ( pIda )
      {
        TC_ITEM Item;
        HWND hwndTabCtrl = GetDlgItem( hwnd, ID_FOLEXP_PROP_TABCTRL );
        ulTabCtrl = TabCtrl_GetCurSel( hwndTabCtrl );
        memset( &Item, 0, sizeof(Item) );
        Item.mask = TCIF_PARAM;
        TabCtrl_GetItem( hwndTabCtrl, ulTabCtrl, &Item );
        ulTabCtrl = Item.lParam;
        ShowWindow( pIda->hwndPages[ ulTabCtrl ], SW_SHOW );
      } /* endif */
      break;
    case TCN_SELCHANGING:
      pIda = ACCESSDLGIDA( hwnd, PFOLEXPIDA );
      if ( pIda )
      {
        /**************************************************************/
        /* Issue a direct call to the appropriate dialog proc with    */
        /* WM_COMMAND, ID_TB_PROP_SET_PB and the second parameter set */
        /* to 1L to force only consistency checking                   */
        /**************************************************************/
        TC_ITEM Item;
        PFNWP pfnWp;
        HWND hwndTabCtrl = GetDlgItem( hwnd, ID_FOLEXP_PROP_TABCTRL );
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
          HWND hwndTabCtrl = GetDlgItem( hwnd, ID_FOLEXP_PROP_TABCTRL );
          memset( &Item, 0, sizeof(Item) );
          Item.mask = TCIF_PARAM;
          TabCtrl_GetItem( hwndTabCtrl, pToolTipText->hdr.idFrom, &Item );
          switch ( (SHORT)Item.lParam )
          {
            case 0:      // first page
              LOADSTRING( hab, hResMod, IDS_FOLEXPPROP_TAB_GENERAL,
                          pToolTipText->szText );
              break;
            case 1:      // second page
              LOADSTRING( hab, hResMod, IDS_FOLEXPPROP_TAB_GENERAL,
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
} /* end of function FolExpPropertySheetNotification */






//+----------------------------------------------------------------------------+
//Internal function
//+----------------------------------------------------------------------------+
//Function name:     FolExpPropertySheetLoad
//+----------------------------------------------------------------------------+
//Function call:     CntPropertySheetLoad( hwnd, mp2 );
//+----------------------------------------------------------------------------+
//Description:       handle changes on the tab page
//+----------------------------------------------------------------------------+
//Parameters:        HWND hwnd   handle of the dialog
//                   LPARAM  mp2    message parameter 2
//+----------------------------------------------------------------------------+
//Returncode type:   MRESULT
//+----------------------------------------------------------------------------+
//Returncodes:       return code from default window proc or FALSE
//+----------------------------------------------------------------------------+
//Function flow:     create any pages,
//                   load the tabctrl text
//                   load the (modeless) dialog, register it and position into
//                     tab area
//                   return
//+----------------------------------------------------------------------------+
BOOL FolExpPropertySheetLoad
(
HWND hwnd,
PFOLEXPIDA     pIda
)
{
  BOOL      fOk = TRUE;
  TC_ITEM   TabCtrlItem;
  USHORT    nItem = 0;
  HWND      hwndTabCtrl;
  HINSTANCE hInst;
  CHAR      szBuffer[80];

  if ( fOk )
  {
    RECT rect;
    HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);

    // remember adress of user area
    hInst = GETINSTANCE( hwnd );
    hwndTabCtrl = GetDlgItem( hwnd, ID_FOLEXP_PROP_TABCTRL );
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
    // IDS_FOLEXPPROP_TAB_GENERAL
    //

    LOADSTRING( hab, hResMod, IDS_FOLEXPPROP_TAB_GENERAL , szBuffer );
    TabCtrlItem.pszText = szBuffer;
    TabCtrlItem.lParam = nItem;
    SendMessage(hwndTabCtrl, TCM_INSERTITEM, nItem, (LPARAM)&TabCtrlItem);
    pIda->hwndPages[nItem] =
    CreateDialogParam( hInst,
                       MAKEINTRESOURCE( ID_FOLEXPPROP_GENERAL_DLG ),
                       hwnd,
                       FOLEXPPROP_GENERAL_DLGPROC,
                       (LPARAM)pIda );

    SetWindowPos( pIda->hwndPages[nItem], HWND_TOP,
                  rect.left, rect.top,
                  rect.right-rect.left, rect.bottom-rect.top, SWP_SHOWWINDOW );
    SetFocus( pIda->hwndPages[nItem] );
    UtlRegisterModelessDlg( pIda->hwndPages[nItem] );
    nItem++;

    //
    // IDS_FOLEXPPROP_TAB_OPTIONS
    //
    LOADSTRING( hab, hResMod, IDS_FOLEXPPROP_TAB_OPTIONS , szBuffer );
    TabCtrlItem.pszText = szBuffer;
    TabCtrlItem.lParam = nItem;
    SendMessage(hwndTabCtrl, TCM_INSERTITEM, nItem, (LPARAM)&TabCtrlItem);
    pIda->hwndPages[nItem] =
    CreateDialogParam( hInst,
                       MAKEINTRESOURCE( ID_FOLEXPPROP_OPTIONS_DLG ),
                       hwnd,
                       FOLEXPPROP_OPTIONS_DLGPROC,
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




//+----------------------------------------------------------------------------+
//Internal function
//+----------------------------------------------------------------------------+
//Function name:     FolExpPropCommand
//+----------------------------------------------------------------------------+
//Function call:
//+----------------------------------------------------------------------------+
//Description:       Handle WM_COMMAND message of property sheet dialog
//+----------------------------------------------------------------------------+
//Parameters:        HWND hwnd   handle of the dialog
//                   WPARAM  mp1    message parameter 1
//                   LPARAM  mp2    message parameter 2
//+----------------------------------------------------------------------------+
//Returncode type:   MRESULT
//+----------------------------------------------------------------------------+
//Returncodes:       return code from default window proc or FALSE
//+----------------------------------------------------------------------------+
MRESULT FolExpPropCommand
(
HWND hwnd,
WPARAM mp1,
LPARAM mp2
)
{
  BOOL fOk = TRUE;
  INT     nItem;
  MRESULT mResult = MRFROMSHORT(TRUE);
  PFOLEXPIDA pIda;                    // ptr to folder export IDA
  EQFINFO     ErrorInfo;              // error code of property handler calls
  BOOL        fOK = TRUE;                    // internal OK flag
  PPROPFOLDERLIST pFllProp;           // ptr to folderlist props
  PPROPFOLDER pFolProp;               // ptr to folder properties
  PVOID       hFolProp;

  mp2;
// --- get IDA pointer ---
  pIda = ACCESSDLGIDA( hwnd, PFOLEXPIDA );

  switch ( WMCOMMANDID( mp1, mp2 ) )
  {
   case ID_FOLEXP_HELP_PB:
      UtlInvokeHelp();
      break;
    case ID_FOLEXP_EXPORT_PB:

      // get currently selected output format
      {
        int item = CBQUERYSELECTION( hwnd, ID_FOLEXP_FORMAT_CB );
        if ( item != -1 )
        {
          pIda->usExportFormat = (USHORT)CBQUERYITEMHANDLE( hwnd, ID_FOLEXP_FORMAT_CB, item );
        }
        else
        {
          pIda->usExportFormat = FOL_TMGR_FORMAT;
        } /* endif */
      }

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
          case 1:
            fOk =  !pfnWp( pIda->hwndPages[nItem], WM_COMMAND,
                           PID_PB_OK, 0L);
            break;



        } /* endswitch */
        nItem++;
      } /* endwhile */


      {
        PFNWP pfnWp = (PFNWP) GetWindowLong( pIda->hwndPages[ 0 ],
                                             DWL_DLGPROC );
        fOk =  !pfnWp( pIda->hwndPages[0], WM_COMMAND,
                       ID_FOLEXP_EXPORT_PB, 0L);

      }



      fOK = TRUE;                         // assume everything is o.k.
      pIda->hatomInError = NULLHANDLE;    // no atom table yet


      //--- if o.k. start export of folder ---
      if ( fOK && pIda->fOk)
      {

        //--- save last used values in folder list properties---
        if ( SetPropAccess( pIda->hFllProp, PROP_ACCESS_WRITE) )
        {
          pFllProp = (PPROPFOLDERLIST)MakePropPtrFromHnd( pIda->hFllProp);
          pFllProp->chFolExpDrive = pIda->chDrive;
          pFllProp->fExportPath = TRUE;
          strcpy(pFllProp->szExportPath, pIda->szExportPath);
          pFllProp->fFolExpWithDict = pIda->pHeader->fContainsDict;
          pFllProp->fUseShortFolderName = (EQF_BOOL)pIda->fUseShortFolderName;
          pFllProp->fFolExpWithMem  = (EQF_BOOL)pIda->fWithFolderMem;
          pFllProp->fFolExpWithROMem= (EQF_BOOL)pIda->fWithROMem;
          pFllProp->fFolExpWithDocMem= (EQF_BOOL)pIda->fWithDocMem;
          pFllProp->fFolExpAddNote  = pIda->pHeader->BitFlags.fWithNote;

          pFllProp->fFolExpDelFolder= pIda->pHeader->BitFlags.fDeleted;

//          pFllProp->fFolExpOldVers = QUERYCHECK( hwnd,
//                                                 ID_FOLEXP_OLDVERS_CHK );
          pFllProp->fFolNonUnicode = QUERYCHECK( hwnd,
                                                 ID_FOLEXP_OLDVERS_CHK );

          pFllProp->fFolExpHistLog = QUERYCHECK( hwnd,
                                                 ID_FOLEXP_HISTLOG_CHK );

          strcpy(pFllProp->szExpOriginator , pIda->szExpOriginator );
          strcpy(pFllProp->szExpEMail      , pIda->szExpEMail) ;
          pFllProp->usFolExpFormat = pIda->usExportFormat;

          SaveProperties( pIda->hFllProp, &ErrorInfo);
        } /* endif */




        //--- save last used values in folder list properties---
        hFolProp = OpenProperties( pIda->szObjPath,
                                   NULL,
                                   PROP_ACCESS_READ,
                                   &ErrorInfo);
        if ( hFolProp )
        {

          if ( SetPropAccess(hFolProp, PROP_ACCESS_WRITE) )
          {


            pFolProp = (PPROPFOLDER)MakePropPtrFromHnd( hFolProp );

            strcpy(pFolProp->szExpOriginator , pIda->szExpOriginator );
            strcpy(pFolProp->szExpEMail      , pIda->szExpEMail) ;

            SaveProperties(hFolProp, &ErrorInfo);
            CloseProperties( hFolProp, PROP_QUIT, &ErrorInfo );
          }//end if
        }//end if hFolProp

        POSTCLOSE( hwnd, TRUE );
      }
      else
      {
        UtlPackCleanup( pIda->pPackCtrl );
        pIda->pPackCtrl = NULL;

        // call general folder export cleanup
        FolExpCleanup( pIda );
      } /* endif */


      break;

    case ID_FOLEXP_FORMAT_CB:
      if ( WMCOMMANDCMD( mp1,mp2 ) == CBN_SELCHANGE )
      {
        int item = CBQUERYSELECTION( hwnd, ID_FOLEXP_FORMAT_CB );
        if ( item != -1 )
        {
          pIda->usExportFormat = (USHORT)CBQUERYITEMHANDLE( hwnd, ID_FOLEXP_FORMAT_CB, item );
          FolPropSetEnableState( pIda->hwndPages[1], pIda );
        } /* endif */
      } /* endif */
      break;


    case ID_FOLEXP_CANCEL_PB:
    case DID_CANCEL:
      POSTEQFCLOSE( hwnd, FALSE );
      break;

    default:
      mResult = WinDefDlgProc( hwnd, WM_COMMAND, mp1, mp2 );
      break;
  } /* endswitch */

  return( mResult );
} /* end of FolExpPropCommand */




//+----------------------------------------------------------------------------+
//Internal function FOLEXPPROPDLGPROC
//+----------------------------------------------------------------------------+
//Function name:
//+----------------------------------------------------------------------------+
//Function call:
//+----------------------------------------------------------------------------+
//Description:
//+----------------------------------------------------------------------------+
//Parameters:
//
//+----------------------------------------------------------------------------+
//Returncode type:
//+----------------------------------------------------------------------------+
//Returncodes:
//+----------------------------------------------------------------------------+
//Function flow:
//
//
//
//
//+----------------------------------------------------------------------------+

INT_PTR CALLBACK FOLEXPPROPDLGPROC
(
HWND hwnd,                       /* handle of dialog window             */
WINMSG msg,
WPARAM mp1,
LPARAM mp2
)
{
  MRESULT  mResult = MRFROMSHORT( FALSE );                // result value of procedure
  PFOLEXPIDA pIda;                      // dialog instance data area
  BOOL     fOk = TRUE;                  // internal O.K. flag

  switch ( msg )
  {
    case WM_EQF_QUERYID: HANDLEQUERYID( ID_FOLEXPPROP_DLG, mp2 ); break;

    case WM_INITDLG:


      // get the export IDA
      pIda = (PFOLEXPIDA) mp2;

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
        if ( pIda->pszObjList )
        {
          // change titlebar text
          SendMessage( hwnd, WM_SETTEXT, 0, (LPARAM)"Export multiple folders" );
        } /* endif */
        FolExpPropertySheetLoad( hwnd, pIda );
      } /* endif */

      // fill format combobox and select last used format
      {
        PPROPFOLDERLIST pFllProp = (PPROPFOLDERLIST)MakePropPtrFromHnd( pIda->hFllProp );
        FolFillFolderFormatCombobox( hwnd, ID_FOLEXP_FORMAT_CB, pFllProp->usFolExpFormat );
      }

      mResult = DIALOGINITRETURN( mResult );
      break;
    case WM_COMMAND:
      mResult = FolExpPropCommand( hwnd, mp1, mp2 );
      break;

    case WM_NOTIFY:

      mResult = FolExpPropertySheetNotification( hwnd, mp1, mp2 );
      break;

    case WM_HELP:
      /*************************************************************/
      /* pass on a HELP_WM_HELP request                            */
      /*************************************************************/
//         EqfDisplayContextHelp( ((LPHELPINFO) mp2)->hItemHandle,
//                                &hlpsubtblRptPropDlg[0] );
      mResult = TRUE;  // message processed
      break;

    case WM_CLOSE:
//-----------------------------------/
      mResult = FolExpClose( hwnd, mp1, mp2 );
      break;



    case WM_EQF_CLOSE :
      // mp1 : TRUE -> start Counting Report window
      // mp2 : not used

      pIda = ACCESSDLGIDA (hwnd, PFOLEXPIDA);  // get access to IDA

      mResult = FolExpClose( hwnd, mp1, mp2 );

      if (pIda)
      {
        ANCHORDLGIDA (hwnd, NULL);
      } // end if

      // destroy dialog pass flag in mp1, to tell the count handler if
      // count instance should be started or not
      // mp1 == FALSE : do not start count instance
      // mp1 == TRUE  : start count instance
      WinDismissDlg (hwnd, SHORT1FROMMP1(mp1));
      break;


    default:
      mResult = WinDefDlgProc( hwnd, msg, mp1, mp2 );
      break;
  } /* endswitch */

  return mResult;
} /* end of FOLEXPPROPDLGPROC */


//+----------------------------------------------------------------------------+
//Internal function  FOLEXPPROP_GENERAL_DLGPROC
//+----------------------------------------------------------------------------+
INT_PTR CALLBACK FOLEXPPROP_GENERAL_DLGPROC
(
HWND hwnd,                       /* handle of dialog window             */
WINMSG msg,
WPARAM mp1,
LPARAM mp2
)
{
  MRESULT  mResult = MRFROMSHORT( FALSE );                // result value of procedure
  PFOLEXPIDA  pIda;
  BOOL    fOk=TRUE;
  SHORT sNotification= WMCOMMANDCMD( mp1, mp2 );
  SHORT sId = WMCOMMANDID( mp1, mp2 );
  switch ( msg )
  {
    case WM_INITDLG:
      //-----------------------------------
      {
        mResult = FolPropExpInit( hwnd, mp1, mp2 );

      }
      break;

    case WM_COMMAND:
      //-----------------------------------
      switch ( WMCOMMANDID( mp1, mp2 ) )
      {



        case ID_FOLEXP_DRIVEDUMMY_PB:
          {

            BROWSEINFO bi;
            LPITEMIDLIST pidlBrowse;    // PIDL selected by user


            pIda = ACCESSDLGIDA( hwnd, PFOLEXPIDA );

            // Fill in the BROWSEINFO structure.
            bi.hwndOwner = hwnd;
            bi.pidlRoot = NULL;


            bi.pszDisplayName = pIda->szExportPath;
            //bi.pszDisplayName[0] = pIda->chFromDrive;
            bi.lpszTitle = "Select an export target directory";
            bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_USENEWUI;

            // if UtlBrowseForFolderCallbackProc is specified as callback the lParam
            // parameter must contain the initial folder directory!!!
            bi.lpfn = UtlBrowseForFolderCallbackProc;
            bi.lParam = (LPARAM)pIda->szExportPath;

            // Browse for a folder and return its PIDL.
            pidlBrowse = SHBrowseForFolder(&bi);
            if (pidlBrowse != NULL)
            {
              // get the selected directory path
              SHGetPathFromIDList( pidlBrowse, pIda->szExportPath );

              // SET TO eqf/export temp
              // GS TO DO
              //pIda->szString[1] = EOS;
              //strcat(pIda->szString,":\\EQF\\EXPORT");

              // activate selected path
              SETTEXT( hwnd, ID_FOLEXP_FILENAME_CB, pIda->szExportPath );
              //DocImpHandlePathInput( hwnd );

              // Free the PIDL returned by SHBrowseForFolder.
              {
                LPMALLOC pMalloc;
                SHGetMalloc( &pMalloc );
#ifdef __cplusplus
                pMalloc->Free(pidlBrowse );
#else
                pMalloc->lpVtbl->Free(pMalloc, pidlBrowse );
#endif
              }
            } /* endif */

            //--- refresh packaged folders listbox ---
            //GS TO DO

            if (pIda->szExportPath[0] != EOS)
            {
              pIda->chDrive =  pIda->szExportPath[0];
            }// end if
          }

          break;
        case ID_FOLEXP_HISTLOG_CHK:
          pIda = ACCESSDLGIDA( hwnd, PFOLEXPIDA );

          if (pIda->fHistlog)
          {
            ENABLECTRL( hwnd, ID_FOLEXP_SELDOCS_CHK, FALSE );
            ENABLECTRL( hwnd, ID_FOLEXP_TRANSLATOR_CHK, FALSE );
            ENABLECTRL( hwnd, ID_FOLEXP_TRANSLATOR_CB , FALSE );
            ENABLECTRL( hwnd, ID_FOLEXP_SELALL_PB, FALSE );
            ENABLECTRL( hwnd, ID_FOLEXP_DESELALL_PB, FALSE );
            ENABLECTRL( hwnd, ID_FOLEXP_DOCS_LB, FALSE );
          }
          else
          {
            ENABLECTRL( hwnd, ID_FOLEXP_SELDOCS_CHK, TRUE );
            ENABLECTRL( hwnd, ID_FOLEXP_TRANSLATOR_CHK, TRUE );
            ENABLECTRL( hwnd, ID_FOLEXP_TRANSLATOR_CB , TRUE );
            if ( QUERYCHECK( hwnd, ID_FOLEXP_SELDOCS_CHK) )
            {
              ENABLECTRL( hwnd, ID_FOLEXP_SELALL_PB, TRUE );
              ENABLECTRL( hwnd, ID_FOLEXP_DESELALL_PB, TRUE );
            } /* endif */
            ENABLECTRL( hwnd, ID_FOLEXP_DOCS_LB, TRUE );
          } /* endif */



          break;


        case ID_FOLEXP_EXPORT_PB:
          {
            HWND hwndTempDocLB = NULLHANDLE;
            pIda = ACCESSDLGIDA( hwnd, PFOLEXPIDA );
            // get selected Drive
            // pIda->chDrive

            QUERYTEXT( hwnd, ID_FOLEXP_FILENAME_CB, pIda->szExportPath );
            UtlSaveLastUsedString( hwnd, ID_FOLEXP_FILENAME_CB, FOLIMPEXPLASTUSEDDIR, 10 );

            // get Vendor name
            QUERYTEXT( hwnd, ID_FOLEXP_TRANSLATOR_CB, pIda->szVendor );
            ANSITOOEM( pIda->szVendor );  // vendor names in properties are in ASCII

            // fill the selected names of the document tree view in a temporary listbox
            // to allow the normal usage of function FolPrepFolderExport
            {
              HTREEITEM hItem;
              HWND      hwndTV = GetDlgItem( hwnd, ID_FOLEXP_DOCS_LB );
              hwndTempDocLB = WinCreateWindow( hwnd, WC_LISTBOX, "", LBS_MULTIPLESEL,
                                               0, 0, 0, 0,
                                               hwnd, HWND_TOP, 1, NULL, NULL );


              hItem = TreeView_GetRoot( hwndTV );

              while ( hItem != NULL )
              {
                FolExpSelCheckedTreeItems( hwndTV, hItem, pIda->hwndLongNameLB,
                                           pIda->pSubFolInfo, pIda->szBuffer, FALSE );
                hItem = TreeView_GetNextSibling( hwndTV, hItem );
              } /* endwhile */
            }

            /*************************************************************/
            /* Check the data and prepare export of folder               */
            /*************************************************************/
            if ( FolPrepFolderExport( pIda,
                                       NULL,
                                       hwnd ) )
            {
              fOk = FALSE;                // an error occured
              pIda->fOk = FALSE;
            }
            else
            {
              fOk = TRUE;
              pIda->fOk = TRUE;
            } /* endif */
          }
          break;

        case PID_PB_OK :
          {
            pIda = ACCESSDLGIDA( hwnd, PFOLEXPIDA );
            pIda->pHeader->BitFlags.fSelectedDocs = QUERYCHECK( hwnd, ID_FOLEXP_SELDOCS_CHK);
            pIda->fTranslatorSelected = QUERYCHECK( hwnd, ID_FOLEXP_TRANSLATOR_CHK);
          }
          break;

        case ID_FOLEXP_SELALL_PB:
        case ID_FOLEXP_DESELALL_PB:
          ENABLEUPDATE_FALSE( hwnd, ID_FOLEXP_DOCS_LB );
            {
              // set/clear selection state of all tree items
              HWND hwndTV = GetDlgItem( hwnd, ID_FOLEXP_DOCS_LB );
              HTREEITEM hCurrent = TreeView_GetRoot( hwndTV );

              while ( hCurrent != NULL )
              {
                FolExpSetTreeCheck( hwndTV, hCurrent, sId == ID_FOLEXP_SELALL_PB );
                hCurrent = TreeView_GetNextSibling( hwndTV, hCurrent );
              } /* endwhile */
            }
          ENABLEUPDATE_TRUE( hwnd, ID_FOLEXP_DOCS_LB );
          break;

        case ID_FOLEXP_TRANSLATOR_CHK:
          //----------------------------
          pIda = ACCESSDLGIDA( hwnd, PFOLEXPIDA );
          switch ( sNotification )
          {
            case BN_CLICKED:
              if ( QUERYCHECK( hwnd, ID_FOLEXP_TRANSLATOR_CHK) )
              {

                // deselect translator
//                SETCHECK( hwnd,ID_FOLEXP_SELDOCS_CHK , FALSE );
//                ENABLECTRL( hwnd, ID_FOLEXP_SELALL_PB, FALSE );
//                ENABLECTRL( hwnd, ID_FOLEXP_DESELALL_PB, FALSE );
//                SHOWCONTROL( hwnd, ID_FOLEXP_HIDELB_TEXT );
//                DELETEALL( hwnd, ID_FOLEXP_DOCS_LB );
//                ENABLECTRL( hwnd, ID_FOLEXP_DOCS_LB, FALSE );

                // Translator names
                ENABLECTRL( hwnd, ID_FOLEXP_TRANSLATOR_CB, TRUE );

                {
                  HWND hwndLB;

                  hwndLB = GetDlgItem(hwnd, ID_FOLEXP_DOCS_LB);
                  UtlSetHorzScrollingForLB(hwndLB);

                }

                //
                // Build Translator List
                //

                CBDELETEALL( hwnd, ID_FOLEXP_TRANSLATOR_CB);

                // get long names of document to the select listbox
                {
                  SHORT sIndex, sInsertIndex;
                  SHORT sNum = QUERYITEMCOUNTHWND( pIda->hwndTranslatorLB );
                  for ( sIndex = 0; sIndex < sNum; sIndex++ )
                  {
                    //get the translator name from the source listbox

                    QUERYITEMTEXTHWND( pIda->hwndTranslatorLB, sIndex, pIda->szLongName );

                    //  add item text to target listbox

                    if (pIda->szLongName[0]!=EOS)
                    {
                      sInsertIndex = CBINSERTITEM( hwnd, ID_FOLEXP_TRANSLATOR_CB, pIda->szLongName );
                    }

                  } /* endfor */
                }
                CBSELECTITEM (hwnd,ID_FOLEXP_TRANSLATOR_CB , 0);
              }
              else
              {
                ENABLECTRL( hwnd, ID_FOLEXP_TRANSLATOR_CB, FALSE );
              }// end if
              break ;
          } // end switch
          break;

        case ID_FOLEXP_SELDOCS_CHK:
          //----------------------------
          pIda = ACCESSDLGIDA( hwnd, PFOLEXPIDA );
          switch ( sNotification )
          {
            case BN_CLICKED:
              if ( QUERYCHECK( hwnd, ID_FOLEXP_SELDOCS_CHK) )
              {
                // deselect translator
//                SETCHECK( hwnd, ID_FOLEXP_TRANSLATOR_CHK, FALSE );
//                ENABLECTRL( hwnd, ID_FOLEXP_TRANSLATOR_CB, FALSE );
                ENABLECTRL( hwnd, ID_FOLEXP_SELALL_PB, TRUE );
                ENABLECTRL( hwnd, ID_FOLEXP_DESELALL_PB, TRUE );
                ENABLECTRL( hwnd, ID_FOLEXP_DOCS_LB, TRUE );
                HIDECONTROL( hwnd, ID_FOLEXP_HIDELB_TEXT );

//                CBDELETEALL( hwnd, ID_FOLEXP_TRANSLATOR_CB);
                // delete all items in tree view control
                SendDlgItemMessage( hwnd, ID_FOLEXP_DOCS_LB, TVM_DELETEITEM,
                                    0, (LPARAM)(HTREEITEM)TVI_ROOT );

                // clear all subfolder flags and handles
                {
                  int i = 0;
                  while ( pIda->pSubFolInfo[i].szName[0] != EOS )
                  {
                    pIda->pSubFolInfo[i].ulValue = 0L;
                    pIda->pSubFolInfo[i].fFlag   = FALSE;
                    i++;
                  } /* enwhile */
                }

                // insert subfolders and remember subfolder item handles in
                // subfolder info table
                {
                  TVINSERTSTRUCT stInsert;
                  int i = 0;
                  BOOL fRedo = FALSE;
                  HWND hwndTV = GetDlgItem( hwnd, ID_FOLEXP_DOCS_LB );

                  while ( pIda->pSubFolInfo[i].szName[0] != EOS )
                  {
                    BOOL fInsertItem = FALSE;
                    HTREEITEM hitemParent = NULL;

                    if ( pIda->pSubFolInfo[i].ulValue != 0L )
                    {
                      // ignore subfolder, it has been added already
                      fInsertItem = FALSE;
                    }
                    else
                    {
                      // look if parent of subfolder has been added already
                      if ( pIda->pSubFolInfo[i].ulParentFolder != 0L )
                      {
                        int j = 0;
                        while ( pIda->pSubFolInfo[j].szName[0] != EOS )
                        {
                          if ( pIda->pSubFolInfo[i].ulParentFolder ==
                               pIda->pSubFolInfo[j].ulID )
                          {
                            if ( pIda->pSubFolInfo[j].ulValue != 0L )
                            {
                              fInsertItem = TRUE;
                              hitemParent = (HTREEITEM)pIda->pSubFolInfo[j].ulValue ;
                            }
                            else
                            {
                              fRedo = TRUE;      // we have to redo the subfolder list
                            } /* endif */
                            break;
                          } /* endif */
                          j++;
                        } /* endwhile */
                      }
                      else
                      {
                        fInsertItem = TRUE;
                        hitemParent = TVI_ROOT;
                      } /* endif */
                    } /* endif */

                    // insert item if parent is available
                    if ( fInsertItem )
                    {
                      memset( &stInsert, 0, sizeof(stInsert) );
                      stInsert.hParent = hitemParent;
                      stInsert.hInsertAfter = TVI_SORT;
                      stInsert.item.mask = TVIF_PARAM | TVIF_TEXT;
                      stInsert.item.pszText = pIda->pSubFolInfo[i].szName;
                      stInsert.item.cchTextMax = sizeof(pIda->pSubFolInfo[i].szName);
                      stInsert.item.lParam = SUBFOLDERFLAG | pIda->pSubFolInfo[i].ulID;
                      pIda->pSubFolInfo[i].ulValue = (ULONG)TreeView_InsertItem( hwndTV, &stInsert );

                      // test test test: expand parent item
                      TreeView_Expand( hwndTV, hitemParent, TVE_EXPAND );

                    } /* endif */

                    // continue with next subfolder
                    i++;
                    if ( fRedo && (pIda->pSubFolInfo[i].szName[0] == EOS) )
                    {
                      i = 0;           // restart list
                      fRedo = FALSE;
                    } /* endif */
                  } /* endwhile */
                }
                // copy long names of document to the select listbox
                {
                  SHORT sIndex, sItemHandle;
                  SHORT sNum = QUERYITEMCOUNTHWND( pIda->hwndLongNameLB );
                  for ( sIndex = 0; sIndex < sNum; sIndex++ )
                  {
                    //get the long name from the source listbox
                    QUERYITEMTEXTHWND( pIda->hwndLongNameLB, sIndex, pIda->szLongName );

                    // get the item handle of this item
                    sItemHandle = (SHORT)QUERYITEMHANDLEHWND( pIda->hwndLongNameLB, sIndex );
					{
					  TVINSERTSTRUCT stInsert;
					  ULONG ulParent = 0L;

					  // clear insert structure
					  memset( &stInsert, 0, sizeof(stInsert) );

					  // get parent ID
					  {
						PPROPDOCUMENT pPropDoc = NULL;
						ULONG ulRead;

						QUERYITEMTEXTHWND( pIda->hwndDocLB, sItemHandle, pIda->szDocName );
						UtlMakeEQFPath( pIda->szDocObjName, pIda->szObjName[0], PROPERTY_PATH,
										pIda->szFolName );
						strcat( pIda->szDocObjName, BACKSLASH_STR );
						strcat( pIda->szDocObjName, pIda->szDocName );


						if ( UtlLoadFileL( pIda->szDocObjName, (PVOID *)&pPropDoc, &ulRead, TRUE, FALSE ) )
						{
						  ulParent = pPropDoc->ulParentFolder;
						} /* endif */

						if ( pPropDoc ) UtlAlloc( (PVOID *)&pPropDoc, 0L, 0L, NOMSG );
					  }

					  // set default parent handle
					  stInsert.hParent = TVI_ROOT;

					  // lookup parent handle in subfolder info table
					  if ( ulParent != 0L )
					  {
						int i = 0;
						while ( pIda->pSubFolInfo[i].szName[0] != EOS )
						{
						  if ( pIda->pSubFolInfo[i].ulID == ulParent )
						  {
							stInsert.hParent = (HTREEITEM)pIda->pSubFolInfo[i].ulValue;
							break;
						  } /* endif */
						  i++;
						} /* endwhile */
					  } /* endif */

					  stInsert.hInsertAfter = TVI_SORT;
					  stInsert.item.mask = TVIF_PARAM | TVIF_TEXT;
					  stInsert.item.pszText = pIda->szLongName;
					  stInsert.item.cchTextMax = sizeof(pIda->szLongName);
					  stInsert.item.lParam = sItemHandle;
					  SendDlgItemMessage( hwnd, ID_FOLEXP_DOCS_LB, TVM_INSERTITEM, 0,
										  (LPARAM)&stInsert );
					}
                  } /* endfor */
                }
            // nothing to do here
              }
              else
              {
                ENABLECTRL( hwnd, ID_FOLEXP_SELALL_PB, FALSE );
                ENABLECTRL( hwnd, ID_FOLEXP_DESELALL_PB, FALSE );
                SHOWCONTROL( hwnd, ID_FOLEXP_HIDELB_TEXT );
                SendDlgItemMessage( hwnd, ID_FOLEXP_DOCS_LB, TVM_DELETEITEM,
                                    0, (LPARAM)(HTREEITEM)TVI_ROOT );

              } /* endif */
              break;
          } /* endswitch */
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
      DelCtrlFont(hwnd, ID_FOLEXP_FILENAME_CB );
      mResult = WinDefDlgProc( hwnd, msg, mp1, mp2 );
      break;

    default:
      mResult = WinDefDlgProc( hwnd, msg, mp1, mp2 );
      break;
  } /* endswitch */

  return mResult;
};

//+----------------------------------------------------------------------------+
//Internal function  FOLEXPPROP_OPTIONS_DLGPROC
//+----------------------------------------------------------------------------+
//Function name:
//+----------------------------------------------------------------------------+
//Function call:
//+----------------------------------------------------------------------------+
//Description:
//+----------------------------------------------------------------------------+
//Parameters:
//
//+----------------------------------------------------------------------------+
//Returncode type:
//+----------------------------------------------------------------------------+
//Returncodes:
//+----------------------------------------------------------------------------+
//Function flow:
//
//
//
//
//+----------------------------------------------------------------------------+

INT_PTR CALLBACK FOLEXPPROP_OPTIONS_DLGPROC
(
HWND hwnd,                       /* handle of dialog window             */
WINMSG msg,
WPARAM mp1,
LPARAM mp2
)
{
  MRESULT  mResult = MRFROMSHORT( FALSE );                // result value of procedure
  PFOLEXPIDA  pIda;
  BOOL    fOk = TRUE;
  SHORT sNotification= WMCOMMANDCMD( mp1, mp2 );

  switch ( msg )
  {
    case WM_INITDLG:
      //-----------------------------------
      {
        mResult = FolPropExpInitOptions( hwnd, mp1, mp2 );
      }
      break;

    case WM_COMMAND:
      //-----------------------------------
      switch ( WMCOMMANDID( mp1, mp2 ) )
      {
        case PID_PB_OK :
          {
            pIda = ACCESSDLGIDA( hwnd, PFOLEXPIDA );
            pIda->fUseShortFolderName = QUERYCHECK( hwnd, ID_FOLEXP_EXPORTSHORT_CHK );
            pIda->fExportAs = QUERYCHECK( hwnd, ID_FOLEXP_EXPORTAS_CHK );
            QUERYTEXT( hwnd, ID_FOLEXP_EXPORTAS_EF, pIda->szExportAs );
            UtlStripBlanks(  pIda->szExportAs );
            if ( QUERYCHECK( hwnd, ID_FOLEXP_HISTLOG_CHK ) )
            {
              pIda->fHistlog = TRUE;
              /***********************************************************/
              /* only histlog data requested...                          */
              /* So we manually set the flags as requested, i.e. selected*/
              /* docs (but empty lb), .....                              */
              /***********************************************************/
              pIda->pHeader->BitFlags.fDeleted = FALSE;
              SendDlgItemMessage( hwnd, ID_FOLEXP_DOCS_LB, TVM_DELETEITEM, 0, (LPARAM)TVI_ROOT );
              pIda->pHeader->BitFlags.fSelectedDocs = TRUE;
              pIda->pHeader->BitFlags.fWithNote = QUERYCHECK( hwnd, ID_FOLEXP_NOTE_CHK);
              pIda->pHeader->fContainsDict = FALSE;
              pIda->pHeader->BitFlags.fContainsMem = FALSE;
              pIda->fOldVers = FALSE;
              pIda->pHeader->BitFlags.fOldVers = (USHORT)pIda->fOldVers;
              pIda->fDelete = FALSE;
              pIda->fWithROMem = FALSE;
              pIda->fWithDocMem = FALSE;
            }
            else
            {
              pIda->fHistlog = FALSE;
              pIda->pHeader->BitFlags.fDeleted = QUERYCHECK( hwnd, ID_FOLEXP_DELETE_CHK );
//                    pIda->pHeader->BitFlags.fSelectedDocs = QUERYCHECK( hwnd, ID_FOLEXP_SELDOCS_CHK);
              pIda->pHeader->BitFlags.fWithNote = QUERYCHECK( hwnd, ID_FOLEXP_NOTE_CHK);
//              pIda->fOldVers = QUERYCHECK( hwnd, ID_FOLEXP_OLDVERS_CHK );
              pIda->fNonUnicode = QUERYCHECK( hwnd, ID_FOLEXP_OLDVERS_CHK );
              pIda->fWithDict = pIda->pHeader->fContainsDict = QUERYCHECK( hwnd, ID_FOLEXP_WITHDICT_CHK );
              pIda->pHeader->BitFlags.fContainsMem = QUERYCHECK( hwnd, ID_FOLEXP_WITHMEM_CHK );
//              pIda->fOldVers = QUERYCHECK( hwnd, ID_FOLEXP_OLDVERS_CHK );
              pIda->pHeader->BitFlags.fNonUnicode = (USHORT)pIda->fNonUnicode;
              pIda->pHeader->BitFlags.fOldVers = (USHORT)pIda->fOldVers;
              pIda->fDelete = QUERYCHECK( hwnd, ID_FOLEXP_DELETE_CHK );
              pIda->fWithROMem = QUERYCHECK( hwnd, ID_FOLEXP_WITHROMEM_CHK );
              pIda->fWithDocMem = QUERYCHECK( hwnd, ID_FOLEXP_WITHDOCMEM_CHK );
              pIda->fWithFolderMem = QUERYCHECK( hwnd, ID_FOLEXP_WITHMEM_CHK );
              pIda->pHeader->BitFlags.fContainsMem = (USHORT)(pIda->fWithROMem || pIda->fWithDocMem || pIda->fWithFolderMem);
                          pIda->pHeader->BitFlags.fMasterFolder = QUERYCHECK( hwnd, ID_FOLEXP_MASTER_CHK );
              pIda->fWithRedundancyData = QUERYCHECK( hwnd, ID_FOLEXP_WITH_REDUNDANCY_DATA_CHK );
              pIda->pHeader->BitFlags.fWithRedundancyData = pIda->fWithRedundancyData;
            } /* endif */

            // Get Originator name and e-mail

            QUERYTEXT( hwnd,ID_FOLEXP_ORIGINATOR_EF , pIda->szExpOriginator );
            QUERYTEXT( hwnd,ID_FOLEXP_ORIGMAIL_EF   , pIda->szExpEMail );
          }
          break;

        case ID_FOLEXP_WITHMEM_CHK:
          pIda = ACCESSDLGIDA( hwnd, PFOLEXPIDA );
          switch ( sNotification )
          {
            case BN_CLICKED:
              break;
          } /* endswitch */
          break;

        case ID_FOLEXP_HISTLOG_CHK:
          pIda = ACCESSDLGIDA( hwnd, PFOLEXPIDA );
          switch ( sNotification )
          {
            case BN_CLICKED:
              if ( QUERYCHECK( hwnd, ID_FOLEXP_HISTLOG_CHK) )
              {

                pIda -> fHistlog = TRUE;
                /********************************************************/
                /* disable all other controls ...                       */
                /********************************************************/
                ENABLECTRL( hwnd, ID_FOLEXP_WITHDICT_CHK, FALSE );
                ENABLECTRL( hwnd, ID_FOLEXP_WITHMEM_CHK, FALSE );
                ENABLECTRL( hwnd, ID_FOLEXP_WITHROMEM_CHK, FALSE );
                ENABLECTRL( hwnd, ID_FOLEXP_WITHDOCMEM_CHK, FALSE );
                ENABLECTRL( hwnd, ID_FOLEXP_DELETE_CHK, FALSE );
                ENABLECTRL( hwnd, ID_FOLEXP_OLDVERS_CHK, FALSE );
              }
              else
              {
                pIda -> fHistlog = FALSE;
                /********************************************************/
                /* enable all other controls again ..                   */
                /********************************************************/
                ENABLECTRL( hwnd, ID_FOLEXP_WITHMEM_CHK, TRUE );
                ENABLECTRL( hwnd, ID_FOLEXP_WITHDICT_CHK, pIda->fDict );
                ENABLECTRL( hwnd, ID_FOLEXP_WITHROMEM_CHK, pIda->fFolderHasROMem );
                ENABLECTRL( hwnd, ID_FOLEXP_WITHDOCMEM_CHK, pIda->fDocMem );
                ENABLECTRL( hwnd, ID_FOLEXP_DELETE_CHK, TRUE );
                ENABLECTRL( hwnd, ID_FOLEXP_OLDVERS_CHK, TRUE );
              } /* endif */

              // send message to General tab
              {
                PFNWP pfnWp = (PFNWP) GetWindowLong( pIda->hwndPages[ 0 ],
                                                     DWL_DLGPROC );


                fOk =  !pfnWp( pIda->hwndPages[0], WM_COMMAND,
                               ID_FOLEXP_HISTLOG_CHK, 0L);


              }
              break;
          } /* endswitch */
          break;

        case ID_FOLEXP_EXPORTAS_CHK:
          pIda = ACCESSDLGIDA( hwnd, PFOLEXPIDA );
          switch ( sNotification )
          {
            case BN_CLICKED:
              // disable/enable export short control and export as entryfield
              if ( QUERYCHECK( hwnd, ID_FOLEXP_EXPORTAS_CHK) )
              {
                ENABLECTRL( hwnd, ID_FOLEXP_EXPORTSHORT_CHK, FALSE );
                ENABLECTRL( hwnd, ID_FOLEXP_EXPORTAS_EF, TRUE );
              }
              else
              {
                ENABLECTRL( hwnd, ID_FOLEXP_EXPORTSHORT_CHK, TRUE );
                ENABLECTRL( hwnd, ID_FOLEXP_EXPORTAS_EF, FALSE );
              } /* endif */
              break;
          } /* endswitch */
          break;

        case ID_FOLEXP_EXPORTAS_EF:
          break;

        case ID_FOLEXP_EXPORTSHORT_CHK:
          pIda = ACCESSDLGIDA( hwnd, PFOLEXPIDA );
          switch ( sNotification )
          {
            case BN_CLICKED:
              // disable/enable export as control
              if ( QUERYCHECK( hwnd, ID_FOLEXP_EXPORTSHORT_CHK) )
              {
                ENABLECTRL( hwnd, ID_FOLEXP_EXPORTAS_CHK, FALSE );
              }
              else
              {
                if ( !pIda->pszObjList )
                   ENABLECTRL( hwnd, ID_FOLEXP_EXPORTAS_CHK, TRUE );
              } /* endif */
              break;
          } /* endswitch */
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
      mResult = WinDefDlgProc( hwnd, msg, mp1, mp2 );
      break;

    default:
      //-----------------------------------
      mResult = WinDefDlgProc( hwnd, msg, mp1, mp2 );
      break;
  } /* endswitch */

  return mResult;
};

// set check mark of a tree view item and of all childs (recursively)
int FolExpSetTreeCheck( HWND hwndTV, HTREEITEM hItem, BOOL fCheck )
{
  TVITEM tvItem;

  // set check mark for this item
  tvItem.mask = TVIF_HANDLE | TVIF_STATE;
  tvItem.hItem = hItem;
  tvItem.stateMask = TVIS_STATEIMAGEMASK;
  tvItem.state = INDEXTOSTATEIMAGEMASK((fCheck ? 2 : 1));
  TreeView_SetItem( hwndTV, &tvItem);

  // set check marks of all childs
  hItem = TreeView_GetChild( hwndTV, hItem );
  while ( hItem != NULL )
  {
    FolExpSetTreeCheck( hwndTV, hItem, fCheck );
    hItem = TreeView_GetNextSibling( hwndTV, hItem );
  } /* endwhile */

  return( 0 );
} /* end of function FolExpSetTreeCheck */


// select checked tree view items in given listbox and set flag of all
// checked subfolders
int FolExpSelCheckedTreeItems( HWND hwndTV, HTREEITEM hItem, HWND hwndLB, PSUBFOLINFO pInfo, PSZ pszBuffer, BOOL fIncludeAll )
{
  TVITEM tvItem;                       // buffer for item data
  BOOL   fChecked = FALSE;             // TRUE = item is checked

  // get check mark for this item
  tvItem.mask = TVIF_HANDLE | TVIF_STATE | TVIF_PARAM | TVIF_TEXT;
  tvItem.hItem = hItem;
  tvItem.stateMask = TVIS_STATEIMAGEMASK;
  tvItem.cchTextMax = MAX_LONGFILESPEC;
  tvItem.pszText = pszBuffer;

  TreeView_GetItem( hwndTV, &tvItem);
  if ( fIncludeAll )
  {
    // treat as checked if fIncludeAll is set
    fChecked = fIncludeAll;
  }
  else
  {
    fChecked = (BOOL)(tvItem.state >> 12) - 1;
  } /* endif */

  // different handling for subfolders and documents
  if ( tvItem.lParam & SUBFOLDERFLAG )
  {
    // strip off subfolder flag
    tvItem.lParam = tvItem.lParam & ~SUBFOLDERFLAG;

    // set flag for subfolder and all parent subfolders in subfolder info table
    if ( fChecked )
    {
      FolExpSetSubFolInfoFlag( (ULONG)tvItem.lParam, pInfo );
      fIncludeAll = TRUE; // all subsequent subfolders and documents are included
    }
  }
  else
  {
    // search and select item if item had been checked in tree view control
    if ( fChecked )
    {
      SHORT sItem = SEARCHITEMHWND( hwndLB, pszBuffer );
      if ( sItem >= 0 )
      {
        SELECTITEMMSHWND( hwndLB, sItem );
      } /* endif */
    } /* endif */
  } /* endif */

  // handle child items
  hItem = TreeView_GetChild( hwndTV, hItem );
  while ( hItem != NULL )
  {
    FolExpSelCheckedTreeItems( hwndTV, hItem, hwndLB, pInfo, pszBuffer, fIncludeAll );
    hItem = TreeView_GetNextSibling( hwndTV, hItem );
  } /* endwhile */

  return( 0 );
} /* end of function FolExpSelCheckedTreeItems */

// set the flag for the given subfolder ID and all its parent subfolders
int FolExpSetSubFolInfoFlag( ULONG ulID, PSUBFOLINFO pInfo )
{
  while ( ulID != 0 )
  {
    int i = 0;
    ULONG ulParent = 0;

    while ( pInfo[i].szName[0] != EOS )
    {
      if ( pInfo[i].ulID == ulID )
      {
        pInfo[i].fFlag = TRUE;
        ulParent = pInfo[i].ulParentFolder;
        break;
      }
      else
      {
        i++;
      } /* endif */
    } /* endwhile */
    ulID = ulParent;
  } /* endwhile */

  return( 0 );
} /* end of function FolExpSetSubFolInfoFlag */

// general cleanup function for folder export
// note: this function undos any changes made during folder export preparation
int FolExpCleanup( PFOLEXPIDA pIda )
{
  // get rid off any copied redundant segment log file
  CHAR szTempSegLog[MAX_EQF_PATH];

  UtlMakeEQFPath( szTempSegLog, pIda->szFolderObjName[0], SYSTEM_PATH, pIda->szFolName );
  strcat( szTempSegLog, BACKSLASH_STR  );
  strcat( szTempSegLog, REDUNDCOUNTFILE );

  if ( UtlFileExist( szTempSegLog ) )
  {
    UtlDelete( szTempSegLog, 0L, FALSE );
  } /* endif */

  // delete any existing memory info files of this folder export
  {
    WIN32_FIND_DATA FindData;

    // setup search path
    UtlMakeEQFPath( pIda->szInfoFileName, NULC, MEM_PATH, NULL );
    strcat( pIda->szInfoFileName, BACKSLASH_STR );
    strcat( pIda->szInfoFileName, pIda->szFolName );
    strcat( pIda->szInfoFileName, "-*" );
    strcat( pIda->szInfoFileName, EXT_OF_MEMORYINFO );

    // loop over all info files
    BOOL fMoreFiles = FALSE;
    HANDLE hDir = FindFirstFile( pIda->szInfoFileName, &FindData );
    if ( hDir != INVALID_HANDLE_VALUE )
    {
      do
      {
        UtlMakeEQFPath( pIda->szInfoFileName, NULC, MEM_PATH, NULL );
        strcat( pIda->szInfoFileName, BACKSLASH_STR );
        strcat( pIda->szInfoFileName, FindData.cFileName );
        UtlDelete( pIda->szInfoFileName, 0L, FALSE );

        fMoreFiles = FindNextFile( hDir, &FindData );
      } while ( fMoreFiles );
      FindClose( hDir );
    }
  }

  // delete any markup table info files of this folder export
  {
    WIN32_FIND_DATA FindData;

    // setup search path
    UtlMakeEQFPath( pIda->szInfoFileName, NULC, TABLE_PATH, NULL );
    strcat( pIda->szInfoFileName, BACKSLASH_STR );
    strcat( pIda->szInfoFileName, pIda->szFolName );
    strcat( pIda->szInfoFileName, "-*" );
    strcat( pIda->szInfoFileName, EXT_OF_TAGTABLEINFO );

    // loop over all info files
    BOOL fMoreFiles = FALSE;
    HANDLE hDir = FindFirstFile( pIda->szInfoFileName, &FindData );
    if ( hDir != INVALID_HANDLE_VALUE )
    {
      do
      {
        UtlMakeEQFPath( pIda->szInfoFileName, NULC, TABLE_PATH, NULL );
        strcat( pIda->szInfoFileName, BACKSLASH_STR );
        strcat( pIda->szInfoFileName, FindData.cFileName );
        UtlDelete( pIda->szInfoFileName, 0L, FALSE );

        fMoreFiles = FindNextFile( hDir, &FindData );
      } while ( fMoreFiles );
      FindClose( hDir );
    }
  }

  return( 0 );
} /* end of function FolExpCleanup */

// fill export/import format combobox and select last used format
void FolFillFolderFormatCombobox( HWND hwnd, int id, USHORT usFormat )
{
  HWND hwndFormatCB = GetDlgItem( hwnd, id );
  if ( hwndFormatCB != NULLHANDLE )
  {
    int item = CBINSERTITEMHWND( hwndFormatCB, "OpenTM2 Folder (FXP)" );
    CBSETITEMHANDLEHWND( hwndFormatCB, item, FOL_TMGR_FORMAT );
    if ( usFormat == FOL_TMGR_FORMAT ) CBSELECTITEMHWND( hwndFormatCB, item );
    item = CBINSERTITEMHWND( hwndFormatCB, "XLIFF folder" );
    CBSETITEMHANDLEHWND( hwndFormatCB, item, FOL_XLIFF_FORMAT );
    if ( usFormat == FOL_XLIFF_FORMAT ) CBSELECTITEMHWND( hwndFormatCB, item );
  } /* endif */
} /* end of function FolFillFolderFormatCombobox */
