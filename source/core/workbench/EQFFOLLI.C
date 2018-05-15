/*! \file
	Description: EQF Folder Handler - List Window Code

	Copyright Notice:

	Copyright (C) 1990-2017, International Business Machines
	Corporation and others. All rights reserved
*/

#define INCL_EQF_FOLDER           // folder list and document list functions
#define INCL_EQF_TAGTABLE         // tag table and format functions
#define INCL_EQF_DLGUTILS         // dialog utilities
#define INCL_EQF_ANALYSIS         // Analysis functions
#include <eqf.h>                       // General Translation Manager include file
#include <eqfstart.h>

#include "eqfdde.h"               // batch mode functions
#include "eqffol00.h"             // Folder Handler defines
#include "eqffll00.h"             // Folder list defines
#include "eqfstart.id"            // EQFSTARR resource IDs
#include "eqffol.id"              // Folder Handler IDs
#include "eqfcolw.id"             // column width IDs

//cv
#include "eqfdoc01.h"

static CHAR ColHdr[MAX_DEFINEDCOLUMNS][80];       // Buffer for column header texts
static CLBCOLDATA ColTable[] =
{ { "",               1,              TEXT_DATA,      DT_LEFT},        // object name
  { ColHdr[1],  CLB_MAX_DOC_LENGTH,   AUTOWIDTHTEXT_DATA, DT_LEFT},    // name
  { "",         CLB_MAX_FNAME,        TEXT_DATA,      DT_LEFT},        // format
  { ColHdr[3],  CLB_MAX_DATE,         DATE_DATA,      DT_LEFT},        // translated
  { ColHdr[4],  CLB_MAX_DATE,         DATE_DATA,      DT_LEFT},        // segmented
  { ColHdr[5],  CLB_MAX_DATE,         DATE_DATA,      DT_LEFT},        // exported
  { ColHdr[6],  CLB_MAX_DATE,         DATE_DATA,      DT_LEFT},        // imported
//  { ColHdr[7],  CLB_MAX_DATE,         DATE_DATA,      DT_LEFT},        // touched
  { ColHdr[7],  CLB_MAX_DOC_LENGTH,   TEXT_DATA,      DT_LEFT},        // translator
  { ColHdr[8],  CLB_MAX_DATE_TIME,    DATETIME_DATA,  DT_LEFT},        // touched date/time
  { ColHdr[9],  CLB_MAX_SIZE_LENGTH,  NUMERIC_DATA,   DT_RIGHT},       // size
  { ColHdr[10], CLB_MAX_SEG_NUM,      NUMERIC_DATA,   DT_RIGHT},       // completed
  { ColHdr[11], CLB_MAX_SEG_NUM,      NUMERIC_DATA,   DT_RIGHT},       // modified
//{ ColHdr[12], CLB_MAX_SEG_NUM,      NUMERIC_DATA,   DT_RIGHT},       // scratch
//{ ColHdr[13], CLB_MAX_SEG_NUM,      NUMERIC_DATA,   DT_RIGHT},       // copied
  { ColHdr[12], CLB_MAX_SEG_NUM,      TEXT_DATA,      DT_RIGHT},       // shipment
  { "",         CLB_MAX_SEG_NUM,      NUMERIC_DATA,   DT_RIGHT},       // currently not in use
  { ColHdr[14], CLB_MAX_FNAME,        TEXT_DATA,      DT_LEFT},        // format
  { ColHdr[15], CLB_MAX_FNAME, AUTOWIDTHTEXT_DATA,    DT_LEFT},        // memory
  { ColHdr[16], CLB_MAX_LANG_LENGTH,  TEXT_DATA,      DT_LEFT},        // source language
  { ColHdr[17], CLB_MAX_LANG_LENGTH,  TEXT_DATA,      DT_LEFT},        // target language
  { ColHdr[18], CLB_MAX_FNAME,        TEXT_DATA,      DT_LEFT},        // editor
  { ColHdr[19], CLB_MAX_DOC_LENGTH,   TEXT_DATA,      DT_LEFT},        // alias
  { ColHdr[20], CLB_MAX_DATE_TIME,    DATETIME_DATA,  DT_LEFT},        // source date
  { ColHdr[21], CLB_MAX_FNAME,        TEXT_DATA,      DT_LEFT},        // name wo path info
  { ColHdr[22], CLB_MAX_FNAME,        TEXT_DATA,      DT_LEFT},        // file name extension
  { NULL,       0,                    TEXT_DATA,            0}};

static SHORT sLastUsedView[MAX_VIEW+1] = { FOL_NAME_IND, CLBLISTEND};
static SHORT sDefaultView[MAX_VIEW+1]  = { FOL_NAME_IND, CLBLISTEND};
static SHORT sNameView[MAX_VIEW+1]     = { FOL_NAME_IND, CLBLISTEND};
static SHORT sDetailsView[MAX_VIEW+1]  = { FOL_NAME_IND, FOL_TRANSLATED_IND, CLBLISTEND};
static SHORT sSortCriteria[MAX_VIEW+1] = { FOL_NAME_IND, CLBLISTEND};

static CLBCTLDATA FolCLBData =
{  sizeof(CLBCTLDATA),                // size of control structure
  21,                                 // we have max 20 data columns
  1,                                  // two character space between columns
  SYSCLR_WINDOWSTATICTEXT,            // paint title in color of static text
  SYSCLR_WINDOW,                      // background is normal window background
  SYSCLR_WINDOWTEXT,                  // paint item in color of window text
  SYSCLR_WINDOW,                      // background is normal window background
  '\x15',                             // use X15 character as data seperator
  sLastUsedView,                      // set current (= last used) view list
  sDefaultView,                       // set default view list
  sDetailsView,                       // set user set details view list
  sNameView,                          // set view list for 'name' view option
  sSortCriteria,                      // set sort criteria list
  ColTable};                          // set address of column definition table

static BOOL  CheckPropertyExist( PSZ pData );
MRESULT FolderCreateCall( PLISTCOMMAREA pCommArea );
int FolCountSelectedElements
(
  HWND hwndLB,
  PSZ  pszBuffer,
  int  *piSubFolders,
  int  *piDocs
);

/**********************************************************************/
/* Handler callback function for folder handler                       */
/**********************************************************************/
MRESULT FolderHandlerCallBack
(
PHANDLERCOMMAREA pCommArea,
HWND             hwnd,
WINMSG           message,
WPARAM           mp1,
LPARAM           mp2
)
{
  MRESULT          mResult = MRFROMSHORT(FALSE);

  switch ( message )
  {
  /******************************************************************/
  /* WM_CREATE: fill variables of communication area                */
  /******************************************************************/
  case WM_CREATE :
    pCommArea->pfnCallBack          = FolderCallBack;
    strcpy( pCommArea->szHandlerName, FOLDERHANDLER );
    pCommArea->sBaseClass           = clsFOLDER;
    pCommArea->sListWindowID        = ID_FOLDER_WINDOW;
    pCommArea->sListboxID           = ID_FOLDER_LB;
    pCommArea->asNotifyClassList[0] = clsFOLDER;
    pCommArea->asNotifyClassList[1] = clsFOLDEREXP;
    pCommArea->asNotifyClassList[2] = clsFOLDERIMP;
    pCommArea->asNotifyClassList[3] = 0;       // end of list
    break;

  case WM_EQF_SHUTDOWN:
    if ( pCommArea->fUserFlag )    // user flag is used as must-not-close flag
    {
      mResult = MRFROMSHORT( TRUE );
    } /* endif */
    break;

  case WM_EQFN_TASKDONE:
    {
      HWND hwndImportInstance = GetFolderImportHandle();

      // post task done notification to import instance
      if ( hwndImportInstance )
      {
        WinPostMsg( hwndImportInstance, message, mp1, mp2 );
      } /* endif */
    }
    break;

  case WM_EQF_DELETE:
    {
      BOOL fOK = FolDeleteFolder( HWNDFROMMP1(mp1), (PSZ) PVOIDFROMMP2(mp2), NULL );
      mResult = MRFROMSHORT(!fOK);
    }
    break;

  case WM_EQF_OPEN:
    {
      HWND       hwndObj;
      PSZ        pszObj = (PSZ) PVOIDFROMMP2(mp2);

      PPROPSYSTEM pPropSys = GetSystemPropPtr();
      if ( !pPropSys->fUseIELikeListWindows )
      {
        /**************************************************************/
        /* open only if not new IELikeListWindow                      */
        /**************************************************************/
        if ( (hwndObj = EqfQueryObject( pszObj, clsFOLDER, 0)) != NULLHANDLE )
        {
          ActivateMDIChild( hwndObj );
          mResult = MRFROMSHORT( TRUE );
        }
        else
        {
          SHORT  sRC = QUERYSYMBOL( pszObj );

          if ( sRC != -1 )
          {
            PSZ   pszParm;
            SubFolObjectNameToName( pszObj, pCommArea->szBuffer );
            OEMTOANSI(pCommArea->szBuffer);
            pszParm = pCommArea->szBuffer;
            UtlError( ERROR_FOLDER_LOCKED, MB_CANCEL, 1, &pszParm, EQF_INFO );
            mResult = MRFROMSHORT(TRUE);
          }
          else
          {
            BOOL fOK = TRUE;
            if ( FolIsSubFolderObject( pszObj ) )
            {
              fOK = UtlFileExist( pszObj );
            }
            else
            {
              fOK = CheckPropertyExist( pszObj );
            } /* endif */
            if ( fOK )
            {
              mResult = WinSendMsg( hwnd, WM_EQF_CREATELISTWINDOW, mp1, mp2 );
            }
            else
            {
              mResult = MRFROMSHORT(TRUE);
            } /* endif */
          } /* endif */
        } /* endif */
      }
      else
      {
        /**************************************************************/
        /* ignore request for IELikeListWindows                       */
        /**************************************************************/
      } /* endif */
    }
    break;

  case WM_EQF_CREATE:
    {
      PSZ        pszObj = (PSZ) PVOIDFROMMP2(mp2);
      OBJNAME    szObjName;
      HWND       hwndObj;
      SHORT      sRC;
      BOOL       fSubFolder = FALSE;

      // check if we have to create a main folder or a subfolder
      // for subfolder creation the supplied object name is the name
      // of the parent folder or parent subfolder without any path information
      if ( pszObj && strncmp( pszObj, SUBFOLDERPREFIX, strlen(SUBFOLDERPREFIX) ) == 0 )
      {
        // the supplied name is the parent object name for subfolder creation
        fSubFolder = TRUE;
      } /* endif */

      if ( fSubFolder )
      {
        szObjName[0] = NULC;

        sRC = FolCreateSubFolder( pszObj + strlen(SUBFOLDERPREFIX));
        if ( sRC )
        {
          if ( sRC != DID_CANCEL)
          {
            UtlError( ERROR_CREATE_FOLDER, MB_CANCEL, 0, (PSZ *) NULP, EQF_ERROR );
          } /* endif */
          mResult = MRFROMSHORT( TRUE );
        } /* endif */
    }
      else if ( pszObj && (( hwndObj = EqfQueryObject( pszObj, clsFOLDER, 0) ) != NULL))
      {
        UtlError( ERROR_FOLDER_EXISTS, MB_CANCEL, 0, &pszObj, EQF_ERROR );
        mResult = MRFROMSHORT( TRUE );
      }
      else
      {
        if ( pszObj )
        {
          strcpy( szObjName, pszObj);  // use given name
        }
        else
        {
          szObjName[0] = NULC;        // allow a new name
        } /* endif */
        sRC = CreateFolder( hwnd, &(pCommArea->fUserFlag), szObjName);
        if ( sRC )
        {
          if ( sRC != DID_CANCEL)
          {
            UtlError( ERROR_CREATE_FOLDER, MB_CANCEL, 0, (PSZ *) NULP, EQF_ERROR );
          } /* endif */
          mResult = MRFROMSHORT( TRUE );
        }
        else
        {
          EqfSend2AllHandlers( WM_EQFN_CREATED,
                               MP1FROMSHORT( clsFOLDER ),
                               MP2FROMP(szObjName) );
          if ( SHORT1FROMMP1( mp1 ) )
          {
            WinSendMsg( hwnd, WM_EQF_OPEN, NULL, szObjName);
          } /* endif */
        } /* endif */
      } /* endif */
    }
    break;

  case WM_EQF_INSERTNAMES:
    mResult = MRFROMSHORT( LoadDocumentNames( (PSZ) PVOIDFROMMP2( mp2 ),
                                              HWNDFROMMP1( mp1 ),
                                              LOADDOCNAMES_INCLSUBFOLDERS,
                                              pCommArea->szBuffer ) );
    break;

  case WM_EQF_QUERYSELECTEDNAMES:
    {
      HWND       hclient;

      if ( (hclient = EqfQueryObject( (PSZ) PVOIDFROMMP2( mp2 ), clsFOLDER, 0)) != NULLHANDLE )
      {
        mResult = WinSendMsg( hclient, message, mp1, mp2 );
      }
      else
      {
        mResult = MRFROMSHORT( 0 ); // no files, as folder is not open
      } /* endif */
    }
    break;

  case WM_EQF_PROCESSTASK:
    switch ( SHORT1FROMMP1( mp1 ) )
    {
    HWND       hwndObj;

    case PID_FILE_MI_EXPORT:
      {
        HWND hwndExportInstance = GetFolderExportHandle();
        if ( hwndExportInstance )
        {
          ActivateMDIChild( hwndExportInstance );
        }
        else if ( (hwndObj = EqfQueryObject( (PSZ)mp2, clsFOLDER, 0)) != NULLHANDLE )
        {
          PSZ        pszParm;
          Utlstrccpy( pCommArea->szBuffer, UtlGetFnameFromPath( (PSZ)mp2 ), DOT );
          ObjShortToLongName( pCommArea->szBuffer, pCommArea->szBuffer, FOLDER_OBJECT );
          OEMTOANSI(pCommArea->szBuffer);
          pszParm = pCommArea->szBuffer;
          UtlError( ERROR_EXPORT_OPEN_FOLDER, MB_CANCEL, 1, &pszParm, EQF_ERROR );
          ActivateMDIChild( hwndObj );
          mResult = MRFROMSHORT(TRUE);
        }
        else
        {
          FolderExport( hwnd, (PSZ) mp2, NULL );
        } /* endif */
      }
      break;

    case PID_FILE_MI_IMPORT:
      {
        HWND hwndImportInstance = GetFolderImportHandle();
        if ( hwndImportInstance )
        {
          ActivateMDIChild( hwndImportInstance );
        }
        else
        {
          FolderImport( hwnd, NULL );
        } /* endif */
      }
      break;

    //case PID_UTILS_MI_DELEXPMAT:
    //  DeleteExportedFolder();
    //  break;

    case RENAMEOBJECT_TASK:
      mResult = (MRESULT)FolRenameFolder( (PSZ)PVOIDFROMMP2(mp2), TRUE );
      break;

    default:
      mResult = MRFROMSHORT( FALSE );   // indicate an error
    } /* endswitch */
    break;

  case  WM_EQF_DDE_REQUEST:
    /************************************************************/
    /*     mp1:  (DDETASK) Task                                 */
    /*     mp2:  (PVOID) pTaskIda                               */
    /************************************************************/
    switch ( SHORT1FROMMP1( mp1 ) )
    {
    case  TASK_FLDCRT:
      {
        PDDEFOLCRT pFolCrt = (PDDEFOLCRT)PVOIDFROMMP2(mp2);
        FolBatchFolderCreate( hwnd, pFolCrt );
      }
      break;
    case  TASK_FLDEXP:
      {
        PDDEFOLEXP pFolExp = (PDDEFOLEXP)PVOIDFROMMP2(mp2);
        FolBatchFolderExport( hwnd, pFolExp );
      }
      break;
    case  TASK_FLDIMP:
      {
        PDDEFOLIMP pFolImp = (PDDEFOLIMP)PVOIDFROMMP2(mp2);
        FolBatchFolderImport( hwnd, pFolImp );
      }
      break;
    case  TASK_FLDDEL:
      {
        PDDEFOLDEL pFolDel = (PDDEFOLDEL)PVOIDFROMMP2(mp2);
        FolBatchFolderDelete( pFolDel );
      }
      break;
    default :
      break;
    } /* endswitch */
    break;
  case WM_DESTROY:
    /****************************************************************/
    /* Nothing to do, as nothing has been allocated by the folder   */
    /* handler callback function                                    */
    /****************************************************************/
    break;

  default:
    break;
  } /* endswitch */
  return( mResult );
} /* end of function FolderHandlerCallBack */

