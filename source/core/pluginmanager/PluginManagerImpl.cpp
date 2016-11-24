/*! \file
	Copyright Notice:

	Copyright (C) 1990-2016, International Business Machines
	Corporation and others. All rights reserved
*/

#include <set>
#include <algorithm> 
#include <string>
#include <Windows.h>
#include "PluginManager.h"
#include "PluginManagerImpl.h"
#include "OtmPlugin.h"

using namespace std;

#define PLUGIN_PATH_MAX_DEPTH          2

#ifndef CPPTEST
extern "C"
{
#endif
	#include "eqf.h"
#ifndef CPPTEST
}
#endif

/*! \typedef PLUGINPROC
	Prototype for the registerPlugins() function
*/
typedef unsigned short (__cdecl *PLUGINPROC) ();   // Modify for P402974

// Add for P403115 start
BOOL DllNameCheck(const char * strName);
// Add end

PluginManagerImpl::PluginManagerImpl()
{
	pluginSet = new PLUGINSET;
	bRegisterAllowed = false;
  iCurrentlyLoadedPluginDLL = -1;
  m_iNextCommandID = ID_TWBM_AAB_TOOLPLUGINS;
}

PluginManagerImpl::~PluginManagerImpl()
{
	delete pluginSet;
}

PluginManager::eRegRc PluginManagerImpl::registerPlugin(OtmPlugin* plugin)
{
	PluginManager::eRegRc eRc = PluginManager::eSuccess;
	if (bRegisterAllowed)
	{
		if (plugin != 0)
		{
			std::string name(plugin->getName());
			if (!name.empty())
      {
        this->Log.writef( "   Registering plugin %s", name.c_str() );

        // insert plugin into list of active plugins
				PLUGINSET::iterator it;
				for (it = pluginSet->begin(); it != pluginSet->end() && eRc == PluginManager::eSuccess; it++)
				{
					std::string tmpName((*it)->getName());
					if (tmpName == name)
					{
						eRc = PluginManager::eAlreadyRegistered;
                        this->Log.writef( "Error:   The plugin %s is already registered.", name.c_str() ); // Add for 403115
					}
				}

        // insert plugin into plugin list of currently loaded plugin DLL
				if (eRc == PluginManager::eSuccess)
				{
					pluginSet->insert(plugin);

          // update list of plugins registered for active plugin DLL
          if ( iCurrentlyLoadedPluginDLL != -1 )
          {
            vLoadedPluginDLLs[iCurrentlyLoadedPluginDLL].vPluginList.push_back(plugin);
          } /* end */             
				}

        // notify all plugin listeners
				if (eRc == PluginManager::eSuccess)
				{
          NotifyListeners( PluginListener::eStarted, plugin->getName(), plugin->getType(), false );
				}
			}
			else
			{
				eRc = PluginManager::eInvalidName;
			}
		}
		else
		{
			eRc = PluginManager::eInvalidParm;
		}
	}
	else
	{
		eRc = PluginManager::eInvalidRequest;
	}

    this->Log.writef( "   Registering plugin %d", eRc );

	return eRc;
}

PluginManager::eRegRc PluginManagerImpl::deregisterPlugin(OtmPlugin* plugin)
{
	PluginManager::eRegRc eRc = PluginManager::eSuccess;

	if (plugin != 0)
	{
    // remove plugin from our list of plugins
    for ( PLUGINSET::iterator it = pluginSet->begin(); it != pluginSet->end(); it++)
		{
			if ((*it) == plugin)
			{
        pluginSet->erase( it );
        break; 
			}
		}

    // remove plugin from plugin list of loadded plugin DLLs
    int iPluginEntry = findPluginEntry( plugin );
    if ( iPluginEntry != -1 )
    {
      for ( std::vector<OtmPlugin *>::iterator it = vLoadedPluginDLLs[iPluginEntry].vPluginList.begin(); it != vLoadedPluginDLLs[iPluginEntry].vPluginList.end(); it++)
		  {
			  if ((*it) == plugin)
			  {
          vLoadedPluginDLLs[iPluginEntry].vPluginList.erase( it );
          break; 
			  }
		  }
    }
	}
	else
	{
		eRc = PluginManager::eInvalidParm;
	}
	return eRc;
}

int PluginManagerImpl::getPluginCount()
{
	return pluginSet->size();
}

