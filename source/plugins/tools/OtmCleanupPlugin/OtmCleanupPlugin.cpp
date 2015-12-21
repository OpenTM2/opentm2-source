/*! \file
	Copyright Notice:

	Copyright (C) 1990-2015, International Business Machines
	Corporation and others. All rights reserved
*/

#include "eqf.h"
#include "core\PluginManager\PluginManager.h"
#include "OtmCleanupPlugin.h"
#include "OtmCleanupPlugin.id"

#include "string"
#include "vector"
#include "windows.h"

#define WM_CLEANUP_GETCHECKSTATE WM_USER + 100
#define WM_CLEANUP_SELECTALL WM_USER + 101

#define CONTROLFILENAME  "OtmCleanup.lst"     // name of control file containing the list of removable temp files
#define PENDINGDELETESFILE "OtmCleanupPendingDeletes.lst" // name of file containing the list of pending deletes

typedef struct _FILETODELETE
{
  boolean     fSelected;                     // file selecte state
  boolean     fisDirectory;                  // true = entry is a directory
  std::string strFileName;                   // fully qualified file name
  _int64      iSize;                         // size of file in kB
  int         iFiles;                        // number of files (in case of directories)
} FILETODELETE, *PFILETODELETE;

typedef struct _CLEANUPDATA
{
  char szControlFileName[512];               // fully qualified name of control file containing the list of removable temp files
  char szLine[2048];                         // buffer for lines of control file
  std::vector<std::string> vRemovePattern;   // list of remove patterns
  std::vector<FILETODELETE> vFileTodelete;   // list of files being deleted
  char        szBuffer[2048];                // general purpose buffer
} CLEANUPDATA, *PCLEANUPDATA;

BOOL CALLBACK CleanupDialogProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam);
boolean LoadControlFile( HWND hwndDlg, PCLEANUPDATA pData );
boolean ScanForFilesToBeDeleted( HWND hwndDlg, PCLEANUPDATA pData );
boolean FullyQualifyPattern( std::string &strSearchPattern );
boolean ScanDirectory( std::string &strDirectory, std::string &strNamePattern, std::vector<FILETODELETE> &vFileTodelete );
boolean ProcessPattern( PCLEANUPDATA pData, std::string &strPattern );
boolean ScanForFilesToBeDeleted( HWND hwndDlg, PCLEANUPDATA pData );
boolean ShowFilesToBeDeleted( HWND hwndDlg, PCLEANUPDATA pData );
boolean RefreshStatistics( HWND hwndDlg, PCLEANUPDATA pData );
boolean SetStateOfAllItems( HWND hwndDlg, PCLEANUPDATA pData, BOOL fCheck );
boolean DeleteFiles( HWND hwndDlg, PCLEANUPDATA pData );
boolean ProcessPendingDeletes();

HINSTANCE hDll;

extern "C" __declspec(dllexport)
BOOL APIENTRY DllMain( HANDLE hModule, DWORD ul_reason_for_call, LPVOID)
{	 
   switch (ul_reason_for_call) {
      case DLL_PROCESS_ATTACH:
         hDll = (HINSTANCE)hModule;
   }
   return TRUE;
}

OtmCleanupPlugin::OtmCleanupPlugin()
{
    
    name        = "OtmCleanupPlugin";
    shortDesc   = "Cleanup plugin";
    longDesc    = "This is a plugin to remove superfluous and temporary OpenTM2 files";
    version     = "1.0";
    supplier    = "International Business Machines Corporation";
    pluginType  = OtmPlugin::eToolType;
    usableState = OtmPlugin::eUsable;
}


OtmCleanupPlugin::~OtmCleanupPlugin()
{
}

const char* OtmCleanupPlugin::getName()
{
	return name.c_str();
}

const char* OtmCleanupPlugin::getShortDescription()
{
	return shortDesc.c_str();
}

const char* OtmCleanupPlugin::getLongDescription()
{
	return longDesc.c_str();
}

const char* OtmCleanupPlugin::getVersion()
{
	return version.c_str();
}

const char* OtmCleanupPlugin::getSupplier()
{
	return supplier.c_str();
}

