//+----------------------------------------------------------------------------+
//|  EQFFOL00.C - EQF Folder Handler  - Functions Part 1                       |
//+----------------------------------------------------------------------------+
//|  Copyright Notice:                                                         |
//|                                                                            |
//|      Copyright (C) 1990-2014, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+

#define INCL_EQF_FOLDER           // folder list and document list functions
#define INCL_EQF_DLGUTILS         // dialog utilities
#define INCL_EQF_TAGTABLE         // required for symbolic markup names
#include <eqf.h>                  // General Translation Manager include file

#include "eqfdde.h"               // batch mode defines
#include "eqffol00.h"             // Folder Handler defines
#include "eqfstart.id"            // EQFSTARR resource IDs
#include "eqffol.id"              // Folder Handler IDs
#include "eqfsetup.h"             // our EQF subdirectory structure
#include "eqfhlog.h"              // historylog file stuff
#include "OTMFUNC.H"            // public defines for function call interface
#include "EQFFUNCI.H"           // private defines for function call interface
#include <OTMGLOBMem.h>         // Global Memory defines

#define SEARCH_ALL_STR  "*.*"     // search the whole folder
#define MAX_POOL_SIZE     1000
// debug messages
// #define DEBUGMESSAGES

#ifdef DEBUGMESSAGES
  #define DEBUGMSG( text ) MessageBox( HWND_DESKTOP, text, "Debug info", MB_OK )
#else
  #define DEBUGMSG( text )
#endif

#ifdef DEBUGMESSAGES
#define DEBUGMSG2( text1, text2 ){ CHAR szTxt[80]; sprintf( szTxt, text1, text2 ); MessageBox( HWND_DESKTOP, szTxt, "Debug info", MB_OK ); }
#else
  #define DEBUGMSG2( text1, text2 )
#endif

#ifdef _DEBUG
//  #define MEASURETIME
#endif

#ifdef MEASURETIME
  static LARGE_INTEGER liLast = { 0 };
  static LARGE_INTEGER liFrequency = { 0 };

  static void GetElapsedTime( LONG64 *plTime )
  {
    LARGE_INTEGER liCurrent;

    QueryPerformanceCounter( &liCurrent );

    if ( liFrequency.QuadPart == 0)
    {
      QueryPerformanceFrequency( &liFrequency );
    } /* endif */

    if ( liLast.QuadPart != 0 )
    {
      if ( liFrequency.QuadPart != 0 )
      {
        LONGLONG ldwDiff = liCurrent.QuadPart - liLast.QuadPart;
        DWORD dwTime = (DWORD)((ldwDiff * 1000000) / liFrequency.QuadPart);
        *plTime = *plTime + dwTime;
      }
    } /* endif */
    liLast.QuadPart = liCurrent.QuadPart;
  } /* end of function GetElapsedTime */
#endif


//+----------------------------------------------------------------------------+
//|  Create a new folder                                                       |
//+----------------------------------------------------------------------------+
SHORT CreateFolder
(
HWND            hwnd,
PBOOL           pfMustNotClose,
PSZ             pszObjName
)
{
  PPROPFOLDER     pProp;
  int             rc=1;
  BOOL            fOK;               // internal ok flag

  fOK =  UtlAlloc( (PVOID *)&pProp, 0L, (LONG) sizeof( *pProp), ERROR_STORAGE );

  if ( fOK )
  {
    HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
    UtlMakeEQFPath( pProp->PropHead.szPath, NULC, SYSTEM_PATH, NULL );
    pProp->PropHead.usClass = PROP_CLASS_FOLDER;
    pProp->PropHead.chType = PROP_TYPE_NEW;  // new folder !
    strcpy( pProp->PropHead.szName, pszObjName); // can have predefine name !
    *pfMustNotClose = TRUE;

    DIALOGBOX( QUERYACTIVEWINDOW(), FOLDERPROPSDLG, hResMod, ID_FOLDERPROPS_DLG, pProp, rc );

    *pfMustNotClose = FALSE;
    if ( rc || !WinIsWindow( (HAB)NULL, hwnd))  // WM_DESTROYed ?
    {
      fOK = FALSE;
    } /* endif */
  } /* endif */

  if ( fOK )
  {
    //--- build object name ---
    UtlMakeEQFPath( pszObjName, NULC, SYSTEM_PATH, NULL );
    strcat( pszObjName, BACKSLASH_STR );
    strcat( pszObjName, pProp->PropHead.szName );
    *pszObjName = pProp->chDrive;
  } /* endif */

  if ( pProp )     UtlAlloc( (PVOID *)&pProp, 0L, 0L, NOMSG );

  return( (SHORT) rc );
}


//+----------------------------------------------------------------------------+
//|  Delete folder datasets and directories                                    |
//+----------------------------------------------------------------------------+
SHORT DeleteFolder( PSZ objname, HWND hwndParent, PUSHORT pusYesToAllMode )
{
  EQFINFO     ErrorInfo;
  CHAR        chOrgDrive;            // original object/folder drive
  CHAR        szFolderName[MAX_LONGFILESPEC];  // buffer for folder name
  PSZ         pszParm;               // ptr to error parameter
  CHAR        szPrimDrive[MAX_DRIVE];// buffer for primary drive
  CHAR        szDriveList[MAX_DRIVELIST];// buffer for EQF drive list
  USHORT      usMBCode = MBID_NO;    // message box return code
  SHORT       sRC = 0;

  /******************************************************************/
  /* Extract folder name out of object name                         */
  /******************************************************************/
  Utlstrccpy( szFolderName, UtlGetFnameFromPath( objname ), DOT );
  ObjShortToLongName( szFolderName, szFolderName, FOLDER_OBJECT );
  OEMTOANSI(szFolderName);
  pszParm = szFolderName;

  DEBUGMSG2( "Deleting folder %s", objname );

  /******************************************************************/
  /* Check accessibility of folder                                  */
  /******************************************************************/
  UtlQueryString( QST_VALIDEQFDRIVES, szDriveList, sizeof(szDriveList) );
  if ( (hwndParent == HWND_FUNCIF) || ISBATCHHWND(hwndParent)  )
  {
    usMBCode = MBID_YES;
  }
  else if ( (pusYesToAllMode != NULL) &&
            (*pusYesToAllMode == MB_EQF_YESTOALL) )
  {
    if ( !UtlDirExist(objname) )
    {
      DEBUGMSG2( "Directory %s not found", objname );
      usMBCode = UtlError( QUERY_FOL_DELETE_GREYEDOUT,
                           MB_EQF_YESTOALL | MB_ICONQUESTION | MB_DEFBUTTON2,
                           1, &pszParm, EQF_QUERY );
    }
    else if ( strchr( szDriveList, *objname) == NULL )
    {
      DEBUGMSG2( "Drive letter of object name %s not in drive list", objname );
      usMBCode = UtlError( QUERY_FOL_DELETE_GREYEDOUT,
                           MB_EQF_YESTOALL | MB_ICONQUESTION | MB_DEFBUTTON2,
                           1, &pszParm, EQF_QUERY );
    }
    else
    {
      usMBCode = UtlError( QUERY_DELETE_FOLDER,
                           MB_EQF_YESTOALL | MB_ICONQUESTION | MB_DEFBUTTON2,
                           1, &pszParm, EQF_QUERY );
    } /* endif */
    if ( usMBCode == MBID_EQF_YESTOALL )
    {
      *pusYesToAllMode = usMBCode = MBID_YES;
    }
    else if ( usMBCode == MBID_CANCEL )
    {
      *pusYesToAllMode = MBID_CANCEL;
    } /* endif */
  }
  else if ( (pusYesToAllMode != NULL) && (*pusYesToAllMode == MBID_YES) )
  {
    usMBCode = MBID_YES;
  }
  else if ( !UtlDirExist(objname) )
  {
    DEBUGMSG2( "Directory %s not found", objname );
    usMBCode = UtlError( QUERY_FOL_DELETE_GREYEDOUT,
                         MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2,
                         1, &pszParm, EQF_QUERY );
  }
  else if ( strchr( szDriveList, *objname) == NULL )
  {
    DEBUGMSG2( "Drive letter of object name %s not in drive list", objname );
    usMBCode = UtlError( QUERY_FOL_DELETE_GREYEDOUT,
                         MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2,
                         1, &pszParm, EQF_QUERY );
  }
  else
  {
    usMBCode = UtlError( QUERY_DELETE_FOLDER,
                         MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2,
                         1, &pszParm, EQF_QUERY );
  } /* endif */

  /******************************************************************/
  /* Delete folder                                                  */
  /******************************************************************/
  if ( usMBCode == MBID_YES )
  {
    USHORT usRC = 0;

    DEBUGMSG2( "Removing directory %s", objname );

    // only try to remove the directory if it exists...
    if ( UtlDirExist( objname ) )
    {
      usRC = UtlRemoveDir( objname, TRUE );
    } /* endif */


    if ( !usRC )
    {
      chOrgDrive = *objname;
      UtlQueryString( QST_PRIMARYDRIVE, szPrimDrive, sizeof(szPrimDrive) );
      *objname = szPrimDrive[0];
      DEBUGMSG2( "Deleting properties %s", objname );
      DeleteProperties( objname, NULL, &ErrorInfo );
      *objname = chOrgDrive;

      EqfSend2AllHandlers( WM_EQFN_DELETED,
                          MP1FROMSHORT( clsFOLDER ),
                          MP2FROMP( objname ));
      EqfSend2AllHandlers( WM_EQFN_DELETEDNAME,
                          MP1FROMSHORT( clsFOLDER ),
                          MP2FROMP( szFolderName ));
    }
    else
    {
      CHAR szRC[10];
      sprintf( szRC, "%u", usRC );
      DEBUGMSG2( "Removing directory failed with rc=%s", szRC );

      UtlError( ERROR_DELETE_FOLDER_FAILED, MB_CANCEL, 1, &pszParm, EQF_QUERY );
      sRC = -1;
    } /* endif */
  }
  else
  {
    sRC = -1;
  } /* endif */
  return( sRC );
}

//+----------------------------------------------------------------------------+
//| FolSetDefaultPos                                                           |
//|                                                                            |
//|  set default position of a folder relative to TWB and active folder or fll |
//+----------------------------------------------------------------------------+
VOID FolSetDefaultPos
(
EQF_PSWP pswpPos                    // where to store the position
)
{
  HWND  hwndFol;                      // handle of active folder or folderlist
  SWP   swpTwb;                       // size/position of TWB
  SWP   swpFol;                       // size/position of folder or folderlist

  WinQueryWindowPos( EqfQueryTwbFrame(), &swpTwb );
  hwndFol = EqfQueryActiveFolderHwnd();
  if ( !hwndFol )
  {
    hwndFol = EqfQueryActiveFolderlistHwnd();
  } /* endif */

  //--- get window position used as base for the new document list window ---
  WinQueryWindowPos( GETPARENT( hwndFol ), &swpFol );

  pswpPos->cx = 400;
  pswpPos->cy = 200;

  if ( (swpFol.x + 30) >= (swpTwb.x + swpTwb.cx) )
  {
    pswpPos->x = 100;
  }
  else
  {
    pswpPos->x = swpFol.x + 30;
  } /* endif */

  if ( (swpFol.y + swpFol.cy - 30) <= swpTwb.y )
  {
    pswpPos->y = 50;
  }
  else
  {
    pswpPos->y = swpFol.y + swpFol.cy - 30 - pswpPos->cy;
  } /* endif */

  pswpPos->fs = EQF_SWP_SHOW | EQF_SWP_MOVE | EQF_SWP_SIZE; //  | SWP_ACTIVATE;

} /* end of FolSetDefaultPos */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     FolUpdLangFromMemory                                     |
//+----------------------------------------------------------------------------+
//|Description:       Update folder properties with languages stored in        |
//|                   memory properties                                        |
//+----------------------------------------------------------------------------+
//|Parameters:        HPROP   hFolProp   folder properties handle              |
//|                   BOOL    fMsg       do-message-handling flag              |
//+----------------------------------------------------------------------------+
USHORT FolUpdLangFromMemory
(
HPROP            hFolProp,           // folder properties handle
BOOL             fMsg                // do-message-handling flag
)
{
  return( FolUpdLangFromMemoryHwnd( hFolProp, fMsg, NULLHANDLE ) );
}

USHORT FolUpdLangFromMemoryHwnd
(
HPROP            hFolProp,           // folder properties handle
BOOL             fMsg,               // do-message-handling flag
HWND             hwnd                // owner handle for error messages
)
{
  OBJNAME          szObjName;          // buffer for object name
  HPROP            hMemProp;           // memory property handle
  PPROPTRANSLMEM   pMemProp;           // pointer to TM properties
  EQFINFO          ErrorInfo;          // error info returned by prop functions
  PSZ              pszTemp;            // working pointer
  PPROPFOLDER      pProp;              // pointer to folder properties
  USHORT           usRC = NO_ERROR;    // function return code

  /********************************************************************/
  /* Access folder properties                                         */
  /********************************************************************/
  pProp = (PPROPFOLDER)MakePropPtrFromHnd( hFolProp );
  if ( pProp->szSourceLang[0] && pProp->szTargetLang[0] )
  {
    /******************************************************************/
    /* Folder properties are complete, return asap ...                */
    /******************************************************************/
    return( usRC );
  } /* endif */

  /********************************************************************/
  /* Build fully-qualified name of translation memoiory property file */
  /********************************************************************/
  UtlMakeEQFPath( szObjName, NULC, PROPERTY_PATH, NULL );
  strcat( szObjName, BACKSLASH_STR );
  strcat( szObjName, pProp->szMemory );
  strcat( szObjName, EXT_OF_MEM);
  if ( !UtlFileExist( szObjName ) )
  {
    /******************************************************************/
    /* no tranlation memory with the given name, return asap          */
    /******************************************************************/
    return( usRC );
  } /* endif */


  /********************************************************************/
  /* Build object name for folder translation memory                  */
  /********************************************************************/
  UtlMakeEQFPath( szObjName, NULC, SYSTEM_PATH, NULL );
  strcat( szObjName, BACKSLASH_STR );
  strcat( szObjName, pProp->szMemory );
  strcat( szObjName, EXT_OF_MEM);

  /********************************************************************/
  /* Open memory properties and update folder properties from         */
  /* memory propterties                                               */
  /********************************************************************/
  hMemProp = OpenProperties( szObjName, NULL, PROP_ACCESS_READ, &ErrorInfo);
  if ( hMemProp == NULL )
  {
    if ( fMsg )
    {
      pszTemp = pProp->szMemory;
      UtlErrorHwnd( (USHORT)ErrorInfo, MB_CANCEL, 1, &pszTemp, PROP_ERROR, hwnd );
    } /* endif */
    usRC = (USHORT)ErrorInfo;
  }
  else
  {
    /******************************************************************/
    /* Get the languages from the memory properties ...               */
    /******************************************************************/
    pMemProp = (PPROPTRANSLMEM)MakePropPtrFromHnd( hMemProp );
    strcpy( pProp->szSourceLang, pMemProp->szSourceLang );
    strcpy( pProp->szTargetLang, pMemProp->szTargetLang );

    /**************************************************************/
    /* Close memory properties                                    */
    /**************************************************************/
    CloseProperties( hMemProp, PROP_QUIT, &ErrorInfo );

    /**************************************************************/
    /* Rewrite folder properties if both languages have been set  */
    /**************************************************************/
    if ( pProp->szSourceLang[0] && pProp->szTargetLang[0] )
    {
      if ( SetPropAccess( hFolProp, PROP_ACCESS_WRITE) )
      {
        SaveProperties( hFolProp, &ErrorInfo);
        ResetPropAccess( hFolProp, PROP_ACCESS_WRITE );
      } /* endif */
    } /* endif */
  } /* endif */

  return( usRC );
} /* end of function FolUpdLangFromMemory */




