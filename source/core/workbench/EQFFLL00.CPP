//+----------------------------------------------------------------------------+
//|  EQFFLL00.CPP - EQF Folder List Handler                                    |
//+----------------------------------------------------------------------------+
//|  Copyright Notice:                                                         |
//|                                                                            |
//|      Copyright (C) 1990-2017, International Business Machines              |
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
//|                                                                            |
//+----------------------------------------------------------------------------+

#include "core\PluginManager\OtmPlugin.h"
#include "core\PluginManager\PluginManager.h"


#pragma pack( push, TM2StructPacking, 1 )

#define INCL_EQF_TAGTABLE         // tag table and format functions
#define INCL_EQF_FOLDER           // folder list and document list functions
#define INCL_EQF_ANALYSIS         // Analysis functions


#include "eqf.h"                  // General .H for EQF
#include "eqfstart.id"            // IDs for EQFSTARR resource
#include "eqffll.id"              // IDs for folder lists
#include "eqffll00.h"             // Folder List Handler defines
#include "eqfdde.h"               // batch mode definitions
#include "eqffol00.h"             // Document List Handler defines
#include "EQFHLOG.H"            // defines for history log processing
#include "eqfcolw.id"             // column width IDs


BOOL FLLMakeFolderListItem ( PSZ, PVOID, PSZ );
MRESULT FolderListCreateCall( PLISTCOMMAREA pCommArea );
BOOL FolIsRemovedDirEmpty( PSZ pszFolObjName );
USHORT FolDeleteDocsFromList( HWND hwndOwner, PSZ pszFolObjName );
USHORT FolRestoreDeletedDocs( HWND hwndOwner, PSZ pszFolObjName );

static CHAR ColHdr[13][80];             // Buffer for column header texts
static CLBCOLDATA ColTable[] =
{ { "",               1,             TEXT_DATA,      DT_LEFT},
  { ColHdr[1], CLB_MAX_FNAME, AUTOWIDTHTEXT_DATA,    DT_LEFT},
  { ColHdr[2], CLB_MAX_DESCRIPTION,  TEXT_DATA,      DT_LEFT},
  { ColHdr[3], CLB_MAX_DATE,         DATE_DATA,      DT_LEFT},
  { ColHdr[4], CLB_MAX_FNAME,        TEXT_DATA,      DT_LEFT},
  { ColHdr[5], CLB_MAX_FNAME, AUTOWIDTHTEXT_DATA,    DT_LEFT},
  { ColHdr[6], CLB_MAX_DRIVE,        TEXT_DATA,      DT_LEFT},
  { ColHdr[7], 10,                   TEXT_DATA,      DT_LEFT},
  { ColHdr[8], CLB_MAX_FNAME,        TEXT_DATA,      DT_LEFT},
  { ColHdr[9], CLB_MAX_LANG_LENGTH,  TEXT_DATA,      DT_LEFT},
  { ColHdr[10],CLB_MAX_LANG_LENGTH,  TEXT_DATA,      DT_LEFT},
  { ColHdr[11],CLB_MAX_FNAME, AUTOWIDTHTEXT_DATA,    DT_LEFT},
  { ColHdr[12],CLB_MAX_FNAME, AUTOWIDTHTEXT_DATA,    DT_LEFT},
  { NULL,      0,             (CLBDATATYPE)        0,             0}};

static SHORT sLastUsedView[] = { FLL_NAME_IND, CLBLISTEND};
static SHORT sDefaultView[]  = { FLL_NAME_IND, FLL_DESCR_IND, CLBLISTEND};
static SHORT sNameView[]     = { FLL_NAME_IND, CLBLISTEND};
static SHORT sDetailsView[]  = { FLL_NAME_IND, FLL_DESCR_IND, CLBLISTEND};
static SHORT sSortCriteria[] = { FLL_NAME_IND, CLBLISTEND};

static CLBCTLDATA FllCLBData =
{  sizeof(CLBCTLDATA),                 // size of control structure
  13,                                 // we have 13 data columns
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
  ColTable};                         // set address of column definition table

// function creating object name list of selected folders
int FolBuildSelItemslist
(
  PLISTCOMMAREA    pCommArea,
  PSZ              *ppszList
);

#pragma pack( pop, TM2StructPacking )


/**********************************************************************/
/* Handler callback function for folder list handler                  */
/**********************************************************************/
MRESULT FolderListHandlerCallBack
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
      pCommArea->pfnCallBack          = FolderListCallBack;
      strcpy( pCommArea->szHandlerName, FOLDERLISTHANDLER );
      pCommArea->sBaseClass           = clsFOLDERLIST;
      pCommArea->sListWindowID        = ID_FLIST_WINDOW;
      pCommArea->sListboxID           = PID_FLIST_LB;
      pCommArea->asNotifyClassList[0] = clsFOLDERLIST;
      pCommArea->asNotifyClassList[1] = 0;       // end of list
      break;




    case WM_EQF_OPEN:
      {
        HWND       hwndObj;
        PSZ        pszObj = (PSZ) PVOIDFROMMP2(mp2);

        if ( (hwndObj = EqfQueryObject( pszObj, clsFOLDERLIST, 0)) != NULLHANDLE )
        {
          SETFOCUSHWND( hwndObj );
          mResult = MRFROMSHORT( TRUE );
        }
        else
        {
          mResult = WinSendMsg( hwnd, WM_EQF_CREATELISTWINDOW, mp1, mp2 );
        } /* endif */
      }
      break;

    case WM_EQF_CREATE:
      UtlError( ERROR_CREATE_FOLDERLIST, MB_CANCEL, 0, (PSZ *) NULP, EQF_ERROR );
      mResult = MRFROMSHORT( TRUE );
      break;

    case WM_EQF_INSERTNAMES:
      mResult = WinSendMsg( EqfQueryActiveFolderlistHwnd(), message, mp1, mp2);
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
} /* end of function FolderListHandlerCallBack */


//void CALLBACK EXPORT TimerProc
void CALLBACK FolderTimerProc
(
   HWND hWnd,      // handle of CWnd that called SetTimer
   UINT nMsg,      // WM_TIMER
   UINT nIDEvent,  // timer identification
   DWORD dwTime    // system time
)
{
  // check for new open document data
  CHAR szOpenDocFile[MAX_EQF_PATH];

  hWnd; nMsg; nIDEvent; dwTime;

  UtlMakeEQFPath( szOpenDocFile, NULC, PROPERTY_PATH, NULL );
  strcat( szOpenDocFile, "\\OPENDOC.DAT" );
  if ( UtlFileExist( szOpenDocFile ) )
  {
    POPENANDPOS pOpenAndPos = NULL;
    USHORT usBytesRead = 0;
    BOOL fOK = UtlLoadFile( szOpenDocFile, (PVOID *)&pOpenAndPos , &usBytesRead, FALSE, FALSE);
    if ( fOK && (usBytesRead >= sizeof(OPENANDPOS)) )
    {
      EqfPost2Handler( DOCUMENTHANDLER, WM_EQF_PROCESSTASK, MP1FROMSHORT(OPEN_AND_POSITION_TASK), MP2FROMP(pOpenAndPos) );
      UtlDelete( szOpenDocFile, 0, FALSE );
    } /* endif */
  } /* endif */
}

void FolderListFolderProperties( HWND hwnd, PLISTCOMMAREA pCommArea, WINMSG message, WPARAM mp1, LPARAM mp2 );

// helper functiuon: check if global memory options have to be verified
BOOL FolderListGlobalMemoryCheckRequired
(
  PSZ              pszFolderObject
)
{
  PPROPFOLDER pProp = NULL;
  ULONG       ulSize = 0;
  CHAR        szPropFile[MAX_EQF_PATH];
  BOOL        fCheckRequired = FALSE;
  
  // setup property file name
  UtlMakeEQFPath( szPropFile, NULC, PROPERTY_PATH, NULL );
  strcat( szPropFile, BACKSLASH_STR );
  strcat( szPropFile, UtlGetFnameFromPath( pszFolderObject ) );

  if ( UtlLoadFileL( szPropFile, (PVOID *)&pProp, &ulSize, FALSE, FALSE ) )
  {
    fCheckRequired = pProp->fGlobalMemOptCheckRequired;
    UtlAlloc( (PVOID *)&pProp, 0, 0, NOMSG );
  } /* endif */     

  return( fCheckRequired );
}

// helper function: check if folder is disabled
BOOL FolIsFolderDisabled
(
  PSZ              pszFolderObject
)
{
  PPROPFOLDER pProp = NULL;
  ULONG       ulSize = 0;
  CHAR        szPropFile[MAX_EQF_PATH];
  BOOL        fDisabled = FALSE;
  
  // setup property file name
  UtlMakeEQFPath( szPropFile, NULC, PROPERTY_PATH, NULL );
  strcat( szPropFile, BACKSLASH_STR );
  strcat( szPropFile, UtlGetFnameFromPath( pszFolderObject ) );

  if ( UtlLoadFileL( szPropFile, (PVOID *)&pProp, &ulSize, FALSE, FALSE ) )
  {
    fDisabled = pProp->fDisabled_UserExitRefresh;
    UtlAlloc( (PVOID *)&pProp, 0, 0, NOMSG );
  } /* endif */     

  return( fDisabled );
}