/*! \brief Initialize plugin 
  This method is called when the workbench window has been created.
  The tool plugin can use the plugin manager methods addMenuItem and addSubMenu to add
  own entries to the workbench toolbar.
*/
void OtmCleanupPlugin::init()
{
	PluginManager *manager = PluginManager::getInstance();
  manager->addMenuItem( this, "Utilities", "Remove temporary files...", ID_CLEANUP_COMMAND );
  ProcessPendingDeletes();
}

/*! \brief Process an action bar command
  This method is called when a menu item created by the tool plugin is selected by the user.
	\param iCommandID Id of the command
*/
void OtmCleanupPlugin::processCommand( int iCommandID )
{
  HWND hwnd = (HWND)UtlQueryULong( QL_TWBFRAME );
  if ( iCommandID == ID_CLEANUP_COMMAND )
  {
    DialogBox( hDll, MAKEINTRESOURCE(ID_CLEANUP_DIALOG), hwnd, CleanupDialogProc );
  }
}


bool OtmCleanupPlugin::stopPlugin( bool fForce  )
{

  // TODO: check for active objects..
  bool fActiveObjects = false;

  // decline stop if we have active objects
  if ( !fForce && fActiveObjects )
  {
    return( false );
  }

  // TODO: terminate active objects, cleanup, free allocated resources

  // de-register plugin
	PluginManager *pPluginManager = PluginManager::getInstance();
	pPluginManager->deregisterPlugin( (OtmPlugin *)this );

  return( true );
}

extern "C" {
__declspec(dllexport)
USHORT registerPlugins()
{
	PluginManager::eRegRc eRc = PluginManager::eSuccess;
	PluginManager *manager = PluginManager::getInstance();
	OtmCleanupPlugin* plugin = new OtmCleanupPlugin();
	eRc = manager->registerPlugin((OtmPlugin*) plugin);
    USHORT usRC = (USHORT) eRc;
    return usRC;
}
}