OtmPlugin* PluginManagerImpl::getPlugin(const char* name)
{
	OtmPlugin* plugin = 0;
	if (name && *name != '\0')
	{
		PLUGINSET::iterator it;
		for (it = pluginSet->begin(); it != pluginSet->end() && plugin == 0; it++)
		{
			if ( strcmp( (*it)->getName(), name ) == 0 )
			{
				plugin = *it;
			}
		}
	}
	return plugin;
}

OtmPlugin* PluginManagerImpl::getPlugin(int iIndex)
{
	OtmPlugin* plugin = 0;
	if (iIndex < getPluginCount())
	{
		PLUGINSET::iterator it;
		for (it = pluginSet->begin(); iIndex > 0; iIndex--, it++);
		plugin = *it;
	}
	return plugin;
}

OtmPlugin* PluginManagerImpl::getPlugin(OtmPlugin::ePluginType type, int n)
{
	OtmPlugin* plugin = 0;
	if (type != OtmPlugin::eUndefinedType)
	{
		PLUGINSET::iterator it;
		for (it = pluginSet->begin(); it != pluginSet->end() && plugin == 0; it++)
		{
    	OtmPlugin* pTest = *it;
			if (pTest->getType() == type)
			{
				n--;
				if (n <= 0)
				{
					plugin = pTest;
				}
			}
		}
	}
	return plugin;
}

USHORT PluginManagerImpl::loadPluginDlls(const char* pszPluginDir)
{
    USHORT usRC = PluginManager::eSuccess;         // function return code add for P402974
	std::string strFileSpec(pszPluginDir);
	BOOL fMoreFiles = TRUE;
	HANDLE hDir = HDIR_CREATE;
	WIN32_FIND_DATA ffb;

  BOOL fLogHasBeenOpened = FALSE;

#ifdef _DEBUG
  if ( !this->Log.isOpen() )
  {
    this->Log.open( "PluginManager" );
      fLogHasBeenOpened = TRUE;
  } /* end */     
  this->Log.writef( "Loading plugin DLLs from directory %s...", pszPluginDir );
#endif

  // check the depths of the cycle
  if (IsDepthOvered( pszPluginDir ))
  {
      this->Log.writef( "...skipping directory, max. nesting level reached" );
      if ( fLogHasBeenOpened ) this->Log.close();
      return usRC;
  }

	// allow calling the registerPlugin()-method
	bRegisterAllowed = true;

	strFileSpec += "\\*.dll";
  this->Log.writef( "   running FindFirst for %s...", strFileSpec.c_str() );

	hDir = FindFirstFile( strFileSpec.c_str(), &ffb );
  
  if ( hDir != INVALID_HANDLE_VALUE )
  {
    fMoreFiles = TRUE;
	  while ( fMoreFiles )
	  {
		  std::string strDll(pszPluginDir);
		  strDll += '\\';
		  strDll += ffb.cFileName;
          // Modify for P403115 start
          if (DllNameCheck(strDll.c_str()))
          {
              this->Log.writef( "   found DLL %s", strDll.c_str() );
          }
          else
          {
              this->Log.writef("Error:   DLL %s is not valid, skip", strDll.c_str());
              fMoreFiles= FindNextFile(hDir, &ffb);
              continue;
          }
          // Modify end
		  USHORT usSubRC = loadPluginDll(strDll.c_str());  // add return value for P402974
          // Add for P402974 start
          if (PluginManager::ePluginExpired == usSubRC)
          {
              // expired is the highest authority
              this->Log.writef("Error:   DLL %s is expired.", strDll.c_str());
              usRC = usSubRC;
          }
          else if (usSubRC && (PluginManager::ePluginExpired != usRC) && (PluginManager::eAlreadyRegistered != usSubRC))
          {
              // else is other error and skip already registered error
              usRC = usSubRC;
          }
          // Add end
		  fMoreFiles= FindNextFile( hDir, &ffb );
	  } /* endwhile */
    FindClose( hDir );
  }
  else
  {
    int iRC = GetLastError();
    this->Log.writef( "   FindFirstFile failed with rc=%ld", iRC );
  } /* endif */     

	// now search all subdirectories
	strFileSpec = pszPluginDir;
	strFileSpec += "\\*";
	hDir = FindFirstFile( strFileSpec.c_str(), &ffb );
  if ( hDir != INVALID_HANDLE_VALUE )
  {
    fMoreFiles = TRUE;

	  while ( fMoreFiles )
	  {
      if ( (ffb.cFileName[0] != '.') && (ffb.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) )
		  {
			  std::string strSubdir(pszPluginDir);
			  strSubdir += '\\';
			  strSubdir += ffb.cFileName;
			  USHORT usSubRC = loadPluginDlls( strSubdir.c_str() ); // Modify for P402974
              // Add for P402974 start
              if (PluginManager::ePluginExpired == usSubRC)
              {
                  // expired is the highest authority
                  this->Log.writef("Error:   DLL %s is expired.", strSubdir.c_str());
                  usRC = usSubRC;
              }
              else if (usSubRC && (PluginManager::ePluginExpired != usRC) && (PluginManager::eAlreadyRegistered != usSubRC))
              {
                  // else is other error and skip already registered error
                  usRC = usSubRC;
              }
              // Add end
		  }
		  fMoreFiles= FindNextFile( hDir, &ffb );
	  }
    FindClose( hDir );
  } /* endif */     
  this->Log.write( "   ...Done" );

	// disallow calling the registerPlugin()-method
	bRegisterAllowed = false;
	
  if ( fLogHasBeenOpened ) this->Log.close();

  return usRC;     // Add for P402974
}