/**********************************************************************/
/* List instance callback function for folder window (document list)  */
/**********************************************************************/
MRESULT FolderCallBack
(
PLISTCOMMAREA    pCommArea,
HWND             hwnd,
WINMSG           message,
WPARAM           mp1,
LPARAM           mp2
)
{
  MRESULT          mResult = MRFROMSHORT(FALSE);
  EQF_BOOL  fAFCFolderRestrict;   // Restricted AFC folder

  UINT uiFormat = UtlQueryUShort(QS_CLIPBOARDFORMAT);

  switch ( message )
  {
  case WM_CREATE :
    FolderCreateCall( pCommArea );
    break;

  case WM_EQF_COLUMNLIST:
    if ( mp2 )
    {
      mResult = FolderCreateCall( (PLISTCOMMAREA)mp2 );
    }
    break;

  case WM_CLOSE :
  case WM_EQF_TERMINATE :
    /**************************************************************/
    /* Save view lists for WM_EQF_TERMINATE only if save flag is  */
    /* on                                                         */
    /**************************************************************/
    if ( (message == WM_CLOSE) || (SHORT1FROMMP1(mp1) & TWBSAVE) )
    {

      if ( FolIsSubFolderObject( pCommArea->szObjName ) )
      {
        ULONG ulLen;
        PPROPFOLDER pProp = NULL;            // loaded property file

        if ( UtlLoadFileL( pCommArea->szObjName , (PVOID *) &pProp, &ulLen, FALSE, FALSE ) )
        {
          memcpy( &pProp->Swp, &(pCommArea->swpSizePos), sizeof(EQF_SWP) );
          memcpy( pProp->sLastUsedViewList, pCommArea->asCurView,
                  sizeof(pProp->sLastUsedViewList) );


          memcpy( (pProp->sLastUsedViewWidth), pCommArea->asCurViewWidth,
                  sizeof( pProp->sLastUsedViewWidth ));

          memcpy( pProp->sDetailsViewList, pCommArea->asDetailsView,
                  sizeof(pProp->sDetailsViewList) );
          memcpy( pProp->sSortList, pCommArea->asSortList,
                  sizeof(pProp->sSortList) );
          memcpy( &pProp->Filter, &pCommArea->Filter,
                  sizeof(pProp->Filter) );
          UtlWriteFileL( pCommArea->szObjName, ulLen, pProp, FALSE );
        } /* endif */
        if ( pProp ) UtlAlloc( (PVOID *) &pProp, 0L, 0L, NOMSG );
      }
      else
      {
        PPROPFOLDER   pProp;
        EQFINFO       ErrorInfo;
        HPROP         hProp;

        /**************************************************************/
        /* Replace drive letter in object name with drive letter      */
        /* of primary drive in order to build property name           */
        /**************************************************************/
        UtlQueryString( QST_PRIMARYDRIVE, pCommArea->szBuffer,
                        sizeof(pCommArea->szBuffer) );
        strcpy( pCommArea->szBuffer + 1, pCommArea->szObjName + 1);
        /**************************************************************/
        /* Open properties                                            */
        /**************************************************************/
        hProp = OpenProperties( pCommArea->szBuffer, NULL,
                                PROP_ACCESS_READ, &ErrorInfo );
        if ( hProp )
        {
          /************************************************************/
          /* Save current view lists and window position              */
          /************************************************************/
          if ( SetPropAccess( hProp, PROP_ACCESS_WRITE ) )
          {
            pProp = (PPROPFOLDER) MakePropPtrFromHnd( hProp);
            memcpy( &pProp->Swp, &(pCommArea->swpSizePos), sizeof(EQF_SWP) );
            memcpy( pProp->sLastUsedViewList, pCommArea->asCurView,
                    sizeof(pProp->sLastUsedViewList) );

            memcpy( (pProp->sLastUsedViewWidth), (pCommArea->asCurViewWidth),
                    sizeof( pProp->sLastUsedViewWidth ));

            memcpy( pProp->sDetailsViewList, pCommArea->asDetailsView,
                    sizeof(pProp->sDetailsViewList) );
            memcpy( pProp->sSortList, pCommArea->asSortList,
                    sizeof(pProp->sSortList) );
            memcpy( &pProp->Filter, &pCommArea->Filter,
                    sizeof(pProp->Filter) );
            SaveProperties( hProp, &ErrorInfo );
          } /* endif */
          CloseProperties( hProp, PROP_QUIT, &ErrorInfo );
        } /* endif */
      } /* endif */
    } /* endif */
    break;

  case WM_DESTROY:
    /****************************************************************/
    /* Free all resource allocated by list instance callback        */
    /* function                                                     */
    /* nothing to do for folder                                     */
    /****************************************************************/
    break;

  case WM_EQF_INITIALIZE:
    /****************************************************************/
    /* Fill column listbox                                          */
    /****************************************************************/
    LoadDocumentNames( pCommArea->szObjName, pCommArea->hwndLB, TRUE,
                       pCommArea->szBuffer );
    FolLoadSubFolderNames( pCommArea->szObjName, pCommArea->hwndLB,
                           LOADSUBFOLNAMES_ITEMTEXT,
                           pCommArea->szBuffer );

    // show global memory option page for this folder when not shown yet 
    // and the folder isn't currently being imported
    if ( FolderListGlobalMemoryCheckRequired( pCommArea->szObjName ) )
    {
      CHAR szFolderListObjName[MAX_EQF_PATH+MAX_EQF_PATH];
      HWND hwndFolderList;

      // check if folder is currently being imported
      strcpy( szFolderListObjName, FOLIMPOBJPREFIX );
      strcat( szFolderListObjName, pCommArea->szObjName );
      hwndFolderList = EqfQueryObject( szFolderListObjName, clsFOLDERIMP, 0);
      if ( hwndFolderList == NULLHANDLE )
      {
        UtlMakeEQFPath( szFolderListObjName, NULC, SYSTEM_PATH, NULL  );
        strcat( szFolderListObjName, BACKSLASH_STR );
        strcat( szFolderListObjName, DEFAULT_FOLDERLIST_NAME );
        hwndFolderList = EqfQueryObject( szFolderListObjName, clsFOLDERLIST, 0);
        if ( hwndFolderList != NULLHANDLE )
        {
          PostMessage( hwndFolderList , WM_COMMAND, MP1FROMSHORT( PID_FILE_MI_PROPERTIES ), 0L);
        } /* endif */         
      } /* endif */         
    } /* endif */      

    break;

  case WM_EQF_BUILDITEMTEXT :
    /****************************************************************/
    /* Setup item text for the object passed in mp2 parameter       */
    /****************************************************************/

    /****************************************************************/
    /* First of all check if item belongs to our list ...           */
    /****************************************************************/
    strcpy( pCommArea->szBuffer, (const char *) PVOIDFROMMP2(mp2) );


    // Check if main folder part of object name is the same
    {
      // cut off anything following the folder name in the object name
      PSZ pszEnd = strchr( pCommArea->szBuffer, BACKSLASH );
      if ( pszEnd ) pszEnd = strchr( pszEnd+1 , BACKSLASH );
      if ( pszEnd ) pszEnd = strchr( pszEnd+1 , BACKSLASH );
      if ( pszEnd ) *pszEnd = EOS;
    }

    if ( strncmp( pCommArea->szObjName, pCommArea->szBuffer, strlen(pCommArea->szBuffer) ) == 0 )
    {
      CHAR       szFormat[MAX_FORMAT];       // folder format / Tag Table
      static CHAR szMemory[MAX_LONGFILESPEC];       // folder Translation Memory
      CHAR       szEditor[MAX_FILESPEC];       // folder editor
      CHAR       szSourceLang[MAX_LANG_LENGTH];// folder source language
      CHAR       szTargetLang[MAX_LANG_LENGTH];// folder target language
      PSZ        pszDoc = (PSZ) PVOIDFROMMP2(mp2);   // ptr to document object name
      HPROP      hDocProp;                     // document properties handle
      EQFINFO    ErrorInfo;                    // return code from prop. handler
      ULONG      ulFolderID;                   // ID of current folder
      BOOL       fDisabled = FALSE;            // folder is disabled flag

      /***********************************************************/
      /* Get settings from folder                                */
      /***********************************************************/
      FolQueryInfoEx( pCommArea->szObjName, szMemory, szFormat, szSourceLang,
                      szTargetLang, szEditor, NULL, NULL, NULL, &fDisabled, NULL, NULL, FALSE, NULLHANDLE );

      // get folder/subfolder ID
      ulFolderID = FolGetSubFolderIdFromObjName(pCommArea->szObjName);

      // check for subfolder or document object (the object name of subfolders
      // is the fully qualified path to the subfolder property file, for documents
      // the property path is not part of the object name)
      if ( UtlFileExist( pszDoc ) )
      {
        PPROPFOLDER pProp = NULL;
        ULONG ulLen;

        // object is a subfolder ...
        if ( UtlLoadFileL( pszDoc, (PVOID *) &pProp, &ulLen, FALSE, FALSE ) )
        {
          if ( pProp->ulParentFolder == ulFolderID )
          {
            if ( pProp->szLongMemory[0] != EOS )
              strcpy( szMemory, pProp->szLongMemory );
            else if ( pProp->szMemory[0] != EOS )
              strcpy( szMemory, pProp->szMemory );
            if ( pProp->szFormat[0] != EOS )
            {
              strcpy( szFormat, pProp->szFormat );
            } /* endif */
            if ( pProp->szSourceLang[0] != EOS ) strcpy( szSourceLang, pProp->szSourceLang );
            if ( pProp->szTargetLang[0] != EOS ) strcpy( szTargetLang, pProp->szTargetLang );
            if ( pProp->szEditor[0] != EOS )     strcpy( szEditor, pProp->szEditor );

            OEMTOANSI( pProp->szLongName );
            sprintf( pCommArea->szBuffer,
                     "%s\x15%s\x15%s\x15%lu\x15%lu\x15%lu\x15%lu\x15%lu\x15%lu\x15%s\x15%lu\x15%d\x15%d\x15%d\x15%s\x15%s\x15%s\x15%s\x15%s\x15%s\x15%lu\x15%s\x15%s\x15",
             /* ObjName     */  pszDoc,
             /* Name        */  pProp->szLongName,
             /* Markup      */  szFormat,
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
             /* Doc format  */  szFormat,
             /* TM          */  szMemory,
             /* SourceLng   */  szSourceLang,
             /* TargetLng   */  szTargetLang,
             /* Editor      */  szEditor,
             /* Alias       */  "",
             /* Sourcedate  */  0L,
             /* Name        */  "",
             /* Extension   */  "" );
            mResult = MRFROMSHORT( TRUE );
          } /* endif */
        } /* endif */
        if ( pProp ) UtlAlloc( (PVOID *) &pProp, 0L, 0L, NOMSG );
      }
      else
      {
        // object is a document ...
        /*************************************************************/
        /* Access document properties                                */
        /*************************************************************/
        hDocProp = OpenProperties( pszDoc, NULL, PROP_ACCESS_READ, &ErrorInfo);
        if ( hDocProp )
        {
          PPROPDOCUMENT pDocProp = (PPROPDOCUMENT) MakePropPtrFromHnd( hDocProp );
          if ( pDocProp->ulParentFolder == ulFolderID )
          {
            MRESULT res = TRUE;
            FOLMakeDocListItem ( pDocProp, szFormat, szMemory,
                                 szSourceLang, szTargetLang,
                                 szEditor,
                                 pCommArea->szBuffer );
            if ( fDisabled ) res |= 4;
            mResult = MRFROMSHORT( res );
          } /* endif */
          CloseProperties( hDocProp, PROP_QUIT, &ErrorInfo );
        } /* endif */
      } /* endif */

    } /* endif */
    break;

  case WM_EQF_ABOUTTOREMOVEDRIVE:
    if ( pCommArea->szObjName[0] == CHAR1FROMMP1(mp1) )
    {
      mResult = MRFROMSHORT( TRUE );   // remove of drive is not possible
    } /* endif */
    break;

  case WM_EQF_INITMENU:
  case WM_INITMENU:
    {
      SHORT         sItem;                // selected listbox item

      UtlMenuDisableAll( hwnd, SHORT1FROMMP1( mp1) );

      // AFC: Disable specific menu items in case of an AFC folder
      // Open Folder Properties
      {
        EQFINFO     ErrorInfo;           // error code of property handler calls
        PPROPFOLDER pFolProp;            // ptr to folder properties
        PVOID       hFolProp;            // handle of folder properties

        fAFCFolderRestrict = FALSE;

        // replace driver letter with primary drive
        UtlQueryString( QST_PRIMARYDRIVE, pCommArea->szBuffer, sizeof(pCommArea->szBuffer) );
        strcpy( pCommArea->szBuffer + 1, pCommArea->szObjName + 1);

        hFolProp = OpenProperties( pCommArea->szBuffer , NULL, PROP_ACCESS_READ,
                                   &ErrorInfo);
        if ( hFolProp )
        {
          pFolProp = (PPROPFOLDER) MakePropPtrFromHnd( hFolProp );

          fAFCFolderRestrict = ( pFolProp->fAFCFolder &&
                                 !pFolProp->fTCMasterFolder );
          CloseProperties( hFolProp, PROP_QUIT, &ErrorInfo );
        }
        else   // error
        {
          fAFCFolderRestrict = FALSE;
        } /* endif */
      } /* endif */


      if (!fAFCFolderRestrict) UtlMenuEnableItem( PID_FILE_MI_IMPORT );

      UtlMenuEnableItem( PID_FILE_MI_DESELECTALL );
      UtlMenuEnableItem( PID_FILE_MI_SELECTALL );
      UtlMenuEnableItem( PID_VIEW_MI_NAMES );
      UtlMenuEnableItem( PID_VIEW_MI_DETAILSDLG );
      UtlMenuEnableItem( PID_VIEW_MI_DETAILS );
      UtlMenuEnableItem( PID_UTILS_MI_MT );
      UtlMenuEnableItem( PID_VIEW_MI_SORT );
      UtlMenuEnableItem( PID_VIEW_MI_SOME );
      UtlMenuEnableItem( PID_VIEW_MI_ALL );
      UtlMenuEnableItem( PID_VIEW_MI_SHRINKPATH );
      UtlMenuEnableItem( PID_VIEW_MI_SHOWPATH );
      UtlMenuEnableItem( PID_VIEW_MI_HIDEPATH );
      UtlMenuEnableItem( PID_FILE_MI_NEW );
      UtlMenuEnableItem( PID_FILE_MI_PRINTLIST );
      if ( QUERYITEMCOUNTHWND( pCommArea->hwndLB ) != 0 )
      {
        UtlMenuEnableItem( PID_FILE_MI_DESELECTALL );
        UtlMenuEnableItem( PID_FILE_MI_SELECTALL );
      } /* endif */

      if (IsClipboardFormatAvailable(uiFormat))
      {
        UtlMenuEnableItem( PID_FILE_MI_PASTE );
      }

      {
        sItem = QUERYNEXTSELECTIONHWND( pCommArea->hwndLB, LIT_FIRST );

        if ( sItem != LIT_NONE )
        {
          if ( WinSendMsg( pCommArea->hwndLB, LM_EQF_QUERYITEMSTATE,
                           MP1FROMSHORT(sItem), 0L ) )
          {
            int iSubFolders = 0;
            int iDocs       = 0;

            FolCountSelectedElements( pCommArea->hwndLB, pCommArea->szBuffer,
                                      &iSubFolders, &iDocs );

            if ( iSubFolders == 0 )
            {
              // only documents are selected
              UtlMenuEnableItem( PID_FILE_MI_OPEN );
              UtlMenuEnableItem( PID_FILE_MI_EXPORT );
              UtlMenuEnableItem( PID_FILE_MI_ANALYZE );
              // cv
              UtlMenuEnableItem( PID_FILE_MI_CUT );
              UtlMenuEnableItem( PID_FILE_MI_COPY );
              //
              UtlMenuEnableItem( PID_UTILS_MI_COUNT );
              UtlMenuEnableItem( PID_UTILS_MI_REPORT );
              UtlMenuEnableItem( PID_UTILS_MI_ARCHTM );
              UtlMenuEnableItem( PID_UTILS_MI_EXPORTSEGS );
              UtlMenuEnableItem( PID_UTILS_MI_DELETEDOCS );
              UtlMenuEnableItem( PID_FILE_MI_PROPERTIES );
              UtlMenuEnableItem( PID_FILE_MI_HTMLPROPS );
              UtlMenuEnableItem( PID_FILE_MI_FINDCHANGE );
              UtlMenuEnableItem( PID_FILE_MI_FUZZYSEARCH );
              UtlMenuEnableItem( PID_FILE_MI_SPELLCHECK );
            }
            else if ( iDocs == 0 )
            {
              // only subfolders are selected
              if ( iSubFolders == 1 )
              {
                UtlMenuEnableItem( PID_FILE_MI_OPEN );
                UtlMenuEnableItem( PID_FILE_MI_PROPERTIES );
                UtlMenuEnableItem( PID_FILE_MI_HTMLPROPS );
              } /* endif */
              UtlMenuEnableItem( PID_FILE_MI_ANALYZE );

              if (IsClipboardFormatAvailable(uiFormat))
              {
                UtlMenuEnableItem( PID_FILE_MI_PASTE );
              }
              UtlMenuEnableItem( PID_UTILS_MI_COUNT );
              UtlMenuEnableItem( PID_UTILS_MI_REPORT );
              UtlMenuEnableItem( PID_UTILS_MI_MT );
              UtlMenuEnableItem( PID_UTILS_MI_ARCHTM );
              UtlMenuEnableItem( PID_UTILS_MI_EXPORTSEGS );
              UtlMenuEnableItem( PID_UTILS_MI_DELETEDOCS );
              UtlMenuEnableItem( PID_FILE_MI_FINDCHANGE );
              UtlMenuEnableItem( PID_FILE_MI_FUZZYSEARCH );
              UtlMenuEnableItem( PID_FILE_MI_SPELLCHECK );
            }
            else
            {
              // documents and subfolders are selected
              UtlMenuEnableItem( PID_FILE_MI_ANALYZE );
              UtlMenuEnableItem( PID_UTILS_MI_COUNT );
              UtlMenuEnableItem( PID_UTILS_MI_REPORT );
              UtlMenuEnableItem( PID_UTILS_MI_MT );
              UtlMenuEnableItem( PID_UTILS_MI_ARCHTM );
              UtlMenuEnableItem( PID_UTILS_MI_EXPORTSEGS );
              UtlMenuEnableItem( PID_UTILS_MI_DELETEDOCS );
              UtlMenuEnableItem( PID_FILE_MI_FINDCHANGE );
              UtlMenuEnableItem( PID_FILE_MI_FUZZYSEARCH );
              UtlMenuEnableItem( PID_FILE_MI_SPELLCHECK );
            } /* endif */
          } /* endif */
          if (!fAFCFolderRestrict) UtlMenuEnableItem( PID_FILE_MI_DELETE );
        } /* endif */
      }
    }
    break;

  case WM_EQF_TOOLBAR_ENABLED:
    switch ( mp1 )
    {
    /**************************************************************/
    /* check for items to be enabled ..                           */
    /**************************************************************/
    case PID_FILE_MI_IMPORT:
    case PID_VIEW_MI_NAMES:
    case PID_VIEW_MI_DETAILSDLG:
    case PID_VIEW_MI_DETAILS:
    case PID_FILE_MI_PRINTLIST:
    case PID_UTILS_MI_MT:
    case PID_VIEW_MI_SORT:
    case PID_VIEW_MI_SOME:
    case PID_VIEW_MI_ALL:
    case PID_FILE_MI_NEW:
    case PID_VIEW_MI_SHRINKPATH:
    case PID_VIEW_MI_HIDEPATH:
    case PID_VIEW_MI_SHOWPATH:
      mResult = MRFROMSHORT(TRUE);
      break;
    case PID_FILE_MI_DESELECTALL:
    case PID_FILE_MI_SELECTALL:
      mResult = MRFROMSHORT( QUERYITEMCOUNTHWND( pCommArea->hwndLB ) != 0 );
      break;
    case PID_FILE_MI_OPEN:
    case PID_FILE_MI_EXPORT:
    case PID_FILE_MI_ANALYZE:
    case PID_FILE_MI_COPY:
    case PID_FILE_MI_CUT:
    case PID_UTILS_MI_COUNT:
    case PID_UTILS_MI_REPORT:
    case PID_UTILS_MI_ARCHTM:
    case PID_UTILS_MI_EXPORTSEGS:
    case PID_UTILS_MI_DELETEDOCS:
    case PID_FILE_MI_PROPERTIES:
    case PID_FILE_MI_HTMLPROPS:
    case PID_FILE_MI_FINDCHANGE:
    case PID_FILE_MI_FUZZYSEARCH:
    case PID_FILE_MI_SPELLCHECK:
      {
        SHORT sItem = QUERYNEXTSELECTIONHWND( pCommArea->hwndLB, LIT_FIRST );
        if ( sItem != LIT_NONE )
        {
          int iSubFolders = 0;
          int iDocs       = 0;
          FolCountSelectedElements( pCommArea->hwndLB, pCommArea->szBuffer,
                                    &iSubFolders, &iDocs );

          if ( iDocs == 0 )
          {
            // for subfolders only a subset of the menu items is active
            switch ( mp1 )
            {
              case PID_FILE_MI_OPEN:
                if ( iSubFolders > 1 )
                {
                  mResult = MRFROMSHORT(FALSE);
                }
                else
                {
                  mResult = WinSendMsg( pCommArea->hwndLB, LM_EQF_QUERYITEMSTATE,
                                        MP1FROMSHORT(sItem), 0L );
                } /* endif */
                break;
              case PID_FILE_MI_ANALYZE:
              case PID_UTILS_MI_COUNT:
              case PID_UTILS_MI_REPORT:
              case PID_UTILS_MI_ARCHTM:
              case PID_UTILS_MI_EXPORTSEGS:
              case PID_UTILS_MI_DELETEDOCS:
              case PID_FILE_MI_PROPERTIES:
              case PID_FILE_MI_FINDCHANGE:
              case PID_FILE_MI_FUZZYSEARCH:
              case PID_FILE_MI_SPELLCHECK:
                mResult = WinSendMsg( pCommArea->hwndLB, LM_EQF_QUERYITEMSTATE,
                                      MP1FROMSHORT(sItem), 0L );
                break;
              default:
                mResult = MRFROMSHORT(FALSE);
                break;
            } /* endswitch */
          }
          else if ( iSubFolders == 0 )
          {
            // only documents are selected
            mResult = WinSendMsg( pCommArea->hwndLB, LM_EQF_QUERYITEMSTATE,
                                  MP1FROMSHORT(sItem), 0L );
          }
          else
          {
            // subfolders and documents are selected
            switch ( mp1 )
            {
              case PID_FILE_MI_ANALYZE:
              case PID_UTILS_MI_COUNT:
              case PID_UTILS_MI_REPORT:
              case PID_UTILS_MI_DELETEDOCS:
              case PID_UTILS_MI_ARCHTM:
              case PID_UTILS_MI_EXPORTSEGS:
              case PID_FILE_MI_PROPERTIES:
              case PID_FILE_MI_FINDCHANGE:
              case PID_FILE_MI_FUZZYSEARCH:
              case PID_FILE_MI_SPELLCHECK:
                mResult = WinSendMsg( pCommArea->hwndLB, LM_EQF_QUERYITEMSTATE,
                                      MP1FROMSHORT(sItem), 0L );
                break;
              default:
                mResult = MRFROMSHORT(FALSE);
                break;
            } /* endswitch */
          } /* endif */
        } /* endif */
      }
      break;
    case PID_FILE_MI_DELETE:
      {
        SHORT sItem = QUERYNEXTSELECTIONHWND( pCommArea->hwndLB, LIT_FIRST );
        mResult = MRFROMSHORT( sItem != LIT_NONE );
      }
      break;
    case PID_FILE_MI_PASTE:
      {
        // GQ: always enable paste even if a document is selected...
        // SHORT sItem = QUERYNEXTSELECTIONHWND( pCommArea->hwndLB, LIT_FIRST );
        // mResult = MRFROMSHORT( sItem == LIT_NONE );
        if (IsClipboardFormatAvailable(uiFormat) )
        {
          mResult = MRFROMSHORT(TRUE);
        }
        else
        {
          mResult = MRFROMSHORT(FALSE);
        } /* endif */
      }
      break;
    default:
      break;
    } /* endswitch */
    break;

  case WM_EQF_COMMAND:
  case WM_COMMAND:
    {
		 USHORT usID = SHORT1FROMMP1(mp1);
         mResult = MRFROMSHORT( TRUE ); // default return code for COMMAND msgs

    if ( (usID >= PID_WIND_MI_TOP) && ( usID <= PID_WIND_MI_BOT) )
	{
		HWND hwndFrame = NULL;
		POBJLST   pObject;
		PTWBMAIN_IDA  pIda;
		USHORT        usI;
		if (mp1 >= (PID_WIND_MI_NEXT ))
		{
		  usI = (USHORT)(mp1 - (PID_WIND_MI_NEXT));
          hwndFrame =  (HWND) UtlQueryULong(QL_TWBFRAME );

          pIda = ACCESSWNDIDA( hwndFrame, PTWBMAIN_IDA);
          if (pIda)
          {
            pObject = pIda->hwndsInMenuWindows + usI;
            SendMessage( (HWND)UtlQueryULong( QL_TWBCLIENT), WM_MDIACTIVATE,
                         MP1FROMHWND(pObject->hwnd), 0L );
	      }
	     }
	     else  // MINALL and RESTOREALL
	     { //  EnumChildWindows( (HWND)UtlQueryULong( QL_TWBCLIENT ),
//		                                    ChildEnumProc,
//		                                    MP2FROMSHORT( SHORT1FROMMP1(mp1) ) );
			int i = 0;
			i = i + 1;
			if (usID == PID_WIND_MI_MINALL)
			{
			  SendMessage( (HWND)UtlQueryULong( QL_TWBCLIENT), WM_EQF_MDIMINALL,
										   MP1FROMSHORT(0) /*MDITILE_HORIZONTAL*/, 0L );
		    }
		    else if (usID == PID_WIND_MI_RESTOREALL)
		    {
				SendMessage( (HWND)UtlQueryULong( QL_TWBCLIENT), WM_EQF_MDIRESTOREALL,
										   MP1FROMSHORT(0) /*MDITILE_HORIZONTAL*/, 0L );
		    }
			  return( (MRESULT)NULL);


	     }
   	}
	else
    {

    switch ( usID)
    {
	case PID_WIND_MI_TILE:
	     SendMessage( (HWND)UtlQueryULong( QL_TWBCLIENT), WM_MDITILE,
							   MP1FROMSHORT(0) /*MDITILE_HORIZONTAL*/, 0L );
			  return( (MRESULT)NULL);
		break;
	case PID_WIND_MI_CASCADE:
	    SendMessage( (HWND)UtlQueryULong( QL_TWBCLIENT), WM_MDICASCADE,
							   0 /*MDITILE_ZORDER*/, 0L );
			 return( (MRESULT)NULL);
		break;
    case PID_FILE_MI_OPEN:
      {
        SHORT sItem, sItemCount;
        int iSubFolders = 0;
        int iDocs       = 0;

        /***************************************************************/
        /* Get number of selected items                                */
        /***************************************************************/
        sItem = LIT_FIRST;
        sItemCount = 0;
        do
        {
          sItem = QUERYNEXTSELECTIONHWND( pCommArea->hwndLB, sItem );
          if ( sItem != LIT_NONE )
          {
            // Win2000 corrupts our data if we use LB_GETTEXT so use own message instead
//          QUERYITEMTEXTHWNDL( pCommArea->hwndLB, sItem, pszItemText, sizeof(pCommArea->szBuffer)  );
            SendMessage( pCommArea->hwndLB, LM_EQF_QUERYITEMTEXT, (WPARAM)sItem, (LPARAM)pCommArea->szBuffer );
            if ( FolIsSubFolderItem( pCommArea->szBuffer ) )
            {
              iSubFolders++;
            }
            else
            {
              iDocs++;
            } /* endif */
            sItemCount++;
          } /* endif */
        } while ( sItem != LIT_NONE ); /* enddo */

        if ( (iSubFolders > 1) || (iSubFolders && iDocs) )
        {
          UtlError( ERROR_MORE_THAN_ONE_SELECTED, MB_CANCEL,
                    0, NULL, EQF_WARNING );
        }
        else if ( iSubFolders == 1 )
        {
          // open a subfolder, item text is already stored in pCommArea->szBuffer
          UtlParseX15( pCommArea->szBuffer, 0 );
          EqfSend2Handler( FOLDERHANDLER, WM_EQF_OPEN, MP1FROMSHORT( 0 ),
                           MP2FROMP( pCommArea->szBuffer ) );
        }
        else
        if ( sItemCount >= 1 )
        {
          /********************************************************/
          /* pass on info of selected documents to DOCUMENTHANDLER*/
          /* use mp1 to determine if dealing with extended option */
          /* (multiple selection).                                */
          /* Document handler takes care of handling such cases.  */
          /********************************************************/
          EqfSend2Handler( DOCUMENTHANDLER, WM_EQF_OPEN,
                           MP1FROMSHORT( MULT_DOCUMENTS ),
                           MP2FROMP( pCommArea->szObjName ) );
        } /* endif */
      }
      break;

    case PID_FILE_MI_NEW:
      {
        // prefix object name with subfolder prefix and start subfolder creation
        strcpy( pCommArea->szBuffer, SUBFOLDERPREFIX );
        strcat( pCommArea->szBuffer, pCommArea->szObjName );
        EqfSend2Handler( FOLDERHANDLER, WM_EQF_CREATE, MP1FROMSHORT( TRUE ),
                         MP2FROMP(pCommArea->szBuffer) );
      }
      break;

    case PID_FILE_MI_PROPERTIES:
      {
        SHORT sItem;
        int iItemCount;
        PSZ pszObjList = NULL;
        BOOL fSubFolder = FALSE;
        BOOL fOK = TRUE;

        // get number of selected objects
        iItemCount = SendMessage( pCommArea->hwndLB, LB_GETSELCOUNT, 0, 0 );

        // build list of selected document objects, check for subfolders
        if ( iItemCount > 1 )
        {
          // allocate buffer for the object name list
          fOK = UtlAlloc( (PVOID *)&pszObjList, 0, (MAX_LONGFILESPEC*iItemCount)+10,
                          ERROR_STORAGE );

          // fill-in object names of selected documents
          if ( fOK )
          {
            PSZ pszCurPos = pszObjList;
            int iSelItems = iItemCount;
            sItem = LIT_FIRST;

            while ( iSelItems )
            {
              sItem = QUERYNEXTSELECTIONHWND( pCommArea->hwndLB, sItem );
              if ( sItem >= 0 )
              {
                SendMessage( pCommArea->hwndLB, LM_EQF_QUERYITEMTEXT, (WPARAM)sItem, (LPARAM)pCommArea->szBuffer );
                if ( FolIsSubFolderItem( pCommArea->szBuffer ) ) fSubFolder = TRUE;
                strcpy( pszCurPos, UtlParseX15( pCommArea->szBuffer, FOL_OBJECT_IND ) );
                pszCurPos += strlen( pszCurPos ) + 1;
              } /* endif */
              iSelItems--;
            } /* endwhile */
            *pszCurPos = EOS;          // terminate list
          } /* endif */

          // check if more subfolders and/or documents and subfolders have been selected
          if ( fOK && fSubFolder )
          {
            UtlError( ERROR_MORE_THAN_ONE_SELECTED, MB_CANCEL,
                      0, NULL, EQF_WARNING );
            fOK = FALSE;
          } /* endif */
        } /* endif */

        if ( fOK  )
        {
          if ( iItemCount == 0 )
          {
            fOK = FALSE;  // nothing selected at all...
          } /* endif */
        } /* endif */

        if ( fOK  )
        {
          PSZ    pszDocObjName;

          sItem = QUERYSELECTIONHWND( pCommArea->hwndLB );
          // Win2000 corrupts our data if we use LB_GETTEXT so use own message instead
          SendMessage( pCommArea->hwndLB, LM_EQF_QUERYITEMTEXT, (WPARAM)sItem, (LPARAM)pCommArea->szBuffer );
          fSubFolder = FolIsSubFolderItem( pCommArea->szBuffer );
          pszDocObjName = UtlParseX15( pCommArea->szBuffer, FOL_OBJECT_IND );
          if ( fSubFolder )
          {
            FolSubFolderProperties(  pszDocObjName );
          }
          else
          {
            DocPropertyDlg( hwnd, pszDocObjName, TRUE, NULL, pszObjList );
          } /* endif */
        } /* endif */
        if ( pszObjList ) UtlAlloc( (PVOID *)&pszObjList, 0, 0, NOMSG );
      }
      break;

    case PID_FILE_MI_HTMLPROPS:
      {
        BOOL fOk = TRUE;
        PSZ pszObj = QuerySelFolderName( pCommArea, TRUE );
        fOk = EqfDocPropsToHtml( pszObj, hwnd );
      }
      break;

    case PID_FILE_MI_DELETE:
      {
        SHORT sItem = LIT_FIRST;
        BOOL   fDone = FALSE;
        PSZ    pszDocObjName;
        OBJNAME szDocObjName;
        USHORT  usMBReturn = 0;

        szDocObjName[0] = EOS;

        do
        {
          sItem = QUERYNEXTSELECTIONHWND( pCommArea->hwndLB, sItem );
          if ( sItem == LIT_NONE )
          {
            fDone = TRUE;         // no more file to delete
          }
          else
          {
            BOOL fSubFolder = FALSE;

           // Win2000 corrupts our data if we use LB_GETTEXT so use own message instead
            SendMessage( pCommArea->hwndLB, LM_EQF_QUERYITEMTEXT, (WPARAM)sItem, (LPARAM)pCommArea->szBuffer );
            fSubFolder = FolIsSubFolderItem( pCommArea->szBuffer );
            pszDocObjName = UtlParseX15( pCommArea->szBuffer, FOL_OBJECT_IND );

            if ( strcmp( pszDocObjName, szDocObjName ) == 0  )
            {
              /***************************************************/
              /* We tried this one already so skip it this time  */
              /***************************************************/
            }
            else
            {
              // get rid off item selection
              //    although delete of item may fail ...
              strcpy( szDocObjName, pszDocObjName );
              DESELECTITEMHWND( pCommArea->hwndLB, sItem );
              if ( fSubFolder )
              {
                PSZ pszParm = UtlParseX15( pCommArea->szBuffer, FOL_NAME_IND );
                OEMTOANSI( pszParm );
                if ( usMBReturn != MBID_EQF_YESTOALL )
                {
                  usMBReturn = UtlError( QUERY_DELETE_SUBFOLDER, MB_EQF_YESTOALL,
                                         1, &pszParm, EQF_QUERY );
                } /* endif */

                if ( (usMBReturn == MBID_YES) ||
                     (usMBReturn == MBID_EQF_YESTOALL) ) //file should not be deleted
                {
                  SubFolderDelete( szDocObjName, FALSE );
                } /* endif */
              }
              else
              {
                usMBReturn = (USHORT)EqfSend2Handler( DOCUMENTHANDLER,
                                                      WM_EQF_DELETE,
                                                      MP1FROMSHORT(usMBReturn),
                                                      MP2FROMP(pszDocObjName) );
              } /* endif */

              if ( usMBReturn == MBID_CANCEL )
              {
                fDone = TRUE;
              }
              else if ( sItem == 0 )
              {
                sItem = LIT_FIRST; // restart at begin of list
              }
              else
              {
                sItem--;           // restart just in front of current item
              } /* endif */
            } /* endif */
          } /* endif */
        } while ( !fDone ); /* enddo */
      }
      break;

    case PID_FILE_MI_IMPORT:
      {
        INT_PTR iRC;
        HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);

        DIALOGBOX( DIALOG_OWNER, FIMPODLGPROPPROC, hResMod, ID_DOCIMP_PROP_DLG,
                   NULL, iRC );
      }
      break;

    case PID_FILE_MI_EXPORT:
      {
        INT_PTR iRC;
        HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);

        DIALOGBOX( DIALOG_OWNER, FEXPORTDLGPROPPROC, hResMod, ID_DOCEXP_PROP_DLG,
                   NULL, iRC );
        }
      break;

    case PID_FILE_MI_COPY:
    case PID_FILE_MI_CUT:
      {
          SHORT sItem, sItemCount;
          LONG  lLen;
          BOOL fOK = TRUE;
          PDOCLIST pList = NULL;
          CHAR *pszTemp;

          INT iSubFolders = 0;
          INT iDocs       = 0;


          /***************************************************************/
          /* Get number of selected items                                */
          /***************************************************************/
          sItem = LIT_FIRST;
          sItemCount = 0;

          sItemCount = (SHORT)SendMessage(pCommArea->hwndLB,LB_GETSELCOUNT,0,0L);
          lLen = sizeof(DOCLIST)+sItemCount*sizeof(DOCPROP);
          fOK = UtlAlloc( (PVOID *) &pList, 0L, lLen, NOMSG );
          if(!fOK)
              break;
          pList->sCount = sItemCount;
          pList->sAction = SHORT1FROMMP1(mp1);
          pList->lBytes = lLen;
          strcpy(pList->szFolder,pCommArea->szObjName);

          if ( FolIsSubFolderObject(pCommArea->szObjName))
          {
              UtlSplitFnameFromPath( pList->szFolder );  // cut off subfolder name
              UtlSplitFnameFromPath( pList->szFolder );  // cut off property directory
          }
          sItemCount = 0;
          do
          {
            sItem = QUERYNEXTSELECTIONHWND( pCommArea->hwndLB, sItem );
            if ( sItem != LIT_NONE )
            {
              // Win2000 corrupts our data if we use LB_GETTEXT so use own message instead
    //          QUERYITEMTEXTHWNDL( pCommArea->hwndLB, sItem, pszItemText, sizeof(pCommArea->szBuffer)  );
              SendMessage( pCommArea->hwndLB, LM_EQF_QUERYITEMTEXT, (WPARAM)sItem, (LPARAM)pCommArea->szBuffer );

              pszTemp = UtlParseX15( pCommArea->szBuffer, FOL_OBJECT_IND );
              strcpy(pList->aDoc[iDocs].szName,UtlGetFnameFromPath(pszTemp));
              strcpy(pList->aDoc[iDocs].szLongName,UtlParseX15( pCommArea->szBuffer, FOL_NAME_IND ));

              if ( SHORT1FROMMP1(mp1) == PID_FILE_MI_CUT )
              {
                WinSendMsg( pCommArea->hwndLB,
                            LM_EQF_SETITEMSTATE,
                            MP1FROMSHORT( sItem ),
                            MP2FROMSHORT( FALSE ) );
              }
              if ( FolIsSubFolderItem( pCommArea->szBuffer ) )
              {
                iSubFolders++;
              }
              else
              {
                iDocs++;
              } /* endif */
               sItemCount++;
            } /* endif */
            else
            {
              if (iDocs)
              {
                HANDLE hClip;
                HGLOBAL hMem;
                LPTSTR lpBuffer;
                CHAR szDrive[MAX_DRIVE];
                INT i;
                INT cbSize = pList->lBytes;
                USHORT usRC;

                OpenClipboard(NULL);
                EmptyClipboard();
                hMem = GlobalAlloc(LMEM_MOVEABLE,cbSize);
                if (hMem)
                {
                    BOOL fSetClipboard = TRUE;
                    //lpBuffer = GlobalLock(hMem);
                    //memcpy(lpBuffer,pList,cbSize);
                    //GlobalUnlock(hMem);
                    //hClip = SetClipboardData(uiFormat,hMem);
                    //CloseClipboard();

                    //if (hClip)
                    {
                        UtlQueryString( QST_PRIMARYDRIVE, szDrive, MAX_DRIVE );
                        BOOL fOverwrite = TRUE;

                        for (i=0;i<iDocs;i++)
                        {   
                            CHAR szPackName[MAX_FILESPEC];
                            usRC = DocExpExportInternal( pList->szFolder, pList->aDoc[i].szLongName, pList->aDoc[i].szName, szDrive[0], hwnd, &fOverwrite, szPackName );
                            if (usRC != UNLOAD_OK)
                            {
                                UtlError( ERROR_EXPORT_INT, MB_CANCEL, 0, NULL, EQF_ERROR );
                                //EmptyClipboard();
                                fSetClipboard = FALSE;
                                break;
                            }
                            else
                            {
                              // adjust document short name / package name
                              strcpy( pList->aDoc[i].szName, szPackName );
                            }
                        }
                    }
                    if (fSetClipboard)
                    {
                        lpBuffer = (LPTSTR) GlobalLock(hMem);
                        memcpy(lpBuffer,pList,cbSize);
                        GlobalUnlock(hMem);
                        hClip = SetClipboardData(uiFormat,hMem);
                    }
                    GlobalFree(hMem);
                }
                CloseClipboard();
              }
            }
          } while ( sItem != LIT_NONE ); /* enddo */
      }
      break;

    case PID_FILE_MI_PASTE:
      {
          PDOCLIST pList;
          CHAR szDocObjName[MAX_LONGFILESPEC];

          PDOCIMPIDA pImpIda;

          if (IsClipboardFormatAvailable(uiFormat))
          {
              HGLOBAL hMem;
              LPTSTR lpBuffer;
              INT i,cbSize;
              CHAR * pszExt;
              CHAR szDrive[MAX_DRIVE];
              CHAR szFolderObjName[MAX_LONGFILESPEC];
              BOOL fOK = TRUE;
              BOOL fIsSubFolder = FALSE;
              USHORT usRC;

              // get clipboard data
              OpenClipboard(NULL);
              hMem = GetClipboardData(uiFormat);
              if (hMem)
              {
                  lpBuffer = (LPTSTR) GlobalLock(hMem);
                  GlobalUnlock(hMem);
                  cbSize = ((PDOCLIST)lpBuffer)->lBytes;
                  fOK = UtlAlloc( (PVOID *) &pList, 0L, cbSize, NOMSG );
                  if (!fOK)
                      break;

                  memcpy(pList,lpBuffer,cbSize);
                  EmptyClipboard();
                  CloseClipboard();

                  fOK = UtlAlloc( (PVOID *) &pImpIda,0L,sizeof(DOCIMPIDA),NOMSG);
                  if (!fOK)
                      break;

                  EqfQueryObjectName( EqfQueryActiveFolderHwnd(), szFolderObjName );
                  strcpy( pImpIda->szParentObjName,szFolderObjName);
                  pImpIda->fYesToAll = FALSE;   // TRUE ???

                  fIsSubFolder = FolIsSubFolderObject( szFolderObjName );
                  if (fIsSubFolder)
                  {
                      ULONG ulFolderID = FolGetSubFolderIdFromObjName(szFolderObjName);
                      pImpIda->ulParentID = ulFolderID;
                      UtlSplitFnameFromPath( szFolderObjName );
                      UtlSplitFnameFromPath( szFolderObjName );
                  }
                  if (strcmp(szFolderObjName,pList->szFolder) == 0)
                      pImpIda->fYesToAll = TRUE;

                  strcpy( pImpIda->szTargetFolder,szFolderObjName);
                  strcpy( pImpIda->szToFolder, UtlGetFnameFromPath( szFolderObjName ) );
                  strcpy( pImpIda->szFromFolder,UtlGetFnameFromPath(pList->szFolder));

                  if ((SHORT)mp2 == PID_FILE_MI_CUT || (SHORT)mp2 == PID_FILE_MI_COPY)
                      pList->sAction = (SHORT)mp2;

                  UtlQueryString( QST_PRIMARYDRIVE, szDrive, MAX_DRIVE );

                  pImpIda->usSelDocs = pList->sCount;

                  for (i=0;i<pList->sCount;i++)
                  {
                    strcpy( pImpIda->szString,pList->aDoc[i].szName);
                    strcpy( pImpIda->stFs.szDrive,szDrive);
                    Utlstrccpy( pImpIda->stFs.szName, pList->aDoc[i].szName, DOT );
                    pszExt = strchr( pList->aDoc[i].szName, DOT );
                    if ( pszExt != NULL )
                    {
                        strcpy( pImpIda->stFs.szExt, pszExt );
                    }
                    else
                    {
                        pImpIda->stFs.szExt[0] = EOS;
                    }

                    usRC = DocImpInternal(pImpIda);
                    if (usRC != LOAD_OK)
                    {
                        UtlError( ERROR_EXPORT_INT, MB_CANCEL, 0, NULL, EQF_ERROR );
                        break;
                    }
                    else
                    {
                      // delete export package and try to remove directories
                      UtlMakeEQFPath( pImpIda->szBuffer, pImpIda->stFs.szDrive[0],
                                      EXPORT_PATH, NULL );
                      strcat( pImpIda->szBuffer, BACKSLASH_STR );
                      strcat( pImpIda->szBuffer, pImpIda->szFromFolder );
                      strcat( pImpIda->szBuffer, BACKSLASH_STR );
                      strcat( pImpIda->szBuffer, pList->aDoc[i].szName );
                      DocImpInternalDeletePackage( pImpIda->szBuffer );
                    } /* endif */

                    if (pList->sAction == PID_FILE_MI_CUT && strcmp(szFolderObjName,pList->szFolder) != 0)
                    {
                      BOOL fIsNew = FALSE;
                      CHAR szDocShortName[MAX_FILESPEC];

                      // build correct document short name (may be different from short name in target folder)
                      FolLongToShortDocName( pList->szFolder, pList->aDoc[i].szLongName, szDocShortName, &fIsNew );

                      // delete document
                      strcpy(szDocObjName,pList->szFolder);
                      strcat(szDocObjName,BACKSLASH_STR);
                      strcat(szDocObjName,szDocShortName);
                      DocumentDelete(szDocObjName,FALSE,&usRC);
                    }

                    // refresh document view
                    strcpy(szDocObjName,pImpIda->szTargetFolder);
                    strcat(szDocObjName,BACKSLASH_STR);
                    strcat(szDocObjName,pImpIda->szToDoc );
                    EqfSend2AllHandlers ( WM_EQFN_CREATED,
                                          MP1FROMSHORT( clsDOCUMENT ),
                                          MP2FROMP( szDocObjName ));
                  }
              }
          }
      }
      break;

    case PID_FILE_MI_ANALYZE:
      EqfSend2Handler( ANALYSISHANDLER, WM_EQF_CREATE, 0,
                       MP2FROMP( pCommArea->szObjName ) );
      break;

    case PID_UTILS_MI_ARCHTM:
      TABuildArchTM( hwnd, pCommArea->szObjName );
      break;
    case PID_UTILS_MI_EXPORTSEGS:
      ExportSegs( hwnd, pCommArea->szObjName );
      break;

    case PID_UTILS_MI_COUNT:
      EqfSend2Handler( COUNTHANDLER, WM_EQF_CREATE,
                       MP1FROMSHORT( TRUE),
                       MP2FROMP( pCommArea->szObjName ) );
      break;

    case PID_UTILS_MI_REPORT:
      EqfSend2Handler( REPORTHANDLER, WM_EQF_CREATE,
                       MP1FROMSHORT( TRUE),
                       MP2FROMP( pCommArea->szObjName ) );
      break;

    case PID_UTILS_MI_MT:
      {
        //PSZ pszObj = QuerySelFolderName( pCommArea, TRUE );
        PSZ pszObj = pCommArea->szObjName;

        if ( pszObj != NULL )
        {
          EqfSend2Handler( MTLISTHANDLER, WM_EQF_OPEN,
                           MP1FROMSHORT( TRUE ), MP2FROMP( pszObj ) );
        } /* endif */
      }
      break;

    case PID_FILE_MI_FINDCHANGE:
      FolGlobFindChange( pCommArea->szObjName, FALSE, FALSE );
      break;

    case PID_FILE_MI_FUZZYSEARCH:
      FolFuzzySearch( pCommArea->szObjName, FALSE, FALSE );
      break;

    case PID_FILE_MI_SPELLCHECK:
      FolSpellcheck( pCommArea->szObjName, FALSE, FALSE );
      break;

    case PID_FILE_MI_PRINTLIST:
      // pass message to column listbox control
      WinSendMsg( pCommArea->hwndLB, message, mp1, mp2 );
      break;


    default:
      if ( message == WM_EQF_COMMAND)
      {
        mResult = MRFROMSHORT( FALSE ); // tell twbmain that we rejected
      } /* endif */
    }
    } /* endif */
    }
    break;

  case WM_EQF_QUERYSELECTEDNAMES:
    {
      HWND       hwndTargetLB = HWNDFROMMP1(mp1);
      SHORT      sItem = LIT_FIRST;

      sItem = QUERYNEXTSELECTIONHWND( pCommArea->hwndLB, sItem );
      while ( sItem != LIT_NONE )
      {
        // get text of selected item
        // Win2000 corrupts our data if we use LB_GETTEXT so use own message instead
//      QUERYITEMTEXTHWNDL( pCommArea->hwndLB, sItem, pszItemText, sizeof(pCommArea->szBuffer)  );
        SendMessage( pCommArea->hwndLB, LM_EQF_QUERYITEMTEXT, (WPARAM)sItem, (LPARAM)pCommArea->szBuffer );

        if ( FolIsSubFolderItem( pCommArea->szBuffer ) )
        {
          // add all documents of subfolder to callers listbox
          UtlParseX15( pCommArea->szBuffer, 0 );
          LoadDocumentNames( pCommArea->szBuffer, hwndTargetLB,
                             LOADDOCNAMES_NAME | LOADDOCNAMES_INCLSUBFOLDERS,
                             pCommArea->szBuffer );
        }
        else
        {
          // get file name within document object name and insert it into
          // target listbox
          UtlParseX15( pCommArea->szBuffer, 0 );
          INSERTITEMHWND( hwndTargetLB,
                          UtlGetFnameFromPath( pCommArea->szBuffer ) );
        } /* endif */
        sItem = QUERYNEXTSELECTIONHWND( pCommArea->hwndLB, sItem );
      } /* endwhile */
      mResult = MRFROMSHORT( QUERYITEMCOUNTHWND( hwndTargetLB ) );
    }
    break;

  default:
    break;
  } /* endswitch */
  return( mResult );
} /* end of function FolderCallBack */