// helper function: start analysis
void FolderListAnalysis
(
  HWND             hwnd,  
  PLISTCOMMAREA    pCommArea,
  WINMSG           message,
  WPARAM           mp1,
  LPARAM           mp2
  )
{
  PSZ pszObj;

  mp1; hwnd;

  // check if we are dealing with a global memory folder and the settings have to be
  // verified before the analysis can be performed


  if ( message == WM_EQF_FLLTREE_COMMAND )
  {
    if ( FolIsFolderDisabled( (PSZ)mp2 ) )
    {
      // ignore command
    }
    else if ( FolderListGlobalMemoryCheckRequired( (PSZ)mp2 ) )
    {
      FolderListFolderProperties( hwnd, pCommArea, message, mp1, mp2 );
    }
    else
    {
      EqfSend2Handler( ANALYSISHANDLER, WM_EQF_CREATE, 0, mp2 );
    } /* endif */       
  }
  else
  {
    int iSelItems = SendMessage( pCommArea->hwndLB, LB_GETSELCOUNT, 0, 0 );
    if ( iSelItems  > 1 )
    {
      PSZ pszSelObjList = NULL;
      FolBuildSelItemslist( pCommArea, &pszSelObjList );
      if ( pszSelObjList )
      {
        EqfSend2Handler( ANALYSISHANDLER, WM_EQF_CREATE, 1, MP2FROMP(pszSelObjList) );
      } /* endif */
    }
    else
    {
      pszObj = QuerySelFolderName( pCommArea, TRUE  );
      if ( pszObj != NULL )
      {
        if ( FolderListGlobalMemoryCheckRequired( pszObj ) )
        {
          FolderListFolderProperties( hwnd, pCommArea, message, mp1, mp2 );
        }
        else
        {
          EqfSend2Handler( ANALYSISHANDLER, WM_EQF_CREATE, 0, MP2FROMP( pszObj ) );
        } /* endif */       
      } /* endif */
    } /* endif */
  } /* endif */
}

// helper function: add a single change information to the changes list
void  FolAddChange( PFOLPROPHIST2 pHistoryRecord, CHAR chObjectID, CHAR chActionID, PSZ pszValue, PULONG pulUsed )
{
  ULONG ulLength = strlen(pszValue) + 3;

  if ( sizeof(pHistoryRecord->szChanges) >= (*pulUsed + ulLength + 1) )  // always leave room for terminating zero value 
  {
    PSZ pszWritePos = pHistoryRecord->szChanges + *pulUsed;
    *pszWritePos++ = chObjectID;
    *pszWritePos++ = chActionID;
    strcpy( pszWritePos, pszValue );
    *pulUsed += ulLength;
  }
  else
  {
    pHistoryRecord->fOverFlow = TRUE;
  } /* endif */
}

// helper function: add change information for a single value to the changes list
void  FolAddChangeInfo( PFOLPROPHIST2 pHistoryRecord, CHAR chObjectID, PSZ pszOldValue, PSZ pszNewValue, PULONG pulUsed )
{
  if ( strcmp( pszOldValue, pszNewValue ) == 0 )
  {
    // data is unchanged
    FolAddChange( pHistoryRecord, chObjectID, FOLPROP_UNCHANGED_ACTION, pszOldValue, pulUsed );
  } 
  else 
  {
    if ( pszOldValue[0] != EOS )
    {
      FolAddChange( pHistoryRecord, chObjectID, FOLPROP_REMOVED_ACTION, pszOldValue, pulUsed );
    } /* endif */
    if ( pszNewValue[0] != EOS )
    {
      FolAddChange( pHistoryRecord, chObjectID, FOLPROP_NEW_ACTION, pszNewValue, pulUsed );
    } /* endif */
  }
}

// helper function: search a name in the given name table
BOOL FolIsInList( PSZ pszEntry, PSZ pszTable, int iTableSize, int iEntrySize )
{
  PSZ pszTest = pszTable;

  int i = 0;
  while ( (*pszTest != EOS) && (i < iTableSize) )
  {
    if ( strcmp( pszEntry, pszTest ) == 0 )
    {
      return( TRUE );
    } /* endif */
    pszTest += iEntrySize;
    i++;
  } /* endwhile */
  return( FALSE );
}

// helper function: add change information for changes in a named list
void  FolAddChangesInListInfo( PFOLPROPHIST2 pHistoryRecord, CHAR chObjectID, PSZ pOldTable, PSZ pNewTable, int iTableSize, int iEntrySize, PULONG pulUsed )
{
  PSZ pszEntry = pOldTable;

  // handle unchanged and removed elements
  int i = 0;
  while ( (*pszEntry != EOS) && (i < iTableSize) )
  {
    if ( FolIsInList( pszEntry, pNewTable, iTableSize, iEntrySize ) )
    {
      FolAddChange( pHistoryRecord, chObjectID, FOLPROP_UNCHANGED_ACTION, pszEntry, pulUsed );
    }
    else
    {
      FolAddChange( pHistoryRecord, chObjectID, FOLPROP_REMOVED_ACTION, pszEntry, pulUsed );
    } /* endif */
    pszEntry += iEntrySize;
    i++;
  } /* endwhile */

  // handle new elements
  i = 0;
  pszEntry = pNewTable;
  while ( (*pszEntry != EOS) && (i < iTableSize) )
  {
    if ( FolIsInList( pszEntry, pOldTable, iTableSize, iEntrySize ) )
    {
      // nothing to do, unchanged entry already added to change list
    }
    else
    {
      FolAddChange( pHistoryRecord, chObjectID, FOLPROP_NEW_ACTION, pszEntry, pulUsed );
    } /* endif */
    pszEntry += iEntrySize;
    i++;
  } /* endwhile */
}

// helper function: write a history log record with the folder property changes
void FolWriteFolPropChangedHistLog( PSZ pszObj, PPROPFOLDER pOrgFolProp, PPROPFOLDER pNewFolProp )
{
  PFOLPROPHIST2 pHistRec = NULL;
  ULONG ulUsed = 0;

  UtlAlloc( (PVOID *)&pHistRec, 0, sizeof(FOLPROPHIST2), NOMSG );
  if ( pHistRec == NULL ) return;

  FolAddChangeInfo( pHistRec, FOLPROP_DESCRIPTION_OBJ, pOrgFolProp->szDescription, pNewFolProp->szDescription, &ulUsed );
  FolAddChangeInfo( pHistRec, FOLPROP_EDITOR_OBJ, pOrgFolProp->szEditor, pNewFolProp->szEditor, &ulUsed );
  FolAddChangeInfo( pHistRec, FOLPROP_MARKUP_OBJ, pOrgFolProp->szFormat, pNewFolProp->szFormat, &ulUsed );
  FolAddChangeInfo( pHistRec, FOLPROP_MEMORY_OBJ, pOrgFolProp->szLongMemory, pNewFolProp->szLongMemory, &ulUsed );
  FolAddChangeInfo( pHistRec, FOLPROP_SOURCELANG_OBJ, pOrgFolProp->szSourceLang, pNewFolProp->szSourceLang, &ulUsed );
  FolAddChangeInfo( pHistRec, FOLPROP_TARGETLANG_OBJ, pOrgFolProp->szTargetLang, pNewFolProp->szTargetLang, &ulUsed );
  FolAddChangesInListInfo( pHistRec, FOLPROP_DICTIONARY_OBJ, (PSZ)pOrgFolProp->aLongDicTbl, (PSZ)pNewFolProp->aLongDicTbl, sizeof(pOrgFolProp->aLongDicTbl)/sizeof(pOrgFolProp->aLongDicTbl[0]), 
    sizeof(pOrgFolProp->aLongDicTbl[0]), &ulUsed );
  FolAddChangesInListInfo( pHistRec, FOLPROP_RO_MEMORY_OBJ, (PSZ)pOrgFolProp->aLongMemTbl, (PSZ)pNewFolProp->aLongMemTbl, sizeof(pOrgFolProp->aLongMemTbl)/sizeof(pOrgFolProp->aLongMemTbl[0]), 
    sizeof(pOrgFolProp->aLongMemTbl[0]), &ulUsed );

  EQFBWriteHistLog2( pszObj, "", FOLPROP_LOGTASK2, sizeof(FOLPROPHIST2), (PVOID)pHistRec, TRUE, NULLHANDLE, NULL );

  UtlAlloc( (PVOID *)&pHistRec, 0, 0, NOMSG );
} /* end of function FolWriteFolPropChangedHistLog */