//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     FolBatchFolderDelete                                     |
//+----------------------------------------------------------------------------+
//|Function call:     FolBatchFolderDelete( pFolCrt );                         |
//+----------------------------------------------------------------------------+
//|Description:       Deletes a folder in batch mode.                          |
//+----------------------------------------------------------------------------+
//|Input parameter:   PFOLDEL    pFolDel     folder delete data structure      |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       0       function completed successfully                  |
//|                   other   number of error message for given error condition|
//+----------------------------------------------------------------------------+
//|Function flow:     create invisible listbox for names of folders            |
//|                   check if folder exists                                   |
//|                   check if folder is currently locked                      |
//|                   delete folder                                            |
//|                   broadcast folder-deleted message                         |
//|                   destroy invisible listbox                                |
//|                   cleanup                                                  |
//+----------------------------------------------------------------------------+
USHORT FolBatchFolderDelete
(
PDDEFOLDEL       pFolDel             // folder create data structure
)
{
  BOOL             fIsNew = TRUE;
  CHAR             szShortName[MAX_FILESPEC]; // buffer for folder short name
  PSZ              pszParm;            // pointer for error parameters
  BOOL             fOK = TRUE;         // internal O.K. flag

  ObjLongToShortName( pFolDel->szFolder, szShortName, FOLDER_OBJECT, &fIsNew );
  if ( fIsNew )
  {
    fOK = FALSE;
    pszParm = pFolDel->szFolder;
    pFolDel->DDEReturn.usRc = ERROR_XLATE_FOLDER_NOT_EXIST;
    UtlErrorHwnd( pFolDel->DDEReturn.usRc, MB_CANCEL, 1,
                  &pszParm, EQF_ERROR, pFolDel->hwndErrMsg );
  } /* endif */
  /********************************************************************/
  /* Create folder object name                                        */
  /********************************************************************/
  if ( fOK )
  {
    HPROP      hFolProp;                     // folder properties handle
    EQFINFO    ErrorInfo;                    // return code from prop. handler

    // setup folder object name using the primary EQF drive
    UtlMakeEQFPath( pFolDel->szObjName, SYSTEM_PATH, NULC, NULL );
    strcat( pFolDel->szObjName, BACKSLASH_STR );
    strcat( pFolDel->szObjName, szShortName );
    strcat( pFolDel->szObjName, EXT_FOLDER_MAIN );

    // access folder properties to get the correct folder drive
    hFolProp = OpenProperties( pFolDel->szObjName, NULL,
                               PROP_ACCESS_READ, &ErrorInfo);
    if ( hFolProp )
    {
      PPROPFOLDER pProp;               // ptr to folder properties

      pProp = (PPROPFOLDER)MakePropPtrFromHnd( hFolProp );
      pFolDel->szObjName[0] = pProp->chDrive;
      CloseProperties( hFolProp, PROP_QUIT, &ErrorInfo );
    }
    else
    {
      // issue folder does not exist or cannot be accessed message
      fOK = FALSE;
      pszParm = pFolDel->szFolder;
      pFolDel->DDEReturn.usRc = ERROR_XLATE_FOLDER_NOT_EXIST;
      UtlErrorHwnd( pFolDel->DDEReturn.usRc, MB_CANCEL, 1,
                    &pszParm, EQF_ERROR, pFolDel->hwndErrMsg );
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* Check if folder is open or in use                                */
  /********************************************************************/
  if ( fOK )
  {
    PPROPSYSTEM pPropSys = GetSystemPropPtr();
    HWND hwndObj = EqfQueryObject( pFolDel->szObjName, clsFOLDER, 0 );

    if ( (hwndObj != NULLHANDLE) && !pPropSys->fUseIELikeListWindows )
    {
      fOK = FALSE;
      pszParm = pFolDel->szFolder;
      pFolDel->DDEReturn.usRc = ERROR_DELETE_FOLDER;
      UtlErrorHwnd( pFolDel->DDEReturn.usRc, MB_CANCEL, 1,
                    &pszParm, EQF_ERROR, pFolDel->hwndErrMsg );
    }
    else if ( EqfSend2AllHandlers( WM_EQF_ABOUTTODELETE,
                                   MP1FROMSHORT( clsFOLDER ),
                                   MP2FROMP( pFolDel->szObjName ) ) )
    {
      fOK = FALSE;
      pszParm = pFolDel->szFolder;
      UtlErrorHwnd( ERROR_FOLDER_LOCKED, MB_CANCEL, 1, &pszParm,
                    EQF_ERROR, pFolDel->hwndErrMsg );
    }
    else if ( QUERYSYMBOL( pFolDel->szObjName ) != -1 )
    {
      fOK = FALSE;
      pszParm = pFolDel->szFolder;
      UtlErrorHwnd( ERROR_FOLDER_LOCKED, MB_CANCEL, 1, &pszParm,
                    EQF_ERROR, pFolDel->hwndErrMsg );
    } /* endif */
  } /* endif */


  /********************************************************************/
  /* Delete the folder                                                */
  /********************************************************************/
  if ( fOK )
  {
    DeleteFolder( pFolDel->szObjName, pFolDel->hwndErrMsg, NULL );
  } /* endif */


  /********************************************************************/
  /* Broadcast folder-deleted message                                 */
  /********************************************************************/
  if ( fOK )
  {
    EqfSend2AllHandlers( WM_EQFN_DELETED, MP1FROMSHORT( clsFOLDER ),
                         MP2FROMP(pFolDel->szObjName) );
  } /* endif */

  /********************************************************************/
  /* Tell DDE handler that task has been completed                    */
  /********************************************************************/
  WinPostMsg( pFolDel->hwndOwner, WM_EQF_DDE_ANSWER,
              NULL, &pFolDel->DDEReturn );

  return( pFolDel->DDEReturn.usRc );
} /* end of function FolBatchFolderDelete */

USHORT FolFuncDeleteFolder
(
  PSZ         pszFolderName            // name of folder being deleted
)
{
  PSZ              pszParm;            // pointer for error parameters
  BOOL             fOK = TRUE;         // internal O.K. flag
  USHORT           usRC = NO_ERROR;    // function return code
  OBJNAME          szFolObjName;       // buffer for main folder object name
  OBJNAME          szSubFolObjName;    // buffer for (sub)folder object name
  BOOL             fIsNew = TRUE;
  CHAR             szShortName[MAX_FILESPEC]; // buffer for folder short name
  BOOL             fIsSubFolder = FALSE;

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
      fIsNew = !SubFolNameToObjectName( pszFolderName,  szSubFolObjName );

      if ( !fIsNew )
      {
        PSZ pszDelim;
        pszDelim = strchr( pszFolderName, BACKSLASH );
        if ( pszDelim ) *pszDelim = EOS;
        ObjLongToShortName( pszFolderName, szShortName, FOLDER_OBJECT, &fIsNew );
        if ( pszDelim ) *pszDelim = BACKSLASH ;
        fIsSubFolder = FolIsSubFolderObject( szSubFolObjName );
      } /* endif */

      if ( fIsNew )
      {
        fOK = FALSE;
        pszParm = pszFolderName;
        usRC = ERROR_XLATE_FOLDER_NOT_EXIST;

        UtlErrorHwnd( usRC, MB_CANCEL, 1, &pszParm, EQF_ERROR, HWND_FUNCIF );
      } /* endif */
    } /* endif */
  } /* endif */

  // create main folder object name
  if ( fOK )
  {
    HPROP      hFolProp;                     // folder properties handle
    EQFINFO    ErrorInfo;                    // return code from prop. handler

    // setup folder object name using the primary EQF drive
    UtlMakeEQFPath( szFolObjName, SYSTEM_PATH, NULC, NULL );
    strcat( szFolObjName, BACKSLASH_STR );
    strcat( szFolObjName, szShortName );
    strcat( szFolObjName, EXT_FOLDER_MAIN );

    // access folder properties to get the correct folder drive
    hFolProp = OpenProperties( szFolObjName, NULL,
                               PROP_ACCESS_READ, &ErrorInfo);
    if ( hFolProp )
    {
      PPROPFOLDER pProp;               // ptr to folder properties

      pProp = (PPROPFOLDER)MakePropPtrFromHnd( hFolProp );
      szFolObjName[0] = pProp->chDrive;
      CloseProperties( hFolProp, PROP_QUIT, &ErrorInfo );
    }
    else
    {
      // issue folder does not exist or cannot be accessed message
      fOK = FALSE;
      pszParm = pszFolderName;
      usRC = ERROR_XLATE_FOLDER_NOT_EXIST;

      UtlErrorHwnd( usRC, MB_CANCEL, 1,
                    &pszParm, EQF_ERROR, HWND_FUNCIF );
    } /* endif */
  } /* endif */

  // check if folder is open or in use
  if ( fOK )
  {
    if ( EqfQueryObject( szFolObjName, clsFOLDER, 0) != NULLHANDLE )
    {
      fOK = FALSE;
      pszParm = pszFolderName;
      usRC = ERROR_DELETE_FOLDER;
      UtlErrorHwnd( usRC, MB_CANCEL, 1,
                    &pszParm, EQF_ERROR, HWND_FUNCIF );
    }
    else if ( QUERYSYMBOL( szFolObjName ) != -1 )
    {
      fOK = FALSE;
      pszParm = pszFolderName;
      usRC = ERROR_FOLDER_LOCKED;
      UtlErrorHwnd( ERROR_FOLDER_LOCKED, MB_CANCEL, 1, &pszParm,
                    EQF_ERROR, HWND_FUNCIF );
    } /* endif */
  } /* endif */


  // delete the (sub)folder
  if ( fOK )
  {
    if ( fIsSubFolder )
    {
      if ( !SubFolderDelete( szSubFolObjName, TRUE ) )
      {
        usRC = ERROR_DELETE_FOLDER_FAILED;
        fOK = FALSE;
      } /* endif */
    }
    else
    {
      if ( DeleteFolder( szFolObjName, HWND_FUNCIF, NULL ) != 0 )
      {
        usRC = ERROR_DELETE_FOLDER_FAILED;
        fOK = FALSE;
      } /* endif */

      // additional check: if folder directories have been removed but not the folder property file
      //                   delete the folder property file directly (w/o) usage of property manager)
      if ( usRC != NO_ERROR )
      {
        if ( !UtlDirExist( szFolObjName ) )
        {
          CHAR szPropFileName[MAX_EQF_PATH];
          
          UtlMakeEQFPath( szPropFileName, NULC, PROPERTY_PATH, NULL );
          strcat( szPropFileName, BACKSLASH_STR );
          strcat( szPropFileName, UtlGetFnameFromPath( szFolObjName ) );
          UtlDelete( szPropFileName, 0, NOMSG );

          usRC = NO_ERROR;
          fOK = TRUE;
        } /* endif */
      } /* endif */
    } /* endif */
  } /* endif */

  // broadcast folder-deleted message
  if ( fOK && !fIsSubFolder )
  {
    ObjBroadcast( WM_EQFN_DELETED, clsFOLDER, szFolObjName );
  } /* endif */

  return( usRC );
} /* end of function FolFuncDeleteFolder */


// Subfolder functions

// NON-DDE function for "Create subfolder"