USHORT PluginManagerImpl::loadPluginDll(const char* pszName)
{
    USHORT usRC = PluginManager::eSuccess;         // function return code add for P402974

    SetErrorMode(  SEM_FAILCRITICALERRORS | SEM_NOOPENFILEERRORBOX  );
    // Add for P402792 start
    if (!vLoadedPluginDLLs.empty())
    {
        for (size_t iInx = 0; iInx < vLoadedPluginDLLs.size(); iInx++)
        {
            if (!stricmp(vLoadedPluginDLLs[iInx].strDll, pszName))
            {
                return usRC;
            }
        }
    }
    // Add end
	HMODULE hMod = LoadLibrary(pszName);
    SetErrorMode(0);
    this->Log.writef( "   Loading plugin %s", pszName );
	if (hMod != 0)
	{
    //this->Log.writef( "   Plugin %s successfully loaded", pszName );
		PLUGINPROC pFunc = (PLUGINPROC) GetProcAddress(hMod, "registerPlugins");
		if (pFunc != 0)
		{
      //this->Log.write( "   Address of function \"registerPlugins()\" successfully resolved" );

      // prepare our loaded DLL entry
      vLoadedPluginDLLs.push_back( LoadedPluginDLL() );
      iCurrentlyLoadedPluginDLL = vLoadedPluginDLLs.size() - 1;
      vLoadedPluginDLLs[iCurrentlyLoadedPluginDLL].hMod = hMod;
      // Add for P402792 start
      memset(vLoadedPluginDLLs[iCurrentlyLoadedPluginDLL].strDll, 0x00, sizeof(vLoadedPluginDLLs[iCurrentlyLoadedPluginDLL].strDll));
      strcpy(vLoadedPluginDLLs[iCurrentlyLoadedPluginDLL].strDll, pszName);
      // Add end

      // call plugin registration entry point
      usRC = pFunc();     // Add return value for P402974
      // Add for P403115 start
      if (usRC)
      {
          // if register dll error, just remove the dll from the group
          this->Log.writef( "Error: register plugin %s failed %d.", pszName, usRC);
         
		  // FOR P403268 beigin
		  // if it's already in pluginSet, also erase it
		  // vLoadedPluginDLLs.pop_back();
		  LoadedPluginDLL lpdll = vLoadedPluginDLLs.back();
		  OtmPlugin *pToDel = lpdll.vPluginList.back();
		  if(pToDel != NULL)
			  pluginSet->erase(pToDel);
		  vLoadedPluginDLLs.pop_back();
		  // FOR P403268 end

          FreeLibrary(hMod);
      }
      // Add end

      // reset active plugin DLL entry
      iCurrentlyLoadedPluginDLL = -1;
		}
		else
		{
      this->Log.write( "   Could not resolve address of function \"registerPlugins()\"" );
			FreeLibrary(hMod);
		}
	}
	else
	{
    this->Log.writef( "   Plugin DLL %s failed to load", pszName );
	}

    return usRC;     // Add for P402974
}

