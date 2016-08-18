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
//|  Fill List Box with document names                                         |
//   Special mode if listbox handle is HWND_FUNCIF in this case
//   the pszBuffer is the address!!! of a buffer pointer and the area
//   pointed to by this pointer will contain the names of the documents on
//   return
//+----------------------------------------------------------------------------+
USHORT LoadDocumentNames( PSZ folder, HWND hlbox, LONG flg, PSZ pszBuffer )
{
  EQFINFO     ErrorInfo;               // error info from EQF API call
  PPROPSYSTEM pSysp;                   // EQF system properties
  FILEFINDBUF stResultBuf;             // DOS file find struct
  USHORT      usCount = 1;             // number of files requested
  USHORT      usRC;                    // return code of called functions
  HDIR        hDirHandle = HDIR_CREATE;// DosFind routine handle
  char       *ppath, *pform;
  PVOID           hProp;               // handle of document properties
  SHORT        sItem;
  CHAR szFormat[MAX_FILESPEC];         // folder format / Tag Table
  static CHAR szMemory[MAX_LONGFILESPEC]; // folder Translation Memory
  static CHAR szFolObjName[MAX_EQF_PATH]; // folder object name
  CHAR szSourceLang[MAX_LANG_LENGTH];  // folder source language
  CHAR szTargetLang[MAX_LANG_LENGTH];  // folder target language
  CHAR szEditor[MAX_FILESPEC];         // folder editor
  PSZ  pszName = RESBUFNAME(stResultBuf);     // address name field in result buffer
  USHORT      usDocs  = 0;             // number of documents found
  PSUBFOLINFO pInfo = NULL;            // subfolder information table

  // variables for document name buffer
  LONG        lBufferSize = 0L;        // size of document buffer
  LONG        lBufferUsed = 0L;        // used bytes in document buffer
  PSZ         pDocNameBuffer = NULL;   // ptr to document buffer
  ULONG       ulFolderID = 0L;         // ID of current folder/subfolder
  BOOL        fDisabled = FALSE;       // folder is disabled flag


#ifdef MEASURETIME
  LONG64 lDummy = 0;
  LONG64 lOther = 0;
  LONG64 lMakeDocListItem = 0;
  LONG64 lInsertItem = 0;
  LONG64 lSubFolCreateInfoTable = 9;
  LONG64 lFolQueryInfo = 0;
  LONG64 lFind = 0;
  LONG64 lPropUpdate = 0;
#endif

  // strip off with-documents-from-included-subfolders flag
  BOOL fWithSubFolderDocs = flg & LOADDOCNAMES_INCLSUBFOLDERS;
  flg &= ~LOADDOCNAMES_INCLSUBFOLDERS;

#ifdef MEASURETIME
  GetElapsedTime( &lDummy );
#endif


  /***********************************************************/
  /* Get settings from folder                                */
  /***********************************************************/
  strcpy( szFolObjName, folder );
  ulFolderID = FolGetSubFolderIdFromObjName( folder );

  // get subfolder information structure for current folder
  if ( fWithSubFolderDocs )
  {
#ifdef MEASURETIME
    GetElapsedTime( &lOther );
#endif
    // build table containing info on all subfolders of active folder
    SubFolCreateInfoTable( folder, &pInfo );

#ifdef MEASURETIME
    GetElapsedTime( &lSubFolCreateInfoTable );
#endif

    // flag all subfolders belonging to current (sub)folder
    {
      PSUBFOLINFO pCurEntry = pInfo;
      while ( pCurEntry->szName[0] != EOS )
      {
        if ( pCurEntry->ulParentFolder == ulFolderID )
        {
          pCurEntry->fFlag = TRUE;
          pCurEntry->ulValue = TRUE; // we have to check the childs of this entry ...
        } /* endif */
        pCurEntry++;
      } /* endwhile */

      // now process all subfolders with ulValue set
      pCurEntry = pInfo;
      while ( pCurEntry->szName[0] != EOS )
      {
        if ( pCurEntry->ulValue )
        {
          PSUBFOLINFO pSubEntry = pInfo;
          while ( pSubEntry->szName[0] != EOS )
          {
            if ( pSubEntry->ulParentFolder == pCurEntry->ulID )
            {
              pSubEntry->fFlag = TRUE;
              pSubEntry->ulValue = TRUE; // we have to check the childs of this entry ...
            } /* endif */
            pSubEntry++;
          } /* endwhile */

          pCurEntry->ulValue = FALSE;// check for child subfolders has been done
          pCurEntry = pInfo;         // restart at the beginning
        }
        else
        {
          pCurEntry++;
        } /* endif */
      } /* endwhile */
    }
  } /* endif */

#ifdef MEASURETIME
    GetElapsedTime( &lDummy );
#endif

  FolQueryInfoEx( folder, szMemory, szFormat, szSourceLang, szTargetLang, szEditor, NULL, NULL, NULL, &fDisabled, NULL, NULL, FALSE, NULLHANDLE );

#ifdef MEASURETIME
    GetElapsedTime( &lFolQueryInfo );
#endif

  if ( hlbox != HWND_FUNCIF )
  {
    ENABLEUPDATEHWND_FALSE( hlbox );
    SETCURSOR( SPTR_WAIT );
  } /* endif */

  pSysp = (PPROPSYSTEM)MakePropPtrFromHnd( EqfQuerySystemPropHnd());
  UtlAlloc( (PVOID *)&ppath, 0L, (LONG) (2 * MAX_PATH144), ERROR_STORAGE );
  pform = ppath + MAX_PATH144;
  if ( FolIsSubFolderObject( folder ) )
  {
    strcpy( szFolObjName, folder );
    UtlSplitFnameFromPath( szFolObjName );  // cut off subfolder name
    UtlSplitFnameFromPath( szFolObjName );  // cut off property directory
  } /* endif */
  sprintf( ppath, "%s\\%s\\*%s", szFolObjName, pSysp->szDirSourceDoc, EXT_DOCUMENT);

#ifdef MEASURETIME
    GetElapsedTime( &lOther );
#endif

  usRC = UtlFindFirst( ppath, &hDirHandle, FILE_NORMAL, &stResultBuf, sizeof( stResultBuf), &usCount, 0L, 0);

#ifdef MEASURETIME
    GetElapsedTime( &lFind );
#endif

  while ( (usRC == NO_ERROR) && usCount )
  {
    //--- check properties of document ---
#ifdef MEASURETIME
    GetElapsedTime( &lOther );
#endif

    hProp = OpenProperties( pszName, szFolObjName, PROP_ACCESS_READ, &ErrorInfo);

#ifdef MEASURETIME
    GetElapsedTime( &lFind );
#endif
    if ( hProp )
    {
      PPROPDOCUMENT pProp = (PPROPDOCUMENT)MakePropPtrFromHnd( hProp );
      BOOL fContainedInFolder = FALSE;

      if ( pProp->ulParentFolder == ulFolderID )
      {
        // document belongs to our subfolder
        fContainedInFolder = TRUE;
      }
      else if ( fWithSubFolderDocs )
      {
        // loop through subfolder info table and check if document belongs
        // to an included subfolder
        PSUBFOLINFO pCurEntry = pInfo;
        while ( pCurEntry->szName[0] != EOS )
        {
          if ( pCurEntry->fFlag && (pCurEntry->ulID == pProp->ulParentFolder) )
          {
            fContainedInFolder = TRUE;
            break;
          }
          else
          {
            pCurEntry++;
          } /* endif */
        } /* endwhile */
      } /* endif */

      if ( !fContainedInFolder )
      {
        // ignore this document, it does not belong to our subfolder
      }
      else if ( (flg == LOADDOCNAMES_ITEMTEXT) && (hlbox != HWND_FUNCIF) )
      {
        // update document properties if one of the parent settings is the same
        // as the document settings
        BOOL fUpdate = FALSE;

        PSZ  pszDocMem = pProp->szLongMemory;
        if (*pszDocMem == EOS )
        {
          pszDocMem = pProp->szMemory;
        } /* endif */

        if ( strcmp( szMemory, pszDocMem ) == 0 )
        {
          // memory is identical to parent memory, blank it out
          pProp->szMemory[0] = EOS;
          pProp->szLongMemory[0] = EOS;
          fUpdate = TRUE;
        } /* endif */

        // fix for S613005 do not update markup table automatically!!!
        //if ( strcmp( szFormat, pProp->szFormat ) == 0 )
        //{
        //  // markup is identical to parent markup, blank it out
        //  pProp->szFormat[0] = EOS;
        //  fUpdate = TRUE;
        //} /* endif */

        if ( strcmp( szSourceLang, pProp->szSourceLang ) == 0 )
        {
          // source language is identical to parent source language, blank it out
          pProp->szSourceLang[0] = EOS;
          fUpdate = TRUE;
        } /* endif */

        if ( strcmp( szTargetLang, pProp->szTargetLang ) == 0 )
        {
          // Target language is identical to parent target language, blank it out
          pProp->szTargetLang[0] = EOS;
          fUpdate = TRUE;
        } /* endif */

        // update document properties if required
        if ( fUpdate )
        {
#ifdef MEASURETIME
    GetElapsedTime( &lOther );
#endif
          if ( SetPropAccess( hProp, PROP_ACCESS_WRITE) )
          {
            SaveProperties( hProp, &ErrorInfo);
            ResetPropAccess( hProp, PROP_ACCESS_WRITE );
          } /* endif */
#ifdef MEASURETIME
    GetElapsedTime( &lPropUpdate );
#endif

        } /* endif */

        // create listbox item
#ifdef MEASURETIME
    GetElapsedTime( &lOther );
#endif

        FOLMakeDocListItem ( pProp, szFormat, szMemory, szSourceLang, szTargetLang, szEditor, pszBuffer );

#ifdef MEASURETIME
    GetElapsedTime( &lMakeDocListItem );
#endif

        sItem = INSERTITEMHWND( hlbox, pszBuffer );
        if ( fDisabled && (sItem != LIT_NONE) )
        {
          WinSendMsg( hlbox, LM_EQF_SETITEMSTATE, MP1FROMSHORT( sItem ), MP2FROMSHORT( FALSE ) );
        } /* endif */


#ifdef MEASURETIME
    GetElapsedTime( &lInsertItem );
#endif

        usDocs++;
      }
      else
      {
        if ( hlbox != HWND_FUNCIF )
        {
          if ( flg == LOADDOCNAMES_OBJNAME )
          {
            OBJNAME szDocObjName;
            strcpy( szDocObjName, pProp->PropHead.szPath );
            strcat( szDocObjName, BACKSLASH_STR );
            strcat( szDocObjName, pProp->PropHead.szName );
            INSERTITEMHWND( hlbox, szDocObjName );
          }
          else if ( flg == LOADDOCNAMES_LONGNAME )
          {
            PSZ pszLongName = pProp->szLongName;
            if ( *pszLongName == EOS ) pszLongName = pszName;

            INSERTITEMHWND( hlbox, pszLongName );
          }
          else
          {
            INSERTITEMHWND( hlbox, pszName );
          } /* endif */
        }
        else
        {
          // document name buffer mode of function
          OBJNAME szDocObjName;
          PSZ pszInsertName;
          LONG lAddLen; 
          
          // get/build document name
          if ( flg == LOADDOCNAMES_OBJNAME )
          {
            strcpy( szDocObjName, pProp->PropHead.szPath );
            strcat( szDocObjName, BACKSLASH_STR );
            strcat( szDocObjName, pProp->PropHead.szName );
            pszInsertName = szDocObjName;
          }
          else if ( flg == LOADDOCNAMES_LONGNAME )
          {
            pszInsertName = pProp->szLongName;
            if ( *pszInsertName == EOS ) pszInsertName = pszName;
          }
          else
          {
            pszInsertName = pszName;
          } /* endif */

          // add document name to document name buffer
          lAddLen = strlen(pszInsertName) + 1;
          if ( lBufferSize < (lBufferUsed + lAddLen) )
          {
            UtlAllocHwnd( (PVOID *)&pDocNameBuffer, lBufferSize,
                          lBufferSize + 8096L, ERROR_STORAGE, HWND_FUNCIF );
            lBufferSize += 8096L;
          } /* endif */

          if ( pDocNameBuffer != NULL )
          {
            strcpy( pDocNameBuffer + lBufferUsed, pszInsertName );
            lBufferUsed += lAddLen;
          } /* endif */
        } /* endif */
        usDocs++;
      } /* endif */

      CloseProperties( hProp, PROP_QUIT, &ErrorInfo );
    }
    else
    {
      /*************************************************************/
      /* Insert as disabled item                                  */
      /*************************************************************/
      if ( (flg == LOADDOCNAMES_ITEMTEXT) && (hlbox != HWND_FUNCIF) )
      {
        sprintf( pszBuffer,
                 "%s\\%s\x15%s\x15 \x15%lu\x15%lu\x15%lu\x15%lu\x15%lu\x15%lu\x15 \x15 \x15 \x15 \x15 \x15",
                 szFolObjName,              // FOL_OBJECT_IND
                 pszName,
                 pszName,                   // FOL_NAME_IND
                 0L,                        // FOL_TRANSLATED_IND
                 0L,                        // FOL_SEGMENTED_IND
                 0L,                        // FOL_EXPORTED_IND
                 0L,                        // FOL_IMPORTED_IND
                 0L,                        // FOL_TOUCHED_IND
                 0L                         // FOL_TOUCHEDTIME_IND
               );
        sItem = INSERTITEMHWND( hlbox, pszBuffer );
        usDocs++;
        if ( sItem != LIT_NONE )
        {
          WinSendMsg( hlbox, LM_EQF_SETITEMSTATE, MP1FROMSHORT( sItem ), MP2FROMSHORT( FALSE ) );
        } /* endif */
      } /* endif */
    } /* endif */

#ifdef MEASURETIME
    GetElapsedTime( &lOther );
#endif
    usRC = UtlFindNext( hDirHandle, &stResultBuf, sizeof( stResultBuf),
                        &usCount, 0);
#ifdef MEASURETIME
    GetElapsedTime( &lFind );
#endif
  } /* endwhile */
  // close search file handle
  if ( hDirHandle != HDIR_CREATE ) UtlFindClose( hDirHandle, FALSE );


  UtlAlloc( (PVOID *)&ppath, 0L, 0L, NOMSG );


  if ( hlbox != HWND_FUNCIF )
  {
    ENABLEUPDATEHWND_TRUE( hlbox);
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

  if ( pInfo ) UtlAlloc( (PVOID *)&pInfo, 0L, 0L, NOMSG );

#ifdef MEASURETIME
  GetElapsedTime( &lOther );

  {
    FILE *hfLog = NULL;
    CHAR szLogFile[MAX_EQF_PATH];

    UtlMakeEQFPath( szLogFile, NULC, LOG_PATH, NULL );
    strcat( szLogFile, "\\FOLLIST.LOG" );

    hfLog = fopen( szLogFile, "a" );

    if ( hfLog )
    {
      fprintf( hfLog, "**** INSERTNAMES for %s ********\n", folder );

      fprintf( hfLog, "Time for MakeDocListItem   : %10I64d ms\n", lMakeDocListItem );
      fprintf( hfLog, "INSERTITEM in listbox time : %10I64d ms\n", lInsertItem );
      fprintf( hfLog, "SubFolCreateInfoTable time : %10I64d ms\n", lSubFolCreateInfoTable );
      fprintf( hfLog, "FolQueryInfo time          : %10I64d ms\n", lFolQueryInfo );
      fprintf( hfLog, "FindFirst/FindNext time    : %10I64d ms\n", lFind );
      fprintf( hfLog, "DocPropUpdate time         : %10I64d ms\n", lPropUpdate );
      fprintf( hfLog, "Other times                : %10I64d ms\n", lOther );
 
      fclose( hfLog );
    } /* endif */


  }
#endif

  return( usDocs );
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
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     FolQueryInfo                                             |
//+----------------------------------------------------------------------------+
//|Description:       Queries information from folder properties.              |
//+----------------------------------------------------------------------------+
//|Parameters:        PSZ     pszFolName  object name of folder                |
//|                   PSZ     pszMemory   buffer for translation memory or NULL|
//|                   PSZ     pszFormat   buffer for document format or NULL   |
//|                   PSZ     pszSrcLng   buffer for source language or NULL   |
//|                   PSZ     pszTrgLng   buffer for target language or NULL   |
//|                   BOOL    fMsg        do-message-handling flag             |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       NO_ERROR          function completed successfully        |
//|                   other             TM/2 error code                        |
//+----------------------------------------------------------------------------+
USHORT FolQueryInfo
(
PSZ              pszFolName,         // object name of folder
PSZ              pszMemory,          // buffer for translation memory or NULL
PSZ              pszFormat,          // buffer for folder format or NULL
PSZ              pszSrcLng,          // buffer for source language or NULL
PSZ              pszTrgLng,          // buffer for target language or NULL
BOOL             fMsg                // do-message-handling flag
)
{
  return( FolQueryInfo2Hwnd( pszFolName, pszMemory, pszFormat, pszSrcLng,
                             pszTrgLng, NULL, fMsg, NULLHANDLE ) );
}

USHORT FolQueryInfo2
(
PSZ              pszFolName,         // object name of folder
PSZ              pszMemory,          // buffer for translation memory or NULL
PSZ              pszFormat,          // buffer for folder format or NULL
PSZ              pszSrcLng,          // buffer for source language or NULL
PSZ              pszTrgLng,          // buffer for target language or NULL
PSZ              pszEditor,          // buffer for editor or NULL
BOOL             fMsg                // do-message-handling flag
)
{
  return( FolQueryInfoEx( pszFolName, pszMemory, pszFormat, pszSrcLng,
                          pszTrgLng, pszEditor,
                          NULL, NULL, NULL, NULL, NULL, NULL,
                          fMsg, NULLHANDLE ) );
}

USHORT FolQueryInfoHwnd
(
PSZ              pszFolName,         // object name of folder
PSZ              pszMemory,          // buffer for translation memory or NULL
PSZ              pszFormat,          // buffer for folder format or NULL
PSZ              pszSrcLng,          // buffer for source language or NULL
PSZ              pszTrgLng,          // buffer for target language or NULL
BOOL             fMsg,               // do-message-handling flag
HWND             hwnd                // owner handle for error messages
)
{
  return( FolQueryInfoEx( pszFolName, pszMemory, pszFormat, pszSrcLng,
                          pszTrgLng, NULL, NULL, NULL, NULL, NULL, NULL, NULL, fMsg, hwnd ) );
}


USHORT FolQueryInfo2Hwnd
(
PSZ              pszFolName,         // object name of folder
PSZ              pszMemory,          // buffer for translation memory or NULL
PSZ              pszFormat,          // buffer for folder format or NULL
PSZ              pszSrcLng,          // buffer for source language or NULL
PSZ              pszTrgLng,          // buffer for target language or NULL
PSZ              pszEditor,          // buffer for editor or NULL
BOOL             fMsg,               // do-message-handling flag
HWND             hwnd                // owner handle for error messages
)
{
  return( FolQueryInfoEx( pszFolName, pszMemory, pszFormat, pszSrcLng,
                          pszTrgLng, pszEditor,
                          NULL, NULL, NULL, NULL, NULL, NULL,
                          fMsg, hwnd ) );
}

USHORT FolQueryInfoEx
(
PSZ              pszFolName,         // object name of folder
PSZ              pszMemory,          // buffer for translation memory or NULL
PSZ              pszFormat,          // buffer for folder format or NULL
PSZ              pszSrcLng,          // buffer for source language or NULL
PSZ              pszTrgLng,          // buffer for target language or NULL
PSZ              pszEditor,          // buffer for editor or NULL
PSZ              pszConversion,      // buffer for conversion or NULL
PSZ              pszVendor,          // buffer for translator name or NULL
PSZ              pszVendorEMail,     // buffer for eMail address or NULL
PBOOL            pfDisabled,         // folder disabled flag
PVOID            pvUnused1,
PVOID            pvUnused2,
BOOL             fMsg,               // do-message-handling flag
HWND             hwnd                // owner handle for error messages
)
{
  PPROPFOLDER      pProp = NULL;       // pointer to folder properties
  USHORT           usRC = NO_ERROR;    // function return code
  CHAR             szSysDrive[MAX_DRIVE]; // buffer for system drive
  PSZ              pszPropName = NULL; // ptr to property file name
  PSZ              pszObjName = NULL;  // ptr to object name
  BOOL             fOK = TRUE;         // success indicator
  BOOL             fSubFolder = FALSE; // TRUE = object is a subfolder
  BOOL             fDone = FALSE;      // completed flag

  pvUnused1; pvUnused2;

  /********************************************************************/
  /* Preset caller's buffers                                          */
  /********************************************************************/
  if ( pszMemory != NULL )      *pszMemory = EOS;
  if ( pszFormat != NULL )      *pszFormat = EOS;
  if ( pszSrcLng != NULL )      *pszSrcLng = EOS;
  if ( pszTrgLng != NULL )      *pszTrgLng = EOS;
  if ( pszEditor != NULL )      *pszEditor = EOS;
  if ( pszConversion != NULL )  *pszConversion = EOS;
  if ( pszVendor != NULL )      *pszVendor = EOS;
  if ( pszVendorEMail != NULL ) *pszVendorEMail = EOS;
  if ( pfDisabled!= NULL )      *pfDisabled = FALSE;

  // allocate buffer for property file names
  fOK = UtlAlloc( (PVOID *) &pszObjName, 0L, (LONG)(2 * MAX_LONGPATH), NOMSG );
  if ( !fOK )
  {
    if ( fMsg )
    {
      UtlErrorHwnd( ERROR_STORAGE, MB_CANCEL, 0, NULL, EQF_ERROR, hwnd );
    } /* endif */
    usRC = ERROR_STORAGE;
  } /* endif */

  // loop over parent subfolder/folder chain until all required data can be supplied
  if ( fOK )
  {
    // start with supplied folder/subfolder object name
    pszPropName = pszObjName + MAX_LONGPATH;
    strcpy( pszObjName, pszFolName );

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
        ULONG ulLen;
        fOK = UtlLoadFileL( pszPropName, (PVOID *)&pProp, &ulLen, FALSE, FALSE );
      } /* endif */

      // do any error handling
      if ( !fOK )
      {
        // Error accessing properties
        if ( fMsg )
        {
          UtlErrorHwnd( ERROR_PROPERTY_ACCESS, MB_CANCEL, 1, &pszFolName,
                        EQF_ERROR, hwnd );
        } /* endif */
        usRC = ERROR_PROPERTY_ACCESS;
      } /* endif */

      // complete caller data with data from properties
      if ( fOK )
      {
        fDone = TRUE;                // assume all info can be provided ...

        if ( pszMemory != NULL )
        {
          if ( pProp->szLongMemory[0] != EOS )
            strcpy( pszMemory, pProp->szLongMemory );
          else if ( pProp->szMemory[0] != EOS )
            strcpy( pszMemory, pProp->szMemory );

          if ( *pszMemory != EOS )
            pszMemory = NULL;       // data has been supplied
          else
            fDone = FALSE;          // still data missing
        } /* endif */

        if ( pszFormat != NULL )
        {
          strcpy( pszFormat, pProp->szFormat );
          if ( *pszFormat != EOS )
            pszFormat = NULL;       // data has been supplied
          else
            fDone = FALSE;          // still data missing
        } /* endif */

        if ( pszSrcLng != NULL )
        {
          strcpy( pszSrcLng, pProp->szSourceLang );
          if ( *pszSrcLng != EOS )
            pszSrcLng = NULL;      // data has been supplied
          else
            fDone = FALSE;          // still data missing
        } /* endif */

        if ( pszTrgLng != NULL )
        {
          strcpy( pszTrgLng, pProp->szTargetLang);
          if ( *pszTrgLng != EOS )
            pszTrgLng = NULL;      // data has been supplied
          else
            fDone = FALSE;          // still data missing
        } /* endif */

        if ( pszEditor != NULL )
        {
          strcpy( pszEditor, pProp->szEditor );
          if ( *pszEditor != EOS )
            pszEditor = NULL;      // data has been supplied
          else
            fDone = FALSE;          // still data missing
        } /* endif */

        if ( pszConversion != NULL )
        {
          strcpy( pszConversion, pProp->szConversion );
          if ( *pszConversion != EOS )
            pszConversion = NULL;  // data has been supplied
          else
            fDone = FALSE;          // still data missing
        } /* endif */

        if ( pszVendor != NULL )
        {
          strcpy( pszVendor, pProp->szVendor );
          if ( *pszVendor != EOS )
            pszVendor = NULL;      // data has been supplied
          else
            fDone = FALSE;          // still data missing
        } /* endif */

        if ( pszVendorEMail != NULL )
        {
          strcpy( pszVendorEMail, pProp->szVendorEMail );
          if ( *pszVendorEMail != EOS )
            pszVendorEMail = NULL;       // data has been supplied
          else
            fDone = FALSE;          // still data missing
        } /* endif */
      } /* endif */


      if ( pfDisabled != NULL )
      {
        *pfDisabled = pProp->fDisabled_UserExitRefresh;
          fDone = FALSE;          // stay in loop until folder is reached
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
} /* end of function FolQueryInfoEx */


//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     FolLongToShortDocName                                    |
//+----------------------------------------------------------------------------+
//|Description:       Converts a long document name to a short one and         |
//|                   ensures that the short names are unique within the       |
//|                   folder.                                                  |
//|                   Do not use UtlMakeEQFPath and OpenProperties because     |
//|                   this function is called from our API, too.               |
//+----------------------------------------------------------------------------+
//|Parameters:        PSZ     pszFolObjName  object name of folder             |
//|                   PSZ     pszLongName    ptr to long file name of document |
//|                   PSZ     pszShortName   buffer for document short name    |
//|                   PBOOL   pfIsNew        TRUE if document is a new one     |
//+----------------------------------------------------------------------------+
//|Returncode type:   BOOL                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:       TRUE              function completed successfully        |
//|                   other             TM/2 error code                        |
//+----------------------------------------------------------------------------+
USHORT FolLongToShortDocName
(
PSZ              pszFolObjName,      // object name of folder
PSZ              pszLongName,        // long document name
PSZ              pszShortName,       // buffer for short document name
PBOOL            pfIsNew             // TRUE if document is a new one
)
{
  // our private data area
  typedef struct _L2SDATA
  {
    CHAR      szShortName[MAX_FILESPEC];    // buffer for short document name
    CHAR      szFolder[MAX_FILESPEC];       // buffer for folder name
    CHAR      szSearchPath[MAX_EQF_PATH];   // document search path
    CHAR      szDocFullPath[MAX_EQF_PATH];  // fully qualified document name
    FILEFINDBUF stResultBuf;                // DOS file find structure
  } L2SDATA, *PL2SDATA;

  // local variables
  PSZ         pszFileName = NULL;      // ptr to private filename area
  PL2SDATA    pData = NULL;            // ptr to private data area
  USHORT      usRC = NO_ERROR;         // function return code
  BOOL        fIsLongName = UtlIsLongFileName( pszLongName );

  // preset callers's variables
  *pfIsNew = TRUE;
  pszShortName[0] = EOS;

  // allocate our private data area
  if ( !UtlAlloc( (PVOID *)&pData, 0L, (LONG)sizeof(L2SDATA), ERROR_STORAGE ) )
  {
    usRC = ERROR_NOT_ENOUGH_MEMORY;
  } /* endif */

  // first try: for short names and mixed-cased names check if document
  // properties exist for the given name
  if ( usRC == NO_ERROR )
  {
    if ( (fIsLongName == 2) ||         // a normal but mixed-case file name
         (fIsLongName == FALSE) )      // or a uppercase short name???
    {
      // Check if we have a document with the given name

      strcpy( pszShortName, pszLongName );

      strcpy( pData->szFolder, UtlGetFnameFromPath( pszFolObjName ) );
      /******************************************************************/
      /* Build path ..                                                  */
      /******************************************************************/
      sprintf( pData->szSearchPath, "%c:\\%s\\%s\\%s", pszFolObjName[0], PATH,
               pData->szFolder, SOURCEDIR );

      strcat( pData->szSearchPath, BACKSLASH_STR );
      strcat( pData->szSearchPath, pszLongName );
      strupr( pData->szSearchPath );
      if ( UtlFileExist( pData->szSearchPath ) ) // is there a document with
      // this name???
      {
        // use the name of the existing document as short name
        strcpy( pszShortName, pszLongName );
        strupr( pszShortName );
        *pfIsNew = FALSE;

        // cleanup
        if ( pData != NULL ) UtlAlloc( (PVOID *)&pData, 0L, 0L,NOMSG );

        // return
        return( usRC );
      } /* endif */
    } /* endif */
  } /* endif */

  // get document short name and setup search path
  if ( usRC == NO_ERROR )
  {
    UtlLongToShortName( pszLongName, pData->szShortName );
    strcpy( pData->szFolder, UtlGetFnameFromPath( pszFolObjName ) );
    /******************************************************************/
    /* Build path ..                                                  */
    /******************************************************************/
    sprintf( pData->szSearchPath, "%c:\\%s\\%s\\%s", pszFolObjName[0], PATH,
             pData->szFolder, SOURCEDIR );

    strcat( pData->szSearchPath, BACKSLASH_STR );
    strcat( pData->szSearchPath, pData->szShortName );
    strcat( pData->szSearchPath, DEFAULT_PATTERN_EXT );
  } /* endif */

  if ( usRC == NO_ERROR )
  {
    UtlAlloc( (PVOID *) &pszFileName, 0L, (LONG) MAX_LONGPATH, NOMSG );
    if ( !pszFileName )
    {
      usRC = ERROR_NOT_ENOUGH_MEMORY;
    } /* endif */
  } /* endif */

  // look for documents having the same short name
  if ( usRC == NO_ERROR )
  {
    USHORT usDosRC = NO_ERROR;         // return code of called DOS functions
    USHORT usCount = 1;                // number of files requested
    HDIR   hDir = HDIR_CREATE;         // file find handle
    BOOL   fOK;
    CHAR   szShortNameCaseMismatch[MAX_FILESPEC]; // buffer for short name which does not match case
    BOOL   fCaseMismatchFound = FALSE; // TRUE = found matching file with different casing

    usDosRC = UtlFindFirst( pData->szSearchPath, &hDir, FILE_NORMAL,
                            &(pData->stResultBuf), sizeof(pData->stResultBuf),
                            &usCount, 0L, FALSE );

    while ( *pfIsNew && (usDosRC == NO_ERROR) && usCount )
    {
      PPROPDOCUMENT pProp = NULL;      // ptr to document properties
      ULONG ulBytesRead;

      sprintf( pszFileName, "%c:\\%s\\%s\\%s\\%s", pszFolObjName[0], PATH,
               pData->szFolder, PROPDIR, RESBUFNAME( pData->stResultBuf ) );

      fOK = UtlLoadFileL( pszFileName,            // name of file to be loaded
                         (PVOID *)&pProp,        // return pointer to loaded file
                         &ulBytesRead,           // length of loaded file
                         FALSE,
                         FALSE );
      if ( !fOK )
      {
        usDosRC = ERROR_PROPERTY_ACCESS;
      } /* endif */

      if ( usDosRC == NO_ERROR )
      {
        BOOL fFound;

        // for real long names use case-sensitive compare else
        // use case-insensitive compare
        if ( fIsLongName == TRUE ) // compare with 1!!! (2 is mixed-case name!)
        {
          // the long name in the properties is stored in ASCII ...
          OEMTOANSI( pProp->szLongName );

          fFound = (strcmp( pszLongName, pProp->szLongName ) == 0);
          if ( !fFound && !fCaseMismatchFound )
          {
            fCaseMismatchFound = (stricmp( pszLongName, pProp->szLongName ) == 0);
            if ( fCaseMismatchFound )
            {
              strcpy( szShortNameCaseMismatch, RESBUFNAME( pData->stResultBuf ) );
            } /* endif */
          } /* endif */
        }
        else
        {
          fFound = (stricmp( pszLongName, pProp->szLongName ) == 0);
        } /* endif */


        if ( fFound )
        {
          *pfIsNew = FALSE;
          strcpy( pszShortName, RESBUFNAME( pData->stResultBuf ) );
        } /* endif */

        UtlAlloc( (PVOID *) &pProp, 0L, 0L, NOMSG );
      } /* endif */

      // try next document
      if ( *pfIsNew )
      {
        usCount = 1;
        usDosRC = UtlFindNext( hDir, &pData->stResultBuf,
                               sizeof(pData->stResultBuf), &usCount, FALSE );
      } /* endif */
    } /* endwhile */

    UtlFindClose( hDir, FALSE );

    if ( *pfIsNew && fCaseMismatchFound )
    {
      *pfIsNew = FALSE;
      strcpy( pszShortName, szShortNameCaseMismatch );
    } /* endif */

  } /* endif */

  // find a unique name if document is not contained in the folder
  if ( (usRC == NO_ERROR) && *pfIsNew )
  {
    SHORT i1 = 0;                      // counter for document extension 
    SHORT i2 = 0;                      // counter for document extension 
    SHORT i3 = 0;                      // counter for document extension 
    BOOL  fOverFlow = FALSE;
    CHAR chLetters[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    CHAR szExt[4] = "000";

    sprintf( pData->szSearchPath, "%c:\\%s\\%s\\%s", pszFolObjName[0], PATH,
             pData->szFolder, SOURCEDIR );

    do
    {
      szExt[0] = chLetters[i3];
      szExt[1] = chLetters[i2];
      szExt[2] = chLetters[i1];

      sprintf( pData->szDocFullPath, "%s\\%s.%s", pData->szSearchPath, pData->szShortName, szExt );

      // get next number
      i1 += 1;
      if ( i1 >= 36 )
      {
        i1 = 0;
        i2 += 1;
        if ( i2 >= 36 )
        {
          i2 = 0;
          i3 += 1;
          if ( i3 >= 36 )
          {
            fOverFlow = TRUE;
          } /* endif */
        } /* endif */
      } /* endif */
    } while ( !fOverFlow && UtlFileExist( pData->szDocFullPath ) ); /* enddo */

    strcpy( pszShortName, UtlGetFnameFromPath( pData->szDocFullPath ) );
  } /* endif */

  // cleanup
  if ( pData != NULL ) UtlAlloc( (PVOID *)&pData, 0L, 0L,NOMSG );

  if ( pszFileName != NULL ) UtlAlloc( (PVOID *)&pszFileName, 0L, 0L,NOMSG );

  return( usRC );
} /* end of function FolLongToShortDocName */

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

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     FolRenameFolder                                          |
//+----------------------------------------------------------------------------+
//|Description:       Physically rename folder and all                         |
//|                   associated files                                         |
//+----------------------------------------------------------------------------+
//|Input parameter:   BOOL        fMsg,              show-error-messages flag  |
//|                   PSZ         pszOldAndNewName   old and new folder name   |
//|                                                  seperated by 0x15         |
//|                                                  (old name = object name,  |
//|                                                  new name = file name only)|
//|                              e.g. H:\EQF\SAMPLE1.F000x15NEWNAME0x00        |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT              error code or 0 if success           |
//+----------------------------------------------------------------------------+
BOOL FolRenameFolder
(
PSZ         pszOldAndNewName,        // old and new folder name (x15 seperated)
BOOL        fMsg                     // show-error-messages flag
)
{
  // our private data area
  typedef struct _PRIVATEDATA
  {
    CHAR       szOldObject[MAX_EQF_PATH];// buffer for old object name
    CHAR       szNewObject[MAX_EQF_PATH];// buffer for new object name
    CHAR       szFolProp[MAX_EQF_PATH]; // buffer for folder property file name
    CHAR       szFolPropDir[MAX_EQF_PATH]; // buffer for folder property directory
    CHAR       szDocPropName[MAX_EQF_PATH]; // buffer for document prop. file name
    CHAR       szOldName[MAX_FNAME];    // buffer for old name
    CHAR       szNewName[MAX_FNAME];    // buffer for new name
    CHAR       szOldPath[MAX_EQF_PATH]; // buffer for old path name
    CHAR       szNewPath[MAX_EQF_PATH]; // buffer for new path name
    CHAR       szFullFolderName[MAX_FILESPEC]; // buffer for new folder name (w/ .EXT)
    PROPFOLDER stFolProp;               // buffer for folder properties
    CHAR       szLongName[MAX_LONGFILESPEC]; // buffer for new long name
    CHAR       szOldLongName[MAX_LONGFILESPEC]; // buffer for old long name
  } PRIVATEDATA, *PPRIVATEDATA;

  BOOL        fOK = TRUE;              // internal O.K. flag and return value
  PPRIVATEDATA pData = NULL;           // ptr to private data area

  // allocate our private data area
  fOK = UtlAlloc( (PVOID *)&pData, 0L, (LONG)sizeof(PRIVATEDATA),
                  (USHORT)(( fMsg ) ? ERROR_STORAGE : NOMSG ) );

  // split input data and store old and new folder name
  if ( fOK )
  {
    PSZ pszNewName = strchr( pszOldAndNewName, X15 );
    if ( pszNewName != NULL )
    {
      PSZ pszNewLongName;
      *pszNewName = EOS;
      pszNewName++;
      pszNewLongName = strchr( pszNewName, X15 );
      if ( pszNewLongName != NULL )
      {
        *pszNewLongName = EOS;
        pszNewLongName++;
        strcpy( pData->szLongName, pszNewLongName );
      }
      else
      {
        pData->szLongName[0] = EOS;
      } /* endif */
      strcpy( pData->szNewName, pszNewName );
    } /* endif */
    strcpy( pData->szOldObject, pszOldAndNewName );
    Utlstrccpy( pData->szOldName, UtlGetFnameFromPath( pData->szOldObject ),
                DOT );
  } /* endif */

  // setup folder property file name
  if ( fOK )
  {
    UtlMakeEQFPath( pData->szFolProp, NULC, PROPERTY_PATH, NULL );
    strcat( pData->szFolProp, BACKSLASH_STR );
    strcat( pData->szFolProp, pData->szOldName );
    strcat( pData->szFolProp, EXT_FOLDER_MAIN );
  } /* endif */

  // load folder property file
  if ( fOK )
  {
    ULONG ulRead;     // number of bytes read from disk
    PVOID pvTemp = (PVOID)&(pData->stFolProp);
    fOK = UtlLoadFileL( pData->szFolProp, &pvTemp, &ulRead,
                       FALSE, fMsg );
  } /* endif */

  // rename folder directory
  if ( fOK )
  {
    // setup old path name
    UtlMakeEQFPath( pData->szOldPath, pData->stFolProp.chDrive,
                    SYSTEM_PATH, NULL );
    strcat( pData->szOldPath, BACKSLASH_STR );
    strcat( pData->szOldPath, pData->szOldName );
    strcat( pData->szOldPath, EXT_FOLDER_MAIN );

    // setup new path name
    UtlMakeEQFPath( pData->szNewPath, pData->stFolProp.chDrive,
                    SYSTEM_PATH, NULL );
    strcat( pData->szNewPath, BACKSLASH_STR );
    strcat( pData->szNewPath, pData->szNewName );
    strcat( pData->szNewPath, EXT_FOLDER_MAIN );

    // actually rename directory using UtlMove...
    fOK = UtlMove( pData->szOldPath, pData->szNewPath, 0L,
                   fMsg ) == NO_ERROR;
  } /* endif */

  // rename folder property file
  if ( fOK )
  {
    // setup old path name
    UtlMakeEQFPath( pData->szOldPath, NULC, PROPERTY_PATH, NULL );
    strcat( pData->szOldPath, BACKSLASH_STR );
    strcat( pData->szOldPath, pData->szOldName );
    strcat( pData->szOldPath, EXT_FOLDER_MAIN );

    // setup new path name
    UtlMakeEQFPath( pData->szNewPath, NULC, PROPERTY_PATH, NULL );
    strcat( pData->szNewPath, BACKSLASH_STR );
    strcat( pData->szNewPath, pData->szNewName );
    strcat( pData->szNewPath, EXT_FOLDER_MAIN );

    // actually rename property file using UtlMove...
    fOK = UtlMove( pData->szOldPath, pData->szNewPath, 0L,
                   fMsg ) == NO_ERROR;
  } /* endif */

  // adjust name in folder property file
  if ( fOK )
  {
    if ( pData->stFolProp.szLongName[0] != EOS )
    {
      strcpy( pData->szOldLongName, pData->stFolProp.szLongName );
    }
    else
    {
      Utlstrccpy( pData->szOldLongName, pData->stFolProp.PropHead.szName, DOT );
    } /* endif */
    strcpy( pData->stFolProp.PropHead.szName, pData->szNewName );
    strcat( pData->stFolProp.PropHead.szName, EXT_FOLDER_MAIN );

    if ( pData->stFolProp.szOrgName[0] == EOS )
    {
      strcpy( pData->stFolProp.szOrgName, pData->szOldName );
    } /* endif */

    if ( pData->stFolProp.szOrgLongName[0] == EOS )
    {
      if ( pData->stFolProp.szLongName[0] != EOS )
      {
        strcpy( pData->stFolProp.szOrgLongName, pData->stFolProp.szLongName );
      } /* endif */
    } /* endif */

    // adjust long name
    if ( strcmp( pData->szLongName, pData->szNewName ) != EOS )
    {
      strcpy( pData->stFolProp.szLongName, pData->szLongName );
    }
    else
    {
      pData->stFolProp.szLongName[0] = EOS;
    } /* endif */
  } /* endif */

  // rewrite folder property file
  if ( fOK )
  {
    fOK = UtlWriteFile( pData->szNewPath,
                        sizeof(pData->stFolProp),
                        &(pData->stFolProp), fMsg ) == NO_ERROR;
  } /* endif */

  // adjust names in property files of the documents contained in this folder
  if ( fOK )
  {
    WIN32_FIND_DATA FileFindData;
    HANDLE hDir = HDIR_CREATE;

    // setup search path: folder document source directory
    {
      strcpy( pData->szFullFolderName, pData->szNewName );
      strcat( pData->szFullFolderName, EXT_FOLDER_MAIN );
      UtlMakeEQFPath( pData->szFolPropDir, pData->stFolProp.chDrive,
                      DIRSOURCEDOC_PATH, pData->szFullFolderName );
      strcat( pData->szFolPropDir, BACKSLASH_STR );
      strcat( pData->szFolPropDir, DEFAULT_PATTERN );
    }

    // loop over all documents and correct property header
    hDir = FindFirstFile( pData->szFolPropDir, &FileFindData );
    if ( hDir != INVALID_HANDLE_VALUE )
    {
      BOOL fMoreFiles = TRUE;

      // setup new path for document property header
      UtlMakeEQFPath( pData->szFolPropDir, pData->stFolProp.chDrive, SYSTEM_PATH, NULL );
      strcat( pData->szFolPropDir, BACKSLASH_STR );
      strcat( pData->szFolPropDir, pData->szNewName );
      strcat( pData->szFolPropDir, EXT_FOLDER_MAIN );

      while ( fOK && fMoreFiles )
      {
        if ( (FileFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0 )
        {
          // get property file name of current item
          UtlMakeEQFPath( pData->szDocPropName, pData->stFolProp.chDrive, PROPERTY_PATH, pData->szFullFolderName );
          strcat( pData->szDocPropName, BACKSLASH_STR );
          strcat( pData->szDocPropName, FileFindData.cFileName );

          // adjust property header
          fOK = FolCorrectPropHead( pData->szDocPropName, NULC, pData->szFolPropDir, NULL, NULLHANDLE );
        } /* endif */

        // continue with next document
        fMoreFiles = FindNextFile( hDir, &FileFindData );
      } /* endwhile */
      FindClose( hDir );
    }
  } /* endif */

  // broadcast changed folder name
  if ( fOK )
  {
    // remove original folder name
    EqfSend2AllHandlers( WM_EQFN_DELETED,
                         MP1FROMSHORT(clsFOLDER),
                         MP2FROMP(pData->szOldObject) );
    OEMTOANSI( pData->szOldLongName );
    EqfSend2AllHandlers( WM_EQFN_DELETEDNAME,
                         MP1FROMSHORT( clsFOLDER ),
                         MP2FROMP( pData->szOldLongName ));

    // add new folder name
    UtlMakeEQFPath( pData->szNewObject, NULC, SYSTEM_PATH, NULL );
    strcat( pData->szNewObject, BACKSLASH_STR );
    strcat( pData->szNewObject, pData->stFolProp.PropHead.szName );
    EqfSend2AllHandlers( WM_EQFN_CREATED, MP1FROMSHORT( clsFOLDER ),
                         MP2FROMP(pData->szNewObject) );
  } /* endif */
  return( fOK );
} /* end of function FolRenameFolder */

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

// convert a folder or subfolder object name to a folder/subfolder name
BOOL SubFolObjectNameToName( PSZ pszObjName, PSZ pszName )
{
  BOOL        fOK = TRUE;              // function return code
  PSZ         pszExtention;            // points to extention of object name
  PPROPFOLDER pProp = NULL;            // loaded property file
  PSZ         pszObjNameBuf = NULL;    // buffer for object name

  // handle folder and subfolder object name differently:
  // the subfolder object name is the fully qualified property file path
  // the folder object name is the path name of the property file w/o the
  // property directory
  pszExtention = strchr( pszObjName, DOT );
  if ( pszExtention == NULL )
  {
    // passed name is no valid object name
    fOK = FALSE;
  }
  else
  {
    // allocate buffer for object names of parent (sub)folders
    fOK = UtlAlloc( (PVOID *)&pszObjNameBuf, 0L, MAX_EQF_PATH, NOMSG );

    // handle folder or subfolder object name
    if ( fOK )
    {
      if ( strcmp( pszExtention, EXT_FOLDER_MAIN ) == 0 )
      {
        ULONG ulLen = 0;

        // process folder

        // insert property directory into object name
        {
          PSZ pszName = strrchr( pszObjName, BACKSLASH );
          if ( pszName )
          {
            *pszName = EOS;
            strcpy( pszObjNameBuf, pszObjName );
            strcat( pszObjNameBuf, BACKSLASH_STR );
            strcat( pszObjNameBuf, PROPDIR );
            *pszName = BACKSLASH;
            strcat( pszObjNameBuf, pszName );

            // replace drive letter with system drive letter
            {
              CHAR szPrimaryDrive[20];
              UtlQueryString( QST_PRIMARYDRIVE, szPrimaryDrive, sizeof(szPrimaryDrive) );
              *pszObjNameBuf = szPrimaryDrive[0];
            }
          }
          else
          {
            // supplied object name is invalid
            fOK = FALSE;
          } /* endif */
        }

        // load folder property file
        if ( fOK )
        {
          fOK = UtlLoadFileL( pszObjNameBuf, (PVOID *)&pProp, &ulLen, FALSE, FALSE );
        } /* endif */

        // extract folder name
        if ( fOK )
        {
          if ( pProp->szLongName[0] )
          {
            strcpy( pszName, pProp->szLongName );
          }
          else
          {
            Utlstrccpy( pszName, pProp->PropHead.szName, DOT );
          } /* endif */
        } /* endif */

      }
      else
      {
        // process subfolder

        // load subfolder property file and get parent ID
        if ( fOK )
        {
          ULONG ulLen;
          fOK = UtlLoadFileL( pszObjName, (PVOID *)&pProp, &ulLen, FALSE, FALSE );
        } /* endif */

        // recursively call function again to get the name of the parent folder
        if ( fOK )
        {
          // build main folder object name
          {
            PSZ pszEndOfFolder;

            strcpy( pszObjNameBuf, pszObjName );
            pszEndOfFolder = strrchr( pszObjNameBuf, BACKSLASH );
            if ( pszEndOfFolder )
            {
              *pszEndOfFolder = EOS;
              pszEndOfFolder = strrchr( pszObjNameBuf, BACKSLASH );
            } /* endif */
            if ( pszEndOfFolder )
            {
              *pszEndOfFolder = EOS;
            }
            else
            {
              fOK = FALSE;           // invalid subfolder object name
            } /* endif */
          }


          if ( pProp->ulParentFolder )
          {
            // there are parent subfolders, call function to get parent subfolder name

            // build name of parent subfolder (main folder name already in buffer)
            sprintf( pszObjNameBuf + strlen(pszObjNameBuf), "\\%s\\%8.8ld%s",
                     PROPDIR, pProp->ulParentFolder, EXT_OF_SUBFOLDER );

            // recursively call for parent subfolder name
            if ( fOK )
            {
              fOK =  SubFolObjectNameToName( pszObjNameBuf, pszName );
            } /* endif */
          }
          else
          {
            // no more parent folder, call function to get main folder name

            // recursively call for parent folder name
            if ( fOK )
            {
              fOK =  SubFolObjectNameToName( pszObjNameBuf, pszName );
            } /* endif */
          } /* endif */
        } /* endif */

        // concatenate name of this subfolder to name already in buffer
        if ( fOK )
        {
          strcat( pszName, BACKSLASH_STR );
          strcat( pszName, pProp->szLongName );
        } /* endif */
      } /* endif */
    } /* endif */
  } /* endif */

  // cleanup
  if ( pProp ) UtlAlloc( (PVOID *)&pProp, 0L, 0L, NOMSG );
  if ( pszObjNameBuf ) UtlAlloc( (PVOID *)&pszObjNameBuf, 0L, 0L, NOMSG );

  return( fOK );
} /* end of function SubFolObjectNameToName*/


// Private data structure used by function SubFolNameToObjectName
typedef struct _SUBFOLNAMETOOBJECTNAMEDATA
{
  PROPFOLDER  Prop;                    // buffer for folder properties
  CHAR        szObjName[MAX_EQF_PATH]; // buffer for object names
  CHAR        szPropName[MAX_EQF_PATH];// buffer for property file names
  CHAR        szShortName[MAX_FILESPEC]; // buffer for folder short name
  CHAR        szFolPropPath[MAX_EQF_PATH]; // buffer for folder property directory path
  CHAR        szSearchPath[MAX_EQF_PATH]; // buffer for subfolder search path
  FILEFINDBUF stFileFindBuf;           // file find buffer
} SUBFOLNAMETOOBJECTNAMEDATA, *PSUBFOLNAMETOOBJECTNAMEDATA;

// convert a folder/subFolder name to a subfolder object name
BOOL SubFolNameToObjectName( PSZ pszName, PSZ pszObjName )
{
  BOOL        fOK = TRUE;              // function return code
  PSZ         pszEndOfName = NULL;     // points to end of currently processed name
  PSUBFOLNAMETOOBJECTNAMEDATA pData = NULL; // ptr to private data area
  ULONG       ulCurParentID = 0;       // ID of current parent
  CHAR        chDrive = EOS;

  // allocate private data area
  fOK = UtlAlloc( (PVOID *)&pData, 0L, sizeof(SUBFOLNAMETOOBJECTNAMEDATA), NOMSG );

  // start with folder name ...
  if ( fOK )
  {
    BOOL fIsNew;

    // isolate folder name
    pszEndOfName = strchr( pszName, BACKSLASH );
    if ( pszEndOfName ) *pszEndOfName =EOS;

    // get folder short name
    ObjLongToShortName( pszName, pData->szShortName, FOLDER_OBJECT, &fIsNew );
    if ( fIsNew )
    {
      fOK = FALSE;                     // folder does not exist
    } /* endif */

    // build folder property file name
    if ( fOK )
    {
      strcat( pData->szShortName, EXT_FOLDER_MAIN );
      UtlMakeEQFPath( pData->szObjName, SYSTEM_PATH, NULC, NULL );
      strcat( pData->szObjName, BACKSLASH_STR );
      strcat( pData->szObjName, pData->szShortName );
    } /* endif */

    // load folder properties to get correct drive letter
    if ( fOK )
    {
//      PVOID pvTemp = &pData->Prop;
//      ULONG ulLen = sizeof(pData->Prop);
//      fOK = UtlLoadFileL( pData->szObjName, &pvTemp, &ulLen, FALSE, TRUE );
        HPROP   hFolProp;
        EQFINFO ErrorInfo;
        hFolProp = OpenProperties( pData->szObjName, NULL, PROP_ACCESS_READ, &ErrorInfo);
        if (hFolProp)
        {
			PPROPFOLDER pProp;
			pProp = (PPROPFOLDER)MakePropPtrFromHnd( hFolProp );
			chDrive = pProp->chDrive;
			CloseProperties( hFolProp, PROP_QUIT, &ErrorInfo);
	    }
	    else
	    {
			fOK = FALSE;
	    }

    } /* endif */

    // build folder object and path name of folders property directory
    if ( fOK )
    {
      UtlMakeEQFPath( pData->szObjName, chDrive /*pData->Prop.chDrive*/, SYSTEM_PATH, pData->szShortName );
      UtlMakeEQFPath( pData->szPropName, chDrive /*pData->Prop.chDrive*/, PROPERTY_PATH, pData->szShortName );
      strcpy( pData->szFolPropPath, pData->szPropName );
    } /* endif */
  } /* endif */

  // look for current subfolder
  if ( fOK )
  {
    while ( fOK && pszEndOfName )      // while OK and miore subfolders to follow ...
    {
      // isolate next subfolder name
      PSZ pszSubFolder = pszEndOfName + 1;
      *pszEndOfName = BACKSLASH;
      pszEndOfName = strchr( pszSubFolder, BACKSLASH );
      if ( pszEndOfName ) *pszEndOfName = EOS;

      // search all subfolders for given name
      {
        HDIR hdir = HDIR_CREATE;
        USHORT usCount = 1;
        USHORT usRC;
        BOOL   fFound = FALSE;

        // setup subfolder search path
        sprintf( pData->szSearchPath, "%s\\*%s", pData->szFolPropPath, EXT_OF_SUBFOLDER );

        // loop over all subfolders
        usRC = UtlFindFirst( pData->szSearchPath, &hdir, FILE_NORMAL, &(pData->stFileFindBuf),
                             sizeof(pData->stFileFindBuf), &usCount, 0L, FALSE );
        while ( !fFound && fOK && (usRC == NO_ERROR) && usCount )
        {
          // load subfolders property file and check parent ID and name
          {
            PVOID pvTemp = &pData->Prop;
            ULONG ulLen = sizeof(pData->Prop);

            sprintf( pData->szObjName, "%s\\%s", pData->szFolPropPath, pData->stFileFindBuf.cFileName );

            fOK = UtlLoadFileL( pData->szObjName, &pvTemp, &ulLen, FALSE, TRUE );
            if ( fOK )
            {
              if ( (pData->Prop.ulParentFolder == ulCurParentID) &&
                   (strcmp( pData->Prop.szLongName, pszSubFolder ) == 0) )
              {
                fFound = TRUE;
                ulCurParentID = pData->Prop.ulSubFolderID;
              } /* endif */
            } /* endif */
          }

          // try next subfolder
          if ( !fFound )
          {
            usRC = UtlFindNext( hdir, &(pData->stFileFindBuf), sizeof(pData->stFileFindBuf),
                                &usCount, 0);
          } /* endif */
        } /* endwhile */

        // close search file handle
        if ( hdir != HDIR_CREATE ) UtlFindClose( hdir, FALSE );

        // end loop if subfolder was not found
        if ( !fFound ) fOK = FALSE;
      }
    } /* endwhile */
  } /* endif */

  // cleanup
  if ( fOK ) strcpy( pszObjName, pData->szObjName );
  if ( pData ) UtlAlloc( (PVOID *)&pData, 0L, 0L, NOMSG );

  return( fOK );
} /* end of function SubFolNameToObjectName */


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

// check if the given object name is the object name of a subfolder
BOOL FolIsSubFolderObject( PSZ pszObjName )
{
  // subfolders object names have an extention of .F01 thereas folders have an
  // exten of .F00, subfolders have the property directory between the folder
  // directory and the subfolder name
  BOOL fSubFolder = FALSE;
  PSZ pszSubFolExt = EXT_OF_SUBFOLDER;
  PSZ pszPropDir = PROPDIR;
  int iLen= strlen(pszSubFolExt );

  if ( strcmp( pszObjName + strlen(pszObjName) - iLen, pszSubFolExt ) == 0 )
  {
    // extention is O.K. now check the property directory name
    PSZ pszPropDirPos = strchr( pszObjName, BACKSLASH );
    if ( pszPropDirPos ) pszPropDirPos = strchr( pszPropDirPos+1, BACKSLASH );
    if ( pszPropDirPos ) pszPropDirPos = strchr( pszPropDirPos+1, BACKSLASH );
    if ( pszPropDirPos )
    {
      iLen = strlen(pszPropDir);
      if ( (strncmp( pszPropDirPos+1, pszPropDir, iLen ) == 0) &&
           (pszPropDirPos[iLen+1] == BACKSLASH) )
      {
        fSubFolder = TRUE;
      } /* endif */
    } /* endif */
  } /* endif */
  return( fSubFolder );
} /* end of function FolIsSubFolderObject */

// function to get the ID of a folder or subfolder using the object name
// for folders an ID of 0 is returned
ULONG FolGetSubFolderIdFromObjName( PSZ pszObjName )
{
  ULONG ulID = 0L;                     // subfolder ID, default is zero

  if ( FolIsSubFolderObject( pszObjName ) )
  {
    // position to subfolder name within object name
    PSZ pszName = strrchr( pszObjName, BACKSLASH );
    if ( pszName )
    {
      PSZ pszExtention = strchr( pszName, DOT );
      if ( pszExtention )
      {
        *pszExtention = EOS;
        ulID = atol( pszName + 1 );
        *pszExtention = DOT;
      } /* endif */
    } /* endif */
  } /* endif */

  return( ulID );
} /* end of function FolGetSubFolderIdFromObjName */

// convert a main folder object name and a subfolder ID to a subfolder object name
BOOL SubFolIdToObjectName( PSZ pszMainFolderObjName, ULONG ulID, PSZ pszObjName )
{
  BOOL        fOK = TRUE;              // function return code

  sprintf( pszObjName, "%s\\%s\\%8.8ld%s", pszMainFolderObjName, PROPDIR, ulID, EXT_OF_SUBFOLDER );

  return( fOK );
} /* end of function SubFolIdToObjectName */


// create the subfolder information table for the given folder
BOOL SubFolCreateInfoTable( PSZ pszFolderObjName, PSUBFOLINFO *ppInfo )
{
  FILEFINDBUF stResultBuf;             // DOS file find struct
  USHORT      usCount = 1;             // number of files requested
  USHORT      usRC;                    // return code of called functions
  HDIR        hDirHandle = HDIR_CREATE;// DosFind routine handle

  PSZ          pszPath;
  PSZ          pszFileName = NULL;     // subfolder properties file name
  PSZ          pszSearchName = NULL;   // subfolder properties search path
  PPROPFOLDER  pProp = NULL;           // subfolder properties
  PSZ  pszName = RESBUFNAME(stResultBuf);// address name field in result buffer
  BOOL        fOK = TRUE;              // function return code
  int         iCurEntry = 0;           // current entry in info table
  int         iTableSize = 0;          // current size (elements) of info table
  PSUBFOLINFO pInfo = NULL;            // ptr to subfolder info table



  fOK = UtlAlloc( (PVOID *)&pszPath, 0L, (LONG) (3 * MAX_PATH144), ERROR_STORAGE );
  if ( fOK )
  {
    pszFileName = pszPath + MAX_PATH144;
    pszSearchName = pszFileName + MAX_PATH144;
  } /* endif */

  // allocate initial subfolder info table with minimal size (for folders which have no subfolders)
  if ( fOK )
  {
    int iNewSize = 1;
    fOK = UtlAlloc( (PVOID *)&pInfo, (iTableSize * sizeof(SUBFOLINFO)),
                    (iNewSize * sizeof(SUBFOLINFO)), NOMSG );
    if ( fOK ) iTableSize = iNewSize;
  } /* endif */

  // build subfolder search path
  if ( fOK )
  {
    strcpy( pszSearchName, pszFolderObjName );
    if ( FolIsSubFolderObject( pszFolderObjName ) )
    {
      UtlSplitFnameFromPath( pszSearchName );  // cut off subfolder name
      UtlSplitFnameFromPath( pszSearchName );  // cut off property directory
    } /* endif */
    strcpy( pszPath, pszSearchName );
    strcat( pszSearchName, BACKSLASH_STR );
    strcat( pszSearchName, PROPDIR );
    strcat( pszSearchName, "\\*" );
    strcat( pszSearchName, EXT_OF_SUBFOLDER );
  } /* endif */

  if ( fOK )
  {
    usRC = UtlFindFirst( pszSearchName, &hDirHandle, FILE_NORMAL, &stResultBuf, sizeof( stResultBuf), &usCount, 0L, 0);
    while ( (usRC == NO_ERROR) && usCount )
    {
      ULONG ulLen;

      // load subfolder properties
      sprintf( pszFileName, "%s\\%s\\%s", pszPath, PROPDIR, pszName );
      if ( UtlLoadFileL( pszFileName, (PVOID *)&pProp, &ulLen, FALSE, FALSE ) )
      {
        // enlarge info table if necessary
        if ( (iCurEntry + 1) >= iTableSize )
        {
          int iNewSize = iTableSize + 20;

          fOK = UtlAlloc( (PVOID *)&pInfo, (iTableSize * sizeof(SUBFOLINFO)),
                          (iNewSize * sizeof(SUBFOLINFO)), NOMSG );
          if ( fOK ) iTableSize = iNewSize;
        } /* endif */

        // add current subfolder to table
        if ( fOK )
        {
          strcpy( pInfo[iCurEntry].szName,  pProp->szLongName );
          pInfo[iCurEntry].ulParentFolder = pProp->ulParentFolder;
          pInfo[iCurEntry].ulID           = pProp->ulSubFolderID;
          iCurEntry++;
        } /* endif */
        UtlAlloc( (PVOID *)&pProp, 0L, 0L, NOMSG );
        pProp = NULL;
      } /* endif */

      usRC = UtlFindNext( hDirHandle, &stResultBuf, sizeof( stResultBuf),
                          &usCount, 0);
    } /* endwhile */
  } /* endif */

  // close search file handle
  if ( hDirHandle != HDIR_CREATE ) UtlFindClose( hDirHandle, FALSE );

  UtlAlloc( (PVOID *)&pszPath, 0L, 0L, NOMSG );

  if ( fOK )
  {
    *ppInfo = pInfo;
  }
  else
  {
    *ppInfo = NULL;
    if ( pInfo ) UtlAlloc( (PVOID *)&pInfo, 0L, 0L, NOMSG );
  } /* endif */

  return( fOK );

} /* end of function SubFolCreateInfoTable */

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


//--------------------------- End of EQFFOL00.C --------------------------------