USHORT FolFuncCreateSubFolder
(
PSZ         pszParentFolder,         // name of parent (sub)folder
PSZ         pszSubFolderName,        // subfolders name
PSZ         pszMemName,              // subfolders Translation Memory or NULL
PSZ         pszMarkup,               // name of Markup used for subfolder
PSZ         pszSourceLanguage,       // Source Language used for subfolder
PSZ         pszTargetLanguage,       // Target Language used for subfolder
PSZ         pszEditor,
PSZ         pszConversion,           // Conversion used for subfolder
PSZ         pszTranslator,           // Name of translator
PSZ         pszTranslatorMail        // Mail of translator
)
{
  USHORT           usRC = NO_ERROR;
  PSUBFOLDERDLGIDA pIda = NULL;
  PSZ              pszParm;                // pointer for error parameters
  BOOL             fOK = TRUE;
  USHORT           usErrorID;
  CHAR             szMemName[MAX_LONGFILESPEC];
  CHAR             szTranslator[MAX_LONGFILESPEC];
  CHAR             szTranslatorMail[MAX_LONGFILESPEC];

// check if folder exists
  if ( (pszParentFolder == NULL) || (*pszParentFolder == EOS) )
  {
    fOK = FALSE;
    usRC = TA_MANDFOLDER;
    UtlErrorHwnd( usRC, MB_CANCEL, 0, NULL, EQF_ERROR, HWND_FUNCIF );
  }

  if ( (pszSubFolderName == NULL) || (*pszSubFolderName == EOS) )
  {
    fOK = FALSE;
    usRC = TA_MANDFOLDER;
    UtlErrorHwnd( usRC, MB_CANCEL, 0, NULL, EQF_ERROR, HWND_FUNCIF );
  }

  if(fOK)
  {
    // Allocate document property dialog IDA
    fOK = UtlAlloc( (PVOID *)&pIda, 0L, (LONG)sizeof(SUBFOLDERDLGIDA), ERROR_STORAGE );
    // allocate empty property area
    pIda->pProp = NULL;
    if(fOK)
        fOK = UtlAlloc( (PVOID *)&(pIda->pProp), 0L, sizeof(PROPFOLDER), TRUE );
  }

  if(fOK)
  {
    strcpy(pIda->szParentFolder, pszParentFolder);
    if ( !SubFolNameToObjectName(pIda->szParentFolder, pIda->szParentObjName)  )
    {
      fOK = FALSE;
      pszParm = pszParentFolder;
      usRC = ERROR_XLATE_FOLDER_NOT_EXIST;
      UtlErrorHwnd( usRC, MB_CANCEL, 1, &pszParm, EQF_ERROR, HWND_FUNCIF );
    }
  }

  // build main folder object and property file name
  if ( fOK )
  { 
    // store parent (sub)folder ID in properties
    pIda->pProp->ulParentFolder = FolGetSubFolderIdFromObjName( pIda->szParentObjName );

    if ( strchr( pIda->szParentFolder, BACKSLASH ) != NULL )
    {
      // parent folder is a subfolder, locate end of folder path in subfolder object name
      PSZ pszFolderEnd;
      strcpy( pIda->szMainFolderObjName, pIda->szParentObjName );
      pszFolderEnd = strchr( pIda->szMainFolderObjName, BACKSLASH );
      if ( pszFolderEnd ) pszFolderEnd = strchr( pszFolderEnd+1, BACKSLASH );
      if ( pszFolderEnd ) pszFolderEnd = strchr( pszFolderEnd+1, BACKSLASH );
      if ( pszFolderEnd ) *pszFolderEnd = EOS;
    }
    else
    {
      // parent folder is a main folder, parent object name is object name of main folder
      strcpy( pIda->szMainFolderObjName, pIda->szParentObjName );
    } /* endif */

    // insert property directory in main folder object name to create property file name
    {
      PSZ pszTemp;
      strcpy( pIda->szMainFolderPropName, pIda->szMainFolderObjName );
      pszTemp = strrchr( pIda->szMainFolderPropName, BACKSLASH );
      if ( pszTemp ) pszTemp[1] = EOS;
      strcat( pIda->szMainFolderPropName, PROPDIR );
      pszTemp = strrchr( pIda->szMainFolderObjName, BACKSLASH );
      if ( pszTemp ) strcat( pIda->szMainFolderPropName, pszTemp );
    }

     strcpy(pIda->szBuffer, pszSubFolderName);
  } /* endif */

  // check if name is valid
  if ( fOK && !UtlCheckLongName( pIda->szBuffer )  )
  {
    PSZ pszParm = pIda->szBuffer;
    fOK = FALSE;
    usRC = ERROR_INV_LONGNAME;
    UtlErrorHwnd( usRC, MB_CANCEL, 1, &pszParm, EQF_ERROR, HWND_FUNCIF );
  } /* endif */


  // check if a subfolder with the specified name exists already
  if ( fOK )
  {
    // convert subfolder name to OEM codepage
    ANSITOOEM( pIda->szBuffer );

    // build full name for this subfolder using the parent name
    strcpy( pIda->szBuffer2, pIda->szParentFolder );
    strcat( pIda->szBuffer2, BACKSLASH_STR );
    strcat( pIda->szBuffer2, pIda->szBuffer );

    // use SubFolNameToObjectName function to check if such a subfolder exists
    if ( SubFolNameToObjectName( pIda->szBuffer2, pIda->szBuffer ) )
    {
      PSZ pszParm = pszSubFolderName;
      usRC = ERROR_SUBFOLDER_EXISTS;
      fOK = FALSE;
      UtlErrorHwnd( usRC, MB_CANCEL, 1, &pszParm, EQF_ERROR, HWND_FUNCIF );
    } /* endif */
  } /* endif */

  // store subfolder name in properties
  if ( fOK )
  {
    UtlStripBlanks( pIda->szBuffer );
    ANSITOOEM( pIda->szBuffer );
    strcpy( pIda->pProp->szLongName, pIda->szBuffer );
  } /* endif */

  // Check if given target language is valid
  if ( fOK )
  {
    if ( !(pszTargetLanguage == NULL) && !(*pszTargetLanguage == EOS) )
    {
      if ( !UtlCheckIfExist( pszTargetLanguage, TARGET_LANGUAGE_OBJECT ) )
      {
        pszParm = pszTargetLanguage;
        usRC = ERROR_PROPERTY_LANG_DATA;
        UtlErrorHwnd( ERROR_PROPERTY_LANG_DATA, MB_CANCEL, 1,
                      &pszParm, EQF_ERROR, HWND_FUNCIF );
        fOK = FALSE;
      } /* endif */
      else if (strcmp(pIda->szTargetLang, pszTargetLanguage) != 0)
      {
        strcpy( pIda->pProp->szTargetLang, pszTargetLanguage );
      }
      else
      {
        pIda->pProp->szTargetLang[0] = EOS;
      }
    }// endif
    else
    {
      pIda->pProp->szTargetLang[0] = EOS;
    }
  } /* endif fOK*/

  if ( fOK )
  {
    if ( !(pszSourceLanguage == NULL) && !(*pszSourceLanguage == EOS) )
    {
      if ( !UtlCheckIfExist( pszSourceLanguage, SOURCE_LANGUAGE_OBJECT ) )
      {
        pszParm = pszSourceLanguage;
        usRC = ERROR_PROPERTY_LANG_DATA;
        UtlErrorHwnd( ERROR_PROPERTY_LANG_DATA, MB_CANCEL, 1,
                      &pszParm, EQF_ERROR, HWND_FUNCIF );
        fOK = FALSE;
      } /* endif */
      else if (strcmp(pIda->szSourceLang, pszSourceLanguage) != 0)
      {
        strcpy( pIda->pProp->szSourceLang, pszSourceLanguage );
      }
      else
      {
        pIda->pProp->szSourceLang[0] = EOS;
      }
    }
    else
    {
      pIda->pProp->szSourceLang[0] = EOS;
    }
  } /* endif fOK*/

  if (fOK)
  {
    if ( strcmp( pIda->pProp->szSourceLang, pIda->pProp->szTargetLang ) == 0 )
    {

      if ( UtlErrorHwnd( ERROR_MEM_SAME_LANGUAGES,
                         MB_YESNO | MB_DEFBUTTON2,
                         0, NULL, EQF_QUERY, HWND_FUNCIF ) == MBID_NO )
      {
        usErrorID = ID_FOLNEW_TARGETLANG_CBS;
      } /* endif */
    } /* endif */
  }// endif

  if(fOK)
  {
    if ( pszConversion!=NULL)
    {
      if ( stricmp( pIda->szConversion, pszConversion) != 0)
      {
        strncpy( pIda->pProp->szConversion, pszConversion, sizeof(pIda->pProp->szConversion)-1 );
      }
      else
      {
        pIda->pProp->szConversion[0] = EOS;
      }
    }
    else
    {
      pIda->pProp->szConversion[0] = EOS;
    }
  }//end if

  // check if translation memory exists
  if ( fOK )
  {
    if ( !(pszMemName == NULL) && !(*pszMemName == EOS) )
    {
      strcpy(szMemName, pszMemName);
      if ( !UtlCheckIfExist( szMemName, TM_OBJECT ) )
      {
        fOK = FALSE;
        pszParm = szMemName;
        usRC = ERROR_MEMORY_NOTFOUND;
        UtlErrorHwnd( usRC, MB_CANCEL, 1,
                      &pszParm, EQF_ERROR, HWND_FUNCIF );
      } /* endif */
      else if ( stricmp( pIda->szMemory, szMemName ) != 0 )
      {
        BOOL fIsNew = FALSE;
        ObjLongToShortName( szMemName, pIda->pProp->szMemory, TM_OBJECT, &fIsNew );
        strcpy( pIda->pProp->szLongMemory, szMemName );
      }
      else
      {
        pIda->pProp->szMemory[0] = EOS;
        pIda->pProp->szLongMemory[0] = EOS;
      } /* endif */
    } // endif
    else
    {
      pIda->pProp->szMemory[0] = EOS;
      pIda->pProp->szLongMemory[0] = EOS;
    }
  } /* endif fOK */

  // Set subfolder vendor if not the same as parent vendor
  if ( fOK )
  {
    if ( !(pszTranslator == NULL) && !(*pszTranslator == EOS) )
    {
      strcpy(szTranslator, pszTranslator);
      UtlStripBlanks( szTranslator );
      if ( stricmp( szTranslator, pIda->szVendor ) != 0 )
      {
        strcpy( pIda->pProp->szVendor, szTranslator );
      }
      else
      {
        pIda->pProp->szVendor[0] = EOS;
      } // endif
    } // endif
    else
    {
      pIda->pProp->szVendor[0] = EOS;
    } // endif
  } // endif fOK

  // Set subfolder eMail if not the same as parent eMail
  if ( fOK )
  {
    if ( !(pszTranslatorMail == NULL) && !(*pszTranslatorMail == EOS) )
    {
      strcpy(szTranslatorMail, pszTranslatorMail);
      UtlStripBlanks( szTranslatorMail );
      if ( stricmp( szTranslatorMail, pIda->szVendorEMail ) != 0 )
      {
        strcpy( pIda->pProp->szVendorEMail, szTranslatorMail );
      }
      else
      {
        pIda->pProp->szVendorEMail[0] = EOS;
      } /* endif */
    } // endif
    else
    {
      pIda->pProp->szVendorEMail[0] = EOS;
    } /* endif */
  } /* endif */

  // Set subfolder editor if not the same as parent editor
  if ( fOK )
  {
    if ( !(pszEditor == NULL) && !(*pszEditor == EOS) )
    {
      if ( !UtlCheckIfExist( pszEditor, EDITOR_OBJECT ) )
      {
        fOK = FALSE;
        pszParm = pszEditor;
        usRC = ERROR_EDITOR_NOT_FOUND;
        UtlErrorHwnd( usRC, MB_CANCEL, 1,
                      &pszParm, EQF_ERROR, HWND_FUNCIF );
      } /* endif */
      else if ( stricmp( pszEditor, pIda->szEditor ) != 0 )
      {
        strcpy( pIda->pProp->szEditor, pszEditor );
      }
      else
      {
        pIda->pProp->szEditor[0] = EOS;
      } /* endif */
    }
    else
    {
      pIda->pProp->szEditor[0] = EOS;
    }
  } /* endif */

  //  check markup name if specified
  if ( fOK )
  {
    if ( (pszMarkup != NULL) && (*pszMarkup != EOS) )
    {
      if ( !UtlCheckIfExist( pszMarkup, MARKUP_OBJECT ) )
      {
        PSZ pszParm = pszMarkup;
        fOK = FALSE;
        usRC = ERROR_NO_FORMAT_TABLE_AVA;
        UtlErrorHwnd( usRC, MB_CANCEL, 1,
                      &pszParm, EQF_ERROR, HWND_FUNCIF );
      } /* endif */
      else if ( stricmp(pszMarkup, pIda->szFormat) != 0)
      {
        strcpy(pIda->pProp->szFormat,pszMarkup);
      }
      else
      {
        pIda->pProp->szFormat[0] = EOS;
      }
    }
    else
    {
      pIda->pProp->szFormat[0] = EOS;
    } /* endif */
  } /* endif */

  // setup name of subfolder properties file:
  // get an unique ID for new subfolders and increment main folders
  // next ID field, build fully qualified path and check existence of file
  if ( fOK )
  {
    PPROPFOLDER pFolProp = NULL;
    ULONG ulLen = 0;

    fOK = UtlLoadFileL( pIda->szMainFolderPropName, (PVOID *)&pFolProp, &ulLen, TRUE, TRUE );

    if ( fOK )
    {
      ULONG ulID = pFolProp->ulNextSubFolderID;
      if ( ulID == 0 ) ulID = 1;
      do
      {
        sprintf( pIda->szBuffer, "%c%s\\%s\\%8.8lu%s", pFolProp->chDrive,
                 pIda->szMainFolderObjName+1, PROPDIR, ulID++, EXT_OF_SUBFOLDER );
      }
      while ( UtlFileExist( pIda->szBuffer ) );

      // update next subfolder ID in folder properties
      pFolProp->ulNextSubFolderID = ulID;
      UtlWriteFileL( pIda->szMainFolderPropName, ulLen, pFolProp, FALSE );

      // store subfolder ID and subfolder object name
      strcpy( pIda->szObjectName, pIda->szBuffer );
      pIda->pProp->ulSubFolderID = ulID - 1; // has been post-incremented!
    } /* endif */

    if ( pFolProp ) UtlAlloc( (PVOID *)&pFolProp, 0L, 0L, NOMSG );
  } /* endif */


  // write or create subfolder properties
  if ( fOK )
  {
    fOK = (UtlWriteFile( pIda->szObjectName, sizeof(PROPFOLDER), pIda->pProp, TRUE ) == NO_ERROR);

  } /* endif */

  // release memory allocated
  if(pIda!=NULL && pIda->pProp != NULL)
  {
    UtlAlloc( (PVOID *)&pIda->pProp, 0L, 0L, NOMSG );
  }

  if(pIda != NULL)
  {
    UtlAlloc( (PVOID *)&pIda, 0L, 0L, NOMSG );
  }

  return( usRC );

}// end function

//----------------------------------------------------------------------------//
//   Create a new sub folder                                                  //
//----------------------------------------------------------------------------//
SHORT FolCreateSubFolder
(
PSZ             pszParentObjName   // object name of parent (sub)folder
)
{
  BOOL             fOK = TRUE;         // function return code
  PSUBFOLDERDLGIDA pIda = NULL;        // pointer to subfolder dlg IDA
  HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);

  // Allocate document property dialog IDA
  fOK = UtlAlloc( (PVOID *)&pIda, 0L, (LONG)sizeof(SUBFOLDERDLGIDA), ERROR_STORAGE );

  // Fill-in initial values
  if ( fOK )
  {
    pIda->fPropDlg = FALSE;
    strcpy( pIda->szParentObjName, pszParentObjName );
  } /* endif */

  // Popup subfolder dialog
  DIALOGBOX( /*hwnd*/QUERYACTIVEWINDOW(), SUBFOLDERDLGPROC, hResMod, ID_SUBFOLDERNEW_DLG, pIda, fOK );

  // Cleanup
  if ( pIda ) UtlAlloc( (PVOID *)&pIda, 0L, 0L, NOMSG );

  return( (SHORT)fOK );
} /* end of function FolCreateSubFolder */

//----------------------------------------------------------------------------//
//   Subfolder property dialog                                                //
//----------------------------------------------------------------------------//
SHORT FolSubFolderProperties
(
PSZ             pszObjName          // object name of subfolder
)
{
  BOOL             fOK = TRUE;         // function return code
  PSUBFOLDERDLGIDA pIda = NULL;        // pointer to subfolder dlg IDA
  HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);

  // Allocate document property dialog IDA
  fOK = UtlAlloc( (PVOID *)&pIda, 0L, (LONG)sizeof(SUBFOLDERDLGIDA), ERROR_STORAGE );

  // Fill-in initial values
  if ( fOK )
  {
    pIda->fPropDlg = TRUE;
    strcpy( pIda->szObjectName, pszObjName );
  } /* endif */

  // Popup subfolder dialog
  DIALOGBOX( /*hwnd*/QUERYACTIVEWINDOW(), SUBFOLDERDLGPROC, hResMod, ID_SUBFOLDERNEW_DLG, pIda, fOK );

  // Cleanup
  if ( pIda ) UtlAlloc( (PVOID *)&pIda, 0L, 0L, NOMSG );

  return( (SHORT)fOK );
} /* end of function FolSubFolderProperties */