BOOL FolDeleteFolder
(
  HWND       hwndParent,
  PSZ        pszObj,
  PUSHORT    pusYesToAllMode
)
{
  HWND       hwndObj;
  PSZ        pszParm;
  CHAR       szLongName[MAX_LONGFILESPEC];
  BOOL       fOK = TRUE;
  PPROPSYSTEM pPropSys = GetSystemPropPtr();
  PSZ        pszTemp = UtlGetFnameFromPath( pszObj );

  // return asap if something goes totally wrong...
  if ( pszTemp == NULL )
  {
    return( FALSE );
  }

  Utlstrccpy( szLongName, pszTemp, DOT );
  ObjShortToLongName( szLongName, szLongName, FOLDER_OBJECT );
  OEMTOANSI(szLongName);
  pszParm = szLongName;

  hwndObj = EqfQueryObject( pszObj, clsFOLDER, 0);

  if ( (hwndObj != NULLHANDLE) && !pPropSys->fUseIELikeListWindows )
  {
    UtlErrorHwnd( ERROR_DELETE_FOLDER, MB_CANCEL, 1, &pszParm,
                  EQF_ERROR, hwndParent );
    ActivateMDIChild( hwndObj );
    fOK = FALSE;
  }
  else if ( EqfSend2AllHandlers( WM_EQF_ABOUTTODELETE,
                                 MP1FROMSHORT( clsFOLDER ),
                                 MP2FROMP( pszObj ) ) )
  {
    UtlErrorHwnd( ERROR_FOLDER_LOCKED, MB_CANCEL, 1, &pszParm,
                  EQF_ERROR, hwndParent );
    fOK = FALSE;
  }
  else
  {
    SHORT  sRC = QUERYSYMBOL( pszObj );

    if ( sRC != -1 )
    {
      UtlErrorHwnd( ERROR_FOLDER_LOCKED, MB_CANCEL, 1, &pszParm,
                    EQF_INFO, hwndParent );
      fOK = FALSE;
    }
    else if ( DeleteFolder( pszObj, hwndParent, pusYesToAllMode ) == 0 )
    {
      EqfSend2AllHandlers( WM_EQFN_DELETED,
                           MP1FROMSHORT( clsFOLDER ),
                           MP2FROMP( pszObj ) );
    }
    else
    {
      fOK = FALSE;
    } /* endif */
  } /* endif */
  return( fOK );
} /* end of function FolDeleteFolder */


