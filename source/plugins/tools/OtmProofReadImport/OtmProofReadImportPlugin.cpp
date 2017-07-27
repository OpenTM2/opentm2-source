/*! \file
	Copyright Notice:

	Copyright (C) 1990-2017, International Business Machines
	Corporation and others. All rights reserved
*/

#define INCL_EQF_TP               // public translation processor functions
#define INCL_EQF_TAGTABLE         // tagtable defines and functions#include "eqf.h"
#include "eqfserno.h"
#include "string"
#include "vector"
#include "windows.h"
#include "core\PluginManager\PluginManager.h"
#include "OtmProofReadImportPlugin.h"
#include "OtmProofReadImportPlugin.id"
#include "core\memory\MemoryFactory.h"
#include "OtmProofReadWindow.h"

// the static plugin infos
static char *pszPluginName = "OtmProofReadImport";
static char *pszShortDescription = "Import validation documents";
static char *pszLongDescription	= "This is a plugin to import validation documents";
static char *pszVersion = STR_DRIVER_LEVEL_NUMBER;
static char *pszSupplier = "International Business Machines Corporation";

UINT_PTR CALLBACK OpenFileHook( HWND hdlg, UINT uiMsg, WPARAM wParam, LPARAM lParam );


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

OtmProofReadImportPlugin::OtmProofReadImportPlugin()
{
	  name = pszPluginName;
	  shortDesc = pszShortDescription;
	  longDesc = pszLongDescription;
	  version = pszVersion;
	  supplier = pszSupplier;
    pluginType  = OtmPlugin::eToolType;
    usableState = OtmPlugin::eUsable;
}


OtmProofReadImportPlugin::~OtmProofReadImportPlugin()
{
}

const char* OtmProofReadImportPlugin::getName()
{
	return name.c_str();
}

const char* OtmProofReadImportPlugin::getShortDescription()
{
	return shortDesc.c_str();
}

const char* OtmProofReadImportPlugin::getLongDescription()
{
	return longDesc.c_str();
}

const char* OtmProofReadImportPlugin::getVersion()
{
	return version.c_str();
}

const char* OtmProofReadImportPlugin::getSupplier()
{
	return supplier.c_str();
}

/*! \brief Initialize plugin 
  This method is called when the workbench window has been created.
  The tool plugin can use the plugin manager methods addMenuItem and addSubMenu to add
  own entries to the workbench toolbar.
*/
void OtmProofReadImportPlugin::init()
{
	PluginManager *manager = PluginManager::getInstance();
  manager->addMenuItem( this, "Utilities", "Import validation documents...", ID_PROOFREADIMPORT_COMMAND );

  // get list of available filters
  fillFilterList( vFilterList );
}

/*! \brief Process an action bar command
  This method is called when a menu item created by the tool plugin is selected by the user.
	\param iCommandID Id of the command
*/
void OtmProofReadImportPlugin::processCommand( int iCommandID )
{
  if ( iCommandID == ID_PROOFREADIMPORT_COMMAND )
  {
    BOOL fOK = TRUE;
    std::string strFilterDLL, strInputFile, strProofReadXML;
    std::vector<std::string> vInputFiles;
    std::vector<std::string> *pvXMLFiles = new std::vector<std::string>;
    HWND hwndParent = (HWND)UtlQueryULong( QL_TWBFRAME );

    fOK = getLastUsedValues( strInputFile, strFilterDLL );

    if ( fOK ) fOK = getImportFiles( strInputFile, vInputFiles, strFilterDLL, vFilterList, hwndParent );

    if ( fOK ) saveLastUsedValues( strInputFile, strFilterDLL );

    if ( fOK ) fOK = prepareXmlFiles( vInputFiles, strFilterDLL, pvXMLFiles );

    // show the XML file in the proof read window
    if ( fOK )
    {
      fOK = showProofReadWindow( pvXMLFiles );
    }
    else
    {
      if ( pvXMLFiles ) delete pvXMLFiles;
    } /* endif */
  } /* endif */
}


bool OtmProofReadImportPlugin::stopPlugin( bool fForce  )
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