INT_PTR CALLBACK SUBFOLDERDLGPROC
(
HWND             hwnd,               // dialog window handle
WINMSG           msg,                // message
WPARAM           mp1,                // first message parameter
LPARAM           mp2                 // second message parameter
)
{
  MRESULT          mResult = FALSE;    // function return value
  BOOL             fOK;                // internal OK flag
  SHORT            sItem;              // item index for listboxes and comboboxes
  PSUBFOLDERDLGIDA pIda;               // pointer to subfolder dlg IDA
  SHORT sNotification= WMCOMMANDCMD( mp1, mp2 );

  switch ( msg )
  {
    case WM_EQF_QUERYID:
      pIda = ACCESSDLGIDA( hwnd, PSUBFOLDERDLGIDA );
      if ( pIda->fPropDlg )
      {
        HANDLEQUERYID( ID_SUBFOLDERPROP_DLG, mp2 );
      }
      else
      {
        HANDLEQUERYID( ID_SUBFOLDERNEW_DLG, mp2 );
      } /* endif */
      break;

    case WM_INITDLG :
      {
        HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);

        fOK = TRUE;

        // get and anchor subfolder dialog IDA pointer
        pIda = (PSUBFOLDERDLGIDA)mp2;
        ANCHORDLGIDA( hwnd, pIda );

        SetCtrlFnt (hwnd, GetCharSet(), ID_SUBFOLDER_NAME_TEXT, 0);

        // load or create subfolder properties
        if ( pIda->fPropDlg )
        {
          // load subfolder properties
          ULONG ulLen;
          fOK = UtlLoadFileL( pIda->szObjectName, (PVOID *)&(pIda->pProp), &ulLen, FALSE, TRUE );

          // get subfolder parent (is only set for create dialog)
          if ( fOK )
          {
            if ( pIda->pProp->ulParentFolder == 0 )
            {
              // parent is the main folder, we have to cut off subfolder name and propdir
              strcpy( pIda->szParentObjName, pIda->szObjectName );
              UtlSplitFnameFromPath( pIda->szParentObjName );
              UtlSplitFnameFromPath( pIda->szParentObjName );
            }
            else
            {
              // parent is a subfolder, cut off subfolder name and replace with parent ID
              strcpy( pIda->szParentObjName, pIda->szObjectName );
              UtlSplitFnameFromPath( pIda->szParentObjName );
              sprintf( pIda->szParentObjName+strlen(pIda->szParentObjName),
                       "\\%8.8ld%s", pIda->pProp->ulParentFolder, EXT_OF_SUBFOLDER );
            } /* endif */
          } /* endif */


          ENABLECTRL( hwnd, ID_SUBFOLDER_NAME_TEXT, FALSE );

          // change dialog title
          LOADSTRING( (HAB)UtlQueryULong(QL_HAB), hResMod, SID_SUBFOL_PROP_TITLE,
                      pIda->szBuffer );
          SETTEXTHWND( hwnd, pIda->szBuffer );

        }
        else
        {
          // allocate empty property area
          fOK = UtlAlloc( (PVOID *)&(pIda->pProp), 0L, sizeof(PROPFOLDER), TRUE );

          // store parent (sub)folder ID in properties
          pIda->pProp->ulParentFolder = FolGetSubFolderIdFromObjName( pIda->szParentObjName );
        } /* endif */


        // get full parent (sub)foldername
        if ( fOK )
        {
          SubFolObjectNameToName( pIda->szParentObjName, pIda->szParentFolder );
        } /* endif */

        // build main folder object and property file name
        if ( fOK )
        {
          if ( strchr( pIda->szParentFolder, BACKSLASH ) != NULL )
          {
            // parent folder is a subfolder, locate end of folder path in subfolder object name
            PSZ pszFolderEnd;
            strcpy( pIda->szMainFolderObjName, pIda->szParentObjName );
            pszFolderEnd = strchr( pIda->szMainFolderObjName, BACKSLASH );
            if ( pszFolderEnd ) pszFolderEnd = strchr( pszFolderEnd+1, BACKSLASH );
            if ( pszFolderEnd ) pszFolderEnd = strchr( pszFolderEnd+1, BACKSLASH );
            if ( pszFolderEnd ) *pszFolderEnd = EOS;
          }
          else
          {
            // parent folder is a main folder, parent object name is object name of main folder
            strcpy( pIda->szMainFolderObjName, pIda->szParentObjName );
          } /* endif */

          // insert property directory in main folder object name to create property file name
          {
            // tk 2001-07-20
            PSZ pszTemp;
            UtlMakeEQFPath( pIda->szMainFolderPropName, NULC, PROPERTY_PATH, NULL );
            pszTemp = strrchr( pIda->szMainFolderObjName, BACKSLASH );
            if ( pszTemp ) strcat( pIda->szMainFolderPropName, pszTemp );
          }
        } /* endif */

        // Fill dialog fields with available data
        if ( fOK )
        {
          SETTEXTLIMIT( hwnd, ID_SUBFOLDER_NAME_TEXT, MAX_LONGPATH - 1 );

          // Fill listbox part of comboboxes
          EqfSend2Handler( TAGTABLEHANDLER, WM_EQF_INSERTNAMES,
                           MP1FROMHWND( WinWindowFromID( hwnd, ID_SUBFOLDER_FORMAT_CB) ),
                           0L );
          CBINSERTITEM( hwnd, ID_SUBFOLDER_FORMAT_CB, EMPTY_STRING );


          UtlFillTableLB( WinWindowFromID( hwnd, ID_SUBFOLDER_SRCLNG_CB ),
                          SOURCE_LANGUAGES );
          CBINSERTITEM( hwnd, ID_SUBFOLDER_SRCLNG_CB, EMPTY_STRING );

          UtlFillTableLB( WinWindowFromID( hwnd, ID_SUBFOLDER_TRGLNG_CB ),
                          ALL_TARGET_LANGUAGES );
          CBINSERTITEM( hwnd, ID_SUBFOLDER_TRGLNG_CB, EMPTY_STRING );

          EqfSend2Handler( MEMORYHANDLER, WM_EQF_INSERTNAMES,
                           MP1FROMHWND( WinWindowFromID( hwnd,
                                                         ID_SUBFOLDER_TM_CB )),
                           MP2FROMP( MEMORY_ALL ) );
          CBINSERTITEM( hwnd, ID_SUBFOLDER_TM_CB, EMPTY_STRING );

          CBSETTEXTLIMIT( hwnd, ID_SUBFOLDER_TM_CB, MAX_LONGFILESPEC - 1 );

          // fill editor combobox
          UtlMakeEQFPath( pIda->szBuffer, NULC, PROPERTY_PATH, NULL );
          UtlMakeFullPath( pIda->szBuffer2, NULL, pIda->szBuffer,
                           DEFAULT_PATTERN_NAME, EXT_OF_EDITOR );
          UtlLoadFileNames( pIda->szBuffer2, FILE_NORMAL,
                            WinWindowFromID( hwnd, ID_SUBFOLDER_EDITOR_CB),
                            NAMFMT_NOEXT );
          CBINSERTITEM( hwnd, ID_SUBFOLDER_EDITOR_CB, EMPTY_STRING );

          // fill conversion combobox
          //UtlHandleConversionStrings( CONVLOAD_MODE,
          //                            WinWindowFromID( hwnd, ID_SUBFOLDER_CONV_CB ),
          //                            NULL, NULL, NULL  );
          //CBINSERTITEM( hwnd, ID_SUBFOLDER_CONV_CB, EMPTY_STRING );
          HIDECONTROL( hwnd, ID_SUBFOLDER_CONV_CB );


          // fill last used values in translator combobox
          {

            HPROP           hFLLProp;   // folder list properties handler
            PPROPFOLDERLIST pFLLProp;   // folder list properties pointer
            EQFINFO         ErrorInfo;  // error returned by property handler
            int             i;

            UtlMakeEQFPath( pIda->szBuffer, NULC, SYSTEM_PATH, NULL );
            strcat( pIda->szBuffer, BACKSLASH_STR );
            strcat( pIda->szBuffer, DEFAULT_FOLDERLIST_NAME );
            hFLLProp = OpenProperties( pIda->szBuffer, NULL,
                                       PROP_ACCESS_READ, &ErrorInfo);
            if ( hFLLProp )
            {
              pFLLProp = (PPROPFOLDERLIST)MakePropPtrFromHnd( hFLLProp );

              i=0;
              while (i < 5 && pFLLProp->szTranslatorList[i][0] != EOS)
              {
                OEMTOANSI( pFLLProp->szTranslatorList[i] );
                CBINSERTITEMEND( hwnd, ID_SUBFOLDER_TRANSL_CB, pFLLProp->szTranslatorList[i] );
                ANSITOOEM( pFLLProp->szTranslatorList[i] );
                i++;
              } // end while
              CloseProperties( hFLLProp, PROP_QUIT, &ErrorInfo);
            } /* endif */
          }

          // show current settings for property dialogs
          if ( pIda->fPropDlg )
          {
            // change name of create pushbutton to "Set"
            LOADSTRING( (HAB)UtlQueryULong(QL_HAB), hResMod, SID_NFOL_CHANGE,
                        pIda->szBuffer);
            SETTEXT( hwnd, ID_SUBFOLDER_CHANGE_PB, pIda->szBuffer );

            // show subfolder name
            OEMTOANSI( pIda->pProp->szLongName );
            SETTEXT( hwnd, ID_SUBFOLDER_NAME_TEXT, pIda->pProp->szLongName );
            ANSITOOEM( pIda->pProp->szLongName );

            if ( pIda->pProp->szFormat[0] != EOS )
            {
              CBSEARCHSELECT( sItem, hwnd, ID_SUBFOLDER_FORMAT_CB, pIda->pProp->szFormat );
              if ( sItem == LIT_NONE )
              {
                PSZ pszErrParm = pIda->pProp->szFormat;
                UtlError( ERROR_FORMAT_NOTFOUND, MB_OK, 1, &pszErrParm, EQF_ERROR );
                CBSELECTITEM( hwnd, ID_SUBFOLDER_FORMAT_CB, 0 );
              } /* endif */
            } /* endif */

            if ( pIda->pProp->szLongMemory[0] != EOS )
            {
              OEMTOANSI( pIda->pProp->szLongMemory );
              CBSEARCHSELECT( sItem, hwnd, ID_SUBFOLDER_TM_CB, pIda->pProp->szLongMemory );
              if ( sItem == LIT_NONE)
              {
                PSZ pszErrParm = pIda->pProp->szLongMemory;
                UtlError( ERROR_MEMORY_NOTFOUND, MB_OK, 1, &pszErrParm, EQF_ERROR );
                SETTEXT( hwnd, ID_SUBFOLDER_TM_CB, pIda->pProp->szLongMemory );
              } /* endif */
              ANSITOOEM( pIda->pProp->szLongMemory );
            }
            else if ( pIda->pProp->szMemory[0] != EOS )
            {
              CBSEARCHSELECT( sItem, hwnd, ID_SUBFOLDER_TM_CB, pIda->pProp->szMemory );
              if ( sItem == LIT_NONE)
              {
                PSZ pszErrParm = pIda->pProp->szMemory;
                UtlError( ERROR_MEMORY_NOTFOUND, MB_OK, 1, &pszErrParm, EQF_ERROR );
                SETTEXT( hwnd, ID_SUBFOLDER_TM_CB, pIda->pProp->szMemory );
              } /* endif */
            } /* endif */


            CBSEARCHSELECT( sItem, hwnd, ID_SUBFOLDER_SRCLNG_CB, pIda->pProp->szSourceLang );
            CBSEARCHSELECT( sItem, hwnd, ID_SUBFOLDER_TRGLNG_CB, pIda->pProp->szTargetLang );

            CBSEARCHSELECT( sItem, hwnd, ID_SUBFOLDER_EDITOR_CB, pIda->pProp->szEditor );
  //        CBSEARCHSELECT( sItem, hwnd, ID_SUBFOLDER_CONV_CB, pIda->pProp->szConversion );

            if ( pIda->pProp->szVendor[0] != EOS )
            {
              OEMTOANSI(pIda->pProp->szVendor);
              CBSEARCHSELECT( sItem, hwnd, ID_SUBFOLDER_TRANSL_CB, pIda->pProp->szVendor );
              if ( sItem == LIT_NONE )
              {
                CBINSERTITEM( hwnd, ID_SUBFOLDER_TRANSL_CB , pIda->pProp->szVendor );
                CBSEARCHSELECT( sItem, hwnd, ID_SUBFOLDER_TRANSL_CB , pIda->pProp->szVendor );
              } /* endif */
              ANSITOOEM(pIda->pProp->szVendor);
            } /* endif */

            if ( pIda->pProp->szVendorEMail[0] != EOS )
            {
              OEMSETTEXT( hwnd, ID_SUBFOLDER_EMAIL_EF, pIda->pProp->szVendorEMail );
            } /* endif */
          }
          else
          {
          } /* endif */

          // Fill folder info fields
          FolQueryInfoEx( pIda->szParentObjName,    // parent (sub)folder object name
                          pIda->szMemory,
                          pIda->szFormat,
                          pIda->szSourceLang,
                          pIda->szTargetLang,
                          pIda->szEditor,
                          pIda->szConversion,
                          pIda->szVendor,
                          pIda->szVendorEMail, NULL, NULL, NULL, TRUE, hwnd );

          SETTEXT( hwnd, ID_SUBFOLDER_PARFORMAT_TEXT, pIda->szFormat );
          SETTEXT( hwnd, ID_SUBFOLDER_PARTM_TEXT,     pIda->szMemory );
          SETTEXT( hwnd, ID_SUBFOLDER_PARSRCLNG_TEXT, pIda->szSourceLang );
          SETTEXT( hwnd, ID_SUBFOLDER_PARTRGLNG_TEXT, pIda->szTargetLang );
          SETTEXT( hwnd, ID_SUBFOLDER_PAREDITOR_TEXT, pIda->szEditor );
          SETTEXT( hwnd, ID_SUBFOLDER_PARCONV_TEXT,   pIda->szConversion );
          OEMSETTEXT( hwnd, ID_SUBFOLDER_PARTRANSL_TEXT, pIda->szVendor );
          OEMSETTEXT( hwnd, ID_SUBFOLDER_PAREMAIL_TEXT,  pIda->szVendorEMail );
          OEMSETTEXT( hwnd, ID_SUBFOLDER_PARENTNAME_TEXT, pIda->szParentFolder );
        } /* endif */

        // End dialog in case of failures or set focus to first control of dialog                                          */
        if ( fOK )
        {
          UtlCheckDlgPos( hwnd, FALSE );
          if ( pIda->fPropDlg )
          {
            SETFOCUS( hwnd, ID_SUBFOLDER_TM_CB );
          }
          else
          {
            SETFOCUS( hwnd, ID_SUBFOLDER_NAME_TEXT );
          } /* endif */
        }
        else
        {
          WinPostMsg( hwnd, WM_EQF_CLOSE, MP1FROMSHORT(FALSE), NULL );
        } /* endif */
      }
      break;

    case WM_EQF_CLOSE :
      DelCtrlFont(hwnd, ID_SUBFOLDER_NAME_TEXT);
      DISMISSDLG( hwnd, mp1 );
      break;

    case WM_DESTROY :
      pIda = ACCESSDLGIDA( hwnd, PSUBFOLDERDLGIDA );
      break;

    case WM_COMMAND :
      switch ( WMCOMMANDID( mp1, mp2 ) )
      {
        case ID_SUBFOLDER_HELP_PB:
          UtlInvokeHelp();
          break;

        case ID_SUBFOLDER_CANCEL_PB :  // CANCEL button selected
        case DID_CANCEL :            // ESC key pressed
          // Leave dialog and discard changes
          WinPostMsg( hwnd, WM_EQF_CLOSE, MP1FROMSHORT(FALSE), NULL );
          break;

        case ID_SUBFOLDER_CHANGE_PB :
          // Get access to IDA
          pIda = ACCESSDLGIDA( hwnd, PSUBFOLDERDLGIDA );
          fOK = TRUE;

          // Check subfolder name
          if ( fOK && !pIda->fPropDlg )
          {
            // get specified subfolder name
            QUERYTEXT( hwnd, ID_SUBFOLDER_NAME_TEXT, pIda->szBuffer );
            UtlStripBlanks( pIda->szBuffer );

            // check if a name has been specified
            if ( pIda->szBuffer[0] == EOS )
            {
              UtlErrorHwnd( ERROR_SUBFOLDERNAME_REQUIRED, MB_CANCEL, 0, NULL, EQF_ERROR, hwnd );
              fOK = FALSE;
              SETFOCUS( hwnd, ID_SUBFOLDER_NAME_TEXT );
            } /* endif */

            // check if name is valid
            if ( fOK && !UtlCheckLongName( pIda->szBuffer )  )
            {
              PSZ pszParm = pIda->szBuffer;
              UtlErrorHwnd( ERROR_INV_LONGNAME, MB_CANCEL, 1, &pszParm, EQF_ERROR, hwnd );
              fOK = FALSE;
              SETFOCUS( hwnd, ID_SUBFOLDER_NAME_TEXT );
            } /* endif */


            // check if a subfolder with the specified name exists already
            if ( fOK )
            {
              // convert subfolder name to OEM codepage
              ANSITOOEM( pIda->szBuffer );

              // build full name for this subfolder using the parent name
              strcpy( pIda->szBuffer2, pIda->szParentFolder );
              strcat( pIda->szBuffer2, BACKSLASH_STR );
              strcat( pIda->szBuffer2, pIda->szBuffer );

              // use SubFolNameToObjectName function to check if such a subfolder exists
              if ( SubFolNameToObjectName( pIda->szBuffer2, pIda->szBuffer ) )
              {
                PSZ pszParm = pIda->szBuffer;
                QUERYTEXT( hwnd, ID_SUBFOLDER_NAME_TEXT, pIda->szBuffer );
                UtlStripBlanks( pIda->szBuffer );
                UtlErrorHwnd( ERROR_SUBFOLDER_EXISTS, MB_CANCEL, 1, &pszParm, EQF_ERROR, hwnd );
                fOK = FALSE;
                SETFOCUS( hwnd, ID_SUBFOLDER_NAME_TEXT );
              } /* endif */
            } /* endif */

            // store subfolder name in properties
            if ( fOK )
            {
              QUERYTEXT( hwnd, ID_SUBFOLDER_NAME_TEXT, pIda->szBuffer );
              UtlStripBlanks( pIda->szBuffer );
              ANSITOOEM( pIda->szBuffer );
              strcpy( pIda->pProp->szLongName, pIda->szBuffer );
            } /* endif */
          } /* endif */

          // Check source and target language
          QUERYTEXT( hwnd, ID_SUBFOLDER_SRCLNG_CB, pIda->szBuffer );
          UtlStripBlanks( pIda->szBuffer );
          if ( pIda->szBuffer[0] == EOS )
          {
            QUERYTEXT( hwnd, ID_SUBFOLDER_PARSRCLNG_TEXT, pIda->szBuffer );
          } /* endif */

          QUERYTEXT( hwnd, ID_SUBFOLDER_TRGLNG_CB, pIda->szBuffer2 );
          UtlStripBlanks( pIda->szBuffer2 );
          if ( pIda->szBuffer2[0] == EOS )
          {
            QUERYTEXT( hwnd, ID_SUBFOLDER_PARTRGLNG_TEXT, pIda->szBuffer2 );
          } /* endif */


          if ( strcmp( pIda->szBuffer, pIda->szBuffer2 ) == 0 )
          {
            if ( UtlErrorHwnd( ERROR_MEM_SAME_LANGUAGES,
                               MB_YESNO | MB_DEFBUTTON2,
                               0, NULL, EQF_QUERY, hwnd ) == MBID_NO )
            {
              fOK = FALSE;
              SETFOCUS( hwnd, ID_SUBFOLDER_SRCLNG_CB );
            } /* endif */
          } /* endif */

          // Check memory, call memory handler to create new memories
          if ( fOK )
          {
            QUERYTEXT( hwnd, ID_SUBFOLDER_TM_CB, pIda->szBuffer );
            ANSITOOEM( pIda->szBuffer );
            QUERYTEXT( hwnd, ID_SUBFOLDER_PARTM_TEXT, pIda->szBuffer2 );
            ANSITOOEM( pIda->szBuffer2 );
            UtlStripBlanks( pIda->szBuffer );
            if ( pIda->szBuffer[0] == EOS )
            {
              // ignore empty document memory
            }
            else if ( stricmp( pIda->szBuffer, pIda->szBuffer2 ) != 0 )
            {
              SHORT sItem;

              // Document memory is different from folder memory
              BOOL fIsNew = FALSE;
              CHAR szShortName[MAX_FNAME];
              strcpy( pIda->szNewMemory, pIda->szBuffer );
              ObjLongToShortName( pIda->szNewMemory, szShortName,
                                  TM_OBJECT, &fIsNew );
              if ( fIsNew )
              {
                // This is the name of a new memory ...
                pIda->fDisabled = TRUE;

                // call the memory handler to create a new TM
                strcat( pIda->szBuffer, X15_STR );
                QUERYTEXT( hwnd, ID_SUBFOLDER_FORMAT_CB, pIda->szBuffer2 );
                if ( pIda->szBuffer2[0] == EOS )
                {
                  QUERYTEXT( hwnd, ID_SUBFOLDER_PARFORMAT_TEXT, pIda->szBuffer2 );
                } /* endif */
                strcat( pIda->szBuffer, pIda->szBuffer2 );
                strcat( pIda->szBuffer, X15_STR );

                QUERYTEXT( hwnd, ID_SUBFOLDER_SRCLNG_CB, pIda->szBuffer2 );
                if ( pIda->szBuffer2[0] == EOS )
                {
                  QUERYTEXT( hwnd, ID_SUBFOLDER_PARSRCLNG_TEXT, pIda->szBuffer2 );
                } /* endif */
                strcat( pIda->szBuffer, pIda->szBuffer2 );
                strcat( pIda->szBuffer, X15_STR );

                QUERYTEXT( hwnd, ID_SUBFOLDER_TRGLNG_CB, pIda->szBuffer2 );
                if ( pIda->szBuffer2[0] == EOS )
                {
                  QUERYTEXT( hwnd, ID_SUBFOLDER_PARTRGLNG_TEXT, pIda->szBuffer2 );
                } /* endif */
                strcat( pIda->szBuffer, pIda->szBuffer2 );
                strcat( pIda->szBuffer, X15_STR );

                sItem = (SHORT)(! (BOOL) EqfSend2Handler( MEMORYHANDLER,
                                                  WM_EQF_CREATE,
                                                  MP1FROMSHORT(0),
                                                  MP2FROMP( pIda->szBuffer ) ));

                pIda->fDisabled = FALSE;
                SETFOCUSHWND( hwnd );

                if ( !sItem )
                {
                  //--- update list of memory dbs
                  CBDELETEALL( hwnd, ID_SUBFOLDER_TM_CB );
                  EqfSend2Handler( MEMORYHANDLER,
                                   WM_EQF_INSERTNAMES,
                                   MP1FROMHWND( WinWindowFromID( hwnd,
                                                                 ID_SUBFOLDER_TM_CB) ),
                                   MP2FROMP( MEMORY_ALL ) );

                  // set name to uppercase
                  SETTEXT( hwnd, ID_SUBFOLDER_TM_CB, pIda->szNewMemory );
                  ANSITOOEM( pIda->szNewMemory );
                }
                else
                {
                  fOK = FALSE;        // something went wrong during TM create
                } /* endif */
              } /* endif */
            } /* endif */
          } /* endif */

          // Check editor
          if ( fOK )
          {
          } /* endif */

          // Update or create supfolder properties
          if ( fOK )
          {
            // Set subfolder format if not the same as parent format
            QUERYTEXT( hwnd, ID_SUBFOLDER_FORMAT_CB, pIda->szBuffer );
            QUERYTEXT( hwnd, ID_SUBFOLDER_PARFORMAT_TEXT, pIda->szBuffer2 );
            if ( stricmp( pIda->szBuffer, pIda->szBuffer2 ) != 0 )
            {
              strcpy( pIda->pProp->szFormat, pIda->szBuffer );
            }
            else
            {
              pIda->pProp->szFormat[0] = EOS;
            } /* endif */

            // Set subfolder memory if not the same as parent memory
            QUERYTEXT( hwnd, ID_SUBFOLDER_TM_CB, pIda->szBuffer );
            QUERYTEXT( hwnd, ID_SUBFOLDER_PARTM_TEXT, pIda->szBuffer2 );
            ANSITOOEM( pIda->szBuffer );
            UtlStripBlanks( pIda->szBuffer );
            if ( pIda->szBuffer[0] == EOS )
            {
              pIda->pProp->szMemory[0] = EOS;
              pIda->pProp->szLongMemory[0] = EOS;
            }
            else if ( stricmp( pIda->szBuffer, pIda->szBuffer2 ) != 0 )
            {
              BOOL fIsNew = FALSE;
              ObjLongToShortName( pIda->szBuffer, pIda->pProp->szMemory, TM_OBJECT, &fIsNew );
              strcpy( pIda->pProp->szLongMemory, pIda->szBuffer );
            }
            else
            {
              pIda->pProp->szMemory[0] = EOS;
              pIda->pProp->szLongMemory[0] = EOS;
            } /* endif */

            // Set subfolder source language if not the same as parent source language
            if ( fOK )
            {
              QUERYTEXT( hwnd, ID_SUBFOLDER_SRCLNG_CB, pIda->szBuffer );
              QUERYTEXT( hwnd, ID_SUBFOLDER_PARSRCLNG_TEXT, pIda->szBuffer2 );
              if ( stricmp( pIda->szBuffer, pIda->szBuffer2 ) != 0 )
              {
                strcpy( pIda->pProp->szSourceLang, pIda->szBuffer );
              }
              else
              {
                pIda->pProp->szSourceLang[0] = EOS;
              } /* endif */
            } /* endif */

            // Set subfolder target language if not the same as parent target language
            if ( fOK )
            {
              QUERYTEXT( hwnd, ID_SUBFOLDER_TRGLNG_CB, pIda->szBuffer );
              QUERYTEXT( hwnd, ID_SUBFOLDER_PARTRGLNG_TEXT, pIda->szBuffer2 );
              if ( stricmp( pIda->szBuffer, pIda->szBuffer2 ) != 0 )
              {
                strcpy( pIda->pProp->szTargetLang, pIda->szBuffer );
              }
              else
              {
                pIda->pProp->szTargetLang[0] = EOS;
              } /* endif */
            } /* endif */

            // Set subfolder editor if not the same as parent editor
            if ( fOK )
            {
              QUERYTEXT( hwnd, ID_SUBFOLDER_EDITOR_CB, pIda->szBuffer );
              QUERYTEXT( hwnd, ID_SUBFOLDER_PAREDITOR_TEXT, pIda->szBuffer2 );
              if ( stricmp( pIda->szBuffer, pIda->szBuffer2 ) != 0 )
              {
                strcpy( pIda->pProp->szEditor, pIda->szBuffer );
              }
              else
              {
                pIda->pProp->szEditor[0] = EOS;
              } /* endif */
            } /* endif */

            // Set subfolder conversion if not the same as parent conversion
            //if ( fOK )
            //{
            //  QUERYTEXT( hwnd, ID_SUBFOLDER_CONV_CB, pIda->szBuffer );
            //  QUERYTEXT( hwnd, ID_SUBFOLDER_PARCONV_TEXT, pIda->szBuffer2 );
            //  UtlStripBlanks( pIda->szBuffer );
            //  UtlStripBlanks( pIda->szBuffer2 );
            //  if ( stricmp( pIda->szBuffer, pIda->szBuffer2 ) != 0 )
            //  {
              //  strcpy( pIda->pProp->szConversion, pIda->szBuffer );
              //}
              //else
              //{
                pIda->pProp->szConversion[0] = EOS;
              //} /* endif */
            //} /* endif */

            // Set subfolder vendor if not the same as parent vendor
            if ( fOK )
            {
              OEMQUERYTEXT( hwnd, ID_SUBFOLDER_TRANSL_CB, pIda->szBuffer );
              OEMQUERYTEXT( hwnd, ID_SUBFOLDER_PARTRANSL_TEXT, pIda->szBuffer2 );
              UtlStripBlanks( pIda->szBuffer );
              UtlStripBlanks( pIda->szBuffer2 );
              if ( stricmp( pIda->szBuffer, pIda->szBuffer2 ) != 0 )
              {
                strcpy( pIda->pProp->szVendor, pIda->szBuffer );
              }
              else
              {
                pIda->pProp->szVendor[0] = EOS;
              } /* endif */
              OEMQUERYTEXT( hwnd, ID_SUBFOLDER_EMAIL_EF, pIda->szBuffer );
              OEMQUERYTEXT( hwnd, ID_SUBFOLDER_PAREMAIL_TEXT, pIda->szBuffer2 );
              UtlStripBlanks( pIda->szBuffer );
              UtlStripBlanks( pIda->szBuffer2 );
              if ( stricmp( pIda->szBuffer, pIda->szBuffer2 ) != 0 )
              {
                strcpy( pIda->pProp->szVendorEMail, pIda->szBuffer );
              }
              else
              {
                pIda->pProp->szVendorEMail[0] = EOS;
              } /* endif */

              // update vendor name last used values
              if (pIda->pProp->szVendor[0] != EOS)
              {

                HPROP           hFLLProp;   // folder list properties handler
                PPROPFOLDERLIST pFLLProp;   // folder list properties pointer
                EQFINFO         ErrorInfo;  // error returned by property handler

                UtlMakeEQFPath( pIda->szBuffer2,  NULC, SYSTEM_PATH, NULL );
                strcat( pIda->szBuffer2, BACKSLASH_STR );
                strcat( pIda->szBuffer2, DEFAULT_FOLDERLIST_NAME );
                hFLLProp = OpenProperties( pIda->szBuffer2, NULL,
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
              } /* endif */
            } /* endif */

            // Set subfolder eMail if not the same as parent eMail
            if ( fOK )
            {
              OEMQUERYTEXT( hwnd, ID_SUBFOLDER_EMAIL_EF, pIda->szBuffer );
              OEMQUERYTEXT( hwnd, ID_SUBFOLDER_PAREMAIL_TEXT, pIda->szBuffer2 );
              UtlStripBlanks( pIda->szBuffer );
              UtlStripBlanks( pIda->szBuffer2 );
              if ( stricmp( pIda->szBuffer, pIda->szBuffer2 ) != 0 )
              {
                strcpy( pIda->pProp->szVendorEMail, pIda->szBuffer );
              }
              else
              {
                pIda->pProp->szVendorEMail[0] = EOS;
              } /* endif */
            } /* endif */

            // setup name of subfolder properties file:
            // get an unique ID for new subfolders and increment main folders
            // next ID field, build fully qualified path and check existence of file
            if ( fOK && !pIda->fPropDlg )
            {
              PPROPFOLDER pFolProp = NULL;
              ULONG ulLen = 0;

              fOK = UtlLoadFileL( pIda->szMainFolderPropName, (PVOID *)&pFolProp, &ulLen, TRUE, TRUE );

              if ( fOK )
              {
                ULONG ulID = pFolProp->ulNextSubFolderID;
                if ( ulID == 0 ) ulID = 1;
                do
                {
                  sprintf( pIda->szBuffer, "%c%s\\%s\\%8.8lu%s", pFolProp->chDrive,
                           pIda->szMainFolderObjName+1, PROPDIR, ulID++, EXT_OF_SUBFOLDER );
                }
                while ( UtlFileExist( pIda->szBuffer ) );

                // update next subfolder ID in folder properties
                pFolProp->ulNextSubFolderID = ulID;
                UtlWriteFileL( pIda->szMainFolderPropName, ulLen, pFolProp, FALSE );

                // store subfolder ID and subfolder object name
                strcpy( pIda->szObjectName, pIda->szBuffer );
                pIda->pProp->ulSubFolderID = ulID - 1; // has been post-incremented!
              } /* endif */

              if ( pFolProp ) UtlAlloc( (PVOID *)&pFolProp, 0L, 0L, NOMSG );
            } /* endif */


            // write or create subfolder properties
            if ( fOK )
            {
              fOK = (UtlWriteFile( pIda->szObjectName, sizeof(PROPFOLDER), pIda->pProp, TRUE ) == NO_ERROR);
            } /* endif */


            // broadcast changed subfolder properties or a new subfolder
            if ( fOK )
            {
              if ( pIda->fPropDlg )
              {
                EqfSend2AllHandlers( WM_EQFN_PROPERTIESCHANGED,
                                     MP1FROMSHORT( PROP_CLASS_DOCUMENT ),
                                     MP2FROMP( pIda->szObjectName ));
              }
              else
              {
                EqfSend2AllHandlers( WM_EQFN_CREATED,
                                     MP1FROMSHORT( clsDOCUMENT ),
                                     MP2FROMP( pIda->szObjectName ));

              } /* endif */
            } /* endif */
          } /* endif */

          // Close dialog
          if ( fOK )
          {
            WinPostMsg( hwnd, WM_EQF_CLOSE, MP1FROMSHORT(0), NULL );
          } /* endif */
          break;

        case ID_SUBFOLDER_NAME_TEXT:
          if ( WMCOMMANDCMD( mp1, mp2 ) == EN_KILLFOCUS )
          {
            ClearIME( hwnd );
          } /* endif */
          break;
        case ID_SUBFOLDER_EMAIL_EF:
          {
            mResult = MRFROMSHORT(TRUE);
          }
          break;

        case ID_SUBFOLDER_TRANSL_CB:

          if (sNotification == CBN_SELCHANGE )
          {
            int iFound = -1;
            int i;
            PPROPFOLDERLIST pFLLProp;   // folder list properties pointer
            HPROP           hFLLProp;   // folder list properties handler
            EQFINFO         ErrorInfo;  // error returned by property handler

            pIda = ACCESSDLGIDA(hwnd, PSUBFOLDERDLGIDA );
            {
              UtlMakeEQFPath( pIda->szBuffer, NULC, SYSTEM_PATH, NULL );
              strcat( pIda->szBuffer, BACKSLASH_STR );
              strcat( pIda->szBuffer, DEFAULT_FOLDERLIST_NAME );
              hFLLProp = OpenProperties( pIda->szBuffer, NULL,
                                         PROP_ACCESS_READ, &ErrorInfo);
              if ( hFLLProp )
              {
                pFLLProp = (PPROPFOLDERLIST)MakePropPtrFromHnd( hFLLProp );
                {
                  int ipos;

                  CBQUERYSELECTEDITEMTEXT (ipos ,hwnd,
                                           ID_SUBFOLDER_TRANSL_CB , pIda->pProp->szVendor);
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
                      SETTEXT(hwnd, ID_SUBFOLDER_EMAIL_EF, pIda->pProp->szVendorEMail);
                      ANSITOOEM( pIda->pProp->szVendorEMail);
                      SETFOCUS( hwnd, ID_SUBFOLDER_EMAIL_EF );
                    } // end if
                  } // end if

                } // end if vendor
                CloseProperties( hFLLProp, PROP_QUIT, &ErrorInfo);

              } // endif
            } // end notification
          } // end case
          mResult = MRFROMSHORT(TRUE);
          break;
      } /* endswitch */
      break;

    default :
      mResult = WinDefDlgProc( hwnd, msg, mp1, mp2 );
      break;
  } /* endswitch */
  return( mResult );
} /* end of function SubFolderDlgProc */