/**********************************************************************/
/* Create the folder list                                             */
/**********************************************************************/
MRESULT FolderCreateCall( PLISTCOMMAREA pCommArea )
{
  MRESULT    mResult = FALSE;
  BOOL       fOK = TRUE;         // initialisation is O.K. flag
  EQFINFO    ErrorInfo;          // error info of property handler
  HPROP      hProp = NULL;       // folder properties handle
  PPROPFOLDER pProp = NULL;      // ptr to folder properties
  BOOL       fSubFolder = FALSE; // TRUE = a subfolder list is being opened
  BOOL       fSetWidth = TRUE;
  HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);

  fSubFolder = FolIsSubFolderObject( pCommArea->szObjName );

  if ( fSubFolder )
  {
    ULONG ulLen;

    // load subfolder property file
    pProp = NULL;
    fOK = UtlLoadFileL( pCommArea->szObjName, (PVOID *) &pProp, &ulLen, TRUE, TRUE );
  }
  else
  {
    /**************************************************************/
    /* Replace drive letter in object name with drive letter      */
    /* of primary drive in order to build property name           */
    /**************************************************************/
    UtlQueryString( QST_PRIMARYDRIVE, pCommArea->szBuffer,
                    sizeof(pCommArea->szBuffer) );
    strcpy( pCommArea->szBuffer + 1, pCommArea->szObjName + 1);

    /**************************************************************/
    /* Open folder properties                                     */
    /**************************************************************/
    hProp = OpenProperties( pCommArea->szBuffer, NULL,
                            PROP_ACCESS_READ, &ErrorInfo);
    if ( hProp )
    {
      pProp = (PPROPFOLDER) MakePropPtrFromHnd( hProp );
    }
    else
    {
      fOK = FALSE;
    } /* endif */

    /**************************************************************/
    /* Check if folder drive is the same as the drive listed      */
    /* in the properties                                          */
    /**************************************************************/
    if ( fOK )
    {
      if ( pCommArea->szObjName[0] != pProp->chDrive )
      {
        fOK = FALSE;
      } /* endif */
    } /* endif */

    /**************************************************************/
    /* Check if folder drive is still a primary or secondary drive*/
    /**************************************************************/
    if ( fOK )
    {
      UtlQueryString( QST_VALIDEQFDRIVES, pCommArea->szBuffer,
                      sizeof(pCommArea->szBuffer) );
      if ( strchr( pCommArea->szBuffer, pProp->chDrive ) == NULL )
      {
        fOK = FALSE;
      } /* endif */
    } /* endif */

    /**************************************************************/
    /* Check if folder directories are accessible                 */
    /**************************************************************/
    if ( fOK )
    {
      UtlMakeEQFPath( pCommArea->szBuffer, pProp->chDrive, PROPERTY_PATH,
                      UtlGetFnameFromPath(pCommArea->szObjName) );
      fOK = UtlDirExist( pCommArea->szBuffer);

      if ( fOK )
      {
        UtlMakeEQFPath( pCommArea->szBuffer, pProp->chDrive, DIRSOURCEDOC_PATH,
                        UtlGetFnameFromPath(pCommArea->szObjName) );
        fOK = UtlDirExist( pCommArea->szBuffer);
      } /* endif */

      if ( fOK )
      {
        UtlMakeEQFPath( pCommArea->szBuffer, pProp->chDrive,
                        DIRSEGSOURCEDOC_PATH,
                        UtlGetFnameFromPath(pCommArea->szObjName) );
        fOK = UtlDirExist( pCommArea->szBuffer);
      } /* endif */

      if ( fOK )
      {
        UtlMakeEQFPath( pCommArea->szBuffer, pProp->chDrive,
                        DIRSEGTARGETDOC_PATH,
                        UtlGetFnameFromPath(pCommArea->szObjName) );
        fOK = UtlDirExist( pCommArea->szBuffer);
      } /* endif */

      if ( fOK )
      {
        UtlMakeEQFPath( pCommArea->szBuffer, pProp->chDrive,
                        DIRTARGETDOC_PATH,
                        UtlGetFnameFromPath(pCommArea->szObjName) );

        fOK = UtlDirExist( pCommArea->szBuffer);
      } /* endif */
    } /* endif */
  } /* endif */

  /**************************************************************/
  /* Load column listbox title strings                          */
  /**************************************************************/
  if ( fOK  )
  {
    LOADSTRING( NULLHANDLE, hResMod, SID_FOL_NAME_COLTITLE,     ColHdr[1]);
    UtlLoadWidth( NULLHANDLE, hResMod, SID_FOL_NAME_COLWIDTH,
                  &(ColTable[1].usWidth) );
    LOADSTRING( NULLHANDLE, hResMod, SID_FOL_TRANSL_COLTITLE,   ColHdr[3]);
    UtlLoadWidth( NULLHANDLE, hResMod, SID_FOL_TRANSL_COLWIDTH,
                  &(ColTable[3].usWidth) );
    LOADSTRING( NULLHANDLE, hResMod, SID_FOL_ANAL_COLTITLE,     ColHdr[4]);
    UtlLoadWidth( NULLHANDLE, hResMod, SID_FOL_ANAL_COLWIDTH,
                  &(ColTable[4].usWidth) );
    LOADSTRING( NULLHANDLE, hResMod, SID_FOL_EXP_COLTITLE,      ColHdr[5]);
    UtlLoadWidth( NULLHANDLE, hResMod, SID_FOL_EXP_COLWIDTH,
                  &(ColTable[5].usWidth) );
    LOADSTRING( NULLHANDLE, hResMod, SID_FOL_IMP_COLTITLE,      ColHdr[6]);
    UtlLoadWidth( NULLHANDLE, hResMod, SID_FOL_IMP_COLWIDTH,
                  &(ColTable[6].usWidth) );

    //LOADSTRING( NULLHANDLE, hResMod, SID_FOL_UPD_COLTITLE,      ColHdr[7]);
    //UtlLoadWidth( NULLHANDLE, hResMod, SID_FOL_UPD_COLWIDTH,
    //              &(ColTable[7].usWidth) );
    LOADSTRING( NULLHANDLE, hResMod, SID_FOL_TRANSLATOR_COLTITLE,   ColHdr[7]);
    UtlLoadWidth( NULLHANDLE, hResMod, SID_FOL_TRANSLATOR_COLWIDTH,
                  &(ColTable[7].usWidth) );


    LOADSTRING( NULLHANDLE, hResMod, SID_FOL_UPDT_COLTITLE,     ColHdr[8]);
    UtlLoadWidth( NULLHANDLE, hResMod, SID_FOL_UPDT_COLWIDTH,
                  &(ColTable[8].usWidth) );
    LOADSTRING( NULLHANDLE, hResMod, SID_FOL_SIZE_COLTITLE,     ColHdr[9]);
    UtlLoadWidth( NULLHANDLE, hResMod, SID_FOL_SIZE_COLWIDTH,
                  &(ColTable[9].usWidth) );
    LOADSTRING( NULLHANDLE, hResMod, SID_FOL_COMPLETE_COLTITLE, ColHdr[10]);
    UtlLoadWidth( NULLHANDLE, hResMod, SID_FOL_COMPLETE_COLWIDTH,
                  &(ColTable[10].usWidth) );
    LOADSTRING( NULLHANDLE, hResMod, SID_FOL_MODIFIED_COLTITLE, ColHdr[11]);
    UtlLoadWidth( NULLHANDLE, hResMod, SID_FOL_MODIFIED_COLWIDTH,
                  &(ColTable[11].usWidth) );

    // new column is nw used for the shipment number
    //LOADSTRING( NULLHANDLE, hResMod, SID_FOL_SCRATCH_COLTITLE,  ColHdr[12]);
    //UtlLoadWidth( NULLHANDLE, hResMod, SID_FOL_SCRATCH_COLWIDTH,
    //              &(ColTable[12].usWidth) );
    LOADSTRING( NULLHANDLE, hResMod, SID_FOL_SHIPMENT_COLTITLE,  ColHdr[12]);
    UtlLoadWidth( NULLHANDLE, hResMod, SID_FOL_SHIPMENT_COLWIDTH,
                  &(ColTable[12].usWidth) );

    // copied column is currently not in use
    //LOADSTRING( NULLHANDLE, hResMod, SID_FOL_COPIED_COLTITLE,   ColHdr[13]);
    //UtlLoadWidth( NULLHANDLE, hResMod, SID_FOL_COPIED_COLWIDTH,
    //              &(ColTable[13].usWidth) );

    LOADSTRING( NULLHANDLE, hResMod, SID_FOL_FORMAT_COLTITLE,   ColHdr[14]);
    UtlLoadWidth( NULLHANDLE, hResMod, SID_FOL_FORMAT_COLWIDTH,
                  &(ColTable[14].usWidth) );
    LOADSTRING( NULLHANDLE, hResMod, SID_FOL_MEMORY_COLTITLE,   ColHdr[15]);
    UtlLoadWidth( NULLHANDLE, hResMod, SID_FOL_MEMORY_COLWIDTH,
                  &(ColTable[15].usWidth) );
    LOADSTRING( NULLHANDLE, hResMod, SID_FOL_SOURCE_COLTITLE,   ColHdr[16]);
    UtlLoadWidth( NULLHANDLE, hResMod, SID_FOL_SOURCE_COLWIDTH,
                  &(ColTable[16].usWidth) );
    LOADSTRING( NULLHANDLE, hResMod, SID_FOL_TARGET_COLTITLE,   ColHdr[17]);
    UtlLoadWidth( NULLHANDLE, hResMod, SID_FOL_TARGET_COLWIDTH,
                  &(ColTable[17].usWidth) );
    LOADSTRING( NULLHANDLE, hResMod, SID_FOL_EDITOR_COLTITLE,  ColHdr[18]);
    UtlLoadWidth( NULLHANDLE, hResMod, SID_FOL_EDITOR_COLWIDTH,
                  &(ColTable[18].usWidth) );
    LOADSTRING( NULLHANDLE, hResMod, SID_FOL_ALIAS_COLTITLE,  ColHdr[19]);
    UtlLoadWidth( NULLHANDLE, hResMod, SID_FOL_ALIAS_COLWIDTH,
                  &(ColTable[19].usWidth) );
    LOADSTRING( NULLHANDLE, hResMod, SID_FOL_SRC_COLTITLE,  ColHdr[20]);
    UtlLoadWidth( NULLHANDLE, hResMod, SID_FOL_SRC_COLWIDTH,
                  &(ColTable[20].usWidth) );
    LOADSTRING( NULLHANDLE, hResMod, SID_FOL_NAMEWOPATH_COLTITLE,  ColHdr[21]);
    UtlLoadWidth( NULLHANDLE, hResMod, SID_FOL_NAMEWOPATH_COLWIDTH,
                  &(ColTable[21].usWidth) );
    LOADSTRING( NULLHANDLE, hResMod, SID_FOL_EXTENSION_COLTITLE,  ColHdr[22]);
    UtlLoadWidth( NULLHANDLE, hResMod, SID_FOL_EXTENSION_COLWIDTH,
                  &(ColTable[22].usWidth) );

  } /* endif */

  /**************************************************************/
  /* Set column listbox view lists                              */
  /**************************************************************/
  if ( fOK  )
  {
    memcpy( pCommArea->asCurView,
            (pProp->sLastUsedViewList[0] != 0) ? pProp->sLastUsedViewList :
            sLastUsedView,
            sizeof(pCommArea->asCurView) );
    FolCLBData.psLastUsedViewList = pCommArea->asCurView;

    memcpy( pCommArea->asDetailsView,
            (pProp->sDetailsViewList[0] != 0) ? pProp->sDetailsViewList :
            sDetailsView,
            sizeof(pCommArea->asDetailsView) );
    FolCLBData.psDetailsViewList = pCommArea->asDetailsView;

    memcpy( &pCommArea->Filter, &pProp->Filter,
            sizeof(pCommArea->Filter) );
    FolCLBData.psSortList = pCommArea->asSortList;
    FolCLBData.pFilter = &pCommArea->Filter;

    if (fSetWidth)
	{
	    int i=0;
		for (i=0;pCommArea->asCurView[i]>0 && i<MAX_VIEW ;i++)
		{
            int index = pCommArea->asCurView[i];
            if (pProp->sLastUsedViewWidth[i] > 0)
            {
                ColTable[index].usWidth = pProp->sLastUsedViewWidth[i];
            }
            else
            {
                pProp->sLastUsedViewWidth[i] = ColTable[index].usWidth;
            }
		}
	}    // endif

  } /* endif */

  /****************************************************************/
  /* supply all information required to create a document list    */
  /****************************************************************/
  if ( fOK )
  {
    pCommArea->sListObjClass = clsFOLDER;
    LOADSTRING( NULLHANDLE, hResMod, SID_FOLI_TITLE, pCommArea->szTitle );
    pCommArea->hIcon          = (HPOINTER) UtlQueryULong(QL_FOLICON); //hiconFOL;
    if ( fSubFolder )
    {
      SubFolObjectNameToName( pCommArea->szObjName,
                              pCommArea->szTitle + strlen(pCommArea->szTitle) );
      OEMTOANSI(pCommArea->szTitle);
    }
    else
    {
      Utlstrccpy( pCommArea->szBuffer, UtlGetFnameFromPath( pCommArea->szObjName ), DOT );
      ObjShortToLongName( pCommArea->szBuffer, pCommArea->szBuffer, FOLDER_OBJECT );
      OEMTOANSI(pCommArea->szBuffer);
      strcpy( pCommArea->szTitle + strlen(pCommArea->szTitle), pCommArea->szBuffer );
    } /* endif */
    pCommArea->sObjNameIndex  = FOL_OBJECT_IND;
    pCommArea->sNameIndex     = FOL_NAME_IND;
    pCommArea->sListWindowID  = ID_FOLDER_WINDOW;
    pCommArea->sListboxID     = ID_FOLDER_LB;


    //
    // AFC: use correct popup
    //

    if (pProp->fAFCFolder && ! pProp->fTCMasterFolder)
    {
      pCommArea->sPopupMenuID   = ID_FOL_POPUP_AFC;
      pCommArea->sGreyedPopupMenuID   = ID_FOL_POPUP_AFC;
      pCommArea->sNoSelPopupMenuID = ID_FOL_POPUP_NOSEL_AFC;
      pCommArea->sMultPopupMenuID = ID_FOL_POPUP_MULTSEL_AFC;
    }
    else
    {
      pCommArea->sPopupMenuID   = ID_FOL_POPUP;
      pCommArea->sGreyedPopupMenuID   = ID_FOL_POPUP;
      pCommArea->sNoSelPopupMenuID = ID_FOL_POPUP_NOSEL;
      pCommArea->sMultPopupMenuID = ID_FOL_POPUP_MULTSEL;
    }
    pCommArea->pColData       = &FolCLBData;
    pCommArea->fMultipleSel   = TRUE;
    pCommArea->sDefaultAction = PID_FILE_MI_OPEN;
    if ( (pProp->Swp.cx == 0) || (pProp->Swp.cy == 0) )
    {
      // fill in default values
      FolSetDefaultPos( &(pProp->Swp) );
    } /* endif */
    memcpy( &(pCommArea->swpSizePos), &(pProp->Swp), sizeof(EQF_SWP) );
    pCommArea->sItemClass     = clsDOCUMENT;
    pCommArea->sItemPropClass = PROP_CLASS_DOCUMENT;
    pCommArea->asMsgsWanted[0] = WM_EQF_QUERYSELECTEDNAMES;
    pCommArea->asMsgsWanted[1] = 0;       // end of list
  } /* endif */

  if ( fSubFolder )
  {
    // free subfolder properties
    if ( pProp ) UtlAlloc( (PVOID *) &pProp, 0L, 0L, NOMSG );
  }
  else
  {
    /**************************************************************/
    /* Close properties                                           */
    /**************************************************************/
    if ( hProp )
    {
      CloseProperties( hProp, PROP_QUIT, &ErrorInfo );
    } /* endif */
  } /* endif */

  /**************************************************************/
  /* In case of errors set error return code                    */
  /**************************************************************/
  if ( !fOK )
  {
    mResult = MRFROMSHORT(DO_NOT_CREATE);
  } /* endif */
  return mResult;
}