BOOL CALLBACK CleanupDialogProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  BOOL fResult = FALSE;

  switch ( message )
  {
    case WM_INITDIALOG:
      {

        PCLEANUPDATA pData = new( CLEANUPDATA ); 
        SetWindowLong( hwndDlg, DWL_USER, (LONG)((PVOID)pData) );

        if ( LoadControlFile( hwndDlg, pData ) )
        {
          ScanForFilesToBeDeleted( hwndDlg, pData );
          ShowFilesToBeDeleted( hwndDlg, pData );
          PostMessage( hwndDlg, WM_CLEANUP_SELECTALL, 0, 0 );
          return( TRUE );
        }
        else
        {
          EndDialog( hwndDlg, 0 );
          return( FALSE );
        }
      }
      return( TRUE );
      break;

    case WM_COMMAND:
      switch ( LOWORD( wParam ) )
      {
        case ID_CLEANUP_SELECTALL:
          {
            PCLEANUPDATA pData = (PCLEANUPDATA )GetWindowLong( hwndDlg, DWL_USER );
            SetStateOfAllItems( hwndDlg, pData, TRUE );
          }
          break;

        case ID_CLEANUP_DESELECTALL:
          {
            PCLEANUPDATA pData = (PCLEANUPDATA )GetWindowLong( hwndDlg, DWL_USER );
            SetStateOfAllItems( hwndDlg, pData, FALSE );
          }
          break;

        case ID_CLEANUP_DELETE:
          {
            PCLEANUPDATA pData = (PCLEANUPDATA )GetWindowLong( hwndDlg, DWL_USER );
            DeleteFiles( hwndDlg, pData );
            EndDialog( hwndDlg, 0 );
          } 
          break;

        case ID_CLEANUP_CANCEL:
        case IDCANCEL:
          EndDialog( hwndDlg, 0 );
          break;
      } 
      break;

    case WM_NOTIFY:
      switch (((LPNMHDR)lParam)->code)
      {
        case TVN_KEYDOWN:
          {
            LPNMTVKEYDOWN pTVKeyDown = (LPNMTVKEYDOWN)lParam;

            if ( pTVKeyDown->hdr.idFrom == ID_CLEANUP_TREE )
            {
              if( pTVKeyDown->wVKey == VK_SPACE )
              {
                HWND hwndTree = GetDlgItem( hwndDlg, ID_CLEANUP_TREE );
                HTREEITEM hItem = TreeView_GetSelection( hwndTree );
                if( hItem != NULL )
                {
                  PostMessage( hwndDlg, WM_CLEANUP_GETCHECKSTATE, 0, (LPARAM)hItem );
                }
              }
            }
         }
         return( 0 );
         break;

        case NM_CLICK:
          if (((LPNMHDR)lParam)->idFrom == ID_CLEANUP_TREE )
          {
            PCLEANUPDATA pData = (PCLEANUPDATA )GetWindowLong( hwndDlg, DWL_USER );
            TVHITTESTINFO pt;
            HWND hwndTree = GetDlgItem( hwndDlg, ID_CLEANUP_TREE );
            DWORD pos = GetMessagePos();
            memset( &pt, 0, sizeof(pt) );
            pt.pt.x = LOWORD(pos);
            pt.pt.y = HIWORD(pos);
            ScreenToClient( hwndTree, &(pt.pt) );
            HTREEITEM hItem = TreeView_HitTest( hwndTree, &pt );
            if ( hItem != NULL )
            {
              // do not check the new state of the checkbox here as the checkbox setting in not complete yet
              // instead post a message
              PostMessage( hwndDlg, WM_CLEANUP_GETCHECKSTATE, 0, (LPARAM)hItem );
            }
            return TRUE;
          }
          break; 
      } /* endswitch */
    break;

    case WM_CLEANUP_GETCHECKSTATE:
      {
        TVITEM item;
        HTREEITEM hItem = (HTREEITEM)lParam;
        HWND hwndTree = GetDlgItem( hwndDlg, ID_CLEANUP_TREE );
        PCLEANUPDATA pData = (PCLEANUPDATA )GetWindowLong( hwndDlg, DWL_USER );
        BOOL fChecked = (TreeView_GetCheckState( hwndTree, hItem ) == 1 );
        memset( &item, 0, sizeof(item) );
        item.hItem = hItem;
        item.mask = TVIF_PARAM;
        TreeView_GetItem( hwndTree, &item );
        pData->vFileTodelete[item.lParam].fSelected = fChecked;
        RefreshStatistics( hwndDlg, pData );
      }
      break;

    case WM_CLEANUP_SELECTALL:
      {
        PCLEANUPDATA pData = (PCLEANUPDATA )GetWindowLong( hwndDlg, DWL_USER );
        SetStateOfAllItems( hwndDlg, pData, TRUE );
      }
      break;

    case WM_CLOSE:
      {
        PCLEANUPDATA pData = (PCLEANUPDATA )GetWindowLong( hwndDlg, DWL_USER );
        if ( pData != NULL ) delete( pData );
        SetWindowLong( hwndDlg, DWL_USER, 0 );
      }
      break;


    default:
      break;
  } /* endswitch */

  return fResult;
}


boolean LoadControlFile( HWND hwndDlg, PCLEANUPDATA pData )
{
  FILE *hFile = NULL;

  // build name of cleanup control file
  UtlMakeEQFPath( pData->szControlFileName, NULC, TABLE_PATH, NULL );
  strcat( pData->szControlFileName, "\\" );
  strcat( pData->szControlFileName, CONTROLFILENAME );

  // open file and handle open errors
  hFile = fopen( pData->szControlFileName, "r" );
  if ( hFile == NULL )
  {
    sprintf( pData->szLine, "Failed to load control file \"%s\" which contains the information on the temporary files\nThe error code is %ld", pData->szControlFileName, GetLastError() );
    HWND hwnd = (HWND)UtlQueryULong( QL_TWBFRAME );
    MessageBox( hwnd, pData->szLine, "OtmCleanup Error", MB_CANCEL );
    return( false );
  }

  // load non-comment lines into list of remove patterns
  pData->vRemovePattern.clear();
  memset( pData->szLine, 0, sizeof(pData->szLine) );
  fgets( pData->szLine, sizeof(pData->szLine), hFile );
  while ( !feof(hFile ) )
  {
    // handle current line
    int iLen = strlen( pData->szLine );
    if ( (iLen > 0) && (pData->szLine[iLen-1] == '\n') ) pData->szLine[iLen-1] = '\0';
    char *pszComment = strstr( pData->szLine, "//" );
    if ( pszComment != NULL ) *pszComment = '\0';
    UtlStripBlanks( pData->szLine );
    iLen = strlen( pData->szLine );
    if ( iLen != 0 )
    {
      pData->vRemovePattern.push_back( std::string( pData->szLine ) );
    }

    // get next line
    memset( pData->szLine, 0, sizeof(pData->szLine) );
    fgets( pData->szLine, sizeof(pData->szLine), hFile );
  } /* endwhile */

  fclose( hFile );

  return( true );
}