//  Fill List Box with subfolder names
//   Special mode if listbox handle is HWND_FUNCIF in this case
//   the pszBuffer is the address!!! of a buffer pointer and the area
//   pointed to by this pointer will contain the names of the subfolders on
//   return
USHORT FolLoadSubFolderNames( PSZ pszParentObjName, HWND hwndLB, LONG lFlag, PSZ pszBuffer )
{
  FILEFINDBUF stResultBuf;             // DOS file find struct
  USHORT      usCount = 1;             // number of files requested
  USHORT      usRC;                    // return code of called functions
  HDIR        hDirHandle = HDIR_CREATE;// DosFind routine handle
  PSZ          pszPath;
  PSZ          pszFileName;            // subfolder properties file name
  PSZ          pszSearchName;          // subfolder properties search path
  PPROPFOLDER  pProp = NULL;           // subfolder properties
  CHAR         szFormat[MAX_FILESPEC]; // parent folder format / Tag Table
  PSZ          pszFormat;
  static CHAR  szMemory[MAX_LONGFILESPEC]; // folder Translation Memory
  PSZ          pszMemory;
  CHAR         szSourceLang[MAX_LANG_LENGTH];  // parent folder source language
  PSZ          pszSourceLang;
  CHAR         szTargetLang[MAX_LANG_LENGTH];  // folder target language
  PSZ          pszTargetLang;
  CHAR         szEditor[MAX_FILESPEC]; // folder editor
  PSZ          pszEditor;
  PSZ  pszName = RESBUFNAME(stResultBuf);// address name field in result buffer
  USHORT      usDocs  = 0;             // number of documents found
  ULONG       ulParentID = 0L;         // ID of parent folder
  BOOL        fAllSubFolders = FALSE;  // TRUE = ignore parentship and add all subfolders
  // variables for document name buffer
  LONG        lBufferSize = 0L;        // size of document buffer
  LONG        lBufferUsed = 0L;        // used bytes in document buffer
  PSZ         pDocNameBuffer = NULL;   // ptr to document buffer

  //get and remove any all-subfolders-flag
  if ( lFlag & LOADSUBFOLNAMES_ALLSUBFOLDERS )
  {
    fAllSubFolders = TRUE;
    lFlag = lFlag & ~LOADSUBFOLNAMES_ALLSUBFOLDERS;
  } /* endif */

  // Get settings from parent (sub)older
  FolQueryInfo2( pszParentObjName, szMemory, szFormat, szSourceLang,
                 szTargetLang, szEditor, FALSE );

  if ( hwndLB != HWND_FUNCIF )
  {
    ENABLEUPDATEHWND_FALSE( hwndLB );
    SETCURSOR( SPTR_WAIT );
  } /* endif */

  UtlAlloc( (PVOID *)&pszPath, 0L, (LONG) (3 * MAX_PATH144), ERROR_STORAGE );
  pszFileName = pszPath + MAX_PATH144;
  pszSearchName = pszFileName + MAX_PATH144;

  // build subfolder search path (cut off subfolder path if necessary)
  // and get ID of parent subfolder (if any)
  {
    PSZ pszEnd;
    strcpy( pszPath, pszParentObjName );
    pszEnd = strchr( pszPath, BACKSLASH );                  // position to EQF dir
    if ( pszEnd ) pszEnd = strchr( pszEnd + 1, BACKSLASH ); // position to folder name
    if ( pszEnd ) pszEnd = strchr( pszEnd + 1, BACKSLASH ); // position to property dir
    if ( pszEnd )
    {
      ULONG ulLen;

      // load subfolder property file
      if ( UtlLoadFileL( pszPath, (PVOID *)&pProp, &ulLen, FALSE, FALSE ) )
      {
        ulParentID = pProp->ulSubFolderID;
        UtlAlloc( (PVOID *)&pProp, 0L, 0L, NOMSG );
        pProp = NULL;
      } /* endif */

      // cutoff property dir and subfolder name from path
      *pszEnd = EOS;
    }
    strcpy( pszSearchName, pszPath );
    strcat( pszSearchName, BACKSLASH_STR );
    strcat( pszSearchName, PROPDIR );
    strcat( pszSearchName, "\\*" );
    strcat( pszSearchName, EXT_OF_SUBFOLDER );
  }

  usRC = UtlFindFirst( pszSearchName, &hDirHandle, FILE_NORMAL, &stResultBuf, sizeof( stResultBuf), &usCount, 0L, 0);
  while ( (usRC == NO_ERROR) && usCount )
  {
    ULONG ulLen;
    BOOL  isSubFolder = TRUE;

    // ignore documents having the extention of a subfolder (in this case
    // there is a file having the same name in the SOURCE directory of the
    // folder)
    sprintf( pszFileName, "%s\\%s\\%s", pszPath, SOURCEDIR, pszName );
    isSubFolder = !UtlFileExist( pszFileName );

    // load subfolder properties
    if ( isSubFolder)
    {
      sprintf( pszFileName, "%s\\%s\\%s", pszPath, PROPDIR, pszName );
      if ( UtlLoadFileL( pszFileName, (PVOID *)&pProp, &ulLen, FALSE, FALSE ) )
      {
        if ( !fAllSubFolders && (pProp->ulParentFolder != ulParentID) )
        {
          // ignore subfolder as it is not located in the parent folder
        }
        else if ( (lFlag & LOADSUBFOLNAMES_ITEMTEXT) && (hwndLB != HWND_FUNCIF) )
        {
          if ( pProp->szLongMemory[0] != EOS )
            pszMemory = pProp->szLongMemory;
          else if ( pProp->szMemory[0] != EOS )
            pszMemory = pProp->szMemory;
          else
            pszMemory = szMemory;
          pszFormat     = (pProp->szFormat[0] != EOS)     ? pProp->szFormat : szFormat;
          pszSourceLang = (pProp->szSourceLang[0] != EOS) ? pProp->szSourceLang : szSourceLang;
          pszTargetLang = (pProp->szTargetLang[0] != EOS) ? pProp->szTargetLang : szTargetLang;
          pszEditor     = (pProp->szEditor[0] != EOS)     ? pProp->szEditor : szEditor;

          OEMTOANSI( pProp->szLongName );
          sprintf( pszBuffer,
                  "%s\x15%s\x15%s\x15%lu\x15%lu\x15%lu\x15%lu\x15%lu\x15%lu\x15%s\x15%lu\x15%d\x15%d\x15%d\x15%s\x15%s\x15%s\x15%s\x15%s\x15%s\x15%lu\x15",
                  /* ObjName     */  pszFileName,
                  /* Name        */  pProp->szLongName,
                  /* Markup      */  pszFormat,
                  /* Translated  */  0L,
                  /* Analyzed    */  0L,
                  /* Exported    */  0L,
                  /* Imported    */  0L,
                  /* Updated     */  0L,
                  /* Updated/Time*/  0L,
                  /* Size        */  "",   // indicator for subfolders!!!
                  /* Complete    */  0L,
                  /* Mod Segs    */  0L,
                  /* New Segs    */  0L,
                  /* Cop. Segs   */  0L,
                  /* Doc format  */  pszFormat,
                  /* TM          */  pszMemory,
                  /* SourceLng   */  pszSourceLang,
                  /* TargetLng   */  pszTargetLang,
                  /* Editor      */  pszEditor,
                  /* Alias       */  "",
                  /* Sourcedate  */  0L);  // added bt 22062001
          INSERTITEMHWND( hwndLB, pszBuffer );
          usDocs++;
        }
        else if ( (lFlag & LOADSUBFOLNAMES_PARENTINFO) && (hwndLB != HWND_FUNCIF) )
        {
          OEMTOANSI( pProp->szLongName );
          sprintf( pszBuffer, "%s\x15%s\x15%lu\x15%lu",
                  /* ObjName     */  pszFileName,
                  /* Name        */  pProp->szLongName,
                  /* Own ID      */  pProp->ulSubFolderID,
                  /* Parent ID   */  pProp->ulParentFolder );
          INSERTITEMHWND( hwndLB, pszBuffer );
          usDocs++;
        }
        else
        {
          if ( hwndLB != HWND_FUNCIF )
          {
            OEMTOANSI( pProp->szLongName );
            INSERTITEMHWND( hwndLB, pProp->szLongName );
          }
          else
          {
            // add subfolder name to document name buffer
            LONG lAddLen = strlen(pProp->szLongName ) + 1;
            if ( lBufferSize < (lBufferUsed + lAddLen) )
            {
              UtlAllocHwnd( (PVOID *)&pDocNameBuffer, lBufferSize,
                            lBufferSize + 8096L, ERROR_STORAGE, HWND_FUNCIF );
              lBufferSize += 8096L;
            } /* endif */

            if ( pDocNameBuffer != NULL )
            {
              strcpy( pDocNameBuffer + lBufferUsed, pProp->szLongName );
              lBufferUsed += lAddLen;
            } /* endif */
          } /* endif */
          usDocs++;
        } /* endif */
        UtlAlloc( (PVOID *)&pProp, 0L, 0L, NOMSG );
        pProp = NULL;
      } /* endif */
    } /* endif */


    usRC = UtlFindNext( hDirHandle, &stResultBuf, sizeof( stResultBuf),
                        &usCount, 0);
  } /* endwhile */

  // close search file handle
  if ( hDirHandle != HDIR_CREATE ) UtlFindClose( hDirHandle, FALSE );

  UtlAlloc( (PVOID *)&pszPath, 0L, 0L, NOMSG );

  if ( hwndLB != HWND_FUNCIF )
  {
    ENABLEUPDATEHWND_TRUE( hwndLB );
    SETCURSOR( SPTR_ARROW );
  }
  else
  {
    if ( pDocNameBuffer == NULL )
    {
      UtlAllocHwnd( (PVOID *)&pDocNameBuffer, 0L, 8096L, ERROR_STORAGE, HWND_FUNCIF );
    } /* endif */
    if ( pDocNameBuffer != NULL )
    {
      // terminate document buffer and return pointer to it
      // it is up to the caller to free the document buffer
      *(pDocNameBuffer + lBufferUsed) = EOS;
    } /* endif */
    *((PSZ *)pszBuffer) = pDocNameBuffer;
  } /* endif */

  return( usDocs );
}