// helper function: folder properties
void FolderListFolderProperties
(
  HWND             hwnd,  
  PLISTCOMMAREA    pCommArea,
  WINMSG           message,
  WPARAM           mp1,
  LPARAM           mp2
)
{
  PSZ pszObj;
  PSZ pszFolObj = NULL;
  mp1;
  HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);

  if ( message == WM_EQF_FLLTREE_COMMAND )
  {
    pszObj = (PSZ) mp2;
  }
  else
  {
    // this command from folder import with Global Memory flag set
	BOOL bSuc = UtlAlloc( (PVOID *)&pszFolObj, 0L, (LONG)sizeof(*pCommArea->szBuffer), ERROR_STORAGE );
	if(bSuc)
	{
		strncpy(pszFolObj,pCommArea->szBuffer,strlen(pCommArea->szBuffer));
		pszObj = pszFolObj;

		char* pTemp = pszFolObj;
		while(*pTemp!='\0' && *pTemp!=X15)
			pTemp++;
		*pTemp = '\0';
	}

	// this is from command where list folder properties directly from GUI
	if(pszObj == NULL)
		pszObj = QuerySelFolderName( pCommArea, TRUE  );

  } /* endif */

  if ( pszObj != NULL )
  {
    CHAR          chFolDrive;// buffer for folder drive letter
    PPROPSYSTEM   pSysProp;  // ptr to EQF system properties
    HPROP         hFolProp;  // handle of folder properties
    PPROPFOLDER   pFolProp;  // ptr to folder properties
    EQFINFO       ErrorInfo; // error code of property handler calls
    PPROPFOLDER   pOrgFolProp = NULL;  // ptr to original folder properties

    chFolDrive = *pszObj;
    pSysProp = (PPROPSYSTEM)MakePropPtrFromHnd( EqfQuerySystemPropHnd());
    *pszObj  = pSysProp->szPrimaryDrive[0];
    hFolProp = OpenProperties( pszObj, NULL,
                                PROP_ACCESS_READ, &ErrorInfo);
    *pszObj = chFolDrive;
    if ( hFolProp )
    {
      INT_PTR  iRC;          // buffer for function return codes

      /****************************************************/
      /* Update properties with language from TM          */
      /****************************************************/
      pFolProp = (PPROPFOLDER)MakePropPtrFromHnd( hFolProp );
      if ( (pFolProp->szSourceLang[0] == EOS) ||
            (pFolProp->szTargetLang[0] == EOS) )
      {
        iRC = FolUpdLangFromMemory( hFolProp, FALSE );
      } /* endif */
      pFolProp = NULL;

      UtlAlloc( (PVOID *)&pFolProp, 0L, (LONG)sizeof(*pFolProp), ERROR_STORAGE );
      GetAllProperties( hFolProp, pFolProp, &ErrorInfo);
      pCommArea->fUserFlag = TRUE;

      // make a copy of original folder properties to allow tracking of changes 
      if ( UtlAlloc( (PVOID *)&pOrgFolProp, 0L, (LONG)sizeof(*pOrgFolProp), ERROR_STORAGE ) )
      {
        memcpy( pOrgFolProp, pFolProp, sizeof(*pOrgFolProp) );
      } /* endif */


      DIALOGBOX( hwnd, FOLDERPROPSDLG, hResMod, ID_FOLDERPROPS_DLG,
                  pFolProp, iRC );
      if ( !iRC)                          // result returned ?
      {
        // Add properties changed record to history log             
        FolWriteFolPropChangedHistLog( pszObj, pOrgFolProp, pFolProp );

        /************************************************************/
        /* SHIPMENT_HANDLER   (all240800.zip)                       */
        /* Add shipment    changed record to history log            */
        /************************************************************/

        if (strcmp(pFolProp->szShipment,pFolProp->szLogShipment))
        {
          FOLPROPHISTSHIPMENT FolPropHistShipment;     // history record for folder

          strcpy( FolPropHistShipment.szShipment,     pFolProp->szShipment );

          EQFBWriteHistLog2( pszObj, "",
                              FOLPROPSHIPMENT_LOGTASK,
                              sizeof(FOLPROPHISTSHIPMENT), (PVOID)&FolPropHistShipment,
                              TRUE, NULLHANDLE, NULL );
          strcpy(pFolProp->szLogShipment, pFolProp->szShipment);

        }



        if ( SetPropAccess( hFolProp, PROP_ACCESS_WRITE))
        {
          // always reset global memory option flag
          pFolProp->fGlobalMemOptCheckRequired = FALSE;

          PutAllProperties( hFolProp, pFolProp, &ErrorInfo);
          SaveProperties( hFolProp, &ErrorInfo);
          ResetPropAccess( hFolProp, PROP_ACCESS_WRITE);
          CloseProperties( hFolProp, PROP_QUIT, &ErrorInfo );

          /********************************************************/
          /* Broadcast 'Properties changed' notification for      */
          /* folder not residing on EQF system drive (for these   */
          /* folders a correct notification message is send by    */
          /* the property handler)                                */
          /********************************************************/
          if ( pszObj[0] != pSysProp->szPrimaryDrive[0] )
          {
            EqfSend2AllHandlers( WM_EQFN_PROPERTIESCHANGED,
                                  MP1FROMSHORT( PROP_CLASS_FOLDER ),
                                  MP2FROMP( pszObj ) );
          } /* endif */

        }
      }
      else
      {
        CloseProperties( hFolProp, PROP_QUIT, &ErrorInfo );
      } /* endif */
      pCommArea->fUserFlag = FALSE;
      if ( pOrgFolProp ) UtlAlloc( (PVOID *)&pOrgFolProp, 0L, 0L, NOMSG );
      UtlAlloc( (PVOID *)&pFolProp, 0L, 0L, NOMSG );
    }
    else
    {
      CHAR  szFolName[MAX_FILESPEC];
      PSZ   pszParm;
      SHORT sItem;
      PSZ   pszFolName;

      pszFolName = UtlGetFnameFromPath( pszObj );
      if ( !pszFolName ) pszFolName = pszObj;
      Utlstrccpy( szFolName, pszFolName, DOT );
      pszParm = szFolName;
      UtlError( ERROR_OPEN_FOL_PROPERTIES, MB_CANCEL, 1, &pszParm,
                EQF_ERROR );
      sItem = QUERYSELECTIONHWND( pCommArea->hwndLB );
      if ( sItem != LIT_NONE )
      {
        CLBSETITEMSTATEHWND( pCommArea->hwndLB, sItem, FALSE );
      } /* endif */
    } /* endif */
  } /* endif */

  if(pszFolObj != NULL)
  {
	 UtlAlloc( (PVOID *)&pszFolObj, 0L, 0L, NOMSG );
  }
}