boolean SetStateOfAllItems( HWND hwndDlg, PCLEANUPDATA pData, BOOL fCheck )
{ 
  TV_INSERTSTRUCT tvIns;
  HWND hwndTree = GetDlgItem( hwndDlg, ID_CLEANUP_TREE );

  memset( &tvIns, 0, sizeof(tvIns) );

  tvIns.item.mask = TVIF_TEXT | TVIF_PARAM;
  tvIns.item.cchTextMax = sizeof(pData->szLine);
  tvIns.hInsertAfter = TVI_ROOT;
  tvIns.hParent = NULL;
  
  // set selection flags in vector
  for( std::vector<FILETODELETE>::iterator it = pData->vFileTodelete.begin(); it != pData->vFileTodelete.end(); it++)
  {
    it->fSelected = fCheck;
  }

  // set check marks in tree view control
  HTREEITEM hItem = TreeView_GetRoot( hwndTree );
  //hItem = TreeView_GetChild( hwndTree, hItem );
  while ( hItem != NULL )
  {
    TreeView_SetCheckState( hwndTree, hItem, fCheck );
    hItem = TreeView_GetNextSibling( hwndTree, hItem );
  }

  RefreshStatistics( hwndDlg, pData );

  return( true );
}

boolean ShowFilesToBeDeleted( HWND hwndDlg, PCLEANUPDATA pData )
{ 
  TV_INSERTSTRUCT tvIns;
  HWND hwndTree = GetDlgItem( hwndDlg, ID_CLEANUP_TREE );

  memset( &tvIns, 0, sizeof(tvIns) );

  tvIns.item.mask = TVIF_TEXT | TVIF_PARAM;
  tvIns.item.cchTextMax = sizeof(pData->szLine);
  tvIns.hInsertAfter = TVI_ROOT;
  tvIns.hParent = NULL;
  
  SendMessage( hwndTree, TVM_DELETEITEM, 0, (LPARAM)TVI_ROOT );
  for( std::vector<FILETODELETE>::iterator it = pData->vFileTodelete.begin(); it != pData->vFileTodelete.end(); it++)
  {
    if ( it->fisDirectory )
    {
      sprintf( pData->szLine, "%s (%ld files, %I64d kB)", it->strFileName.c_str(), it->iFiles, it->iSize );
    }
    else
    {
      sprintf( pData->szLine, "%s (%I64d kB)", it->strFileName.c_str(), it->iSize );
    }
    tvIns.item.pszText = pData->szLine;
    tvIns.item.lParam = it - pData->vFileTodelete.begin();
    SendMessage( hwndTree, TVM_INSERTITEM, 0, (LPARAM)&tvIns );
    tvIns.hInsertAfter = TVI_LAST;
  }
  return( true );
}

boolean ScanForFilesToBeDeleted( HWND hwndDlg, PCLEANUPDATA pData )
{
  for( std::vector<std::string>::iterator it = pData->vRemovePattern.begin(); it != pData->vRemovePattern.end(); it++)
  {
    std::string currentFile = *it;
    ProcessPattern( pData, *it );
  }
  return( true );
}