// private subfolder delete data area
typedef struct _SUBFOLDERDELETEDATA
{
  CHAR        szSearchPath[MAX_LONGPATH];        // search path
  CHAR        szPathBuffer[MAX_LONGPATH];        // path name buffer
  CHAR        szFullName[MAX_LONGPATH];          // full subfolder name
  FILEFINDBUF stResultBuf;                       // FindFirst/FindNext buffer
} SUBFOLDERDELETEDATA, *PSUBFOLDERDELETEDATA;

// delete a subfolder and all subfolders and documents it contains
BOOL SubFolderDelete( PSZ pszSubFolderObjName, BOOL fBatch )
{
  BOOL        fOK = TRUE;
  ULONG       ulCurrentID;
  PSUBFOLDERDELETEDATA pData = NULL;

  // get subfolder ID
  ulCurrentID = FolGetSubFolderIdFromObjName( pszSubFolderObjName );

  // allocate private data area
  fOK = UtlAllocHwnd( (PVOID *)&pData, 0L, sizeof(SUBFOLDERDELETEDATA), ERROR_STORAGE,
                      fBatch ? HWND_FUNCIF : NULLHANDLE );

  // get full subfolder name
  if ( fOK )
  {
    SubFolObjectNameToName( pszSubFolderObjName, pData->szFullName );
  } /* endif */

  // build main folder property dir search path
  if ( fOK )
  {
    PSZ pszName;
    strcpy( pData->szSearchPath, pszSubFolderObjName );
    pszName = strrchr( pData->szSearchPath, BACKSLASH );
    if ( pszName )
    {
      strcpy( pszName+1, DEFAULT_PATTERN );
    }
    else
    {
      fOK = FALSE;                     // seems to be an internal error
    } /* endif */
  } /* endif */

  // loop over all files in main folder property directory
  if ( fOK )
  {
    HDIR hdir = HDIR_CREATE;
    USHORT usRC = NO_ERROR;
    USHORT usCount = 1;

    usRC = UtlFindFirst( pData->szSearchPath, &hdir, FILE_NORMAL,
                         &pData->stResultBuf, sizeof(pData->stResultBuf), &usCount, 0L, 0);
    while ( (usRC == NO_ERROR) && usCount )
    {
      // process found property file but ignore any history log files
      if ( strcmp( RESBUFNAME(pData->stResultBuf), HISTLOGFILE ) != 0 )
      {
        PPROPDOCUMENT pProp = NULL;
        ULONG ulLen = 0;

        // build property file path
        strcpy( pData->szPathBuffer, pData->szSearchPath );
        UtlSplitFnameFromPath( pData->szPathBuffer );
        strcat( pData->szPathBuffer, BACKSLASH_STR );
        strcat( pData->szPathBuffer, RESBUFNAME(pData->stResultBuf) );

        // handle subfolder or document
        // (the property head part of an subfolder property file is intentionally
        // left empty)
        if ( UtlLoadFileL( pData->szPathBuffer, (PVOID *)&pProp, &ulLen, FALSE, FALSE ) )
        {
          if ( pProp->PropHead.szName[0] == EOS )
          {
            // delete subfolder if it belongs to current subfolder
            PPROPFOLDER pFolProp = (PPROPFOLDER)pProp;
            if ( pFolProp->ulParentFolder == ulCurrentID )
            {
              // recursively call subfolder delete function
              fOK = SubFolderDelete( pData->szPathBuffer, fBatch );
            } /* endif */
          }
          else
          {
            // delete document if it belongs to current subfolder
            if ( pProp->ulParentFolder == ulCurrentID )
            {
              USHORT usMBReturn;

              // build document object name
              strcpy( pData->szPathBuffer, pData->szSearchPath );
              UtlSplitFnameFromPath( pData->szPathBuffer );
              UtlSplitFnameFromPath( pData->szPathBuffer );
              strcat( pData->szPathBuffer, BACKSLASH_STR );
              strcat( pData->szPathBuffer, RESBUFNAME(pData->stResultBuf) );

              // call document delete function
              DocumentDelete( pData->szPathBuffer, FALSE, &usMBReturn );
            } /* endif */
          } /* endif */
          UtlAlloc( (PVOID *)&pProp, 0L, 0L, NOMSG );
        } /* endif */
      } /* endif */

      // get next object
      usRC = UtlFindNext( hdir, &pData->stResultBuf, sizeof(pData->stResultBuf), &usCount, FALSE );
    } /* endwhile */
    if ( hdir != HDIR_CREATE ) UtlFindClose( hdir, FALSE );
  } /* endif */


  // delete subfolder property file
  if ( fOK )
  {
    fOK = (UtlDeleteHwnd( pszSubFolderObjName, 0L, TRUE,
                          fBatch ? HWND_FUNCIF : NULLHANDLE ) == NO_ERROR);
  } /* endif */

  // broadcast delete message
  if ( fOK && !fBatch )
  {
    EqfSend2AllHandlers( WM_EQFN_DELETED,
                         MP1FROMSHORT( clsDOCUMENT ),
                         MP2FROMP( pszSubFolderObjName ));
    OEMTOANSI( pData->szFullName );
    EqfSend2AllHandlers( WM_EQFN_DELETEDNAME,
                         MP1FROMSHORT( clsDOCUMENT ),
                         MP2FROMP( pData->szFullName ));
  } /* endif */

  // cleanup
  if ( pData ) UtlAlloc( (PVOID *)&pData, 0L, 0L, NOMSG );

  return( fOK );
} /* end of function SubFolderDelete */