/**********************************************************************/
/* Check if folder property file exist ...                            */
/* Note:                                                              */
/*   Quick check necessary to ensure correct window creation (restart)*/
/*   under MFC environment...                                         */
/**********************************************************************/
static BOOL  CheckPropertyExist( PSZ pData )
{
  BOOL       fOK = TRUE;         // initialisation is O.K. flag
  EQFINFO    ErrorInfo;          // error info of property handler
  HPROP      hProp = NULL;       // folder properties handle
  CHAR       chObjName[MAX_PATH144+MAX_FILESPEC];

  /**************************************************************/
  /* Replace drive letter in object name with drive letter      */
  /* of primary drive in order to build property name           */
  /**************************************************************/
  UtlQueryString( QST_PRIMARYDRIVE, chObjName, sizeof(chObjName) );
  strcpy( chObjName + 1, pData + 1);

  /**************************************************************/
  /* Open folder properties                                     */
  /**************************************************************/
  hProp = OpenProperties( chObjName, NULL,
                          PROP_ACCESS_READ, &ErrorInfo);
  if ( hProp )
  {
    /******************************************************************/
    /* close folder properties                                        */
    /******************************************************************/
    CloseProperties( hProp, PROP_QUIT, &ErrorInfo );
  }
  else
  {
    fOK = FALSE;
  } /* endif */

  return fOK;
}

