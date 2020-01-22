//+----------------------------------------------------------------------------+
//|  EQFFOL01.C - EQF Folder Handler dialog procedures                         |
//+----------------------------------------------------------------------------+
//|  Copyright Notice:                                                         |
//|                                                                            |
//|      Copyright (C) 1990-2016, International Business Machines              |
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
//|   - Folder export: error handling for access errors on properties;         |
//|                    error handling while processing WM_INITDLG;             |
//|                    delete folder function and appropriate checks;          |
//|                    close dialog while creating package;                    |
//|                    locking for packaged files;                             |
//|                                                                            |
//+----------------------------------------------------------------------------+

#define INCL_EQF_FOLDER           // folder list and document list functions
#define INCL_EQF_TM               // general Transl. Memory functions
#define INCL_EQF_TAGTABLE         // tag table and format table functions
#define INCL_EQF_EDITORAPI        // for WM_EQF_SHOWHTML message

#define INCL_EQF_ASD              // dictionary access functions (Asd...)
#define INCL_EQF_LDB              // dictionary data encoding functions
#define INCL_EQF_DLGUTILS         // dialog utilities

#include <eqf.h>                  // General Translation Manager include file
#include <eqfsetup.h>

#include <eqf.id>                 // general EQF ids
//#include <eqftwb.id>              // ids for Troja workbench
//#include <eqfstart.id>            // ids for EQFRES
#include <eqffol.id>            // ids for Folder

#include "eqfdde.h"                    // batch mode functions
#include "eqffol00.h"                  // our .h stuff
#include "eqffol.id"                   // our ID file
#include <time.h>                      // C standard time functions
  #include "OTMFUNC.H"            // function call interface public defines
  #include "eqffunci.h"           // function call interface private defines
  #include "SHLOBJ.H"             // shell functions
#include <eqfiana1.id>            // analysis dialog ids
#include "EQFHLOG.H"            // defines for history log processing
#include <OTMGLOBMem.h>         // Global Memory defines

#include <eqfdtag.h>              // include tag definitions
#include "eqfutdlg.h"
#include "OtmDictionaryIF.H"
#include "eqfdicti.h"             // Private include file of dictionary handler

extern HELPSUBTABLE hlpsubtblFolderProps[];
extern HELPSUBTABLE hlpsubtblCreateFolder[];

// static buffer for object name when starting analysis from folder properties dialog
static OBJNAME szObjNameBuffer;

#define FOL_EXPORT_TASK USER_TASK + 1  // export-a-folder task
#define FOL_IMPORT_TASK USER_TASK + 2  // import-a-folder task

#define FOLDEREXPORT_EXT ".FXP"        // extension of an exported folder

static PFNWP pfnwpFolImpFrameProc;        // address of old frame proc
static PFNWP pfnwpFolExpFrameProc;        // address of old frame proc

MRESULT EXPENTRY FolImpFrameSubProc( HWND, USHORT, WPARAM, LPARAM );
MRESULT EXPENTRY FolExpFrameSubProc( HWND, USHORT, WPARAM, LPARAM );
MRESULT EXPENTRY FolExportWP( HWND, USHORT, WPARAM, LPARAM );
INT_PTR APIENTRY FOLMEMSEL_DLGPROC( HWND hwndDlg, WINMSG msg, WPARAM mp1, LPARAM mp2 );
INT_PTR APIENTRY DelHistLogDlgProc( HWND hwndDlg, WINMSG msg, WPARAM mp1, LPARAM mp2 );
BOOL FolShowSelectionDialog( HWND hwndDlg, PFOLPROPIDA pIda, BOOL fPID );
BOOL FolDictPIDSelection( HWND hwndDlg, PFOLPROPIDA pIda );
void FolFillHistlogSize( PFOLPROPIDA pIda, HWND hwndDlg  );
void FolDeleteHistlog( PFOLPROPIDA pIda, HWND hwndDlg  );
void FolMakeHistlogFileName( PFOLPROPIDA pIda, PSZ pszBuffer );
USHORT FolFuncCheckObjects( PSZ pszObjList, SHORT sObjType );
void FolFuncCopyObjectsToProps( PSZ pszObjList, SHORT sObjType, PPROPFOLDER pProp );
void FolGMCleanup( PVOID pvData );
USHORT TrackIDToDocSegment( PSZ pszFolder, PSZ pszTrackID, PSZ pszDocName, ULONG *ulSegNum );
void CheckDictPIDValues( PSZ pszValues );

// compare two characters and return result
int cCompare( const void *arg1, const void *arg2 )
{
  return( *((PCHAR)arg1) - *((PCHAR)arg2) );
}/* end cCompare */

static void
FolSetDocListView(PPROPFOLDER pProp);

MRESULT FolNewControl
(
HWND   hwndDlg,                     // dialog handle
SHORT  sId,                         // id in action
SHORT  sNotification                // notification
)
{
  PFOLPROPIDA      pIda;               // ptr to instance data area
  MRESULT          mResult = FALSE;    // dialog procedure return value

  pIda = ACCESSDLGIDA( hwndDlg, PFOLPROPIDA );

  switch ( sId )
  {
    case ID_FOLNEW_EDITOR_CBS:
      if ( (sNotification == CBN_EFCHANGE) &&
           (pIda->pProp->PropHead.chType != PROP_TYPE_NEW ) &&
           (!pIda->fCBChange) )
      {
        QUERYTEXT( hwndDlg, ID_FOLNEW_EDITOR_CBS, pIda->szBuffer1);
        if ( stricmp( pIda->pProp->szEditor, pIda->szBuffer1 ) != 0 )
        {
          // check if folder is open
          strcpy( pIda->szBuffer1, pIda->pProp->PropHead.szPath );
          pIda->szBuffer1[0] = pIda->pProp->chDrive;
          strcat( pIda->szBuffer1, "\\" );
          strcat( pIda->szBuffer1, pIda->pProp->PropHead.szName );
          if ( EqfQueryObject( pIda->szBuffer1, clsFOLDER, 0) )
          {
            UtlErrorHwnd( ERROR_FOLPROP_FOLDER_IS_OPEN,
                          MB_CANCEL,
                          0, (PSZ *)NULP,
                          EQF_ERROR, hwndDlg );
            pIda->fCBChange = TRUE;
            SETTEXT( hwndDlg, ID_FOLNEW_EDITOR_CBS, pIda->pProp->szEditor );
            pIda->fCBChange = FALSE;
          } /* endif */
        } /* endif */
      } /* endif */
      break;
  } /* endswitch */

  return( mResult );
} /* end of function FolNewControl */


INT_PTR CALLBACK FOLMODELDLGPROC
(
HWND   hwndDlg,
WINMSG message,
WPARAM mp1,
LPARAM mp2
)
{
  PFOLPROPIDA   pIda;                 // ptr to instance data area
  PPROPFOLDER   psrc;                 // ptr to model folder
  HPROP         hsrc;                 // handle of model folder
  EQFINFO       ErrorInfo;            // error returned by property handler
  PSZ           pTemp;
  SHORT         sItem;                // index of selected listbox item
  BOOL           fOK;                 // ok flag returned to caller
  MRESULT        mResult = FALSE;

  switch ( message)
  {
    case WM_EQF_QUERYID: HANDLEQUERYID( ID_FOLMODEL_DLG, mp2 ); break;
/*--------------------------------------------------------------------------*/
    case WM_INITDLG:
      //--- anchor IDA ---
      pIda = (PFOLPROPIDA) mp2;
      ANCHORDLGIDA( hwndDlg, pIda);
      pIda->hwndDlg = hwndDlg;
      EqfSend2Handler( FOLDERLISTHANDLER, WM_EQF_INSERTNAMES,
                       MP1FROMHWND( WinWindowFromID( hwndDlg, ID_FOLMODEL_FOLDER_LB ) ),
                       0L );

      SELECTITEM( hwndDlg, ID_FOLMODEL_FOLDER_LB, 0 );

      break;

/*--------------------------------------------------------------------------*/
    case WM_COMMAND:
      pIda = ACCESSDLGIDA( hwndDlg, PFOLPROPIDA );
      mResult = MRFROMSHORT( TRUE);
      fOK = TRUE;                    // assume everything is ok

      switch ( WMCOMMANDID(mp1,mp2) )
      {
	      case ID_FOLMODEL_HELP_PB:
	        mResult = UtlInvokeHelp();
  	      break;
        case DID_CANCEL:
        case ID_FOLMODEL_CANCEL_PB:
          WinDismissDlg( hwndDlg, FALSE );
          break;

        case ID_FOLMODEL_OK_PB:
          sItem = QUERYSELECTION( hwndDlg, ID_FOLMODEL_FOLDER_LB );

          if ( sItem != LIT_NONE )
          {
            QUERYITEMTEXT( hwndDlg, ID_FOLMODEL_FOLDER_LB, sItem,
                           pIda->szBuffer1 );
            ANSITOOEM( pIda->szBuffer1 );
            ObjLongToShortName( pIda->szBuffer1, pIda->szBuffer2,
                                FOLDER_OBJECT, NULL );
            strcpy( pIda->szBuffer1, pIda->szBuffer2 );
            UtlMakeEQFPath( pIda->szBuffer2, NULC, SYSTEM_PATH, NULL );
            strcat( pIda->szBuffer1, EXT_FOLDER_MAIN);

            if ( (hsrc = OpenProperties( pIda->szBuffer1, pIda->szBuffer2,
                                          PROP_ACCESS_READ, &ErrorInfo))== NULL)
            {
              pTemp = pIda->szBuffer1;
              UtlErrorHwnd( ERROR_OPEN_PROPERTIES, MB_OK, 1, &pTemp,
                            EQF_ERROR, hwndDlg );
              fOK = FALSE;
            }
            else
            {
              //--- get pointer to model folder properties ---
              psrc = (PPROPFOLDER)MakePropPtrFromHnd( hsrc);

              //--- copy selected items to new folder properties
              strcpy( pIda->pProp->szFormat, psrc->szFormat );
              strcpy( pIda->pProp->szLongMemory, psrc->szLongMemory );
              strcpy( pIda->pProp->szMemory, psrc->szMemory );
              strcpy( pIda->pProp->DicTbl,   psrc->DicTbl );
              strcpy( pIda->pProp->szEditor, psrc->szEditor );
              strcpy( pIda->pProp->szSourceLang, psrc->szSourceLang );
              strcpy( pIda->pProp->szTargetLang, psrc->szTargetLang );
              strcpy( pIda->pProp->MemTbl,   psrc->MemTbl );
              memcpy( pIda->pProp->sLastUsedViewList, psrc->sLastUsedViewList, sizeof(pIda->pProp->sLastUsedViewList) );
              memcpy( pIda->pProp->sDetailsViewList, psrc->sDetailsViewList, sizeof(pIda->pProp->sDetailsViewList) );
              memcpy( pIda->pProp->sSortList, psrc->sSortList, sizeof(pIda->pProp->sSortList) );
              memcpy( &(pIda->pProp->Filter), &(psrc->Filter), sizeof(pIda->pProp->Filter) );
              memcpy( &(pIda->pProp->sLastUsedViewWidth), &(psrc->sLastUsedViewWidth), sizeof(pIda->pProp->sLastUsedViewWidth) );
              memcpy( pIda->pProp->aLongDicTbl, psrc->aLongDicTbl, sizeof(psrc->aLongDicTbl) );
              memcpy( pIda->pProp->aLongMemTbl, psrc->aLongMemTbl, sizeof(psrc->aLongMemTbl) );
              strcpy( pIda->pProp->szConversion, psrc->szConversion );
              //--- close model folder properties ---
              CloseProperties( hsrc, PROP_QUIT, &ErrorInfo);
            } /* endif */
          } /* endif */
          if ( fOK )
          {
            WinDismissDlg( hwndDlg, TRUE );
          } /* endif */
          break;
      } /* endswitch */
      break;

    default:
      mResult = WinDefDlgProc (hwndDlg, message, mp1, mp2 );
/*--------------------------------------------------------------------------*/
  }
  return( mResult );
}


//+--------------------------------------------------------------------------+
//|  Create a new folder directory                                           |
//+--------------------------------------------------------------------------+
SHORT CreateFolderDir( PPROPFOLDER pProp, BOOL fMsg )
{
//  SHORT CreateDir( PSZ Path, PSZ Name, BOOL msgflg);
  char t1buf[ MAX_PATH144];
  PPROPSYSTEM psp;
  SHORT rc;
  BOOL  fTrueFalse = TRUE & FALSE;

  pProp->PropHead.szPath[0] = pProp->chDrive;

  strcat( strcpy( t1buf, pProp->PropHead.szPath), "\\");
  strcat( t1buf, pProp->PropHead.szName );
  psp = (PPROPSYSTEM)MakePropPtrFromHnd( EqfQuerySystemPropHnd());
  do
  {
    if ( (rc= CreateDir( pProp->PropHead.szPath, pProp->PropHead.szName, fMsg))!=0)
      break;
    if ( (rc = CreateDir( t1buf, psp->szPropertyPath, fMsg)) != 0 ) break;
    if ( (rc = CreateDir( t1buf, psp->szDirSourceDoc, fMsg)) != 0 ) break;
    if ( (rc = CreateDir( t1buf, psp->szDirTargetDoc, fMsg)) != 0 ) break;
    if ( (rc = CreateDir( t1buf, psp->szDirSegSourceDoc, fMsg))!=0) break;
    if ( (rc = CreateDir( t1buf, psp->szDirSegTargetDoc, fMsg))!=0) break;
  } while (fTrueFalse);     /*( TRUE & FALSE);*/
  /******************************************************************/
  /* if not all created correctly remove folder directories         */
  /******************************************************************/
  if ( rc )                                                       /*KIT1257*/
  {
    /*KIT1257*/
    strcat( strcpy( t1buf, pProp->PropHead.szPath), "\\");        /*KIT1257*/
    strcat( t1buf, pProp->PropHead.szName);                       /*KIT1257*/
    UtlRemoveDir( t1buf, FALSE );                                 /*KIT1257*/
  } /* endif */                                                   /*KIT1257*/

  pProp->PropHead.szPath[0] = psp->szPrimaryDrive[0];

  return( rc);
}

/*+--------------------------------------------------------------------------+
  |  Create a folder directory                                               |
  +--------------------------------------------------------------------------+
*/
SHORT CreateDir( PSZ Path, PSZ Name, BOOL msgflg)
{
  char t1buf[ MAX_PATH144];
  SHORT rc;

  strcat( strcat( strcpy( t1buf, Path), "\\"), Name);

  if ( !UtlDirExist( t1buf ) )
  {
    if ( (rc = UtlMkDir( t1buf, 0L, 0)) != NO_ERROR )
      if ( msgflg)
        switch ( rc )
        {
          case ERROR_DISK_FULL :
            // if full path name given, leave drive letter only  /*KIT1256*/
            if ( Path[0] && (Path[1] == ':') )                   /*KIT1256*/
            {
              /*KIT1256*/
              Path[2] = EOS;                                     /*KIT1256*/
            }                                                    /*KIT1256*/
            else                                                 /*KIT1256*/
            {
              /*KIT1256*/
              Path[0] = EOS;                                     /*KIT1256*/
            } /* endif */                                        /*KIT1256*/
            UtlError( ERROR_DISK_IS_FULL, MB_CANCEL, 1,          /*KIT1256*/
                      &Path, EQF_ERROR);                         /*KIT1256*/
            break;
          default :
            UtlError( ERROR_CREATE_SUBDIR, MB_CANCEL, 0, (PSZ *)NULP, EQF_ERROR);
            break;
        } /* endswitch */
  }
  else
  {
    rc = 0;
  } /* endif */
  return( rc);
}

//+--------------------------------------------------------------------------+
//|  Load fields for existing properties                                     |
//+--------------------------------------------------------------------------+
void LoadFields_1( HWND hwnd, PFOLPROPIDA pIda)
{
  SHORT rc;
  PSZ   p;
  PPROPSYSTEM   pSysProp;             // ptr to system props

  CBSEARCHSELECT( rc, hwnd, ID_FOLNEW_FORMAT_CBS, pIda->pProp->szFormat );
  if ( rc == LIT_NONE )
  {
    p = pIda->pProp->szFormat;
    UtlError( ERROR_FORMAT_NOTFOUND, MB_OK, 1, &p, EQF_ERROR );
    CBSELECTITEM( hwnd, ID_FOLNEW_FORMAT_CBS, 0 );
  } /* endif */

  if ( pIda->pProp->szLongMemory[0] != EOS )
  {
    OEMTOANSI( pIda->pProp->szLongMemory );
    CBSEARCHSELECT( rc, hwnd, ID_FOLNEW_MEMORY_CBS, pIda->pProp->szLongMemory );
    ANSITOOEM( pIda->pProp->szLongMemory );
  }
  else
  {
    CBSEARCHSELECT( rc, hwnd, ID_FOLNEW_MEMORY_CBS, pIda->pProp->szMemory );
  }
  if ( rc == LIT_NONE)
  {
    if ( pIda->pProp->szLongMemory[0] != EOS )
      p = pIda->pProp->szLongMemory;
    else
      p = pIda->pProp->szMemory;
    OEMTOANSI( p );
    UtlError( ERROR_MEMORY_NOTFOUND, MB_OK, 1, &p, EQF_ERROR );
    SETTEXT( hwnd, ID_FOLNEW_MEMORY_CBS, p );
    ANSITOOEM( p );
  }

  // check if system properties default editor field is filled
  pSysProp = (PPROPSYSTEM)MakePropPtrFromHnd( EqfQuerySystemPropHnd() );
  if ( !pSysProp->szDefaultEditor[0] )
  {
    Utlstrccpy( pSysProp->szDefaultEditor, EDITOR_PROPERTIES_NAME, '.' );
  } /* endif */

  // if no editor association exists, use default editor
  if ( !pIda->pProp->szEditor[0] )
  {
    strcpy( pIda->pProp->szEditor, pSysProp->szDefaultEditor );
  } /* endif */

  pIda->fCBChange = TRUE;
  CBSEARCHSELECT( rc, hwnd, ID_FOLNEW_EDITOR_CBS, pIda->pProp->szEditor );
  if ( rc == LIT_NONE)
  {
    p = pIda->pProp->szEditor;
//    UtlErrorHwnd( ERROR_EDITOR_NOT_FOUND, MB_OK, 1, &p, EQF_ERROR, hwnd);
    UtlError( ERROR_EDITOR_NOT_FOUND, MB_OK, 1, &p, EQF_ERROR );
    CBSEARCHSELECT( rc, hwnd, ID_FOLNEW_EDITOR_CBS, pSysProp->szDefaultEditor );
    if ( rc == LIT_NONE)
    {
      SHORT sItem = LIT_FIRST;
      CBSELECTITEM( hwnd, ID_FOLNEW_EDITOR_CBS, sItem );
    } /* endif */
  } /* endif */
  pIda->fCBChange = FALSE;

  pIda->fCBChange = TRUE;
  //if ( pIda->pProp->szConversion[0] != EOS )
  //{
  //  CBSEARCHSELECT( rc, hwnd, ID_FOLNEW_CONV_CBS, pIda->pProp->szConversion );
  //} /* endif */
  HIDECONTROL( hwnd, ID_FOLNEW_CONV_CBS );
  HIDECONTROL( hwnd, ID_FOLNEW_CONV_STATIC );
  
  pIda->fCBChange = FALSE;

  if ( pIda->pProp->aLongDicTbl[0][0] != EOS )
  {
    int i = 0;
    WinSendMsg( pIda->hDicLBS, LM_DELETEALL, NULL, NULL);
    while ( (i < MAX_NUM_OF_FOLDER_DICS) &&
            (pIda->pProp->aLongDicTbl[i][0] != EOS) )
    {
      strcpy( pIda->szBuffer1, pIda->pProp->aLongDicTbl[i] );
      OEMTOANSI( pIda->szBuffer1 );
      p = pIda->szBuffer1;
      rc = SEARCHITEMHWND( pIda->hDicLBA, p );
      if ( rc == LIT_NONE) UtlError( ERROR_DIC_NOTFOUND, MB_OK, 1, &p, EQF_ERROR );
      INSERTITEMENDHWND( pIda->hDicLBS, p );
      i++;
    } /* endwhile */
  }
  else
  {
    WinSendMsg( pIda->hDicLBS, LM_DELETEALL, NULL, NULL);
    p = strtok( pIda->pProp->DicTbl, "\x15");

    for (; p; p=strtok( NULL, "\x15"))
    {
      rc = SEARCHITEMHWND( pIda->hDicLBA, p );
      if ( rc == LIT_NONE)
        UtlError( ERROR_DIC_NOTFOUND, MB_OK, 1, &p, EQF_ERROR );
      INSERTITEMENDHWND( pIda->hDicLBS, p );
    }
  }
  if ( QUERYITEMCOUNTHWND( pIda->hDicLBA ) != 0 )
  {
     ENABLECTRL( hwnd, ID_FOLNEW_DICTPID_CHK, TRUE );
  } else {
     pIda->pProp->fDictPIDSelect = FALSE ;
     ENABLECTRL( hwnd, ID_FOLNEW_DICTPID_CHK, pIda->pProp->fDictPIDSelect );
  }
  ENABLECTRL( hwnd, ID_FOLNEW_DICTPID_EF, pIda->pProp->fDictPIDSelect );
  ENABLECTRL( hwnd, ID_FOLNEW_DICTPID_PB, pIda->pProp->fDictPIDSelect );
  SETCHECK( hwnd, ID_FOLNEW_DICTPID_CHK, pIda->pProp->fDictPIDSelect );

  if ( pIda->pProp->aLongMemTbl[0][0] != EOS )
  {
    int i = 0;
    WinSendMsg( pIda->hMemLBS, LM_DELETEALL, NULL, NULL);
    while ( (i < MAX_NUM_OF_READONLY_MDB) &&
            (pIda->pProp->aLongMemTbl[i][0] != EOS) )
    {
      strcpy( pIda->szBuffer1, pIda->pProp->aLongMemTbl[i] );
      OEMTOANSI( pIda->szBuffer1 );
      p = pIda->szBuffer1;
      rc = SEARCHITEMHWND( pIda->hMemLBA, p );
      if ( rc == LIT_NONE)
        UtlError( ERROR_MEMORY_NOTFOUND, MB_OK, 1, &p, EQF_ERROR );
      INSERTITEMENDHWND( pIda->hMemLBS, p );
      i++;
    } /* endwhile */
  }
  else
  {
    WinSendMsg( pIda->hMemLBS, LM_DELETEALL, NULL, NULL);
    p = strtok( pIda->pProp->MemTbl, "\x15");

    for (; p; p=strtok( NULL, "\x15"))
    {
      rc = SEARCHITEMHWND( pIda->hMemLBA, p );
      if ( rc == LIT_NONE)
        UtlError( ERROR_MEMORY_NOTFOUND, MB_OK, 1, &p, EQF_ERROR );
      INSERTITEMENDHWND( pIda->hMemLBS, p );
    }
  }

  /************************************************************/
  /* Select current settings for source and target language   */
  /************************************************************/
  CBSEARCHSELECT( rc, hwnd, ID_FOLNEW_SOURCELANG_CBS,
                  pIda->pProp->szSourceLang );
  if ( rc == LIT_NONE )
  {
    p = pIda->pProp->szSourceLang;
    UtlError( ERROR_SOURCELANG_NOTINSTALLED, MB_OK, 1, &p, EQF_ERROR );
  } /* endif */

  CBSEARCHSELECT( rc, hwnd, ID_FOLNEW_TARGETLANG_CBS,
                  pIda->pProp->szTargetLang );
  if ( rc == LIT_NONE )
  {
    p = pIda->pProp->szTargetLang;
    UtlError( ERROR_TARGETLANG_NOTINSTALLED, MB_OK, 1, &p, EQF_ERROR );
  } /* endif */

}


//+--------------------------------------------------------------------------+
//|  FolDeleteSegDocuments - delete segment source and target of all         |
//|                          folder documents, update document properties    |
//+--------------------------------------------------------------------------+
BOOL FolDeleteSegDocuments( PFOLPROPIDA pIda )
{
  HPROP      hDocProp;                // document properties handle
  PPROPDOCUMENT pDocProp;             // document properties pointer
  EQFINFO    ErrorInfo;               // error returned by property handler
  FILEFINDBUF stResultBuf;            // DOS file find struct
  USHORT     usCount = 1;             // number of files requested
  HDIR       hDirHandle = HDIR_CREATE;// DosFind routine handle
  BOOL       fOK = TRUE;              // internal OK flag
  PSZ        pszName = RESBUFNAME(stResultBuf); // ptr to name in stResultBuf

  // setup search path in pIda->szBuffer1 and folder path in pIda->szBuffer2
  UtlMakeEQFPath( pIda->szBuffer1,
                  pIda->pProp->chDrive,
                  PROPERTY_PATH,
                  pIda->pProp->PropHead.szName );
  strcat( pIda->szBuffer1, "\\" );
  strcat( pIda->szBuffer1, DEFAULT_PATTERN );
  UtlMakeEQFPath( pIda->szBuffer2,
                  pIda->pProp->chDrive,
                  SYSTEM_PATH,
                  pIda->pProp->PropHead.szName );


  UtlFindFirst( pIda->szBuffer1, &hDirHandle, FILE_NORMAL,
                &stResultBuf, sizeof( stResultBuf), &usCount, 0L, 0);
  while ( fOK && usCount )
  {
    //--- open document properties ---
    hDocProp = OpenProperties(
                             pszName,
                             pIda->szBuffer2,
                             PROP_ACCESS_READ, &ErrorInfo);
    if ( hDocProp )
    {
      if ( SetPropAccess( hDocProp, PROP_ACCESS_WRITE) )
      {
        pDocProp = (PPROPDOCUMENT)MakePropPtrFromHnd( hDocProp );

        pDocProp->ulXLated  = 0L;
        pDocProp->ulSeg     = 0L;
        pDocProp->ulTouched = 0L;
        pDocProp->usComplete=  0;
        pDocProp->usCopied  =  0;
        pDocProp->usModified=  0;
        pDocProp->usScratch =  0;

        SaveProperties( hDocProp, (PEQFINFO)&ErrorInfo );

        ResetPropAccess( hDocProp, PROP_ACCESS_WRITE);
      } /* endif */

      CloseProperties( hDocProp, PROP_QUIT, &ErrorInfo );
    } /* endif */

    // delete segmented source of document
    UtlMakeEQFPath( pIda->szBuffer1,
                    pIda->pProp->chDrive,
                    DIRSEGSOURCEDOC_PATH,
                    pIda->pProp->PropHead.szName );
    strcat( pIda->szBuffer1, "\\" );
    strcat( pIda->szBuffer1, pszName );
    UtlDelete( pIda->szBuffer1, 0L, FALSE );

    // delete segmented target of document
    UtlMakeEQFPath( pIda->szBuffer1,
                    pIda->pProp->chDrive,
                    DIRSEGTARGETDOC_PATH,
                    pIda->pProp->PropHead.szName );
    strcat( pIda->szBuffer1, "\\" );
    strcat( pIda->szBuffer1, pszName );
    UtlDelete( pIda->szBuffer1, 0L, FALSE );

    UtlFindNext( hDirHandle, &stResultBuf, sizeof( stResultBuf), &usCount, 0);
  } /* endwhile */

  // close search file handle
  if ( hDirHandle != HDIR_CREATE ) UtlFindClose( hDirHandle, FALSE );

  return( fOK );
} /* end of FolDeleteSegDocuments */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     FolGetSourceLng                                          |
//+----------------------------------------------------------------------------+
//|Function call:     FolGetSourceLng( PFOLPROPIDA pIda, HWND hwndListbox,     |
//|                                    SHORT sItem );                          |
//+----------------------------------------------------------------------------+
//|Description:       Get the source language of a dictionary. The dictionary  |
//|                   name is stored in listbox hwndListBox and is the item    |
//|                   sItem. The source language is stored in the source       |
//|                   language array anchored in the IDA.                      |
//+----------------------------------------------------------------------------+
//|Input parameter:   PFOLPROPIDA pIda      pointer to properties dlg IDA      |
//|                   HWND    hwndListbox,  handle of dictionary name listbox  |
//|                   SHORT   sItem,        item index of requested dictionary |
//+----------------------------------------------------------------------------+
//|Returncode type:   PSZ                                                      |
//+----------------------------------------------------------------------------+
//|Returncodes:       any value      ptr to dictionary source language         |
//+----------------------------------------------------------------------------+
//|Prerequesits:      none                                                     |
//+----------------------------------------------------------------------------+
//|Side effects:      none                                                     |
//+----------------------------------------------------------------------------+
PSZ FolGetSourceLng
(
PFOLPROPIDA pIda,                    // pointer to properties dlg IDA
PSZ         pszDictName
)
{
  CHAR    szDictProp[MAX_EQF_PATH];    // buffer for property path
  EQFINFO ErrorInfo;                   // error info of property manager
  HPROP   hDictProp;                   // handle of dict properties
  PPROPDICTIONARY pDictProp;           // ptr to dictionary properties
  BOOL    fOK = TRUE;                  // internal OK flag
  PDICTLANG pDict = NULL;                     // ptr to current dict/language entry

  /********************************************************************/
  /* search dictionary in dictionary/language table                   */
  /********************************************************************/
  if ( fOK )
  {
    pDict = pIda->pDictLang;
    while ( pDict->szDict[0] && (strcmp( pDict->szDict, pszDictName ) != 0 ) )
    {
      pDict++;
    } /* endwhile */
  } /* endif */

  /********************************************************************/
  /* Build dictionary property name                                   */
  /********************************************************************/
  if ( fOK && !pDict->szDict[0] )
  {
    CHAR szShortName[MAX_FNAME];
    BOOL fIsNew = FALSE;         // folder-is-new flag
    strcpy( pIda->szBuffer2, pszDictName );
    ANSITOOEM( pIda->szBuffer2 );
    ObjLongToShortName( pIda->szBuffer2, szShortName, DICT_OBJECT, &fIsNew );
    OEMTOANSI( pIda->szBuffer2 );
    UtlMakeEQFPath( szDictProp, NULC, SYSTEM_PATH, NULL );
    strcat( szDictProp, BACKSLASH_STR );
    strcat( szDictProp, szShortName );
    strcat( szDictProp, EXT_OF_DICTPROP );
  } /* endif */

  /********************************************************************/
  /* Access dictionary properties and get source language             */
  /********************************************************************/
  if ( fOK && !pDict->szDict[0] )
  {
    hDictProp = OpenProperties( szDictProp, NULL, PROP_ACCESS_READ, &ErrorInfo);
    if ( hDictProp )
    {
      pDictProp = (PPROPDICTIONARY)MakePropPtrFromHnd( hDictProp );
      strcpy( pDict->szDict, pIda->szBuffer2 );
      strcpy( pDict->szLang, pDictProp->szSourceLang );
      CloseProperties( hDictProp, PROP_QUIT, &ErrorInfo );
    }
    else
    {
      fOK = FALSE;
    } /* endif */
  } /* endif */

  return( fOK ? pDict->szLang : EMPTY_STRING );
} /* end of function FolGetSourceLng */

//+----------------------------------------------------------------------------+
//|  Folder Language Dialog                                                    |
//+----------------------------------------------------------------------------+
INT_PTR CALLBACK FOLLANGDLGPROC
(
HWND hwndDlg,
WINMSG msg,
WPARAM mp1,
LPARAM mp2
)
{
  PFOLLANGIDA   pIda;                  // ptr to instance data area
  BOOL          fOK;                   // internal O.K. flag
  MRESULT       mResult = MRFROMSHORT(FALSE); // function return code

  switch ( msg )
  {
    case WM_EQF_QUERYID: HANDLEQUERYID( ID_FOLLANG_DLG, mp2 ); break;

    case WM_INITDLG:
      /***************************************************************/
      /* Allocate and anchor IDA                                     */
      /***************************************************************/
      UtlAlloc( (PVOID *)&pIda, 0L, (LONG)sizeof(FOLLANGIDA), ERROR_STORAGE );
      if ( !pIda )
      {
        WinDismissDlg( hwndDlg, FALSE );
        return( MRFROMSHORT(FALSE) );
      } /* endif */
      ANCHORDLGIDA( hwndDlg, pIda);

      /***************************************************************/
      /* Remember pointer to folder properties                       */
      /***************************************************************/
      pIda->pProp = (PPROPFOLDER)PVOIDFROMMP2(mp2);

      /***************************************************************/
      /* Set folder name and description                             */
      /***************************************************************/
      {
        CHAR szName[MAX_FNAME];

        Utlstrccpy( szName, pIda->pProp->PropHead.szName, DOT );
        SETTEXT( hwndDlg, ID_FOLLANG_NAME_TEXT, szName );
      }

      SetCtrlFnt(hwndDlg, GetCharSet(), ID_FOLLANG_DESCR_TEXT, 0);
      OEMTOANSI( pIda->pProp->szDescription );
      SETTEXT( hwndDlg, ID_FOLLANG_DESCR_TEXT, pIda->pProp->szDescription );
      ANSITOOEM( pIda->pProp->szDescription );

      /**************************************************************/
      /* Fill source language combobox                              */
      /**************************************************************/
      UtlFillTableLB( WinWindowFromID( hwndDlg, ID_FOLLANG_SOURCELANG_CBS ),
                      SOURCE_LANGUAGES );

      /**************************************************************/
      /* Fill target language combobox                              */
      /**************************************************************/
      UtlFillTableLB( WinWindowFromID( hwndDlg, ID_FOLLANG_TARGETLANG_CBS ),
                      ALL_TARGET_LANGUAGES );
      SETFOCUS( hwndDlg, ID_FOLLANG_SOURCELANG_CBS );


      mResult = MRFROMSHORT(TRUE);   // leave the focus where we put it
      break;

    case WM_DESTROY:
      pIda = ACCESSDLGIDA( hwndDlg, PFOLLANGIDA );
      if ( pIda )
      {
        UtlAlloc( (PVOID *)&pIda, 0L, 0L, ERROR_STORAGE );
      } /* endif */
      break;

    case WM_COMMAND:
      switch ( WMCOMMANDID(mp1,mp2) )
      {
        case DID_CANCEL:
        case ID_FOLLANG_CANCEL_PB:
          DelCtrlFont(hwndDlg, ID_FOLLANG_DESCR_TEXT );
          WinDismissDlg( hwndDlg, FALSE );
          break;

        case ID_FOLLANG_OK_PB:
          /********************************************************/
          /* Check folder source language                         */
          /********************************************************/
          fOK = TRUE;
          pIda = ACCESSDLGIDA( hwndDlg, PFOLLANGIDA );
          if ( fOK )
          {
            QUERYTEXT ( hwndDlg, ID_FOLLANG_SOURCELANG_CBS, pIda->szSource );
            if ( pIda->szSource[0] == EOS )
            {
              UtlErrorHwnd( ERROR_NO_SOURCELANG, MB_CANCEL, 0, NULL,
                            EQF_ERROR, hwndDlg);
              SETFOCUS( hwndDlg, ID_FOLLANG_SOURCELANG_CBS );
              fOK = FALSE;
            } /* endif */
          } /* endif */

          /********************************************************/
          /* Check folder target language                         */
          /********************************************************/
          if ( fOK )
          {
            QUERYTEXT ( hwndDlg, ID_FOLLANG_TARGETLANG_CBS, pIda->szTarget );
            if ( pIda->szTarget[0] == EOS )
            {
              UtlErrorHwnd( ERROR_NO_TARGETLANG, MB_CANCEL, 0, NULL,
                            EQF_ERROR, hwndDlg);
              SETFOCUS( hwndDlg, ID_FOLLANG_TARGETLANG_CBS );
              fOK = FALSE;
            } /* endif */
          } /* endif */

          /********************************************************/
          /* Check source language against target language        */
          /********************************************************/
          if ( fOK )
          {
            if ( strcmp( pIda->szSource, pIda->szTarget ) == 0 )
            {
              if ( UtlErrorHwnd( ERROR_MEM_SAME_LANGUAGES,
                                 MB_YESNO | MB_DEFBUTTON2,
                                 0, NULL, EQF_QUERY, hwndDlg ) == MBID_NO )
              {
                fOK = FALSE;
              } /* endif */
            } /* endif */
          } /* endif */

          /********************************************************/
          /* Update folder properties and leave dialog if OK      */
          /********************************************************/
          if ( fOK )
          {
            strcpy( pIda->pProp->szSourceLang, pIda->szSource );
            strcpy( pIda->pProp->szTargetLang, pIda->szTarget );
            DelCtrlFont(hwndDlg, ID_FOLLANG_DESCR_TEXT );
            WinDismissDlg( hwndDlg, TRUE );
          } /* endif */
          break;
        case ID_FOLLANG_HELP_PB:
          UtlInvokeHelp();
          break;
      } /* endswitch */
      break;

    default:
      mResult = WinDefDlgProc( hwndDlg, msg, mp1, mp2 );

  } /* endswitch */
  return( mResult );
} /* end of function FolLangDlgProc */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     FolBatchFolderCreate                                     |
//+----------------------------------------------------------------------------+
//|Function call:     FolBatchFolderCreate( pFolCrt );                         |
//+----------------------------------------------------------------------------+
//|Description:       Creates a folder in batch mode.                          |
//+----------------------------------------------------------------------------+
//|Input parameter:   PFOLCRT    pFolCrt     folder create data structure      |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       0       function completed successfully                  |
//|                   other   number of error message for given error condition|
//+----------------------------------------------------------------------------+
//|Function flow:     create invisible listbox for names of folder/memories/...|
//|                   check folder name syntax                                 |
//|                   check if folder name is unique                           |
//|                   check if folder target drive is valid                    |
//|                   check if translation memory exists                       |
//|                   check if format is valid                                 |
//|                   check if editor is valid                                 |
//|                   check if dictionary/ies exist(s)                         |
//|                   allocate folder property memory and fill-in values       |
//|                   create folder properties                                 |
//|                   create folder directory                                  |
//|                   broadcast folder-created message                         |
//|                   destroy invisible listbox                                |
//|                   cleanup                                                  |
//+----------------------------------------------------------------------------+
USHORT FolBatchFolderCreate
(
HWND             hwndParent,         // handle of folder handler window
PDDEFOLCRT       pFolCrt             // folder create data structure
)
{
  HWND             hwndLB = NULLHANDLE;// handle of invisible listbox
  SHORT            sItem;              // index of a listbox item
  PSZ              pszParm;            // pointer for error parameters
  PPROPFOLDER      pProp = NULL;       // ptr to properties of new folder
  BOOL             fPropsCreated = FALSE; // props.-have-been-created flag
  BOOL             fOK = TRUE;         // internal O.K. flag
  CHAR             szShortName[MAX_FILESPEC]; // buffer for folder short name

  /********************************************************************/
  /* Create invisible listbox for names of folder/memories/...        */
  /********************************************************************/
  hwndLB = WinCreateWindow( hwndParent, WC_LISTBOX, "", 0L, 0, 0, 0, 0,
                            hwndParent, HWND_TOP, 1, NULL, NULL );

  /********************************************************************/
  /* Check folder name syntax                                         */
  /********************************************************************/
  if ( fOK )
  {
    fOK = UtlCheckLongName( pFolCrt->szName );

    if ( !fOK )
    {
      pFolCrt->DDEReturn.usRc = ERROR_FIELD_INPUT;
      pszParm = pFolCrt->szName;
      UtlErrorHwnd( ERROR_FIELD_INPUT, MB_CANCEL, 1,
                    &pszParm, EQF_ERROR, pFolCrt->hwndErrMsg );
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* Check if folder name is unique                                   */
  /********************************************************************/
  if ( fOK )
  {
    BOOL fIsNew = FALSE;         // folder-is-new flag
    ObjLongToShortName( pFolCrt->szName, szShortName,
                        FOLDER_OBJECT, &fIsNew );
    if ( !fIsNew )
    {
      fOK = FALSE;
      pszParm = pFolCrt->szName;
      pFolCrt->DDEReturn.usRc = ERROR_NEWFOLDER_EXISTS;
      UtlErrorHwnd( pFolCrt->DDEReturn.usRc, MB_CANCEL, 1,
                    &pszParm, EQF_ERROR, pFolCrt->hwndErrMsg );
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* Check if folder target drive is valid                            */
  /********************************************************************/
  if ( fOK && (pFolCrt->chToDrive != EOS) )
  {
    CHAR szEqfDrives[MAX_DRIVELIST];   // buffer for EQF drive letters

    /******************************************************************/
    /* Get valid EQF drives                                           */
    /******************************************************************/
    UtlGetCheckedEqfDrives( szEqfDrives );

    /******************************************************************/
    /* Check if specified target drive is in list of valid drives     */
    /******************************************************************/
    if ( strchr( szEqfDrives, toupper(pFolCrt->chToDrive) ) == NULL )
    {
      CHAR szDrive[MAX_DRIVE];

      fOK = FALSE;
      szDrive[0] = pFolCrt->chToDrive;
      szDrive[1] = COLON;
      szDrive[2] = EOS;
      pszParm = szDrive;
      pFolCrt->DDEReturn.usRc = ERROR_EQF_DRIVE_NOT_VALID;
      UtlErrorHwnd( pFolCrt->DDEReturn.usRc, MB_CANCEL, 1,
                    &pszParm, EQF_ERROR, pFolCrt->hwndErrMsg );
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* Check if translation memory exists                               */
  /********************************************************************/
  if ( fOK )
  {
    /******************************************************************/
    /* Fill our listbox with the names of all TMs                     */
    /******************************************************************/
    EqfSend2Handler( MEMORYHANDLER, WM_EQF_INSERTNAMES,
                     MP1FROMHWND( hwndLB ), MP2FROMP( MEMORY_ALL ) );

    /******************************************************************/
    /* Search TM name in listbox                                      */
    /******************************************************************/
    sItem = SEARCHITEMHWND( hwndLB, pFolCrt->szMem );
    if ( sItem < 0 )
    {
      fOK = FALSE;
      pszParm = pFolCrt->szMem;
      pFolCrt->DDEReturn.usRc = ERROR_MEMORY_NOTFOUND;
      UtlErrorHwnd( pFolCrt->DDEReturn.usRc, MB_CANCEL, 1,
                    &pszParm, EQF_ERROR, pFolCrt->hwndErrMsg );
    } /* endif */

    if ( fOK )
    {
      /******************************************************************/
      /* Search names of R/O TMs in listbox                             */
      /******************************************************************/
      USHORT usI = 0;

      while ( fOK && (usI < MAX_NUM_OF_READONLY_MDB) )
      {
        if ( pFolCrt->MemTbl[usI][0] != EOS )
        {
          sItem = SEARCHITEMHWND( hwndLB, pFolCrt->MemTbl[usI] );
          if ( sItem < 0 )
          {
            fOK = FALSE;
            pszParm = pFolCrt->MemTbl[usI];
            pFolCrt->DDEReturn.usRc = ERROR_MEMORY_NOTFOUND;
            UtlErrorHwnd( pFolCrt->DDEReturn.usRc, MB_CANCEL, 1,
                          &pszParm, EQF_ERROR, pFolCrt->hwndErrMsg );
          } /* endif */
        } /* endif */
        usI++;
      } /* endwhile */
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* Check if format is valid                                         */
  /********************************************************************/
  if ( fOK )
  {
    /******************************************************************/
    /* Fill our listbox with the names of all format tables           */
    /******************************************************************/
    EqfSend2Handler( TAGTABLEHANDLER, WM_EQF_INSERTNAMES,
                     MP1FROMHWND( hwndLB ), 0L );

    /******************************************************************/
    /* Search format name in listbox                                  */
    /******************************************************************/
    sItem = SEARCHITEMHWND( hwndLB, pFolCrt->szFormat );
    if ( sItem < 0 )
    {
      fOK = FALSE;
      pszParm = pFolCrt->szFormat;
      pFolCrt->DDEReturn.usRc = ERROR_NO_FORMAT_TABLE_AVA;
      UtlErrorHwnd( pFolCrt->DDEReturn.usRc, MB_CANCEL, 1,
                    &pszParm, EQF_ERROR, pFolCrt->hwndErrMsg );
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* Check if editor is valid                                         */
  /********************************************************************/
  if ( fOK )
  {
    CHAR szSearchPath[MAX_EQF_PATH];   // buffer for search path

    /******************************************************************/
    /* Fill our listbox with the names of all editors                 */
    /******************************************************************/
    UtlMakeEQFPath( szSearchPath, NULC, PROPERTY_PATH, NULL );
    strcat( szSearchPath, BACKSLASH_STR );
    strcat( szSearchPath, DEFAULT_PATTERN_NAME );
    strcat( szSearchPath, EXT_OF_EDITOR );
    UtlLoadFileNames( szSearchPath, FILE_NORMAL, hwndLB, NAMFMT_NOEXT );

    /******************************************************************/
    /* Search editor name in listbox                                  */
    /******************************************************************/
    sItem = SEARCHITEMHWND( hwndLB, pFolCrt->szEdit );
    if ( sItem < 0 )
    {
      fOK = FALSE;
      pszParm = pFolCrt->szEdit;
      pFolCrt->DDEReturn.usRc = ERROR_EDITOR_NOT_FOUND;
      UtlErrorHwnd( pFolCrt->DDEReturn.usRc, MB_CANCEL, 1,
                    &pszParm, EQF_ERROR, pFolCrt->hwndErrMsg );
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* Check if dictionary/ies exist(s)                                 */
  /********************************************************************/
  if ( fOK )
  {
    SHORT sI;                          // loop index

    /******************************************************************/
    /* Fill our listbox with the names of all dictionaries            */
    /******************************************************************/
    EqfSend2Handler( DICTIONARYHANDLER, WM_EQF_INSERTNAMES,
                     MP1FROMHWND( hwndLB ), 0L );

    /******************************************************************/
    /* Search dictionaries in listbox                                 */
    /******************************************************************/
    sI = 0;
    while ( (pFolCrt->szDicts[sI][0] != EOS) &&
            (sI < NUM_OF_FOLDER_DICS) &&
            fOK )
    {
      sItem = SEARCHITEMHWND( hwndLB, pFolCrt->szDicts[sI] );
      if ( sItem < 0 )
      {
        fOK = FALSE;
        pszParm = pFolCrt->szDicts[sI];
        pFolCrt->DDEReturn.usRc = ERROR_DIC_NOTFOUND;
        UtlErrorHwnd( pFolCrt->DDEReturn.usRc, MB_CANCEL, 1,
                      &pszParm, EQF_ERROR, pFolCrt->hwndErrMsg );
      }
      else
      {
        // check if dictionary has been specified already
        SHORT sJ = 0;

        while ( sJ < sI )
        {
          if ( stricmp( pFolCrt->szDicts[sJ],
                        pFolCrt->szDicts[sI]) == 0 )
          {
            fOK = FALSE;
            pszParm = pFolCrt->szDicts[sI];
            pFolCrt->DDEReturn.usRc = ERROR_DIC_SELECTED;
            UtlErrorHwnd( pFolCrt->DDEReturn.usRc, MB_CANCEL, 1,
                          &pszParm, EQF_ERROR, pFolCrt->hwndErrMsg );
            break;
          } /* endif */
          sJ++;
        } /* endwhile */
      } /* endif */
      sI++;
    } /* endwhile */
  } /* endif */

  /********************************************************************/
  /* Check if source language is valid                                */
  /********************************************************************/
  if ( fOK )
  {
    SHORT sItem;                       // listbox item index

    DELETEALLHWND( hwndLB );
    UtlFillTableLB( hwndLB, SOURCE_LANGUAGES );
    sItem = ISEARCHITEMHWND( hwndLB, pFolCrt->szSourceLang );
    if ( sItem < 0 )
    {
      pszParm = pFolCrt->szSourceLang;
      pFolCrt->DDEReturn.usRc = ERROR_PROPERTY_LANG_DATA;
      UtlErrorHwnd( ERROR_PROPERTY_LANG_DATA, MB_CANCEL, 1,
                    &pszParm, EQF_ERROR, pFolCrt->hwndErrMsg );
      fOK = FALSE;
    }
    else
    {
      QUERYITEMTEXTHWND( hwndLB, sItem, pFolCrt->szSourceLang );
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* Check if target language is valid                                */
  /********************************************************************/
  if ( fOK )
  {
    SHORT sItem;                       // listbox item index

    DELETEALLHWND( hwndLB );
    UtlFillTableLB( hwndLB, TARGET_LANGUAGES );
    sItem = ISEARCHITEMHWND( hwndLB, pFolCrt->szTargetLang );
    if ( sItem < 0 )
    {
      pszParm = pFolCrt->szTargetLang;
      pFolCrt->DDEReturn.usRc = ERROR_PROPERTY_LANG_DATA;
      UtlErrorHwnd( ERROR_PROPERTY_LANG_DATA, MB_CANCEL, 1,
                    &pszParm, EQF_ERROR, pFolCrt->hwndErrMsg );
      fOK = FALSE;
    }
    else
    {
      QUERYITEMTEXTHWND( hwndLB, sItem, pFolCrt->szTargetLang );
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* Allocate folder property memory and fill-in values               */
  /********************************************************************/
  if ( fOK )
  {
    fOK =  UtlAlloc( (PVOID *)&pProp, 0L, (LONG)sizeof(*pProp), NOMSG );
    if ( fOK )
    {
      USHORT usI;                      // loop index

      UtlMakeEQFPath( pProp->PropHead.szPath, NULC, SYSTEM_PATH, NULL );
      pProp->PropHead.usClass = PROP_CLASS_FOLDER;
      pProp->PropHead.chType  = PROP_TYPE_INSTANCE;
      strcpy( pProp->PropHead.szName, szShortName );
      strcpy( pProp->szLongName, pFolCrt->szName );
      strcat( pProp->PropHead.szName, EXT_FOLDER_MAIN );

      FolSetDefaultPos( &pProp->Swp );
      FolSetDocListView(pProp);

      strcpy( pProp->szDescription, pFolCrt->szDescr );

      {
        BOOL fIsNew = FALSE;         // folder-is-new flag
        ObjLongToShortName( pFolCrt->szMem, pProp->szMemory, TM_OBJECT, &fIsNew );
        strcpy( pProp->szLongMemory, pFolCrt->szMem );
      }

      strcpy( pProp->szFormat, pFolCrt->szFormat );
      strcpy( pProp->szEditor, pFolCrt->szEdit );
      strcpy( pProp->szSourceLang, pFolCrt->szSourceLang );
      strcpy( pProp->szTargetLang, pFolCrt->szTargetLang );
      strcpy( pProp->szConversion, pFolCrt->szConversion );

      if ( pFolCrt->chToDrive == EOS )
      {
        pProp->chDrive = pProp->PropHead.szPath[0]; // use system drive
      }
      else
      {
        pProp->chDrive = pFolCrt->chToDrive;
      } /* endif */

      usI = 0;
      pProp->DicTbl[0] = EOS;
      while ( (pFolCrt->szDicts[usI][0] != EOS) &&
              (usI < NUM_OF_FOLDER_DICS) )
      {
        strcat( pProp->DicTbl, pFolCrt->szDicts[usI] );
        strcat( pProp->DicTbl, X15_STR );
        usI++;
      } /* endwhile */

      usI = 0;
      pProp->MemTbl[0] = EOS;
      while ( (pFolCrt->MemTbl[usI][0] != EOS) &&
              (usI < MAX_NUM_OF_READONLY_MDB) )
      {
        strcat( pProp->MemTbl, pFolCrt->MemTbl[usI] );
        strcat( pProp->MemTbl, X15_STR );
        usI++;
      } /* endwhile */
    }
    else
    {
      pFolCrt->DDEReturn.usRc = ERROR_STORAGE;
      UtlErrorHwnd( pFolCrt->DDEReturn.usRc, MB_CANCEL, 0,
                    NULL, EQF_ERROR, pFolCrt->hwndErrMsg );
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* Create folder properties                                         */
  /********************************************************************/
  if ( fOK )
  {
    EQFINFO         ErrorInfo;         // property handler error info
    HPROP           hProp = NULL;      // handle to properties
    CHAR            szObjName[MAX_EQF_PATH]; // buffer for folder object name

    /******************************************************************/
    /* Delete any old property file                                   */
    /******************************************************************/
    UtlMakeEQFPath( szObjName, NULC, PROPERTY_PATH, NULL );
    strcat( szObjName, BACKSLASH_STR );
    strcat( szObjName, pProp->PropHead.szName );

    UtlDelete( szObjName, 0L, FALSE );

    /******************************************************************/
    /* Create new property file                                       */
    /******************************************************************/
    strcpy( szObjName, pProp->PropHead.szPath );
    strcat( szObjName, BACKSLASH_STR );
    strcat( szObjName, pProp->PropHead.szName);
    hProp = CreateProperties( szObjName, (PSZ)NULP, PROP_CLASS_FOLDER, &ErrorInfo);
    if ( !hProp )
    {
      pszParm = pProp->PropHead.szName;
      pFolCrt->DDEReturn.usRc = ERROR_CREATE_PROPERTIES;
      UtlErrorHwnd( pFolCrt->DDEReturn.usRc, MB_CANCEL, 0,
                    NULL, EQF_ERROR, pFolCrt->hwndErrMsg );
      fOK = FALSE;
    }
    else if ( PutAllProperties( hProp, pProp, &ErrorInfo ) ||
              SaveProperties( hProp, &ErrorInfo) )
    {
      pszParm = pProp->PropHead.szName;
      pFolCrt->DDEReturn.usRc = ERROR_WRITE_PROPERTIES;
      UtlErrorHwnd( pFolCrt->DDEReturn.usRc, MB_CANCEL, 0,
                    NULL, EQF_ERROR, pFolCrt->hwndErrMsg );
      fOK = FALSE;
    }
    else
    {
      fPropsCreated = TRUE;
    } /* endif */
    if ( hProp ) CloseProperties( hProp, PROP_QUIT, &ErrorInfo );
  } /* endif */

  /********************************************************************/
  /* Create folder directory                                          */
  /********************************************************************/
  if ( fOK )
  {
    pFolCrt->DDEReturn.usRc = CreateFolderDir( pProp, FALSE );
    if ( pFolCrt->DDEReturn.usRc != 0 )
    {
      UtlErrorHwnd( pFolCrt->DDEReturn.usRc, MB_CANCEL, 0,
                    NULL, EQF_ERROR, pFolCrt->hwndErrMsg );
      fOK = FALSE;
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* Broadcast folder-created message                                 */
  /********************************************************************/
  if ( fOK )
  {
    CHAR            szObjName[MAX_EQF_PATH]; // buffer for folder object name

    UtlMakeEQFPath( szObjName, NULC, SYSTEM_PATH, NULL );
    strcat( szObjName, BACKSLASH_STR );
    strcat( szObjName, pProp->PropHead.szName );
    EqfSend2AllHandlers( WM_EQFN_CREATED, MP1FROMSHORT( clsFOLDER ),
                         MP2FROMP(szObjName) );
  } /* endif */

  /********************************************************************/
  /* Destroy invisible listbox                                        */
  /********************************************************************/
  if ( hwndLB )  WinDestroyWindow( hwndLB );

  /********************************************************************/
  /* Cleanup                                                          */
  /********************************************************************/
  if ( !fOK && fPropsCreated )
  {
    CHAR            szObjName[MAX_EQF_PATH]; // buffer for folder object name

    /******************************************************************/
    /* Delete property file in case of errors                         */
    /******************************************************************/
    UtlMakeEQFPath( szObjName, NULC, PROPERTY_PATH, NULL );
    strcat( szObjName, BACKSLASH_STR );
    strcat( szObjName, pProp->PropHead.szName );
    UtlDelete( szObjName, 0L, FALSE );
  } /* endif */
  if ( pProp ) UtlAlloc( (PVOID *)&pProp, 0L, 0L, NOMSG );

  /********************************************************************/
  /* Tell DDE handler that task has been completed                    */
  /********************************************************************/
  WinPostMsg( pFolCrt->hwndOwner, WM_EQF_DDE_ANSWER,
              NULL, &pFolCrt->DDEReturn );

  return( pFolCrt->DDEReturn.usRc );
} /* end of function FolBatchFolderCreate */

USHORT FolFuncCreateFolder
(
PSZ         pszFolderName,           // name of folder
PSZ         pszDescription,          // folder description or NULL
CHAR        chTargetDrive,           // folder target drive
PSZ         pszMemName,              // folder Translation Memory
PSZ         pszMarkup,               // folder markup
PSZ         pszEditor,               // folder editor
PSZ         pszDictionaries,         // list of dictionaries or NULL
PSZ         pszSourceLanguage,       // folder source language
PSZ         pszTargetLanguage,       // folder target language
PSZ         pszConversion,           // document export conversion
PSZ         pszReadOnlyMems          // list of readonly TMs or NULL
)
{
  PSZ              pszParm;            // pointer for error parameters
  PPROPFOLDER      pProp = NULL;       // ptr to properties of new folder
  BOOL             fPropsCreated = FALSE; // props.-have-been-created flag
  BOOL             fOK = TRUE;         // internal O.K. flag
  USHORT           usRC = NO_ERROR;    // function return code
  CHAR             szShortName[MAX_FILESPEC]; // buffer for folder short name
  BOOL             fReserved = FALSE;  // TRUE = a short name has been reserved for the folder

  // check required parameters
  if ( fOK )
  {
    if ( (pszFolderName == NULL) || (*pszFolderName == EOS) )
    {
      fOK = FALSE;
      usRC = TA_MANDFOLDER;
      UtlErrorHwnd( usRC, MB_CANCEL, 0, NULL, EQF_ERROR, HWND_FUNCIF );
    } /* endif */
  } /* endif */

  if ( fOK )
  {
    if ( (pszMemName == NULL) || (*pszMemName == EOS) )
    {
      fOK = FALSE;
      usRC = TA_MANDTM;
      UtlErrorHwnd( usRC, MB_CANCEL, 0, NULL, EQF_ERROR, HWND_FUNCIF );
    } /* endif */
  } /* endif */

  if ( fOK )
  {
    if ( (pszMarkup == NULL) || (*pszMarkup == EOS) )
    {
      fOK = FALSE;
      usRC = TA_MANDMARKUP;
      UtlErrorHwnd( usRC, MB_CANCEL, 0, NULL, EQF_ERROR, HWND_FUNCIF );
    } /* endif */
  } /* endif */

  if ( fOK )
  {
    if ( (pszSourceLanguage == NULL) || (*pszSourceLanguage == EOS) )
    {
      fOK = FALSE;
      usRC = ERROR_NO_SOURCELANG;
      UtlErrorHwnd( usRC, MB_CANCEL, 0, NULL, EQF_ERROR, HWND_FUNCIF );
    } /* endif */
  } /* endif */

  if ( fOK )
  {
    if ( (pszTargetLanguage == NULL) || (*pszTargetLanguage == EOS) )
    {
      fOK = FALSE;
      usRC = ERROR_NO_TARGETLANG;
      UtlErrorHwnd( usRC, MB_CANCEL, 0, NULL, EQF_ERROR, HWND_FUNCIF );
    } /* endif */
  } /* endif */

  // allocate folder property memory
  if ( fOK )
  {
    fOK =  UtlAlloc( (PVOID *)&pProp, 0L, (LONG)sizeof(*pProp), NOMSG );
    if ( !fOK )
    {
      fOK = FALSE;
      usRC = ERROR_STORAGE;
      UtlErrorHwnd( usRC, MB_CANCEL, 0,
                    NULL, EQF_ERROR, HWND_FUNCIF );
    } /* endif */
  } /* endif */



  // Check folder name syntax
  if ( fOK )
  {
    fOK = UtlCheckLongName( pszFolderName );
    if ( !fOK )
    {
      usRC = ERROR_FIELD_INPUT;
      pszParm = pszFolderName;
      UtlErrorHwnd( ERROR_FIELD_INPUT, MB_CANCEL, 1,
                    &pszParm, EQF_ERROR, HWND_FUNCIF );
    } /* endif */
  } /* endif */

  // Check if folder name is unique
  if ( fOK )
  {
    OBJLONGTOSHORTSTATE ObjState;

    ObjLongToShortNameEx2( pszFolderName, EOS, szShortName,
                           FOLDER_OBJECT, &ObjState, TRUE, &fReserved );
    if ( ObjState == OBJ_EXISTS_ALREADY )
    {
      fOK = FALSE;
      pszParm = pszFolderName;
      usRC = ERROR_NEWFOLDER_EXISTS;
      UtlErrorHwnd( usRC, MB_CANCEL, 1,
                    &pszParm, EQF_ERROR, HWND_FUNCIF );
    } /* endif */
  } /* endif */

  // Check if folder target drive is valid
  if ( fOK && (chTargetDrive != EOS) )
  {
    CHAR szEqfDrives[MAX_DRIVELIST];   // buffer for EQF drive letters

    /* Get valid EQF drives                                           */
    UtlGetCheckedEqfDrives( szEqfDrives );

    /* Check if specified target drive is in list of valid drives     */
    if ( strchr( szEqfDrives, toupper(chTargetDrive) ) == NULL )
    {
      CHAR szDrive[MAX_DRIVE];

      fOK = FALSE;
      szDrive[0] = chTargetDrive;
      szDrive[1] = COLON;
      szDrive[2] = EOS;
      pszParm = szDrive;
      usRC = ERROR_EQF_DRIVE_NOT_VALID;
      UtlErrorHwnd( usRC, MB_CANCEL, 1,
                    &pszParm, EQF_ERROR, HWND_FUNCIF );
    } /* endif */
  } /* endif */

  // check if TM has been specified
  if ( fOK )
  {
    if ( (pszMemName == NULL) || (*pszMemName == EOS) )
    {
      fOK = FALSE;
      UtlErrorHwnd( TMT_MANDCMDLINE, MB_CANCEL, 0, NULL,
                    EQF_ERROR, HWND_FUNCIF );
    } /* endif */
  } /* endif */

  // check if translation memory exists
  if ( fOK )
  {
    if ( !UtlCheckIfExist( pszMemName, TM_OBJECT ) )
    {
      fOK = FALSE;
      pszParm = pszMemName;
      usRC = ERROR_MEMORY_NOTFOUND;
      UtlErrorHwnd( usRC, MB_CANCEL, 1,
                    &pszParm, EQF_ERROR, HWND_FUNCIF );
    } /* endif */
  } /* endif */


  // check R/O TMs if specified
  if ( fOK && (pszReadOnlyMems != NULL) && (*pszReadOnlyMems != EOS) )
  {
    PSZ pszCurName = pszReadOnlyMems;
    PSZ pszCurEnd;
    USHORT usNum = 0;

    // skip whitespace up to begin of first TM Name
    while ( isspace(*pszCurName) ) pszCurName++;

    // loop over all TMs specified
    do
    {
      // find end of current name
      pszCurEnd = pszCurName;

      // in case of there is space in memory name, as"MINE 1409T EN-DE", 
      // the program can't process it correctly
      // so delete space checking, and then delete the end space later as following
      while ( (*pszCurEnd != ',') &&
              /*!isspace(*pszCurEnd) &&*/
              (*pszCurEnd != EOS) )
      {
        pszCurEnd++;
      } /* endwhile */

      // remove all the end space if have
      while( pszCurEnd!=pszCurName && isspace(*(pszCurEnd-1)) ) pszCurEnd--;

      // process current name
      if ( pszCurEnd != pszCurName )
      {
        CHAR chTemp = *pszCurEnd;
        *pszCurEnd = EOS;
        if ( UtlCheckIfExist( pszCurName, TM_OBJECT ) )
        {
          if (usNum < NUM_OF_READONLY_MDB)
          {
            strcpy( pProp->aLongMemTbl[usNum], pszCurName );
            usNum++;
          }
          else
          {
            // too much TMs specified, ignore this one
          } /* endif */
        }
        else
        {
          fOK = FALSE;
          pszParm = pszCurName;
          usRC = ERROR_MEMORY_NOTFOUND;
          UtlErrorHwnd( usRC, MB_CANCEL, 1,
                        &pszParm, EQF_ERROR, HWND_FUNCIF );
        } /* endif */

        // restore end character of current name
        *pszCurEnd = chTemp;
        pszCurName = pszCurEnd;
        if ( *pszCurName != EOS ) pszCurName++;
      } /* endif */

      // skip whitespacea
      while ( isspace(*pszCurName) ) pszCurName++;

      // skip delimiters
      while ( *pszCurName == ',' ) pszCurName++;

      // skip whitespacea
      while ( isspace(*pszCurName) ) pszCurName++;
    }
    while ( fOK && (*pszCurName != EOS) );

  } /* endif */

  // Check if format is valid
  if ( fOK )
  {
    if ( !UtlCheckIfExist( pszMarkup, MARKUP_OBJECT ) )
    {
      fOK = FALSE;
      pszParm = pszMarkup;
      usRC = ERROR_NO_FORMAT_TABLE_AVA;
      UtlErrorHwnd( usRC, MB_CANCEL, 1,
                    &pszParm, EQF_ERROR, HWND_FUNCIF );
    } /* endif */
  } /* endif */

  // Check if editor is valid
  if ( fOK && (pszEditor != NULL) && (*pszEditor != EOS) )
  {
    if ( !UtlCheckIfExist( pszEditor, EDITOR_OBJECT ) )
    {
      fOK = FALSE;
      pszParm = pszEditor;
      usRC = ERROR_EDITOR_NOT_FOUND;
      UtlErrorHwnd( usRC, MB_CANCEL, 1,
                    &pszParm, EQF_ERROR, HWND_FUNCIF );
    } /* endif */
  } /* endif */

  // Check if dictionary/ies exist(s)
  if ( fOK && (pszDictionaries != NULL) && (*pszDictionaries != EOS) )
  {
    PSZ pszCurName = pszDictionaries;
    PSZ pszCurEnd;
    USHORT usNum = 0;

    // skip whitespace up to begin of first Name
    while ( isspace(*pszCurName) ) pszCurName++;

    // loop over all dictionaries specified
    do
    {
      // find end of current name
      pszCurEnd = pszCurName;

      // in case of there is space in diction name, as"MINE 1409T EN-DE", 
      // the program can't process it correctly
      // so delete space checking, and then delete the end space later as following
      while ( (*pszCurEnd != ',') &&
              /*!isspace(*pszCurEnd) &&*/
              (*pszCurEnd != EOS) )
      {
        pszCurEnd++;
      } /* endwhile */

      // remove all the end space if have
      while( pszCurEnd!=pszCurName && isspace(*(pszCurEnd-1)) ) pszCurEnd--;

      // process current name
      if ( pszCurEnd != pszCurName )
      {
        CHAR chTemp = *pszCurEnd;
        *pszCurEnd = EOS;
        if ( UtlCheckIfExist( pszCurName, DICT_OBJECT ) )
        {
          if (usNum < NUM_OF_FOLDER_DICS)
          {
            strcpy( pProp->aLongDicTbl[usNum], pszCurName );
            usNum++;
          }
          else
          {
            // too much dictionaries specified, ignore this one
          } /* endif */
        }
        else
        {
          fOK = FALSE;
          pszParm = pszCurName;
          usRC = ERROR_DIC_NOTFOUND;
          UtlErrorHwnd( usRC, MB_CANCEL, 1,
                        &pszParm, EQF_ERROR, HWND_FUNCIF );
        } /* endif */

        // restore end character of current name
        *pszCurEnd = chTemp;
        pszCurName = pszCurEnd;
        if ( *pszCurName != EOS ) pszCurName++;
      } /* endif */

      // skip whitespacea
      while ( isspace(*pszCurName) ) pszCurName++;

      // skip delimiters
      while ( *pszCurName == ',' ) pszCurName++;

      // skip whitespacea
      while ( isspace(*pszCurName) ) pszCurName++;
    }
    while ( fOK && (*pszCurName != EOS) );

  } /* endif */

  /* Check if source language is valid                                */
  if ( fOK )
  {
    if ( !UtlCheckIfExist( pszSourceLanguage, SOURCE_LANGUAGE_OBJECT ) )
    {
      pszParm = pszSourceLanguage;
      usRC = ERROR_PROPERTY_LANG_DATA;
      UtlErrorHwnd( ERROR_PROPERTY_LANG_DATA, MB_CANCEL, 1,
                    &pszParm, EQF_ERROR, HWND_FUNCIF );
      fOK = FALSE;
    } /* endif */
  } /* endif */

  /* Check if target language is valid                                */
  if ( fOK )
  {
    if ( !UtlCheckIfExist( pszTargetLanguage, TARGET_LANGUAGE_OBJECT ) )
    {
      pszParm = pszTargetLanguage;
      usRC = ERROR_PROPERTY_LANG_DATA;
      UtlErrorHwnd( ERROR_PROPERTY_LANG_DATA, MB_CANCEL, 1,
                    &pszParm, EQF_ERROR, HWND_FUNCIF );
      fOK = FALSE;
    } /* endif */
  } /* endif */

  // fill folder properties
  if ( fOK )
  {
    UtlMakeEQFPath( pProp->PropHead.szPath, NULC, SYSTEM_PATH, NULL );
    pProp->PropHead.usClass = PROP_CLASS_FOLDER;
    pProp->PropHead.chType  = PROP_TYPE_INSTANCE;
    strcpy( pProp->PropHead.szName, szShortName );
    strcpy( pProp->szLongName, pszFolderName );
    strcat( pProp->PropHead.szName, EXT_FOLDER_MAIN );

    pProp->Swp.fs = EQF_SWP_SHOW | EQF_SWP_MOVE | EQF_SWP_SIZE;
    pProp->Swp.cx = 500;
    pProp->Swp.cy = 300;
    pProp->Swp.x = 30;
    pProp->Swp.y = 30;
    strcpy( pProp->szShipment, "1" );            // GQ: we always start with shipment 1

    if ( pszDescription != NULL )
    {
      strncpy( pProp->szDescription, pszDescription, sizeof(pProp->szDescription)-1 );
    } /* endif */
    {
      BOOL fIsNew;
      ObjLongToShortName( pszMemName, pProp->szMemory, TM_OBJECT, &fIsNew );
      if ( strcmp( pszMemName, pProp->szMemory ) != 0 )
      {
        strcpy( pProp->szLongMemory, pszMemName );
      } /* endif */
    }
    strcpy( pProp->szFormat, pszMarkup );
    if ( (pszEditor != NULL) && (*pszEditor != EOS) )
    {
      strcpy( pProp->szEditor, pszEditor );
    }
    else
    {
      Utlstrccpy( pProp->szEditor, EDITOR_PROPERTIES_NAME, DOT );
    } /* endif */
    strcpy( pProp->szSourceLang, pszSourceLanguage );
    strcpy( pProp->szTargetLang, pszTargetLanguage );
    if ( pszConversion != NULL )
    {
      strncpy( pProp->szConversion, pszConversion, sizeof(pProp->szConversion)-1 );
    }
    else
    {
      pProp->szConversion[0];
    } /* endif */

    if ( chTargetDrive == EOS )
    {
      pProp->chDrive = pProp->PropHead.szPath[0]; // use system drive
    }
    else
    {
      pProp->chDrive = (CHAR)toupper(chTargetDrive);
    } /* endif */
  } /* endif */

  /* Create folder properties                                         */
  if ( fOK )
  {
    EQFINFO         ErrorInfo;         // property handler error info
    HPROP           hProp = NULL;      // handle to properties
    CHAR            szObjName[MAX_EQF_PATH]; // buffer for folder object name
    HANDLE hMutexSem = NULL;

    /* Delete any old property file                                   */
    UtlMakeEQFPath( szObjName, NULC, PROPERTY_PATH, NULL );
    strcat( szObjName, BACKSLASH_STR );
    strcat( szObjName, pProp->PropHead.szName );

    GETMUTEX(hMutexSem);
    UtlDelete( szObjName, 0L, FALSE );

    /* Create new property file                                       */
    strcpy( szObjName, pProp->PropHead.szPath );
    strcat( szObjName, BACKSLASH_STR );
    strcat( szObjName, pProp->PropHead.szName);
    hProp = CreatePropertiesEx( szObjName, (PSZ)NULP, PROP_CLASS_FOLDER, &ErrorInfo, fReserved );
    RELEASEMUTEX(hMutexSem);

    if ( !hProp )
    {
      pszParm = pszFolderName;
      usRC = ERROR_CREATE_PROPERTIES;
      UtlErrorHwnd( usRC, MB_CANCEL, 0,
                    NULL, EQF_ERROR, HWND_FUNCIF );
      fOK = FALSE;
    }
    else if ( PutAllProperties( hProp, pProp, &ErrorInfo ) ||
              SaveProperties( hProp, &ErrorInfo) )
    {
      pszParm = pszFolderName;
      usRC = ERROR_WRITE_PROPERTIES;
      UtlErrorHwnd( usRC, MB_CANCEL, 0,
                    NULL, EQF_ERROR, HWND_FUNCIF );
      fOK = FALSE;
    }
    else
    {
      fPropsCreated = TRUE;
    } /* endif */
    if ( hProp ) CloseProperties( hProp, PROP_QUIT, &ErrorInfo );
  } /* endif */

  /* Create folder directory                                          */
  if ( fOK )
  {
    usRC = CreateFolderDir( pProp, FALSE );
    if ( usRC != 0 )
    {
      UtlErrorHwnd( usRC, MB_CANCEL, 0,
                    NULL, EQF_ERROR, HWND_FUNCIF );
      fOK = FALSE;
    } /* endif */
  } /* endif */

  /* Broadcast folder-created message                                 */
  if ( fOK )
  {
//    CHAR            szObjName[MAX_EQF_PATH]; // buffer for folder object name
//
//    UtlMakeEQFPath( szObjName, NULC, SYSTEM_PATH, NULL );
//    strcat( szObjName, BACKSLASH_STR );
//    strcat( szObjName, pProp->PropHead.szName );
//    EqfSend2AllHandlers( WM_EQFN_CREATED, MP1FROMSHORT( clsFOLDER ),
//                         MP2FROMP(szObjName) );
  } /* endif */

  /* Cleanup                                                          */
  if ( !fOK && fReserved )
  {
    CHAR szPropName[MAX_EQF_PATH]; // buffer for folder property file name

    UtlMakeEQFPath( szPropName, NULC, PROPERTY_PATH, NULL );
    strcat( szPropName, BACKSLASH_STR );
    strcat( szPropName, szShortName );
    strcat( szPropName, EXT_FOLDER_MAIN );
    UtlDelete( szPropName, 0, FALSE );
  } /* endif */

  if ( !fOK && fPropsCreated )
  {
    CHAR            szObjName[MAX_EQF_PATH]; // buffer for folder object name

    /* Delete property file in case of errors                         */
    UtlMakeEQFPath( szObjName, NULC, PROPERTY_PATH, NULL );
    strcat( szObjName, BACKSLASH_STR );
    strcat( szObjName, pProp->PropHead.szName );
    UtlDelete( szObjName, 0L, FALSE );
  } /* endif */

  if ( pProp ) UtlAlloc( (PVOID *)&pProp, 0L, 0L, NOMSG );

  return( usRC );
} /* end of function FolFuncCreateFolder */



// create an internal folder w/o checking and without long name handling as well
USHORT FolCreateIntFolder
(
PSZ         pszFolderName,           // name of folder
PSZ         pszDescription,          // folder description or NULL
CHAR        chTargetDrive,           // folder target drive
PSZ         pszMemName,              // folder Translation Memory or NULL
PSZ         pszMarkup,               // folder markup or NULL
PSZ         pszEditor,               // folder editor or NULL
PSZ         pszSourceLanguage,       // folder source language or NULL
PSZ         pszTargetLanguage        // folder target language or NULL
)
{
  PPROPFOLDER      pProp = NULL;       // ptr to properties of new folder
  BOOL             fPropsCreated = FALSE; // props.-have-been-created flag
  BOOL             fOK = TRUE;         // internal O.K. flag
  USHORT           usRC = NO_ERROR;    // function return code

  // allocate folder property memory
  if ( fOK )
  {
    fOK =  UtlAlloc( (PVOID *)&pProp, 0L, (LONG)sizeof(*pProp), NOMSG );
  } /* endif */

  // fill folder properties
  if ( fOK )
  {
    UtlMakeEQFPath( pProp->PropHead.szPath, NULC, SYSTEM_PATH, NULL );
    pProp->PropHead.usClass = PROP_CLASS_FOLDER;
    pProp->PropHead.chType  = PROP_TYPE_INSTANCE;
    strcpy( pProp->PropHead.szName, pszFolderName );
    strcat( pProp->PropHead.szName, EXT_FOLDER_MAIN );

    pProp->Swp.fs = EQF_SWP_SHOW | EQF_SWP_MOVE | EQF_SWP_SIZE;
    pProp->Swp.cx = 500;
    pProp->Swp.cy = 300;
    pProp->Swp.x = 30;
    pProp->Swp.y = 30;

    if ( pszDescription ) strncpy( pProp->szDescription, pszDescription, sizeof(pProp->szDescription)-1 );
    if ( pszMemName ) strncpy( pProp->szMemory, pszMemName, sizeof(pProp->szMemory)-1 );
    if ( pszMarkup ) strncpy( pProp->szFormat, pszMarkup, sizeof(pProp->szFormat)-1 );
    Utlstrccpy( pProp->szEditor, EDITOR_PROPERTIES_NAME, DOT );
    if ( pszEditor ) strncpy( pProp->szEditor, pszEditor, sizeof(pProp->szEditor)-1 );
    if ( pszEditor ) strncpy( pProp->szSourceLang, pszSourceLanguage, sizeof(pProp->szSourceLang)-1 );
    if ( pszEditor ) strncpy( pProp->szTargetLang, pszTargetLanguage, sizeof(pProp->szEditor)-1 );
    if ( chTargetDrive == EOS )
    {
      pProp->chDrive = pProp->PropHead.szPath[0]; // use system drive
    }
    else
    {
      pProp->chDrive = (CHAR)toupper(chTargetDrive);
    } /* endif */
  } /* endif */

  /* Create folder properties                                         */
  if ( fOK )
  {
    EQFINFO         ErrorInfo;         // property handler error info
    HPROP           hProp = NULL;      // handle to properties
    CHAR            szObjName[MAX_EQF_PATH]; // buffer for folder object name
    HANDLE          hMutexSem = NULL;

    /* Delete any old property file                                   */
    UtlMakeEQFPath( szObjName, NULC, PROPERTY_PATH, NULL );
    strcat( szObjName, BACKSLASH_STR );
    strcat( szObjName, pProp->PropHead.szName );
    GETMUTEX(hMutexSem);
    UtlDelete( szObjName, 0L, FALSE );

    /* Create new property file                                       */
    strcpy( szObjName, pProp->PropHead.szPath );
    strcat( szObjName, BACKSLASH_STR );
    strcat( szObjName, pProp->PropHead.szName);
    hProp = CreateProperties( szObjName, (PSZ)NULP, PROP_CLASS_FOLDER, &ErrorInfo);
    RELEASEMUTEX(hMutexSem);
    if ( !hProp )
    {
      fOK = FALSE;
    }
    else if ( PutAllProperties( hProp, pProp, &ErrorInfo ) ||
              SaveProperties( hProp, &ErrorInfo) )
    {
      fOK = FALSE;
    }
    else
    {
      fPropsCreated = TRUE;
    } /* endif */
    if ( hProp ) CloseProperties( hProp, PROP_QUIT, &ErrorInfo );
  } /* endif */

  /* Create folder directory                                          */
  if ( fOK )
  {
    usRC = CreateFolderDir( pProp, FALSE );
    if ( usRC != 0 )
    {
      fOK = FALSE;
    } /* endif */
  } /* endif */

  /* Cleanup                                                          */
  if ( !fOK && fPropsCreated )
  {
    CHAR            szObjName[MAX_EQF_PATH]; // buffer for folder object name

    /* Delete property file in case of errors                         */
    UtlMakeEQFPath( szObjName, NULC, PROPERTY_PATH, NULL );
    strcat( szObjName, BACKSLASH_STR );
    strcat( szObjName, pProp->PropHead.szName );
    UtlDelete( szObjName, 0L, FALSE );
  } /* endif */

  if ( pProp ) UtlAlloc( (PVOID *)&pProp, 0L, 0L, NOMSG );

  return( usRC );
} /* end of function FolCreateIntFolder */


USHORT FolFuncCreateControlledFolder
(
PSZ         pszFolderName,           // name of folder
PSZ         pszDescription,          // folder description or NULL
CHAR        chTargetDrive,           // folder target drive
PSZ         pszMemName,              // folder Translation Memory
PSZ         pszMarkup,               // folder markup
PSZ         pszEditor,               // folder editor
PSZ         pszDictionaries,         // list of dictionaries or NULL
PSZ         pszSourceLanguage,       // folder source language
PSZ         pszTargetLanguage,       // folder target language
PSZ         pszConversion,           // document export conversion
PSZ         pszReadOnlyMems,         // list of readonly TMs or NULL
PSZ         pszPassword,             // password
PSZ         pszProjCoordName,        // name of the project coordinator
PSZ         pszProjCoordMail,        // project coordinator's mail
PSZ         pszTranslatorName,       // name of the translator
PSZ         pszTranslatorMail,       // translator's mail
PSZ         pszProductName,          // Name of the product
PSZ         pszProductFamily,        // Product Famiily
PSZ         pszSimilarProduct,       // Similar Product Family
PSZ         pszProductDict,          // Product subject area dictionary
PSZ         pszProductMem,           // Product subject area memory
PSZ         pszPreviousVersion,      // Previous version of the product
PSZ         pszVersion,              // Version of the Product
PSZ         pszShipmentNumber        // Shipment number
)
{
  USHORT           usRC = NO_ERROR;    // function return code
  PSZ              pszParm;            // pointer for error parameters
  PPROPFOLDER      pProp = NULL;       // ptr to properties of new folder
  BOOL             fPropsCreated = FALSE; // props.-have-been-created flag
  BOOL             fOK = TRUE;         // internal O.K. flag
  CHAR             szShortName[MAX_FILESPEC]; // buffer for folder short name
  CHAR szFolderName[MAX_LONGFILESPEC];
  CHAR szMemName[MAX_LONGFILESPEC];
  CHAR szMarkup[MAX_LONGFILESPEC];
  CHAR szReadOnlyMems[MAX_FILESPEC];
  CHAR szDictionaries[MAX_LONGFILESPEC];
  CHAR            szObjName[MAX_EQF_PATH]; // buffer for folder object name

  // check required parameters
  if ( fOK )
  {
    if ( (pszFolderName == NULL) || (*pszFolderName == EOS) )
    {
      fOK = FALSE;
      usRC = TA_MANDFOLDER;
      UtlErrorHwnd( usRC, MB_CANCEL, 0, NULL, EQF_ERROR, HWND_FUNCIF );
    } /* endif */
  } /* endif */

  if ( fOK )
  {
    if ( (pszMemName == NULL) || (*pszMemName == EOS) )
    {
      fOK = FALSE;
      usRC = TA_MANDTM;
      UtlErrorHwnd( usRC, MB_CANCEL, 0, NULL, EQF_ERROR, HWND_FUNCIF );
    } /* endif */
  } /* endif */

  if ( fOK )
  {
    if ( (pszMarkup == NULL) || (*pszMarkup == EOS) )
    {
      fOK = FALSE;
      usRC = TA_MANDMARKUP;
      UtlErrorHwnd( usRC, MB_CANCEL, 0, NULL, EQF_ERROR, HWND_FUNCIF );
    } /* endif */
  } /* endif */

  if ( fOK )
  {
    if ( (pszSourceLanguage == NULL) || (*pszSourceLanguage == EOS) )
    {
      fOK = FALSE;
      usRC = ERROR_NO_SOURCELANG;
      UtlErrorHwnd( usRC, MB_CANCEL, 0, NULL, EQF_ERROR, HWND_FUNCIF );
    } /* endif */
  } /* endif */

  if ( fOK )
  {
    if ( (pszTargetLanguage == NULL) || (*pszTargetLanguage == EOS) )
    {
      fOK = FALSE;
      usRC = ERROR_NO_TARGETLANG;
      UtlErrorHwnd( usRC, MB_CANCEL, 0, NULL, EQF_ERROR, HWND_FUNCIF );
    } /* endif */
  } /* endif */

  if ( fOK )
  {
    if ( (pszPassword == NULL) || (*pszPassword == EOS) )
    {
      fOK = FALSE;
      usRC = TA_MANDPASSWORD; // WL to change
      UtlErrorHwnd( usRC, MB_CANCEL, 0, NULL, EQF_ERROR, HWND_FUNCIF );
    } /* endif */
    if ( fOK )
    {
       if (strlen(pszPassword)>6)
       {
         fOK = FALSE;
         usRC = TA_PASSWORDTOOLONG; // WL to change
         UtlErrorHwnd( usRC, MB_CANCEL, 0, NULL, EQF_ERROR, HWND_FUNCIF );
       }
    }
  } /* endif */
  // allocate folder property memory
  if ( fOK )
  {
    fOK =  UtlAlloc( (PVOID *)&pProp, 0L, (LONG)sizeof(*pProp), NOMSG );
    if ( !fOK )
    {
      fOK = FALSE;
      usRC = ERROR_STORAGE;
      UtlErrorHwnd( usRC, MB_CANCEL, 0,
                    NULL, EQF_ERROR, HWND_FUNCIF );
    } /* endif */
  } /* endif */

  // Check folder name syntax
  if ( fOK )
  {
    strcpy(szFolderName, pszFolderName);
    fOK = UtlCheckLongName( szFolderName );
    if ( !fOK )
    {
      usRC = ERROR_FIELD_INPUT;
      pszParm = pszFolderName;
      UtlErrorHwnd( ERROR_FIELD_INPUT, MB_CANCEL, 1,
                    &pszParm, EQF_ERROR, HWND_FUNCIF );
    } /* endif */
  } /* endif */

  // Check if folder name is unique
  if ( fOK )
  {
    BOOL fIsNew = FALSE;         // folder-is-new flag
    ObjLongToShortName( szFolderName, szShortName,
                        FOLDER_OBJECT, &fIsNew );
    if ( !fIsNew )
    {
      fOK = FALSE;
      pszParm = pszFolderName;
      usRC = ERROR_NEWFOLDER_EXISTS;
      UtlErrorHwnd( usRC, MB_CANCEL, 1,
                    &pszParm, EQF_ERROR, HWND_FUNCIF );
    } /* endif */
  } /* endif */

  // Check if folder target drive is valid
  if ( fOK && (chTargetDrive != EOS) )
  {
    CHAR szEqfDrives[MAX_DRIVELIST];   // buffer for EQF drive letters

    /* Get valid EQF drives                                           */
    UtlGetCheckedEqfDrives( szEqfDrives );

    /* Check if specified target drive is in list of valid drives     */
    if ( strchr( szEqfDrives, toupper(chTargetDrive) ) == NULL )
    {
      CHAR szDrive[MAX_DRIVE];

      fOK = FALSE;
      szDrive[0] = chTargetDrive;
      szDrive[1] = COLON;
      szDrive[2] = EOS;
      pszParm = szDrive;
      usRC = ERROR_EQF_DRIVE_NOT_VALID;
      UtlErrorHwnd( usRC, MB_CANCEL, 1,
                    &pszParm, EQF_ERROR, HWND_FUNCIF );
    } /* endif */
  } /* endif */

  // check if TM has been specified
  if ( fOK )
  {
    if ( (pszMemName == NULL) || (*pszMemName == EOS) )
    {
      fOK = FALSE;
      UtlErrorHwnd( TMT_MANDCMDLINE, MB_CANCEL, 0, NULL,
                    EQF_ERROR, HWND_FUNCIF );
    } /* endif */
  } /* endif */

  // check if translation memory exists
  if ( fOK )
  {
    strcpy(szMemName, pszMemName);
    if ( !UtlCheckIfExist( szMemName, TM_OBJECT ) )
    {
      fOK = FALSE;
      pszParm = pszMemName;
      usRC = ERROR_MEMORY_NOTFOUND;
      UtlErrorHwnd( usRC, MB_CANCEL, 1,
                    &pszParm, EQF_ERROR, HWND_FUNCIF );
    } /* endif */
  } /* endif */
  // check R/O TMs if specified
  if ( fOK && (pszReadOnlyMems != NULL) && (*pszReadOnlyMems != EOS) )
  {
    PSZ pszCurName;
    PSZ pszCurEnd;
    USHORT usNum = 0;

    strcpy(szReadOnlyMems, pszReadOnlyMems);
    pszCurName = szReadOnlyMems;

    // skip whitespace up to begin of first TM Name
    while ( isspace(*pszCurName) ) pszCurName++;

    // loop over all TMs specified
    do
    {
      // find end of current name
      pszCurEnd = pszCurName;

      // in case of there is space in mem name, as"MINE 1409T EN-DE", 
      // the program can't process it correctly
      // so delete space checking, and then delete the end space later as following
      while ( (*pszCurEnd != ',') &&
              /*!isspace(*pszCurEnd) &&*/
              (*pszCurEnd != EOS) )
      {
        pszCurEnd++;
      } /* endwhile */

      // remove all the end space if have
      while( pszCurEnd!=pszCurName && isspace(*(pszCurEnd-1)) ) pszCurEnd--;

      // process current name
      if ( pszCurEnd != pszCurName )
      {
        CHAR chTemp = *pszCurEnd;
        *pszCurEnd = EOS;
        if ( UtlCheckIfExist( pszCurName, TM_OBJECT ) )
        {
          if (usNum < MAX_NUM_OF_READONLY_MDB)
          {
            strcpy( &(pProp->aLongMemTbl[usNum][0]), pszCurName );
            usNum++;
          }
          else
          {
            // too much TMs specified, ignore this one
          } /* endif */
        }
        else
        {
          fOK = FALSE;
          pszParm = pszCurName;
          usRC = ERROR_MEMORY_NOTFOUND;
          UtlErrorHwnd( usRC, MB_CANCEL, 1,
                        &pszParm, EQF_ERROR, HWND_FUNCIF );
        } /* endif */

        // restore end character of current name
        *pszCurEnd = chTemp;
        pszCurName = pszCurEnd;
        if ( *pszCurName != EOS ) pszCurName++;
      } /* endif */

      // skip whitespacea
      while ( isspace(*pszCurName) ) pszCurName++;

      // skip delimiters
      while ( *pszCurName == ',' ) pszCurName++;

      // skip whitespacea
      while ( isspace(*pszCurName) ) pszCurName++;
    }
    while ( fOK && (*pszCurName != EOS) );

  } /* endif */

  // Check if format is valid
  if ( fOK )
  {
    strcpy(szMarkup, pszMarkup);
    if ( !UtlCheckIfExist( szMarkup, MARKUP_OBJECT ) )
    {
      fOK = FALSE;
      pszParm = pszMarkup;
      usRC = ERROR_NO_FORMAT_TABLE_AVA;
      UtlErrorHwnd( usRC, MB_CANCEL, 1,
                    &pszParm, EQF_ERROR, HWND_FUNCIF );
    } /* endif */
  } /* endif */

  // Check if editor is valid
  if ( fOK && (pszEditor != NULL) && (*pszEditor != EOS) )
  {
    if ( !UtlCheckIfExist( pszEditor, EDITOR_OBJECT ) )
    {
      fOK = FALSE;
      pszParm = pszEditor;
      usRC = ERROR_EDITOR_NOT_FOUND;
      UtlErrorHwnd( usRC, MB_CANCEL, 1,
                    &pszParm, EQF_ERROR, HWND_FUNCIF );
    } /* endif */
  } /* endif */

  // Check if dictionary/ies exist(s)
  if ( fOK && (pszDictionaries != NULL) && (*pszDictionaries != EOS) )
  {
    PSZ pszCurName;
    PSZ pszCurEnd;
    USHORT usNum = 0;

    strcpy(szDictionaries, pszDictionaries);
    pszCurName = szDictionaries;

    // skip whitespace up to begin of first Name
    while ( isspace(*pszCurName) ) pszCurName++;

    // loop over all dictionaries specified
    do
    {
      // find end of current name
      pszCurEnd = pszCurName;
      
      // in case of there is space in diction name, as"MINE 1409T EN-DE", 
      // the program can't process it correctly
      // so delete space checking, and then delete the end space later as following
      while ( (*pszCurEnd != ',') &&
              /*!isspace(*pszCurEnd) &&*/
              (*pszCurEnd != EOS) )
      {
        pszCurEnd++;
      } /* endwhile */

      // remove all the end space if have
      while( pszCurEnd!=pszCurName && isspace(*(pszCurEnd-1)) ) pszCurEnd--;

      // process current name
      if ( pszCurEnd != pszCurName )
      {
        CHAR chTemp = *pszCurEnd;
        *pszCurEnd = EOS;
        if ( UtlCheckIfExist( pszCurName, DICT_OBJECT ) )
        {
          if (usNum < NUM_OF_FOLDER_DICS)
          {
            strcpy( pProp->aLongDicTbl[usNum], pszCurName );
            usNum++;
          }
          else
          {
            // too much dictionaries specified, ignore this one
          } /* endif */
        }
        else
        {
          fOK = FALSE;
          pszParm = pszCurName;
          usRC = ERROR_DIC_NOTFOUND;
          UtlErrorHwnd( usRC, MB_CANCEL, 1,
                        &pszParm, EQF_ERROR, HWND_FUNCIF );
        } /* endif */

        // restore end character of current name
        *pszCurEnd = chTemp;
        pszCurName = pszCurEnd;
        if ( *pszCurName != EOS ) pszCurName++;
      } /* endif */

      // skip whitespacea
      while ( isspace(*pszCurName) ) pszCurName++;

      // skip delimiters
      while ( *pszCurName == ',' ) pszCurName++;

      // skip whitespacea
      while ( isspace(*pszCurName) ) pszCurName++;
    }
    while ( fOK && (*pszCurName != EOS) );

  } /* endif */

  /* Check if source language is valid                                */
  if ( fOK )
  {
    if ( !UtlCheckIfExist( pszSourceLanguage, SOURCE_LANGUAGE_OBJECT ) )
    {
      pszParm = pszSourceLanguage;
      usRC = ERROR_PROPERTY_LANG_DATA;
      UtlErrorHwnd( ERROR_PROPERTY_LANG_DATA, MB_CANCEL, 1,
                    &pszParm, EQF_ERROR, HWND_FUNCIF );
      fOK = FALSE;
    } /* endif */
  } /* endif */

  /* Check if target language is valid                                */
  if ( fOK )
  {
    if ( !UtlCheckIfExist( pszTargetLanguage, TARGET_LANGUAGE_OBJECT ) )
    {
      pszParm = pszTargetLanguage;
      usRC = ERROR_PROPERTY_LANG_DATA;
      UtlErrorHwnd( ERROR_PROPERTY_LANG_DATA, MB_CANCEL, 1,
                    &pszParm, EQF_ERROR, HWND_FUNCIF );
      fOK = FALSE;
    } /* endif */
  } /* endif */

  // fill folder properties
  if ( fOK )
  {
    UtlMakeEQFPath( pProp->PropHead.szPath, NULC, SYSTEM_PATH, NULL );
    pProp->PropHead.usClass = PROP_CLASS_FOLDER;
    pProp->PropHead.chType  = PROP_TYPE_INSTANCE;
    strcpy( pProp->PropHead.szName, szShortName );
    strcpy( pProp->szLongName, szFolderName );
    strcat( pProp->PropHead.szName, EXT_FOLDER_MAIN );

    pProp->Swp.fs = EQF_SWP_SHOW | EQF_SWP_MOVE | EQF_SWP_SIZE;
    pProp->Swp.cx = 500;
    pProp->Swp.cy = 300;
    pProp->Swp.x = 30;
    pProp->Swp.y = 30;

    if ( pszDescription != NULL )
    {
      strcpy( pProp->szDescription, pszDescription );
    } /* endif */

    {
      BOOL fIsNew;
      ObjLongToShortName( szMemName, pProp->szMemory, TM_OBJECT, &fIsNew );
      if ( strcmp( szMemName, pProp->szMemory ) != 0 )
      {
        strcpy( pProp->szLongMemory, szMemName );
      } /* endif */
    }
    strcpy( pProp->szFormat, szMarkup );
    if ( (pszEditor != NULL) && (*pszEditor != EOS) )
    {
      strcpy( pProp->szEditor, pszEditor );
    }
    else
    {
      Utlstrccpy( pProp->szEditor, EDITOR_PROPERTIES_NAME, DOT );
    } /* endif */
    strcpy( pProp->szSourceLang, pszSourceLanguage );
    strcpy( pProp->szTargetLang, pszTargetLanguage );
    if ( pszConversion != NULL )
    {
      strncpy( pProp->szConversion, pszConversion, sizeof(pProp->szConversion)-1 );
    }
    else
    {
      pProp->szConversion[0];
    } /* endif */

    if ( chTargetDrive == EOS )
    {
      pProp->chDrive = pProp->PropHead.szPath[0]; // use system drive
    }
    else
    {
      pProp->chDrive = (CHAR)toupper(chTargetDrive);
    } /* endif */

    // new stuff for controlled folder handling

    pProp->fTCMasterFolder = TRUE;
    pProp->fAFCFolder = TRUE;

    if ( pszPassword != NULL )
    {
      strcpy( pProp->szAFCPassword, pszPassword );
    } /* endif */

    if ( pszProjCoordName != NULL )
    {
      strcpy( pProp->szCoordinator, pszProjCoordName );
    } /* endif */

    if ( pszProjCoordMail != NULL )
    {
      strcpy( pProp->szCoordinatorEMail, pszProjCoordMail );
    } /* endif */

    if ( pszTranslatorName != NULL )
    {
      strcpy( pProp->szVendor, pszTranslatorName );
    } /* endif */

    if ( pszTranslatorMail != NULL )
    {
      strcpy( pProp->szVendorEMail, pszTranslatorMail );
    } /* endif */

    if ( pszProductName != NULL )
    {
      strcpy( pProp->szProduct, pszProductName );
    } /* endif */

    if ( pszProductFamily != NULL )
    {
      strcpy( pProp->szProductFamily, pszProductFamily );
    } /* endif */

    if ( pszSimilarProduct != NULL )
    {
      strcpy( pProp->szSimilarProduct, pszSimilarProduct );
    } /* endif */

    if ( pszProductDict != NULL )
    {
      strcpy( pProp->szSubjectDict, pszProductDict );
    } /* endif */

    if ( pszProductMem != NULL )
    {
      strcpy( pProp->szSubjectMem, pszProductMem );
    } /* endif */

    if ( pszPreviousVersion != NULL )
    {
      strcpy( pProp->szPrevVersion, pszPreviousVersion );
    } /* endif */

    if ( pszVersion != NULL )
    {
      strcpy( pProp->szVersion, pszVersion );
    } /* endif */

    if ( pszShipmentNumber != NULL )
    {
      strcpy( pProp->szShipment, pszShipmentNumber );
    }
    else
    {
      strcpy( pProp->szShipment, "1" );
    } /* endif */
  } /* endif Fill Folder Properties*/

  /* Create folder properties                                         */
  if ( fOK )
  {
    EQFINFO         ErrorInfo;         // property handler error info
    HPROP           hProp = NULL;      // handle to properties
    HANDLE hMutexSem = NULL;
    /* Delete any old property file                                   */
    UtlMakeEQFPath( szObjName, NULC, PROPERTY_PATH, NULL );
    strcat( szObjName, BACKSLASH_STR );
    strcat( szObjName, pProp->PropHead.szName );

    GETMUTEX(hMutexSem);
    UtlDelete( szObjName, 0L, FALSE );

    /* Create new property file                                       */
    strcpy( szObjName, pProp->PropHead.szPath );
    strcat( szObjName, BACKSLASH_STR );
    strcat( szObjName, pProp->PropHead.szName);
    hProp = CreateProperties( szObjName, (PSZ)NULP, PROP_CLASS_FOLDER, &ErrorInfo);
    RELEASEMUTEX(hMutexSem);

    if ( !hProp )
    {
      pszParm = szFolderName;
      usRC = ERROR_CREATE_PROPERTIES;
      UtlErrorHwnd( usRC, MB_CANCEL, 0,
                    NULL, EQF_ERROR, HWND_FUNCIF );
      fOK = FALSE;
    }
    else if ( PutAllProperties( hProp, pProp, &ErrorInfo ) ||
              SaveProperties( hProp, &ErrorInfo) )
    {
      pszParm = szFolderName;
      usRC = ERROR_WRITE_PROPERTIES;
      UtlErrorHwnd( usRC, MB_CANCEL, 0,
                    NULL, EQF_ERROR, HWND_FUNCIF );
      fOK = FALSE;
    }
    else
    {
      fPropsCreated = TRUE;
    } /* endif */
    if ( hProp ) CloseProperties( hProp, PROP_QUIT, &ErrorInfo );
  } /* endif */

  /* Create folder directory                                          */
  if ( fOK )
  {
    usRC = CreateFolderDir( pProp, FALSE );
    if ( usRC != 0 )
    {
      UtlErrorHwnd( usRC, MB_CANCEL, 0,
                    NULL, EQF_ERROR, HWND_FUNCIF );
      fOK = FALSE;
    } /* endif */
  } /* endif */

  /* Broadcast folder-created message                                 */
  if ( fOK )
  {
//    CHAR            szObjName[MAX_EQF_PATH]; // buffer for folder object name
//
//    UtlMakeEQFPath( szObjName, NULC, SYSTEM_PATH, NULL );
//    strcat( szObjName, BACKSLASH_STR );
//    strcat( szObjName, pProp->PropHead.szName );
//    EqfSend2AllHandlers( WM_EQFN_CREATED, MP1FROMSHORT( clsFOLDER ),
//                         MP2FROMP(szObjName) );
  } /* endif */

  // write shipment record to history log
  if ( fOK )
  {
    if ( pszShipmentNumber && *pszShipmentNumber )
    {
      FOLPROPHISTSHIPMENT FolPropHistShipment;     // history record for folder

      strcpy( FolPropHistShipment.szShipment, pszShipmentNumber );

      EQFBWriteHistLog2( szObjName, "", FOLPROPSHIPMENT_LOGTASK, sizeof(FOLPROPHISTSHIPMENT),
                        (PVOID)&FolPropHistShipment, FALSE, NULLHANDLE, NULL );
    } /* endif */
  } /* endif */

  /* Cleanup                                                          */
  if ( !fOK && fPropsCreated )
  {
    CHAR            szObjName[MAX_EQF_PATH]; // buffer for folder object name

    /* Delete property file in case of errors                         */
    UtlMakeEQFPath( szObjName, NULC, PROPERTY_PATH, NULL );
    strcat( szObjName, BACKSLASH_STR );
    strcat( szObjName, pProp->PropHead.szName );
    UtlDelete( szObjName, 0L, FALSE );
  } /* endif */

  if ( pProp ) UtlAlloc( (PVOID *)&pProp, 0L, 0L, NOMSG );

  return( usRC );
} /* end of function FolFuncCreateControlledFolder */


USHORT FolFuncChangeFolProps
(
  PSZ         pszFolderName,           // name of the folder
  CHAR        chTargetDrive,           // target drive
  PSZ         pszTargetLanguage,       // target language or NULL  
  PSZ         pszMemName,              // folder Translation Memory or NULL
  PSZ         pszDictionaries,         // list of dictionaries or NULL
  PSZ         pszROMemories,           // list of read-only search memories or NULL
  PSZ         pszDescription,          // folder description or NULL
  PSZ         pszProfile,              // profile name or NULL
  PSZ         pszShipment              // shipment number or NULL
)
{
  USHORT           usRC = NO_ERROR;        // function return code
  PSZ              pszParm;                // pointer for error parameters
  BOOL             fOK = TRUE;             // internal O.K. flag
  CHAR             szShortName[MAX_FILESPEC]; // buffer for folder short name
  BOOL             fFolExists = TRUE;
  BOOL             fIsSubFolder = FALSE;
  HPROP            hProp = NULL;
  PPROPFOLDER      pFolProp = NULL;
  EQFINFO          ErrorInfo;
  USHORT           usErrorID;
  CHAR szFolPath[MAX_EQF_PATH];
  CHAR szFolderName[MAX_LONGFILESPEC];
  CHAR szMemoryName[MAX_LONGFILESPEC];
  CHAR szTargetLanguage[MAX_LONGFILESPEC];
  CHAR szDictionaries[MAX_LONGFILESPEC];
  CHAR szObjName[MAX_LONGFILESPEC];

  szMemoryName[0] = EOS;
  szTargetLanguage[0] = EOS;
  szDictionaries[0] = EOS;

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

      strcpy(szFolderName, pszFolderName);
      fOK = UtlCheckLongName( szFolderName );
      if (!fOK)
      {
           fOK = SubFolNameToObjectName( pszFolderName,  szObjName);
           if ( fOK )
           {
               strcpy(pszFolderName,szObjName);
               strcpy(szFolderName,szObjName);
               strcpy(szFolPath,szObjName);
               fIsSubFolder = TRUE;
           }
      }
      if ( !fOK )
      {
        usRC = ERROR_FIELD_INPUT;
        pszParm = szFolderName;
        UtlErrorHwnd( ERROR_FIELD_INPUT, MB_CANCEL, 1,
                      &pszParm, EQF_ERROR, HWND_FUNCIF );
      } /* endif */

      if ( fOK && !fIsSubFolder )
      {
        // UtlStripBlancs function is done in ObjLongToShortName
        ObjLongToShortName( szFolderName, szShortName, FOLDER_OBJECT, &fFolExists );
        if ( fFolExists )
        {
          fOK = FALSE;
          pszParm = szFolderName;
          usRC = ERROR_XLATE_FOLDER_NOT_EXIST;
          UtlErrorHwnd( usRC, MB_CANCEL, 1, &pszParm, EQF_ERROR, HWND_FUNCIF );
        } /* endif */
      } // end if fOK
    } /* endif */
  } /* endif fOK */

  // Check if folder target drive is valid
  if ( fOK && (chTargetDrive != EOS) )
  {
    CHAR szEqfDrives[MAX_DRIVELIST];   // buffer for EQF drive letters

    /* Get valid EQF drives                                           */
    UtlGetCheckedEqfDrives( szEqfDrives );

    /* Check if specified target drive is in list of valid drives     */
    if ( strchr( szEqfDrives, toupper(chTargetDrive) ) == NULL )
    {
      CHAR szDrive[MAX_DRIVE];

      fOK = FALSE;
      szDrive[0] = chTargetDrive;
      szDrive[1] = COLON;
      szDrive[2] = EOS;
      pszParm = szDrive;
      usRC = ERROR_EQF_DRIVE_NOT_VALID;
      UtlErrorHwnd( usRC, MB_CANCEL, 1,
                    &pszParm, EQF_ERROR, HWND_FUNCIF );
    } /* endif */
  } /* endif */

  // Check if given target language is valid
  if ( fOK )
  {
    if ( !(pszTargetLanguage == NULL) && !(*pszTargetLanguage == EOS) )
    {
      strcpy(szTargetLanguage, pszTargetLanguage);
      if ( !UtlCheckIfExist( szTargetLanguage, TARGET_LANGUAGE_OBJECT ) )
      {
        pszParm = szTargetLanguage;
        usRC = (USHORT)ERROR_PROPERTY_LANG_DATA;
        UtlErrorHwnd( ERROR_PROPERTY_LANG_DATA, MB_CANCEL, 1,
                      &pszParm, EQF_ERROR, HWND_FUNCIF );
        fOK = FALSE;
      } /* endif */
    } /* endif */
  } /* endif */

  // check if TM has been specified
  if ( fOK )
  {
    if (!(pszMemName == NULL) && !(*pszMemName == EOS) )
    {
      strcpy(szMemoryName, pszMemName);
      if ( !UtlCheckIfExist( szMemoryName, TM_OBJECT ) )
      {
        fOK = FALSE;
        pszParm = szMemoryName;
        usRC = ERROR_MEMORY_NOTFOUND;
        UtlErrorHwnd( usRC, MB_CANCEL, 1,
                      &pszParm, EQF_ERROR, HWND_FUNCIF );
      } /* endif UtlCheckIfExist*/
    } /* endif !== NULL*/
  } // end if fOK

  // Check if dictionary/ies exist(s)
  if ( fOK && (pszDictionaries != NULL) && (*pszDictionaries != EOS) )
  {
    usRC = FolFuncCheckObjects( pszDictionaries, DICT_OBJECT );
    fOK = (usRC == 0);
  }

  // Check if r/o memories exist(s)
  if ( fOK && (pszROMemories != NULL) && (*pszROMemories != EOS) )
  {
    PSZ pszTemp = (*pszROMemories == '+') ? (pszROMemories + 1) : pszROMemories;
    usRC = FolFuncCheckObjects( pszTemp, TM_OBJECT );

    fOK = (usRC == 0);
  }

  if( !fIsSubFolder )
  {
    PPROPSYSTEM   pSysProp;

    pSysProp = (PPROPSYSTEM)MakePropPtrFromHnd( EqfQuerySystemPropHnd());
    UtlMakeEQFPath( szFolPath, pSysProp->szPrimaryDrive[0], SYSTEM_PATH, szShortName );
    strcat( szFolPath, EXT_FOLDER_MAIN );
  }
  // fill folder properties

  if ( fOK )
  {
    if ( !fIsSubFolder )
    {
      hProp = OpenProperties( szFolPath, NULL, PROP_ACCESS_READ, &ErrorInfo);
      if (!hProp)
      {
        pszParm = szFolderName;
        usRC = ERROR_OPEN_PROPERTIES;
        UtlErrorHwnd( usRC, MB_OK, 1, &pszParm,
                      EQF_ERROR, HWND_FUNCIF );
        fOK = FALSE;
      }
      else
      {
        fOK = SetPropAccess( hProp, PROP_ACCESS_WRITE);
        if ( fOK )
        {
          pFolProp = (PPROPFOLDER)MakePropPtrFromHnd( hProp );
        }
      } // end if !hProp
    }
    else
    {
      ULONG ulBytesRead = 0;

      fOK = UtlLoadFileL( szFolPath, (PVOID *)&pFolProp, &ulBytesRead, FALSE, TRUE );
    }
  }

  if ( fOK && pFolProp )
  {
    if ( strcmp( pFolProp->szSourceLang, szTargetLanguage ) == 0 )
    {
      if ( UtlErrorHwnd( ERROR_MEM_SAME_LANGUAGES,
                         MB_YESNO | MB_DEFBUTTON2,
                         0, NULL, EQF_QUERY, HWND_FUNCIF ) == MBID_NO )
      {
        usErrorID = ID_FOLNEW_TARGETLANG_CBS;
      } /* endif */
    } /* endif */


    if ( szTargetLanguage[0] != EOS) strcpy( pFolProp->szTargetLang, szTargetLanguage );

    if (!(pszMemName == NULL) && !(*pszMemName == EOS) )
    {
      BOOL fIsNew;
      ObjLongToShortName( szMemoryName, pFolProp->szMemory, TM_OBJECT, &fIsNew );
      if ( strcmp( szMemoryName, pFolProp->szMemory ) != 0 )
      {
        strcpy( pFolProp->szLongMemory, szMemoryName );
      }
      else
      {
        pFolProp->szLongMemory[0] = EOS;
      } /* endif */
    }

    if ( fOK && !fIsSubFolder && (pszDictionaries != NULL) )
    {
      FolFuncCopyObjectsToProps( pszDictionaries, DICT_OBJECT, pFolProp );
    } // endif OK

    if ( fOK && !fIsSubFolder && (pszROMemories != NULL) )
    {
      FolFuncCopyObjectsToProps( pszROMemories, TM_OBJECT, pFolProp );
    } // endif OK

    if ( fOK && !fIsSubFolder && (pszDescription != NULL) )
    {
      memset( pFolProp->szDescription, 0, sizeof(pFolProp->szDescription) );
      strncpy( pFolProp->szDescription, pszDescription, sizeof(pFolProp->szDescription) - 1 );
    } // endif OK

    if ( fOK && !fIsSubFolder && (pszProfile != NULL) )
    {
      memset( pFolProp->szProfile, 0, sizeof(pFolProp->szProfile) );
      strncpy( pFolProp->szProfile, pszProfile, sizeof(pFolProp->szProfile) - 5 );
      strcat( pFolProp->szProfile, ".R00" );
    } // endif OK

    if ( fOK && !fIsSubFolder && (pszShipment != NULL) )
    {
      memset( pFolProp->szShipment, 0, sizeof(pFolProp->szShipment) );
      strncpy( pFolProp->szShipment, pszShipment, sizeof(pFolProp->szShipment) - 1 );
    } // endif OK


    if (fOK)
    {
      if (!fIsSubFolder)
      {
        SaveProperties( hProp, &ErrorInfo );
        ResetPropAccess( hProp, PROP_ACCESS_WRITE);
        CloseProperties( hProp, PROP_FILE, &ErrorInfo);
      } else
      {
        UtlWriteFile(szFolPath, sizeof(PROPFOLDER), pFolProp, FALSE);
        UtlAlloc( (PVOID *)&pFolProp,0L,0L,NOMSG);
      }
    } /* endif fOK*/
  }
  return( usRC );
}// end function


USHORT FolFuncGetFolProps
(
  PSZ         pszFolderName,           // name of the folder
  PEXTFOLPROP pExtFolProps             // structure to fill in folder properties
)
{
  PPROPFOLDER      pProp = NULL;       // pointer to folder properties
  USHORT           usRC = NO_ERROR;    // function return code
  CHAR             szSysDrive[MAX_DRIVE]; // buffer for system drive
  CHAR             szFolderName[MAX_LONGFILESPEC];
  CHAR             szObjName[MAX_LONGFILESPEC];
  CHAR             szShortName[MAX_FILESPEC];
  PSZ              pszPropName = NULL; // ptr to property file name
  PSZ              pszObjName = NULL;  // ptr to object name
  BOOL             fOK = TRUE;         // success indicator
  BOOL             fSubFolder = FALSE; // TRUE = object is a subfolder
  BOOL             fDone = FALSE;      // completed flag
  BOOL             fFolExists = FALSE;
  int              i=0;

// Check if folder name is valid

  if (fOK)
  {
// allocate buffer for property file names
      fOK = UtlAlloc( (PVOID *) &pszObjName, 0L, (LONG)(2 * MAX_LONGPATH), NOMSG );
      if ( !fOK )
      {
        UtlErrorHwnd( ERROR_STORAGE, MB_CANCEL, 0, NULL, EQF_ERROR, HWND_FUNCIF );
        usRC = ERROR_STORAGE;
      } /* endif */
  }

  if ( (pszFolderName == NULL) || (*pszFolderName == EOS) )
  {
    fOK = FALSE;
    usRC = TA_MANDFOLDER;
    UtlErrorHwnd( usRC, MB_CANCEL, 0, NULL, EQF_ERROR, HWND_FUNCIF );
  }
  else
  {
    strcpy(szFolderName,pszFolderName);
    ObjLongToShortName( szFolderName, szShortName, FOLDER_OBJECT, &fFolExists );
    if(!fFolExists)
    {
      UtlMakeEQFPath( szObjName, SYSTEM_PATH, NULC, NULL );
      strcpy(pszObjName,szObjName);
      strcat( pszObjName, BACKSLASH_STR );
      strcat( pszObjName, szShortName );
      strcat( pszObjName, EXT_FOLDER_MAIN );
    }
    else
    {
      fOK = UtlCheckLongName( szFolderName );
      if (!fOK)
      {
        fOK = SubFolNameToObjectName( szFolderName,  szObjName);
        // continue and check later if it is subfolder
        if ( fOK )
        {
          strcpy(pszObjName,szObjName);
        }
      }
    }
  }

  if (fOK)
  {
/********************************************************************/
/* Preset caller's buffers                                          */
/********************************************************************/
      if(pExtFolProps)
      {
        pExtFolProps->chDrive = EOS;
        pExtFolProps->szTargetLang [0] = EOS;
        pExtFolProps->szRWMemory[0] = EOS;

        for ( i=0; i<MAX_NUM_OF_READONLY_MDB; i++ )
        {
          pExtFolProps->szROMemTbl[i][0] = EOS;
        }
        for ( i=0; i<NUM_OF_FOLDER_DICS; i++)
        {
          pExtFolProps->szDicTbl[i][0] = EOS;
        }
      }
  }
// loop over parent subfolder/folder chain until all required data can be supplied
  if ( fOK )
  {
    // start with supplied folder/subfolder object name
    pszPropName = pszObjName + MAX_LONGPATH;
    //strcpy( pszObjName, pszFolderName );

    do
    {
      fSubFolder = FolIsSubFolderObject( pszObjName );

      if ( fSubFolder )
      {
        // object name is already the fully qualified property file name
        strcpy( pszPropName, pszObjName );
      }
      else
      {
        // setup folder property file name
        PSZ pszName = strrchr( pszObjName, BACKSLASH );
        if ( pszName )
        {
          *pszName = EOS;
          UtlQueryString( QST_PRIMARYDRIVE, szSysDrive, sizeof(szSysDrive) );
          sprintf( pszPropName, "%c%s\\%s\\%s", szSysDrive[0], pszObjName + 1, PROPDIR, pszName + 1 );
          *pszName = BACKSLASH;
        }
        else
        {
          fOK = FALSE;
        } /* endif */
      } /* endif */

      // load property file
      if ( fOK )
      {
        ULONG  ulLen;
        fOK = UtlLoadFileL( pszPropName, (PVOID *)&pProp, &ulLen, FALSE, FALSE );
      } /* endif */

      // do any error handling
      if ( !fOK )
      {
        UtlErrorHwnd( ERROR_PROPERTY_ACCESS, MB_CANCEL, 1, &pszFolderName,
                        EQF_ERROR, HWND_FUNCIF );
        usRC = ERROR_PROPERTY_ACCESS;
      } /* endif */

      // complete caller data with data from properties
      if ( fOK )
      {
        fDone = TRUE;                // assume all info can be provided ...

        if ( pExtFolProps->chDrive == EOS )
        {
          if ( pProp->chDrive )
          {
            pExtFolProps->chDrive = pProp->chDrive;
          }
          else
            fDone = FALSE;
        }

        if ( pExtFolProps->szRWMemory != NULL && pExtFolProps->szRWMemory[0] == EOS)
        {
          if ( pProp->szLongMemory[0] != EOS )
            strcpy( pExtFolProps->szRWMemory, pProp->szLongMemory );
          else if ( pProp->szMemory[0] != EOS )
            strcpy( pExtFolProps->szRWMemory, pProp->szMemory );
          else
            fDone = FALSE;
        } /* endif */

        if ( pExtFolProps->szTargetLang != NULL && pExtFolProps->szTargetLang[0] == EOS)
        {
          if(pProp->szTargetLang != EOS)
            strcpy( pExtFolProps->szTargetLang, pProp->szTargetLang);
          else
            fDone = FALSE;
        } /* endif */

        for (i=0;i<MAX_NUM_OF_READONLY_MDB;i++)
        {
          if(pExtFolProps->szROMemTbl != NULL && pExtFolProps->szROMemTbl[i][0] == EOS)
          {
            if( pProp->aLongMemTbl[i][0] != EOS )
              strcpy(pExtFolProps->szROMemTbl[i],pProp->aLongMemTbl[i]);
            else
              fDone = FALSE;
          }
        }

        for ( i=0; i<NUM_OF_DICTS; i++)
        {
          if (pExtFolProps->szDicTbl != NULL && pExtFolProps->szDicTbl[i][0] == EOS)
          {
            if ( pProp->aLongDicTbl[i][0] != EOS )
              strcpy(pExtFolProps->szDicTbl[i],pProp->aLongDicTbl[i]);
            else
              fDone = FALSE;
          }
        }
      } /* endif */

      // prepare data lookup from parent if necessary
      if ( fOK && !fDone )
      {
        if ( !fSubFolder )
        {
          fDone = TRUE;                // no parent for folders available
        }
        else
        {
          if ( pProp->ulParentFolder == 0 )
          {
            // next parent is the main folder, cut off subfolder name and property
            // directory to create the main folder object name
            PSZ pszName = strrchr( pszObjName, BACKSLASH );
            if ( pszName )
            {
              *pszName = EOS;
              pszName = strrchr( pszObjName, BACKSLASH );
            } /* endif */
            if ( pszName )
            {
              *pszName = EOS;
            }
            else
            {
              // subfolder object name is damaged
              fOK = FALSE;
            } /* endif */
          }
          else
          {
            // next parent is a subfolder, replace name part of object name
            // with name of parent folder
            PSZ pszName = strrchr( pszObjName, BACKSLASH );
            if ( pszName )
            {
              pszName++;               // position to name part
              sprintf( pszName, "%8.8ld%s", pProp->ulParentFolder, EXT_OF_SUBFOLDER );
            }
            else
            {
              // subfolder object name is damaged
              fOK = FALSE;
            } /* endif */
          } /* endif */
        } /* endif */
      } /* endif */

      if ( pProp )
      {
        UtlAlloc( (PVOID *)&pProp, 0L, 0L, NOMSG );
        pProp = NULL;
      } /* endif */
    } while ( fOK && !fDone );
  } /* endif */

// Cleanup
  if ( pszObjName )  UtlAlloc( (PVOID *)&pszObjName, 0L, 0L, NOMSG );

  return( usRC );
}

// data structure for FolFuncGetFolPropEx function
typedef struct _GETFOLPROPEXDATA
{
  CHAR             szFolderName[MAX_LONGFILESPEC];
  CHAR             szObjName[MAX_LONGFILESPEC];
  CHAR             szShortName[MAX_FILESPEC];
  CHAR             szPropFileName[MAX_FILESPEC];
  CHAR             szData[(MAX_NUM_OF_READONLY_MDB+10)*MAX_LONGFILESPEC];
} GETFOLPROPEXDATA, *PGETFOLPROPEXDATA;

USHORT FolFuncGetFolPropEx
(
  PSZ         pszFolderName,           // mand: name of the folder to get the property value from
  PSZ         pszKey,                  // mand: name of the requested value
                                       //@: DRIVE,TARGETLANGUAGE,SOURCELANGUAGE,MEMORY,DICTIONARIES,ROMEMORIES,DESCRIPTION,PROFILE,SHIPMENT
  PSZ         pszBuffer,               // mand: buffer for the returned value
  int         iBufSize                 // mand: size of the buffer
)
{
  PPROPFOLDER      pProp = NULL;       // pointer to folder properties
  USHORT           usRC = NO_ERROR;    // function return code
  BOOL             fOK = TRUE;         // success indicator
  BOOL             fFolIsNew = FALSE;
  PGETFOLPROPEXDATA pData = NULL;
  PSZ              pszParm;
  CHAR             szLocalBuffer[40];  // local buffer for values needing some preprocessing

  // allocate our data buffer 
  if (fOK)
  {
    fOK = UtlAlloc( (PVOID *) &pData, 0L,sizeof(GETFOLPROPEXDATA), NOMSG );
    if ( !fOK )
    {
      UtlErrorHwnd( ERROR_STORAGE, MB_CANCEL, 0, NULL, EQF_ERROR, HWND_FUNCIF );
      usRC = ERROR_STORAGE;
    } /* endif */
  } /* endif */

  // Check if folder name is valid
  if (fOK)
  {
    if ( (pszFolderName == NULL) || (*pszFolderName == EOS) )
    {
      fOK = FALSE;
      usRC = TA_MANDFOLDER;
      UtlErrorHwnd( usRC, MB_CANCEL, 0, NULL, EQF_ERROR, HWND_FUNCIF );
    }
    else
    {
      strcpy( pData->szFolderName, pszFolderName );
      ObjLongToShortName( pData->szFolderName, pData->szShortName, FOLDER_OBJECT, &fFolIsNew );
      if( fFolIsNew )
      {
        fOK = FALSE;
        pszParm = pData->szFolderName;
        usRC = ERROR_XLATE_FOLDER_NOT_EXIST;
        UtlErrorHwnd( usRC, MB_CANCEL, 1, &pszParm, EQF_ERROR, HWND_FUNCIF );
      }
      else
      {
        UtlMakeEQFPath( pData->szPropFileName, NULC, PROPERTY_PATH, NULL );
        strcat( pData->szPropFileName, BACKSLASH_STR );
        strcat( pData->szPropFileName, pData->szShortName );
        strcat( pData->szPropFileName, EXT_FOLDER_MAIN );
      }
    }
  } /* endif */

  if (fOK)
  {
    // preset caller's buffer
    if( pszBuffer )
    {
      *pszBuffer = EOS;
    }
    else
    {
      fOK = FALSE;
      usRC = EQFRS_AREA_TOO_SMALL;
      UtlErrorHwnd( usRC, MB_CANCEL, 0, NULL, EQF_ERROR, HWND_FUNCIF );
    } /* endif */
  }

  // load the folder property file into memory
  if ( fOK )
  {
    ULONG  ulLen;
    fOK = UtlLoadFileL( pData->szPropFileName, (PVOID *)&pProp, &ulLen, FALSE, FALSE );
    if ( !fOK )
    {
      pszParm = pData->szPropFileName;
      UtlErrorHwnd( ERROR_PROPERTY_ACCESS, MB_CANCEL, 1, &pszParm, EQF_ERROR, HWND_FUNCIF );
      usRC = ERROR_PROPERTY_ACCESS;
    } /* endif */
  } /* endif */

  // check key parameter and access requested folder property value
  if ( fOK )
  {
    PSZ pszValue = NULL;

    if ( (pszKey == NULL) || (*pszKey == EOS) )
    {
      PSZ pszParms[2];
      fOK = FALSE;
      pszParms[0] = "";
      pszParms[1] = "KEY";
      usRC = BATCH_WRONGVALUE;
      UtlErrorHwnd( usRC, MB_CANCEL, 2, pszParms, EQF_ERROR, HWND_FUNCIF );
    }
    else if ( stricmp( pszKey, "DRIVE" ) == 0 )
    {
      pData->szData[0] = pProp->chDrive;
      pData->szData[1] = EOS;
      pszValue = pData->szData;
    }
    else if ( stricmp( pszKey, "TARGETLANGUAGE" ) == 0 )
    {
      pszValue = pProp->szTargetLang;
    }
    else if ( stricmp( pszKey, "SOURCELANGUAGE" ) == 0 )
    {
      pszValue = pProp->szSourceLang;
    }
    else if ( stricmp( pszKey, "MEMORY" ) == 0 )
    {
      strcpy( pData->szData, pProp->szLongMemory[0] ? pProp->szLongMemory : pProp->szMemory );
      OEMTOANSI( pData->szData );
      pszValue = pData->szData;
    }
    else if ( stricmp( pszKey, "DICTIONARIES" ) == 0 )
    {
      pData->szData[0] = EOS;
      for( int i = 0; i < MAX_NUM_OF_FOLDER_DICS; i++ )
      {
        if ( pProp->aLongDicTbl[i][0] != EOS )
        {
          if ( i != 0 ) strcat( pData->szData, "," );
          strcat( pData->szData, pProp->aLongDicTbl[i] );
        } /* endif */
      } /* endfor */
      OEMTOANSI( pData->szData );
      pszValue = pData->szData;
    }
    else if ( stricmp( pszKey, "ROMEMORIES" ) == 0 )
    {  
      pData->szData[0] = EOS;
      for( int i = 0; i < MAX_NUM_OF_READONLY_MDB; i++ )
      {
        if ( pProp->aLongMemTbl[i][0] != EOS )
        {
          if ( i != 0 ) strcat( pData->szData, "," );
          strcat( pData->szData, pProp->aLongMemTbl[i] );
        } /* endif */
      } /* endfor */
      OEMTOANSI( pData->szData );
      pszValue = pData->szData;
    }
    else if ( stricmp( pszKey, "DESCRIPTION" ) == 0 )
    {
      strcpy( pData->szData, pProp->szDescription );
      OEMTOANSI( pData->szData );
      pszValue = pData->szData;
    }
    else if ( stricmp( pszKey, "ANALYSISPROFILE" ) == 0 )
    {
      pszValue = pProp->szSavedDlgIanaProfile; 
    }
    else if ( stricmp( pszKey, "COUNTINGPROFILE" ) == 0 )
    {
      Utlstrccpy( szLocalBuffer, pProp->szProfile, DOT );
      pszValue = szLocalBuffer; 
    }
    else if ( stricmp( pszKey, "SHIPMENT" ) == 0 )
    {
      pszValue = pProp->szShipment;  
    }
    else
    {
        PSZ pszParms[2];
        fOK = FALSE;
        pszParms[0] = pszKey;
        pszParms[1] = "KEY";
        usRC = BATCH_WRONGVALUE;
        UtlErrorHwnd( usRC, MB_CANCEL, 2, pszParms, EQF_ERROR, HWND_FUNCIF );
    } /* endif */

    if ( fOK )
    {
      if ( (int)strlen( pszValue ) >= iBufSize )
      {
         fOK = FALSE;
         usRC = EQFRS_AREA_TOO_SMALL;
         UtlErrorHwnd( usRC, MB_CANCEL, 0, NULL, EQF_ERROR, HWND_FUNCIF );
      }
      else
      {
        strcpy( pszBuffer, pszValue );
      } /* endif */
    } /* endif */
  } /* endif */


  // Cleanup
  if ( pProp )  UtlAlloc( (PVOID *)&pProp, 0L, 0L, NOMSG );
  if ( pData )  UtlAlloc( (PVOID *)&pData, 0L, 0L, NOMSG );

  return( usRC );
}


///////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////

//+----------------------------------------------------------------------------+
//|                                                                            |
//|  Property Sheet Folder Properties                                          |
//|                                                                            |
//+----------------------------------------------------------------------------+


///////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////

INT_PTR CALLBACK FOLDERPROPSDLG
( HWND hwndDlg, WINMSG message, WPARAM mp1, LPARAM mp2)
{
  PFOLPROPIDA   pIda;                 // ptr to instance data area
  PPROPSYSTEM   pSysProp;             // ptr to system props

  INT           nItem;
  BOOL          fOk = TRUE;
  MRESULT       mResult = FALSE;      // dialog procedure return value

  switch ( message)
  {
    case WM_EQF_QUERYID:
      pIda = ACCESSDLGIDA( hwndDlg, PFOLPROPIDA );
      if ( pIda->pProp->PropHead.chType == PROP_TYPE_NEW)
      {
        HANDLEQUERYID( ID_NEWFOLDERPROPS_DLG, mp2 );
      }
      else
      {
        HANDLEQUERYID( ID_FOLDERPROPS_DLG, mp2 );
      } /* endif */
      break;
/*--------------------------------------------------------------------------*/
    case WM_INITDLG:
      {
        HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);

        //--- create IDA and anchor it ---
        UtlAlloc( (PVOID *)&pIda, 0L, (LONG) sizeof( FOLPROPIDA), ERROR_STORAGE );
        if ( !pIda )
        {
          WinDismissDlg( hwndDlg, FALSE );
          return( MRFROMSHORT(FALSE) );
        } /* endif */
        ANCHORDLGIDA( hwndDlg, pIda);



        //--- remember pointer to properties ---
        pIda->pProp = (PPROPFOLDER)PVOIDFROMMP2(mp2);
        pIda->hwndDlg = hwndDlg;

        //-- get pointer to system properties ---
        pSysProp = (PPROPSYSTEM)MakePropPtrFromHnd( EqfQuerySystemPropHnd() );

        //--- set ID of dialog to 'Folder Properties' or 'Create Folder' dlg
        if ( pIda->pProp->PropHead.chType == PROP_TYPE_NEW)
        {
          SETWINDOWID( hwndDlg, ID_NEWFOLDERPROPS_DLG );
        }
        else
        {
          SETWINDOWID( hwndDlg, ID_FOLDERPROPS_DLG );
        } /* endif */


        if ( !fOk )
        {
          //--- close analysis dialog, FALSE means: - do not start analysis instance
          //Close_proc( hwnd, FALSE );
        }
        else
        {
          FolderPropertySheetLoad( hwndDlg, pIda );
        } /* endif */

        if ( pIda->pProp->PropHead.chType != PROP_TYPE_NEW)
        {

          // --- title of panel ("New Properties" or "Folder Properties") ---

          LOADSTRING( (HAB)UtlQueryULong(QL_HAB), hResMod, SID_PROPFOL_TITLE1,
                      pIda->szBuffer1);
          SETTEXTHWND( hwndDlg, pIda->szBuffer1 );



          // --- name of first pushbutton ("Create" or "Change")---

          LOADSTRING( (HAB)UtlQueryULong(QL_HAB), hResMod, SID_FOLDERPROPS_CHANGE,
                      pIda->szBuffer1);
          SETTEXT( hwndDlg, ID_FOLDERPROPS_SET_PB, pIda->szBuffer1);
        }
        else
        {
          // --- name of first pushbutton ---
          LOADSTRING( (HAB)UtlQueryULong(QL_HAB), hResMod, SID_FOLDERPROPS_CREATE,
                      pIda->szBuffer1);
          SETTEXT( hwndDlg, ID_FOLDERPROPS_SET_PB, pIda->szBuffer1);

        }



        mResult = DIALOGINITRETURN( mResult );

        pIda->fInitComplete = TRUE;
      }
      break;

/*--------------------------------------------------------------------------*/

    case WM_DESTROY:
      pIda = ACCESSDLGIDA( hwndDlg, PFOLPROPIDA );
      if ( pIda )
      {
        if ( pIda->pDictLang ) UtlAlloc( (PVOID *)&pIda->pDictLang, 0L, 0L, NOMSG );
        if ( pIda->hwndFolLB ) WinDestroyWindow( pIda->hwndFolLB );
        if ( pIda->hDicLBA ) WinDestroyWindow( pIda->hDicLBA );
        if ( pIda->hDictPIDLBA ) WinDestroyWindow( pIda->hDictPIDLBA );
        if ( pIda->hMemLBA ) WinDestroyWindow( pIda->hMemLBA );
        if ( pIda->pvGMOptList ) GlobMemFreeOptionFile( &(pIda->pvGMOptList) ); 
        if ( pIda->pvGMData ) FolGMCleanup( pIda->pvGMData );
        if ( pIda->fStartAnalysis )
        {
          // we have to use our static buffer for the folder object name as we post the "start analysis" task using Post message
          strcpy( szObjNameBuffer, pIda->pProp->PropHead.szPath );
          strcat( szObjNameBuffer, BACKSLASH_STR );
          strcat( szObjNameBuffer, pIda->pProp->PropHead.szName );
          EqfPost2Handler( ANALYSISHANDLER, WM_EQF_CREATE, 0, MP2FROMP(szObjNameBuffer) );
        } /* endif */           
        UtlAlloc( (PVOID *)&pIda, 0L, 0L, ERROR_STORAGE );
      } /* endif */
      break;

// -------------------------------------------------------------------
    case WM_CHAR:
      {
        UCHAR ucCode = (UCHAR) LOWORD( mp1 );
        if ( (ucCode == VK_UP ) || (ucCode == VK_DOWN) )
        {
        }
        else
        {
          mResult = WinDefDlgProc( hwndDlg, WM_CHAR, mp1, mp2 );
        } /* endif */           
      }
      break;
// -------------------------------------------------------------------
    case DM_GETDEFID:
      if ( GetKeyState(VK_RETURN) & 0x8000 )
      {

        pIda = ACCESSDLGIDA( hwndDlg, PFOLPROPIDA );
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
              fOk =  pfnWp( pIda->hwndPages[nItem], WM_COMMAND,
                            DM_GETDEFID , 0L);
              break;
            case 1:
              fOk =  pfnWp( pIda->hwndPages[nItem], WM_COMMAND,
                            DM_GETDEFID , 0L);
              break;

            case 2:
              fOk =  pfnWp( pIda->hwndPages[nItem], WM_COMMAND,
                            DM_GETDEFID , 0L);
              break;

            case 4:
              fOk =  pfnWp( pIda->hwndPages[nItem], WM_COMMAND,
                            DM_GETDEFID , 0L);
              break;
          } /* endswitch */
          nItem++;
        } /* endwhile */
      }// end if
      mResult = TRUE;
      break;

/*--------------------------------------------------------------------------*/
    case WM_NOTIFY:
      mResult = FolderPropsPropertySheetNotification( hwndDlg, mp1, mp2 );
      break;
/*--------------------------------------------------------------------------*/
    case WM_COMMAND:
      mResult = FolderPropsCommand( hwndDlg, WMCOMMANDID( mp1, mp2 ),
                                    WMCOMMANDCMD( mp1, mp2 ) );
      break;
/*--------------------------------------------------------------------------*/
    case WM_HELP:
      {

        /*************************************************************/
        /* pass on a HELP_WM_HELP request                            */
        /*************************************************************/


        pIda = ACCESSDLGIDA(hwndDlg, PFOLPROPIDA );

        if ( pIda->pProp->PropHead.chType != PROP_TYPE_NEW) // existing folder
        {
          EqfDisplayContextHelp( (HWND)((LPHELPINFO) mp2)->hItemHandle,
                                 &hlpsubtblFolderProps[0] );
        }
        else   // new folder
        {
          EqfDisplayContextHelp( (HWND)((LPHELPINFO) mp2)->hItemHandle,
                                 &hlpsubtblCreateFolder[0] );
        }

        mResult = TRUE;  // message processed
        break;
      } // end case WM_HELP



/*--------------------------------------------------------------------------*/
    default:
      mResult = WinDefDlgProc( hwndDlg, message, mp1, mp2 );
/*--------------------------------------------------------------------------*/
  }
  return( mResult );
}



//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     FolderPropertySheetLoad                                  |
//+----------------------------------------------------------------------------+
//|Function call:     FolderPropertySheetLoad( hwndDlg, mp2 );                 |
//+----------------------------------------------------------------------------+
//|Description:       handle changes on the tab page                           |
//+----------------------------------------------------------------------------+
//|Parameters:        HWND hwndDlg   handle of the dialog                      |
//|                   LPARAM  mp2    message parameter 2                       |
//+----------------------------------------------------------------------------+
//|Returncode type:   MRESULT                                                  |
//+----------------------------------------------------------------------------+
//|Returncodes:       return code from default window proc or FALSE            |
//+----------------------------------------------------------------------------+
//|Function flow:     create any pages,                                        |
//|                   load the tabctrl text                                    |
//|                   load the (modeless) dialog, register it and position into|
//|                     tab area                                               |
//|                   return                                                   |
//+----------------------------------------------------------------------------+
BOOL FolderPropertySheetLoad ( HWND hwnd, PFOLPROPIDA pFolIda )
{
  TC_ITEM   TabCtrlItem;
  USHORT    nItem = 0;
  HWND      hwndTabCtrl = NULLHANDLE;
  HINSTANCE hInst;
  CHAR      szBuffer[80];
  int       iDisplayTab = 0;
  BOOL      fOk = TRUE;
  HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);

  if ( fOk )
  {
    RECT rect;
    // remember adress of user area
    hInst = GETINSTANCE( hwnd );
    hwndTabCtrl = GetDlgItem( hwnd, ID_FOLDERPROPS_TABCTRL );
    pFolIda->hwndTabCtrl = hwndTabCtrl;
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
    // IDS_FOLDERPROPS_TAB_GENERAL
    //

    LOADSTRING( hab, hResMod, IDS_FOLDERPROPS_TAB_GENERAL , szBuffer );
    TabCtrlItem.pszText = szBuffer;
    TabCtrlItem.lParam = nItem;
    SendMessage(hwndTabCtrl, TCM_INSERTITEM, nItem, (LPARAM)&TabCtrlItem);
    pFolIda->hwndPages[nItem] =
    CreateDialogParam( hInst,
                       MAKEINTRESOURCE( ID_FOLDERPROPS_GENERAL_DLG ),
                       hwnd,
                       FOLDERPROPS_GENERAL_DLGPROC,
                       (LPARAM)pFolIda );

    SetWindowPos( pFolIda->hwndPages[nItem], HWND_TOP,
                  rect.left, rect.top,
                  rect.right-rect.left, rect.bottom-rect.top, SWP_SHOWWINDOW );
    SetFocus( pFolIda->hwndPages[nItem] );
    UtlRegisterModelessDlg( pFolIda->hwndPages[nItem] );
    nItem++;

    //
    // IDS_FOLDERPROPS_TAB_AFC
    //

    LOADSTRING( hab, hResMod, IDS_FOLDERPROPS_TAB_AFC , szBuffer );
    TabCtrlItem.pszText = szBuffer;
    TabCtrlItem.lParam = nItem;
    SendMessage(hwndTabCtrl, TCM_INSERTITEM, nItem, (LPARAM)&TabCtrlItem);
    pFolIda->hwndPages[nItem] =
    CreateDialogParam( hInst,
                       MAKEINTRESOURCE( ID_FOLDERPROPS_AFC_DLG ),
                       hwnd,
                       FOLDERPROPS_AFC_DLGPROC,
                       (LPARAM)pFolIda );

    SetWindowPos( pFolIda->hwndPages[nItem], HWND_TOP,
                  rect.left, rect.top,
                  rect.right-rect.left, rect.bottom-rect.top, SWP_SHOWWINDOW );
    SetFocus( pFolIda->hwndPages[nItem] );
    UtlRegisterModelessDlg( pFolIda->hwndPages[nItem] );
    nItem++;

    //
    // IDS_FOLDERPROPS_TAB_PROJECTINFO
    //

    LOADSTRING( hab, hResMod, IDS_FOLDERPROPS_TAB_PROJECTINFO , szBuffer );
    TabCtrlItem.pszText = szBuffer;
    TabCtrlItem.lParam = nItem;
    SendMessage(hwndTabCtrl, TCM_INSERTITEM, nItem, (LPARAM)&TabCtrlItem);
    pFolIda->hwndPages[nItem] =
    CreateDialogParam( hInst,
                       MAKEINTRESOURCE( ID_FOLDERPROPS_PROJECTINFO_DLG ),
                       hwnd,
                       FOLDERPROPS_PROJECTINFO_DLGPROC,
                       (LPARAM)pFolIda );

    SetWindowPos( pFolIda->hwndPages[nItem], HWND_TOP,
                  rect.left, rect.top,
                  rect.right-rect.left, rect.bottom-rect.top, SWP_SHOWWINDOW );
    SetFocus( pFolIda->hwndPages[nItem] );
    UtlRegisterModelessDlg( pFolIda->hwndPages[nItem] );
    nItem++;


    //
    // IDS_FOLDERPROPS_TAB_SHOWPROPS
    //

//  LOADSTRING( hab, hResMod, IDS_FOLDERPROPS_TAB_SHOWPROPS , szBuffer );
//  TabCtrlItem.pszText = szBuffer;
//  TabCtrlItem.lParam = nItem;
//  SendMessage(hwndTabCtrl, TCM_INSERTITEM, nItem, (LPARAM)&TabCtrlItem);
//  pFolIda->hwndPages[nItem] =
//  CreateDialogParam( hInst,
//                     MAKEINTRESOURCE( ID_FOLDERPROPS_SHOWPROPS_DLG ),
//                     hwnd,
//                     FOLDERPROPS_SHOWPROPS_DLGPROC,
//                     (LPARAM)pFolIda );
//
//  SetWindowPos( pFolIda->hwndPages[nItem], HWND_TOP,
//                rect.left, rect.top,
//                rect.right-rect.left, rect.bottom-rect.top, SWP_SHOWWINDOW );
//  SetFocus( pFolIda->hwndPages[nItem] );
//  UtlRegisterModelessDlg( pFolIda->hwndPages[nItem] );
//  nItem++;
//

    //
    // IDS_FOLDERPROPS_TAB_GLOBALMEMOPT
    //

    if ( pFolIda->pProp->szGlobalMemOptFile[0] != EOS )
    {
      // setup fully qualified filter file name
      UtlMakeEQFPath( pFolIda->szBuffer1, pFolIda->pProp->chDrive, SYSTEM_PATH, NULL );
      strcat( pFolIda->szBuffer1, BACKSLASH_STR );
      strcat( pFolIda->szBuffer1, pFolIda->pProp->PropHead.szName );
      strcat( pFolIda->szBuffer1, BACKSLASH_STR );
      strcat( pFolIda->szBuffer1, pFolIda->pProp->szGlobalMemOptFile );

      // load the CTID memory option file into memory, create the tab page if successful
      if ( GlobMemLoadOptionFile( pFolIda->szBuffer1, &(pFolIda->pvGMOptList) ) )
      {
        // create the TAB for the CTID memory options 
        LOADSTRING( hab, hResMod, IDS_FOLDERPROPS_TAB_GLOBALMEMOPT , szBuffer );
        TabCtrlItem.pszText = szBuffer;
        TabCtrlItem.lParam = nItem;
        SendMessage(hwndTabCtrl, TCM_INSERTITEM, nItem, (LPARAM)&TabCtrlItem);
        pFolIda->hwndPages[nItem] =
        CreateDialogParam( hInst, MAKEINTRESOURCE( ID_FOLDERPROPS_GLOBALMEMOPT_DLG ), 
                          hwnd, FOLDERPROPS_GLOBALMEMOPT_DLGPROC, (LPARAM)pFolIda );

        SetWindowPos( pFolIda->hwndPages[nItem], HWND_TOP, rect.left, rect.top, rect.right-rect.left, rect.bottom-rect.top, SWP_SHOWWINDOW );
        SetFocus( pFolIda->hwndPages[nItem] );
        UtlRegisterModelessDlg( pFolIda->hwndPages[nItem] );
        // show this tab initially when filter file hasn't been verified by user
        if ( pFolIda->pProp->fGlobalMemOptCheckRequired )
        {
          iDisplayTab = nItem; 

          // reset "check required" flag
          pFolIda->pProp->fGlobalMemOptCheckRequired = FALSE;
          UtlMakeEQFPath( pFolIda->szBuffer1, NULC, PROPERTY_PATH, NULL );
          strcat( pFolIda->szBuffer1, BACKSLASH_STR );
          strcat( pFolIda->szBuffer1, pFolIda->pProp->PropHead.szName );
          UtlWriteFileL( pFolIda->szBuffer1, sizeof(PROPFOLDER), pFolIda->pProp, FALSE );
        } /* endif */           
        nItem++;
      } /* endif */         
    } /* endif */       


  } /* endif */


  // -----------------------------------------------------------------
  //
  // hide all dialog pages but the one to be displayed
  //
  // -----------------------------------------------------------------

  if ( fOk )
  {
    int i = 0;
    while ( pFolIda->hwndPages[i] )
    {
      if ( i != iDisplayTab )
      {
        ShowWindow( pFolIda->hwndPages[i], SW_HIDE );
      } /* endif */         
      i++;
    } /* endwhile */
    WinSendMsg( hwndTabCtrl, TCM_SETCURSEL, iDisplayTab, 0L );

  } /* endif */

  if ( !fOk )
  {
    POSTEQFCLOSE( hwnd, FALSE );
  } /* endif */

  return fOk;
}



//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     FolderPropsPropertySheetNotification                     |
//+----------------------------------------------------------------------------+
//|Function call:     FolderPropsPropertySheetNotification( hwndDlg, mp1, mp2);|
//+----------------------------------------------------------------------------+
//|Description:       handle changes on the tab page                           |
//+----------------------------------------------------------------------------+
//|Parameters:        HWND hwndDlg   handle of the dialog                      |
//|                   WPARAM  mp1    message parameter 1                       |
//|                   LPARAM  mp2    message parameter 2                       |
//+----------------------------------------------------------------------------+
//|Returncode type:   MRESULT                                                  |
//+----------------------------------------------------------------------------+
//|Returncodes:       return code from default window proc or FALSE            |
//+----------------------------------------------------------------------------+
//|Function flow:     switch ( pNMHdr->code )                                  |
//|                     case TCN_SELCHANGE:                                    |
//|                       activate new page                                    |
//|                     case TCN_SELCHANGING                                   |
//|                       hide the dialog                                      |
//|                   return                                                   |
//+----------------------------------------------------------------------------+
MRESULT FolderPropsPropertySheetNotification
(
HWND hwndDlg,
WPARAM  mp1,
LPARAM  mp2
)
{
  NMHDR *       pNMHdr;
  ULONG         ulTabCtrl;
  MRESULT       mResult = FALSE;
  PFOLPROPIDA   pIda;
  pNMHdr = (LPNMHDR)mp2;
  mp1;

  switch ( pNMHdr->code )
  {
    case TCN_SELCHANGE:
      pIda = ACCESSDLGIDA(hwndDlg, PFOLPROPIDA );
      if ( pIda )
      {
        TC_ITEM Item;
        HWND hwndTabCtrl = GetDlgItem( hwndDlg, ID_FOLDERPROPS_TABCTRL );
        ulTabCtrl = TabCtrl_GetCurSel( hwndTabCtrl );
        memset( &Item, 0, sizeof(Item) );
        Item.mask = TCIF_PARAM;
        TabCtrl_GetItem( hwndTabCtrl, ulTabCtrl, &Item );
        ulTabCtrl = Item.lParam;
        ShowWindow( pIda->hwndPages[ ulTabCtrl ], SW_SHOW );
      } /* endif */
      break;
    case TCN_SELCHANGING:
      pIda = ACCESSDLGIDA( hwndDlg, PFOLPROPIDA );
      if ( pIda )
      {
        /**************************************************************/
        /* Issue a direct call to the appropriate dialog proc with    */
        /* WM_COMMAND, ID_TB_PROP_SET_PB and the second parameter set */
        /* to 1L to force only consistency checking                   */
        /**************************************************************/
        TC_ITEM Item;
        PFNWP pfnWp;
        HWND hwndTabCtrl = GetDlgItem( hwndDlg, ID_FOLDERPROPS_TABCTRL );
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
          WinPostMsg( hwndDlg, TCM_SETCURSEL, ulTabCtrl, 0L );
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
          HWND hwndTabCtrl = GetDlgItem( hwndDlg, ID_FOLDERPROPS_TABCTRL );
          memset( &Item, 0, sizeof(Item) );
          Item.mask = TCIF_PARAM;
          TabCtrl_GetItem( hwndTabCtrl, pToolTipText->hdr.idFrom, &Item );
          switch ( (SHORT)Item.lParam )
          {
            case 0:      // first page
              LOADSTRING( hab, hResMod, IDS_FOLDERPROPS_TTIP_GENERAL,
                          pToolTipText->szText );
              break;
            case 1:      // first page
              LOADSTRING( hab, hResMod, IDS_FOLDERPROPS_TTIP_AFC,
                          pToolTipText->szText );
              break;

            case 3:      
              LOADSTRING( hab, hResMod, IDS_FOLDERPROPS_TTIP_GLOBALMEMOPT,
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
} /* end of function FolderPropsPropertySheetNotification */
//









//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     FolderPropsCommand                                       |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//+----------------------------------------------------------------------------+
//|Description:       Handle WM_COMMAND message of property sheet dialog       |
//+----------------------------------------------------------------------------+
//|Parameters:        HWND hwnd   handle of the dialog                         |
//|                   WPARAM  mp1    message parameter 1                       |
//|                   LPARAM  mp2    message parameter 2                       |
//+----------------------------------------------------------------------------+
//|Returncode type:   MRESULT                                                  |
//+----------------------------------------------------------------------------+
//|Returncodes:       return code from default window proc or FALSE            |
//+----------------------------------------------------------------------------+


MRESULT FolderPropsCommand
(
HWND hwndDlg,
WPARAM mp1,
LPARAM mp2
)
{
  BOOL fOk = TRUE;
  INT     nItem;
  MRESULT mResult = MRFROMSHORT(TRUE);
  PFOLPROPIDA     pIda;               // ptr to instance data area

  pIda = ACCESSDLGIDA( hwndDlg, PFOLPROPIDA );

  mp2;
  switch ( WMCOMMANDID( mp1, mp2 ) )
  {
    case PID_PB_HELP:
      UtlInvokeHelp();
      break;

    case ID_FOLDERPROPS_SET_PB:
      {

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
              fOk =  pfnWp( pIda->hwndPages[nItem], WM_COMMAND,
                            PID_PB_OK, 0L);
              break;

              // AFC  settings
            case 1:
              fOk =  pfnWp( pIda->hwndPages[nItem], WM_COMMAND,
                            PID_PB_OK, 0L);
              break;

              // Project Informations
            case 2:
              fOk =  pfnWp( pIda->hwndPages[nItem], WM_COMMAND,
                            PID_PB_OK, 0L);
              break;

              // Global Memory Options
            case 3:
              fOk =  pfnWp( pIda->hwndPages[nItem], WM_COMMAND,
                            PID_PB_OK, 0L);
              break;

          } /* endswitch */
          if ( !fOk )
          {
            // switch to page causing the error
            HWND hwndTabCtrl = GetDlgItem( hwndDlg, ID_FOLDERPROPS_TABCTRL );
            ULONG  ulCurrent = TabCtrl_GetCurSel( hwndTabCtrl );
            TabCtrl_SetCurSel( hwndTabCtrl, nItem );
            ShowWindow( pIda->hwndPages[ulCurrent], SW_HIDE );
            ShowWindow( pIda->hwndPages[nItem], SW_SHOW );
          } /* endif */
          nItem++;
        } /* endwhile */


        //-- nothing went wrong, create or update folder ---

        //if ( !usErrMsg && !usErrorID )
        if ( fOk )
        {
          HPROP           hFLLProp;   // folder list properties handler
          PPROPFOLDERLIST pFLLProp;   // folder list properties pointer
          EQFINFO         ErrorInfo;  // error returned by property handler

          /************************************************************/
          /* Save last-used values for source and target language   */
          /************************************************************/
          UtlMakeEQFPath( pIda->szBuffer1, NULC, SYSTEM_PATH, NULL );
          strcat( pIda->szBuffer1, BACKSLASH_STR );
          strcat( pIda->szBuffer1, DEFAULT_FOLDERLIST_NAME );
          hFLLProp = OpenProperties( pIda->szBuffer1, NULL,
                                     PROP_ACCESS_READ, &ErrorInfo);
          if ( hFLLProp )
          {
            if ( SetPropAccess( hFLLProp, PROP_ACCESS_WRITE) )
            {
              pFLLProp = (PPROPFOLDERLIST)MakePropPtrFromHnd( hFLLProp );
              strcpy( pFLLProp->szSourceLang, pIda->pProp->szSourceLang );
              strcpy( pFLLProp->szTargetLang, pIda->pProp->szTargetLang );
              strcpy( pFLLProp->szFormat, pIda->pProp->szFormat );
              strcpy( pFLLProp->szEditor, pIda->pProp->szEditor );
              SaveProperties( hFLLProp, &ErrorInfo );
              ResetPropAccess( hFLLProp, PROP_ACCESS_WRITE);
            } /* endif */
            CloseProperties( hFLLProp, PROP_FILE, &ErrorInfo);
          } /* endif */

          /****************************************************/
          /* Create folder and remove dialog                  */
          /****************************************************/
          UtlTime( (PLONG)&pIda->pProp->ulLastChange );
          if ( pIda->pProp->PropHead.chType == PROP_TYPE_NEW ||
               pIda->pProp->PropHead.chType == PROP_TYPE_TEMPLATE)
          {
            HPROP  hProp;
            BOOL   fOK = TRUE;

            /*********************************************************/
            /* Create folder directories                             */
            /*********************************************************/
            fOK = ( CreateFolderDir( pIda->pProp, TRUE ) == NO_ERROR);

            /*********************************************************/
            /* Complete and create folder properties                 */
            /*********************************************************/
            if ( fOK )
            {
              EQFINFO ErrorInfo;

              FolSetDefaultPos( &pIda->pProp->Swp );

              // set default view list if no settings have been copied from a model folder
              if ( !(pIda->pProp->sLastUsedViewList[0]) )
              {
                FolSetDocListView(pIda->pProp);
              } /* endif */
              pIda->pProp->PropHead.chType = PROP_TYPE_INSTANCE;

              // delete any old property file
              UtlMakeEQFPath( pIda->szBuffer1, NULC, PROPERTY_PATH, NULL );
              strcat( pIda->szBuffer1, BACKSLASH_STR );
              strcat( pIda->szBuffer1, pIda->pProp->PropHead.szName );
              UtlDelete( pIda->szBuffer1, 0L, FALSE );

              strcpy( pIda->szBuffer1, pIda->pProp->PropHead.szPath );
              strcat( pIda->szBuffer1, BACKSLASH_STR );
              strcat( pIda->szBuffer1, pIda->pProp->PropHead.szName);
              ErrorInfo = 0L;
              hProp = CreateProperties( pIda->szBuffer1, (PSZ)NULP,
                                        PROP_CLASS_FOLDER, &ErrorInfo);
              if ( !hProp)
              {
                fOK = FALSE;
              } /* endif */

              if ( fOK )
              {
                fOK = (PutAllProperties( hProp, pIda->pProp, &ErrorInfo) == 0);
              } /* endif */

              if ( fOK )
              {
                fOK = (SaveProperties( hProp, &ErrorInfo) == 0);
              } /* endif */

              if ( !fOK )
              {
                PSZ pszReplace;
                CHAR szDrive[MAX_DRIVE];

                switch ( (USHORT)ErrorInfo )
                {
                  case Err_NoDiskSpace :
                    pszReplace = szDrive;
                    sprintf( szDrive, "%c:", pIda->pProp->PropHead.szPath[0] );
                    UtlError( ERROR_DISK_IS_FULL, MB_CANCEL,
                              1, &pszReplace, EQF_ERROR );
                    break;
                  case Err_NoStorage :
                    UtlError( ERROR_STORAGE, MB_CANCEL, 0, NULL, EQF_ERROR );
                    break;
                  default :
                    pszReplace = pIda->pProp->PropHead.szName;
                    UtlError( ERROR_CREATE_PROP, MB_CANCEL, 1,
                              &pszReplace, EQF_ERROR );
                    break;
                } /* endswitch */

                /******************************************************/
                /* Delete any created property file in case of errors */
                /******************************************************/
                UtlMakeEQFPath( pIda->szBuffer1, NULC, PROPERTY_PATH, NULL );
                strcat( pIda->szBuffer1, BACKSLASH_STR );
                strcat( pIda->szBuffer1, pIda->pProp->PropHead.szName );
                UtlDelete( pIda->szBuffer1, 0L, FALSE );

                /**************************************************************/
                /* Remove folder directories in case of errors                */
                /**************************************************************/
                UtlMakeEQFPath( pIda->szBuffer1,
                                pIda->pProp->PropHead.szPath[0],
                                SYSTEM_PATH, NULL );
                strcat( pIda->szBuffer1, BACKSLASH_STR );
                strcat( pIda->szBuffer1, pIda->pProp->PropHead.szName );
                UtlRemoveDir( pIda->szBuffer1, FALSE );
              } /* endif */

              // for controlled folders: write shipment record to history log
              if ( fOK )
              {
                if ( pIda->pProp->fAFCFolder && pIda->pProp->szShipment[0] )
                {
                  FOLPROPHISTSHIPMENT FolPropHistShipment;     // history record for folder

                  strcpy( pIda->szBuffer1, pIda->pProp->PropHead.szPath );
                  strcat( pIda->szBuffer1, BACKSLASH_STR );
                  strcat( pIda->szBuffer1, pIda->pProp->PropHead.szName);
                  strcpy( FolPropHistShipment.szShipment, pIda->pProp->szShipment );

                  EQFBWriteHistLog2( pIda->szBuffer1, "", FOLPROPSHIPMENT_LOGTASK, sizeof(FOLPROPHISTSHIPMENT),
                                    (PVOID)&FolPropHistShipment, FALSE, NULLHANDLE, NULL );
                } /* endif */
              } /* endif */


              if ( hProp )
              {
                CloseProperties( hProp, PROP_QUIT, &ErrorInfo );
              } /* endif */
            } /* endif */

            if ( fOK )
            {
              DelCtrlFont (hwndDlg, ID_FOLNEW_DESCR_EF);
              WinDismissDlg( hwndDlg, 0 );
            }
            else
            {
              /*******************************************************/
              /* Restore old property type                           */
              /*******************************************************/
              pIda->pProp->PropHead.chType = PROP_TYPE_NEW;
            } /* endif */
          }
          else
          {
            /********************************************************/
            /* Remove dialog                                        */
            /********************************************************/
            DelCtrlFont (hwndDlg, ID_FOLNEW_DESCR_EF);
            WinDismissDlg( hwndDlg, 0 );
          } /* endif */
        } /* endif */
      } //end case

      break;

    case DID_CANCEL:
    case ID_FOLDERPROPS_CANCEL_PB:
      DelCtrlFont (hwndDlg, ID_FOLNEW_DESCR_EF);
      WinDismissDlg( hwndDlg, DID_CANCEL );
      break;
  } /* endswitch */

  return( mResult );
} /* end of function FolderPropsCommand */

//+----------------------------------------------------------------------------+
//|Internal function  FOLDERPROPS_GENERAL_DLGPROC                              |
//+----------------------------------------------------------------------------+
//|Function name:                                                              |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//+----------------------------------------------------------------------------+
//|Parameters:                                                                 |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Returncode type:                                                            |
//+----------------------------------------------------------------------------+
//|Returncodes:                                                                |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                                                                            |
//|                                                                            |
//|                                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+

INT_PTR CALLBACK FOLDERPROPS_GENERAL_DLGPROC
(
HWND hwndDlg,                                // handle of dialog window
WINMSG msg,
WPARAM mp1,
LPARAM mp2
)
{
  MRESULT  mResult = MRFROMSHORT( FALSE );  // result value of procedure
  PFOLPROPIDA   pIda;                       // ptr to instance data area
  PPROPSYSTEM   pSysProp;                   // ptr to system props
  SHORT         sItem;                      // index of listbox items
  ULONG         ulEFStyle;                  // buffer for current entry field style
  CHAR          szSecDrives[MAX_DRIVELIST]; // buffer for secondary drives
  INT_PTR       iMBRC;                     // return code of UtlError call

//------------------- drive parameters------------------------

  CHAR szBuffer[MAX_PATH+100];
  UINT nDrive=0;
  DWORD dwLogicalDrives;
  SHFILEINFO FileInfo;
  DWORD r ;
  // HIMAGELIST hImageList;
  int ipos=0;
  int npos=0;
  int ifound=0;
  //int i;
  COMBOBOXEXITEM CBEItem;
//---------------------------------------------------------------
  SHORT sNotification= WMCOMMANDCMD( mp1, mp2 );
  memset (&CBEItem, 0, sizeof(CBEItem));

  switch ( msg )
  {
    case WM_INITDLG:

      pIda = (PFOLPROPIDA)PVOIDFROMMP2( mp2 );
      ANCHORDLGIDA( hwndDlg, pIda );
      //--- get pointer to system properties ---

      pSysProp = (PPROPSYSTEM)MakePropPtrFromHnd( EqfQuerySystemPropHnd() );
      //--- set text limits for entry fields and combo boxes ---
      SETTEXTLIMIT( hwndDlg, ID_FOLNEW_NAME_EF, MAX_LONGPATH - 1 );
      CBSETTEXTLIMIT( hwndDlg, ID_FOLNEW_MEMORY_CBS, MAX_LONGPATH - 1 );
      SETTEXTLIMIT( hwndDlg, ID_FOLNEW_DESCR_EF,
                    sizeof( pIda->pProp->szDescription) - 1);
      SetCtrlFnt (hwndDlg, GetCharSet(), ID_FOLNEW_DESCR_EF, 0 );

      //--- create invisible listbox for available folders ---

      pIda->hwndFolLB = WinCreateWindow( hwndDlg, WC_LISTBOX, "", WS_CHILD | LBS_STANDARD, 0, 0, 0, 0, 
                                         hwndDlg, HWND_TOP, ID_FOLNEW_AVAILFOLDER_LB, NULL, NULL );
      pIda->hDicLBA = WinCreateWindow( hwndDlg, WC_LISTBOX, "", WS_CHILD | LBS_STANDARD, 0, 0, 0, 0, 
                                         hwndDlg, HWND_TOP, 4711, NULL, NULL );
      pIda->hMemLBA = WinCreateWindow( hwndDlg, WC_LISTBOX, "", WS_CHILD | LBS_STANDARD, 0, 0, 0, 0, 
                                         hwndDlg, HWND_TOP, 4712, NULL, NULL );
      pIda->hDictPIDLBA = WinCreateWindow( hwndDlg, WC_LISTBOX, "", WS_CHILD | LBS_STANDARD, 0, 0, 0, 0, 
                                           hwndDlg, HWND_TOP, 4713, NULL, NULL );

      // for performance purposes
      pIda->hDicLBS = WinWindowFromID( hwndDlg, ID_FOLNEW_SELECTED_LB );

      // fill translation memory listbox
      EqfSend2Handler( MEMORYHANDLER, WM_EQF_INSERTNAMES,
                       MP1FROMHWND( WinWindowFromID( hwndDlg, ID_FOLNEW_MEMORY_CBS) ),
                       MP2FROMP( MEMORY_ALL ) );
      EqfSend2Handler( DICTIONARYHANDLER, WM_EQF_INSERTNAMES, MP1FROMHWND( pIda->hDicLBA ), 0L );
      EqfSend2Handler( MEMORYHANDLER, WM_EQF_INSERTNAMES, MP1FROMHWND( pIda->hMemLBA ), MP2FROMP( MEMORY_ALL )  );


      /**************************************************************/
      /* Allocate and initialize source language array for          */
      /* available dictionaries                                     */
      /**************************************************************/
      sItem = QUERYITEMCOUNTHWND( pIda->hDicLBA  );
      if ( sItem )
      {
        if ( !UtlAlloc( (PVOID *)&pIda->pDictLang, 0L, (LONG)(sItem * sizeof(DICTLANG)), ERROR_STORAGE ) )
        {
          UtlAlloc( (PVOID *)&pIda, 0L, 0L, NOMSG );
          WinDismissDlg( hwndDlg, FALSE );
          return( MRFROMSHORT(FALSE) );
        } /* endif */
      } /* endif */


      // fill markup language listbox
      EqfSend2Handler( TAGTABLEHANDLER, WM_EQF_INSERTNAMES,
                       MP1FROMHWND( WinWindowFromID( hwndDlg, ID_FOLNEW_FORMAT_CBS) ),
                       0L );

      // fill conversion combobox
      // in OpenTM2 we do not use conversion
      //UtlHandleConversionStrings( CONVLOAD_MODE, WinWindowFromID( hwndDlg, ID_FOLNEW_CONV_CBS ), NULL, NULL, NULL  );
      //CBINSERTITEM( hwndDlg, ID_FOLNEW_CONV_CBS, "" );
      HIDECONTROL( hwndDlg, ID_FOLNEW_CONV_STATIC );
      HIDECONTROL( hwndDlg, ID_FOLNEW_CONV_CBS );

      // fill editor listbox

      UtlMakeEQFPath( pIda->szBuffer1, NULC, PROPERTY_PATH, NULL );
      UtlMakeFullPath( pIda->szBuffer2, NULL, pIda->szBuffer1,
                       DEFAULT_PATTERN_NAME, EXT_OF_EDITOR );
      pIda->fCBChange = TRUE;
      UtlLoadFileNames( pIda->szBuffer2, FILE_NORMAL,
                        WinWindowFromID( hwndDlg, ID_FOLNEW_EDITOR_CBS),
                        NAMFMT_NOEXT );
      pIda->fCBChange = FALSE;

      /**************************************************************/
      /* Fill source language combobox                              */
      /**************************************************************/
      UtlFillTableLB( WinWindowFromID( hwndDlg, ID_FOLNEW_SOURCELANG_CBS ),
                      SOURCE_LANGUAGES );

      /**************************************************************/
      /* Fill target language combobox                              */
      /**************************************************************/
      UtlFillTableLB( WinWindowFromID( hwndDlg, ID_FOLNEW_TARGETLANG_CBS ),
                      ALL_TARGET_LANGUAGES );
      //
      // TO DRIVE
      //

      //--- get available drives ---
      pSysProp = GetSystemPropPtr();
      UtlGetCheckedEqfDrives( szSecDrives );

      //--- sort drive letters alphabetically ---
      qsort( (void *)szSecDrives, (size_t)strlen(szSecDrives),
             (size_t)sizeof(CHAR), cCompare );

      if ( pIda->pProp->PropHead.chType == PROP_TYPE_NEW)
      {
        pIda->pProp->chDrive = pSysProp->szPrimaryDrive[0];
      }//end if

      // Drive letters To drive
      CBEItem.mask        = CBEIF_IMAGE |
                            CBEIF_SELECTEDIMAGE | CBEIF_TEXT | CBEIF_LPARAM;
      CBEItem.cchTextMax  = 0;  // is ignored
      CBEItem.iItem       = -1; // insert at end

      CBDELETEALL(hwndDlg, ID_FOLNEW_DRIVE_PB);

      dwLogicalDrives = GetLogicalDrives();

      if (dwLogicalDrives)
      {

        for (nDrive=0; nDrive <32; nDrive++)
        {
          if (dwLogicalDrives &(1 << nDrive))
          {
            wsprintf(szBuffer,"%c:\\",nDrive+'A');

            r = SHGetFileInfo( szBuffer, 0, &FileInfo, sizeof( FileInfo ),
                               SHGFI_DISPLAYNAME | SHGFI_SYSICONINDEX | SHGFI_SMALLICON );
            if ( r )
            {
              CHAR chDrive = (CHAR)(nDrive+'A');
              // insert icon and string into list box
              CBEItem.pszText = FileInfo.szDisplayName;
              CBEItem.lParam  = ( LPARAM )nDrive;

              CBEItem.iSelectedImage  =
              CBEItem.iImage          = FileInfo.iIcon; // index into system image list
              if (strchr( szSecDrives, chDrive ))
              {

                //  if ( !pIda->fExists )
                if ( pIda->pProp->PropHead.chType == PROP_TYPE_NEW)
                {
                  int iItem = CBINSERTITEMEND( hwndDlg, ID_FOLNEW_DRIVE_PB, CBEItem.pszText);
                  CBSETITEMHANDLE( hwndDlg, ID_FOLNEW_DRIVE_PB, iItem, nDrive );
                  npos++;
                }
                else if (chDrive == pIda->pProp->chDrive)
                {
                  int iItem = CBINSERTITEMEND( hwndDlg, ID_FOLNEW_DRIVE_PB, CBEItem.pszText );
                  CBSETITEMHANDLE( hwndDlg, ID_FOLNEW_DRIVE_PB, iItem, nDrive );
                  npos++;
                }
              }// end if
            }  //end if(r)
          } // end if
        }  //end for

        //
        // Set system drive
        //

        for (ipos=0; ipos<npos;ipos=ipos+1)
        {
          int nDrive = CBQUERYITEMHANDLE( hwndDlg,ID_FOLNEW_DRIVE_PB ,ipos );
          char chDrive = (CHAR)((CHAR)'A' + nDrive);
          if ( chDrive == pIda->pProp->chDrive)
          {
            ifound=ipos;
          }// end if
        } // end for
        CBSELECTITEM (hwndDlg,ID_FOLNEW_DRIVE_PB ,ifound );
      }  // end if logical

      // ---------------------------------------------------------------


      // for performance purposes
      pIda->hMemLBS = WinWindowFromID( hwndDlg, ID_FOLNEW_MEMSELECT_LB );


//------------------ special init for new folder / folder properties -----------------------------

      if ( pIda->pProp->PropHead.chType != PROP_TYPE_NEW) // exist
      {
        //--- init fields for existing folder properties ---
        pIda->fNew = FALSE;

        //--- show or hide original name controls ---
        if ( (pIda->pProp->szOrgName[0] == EOS) &&
             (pIda->pProp->szOrgLongName[0] == EOS) )
        {
          HIDECONTROL( hwndDlg, ID_FOLNEW_ORGNAME_TEXT );
          HIDECONTROL( hwndDlg, ID_FOLNEW_ORGNAME_EF );
        }
        else
        {
          SHOWCONTROL( hwndDlg, ID_FOLNEW_ORGNAME_TEXT );
          SHOWCONTROL( hwndDlg, ID_FOLNEW_ORGNAME_EF );
          if ( pIda->pProp->szOrgLongName[0] != EOS )
          {
            OEMTOANSI( pIda->pProp->szOrgLongName );
            SETTEXT( hwndDlg, ID_FOLNEW_ORGNAME_EF, pIda->pProp->szOrgLongName );
            ANSITOOEM( pIda->pProp->szOrgLongName );
          }
          else
          {
            SETTEXT( hwndDlg, ID_FOLNEW_ORGNAME_EF, pIda->pProp->szOrgName );
          } /* endif */
        } /* endif */

        // --- name of folder ---
        Utlstrccpy( pIda->szBuffer1, pIda->pProp->PropHead.szName, DOT );

        SETTEXT( hwndDlg, ID_FOLNEW_SHORTNAME_EF, pIda->szBuffer1 );

        if ( pIda->pProp->szLongName[0] != EOS )
        {
          strcpy( pIda->szBuffer1, pIda->pProp->szLongName );
          OEMTOANSI( pIda->szBuffer1 );
          SETTEXT( hwndDlg, ID_FOLNEW_NAME_EF, pIda->szBuffer1 );
        }
        else
        {
          SETTEXT( hwndDlg, ID_FOLNEW_NAME_EF, pIda->szBuffer1 );
        } /* endif */

        // --- style of name entry field ---

        ulEFStyle = STYLEFROMWINDOW( WinWindowFromID( hwndDlg, ID_FOLNEW_NAME_EF ) );
        ulEFStyle &= ~WS_TABSTOP;
        SETWINDOWSTYLE( WinWindowFromID( hwndDlg, ID_FOLNEW_NAME_EF ),
                        ulEFStyle );
        WinSendDlgItemMsg( hwndDlg, ID_FOLNEW_NAME_EF, EM_SETREADONLY,
                           MP1FROMSHORT(TRUE), 0L );

        //--- style of model folder push button ---
        HIDECONTROL( hwndDlg, ID_FOLNEW_BASEFOLDER_PB );
        HIDECONTROL( hwndDlg, ID_FOLNEW_BASEFOLDER_TEXT );

        // --- folder description ---

        OEMSETTEXT( hwndDlg, ID_FOLNEW_DESCR_EF, pIda->pProp->szDescription);
        SETFOCUS( hwndDlg, ID_FOLNEW_DESCR_EF );

        // --------- existing folder is perhaps an AFC Folder --------

        if (pIda->pProp->fAFCFolder)
        {
          if (pIda->pProp->fTCMasterFolder)      // TCMaster can change all properties
          {
            ENABLECTRL( hwndDlg, ID_FOLNEW_FORMAT_CBS, TRUE );
            ENABLECTRL( hwndDlg, ID_FOLNEW_MEMORY_CBS, TRUE );
            ENABLECTRL( hwndDlg, ID_FOLNEW_SOURCELANG_CBS, TRUE );
            ENABLECTRL( hwndDlg, ID_FOLNEW_TARGETLANG_CBS, TRUE );
//            ENABLECTRL( hwndDlg, ID_FOLNEW_CONV_CBS, TRUE );
            ENABLECTRL( hwndDlg, ID_FOLNEW_EDITOR_CBS, TRUE );
          }
          else   // only AFC Folder (possible to free the field via Password)
          {
            ENABLECTRL( hwndDlg, ID_FOLNEW_FORMAT_CBS, FALSE );
            ENABLECTRL( hwndDlg, ID_FOLNEW_MEMORY_CBS, FALSE );
            ENABLECTRL( hwndDlg, ID_FOLNEW_SOURCELANG_CBS, FALSE );
            ENABLECTRL( hwndDlg, ID_FOLNEW_TARGETLANG_CBS, FALSE );
//            ENABLECTRL( hwndDlg, ID_FOLNEW_CONV_CBS, FALSE );
            ENABLECTRL( hwndDlg, ID_FOLNEW_EDITOR_CBS, FALSE );

          }
        }

        //-- set state of stop at first... checkbox
        SETCHECK( hwndDlg, ID_FOLNEW_STOPATFIRSTEXACT_CHK, pIda->pProp->fStopAtFirstExact );

      }
      else
      {
        HPROP           hFLLProp;   // folder list properties handler
        PPROPFOLDERLIST pFLLProp;   // folder list properties pointer
        EQFINFO         ErrorInfo;  // error returned by property handler

        //
        // init fields for new folder properties
        //
        pIda->fNew = TRUE;

        HIDECONTROL( hwndDlg, ID_FOLNEW_ORGNAME_TEXT );
        HIDECONTROL( hwndDlg, ID_FOLNEW_ORGNAME_EF );
        HIDECONTROL( hwndDlg, ID_FOLNEW_SHORTNAME_TEXT );
        HIDECONTROL( hwndDlg, ID_FOLNEW_SHORTNAME_EF );

        //---- fill invisible listbox for available folders ---
        // ( this listbox is used to check for duplicate folder names)
        EqfSend2Handler( FOLDERLISTHANDLER, WM_EQF_INSERTNAMES, MP1FROMHWND( pIda->hwndFolLB ), 0L );

        //--- style of model folder push button ---
        if ( QUERYITEMCOUNTHWND( pIda->hwndFolLB ) )
        {
          ENABLECTRL( hwndDlg, ID_FOLNEW_BASEFOLDER_PB, TRUE );
        }
        else
        {
          ENABLECTRL( hwndDlg, ID_FOLNEW_BASEFOLDER_PB, FALSE );
        } /* endif */

        //--- set drive to default value (= EQF system drive) ---
//      pIda->pProp->chDrive = pSysProp->szPrimaryDrive[0];
        if ( pIda->pProp->szLongName[0] != EOS )
        {
          strcpy( pIda->szBuffer1, pIda->pProp->szLongName );
          OEMTOANSI( pIda->szBuffer1 );
          SETTEXT( hwndDlg, ID_FOLNEW_NAME_EF, pIda->szBuffer1 );
        }
        else if ( pIda->pProp->PropHead.szName[0] )            // predefined name ?
        {
          SETTEXT( hwndDlg, ID_FOLNEW_NAME_EF, pIda->pProp->PropHead.szName );
        } /* endif */
        //--- set focus to first entry field ---
        SETFOCUS( hwndDlg, ID_FOLNEW_NAME_EF );

        //--- set initial match level ---
        pIda->pProp->usMatchLevel = BASE_EXACT_DATE;

        /************************************************************/
        /* select editor if only one editor is available            */
        /************************************************************/
        if ( CBQUERYITEMCOUNT( hwndDlg, ID_FOLNEW_EDITOR_CBS) == 1 )
        {
          pIda->fCBChange = TRUE;
          CBSELECTITEM( hwndDlg, ID_FOLNEW_EDITOR_CBS, 0 );
          pIda->fCBChange = FALSE;
        } /* endif */

        /************************************************************/
        /* Select last-used values                                  */
        /************************************************************/
        UtlMakeEQFPath( pIda->szBuffer1, NULC, SYSTEM_PATH, NULL );
        strcat( pIda->szBuffer1, BACKSLASH_STR );
        strcat( pIda->szBuffer1, DEFAULT_FOLDERLIST_NAME );
        hFLLProp = OpenProperties( pIda->szBuffer1, NULL,
                                   PROP_ACCESS_READ, &ErrorInfo);
        if ( hFLLProp )
        {
          pFLLProp = (PPROPFOLDERLIST)MakePropPtrFromHnd( hFLLProp );
          CBSEARCHSELECT( sItem, hwndDlg, ID_FOLNEW_SOURCELANG_CBS,
                          pFLLProp->szSourceLang );
          CBSEARCHSELECT( sItem, hwndDlg, ID_FOLNEW_TARGETLANG_CBS,
                          pFLLProp->szTargetLang );
          CBSEARCHSELECT( sItem, hwndDlg, ID_FOLNEW_FORMAT_CBS,
                          pFLLProp->szFormat );
          if ( pFLLProp->szEditor[0] != EOS )
          {
            pIda->fCBChange = TRUE;
            CBSEARCHSELECT( sItem, hwndDlg, ID_FOLNEW_EDITOR_CBS,
                            pFLLProp->szEditor );
            pIda->fCBChange = FALSE;
          }
          else if ( CBQUERYITEMCOUNT( hwndDlg, ID_FOLNEW_EDITOR_CBS) == 1 )
          {
            /************************************************************/
            /* select editor if only one editor is available            */
            /************************************************************/
            pIda->fCBChange = TRUE;
            CBSELECTITEM( hwndDlg, ID_FOLNEW_EDITOR_CBS, 0 );
            pIda->fCBChange = FALSE;
          } /* endif */

          CloseProperties( hFLLProp, PROP_QUIT, &ErrorInfo);
        } /* endif */
      } /* endif */


      //--- if selected dictionary listbox contains items ... ---
      if ( QUERYITEMCOUNT( hwndDlg, ID_FOLNEW_SELECTED_LB)  != 0 )
      {
        //--- ... select first listbox item ---
        SELECTITEM( hwndDlg, ID_FOLNEW_SELECTED_LB, 0 );
      } /* endif */

      //--- if selected memory listbox contains items ... ---
      if ( QUERYITEMCOUNT( hwndDlg, ID_FOLNEW_MEMSELECT_LB)  != 0 )
      {
        //--- ... select first listbox item ---
        SELECTITEM( hwndDlg, ID_FOLNEW_MEMSELECT_LB, 0 );
      } /* endif */

      //--- position dialog window ---
      UtlCheckDlgPos( hwndDlg, TRUE );

      // --- disable change pushbutton if folder is open or analyze for
      //     this folder is running ---
      strcpy( pIda->szBuffer1, pIda->pProp->PropHead.szPath );
      strcat( pIda->szBuffer1, BACKSLASH_STR );
      strcat( pIda->szBuffer1, pIda->pProp->PropHead.szName );
      strcpy( pIda->szBuffer2, "ANALYSIS" );
      strcat( pIda->szBuffer2, pIda->szBuffer1 );
      pIda->szBuffer1[0] = pIda->pProp->chDrive;
      pIda->szBuffer2[0] = pIda->pProp->chDrive;
      if ( EqfQueryObject( pIda->szBuffer1, clsFOLDER, 0) ||
           EqfQueryObject( pIda->szBuffer2, clsANALYSIS, 0) )
      {
        ENABLECTRL( hwndDlg, ID_FOLDERPROPS_SET_PB, FALSE );
        UtlErrorHwnd( WARNING_NO_PROP_CHANGE_ALLOWED, MB_OK,
                      0, NULL, EQF_INFO, hwndDlg );
      } /* endif */

      if ( !pIda->fNew )
      {
        LoadFields_1( hwndDlg, pIda);
      } /* endif */

      SETTEXTLIMIT( hwndDlg, ID_FOLNEW_DICTPID_EF, MAX_DICTPID_VALUES-1 ) ;
      if ( pIda->pProp->szDictPIDSelect1[0] ) {  // Old value in 1.2.4 
         strcpy( pIda->pProp->szDictPIDSelect2, pIda->pProp->szDictPIDSelect1 );
         memset( pIda->pProp->szDictPIDSelect1, NULC, sizeof( pIda->pProp->szDictPIDSelect1) );
      }
      strcpy( pIda->szBuffer1, pIda->pProp->szDictPIDSelect2 ) ;
      OEMTOANSI( pIda->szBuffer1 );
      SETTEXT( hwndDlg, ID_FOLNEW_DICTPID_EF, pIda->szBuffer1 );

      // enable hizontal scrolling in our selection listboxes
      UtlSetHorzScrollingForLB( GetDlgItem( hwndDlg, ID_FOLNEW_MEMSELECT_LB ) );
      UtlSetHorzScrollingForLB( GetDlgItem( hwndDlg, ID_FOLNEW_SELECTED_LB ) );

      mResult = MRFROMSHORT(TRUE);   // leave the focus where we put it

      break;

/*--------------------------------------------------------------------------*/

    case WM_COMMAND:
      switch ( WMCOMMANDID( mp1, mp2 ) )
      {

        case PID_PB_OK :
          if ( mp2 == 0 )   // FALSE = validate and save
          {

            PFOLPROPIDA     pIda;               // ptr to instance data area
            SHORT           sItem;              // index of listbox items
            USHORT          usErrorID;          // ID of control in error
            USHORT          usErrMsg;           // message number to display in UtlError
            PSZ             pszErrParm[4];      // error parameter
            PSZ             pszDicts;           // ptr for dictionary processing
            BOOL            fOldStyleName = TRUE;      // TRUE = name is of old style (8 chars)
            USHORT          usErrParms = 1;     // number of message box parameters

            usErrMsg  = 0;           // no error yet
            usErrorID = 0;           // no erraneous control yet
            pszErrParm[0] = "";      // no error parameter yet


            pIda = ACCESSDLGIDA(hwndDlg, PFOLPROPIDA );


            //--- switch to hourglass pointer while checking the input ---

            SETCURSOR( SPTR_WAIT );


            //--- check selected drive ---
            {
              int iItem = CBQUERYSELECTION( hwndDlg, ID_FOLNEW_DRIVE_PB );
              int nDrive = CBQUERYITEMHANDLE( hwndDlg,ID_FOLNEW_DRIVE_PB, iItem );
             pIda->pProp->chDrive = (CHAR)((CHAR)'A' + nDrive);
            }


            //--- check if a name was specified ---
            if ( pIda->pProp->PropHead.chType == PROP_TYPE_NEW )
            {
              QUERYTEXT( hwndDlg, ID_FOLNEW_NAME_EF, pIda->szBuffer1 );
              UtlStripBlanks( pIda->szBuffer1 );

              if ( pIda->szBuffer1[0] == EOS )
              {
                usErrorID = ID_FOLNEW_NAME_EF;
                usErrMsg  = ERROR_FIELD_INPUT;
              } /* endif */

              // check folder name syntax
              if ( !usErrMsg )
              {
                // make copy of specified name as UtlCheckIntName uses
                // strupr to uppercase the name
                strcpy( pIda->szBuffer2, pIda->szBuffer1 );

                fOldStyleName = UtlCheckIntName( pIda->szBuffer1, pIda->szBuffer1 );
                if ( fOldStyleName )
                {
                  if ( strcmp(pIda->szBuffer2, pIda->szBuffer1) != 0 )
                  {
                    fOldStyleName = FALSE;   // name uses lowercase characters...
                    strcpy( pIda->szBuffer1, pIda->szBuffer2 );
                  } /* endif */
                } /* endif */
                if ( !UtlCheckLongName( pIda->szBuffer1 ) )
                {
                  usErrorID = ID_FOLNEW_NAME_EF;
                  usErrMsg  = ERROR_INV_LONGNAME;
                  pszErrParm[0] = pIda->szBuffer1;
                  usErrParms = 1;
                } /* endif */
              } /* endif */

              //--- check if there is already a folder with the specified name ---
              if ( !usErrMsg )
              {
                CHAR szShortNameBuf[MAX_FILESPEC];

                BOOL fIsNew = FALSE;         // folder-is-new flag
                if ( fOldStyleName )
                {
                  UtlUpper( pIda->szBuffer1 );
                  sItem = SEARCHITEMHWND( pIda->hwndFolLB, pIda->szBuffer1 );
                  fIsNew = !(sItem != LIT_NONE);
                  strcpy( pIda->pProp->PropHead.szName, pIda->szBuffer1 );
                }
                else
                {
                  ANSITOOEM( pIda->szBuffer1 );
                  ObjLongToShortName( pIda->szBuffer1, pIda->pProp->PropHead.szName,
                                      FOLDER_OBJECT, &fIsNew );
                  strcpy( szShortNameBuf, pIda->pProp->PropHead.szName );
                } /* endif */


                if ( !usErrMsg )
                {
                  // we have to check if there is a long name folder using the specified name as
                  // short folder name
                  if ( fIsNew )
                  {
                    CHAR szLongName[MAX_LONGFILESPEC];
                    szLongName[0] = EOS;
                    ObjShortToLongName( szShortNameBuf, szLongName, FOLDER_OBJECT );
                    if ( szLongName[0] != EOS )
                    {
                      OEMTOANSI(szLongName);
                      usErrorID  = ID_FOLNEW_NAME_EF;
                      usErrMsg   = ERROR_FOLDER_SHORTNAME_EXISTS;
                      OEMTOANSI( szShortNameBuf );
                      pszErrParm[0] = szShortNameBuf;
                      pszErrParm[1] = szLongName;
                      usErrParms = 2;
                    } /* endif */
                  } /* endif */
                } /* endif */


                if ( !usErrMsg )
                {
                  if ( !fIsNew && (pIda->pProp->PropHead.chType == PROP_TYPE_NEW) )
                  {
                    usErrorID  = ID_FOLNEW_NAME_EF;
                    usErrMsg   = ERROR_NEWFOLDER_EXISTS;
                    OEMTOANSI( pIda->szBuffer1 );
                    pszErrParm[0] = pIda->szBuffer1;
                  }
                  else
                  {
                    if ( fOldStyleName )
                    {
                      pIda->pProp->szLongName[0] = EOS;
                    }
                    else
                    {
                      strcpy( pIda->pProp->szLongName, pIda->szBuffer1 );
                    } /* endif */
                    strcat( pIda->pProp->PropHead.szName, EXT_FOLDER_MAIN );
                  } /* endif */
                } /* endif */
              } /* endif */
            } /* endif */

            //--- get folder description text ---
            if ( !usErrMsg )
            {
              OEMQUERYTEXT( hwndDlg, ID_FOLNEW_DESCR_EF, pIda->pProp->szDescription);
            } /* endif */

            //--- get / check translation memory ---
            if ( !usErrMsg )
            {
              if ( !QUERYTEXT( hwndDlg, ID_FOLNEW_MEMORY_CBS,
                               pIda->szBuffer1 ))
              {
                usErrorID = ID_FOLNEW_MEMORY_CBS;
                usErrMsg     = ERROR_FIELD_INPUT;
              }
              else
              {
                // is this a new memory db  ???
                BOOL fIsNew = FALSE;
                CHAR szShortName[MAX_FNAME];
                ANSITOOEM( pIda->szBuffer1 );
                ObjLongToShortName( pIda->szBuffer1, szShortName,
                                    TM_OBJECT, &fIsNew );
                if ( fIsNew )
                {
                  //--- set 'dialog is disabled' flag
                  // Note: using WinEnableWindow to disable the dialog
                  //       does not work correctly (probably the dialog
                  //       owner/parentship is not set correctly within
                  //       the memory handler)
                  pIda->fDisabled = TRUE;

                  //--- reset pointer to normal arrow pointer ---
                  // Note: this enables the user to use the mouse in the
                  //       memory create dialog
                  SETCURSOR( SPTR_ARROW );

                  //--- call the memory handler to create a new TM ---
                  strcat( pIda->szBuffer1, X15_STR );
                  QUERYTEXT( hwndDlg, ID_FOLNEW_FORMAT_CBS, pIda->szBuffer2 );
                  strcat( pIda->szBuffer1, pIda->szBuffer2 );
                  strcat( pIda->szBuffer1, X15_STR );
                  QUERYTEXT( hwndDlg, ID_FOLNEW_SOURCELANG_CBS, pIda->szBuffer2 );
                  strcat( pIda->szBuffer1, pIda->szBuffer2 );
                  strcat( pIda->szBuffer1, X15_STR );
                  QUERYTEXT( hwndDlg, ID_FOLNEW_TARGETLANG_CBS, pIda->szBuffer2 );
                  strcat( pIda->szBuffer1, pIda->szBuffer2 );
                  strcat( pIda->szBuffer1, X15_STR );
                  sItem = (SHORT)( ! (BOOL) EqfSend2Handler( MEMORYHANDLER,
                                                    WM_EQF_CREATE,
                                                    MP1FROMSHORT(0),
                                                    MP2FROMP( pIda->szBuffer1 ) ));

                  //--- switch back to hourglass pointer ---
                  SETCURSOR( SPTR_WAIT );

                  //--- reset 'dialog is disabled' flag, dialog is active
                  pIda->fDisabled = FALSE;
                  SETFOCUSHWND( hwndDlg );

                  if ( sItem )
                  {
                    usErrorID = ID_FOLNEW_MEMORY_CBS;
                  }
                  else
                  {

                    //--- update list of memory dbs
                    CBDELETEALL( hwndDlg, ID_FOLNEW_MEMORY_CBS );
                    EqfSend2Handler( MEMORYHANDLER,
                                     WM_EQF_INSERTNAMES,
                                     MP1FROMHWND( WinWindowFromID( hwndDlg,
                                                                   ID_FOLNEW_MEMORY_CBS) ),
                                     MP2FROMP( MEMORY_ALL ) );

                    OEMTOANSI( pIda->szBuffer1 );
                    SETTEXT( hwndDlg, ID_FOLNEW_MEMORY_CBS, pIda->szBuffer1 );
                    ANSITOOEM( pIda->szBuffer1 );
                  } /* endif */
                } /* endif */
                if ( !usErrorID )
                {
                  BOOL fIsNew = FALSE;         // folder-is-new flag
                  ObjLongToShortName( pIda->szBuffer1, pIda->pProp->szMemory,
                                      TM_OBJECT, &fIsNew );
                  strcpy( pIda->pProp->szLongMemory, pIda->szBuffer1 );
                } /* endif */
              } /* endif */
            } /* endif */

            //--- get folder format table ---
            if ( !usErrMsg )
            {
              if ( !QUERYTEXT( hwndDlg, ID_FOLNEW_FORMAT_CBS, pIda->pProp->szFormat ) )
              {
                usErrorID = ID_FOLNEW_FORMAT_CBS;
                usErrMsg  = ERROR_FIELD_INPUT;
              } /* endif */
            } /* endif */

            //--- get folder editor ---
            if ( !usErrMsg )
            {
              if ( !QUERYTEXT( hwndDlg, ID_FOLNEW_EDITOR_CBS,
                               pIda->szBuffer1 ) )
              {
                usErrorID = ID_FOLNEW_EDITOR_CBS;
                usErrMsg  = ERROR_FIELD_INPUT;
              }
              else
              {
                strcpy( pIda->pProp->szEditor, pIda->szBuffer1 );
              } /* endif */
            } /* endif */


            /********************************************************/
            /* Get folder source language                           */
            /********************************************************/
            if ( !usErrMsg && !usErrorID )
            {
              QUERYTEXT ( hwndDlg, ID_FOLNEW_SOURCELANG_CBS,
                          pIda->pProp->szSourceLang );
              if ( pIda->pProp->szSourceLang[0] == EOS )
              {
                usErrorID = ID_FOLNEW_SOURCELANG_CBS;
                usErrMsg  = ERROR_NO_SOURCELANG;
              } /* endif */
            } /* endif */

            /********************************************************/
            /* Get folder target language                           */
            /********************************************************/
            if ( !usErrMsg && !usErrorID )
            {
              QUERYTEXT( hwndDlg, ID_FOLNEW_TARGETLANG_CBS, pIda->pProp->szTargetLang );
              if ( pIda->pProp->szTargetLang[0] == EOS )
              {
                usErrorID = ID_FOLNEW_TARGETLANG_CBS;
                usErrMsg  = ERROR_NO_TARGETLANG;
              } /* endif */
            } /* endif */

            // get folder conversion
            //QUERYTEXT( hwndDlg, ID_FOLNEW_CONV_CBS, pIda->pProp->szConversion );
            //UtlStripBlanks( pIda->pProp->szConversion );
            pIda->pProp->szConversion[0] = EOS;

            /********************************************************/
            /* Check source language against target language        */
            /********************************************************/
            if ( !usErrMsg && !usErrorID )
            {
              if ( strcmp( pIda->pProp->szSourceLang,
                           pIda->pProp->szTargetLang ) == 0 )
              {
                if ( UtlErrorHwnd( ERROR_MEM_SAME_LANGUAGES,
                                   MB_YESNO | MB_DEFBUTTON2,
                                   0, NULL, EQF_QUERY, hwndDlg ) == MBID_NO )
                {
                  usErrorID = ID_FOLNEW_TARGETLANG_CBS;
                } /* endif */
              } /* endif */
            } /* endif */

            /**********************************************************/
            /* check that selected editor supports both source and tgt*/
            /* language                                               */
            /**********************************************************/
            if ( !usErrMsg && !usErrorID )
            {
              PSZ pFail = UtlEditorLangSupport( pIda->pProp->szEditor,
                                                pIda->pProp->szSourceLang,
                                                pIda->pProp->szTargetLang );
              if ( pFail )
              {
                pszErrParm[0] = pFail;
                UtlErrorHwnd( TB_LANG_NOT_SUPPORTED_BY_EDITOR,
                              MB_CANCEL,
                              1, pszErrParm, EQF_ERROR, hwndDlg );

                usErrorID = ID_FOLNEW_EDITOR_CBS;
              } /* endif */
            } /* endif */

            // get selected conversion
            if ( !usErrMsg && !usErrorID )
            {
              //QUERYTEXT( hwndDlg, ID_FOLNEW_CONV_CBS, pIda->pProp->szConversion );
              //UtlStripBlanks( pIda->pProp->szConversion );
              pIda->pProp->szConversion[0] = EOS;
            } /* endif */

            //-- check selected dictionaries ---
            if ( !usErrMsg && !usErrorID )
            {
              sItem = QUERYITEMCOUNT( hwndDlg, ID_FOLNEW_SELECTED_LB );
              if ( sItem > NUM_OF_FOLDER_DICS )
              {
                usErrorID = ID_FOLNEW_SELECTED_LB;
                usErrMsg  = ERROR_FIELD_INPUT;
              } /* endif */
            } /* endif */

            //-- get selected dictionaries ---
            if ( !usErrMsg && !usErrorID )
            {
              USHORT  usNoOfEntries;

              pszDicts = (PSZ) pIda->pProp->DicTbl;
              memset( pszDicts, NULC, sizeof( pIda->pProp->DicTbl) );
              usNoOfEntries = QUERYITEMCOUNTHWND( pIda->hDicLBS );
              sItem = 0;
              memset( pIda->pProp->aLongDicTbl, NULC, sizeof(pIda->pProp->aLongDicTbl) );
              while ( sItem < (SHORT)usNoOfEntries )
              {
                QUERYITEMTEXTHWND( pIda->hDicLBS, sItem, pIda->szBuffer1 );
                ANSITOOEM( pIda->szBuffer1 );
                strcpy( pIda->pProp->aLongDicTbl[sItem], pIda->szBuffer1 );
                sItem++;
              } /* endwhile */
              if ( sItem > 0 ) {
                 pIda->pProp->fDictPIDSelect = QUERYCHECK( hwndDlg, ID_FOLNEW_DICTPID_CHK );
              } else {
                 pIda->pProp->fDictPIDSelect = FALSE;
              }
              memset( pIda->pProp->szDictPIDSelect2, NULC, sizeof( pIda->pProp->szDictPIDSelect2) );
              QUERYTEXT( hwndDlg, ID_FOLNEW_DICTPID_EF, pIda->pProp->szDictPIDSelect2 );
              ANSITOOEM( pIda->pProp->szDictPIDSelect2 );
              CheckDictPIDValues( pIda->pProp->szDictPIDSelect2 );
            } /* endif */

            //-- check selected translation memory databases
            if ( !usErrMsg && !usErrorID )
            {
              sItem = QUERYITEMCOUNT( hwndDlg, ID_FOLNEW_MEMSELECT_LB );
              if ( sItem > MAX_NUM_OF_FOLDER_MDB )
              {
                usErrorID = ID_FOLNEW_MEMSELECT_LB;
                usErrMsg  = ERROR_FIELD_INPUT;
              } /* endif */
            } /* endif */

            //-- get selected translation memory databases
            if ( !usErrMsg && !usErrorID )
            {
              USHORT  usNoOfEntries;
              memset( pIda->pProp->MemTbl, NULC, sizeof( pIda->pProp->MemTbl) );
              memset( pIda->pProp->aLongMemTbl, NULC, sizeof(pIda->pProp->aLongMemTbl) );
              usNoOfEntries = QUERYITEMCOUNTHWND( pIda->hMemLBS );
              sItem = 0;
              while ( sItem < (SHORT)usNoOfEntries )
              {
                QUERYITEMTEXTHWND( pIda->hMemLBS, sItem, pIda->szBuffer1 );
                ANSITOOEM( pIda->szBuffer1 );
                strcpy( pIda->pProp->aLongMemTbl[sItem], pIda->szBuffer1 );
                sItem++;
              } /* endwhile */
            } /* endif */

            //-- get state of stop at first... checkbox
            if ( !usErrMsg && !usErrorID )
            {
              pIda->pProp->fStopAtFirstExact = QUERYCHECK( hwndDlg, ID_FOLNEW_STOPATFIRSTEXACT_CHK );
            } /* endif */

            //--- reset pointer to normal arrow pointer ---
            // Note: this is done to allow the usage of the mouse in
            //       messages boxes popped up by UtlError
            SETCURSOR( SPTR_ARROW );

            if ( !usErrMsg && !usErrorID )
            {
              //-- issue warning if no dictionary selected ---
              sItem = QUERYITEMCOUNTHWND( pIda->hDicLBS );
              if ( !sItem )
              {
                if ( pIda->pProp->PropHead.chType == PROP_TYPE_NEW ||
                     pIda->pProp->PropHead.chType == PROP_TYPE_TEMPLATE)
                {
                  iMBRC = UtlErrorHwnd( ERROR_NODICT_SELECTED,
                                         MB_YESNO, 0, (PSZ *)NULP,
                                         EQF_QUERY, hwndDlg);
                  if ( iMBRC == MBID_NO )
                  {
                    usErrorID = ID_FOLNEW_DICCHANGE_PB;
                  } /* endif */
                } /* endif */
              } /* endif */
            } /* endif */

            //-- issue error message if needed ---
            if ( usErrMsg )
            {
              UtlErrorHwnd( usErrMsg, MB_OK, usErrParms, pszErrParm, EQF_ERROR,
                            hwndDlg);
            } /* endif */

            //-- set focus to erraneous control ---
            if ( usErrorID )
            {
              SETFOCUS( hwndDlg, usErrorID );
            } /* endif */

            /********************************************************/
            /* Check if folder properties or folder directory       */
            /* exists and issue approbriate error message           */
            /********************************************************/
            if ( !usErrMsg && !usErrorID &&
                 (pIda->pProp->PropHead.chType == PROP_TYPE_NEW) )
            {
              /******************************************************/
              /* Check if properties for this folder exist          */
              /******************************************************/
              UtlMakeEQFPath( pIda->szBuffer1, NULC, PROPERTY_PATH, NULL );
              strcat( pIda->szBuffer1, BACKSLASH_STR );
              strcat( pIda->szBuffer1, pIda->pProp->PropHead.szName );
              if ( UtlFileExist( pIda->szBuffer1 ) )
              {
                usErrMsg = ERROR_FOLDER_FILES_EXIST;
              } /* endif */

              /******************************************************/
              /* Check if folder directory exists                   */
              /******************************************************/
              if ( !usErrMsg )
              {
                strcpy( pIda->szBuffer1, pIda->pProp->PropHead.szPath );
                strcat( pIda->szBuffer1, BACKSLASH_STR );
                strcat( pIda->szBuffer1, pIda->pProp->PropHead.szName );
                pIda->szBuffer1[0] = pIda->pProp->chDrive;
                if ( UtlFileExist( pIda->szBuffer1 ) )
                {
                  usErrMsg = ERROR_FOLDER_FILES_EXIST;
                } /* endif */
              } /* endif */


              if ( usErrMsg )
              {
                Utlstrccpy( pIda->szBuffer1, pIda->pProp->PropHead.szName,
                            DOT );
                pszErrParm[0] = pIda->szBuffer1;
                UtlErrorHwnd( usErrMsg, MB_OK, 1, pszErrParm, EQF_ERROR,
                              hwndDlg);
                SETFOCUS( hwndDlg, ID_FOLNEW_NAME_EF );
              } /* endif */
            } /* endif */




            if (!usErrMsg && !usErrorID )
            {
              mResult = MRFROMSHORT( TRUE );  // result value of procedure
            }
            else
            {
              mResult = FALSE;
            }

          }//end case
          break;

          // ---------------------------------- end case PID_PB_OK -----------------------------




        case ID_FOLNEW_EDITOR_CBS:

          pIda = ACCESSDLGIDA(hwndDlg, PFOLPROPIDA );

          if ( (sNotification == CBN_EFCHANGE) &&
               (pIda->pProp->PropHead.chType != PROP_TYPE_NEW ) &&
               (!pIda->fCBChange) )
          {
            QUERYTEXT( hwndDlg, ID_FOLNEW_EDITOR_CBS, pIda->szBuffer1);
            if ( stricmp( pIda->pProp->szEditor, pIda->szBuffer1 ) != 0 )
            {
              // check if folder is open
              strcpy( pIda->szBuffer1, pIda->pProp->PropHead.szPath );
              pIda->szBuffer1[0] = pIda->pProp->chDrive;
              strcat( pIda->szBuffer1, "\\" );
              strcat( pIda->szBuffer1, pIda->pProp->PropHead.szName );
              if ( EqfQueryObject( pIda->szBuffer1, clsFOLDER, 0) )
              {
                UtlErrorHwnd( ERROR_FOLPROP_FOLDER_IS_OPEN,
                              MB_CANCEL,
                              0, (PSZ *)NULP,
                              EQF_ERROR, hwndDlg );
                pIda->fCBChange = TRUE;
                SETTEXT( hwndDlg, ID_FOLNEW_EDITOR_CBS, pIda->pProp->szEditor );
                pIda->fCBChange = FALSE;
              } /* endif */
            } /* endif */
          } /* endif */
          break;


        case ID_FOLNEW_DESCR_EF:

          if ( sNotification == EN_KILLFOCUS )
          {
            ClearIME( hwndDlg );
          } /* endif */
          break;

/*--------------------------------------------------------------------------*/
        case DM_GETDEFID:
          // GQ: The handling of the DM_GETDEFID message here is necessary for the outer
          //     dialog procedure which does not get this message if this code is missing
          {
            HWND   hwndFocus;          // handle of focus window

            if ( GetKeyState(VK_RETURN) & 0x8000 )
            {
              hwndFocus = GetFocus();
              if ( hwndFocus == GetDlgItem( hwndDlg, ID_FOLNEW_SELECTED_LB ) )
              {
                FolNewControl( hwndDlg, ID_FOLNEW_SELECTED_LB, LN_ENTER );
                mResult = TRUE;
              }
              else if ( hwndFocus == GetDlgItem( hwndDlg, ID_FOLNEW_MEMSELECT_LB ) )
              {
                FolNewControl( hwndDlg, ID_FOLNEW_MEMSELECT_LB, LN_ENTER );
                mResult = TRUE;
              } /* endif */
            } /* endif */

          }

          mResult = TRUE;
          break;
        case ID_FOLNEW_BASEFOLDER_PB:
          {
            HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
            pIda = ACCESSDLGIDA(hwndDlg, PFOLPROPIDA );

            DIALOGBOX( hwndDlg, FOLMODELDLGPROC, hResMod, ID_FOLMODEL_DLG, pIda, iMBRC );
            if ( iMBRC )
            {
              LoadFields_1( hwndDlg, pIda);
            } /* endif */
          }
          break;

        case ID_FOLNEW_SELECTED_LB:
          if ( sNotification == LN_ENTER )
          {
            pIda = ACCESSDLGIDA(hwndDlg, PFOLPROPIDA );

            sItem = QUERYSELECTION( hwndDlg, ID_FOLNEW_SELECTED_LB );
            DELETEITEM( hwndDlg, ID_FOLNEW_SELECTED_LB, sItem );
            //--- if this wasn't the last listbox item ... ---
            if ( QUERYITEMCOUNT( hwndDlg, ID_FOLNEW_SELECTED_LB)  != 0 )
            {
              //--- ... select first listbox item ---
              SELECTITEM( hwndDlg, ID_FOLNEW_SELECTED_LB, 0 );
            } else {
               pIda->pProp->fDictPIDSelect = FALSE ;
               ENABLECTRL( hwndDlg, ID_FOLNEW_DICTPID_CHK, pIda->pProp->fDictPIDSelect );
               ENABLECTRL( hwndDlg, ID_FOLNEW_DICTPID_EF, pIda->pProp->fDictPIDSelect );
               ENABLECTRL( hwndDlg, ID_FOLNEW_DICTPID_PB, pIda->pProp->fDictPIDSelect );
               SETCHECK( hwndDlg, ID_FOLNEW_DICTPID_CHK, pIda->pProp->fDictPIDSelect );
            }

            // adjust our list of dictionaries
            {
              USHORT  usNoOfEntries;

              memset( pIda->pProp->DicTbl, NULC, sizeof( pIda->pProp->DicTbl) );
              usNoOfEntries = QUERYITEMCOUNT( hwndDlg, ID_FOLNEW_SELECTED_LB );
              sItem = 0;
              memset( pIda->pProp->aLongDicTbl, NULC, sizeof(pIda->pProp->aLongDicTbl) );
              while ( sItem < (SHORT)usNoOfEntries )
              {
                QUERYITEMTEXT( hwndDlg, ID_FOLNEW_SELECTED_LB, sItem, pIda->szBuffer1 );
                ANSITOOEM( pIda->szBuffer1 );
                strcpy( pIda->pProp->aLongDicTbl[sItem], pIda->szBuffer1 );
                sItem++;
              } /* endwhile */
            }
          } /* endif */
          break;

        case ID_FOLNEW_MEMSELECT_LB:
          if ( sNotification == LN_ENTER )
          {
            pIda = ACCESSDLGIDA(hwndDlg, PFOLPROPIDA );
            sItem = QUERYSELECTION( hwndDlg, ID_FOLNEW_MEMSELECT_LB );
            DELETEITEM( hwndDlg, ID_FOLNEW_MEMSELECT_LB, sItem );
            //--- if this wasn't the last listbox item ... ---
            if ( QUERYITEMCOUNT( hwndDlg, ID_FOLNEW_MEMSELECT_LB)  != 0 )
            {
              //--- ... select first listbox item ---
              SELECTITEM( hwndDlg, ID_FOLNEW_MEMSELECT_LB, 0 );
            } /* endif */

            // adjust list of memories
            {
              USHORT  usNoOfEntries;
              memset( pIda->pProp->MemTbl, NULC, sizeof( pIda->pProp->MemTbl) );
              memset( pIda->pProp->aLongMemTbl, NULC, sizeof(pIda->pProp->aLongMemTbl) );
              usNoOfEntries = QUERYITEMCOUNT( hwndDlg, ID_FOLNEW_MEMSELECT_LB );
              sItem = 0;
              while ( sItem < (SHORT)usNoOfEntries )
              {
                QUERYITEMTEXT( hwndDlg, ID_FOLNEW_MEMSELECT_LB, sItem, pIda->szBuffer1 );
                ANSITOOEM( pIda->szBuffer1 );
                strcpy( pIda->pProp->aLongMemTbl[sItem], pIda->szBuffer1 );
                sItem++;
              } /* endwhile */
            } 
          } /* endif */
          break;

        case ID_FOLNEW_DICCHANGE_PB:
        case ID_FOLNEW_MEMCHANGE_PB:
          pIda = ACCESSDLGIDA(hwndDlg, PFOLPROPIDA );
          pIda->fMemSel = (WMCOMMANDID( mp1, mp2 ) == ID_FOLNEW_MEMCHANGE_PB);

          if ( FolShowSelectionDialog( hwndDlg, pIda, FALSE ) )
          {
            // refresh list of selected memories or dictionaries
            if ( pIda->fMemSel )
            {
              int i = 0;
              WinSendMsg( pIda->hMemLBS, LM_DELETEALL, NULL, NULL);
              while ( (i < MAX_NUM_OF_READONLY_MDB) && (pIda->pProp->aLongMemTbl[i][0] != EOS) )
              {
                strcpy( pIda->szBuffer1, pIda->pProp->aLongMemTbl[i] );
                OEMTOANSI( pIda->szBuffer1 );
                INSERTITEMENDHWND( pIda->hMemLBS, pIda->szBuffer1  );
                i++;
              } /* endwhile */
              UtlSetHorzScrollingForLB( GetDlgItem( hwndDlg, ID_FOLNEW_MEMSELECT_LB ) );
            }
            else
            {
              int i = 0;
              WinSendMsg( pIda->hDicLBS, LM_DELETEALL, NULL, NULL);
              while ( (i < MAX_NUM_OF_FOLDER_DICS) && (pIda->pProp->aLongDicTbl[i][0] != EOS) )
              {
                strcpy( pIda->szBuffer1, pIda->pProp->aLongDicTbl[i] );
                OEMTOANSI( pIda->szBuffer1 );
                INSERTITEMENDHWND( pIda->hDicLBS, pIda->szBuffer1  );
                i++;
              } /* endwhile */
              UtlSetHorzScrollingForLB( GetDlgItem( hwndDlg, ID_FOLNEW_SELECTED_LB ) );
              if ( i > 0 ) 
              {
                 ENABLECTRL( hwndDlg, ID_FOLNEW_DICTPID_CHK, TRUE );
              } else {
                 pIda->pProp->fDictPIDSelect = FALSE ;
                 ENABLECTRL( hwndDlg, ID_FOLNEW_DICTPID_CHK, pIda->pProp->fDictPIDSelect );
              }
              ENABLECTRL( hwndDlg, ID_FOLNEW_DICTPID_EF, pIda->pProp->fDictPIDSelect );
              ENABLECTRL( hwndDlg, ID_FOLNEW_DICTPID_PB, pIda->pProp->fDictPIDSelect );
              SETCHECK( hwndDlg, ID_FOLNEW_DICTPID_CHK, pIda->pProp->fDictPIDSelect );
            } /* endif */
          } /* endif */
          break;

        case ID_FOLNEW_DICTPID_CHK:          // PID selection checkbox
          pIda = ACCESSDLGIDA(hwndDlg, PFOLPROPIDA );
          pIda->pProp->fDictPIDSelect = QUERYCHECK( hwndDlg, ID_FOLNEW_DICTPID_CHK );
          ENABLECTRL( hwndDlg, ID_FOLNEW_DICTPID_EF, pIda->pProp->fDictPIDSelect );
          ENABLECTRL( hwndDlg, ID_FOLNEW_DICTPID_PB, pIda->pProp->fDictPIDSelect );
          break;

        case ID_FOLNEW_DICTPID_PB:       // PID selection "Select" pushbutton 
          pIda = ACCESSDLGIDA(hwndDlg, PFOLPROPIDA );
          FolDictPIDSelection( hwndDlg, pIda ) ;
          break;

          // ------------------------- AFC Folder -----------------------------
        case AFC_PASSWORD_OK:
          {
            pIda = ACCESSDLGIDA(hwndDlg, PFOLPROPIDA );
// WL   // pIda->pProp->fTCMasterFolder = TRUE;  // really TCMaster or only enable all entry fields?
            ENABLECTRL( hwndDlg, ID_FOLNEW_FORMAT_CBS, TRUE );
            ENABLECTRL( hwndDlg, ID_FOLNEW_MEMORY_CBS, TRUE );
            ENABLECTRL( hwndDlg, ID_FOLNEW_SOURCELANG_CBS, TRUE );
            ENABLECTRL( hwndDlg, ID_FOLNEW_TARGETLANG_CBS, TRUE );
//            ENABLECTRL( hwndDlg, ID_FOLNEW_CONV_CBS, TRUE );
            ENABLECTRL( hwndDlg, ID_FOLNEW_EDITOR_CBS, TRUE );
            mResult=TRUE;
          }
          break;


        default:
          break;



      }//end switch
      break;

    case WM_HELP:
      {

        /*************************************************************/
        /* pass on a HELP_WM_HELP request                            */
        /*************************************************************/


        pIda = ACCESSDLGIDA(hwndDlg, PFOLPROPIDA );

        if ( pIda->pProp->PropHead.chType != PROP_TYPE_NEW) // existing folder
        {
          EqfDisplayContextHelp( (HWND)((LPHELPINFO) mp2)->hItemHandle,
                                 &hlpsubtblFolderProps[0] );
        }
        else   // new folder
        {
          EqfDisplayContextHelp( (HWND)((LPHELPINFO) mp2)->hItemHandle,
                                 &hlpsubtblCreateFolder[0] );
        }

        mResult = TRUE;  // message processed
        break;
      } // end case WM_HELP
  } /* endswitch */

  return mResult;
};





//+----------------------------------------------------------------------------+
//|Internal function  UPDATE_ID_TRANSLATOR_NAME_EF                             |
//+----------------------------------------------------------------------------+
//|Function name:                                                              |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//+----------------------------------------------------------------------------+
//|Parameters:                                                                 |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Returncode type:                                                            |
//+----------------------------------------------------------------------------+
//|Returncodes:                                                                |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                                                                            |
//|                                                                            |
//|                                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+


MRESULT APIENTRY UPDATE_ID_TRANSLATOR_NAME_EF
(
PFOLPROPIDA   pIda,
HWND hwndDlg                                // handle of dialog window
)

{

  //---- init project translator's name field --------------------

  ENABLECTRL( hwndDlg, ID_TRANSLATOR_NAME_EF, TRUE );

  CBDELETEALL(hwndDlg, ID_TRANSLATOR_NAME_EF);


  OEMTOANSI( pIda->pProp->szVendor);
  CBINSERTITEMEND( hwndDlg,ID_TRANSLATOR_NAME_EF , pIda->pProp->szVendor);
  ANSITOOEM( pIda->pProp->szVendor);


  {

    HPROP           hFLLProp;   // folder list properties handler
    PPROPFOLDERLIST pFLLProp;   // folder list properties pointer
    EQFINFO         ErrorInfo;  // error returned by property handler
    int             i;

    /************************************************************/
    /* Select last-used values                                  */
    /************************************************************/
    UtlMakeEQFPath( pIda->szBuffer1, NULC, SYSTEM_PATH, NULL );
    strcat( pIda->szBuffer1, BACKSLASH_STR );
    strcat( pIda->szBuffer1, DEFAULT_FOLDERLIST_NAME );
    hFLLProp = OpenProperties( pIda->szBuffer1, NULL,
                               PROP_ACCESS_READ, &ErrorInfo);
    if ( hFLLProp )
    {
      pFLLProp = (PPROPFOLDERLIST)MakePropPtrFromHnd( hFLLProp );

      i=0;
      while (i < 5 && pFLLProp->szTranslatorList[i][0] != EOS)
      {
        SHORT sItem = CBSEARCHITEM(hwndDlg,ID_TRANSLATOR_NAME_EF ,
                                   pFLLProp->szTranslatorList[i]);

        if (sItem == LIT_NONE)
        {

          OEMTOANSI( pFLLProp->szTranslatorList[i]);
          CBINSERTITEMEND( hwndDlg,ID_TRANSLATOR_NAME_EF , pFLLProp->szTranslatorList[i]);
          ANSITOOEM( pFLLProp->szTranslatorList[i]);
        }
        i++;

      }// end while



      CloseProperties( hFLLProp, PROP_QUIT, &ErrorInfo);
    } /* endif */


  }

  {
    SHORT sItem = CBSEARCHITEM(hwndDlg,ID_TRANSLATOR_NAME_EF ,
                               pIda->pProp->szVendor);
    CBSELECTITEM (hwndDlg,ID_TRANSLATOR_NAME_EF ,max(sItem,0) );

  }


  return( 0 );

} // end of function UPDATE_ID_TRANSLATOR_NAME_EF




//+----------------------------------------------------------------------------+
//|Internal function  FOLDERPROPS_AFC_DLGPROC                                  |
//+----------------------------------------------------------------------------+
//|Function name:                                                              |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//+----------------------------------------------------------------------------+
//|Parameters:                                                                 |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Returncode type:                                                            |
//+----------------------------------------------------------------------------+
//|Returncodes:                                                                |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                                                                            |
//|                                                                            |
//|                                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+

INT_PTR CALLBACK FOLDERPROPS_AFC_DLGPROC
(
HWND hwndDlg,                                // handle of dialog window
WINMSG msg,
WPARAM mp1,
LPARAM mp2
)
{
  MRESULT  mResult = MRFROMSHORT( TRUE );  // result value of procedure

  PFOLPROPIDA   pIda;                       // ptr to instance data area

  SHORT sNotification= WMCOMMANDCMD( mp1, mp2 );

  switch ( msg )
  {
    case WM_INITDLG:
      {

        pIda = (PFOLPROPIDA)PVOIDFROMMP2( mp2 );
        ANCHORDLGIDA( hwndDlg, pIda );

        /*********************************************/
        /* Handle User Name and Password             */
        /*********************************************/

        FolFillHistlogSize( pIda, hwndDlg  );



        // set entry field length limit
        SETTEXTLIMIT (hwndDlg, ID_PASSWORD_EF, 6);


        if ( pIda->pProp->PropHead.chType != PROP_TYPE_NEW)   // existing folder
        {

          //--- init fields for existing folder properties ---

          pIda->fNew = FALSE;

          if ( pIda->pProp->fAFCFolder)  // the existing folder is an AFC folder
          {
            if ( pIda->pProp->fTCMasterFolder) // the existing folder is also a TC Master Folder
            {
              ENABLECTRL( hwndDlg, ID_TCMASTER_FOLDER_EF, FALSE );
              SETTEXT( hwndDlg, ID_TCMASTER_FOLDER_EF, "(Master Folder)" );
            }
            else
            {
              ENABLECTRL( hwndDlg,ID_TCMASTER_FOLDER_EF , FALSE );
              SETTEXT( hwndDlg, ID_TCMASTER_FOLDER_EF, "(Controlled Folder - restricted rights)" );
            }

            //---- init the checkbox field -------------------------------


            SETCHECK_TRUE( hwndDlg, DID_AFC_FOLDER_CHK );

            if ( pIda->pProp->fTCMasterFolder) // the existing folder is also a TC Master Folder
            {
              ENABLECTRL( hwndDlg, DID_AFC_FOLDER_CHK, TRUE );
            }
            else
            {
              ENABLECTRL( hwndDlg, DID_AFC_FOLDER_CHK, FALSE );
            }

            //---- init the password field ---------------------------------


            if ( pIda->pProp->fTCMasterFolder) // the existing folder is also a TC Master Folder
            {
              if (pIda->pProp->szAFCPassword[0] != EOS )
              {
                OEMTOANSI(pIda->pProp->szAFCPassword);
                SETTEXT( hwndDlg, ID_PASSWORD_EF, pIda->pProp->szAFCPassword );
                ANSITOOEM(pIda->pProp->szAFCPassword);
                ENABLECTRL( hwndDlg, ID_PASSWORD_EF, TRUE );
              }
              else
              {
                ENABLECTRL( hwndDlg, ID_PASSWORD_EF, TRUE );
              }
            }
            else
            {
              // SETTEXT( hwndDlg, ID_PASSWORD_EF, "" );   // do not show the Master's Password
              ENABLECTRL( hwndDlg, ID_PASSWORD_EF, TRUE );
            }


            //---- init the project coordinator's name field ------------------


            if ( pIda->pProp->fTCMasterFolder) // the existing folder is also a TC Master Folder
            {
              ENABLECTRL( hwndDlg, ID_PROJCOORD_NAME_EF, TRUE  );
            }
            else
            {
              ENABLECTRL( hwndDlg, ID_PROJCOORD_NAME_EF, FALSE  );
            }

            if ( pIda->pProp->szCoordinator[0] != EOS )
            {
              OEMTOANSI(pIda->pProp->szCoordinator);
              SETTEXT( hwndDlg, ID_PROJCOORD_NAME_EF, pIda->pProp->szCoordinator );
              ANSITOOEM(pIda->pProp->szCoordinator);
            }


            //---- init project coordinator's mail field --------------------

            if ( pIda->pProp->fTCMasterFolder) // the existing folder is also a TC Master Folder
            {
              ENABLECTRL( hwndDlg, ID_PROJCOORD_MAIL_EF, TRUE );
            }
            else
            {
              ENABLECTRL( hwndDlg, ID_PROJCOORD_MAIL_EF, FALSE );
            }

            if ( pIda->pProp->szCoordinatorEMail[0] != EOS )
            {
              OEMTOANSI(pIda->pProp->szCoordinatorEMail);
              SETTEXT( hwndDlg, ID_PROJCOORD_MAIL_EF, pIda->pProp->szCoordinatorEMail );
              ANSITOOEM(pIda->pProp->szCoordinatorEMail);
            }


            //---- init project originator's name field --------------------

            if ( pIda->pProp->fTCMasterFolder) // the existing folder is also a TC Master Folder
            {
              // the originator field is filled by the export process and cannot be changed manually

              ENABLECTRL( hwndDlg, ID_ORIGINATOR_NAME_EF, FALSE );
            }
            else
            {
              ENABLECTRL( hwndDlg, ID_ORIGINATOR_NAME_EF, FALSE );
            }

            if ( pIda->pProp->szParent[0] != EOS )
            {
//              OEMTOANSI(pIda->pProp->szParent);
              SETTEXT( hwndDlg, ID_ORIGINATOR_NAME_EF, pIda->pProp->szParent );
//              ANSITOOEM(pIda->pProp->szParent);
            }


            //---- init project originator's mail field --------------------

            if ( pIda->pProp->fTCMasterFolder) // the existing folder is also a TC Master Folder
            {
              // the originator field is filled by the export process and cannot be changed manually

              ENABLECTRL( hwndDlg, ID_ORIGINATOR_MAIL_EF, FALSE );
            }
            else
            {
              ENABLECTRL( hwndDlg, ID_ORIGINATOR_MAIL_EF, FALSE );
            }

            if ( pIda->pProp->szParentEMail[0] != EOS )
            {
//              OEMTOANSI(pIda->pProp->szParentEMail);
              SETTEXT( hwndDlg, ID_ORIGINATOR_MAIL_EF, pIda->pProp->szParentEMail );
//              ANSITOOEM(pIda->pProp->szParentEMail);
            }


            //---- init project translator's name field --------------------


            UPDATE_ID_TRANSLATOR_NAME_EF(pIda, hwndDlg);



            //---- init project translator's mail field --------------------

            ENABLECTRL( hwndDlg, ID_TRANSLATOR_MAIL_EF, TRUE );
            if ( pIda->pProp->szVendorEMail[0] != EOS )
            {
              OEMTOANSI( pIda->pProp->szVendorEMail);
              SETTEXT( hwndDlg, ID_TRANSLATOR_MAIL_EF, pIda->pProp->szVendorEMail );
              ANSITOOEM( pIda->pProp->szVendorEMail);
            }


          } // end if AFC Folder = TRUE
          else //Folder is an existing folder, but not an AFC Folder
          {

            ENABLECTRL( hwndDlg, ID_TCMASTER_FOLDER_EF, FALSE );
            ENABLECTRL( hwndDlg, ID_PASSWORD_EF, FALSE );

            ENABLECTRL( hwndDlg, ID_PROJCOORD_NAME_EF, FALSE );
            ENABLECTRL( hwndDlg, ID_PROJCOORD_MAIL_EF, FALSE );

            ENABLECTRL( hwndDlg, ID_ORIGINATOR_NAME_EF, FALSE );
            ENABLECTRL( hwndDlg, ID_ORIGINATOR_MAIL_EF, FALSE );

            ENABLECTRL( hwndDlg, ID_TRANSLATOR_NAME_EF, FALSE );
            ENABLECTRL( hwndDlg, ID_TRANSLATOR_MAIL_EF, FALSE );

          } // end else
        } // end if Folder is not a new folder
        else // folder is a new folder
        {
          ENABLECTRL( hwndDlg, ID_TCMASTER_FOLDER_EF, FALSE );

          ENABLECTRL( hwndDlg, ID_PASSWORD_EF, FALSE );

          ENABLECTRL( hwndDlg, ID_PROJCOORD_NAME_EF, FALSE );
          ENABLECTRL( hwndDlg, ID_PROJCOORD_MAIL_EF, FALSE );

          ENABLECTRL( hwndDlg, ID_ORIGINATOR_NAME_EF, FALSE );
          ENABLECTRL( hwndDlg, ID_ORIGINATOR_MAIL_EF, FALSE );

          ENABLECTRL( hwndDlg, ID_TRANSLATOR_NAME_EF, FALSE );
          ENABLECTRL( hwndDlg, ID_TRANSLATOR_MAIL_EF, FALSE );

        }  // end else



        mResult = MRFROMSHORT(TRUE);   // leave the focus where we put it
        mResult = TRUE;
      }
      break;  //INIT


    case WM_COMMAND:
      switch ( WMCOMMANDID( mp1, mp2 ) )
      {
        case PID_PB_OK :
          {

            USHORT          usErrMsg = 0;           // message number to display in UtlError


            pIda = ACCESSDLGIDA(hwndDlg, PFOLPROPIDA );


            if ( QUERYCHECK( hwndDlg, DID_AFC_FOLDER_CHK) )    // checkbox "AFC Folder" is active
            {

              if ( pIda->pProp->PropHead.chType == PROP_TYPE_NEW)   // new folder -> becomes TCMaster folder
              {
                pIda->pProp->fTCMasterFolder = TRUE;
                pIda->pProp->fAFCFolder = TRUE;
              }
              else  //existing folder
              {
                if ( pIda->pProp->fAFCFolder) // existing folder is an AFC folder
                {
                  // -> nothing to change
                }
                else  // folder is an existing folder, but neither AFC nor TCMaster folder
                {
                  // -> becomes TCMaster folder
                  pIda->pProp->fTCMasterFolder = TRUE;
                  pIda->pProp->fAFCFolder = TRUE;
                }
              }


              //--- get password ---

              if ( pIda->pProp->fTCMasterFolder) // only TCMaster folder has the right to change the password
              {
                if ( !usErrMsg )
                {
                  OEMQUERYTEXT( hwndDlg, ID_PASSWORD_EF, pIda->pProp->szAFCPassword);
                } /* endif */
              }

              if ( !usErrMsg )   // check if password field is not empty -> mandatory!
              {
                 if ( ( pIda->pProp->szAFCPassword== NULL) || (*(pIda->pProp->szAFCPassword) == EOS) )
                 {
                   usErrMsg = TA_MANDPASSWORD;
                   UtlErrorHwnd( usErrMsg, MB_CANCEL, 0, NULL, EQF_ERROR, hwndDlg );
                   SETFOCUS( hwndDlg, ID_PASSWORD_EF );

                 } /* endif */
              } /* endif */




              //--- get Project Coordinator's name ---

              if ( !usErrMsg )
              {
                OEMQUERYTEXT( hwndDlg, ID_PROJCOORD_NAME_EF, pIda->pProp->szCoordinator);
              } /* endif */

              //--- get Project Coordinator's mail ---

              if ( !usErrMsg )
              {
                OEMQUERYTEXT( hwndDlg, ID_PROJCOORD_MAIL_EF, pIda->pProp->szCoordinatorEMail);
              } /* endif */

              //--- get Originator's name ---

              // get the Originator's name and mail via export/import

              //--- get Translator's name ---

              if ( !usErrMsg )
              {
                OEMQUERYTEXT( hwndDlg, ID_TRANSLATOR_NAME_EF, pIda->pProp->szVendor);
                OEMQUERYTEXT( hwndDlg, ID_TRANSLATOR_MAIL_EF, pIda->pProp->szVendorEMail);
              } /* endif */


              //--- update last used translators list ---

              if (pIda->pProp->szVendor[0] != EOS)
              {

                HPROP           hFLLProp;   // folder list properties handler
                PPROPFOLDERLIST pFLLProp;   // folder list properties pointer
                EQFINFO         ErrorInfo;  // error returned by property handler

                /************************************************************/
                /* Set last-used values                                     */
                /************************************************************/
                UtlMakeEQFPath( pIda->szBuffer1, NULC, SYSTEM_PATH, NULL );
                strcat( pIda->szBuffer1, BACKSLASH_STR );
                strcat( pIda->szBuffer1, DEFAULT_FOLDERLIST_NAME );
                hFLLProp = OpenProperties( pIda->szBuffer1, NULL,
                                           PROP_ACCESS_READ, &ErrorInfo);
                if ( hFLLProp )
                {

                  int             i;
                  int             iFound=-1;


                  SetPropAccess( hFLLProp, PROP_ACCESS_WRITE);

                  pFLLProp = (PPROPFOLDERLIST)MakePropPtrFromHnd( hFLLProp );


                  // is insertion necessary ????
                  for (i=0;i<5;i++)
                  {
                    if (!strcmp(pIda->pProp->szVendor,pFLLProp->szTranslatorList[i] )) iFound = i;
                  }// end for


                  if (iFound>=0)
                  {
                    for (i=iFound;i<4;i++)
                    {
                      strcpy(pFLLProp->szTranslatorList[i] , pFLLProp->szTranslatorList[i+1]);
                      strcpy(pFLLProp->szTranslatorMailList[i] , pFLLProp->szTranslatorMailList[i+1]);
                    }// end for

                  } // end iFound

                  for (i=4;i>=1;i--)
                  {
                    strcpy(pFLLProp->szTranslatorList[i] , pFLLProp->szTranslatorList[i-1]);
                    strcpy(pFLLProp->szTranslatorMailList[i] , pFLLProp->szTranslatorMailList[i-1]);
                  }// end for

                  strcpy(pFLLProp->szTranslatorList[0] , pIda->pProp->szVendor);
                  strcpy(pFLLProp->szTranslatorMailList[0] , pIda->pProp->szVendorEMail);

                  SaveProperties( hFLLProp, &ErrorInfo );
                  CloseProperties( hFLLProp, PROP_QUIT, &ErrorInfo);
                } /* endif */

              }




            }  // end if QueryCheck ("AFC Folder" checkbox is active)
            else
            {
              pIda->pProp->fTCMasterFolder = FALSE;
              pIda->pProp->fAFCFolder = FALSE;
            }

            if (!usErrMsg )
            {
              mResult = MRFROMSHORT( TRUE );  // result value of procedure
            }
            else
            {
              mResult = FALSE;
            }

        //    mResult = TRUE;

          }    // end case
          break;       //PID_PB_OK


        case ID_TRANSLATOR_MAIL_EF:
          {
            mResult = MRFROMSHORT(TRUE);
          }
          break;

        case ID_TRANSLATOR_NAME_EF:

          if (sNotification == CBN_SELCHANGE )
          {
            int iFound = -1;
            int i;
            PPROPFOLDERLIST pFLLProp;   // folder list properties pointer
            HPROP           hFLLProp;   // folder list properties handler
            EQFINFO         ErrorInfo;  // error returned by property handler

            pIda = ACCESSDLGIDA(hwndDlg, PFOLPROPIDA );

            {
              UtlMakeEQFPath( pIda->szBuffer1, NULC, SYSTEM_PATH, NULL );
              strcat( pIda->szBuffer1, BACKSLASH_STR );
              strcat( pIda->szBuffer1, DEFAULT_FOLDERLIST_NAME );
              hFLLProp = OpenProperties( pIda->szBuffer1, NULL,
                                         PROP_ACCESS_READ, &ErrorInfo);
              if ( hFLLProp )
              {

                pFLLProp = (PPROPFOLDERLIST)MakePropPtrFromHnd( hFLLProp );

                {
                  int ipos;

                  CBQUERYSELECTEDITEMTEXT (ipos ,hwndDlg,
                                           ID_TRANSLATOR_NAME_EF , pIda->pProp->szVendor);
                }


                if (pIda->pProp->szVendor[0] != EOS)
                {
                  for (i=0;i<5;i++)
                  {
                    if (!strcmp(pIda->pProp->szVendor,pFLLProp->szTranslatorList[i] )) iFound = i;
                  }// end for


                  // perhaps iFound does not exist

                  if (0 <= iFound && iFound < 5)
                  {
                    if (pFLLProp->szTranslatorMailList[iFound][0] != EOS)
                    {
                      strcpy(pIda->pProp->szVendorEMail, pFLLProp->szTranslatorMailList[iFound]);

                      OEMTOANSI( pIda->pProp->szVendorEMail);
                      SETTEXT(hwndDlg, ID_TRANSLATOR_MAIL_EF, pIda->pProp->szVendorEMail);
                      ANSITOOEM( pIda->pProp->szVendorEMail);
                      SETFOCUS( hwndDlg, ID_TRANSLATOR_MAIL_EF );
                    } // end if
                  } // end if

                } // end if vendor
                CloseProperties( hFLLProp, PROP_QUIT, &ErrorInfo);

              } // endif
            } // end notification
          } // end case
          mResult = MRFROMSHORT(TRUE);

          break;

        case DID_AFC_FOLDER_CHK:
          {
            BOOL fOk = TRUE;
            INT        nItem;


            pIda = ACCESSDLGIDA(hwndDlg, PFOLPROPIDA );

            //
            // This is not an AFC Folder
            //--------------------------
            if (!pIda->pProp->fAFCFolder)
            {


              if ( QUERYCHECK( hwndDlg, DID_AFC_FOLDER_CHK)  )  //checkbox is active
              {
                pIda->pProp->fAFCFolder       = TRUE;
                pIda->pProp->fTCMasterFolder  = TRUE;


                SETTEXT( hwndDlg, ID_TCMASTER_FOLDER_EF, "(Master Folder)" );

                ENABLECTRL( hwndDlg, ID_PASSWORD_EF, TRUE );

                ENABLECTRL( hwndDlg, ID_PROJCOORD_NAME_EF, TRUE  );
                ENABLECTRL( hwndDlg, ID_PROJCOORD_MAIL_EF, TRUE );

                ENABLECTRL( hwndDlg, ID_ORIGINATOR_NAME_EF, FALSE );
                ENABLECTRL( hwndDlg, ID_ORIGINATOR_MAIL_EF, FALSE );

                ENABLECTRL( hwndDlg, ID_TRANSLATOR_NAME_EF, TRUE );
                UPDATE_ID_TRANSLATOR_NAME_EF(pIda, hwndDlg);

                ENABLECTRL( hwndDlg, ID_TRANSLATOR_MAIL_EF, TRUE );
              } // end if
              else
              {

                pIda->pProp->fAFCFolder       = FALSE;
                pIda->pProp->fTCMasterFolder  = FALSE;

                SETTEXT( hwndDlg, ID_TCMASTER_FOLDER_EF, "" );
                ENABLECTRL( hwndDlg, ID_PASSWORD_EF, FALSE );

                ENABLECTRL( hwndDlg, ID_PROJCOORD_NAME_EF, FALSE );
                ENABLECTRL( hwndDlg, ID_PROJCOORD_MAIL_EF, FALSE );

                ENABLECTRL( hwndDlg, ID_ORIGINATOR_NAME_EF, FALSE );
                ENABLECTRL( hwndDlg, ID_ORIGINATOR_MAIL_EF, FALSE );

                ENABLECTRL( hwndDlg, ID_TRANSLATOR_NAME_EF, FALSE );
                ENABLECTRL( hwndDlg, ID_TRANSLATOR_MAIL_EF, FALSE );
              }   // end else

            }
            //
            // This is an AFC folder
            //--------------------------
            else
            {
              if ( pIda->pProp->fTCMasterFolder )  //checkbox is active
              {

                if ( QUERYCHECK( hwndDlg, DID_AFC_FOLDER_CHK)  )  //checkbox is active
                {

                  SETTEXT( hwndDlg, ID_TCMASTER_FOLDER_EF, "(Master Folder)" );

                  ENABLECTRL( hwndDlg, ID_PASSWORD_EF, TRUE );

                  ENABLECTRL( hwndDlg, ID_PROJCOORD_NAME_EF, TRUE  );
                  ENABLECTRL( hwndDlg, ID_PROJCOORD_MAIL_EF, TRUE );

                  ENABLECTRL( hwndDlg, ID_ORIGINATOR_NAME_EF, TRUE );
                  ENABLECTRL( hwndDlg, ID_ORIGINATOR_MAIL_EF, TRUE );

                  ENABLECTRL( hwndDlg, ID_TRANSLATOR_NAME_EF, TRUE );
                  UPDATE_ID_TRANSLATOR_NAME_EF(pIda, hwndDlg);

                  ENABLECTRL( hwndDlg, ID_TRANSLATOR_MAIL_EF, TRUE );
                }
                else
                {
                  pIda->pProp->fAFCFolder       = FALSE;
                  pIda->pProp->fTCMasterFolder  = FALSE;

                  SETTEXT( hwndDlg, ID_TCMASTER_FOLDER_EF, "" );
                  ENABLECTRL( hwndDlg, ID_PASSWORD_EF, FALSE );

                  ENABLECTRL( hwndDlg, ID_PROJCOORD_NAME_EF, FALSE );
                  ENABLECTRL( hwndDlg, ID_PROJCOORD_MAIL_EF, FALSE );

                  ENABLECTRL( hwndDlg, ID_ORIGINATOR_NAME_EF, FALSE );
                  ENABLECTRL( hwndDlg, ID_ORIGINATOR_MAIL_EF, FALSE );

                  ENABLECTRL( hwndDlg, ID_TRANSLATOR_NAME_EF, FALSE );
                  ENABLECTRL( hwndDlg, ID_TRANSLATOR_MAIL_EF, FALSE );


                } //end if


              } // end if
              else
              {
                SETTEXT( hwndDlg, ID_TCMASTER_FOLDER_EF, "(Controlled Folder - restricted rights)" );
                ENABLECTRL( hwndDlg, ID_PASSWORD_EF, FALSE );

                ENABLECTRL( hwndDlg, ID_PROJCOORD_NAME_EF, FALSE );
                ENABLECTRL( hwndDlg, ID_PROJCOORD_MAIL_EF, FALSE );

                ENABLECTRL( hwndDlg, ID_ORIGINATOR_NAME_EF, FALSE );
                ENABLECTRL( hwndDlg, ID_ORIGINATOR_MAIL_EF, FALSE );

                ENABLECTRL( hwndDlg, ID_TRANSLATOR_NAME_EF, FALSE );
                ENABLECTRL( hwndDlg, ID_TRANSLATOR_MAIL_EF, FALSE );
              }   // end else






            }//end if



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
                  break;

                  // AFC  settings
                case 1:
                  break;

                  // Project Informations
                case 2:
                  if ( QUERYCHECK( hwndDlg, DID_AFC_FOLDER_CHK) )
                  {
                    fOk =  pfnWp( pIda->hwndPages[nItem], WM_COMMAND,
                                  TCMASTER_ON, 0L);
                  }
                  else
                  {
                    fOk =  pfnWp( pIda->hwndPages[nItem], WM_COMMAND,
                                  TCMASTER_OFF, 0L);
                  }
                  break;

              } /* endswitch */
              nItem++;
            } /* endwhile */


          } // end case
          break;  // case DID_TCMASTER_CHK

          /*--------------------------------------------------------------------------*/

        case DM_GETDEFID:
          {
            HWND   hwndFocus;          // handle of focus window
            BOOL fOk = TRUE;
            INT        nItem;

            CHAR      AFCPassword[7];         // password compare

            pIda = ACCESSDLGIDA(hwndDlg, PFOLPROPIDA );


            if ( GetKeyState(VK_RETURN) & 0x8000 )
            {
              hwndFocus = GetFocus();
              if ( hwndFocus == GetDlgItem( hwndDlg, ID_PASSWORD_EF ) )
              {

                OEMQUERYTEXT( hwndDlg, ID_PASSWORD_EF, AFCPassword);


                // check if Password is correct
                // but do check only for AFC Folders which are no masterfolders
                if ( !pIda->pProp->fAFCFolder || pIda->pProp->fTCMasterFolder )
                {
                  // master folder or no AFC folder, nothing to do here
                }  
                else if (AFCPassword[0] != EOS && pIda->pProp->szAFCPassword[0] !=EOS &&
                    !strcmp(pIda->pProp->szAFCPassword, AFCPassword) )   // Password is correct
                {
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
                        fOk =  pfnWp( pIda->hwndPages[nItem], WM_COMMAND,
                                      AFC_PASSWORD_OK, 0L);
                        break;

                        // AFC  settings
                      case 1:
                        fOk =  pfnWp( pIda->hwndPages[nItem], WM_COMMAND,
                                      AFC_PASSWORD_OK, 0L);
                        break;

                        // Project Informations
                      case 2:
                        fOk =  pfnWp( pIda->hwndPages[nItem], WM_COMMAND,
                                      AFC_PASSWORD_OK, 0L);
                        break;

                    } /* endswitch */
                    nItem++;
                  } /* endwhile */

                } // end if Password is correct
                else
                {
                  // nothing to change

                  UtlErrorHwnd( AFC_PASSWORD_INVALID, MB_CANCEL, 0,
                                NULL, EQF_ERROR, hwndDlg );
                  SETFOCUS( hwndDlg, ID_PASSWORD_EF );

                } // end else
                mResult = TRUE;
              } // end if GetDlgItem

              mResult=TRUE;
            } // end if GetKeyState
            mResult=TRUE;
          } // end case

          break;


        case AFC_PASSWORD_OK:
          {
            pIda = ACCESSDLGIDA(hwndDlg, PFOLPROPIDA );

            ENABLECTRL( hwndDlg, ID_PASSWORD_EF, TRUE );

            ENABLECTRL( hwndDlg, ID_PROJCOORD_NAME_EF, TRUE  );
            ENABLECTRL( hwndDlg, ID_PROJCOORD_MAIL_EF, TRUE );

            ENABLECTRL( hwndDlg, ID_ORIGINATOR_NAME_EF, FALSE );  // filled via export/import
            ENABLECTRL( hwndDlg, ID_ORIGINATOR_MAIL_EF, FALSE );

            ENABLECTRL( hwndDlg, ID_TRANSLATOR_NAME_EF, TRUE );
            UPDATE_ID_TRANSLATOR_NAME_EF(pIda, hwndDlg);

            ENABLECTRL( hwndDlg, ID_TRANSLATOR_MAIL_EF, TRUE );

            mResult=TRUE;
          } // end case
          break;

        case ID_FOLNEW_DELHISTLOG_PB:
          {
            HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
            BOOL fDelete = FALSE;

            pIda = ACCESSDLGIDA(hwndDlg, PFOLPROPIDA );

            DIALOGBOX( hwndDlg, DelHistLogDlgProc, hResMod, ID_FOL_DELHLOG_DLG, pIda, fDelete );
            FolFillHistlogSize( pIda, hwndDlg  );

          }
          break;

        case ID_FOLNEW_CLEANHISTLOG_PB:
          pIda = ACCESSDLGIDA(hwndDlg, PFOLPROPIDA );
          FolMakeHistlogFileName( pIda, pIda->szBuffer1 );
          FolCleanHistlogEx( pIda->szBuffer1, FALSE, hwndDlg, TRUE );
          FolFillHistlogSize( pIda, hwndDlg  );
          break;

        case ID_FOLNEW_RESETWCOUNT_PB:
          pIda = ACCESSDLGIDA(hwndDlg, PFOLPROPIDA );

          strcpy( pIda->szBuffer1, pIda->pProp->PropHead.szPath );
          strcat( pIda->szBuffer1, BACKSLASH_STR );
          strcat( pIda->szBuffer1, pIda->pProp->PropHead.szName);

          EQFBWriteHistLog2( pIda->szBuffer1, "", HISTDATA_RESET_LOGTASK, 0, NULL, FALSE, NULLHANDLE, "" );

          FolFillHistlogSize( pIda, hwndDlg  );

          UtlErrorHwnd( RESET_WORDCOUNT_INFO, MB_OK, 0, (PSZ *)NULP, EQF_INFO, hwndDlg );

          break;


      } // end switch WM_COMMAND
      break;  // case WM_COMMAND


    case WM_HELP:
      {

        /*************************************************************/
        /* pass on a HELP_WM_HELP request                            */
        /*************************************************************/


        pIda = ACCESSDLGIDA(hwndDlg, PFOLPROPIDA );

        if ( pIda->pProp->PropHead.chType != PROP_TYPE_NEW) // existing folder
        {
          EqfDisplayContextHelp( (HWND)((LPHELPINFO) mp2)->hItemHandle,
                                 &hlpsubtblFolderProps[0] );
        }
        else   // new folder
        {
          EqfDisplayContextHelp( (HWND)((LPHELPINFO) mp2)->hItemHandle,
                                 &hlpsubtblCreateFolder[0] );
        }

        mResult = TRUE;  // message processed
        break;
      } // end case WM_HELP

    default:
//    mResult=TRUE;
      mResult = WinDefDlgProc( hwndDlg, message, mp1, mp2 );
      break;
  } // end switch
  return mResult;

}; // end function



//+----------------------------------------------------------------------------+
//|Internal function  FOLDERPROPS_PROJECTINFO_DLGPROC                          |
//+----------------------------------------------------------------------------+
//|Function name:                                                              |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//+----------------------------------------------------------------------------+
//|Parameters:                                                                 |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Returncode type:                                                            |
//+----------------------------------------------------------------------------+
//|Returncodes:                                                                |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                                                                            |
//|                                                                            |
//|                                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+
INT_PTR CALLBACK FOLDERPROPS_PROJECTINFO_DLGPROC
(
HWND hwndDlg,                                // handle of dialog window
WINMSG msg,
WPARAM mp1,
LPARAM mp2
)
{
  MRESULT  mResult = MRFROMSHORT( FALSE );  // result value of procedure

  PFOLPROPIDA   pIda;                       // ptr to instance data area



  switch ( msg )
  {
    case WM_INITDLG:
      {

        pIda = (PFOLPROPIDA)PVOIDFROMMP2( mp2 );
        ANCHORDLGIDA( hwndDlg, pIda );

        // fill shipment number combobox
        {
          int iShipment;

//          CBINSERTITEM( hwndDlg, ID_PRODUCT_SHIPMENT_EF, "" );
          for (iShipment = 1; iShipment <= 200; iShipment++ )
          {
            char szShipment[20];

            itoa( iShipment, szShipment, 10 );
            CBINSERTITEMEND( hwndDlg, ID_PRODUCT_SHIPMENT_EF, szShipment  );
          } /* endfor */
        }


        if ( pIda->pProp->PropHead.chType != PROP_TYPE_NEW)   // existing folder
        {

          //--- init fields for existing folder properties ---

          pIda->fNew = FALSE;

          if ( pIda->pProp->fAFCFolder)  // the existing folder is an AFC folder
          {

            //---- init the product's name field ------------------

            if ( pIda->pProp->fTCMasterFolder) // the existing folder is also a TC Master Folder
            {
              ENABLECTRL( hwndDlg, ID_PRODUCT_NAME_EF, TRUE  );
            }
            else
            {
              ENABLECTRL( hwndDlg, ID_PRODUCT_NAME_EF, FALSE  );
            }

            if ( pIda->pProp->szProduct[0] != EOS )
            {
              OEMTOANSI(pIda->pProp->szProduct);
              SETTEXT( hwndDlg, ID_PRODUCT_NAME_EF, pIda->pProp->szProduct );
              ANSITOOEM(pIda->pProp->szProduct);
            }

            //---- init the product's family field ------------------

            if ( pIda->pProp->fTCMasterFolder) // the existing folder is also a TC Master Folder
            {
              ENABLECTRL( hwndDlg, ID_PRODUCT_FAMILY_EF, TRUE  );
            }
            else
            {
              ENABLECTRL( hwndDlg, ID_PRODUCT_FAMILY_EF, FALSE  );
            }

            if ( pIda->pProp->szProductFamily[0] != EOS )
            {
              OEMTOANSI(pIda->pProp->szProductFamily);
              SETTEXT( hwndDlg, ID_PRODUCT_FAMILY_EF, pIda->pProp->szProductFamily );
              ANSITOOEM(pIda->pProp->szProductFamily);
            }

            //---- init the similar product's family field ------------------

            if ( pIda->pProp->fTCMasterFolder) // the existing folder is also a TC Master Folder
            {
              ENABLECTRL( hwndDlg, ID_SIMILAR_PRODUCT_EF, TRUE  );
            }
            else
            {
              ENABLECTRL( hwndDlg, ID_SIMILAR_PRODUCT_EF, FALSE  );
            }

            if ( pIda->pProp->szSimilarProduct[0] != EOS )
            {
              OEMTOANSI(pIda->pProp->szSimilarProduct);
              SETTEXT( hwndDlg, ID_SIMILAR_PRODUCT_EF, pIda->pProp->szSimilarProduct );
              ANSITOOEM(pIda->pProp->szSimilarProduct);
            }

            //---- init the product's subject area dictionary field ------------------

            if ( pIda->pProp->fTCMasterFolder) // the existing folder is also a TC Master Folder
            {
              ENABLECTRL( hwndDlg, ID_PRODUCT_DICT_EF, TRUE  );
            }
            else
            {
              ENABLECTRL( hwndDlg, ID_PRODUCT_DICT_EF, FALSE  );
            }

            if ( pIda->pProp->szSubjectDict[0] != EOS )
            {
              OEMTOANSI(pIda->pProp->szSubjectDict);
              SETTEXT( hwndDlg, ID_PRODUCT_DICT_EF, pIda->pProp->szSubjectDict );
              ANSITOOEM(pIda->pProp->szSubjectDict);
            }

            //---- init the product's subject area Memory field ------------------

            if ( pIda->pProp->fTCMasterFolder) // the existing folder is also a TC Master Folder
            {
              ENABLECTRL( hwndDlg, ID_PRODUCT_MEM_EF, TRUE  );
            }
            else
            {
              ENABLECTRL( hwndDlg, ID_PRODUCT_MEM_EF, FALSE  );
            }

            if ( pIda->pProp->szSubjectMem[0] != EOS )
            {
              OEMTOANSI(pIda->pProp->szSubjectMem);
              SETTEXT( hwndDlg, ID_PRODUCT_MEM_EF, pIda->pProp->szSubjectMem );
              ANSITOOEM(pIda->pProp->szSubjectMem);
            }

            //---- init the product's previous version field ------------------

            if ( pIda->pProp->fTCMasterFolder) // the existing folder is also a TC Master Folder
            {
              ENABLECTRL( hwndDlg, ID_PRODUCT_PREVIOUS_EF, TRUE  );
            }
            else
            {
              ENABLECTRL( hwndDlg, ID_PRODUCT_PREVIOUS_EF, FALSE  );
            }

            if ( pIda->pProp->szPrevVersion[0] != EOS )
            {
              OEMTOANSI(pIda->pProp->szPrevVersion);
              SETTEXT( hwndDlg, ID_PRODUCT_PREVIOUS_EF, pIda->pProp->szPrevVersion );
              ANSITOOEM(pIda->pProp->szPrevVersion);
            }

            //---- init the product's version field ------------------

            if ( pIda->pProp->fTCMasterFolder) // the existing folder is also a TC Master Folder
            {
              ENABLECTRL( hwndDlg, ID_PRODUCT_VERSION_EF, TRUE  );
            }
            else
            {
              ENABLECTRL( hwndDlg, ID_PRODUCT_VERSION_EF, FALSE  );
            }

            if ( pIda->pProp->szVersion[0] != EOS )
            {
              OEMTOANSI(pIda->pProp->szVersion);
              SETTEXT( hwndDlg, ID_PRODUCT_VERSION_EF, pIda->pProp->szVersion );
              ANSITOOEM(pIda->pProp->szVersion);
            }

            //---- init the product's shipment number field ------------------

            if ( pIda->pProp->fTCMasterFolder) // the existing folder is also a TC Master Folder
            {
              ENABLECTRL( hwndDlg, ID_PRODUCT_SHIPMENT_EF, TRUE  );
            }
            else
            {
              ENABLECTRL( hwndDlg, ID_PRODUCT_SHIPMENT_EF, FALSE  );
            }

            if ( pIda->pProp->szShipment[0] != EOS )
            {
              int iItemIndex = -1;
              UtlStripBlanks( pIda->pProp->szShipment );
              CBSEARCHSELECT( iItemIndex, hwndDlg, ID_PRODUCT_SHIPMENT_EF, pIda->pProp->szShipment );
              if ( iItemIndex < 0 )
              {
                // old shipment number is not in list???
                // convert old shipment number to integer and try again
                int iShipment = atoi( pIda->pProp->szShipment );
                if ( iShipment == 0 )
                {
                  PSZ pszErrParm = pIda->pProp->szShipment;
                  UtlError( ERROR_INVALID_OLD_SHIPMENT_NUMBER, MB_CANCEL, 1,
                            &pszErrParm, EQF_ERROR );
                }
                else
                {
                  char szShipment[20];
                  itoa( iShipment, szShipment, 10 );
                  CBSEARCHSELECT( iItemIndex, hwndDlg, ID_PRODUCT_SHIPMENT_EF, szShipment );
                } /* endif */
              } /* endif */
            }


          } // end if AFC Folder = TRUE
          else //Folder is an existing folder, but not an AFC Folder
          {
            ENABLECTRL( hwndDlg, ID_PRODUCT_NAME_EF, FALSE );
            ENABLECTRL( hwndDlg, ID_PRODUCT_FAMILY_EF, FALSE );
            ENABLECTRL( hwndDlg, ID_SIMILAR_PRODUCT_EF, FALSE );
            ENABLECTRL( hwndDlg, ID_PRODUCT_DICT_EF, FALSE );
            ENABLECTRL( hwndDlg, ID_PRODUCT_MEM_EF, FALSE );
            ENABLECTRL( hwndDlg, ID_PRODUCT_VERSION_EF, FALSE );
            ENABLECTRL( hwndDlg, ID_PRODUCT_PREVIOUS_EF, FALSE );
            ENABLECTRL( hwndDlg, ID_PRODUCT_SHIPMENT_EF, FALSE );
          } // end else
        } // end if Folder is not a new folder
        else // folder is a new folder
        {
          ENABLECTRL( hwndDlg, ID_PRODUCT_NAME_EF, FALSE );
          ENABLECTRL( hwndDlg, ID_PRODUCT_FAMILY_EF, FALSE );
          ENABLECTRL( hwndDlg, ID_SIMILAR_PRODUCT_EF, FALSE );
          ENABLECTRL( hwndDlg, ID_PRODUCT_DICT_EF, FALSE );
          ENABLECTRL( hwndDlg, ID_PRODUCT_MEM_EF, FALSE );
          ENABLECTRL( hwndDlg, ID_PRODUCT_VERSION_EF, FALSE );
          ENABLECTRL( hwndDlg, ID_PRODUCT_PREVIOUS_EF, FALSE );
          ENABLECTRL( hwndDlg, ID_PRODUCT_SHIPMENT_EF, FALSE );
          strcpy( pIda->pProp->szShipment, "1" );      // GQ: we always start with shipment 1
        }  // end else


        mResult = MRFROMSHORT(TRUE);   // leave the focus where we put it
      }
      break;


    case WM_COMMAND:
      switch ( WMCOMMANDID( mp1, mp2 ) )
      {
        case TCMASTER_ON:
          pIda = ACCESSDLGIDA(hwndDlg, PFOLPROPIDA );
          ENABLECTRL( hwndDlg, ID_PRODUCT_NAME_EF, TRUE );
          ENABLECTRL( hwndDlg, ID_PRODUCT_FAMILY_EF, TRUE );
          ENABLECTRL( hwndDlg, ID_SIMILAR_PRODUCT_EF, TRUE );
          ENABLECTRL( hwndDlg, ID_PRODUCT_DICT_EF, TRUE );
          ENABLECTRL( hwndDlg, ID_PRODUCT_MEM_EF, TRUE );
          ENABLECTRL( hwndDlg, ID_PRODUCT_VERSION_EF, TRUE );
          ENABLECTRL( hwndDlg, ID_PRODUCT_PREVIOUS_EF, TRUE );
          ENABLECTRL( hwndDlg, ID_PRODUCT_SHIPMENT_EF, TRUE );
          if ( pIda->pProp->szShipment[0] == EOS )
          {
            int iIndex = 0;
            strcpy( pIda->pProp->szShipment, "1" );
            CBSEARCHSELECT( iIndex, hwndDlg, ID_PRODUCT_SHIPMENT_EF, pIda->pProp->szShipment );
          } /* endif */
          mResult = TRUE;

          break;

        case TCMASTER_OFF:
          ENABLECTRL( hwndDlg, ID_PRODUCT_NAME_EF, FALSE );
          ENABLECTRL( hwndDlg, ID_PRODUCT_FAMILY_EF, FALSE );
          ENABLECTRL( hwndDlg, ID_SIMILAR_PRODUCT_EF, FALSE );
          ENABLECTRL( hwndDlg, ID_PRODUCT_DICT_EF, FALSE );
          ENABLECTRL( hwndDlg, ID_PRODUCT_MEM_EF, FALSE );
          ENABLECTRL( hwndDlg, ID_PRODUCT_VERSION_EF, FALSE );
          ENABLECTRL( hwndDlg, ID_PRODUCT_PREVIOUS_EF, FALSE );
          ENABLECTRL( hwndDlg, ID_PRODUCT_SHIPMENT_EF, FALSE );
          mResult = TRUE;
          break;

        case AFC_PASSWORD_OK:
          {
            ENABLECTRL( hwndDlg, ID_PRODUCT_NAME_EF, TRUE );

            ENABLECTRL( hwndDlg, ID_PRODUCT_FAMILY_EF, TRUE );
            ENABLECTRL( hwndDlg, ID_SIMILAR_PRODUCT_EF, TRUE );

            ENABLECTRL( hwndDlg, ID_PRODUCT_DICT_EF, TRUE );
            ENABLECTRL( hwndDlg, ID_PRODUCT_MEM_EF, TRUE );

            ENABLECTRL( hwndDlg, ID_PRODUCT_VERSION_EF, TRUE );
            ENABLECTRL( hwndDlg, ID_PRODUCT_PREVIOUS_EF, TRUE );

            ENABLECTRL( hwndDlg, ID_PRODUCT_SHIPMENT_EF, TRUE );
            mResult=TRUE;
          }
          break;

        case PID_PB_OK :
          {

            USHORT          usErrMsg = 0;           // message number to display in UtlError
            mResult = TRUE;


            pIda = ACCESSDLGIDA(hwndDlg, PFOLPROPIDA );


            //--- get Product's name ---

            if ( !usErrMsg )
            {
              OEMQUERYTEXT( hwndDlg, ID_PRODUCT_NAME_EF, pIda->pProp->szProduct);
            } /* endif */

            //--- get Product's family ---

            if ( !usErrMsg )
            {
              OEMQUERYTEXT( hwndDlg, ID_PRODUCT_FAMILY_EF, pIda->pProp->szProductFamily);
            } /* endif */

            //--- get Similar Product ---

            if ( !usErrMsg )
            {
              OEMQUERYTEXT( hwndDlg, ID_SIMILAR_PRODUCT_EF, pIda->pProp->szSimilarProduct);
            } /* endif */

            //--- get Product's area dictionary ---

            if ( !usErrMsg )
            {
              OEMQUERYTEXT( hwndDlg, ID_PRODUCT_DICT_EF, pIda->pProp->szSubjectDict);
            } /* endif */

            //--- get Product's area Memory ---

            if ( !usErrMsg )
            {
              OEMQUERYTEXT( hwndDlg, ID_PRODUCT_MEM_EF, pIda->pProp->szSubjectMem);
            } /* endif */

            //--- get Product's Previous Version ---

            if ( !usErrMsg )
            {
              OEMQUERYTEXT( hwndDlg, ID_PRODUCT_PREVIOUS_EF, pIda->pProp->szPrevVersion);
            } /* endif */

            //--- get Product's Version ---

            if ( !usErrMsg )
            {
              OEMQUERYTEXT( hwndDlg, ID_PRODUCT_VERSION_EF, pIda->pProp->szVersion);
            } /* endif */

            //--- get Product's Shipment ---

            if ( !usErrMsg )
            {
              // check if change in shipment number is valid
              char szNewShipmentNumber[MAX_DESCRIPTION];
              int iNewShipment = 0;
              int iOldShipment = atoi( pIda->pProp->szShipment );
              OEMQUERYTEXT( hwndDlg, ID_PRODUCT_SHIPMENT_EF, szNewShipmentNumber );
              UtlStripBlanks( szNewShipmentNumber );
              iNewShipment = atoi( szNewShipmentNumber );
              if ( szNewShipmentNumber[0] == EOS )
              {
                // no shipment selected, leave existing shipment number as-is
              }
              else if ( iNewShipment < iOldShipment )
              {
                // warning: shipment number decreased
                USHORT usMBCode = UtlError( WARNING_DECREASED_SHIPMENT_NUMBER,
                                            MB_OKCANCEL, 0,
                                            NULL, EQF_INFO );
                if ( usMBCode != MBID_OK )
                {
                  int iItem;
                  CBSEARCHSELECT( iItem, hwndDlg, ID_PRODUCT_SHIPMENT_EF,
                                  pIda->pProp->szShipment );
                  usErrMsg = WARNING_DECREASED_SHIPMENT_NUMBER;
                }
                else
                {
                  strcpy( pIda->pProp->szShipment, szNewShipmentNumber );
                } /* endif */
              }
              else if ( iNewShipment > (iOldShipment + 1) )
              {
                // warning: new shipment increment is more than 1
                USHORT usMBCode;
                PSZ pszErrParm[2];
                pszErrParm[0] = szNewShipmentNumber;
                pszErrParm[1] = pIda->pProp->szShipment;
                usMBCode = UtlError( WARNING_LARGEINCREMENT_SHIPMENT_NUMBER,
                                            MB_OKCANCEL, 2,
                                            pszErrParm, EQF_INFO );
                if ( usMBCode != MBID_OK )
                {
                  int iItem;
                  CBSEARCHSELECT( iItem, hwndDlg, ID_PRODUCT_SHIPMENT_EF,
                                  pIda->pProp->szShipment );
                  usErrMsg = WARNING_DECREASED_SHIPMENT_NUMBER;
                }
                else
                {
                  strcpy( pIda->pProp->szShipment, szNewShipmentNumber );
                } /* endif */
              }
              else
              {
                strcpy( pIda->pProp->szShipment, szNewShipmentNumber );
              } /* endif */
            } /* endif */



            if (!usErrMsg )
            {
              mResult = MRFROMSHORT( TRUE );  // result value of procedure
            }
            else
            {
              mResult = FALSE;
            }
          }


          break;



      } // end switch
      break;  // case WM_COMMAND

      /*--------------------------------------------------------------------------*/
    case WM_HELP:
      {

        /*************************************************************/
        /* pass on a HELP_WM_HELP request                            */
        /*************************************************************/


        pIda = ACCESSDLGIDA(hwndDlg, PFOLPROPIDA );

        if ( pIda->pProp->PropHead.chType != PROP_TYPE_NEW) // existing folder
        {
          EqfDisplayContextHelp( (HWND)((LPHELPINFO) mp2)->hItemHandle,
                                 &hlpsubtblFolderProps[0] );
        }
        else   // new folder
        {
          EqfDisplayContextHelp( (HWND)((LPHELPINFO) mp2)->hItemHandle,
                                 &hlpsubtblCreateFolder[0] );
        }

        mResult = TRUE;  // message processed
        break;
      } // end case WM_HELP


    default:
      break;

  } // end switch
  return mResult;

}; // end function










//+----------------------------------------------------------------------------+
//|Internal function  FOLDERPROPS_SHOWPROPS_DLGPROC                            |
//+----------------------------------------------------------------------------+
//|Function name:                                                              |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//+----------------------------------------------------------------------------+
//|Parameters:                                                                 |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Returncode type:                                                            |
//+----------------------------------------------------------------------------+
//|Returncodes:                                                                |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                                                                            |
//|                                                                            |
//|                                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+

MRESULT APIENTRY FOLDERPROPS_SHOWPROPS_DLGPROC
(
HWND hwndDlg,                                // handle of dialog window
WINMSG msg,
WPARAM mp1,
LPARAM mp2
)
{
  MRESULT  mResult = MRFROMSHORT( FALSE );  // result value of procedure

  PFOLPROPIDA   pIda;                       // ptr to instance data area



  switch ( msg )
  {
    case WM_INITDLG:
      {

        pIda = (PFOLPROPIDA)PVOIDFROMMP2( mp2 );
        ANCHORDLGIDA( hwndDlg, pIda );

        //     fOk = EqfFolderPropsToHtml( pIda->pProp->szOrgName, hwndDlg );
        //     htmlctrl.CreateFromStatic(ID_FOLDERPROPS_SHOW_?, hwndDlg);

        //     mResult = MRFROMSHORT(TRUE);   // leave the focus where we put it
      }
      break;


    case WM_COMMAND:
      switch ( WMCOMMANDID( mp1, mp2 ) )
      {
        case WM_EQF_SHOWHTML:
          //     htmlctrl.Navigate(mp2);
          break;

        case PID_PB_OK :
          {
            pIda = ACCESSDLGIDA(hwndDlg, PFOLPROPIDA );

          } // end case
          break;  // case DID_TCMASTER_CHK
      } // end switch
      break;  // case WM_COMMAND

    case WM_HELP:
      {

        /*************************************************************/
        /* pass on a HELP_WM_HELP request                            */
        /*************************************************************/


        pIda = ACCESSDLGIDA(hwndDlg, PFOLPROPIDA );

        if ( pIda->pProp->PropHead.chType != PROP_TYPE_NEW) // existing folder
        {
          EqfDisplayContextHelp( (HWND)((LPHELPINFO) mp2)->hItemHandle,
                                 &hlpsubtblFolderProps[0] );
        }
        else   // new folder
        {
          EqfDisplayContextHelp( (HWND)((LPHELPINFO) mp2)->hItemHandle,
                                 &hlpsubtblCreateFolder[0] );
        }

        mResult = TRUE;  // message processed
        break;
      } // end case WM_HELP



    default:
      break;
  } // end switch

  return mResult;

}

static VOID
FolSetDocListView
(
	PPROPFOLDER  pProp
)
{ 
  PSHORT psView = NULL;
  USHORT usLen = 0;

  // preset view with hard-coded values
  // P021037: Default Details view items for newly created folder
  pProp->sDetailsViewList[0] = 1;            // SID_FOL_NAME_COLTITLE
  pProp->sDetailsViewList[1] = 3;            // SID_FOL_TRANSL_COLTITLE
  pProp->sDetailsViewList[2] = 4;            // SID_FOL_ANAL_COLTITLE
  pProp->sDetailsViewList[3] = 9;            // SID_FOL_SIZE_COLTITLE
  pProp->sDetailsViewList[4] = 10;           //SID_FOL_COMPLETE_COLTITLE
  pProp->sDetailsViewList[5] = 14;           // SID_FOL_FORMAT_COLTITLE
  pProp->sDetailsViewList[6] = 15;           // SID_FOL_MEMORY_COLTITLE
  pProp->sDetailsViewList[7] = -1;

  // try to load user document default view list
  CHAR szDefaults[MAX_LONGFILESPEC];

  UtlMakeEQFPath( szDefaults, NULC, PROPERTY_PATH, NULL );
  strcat( szDefaults, "\\FOLDERVIEW.DEFAULT" );

  if ( UtlLoadFile( szDefaults, (PVOID *)&psView, &usLen, FALSE, FALSE ) )
  {
    if ( usLen <= (MAX_VIEW * sizeof(USHORT)) )   // is size valid?
    {
      int iTarget = 0;
      for( int i = 0; i < MAX_VIEW; i++ )
      {
        if ( psView[i] == -1 ) break;

        if ( (psView[i] > 0) && (psView[i] < 30) )   // is column number acceptable?
        {
          pProp->sDetailsViewList[iTarget++] = psView[i];
        } /* endif */
      } /* endfor */
      pProp->sDetailsViewList[iTarget] = -1;
    } /* endif */
    UtlAlloc( (PVOID *)&psView, 0, 0, NOMSG );
  } /* endif */

  // doc. list of new folder should start with details view
  memcpy( pProp->sLastUsedViewList, pProp->sDetailsViewList, sizeof(pProp->sLastUsedViewList) );

  return;
}

BOOL FolShowSelectionDialog
(
  HWND        hwndDlg,
  PFOLPROPIDA pIda,
  BOOL        fDictPID
)
{
  BOOL fOK = TRUE;
  PSELECTDLGIDA pSelDlgIda = NULL;
  int i = 0;

  fOK = UtlAlloc( (PVOID *)&pSelDlgIda, 0, sizeof(SELECTDLGIDA), ERROR_STORAGE );

  if ( fOK)
  {
    pSelDlgIda->fMemSel = pIda->fMemSel;
    if ( pIda->fMemSel )
    {
      pSelDlgIda->iDialogID = ID_FOLMEMSEL_DLG;
      pSelDlgIda->hwndAvailLB = pIda->hMemLBA;
      pSelDlgIda->iMaxSelected = MAX_NUM_OF_READONLY_MDB;
      strcpy( pSelDlgIda->szTitleBar, "Select Translation Memory databases to be searched" );

      if ( pIda->pProp->aLongMemTbl[0][0] != EOS )
      {
        i = 0;
        while ( (i < MAX_NUM_OF_READONLY_MDB) && (pIda->pProp->aLongMemTbl[i][0] != EOS) )
        {
          strcpy( pSelDlgIda->aSelected[i], pIda->pProp->aLongMemTbl[i] );
          i++;
        } /* endwhile */
      }
      else
      {
        PSZ pszSource = pIda->pProp->MemTbl;
        PSZ pszTarget = pIda->szBuffer1; 
        i = 0;
        while ( *pszSource )
        {
          while ( *pszSource == X15 ) pszSource++;
          while ( *pszSource && (*pszSource != X15) ) *pszTarget++ = *pszSource++;
          *pszTarget = EOS;
          strcpy( pSelDlgIda->aSelected[i++], pIda->szBuffer1 );
          while ( *pszSource == X15 ) pszSource++;
        } /*endwhile */
      } /* endif */
    }
    else
    if ( ! fDictPID )
    {
      pSelDlgIda->iDialogID = ID_FOLDICSEL_DLG;
      pSelDlgIda->hwndAvailLB = pIda->hDicLBA ;
      pSelDlgIda->iMaxSelected = MAX_NUM_OF_FOLDER_DICS;
      strcpy( pSelDlgIda->szTitleBar, "Select dictionaries to be searched" );
      pSelDlgIda->pFolPropIda = pIda;
      if ( pIda->pProp->aLongDicTbl[0][0] != EOS )
      {
        i = 0;
        while ( (i < MAX_NUM_OF_FOLDER_DICS) && (pIda->pProp->aLongDicTbl[i][0] != EOS) )
        {
          strcpy( pSelDlgIda->aSelected[i], pIda->pProp->aLongDicTbl[i] );
          i++;
        } /* endwhile */
      }
      else
      {
        PSZ pszSource = pIda->pProp->DicTbl;
        PSZ pszTarget = pIda->szBuffer1; 
        i = 0;
        while ( *pszSource )
        {
          while ( *pszSource == X15 ) pszSource++;
          while ( *pszSource && (*pszSource != X15) ) *pszTarget++ = *pszSource++;
          *pszTarget = EOS;
          strcpy( pSelDlgIda->aSelected[i++], pIda->szBuffer1 );
          while ( *pszSource == X15 ) pszSource++;
        } /*endwhile */
      } /* endif */
    } /* endif */
    else 
    {
       pSelDlgIda->iDialogID = ID_FOLDICSEL_DLG;
       pSelDlgIda->hwndAvailLB = pIda->hDictPIDLBA ;
       pSelDlgIda->fDictPIDSel = TRUE ;
       pSelDlgIda->iMaxSelected = 40;
       strcpy( pSelDlgIda->szTitleBar, "Select dictionary PID values" );
       pSelDlgIda->pFolPropIda = pIda;
       strcpy( pSelDlgIda->szBuffer, pIda->pProp->szDictPIDSelect2 ) ;
       PSZ pszValue = strtok( pSelDlgIda->szBuffer, ";," ) ;
       i = 0;
       while( pszValue) 
       {
          for( ; *pszValue && isspace(*pszValue) ; ++pszValue);
          PSZ pszTemp = pszValue + strlen(pszValue) - 1 ;
          for( ; pszTemp>pszValue && isspace(*pszTemp) ; *(pszTemp--)=0 ) ;
          if ( strlen(pszTemp) > 0 ) {
             strcpy( pSelDlgIda->aSelected[i], pszValue );
             i++;
          }
         pszValue = strtok( NULL, ";," ) ;
       } /* endwhile */
    }
  } /* endif */

  if ( fOK )
  {
    HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
    DIALOGBOX( hwndDlg, FOLMEMSEL_DLGPROC, hResMod, ID_FOLMEMSEL_DLG, pSelDlgIda, fOK );
  } /* endif */

  if ( fOK )
  {
    if ( pIda->fMemSel )
    {
      int i = 0;
      memset( pIda->pProp->MemTbl, NULC, sizeof( pIda->pProp->MemTbl) );
      memset( pIda->pProp->aLongMemTbl, NULC, sizeof(pIda->pProp->aLongMemTbl) );
      while ( i < pSelDlgIda->iSelected )
      {
        strcpy( pIda->pProp->aLongMemTbl[i], pSelDlgIda->aSelected[i] );
        i++;
      } /* endwhile */
    }
    else
    if ( ! fDictPID ) 
    {
      int i = 0;
      memset( pIda->pProp->DicTbl, NULC, sizeof( pIda->pProp->DicTbl) );
      memset( pIda->pProp->aLongDicTbl, NULC, sizeof(pIda->pProp->aLongDicTbl) );
      while ( i < pSelDlgIda->iSelected )
      {
        strcpy( pIda->pProp->aLongDicTbl[i], pSelDlgIda->aSelected[i] );
        i++;
      } /* endwhile */
    } /* endif */
    else
    {
       memset( pIda->pProp->szDictPIDSelect2, NULC, sizeof( pIda->pProp->szDictPIDSelect2) );
       for( int i=0 ; i < pSelDlgIda->iSelected ; ++i )
       {
         if ( (strlen(pIda->pProp->szDictPIDSelect2)+strlen(pSelDlgIda->aSelected[i])+1) < sizeof(pIda->pProp->szDictPIDSelect2) ) {
            if ( pIda->pProp->szDictPIDSelect2[0] ) 
               strcat( pIda->pProp->szDictPIDSelect2, ";" );
            strcat( pIda->pProp->szDictPIDSelect2, pSelDlgIda->aSelected[i] );
         }
         else 
         {
            /* Value will not fit in PID variable (400 chars) */
         }
       } 
    }
  } /* endif */

  if ( pSelDlgIda ) UtlAlloc( (PVOID *)&pSelDlgIda, 0, 0, NOMSG );

  return( fOK );
}

// dialog procedure for memory/dictionary selection
INT_PTR APIENTRY FOLMEMSEL_DLGPROC( HWND hwndDlg, WINMSG msg, WPARAM mp1, LPARAM mp2 )
{
  MRESULT  mResult = MRFROMSHORT( FALSE );       // result value of procedure

  switch ( msg )
  {
    case WM_EQF_QUERYID: 
      {
        PSELECTDLGIDA pIda = ACCESSDLGIDA( hwndDlg, PSELECTDLGIDA );
        if ( pIda )
        {
          HANDLEQUERYID( ((SHORT)pIda->iDialogID), mp2 ); 
        } /* endif */

      }
      break;

    case WM_INITDLG:
      {
        int i = 0;
        PSELECTDLGIDA pIda = (PSELECTDLGIDA)PVOIDFROMMP2( mp2 );

        ANCHORDLGIDA( hwndDlg, pIda );
        SETWINDOWID( hwndDlg, pIda->iDialogID );
        SetWindowText( hwndDlg, pIda->szTitleBar );

        // fill available listbox
        UtlCopyListBox( GetDlgItem( hwndDlg, ID_FOLMEMSEL_AVAILABLE_LB ), pIda->hwndAvailLB );

        // fill selected listbox
        DELETEALL( hwndDlg, ID_FOLMEMSEL_SELECTED_LB );
        while ( (i < pIda->iMaxSelected) && (pIda->aSelected[i][0] != EOS) )
        {
          strcpy( pIda->szBuffer, pIda->aSelected[i] );
          OEMTOANSI( pIda->szBuffer );
          INSERTITEMEND( hwndDlg, ID_FOLMEMSEL_SELECTED_LB , pIda->szBuffer );
          i++;
        } /* endwhile */

        UtlSetHorzScrollingForLB( GetDlgItem( hwndDlg, ID_FOLMEMSEL_AVAILABLE_LB ) );
        UtlSetHorzScrollingForLB( GetDlgItem( hwndDlg, ID_FOLMEMSEL_SELECTED_LB ) );
      }
      break;


    case WM_COMMAND:
      switch ( LOWORD( mp1 ) )
      {
        case ID_FOLMEMSEL_AVAILABLE_LB:
          if ( HIWORD( mp1 ) == LN_ENTER )
          {
            PSELECTDLGIDA pIda = ACCESSDLGIDA(hwndDlg, PSELECTDLGIDA );
            SHORT sItem = QUERYSELECTION( hwndDlg, ID_FOLMEMSEL_AVAILABLE_LB );

            if ( sItem != LIT_NONE )
            {
              USHORT usAlreadySelectedMsg = ( pIda->fMemSel ) ? INFO_MEM_SELECTED : INFO_DIC_SELECTED;
              if ( pIda->fDictPIDSel ) 
                 usAlreadySelectedMsg = INFO_DICTPID_SELECTED;
              int iMaxFiles = pIda->iMaxSelected;
              USHORT usMaxSelectedMsg = ( pIda->fMemSel ) ? ERROR_MAX_READONLY_TM : INFO_DIC_MAXSELECTED;
              if ( pIda->fDictPIDSel ) 
                 usMaxSelectedMsg = ERROR_DICTPID_MAXSELECTED;

              QUERYITEMTEXT( hwndDlg, ID_FOLMEMSEL_AVAILABLE_LB, sItem, pIda->szBuffer );
              sItem = SEARCHITEM( hwndDlg, ID_FOLMEMSEL_SELECTED_LB, pIda->szBuffer );
              if ( sItem != LIT_NONE)
              {
                UtlErrorHwnd( usAlreadySelectedMsg, MB_OK, 0, (PSZ *)NULP, EQF_INFO, hwndDlg );
              }
              else if ( (iMaxFiles - 1) <= QUERYITEMCOUNT( hwndDlg, ID_FOLMEMSEL_SELECTED_LB ) )
              {
                UtlErrorHwnd( usMaxSelectedMsg, MB_OK, 0, (PSZ *)NULP, EQF_WARNING, hwndDlg );
              }
              else if ( !pIda->fMemSel && !pIda->fDictPIDSel )
              {
                if ( pIda->pFolPropIda != NULL )
                {
                  USHORT usMBRC = MBID_OK;
                  PSZ pszOldLang, pszNewLang;
                  PFOLPROPIDA pFolPropIda = (PFOLPROPIDA)pIda->pFolPropIda;

                  // check source language of dictionary            
                  pszNewLang = FolGetSourceLng( (PFOLPROPIDA)pIda->pFolPropIda, pIda->szBuffer );
                  if ( QUERYITEMCOUNT( hwndDlg, ID_FOLMEMSEL_SELECTED_LB)  > 0 )
                  {
                    QUERYITEMTEXT( hwndDlg, ID_FOLMEMSEL_SELECTED_LB, 0, pIda->szBuffer2 );
                    pszOldLang = FolGetSourceLng( pFolPropIda, pIda->szBuffer2 );
                  }
                  else
                  {
                    pszOldLang = pszNewLang;
                  } /* endif */

                  if ( *pszNewLang && *pszOldLang && (stricmp( pszNewLang, pszOldLang) != 0 ) )
                  {
                    PSZ pszErrParm[4];      
                    pszErrParm[0] = pIda->szBuffer;
                    pszErrParm[1] = pszNewLang;
                    pszErrParm[2] = pszOldLang;
                    usMBRC = UtlErrorHwnd( SOURCE_LANG_DIFFERENT, MB_OKCANCEL | MB_DEFBUTTON2, 3, 
                                          pszErrParm, EQF_WARNING, hwndDlg );
                  } /* endif */

                  if ( usMBRC == MBID_OK )
                  {
                    INSERTITEMEND( hwndDlg, ID_FOLMEMSEL_SELECTED_LB, pIda->szBuffer );
                    if ( QUERYITEMCOUNT( hwndDlg, ID_FOLMEMSEL_SELECTED_LB)  == 1 ) 
                      SELECTITEM( hwndDlg, ID_FOLMEMSEL_SELECTED_LB, 0 );
                    UtlSetHorzScrollingForLB( GetDlgItem( hwndDlg, ID_FOLMEMSEL_SELECTED_LB ) );
                  } /* endif */
                } /* endif */
              }
              else if ( pIda->fDictPIDSel )
              {
                 int iLength = 0 ;
                 int iCount = QUERYITEMCOUNT( hwndDlg, ID_FOLMEMSEL_SELECTED_LB );
                 for( int i=0 ; i<iCount ; i++ ) {
                    QUERYITEMTEXT( hwndDlg, ID_FOLMEMSEL_SELECTED_LB, i, pIda->szBuffer2 );
                    iLength += strlen(pIda->szBuffer2) + 1 ;
                 }
                 if ( iLength+ strlen(pIda->szBuffer)+1 > MAX_DICTPID_VALUES ) {
                    UtlErrorHwnd( ERROR_DICTPID_MAXSELECTED, MB_OK, 0, (PSZ *)NULP, EQF_WARNING, hwndDlg );
                 } 
                 else 
                 {
                   INSERTITEMEND( hwndDlg, ID_FOLMEMSEL_SELECTED_LB, pIda->szBuffer );
                   if ( QUERYITEMCOUNT( hwndDlg, ID_FOLMEMSEL_SELECTED_LB)  == 1 ) 
                     SELECTITEM( hwndDlg, ID_FOLMEMSEL_SELECTED_LB, 0 );
                   UtlSetHorzScrollingForLB( GetDlgItem( hwndDlg, ID_FOLMEMSEL_SELECTED_LB ) );
                 } /* endif */
              }
              else 
              {
                INSERTITEMEND( hwndDlg, ID_FOLMEMSEL_SELECTED_LB, pIda->szBuffer );
                if ( QUERYITEMCOUNT( hwndDlg, ID_FOLMEMSEL_SELECTED_LB)  == 1 ) 
                  SELECTITEM( hwndDlg, ID_FOLMEMSEL_SELECTED_LB, 0 );
                UtlSetHorzScrollingForLB( GetDlgItem( hwndDlg, ID_FOLMEMSEL_SELECTED_LB ) );
              } /* endif */
            } /* endif */
          } /* endif */
          break;

        case ID_FOLMEMSEL_SELECTED_LB:
          if ( HIWORD( mp1 ) == LN_ENTER )
          {
            SHORT sItem = QUERYSELECTION( hwndDlg, ID_FOLMEMSEL_SELECTED_LB );
            DELETEITEM( hwndDlg, ID_FOLMEMSEL_SELECTED_LB, sItem );
            if ( QUERYITEMCOUNT( hwndDlg, ID_FOLMEMSEL_SELECTED_LB)  != 0 )
            {
              SELECTITEM( hwndDlg, ID_FOLMEMSEL_SELECTED_LB, 0 );
            } /* endif */
          } /* endif */
          break;

        case ID_FOLMEMSEL_OK_PB :
          {
            PSELECTDLGIDA pIda = ACCESSDLGIDA(hwndDlg, PSELECTDLGIDA );
            int i = 0;

            memset( pIda->aSelected, NULC, sizeof(pIda->aSelected) );

            pIda->iSelected = QUERYITEMCOUNT( hwndDlg, ID_FOLMEMSEL_SELECTED_LB );
            while ( i < pIda->iSelected )
            {
              QUERYITEMTEXT( hwndDlg, ID_FOLMEMSEL_SELECTED_LB, i, pIda->szBuffer );
              ANSITOOEM( pIda->szBuffer );
              strcpy( pIda->aSelected[i], pIda->szBuffer );
              i++;
            } /* endwhile */
            WinDismissDlg( hwndDlg, TRUE );
          } // end case
          break;  

        case DID_CANCEL:
        case ID_FOLMEMSEL_CANCEL_PB :
          WinDismissDlg( hwndDlg, FALSE );
          break;
      } // end switch
      break;  // case WM_COMMAND

    case DM_GETDEFID:
      // check if user pressed the ENTER key, but wants only to select/deselect an item of the listbox 
      // via a simulated (keystroke) double click.  
      if ( GetKeyState(VK_RETURN) & 0x8000 )
      {
        HWND hwndFocus = GetFocus();
        if ( hwndFocus == GetDlgItem( hwndDlg, ID_FOLMEMSEL_AVAILABLE_LB ) )
        {
          FOLMEMSEL_DLGPROC( hwndDlg, WM_COMMAND, MAKEWPARAM( ID_FOLMEMSEL_AVAILABLE_LB, LN_ENTER), 0L );
        }
        else if ( hwndFocus == GetDlgItem( hwndDlg, ID_FOLMEMSEL_SELECTED_LB ) )
        {
          FOLMEMSEL_DLGPROC( hwndDlg, WM_COMMAND, MAKEWPARAM( ID_FOLMEMSEL_SELECTED_LB, LN_ENTER), 0L );
        } /* endif */
      } /* endif */
      mResult = TRUE;
      break;

    case WM_HELP:
      EqfDisplayContextHelp( (HWND)((LPHELPINFO) mp2)->hItemHandle, &hlpsubtblFolderProps[0] );
      mResult = TRUE;  // message processed
      break;

    default:
      break;
  } // end switch

  return mResult;
} /* end of function FOLMEMSEL_DLGPROC */


void FolMakeHistlogFileName( PFOLPROPIDA pIda, PSZ pszBuffer )
{
  // setup history log name
  strcpy( pszBuffer, pIda->pProp->PropHead.szPath );
  strcat( pszBuffer, BACKSLASH_STR );
  strcat( pszBuffer, pIda->pProp->PropHead.szName );
  strcat( pszBuffer, BACKSLASH_STR );
  strcat( pszBuffer, PROPDIR );
  strcat( pszBuffer, BACKSLASH_STR );
  strcat( pszBuffer, HISTLOGFILE );
}

void FolFillHistlogSize( PFOLPROPIDA pIda, HWND hwndDlg  )
{
  HANDLE hDir;
  WIN32_FIND_DATA FindData;

  FolMakeHistlogFileName( pIda, pIda->szBuffer1 );

  // get file size
  hDir = FindFirstFile( pIda->szBuffer1, &FindData );
  if ( hDir != INVALID_HANDLE_VALUE)
  {
    INT64 size = ((INT64)FindData.nFileSizeHigh << 32) + FindData.nFileSizeLow; 

    if ( size > 1024 )
    {
      size = size / 1024;
      if ( size > 1024 )
      {
        size = size / 1024;
        if ( size > 1024 )
        {
          size = size / 1024;
          sprintf( pIda->szBuffer1, "%ld GB", (LONG)size );
        }
        else
        {
          sprintf( pIda->szBuffer1, "%ld MB", (LONG)size );
        } /* endif */
      }
      else
      {
        sprintf( pIda->szBuffer1, "%ld kB", (LONG)size );
      } /* endif */
    }
    else
    {
      sprintf( pIda->szBuffer1, "%ld B", (LONG)size );
    } /* endif */
    FindClose( hDir );
  }
  else
  {
    strcpy( pIda->szBuffer1, "0 B" );
  } /* endif */


  SETTEXT( hwndDlg, ID_FOLNEW_HISTLOGSIZE_TEXT, pIda->szBuffer1 );
}

void FolDeleteHistlog( PFOLPROPIDA pIda, HWND hwndDlg  )
{
  hwndDlg; 

  FolMakeHistlogFileName( pIda, pIda->szBuffer1 );

  UtlDelete( pIda->szBuffer1, 0, FALSE );
}


// create a random captcha
void FolCreateCaptcha( PSZ pszCaptcha, PSZ pszDispCaptcha, int iLen )
{
  int i;
  long lTime;
  int iRandMax = 10 /* digits */ + 26 /* uppercase characters */ + 26 /* lowercase characters */;

  // set sees for random number
  UtlTime( &lTime );
  srand( (int)lTime );

  // generate characters for captcha
  for( i = 0; i < iLen; i++ )
  {
    CHAR c;

    int iRandom = rand() % iRandMax;
    if ( iRandom < 10 )
    {
      c = (CHAR)('0' + iRandom);
    }
    else if ( iRandom < 36 )
    {
      c = (CHAR)('A' + (iRandom - 10));
    }
    else
    {
      c = (CHAR)('a' + (iRandom - 36));
    } /* endif */

    *pszCaptcha++ = c;
    *pszDispCaptcha++ = c;
    *pszDispCaptcha++ = ' ';
    *pszDispCaptcha++ = ' ';
  } /* endfor */
  *pszCaptcha = EOS;
  *pszDispCaptcha = EOS;
}


// dialog procedure for memory/dictionary selection
INT_PTR APIENTRY DelHistLogDlgProc( HWND hwndDlg, WINMSG msg, WPARAM mp1, LPARAM mp2 )
{
  MRESULT  mResult = MRFROMSHORT( FALSE );       // result value of procedure
  PFOLPROPIDA   pIda;                 // ptr to instance data area

  switch ( msg )
  {
    case WM_EQF_QUERYID: 
      HANDLEQUERYID( ID_FOL_DELHLOG_DLG, mp2 ); 
      break;

    case WM_INITDLG:
      {
        USHORT usPriv = 0;

        pIda = (PFOLPROPIDA) mp2;
        ANCHORDLGIDA( hwndDlg, pIda);


        pIda->szUser[0] = EOS;
        UtlGetLANUserID( pIda->szUser, &usPriv, 0 );

        if ( pIda->szUser[0] != EOS )
        {
          SETTEXT( hwndDlg, ID_FOL_DELHLOG_USERID_EF, pIda->szUser );
          ENABLECTRL( hwndDlg, ID_FOL_DELHLOG_USERID_EF, FALSE );
          SETFOCUS( hwndDlg, ID_FOL_DELHLOG_CONFIRM_EF );
        } /* endif */

        FolCreateCaptcha( pIda->szCaptcha, pIda->szDispCaptcha, 6 );

        SETTEXT( hwndDlg, ID_FOL_DELHLOG_CAPTCHA_EF, pIda->szDispCaptcha );
      }
      break;

    case WM_COMMAND:
      switch ( LOWORD( mp1 ) )
      {
        case ID_FOL_DELHLOG_DEL_PB :
          {
            pIda = ACCESSDLGIDA( hwndDlg, PFOLPROPIDA );
 
            QUERYTEXT( hwndDlg, ID_FOL_DELHLOG_USERID_EF, pIda->szUser );
            QUERYTEXT( hwndDlg, ID_FOL_DELHLOG_CONFIRM_EF, pIda->szUserCaptcha );

            UtlStripBlanks( pIda->szUser );
            UtlStripBlanks( pIda->szUserCaptcha );

            if ( pIda->szUser[0] == EOS )
            {
              UtlError( ERROR_NO_USER_ID, MB_OK, 0, NULL, EQF_ERROR );
              SETFOCUS( hwndDlg, ID_FOL_DELHLOG_USERID_EF );
            }
            else if ( pIda->szUserCaptcha[0] == EOS )
            {
              UtlError( ERROR_NO_CONFIRM, MB_OK, 0, NULL, EQF_ERROR );
              SETFOCUS( hwndDlg, ID_FOL_DELHLOG_CONFIRM_EF );
            }
            else if ( strcmp( pIda->szUserCaptcha, pIda->szCaptcha ) != 0 )
            {
              UtlError( ERROR_CONFIRM_MISMATCH, MB_OK, 0, NULL, EQF_ERROR );
              SETFOCUS( hwndDlg, ID_FOL_DELHLOG_CONFIRM_EF );
            }
            else
            {
              DELHISTLOG  DelHistlogData;     // record for history log  

              FolDeleteHistlog( pIda, hwndDlg );
              UtlError( INFO_HISTLOG_DELETED, MB_OK, 0, NULL, EQF_INFO );

              // write delete record to new history log
              UtlMakeEQFPath( pIda->szObjName, NULC, SYSTEM_PATH, NULL );
              strcat( pIda->szObjName, BACKSLASH_STR );
              strcat( pIda->szObjName, pIda->pProp->PropHead.szName );

              memset( &DelHistlogData, 0, sizeof(DelHistlogData) );
              strcpy( DelHistlogData.szUser, pIda->szUser );

              EQFBWriteHistLog2( pIda->szObjName, "", HISTLOGDELETE_LOGTASK, sizeof(DELHISTLOG),
                 (PVOID)&DelHistlogData, FALSE, NULLHANDLE, NULL );

              WinDismissDlg( hwndDlg, TRUE );
            } /* endif */
          } // end case
          break;  

        case DID_CANCEL:
        case ID_FOL_DELHLOG_CANCEL_PB :
          WinDismissDlg( hwndDlg, FALSE );
          break;
      } // end switch
      break;  // case WM_COMMAND


    case WM_HELP:
      //EqfDisplayContextHelp( ((LPHELPINFO) mp2)->hItemHandle, &hlpsubtblFolderProps[0] );
      mResult = TRUE;  // message processed
      break;

    default:
      break;
  } // end switch

  return mResult;
} /* end of function DelHistLogDlgProc */


USHORT FolFuncOpenDoc
(
  PSZ         pszFolderName,           // name of the folder
  PSZ         pszDocument,             // name of document
  ULONG       ulSegNum,                // segment number of segment to be activated
  ULONG       ulLine,                  // line to be activated (ulSegNum has to be 0)
  PSZ_W       pszSearch,               // UTF-16 search string (ulSegNum and ulLine have to be 0)
  PSZ         pszTrackID               // TVT tracking ID (pszDocument = pszSearch = NULL, ulSegNum = ulLine = 0)
)
{
  PSZ         pszParm;                 // pointer for error parameters
  char        szTrackDocName[ MAX_PATH144];
  BOOL        fOK = TRUE;              // internal O.K. flag
  USHORT      usRC = NO_ERROR;         // function return code
  POPENANDPOS pOpenAndPos = NULL;      // pointer to open document area
  BOOL        fIsNew = TRUE;

   // allocate open document area
   if ( fOK )
   {
     fOK = UtlAlloc( (PVOID *)&pOpenAndPos, 0L, (LONG)sizeof(OPENANDPOS), NOMSG );

     if ( !fOK )
     {
       usRC = ERROR_STORAGE;
       UtlErrorHwnd( usRC, MB_CANCEL, 0, NULL, EQF_ERROR, HWND_FUNCIF );
     } /* endif */
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
       fIsNew = !SubFolNameToObjectName( pszFolderName, pOpenAndPos->szDocName);

       if ( fIsNew )
       {
        fOK = FALSE;
        pszParm = pszFolderName;
        usRC = ERROR_XLATE_FOLDER_NOT_EXIST;
        UtlErrorHwnd( usRC, MB_CANCEL, 1, &pszParm, EQF_ERROR, HWND_FUNCIF );
       } /* endif */
     } /* endif */
   } /* endif */

   // check if TVT tracking ID is valid
   if ( fOK && (pszTrackID != NULL) && (*pszTrackID != EOS) )
   {
      if ( (pszDocument != NULL) && (*pszDocument != EOS) )
      {
        fOK = FALSE;
        usRC = FUNC_MANDFILES;
        UtlErrorHwnd( usRC, MB_CANCEL, 0, NULL, EQF_ERROR, HWND_FUNCIF );
      } /* endif */

      if ( fOK ) 
      {
        ULONG   ulTrackSegNum ;
        usRC = TrackIDToDocSegment( pOpenAndPos->szDocName, pszTrackID, szTrackDocName, &ulTrackSegNum ) ;
        if ( usRC != 0 ) {
           fOK = FALSE;
           usRC = FUNC_MANDFILES;
           UtlErrorHwnd( usRC, MB_CANCEL, 0, NULL, EQF_ERROR, HWND_FUNCIF );
        }
        pszDocument = szTrackDocName ;
        ulSegNum = ulTrackSegNum ;
      }
   }

   // check if document has been specified
   if ( fOK && ((pszDocument == NULL) || (*pszDocument == EOS)) )
   {
     fOK = FALSE;
     usRC = FUNC_MANDFILES;
     UtlErrorHwnd( usRC, MB_CANCEL, 0, NULL, EQF_ERROR, HWND_FUNCIF );
   } /* endif */

   // check if document exists
   if ( fOK )
   {
     // build folder object name (access to folder properties is
     // required to correct folder drive letter)
     {
       PPROPFOLDER  ppropFolder;        // pointer to folder properties
       HPROP        hpropFolder;        // folder properties handle
       ULONG        ulErrorInfo;        // error indicator from property handler

       hpropFolder = OpenProperties( pOpenAndPos->szDocName, NULL, PROP_ACCESS_READ, &ulErrorInfo);
       if( hpropFolder )
       {
         ppropFolder = (PPROPFOLDER)MakePropPtrFromHnd( hpropFolder );
         if ( (ppropFolder->chDrive != EOS) && (ppropFolder->chDrive != ' ') )
         {
           pOpenAndPos->szDocName[0] = ppropFolder->chDrive;
         } /* endif */
         CloseProperties( hpropFolder, PROP_QUIT, &ulErrorInfo );
       } /* endif */
     }

     if ( fOK )
     {
       BOOL fIsNew = FALSE;

       CHAR szDocShortName[MAX_FILESPEC];

       FolLongToShortDocName( pOpenAndPos->szDocName, pszDocument, szDocShortName, &fIsNew );

       if ( fIsNew )
       {
          fOK = FALSE;
          pszParm = pszDocument;
          usRC = ERROR_TA_SOURCEFILE;
          UtlErrorHwnd( usRC, MB_CANCEL, 1, &pszParm, EQF_ERROR, HWND_FUNCIF );
        }
        else
        {
          // append document short name to document name buffer
          strcat( pOpenAndPos->szDocName, BACKSLASH_STR );
          strcat( pOpenAndPos->szDocName, szDocShortName );
        } /* endif */
     } /* endif */
   } /* endif */

   // complete open and position data and write to open trigger file
   if ( fOK )
   {  
     CHAR szOpenDocFile[MAX_EQF_PATH];

     pOpenAndPos->ulSeg = ulSegNum;
     pOpenAndPos->ulLine = ulLine;
     if ( pszSearch != NULL )
     {
       wcscpy( pOpenAndPos->chFind, pszSearch );
     } 
     else
     {
       pOpenAndPos->chFind[0] = 0;
     } /* end */        
     UtlMakeEQFPath( szOpenDocFile, NULC, PROPERTY_PATH, NULL );
     strcat( szOpenDocFile, "\\OPENDOC.DAT" );
     UtlWriteFile( szOpenDocFile, sizeof(OPENANDPOS), pOpenAndPos, FALSE );
   } /* endif */

   // start TM if it is not running already
   if ( fOK )
   {
      HWND    hwndTMgr;                // handle of TWB window

      // Check if there is a TranslationManager window already
      hwndTMgr = FindWindow( TWBMAIN, NULL );
      if ( hwndTMgr != NULL )
      {
        SetFocus( hwndTMgr );
      }
      else
      {
        //start TM
        CHAR szEQFD[MAX_EQF_PATH];
        UtlMakeEQFPath( szEQFD, NULC, PROGRAM_PATH, NULL );
        strcat( szEQFD, "\\OPENTM2.EXE" );
        ShellExecute( GetDesktopWindow(), "open", szEQFD, NULL, NULL, SW_SHOWNORMAL );
      } /* endif */
   } /* endif */

   // cleanup
   if ( pOpenAndPos ) UtlAlloc( (PVOID *)&pOpenAndPos, 0L, 0L, NOMSG );

   return( usRC );
} /* end of FolFuncOpenDoc */


// check comma seperated list of dictionaries or TMs
USHORT FolFuncCheckObjects( PSZ pszObjList, SHORT sObjType )
{
  USHORT      usRC = 0;
  PSZ         pszName;
  PSZ         pszCurPos = pszObjList;
  CHAR        szObjectName[MAX_LONGFILESPEC];

  while ( (usRC == 0) && *pszCurPos  )
  {
    // skip any delimiters
    while ( isspace(*pszCurPos) || (*pszCurPos == ','))  pszCurPos++;   // skip any whitespace or comma

    // copy current name to name buffer
    pszName = szObjectName;

    // in case of there is space in diction name, as"MINE 1409T EN-DE", 
    // the program can't process it correctly
    // so delete space checking, and then delete the end space later as following
    while ( (*pszCurPos != ',') && (*pszCurPos != EOS) /*&& !isspace(*pszCurPos)*/ )
    {
      *pszName++ = *pszCurPos++;
    } /* endwhile */       
   
    // remove all the end space if have
    while( pszName!=szObjectName && isspace(*(pszName-1)) ) pszName--;

    *pszName = EOS;

    // check current name
    if ( szObjectName[0] != EOS )
    {
      if ( !UtlCheckIfExist( szObjectName, sObjType ) )
      {
        PSZ pszParm = szObjectName;
        usRC = ( sObjType == DICT_OBJECT ) ? ERROR_DIC_NOTFOUND : EQFS_TM_NOT_FOUND;
        UtlErrorHwnd( usRC, MB_CANCEL, 1, &pszParm, EQF_ERROR, HWND_FUNCIF );
      } /* endif */
    } /* endif */       
  } /* endwhile */      
  return( usRC );
} /* end of function FolFuncCheckObjects */

// copy comma seperated list of dictionaries or TMs 
void FolFuncCopyObjectsToProps( PSZ pszObjList, SHORT sObjType, PPROPFOLDER pProp )
{
  PSZ pszObjectList = NULL;
  int iNumOfObjects = 0;
  int iElement = 0;
  PSZ         pszName;
  PSZ         pszCurPos = pszObjList;
  CHAR        szObjectName[MAX_LONGFILESPEC];
  BOOL        fAddToList = FALSE;
  PSZ         pszStartOfList;

  if ( *pszObjList == '+' )
  {
    fAddToList = TRUE;
    pszObjList++;
    pszCurPos++; 
  } /* endif */     

  if ( sObjType == TM_OBJECT )
  {
    memset( pProp->MemTbl, 0, sizeof(pProp->MemTbl) );
    if ( !fAddToList ) memset( pProp->aLongMemTbl, 0, sizeof(pProp->aLongMemTbl) );
    pszObjectList = pProp->aLongMemTbl[0];
    iNumOfObjects = MAX_NUM_OF_FOLDER_MDB;
  }
  else
  {
    memset( pProp->DicTbl, 0, sizeof(pProp->DicTbl) );
    memset( pProp->aLongDicTbl, 0, sizeof(pProp->aLongDicTbl) );
    pszObjectList = pProp->aLongDicTbl[0];
    iNumOfObjects = MAX_NUM_OF_FOLDER_DICS;
  } /* endif */     

  pszStartOfList = pszObjectList;

  // in add-to-list mode position to end of existing entries
  if ( fAddToList )
  {
    while ( (iElement < iNumOfObjects) && (*pszObjectList != EOS) )
    {
      pszObjectList += MAX_LONGFILESPEC;
      iElement++;
    } /* endwhile */      
  } /* endif */     
  
  while ( (iElement < iNumOfObjects) && *pszCurPos  )
  {
    // skip any delimiters
    while ( isspace(*pszCurPos) || (*pszCurPos == ','))  pszCurPos++;   // skip any whitespace or comma

    // copy current name to name buffer
    // in case of there is space in obj name, as"MINE 1409T EN-DE", 
    // the program can't process it correctly
    // so delete space checking, and then delete the end space later as following
    pszName = szObjectName;
    while ( (*pszCurPos != ',') && (*pszCurPos != EOS) /*&& !isspace(*pszCurPos)*/ )
    {
      *pszName++ = *pszCurPos++;
    } /* endwhile */       

    // remove all the end space if have
    while( pszName!=szObjectName && isspace(*(pszName-1)) ) pszName--;

    *pszName = EOS;


    // check if name is already contained in the list
    {
      int iTest = 0;
      PSZ pszElement = pszStartOfList;
      while ( (iTest < iElement) && (szObjectName[0] != EOS) )
      {
        if ( strcmp( pszElement, szObjectName ) == 0 )
        {
          // object is already in list, ignore it
          szObjectName[0] = EOS;
        } /* endif */           
        pszElement += MAX_LONGFILESPEC;
        iTest++;
      } /* endwhile */         
    }
	
    // add name to object array
    if ( szObjectName[0] != EOS )
    {
      strcpy( pszObjectList, szObjectName );
      pszObjectList += MAX_LONGFILESPEC;
      iElement++;
    } /* endif */       
  } /* endwhile */      
} /* end of FolFuncCopyObjectsToProps */


USHORT FolFuncDeleteMTLog
(
  PSZ         pszFolderName            // name of the folder
)
{
  PSZ         pszParm;                 // pointer for error parameters
  BOOL        fOK = TRUE;              // internal O.K. flag
  USHORT      usRC = NO_ERROR;         // function return code
  BOOL        fIsNew = TRUE;
  CHAR        szFolObjName[MAX_EQF_PATH];

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
       fIsNew = !SubFolNameToObjectName( pszFolderName, szFolObjName );

       if ( fIsNew )
       {
        fOK = FALSE;
        pszParm = pszFolderName;
        usRC = ERROR_XLATE_FOLDER_NOT_EXIST;
        UtlErrorHwnd( usRC, MB_CANCEL, 1, &pszParm, EQF_ERROR, HWND_FUNCIF );
       } /* endif */
     } /* endif */
  } /* endif */

   // delete the MT log files of the folder
   if ( fOK  )
   {
      PPROPFOLDER  ppropFolder;        // pointer to folder properties
      HPROP        hpropFolder;        // folder properties handle
      ULONG        ulErrorInfo;        // error indicator from property handler
      CHAR         szMTLogPath[MAX_EQF_PATH];
      CHAR         chDrive;

      hpropFolder = OpenProperties( szFolObjName, NULL, PROP_ACCESS_READ, &ulErrorInfo);
      if( hpropFolder )
      {
        ppropFolder = (PPROPFOLDER)MakePropPtrFromHnd( hpropFolder );
        if ( (ppropFolder->chDrive != EOS) && (ppropFolder->chDrive != ' ') )
        {
          chDrive = ppropFolder->chDrive;
        }
        else
        {
          chDrive = szFolObjName[0];
        } /* endif */

        UtlMakeEQFPath( szMTLogPath, chDrive, MTLOG_PATH, ppropFolder->PropHead.szName );
        UtlRemoveDir( szMTLogPath, FALSE );
        UtlMkDir( szMTLogPath, 0, FALSE );

        CloseProperties( hpropFolder, PROP_QUIT, &ulErrorInfo );
      } /* endif */





   } /* endif */

   return( usRC );
} /* end of FolFuncDeleteMTLog */

#define MAX_GMOPT_ROWS 20

// structure for the data used by the global memory option tab
typedef struct _GLOBMEMTABDATA
{
  // general stuff
  PFOLPROPIDA pIda;
  int iVisibleRows;                    // number of visible rows
  int iFirstVisibleRow;                // index of first visible row

  // tables with the IDs and window handles of the generated controls
  int idSeqControl[MAX_GMOPT_ROWS];
  HWND hSeqControl[MAX_GMOPT_ROWS];
  int idProjControl[MAX_GMOPT_ROWS];
  HWND hProjControl[MAX_GMOPT_ROWS];
  int idIUCControl[MAX_GMOPT_ROWS];
  HWND hIUCControl[MAX_GMOPT_ROWS];
  int idArchControl[MAX_GMOPT_ROWS];
  HWND hArchControl[MAX_GMOPT_ROWS];
  int idDivControl[MAX_GMOPT_ROWS];
  HWND hDivControl[MAX_GMOPT_ROWS];
  int idWordsControl[MAX_GMOPT_ROWS];
  HWND hWordsControl[MAX_GMOPT_ROWS];
  int idSegsControl[MAX_GMOPT_ROWS];
  HWND hSegsControl[MAX_GMOPT_ROWS];
  int idDateControl[MAX_GMOPT_ROWS];
  HWND hDateControl[MAX_GMOPT_ROWS];
  int idSubstControl[MAX_GMOPT_ROWS];
  HWND hSubstControl[MAX_GMOPT_ROWS];
  int idHControl[MAX_GMOPT_ROWS];
  HWND hHControl[MAX_GMOPT_ROWS];
  int idHStarControl[MAX_GMOPT_ROWS];
  HWND hHStarControl[MAX_GMOPT_ROWS];
  int idExcludeControl[MAX_GMOPT_ROWS];
  HWND hExcludeControl[MAX_GMOPT_ROWS];
  int idSelFrameControl[MAX_GMOPT_ROWS];
  HWND hSelFrameControl[MAX_GMOPT_ROWS];

  // loaded option file
  PGMOPTIONFILE pCTIDOptions;

  // ID of last pressed sort column button
  int idSortButton;

  // current sort state of active column
  BOOL fUpwards;

  // pointer array with sorted options
  PGM_CTID_OPTIONS *pSortedArray;

} GLOBMEMTABDATA, *PGLOBMEMTABDATA;

// static variable to pass PGLOBMEMTABDATA to FolGMOptCompare function
static PGLOBMEMTABDATA pStaticSortData;

// helper function to compare two option entries based on currently active sort column and sort mode
int FolGMOptCompare( const void *pvOpt1, const void *pvOpt2 )
{
  int iResult = 0;
  PGM_CTID_OPTIONS pOpt1 = *((PGM_CTID_OPTIONS *)pvOpt1);
  PGM_CTID_OPTIONS pOpt2 = *((PGM_CTID_OPTIONS *)pvOpt2);

  switch ( pStaticSortData->idSortButton )
  {
    case ID_FOLDERPROPS_GM_SEQ_PB : iResult = pOpt1 - pOpt2; break;
    case ID_FOLDERPROPS_GM_PROJ_PB : iResult = wcscmp( pOpt1->szProjectID, pOpt2->szProjectID ); break;
    case ID_FOLDERPROPS_GM_IUC_PB : iResult = pOpt1->chICU - pOpt2->chICU; break;
    case ID_FOLDERPROPS_GM_ARCH_PB : iResult = pOpt1->fTmbArchived - pOpt2->fTmbArchived; break;
    case ID_FOLDERPROPS_GM_DIV_PB : iResult = wcscmp( pOpt1->szDivision, pOpt2->szDivision ); break;
    case ID_FOLDERPROPS_GM_WORDS_PB : iResult = pOpt1->lWords - pOpt2->lWords; break;
    case ID_FOLDERPROPS_GM_SEGMENTS_PB : iResult = iResult = pOpt1->lSegments - pOpt2->lSegments; break;
    case ID_FOLDERPROPS_GM_DATE_PB : iResult = wcscmp( pOpt1->szArrivalDate, pOpt2->szArrivalDate ); break;
  } /* endswitch */     

  // use pointer value if main sort keys are equal to preserve original sort order
  if ( iResult == 0 )
  {
    iResult = ( pvOpt1 > pvOpt1 ) ? 1 : -1;
  } /* endif */    

  // reverse result when sorting downwards
  if ( !pStaticSortData->fUpwards )
  {
    iResult = iResult * -1;
  } /* endif */     

  return( iResult );
} /* end of function FolGMOptCompare */

// helper function to hide all selection frames but one
void FolGMHideSelectionFrame
( 
  PGLOBMEMTABDATA pData,
  int  iRow
)
{
  int i = 0;
  for ( i = 0; i < pData->iVisibleRows; i++ )
  {
    ShowWindow( pData->hSelFrameControl[i], (i == iRow) ? SW_SHOWNORMAL : SW_HIDE );
  } /* endfor */     
  return;
}

// find control in our control tables and supply row and option for the control
BOOL FolGMOptFindControl
( 
  PGLOBMEMTABDATA pData,
  int             iControlID,
  GMMEMOPT       *pOption, 
  int            *piRow
)
{
  int iRow = 0;
  BOOL fFound = FALSE;
  *pOption = GM_HFLAG_OPT;
  while ( !fFound && (iRow < pData->iVisibleRows) )
  {
    if ( iControlID == pData->idSubstControl[iRow] )
    {
      fFound = TRUE;
      *pOption = GM_SUBSTITUTE_OPT;
    }
    else if ( iControlID == pData->idHControl[iRow] )
    {
      fFound = TRUE;
      *pOption = GM_HFLAG_OPT;
    }
    else if ( iControlID == pData->idHStarControl[iRow] )
    {
      fFound = TRUE;
      *pOption = GM_HFLAGSTAR_OPT;
    }
    else if ( iControlID == pData->idExcludeControl[iRow] )
    {
      fFound = TRUE;
      *pOption = GM_EXCLUDE_OPT ;
    } /* endif */                   
    if ( !fFound ) iRow++;
  } /* endwhile */ 
  *piRow = iRow;
  return( fFound );
}

// helper function refreshing the the controls inside the table
void FolGMRefreshTableControls
(
  HWND hwnd,
  PGLOBMEMTABDATA pData
)
{
  int iRow = 0;
  CHAR szBuffer[40];

  while ( ((iRow + pData->iFirstVisibleRow) < pData->pCTIDOptions->lNumOfEntries) && (iRow < pData->iVisibleRows) )
  {
    PGM_CTID_OPTIONS pOption;
    int iSeq;
    if ( pData->pSortedArray )
    {
      pOption = pData->pSortedArray[ iRow + pData->iFirstVisibleRow];
    }
    else
    {
      pOption = pData->pCTIDOptions->Options + (iRow + pData->iFirstVisibleRow);
    } /* endif */       
    iSeq = pOption - pData->pCTIDOptions->Options;
    sprintf( szBuffer, "%ld", iSeq + 1 );
    SETTEXTHWND( pData->hSeqControl[iRow], szBuffer );
    ShowWindow( pData->hSeqControl[iRow], SW_SHOWNORMAL );
    SETTEXTHWNDW( pData->hProjControl[iRow], pOption->szProjectID );
    ShowWindow( pData->hProjControl[iRow], SW_SHOWNORMAL );
    sprintf( szBuffer, "%C", pOption->chICU );
    SETTEXTHWND( pData->hIUCControl[iRow], szBuffer );
    ShowWindow( pData->hIUCControl[iRow], SW_SHOWNORMAL );
    SETTEXTHWND( pData->hArchControl[iRow], (pOption->fTmbArchived ? "Y" : "N" ) );
    ShowWindow( pData->hArchControl[iRow], SW_SHOWNORMAL );
    SETTEXTHWNDW( pData->hDivControl[iRow], pOption->szDivision );
    ShowWindow( pData->hDivControl[iRow], SW_SHOWNORMAL );
    sprintf( szBuffer, "%ld", pOption->lWords );
    SETTEXTHWND( pData->hWordsControl[iRow], szBuffer );
    ShowWindow( pData->hWordsControl[iRow], SW_SHOWNORMAL );
    sprintf( szBuffer, "%ld", pOption->lSegments );
    SETTEXTHWND( pData->hSegsControl[iRow], szBuffer );
    ShowWindow( pData->hSegsControl[iRow], SW_SHOWNORMAL );
    SETTEXTHWNDW( pData->hDateControl[iRow], pOption->szArrivalDate );
    ShowWindow( pData->hDateControl[iRow], SW_SHOWNORMAL );
    SendMessage( pData->hSubstControl[iRow], BM_SETCHECK, (pOption->Option == GM_SUBSTITUTE_OPT) ? BST_CHECKED : BST_UNCHECKED, 0 );
    ShowWindow( pData->hSubstControl[iRow], SW_SHOWNORMAL );
    SendMessage( pData->hHControl[iRow], BM_SETCHECK, (pOption->Option == GM_HFLAG_OPT) ? BST_CHECKED : BST_UNCHECKED, 0 );
    ShowWindow( pData->hHControl[iRow], SW_SHOWNORMAL );
    SendMessage( pData->hHStarControl[iRow], BM_SETCHECK, (pOption->Option == GM_HFLAGSTAR_OPT) ? BST_CHECKED : BST_UNCHECKED, 0 );
    ShowWindow( pData->hHStarControl[iRow], SW_SHOWNORMAL );
    SendMessage( pData->hExcludeControl[iRow], BM_SETCHECK, (pOption->Option == GM_EXCLUDE_OPT) ? BST_CHECKED : BST_UNCHECKED, 0 );
    ShowWindow( pData->hExcludeControl[iRow], SW_SHOWNORMAL );
    iRow++;
  } /* endwhile */     

  while ( iRow < pData->iVisibleRows )
  {
    ShowWindow( pData->hSeqControl[iRow], SW_HIDE );
    ShowWindow( pData->hProjControl[iRow], SW_HIDE );
    ShowWindow( pData->hIUCControl[iRow], SW_HIDE );
    ShowWindow( pData->hArchControl[iRow], SW_HIDE );
    ShowWindow( pData->hDivControl[iRow], SW_HIDE );
    ShowWindow( pData->hWordsControl[iRow], SW_HIDE );
    ShowWindow( pData->hSegsControl[iRow], SW_HIDE );
    ShowWindow( pData->hDateControl[iRow], SW_HIDE );
    ShowWindow( pData->hSubstControl[iRow], SW_HIDE );
    ShowWindow( pData->hHControl[iRow], SW_HIDE );
    ShowWindow( pData->hHStarControl[iRow], SW_HIDE );
    ShowWindow( pData->hExcludeControl[iRow], SW_HIDE );
    iRow++;
  } /* endwhile */     

  SetScrollPos( GetDlgItem( hwnd, ID_FOLDERPROPS_GM_SCROLL), SB_CTL, pData->iFirstVisibleRow + 1, TRUE );
}

// help function to set the input focus to a radio button for the option of the active row
void GlobMemSetFocus
(
  PGLOBMEMTABDATA pData,
  int iRow
)
{
  PGM_CTID_OPTIONS pOption;
  if ( pData->pSortedArray )
  {
    pOption = pData->pSortedArray[ iRow + pData->iFirstVisibleRow];
  }
  else
  {
    pOption = pData->pCTIDOptions->Options + (iRow + pData->iFirstVisibleRow);
  } /* endif */                     

  if ( pOption->Option == GM_SUBSTITUTE_OPT )
  {
    SetFocus( pData->hSubstControl[iRow] );
  }
  else if ( pOption->Option == GM_HFLAG_OPT )
  {
    SetFocus( pData->hHControl[iRow] );
  }
  else if ( pOption->Option == GM_HFLAGSTAR_OPT )
  {
    SetFocus( pData->hHStarControl[iRow] );
  }
  else if ( pOption->Option == GM_EXCLUDE_OPT )
  {
    SetFocus( pData->hExcludeControl[iRow] );
  } /* endif */                 
  return;
}

static FARPROC pfnGlobMemOrgRadioButton;      // original radiobutton window procedure

LONG FAR PASCAL GlobMemRadioButtonSubclassProc
(
  HWND hwnd,
  WINMSG msg,
  WPARAM mp1,
  LPARAM mp2
)
{
  MRESULT mResult;

  switch ( msg )
  {
    case WM_GETDLGCODE:
      mResult = CallWindowProc( (WNDPROC)pfnGlobMemOrgRadioButton, hwnd, msg, mp1, mp2 );
      if ( (mp1 == VK_RETURN) || (mp1 == VK_UP) || (mp1 == VK_DOWN) )
      {
        mResult |= DLGC_WANTALLKEYS;
      } /* endif */
      return ( mResult );

    case WM_KEYDOWN:
      switch ( mp1 )
      {
        case VK_UP:
          {
            int iRow = 0;
            GMMEMOPT option = GM_HFLAG_OPT;
            HWND hwndDlg = GetParent( hwnd );
            PFOLPROPIDA  pIda = ACCESSDLGIDA( GetParent( hwnd ), PFOLPROPIDA );                      
            PGLOBMEMTABDATA pData = (PGLOBMEMTABDATA )pIda->pvGMData; 
            BOOL fMove = FALSE;
            BOOL fFound = FolGMOptFindControl( pData, (int)GetWindowLong( hwnd, GWL_ID), &option, &iRow ); 
            if ( fFound )
            {
              if ( iRow > 0 )
              {
                // position one row up
                iRow--;
                fMove = TRUE;
              }
              else if ( pData->iFirstVisibleRow != 0 )
              {
                // scroll one line up
                pData->iFirstVisibleRow -= 1;
                FolGMRefreshTableControls( hwndDlg, pData );
                SetScrollPos( GetDlgItem( hwndDlg, ID_FOLDERPROPS_GM_SCROLL), SB_CTL, pData->iFirstVisibleRow, TRUE );
                fMove = TRUE;
              } /* endif */                 

              // set focus to radiobutton for active option
              if ( fMove )
              {
                GlobMemSetFocus( pData, iRow );
                FolGMHideSelectionFrame( pData, iRow );
              } /* endif */
            } /* endif */               
          }
          return ( 0L );
          break;

        case VK_DOWN:
          {
            int iRow = 0;
            GMMEMOPT option = GM_HFLAG_OPT;
            HWND hwndDlg = GetParent( hwnd );
            PFOLPROPIDA  pIda = ACCESSDLGIDA( GetParent( hwnd ), PFOLPROPIDA );                      
            PGLOBMEMTABDATA pData = (PGLOBMEMTABDATA )pIda->pvGMData; 
            BOOL fMove = FALSE;
            BOOL fFound = FolGMOptFindControl( pData, (int)GetWindowLong( hwnd, GWL_ID), &option, &iRow ); 
            if ( fFound )
            {
              if ( (iRow + 1) < pData->iVisibleRows )
              {
                // position one row down
                iRow++;
                fMove = TRUE;
              }
              else if ( (pData->iFirstVisibleRow + pData->iVisibleRows) < pData->pCTIDOptions->lNumOfEntries )
              {
                // scroll one line down
                pData->iFirstVisibleRow += 1;
                FolGMRefreshTableControls( hwndDlg, pData );
                SetScrollPos( GetDlgItem( hwndDlg, ID_FOLDERPROPS_GM_SCROLL), SB_CTL, pData->iFirstVisibleRow, TRUE );
                if ( (pData->iFirstVisibleRow + iRow + 1) >= pData->pCTIDOptions->lNumOfEntries ) iRow--;
                fMove = TRUE;
              } /* endif */                 

              // set focus to radiobutton for active option
              if ( fMove )
              {
                GlobMemSetFocus( pData, iRow );
                FolGMHideSelectionFrame( pData, iRow );
              } /* endif */
            } /* endif */               
          }
          return ( 0L );
          break;
      } /* endswitch */
      break;
  } /* endswitch */
  return( CallWindowProc( (WNDPROC)pfnGlobMemOrgRadioButton, hwnd, msg, mp1, mp2 ) );
} /* end of function ComboEditSubclassProc */


// helper function to create a new dialog control bases on an existing one
HWND FolGMCloneControl
(
  HWND   hwndDlg,                                // dialog window handle
  HWND   hwndBase,                               // handle of control being cloned
  HWND   hwndBefore,                             // handle of control which is before the new control in Z-order
  int    iID,                                    // ID to be used for cloned control
  int    iOffset,                                // vertical offset between base control and cloned control  
  BOOL   fSubclass                               // subclass the control
)
{
  WINDOWINFO WindowInfo;
  CHAR szClassName[40];
  HWND hwndNew = NULLHANDLE;
  WINDOWPLACEMENT Placement;
  HFONT   hFont;
  
  memset( &WindowInfo, 0, sizeof(WindowInfo) );
  GetWindowInfo( hwndBase, &WindowInfo );
  GetWindowPlacement( hwndBase, &Placement );
  GetClassName( hwndBase, szClassName, sizeof(szClassName) );
  hwndNew = CreateWindowEx( WindowInfo.dwExStyle, szClassName, "", WindowInfo.dwStyle,  
                            Placement.rcNormalPosition.left, Placement.rcNormalPosition.top + iOffset, 
                            Placement.rcNormalPosition.right - Placement.rcNormalPosition.left, 
                            Placement.rcNormalPosition.bottom - Placement.rcNormalPosition.top, 
                            hwndDlg, (HMENU)iID, NULLHANDLE, 0 );

   hFont = (HFONT)SendMessage( hwndBase, WM_GETFONT, 0, 0L );
   if ( hwndNew != NULLHANDLE )
   {
    if ( hFont != NULL ) SendMessage( hwndNew, WM_SETFONT, (WPARAM)hFont, 0L ); 
    SetWindowPos( hwndNew, hwndBefore, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE );

    if ( fSubclass )
    {
      pfnGlobMemOrgRadioButton = (FARPROC)SetWindowLong( hwndNew, GWL_WNDPROC, (LONG)GlobMemRadioButtonSubclassProc );
    } /* endif */       
   } /* endif */      

  return( hwndNew );
}

// helper function creating additional dialog controls based on the controls in the first row
// in addition the function fills the control handle and ID arrays
void FolGMCreateControls
(
  HWND hwnd,
  PGLOBMEMTABDATA pData
)
{
  int iRow = 0;
  int iOffset = 0;
  int iCurID = ID_FOLDERPROPS_GM_OFFSET_DUMMY + 1;
  BOOL fRoomLeft = TRUE;
  HWND hwndPrevControl = GetDlgItem( hwnd, ID_FOLDERPROPS_GM_EXCLUDE_RB );

  // get IDs and window handles of the predefined controls in the first column
  pData->idSeqControl[iRow] = ID_FOLDERPROPS_GM_SEQ_TEXT;
  pData->hSeqControl[iRow] = GetDlgItem( hwnd, ID_FOLDERPROPS_GM_SEQ_TEXT );
  pData->idProjControl[iRow] = ID_FOLDERPROPS_GM_PROJ_TEXT;
  pData->hProjControl[iRow] = GetDlgItem( hwnd, ID_FOLDERPROPS_GM_PROJ_TEXT );
  pData->idIUCControl[iRow] = ID_FOLDERPROPS_GM_IUC_TEXT;
  pData->hIUCControl[iRow] = GetDlgItem( hwnd, ID_FOLDERPROPS_GM_IUC_TEXT );
  pData->idArchControl[iRow] = ID_FOLDERPROPS_GM_ARCH_TEXT;
  pData->hArchControl[iRow] = GetDlgItem( hwnd, ID_FOLDERPROPS_GM_ARCH_TEXT );
  pData->hDivControl[iRow] = GetDlgItem( hwnd, ID_FOLDERPROPS_GM_DIV_TEXT );
  pData->idWordsControl[iRow] = ID_FOLDERPROPS_GM_WORDS_TEXT;
  pData->hWordsControl[iRow] = GetDlgItem( hwnd, ID_FOLDERPROPS_GM_WORDS_TEXT );
  pData->idSegsControl[iRow] = ID_FOLDERPROPS_GM_SEGMENTS_TEXT;
  pData->hSegsControl[iRow] = GetDlgItem( hwnd, ID_FOLDERPROPS_GM_SEGMENTS_TEXT );
  pData->idDateControl[iRow] = ID_FOLDERPROPS_GM_DATE_TEXT;
  pData->hDateControl[iRow] = GetDlgItem( hwnd, ID_FOLDERPROPS_GM_DATE_TEXT );

  pData->idSelFrameControl[iRow] = ID_FOLDERPROPS_GM_SEL_FRAME;
  pData->hSelFrameControl[iRow] = GetDlgItem( hwnd, ID_FOLDERPROPS_GM_SEL_FRAME );
  
  pData->idSubstControl[iRow] = ID_FOLDERPROPS_GM_SUBST_RB;
  pData->hSubstControl[iRow] = GetDlgItem( hwnd, ID_FOLDERPROPS_GM_SUBST_RB );
  pfnGlobMemOrgRadioButton = (FARPROC)SetWindowLong( pData->hSubstControl[iRow], GWL_WNDPROC, (LONG)GlobMemRadioButtonSubclassProc );

  pData->idHControl[iRow] = ID_FOLDERPROPS_GM_H_RB;
  pData->hHControl[iRow] = GetDlgItem( hwnd, ID_FOLDERPROPS_GM_H_RB );
  pfnGlobMemOrgRadioButton = (FARPROC)SetWindowLong( pData->hHControl[iRow], GWL_WNDPROC, (LONG)GlobMemRadioButtonSubclassProc );

  pData->idHStarControl[iRow] = ID_FOLDERPROPS_GM_HSTAR_RB;
  pData->hHStarControl[iRow] = GetDlgItem( hwnd, ID_FOLDERPROPS_GM_HSTAR_RB );
  pfnGlobMemOrgRadioButton = (FARPROC)SetWindowLong( pData->hHStarControl[iRow], GWL_WNDPROC, (LONG)GlobMemRadioButtonSubclassProc );

  pData->idExcludeControl[iRow] = ID_FOLDERPROPS_GM_EXCLUDE_RB;
  pData->hExcludeControl[iRow] = GetDlgItem( hwnd, ID_FOLDERPROPS_GM_EXCLUDE_RB );
  pfnGlobMemOrgRadioButton = (FARPROC)SetWindowLong( pData->hExcludeControl[iRow], GWL_WNDPROC, (LONG)GlobMemRadioButtonSubclassProc );

  // compute offset between table rows using the dummy control
  {
    WINDOWPLACEMENT placementRow1, placementDummy;
    GetWindowPlacement( pData->hSeqControl[0], &placementRow1 );
    GetWindowPlacement( GetDlgItem( hwnd, ID_FOLDERPROPS_GM_OFFSET_DUMMY), &placementDummy );
    iOffset = placementDummy.rcNormalPosition.top - placementRow1.rcNormalPosition.top;
    ShowWindow( GetDlgItem( hwnd, ID_FOLDERPROPS_GM_OFFSET_DUMMY), SW_HIDE );
  }

  // create control rows until end of table or end of table space
  while ( fRoomLeft && (iRow < pData->pCTIDOptions->lNumOfEntries) )
  {
    // create the controls of the current row
    iRow++;
    pData->idSeqControl[iRow] = iCurID;
    pData->hSeqControl[iRow] = hwndPrevControl = FolGMCloneControl( hwnd, pData->hSeqControl[iRow-1], hwndPrevControl, iCurID++, iOffset, FALSE ); 
    pData->idProjControl[iRow] = iCurID;
    pData->hProjControl[iRow] = hwndPrevControl = FolGMCloneControl( hwnd, pData->hProjControl[iRow-1], hwndPrevControl, iCurID++, iOffset, FALSE ); 
    pData->idIUCControl[iRow] = iCurID;
    pData->hIUCControl[iRow] = hwndPrevControl = FolGMCloneControl( hwnd, pData->hIUCControl[iRow-1], hwndPrevControl, iCurID++, iOffset, FALSE ); 
    pData->idArchControl[iRow] = iCurID;
    pData->hArchControl[iRow] = hwndPrevControl = FolGMCloneControl( hwnd, pData->hArchControl[iRow-1], hwndPrevControl, iCurID++, iOffset, FALSE ); 
    pData->idDivControl[iRow] = iCurID;
    pData->hDivControl[iRow] = hwndPrevControl = FolGMCloneControl( hwnd, pData->hDivControl[iRow-1], hwndPrevControl, iCurID++, iOffset, FALSE );
    pData->idWordsControl[iRow] = iCurID;
    pData->hWordsControl[iRow] = hwndPrevControl = FolGMCloneControl( hwnd, pData->hWordsControl[iRow-1], hwndPrevControl, iCurID++, iOffset, FALSE );
    pData->idSegsControl[iRow] = iCurID;
    pData->hSegsControl[iRow] = hwndPrevControl = FolGMCloneControl( hwnd, pData->hSegsControl[iRow-1], hwndPrevControl, iCurID++, iOffset, FALSE );
    pData->idDateControl[iRow] = iCurID;
    pData->hDateControl[iRow] = hwndPrevControl = FolGMCloneControl( hwnd, pData->hDateControl[iRow-1], hwndPrevControl, iCurID++, iOffset, FALSE );
    pData->idSelFrameControl[iRow] = iCurID;
    pData->hSelFrameControl[iRow] = hwndPrevControl = FolGMCloneControl( hwnd, pData->hSelFrameControl[iRow-1], hwndPrevControl, iCurID++, iOffset, FALSE );
    pData->idSubstControl[iRow] = iCurID;
    pData->hSubstControl[iRow] = hwndPrevControl = FolGMCloneControl( hwnd, pData->hSubstControl[iRow-1], hwndPrevControl, iCurID++, iOffset, TRUE );
    pData->idHControl[iRow] = iCurID;
    pData->hHControl[iRow] = hwndPrevControl = FolGMCloneControl( hwnd, pData->hHControl[iRow-1], hwndPrevControl, iCurID++, iOffset, TRUE );
    pData->idHStarControl[iRow] = iCurID;
    pData->hHStarControl[iRow] = hwndPrevControl = FolGMCloneControl( hwnd, pData->hHStarControl[iRow-1], hwndPrevControl, iCurID++, iOffset, TRUE );
    pData->idExcludeControl[iRow] = iCurID;
    pData->hExcludeControl[iRow] = hwndPrevControl = FolGMCloneControl( hwnd, pData->hExcludeControl[iRow-1], hwndPrevControl, iCurID++, iOffset, TRUE );
  
    // check if there is enough room left for another row
    {
      WINDOWPLACEMENT placementLastRow, placementFrame;
      GetWindowPlacement( pData->hSeqControl[iRow], &placementLastRow );
      GetWindowPlacement( GetDlgItem( hwnd, ID_FOLDERPROPS_GM_MEMORY_GB), &placementFrame );
      fRoomLeft  = (placementLastRow.rcNormalPosition.bottom + iOffset + 2 ) < placementFrame.rcNormalPosition.bottom;
    }
  } /* endwhile */     

  // 
  pData->iVisibleRows = iRow + 1;
}

// helper function setting all option flags to the same value
void FolGMSetAllFlags
(
  HWND hwnd,
  PGLOBMEMTABDATA pData,
  GMMEMOPT NewValue
)
{
  int i = 0;

  hwnd;

  for ( i = 0; i < pData->pCTIDOptions->lNumOfEntries; i++ )
  {
    pData->pCTIDOptions->Options[i].Option = NewValue;
  } /* endfor */     
}

// helper function getting the state for a header radio button depending on number of selected elements
int FolGMGetButtonState( int iSelected, int iTotal )
{
  if ( iSelected == 0 ) return( BST_UNCHECKED );
  if ( iSelected == iTotal ) return( BST_CHECKED );
  return( BST_UNCHECKED );   // Note: BST_INDETERMINATE  can't be used for radio buttons, so we have to use BST_UNCHECKED instead
}

// helper function setting the check state of header radio buttons
void FolGMSetHeaderButtonState
(
  HWND hwnd,
  PGLOBMEMTABDATA pData
)
{
  int iSubstitute = 0;
  int iHFlag = 0;
  int iHFlagStar = 0;
  int iExclude = 0;
  int i = 0;
  for ( i = 0; i < pData->pCTIDOptions->lNumOfEntries; i++ )
  {
    switch ( pData->pCTIDOptions->Options[i].Option )
    {
      case GM_SUBSTITUTE_OPT : iSubstitute++; break;
      case GM_HFLAG_OPT : iHFlag++; break;
      case GM_HFLAGSTAR_OPT : iHFlagStar++; break;
      case GM_EXCLUDE_OPT : iExclude++; break;
    } /* endswitch */     

  } /* endfor */     

  SendDlgItemMessage( hwnd, ID_FOLDERPROPS_GM_SUBST_HDR_RB, BM_SETCHECK, FolGMGetButtonState( iSubstitute, pData->pCTIDOptions->lNumOfEntries ), 0 );
  SendDlgItemMessage( hwnd, ID_FOLDERPROPS_GM_H_HDR_RB, BM_SETCHECK, FolGMGetButtonState( iHFlag, pData->pCTIDOptions->lNumOfEntries ), 0 );
  SendDlgItemMessage( hwnd, ID_FOLDERPROPS_GM_HSTAR_HDR_RB, BM_SETCHECK, FolGMGetButtonState( iHFlagStar, pData->pCTIDOptions->lNumOfEntries ), 0 );
  SendDlgItemMessage( hwnd, ID_FOLDERPROPS_GM_EXCLUDE_HDR_RB, BM_SETCHECK, FolGMGetButtonState( iExclude, pData->pCTIDOptions->lNumOfEntries ), 0 );
}

void FolGMCleanup( PVOID pvData )
{
  PGLOBMEMTABDATA pData = (PGLOBMEMTABDATA)pvData;

  if ( pData != NULL )
  {
    if ( pData->pSortedArray != NULL )
    {
      UtlAlloc( (PVOID *)&(pData->pSortedArray), 0L, 0L, NOMSG ); 
    } /* endif */     
    UtlAlloc( (PVOID *)&pData, 0L, 0L, NOMSG ); 
  } /* endif */     
}


INT_PTR CALLBACK FOLDERPROPS_GLOBALMEMOPT_DLGPROC
(
  HWND hwndDlg,                                // handle of dialog window
  WINMSG msg,
  WPARAM mp1,
  LPARAM mp2
)
{
  MRESULT  mResult = MRFROMSHORT( FALSE );  // result value of procedure

  PFOLPROPIDA   pIda;                       // ptr to instance data area
  PGLOBMEMTABDATA pData;                    // our internal data area


  switch ( msg )
  {
    case WM_INITDLG:
      {
        // anchor dialog IDA 
        pIda = (PFOLPROPIDA)PVOIDFROMMP2( mp2 );
        ANCHORDLGIDA( hwndDlg, pIda );

        // allocate our private data area
        pData = NULL;
        UtlAlloc( (PVOID *)&pData, 0L, (LONG)sizeof(GLOBMEMTABDATA), ERROR_STORAGE );
        pData->pIda = pIda;
        pIda->pvGMData = (PVOID)pData; 

        pData->pCTIDOptions = (PGMOPTIONFILE)(pIda->pvGMOptList);

        // prepare sort array and fill initial sorting order (based on sequence number)
        {
          LONG lLength = max( (pData->pCTIDOptions->lNumOfEntries * sizeof(PGMOPTIONFILE)), MIN_ALLOC );
          if ( UtlAlloc( (PVOID *)&(pData->pSortedArray), 0, lLength, ERROR_STORAGE ) )
          {
            LONG i = 0;
            for ( i = 0; i < pData->pCTIDOptions->lNumOfEntries; i++ )
            {
              pData->pSortedArray[i] = pData->pCTIDOptions->Options + i;
            } /* endfor */               
          } /* endif */           
          pData->idSortButton = ID_FOLDERPROPS_GM_SEQ_PB;
        }

        // create additional memory option table rows when required and setup our control array
        FolGMCreateControls( hwndDlg, pData );

        // hide all selection frames
        FolGMHideSelectionFrame( pData, -1 );

        // fill memory option controls
        SETTEXT( hwndDlg, ID_FOLDERPROPS_GM_FILENAME_TEXT, pData->pIda->pProp->szGlobalMemOptFile );
        FolGMRefreshTableControls( hwndDlg, pData );
        SetScrollRange( GetDlgItem( hwndDlg, ID_FOLDERPROPS_GM_SCROLL), SB_CTL, 0, (pData->pCTIDOptions->lNumOfEntries - pData->iVisibleRows), TRUE );

        mResult = MRFROMSHORT(TRUE);   // leave the focus where we put it
      }
      break;

    case WM_COMMAND:
      {
        SHORT sNotification = WMCOMMANDCMD( mp1, mp2 );
        SHORT sCommandID = WMCOMMANDID( mp1, mp2 );
        pIda = ACCESSDLGIDA(hwndDlg, PFOLPROPIDA );
        pData = (PGLOBMEMTABDATA )pIda->pvGMData; 

        switch ( sCommandID )
        {
          case PID_PB_OK :
            {
              USHORT          usErrMsg = 0;           // message number to display in UtlError
              mResult = TRUE;

              // save global memory filte file 
              UtlMakeEQFPath( pIda->szBuffer1, pIda->pProp->chDrive, SYSTEM_PATH, NULL );
              strcat( pIda->szBuffer1, BACKSLASH_STR );
              strcat( pIda->szBuffer1, pIda->pProp->PropHead.szName );
              strcat( pIda->szBuffer1, BACKSLASH_STR );
              strcat( pIda->szBuffer1, pIda->pProp->szGlobalMemOptFile );
              GlobMemSaveOptionFile( pIda->szBuffer1, pIda->pvGMOptList );

              if (!usErrMsg )
              {
                mResult = MRFROMSHORT( TRUE );  // result value of procedure
              }
              else
              {
                mResult = FALSE;
              }
            }
            break;

          case ID_FOLDERPROPS_GM_SUBST_HDR_RB:
            if ( pIda->fInitComplete && (sNotification == BN_CLICKED) ) 
            {
              FolGMSetAllFlags( hwndDlg, pData, GM_SUBSTITUTE_OPT );
              FolGMRefreshTableControls( hwndDlg, pData );
              FolGMSetHeaderButtonState( hwndDlg, pData );
            } /* endif */               
            break;

          case ID_FOLDERPROPS_GM_H_HDR_RB:
            if ( pIda->fInitComplete && (sNotification == BN_CLICKED) ) 
            {
              FolGMSetAllFlags( hwndDlg, pData, GM_HFLAG_OPT );
              FolGMRefreshTableControls( hwndDlg, pData );
              FolGMSetHeaderButtonState( hwndDlg, pData );
            } /* endif */               
            break;

          case ID_FOLDERPROPS_GM_HSTAR_HDR_RB:
            if ( pIda->fInitComplete && (sNotification == BN_CLICKED) ) 
            {
              FolGMSetAllFlags( hwndDlg, pData, GM_HFLAGSTAR_OPT );
              FolGMRefreshTableControls( hwndDlg, pData );
              FolGMSetHeaderButtonState( hwndDlg, pData );
            } /* endif */               
            break;

          case ID_FOLDERPROPS_GM_EXCLUDE_HDR_RB:
            if ( pIda->fInitComplete && (sNotification == BN_CLICKED) ) 
            {
              FolGMSetAllFlags( hwndDlg, pData, GM_EXCLUDE_OPT );
              FolGMRefreshTableControls( hwndDlg, pData );
              FolGMSetHeaderButtonState( hwndDlg, pData );
            } /* endif */               
            break;

          case ID_FOLDERPROPS_GM_SAVE_PB:
            UtlMakeEQFPath( pIda->szBuffer1, pIda->pProp->chDrive, SYSTEM_PATH, NULL );
            strcat( pIda->szBuffer1, BACKSLASH_STR );
            strcat( pIda->szBuffer1, pIda->pProp->PropHead.szName );
            strcat( pIda->szBuffer1, BACKSLASH_STR );
            strcat( pIda->szBuffer1, pIda->pProp->szGlobalMemOptFile );
            GlobMemSaveOptionFile( pIda->szBuffer1, pIda->pvGMOptList );
            break;

          case ID_FOLDERPROPS_GM_ANALYZE_PB:
            pIda->fStartAnalysis = TRUE;

            // save the file file
            UtlMakeEQFPath( pIda->szBuffer1, pIda->pProp->chDrive, SYSTEM_PATH, NULL );
            strcat( pIda->szBuffer1, BACKSLASH_STR );
            strcat( pIda->szBuffer1, pIda->pProp->PropHead.szName );
            strcat( pIda->szBuffer1, BACKSLASH_STR );
            strcat( pIda->szBuffer1, pIda->pProp->szGlobalMemOptFile );
            GlobMemSaveOptionFile( pIda->szBuffer1, pIda->pvGMOptList );
            PostMessage( pIda->hwndDlg, WM_COMMAND, ID_FOLDERPROPS_SET_PB, 0 );
            break;

          // table sort headers
          case ID_FOLDERPROPS_GM_SEQ_PB:
          case ID_FOLDERPROPS_GM_PROJ_PB:
          case ID_FOLDERPROPS_GM_IUC_PB:
          case ID_FOLDERPROPS_GM_ARCH_PB:
          case ID_FOLDERPROPS_GM_DIV_PB:
          case ID_FOLDERPROPS_GM_WORDS_PB:
          case ID_FOLDERPROPS_GM_SEGMENTS_PB:
          case ID_FOLDERPROPS_GM_DATE_PB:
            if ( pData->pSortedArray != NULL  )
            {
              if ( sCommandID == pData->idSortButton ) 
              {
                pData->fUpwards = !pData->fUpwards;
              }
              else
              {
                pData->fUpwards = TRUE;
              } /* endif */                 
              pData->idSortButton = sCommandID;

              // sort the data
              pStaticSortData = pData;
              qsort( pData->pSortedArray, pData->pCTIDOptions->lNumOfEntries, sizeof(PGM_CTID_OPTIONS), FolGMOptCompare );

              // refresh the data
              pData->iFirstVisibleRow = 0;
              FolGMRefreshTableControls( hwndDlg, pData );
            } /* endif */               
            break;
         
          default:
            // check for a click of one of the table radio buttons
            if ( sNotification == BN_CLICKED ) 
            {
              int iRow = 0;
              BOOL fFound = FALSE;
              GMMEMOPT newOption = GM_HFLAG_OPT;

              fFound = FolGMOptFindControl( pData, sCommandID, &newOption, &iRow ); 

              if ( fFound )
              {
                if ( pData->pSortedArray )
                {
                  pData->pSortedArray[iRow+pData->iFirstVisibleRow]->Option = newOption;
                }
                else
                {
                  pData->pCTIDOptions->Options[iRow+pData->iFirstVisibleRow].Option = newOption;
                } /* endif */       
                SendMessage( pData->hSubstControl[iRow], BM_SETCHECK, (newOption == GM_SUBSTITUTE_OPT) ? BST_CHECKED : BST_UNCHECKED, 0 );
                SendMessage( pData->hHControl[iRow], BM_SETCHECK, (newOption == GM_HFLAG_OPT) ? BST_CHECKED : BST_UNCHECKED, 0 );
                SendMessage( pData->hHStarControl[iRow], BM_SETCHECK, (newOption == GM_HFLAGSTAR_OPT) ? BST_CHECKED : BST_UNCHECKED, 0 );
                SendMessage( pData->hExcludeControl[iRow], BM_SETCHECK, (newOption == GM_EXCLUDE_OPT) ? BST_CHECKED : BST_UNCHECKED, 0 );
                FolGMSetHeaderButtonState( hwndDlg, pData );
                FolGMHideSelectionFrame( pData, iRow );
              } /* endif */                   
            }
            else if ( sNotification == BN_SETFOCUS )
            {
              int iRow = 0;
              BOOL fFound = FALSE;
              GMMEMOPT newOption = GM_HFLAG_OPT;

              fFound = FolGMOptFindControl( pData, sCommandID, &newOption, &iRow ); 

              if ( fFound )
              {
                FolGMHideSelectionFrame( pData, iRow );
              } /* endif */                 
            }
            else if ( sNotification == BN_KILLFOCUS )
            {
              int iRow = 0;
              BOOL fFound = FALSE;
              GMMEMOPT newOption = GM_HFLAG_OPT;

              fFound = FolGMOptFindControl( pData, sCommandID, &newOption, &iRow ); 

              if ( fFound )
              {
                FolGMHideSelectionFrame( pData, -1 );
              } /* endif */                 
            } /* endif */               
        } // end switch
      }
      break;  // case WM_COMMAND




      /*--------------------------------------------------------------------------*/
   case WM_VSCROLL:
      {
        SHORT sNotification = LOWORD(mp1);
        LONG lScrollPos = HIWORD(mp1);
        BOOL fScroll = FALSE;
        pIda = ACCESSDLGIDA(hwndDlg, PFOLPROPIDA );
        pData = (PGLOBMEMTABDATA )pIda->pvGMData; 

        switch ( sNotification )           // case scroll command ...
        {
          case SB_LINEUP:   fScroll = TRUE; lScrollPos = pData->iFirstVisibleRow - 1; break;
          case SB_LINEDOWN: fScroll = TRUE; lScrollPos = pData->iFirstVisibleRow + 1; break;
          case SB_PAGEUP:   fScroll = TRUE; lScrollPos = pData->iFirstVisibleRow - pData->iVisibleRows; break;
          case SB_PAGEDOWN: fScroll = TRUE; lScrollPos = pData->iFirstVisibleRow + pData->iVisibleRows; break;
          case SB_THUMBPOSITION: fScroll = TRUE; break;
        } /* endswitch */

        if ( fScroll )
        {
          if ( lScrollPos < 0 ) 
          {
            pData->iFirstVisibleRow = 0;
          }
          else if ( lScrollPos >= (pData->pCTIDOptions->lNumOfEntries - pData->iVisibleRows) )
          {
            pData->iFirstVisibleRow = pData->pCTIDOptions->lNumOfEntries - pData->iVisibleRows;
          }
          else 
          {
            pData->iFirstVisibleRow = lScrollPos;
          } /* endif */           

          FolGMRefreshTableControls( hwndDlg, pData );
          SetScrollPos( GetDlgItem( hwndDlg, ID_FOLDERPROPS_GM_SCROLL), SB_CTL, pData->iFirstVisibleRow, TRUE );
        } /* endif */           
      }
      break;

    case WM_HELP:
      {
        pIda = ACCESSDLGIDA(hwndDlg, PFOLPROPIDA );

        if ( pIda->pProp->PropHead.chType != PROP_TYPE_NEW) // existing folder
        {
          EqfDisplayContextHelp( (HWND)((LPHELPINFO) mp2)->hItemHandle,
                                 &hlpsubtblFolderProps[0] );
        }
        else   // new folder
        {
          EqfDisplayContextHelp( (HWND)((LPHELPINFO) mp2)->hItemHandle,
                                 &hlpsubtblCreateFolder[0] );
        }

        mResult = TRUE;  // message processed
        break;
      } // end case WM_HELP


    default:
      break;

  } // end switch
  return mResult;

}; // end function


//  Control the selection of the Dictionary PID values.

BOOL FolDictPIDSelection
(
  HWND        hwndDlg,
  PFOLPROPIDA pIda
)
{
  HPROP            hDicProp;     // dictionary properties handle
  PPROPDICTIONARY  pDicProp;     // ptr to dictionary properties
  EQFINFO          ErrorInfo;    // error returned by property handler
  HDCB             hDict;
  HDCB             hDictFound;
  HUCB             hUser;
  CHAR_W           szTermBuf[MAX_TERM_LEN+1];
  CHAR_W           szPID[MAX_TERM_LEN+1];
  PSZ              pszDict;
  PSZ_W            pszTermData = NULL ;
  PSZ_W            pszPIDList = NULL ;
  ULONG            ulTermNum;
  ULONG            ulDataLen;
  ULONG            ulMaxList = MAX_SEGMENT_SIZE * sizeof(CHAR_W) * 2;  // size for list of all PID values
  USHORT           usNlProductLevel ;
  USHORT           usNlProductField ;
  USHORT           usErrDict;
  USHORT           usRc;
  int              i ;
  BOOL             fOK;
  BOOL             fReturn = TRUE;


  USHORT usLevel, usField;
  ULONG j;
  int iField;
  int iStart;
  BOOL  fFieldStart ;

  fOK = UtlAlloc( (PVOID *)&pszPIDList, 0L, ulMaxList, NOMSG );
  if ( fOK ) {
    *pszPIDList = EOS;
    fOK = UtlAlloc( (PVOID *)&pszTermData, 0L, MAXI_SIZE, NOMSG );
  }

  //  Search each of the folder dictionaries for PID values
  for( i=0 ; i<MAX_NUM_OF_FOLDER_DICS && (pIda->pProp->aLongDicTbl[i][0] != EOS) && fOK ; ++i ) {
    ObjLongToShortName( pIda->pProp->aLongDicTbl[i], pIda->szBuffer2, 
                        DICT_OBJECT, NULL );
    UtlMakeEQFPath( pIda->szBuffer1, NULC, SYSTEM_PATH, NULL );
    strcat( pIda->szBuffer1, BACKSLASH_STR );
    strcat( pIda->szBuffer1, pIda->szBuffer2 );
    strcat( pIda->szBuffer1, EXT_OF_DICTPROP);
    hDicProp = OpenProperties( pIda->szBuffer1, NULL, PROP_ACCESS_READ, &ErrorInfo);
    if ( hDicProp )
    {

      //  Determine if dictionary contains "NL Product" field
      pDicProp = (PPROPDICTIONARY) MakePropPtrFromHnd( hDicProp );
      usNlProductLevel = 0 ;
      usNlProductField = 0 ;
      for (j=0,iField=0 ; j<=MAX_PROF_ENTRIES; j++)
      {
         iField++;
         if ( pDicProp->ProfEntry[j].usLevel > usNlProductLevel ) {
            usNlProductLevel = pDicProp->ProfEntry[j].usLevel ;
            iField = 0 ;
         }
         if ( ! stricmp( pDicProp->ProfEntry[j].chUserName, "NL PRODUCT" ) ) {
            usNlProductField = (USHORT)iField ;
            break;
         }
      }
      CloseProperties( hDicProp, PROP_QUIT, &ErrorInfo );

      //  If dictionary contains "NL Product" field, look for values
      if ( usNlProductField > 0 ) {

         usRc = AsdBegin( 1,  &hUser );
         if ( usRc == LX_RC_OK_ASD )
         {
            ObjLongToShortName( pIda->pProp->aLongDicTbl[i], pIda->szBuffer2, 
                                DICT_OBJECT, NULL );
            UtlMakeEQFPath( pIda->szBuffer1, NULC, PROPERTY_PATH, NULL );
            strcat( pIda->szBuffer1, BACKSLASH_STR );
            strcat( pIda->szBuffer1, pIda->szBuffer2 );
            strcat( pIda->szBuffer1, EXT_OF_DICTPROP);
            pszDict = pIda->szBuffer1;
            usRc = AsdOpen( hUser,            // user ctl block handle
                            ASD_READONLY,     // open flags
                            1,                // nr of dict in ppszDict
                            &pszDict,         // dictionary properties
                            &hDict,           // dict ctl block handle
                            &usErrDict );     // number of failing dict

            if ( usRc == LX_RC_OK_ASD )
            {
               ulTermNum = 0 ;
               while ( ( usRc != LX_EOF_ASD ) )  // Search all entries
               {

                  //get next entry in asd dict
                  usRc = AsdNxtTermW( hDict,              //in, dict handle
                                      hUser,              //in, user handle
                                      szTermBuf,          //out, matching term(PSZ_W)
                                      &ulTermNum,         //out, term number
                                      &ulDataLen,         //out, length of above data in # of w's
                                      &hDictFound );      //out, which dict had a match

                  if ( usRc == LX_EOF_ASD )  //end of dictionary
                  {
                      break;
                  }
                  else 
                  if ( usRc == LX_RC_OK_ASD )
                  {
                      //get entry
                      usRc = AsdRetEntryW( hDict,
                                           hUser,
                                           szTermBuf,
                                           &ulTermNum,
                                           pszTermData,
                                           &ulDataLen,
                                           &hDictFound );

                      if ( usRc == LX_RC_OK_ASD )
                      {
                        ulTermNum++;
                        usLevel = 0;
                        usField = 0;

                        j=QLDB_START_CTRL_INFO ; 
                        while( j<ulDataLen && (pszTermData[j] != QLDB_END_OF_REC) ) 
                        {
                          fFieldStart = TRUE;
                          switch ( pszTermData[j++] )
                          {
                            case QLDB_FIRST_LEVEL :
                              usLevel = 1;
                              usField = 0;
                              break;
                            case QLDB_SECOND_LEVEL :
                              usLevel = 2;
                              usField = 0;
                              break;
                            case QLDB_THIRD_LEVEL :
                              usLevel = 3;
                              usField = 0;
                              break;
                            case QLDB_FOURTH_LEVEL :
                              usLevel = 4;
                              usField = 0;
                              break;
                            case QLDB_ESC_CHAR:
                              j++;
                              fFieldStart = FALSE;
                              break;
                            case QLDB_FIELD_DELIMITER :
                              usField++;
                              switch ( pszTermData[j] )
                              {
                                case QLDB_FIRST_LEVEL :
                                case QLDB_SECOND_LEVEL :
                                case QLDB_THIRD_LEVEL :
                                case QLDB_FOURTH_LEVEL :
                                case QLDB_END_OF_REC :
                                  fFieldStart = FALSE;
                                  break;
                                default :
                                  break;
                              } /* endswitch */
                              break;
                            default :
                              fFieldStart = FALSE;
                              break;
                          } /* endswitch */

                          if ( ( fFieldStart ) &&
                               ( pszTermData[j] != EOS ) ) {
                             iStart = j ;
                             for( ++j ; pszTermData[j]!=EOS ; ++j );
                             if ( ( usField == usNlProductField ) &&
                                  ( usLevel == usNlProductLevel ) ) {
                                for( PSZ_W ptrPID=wcstok( &pszTermData[iStart], L";," ) ;
                                     ptrPID ;
                                     ptrPID=wcstok( NULL, L";," ) ) {
                                   for( ; *ptrPID && iswspace(*ptrPID) ; ++ptrPID);
                                   PSZ_W ptrTemp = ptrPID + wcslen(ptrPID) - 1 ;
                                   for( ; ptrTemp>ptrPID && iswspace(*ptrTemp) ; *(ptrTemp--)=EOS ) ;
                                   if ( wcslen(ptrPID) > 0 ) {
                                      swprintf( szPID, L";%s;", ptrPID ) ;
                                      if ( ! wcsstr( pszPIDList, szPID )  ) {
                                         if ( *pszPIDList ) {
                                            if ( wcslen(pszPIDList)+wcslen(&szPID[1]) < ulMaxList/sizeof(WCHAR) ) {
                                               wcscat( pszPIDList, &szPID[1] ) ;
                                            }
                                         } else {
                                            wcscpy( pszPIDList, szPID ) ;
                                         }
                                      }
                                   }
                                }
             //                 break;    // Handle all trans terms for the head term.
                             }
                          }
                        }
                      } /* endif */
                  }
               }   //  end dictionary entry loop


               AsdClose( hUser, hDict );   // close this dictionary
            }
            AsdEnd( hUser );
         }
      }   // end dictionary contains "NL Product" field
    }   
  }   // end dictionary loop

  if ( *pszPIDList ) {
     pIda->fMemSel = FALSE ;
     UtlDirectUnicode2Ansi( pszPIDList, (CHAR*)pszTermData, 0L );

     WinSendMsg( pIda->hDictPIDLBA, LM_DELETEALL, NULL, NULL);
     PSZ pszValue = strtok( (CHAR*)pszTermData, ";" ) ;
     while( pszValue) 
     {
       INSERTITEMHWND( pIda->hDictPIDLBA, pszValue );
       pszValue = strtok( NULL, ";" ) ;
     }

     if ( FolShowSelectionDialog( hwndDlg, pIda, TRUE ) )
     {
        strcpy( pIda->szBuffer1, pIda->pProp->szDictPIDSelect2 ) ;
        OEMTOANSI( pIda->szBuffer1 ) ;
        SETTEXT( hwndDlg, ID_FOLNEW_DICTPID_EF, pIda->szBuffer1 );
        if ( pIda->pProp->szDictPIDSelect2[0] ) {
           pIda->pProp->fDictPIDSelect = TRUE ;
        } else {
           pIda->pProp->fDictPIDSelect = FALSE ;
        }
        SETCHECK( hwndDlg, ID_FOLNEW_DICTPID_CHK, pIda->pProp->fDictPIDSelect );

     } /* endif */
  } else {
     /* no dictionary info is available */
     pIda->pProp->fDictPIDSelect = FALSE ;
     SETCHECK( hwndDlg, ID_FOLNEW_DICTPID_CHK, pIda->pProp->fDictPIDSelect );
     ENABLECTRL( hwndDlg, ID_FOLNEW_DICTPID_EF, pIda->pProp->fDictPIDSelect );
     ENABLECTRL( hwndDlg, ID_FOLNEW_DICTPID_PB, pIda->pProp->fDictPIDSelect );
     UtlErrorHwnd( ERROR_DICT_PID_NL_PRODUCT,
                   MB_CANCEL,
                   0, (PSZ *)NULP,
                   EQF_ERROR, hwndDlg );
  }


  if ( pszTermData )
     UtlAlloc( (PVOID *)&pszTermData, 0L, 0L, ERROR_STORAGE );
  if ( pszPIDList )
     UtlAlloc( (PVOID *)&pszPIDList, 0L, 0L, ERROR_STORAGE );

  return( fReturn );
} // end FolDictPIDSelection


//  Convert TVT track ID to document name and segment number

USHORT  TrackIDToDocSegment
(
  PSZ         pszFolder,
  PSZ         pszTrackID,
  PSZ         pszDocName,
  ULONG       *pulSegNum
)
{
  CHAR_W      swTrackID[256] ;
  CHAR_W      *ptrID_W ;
  CHAR        *ptrID ;
  CHAR        *ptrSplit ;
  char        szSearchName[ MAX_PATH144];
  char        szFolderPath[ MAX_PATH144];
  int         i ;
  int         iBinary = 0 ;
  int         iHex = 0 ;
  ULONG       ulDocNum = 0 ;
  ULONG       ulSegNum = 0 ;
  USHORT      usRC = 0 ;

  CHAR_W      c_ZERO = L'\x200c' ;
  CHAR_W      c_ONE  = L'\x200d' ;
  CHAR_W      c_SEP  = L'\x2060' ;


  // Track ID is in UTF-16
  if ( ( (CHAR_W)*pszTrackID == c_ZERO ) ||
       ( (CHAR_W)*pszTrackID == c_ONE  ) ||
       ( (CHAR_W)*pszTrackID == c_SEP  ) ) {
     if ( (CHAR_W)*pszTrackID == c_SEP ) 
        wcscpy( swTrackID, (CHAR_W*)(pszTrackID+1) ) ;
     else
        wcscpy( swTrackID, (CHAR_W*)pszTrackID ) ;
  } else

  // TrackID is in UTF-8.  Convert to UTF-16.
  if ( *pszTrackID == '\xE2' ) {
     UTF82UnicodeBuf( pszTrackID, swTrackID, sizeof(swTrackID)/sizeof(CHAR_W) ) ;
  } else 
  {
     // TrackID is either binary or hexadeciml number.
     for( ptrID=pszTrackID, i=iBinary=iHex=0,ptrSplit=0 ; *ptrID ; ptrID++ ) {
        if ( ( *ptrID == '0' ) ||
             ( *ptrID == '1' ) ) 
           ++iBinary ;
        else
        if ( ( ( *ptrID >= '2' ) &&
               ( *ptrID <= '9' ) ) ||
             ( ( *ptrID >= 'a' ) &&
               ( *ptrID <= 'f' ) ) ||
             ( ( *ptrID >= 'A' ) &&
               ( *ptrID <= 'F' ) ) ) 
           ++iHex ;
        else
           ptrSplit=ptrID ;
     }
     // Convert track ID from hexadecimal to document and segment numbers.
     if ( iHex > 0 ) {
        if ( ptrSplit ) {
           *ptrSplit = 0 ;
           ulDocNum = strtoul( pszTrackID, NULL, 16 ) ;
           ulSegNum = strtoul( ptrSplit+1, NULL, 16 ) ;
           if ( (ulDocNum==0) || (ulSegNum==0 ) )
              usRC = 7 ;
        } else {
           usRC = 7 ;
        }
     } else {
       // Convert track ID from binary to UTF-16.
       for( i=0, ptrID = pszTrackID ; *ptrID && (usRC==0) ; ptrID++ ) 
       {
          if ( *ptrID == '0' ) 
             swTrackID[i] = c_ZERO ;
          else
          if ( *ptrID == '1' ) 
             swTrackID[i] = c_ONE ;
          else
             swTrackID[i] = c_SEP ;
          ++i ;   
       }
       swTrackID[i] = NULL ;
     }
  }

  if ( iHex == 0 ) {

     // Convert document number from ID
     for( ptrID_W=swTrackID ; 
          *ptrID_W && (usRC==0) && (*ptrID_W!=L'\x2060') ;
          ptrID_W++ ) {
        ulDocNum = ulDocNum * 2 ;
        if ( *ptrID_W == c_ONE ) 
           ulDocNum++ ;
        else
        if ( *ptrID_W != c_ZERO ) 
           usRC = 1 ;
     }

     // Required separator
     if ( (usRC==0) && (*ptrID_W!=c_SEP) ) 
        usRC = 2 ;

     // Convert segment number from ID
     for( ptrID_W++ ; *ptrID_W && (usRC==0) ; ptrID_W++ ) {
        ulSegNum = ulSegNum * 2 ;
        if ( *ptrID_W == c_ONE ) 
           ulSegNum++ ;
        else
        if ( *ptrID_W != c_ZERO ) 
           usRC = 3 ;
     }

     // Invalid characters in track ID
     if ( ( usRC == 0 ) && (*ptrID_W != NULL) )
        usRC = 4 ;
  }

  // Get folder properties
  if ( usRC == 0 ) {
    PPROPFOLDER  ppropFolder;        // pointer to folder properties
    HPROP        hpropFolder;        // folder properties handle
    ULONG        ulErrorInfo;        // error indicator from property handler

    usRC = 5 ;
    hpropFolder = OpenProperties( pszFolder, NULL, PROP_ACCESS_READ, &ulErrorInfo);
    if ( hpropFolder ) {
      ppropFolder = (PPROPFOLDER)MakePropPtrFromHnd( hpropFolder );
      if ( (ppropFolder->chDrive != EOS) && (ppropFolder->chDrive != ' ') ) {
        usRC = 0 ;
        UtlMakeEQFPath( szSearchName,
                        ppropFolder->chDrive,
                        PROPERTY_PATH,
                        ppropFolder->PropHead.szName );
        strcat( szSearchName, "\\" );
        strcat( szSearchName, DEFAULT_PATTERN );
        UtlMakeEQFPath( szFolderPath,
                        ppropFolder->chDrive,
                        SYSTEM_PATH,
                        ppropFolder->PropHead.szName );
      } /* endif */
      CloseProperties( hpropFolder, PROP_QUIT, &ulErrorInfo );
    } /* endif */
  }

  // Find document which matches track ID's document number
  if ( usRC == 0 ) {
    HPROP      hDocProp;                // document properties handle
    PPROPDOCUMENT pDocProp;             // document properties pointer
    EQFINFO    ErrorInfo;               // error returned by property handler
    FILEFINDBUF stResultBuf;            // DOS file find struct
    USHORT     usCount = 1;             // number of files requested
    HDIR       hDirHandle = HDIR_CREATE;// DosFind routine handle
    PSZ        pszName = RESBUFNAME(stResultBuf); // ptr to name in stResultBuf
   
    usRC = 6 ;
    UtlFindFirst( szSearchName, &hDirHandle, FILE_NORMAL,
                  &stResultBuf, sizeof( stResultBuf), &usCount, 0L, 0);
    while( usCount && usRC==6 ) {
      //--- open document properties ---
      hDocProp = OpenProperties( pszName, szFolderPath, PROP_ACCESS_READ, &ErrorInfo);
      if ( hDocProp ) {
        pDocProp = (PPROPDOCUMENT)MakePropPtrFromHnd( hDocProp );
        if ( pDocProp->usTrackDocNum == ulDocNum ) {
          usRC = 0 ;
          strcpy( pszDocName, pDocProp->szLongName ) ;
        }
        CloseProperties( hDocProp, PROP_QUIT, &ErrorInfo );
      } /* endif */
   
      UtlFindNext( hDirHandle, &stResultBuf, sizeof( stResultBuf), &usCount, 0);
    } /* endwhile */
   
    // close search file handle
    if ( hDirHandle != HDIR_CREATE ) UtlFindClose( hDirHandle, FALSE );
  }

  if ( usRC == 0 ) {
     *pulSegNum = ulSegNum ;
  } else {
     *pszDocName = EOS ;
     *pulSegNum = 0 ;
  }

  return( usRC );
} // end TrackIDToSegment


//  Check Dictionary PID values

void  CheckDictPIDValues
(
  PSZ         pszValues
)
{
  PSZ         ptrChar ;
  USHORT      usState = 1 ;    /* Remove leading blanks */

  for( ptrChar=pszValues ; ; ++ptrChar ) 
  {
     if ( usState == 1 ) {        /* Remove leading blanks */
        for( ; *ptrChar && isspace(*ptrChar) ; ) {
           memmove( ptrChar, ptrChar+1, strlen(ptrChar+1)+1 ) ;
        }
        usState = 0 ;
     }

     if ( ( *ptrChar == ';' ) ||  /* Remove trailing blanks */
          ( *ptrChar == ',' ) ||
          ( *ptrChar == NULL ) ) {
        if ( *ptrChar == ',' ) 
           *ptrChar = ';' ;
        for( ; ptrChar>pszValues ; )
        {
           if ( isspace(*(ptrChar-1)) )  {
              --ptrChar ;
              memmove( ptrChar, ptrChar+1, strlen(ptrChar+1)+1 ) ;
           } else {
              break;
           }
        }
        usState = 1 ;
     }
     if ( *ptrChar == NULL ) 
        break;
  }
  return ;
} // end CheckDictPIDValues





//   End of EQFFOL01.C