// check if an item of a document list is a subfolder
BOOL FolIsSubFolderItem( PSZ pszItem )
{
  // subfolders have an empty string as size value whereas documents with a zero size
  // will have an "0" as size
  BOOL fSubFolder = FALSE;
  PSZ pszSize  = UtlParseX15( pszItem, FOL_SIZE_IND );
  if ( pszSize[0] == EOS ) fSubFolder = TRUE;
  pszSize[strlen(pszSize)] = X15;
  return( fSubFolder );
} /* end of function FolIsSubFolderItem */


// checks if the given subfolder is contained in the parent folder
BOOL IsSubFolContainedInParent( ULONG ulParent, ULONG ulChild, PSUBFOLINFO pInfo )
{
  BOOL fResult = FALSE;
  BOOL fDone = FALSE;

  if ( ulParent == 0L)
  {
    // all folders are automatically part of the main folder
    fResult = TRUE;
  }
  else
  {
    // follow subfolder parentship chain looking for parent folder
    do
    {
      PSUBFOLINFO pSubFolderInfo = pInfo;
      BOOL fFound = FALSE;

      // look for child subfolder in subfolder info table
      while ( !fFound && (pSubFolderInfo->szName[0] != EOS) )
      {
        if ( pSubFolderInfo->ulID == ulChild )
        {
          fFound = TRUE;
        }
        else
        {
          // try next entry
          pSubFolderInfo++;
        } /* endif */
      } /* endwhile */

      // if found use parent as new child ID if not already parent being looked for
      if ( fFound )
      {
        if ( pSubFolderInfo->ulParentFolder == ulParent )
        {
          // ok, we are done
          fDone = TRUE;
          fResult = TRUE;
        }
        else if ( pSubFolderInfo->ulParentFolder == 0L )
        {
          // we are at the end of the parentship chain, the subfolder is not
          // contained in the given parent folder
          fDone = TRUE;
          fResult = FALSE;
        }
        else
        {
          // continue using the parent folder as child ID
          ulChild = pSubFolderInfo->ulParentFolder;
        } /* endif */
      }
      else
      {
        // somthing went wrong, subfolder is not contained in our list
        fDone = TRUE;
        fResult = FALSE;
      } /* endif */
    } while ( !fDone );
  } /* endif */
  return( fResult );

} /* end of function IsSubFolContainedInParent */


// delete an internal folder without checking and long name handling
USHORT FolDeleteIntFolder
(
PSZ         pszFolderName            // name of folder being deleted
)
{
  BOOL             fOK = TRUE;         // internal O.K. flag
  USHORT           usRC = NO_ERROR;    // function return code
  OBJNAME          szFolObjName;       // buffer for main folder object name

  // create main folder object name
  if ( fOK )
  {
    HPROP      hFolProp;                     // folder properties handle
    EQFINFO    ErrorInfo;                    // return code from prop. handler

    // setup folder object name using the primary EQF drive
    UtlMakeEQFPath( szFolObjName, SYSTEM_PATH, NULC, NULL );
    strcat( szFolObjName, BACKSLASH_STR );
    strcat( szFolObjName, pszFolderName );
    strcat( szFolObjName, EXT_FOLDER_MAIN );

    // access folder properties to get the correct folder drive
    hFolProp = OpenProperties( szFolObjName, NULL,
                               PROP_ACCESS_READ, &ErrorInfo);
    if ( hFolProp )
    {
      PPROPFOLDER pProp;               // ptr to folder properties

      pProp = (PPROPFOLDER)MakePropPtrFromHnd( hFolProp );
      szFolObjName[0] = pProp->chDrive;
      CloseProperties( hFolProp, PROP_QUIT, &ErrorInfo );
    }
    else
    {
      fOK = FALSE;
    } /* endif */
  } /* endif */

  // delete the folder
  if ( fOK )
  {
    DeleteFolder( szFolObjName, HWND_FUNCIF, NULL );
  } /* endif */

  return( usRC );
} /* end of function FolDeleteIntFolder */


// API function to remove a group of documents specified in a text file
USHORT FolFuncRemoveDocs
(
  PSZ         pszFolderName,           // name of the folder
  PSZ         pszListFile              // name of list file 
)
{
  USHORT usRC = NO_ERROR;
  OBJNAME szFolderObj;
  BOOL fIsNew = FALSE;

    // check input data
  if ( !usRC )
  {
    if ( (pszFolderName == NULL) || (*pszFolderName == EOS) || (pszListFile == NULL) || (*pszListFile == EOS) )
    {
      return( WRONG_OPTIONS_RC );
    } /* endif */
  } /* endif */

  // setup folder object name
  UtlMakeEQFPath( szFolderObj, NULC, SYSTEM_PATH, NULL );
  strcat( szFolderObj, BACKSLASH_STR );
  ObjLongToShortName( pszFolderName, szFolderObj+strlen(szFolderObj), FOLDER_OBJECT, &fIsNew );
  strcat( szFolderObj, EXT_FOLDER_MAIN );
  if ( fIsNew )
  {
    usRC = ERROR_XLATE_FOLDER_NOT_EXIST; 
    UtlErrorHwnd( usRC, MB_CANCEL, 1, &pszFolderName, EQF_ERROR, HWND_FUNCIF );
    return( usRC );
  } /* endif */     

  // call internal function 
  usRC = FolIntRemoveDocs( szFolderObj, pszListFile, NULL, HWND_FUNCIF );

  return( usRC );
} /* end of function FolFuncRemoveDocs */


// list of sub-directory string IDs for directories containing document files
static int iPathID[] = { QST_PROPDIR, QST_SOURCEDIR, QST_SEGSOURCEDIR, QST_TARGETDIR, QST_SEGTARGETDIR,
                  QST_DIRSEGRTFDIR, QST_EADATAPATH, QST_MTLOGPATH, QST_MISCPATH, QST_ENTITY, QST_XLIFFPATH, 
                  QST_METADATAPATH, -1 /* end of array identifier */ };

// data area for FolIntRemoveDocs function
typedef struct _FOLINTREMOVEDOCSDATA
{
  CHAR        szLine[1024];            // buffer for line input
  CHAR        szShortName[MAX_FILESPEC];// buffer for document short name
  CHAR        szCurSubDir[MAX_FILESPEC];// buffer for current sub-directory
  CHAR        szSource[MAX_LONGFILESPEC];// buffer for source file name
  CHAR        szTarget[MAX_LONGFILESPEC];// buffer for target file name
  CHAR        szRemovedSubDir[MAX_FILESPEC]; // buffer for name of directory for removed documents
} FOLINTREMOVEDOCSDATA, *PFOLINTREMOVEDOCSDATA;

// internal function to remove a group of documents specified in a text file
USHORT FolIntRemoveDocs
(
  PSZ         pszFolObjName,           // folder object name
  PSZ         pszListFile,             // name of list file 
  int         *piNumOfDocs,            // callers buffer for the number of removed documents
  HWND        hwndErrMsg
)
{
  USHORT usRC = NO_ERROR;
  FILE *hInFile = NULL;                // file handle of list file
  PFOLINTREMOVEDOCSDATA pData = NULL;  // our data area

  // initialize callers document counter
  if ( piNumOfDocs != NULL ) *piNumOfDocs = 0;

  // allocate our data area
  if ( !UtlAllocHwnd( (PVOID *)&pData, 0L, sizeof(FOLINTREMOVEDOCSDATA), ERROR_STORAGE, hwndErrMsg ) )
  {
    return( ERROR_NOT_ENOUGH_MEMORY );
  }
  else
  {
    UtlQueryString( QST_REMOVEDDOCDIR, pData->szRemovedSubDir, sizeof(pData->szRemovedSubDir) );
  } /* endif */     

  // open file containing document names
  hInFile = fopen( pszListFile, "r" );
  if ( hInFile == NULL )
  {
    usRC = ERROR_FILE_OPEN_FAILED;
    UtlErrorHwnd( usRC, MB_CANCEL, 1, &pszListFile, EQF_ERROR, hwndErrMsg );
    return( usRC );
  } /* end */     

  // process lines until complete
  while ( !usRC && (fgets( pData->szLine, sizeof(pData->szLine), hInFile ) != NULL) )
  {
    // process document name
    int iLen = strlen(pData->szLine);
    if ( (iLen >= 1) && (pData->szLine[iLen-1] == '\n') ) pData->szLine[iLen-1] = EOS;
    UtlStripBlanks( pData->szLine );
    if ( (pData->szLine[0] != EOS) && (pData->szLine[0] != '*') && (pData->szLine [0] != '#')  )
    {
      BOOL fIsNew = FALSE;

      // convert to short document name
      FolLongToShortDocName( pszFolObjName, pData->szLine, pData->szShortName, &fIsNew );
      if ( fIsNew )
      {
        PSZ pszParm = pData->szLine;
        usRC = ERROR__DELDOC_INVALID_DOC;
        UtlErrorHwnd( usRC, MB_CANCEL, 1, &pszParm, EQF_ERROR, hwndErrMsg );
      } /* end */         

      // process document
      if ( !usRC )
      {
        // list of sub-directories with document files
        int iPathID[] = { QST_PROPDIR, QST_SOURCEDIR, QST_SEGSOURCEDIR, QST_TARGETDIR, QST_SEGTARGETDIR,
                          QST_DIRSEGRTFDIR, QST_EADATAPATH, QST_MTLOGPATH, QST_MISCPATH, QST_ENTITY, QST_XLIFFPATH, 
                          QST_METADATAPATH, -1 };

        int i = 0;

        // move all files of the document
        while ( !usRC && iPathID[i] >= 0 )
        {
          // get sub-directory name
          UtlQueryString( (SHORT)iPathID[i], pData->szCurSubDir, sizeof(pData->szCurSubDir) );

          // setup source path 
          sprintf( pData->szSource, "%s\\%s\\%s", pszFolObjName, pData->szCurSubDir, pData->szShortName  );

          // handle the file
          if ( UtlFileExist(pData->szSource)  )
          {
            // setup target directory name
            sprintf( pData->szTarget, "%s\\%s\\%s", pszFolObjName, pData->szRemovedSubDir, pData->szCurSubDir );

            // create directories when required
            UtlMkMultDir( pData->szTarget, FALSE );

            // complete target file name
            sprintf( pData->szTarget + strlen(pData->szTarget), "\\%s", pData->szShortName   );

            // move the file
            usRC = UtlMoveHwnd( pData->szSource, pData->szTarget, 0L, TRUE, hwndErrMsg );
          } /* endif */             

          // next path ID
          i++;
        } /* endwhile */           

        // send a document delete notification
        if ( !usRC )
        {
          sprintf( pData->szSource, "%s\\%s", pszFolObjName, pData->szShortName  );
          if ( hwndErrMsg != HWND_FUNCIF )
          {
            EqfSend2AllHandlers( WM_EQFN_DELETED, MP1FROMSHORT( clsDOCUMENT ), MP2FROMP( pData->szSource ));
          }
          else
          {
            ObjBroadcast( WM_EQFN_DELETED, clsDOCUMENT, pData->szSource );
          } /* end */             

          if ( piNumOfDocs != NULL ) *piNumOfDocs = *piNumOfDocs + 1;
        } /* end */           
      } /* end */         
    } /* end */       
  } /* endwhile */     

  // close file
  fclose( hInFile );

  // cleanup
  if ( pData != NULL ) UtlAlloc( (PVOID *)&pData, 0, 0, NOMSG );

  return( usRC );
} /* end of function FolIntRemoveDocs */



// API function to restore documents removed using FolFuncRemoveDocs
USHORT FolFuncRestoreDocs
(
  PSZ         pszFolderName            // name of the folder
)
{
  USHORT usRC = NO_ERROR;
  OBJNAME szFolderObj;
  BOOL fIsNew = FALSE;

    // check input data
  if ( !usRC )
  {
    if ( (pszFolderName == NULL) || (*pszFolderName == EOS)  )
    {
      return( WRONG_OPTIONS_RC );
    } /* endif */
  } /* endif */

  // setup folder object name
  UtlMakeEQFPath( szFolderObj, NULC, SYSTEM_PATH, NULL );
  strcat( szFolderObj, BACKSLASH_STR );
  ObjLongToShortName( pszFolderName, szFolderObj+strlen(szFolderObj), FOLDER_OBJECT, &fIsNew );
  strcat( szFolderObj, EXT_FOLDER_MAIN );
  if ( fIsNew )
  {
    usRC = ERROR_XLATE_FOLDER_NOT_EXIST; 
    UtlErrorHwnd( usRC, MB_CANCEL, 1, &pszFolderName, EQF_ERROR, HWND_FUNCIF );
    return( usRC );
  } /* endif */     

  // call internal function 
  usRC = FolIntRestoreDocs( szFolderObj, NULL, HWND_FUNCIF );

  return( usRC );
} /* end of function FolFuncRestoreDocs */

// data area for FolIntRemoveDocs function
typedef struct _FOLINTRESTOREDOCSDATA
{
  CHAR        szShortName[MAX_FILESPEC];// buffer for document short name
  CHAR        szCurSubDir[MAX_FILESPEC];// buffer for current sub-directory
  CHAR        szSource[MAX_LONGFILESPEC];// buffer for source file name
  CHAR        szTarget[MAX_LONGFILESPEC];// buffer for target file name
  CHAR        szRemovedSubDir[MAX_FILESPEC]; // buffer for name of directory for removed documents
  CHAR        szSearch[MAX_LONGFILESPEC];// buffer for search file pattern
  WIN32_FIND_DATA FindData;              // buffer for file info   
} FOLINTRESTOREDOCSDATA, *PFOLINTRESTOREDOCSDATA;

// internal function to restore documents removed using FolIntRemoveDocs
USHORT FolIntRestoreDocs
(
  PSZ         pszFolObjName,           // folder object name
  int         *piNumOfDocs,            // callers buffer for the number of restored documents
  HWND        hwndErrMsg
)
{
  USHORT usRC = NO_ERROR;
  PFOLINTRESTOREDOCSDATA pData = NULL;  // our data area
  HANDLE hDir;

  // initialize callers document counter
  if ( piNumOfDocs != NULL ) *piNumOfDocs = 0;

  // allocate our data area
  if ( !UtlAllocHwnd( (PVOID *)&pData, 0L, sizeof(FOLINTRESTOREDOCSDATA), ERROR_STORAGE, hwndErrMsg ) )
  {
    return( ERROR_NOT_ENOUGH_MEMORY );
  }
  else
  {
    UtlQueryString( QST_REMOVEDDOCDIR, pData->szRemovedSubDir, sizeof(pData->szRemovedSubDir) );
  } /* endif */     

  // loop over all document property files in directory for removed documents
  sprintf( pData->szSearch, "%s\\%s\\", pszFolObjName, pData->szRemovedSubDir );
  UtlQueryString( QST_PROPDIR, pData->szSearch+strlen(pData->szSearch), MAX_FILESPEC );
  strcat( pData->szSearch, "\\*.*" );
  hDir = FindFirstFile( pData->szSearch, &pData->FindData );
  if ( hDir != INVALID_HANDLE_VALUE )
  {
    do
    {
      if ( !(pData->FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) )
      {
        int i = 0;

        // move all files of the document
        while ( !usRC && iPathID[i] >= 0 )
        {
          // get sub-directory name
          UtlQueryString( (SHORT)iPathID[i], pData->szCurSubDir, sizeof(pData->szCurSubDir) );

          // setup source path 
          sprintf( pData->szSource, "%s\\%s\\%s\\%s", pszFolObjName, pData->szRemovedSubDir, pData->szCurSubDir, pData->FindData.cFileName );

          // handle the file
          if ( UtlFileExist(pData->szSource)  )
          {
            // setup target directory name
            sprintf( pData->szTarget, "%s\\%s", pszFolObjName, pData->szCurSubDir);

            // create directories when required
            UtlMkMultDir( pData->szTarget, FALSE );

            // complete target file name
            sprintf( pData->szTarget + strlen(pData->szTarget), "\\%s", pData->FindData.cFileName );

            // move the file
            usRC = UtlMoveHwnd( pData->szSource, pData->szTarget, 0L, TRUE, hwndErrMsg );
          } /* endif */             

          // next path ID
          i++;
        } /* endwhile */           

        // send a document created notification
        if ( !usRC )
        {
          sprintf( pData->szSource, "%s\\%s", pszFolObjName, pData->FindData.cFileName );
          if ( hwndErrMsg != HWND_FUNCIF )
            {
              EqfSend2AllHandlers( WM_EQFN_CREATED, MP1FROMSHORT( clsDOCUMENT ), MP2FROMP( pData->szSource ));
            }
            else
            {
              ObjBroadcast( WM_EQFN_CREATED, clsDOCUMENT, pData->szSource );
            } /* end */             
          if ( piNumOfDocs != NULL ) *piNumOfDocs = *piNumOfDocs + 1;
        } /* end */           
      } /* end */       
    } while ( FindNextFile( hDir, &pData->FindData  ) );
    FindClose( hDir );

    // remove complete directory
    sprintf( pData->szSearch, "%s\\%s", pszFolObjName, pData->szRemovedSubDir );
    UtlRemoveDir( pData->szSearch, FALSE );

  } /* endif */

  // cleanup
  if ( pData != NULL ) UtlAlloc( (PVOID *)&pData, 0, 0, NOMSG );

  return( usRC );
} /* end of function FolIntRestoreDocs */