/*! \brief Fill the list of available filters
*/
void OtmProofReadImportPlugin::fillFilterList( std::vector<FILTERENTRY> &vFilterList )
{
  // setup filter search path
  char szFilterPath[MAX_PATH];
  UtlMakeEQFPath( szFilterPath, NULC, PLUGIN_PATH, NULL );
  strcat( szFilterPath, "\\" );
  strcat( szFilterPath, szFilterDir );
  int iPathLen = strlen( szFilterPath );
  strcat( szFilterPath, "\\*.DLL" );

  // loop over all filter DLLs 
  HANDLE hDir;
  WIN32_FIND_DATA FindData;

  hDir = FindFirstFile( szFilterPath, &FindData );
  if ( hDir != INVALID_HANDLE_VALUE )
  {
    do
    {
      if ( !(FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) )
      {
        sprintf( szFilterPath + iPathLen, "\\%s", FindData.cFileName );

        // load the DLL
        SetErrorMode(  SEM_FAILCRITICALERRORS | SEM_NOOPENFILEERRORBOX  );
      	HMODULE hMod = LoadLibrary( szFilterPath );
        SetErrorMode(0);

        // get the getinfo entry point and use it to get the the filter name and the file extension
	      if (hMod != 0)
	      {
      		GETINFOFUNC pInfoFunc = (GETINFOFUNC)GetProcAddress( hMod, GETNFOFUNC_NAME );
		      if ( pInfoFunc != 0 )
		      {
            FILTERENTRY Entry;
            memset( &Entry, 0, sizeof(Entry) );
            strcpy( Entry.szDLLName, FindData.cFileName );
            strcpy( Entry.szPathName, szFilterPath );
            if ( pInfoFunc( Entry.szDisplayName, sizeof(Entry.szDisplayName), Entry.szFileExtension, sizeof(Entry.szFileExtension), NULL, 0 ) == 0 )
            {
              vFilterList.push_back( Entry );
            } /* endif */
          } /* endif */
          FreeLibrary( hMod );
        } /* endif */
      } /* endif */
    } while ( FindNextFile( hDir, &FindData  ) );
    FindClose( hDir );
  }
} /* end of OtmProofReadImportPlugin::fillFilterList */