// count selected documents,
//    0 = no element of type selected
//    1 = one element of type is selected
//    2 = 2 or more elelemnts of type are selected
int FolCountSelectedElements
(
  HWND hwndLB,
  PSZ  pszBuffer,
  int  *piSubFolders,
  int  *piDocs
)
{
  int iItem = 0;
  int iSelItems = 0;
  int *piSelItems = NULL;

  *piSubFolders = 0;
  *piDocs       = 0;

  // get number of selected items
  iSelItems = SendMessage( hwndLB, LB_GETSELCOUNT, 0, 0L );

  // Allocate a temporary buffer for the selected items
  UtlAlloc( (PVOID *) &piSelItems, 0L, (LONG)(sizeof(int) * iSelItems), ERROR_STORAGE );

  // Get selected items into our temporary buffer
  if ( piSelItems != NULL )
  {
    SendMessage( hwndLB, LB_GETSELITEMS, iSelItems, MP2FROMP(piSelItems) );
  } /* endif */

  if ( piSelItems != NULL )
  {
    while( (iItem < iSelItems) &&
           ( ((*piDocs < 2) || (*piSubFolders < 2)) ) )
    {
      // Win2000 corrupts our data if we use LB_GETTEXT so use own message instead
      SendMessage( hwndLB, LM_EQF_QUERYITEMTEXT, (WPARAM)piSelItems[iItem], (LPARAM)pszBuffer );
      if ( FolIsSubFolderItem( pszBuffer ) )
      {
        *piSubFolders = *piSubFolders + 1;
      }
      else
      {
        *piDocs = *piDocs + 1;
      } /* endif */
      iItem++;
    } /* endwhile */
  } /* endif */

  if ( piSelItems ) UtlAlloc( (PVOID *) &piSelItems, 0L, 0L, NOMSG );

  return( 0 );
} /* FolCountSelectedElements */