// API function to associate a Global memory CTID options file to a folder
USHORT FolFuncAddCTIDList
(
  PSZ         pszFolderName,           // name of the folder
  PSZ         pszCTIDListFile          // fully qualified name of the CTID list file
)
{
  USHORT usRC = NO_ERROR;
  OBJNAME szFolderObj;
  CHAR   szShortName[MAX_FILESPEC];
  BOOL fIsNew = FALSE;

    // check input data
  if ( !usRC )
  {
    if ( (pszFolderName == NULL) || (*pszFolderName == EOS) || (pszCTIDListFile == NULL) || (*pszCTIDListFile == EOS)  )
    {
      return( WRONG_OPTIONS_RC );
    } /* endif */
  } /* endif */

  // setup folder object name
  UtlMakeEQFPath( szFolderObj, NULC, SYSTEM_PATH, NULL );
  strcat( szFolderObj, BACKSLASH_STR );
  ObjLongToShortName( pszFolderName, szShortName, FOLDER_OBJECT, &fIsNew );
  strcat( szShortName, EXT_FOLDER_MAIN );
  strcat( szFolderObj, szShortName );
  if ( fIsNew )
  {
    usRC = ERROR_XLATE_FOLDER_NOT_EXIST; 
    UtlErrorHwnd( usRC, MB_CANCEL, 1, &pszFolderName, EQF_ERROR, HWND_FUNCIF );
    return( usRC );
  } /* endif */     

  // check existence of CTIDList file
  if ( !UtlFileExist( pszCTIDListFile ) )
  {
    usRC = FILE_NOT_EXISTS; 
    UtlErrorHwnd( usRC, MB_CANCEL, 1, &pszCTIDListFile, EQF_ERROR, HWND_FUNCIF );
    return( usRC );
  } /* endif */     

  // copy CTID list file to folder directory
  {
    PSZ pszObjNameEnd = szFolderObj + strlen(szFolderObj);
    strcat( szFolderObj, BACKSLASH_STR );
    strcat( szFolderObj, UtlGetFnameFromPath( pszCTIDListFile ) );
    if ( UtlFileExist( szFolderObj ) ) UtlDelete( szFolderObj, 0, FALSE );
    UtlCopy( pszCTIDListFile, szFolderObj, 1, 0L, FALSE );
    *pszObjNameEnd = EOS;
  }

  // adjust folder properties
  {
    ULONG ulLen = 0;
    PPROPFOLDER pProp = NULL;

    // load property file
    UtlMakeEQFPath( szFolderObj, NULC, PROPERTY_PATH, NULL );
    strcat( szFolderObj, BACKSLASH_STR );
    strcat( szFolderObj, szShortName );
    if ( !UtlLoadFileL( szFolderObj, (PVOID *)&pProp, &ulLen, FALSE, FALSE ) )
    {
      usRC = FOL_FOLDER_ACCESS_ERROR; 
      UtlErrorHwnd( usRC, MB_CANCEL, 1, &pszFolderName, EQF_ERROR, HWND_FUNCIF );
      return( usRC );
    } /* endif */       

    strcpy( pProp->szGlobalMemOptFile, UtlGetFnameFromPath( pszCTIDListFile ) );
    pProp->fGlobalMemOptCheckRequired = TRUE;

    UtlWriteFileL( szFolderObj, ulLen, pProp, FALSE );
    UtlAlloc( (PVOID *)&pProp, 0L, 0L, NOMSG );
  }

  return( usRC );
} /* end of function FolFuncAddCTIDList */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     ResolveLongMultFileNames                                 |
//+----------------------------------------------------------------------------+
//|Function call:     fOK = ResolveLongMultFileNames( pInFile, &ppOutFiles );  |
//+----------------------------------------------------------------------------+
//|Description:       This function will resolve any single or multiple subst. |
//|                   symbols and returns a list of longfilenames.             |
//|                   If no single or multiple substitution is nec. NO output  |
//|                   file list will be allocated                              |
//+----------------------------------------------------------------------------+
//|Parameters:        PSZ   pInFile     input file name                        |
//|                   PSZ  *ppOutFiles  output file list                       |
//+----------------------------------------------------------------------------+
//|Returncode type:   BOOL                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:       FALSE    problems in allocating memory                   |
//|                   TRUE     everything okay                                 |
//+----------------------------------------------------------------------------+
//|Function flow:     check if single or multiple substitution available       |
//|                   allocate pool for filename strings                       |
//|                   if okay                                                  |
//|                     call UtlFindFirst with *.* to setup the loop           |
//|                     while files found                                      |
//|                       get short/dummy filenames out of                     |
//|                         SOURCE or TARGET directory                         |
//|                       get corresponding long filename                      |
//|                         out of PROPERTIES                                  |
//|                       if  long filename fullfills search criterium         |
//|                          store the filename; increase pool if necessary    |
//|                       end if                                               |
//|                       call UtlFindNext with *.*                            |
//|                     wend                                                   |
//|                     set ppOutFile pointer                                  |
//|                   endif                                                    |
//|                   return success indicator                                 |
//+----------------------------------------------------------------------------+
BOOL ResolveLongMultFileNames
(
  PSZ  pInFile,                        // input file name
  PSZ  pSearchName,                    // searched file name
  PSZ  *ppOutFiles,                    // pointer to list array
  ULONG ulParentFolder                 // document parent folder
)
{
  BOOL   fOK = TRUE;
  BOOL   fMult = FALSE;                // multiple or single substitution
  PSZ    pFileName;                    // pointer to FileName
  CHAR   c;                            // active character
  PCHAR  pStrPool = NULL;              // string pool
  PCHAR  pStartPool;                   // string pool start
  HDIR   hDir;

  PSZ    pBuffer;                      // pointer to file name
  PSZ    pszSearchCriterium = NULL;    // search criterium for
                                       // multi-file-names
  PSZ    pszPropertyPath = NULL;       // Full Pathname of File-Property
  PSZ    pszLongFileName = NULL;       // Long file name out of properties
  PSZ    pszShortFileName = NULL;      // Short file name

  FILEFINDBUF   stResultBuf;

  USHORT usCount;
  ULONG  ulLen, ulLeft;                // length of strings
  ULONG  ulPathLen;                    // path length of mult/single subst.
  ULONG  ulAllocLen;                   // allocated length
  BOOL   fMatch;                       // matching strings flag

  int    nFiles=0;                     // number of files with matches

  PSUBFOLINFO pSubFolInfo = NULL;      // subfolder information table


   // alloc memory for different file names in use
   fOK=UtlAlloc((PVOID *) &pszSearchCriterium,0L,
                (LONG) MAX_LONGPATH,ERROR_STORAGE);
   if (fOK) fOK=UtlAlloc((PVOID *) &pszPropertyPath,0L,
                (LONG) MAX_LONGPATH,ERROR_STORAGE);
   if (fOK) fOK=UtlAlloc((PVOID *) &pszLongFileName,0L,
                (LONG) MAX_LONGPATH,ERROR_STORAGE);
   if (fOK) fOK=UtlAlloc((PVOID *) &pszShortFileName,0L,
                (LONG) MAX_LONGPATH,ERROR_STORAGE);



  /********************************************************************/
  /* check if some of the list files are not explicitly specified,    */
  /* i.e. if they contain single or multiple substitution characters  */
  /* if so allocate new space and use UtlFindFirst and UtlFindNext    */
  /* to resolve any multiple file names, first get the short/dummy    */
  /* filename, then the corresponding long name                       */
  /********************************************************************/

  pFileName = pSearchName;
  while ( ((c = *pFileName++)!= NULC) && !fMult )
  {
    fMult = (c == MULTIPLE_SUBSTITUTION) || (c == SINGLE_SUBSTITUTION);
  } /* endwhile */

  if ( fMult && fOK )
  {
    // get folder info table for current folder if ulParenFolder specified
    if ( fOK && ulParentFolder )
    {
       strcpy(pszPropertyPath,pInFile);
       pBuffer=UtlSplitFnameFromPath(pszPropertyPath);
       pBuffer=UtlSplitFnameFromPath(pszPropertyPath);
       SubFolCreateInfoTable( pszPropertyPath, &pSubFolInfo );
    } /* endif */

     // check for the actual search criterium
     pBuffer=UtlGetFnameFromPath(pInFile);
     strcpy(pszSearchCriterium,pSearchName);

     // remove search criterium; insert *.*
     pBuffer=UtlSplitFnameFromPath(pInFile);
     if (*pInFile)
     {
        strcat(pInFile,BACKSLASH_STR);
        strcat(pInFile,SEARCH_ALL_STR);
     }
     else
     {
        strcpy(pInFile,SEARCH_ALL_STR);
     }

    /******************************************************************/
    /* resolve it, i.e.                                               */
    /*   allocate memory                                              */
    /*   loop thru all files in folder                                */
    /*     fetch longfilename, compare with search criterium          */
    /*     either copy entries without multi/single substitutions     */
    /*     or run resolution of single/multiple                       */
    /*   endwhile                                                     */
    /******************************************************************/
    fOK = UtlAlloc( (PVOID *) &pStartPool, 0L, (LONG) MAX_POOL_SIZE, ERROR_STORAGE );

    if ( fOK )
    {
      ulAllocLen = ulLeft = MAX_POOL_SIZE;
      pStrPool = pStartPool;
      /**************************************************************/
      /* Search for filenames                                       */
      /**************************************************************/
      usCount = 1;
      hDir = HDIR_CREATE;


      if ( UtlFindFirst( pInFile, &hDir, FILE_NORMAL,
                    &stResultBuf, sizeof( stResultBuf), &usCount, 0L, 0) )
      {
        usCount = 0;  // no files as return code is set
      } /* endif */

      /************************************************************/
      /* extract the path info (if any )...                       */
      /************************************************************/
      pFileName = UtlGetFnameFromPath( pInFile );
      if ( pFileName )
      {
        *pFileName = EOS;            // extract the path info
        ulPathLen = strlen( pInFile );
      }
      else
      {
        pFileName = pInFile;
        *pFileName = EOS;
        ulPathLen = 0;
      } /* endif */

      /************************************************************/
      /* Loop all files in the folder                             */
      /* then check matches                                       */
      /************************************************************/
      while ( usCount && fOK )
      {
        ULONG ulParentID;            // ID of documents parent folder

        /************************************************************/
        /* Get Property Name  and                                   */
        /* Long Filename out of properties                          */
        /* Check long filename for search criterium                 */
        /************************************************************/

        // get all short resp. shrinked filenames
        strcpy(pszShortFileName, RESBUFNAME(stResultBuf));

        // construct property path
        strcpy(pszPropertyPath,pInFile);
        pBuffer=UtlSplitFnameFromPath(pszPropertyPath);
        pBuffer=UtlSplitFnameFromPath(pszPropertyPath);

        if(*pszPropertyPath)
        {
          strcat(pszPropertyPath,BACKSLASH_STR);
          UtlQueryString( QST_PROPDIR, pszPropertyPath + strlen(pszPropertyPath), 20 );
          strcat(pszPropertyPath,BACKSLASH_STR);
          strcat(pszPropertyPath,pszShortFileName);
        }
        else
        {
          strcat(pszPropertyPath,pszShortFileName);
        }


        // get long name and parent (sub)folder ID
        {
          PPROPDOCUMENT pProp = NULL;
          ULONG  ulLen;

          ulParentID = 0;
          *pszLongFileName = EOS;

          if ( UtlLoadFileL( pszPropertyPath, (PVOID *)&pProp, &ulLen, FALSE, FALSE ) )
          {
            // remember long name and parent ID
            strcpy( pszLongFileName, pProp->szLongName );
            ulParentID = pProp->ulParentFolder;
            UtlAlloc( (PVOID *)&pProp, 0L, 0L, NOMSG );
          } /* endif */
        }

        // if no longfilename use short one
        if (*pszLongFileName==EOS) strcpy(pszLongFileName,pszShortFileName);

        // check LongFileName for search criterium
//      fMatch = WildCompares(pszLongFileName,pszSearchCriterium);
        UtlMatchStrings( pszLongFileName, pszSearchCriterium, &fMatch );

        // if parent folder specified check if document belongs to this parent folder
        if ( fMatch && ulParentFolder && pSubFolInfo && (ulParentFolder != ulParentID) )
        {
          fMatch = IsSubFolContainedInParent( ulParentFolder, ulParentID, pSubFolInfo );
        } /* endif */

        if ( fMatch )
        {

          /************************************************************/
          /* Add items to our list                                    */
          /* do a realloc if necessary                                */
          /************************************************************/

          ulLen = strlen(pszLongFileName) + ulPathLen + 1;

          if ( ulLen >= ulLeft )
          {
            fOK = UtlAlloc( (PVOID *) &pStartPool, (LONG) ulAllocLen,
                                         (LONG) ulAllocLen + MAX_POOL_SIZE,
                                         ERROR_STORAGE );
            if ( fOK )
            {
              pStrPool = pStartPool + ulAllocLen - ulLeft;
              ulLeft += MAX_POOL_SIZE;
              ulAllocLen += MAX_POOL_SIZE;
            } /* endif */
          } /* endif */

          if ( ulLen < ulLeft )
          {
            nFiles++;
            strcpy(pStrPool, pInFile );
            strcat(pStrPool, pszLongFileName);

            pStrPool += ulLen;
            ulLeft -= ulLen;
          } /* endif */

        } /* endif WildCompares */

       /************************************************************/
       /* Find next list                                           */
       /************************************************************/

        if (fOK)
        {
           UtlFindNext( hDir, &stResultBuf, sizeof( stResultBuf),
                 &usCount, 0);
        }/* endif fOK */

      } /* endwhile */

      // close search file handle
      if ( hDir != HDIR_CREATE ) UtlFindClose( hDir, FALSE );


      /****************************************************************/
      /* set the pointer to the string pool - will be freed by caller */
      /****************************************************************/
      *ppOutFiles = pStartPool;

    } /* endif */
  } /* endif  mult*/


   if (fMult && nFiles==0)
   {
     strcpy(pStrPool, pszSearchCriterium);
     fOK=FALSE;
   } /* end if */


  // free memory for different file names in use
   UtlAlloc((PVOID *) &pszSearchCriterium,0L,0L,NOMSG);
   UtlAlloc((PVOID *) &pszPropertyPath,0L,0L,NOMSG);
   UtlAlloc((PVOID *) &pszLongFileName,0L,0L,NOMSG);
   UtlAlloc((PVOID *) &pszShortFileName,0L,0L,NOMSG);
   if ( pSubFolInfo ) UtlAlloc((PVOID *)&pSubFolInfo,0L,0L,NOMSG);

  return fOK;
} /* end of function ResolveLongMultFileNames */


//--------------------------- End of EQFFOL00.C --------------------------------