/*! \brief Get the names of the input files using the standard Windows open file dialog
	\param strLastImportDir a std::string containing the last used import directory
	\param vInputFiles a vector of std::string receiving the selected input files
	\param strFilterDLL this string will contain the name of the LU filter DLL (w/o path), on return it is filled with the name of the selected filter DLL
	\returns TRUE if the user selected successfully a input file
*/
BOOL OtmProofReadImportPlugin::getImportFiles( std::string &strLastImportDir, std::vector<std::string> &vInputFiles, std::string &strFilterDLL, std::vector<FILTERENTRY> &vFilterList, HWND hwndParent )
{
  PSZ pszFileBuffer = NULL;                 // buffer for returned file names
  PSZ pszInitialDir = new char[MAX_PATH+1]; // buffer for initial directory
  PSZ pszFilterList = NULL;                 // buffer for list of available filters 
  int iFilterIndex = 0;                     // index of currently selected filter
  BOOL fOK = TRUE;                          // return code

  // build filter name and file extension list
  {
    // compute required buffer size
    // for each filter an entry is created in this format:  "filtername (*extension)0x00*extension0x00"
    // the required length for one entry is length(filtername) + 2 * length(extension) + 7
    int iLen = 1; // we need at least one character for the terminating zero character

    for( size_t i = 0; i < vFilterList.size(); i++ )
    {
      iLen += strlen( vFilterList[i].szDisplayName ) + (2 * strlen( vFilterList[i].szFileExtension ) ) + 7;
    }

    // allocate buffer
    pszFilterList = new char[iLen];

    // fill-in filter names and filter extensions
    PSZ pszCurEntry = pszFilterList;
    for( size_t i = 0; i < vFilterList.size(); i++ )
    {
      sprintf( pszCurEntry, "%s (*%s)", vFilterList[i].szDisplayName, vFilterList[i].szFileExtension );
      pszCurEntry += strlen(pszCurEntry) + 1;
      sprintf( pszCurEntry, "*%s", vFilterList[i].szFileExtension );
      pszCurEntry += strlen(pszCurEntry) + 1;

      // check if filter is the currently selected one
      if ( strFilterDLL.compare( vFilterList[i].szDLLName ) == 0 )
      {
        iFilterIndex = i;
      }
    }
    *pszCurEntry = 0;
  }

  // fill open file structure
  int iBufSize = MAX_PATH * 40;
  pszFileBuffer = new char[iBufSize];
  memset( pszFileBuffer, 0, iBufSize );
  OPENFILENAME OpenStruct;
  memset( &OpenStruct, 0, sizeof(OpenStruct) );
  OpenStruct.lStructSize = sizeof(OpenStruct);
  OpenStruct.lpstrFilter = pszFilterList;
  OpenStruct.lpstrCustomFilter = NULL;
  OpenStruct.hwndOwner = hwndParent;
  OpenStruct.lpstrFilter = pszFilterList;
  OpenStruct.nFilterIndex = iFilterIndex + 1; // the filter index is not 0 based...
  OpenStruct.lpstrCustomFilter = NULL;
  strcpy( pszInitialDir, strLastImportDir.c_str() );
  OpenStruct.lpstrInitialDir = pszInitialDir;
  OpenStruct.lpstrFile = pszFileBuffer;
  OpenStruct.nMaxFile = iBufSize;
  OpenStruct.lpstrFileTitle = NULL;
  OpenStruct.lpfnHook = OpenFileHook;
  OpenStruct.nMaxFileTitle = 0;
  OpenStruct.lpstrTitle = "Import Validation Document";
  OpenStruct.Flags = OFN_FILEMUSTEXIST | OFN_ENABLEHOOK | OFN_LONGNAMES | OFN_EXPLORER | OFN_NOCHANGEDIR | OFN_PATHMUSTEXIST | OFN_ALLOWMULTISELECT;

  if ( GetOpenFileName( &OpenStruct ) )
  {
    vInputFiles.clear();
    strFilterDLL = vFilterList[OpenStruct.nFilterIndex-1].szDLLName;

    // get all selected files
    strcpy( pszInitialDir, pszFileBuffer );
    if ( OpenStruct.nFileExtension != 0 )
    {
      // only a single file is selected, so pszFileBuffer contains the fully qualified file name
      UtlSplitFnameFromPath( pszInitialDir );
    }
    strLastImportDir = pszInitialDir;
    PSZ pszCurName = pszFileBuffer + OpenStruct.nFileOffset;
    std::string strFile;
    while ( *pszCurName != 0 )
    {
      strcat( pszInitialDir, "\\" );
      strcat( pszInitialDir, pszCurName );
      strFile = pszInitialDir;
      vInputFiles.push_back( strFile );
      UtlSplitFnameFromPath( pszInitialDir );
      pszCurName = pszCurName + strlen(pszCurName) + 1;
    } /* endwhile */
    fOK = TRUE;
  }
  else
  {
    int iRC = CommDlgExtendedError();
    fOK = FALSE;
  }

  // cleanup
  if ( pszFileBuffer ) delete pszFileBuffer; 
  if ( pszInitialDir ) delete pszInitialDir;
  if ( pszFilterList ) delete pszFilterList;

  return( fOK );
}



/*! \brief make file name for last used values profile
	\param pszLastUsedValuesFile buffer for the file name
	\returns TRUE if the function completed successfully
*/
BOOL OtmProofReadImportPlugin::getLastUsedValuesProfileName( char *pszLastUsedValuesFile  )
{
  UtlMakeEQFPath( pszLastUsedValuesFile, NULC, PROPERTY_PATH, NULL );
  strcat( pszLastUsedValuesFile, "\\OtmProofReadImportPlugin.LastUsed" );
  return( TRUE );
}

/*! \brief Get the last used values 
	\param strInputFile this string will receive the fully qualified last used input file name 
	\param strFilterDLL this string will receive the last used filter DLL name  (w/o path)
	\returns TRUE if the function completed successfully
*/
BOOL OtmProofReadImportPlugin::getLastUsedValues( std::string &strInputFile, std::string &strFilterDLL )
{
  char szLastUsedValues[MAX_PATH];
  char szValue[MAX_PATH];
  
  getLastUsedValuesProfileName( szLastUsedValues );
  GetPrivateProfileString( "LastUsed", "ImportFile", "", szValue, sizeof(szValue), szLastUsedValues );
  strInputFile = szValue;
  GetPrivateProfileString( "LastUsed", "Filter", "", szValue, sizeof(szValue), szLastUsedValues );
  strFilterDLL = szValue;
  return( TRUE );
}