int PluginManagerImpl::findPluginEntry( OtmPlugin *pPlugin )
{
  for ( int i = 0; i < (int)vLoadedPluginDLLs.size(); i++ )
  {
    for( std::vector<OtmPlugin*>::iterator itPlugin = vLoadedPluginDLLs[i].vPluginList.begin(); itPlugin != vLoadedPluginDLLs[i].vPluginList.end(); ++itPlugin )
    {
      if ( *itPlugin == pPlugin )
      {
        return( i );
      }
    }
  }
  return( -1 );
}

bool PluginManagerImpl::stopPlugin( OtmPlugin* pPlugin, bool fForce )
{
   BOOL fLogHasBeenOpened = FALSE;

  // get some plugin information
  OtmPlugin::ePluginType eType = pPlugin->getType();
  std::string strName = pPlugin->getName();
  const char* strNameC = pPlugin->getName();
#ifdef _DEBUG
  if ( !this->Log.isOpen() ) {
     this->Log.open( "PluginManager" );
     fLogHasBeenOpened = TRUE;
  }
#endif
  this->Log.writef( "Stopping plugin: %s ", strNameC );

  // find DLL entry containing this plugin
  int iPluginEntry = findPluginEntry( pPlugin );
  if ( iPluginEntry == -1 ) return ( false );

  // inform all listeners that plugin is about to be stopped
  NotifyListeners( PluginListener::eAboutToStop, strName.c_str(), eType, fForce );

  // stop the plugin
  bool fStopped = pPlugin->stopPlugin( fForce );

  // inform all listeners that plugin has been stopped
  if ( fStopped )
  {
    NotifyListeners( PluginListener::eStopped, strName.c_str(), eType, fForce );
  }

  // for tool plugins we have to remove all menu items added by the plugin
  if ( (eType == OtmPlugin::eToolType) && !m_ToolPluginMenuItems.empty() )  // Modify for P402792
  {
    size_t i = m_ToolPluginMenuItems.size();
    do
    {
      i--;
      if ( m_ToolPluginMenuItems[i].pPlugin == pPlugin )
      {
        UtlDeleteMenuItem( m_ToolPluginMenuItems[i].szMenuName, m_ToolPluginMenuItems[i].iMenuItemID );
        m_ToolPluginMenuItems.erase( m_ToolPluginMenuItems.begin() + i );
      }
    } while (i != 0 );
  }

  // unload DLL when the last plugin of the DLL has been de-registered
  if ( (fStopped || fForce) && (vLoadedPluginDLLs[iPluginEntry].vPluginList.size() == 0) )
  {
    FreeLibrary( vLoadedPluginDLLs[iPluginEntry].hMod );
    vLoadedPluginDLLs.erase( (vLoadedPluginDLLs.begin()+iPluginEntry) );
  }

  this->Log.writef( "   Plugin stopped." );
  if ( fLogHasBeenOpened ) this->Log.close();

  return( fStopped );
}

bool PluginManagerImpl::stopPlugin( const char *pszPluginName, bool fForce )
{
  OtmPlugin *pPlugin = getPlugin( pszPluginName );
  if ( pPlugin != NULL ) return( stopPlugin( pPlugin, fForce ) );
  return( FALSE );
}

bool PluginManagerImpl::stopAllPlugins()
{
  size_t i = 0;
  BOOL fLogHasBeenOpened = FALSE;

#ifdef _DEBUG
  if ( !this->Log.isOpen() ) {
    this->Log.open( "PluginManager" );
    fLogHasBeenOpened = TRUE;
  } /* end */     
  this->Log.writef( "Stopping all plugins...");
#endif

  while ( i < vLoadedPluginDLLs.size() )
  {
    int iNumOfPluginDLLs = vLoadedPluginDLLs.size();
    
    // make a copy of the plugin list (original plugin list will be modified when stopping a plugin)
    std::vector<OtmPlugin *>vPluginList = vLoadedPluginDLLs[i].vPluginList;

    // loop over all entries in the plugin list
    for( std::vector<OtmPlugin*>::iterator itPlugin = vPluginList.begin(); itPlugin != vPluginList.end(); ++itPlugin )
    {
      stopPlugin( *itPlugin, true );
    }

    if ( (size_t)iNumOfPluginDLLs == vLoadedPluginDLLs.size() )
    {
      // remove of entry failed, continue with next one
      i++;
    } /* endif */
  } /* endwhile */
  if ( fLogHasBeenOpened ) this->Log.close();
  return( true );
}