boolean ProcessPattern( PCLEANUPDATA pData, std::string &strPattern )
{
  std::string strSearchPattern = strPattern;
  boolean fOK = true;

  // get fully qualified search pattern
  fOK = FullyQualifyPattern( strSearchPattern );

   // slit search pattern in directory and name part
  int iNamePos = strSearchPattern.rfind( '\\' );
  if ( iNamePos == std::string::npos ) return( false );
  std::string strDirectory = strSearchPattern.substr( 0, iNamePos );
  std::string strName = strSearchPattern.substr( iNamePos + 1, std::string::npos );

  // scan directory
  ScanDirectory( strDirectory, strName, pData->vFileTodelete );

  return( true );
}

boolean  CountFilesInDirectory( std::string &strDirectory, int *piFiles, _int64 *piSize )
{
  WIN32_FIND_DATA *pFindFileData = (WIN32_FIND_DATA *)malloc( sizeof(WIN32_FIND_DATA) );
  HANDLE hFind;

  if ( pFindFileData == NULL ) return( false );

  std::string strSearchPattern = strDirectory + "\\*.*";
  hFind = FindFirstFile( strSearchPattern.c_str(), pFindFileData );
  if (hFind != INVALID_HANDLE_VALUE) 
  {
    do
    {
      if ( (strcmp( pFindFileData->cFileName, "." ) != 0) && (strcmp( pFindFileData->cFileName, ".." ) != 0))
      {
        if ( pFindFileData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
        {
          std::string strRecurseInto = strDirectory + "\\" + pFindFileData->cFileName;
          CountFilesInDirectory( strRecurseInto, piFiles, piSize );
        }
        else
        {
          *piFiles += 1;
          *piSize += (((_int64)pFindFileData->nFileSizeHigh * ((_int64)MAXDWORD+1)) + (_int64)pFindFileData->nFileSizeLow) / 1024;
        }
      }
    }
    while ( FindNextFile( hFind, pFindFileData ) );
    FindClose(hFind);
  }

  free( (void *)pFindFileData );

  return( true );
}

boolean ScanDirectory( std::string &strDirectory, std::string &strNamePattern, std::vector<FILETODELETE> &vFileTodelete )
{
  WIN32_FIND_DATA FindFileData;
  HANDLE hFind;

  std::string strSearchPattern = strDirectory + "\\" + strNamePattern;
  hFind = FindFirstFile( strSearchPattern.c_str(), &FindFileData);
  if (hFind == INVALID_HANDLE_VALUE) return( false );
  do
  {
    if ( (strcmp( FindFileData.cFileName, "." ) != 0) && (strcmp( FindFileData.cFileName, ".." ) != 0))
    {
      FILETODELETE NewFile;
      NewFile.fSelected = true;
      NewFile.strFileName = strDirectory + "\\" + FindFileData.cFileName;
      if ( FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
      {
        NewFile.iFiles = 0;
        NewFile.iSize = 0;
        NewFile.fisDirectory = true;
        CountFilesInDirectory( NewFile.strFileName, &(NewFile.iFiles), &(NewFile.iSize) );
      }
      else
      {
        NewFile.iFiles = 1;
        NewFile.fisDirectory = false;
        NewFile.iSize = (((_int64)FindFileData.nFileSizeHigh * ((_int64)MAXDWORD+1)) + (_int64)FindFileData.nFileSizeLow) / 1024;
      }
      vFileTodelete.push_back( NewFile );
    }
  }
  while ( FindNextFile( hFind, &FindFileData ) );
  FindClose(hFind);
  return( true );
}

boolean FullyQualifyPattern( std::string &strSearchPattern )
{
  char szSysPath[60];
  UtlMakeEQFPath( szSysPath, '\0', SYSTEM_PATH, NULL );

  if ( strSearchPattern.length() < 2 ) return( false );

  if ( isalpha( strSearchPattern[0] ) &&  isalpha( strSearchPattern[1] == ':') ) 
  {
    // path starts with a drive letter, so it seems to be fully qualified
  }
  else if ( strSearchPattern[0] == '\\' ) 
  {
    // path starts in root so add the missing drive letter only
    szSysPath[2] = '\0';
    strSearchPattern = std::string( szSysPath ) + strSearchPattern;
  }
  else
  {
    // add the complete systempath
    strSearchPattern = std::string( szSysPath ) + "\\" + strSearchPattern;
  }
  return( true );
}

boolean RefreshStatistics( HWND hwndDlg, PCLEANUPDATA pData )
{
  _int64 iTotalSize = 0;
  int iTotalFiles = 0;
  for( std::vector<FILETODELETE>::iterator it = pData->vFileTodelete.begin(); it != pData->vFileTodelete.end(); it++)
  {
    if ( it->fSelected )
    {
      iTotalSize += it->iSize;
      iTotalFiles += it->iFiles;
    }
  }

  sprintf( pData->szLine, "%I64d kB", iTotalSize );
  SetDlgItemText( hwndDlg, ID_CLEANUP_SIZE, pData->szLine );

  sprintf( pData->szLine, "%ld", iTotalFiles );
  SetDlgItemText( hwndDlg, ID_CLEANUP_FILES, pData->szLine );

  return( true );
}

boolean DeleteFiles( HWND hwndDlg, PCLEANUPDATA pData )
{
  int iDeletedFiles = 0;
  int iFailedDeletes = 0;
  FILE *hfPendingDeletes = NULL;

  // setup name of file containing list of pending deletes
  UtlMakeEQFPath( pData->szBuffer, '\0', TABLE_PATH, NULL );
  strcat( pData->szBuffer, "\\" );
  strcat( pData->szBuffer, PENDINGDELETESFILE );

  // delete the files
  for( std::vector<FILETODELETE>::iterator it = pData->vFileTodelete.begin(); it != pData->vFileTodelete.end(); it++)
  {
    if ( it->fSelected )
    {
      if ( it->fisDirectory )
      {
        UtlRemoveDir( (PSZ)it->strFileName.c_str(), FALSE );
        iDeletedFiles += it->iFiles;
      }
      else if ( DeleteFile( it->strFileName.c_str() ) )
      {
        iDeletedFiles++;
      }
      else
      {
        iFailedDeletes++;

        // add file to list of pending deletes
        if ( hfPendingDeletes == NULL )
        {
          hfPendingDeletes = fopen( pData->szBuffer, "w" );
        }
        if ( hfPendingDeletes != NULL )
        {
          fprintf( hfPendingDeletes, "%s\n", it->strFileName.c_str() );
        }
      }
    }
  }

  if ( iFailedDeletes != 0 )
  {
    sprintf( pData->szLine, "Successfully deleted %ld files. %ld files could not be deleted as the files are currently in use. These files will be deleted automatically when OpenTM2 is restarted.", iDeletedFiles, iFailedDeletes );
    MessageBox( hwndDlg, pData->szLine, "OtmCleanup Info", MB_CANCEL );
  }
  else
  {
    sprintf( pData->szLine, "Successfully deleted %ld files.", iDeletedFiles );
    MessageBox( hwndDlg, pData->szLine, "OtmCleanup Info", MB_CANCEL );
  }

  if ( hfPendingDeletes != NULL )
  {
    fclose( hfPendingDeletes );
  }

  return( true );
}

// process any pending deletes which could not be performed as the files were in use
boolean ProcessPendingDeletes()
{
  char *pszBuffer = (char *)malloc( 2048 );
  if ( pszBuffer == NULL ) return( false );

  // setup name of file containing list of pending deletes
  UtlMakeEQFPath( pszBuffer, '\0', TABLE_PATH, NULL );
  strcat( pszBuffer, "\\" );
  strcat( pszBuffer, PENDINGDELETESFILE );

  // try to open the file
  FILE *hfDeletes = fopen( pszBuffer, "r" );
  if ( hfDeletes == NULL )
  {
    free( pszBuffer );
    return( false );
  }

  // process any delete commands
  fgets( pszBuffer, 2048, hfDeletes );
  while( !feof( hfDeletes ) )
  {
    DeleteFile( pszBuffer );
    fgets( pszBuffer, 2048, hfDeletes );
  }
  fclose( hfDeletes );

  // delete the file containing the list of pending deletes
  UtlMakeEQFPath( pszBuffer, '\0', TABLE_PATH, NULL );
  strcat( pszBuffer, "\\" );
  strcat( pszBuffer, PENDINGDELETESFILE );
  DeleteFile( pszBuffer );

  free( pszBuffer );

  return( true );
}