/*! \brief Save the last used values 
	\param strInputFile string with the last used file name
	\param strFilterDLL string with the last used filter DLL
	\returns TRUE if the function completed successfully
*/
BOOL OtmProofReadImportPlugin::saveLastUsedValues( std::string &strInputFile, std::string &strFilterDLL )
{
  char szLastUsedValues[MAX_PATH];
  
  getLastUsedValuesProfileName( szLastUsedValues );
  WritePrivateProfileString( "LastUsed", "ImportFile", strInputFile.c_str(), szLastUsedValues );
  WritePrivateProfileString( "LastUsed", "Filter", strFilterDLL.c_str(), szLastUsedValues );
  return( TRUE );
}


/*! \brief Call a filter DLL to convert the input file into our proofread import XML format
	\param strInputFile the input file being converted
	\param strFilterDLL the filter DLL selected by the user
	\param strProofReadXML the fully qualified name of the proofread import XML file to be created by the filter
	\returns TRUE if the filter completed successfully
*/
BOOL OtmProofReadImportPlugin::applyFilter( std::string strInputFile, std::string strFilterDLL, std::string strProofReadXML, std::vector<FILTERENTRY> &vFilterList )
{
  BOOL fOK = TRUE;

  // find filter entry in vFilterList
  size_t iFilterIndex = 0;
  while ( iFilterIndex < vFilterList.size() )
  {
    if ( strFilterDLL.compare( vFilterList[iFilterIndex].szDLLName ) == 0 )
    {
      break;
    }
    iFilterIndex++;
  } /* endwhile */
  if ( iFilterIndex >= vFilterList.size() ) return( FALSE );


  // load filter DLL
  SetErrorMode(  SEM_FAILCRITICALERRORS | SEM_NOOPENFILEERRORBOX  );
  HMODULE hMod = LoadLibrary( vFilterList[iFilterIndex].szPathName );
  SetErrorMode(0);
  if ( hMod == NULL ) return ( FALSE );

  // get the convert entry point and use it to get the the filter name and the file extension
  CONVERTFUNC pConvertFunc = (CONVERTFUNC)GetProcAddress( hMod, CONVERTFUNC_NAME );

  // let the filter DLL do the conversion
  if ( pConvertFunc != NULL )
  {
    fOK = (pConvertFunc( strInputFile.c_str(), strProofReadXML.c_str() ) == 0); 
  }
  else
  {
    fOK = FALSE;
  }

  return( fOK );
}

/*! \brief Prepare all input files and show a wait window
	\param vInputFiles  the input files being converted
	\param strFilterDLL the name of the filter DLL
	\param strProofReadXML the XML import files after applying the filter
	\returns TRUE if the function completed successfully
*/
BOOL OtmProofReadImportPlugin::prepareXmlFiles( std::vector<std::string> &vInputFiles, std::string &strFilterDLL, std::vector<std::string> *pvXMLFiles )
{
  BOOL fOK = TRUE;
  std::string strProofReadXML;
  HWND hwndWaitWindow = NULL;

  // create the wait window
  {
     hwndWaitWindow = CreateWindow( "STATIC", "\n\nPreparing imported validation documents\nplease wait...", WS_BORDER | WS_CHILD | SS_CENTER | WS_VISIBLE, 
        200, 300, 380, 100, (HWND)UtlQueryULong( QL_TWBCLIENT ), NULL, NULL, NULL );

    if ( hwndWaitWindow )
    {
      ShowWindow( hwndWaitWindow, SW_SHOWNORMAL );
      UpdateWindow( hwndWaitWindow );
    } /* endif */
  }

  // convert the input files into our XML format
  for( size_t i = 0; fOK && (i < vInputFiles.size()); i++ )
  {
    strProofReadXML = vInputFiles[i];
    strProofReadXML.append( ".tempxml" );
    if ( fOK ) fOK = applyFilter( vInputFiles[i], strFilterDLL, strProofReadXML, vFilterList );
    if ( fOK )
    {
      pvXMLFiles->push_back( strProofReadXML );
    } /* endif */
  } /* endfor */

  // remove the wait window
  if ( hwndWaitWindow )
  { 
    DestroyWindow( hwndWaitWindow );
  } /* endif */
  return( fOK );
}



/*! \brief Show the proofreading process window
	\param strProofReadXML the fully qualified name of the proofread import XML file
	\returns TRUE if the function completed successfully
*/
BOOL OtmProofReadImportPlugin::showProofReadWindow( std::vector<std::string> *pvProofReadXML )
{
  BOOL fOK = TRUE;

  fOK = ProofReadWindow( pvProofReadXML, hDll );

  return( fOK );
}