// remove document in internal format and export directories (if they are empty)
int DocImpInternalDeletePackage
(
  PSZ pszPackage                       // fully qualified file name of package being deleted
)
{
  CHAR szTempPath[MAX_EQF_PATH];

  // delete package file
  UtlDelete( pszPackage, 0L, FALSE );

  // remove folder property file if it is the only file left in the directory
  {
    WIN32_FIND_DATA FileFindData;
    CHAR szFolName[MAX_FILESPEC];
    HANDLE hDir;
    BOOL fMoreFilesExist = FALSE;

    strcpy( szTempPath, pszPackage );
    UtlSplitFnameFromPath( szTempPath );
    strcpy( szFolName, UtlGetFnameFromPath( szTempPath ) );
    strcat( szTempPath, "\\*.*" );

    hDir = FindFirstFile( szTempPath, &FileFindData );
    if ( hDir != INVALID_HANDLE_VALUE )
    {
      BOOL fMoreFiles = TRUE;

      do
      {
        if ( (FileFindData.nFileSizeHigh != 0) || (FileFindData.nFileSizeLow != 0) )
        {
          // check if we deal with a real file or the dummy file containing the folder long name
          if ( stricmp( FileFindData.cFileName, szFolName ) != 0 )
          {
            fMoreFilesExist = TRUE;
          } /* endif */
        } /* endif */
        fMoreFiles = FindNextFile( hDir, &FileFindData );
      } while ( fMoreFiles );
      FindClose( hDir );

      // delete the dummy file if no other files exist in the directory
      if ( !fMoreFilesExist )
      {
        strcpy( szTempPath, pszPackage );
        UtlSplitFnameFromPath( szTempPath );
        strcat( szTempPath, BACKSLASH_STR );
        strcat( szTempPath, szFolName );
        UtlDelete( szTempPath, 0L, FALSE );
      } /* endif */
    } /* endif */
  }

  // the following calls will fail if the directory is not
  // empty; i.e. the directory will be removed only if
  // there are no more exported files in the directory
  strcpy( szTempPath, pszPackage );
  UtlSplitFnameFromPath( szTempPath );
  RemoveDirectory( szTempPath );
  UtlSplitFnameFromPath( szTempPath );
  RemoveDirectory( szTempPath );
  return( 0 );
} /* end of function DocImpInternalDeletePackage */