/**********************************************************************/
/* List instance callback function for folder list window             */
/**********************************************************************/
MRESULT FolderListCallBack
(
PLISTCOMMAREA    pCommArea,
HWND             hwnd,
WINMSG           message,
WPARAM           mp1,
LPARAM           mp2
)
{
  MRESULT          mResult = MRFROMSHORT(FALSE);

  switch ( message )
  {
    case WM_CREATE :
      FolderListCreateCall( pCommArea );
      SetTimer( hwnd, ID_FLIST_WINDOW, 1000, FolderTimerProc );

      break;

    case WM_EQF_COLUMNLIST:
      if (mp2)
      {
        FolderListCreateCall( (PLISTCOMMAREA) mp2 );
      } /* endif */
      break;
    case WM_CLOSE :
    case WM_EQF_TERMINATE :
      /**************************************************************/
      /* Save view lists for WM_EQF_TERMINATE only if save flag is  */
      /* on                                                         */
      /**************************************************************/
      if ( (message == WM_EQF_TERMINATE) &&
           pCommArea->fUserFlag         &&
           !(SHORT1FROMMP1(mp1) & TWBFORCE) )
      {
        /**************************************************************/
        /* Reject termination request                                 */
        /**************************************************************/
        mResult = MRFROMSHORT( TRUE );
      }
      else if ( (message == WM_CLOSE) || (SHORT1FROMMP1(mp1) & TWBSAVE) )
      {
        PPROPFOLDERLIST pProp;
        EQFINFO       ErrorInfo;
        HPROP         hProp;

        /**************************************************************/
        /* Open properties                                            */
        /**************************************************************/
        hProp = OpenProperties( pCommArea->szObjName, NULL,
                                PROP_ACCESS_READ, &ErrorInfo );
        if ( hProp )
        {
          /************************************************************/
          /* Save current view lists and window position              */
          /************************************************************/
          if ( SetPropAccess( hProp, PROP_ACCESS_WRITE ) )
          {
            pProp = (PPROPFOLDERLIST) MakePropPtrFromHnd( hProp);

            memcpy( &pProp->Swp, &(pCommArea->swpSizePos), sizeof(EQF_SWP) );
            memcpy( pProp->sLastUsedViewList, pCommArea->asCurView,
                    sizeof(pProp->sLastUsedViewList) );

            memcpy( (pProp->sLastUsedViewWidth), pCommArea->asCurViewWidth,
                    sizeof( pProp->sLastUsedViewWidth ));

            memcpy( pProp->sDetailsViewList, pCommArea->asDetailsView,
                    sizeof(pProp->sDetailsViewList) );
            memcpy( pProp->sSortList, pCommArea->asSortList,
                    sizeof(pProp->sSortList) );
            memcpy( &(pProp->Filter), &(pCommArea->Filter),
                    sizeof(pProp->Filter) );
            SaveProperties( hProp, &ErrorInfo );
          } /* endif */
          CloseProperties( hProp, PROP_QUIT, &ErrorInfo );

        } /* endif */
      } /* endif */
      break;

    case WM_DESTROY:
      /****************************************************************/
      /* Free all resource allocated by list instance callback        */
      /* function                                                     */
      /* nothing to do for folder list                                */
      /****************************************************************/
      KillTimer( hwnd, ID_FLIST_WINDOW );
      break;

    case WM_EQF_INITIALIZE:
      /****************************************************************/
      /* Fill column listbox                                          */
      /****************************************************************/
      FllLoadFolderNames( pCommArea->hwndLB, pCommArea->szBuffer );
      break;

    case WM_EQF_BUILDITEMTEXT :
      /****************************************************************/
      /* Setup item text for the object passed in mp2 parameter       */
      /****************************************************************/
      {
        PSZ        pszFol = (PSZ) PVOIDFROMMP2(mp2);   // ptr to folder object name
        HPROP      hFolProp;                     // folder properties handle
        EQFINFO    ErrorInfo;                    // return code from prop. handler
        PPROPSYSTEM   pSysProp;                  // ptr to EQF system properties

        /*************************************************************/
        /* Access folder properties                                  */
        /*************************************************************/
        strcpy(  pCommArea->szBuffer, pszFol );
        pSysProp = (PPROPSYSTEM) MakePropPtrFromHnd( EqfQuerySystemPropHnd() );
        pCommArea->szBuffer[0] = pSysProp->szPrimaryDrive[0];
        hFolProp = OpenProperties( pCommArea->szBuffer, NULL,
                                   PROP_ACCESS_READ, &ErrorInfo);
        if ( hFolProp )
        {
          FLLMakeFolderListItem ( pszFol, hFolProp, pCommArea->szBuffer );
          CloseProperties( hFolProp, PROP_QUIT, &ErrorInfo );
          mResult = MRFROMSHORT( TRUE );
        } /* endif */
      }
      break;

    case WM_EQFN_DRIVEREMOVED:
    case WM_EQFN_DRIVEADDED:
      /****************************************************************/
      /* Refresh column listbox                                       */
      /****************************************************************/
      FllLoadFolderNames( pCommArea->hwndLB, pCommArea->szBuffer );
      break;

    case WM_EQF_INITMENU:
    case WM_INITMENU:
      {
        SHORT         sItem;                // selected listbox item
        int iSelItems;                      // number of selected items

        UtlMenuDisableAll( hwnd, SHORT1FROMMP1( mp1) );
        if ( EqfQueryObject( NULL, clsFOLDERIMP, 0 ) == NULLHANDLE )
        {
          UtlMenuEnableItem( PID_FILE_MI_IMPORT );
        } /* endif */
        /**************************************************************/
        /* enable New folder only if not in Personal TM               */
        /**************************************************************/
        UtlMenuEnableItem( PID_FILE_MI_NEW );
        UtlMenuEnableItem( PID_UTILS_MI_MT     );
        UtlMenuEnableItem( PID_VIEW_MI_NAMES );
        UtlMenuEnableItem( PID_VIEW_MI_DETAILSDLG );
        UtlMenuEnableItem( PID_VIEW_MI_DETAILS );
        UtlMenuEnableItem( PID_UTILS_MI_DELEXPMAT );
        UtlMenuEnableItem( PID_VIEW_MI_SORT );
        UtlMenuEnableItem( PID_VIEW_MI_SOME );
        UtlMenuEnableItem( PID_VIEW_MI_ALL );

        // selectall/deselect all is only valid for non-tree view folder list...
        {
          PPROPSYSTEM pPropSys = GetSystemPropPtr();

          if ( !pPropSys->fUseIELikeListWindows )
          {
            UtlMenuEnableItem( PID_FILE_MI_SELECTALL );
            UtlMenuEnableItem( PID_FILE_MI_DESELECTALL );
          } /* endif */
        }

        UtlMenuEnableItem( PID_FILE_MI_PRINTLIST );

        iSelItems = SendMessage( pCommArea->hwndLB, LB_GETSELCOUNT, 0, 0 );

        sItem = QUERYSELECTIONHWND( pCommArea->hwndLB );
        if ( sItem != LIT_NONE )
        {
          if ( (BOOL)WinSendMsg( pCommArea->hwndLB,
                                 LM_EQF_QUERYITEMSTATE,
                                 MP1FROMSHORT( sItem ),
                                 NULL ) )
          {
            UtlMenuEnableItem( PID_FILE_MI_OPEN );
            if ( EqfQueryObject( NULL, clsFOLDEREXP, 0 ) == NULLHANDLE )
            {
              UtlMenuEnableItem( PID_FILE_MI_EXPORT );
            } /* endif */
            UtlMenuEnableItem( PID_FILE_MI_PROPERTIES );
            UtlMenuEnableItem( PID_FILE_MI_HTMLPROPS );
            UtlMenuEnableItem( PID_FILE_MI_RENAME );

            /**********************************************************/
            /* Check if folder contains documents before enabling     */
            /* document based menu items                              */
            /**********************************************************/
            {
              PSZ       pszObj = QuerySelFolderName( pCommArea, TRUE );
              if ( !FolIsFolderEmpty(pszObj) )
              {
                UtlMenuEnableItem( PID_UTILS_MI_REPORT );
                UtlMenuEnableItem( PID_FILE_MI_ANALYZE );
                UtlMenuEnableItem( PID_UTILS_MI_COUNT );
                if ( iSelItems <= 1 )
                {
                  UtlMenuEnableItem( PID_UTILS_MI_ARCHTM );
                  UtlMenuEnableItem( PID_UTILS_MI_EXPORTSEGS );
                  UtlMenuEnableItem( PID_UTILS_MI_DELETEDOCS );
                  UtlMenuEnableItem( PID_FILE_MI_FINDCHANGE );
                  UtlMenuEnableItem( PID_FILE_MI_FUZZYSEARCH );
                  UtlMenuEnableItem( PID_FILE_MI_SPELLCHECK );
                } /* endif */
              }
              else
              {
                UtlMenuEnableItem( PID_UTILS_MI_REPORT );
              } /* endif */

              // enable restore documents only when documents have been deleted
              if ( !FolIsRemovedDirEmpty( pszObj ) )
              {
                UtlMenuEnableItem( PID_UTILS_MI_RESTOREDOCS );
              } /* endif */

            }
          } /* endif */
          UtlMenuEnableItem( PID_FILE_MI_DELETE );
        } /* endif */
      }
      break;

    case WM_EQF_TOOLBAR_ENABLED:
      switch ( mp1 )
      {
        /**************************************************************/
        /* check for items to be enabled ..                           */
        /**************************************************************/
        case PID_FILE_MI_IMPORT:
          mResult = ( EqfQueryObject( NULL, clsFOLDERIMP, 0) == NULLHANDLE );
          break;
        case PID_UTILS_MI_MT:
        case PID_FILE_MI_NEW:
        case PID_VIEW_MI_NAMES:
        case PID_VIEW_MI_DETAILSDLG:
        case PID_VIEW_MI_DETAILS:
        case PID_UTILS_MI_DELEXPMAT:
        case PID_FILE_MI_PRINTLIST:
        case PID_VIEW_MI_SORT:
        case PID_VIEW_MI_SOME:
        case PID_VIEW_MI_ALL:
          mResult = MRFROMSHORT(TRUE);
          break;
        case PID_FILE_MI_SELECTALL:
        case PID_FILE_MI_DESELECTALL:
          // selectall/deselect all is only valid for non-tree view folder list...
          {
            PPROPSYSTEM pPropSys = GetSystemPropPtr();

            if ( !pPropSys->fUseIELikeListWindows )
            {
              mResult = MRFROMSHORT(TRUE);
            } /* endif */
          }
          break;
        case PID_FILE_MI_OPEN:
        case PID_FILE_MI_PROPERTIES:
        case PID_FILE_MI_HTMLPROPS:
        case PID_FILE_MI_RENAME:
          {
            SHORT sItem = QUERYSELECTIONHWND( pCommArea->hwndLB );
            if ( sItem != LIT_NONE )
            {
              mResult = WinSendMsg( pCommArea->hwndLB, LM_EQF_QUERYITEMSTATE,
                                    MP1FROMSHORT(sItem), 0L );
            } /* endif */
          }
          break;
        case PID_FILE_MI_EXPORT:
          {
            SHORT sItem = QUERYSELECTIONHWND( pCommArea->hwndLB );
            if ( sItem != LIT_NONE )
            {
              mResult = (WinSendMsg( pCommArea->hwndLB, LM_EQF_QUERYITEMSTATE,
                                     MP1FROMSHORT(sItem), 0L )
                         && (EqfQueryObject( NULL, clsFOLDEREXP, 0) == NULLHANDLE));
            } /* endif */
          }
          break;

        // one or more folders are allowed
        case PID_FILE_MI_ANALYZE:
        case PID_UTILS_MI_COUNT:
        case PID_UTILS_MI_REPORT:
          {
            SHORT sItem = QUERYSELECTIONHWND( pCommArea->hwndLB );
            if ( sItem != LIT_NONE )
            {
              if ( WinSendMsg( pCommArea->hwndLB, LM_EQF_QUERYITEMSTATE,
                               MP1FROMSHORT(sItem), 0L ))
              {
                PSZ       pszObj = QuerySelFolderName( pCommArea, TRUE );
                if ( !FolIsFolderEmpty(pszObj) )
                {
                  mResult = MRFROMSHORT( TRUE );
                }
                else if ( mp1 == PID_UTILS_MI_REPORT )
                {
                  mResult = MRFROMSHORT( TRUE );
                } /* endif */
              }
            } /* endif */
          }
          break;

        // only one folder may be selected
        case PID_UTILS_MI_ARCHTM:
        case PID_UTILS_MI_EXPORTSEGS:
        case PID_UTILS_MI_DELETEDOCS:
          {
            int iSelItems;                      // number of selected items

            SHORT sItem = QUERYSELECTIONHWND( pCommArea->hwndLB );
            iSelItems = SendMessage( pCommArea->hwndLB, LB_GETSELCOUNT, 0, 0 );
            if ( (sItem != LIT_NONE) && (iSelItems <= 1) )
            {
              if ( WinSendMsg( pCommArea->hwndLB, LM_EQF_QUERYITEMSTATE,
                               MP1FROMSHORT(sItem), 0L ))
              {
                PSZ       pszObj = QuerySelFolderName( pCommArea, TRUE );
                if ( !FolIsFolderEmpty(pszObj) )
                {
                  mResult = MRFROMSHORT( TRUE );
                } /* endif */
              }
            } /* endif */
          }
          break;

        case PID_UTILS_MI_RESTOREDOCS:
          {
            int iSelItems;                      // number of selected items

            SHORT sItem = QUERYSELECTIONHWND( pCommArea->hwndLB );
            iSelItems = SendMessage( pCommArea->hwndLB, LB_GETSELCOUNT, 0, 0 );
            if ( (sItem != LIT_NONE) && (iSelItems <= 1) )
            {
              if ( WinSendMsg( pCommArea->hwndLB, LM_EQF_QUERYITEMSTATE, MP1FROMSHORT(sItem), 0L ))
              {
                PSZ       pszObj = QuerySelFolderName( pCommArea, TRUE );
                if ( !FolIsRemovedDirEmpty(pszObj) )
                {
                  mResult = MRFROMSHORT( TRUE );
                } /* endif */
              }
            } /* endif */
          }
          break;

 
        case PID_FILE_MI_FINDCHANGE:
        case PID_FILE_MI_FUZZYSEARCH:
        case PID_FILE_MI_SPELLCHECK:
        case PID_FILE_MI_DELETE:
          {
            SHORT sItem = QUERYSELECTIONHWND( pCommArea->hwndLB );
            mResult = MRFROMSHORT( (sItem != LIT_NONE) );
          }
          break;
        default:
          // always enable tool plugin menu items
          if ( (mp1 >= ID_TWBM_AAB_TOOLPLUGINS) && (mp1  < ID_TWBM_AAB_LAST) )
          {
            mResult = TRUE;
          }
          break;
      } /* endswitch */
      break;

    case WM_EQF_FLLTREE_COMMAND:
    case WM_EQF_COMMAND:
    case WM_COMMAND:
      mResult = MRFROMSHORT( TRUE ); // default return code for COMMAND msgs
      switch ( SHORT1FROMMP1(mp1) )
      {
        case PID_FILE_MI_NEW:
          EqfSend2Handler( FOLDERHANDLER, WM_EQF_CREATE, MP1FROMSHORT( TRUE ), 0L);
          break;

        case PID_FILE_MI_OPEN:
          {
            PSZ pszObj;
            if ( message == WM_EQF_FLLTREE_COMMAND )
            {
              pszObj = (PSZ) mp2;
            }
            else
            {
              pszObj = QuerySelFolderName( pCommArea, TRUE );
            } /* endif */

            if ( pszObj != NULL )
            {
              if ( !(BOOL)EqfSend2Handler( FOLDERHANDLER,
                                           WM_EQF_OPEN, MP1FROMSHORT(0),
                                           MP2FROMP(pszObj) ) )
              {
                SHORT sItem;

                sItem = QUERYSELECTIONHWND( pCommArea->hwndLB );
                if ( sItem != LIT_NONE )
                {
                  /****************************************************/
                  /* Open of folder failed, grey folder name          */
                  /****************************************************/
                  CLBSETITEMSTATEHWND( pCommArea->hwndLB, sItem, FALSE );
                } /* endif */
              } /* endif */
            } /* endif */
          }
          break;

        case PID_FILE_MI_DELETE:
          {
            if ( message == WM_EQF_FLLTREE_COMMAND )
            {
              if ( mp2 ) EqfSend2Handler( FOLDERHANDLER, WM_EQF_DELETE, MP1FROMSHORT(0), mp2 );
            }
            else
            {
              int iSelItems = SendMessage( pCommArea->hwndLB, LB_GETSELCOUNT, 0, 0 );
              if ( iSelItems  > 1 )
              {
                PSZ pszSelObjList = NULL;
                FolBuildSelItemslist( pCommArea, &pszSelObjList );
                if ( pszSelObjList )
                {
                  USHORT usYesToAllMode = MB_EQF_YESTOALL;
                  PSZ pszCurObj = pszSelObjList;
                  do
                  {
                    FolDeleteFolder( hwnd, pszCurObj, &usYesToAllMode );
                    pszCurObj += strlen(pszCurObj) + 1;
                  } while ( (usYesToAllMode != MBID_CANCEL) && *pszCurObj );
                  UtlAlloc( (PVOID *)&pszSelObjList, 0L, 0L, NOMSG );
                } /* endif */
              }
              else
              {
                PSZ pszObj = QuerySelFolderName( pCommArea, FALSE );
                if ( pszObj != NULL )
                {
                  EqfSend2Handler( FOLDERHANDLER, WM_EQF_DELETE, MP1FROMSHORT(0),
                                   MP2FROMP(pszObj));
                } /* endif */
              } /* endif */
            } /* endif */
          }
          break;

        case PID_FILE_MI_EXPORT:
          {
            if ( message == WM_EQF_FLLTREE_COMMAND )
            {
              EqfSend2Handler( FOLDERHANDLER, WM_EQF_PROCESSTASK,
                               MP1FROMSHORT( PID_FILE_MI_EXPORT ),
                               mp2 );
            }
            else
            {
              int iSelItems = SendMessage( pCommArea->hwndLB, LB_GETSELCOUNT, 0, 0 );
              if ( iSelItems  > 1 )
              {
                PSZ pszSelObjList = NULL;
                FolBuildSelItemslist( pCommArea, &pszSelObjList );
                if ( pszSelObjList )
                {
                  FolderExport( hwnd, pszSelObjList, pszSelObjList );
                } /* endif */
              }
              else
              {
                PSZ pszObj = QuerySelFolderName( pCommArea, TRUE  );
                if ( pszObj != NULL )
                {
                  EqfSend2Handler( FOLDERHANDLER, WM_EQF_PROCESSTASK,
                                   MP1FROMSHORT( PID_FILE_MI_EXPORT ),
                                   MP2FROMP( pszObj ) );
                } /* endif */
              } /* endif */
            } /* endif */
          }
          break;

        case PID_FILE_MI_IMPORT:
          EqfPost2Handler( FOLDERHANDLER, WM_EQF_PROCESSTASK,
                           MP1FROMSHORT( PID_FILE_MI_IMPORT ),
                           MP2FROMP( NULL ) );
          break;

        case PID_UTILS_MI_DELEXPMAT:
          EqfPost2Handler( FOLDERHANDLER, WM_EQF_PROCESSTASK,
                           MP1FROMSHORT( PID_UTILS_MI_DELEXPMAT ),
                           MP2FROMP( NULL ) );
          break;

        case PID_FILE_MI_ANALYZE:
          FolderListAnalysis( hwnd, pCommArea, message, mp1, mp2 );
          break;

        case PID_UTILS_MI_ARCHTM:
          {
            PSZ pszObj = NULL;
            if ( message == WM_EQF_FLLTREE_COMMAND )
            {
              pszObj = (PSZ) mp2;
            }
            else
            {
              pszObj = QuerySelFolderName( pCommArea, TRUE  );
            } /* endif */

            if ( pszObj != NULL )
            {
              TABuildArchTM( hwnd, pszObj );
            } /* endif */
          }
          break;

        case PID_UTILS_MI_EXPORTSEGS:
          {
            PSZ pszObj = NULL;
            if ( message == WM_EQF_FLLTREE_COMMAND )
            {
              pszObj = (PSZ) mp2;
            }
            else
            {
              pszObj = QuerySelFolderName( pCommArea, TRUE  );
            } /* endif */

            if ( pszObj != NULL )
            {
              ExportSegs( hwnd, pszObj );
            } /* endif */
          }
          break;

        case PID_UTILS_MI_DELETEDOCS:
          {
            PSZ pszObj = NULL;
            if ( message == WM_EQF_FLLTREE_COMMAND )
            {
              pszObj = (PSZ) mp2;
            }
            else
            {
              pszObj = QuerySelFolderName( pCommArea, TRUE  );
            } /* endif */

            if ( pszObj != NULL )
            {
              FolDeleteDocsFromList( hwnd, pszObj );
            } /* endif */
          }
          break;

        case PID_UTILS_MI_RESTOREDOCS:
          {
            PSZ pszObj = NULL;
            if ( message == WM_EQF_FLLTREE_COMMAND )
            {
              pszObj = (PSZ) mp2;
            }
            else
            {
              pszObj = QuerySelFolderName( pCommArea, TRUE  );
            } /* endif */

            if ( pszObj != NULL )
            {
              FolRestoreDeletedDocs( hwnd, pszObj );
            } /* endif */
          }
          break;


        case PID_UTILS_MI_COUNT:
          {
            PSZ pszObj;
            if ( message == WM_EQF_FLLTREE_COMMAND )
            {
              pszObj = (PSZ) mp2;
            }
            else
            {
              pszObj = QuerySelFolderName( pCommArea, TRUE  );
            } /* endif */

            if ( pszObj != NULL )
            {
              EqfSend2Handler( COUNTHANDLER, WM_EQF_CREATE,
                               MP1FROMSHORT( TRUE ), MP2FROMP( pszObj ) );
            } /* endif */
          }
          break;

        case PID_UTILS_MI_REPORT:
          {
            PSZ pszObj;
            if ( message == WM_EQF_FLLTREE_COMMAND )
            {
              pszObj = (PSZ) mp2;
            }
            else
            {
              pszObj = QuerySelFolderName( pCommArea, TRUE  );
            } /* endif */

            if ( pszObj != NULL )
            {
              EqfSend2Handler( REPORTHANDLER, WM_EQF_CREATE,
                               MP1FROMSHORT( TRUE ), MP2FROMP( pszObj ) );
            } /* endif */
          }
          break;

        case PID_FILE_MI_FINDCHANGE:
          {
            PSZ pszObj;
            if ( message == WM_EQF_FLLTREE_COMMAND )
            {
              pszObj = (PSZ) mp2;
              if ( !FolIsFolderDisabled( pszObj ) )
              {
                FolGlobFindChange( pszObj, TRUE, FALSE );
              } /* endif */                 
            }
            else
            {
              int iSelItems = SendMessage( pCommArea->hwndLB, LB_GETSELCOUNT, 0, 0 );
              if ( iSelItems  > 1 )
              {
                PSZ pszSelObjList = NULL;
                FolBuildSelItemslist( pCommArea, &pszSelObjList );
                if ( pszSelObjList )
                {
                  FolGlobFindChange( pszSelObjList, TRUE, TRUE );
                } /* endif */
              }
              else
              {
                pszObj = QuerySelFolderName( pCommArea, TRUE  );
                if ( pszObj != NULL )
                {
                  FolGlobFindChange( pszObj, TRUE, FALSE );
                } /* endif */
              } /* endif */
            } /* endif */
          }
          break;

        case PID_FILE_MI_FUZZYSEARCH:
          {
            PSZ pszObj;
            if ( message == WM_EQF_FLLTREE_COMMAND )
            {
              pszObj = (PSZ) mp2;
              if ( !FolIsFolderDisabled( pszObj ) )
              {
                FolFuzzySearch( pszObj, TRUE, FALSE );
              } /* endif */                     }
            else
            {
              int iSelItems = SendMessage( pCommArea->hwndLB, LB_GETSELCOUNT, 0, 0 );
              if ( iSelItems  > 1 )
              {
                PSZ pszSelObjList = NULL;
                FolBuildSelItemslist( pCommArea, &pszSelObjList );
                if ( pszSelObjList )
                {
                  FolFuzzySearch( pszSelObjList, TRUE, TRUE );
                } /* endif */
              }
              else
              {
                pszObj = QuerySelFolderName( pCommArea, TRUE  );
                if ( pszObj != NULL )
                {
                  FolFuzzySearch( pszObj, TRUE, FALSE );
                } /* endif */
              } /* endif */
            } /* endif */
          }
          break;

        case PID_FILE_MI_SPELLCHECK:
          {
            PSZ pszObj;
            if ( message == WM_EQF_FLLTREE_COMMAND )
            {
              pszObj = (PSZ) mp2;
              if ( !FolIsFolderDisabled( pszObj ) )
              {
                FolSpellcheck( pszObj, TRUE, FALSE );
              } /* endif */                     }
            else
            {
              int iSelItems = SendMessage( pCommArea->hwndLB, LB_GETSELCOUNT, 0, 0 );
              if ( iSelItems  > 1 )
              {
                PSZ pszSelObjList = NULL;
                FolBuildSelItemslist( pCommArea, &pszSelObjList );
                if ( pszSelObjList )
                {
                  FolSpellcheck( pszSelObjList, TRUE, TRUE );
                } /* endif */
              }
              else
              {
                pszObj = QuerySelFolderName( pCommArea, TRUE  );
                if ( pszObj != NULL )
                {
                  FolSpellcheck( pszObj, TRUE, FALSE );
                } /* endif */
              } /* endif */
            } /* endif */
          }
          break;

        case PID_UTILS_MI_MT:
          {
            PSZ pszObj;
            if ( message == WM_EQF_FLLTREE_COMMAND )
            {
              pszObj = (PSZ) mp2;
            }
            else
            {
              pszObj = QuerySelFolderName( pCommArea, TRUE  );
            } /* endif */

            if ( pszObj != NULL )
            {
              EqfSend2Handler( MTLISTHANDLER, WM_EQF_OPEN,
                               MP1FROMSHORT( TRUE ), MP2FROMP( pszObj ) );
            } /* endif */
          }
          break;

        case PID_FILE_MI_RENAME:
          {
            PSZ pszObj;
            if ( message == WM_EQF_FLLTREE_COMMAND )
            {
              pszObj = (PSZ) mp2;
            }
            else
            {
              pszObj = QuerySelFolderName( pCommArea, TRUE  );
            } /* endif */

            if ( pszObj != NULL )
            {
              UtlRenameObjectDlg( pszObj, clsFOLDER );
            } /* endif */
          }
          break;

        case PID_FILE_MI_PRINTLIST:
          // pass message to column listbox control
          WinSendMsg( pCommArea->hwndLB, message, mp1, mp2 );
          break;

        case PID_FILE_MI_PROPERTIES:
          FolderListFolderProperties( hwnd, pCommArea, message, mp1, mp2 );
          break;

        case PID_FILE_MI_HTMLPROPS:
          {
            BOOL fOk = TRUE;
            PSZ pszObj;
            if ( message == WM_EQF_FLLTREE_COMMAND )
            {
              pszObj = (PSZ) mp2;
            }
            else
            {
              pszObj = QuerySelFolderName( pCommArea, TRUE  );
            } /* endif */

            fOk = EqfFolderPropsToHtml( pszObj, hwnd );

          }
          break;


        default:
          {
            short sCommand = SHORT1FROMMP1(mp1);
            if ( (sCommand >= ID_TWBM_AAB_TOOLPLUGINS) && (sCommand  < ID_TWBM_AAB_LAST) )
            {
              PluginManager *pPluginManager = PluginManager::getInstance();
              pPluginManager->processToolCommand( sCommand );
            }
            else if ( message == WM_EQF_COMMAND)
            {
              mResult = MRFROMSHORT( FALSE ); // tell twbmain that we rejected
            } /* endif */
          }
      }
      break;

    default:
      break;
  } /* endswitch */
  return( mResult );
} /* end of function FolderListCallBack */