//this method adds an listener to the vector of listeners
bool PluginManagerImpl::addPluginListener( PluginListener* pListener)
{
	std::vector<PluginListener*>::iterator temp = std::find(m_vPluginListener.begin(), m_vPluginListener.end(), pListener );
	//Return false if the listener is already in the vector. This is not expected. But there is nothing really wrong either
	if ( temp != m_vPluginListener.end() )
		return false;

	m_vPluginListener.push_back( pListener );
	return true;
}

//This method removes an observer from the vector
bool PluginManagerImpl::removePluginListener( PluginListener *pListener )
{
	std::vector<PluginListener*>::iterator temp = std::find(m_vPluginListener.begin(), m_vPluginListener.end(), pListener );
	//Return false if the listener could not be found (and evidently can’t be removed.
	if ( temp == m_vPluginListener.end() )
		return false;
	else
		m_vPluginListener.erase(std::remove(m_vPluginListener.begin(), m_vPluginListener.end(), pListener));
	return true;
		
	
}

//This Method notifies all listeners
bool PluginManagerImpl::NotifyListeners( PluginListener::eNotifcationType eNotifcation, const char *pszPluginName, OtmPlugin::ePluginType eType, bool fForce )
{
  for( std::vector<PluginListener *>::iterator itListener = m_vPluginListener.begin(); itListener != m_vPluginListener.end(); ++itListener )
  {
      (*itListener)->Notify( eNotifcation, pszPluginName, eType, fForce );
  }
	return (m_vPluginListener.size() > 0);
}

BOOL PluginManagerImpl::IsDepthOvered(const char * strPath)
{
    BOOL bOvered = FALSE;
    CHAR szPluginPath[ MAX_PATH ];
    UtlQueryString( QST_PLUGINPATH, szPluginPath, sizeof( szPluginPath ));
    UtlMakeEQFPath( szPluginPath, NULC, PLUGIN_PATH, NULL );

    string strCheckPath(strPath);
    if (!stricmp(strCheckPath.c_str(), szPluginPath))
    {
        return bOvered;
    }

    // convert to upper case
    strupr(szPluginPath);
    transform(strCheckPath.begin(), strCheckPath.end(), strCheckPath.begin(), toupper);

    string::size_type nPos = strCheckPath.find(szPluginPath);
    if (string::npos == nPos)
    {
        return bOvered;
    }

    int nLevel = 2;
    nPos += strlen(szPluginPath) + 1; // plus 1 mean there is no backslash after PLUGINS directory
    while ((nPos < strCheckPath.length()) && (string::npos != (nPos = strCheckPath.find("\\", nPos))))
    {
        nLevel++;
        nPos++;
    }

    if (nLevel > PLUGIN_PATH_MAX_DEPTH)
    {
        bOvered = TRUE;
    }

    return bOvered;
}

/*
void PluginManagerImpl::destroy()
{
	if (instance != 0)
		delete instance;
	instance = 0;
}
*/

/*! \brief Add a menu item to the OpenTM2 actionbar
	\param pToolPlugin pointer to the tool plugin processing the menu item
	\param pszMenuName name of the menu which is receiving the new menu item
	\param pszMenuItem name of the new menu item
	\param iCommandID command to be passed to tool plugin when the user selects the new menu item
	\returns TRUE when the menu item has beed added successfully
*/
bool PluginManagerImpl::addMenuItem( OtmToolPlugin *pToolPlugin, char *pszMenuName, char *pszMenuItem, int iCommandID )
{
  bool fOk = false;

  if ( (pToolPlugin == NULL) || (pszMenuName == NULL) || (pszMenuItem == NULL) ) return( false );

  // Add for P402792 start
 if (!m_ToolPluginMenuItems.empty())
  {
      for (size_t iInx = 0; iInx < m_ToolPluginMenuItems.size(); iInx++)
      {
          if (m_ToolPluginMenuItems[iInx].pPlugin == pToolPlugin &&
			  m_ToolPluginMenuItems[iInx].iCommandID == iCommandID )
          {
              fOk = true;
              return fOk;
          }
      }
  }
  // Add end

  fOk = UtlAddMenuItem( pszMenuName, pszMenuItem,  m_iNextCommandID, FALSE );

  // add entry to our list of tool plugin menu items
  if ( fOk )
  {
    TOOLPLUGINMENUITEM NewEntry;
    memset( &NewEntry, 0, sizeof(NewEntry) );
    NewEntry.iCommandID = iCommandID;
    NewEntry.pPlugin = pToolPlugin;
    NewEntry.iMenuItemID = m_iNextCommandID;
    strncpy( NewEntry.szMenuName, pszMenuName, sizeof(NewEntry.szMenuName)-1 );
    m_iNextCommandID += 1;
    m_ToolPluginMenuItems.push_back( NewEntry );
  }
  return( true );
}