extern "C" {
__declspec(dllexport)
USHORT registerPlugins()
{
	PluginManager::eRegRc eRc = PluginManager::eSuccess;
	PluginManager *manager = PluginManager::getInstance();
	OtmProofReadImportPlugin* plugin = new OtmProofReadImportPlugin();
	eRc = manager->registerPlugin((OtmPlugin*) plugin);
    USHORT usRC = (USHORT) eRc;
    return usRC;
}
}


extern "C" {
  __declspec(dllexport)
  unsigned short getPluginInfo( POTMPLUGININFO pPluginInfo )
  {
    strcpy( pPluginInfo->szName, pszPluginName );
    strcpy( pPluginInfo->szShortDescription, pszShortDescription );
    strcpy( pPluginInfo->szLongDescription, pszLongDescription );
    strcpy( pPluginInfo->szVersion, pszVersion );
    strcpy( pPluginInfo->szSupplier, pszSupplier );
    pPluginInfo->eType = OtmPlugin::eToolType;
    strcpy( pPluginInfo->szDependencies, "" );
    pPluginInfo->iMinOpenTM2Version= -1;
    return( 0 );
  }
}

//
// hook procedure for standard file open dialog
//
UINT_PTR CALLBACK OpenFileHook( HWND hdlg, UINT uiMsg, WPARAM wParam, LPARAM lParam )
{
  UINT uiResult = 0;

  wParam; 

  switch ( uiMsg )
  {
    case WM_INITDIALOG:
      {
        BOOL fOK = TRUE;
        RECT rctFormatStatic, rctReadOnlyCheck, rctFormatCombo;
        HWND hwndDialog = NULLHANDLE;
        HWND hwndFormatStatic = NULLHANDLE;
        HWND hwndReadOnlyCheck = NULLHANDLE;
        HWND hwndFormatCombo = NULLHANDLE;

        // intialize RECT structures
        memset( &rctReadOnlyCheck, 0, sizeof(rctReadOnlyCheck) );
        memset( &rctFormatStatic, 0, sizeof(rctFormatStatic) );
        memset( &rctFormatCombo, 0, sizeof(rctFormatCombo) );

        // get dialog handle
        hwndDialog = GetParent( hdlg );
        fOK = (hwndDialog != NULLHANDLE);

        // change to English labelled controls
        SetDlgItemText( hwndDialog, 1091, "Look &in:" );
        SetDlgItemText( hwndDialog, 1090, "File &name:" );
        SetDlgItemText( hwndDialog, 1089, "&Format:" );
        SetDlgItemText( hwndDialog, IDCANCEL, "Cancel" );
        SetDlgItemText( hwndDialog, IDOK, "&Import" );

        // get handle of dialog controls
        if ( fOK )
        {
          hwndReadOnlyCheck = GetDlgItem( hwndDialog, 1040 );
          hwndFormatStatic  = GetDlgItem( hwndDialog, 1089 );
          hwndFormatCombo   = GetDlgItem( hwndDialog, 1136 );
          fOK = (hwndReadOnlyCheck != NULLHANDLE) && 
                (hwndFormatStatic != NULLHANDLE) && 
                (hwndFormatCombo != NULLHANDLE) ; 
        } /* endif */

        // get size and position of dialog controls and map to window coordinates
        if ( fOK )
        {
          GetWindowRect( hwndReadOnlyCheck, &rctReadOnlyCheck );
          MapWindowPoints( HWND_DESKTOP, hwndDialog, (LPPOINT)&rctReadOnlyCheck, 2 );
          GetWindowRect( hwndFormatStatic,  &rctFormatStatic );
          MapWindowPoints( HWND_DESKTOP, hwndDialog, (LPPOINT)&rctFormatStatic, 2 );
          GetWindowRect( hwndFormatCombo,   &rctFormatCombo );
          MapWindowPoints( HWND_DESKTOP, hwndDialog, (LPPOINT)&rctFormatCombo, 2 );
        } /* endif */

        // hide read-only checkbox
        ShowWindow( hwndReadOnlyCheck, SW_HIDE );
      }
      break;

    default:
        break;
  } /*endswitch */
  return( uiResult );
} /* end of function OpenFileHook */