/**********************************************************************/
/* Handle the WM_CREATE message                                       */
/**********************************************************************/
MRESULT  FolderListCreateCall( PLISTCOMMAREA pCommArea )
{
  MRESULT    mResult = MRFROMSHORT(FALSE);
  BOOL       fOK = TRUE;         // initialisation is O.K. flag
  EQFINFO    ErrorInfo;          // error info of property handler
  HPROP      hProp = NULL;       // folder list properties handle
  PPROPFOLDERLIST pProp = NULL;  // ptr to folder list properties
  PSZ        pszParm;            // ptr to error message parameters
  HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
  //BOOL       fSetWidth = TRUE;

  /**************************************************************/
  /* Open folder list properties                                */
  /**************************************************************/
  hProp = OpenProperties( pCommArea->szObjName, NULL,
                          PROP_ACCESS_READ, &ErrorInfo);
  if ( hProp )
  {
    pProp = (PPROPFOLDERLIST) MakePropPtrFromHnd( hProp );
  }
  else
  {
    pszParm = pCommArea->szObjName;
    UtlError( ERROR_OPEN_PROPERTIES, MB_CANCEL, 1, &pszParm, EQF_ERROR );
    fOK = FALSE;
  } /* endif */

  /**************************************************************/
  /* Load column listbox title strings                          */
  /**************************************************************/
  if ( fOK  )
  {
    LOADSTRING( NULLHANDLE, hResMod, SID_FLL_NAME_COLTITLE,   ColHdr[1]);
    UtlLoadWidth( NULLHANDLE, hResMod, SID_FLL_NAME_COLWIDTH,
                  &(ColTable[1].usWidth) );
    LOADSTRING( NULLHANDLE, hResMod, SID_FLL_DESCR_COLTITLE,  ColHdr[2]);
    UtlLoadWidth( NULLHANDLE, hResMod, SID_FLL_DESCR_COLWIDTH,
                  &(ColTable[2].usWidth) );
    LOADSTRING( NULLHANDLE, hResMod, SID_FLL_DATE_COLTITLE,   ColHdr[3]);
    UtlLoadWidth( NULLHANDLE, hResMod, SID_FLL_DATE_COLWIDTH,
                  &(ColTable[3].usWidth) );
    LOADSTRING( NULLHANDLE, hResMod, SID_FLL_FORMAT_COLTITLE, ColHdr[4]);
    UtlLoadWidth( NULLHANDLE, hResMod, SID_FLL_FORMAT_COLWIDTH,
                  &(ColTable[4].usWidth) );
    LOADSTRING( NULLHANDLE, hResMod, SID_FLL_MEM_COLTITLE,    ColHdr[5]);
    UtlLoadWidth( NULLHANDLE, hResMod, SID_FLL_MEM_COLWIDTH,
                  &(ColTable[5].usWidth) );
    LOADSTRING( NULLHANDLE, hResMod, SID_FLL_DRIVE_COLTITLE,  ColHdr[6]);
    UtlLoadWidth( NULLHANDLE, hResMod, SID_FLL_DRIVE_COLWIDTH,
                  &(ColTable[6].usWidth) );
    LOADSTRING( NULLHANDLE, hResMod, SID_FLL_EDIT_COLTITLE,   ColHdr[8]);
    UtlLoadWidth( NULLHANDLE, hResMod, SID_FLL_EDIT_COLWIDTH,
                  &(ColTable[8].usWidth) );
    LOADSTRING( NULLHANDLE, hResMod, SID_FLL_SOURCE_COLTITLE, ColHdr[9]);
    UtlLoadWidth( NULLHANDLE, hResMod, SID_FLL_SOURCE_COLWIDTH,
                  &(ColTable[9].usWidth) );
    LOADSTRING( NULLHANDLE, hResMod, SID_FLL_TARGET_COLTITLE, ColHdr[10]);
    UtlLoadWidth( NULLHANDLE, hResMod, SID_FLL_TARGET_COLWIDTH,
                  &(ColTable[10].usWidth) );

    LOADSTRING( NULLHANDLE, hResMod, SID_FLL_DICTLIST_COLTITLE, ColHdr[11]);
    UtlLoadWidth( NULLHANDLE, hResMod, SID_FLL_DICTLIST_COLWIDTH,
                  &(ColTable[11].usWidth) );
    LOADSTRING( NULLHANDLE, hResMod, SID_FLL_MEMLIST_COLTITLE, ColHdr[12]);
    UtlLoadWidth( NULLHANDLE, hResMod, SID_FLL_MEMLIST_COLWIDTH,
                  &(ColTable[12].usWidth) );
  } /* endif */

  /**************************************************************/
  /* Set column listbox view lists                              */
  /**************************************************************/
  if ( fOK  )
  {
    int i;
    memcpy( pCommArea->asCurView,
            (pProp->sLastUsedViewList[0] != 0) ? pProp->sLastUsedViewList :
            sLastUsedView,
            sizeof(pCommArea->asCurView) );
    FllCLBData.psLastUsedViewList = pCommArea->asCurView;

    memcpy( pCommArea->asDetailsView,
            (pProp->sDetailsViewList[0] != 0) ? pProp->sDetailsViewList :
            sDetailsView,
            sizeof(pCommArea->asDetailsView) );
    FllCLBData.psDetailsViewList = pCommArea->asDetailsView;

    memcpy( pCommArea->asSortList,
            (pProp->sSortList[0] != 0) ? pProp->sSortList :
            sSortCriteria,
            sizeof(pCommArea->asSortList) );
    FllCLBData.psSortList = pCommArea->asSortList;

    memcpy( &(pCommArea->Filter), &(pProp->Filter),
            sizeof(pCommArea->Filter) );
    FllCLBData.pFilter = &(pCommArea->Filter);

    for (i=0;pCommArea->asCurView[i]>0 && i<MAX_VIEW;i++)
    {
      int index = pCommArea->asCurView[i];
      if ( index < FllCLBData.usNoOfColumns )
      {
        if (pProp->sLastUsedViewWidth[i] > 0)
        {
          ColTable[index].usWidth = pProp->sLastUsedViewWidth[i];
        }
        else
        {
          pProp->sLastUsedViewWidth[i] = ColTable[index].usWidth;
        } /* endif */
      } /* endif */
    } /* endfor */
    memcpy( pCommArea->asCurViewWidth, (pProp->sLastUsedViewWidth),
            sizeof(pCommArea->asCurViewWidth) );
  } /* endif */

  /****************************************************************/
  /* supply all information required to create a folder list      */
  /****************************************************************/
  if ( fOK )
  {
    PPROPSYSTEM pPropSys = GetSystemPropPtr();

    pCommArea->sListObjClass = clsFOLDERLIST;
    LOADSTRING( NULLHANDLE, hResMod, SID_FLLI_TITLE, pCommArea->szTitle );
    pCommArea->hIcon          = (HPOINTER) UtlQueryULong(QL_FLLICON); //hiconFLL;
    pCommArea->fNoClose       = TRUE;
    pCommArea->sObjNameIndex  = FLL_OBJECT_IND;
    pCommArea->sNameIndex     = FLL_NAME_IND;
    pCommArea->sListWindowID  = ID_FLIST_WINDOW;
    pCommArea->sListboxID     = PID_FLIST_LB;
    pCommArea->sPopupMenuID   = ID_FLL_POPUP;

    pCommArea->sGreyedPopupMenuID   = ID_FLL_POPUP;
    if ( pPropSys->fUseIELikeListWindows )
    {
      pCommArea->sNoSelPopupMenuID = ID_FLL_POPUP_IE_NOSEL;
    }
    else
    {
      pCommArea->sNoSelPopupMenuID = ID_FLL_POPUP_NOSEL;
    } /* endif */
    pCommArea->pColData       = &FllCLBData;
    pCommArea->fMultipleSel   = TRUE;
    pCommArea->sMultPopupMenuID = ID_FLL_POPUP_MULTSEL;
    pCommArea->sDefaultAction = PID_FILE_MI_OPEN;
    memcpy( &(pCommArea->swpSizePos), &(pProp->Swp),
            sizeof(EQF_SWP) );
    pCommArea->sItemClass     = clsFOLDER;
    pCommArea->sItemPropClass = PROP_CLASS_FOLDER;

  } /* endif */

  /**************************************************************/
  /* Close properties                                           */
  /**************************************************************/
  if ( hProp )
  {
    CloseProperties( hProp, PROP_QUIT, &ErrorInfo );
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

/*+--------------------------------------------------------------------------+
  |  Fill List Box with folder names                                         |
  +--------------------------------------------------------------------------+
*/
void FllLoadFolderNames
(
HWND   lbox,                        // handle of listbox
PSZ    pszBuffer                    // buffer for item text
)
{
  PVOID           hProp;              // handle of folder properties
  PPROPFOLDER     pProp;              // pointer to folder properties
  EQFINFO         ErrorInfo;          // error code of property handler calls
  FILEFINDBUF     ResultBuf;          // DOS file find struct
  USHORT          usCount;
  USHORT          usRC;               // return value of Utl/Dos calls
  HDIR            hDirHandle = HDIR_CREATE; // DosFind routine handle
  PSZ             pszObjName;         // ptr to buffer for object name
  PSZ             pszObjPath;         // ptr to buffer for object paths
  PSZ             pszSearchPath;      // ptr to buffer for search path
  PSZ             pszName;            // ptr to name in FILEFINDBUF structure
  PPROPSYSTEM     pSysProp;           // ptr to EQF system properties
  SHORT           sItem;
  CHAR            chTempName[MAX_FNAME];   // buffer for folder name

  SETCURSOR( SPTR_WAIT );
  ENABLEUPDATEHWND_FALSE( lbox );
  DELETEALLHWND( lbox );
  pSysProp = (PPROPSYSTEM) MakePropPtrFromHnd( EqfQuerySystemPropHnd());

  //--- allocate buffer for paths ---
  UtlAlloc( (PVOID *)&pszObjName, 0L, (LONG) (3 * MAX_EQF_PATH), ERROR_STORAGE );
  pszSearchPath = pszObjName + MAX_EQF_PATH;
  pszObjPath    = pszSearchPath + MAX_EQF_PATH;
  pszName = RESBUFNAME(ResultBuf);

  if ( pszObjName )
  {
    //--- get path to folder properties ---
    UtlMakeEQFPath( pszObjPath, NULC,  SYSTEM_PATH, NULL );
    UtlMakeEQFPath( pszSearchPath, NULC, PROPERTY_PATH, NULL );
    sprintf( pszSearchPath + strlen(pszSearchPath), "\\*%s", EXT_FOLDER_MAIN );

    //--- loop through all folder properties ---
    usCount = 1;
    usRC = UtlFindFirst( pszSearchPath, &hDirHandle, FILE_NORMAL,
                         &ResultBuf, sizeof( ResultBuf),
                         &usCount, 0L, TRUE );
    usCount = ( usRC ) ? 0 : usCount;
    while ( usCount)
    {
      /**************************************************************/
      /* Only process folders which names which do not start with   */
      /* $$ (identifier for ITM temp folders)                       */
      /**************************************************************/
      if ( !((pszName[0] == '$') && (pszName[1] == '$')) )
      {
        //--- check properties of folder and folder directory ---
        hProp = OpenProperties( pszName,
                                pszObjPath,
                                PROP_ACCESS_READ,
                                &ErrorInfo);
        if ( hProp )
        {
          BOOL fFolDirsExist = TRUE;  // folder-directories-exist flag

          pProp = (PPROPFOLDER) MakePropPtrFromHnd( hProp );
          strcpy( pszObjName, pszObjPath );
          strcat( pszObjName, "\\" );
          strcat( pszObjName, pszName );

          // correct older property files which have garbage instead of
          // drive letters in chDrive
          if ( (pProp->chDrive < 'A') || (pProp->chDrive > 'Z') )
          {
            pProp->chDrive = pSysProp->szPrimaryDrive[0];
          } /* endif */

          /***********************************************************/
          /* Insert item into listbox                                */
          /***********************************************************/
          *pszObjName = pProp->chDrive;
          FLLMakeFolderListItem( pszObjName, hProp, pszBuffer );
          sItem = INSERTITEMHWND( lbox, pszBuffer );

          /***********************************************************/
          /* Check folder directories                                */
          /***********************************************************/
          if ( sItem >= 0 )                // if item has been inserted ...
          {
            if ( strchr( pSysProp->szDriveList, pProp->chDrive ) == NULL )
            {
              // drive is not in our list of valid EQF drives
              fFolDirsExist = FALSE;
            } /* endif */

            if ( fFolDirsExist )
            {
              fFolDirsExist = UtlDirExist( pszObjName );
            } /* endif */

            if ( fFolDirsExist )
            {
              UtlMakeEQFPath( pszObjName, pProp->chDrive, PROPERTY_PATH,
                              pszName );
              fFolDirsExist = UtlDirExist( pszObjName);
            } /* endif */


            if ( fFolDirsExist )
            {
              UtlMakeEQFPath( pszObjName, pProp->chDrive, DIRSOURCEDOC_PATH,
                              pszName );
              fFolDirsExist = UtlDirExist(pszObjName);
            } /* endif */

            if ( fFolDirsExist )
            {
              UtlMakeEQFPath( pszObjName, pProp->chDrive, DIRSEGSOURCEDOC_PATH,
                              pszName );
              fFolDirsExist = UtlDirExist(pszObjName);
            } /* endif */

            if ( fFolDirsExist )
            {
              UtlMakeEQFPath( pszObjName, pProp->chDrive, DIRSEGTARGETDOC_PATH,
                              pszName );
              fFolDirsExist = UtlDirExist(pszObjName);
            } /* endif */

            if ( fFolDirsExist )
            {
              UtlMakeEQFPath( pszObjName, pProp->chDrive, DIRTARGETDOC_PATH,
                              pszName );
              fFolDirsExist = UtlDirExist(pszObjName);
            } /* endif */

            /***********************************************************/
            /* Disable item if not all directories of folder exist     */
            /***********************************************************/
            if ( !fFolDirsExist || pProp->fDisabled_UserExitRefresh )
            {
              CLBSETITEMSTATEHWND( lbox, sItem, FALSE );
            } /* endif */
          } /* endif */

          CloseProperties( hProp, PROP_QUIT, &ErrorInfo );
        }
        else
        {
          /*************************************************************/
          /* Insert as disabled item                                  */
          /*************************************************************/
          Utlstrccpy( chTempName, pszName, DOT );
          sprintf( pszBuffer,
                   "%s\\%s\x15%s\x15 \x15 0\x15 \x15 \x15 \x15 \x15 \x15",
                   pszObjPath,                // FLL_OBJ_IND
                   pszName,
                   chTempName                 // FLL_NAME_IND
                 );
          sItem = INSERTITEMHWND( lbox, pszBuffer);
          if ( sItem != LIT_NONE )
          {
            CLBSETITEMSTATEHWND( lbox, sItem, FALSE );
          } /* endif */
        } /* endif */
      } /* endif */

      usRC = UtlFindNext( hDirHandle, &ResultBuf, sizeof( ResultBuf),
                          &usCount, TRUE );
      usCount = ( usRC ) ? 0 : usCount;
    } /* endwhile */
    // close search file handle
    if ( hDirHandle != HDIR_CREATE ) UtlFindClose( hDirHandle, FALSE );


  } /* endif */

  if ( pszObjName )
  {
    UtlAlloc( (PVOID *)&pszObjName, 0L, 0L, NOMSG );
  } /* endif */

  SETCURSOR( SPTR_ARROW );

  if ( QUERYITEMCOUNTHWND( lbox ) )
  {
    SELECTITEMHWND( lbox, 0 );
  } /* endif */

  ENABLEUPDATEHWND_TRUE( lbox );
}

BOOL FLLMakeFolderListItem
(
PSZ       pszObjName,               // folder object name
PVOID     hProp,                    // property handle
PSZ       pszBuffer                 // buffer to fill with folderlist item
)
{
  PPROPFOLDER     pProp;              // ptr to folder properties
  CHAR      chTempName[MAX_FNAME];   // buffer for folder name
  CHAR      chDrive[3];               // buffer for drive letter string
  CHAR      chDictionary[MAX_FILESPEC];        // buffer for first selected dictionary
  static CHAR szDicts[NUM_OF_FOLDER_DICS*MAX_LONGFILESPEC];// buffer for dicts
  static CHAR szTMs[MAX_NUM_OF_READONLY_MDB*MAX_LONGFILESPEC]; // buffer for TMs

  pProp = (PPROPFOLDER) MakePropPtrFromHnd( hProp );

  /*******************************************************************/
  /* Set languages for older folders ...                             */
  /*******************************************************************/
  if ( (pProp->szSourceLang[0] == EOS) || (pProp->szTargetLang[0] == EOS) )
  {
    FolUpdLangFromMemory( hProp, FALSE );
  } /* endif */


  //--- get filename up to extension delimiter ---
  Utlstrnccpy( chTempName,
               pProp->PropHead.szName,
               sizeof(chTempName) - 1,
               '.' );

  //--- build drive letter string ---
  sprintf( chDrive, "%c:", pProp->chDrive );

  //--- extract name of first dictionary ---
  Utlstrnccpy( chDictionary,
               pProp->DicTbl,
               sizeof(chDictionary) - 1,
               '\x15' );

  // setup strings for search TMs and dictionaries

  // copy table to buffer and replace all X15 by comma
  if ( pProp->aLongDicTbl[0][0] != EOS )
  {
    int i = 0;
    szDicts[0] = EOS;
    while ( (i < MAX_NUM_OF_FOLDER_DICS) &&
            (pProp->aLongDicTbl[i][0] != EOS) )
    {
      if ( szDicts[0] != EOS )        // not first dict in buffer?
      {
        strcat( szDicts, "," );
      } /* endif */
      strcat( szDicts, pProp->aLongDicTbl[i] );
      i++;
    } /* endwhile */
  }
  else
  {
    strcpy( szDicts, pProp->DicTbl );
    {
      PSZ pszTemp = szDicts;
      while ( *pszTemp != EOS )
      {
        if ( *pszTemp == X15 )
        {
          if ( pszTemp[1] == EOS )      // last X15 in string ???
          {
            *pszTemp = EOS;             //   yes -> discard X15
          }
          else
          {
            *pszTemp = COMMA;           //   no -> replace by comma
          } /* endif */
        } /* endif */
        pszTemp++;
      } /* endwhile */
    }
  } /* endif */

  // copy table to buffer and replace all X15 by comma
  if ( pProp->aLongMemTbl[0][0] != EOS )
  {
    int i = 0;
    szTMs[0] = EOS;
    while ( (i < MAX_NUM_OF_READONLY_MDB) &&
            (pProp->aLongMemTbl[i][0] != EOS) )
    {
      if ( szTMs[0] != EOS )        // not first dict in buffer?
      {
        strcat( szTMs, "," );
      } /* endif */
      strcat( szTMs, pProp->aLongMemTbl[i] );
      i++;
    } /* endwhile */
  }
  else
  {
    strcpy( szTMs, pProp->MemTbl );
    {
      PSZ pszTemp = szTMs;
      while ( *pszTemp != EOS )
      {
        if ( *pszTemp == X15 )
        {
          if ( pszTemp[1] == EOS )      // last X15 in string ???
          {
            *pszTemp = EOS;             //   yes -> discard X15
          }
          else
          {
            *pszTemp = COMMA;           //   no -> replace by comma
          } /* endif */
        } /* endif */
        pszTemp++;
      } /* endwhile */
    }
  } /* endif */

  OEMTOANSI( pProp->szDescription );
  OEMTOANSI( pProp->szLongName );
  OEMTOANSI( pProp->szLongMemory );
  OEMTOANSI( szDicts );
  OEMTOANSI( szTMs );
  sprintf( pszBuffer,
           "%c%s\x15%s\x15%s\x15%lu\x15%s\x15%s\x15%s\x15%s\x15%s\x15%s\x15%s\x15%s\x15%s\x15",
           pProp->chDrive,            // property drive letter
           pszObjName+1,              // FLL_OBJ_IND
           (pProp->szLongName[0] != EOS) ? pProp->szLongName : chTempName,
           pProp->szDescription,      // FLL_DESCR_IND
           pProp->ulLastChange,       // FLL_DATE_IND
           pProp->szFormat,           // FLL_FORMAT_IND
           (pProp->szLongMemory[0] != EOS) ? pProp->szLongMemory : pProp->szMemory,
           chDrive,                   // FLL_DRIVE_IND
           chDictionary,              // FLL_DICT_IND
           pProp->szEditor,           // FLL_EDIT_IND
           pProp->szSourceLang,       // FLL_SOURCE_IND
           pProp->szTargetLang,       // FLL_TARGET_IND
           szDicts,
           szTMs
         );
  ANSITOOEM( pProp->szDescription );
  ANSITOOEM( pProp->szLongName );
  ANSITOOEM( pProp->szLongMemory );

  return( TRUE );
}


/*+--------------------------------------------------------------------------+
  |  Query selected item text from list box                                  |
  +--------------------------------------------------------------------------+
*/
PSZ QuerySelFolderName( PLISTCOMMAREA pCommArea, BOOL fCheckItemState )
{
  PSZ  pszObject = NULL;
  SHORT sItem;

  sItem = QUERYSELECTIONHWND( pCommArea->hwndLB );
  if ( sItem != LIT_NONE )
  {
    // Win2000 corrupts our data if we use LB_GETTEXT so use own message instead
//  QUERYITEMTEXTHWND( pCommArea->hwndLB, sItem, pCommArea->szBuffer );
    SendMessage( pCommArea->hwndLB, LM_EQF_QUERYITEMTEXT, (WPARAM)sItem, (LPARAM)pCommArea->szBuffer );
    if ( fCheckItemState && !CLBQUERYITEMSTATEHWND( pCommArea->hwndLB, sItem ) )
    {
      PSZ   pszParms[2];

      /******************************************************/
      /* Folder is disabled and cannot be worked with       */
      /******************************************************/
      pszParms[0] = UtlParseX15( pCommArea->szBuffer, FLL_NAME_IND );
      pszParms[1] = UtlParseX15( pCommArea->szBuffer, FLL_DRIVE_IND );
      UtlError( FOL_FOLDER_ACCESS_ERROR, MB_CANCEL, 2, pszParms, EQF_ERROR );
    }
    else
    {
      pszObject = UtlParseX15( pCommArea->szBuffer, FLL_OBJECT_IND );
    } /* endif */
  } /* endif */
  return( pszObject );
}

// function creating object name list of selected folders
int FolBuildSelItemslist
(
  PLISTCOMMAREA    pCommArea,
  PSZ              *ppszList
)
{
  int iSelItems, iItems;
  PSZ pszList = NULL;

  // get number of selected TMs and build name list
  iItems = iSelItems = SendMessage( pCommArea->hwndLB, LB_GETSELCOUNT, 0, 0 );

  if ( iSelItems )
  {
    // allocate buffer for the object name list
    BOOL fOK = UtlAlloc( (PVOID *)&pszList, 0, (sizeof(OBJNAME)*iSelItems)+10,
                         ERROR_STORAGE );

    // fill-in object names of selected folders
    if ( fOK )
    {
      PSZ pszCurPos = pszList;
      SHORT sItem = LIT_FIRST;

      while ( iSelItems )
      {
        sItem = QUERYNEXTSELECTIONHWND( pCommArea->hwndLB, sItem );
        if ( sItem >= 0 )
        {
          SendMessage( pCommArea->hwndLB, LM_EQF_QUERYITEMTEXT, (WPARAM)sItem, (LPARAM)pCommArea->szBuffer );
          strcpy( pszCurPos, UtlParseX15( pCommArea->szBuffer, FOL_OBJECT_IND ) );
          pszCurPos += strlen( pszCurPos ) + 1;
        } /* endif */
        iSelItems--;
      } /* endwhile */
      *pszCurPos = EOS;          // terminate list
    } /* endif */
  } /* endif */
  *ppszList = pszList;
  return( iItems );
} /* end of function FolBuildSelItemslist */

// check if directory for removed documents is empty
BOOL FolIsRemovedDirEmpty( PSZ pszFolObjName )
{
  CHAR szDocPath[MAX_EQF_PATH];
  USHORT    usCount = 1;
  FILEFINDBUF ResultBuf;
  USHORT    usRC;
  HDIR      hDirHandle = HDIR_CREATE;
  BOOL      fIsEmpty = TRUE;

  // setup path to folder documents
  strcpy( szDocPath, pszFolObjName );
  strcat( szDocPath, BACKSLASH_STR );
  UtlQueryString( QST_REMOVEDDOCDIR, szDocPath + strlen(szDocPath), MAX_FILESPEC );
  strcat( szDocPath, BACKSLASH_STR );
  UtlQueryString( QST_PROPDIR, szDocPath + strlen(szDocPath), MAX_FILESPEC );
  strcat( szDocPath, BACKSLASH_STR );
  strcat( szDocPath, DEFAULT_PATTERN );

  // look for removed documents
  usRC = UtlFindFirst( szDocPath, &hDirHandle, FILE_NORMAL, &ResultBuf,
                       sizeof( ResultBuf), &usCount, 0L, FALSE );
  if ( usRC == NO_ERROR )
  {
    UtlFindClose( hDirHandle, FALSE );
    fIsEmpty = (usCount == 0);
  } /* endif */
  return( fIsEmpty );
} /* end of function FolIsRemovedDirEmpty */

// function to remove documents from a folder based on a list in a text file (GUI version)
USHORT FolDeleteDocsFromList( HWND hwndOwner, PSZ pszFolObjName )
{
  USHORT usRC = 0;                     // function return code
  int iNumOfDocs = 0;                  // number of documents deleted
  CHAR szFileName[MAX_LONGFILESPEC];
  OPENFILENAME OpenStruct;

  // use standard file open dialog for file containing the document names

  // initial processing
  szFileName[0] = EOS;
  memset( &OpenStruct, 0, sizeof(OpenStruct) );
  OpenStruct.lStructSize = sizeof(OpenStruct);
  OpenStruct.hwndOwner = hwndOwner;
  OpenStruct.lpstrInitialDir = NULL;
  OpenStruct.lpstrFile = szFileName;
  OpenStruct.nMaxFile = sizeof(szFileName)-1;
  OpenStruct.lpstrFileTitle = NULL;
  OpenStruct.nMaxFileTitle = 0;
  OpenStruct.lpstrTitle = "Select file containing the list of documents being removed";
  OpenStruct.Flags = OFN_FILEMUSTEXIST | OFN_LONGNAMES | OFN_EXPLORER | OFN_NOCHANGEDIR | OFN_PATHMUSTEXIST;
  if ( GetOpenFileName( &OpenStruct ) )
  {
    // call internal function doing the actual processing
    usRC = FolIntRemoveDocs( pszFolObjName, szFileName, &iNumOfDocs, hwndOwner );

    // show completion message
    if ( !usRC )
    {
      CHAR szDocs[10];
      PSZ  pszParm = szDocs;
      sprintf( szDocs, "%ld", iNumOfDocs );
      UtlError( INFO_REMOVE_COMPLETE, MB_OK, 1, &pszParm, EQF_INFO );
    } /* end */     
  } /* endif */

  return( usRC );
} /* end of function FolDeleteDocsFromList */


// function to restored documents of a folder previously removed using FolDeleteDocsFromList
USHORT FolRestoreDeletedDocs( HWND hwndOwner, PSZ pszFolObjName )
{
  USHORT usRC = 0;                     // function return code
  int iNumOfDocs = 0;                  // number of documents restored

  // get user confirmation
  UtlError( QUERY_DO_RESTORE, MB_OKCANCEL, 0, NULL, EQF_QUERY );

  // call internal function doing the actual processing
  usRC = FolIntRestoreDocs( pszFolObjName, &iNumOfDocs, hwndOwner );

  // show completion message
  if ( !usRC )
  {
    CHAR szDocs[10];
    PSZ  pszParm = szDocs;
    sprintf( szDocs, "%ld", iNumOfDocs );
    UtlError( INFO_RESTORE_COMPLETE, MB_OK, 1, &pszParm, EQF_INFO );
  } /* end */     

  return( usRC );
} /* end of function FolRestoreDeletedDocs */


//   End of EQFFLL00.CPP