/*! \brief Add a sub menu item to the OpenTM2 actionbar
	\param pToolPlugin pointer to the tool plugin processing the menu item
	\param pszMenuName name of the menu which is receiving the new sub menu
	\param pszSubMenu name of the new sub menu
	\returns TRUE when the sub menu has been added successfully
*/
bool PluginManagerImpl::addSubMenu( OtmToolPlugin *pToolPlugin, char *pszMenuName, char *pszSubMenu )
{
  if ( (pToolPlugin == NULL) || (pszMenuName == NULL) || (pszSubMenu == NULL) ) return( false );

  // Don't add sub menu for a plugin more than one time
  if (!m_ToolPluginMenuItems.empty())
  {
      for (size_t iInx = 0; iInx < m_ToolPluginMenuItems.size(); iInx++)
      {
          if (m_ToolPluginMenuItems[iInx].pPlugin == pToolPlugin)
          {
              return true;
          }
      }
  }

  BOOL fOk = UtlAddMenuItem( pszMenuName, pszSubMenu,  m_iNextCommandID, TRUE );

  // add entry to our list of tool plugin menu items
  if ( fOk )
  {
    TOOLPLUGINMENUITEM NewEntry;
    memset( &NewEntry, 0, sizeof(NewEntry) );
    NewEntry.iCommandID = m_iNextCommandID;
    NewEntry.pPlugin = pToolPlugin;
    NewEntry.iMenuItemID = m_iNextCommandID;
    strncpy( NewEntry.szMenuName, pszMenuName, sizeof(NewEntry.szMenuName)-1 );
    m_iNextCommandID += 1;
    m_ToolPluginMenuItems.push_back( NewEntry );
  }

  return( fOk ? true : false );
}

/*! \brief Activate all tool plugins. This method is called by the workbench once the main window has been created
*/
void PluginManagerImpl::activateToolPlugins()
{
	PLUGINSET::iterator it;
	for (it = pluginSet->begin(); it != pluginSet->end(); it++)
	{
    if ( (*it)->getType() == OtmPlugin::eToolType )
		{
      OtmToolPlugin *plugin = (OtmToolPlugin *)*it;
      plugin->init();
		}
	}

  return;
}

/*! \brief Process a tool plugin command. This method is called by the workbench when a menu item of a tool plugin is selected
	\param iCommandID ID of selected menu item
*/
void PluginManagerImpl::processToolCommand( int iCommandID )
{
  for ( size_t i = 0; i < m_ToolPluginMenuItems.size(); i++ )
	{
    if ( m_ToolPluginMenuItems[i].iMenuItemID == iCommandID )
		{
      m_ToolPluginMenuItems[i].pPlugin->processCommand( m_ToolPluginMenuItems[i].iCommandID );
      break;
		}
	}
}

// Add for P403115 start
BOOL DllNameCheck(const char * strName)
{
    BOOL bValid = TRUE;
    char strDiv[_MAX_DRIVE];
    char strDir[_MAX_DIR];
    char strFileName[_MAX_FNAME];
    char strExt[_MAX_EXT];

    _splitpath(strName, strDiv, strDir, strFileName, strExt);

    if ((NULL == strExt) || (strlen(strExt) == 0))
    {
        bValid = FALSE;
        return bValid;
    }

    if (stricmp(strExt, ".DLL") != 0)
    {
        bValid = FALSE;
    }

    return bValid;
}
// Add end
