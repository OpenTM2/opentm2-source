//+----------------------------------------------------------------------------+
//|GetToolInfo.CPP     OTM  Plugin Manager function                            |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|          Copyright (C) 1990-2016, International Business Machines          |
//|          Corporation and others. All rights reserved                       |
//|                                                                            |
//|                                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Author:             Flora Lee                                               |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Description:        This is module contains some functions which are used   |
//|                    during plugin manager parser                            |
//+----------------------------------------------------------------------------+
//|Entry Points:                                                               |
//|                                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Internals:                                                                  |
//|                                                                            |
//+----------------------------------------------------------------------------+
#include "StdAfx.h"
#include "GetToolInfo.h"

/*! \typedef PLUGINPROC
	Prototype for the registerPlugins() function
*/
typedef unsigned short (__cdecl *PLUGINPROC) ();   

/*! \typedef PLUGININFOPROC
	Prototype for the GetPluginInfo() function
*/
typedef unsigned short (__cdecl *PLUGININFOPROC) ( POTMPLUGININFO pInfo );  


USHORT GetDriveList( BYTE *szList, PULONG pulDriveListTime );

//
// GQ: The functions below have been borrowed from the OtmBase DLL in order to create a stand-alone version of the OtmGetToolInfo tool
//

void OtmCenterWindow(HWND hWnd)
{
    HWND hParentOrOwner;
    RECT rcParent, rcWnd;
    int x, y;
    if ((hParentOrOwner = GetParent(hWnd)) == NULL)
    {
        SystemParametersInfo(SPI_GETWORKAREA, 0, &rcParent, 0);
    }
    else
    {
        GetWindowRect(hParentOrOwner, &rcParent);
    }
    GetWindowRect(hWnd, &rcWnd);
    x = ((rcParent.right-rcParent.left) - (rcWnd.right-rcWnd.left)) / 2 + rcParent.left;
    y = ((rcParent.bottom-rcParent.top) - (rcWnd.bottom-rcWnd.top)) / 2 + rcParent.top;
    SetWindowPos(hWnd, HWND_TOP, x, y, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW);
}

USHORT APIENTRY DosLoadModule
 (
    PSZ pszFailName,                    // object name buffer
    USHORT cbFileName,                  // length of object name buffer
    PSZ pszModName,                     // dynamic link module name string
    PHMODULE phMod                      // module handle (returned)
 )
 {
   USHORT usRc = 0;                     // success indicator
   HINSTANCE hmod;

   pszFailName; cbFileName;
   DosError(0);                         // avoid error popup...
   hmod = LoadLibrary(pszModName );
   DosError(1);
   if ( hmod == NULL)
   {
     usRc = (USHORT)GetLastError();
     *phMod = (HMODULE) NULL;
   }
   else
   {
     *phMod = hmod;
   } /* endif */
   return usRc;
 }

USHORT APIENTRY DosError
 (
   USHORT fEnable                       // action flag (bit field)
 )
 {
   switch ( fEnable )
   {
     case 0:
       SetErrorMode( SEM_FAILCRITICALERRORS | SEM_NOOPENFILEERRORBOX );
       break;
     case 1:
       SetErrorMode(0);
       break;
   } /* endswitch */
   return 0;
 }

USHORT EqfDriveType(USHORT iDrive)
{
    USHORT   iType;

    //Validate possible drive indices
    if (0 > iDrive  || 25 < iDrive)
        return 0xFFFF;

        {
          DosError(0);
          CHAR szRoot[4] = "A:\\";
          szRoot[0] = (CHAR)('A' + iDrive);
          iType=(USHORT)GetDriveType(szRoot);
          DosError(1);
        }
    /*
     * Under Windows NT, GetDriveType returns complete information
     * not provided under Windows 3.x which we now get through other
     * means.
     */
    return iType;
}

SHORT UtlDriveType( CHAR chDrive )
{
  SHORT   sDriveID;                    // numeric ID (index) of drive

  toupper( chDrive );

  /********************************************************************/
  /* Check for valid drive chars                                      */
  /********************************************************************/
  if ( (chDrive < 'A') || (chDrive > 'Z') )
  {
    return( -1 );
  } /* endif */

  /********************************************************************/
  /* Get index of drive                                               */
  /********************************************************************/
  sDriveID = chDrive - 'A';

  /********************************************************************/
  /* Get type of drive                                                */
  /********************************************************************/
  return ( (SHORT)EqfDriveType( sDriveID ) );
} /* end of function UtlDriveType */

BYTE UtlQCurDisk()
{
    static CHAR szCurDir[MAX_PATH+100];

    DosError(0);
    if ( GetCurrentDirectory( sizeof(szCurDir), szCurDir ) == 0 )
    {
      szCurDir[0] = EOS;
    } /* endif */
    DosError(1);

    return (BYTE)szCurDir[0];
}

USHORT GetDriveList( BYTE *szList, PULONG pulDriveListTime )
{
    BYTE   bDrive;                     // currently logged drive
    WORD   wReturn;                    // return value from GetDriveType
    register int i;

    ULONG ulTime = GetTickCount();

    for (i=0, wReturn=0; i<26;i++)
    {
      CHAR szRoot[4] = "C:\\";
      szRoot[0] = (CHAR)('A' + i);
      wReturn = (WORD)GetDriveType( szRoot );
      switch ( wReturn )
      {
        case DRIVE_REMOVABLE :
        case DRIVE_FIXED   :
        case DRIVE_REMOTE  :
        case DRIVE_CDROM   :
        case DRIVE_RAMDISK :
          *szList++ = (BYTE) ('A' + i);
          break;

        case DRIVE_UNKNOWN :
        case DRIVE_NO_ROOT_DIR :
        default:
          break;
      } /* endswitch */
    } /* endfor */

    *szList = '\0';

    bDrive = UtlQCurDisk();

    if ( pulDriveListTime != NULL )
    {
      *pulDriveListTime = GetTickCount() - ulTime;
    } /* endif */
    return( (USHORT) (bDrive - 'A' + 1));   // set index relative to 0
}

USHORT UtlGetLANDriveList( PBYTE pszList )
{
  PBYTE pSource, pTarget;              // pointer for drive list processing
  SHORT sDriveType;                    // type of currently tested drive
  USHORT usRC;

  usRC = GetDriveList( pszList, NULL );    // get all available drives

  pSource = pTarget = pszList;         // start at begin of drive list

  while ( *pSource != NULC )           // while not end of list ....
  {
    sDriveType = UtlDriveType( *pSource );
    switch ( sDriveType )
    {
      case DRIVE_FIXED  :              
        pSource++;                     // ignore drive
        break;

      case DRIVE_REMOTE :
        *pTarget++ = *pSource++;       // leave drive in list
        break;

      default :
        pSource++;                     // ignore drive
        break;
    } /* endswitch */
  } /* endwhile */
  *pTarget = NULC;                     // terminate new drive list

  return( usRC );
} /* end of function UtlGetLANDriveList */

int GetToolInfo::ShowToolInfo()
{
    int nRC = NO_ERROR;

    nRC = DoInitialize();
    if (nRC)
    {
        return nRC;
    }

    if (!m_bCmd)
    {
//        nRC = ShowToolInfoByUI();
        return nRC;
    }
    else
    {
        ShowCmdHeader();
    }

    ShowRptHeader();

    nRC = ShowSystemInfo();
    if (nRC)
    {
        return nRC;
    }

    nRC = ShowNetworkInfo();
    if (nRC)
    {
        return nRC;
    }

    nRC = ShowToolRelatedInfo();

    char strShowMsg[MAX_BUF_SIZE];
    memset(strShowMsg, 0x00, sizeof(strShowMsg));

    if (!nRC)
    {
        char strLogPath[MAX_PATH];
        memset(strLogPath, 0x00, sizeof(strLogPath));
        sprintf(strLogPath, "\\%s\\%s\\", OTM_FOLDER_KEY, LOG_FOLDER_KEY);
        sprintf(strShowMsg, COMPLETE_SUCCESS_STR, LOG_FILE_NAME, strLogPath);
    }
    else
    {
        sprintf(strShowMsg, COMPLETE_FAIL_STR);
    }
    ShowInfoMsg(LINE_BREAK_STR);
    ShowInfoMsg(strShowMsg);

    printf(LINE_BREAK_STR);
    printf(strShowMsg);

    return nRC;
}

int GetToolInfo::ShowSystemInfo()
{
    int nRC = NO_ERROR;

    // Show title Info
    ShowInfoMsg(RPT_HEAD_1);
    ShowInfoMsg(FIRST_PATH_TITLE_STR);
    ShowInfoMsg(RPT_HEAD_1);

    // Show windows version
    char strShowMsg[MAX_LEN_MSG];
    memset(strShowMsg, 0x00, sizeof(strShowMsg));

    sprintf(strShowMsg, WIN_INFO_FORMAT_STR, GetSystemName());
    ShowInfoMsg(strShowMsg);

    memset(strShowMsg, 0x00, sizeof(strShowMsg));
    if (m_bWow64)
    {
        sprintf(strShowMsg, SYS_TYPE_STR, SYS_TYPE_64_STR);
    }
    else
    {
        sprintf(strShowMsg, SYS_TYPE_STR, SYS_TYPE_32_STR);
    }
    ShowInfoMsg(strShowMsg);

    // show environment variable info
    nRC = ShowEnvValue();

    // show all registry entries of OpenTM2
    nRC = ShowOTMRegInfo();

    return nRC;
}

int GetToolInfo::ShowNetworkInfo()
{
    int nRC = NO_ERROR;

    // show title info
    ShowInfoMsg(RPT_HEAD_1);
    ShowInfoMsg(SECOND_PATH_TITLE_STR);
    ShowInfoMsg(RPT_HEAD_1);

    // Show Drives Info
    nRC = ShowDrivesInfo();
    if (nRC)
    {
        return nRC;
    }

    // Show Memory info
    nRC = ShowMemoriesInfo();

    return nRC;
}

int GetToolInfo::ShowToolRelatedInfo()
{
    int nRC = NO_ERROR;

    ShowInfoMsg(LINE_BREAK_STR);

    // Show title Info
    ShowInfoMsg(RPT_HEAD_1);
    ShowInfoMsg(THREE_PATH_TITLE_STR);
    ShowInfoMsg(RPT_HEAD_1);

    // Show Version Info
    nRC = ShowRevisionInfo();
    if (nRC)
    {
        return nRC;
    }

    // Show plugins info
    nRC = ShowPluginsInfo();

    // Show all files in WIN folder
    nRC = ShowOtmWinFolder();
    if (nRC)
    {
        return nRC;
    }

    // Show Plugin folders
    nRC = ShowPluginFolder();

    return nRC;
}

int GetToolInfo::ShowRevisionInfo()
{
    int nRC = NO_ERROR;

    char strShowMsg[MAX_LEN_MSG];
    memset(strShowMsg, 0x00, sizeof(strShowMsg));

    // Get the version info
    HMODULE hResMod;
    DosLoadModule (NULL, NULLHANDLE, m_strResPath, &hResMod);

    char strRevision[MAX_OTM_VER_LEN];
    memset(strRevision, 0x00, sizeof(strRevision));
    LOADSTRING(NULLHANDLE, hResMod, SID_LOGO_REVISION, strRevision);
    sprintf(strShowMsg, OTM_VER_FORMAT_STR, strRevision);
    ShowInfoMsg(strShowMsg);

    return nRC;
}

int GetToolInfo::ShowEnvValue()
{
    int nRC = NO_ERROR;

    ShowInfoMsg(LINE_BREAK_STR);

    char strShowMsg[MAX_LEN_MSG];
    memset(strShowMsg, 0x00, sizeof(strShowMsg));
    sprintf(strShowMsg, SYS_ENV_TITLE_STR);
    ShowInfoMsg(strShowMsg);

    // show environment variable of PATH
    memset(strShowMsg, 0x00, sizeof(strShowMsg));

    LPSTR strSysPath = NULL;
    GetValFromRegEx(HKEY_LOCAL_MACHINE, REG_KEY_SYS_ENV, ENV_PATH_STR, strSysPath);

    if ((NULL == strSysPath) || (strlen(strSysPath) == 0))
    {
        sprintf(strShowMsg, ERROR_INFO_MISSING_STR, ENV_PATH_STR);
    }
    else
    {
        sprintf(strShowMsg, PATH_FORMAT_STR, strSysPath);
    }
    ShowInfoMsg(strShowMsg);

    // show environment variable of LOCPATH
    memset(strShowMsg, 0x00, sizeof(strShowMsg));
    char * strTemp = getenv(ENV_LOCPATH_STR);
    if ((NULL == strTemp) || (strlen(strTemp) == 0))
    {
        sprintf(strShowMsg, ERROR_INFO_MISSING_STR, ENV_LOCPATH_STR);
    }
    else
    {
        sprintf(strShowMsg, LOCPATH_FORMAT_STR, strTemp);
    }
    ShowInfoMsg(strShowMsg);

    ShowInfoMsg(LINE_BREAK_STR);

    memset(strShowMsg, 0x00, sizeof(strShowMsg));
    sprintf(strShowMsg, USR_ENV_TITLE_STR);
    ShowInfoMsg(strShowMsg);

    // show user variable of PATH
    char strUsrPath[MAX_BUF_SIZE];
    memset(strUsrPath, 0x00, sizeof(strUsrPath));
    GetValFromReg(HKEY_CURRENT_USER, KEY_ENVIRONMENT, ENV_PATH_STR, strUsrPath);

    // show user variable of PATH
    memset(strShowMsg, 0x00, sizeof(strShowMsg));
    if ((NULL == strUsrPath) || (strlen(strUsrPath) == 0))
    {
        sprintf(strShowMsg, ERROR_INFO_MISSING_STR, ENV_PATH_STR);
    }
    else
    {
        sprintf(strShowMsg, PATH_FORMAT_STR, strUsrPath);
    }
    ShowInfoMsg(strShowMsg);

    memset(strUsrPath, 0x00, sizeof(strUsrPath));
    GetValFromReg(HKEY_CURRENT_USER, KEY_ENVIRONMENT, ENV_LOCPATH_STR, strUsrPath);

    // show environment variable of LOCPATH
    memset(strShowMsg, 0x00, sizeof(strShowMsg));
    if ((NULL == strUsrPath) || (strlen(strUsrPath) == 0))
    {
        sprintf(strShowMsg, ERROR_INFO_MISSING_STR, ENV_LOCPATH_STR);
    }
    else
    {
        sprintf(strShowMsg, LOCPATH_FORMAT_STR, strUsrPath);
    }
    ShowInfoMsg(strShowMsg);

    return nRC;
}

int GetToolInfo::ShowOTMRegInfo()
{
    int nRC = NO_ERROR;

    ShowInfoMsg(LINE_BREAK_STR);

    // show title
    char strShowMsg[MAX_LEN_MSG];
    memset(strShowMsg, 0x00, sizeof(strShowMsg));
    sprintf(strShowMsg, REG_INFO_TITLE_STR);
    ShowInfoMsg(strShowMsg);

    EnumQueryKey(HKEY_LOCAL_MACHINE, REG_KEY_LOCAL_MACHINE_STR, OPENTM2_APP_NAME_STR);

    EnumQueryKey(HKEY_CURRENT_USER, REG_KEY_CURRENT_USER_STR, OPENTM2_APP_NAME_STR);

    if (!m_bHasLocMacUni)
    {
        memset(strShowMsg, 0x00, sizeof(strShowMsg));
        sprintf(strShowMsg, ERROR_INFO_MISSING_STR, m_strLocMacUni);
        ShowInfoMsg(strShowMsg);
    }

    if (!m_bHasLocMac)
    {
        memset(strShowMsg, 0x00, sizeof(strShowMsg));
        sprintf(strShowMsg, ERROR_INFO_MISSING_STR, m_strLocMac);
        ShowInfoMsg(strShowMsg);
    }

    if (!m_bHasCurUsr)
    {
        memset(strShowMsg, 0x00, sizeof(strShowMsg));
        sprintf(strShowMsg, ERROR_INFO_MISSING_STR, m_strCurUsr);
        ShowInfoMsg(strShowMsg);
    }

    if (!m_bHasCurUsrVirUni)
    {
        memset(strShowMsg, 0x00, sizeof(strShowMsg));
        sprintf(strShowMsg, ERROR_INFO_MISSING_STR, m_strCurUsrVirUni);
        ShowInfoMsg(strShowMsg);
    }

    if (!m_bHasCurUsrVir)
    {
        memset(strShowMsg, 0x00, sizeof(strShowMsg));
        sprintf(strShowMsg, ERROR_INFO_MISSING_STR, m_strCurUsrVir);
        ShowInfoMsg(strShowMsg);
    }

    ShowInfoMsg(LINE_BREAK_STR);

    return nRC;
}

int GetToolInfo::ShowDrivesInfo()
{
    int nRC = NO_ERROR;

    char strShowMsg[MAX_LEN_MSG];
    memset(strShowMsg, 0x00, sizeof(strShowMsg));

    // Get the Drive Info
    CHAR szDrives[MAX_DRIVELIST];
    ULONG ulDriveListTime = 0;
    GetDriveList((BYTE *) szDrives, &ulDriveListTime );

    sprintf(strShowMsg, DRIVE_INFO_TITLE_STR);
    ShowInfoMsg(strShowMsg);
    ShowDriveInfo(szDrives);
    sprintf(strShowMsg, DRIVE_LIST_TIME_FORMAT_STR, ulDriveListTime );
    ShowInfoMsg(strShowMsg);
    ShowInfoMsg(LINE_BREAK_STR);

    // Get the LAN Drives
    char strDriveList[MAX_PATH];
    memset(strDriveList, 0x00, sizeof(strDriveList));

    char szLANDriveList[MAX_DRIVELIST];
    UtlGetLANDriveList((PBYTE)szLANDriveList);
    if ((NULL != szDrives) && (strlen(szDrives) > 0))
    {
        AddComma(szLANDriveList, strDriveList);
    }

    memset(strShowMsg, 0x00, sizeof(strShowMsg));
    sprintf(strShowMsg, LAN_INFO_TITLE_STR, strDriveList);
    ShowInfoMsg(strShowMsg);
    ShowInfoMsg(LINE_BREAK_STR);

    return nRC;
}

BOOL GetToolInfo::CheckDllName(const char * strFullName)
{
    BOOL bValid = TRUE;
    static char strExt[_MAX_EXT];

    _splitpath( strFullName, NULL, NULL, NULL, strExt);

    if ( strlen(strExt) == 0 )
    {
        bValid = FALSE;
        return bValid;
    }

    if (stricmp( strExt, ".DLL") != 0 )
    {
        bValid = FALSE;
    }

    return bValid;
}

int GetToolInfo::ShowPluginInfo(const char* pszName)
{
  int nRC = NO_ERROR;
  static OTMPLUGININFO PluginInfo;
  char strShowMsg[MAX_LEN_MSG];
  int i = 1;


  SetErrorMode(  SEM_FAILCRITICALERRORS | SEM_NOOPENFILEERRORBOX  );
	HMODULE hMod = LoadLibrary(pszName);
  SetErrorMode(0);
  if ( hMod == 0 )
  {
    // get error message text
    LPTSTR errorText = NULL;
    DWORD dwError = GetLastError();
    FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, dwError, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&errorText, 0, NULL);   
    if ( NULL != errorText )
    {
       // ... do something with the string `errorText` - log it, display it to the user, etc.

       // release memory allocated by FormatMessage()
       LocalFree(errorText);
       errorText = NULL;
    }
  }
	if ( hMod != 0 )
	{
		PLUGINPROC pFunc = (PLUGINPROC) GetProcAddress(hMod, "registerPlugins");
		if ( pFunc != 0 )
		{
      // get plugin info (when provided)
		  PLUGININFOPROC pInfoFunc = (PLUGININFOPROC) GetProcAddress(hMod, "getPluginInfo");
      memset( &PluginInfo, 0, sizeof(PluginInfo) );
      if ( pInfoFunc != NULL )
      {
        // use plugin info function to retrieve the plugin information
        memset( &PluginInfo, 0, sizeof(PluginInfo) );
        pInfoFunc( &PluginInfo );
      }

      // use DLL name as plugin name when no info is available
      if ( PluginInfo.szName[0] == 0 )
      {
        _splitpath( pszName, NULL, NULL, PluginInfo.szName, NULL );
        strcpy( PluginInfo.szVersion, "n/a (pre-Otm1.3.2 plugin)" );
      }

      memset(strShowMsg, 0x00, sizeof(strShowMsg));
      sprintf(strShowMsg, "      (%2d) %-39s%s\n", i++, PluginInfo.szName, PluginInfo.szVersion );
      ShowInfoMsg(strShowMsg);
    }
    FreeLibrary(hMod);
  }
  return nRC;
}

int GetToolInfo::ScanPluginDir( const char *pszPluginDir, int iDepth )
{
  int nRC = NO_ERROR;
	std::string strFileSpec(pszPluginDir);
	BOOL fMoreFiles = TRUE;
	HANDLE hDir = HDIR_CREATE;
	WIN32_FIND_DATA ffb;


  // check the depths of the cycle
  if ( iDepth > 1)
  {
      return nRC;
  }

	strFileSpec += "\\*.dll";

  // scan all DLLs located in current directory
	hDir = FindFirstFile( strFileSpec.c_str(), &ffb );
  if ( hDir != INVALID_HANDLE_VALUE )
  {
    fMoreFiles = TRUE;
	  while ( fMoreFiles )
	  {
		  std::string strDll(pszPluginDir);
		  strDll += '\\';
		  strDll += ffb.cFileName;

      if ( CheckDllName(strDll.c_str()) )
      {
        ShowPluginInfo( strDll.c_str() );
      }

		  fMoreFiles= FindNextFile( hDir, &ffb );
	  } /* endwhile */
    FindClose( hDir );
  } /* endif */     

  // scan all subdirectories
  if ( iDepth < 1 )
  {
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
          nRC = ScanPluginDir( strSubdir.c_str(), iDepth + 1 );
		    }
		    fMoreFiles= FindNextFile( hDir, &ffb );
	    }
      FindClose( hDir );
    } /* endif */     
  }

  return nRC;     
}

int GetToolInfo::ShowPluginsInfo()
{
    int nRC = NO_ERROR;

    ShowInfoMsg(LINE_BREAK_STR);
    ShowInfoMsg(PLUGIN_INFO_STR);

    nRC = ScanPluginDir( m_strPluginPath, 0 );

    return nRC;
}

int GetToolInfo::ShowOtmWinFolder()
{
    int nRC = NO_ERROR;

    ShowInfoMsg(LINE_BREAK_STR);

    // show title
    char strWinPath[MAX_PATH];
    memset(strWinPath, 0x00, sizeof(strWinPath));
    sprintf(strWinPath, "\\%s\\%s\\", OTM_FOLDER_KEY, WIN_FOLDER_KEY);

    char strShowMsg[MAX_BUF_SIZE];
    memset(strShowMsg, 0x00, sizeof(strShowMsg));
    sprintf(strShowMsg, WIN_FLD_TITLE_STR, strWinPath);
    ShowInfoMsg(strShowMsg);

    nRC = ShowFileName(m_strAppMainPath);

    return nRC;
}

int GetToolInfo::ShowFileName(const char * strStartPath)
{
    int nRC = NO_ERROR;

    WIN32_FIND_DATA otmFindData;
    ZeroMemory(&otmFindData, sizeof(WIN32_FIND_DATA));

    HANDLE hFindHandle;
    BOOL bFinished = FALSE;

    char strSearchPath[MAX_PATH];
    char strTmpTgtDir[MAX_PATH];
    memset(strSearchPath, 0x00, sizeof(strSearchPath));
    memset(strTmpTgtDir,  0x00, sizeof(strTmpTgtDir));

    strcpy(strSearchPath, strStartPath);
    if (strSearchPath[strlen(strSearchPath) - 1] != '\\')
    {
       strcat(strSearchPath, "\\");
    }
    strcpy(strTmpTgtDir, strSearchPath);
    strcat(strSearchPath, "*");

    hFindHandle = FindFirstFile((LPCTSTR)strSearchPath, &otmFindData);
    if (INVALID_HANDLE_VALUE == hFindHandle)
    {
        bFinished = TRUE;           // this is nothing in the target directory
    }

    char strShowMsg[MAX_LEN_MSG];

    while (!bFinished)
    {
        sprintf(strSearchPath, "%s%s", strTmpTgtDir, otmFindData.cFileName);
        // if directory is searched
        if (otmFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            if (otmFindData.cFileName[0] != '.')
            {
                ShowFileName(strSearchPath);
            }
        }
        else
        {
            // show file name
            char strFileDate[MAX_PATH];
            memset(strFileDate, 0x00, sizeof(strFileDate));

            SYSTEMTIME   stLstTime;
            FileTimeToSystemTime(&otmFindData.ftLastWriteTime, &stLstTime);
            sprintf(strFileDate, "%d-%d-%d %d:%d:%d",stLstTime.wYear,
                    stLstTime.wMonth,stLstTime.wDay,stLstTime.wHour,stLstTime.wMinute,stLstTime.wSecond);

            _int64 nFileSize =  ((_int64)otmFindData.nFileSizeHigh * ((_int64)MAXDWORD + 1)) + (_int64)otmFindData.nFileSizeLow;

            memset(strShowMsg, 0x00, sizeof(strShowMsg));
            sprintf(strShowMsg, "      %-40s    %-20s    %I64d\n", otmFindData.cFileName, strFileDate, nFileSize);
            ShowInfoMsg(strShowMsg);
        }

        if (!FindNextFile(hFindHandle, &otmFindData))
        {
            bFinished = TRUE;
        }
    }
    FindClose(hFindHandle);

    return nRC;
}

int GetToolInfo::ShowPluginFolder()
{
    int nRC = NO_ERROR;

    ShowInfoMsg(LINE_BREAK_STR);

    // show title
    char strPluginPath[MAX_PATH];
    memset(strPluginPath, 0x00, sizeof(strPluginPath));
    sprintf(strPluginPath, "\\%s\\%s\\", OTM_FOLDER_KEY, PLUGIN_FOLDER_KEY);

    char strShowMsg[MAX_BUF_SIZE];
    memset(strShowMsg, 0x00, sizeof(strShowMsg));
    sprintf(strShowMsg, PLUGIN_FLD_TITLE_STR, strPluginPath);
    ShowInfoMsg(strShowMsg);

    // show path of Plugins
    nRC = ShowPluginPaths(strPluginPath);

    return nRC;
}

int GetToolInfo::ShowPluginPaths(const char * strStartPath)
{
    int nRC = NO_ERROR;

    STRINGGRP vecSubDir;
    nRC = ShowPluginPath(strStartPath, vecSubDir);
    if (nRC || vecSubDir.empty())
    {
        return nRC;
    }

    for (size_t iInx = 0; iInx < vecSubDir.size(); iInx++)
    {
        char strShowMsg[MAX_BUF_SIZE];
        memset(strShowMsg, 0x00, sizeof(strShowMsg));
        sprintf(strShowMsg, "\n      %s\n", vecSubDir[iInx].c_str());
        ShowInfoMsg(strShowMsg);

        STRINGGRP vecSubSubDir;
        if (!m_bDetails)
        {
            nRC = ShowPluginPath(vecSubDir[iInx].c_str(), vecSubSubDir);
        }
        else
        {
            nRC = ShowPluginPaths(vecSubDir[iInx].c_str());
        }

        if (nRC)
        {
            break;
        }
    }

    return nRC;
}

int GetToolInfo::ShowPluginPath(const char * strStartPath, STRINGGRP & vecSubDir)
{
    int nRC = NO_ERROR;

    WIN32_FIND_DATA otmFindData;
    ZeroMemory(&otmFindData, sizeof(WIN32_FIND_DATA));

    HANDLE hFindHandle;
    BOOL bFinished = FALSE;

    char strSearchPath[MAX_PATH];
    char strTmpTgtDir[MAX_PATH];
    memset(strSearchPath, 0x00, sizeof(strSearchPath));
    memset(strTmpTgtDir,  0x00, sizeof(strTmpTgtDir));

    strcpy(strSearchPath, strStartPath);
    if (strSearchPath[strlen(strSearchPath) - 1] != '\\')
    {
       strcat(strSearchPath, "\\");
    }
    strcpy(strTmpTgtDir, strSearchPath);
    strcat(strSearchPath, "*");

    hFindHandle = FindFirstFile((LPCTSTR)strSearchPath, &otmFindData);
    if (INVALID_HANDLE_VALUE == hFindHandle)
    {
        bFinished = TRUE;           // this is nothing in the target directory
    }

    char strShowMsg[MAX_LEN_MSG];

    while (!bFinished)
    {
        sprintf(strSearchPath, "%s%s", strTmpTgtDir, otmFindData.cFileName);
        // if directory is searched
        if (otmFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            if (otmFindData.cFileName[0] != '.')
            {
                vecSubDir.push_back(strSearchPath);
            }
        }
        else
        {
            // show file name
            char strFileDate[MAX_PATH];
            memset(strFileDate, 0x00, sizeof(strFileDate));

            SYSTEMTIME   stLstTime;
            FileTimeToSystemTime(&otmFindData.ftLastWriteTime, &stLstTime);
            sprintf(strFileDate, "%d-%d-%d %d:%d:%d",stLstTime.wYear,
                    stLstTime.wMonth,stLstTime.wDay,stLstTime.wHour,stLstTime.wMinute,stLstTime.wSecond);

            _int64 nFileSize =  ((_int64)otmFindData.nFileSizeHigh * ((_int64)MAXDWORD + 1)) + (_int64)otmFindData.nFileSizeLow;

            memset(strShowMsg, 0x00, sizeof(strShowMsg));
            sprintf(strShowMsg, "      | %-40s    %-20s    %I64d\n", otmFindData.cFileName, strFileDate, nFileSize);
            ShowInfoMsg(strShowMsg);
        }

        if (!FindNextFile(hFindHandle, &otmFindData))
        {
            bFinished = TRUE;
        }
    }
    FindClose(hFindHandle);

    return nRC;
}

int GetToolInfo::ShowFileDetails(const char * strStartPath)
{
    int nRC = NO_ERROR;

    WIN32_FIND_DATA otmFindData;
    ZeroMemory(&otmFindData, sizeof(WIN32_FIND_DATA));

    HANDLE hFindHandle;
    BOOL bFinished = FALSE;

    char strSearchPath[MAX_PATH];
    char strTmpTgtDir[MAX_PATH];
    memset(strSearchPath, 0x00, sizeof(strSearchPath));
    memset(strTmpTgtDir,  0x00, sizeof(strTmpTgtDir));

    strcpy(strSearchPath, strStartPath);
    if (strSearchPath[strlen(strSearchPath) - 1] != '\\')
    {
       strcat(strSearchPath, "\\");
    }
    strcpy(strTmpTgtDir, strSearchPath);
    strcat(strSearchPath, "*");

    hFindHandle = FindFirstFile((LPCTSTR)strSearchPath, &otmFindData);
    if (INVALID_HANDLE_VALUE == hFindHandle)
    {
        bFinished = TRUE;           // this is nothing in the target directory
    }

    char strShowMsg[MAX_LEN_MSG];

    while (!bFinished)
    {
        sprintf(strSearchPath, "%s%s", strTmpTgtDir, otmFindData.cFileName);
        // if directory is searched
        if (otmFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            if (otmFindData.cFileName[0] != '.' && m_bDetails)
            {
                ShowFileName(strSearchPath);
            }
        }
        else
        {
            // show file name
            char strFileDate[MAX_PATH];
            memset(strFileDate, 0x00, sizeof(strFileDate));

            SYSTEMTIME   stLstTime;
            FileTimeToSystemTime(&otmFindData.ftLastWriteTime, &stLstTime);
            sprintf(strFileDate, "%d-%d-%d %d:%d:%d",stLstTime.wYear,
                    stLstTime.wMonth,stLstTime.wDay,stLstTime.wHour,stLstTime.wMinute,stLstTime.wSecond);

            _int64 nFileSize =  ((_int64)otmFindData.nFileSizeHigh * ((_int64)MAXDWORD + 1)) + (_int64)otmFindData.nFileSizeLow;

            memset(strShowMsg, 0x00, sizeof(strShowMsg));
            sprintf(strShowMsg, "      %-40s    %-20s    %I64d\n", otmFindData.cFileName, strFileDate, nFileSize);
            ShowInfoMsg(strShowMsg);
        }

        if (!FindNextFile(hFindHandle, &otmFindData))
        {
            bFinished = TRUE;
        }
    }
    FindClose(hFindHandle);

    return nRC;
}

int GetToolInfo::ShowToolInfoByUI()
{
    int nRC = NO_ERROR;

    HWND hwndGetTInfoDlg;

    hwndGetTInfoDlg = CreateDialogParam(m_hInstance, MAKEINTRESOURCE(IDD_DIALOG1), NULL, DialogProc, NULL);
    if (hwndGetTInfoDlg == NULL)
    {
        MessageBox(NULL, "Fail to create the dialog.", "", MB_OK);
        return 0;
    }

    MSG msgGetTInfo;

    while (GetMessage(&msgGetTInfo, NULL, NULL, NULL))
    {
        if(!IsDialogMessage(hwndGetTInfoDlg, &msgGetTInfo))
        {
            TranslateMessage(&msgGetTInfo);
            DispatchMessage(&msgGetTInfo);
        }
    }

    return nRC;
}

const char * GetToolInfo::GetSystemName()
{
    SYSTEM_INFO systemInfo;
    GetSystemInfo(&systemInfo);

    OSVERSIONINFOEX osVerInfo;
    osVerInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

    if (GetVersionEx((OSVERSIONINFO *)&osVerInfo))
    {
        m_dwMajVer = osVerInfo.dwMajorVersion;

        switch (osVerInfo.dwMajorVersion)                            // major version
        {
        case 4:
            switch(osVerInfo.dwMinorVersion)                        // minor version
            {
            case 0:
                if(osVerInfo.dwPlatformId == VER_PLATFORM_WIN32_NT)
                {
                    return WIN_NT_STR;
                }
                else if(osVerInfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
                {
                    return WIN_95_STR;
                }
                break;
            case 10:
                return WIN_98_STR;
            case 90:
                return WIN_ME_STR;
            }
            break;
        case 5:
            switch (osVerInfo.dwMinorVersion)
            {
            case 0:
                return WIN_2000_STR;
            case 1:
                return WIN_XP_STR;
            case 2:
                if ((osVerInfo.wProductType == VER_NT_WORKSTATION) &&
                    (systemInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64))
                {
                    return WIN_XP_64_STR;
                }
                else if (GetSystemMetrics(SM_SERVERR2) == 0)
                {
                    return WIN_2003_STR;
                }
                else if (GetSystemMetrics(SM_SERVERR2) != 0)
                {
                    return WIN_2003_R2_STR;
                }
                break;
            }
            break;
        case 6:
            switch(osVerInfo.dwMinorVersion)
            {
            case 0:
                if (osVerInfo.wProductType == VER_NT_WORKSTATION)
                {
                    return WIN_VISTA_STR;
                }
                else
                {
                    return WIN_2008_SEV_STR;
                }
                break;
            case 1:
                if (osVerInfo.wProductType==VER_NT_WORKSTATION)
                {
                    return WIN_2007_STR;
                }
                else
                {
                    return WIN_2008_R2_STR;
                }
                break;
            case 2:
                return WIN_2007_STR;
            }
            break;
        default:
            return UNKNOW_SYS_STR;
        }
    }

    return UNKNOW_SYS_STR;
}

int GetToolInfo::GetAppVersion()
{
    int nRC = NO_ERROR;
    char strModName[MAX_PATH];

    memset(strModName, 0x00, sizeof(strModName));
    DWORD dwVerHandle, dwVerInfoSize;

    GetModuleFileName(NULL, strModName, sizeof(strModName));

    dwVerInfoSize = GetFileVersionInfoSize(strModName, &dwVerHandle);
    if (dwVerInfoSize == 0)
    {
        return nRC;
    }

    char * strVerInfoBuf = new char [dwVerInfoSize];
    GetFileVersionInfo(strModName, 0, dwVerInfoSize, strVerInfoBuf);

    unsigned int  nTranslate = 0;
    struct LANGANDCODEPAGE {
        WORD wLanguage;
        WORD wCodePage;
    } * lpTranslate;

    VerQueryValue(strVerInfoBuf, TEXT("\\VarFileInfo\\Translation"), (LPVOID*)&lpTranslate, &nTranslate);

    int nTotal = (int)(nTranslate / sizeof(struct LANGANDCODEPAGE));
    // Read the file description for the first language and code page.
    // use the first version as the tool version
//    for (int iInx = 0; iInx < nTotal; iInx++)
    if (nTotal > 0)
    {
        char strSubBlock[MAX_BUF_SIZE];
        memset(strSubBlock, 0x00, sizeof(strSubBlock));

        sprintf(strSubBlock, TEXT("\\StringFileInfo\\%04x%04x\\FileVersion"), lpTranslate[0].wLanguage, lpTranslate[0].wCodePage);
 
        void * lpBuffer = NULL;
        unsigned int dwBytes = 0;

        VerQueryValue(strVerInfoBuf, strSubBlock, &lpBuffer, &dwBytes);
        strncpy(m_strAppVer, (char *)lpBuffer, dwBytes);
    }

    return nRC;
}

int GetToolInfo::GetToolAppPath()
{
    int nRC = NO_ERROR;

    // get OpenTM2 properties path
    HKEY hKey = NULL;

    REGSAM regOption = NULL;
    char strSubKey[MAX_PATH];
    memset(strSubKey, 0x00, sizeof(strSubKey));

    if (m_bWow64)
    {
        regOption = KEY_READ | KEY_WOW64_64KEY;
        sprintf(strSubKey, "%s\\%s", APP_SOFTWARE_STR, REG_KEY_WOW_6432_NODE_STR);
    }
    else
    {
        regOption = KEY_READ | KEY_WOW64_32KEY;
        sprintf(strSubKey, "%s", APP_SOFTWARE_STR);
    }

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, strSubKey, 0, regOption, &hKey) == ERROR_SUCCESS)
    {
        HKEY hSubKey = NULL;
        if (RegOpenKeyEx(hKey, OPENTM2_APP_NAME_STR, 0, regOption, &hSubKey) == ERROR_SUCCESS)
        {
            DWORD dwType = REG_SZ;
            DWORD iSize = sizeof(m_strAppPath);
            int iSuccess = RegQueryValueEx(hSubKey, KEY_DRIVE, 0, &dwType, (LPBYTE)m_strAppPath, &iSize);
            if (iSuccess == ERROR_SUCCESS)
            {
                dwType = REG_SZ;
                iSize = sizeof(m_strAppPath);
                strcat(m_strAppPath, "\\");
                iSuccess = RegQueryValueEx(hSubKey, KEY_PATH, 0, &dwType, (LPBYTE)(m_strAppPath + strlen(m_strAppPath)), &iSize);
                if (iSuccess != ERROR_SUCCESS)
                {
                    m_strAppPath[0] = 0;
                }
            }
            RegCloseKey(hSubKey);
        }
        RegCloseKey(hKey);
    }

    if ((NULL == m_strAppPath) || (m_strAppPath[0] == 0))
    {
        char strModule[MAX_PATH];
        memset(strModule, 0x00, sizeof(strModule));

        GetModuleFileName(NULL, strModule, sizeof(strModule));
        GetModuleAppPath(strModule);
    }

    if ((NULL == m_strAppPath) || (m_strAppPath[0] == 0))
    {
        ShowErrMsg(ERROR_CANNOT_GET_REGISTRY_INFO_STR);
        return ERROR_CANNOT_GET_REGISTRY_INFO;
    }

    Join2Path(m_strAppPath, WIN_FOLDER_KEY, m_strAppMainPath);
    Join2Path(m_strAppPath, PLUGIN_FOLDER_KEY, m_strPluginPath);

    // remove the previous report file
    Join2Path(m_strAppPath, LOG_FOLDER_KEY, m_strRptPath);
    Join2Path(m_strRptPath, LOG_FILE_NAME,  m_strRptFile);
    Join2Path(m_strAppMainPath, RES_DLL_KEY, m_strResPath);

    if ((NULL == m_strAppMainPath) || (m_strAppMainPath[0] == 0) ||
        (NULL == m_strPluginPath) || (m_strPluginPath[0] == 0) || 
        (NULL == m_strRptPath) || (m_strRptPath[0] == 0) || 
        (NULL == m_strResPath) || (m_strResPath[0] == 0))
    {
        ShowErrMsg(ERROR_CANNOT_GET_APP_PATH_STR);
        return ERROR_CANNOT_GET_APP_PATH;
    }

    remove(m_strRptFile);

    return nRC;
}

BOOL GetToolInfo::OtmIsWow64()
{
    BOOL bWow64 = FALSE; 

    typedef BOOL (WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);
    LPFN_ISWOW64PROCESS fnIsWow64Process;
    fnIsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress(GetModuleHandle("kernel32"), "IsWow64Process");
    if (NULL != fnIsWow64Process)
    {
        fnIsWow64Process(GetCurrentProcess(), &bWow64);
    }

    return bWow64;
}

void GetToolInfo::GetModuleAppPath(const char * strModule)
{
    char strDrive[_MAX_DRIVE];
    char strDir[_MAX_DIR];
    char strFname[_MAX_FNAME];
    char strExt[_MAX_EXT];
    _splitpath(strModule, strDrive, strDir, strFname, strExt);

    // remove last path delimiter
    int nLen = strlen(strDir);
    strDir[nLen-1] = '\0';

    // find the last path delimiter
    char * strPos = strrchr(strDir, '\\');
    nLen = strPos - strDir;

    if (nLen <= 0)
    {
        return;
    }

    strDir[nLen] = '\0';
    sprintf(m_strAppPath, "%s\\%s", strDrive, strDir);

}

//BOOL GetToolInfo::SetupUtils( HAB hab, PSZ  pMsgFile )
//{
//    BOOL   fOK;                          // success indicator
//    CHAR   szDrive[MAX_DRIVE];           // buffer for drive list
//    CHAR   szLanDrive[2];                // buffer for LAN drive
//    CHAR   EqfSystemMsgFile[MAX_EQF_PATH];  // global message file
//    CHAR   EqfSystemHlpFile[MAX_EQF_PATH];  // global help file
//    CHAR   szEqfSysLanguage[MAX_EQF_PATH];  // system language
//    CHAR   szEqfResFile[MAX_EQF_PATH];      // resource
//
//    UtlSetULong( QL_HAB, (ULONG) hab );
//
//    UtlSetULong( QL_TWBFRAME, (ULONG) HWND_DESKTOP );
//    UtlSetULong( QL_TWBCLIENT, (ULONG) HWND_DESKTOP );
//
//    GetStringFromRegistry( APPL_Name, KEY_Drive, szDrive, sizeof( szDrive  ), "" );
//    strupr( szDrive );
//
//    GetStringFromRegistry( APPL_Name, KEY_LanDrive, szLanDrive, sizeof( szLanDrive  ), "" );
//    strupr( szLanDrive );
//
//    // Get name of current language
//    GetStringFromRegistry( APPL_Name, KEY_SYSLANGUAGE, szEqfSysLanguage, sizeof( szEqfSysLanguage ), DEFAULT_SYSTEM_LANGUAGE );
//
//    // Set name of resource, help file and message file for selected language
//    fOK = UtlQuerySysLangFile( szEqfSysLanguage, szEqfResFile, EqfSystemHlpFile, EqfSystemMsgFile );
//
//    UtlSetString( QST_PRIMARYDRIVE, szDrive );
//    UtlSetString( QST_LANDRIVE,     szLanDrive );
//    UtlSetString( QST_PROPDIR,      "PROPERTY" );
//    UtlSetString( QST_CONTROLDIR,   "CTRL" );
//    UtlSetString( QST_PROGRAMDIR,   "PGM" );
//    UtlSetString( QST_DICDIR,       "DICT" );
//    UtlSetString( QST_MEMDIR,       "MEM" );
//    UtlSetString( QST_TABLEDIR,     "TABLE" );
//    UtlSetString( QST_LISTDIR,      "LIST" );
//    UtlSetString( QST_DLLDIR,       "DLL" );
//    UtlSetString( QST_MSGDIR,       "MSG" );
//    UtlSetString( QST_EXPORTDIR,    "EXPORT" );
//    UtlSetString( QST_IMPORTDIR,    "IMPORT" );
//    UtlSetString( QST_SYSTEMDIR,    "OTM" );
//    UtlSetString( QST_SOURCEDIR,    "SOURCE" );
//    UtlSetString( QST_TARGETDIR,    "TARGET" );
//    UtlSetString( QST_SEGSOURCEDIR, "SSOURCE" );
//    UtlSetString( QST_SEGTARGETDIR, "STARGET" );
//    UtlSetString( QST_DIRSEGNOMATCHDIR, "SNOMATCH" );
//    UtlSetString( QST_MTLOGPATH,    "MTLOG" );
//    UtlSetString( QST_DIRSEGMTDIR, "MT" );
//    UtlSetString( QST_DIRSEGRTFDIR, "RTF" );
//    UtlSetString( QST_LOGPATH,           "LOGS" );
//    UtlSetString( QST_XLIFFPATH,         "XLIFF" );
//    UtlSetString( QST_METADATAPATH,      "METADATA" );
//    UtlSetString( QST_JAVAPATH,          "JAVA" );
//    UtlSetString( QST_ENTITY,            "ENTITY" );
//    UtlSetString( QST_REMOVEDDOCDIR,     "REMOVED" );
//    UtlSetString( QST_MSGFILE,       EqfSystemMsgFile );
//    UtlSetString( QST_HLPFILE,       EqfSystemHlpFile );
//    UtlSetString( QST_PLUGINPATH,     "PLUGINS" );
//
//    UtlSetString( QST_RESFILE,       szEqfResFile );
//
//    /********************************************************************/
//    /* return message file name - if requested                          */
//    /********************************************************************/
//    if ( pMsgFile )
//    {
//        strcpy( pMsgFile, EqfSystemMsgFile );
//    } /* endif */
//
//    return fOK;
//} /* end of SetupUtils */

int GetToolInfo::GetValFromReg(HKEY hStartKey, const char * strSubKey, const char * strKey, char * strValue)
{
    int nRC = NO_ERROR;
    // get OpenTM2 properties path
    HKEY hKey = NULL;
    DWORD dRet = RegOpenKeyEx(hStartKey, strSubKey, 0, KEY_READ, &hKey);

    if (dRet == ERROR_SUCCESS)
    {
        DWORD dwType = REG_SZ;
        DWORD iSize = MAX_BUF_SIZE;
        dRet = RegQueryValueEx(hKey, strKey, 0, &dwType, (LPBYTE)strValue, &iSize);
        if (dRet != ERROR_SUCCESS)
        {
            nRC = ERROR_GET_REG_VALUE;
        }
        RegCloseKey(hKey);
    }
    else
    {
        nRC = ERROR_GET_REG_VALUE;
    }

    /*if (dRet && (ERROR_FILE_NOT_FOUND != dRet))
    {
        ShowWinErrMsg(dRet);
    }*/

    return nRC;
}

int GetToolInfo::GetValFromRegEx(HKEY hStartKey, const char * strSubKey, const char * strKey, LPSTR & strValue)
{
    int nRC = NO_ERROR;
    // get OpenTM2 properties path
    HKEY hKey = NULL;
    DWORD dRet = RegOpenKeyEx(hStartKey, strSubKey, 0, KEY_READ, &hKey);

    if (dRet != ERROR_SUCCESS)
    {
        nRC = ERROR_GET_REG_VALUE;
        return nRC;
    }

    DWORD dwDataSize = MAX_BUF_SIZE;
    strValue = (LPTSTR) malloc (dwDataSize);
    memset(strValue, 0x00, sizeof(strValue));
    if (NULL == strValue)
    {
        nRC = ERROR_NO_MORE_SPACE;
        return nRC;
    }

    //DWORD dwResType = RRF_RT_REG_EXPAND_SZ | RRF_NOEXPAND;
    //dRet = RegGetValue(hKey, NULL, strKey, dwResType, NULL, (LPBYTE)strValue, &dwDataSize);
    dRet = RegQueryValueEx( hKey, strKey, NULL, NULL, (LPBYTE)strValue, &dwDataSize );

    //// add for type not support start
    //if (ERROR_UNSUPPORTED_TYPE == dRet)
    //{
    //    dwResType = RRF_RT_REG_SZ;
    //    dRet = RegGetValue(hKey, NULL, "PATH", dwResType, NULL, (LPBYTE)strValue, &dwDataSize);
    //}
    //// add end

    while (dRet == ERROR_MORE_DATA)
    {
        dwDataSize += MAX_BUF_SIZE;
        strValue = (LPTSTR) realloc(strValue, dwDataSize);
        memset(strValue, 0x00, sizeof(strValue));

        if (NULL == strValue)
        {
            nRC = ERROR_NO_MORE_SPACE;
            break;
        }

        //dRet = RegGetValue(hKey, NULL, strKey, dwResType, NULL, (LPBYTE)strValue, &dwDataSize);
        dRet = RegQueryValueEx( hKey, strKey, NULL, NULL, (LPBYTE)strValue, &dwDataSize );
    }

    if ((nRC != ERROR_NO_MORE_SPACE) && (nRC != ERROR_SUCCESS))
    {
        nRC = dRet;
    }

    RegCloseKey(hKey);

    return nRC;
}

BOOL CALLBACK DialogProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  lParam;

    switch (uMsg)
    {
    case WM_INITDIALOG:
        OtmCenterWindow(hWnd);
        ShowWindow(hWnd, SW_SHOW);
        break;

    case WM_COMMAND:
        if(LOWORD(wParam) == IDOK)
        {
        }
        else if(LOWORD(wParam) == IDCANCEL)
        {
            DestroyWindow(hWnd);
        }
        return TRUE;
    case WM_CLOSE:
        DestroyWindow(hWnd);
        return TRUE;
   
    case WM_DESTROY:
        PostQuitMessage(0);
        return TRUE;
    }

    return FALSE;
}

int GetToolInfo::DoInitialize()
{
    int nRC = NO_ERROR;

    m_bWow64 = OtmIsWow64();

    sprintf(m_strCurUsr, "%s\\%s\\%s", REG_KEY_CURRENT_USER_STR, REG_KEY_SOFTWARE_STR, OPENTM2_APP_NAME_STR);

    if (m_bWow64)
    {
        sprintf(m_strLocMacUni, "%s\\%s\\%s\\%s\\%s", REG_KEY_LOCAL_MACHINE_STR, REG_KEY_SOFTWARE_STR, REG_KEY_WOW_6432_NODE_STR, REG_KEY_UNINSATALL_STR, OPENTM2_APP_NAME_STR);
        sprintf(m_strLocMac, "%s\\%s\\%s\\%s", REG_KEY_LOCAL_MACHINE_STR, REG_KEY_SOFTWARE_STR, REG_KEY_WOW_6432_NODE_STR, OPENTM2_APP_NAME_STR);
        sprintf(m_strCurUsrVirUni, "%s\\%s\\%s\\%s\\%s\\%s\\%s", 
            REG_KEY_CURRENT_USER_STR, REG_KEY_SOFTWARE_STR, REG_KEY_VIRTUAL_STR, REG_KEY_SOFTWARE_STR, REG_KEY_WOW_6432_NODE_STR, REG_KEY_UNINSATALL_STR, OPENTM2_APP_NAME_STR);
        sprintf(m_strCurUsrVir, "%s\\%s\\%s\\%s\\%s\\%s", 
            REG_KEY_CURRENT_USER_STR, REG_KEY_SOFTWARE_STR, REG_KEY_VIRTUAL_STR, REG_KEY_SOFTWARE_STR, REG_KEY_WOW_6432_NODE_STR, OPENTM2_APP_NAME_STR);
    }
    else
    {
        sprintf(m_strLocMacUni, "%s\\%s\\%s\\%s", REG_KEY_LOCAL_MACHINE_STR, REG_KEY_SOFTWARE_STR, REG_KEY_UNINSATALL_STR, OPENTM2_APP_NAME_STR);
        sprintf(m_strLocMac, "%s\\%s\\%s", REG_KEY_LOCAL_MACHINE_STR, REG_KEY_SOFTWARE_STR, OPENTM2_APP_NAME_STR);
        sprintf(m_strCurUsrVirUni, "%s\\%s\\%s\\%s\\%s\\%s", 
            REG_KEY_CURRENT_USER_STR, REG_KEY_SOFTWARE_STR, REG_KEY_VIRTUAL_STR, REG_KEY_SOFTWARE_STR, REG_KEY_UNINSATALL_STR, OPENTM2_APP_NAME_STR);
        sprintf(m_strCurUsrVir, "%s\\%s\\%s\\%s\\%s\\%s", 
            REG_KEY_CURRENT_USER_STR, REG_KEY_SOFTWARE_STR, REG_KEY_VIRTUAL_STR, REG_KEY_SOFTWARE_STR, REG_KEY_WOW_6432_NODE_STR, OPENTM2_APP_NAME_STR);
    }

    nRC = GetAppVersion();
    if (nRC)
    {
        return nRC;
    }

    nRC = GetToolAppPath();
    if (nRC)
    {
        return nRC;
    }

    //HAB  hAB = NULL;

    //SetupUtils(hAB, m_strMsgFile);
    return nRC;
}

void GetToolInfo::CopyString(char * strTar, const char * strSrc)
{
    if (NULL == strTar)
    {
        return;
    }

    if ((NULL == strSrc) || (strlen(strSrc) == 0))
    {
        return;
    }

    int nLen = min(sizeof(strTar), strlen(strSrc));
    strncpy(strTar, strSrc, nLen);
}

void GetToolInfo::Join2Path(const char * strInPath1, const char * strInPath2, char * strPathOut)
{
    if ((NULL == strInPath1) || (strlen(strInPath1) == 0))
    {
        return;
    }

    if ((NULL == strInPath2) || (strlen(strInPath2) == 0))
    {
        CopyString(strPathOut, strInPath1);
        return;
    }

    int nPos = strlen(strInPath1) - 1;
    if (strInPath1[nPos] == '\\')
    {
        sprintf(strPathOut, "%s%s", strInPath1, strInPath2);
    }
    else
    {
        sprintf(strPathOut, "%s\\%s", strInPath1, strInPath2);
    }
}

void GetToolInfo::EnumQueryKey(HKEY hKey, char * strParentKey, const char * strTarKey)
{
    DWORD    nSubKeys = 0;          // number of subkeys
    DWORD    dMaxSubKey;            // longest subkey size
    DWORD    dValues;               // number of values for key
    DWORD    dMaxValue;             // longest value name
    DWORD    dMaxValueData;         // longest value data

    DWORD  dRet;

    // get the key info first
    dRet = RegQueryInfoKey(hKey, NULL,  NULL, NULL, &nSubKeys, &dMaxSubKey, NULL, &dValues,
                           &dMaxValue, &dMaxValueData, NULL, NULL);

    // Enumerate the subkeys, until RegEnumKeyEx fails.
    for (int iInx = 0; iInx < (int) nSubKeys; iInx++)
    {
        char   strSubKey[MAX_BUF_SIZE];         // buffer for subkey name
        DWORD  nSubKeyLen = MAX_BUF_SIZE;       // size of name string

        memset(strSubKey, 0x00, sizeof(strSubKey));
        dRet = RegEnumKeyEx(hKey, iInx, strSubKey, &nSubKeyLen, NULL, NULL, NULL, NULL);

        if (dRet != ERROR_SUCCESS)
        {
            continue;
        }

        char strNewParentKey[MAX_LEN_MSG];
        memset(strNewParentKey, 0x00, sizeof(strNewParentKey));
        sprintf(strNewParentKey, "%s\\%s", strParentKey, strSubKey);

        // open the subkey
        HKEY hSubkey = NULL;
        if (m_bWow64)
        {
            dRet = RegOpenKeyEx(hKey, strSubKey, 0, KEY_READ | KEY_WOW64_64KEY, &hSubkey);
        }
        else
        {
            dRet = RegOpenKeyEx(hKey, strSubKey, 0, KEY_READ | KEY_WOW64_32KEY, &hSubkey);
        }

        if (!dRet && !stricmp(strSubKey, strTarKey))
        {
            // show the parent Key
            char strShowMsg[MAX_LEN_MSG];
            memset(strShowMsg, 0x00, sizeof(strShowMsg));
            sprintf(strShowMsg, "      %s\n", strNewParentKey);
            ShowInfoMsg(strShowMsg);

            if (!stricmp(m_strLocMacUni, strNewParentKey))
            {
                m_bHasLocMacUni = TRUE;
            }
            else if (!stricmp(m_strLocMac, strNewParentKey))
            {
                m_bHasLocMac = TRUE;
            }
            else if (!stricmp(m_strCurUsr, strNewParentKey))
            {
                m_bHasCurUsr = TRUE;
            }
            else if (!stricmp(m_strCurUsrVirUni, strNewParentKey))
            {
                m_bHasCurUsrVirUni = TRUE;
            }
            else if (!stricmp(m_strCurUsrVir, strNewParentKey))
            {
                m_bHasCurUsrVir = TRUE;
            }

            // enum the values of the key
            EnumKeyValues(hSubkey);
            ShowInfoMsg(LINE_BREAK_STR);
        }

        if (!dRet)
        {
            EnumQueryKey(hSubkey, strNewParentKey, strTarKey);
        }

        RegCloseKey(hSubkey);
    }
}

void GetToolInfo::EnumKeyValues(HKEY hKey)
{
    DWORD    nSubKeys = 0;            // number of subkeys
    DWORD    dMaxSubKey;              // longest subkey size
    DWORD    dValuesCnt;              // number of values for key
    DWORD    dMaxValue;               // longest value name
    DWORD    dMaxValueData;           // longest value data

    DWORD  dRet;

    // get the key info first
    dRet = RegQueryInfoKey(hKey, NULL, NULL, NULL, &nSubKeys, &dMaxSubKey, NULL, &dValuesCnt,
                           &dMaxValue, &dMaxValueData, NULL, NULL);

    // Enumerate the key vaes.
    for (int iInx = 0, dRet = ERROR_SUCCESS; iInx < (int) dValuesCnt; iInx++)
    {
        CHAR     strEnumVal[MAX_BUF_SIZE];   // buffer for subkey name
        DWORD    nEnumLen;                   // size of name string
        DWORD    dType;
        BYTE     byteData[MAX_LEN_MSG];
        DWORD    dDataSize;

        nEnumLen = MAX_BUF_SIZE;
        dDataSize = MAX_LEN_MSG;
        memset(strEnumVal, 0x00, sizeof(strEnumVal));
        dRet = RegEnumValue(hKey, iInx, strEnumVal, &nEnumLen, NULL, &dType, byteData, &dDataSize);

        char strShowMsg[MAX_LEN_MSG];
        memset(strShowMsg, 0x00, sizeof(strShowMsg));

        if (dRet != ERROR_SUCCESS)
        {
            continue;
        }

        if ((NULL == strEnumVal) || (strlen(strEnumVal) == 0))
        {
            sprintf(strEnumVal, "%s", REG_KEY_DEFAULT_STR);
        }

        if (dType == REG_DWORD)
        {
            sprintf(strShowMsg, "      (%d) %s=0x%08x(%d)\n", iInx + 1, strEnumVal, *(DWORD *)byteData, *(DWORD *)byteData);
        }
        else if (dType == REG_SZ)
        {
            sprintf(strShowMsg, "      (%d) %s=%s\n", iInx + 1, strEnumVal, (char *)byteData);
        }
        else
        {
            sprintf(strShowMsg, "      (%d) %s=%d\n", iInx + 1, strEnumVal, (int)byteData);
        }
        ShowInfoMsg(strShowMsg);
    }
}

void GetToolInfo::ShowCmdHeader()
{
    printf(CMD_HEAD_2, GET_TOOL_INFO_NAME);
    printf(CMD_HEAD_3, m_strAppVer);
    printf(CMD_HEAD_4);
    printf(CMD_HEAD_5);
    printf(CMD_HEAD_6, GET_TOOL_INFO_NAME);
    printf(CMD_HEAD_7);
    printf(CMD_HEAD_8);
    printf(CMD_HEAD_9);
    printf(CMD_HEAD_10);
    printf(LINE_BREAK_STR);
}

void GetToolInfo::ShowRptHeader()
{
    ShowInfoMsg(RPT_HEAD_1);

    char strShowMsg[MAX_BUF_SIZE];
    memset(strShowMsg, 0x00, sizeof(strShowMsg));
    sprintf(strShowMsg, RPT_HEAD_2, GET_TOOL_INFO_NAME);
    ShowInfoMsg(strShowMsg);

    ShowInfoMsg(RPT_HEAD_3);

    memset(strShowMsg, 0x00, sizeof(strShowMsg));
    sprintf(strShowMsg, RPT_HEAD_4, GetDateTimeStr());
    ShowInfoMsg(strShowMsg);

    char strLogPath[MAX_PATH];
    memset(strLogPath, 0x00, sizeof(strLogPath));
    sprintf(strLogPath, "\\%s\\%s\\", OTM_FOLDER_KEY, LOG_FOLDER_KEY);

    memset(strShowMsg, 0x00, sizeof(strShowMsg));
    sprintf(strShowMsg, RPT_HEAD_5, strLogPath);
    ShowInfoMsg(strShowMsg);

    char strOptions[MAX_PATH];
    memset(strOptions, 0x00, sizeof(strOptions));
    memset(strShowMsg, 0x00, sizeof(strShowMsg));

    if (m_bDetails)
    {
        strcat(strOptions, OPTION_DETAILS);
    }
    sprintf(strShowMsg, RPT_HEAD_6, strOptions);
    ShowInfoMsg(strShowMsg);

    ShowInfoMsg(RPT_HEAD_1);

    ShowInfoMsg(LINE_BREAK_STR);

}

void GetToolInfo::ShowInfoMsg(const char * strMsg)
{
    if (m_bCmd)
    {
//        printf("%s", strMsg);
        AddToLog(strMsg);
    }
    else
    {
        //
    }
}

void GetToolInfo::ShowErrMsg(const char * strErrMsg)
{
    if (m_bCmd)
    {
        printf("Error: %s", strErrMsg);
    }
    else
    {
        MessageBox(HWND_DESKTOP, strErrMsg, APP_GET_TOOL_INFO_STR, MB_ICONEXCLAMATION | MB_OK);
    }
}

void GetToolInfo::ShowWinErrMsg(DWORD dRet)
{
    char strShowMsg[MAX_LEN_MSG];
    memset(strShowMsg, 0x00, sizeof(strShowMsg));

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM, 
        NULL,
        dRet,
        MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
        strShowMsg,
        0, NULL);

    ShowErrMsg(strShowMsg);
}

const char * GetToolInfo::GetDateTimeStr()
{
    SYSTEMTIME systemDateTime;
    static char strDateTime[MAX_TIMESTAMP_LEN];

    ::GetLocalTime(&systemDateTime);

    sprintf(strDateTime, "%4d-%02d-%02d, %02d:%02d:%02d", systemDateTime.wYear, systemDateTime.wMonth, 
            systemDateTime.wDay, systemDateTime.wHour, systemDateTime.wMinute, systemDateTime.wSecond);

    return strDateTime;
}

void GetToolInfo::AddToLog(const char * strMsg)
{
    FILE   *m_fLog;
    if ((m_fLog = fopen(m_strRptFile, "a")) == NULL)
    {
        return;
    }

    fputs(strMsg, m_fLog);
    fclose(m_fLog);
}

void GetToolInfo::AddComma(const char * strOri, char * strTar)
{
    if (NULL == strTar)
    {
        return;
    }

    for (int iInx = 0; iInx < (int) strlen(strOri); iInx++)
    {
        if (0 == iInx)
        {
            sprintf(strTar, "%c", strOri[iInx]);
        }
        else
        {
            sprintf(strTar, "%s,%c", strTar, strOri[iInx]);
        }
    }
}

void GetToolInfo::ShowDriveInfo(const char * strDrives)
{
    if (NULL == strDrives)
    {
        ShowInfoMsg(LINE_BREAK_STR);
        return;
    }

    for (int iInx = 0; iInx < (int) strlen(strDrives); iInx++)
    {
        char strDir[MAX_PATH];
        memset(strDir, 0x00, sizeof(strDir));
        sprintf(strDir, "%c:\\", strDrives[iInx]);

        BOOL fResult;
        unsigned _int64 i64FreeBytesToCaller;
        unsigned _int64 i64TotalBytes;
        unsigned _int64 i64FreeBytes;
        ULONG ulAccessTime = GetTickCount();
        fResult = GetDiskFreeSpaceEx(strDir,
                                     (PULARGE_INTEGER)&i64FreeBytesToCaller, 
                                     (PULARGE_INTEGER)&i64TotalBytes, 
                                     (PULARGE_INTEGER)&i64FreeBytes);
        ulAccessTime = GetTickCount() - ulAccessTime;
        if (fResult)
        {
            char strShowMsg[MAX_LEN_MSG];
            memset(strShowMsg, 0x00, sizeof(strShowMsg));
            sprintf(strShowMsg, DRIVE_SIZE_FORMAT_STR, strDrives[iInx], (float)i64TotalBytes/1000/1000, (float)i64FreeBytesToCaller/1000/1000, ulAccessTime );
            ShowInfoMsg(strShowMsg);
        }
        else
        {
            char strShowMsg[MAX_LEN_MSG];
            memset(strShowMsg, 0x00, sizeof(strShowMsg));
            sprintf(strShowMsg, DRIVE_NOSIZE_AVAIL_STR, strDrives[iInx], ulAccessTime );
            ShowInfoMsg(strShowMsg);
        }
    }
}

int GetToolInfo::ShowMemoriesInfo()
{
    int nRC = NO_ERROR;

    MEMORYSTATUSEX statex;
    statex.dwLength = sizeof(MEMORYSTATUSEX);

    char strShowMsg[MAX_LEN_MSG];
    memset(strShowMsg, 0x00, sizeof(strShowMsg));

    sprintf(strShowMsg, MEM_INFO_TITLE_STR);
    ShowInfoMsg(strShowMsg);


    if (GlobalMemoryStatusEx(&statex))
    {
        DWORD dGBSize = 1024 * 1024 * 1024;
        DWORD dKBSzie = 1024 * 1024;

        // Installed Physical Memory (RAM)
        memset(strShowMsg, 0x00, sizeof(strShowMsg));

        // disabled code below as GetPhysicallyInstalledSystemMemory is not supported under WinXP
        //if (m_dwMajVer >= 6)
        //{
        //    // if the version is vista and over
        //    PULONGLONG pTotalMemInKilo = new ULONGLONG;
        //    if (GetPhysicallyInstalledSystemMemory(pTotalMemInKilo))
        //    {
        //        sprintf(strShowMsg, INSTALLED_PHY_MEM_STR, (float)*pTotalMemInKilo/dKBSzie);
        //    }
        //    else
        //    {
        //        sprintf(strShowMsg, INSTALLED_PHY_MEM_STR, 0);
        //    }
        //    delete pTotalMemInKilo;
        //}
        //else
        //{
            sprintf(strShowMsg, INSTALLED_PHY_MEM_STR, (float) statex.ullTotalPageFile/dGBSize);
        //}
        ShowInfoMsg(strShowMsg);

        // Total Physical Memory
        memset(strShowMsg, 0x00, sizeof(strShowMsg));
        sprintf(strShowMsg, TOTAL_PHY_MEM_STR, (float) statex.ullTotalPhys/dGBSize);
        ShowInfoMsg(strShowMsg);

        // Available Physical Memory
        memset(strShowMsg, 0x00, sizeof(strShowMsg));
        sprintf(strShowMsg, AVAIL_PHY_MEM_STR, (float) statex.ullAvailPhys/dGBSize);
        ShowInfoMsg(strShowMsg);

        // Total Virtual Memory
        memset(strShowMsg, 0x00, sizeof(strShowMsg));
        sprintf(strShowMsg, TOTAL_VIR_MEM_STR, (float) statex.ullTotalPageFile/dGBSize);
        ShowInfoMsg(strShowMsg);

        // Available Virtual Memory
        memset(strShowMsg, 0x00, sizeof(strShowMsg));
        sprintf(strShowMsg, AVAIL_VIR_MEM_STR, (float) statex.ullAvailPageFile/dGBSize);
        ShowInfoMsg(strShowMsg);
    }
    else
    {
        sprintf(strShowMsg, "Failed to get the memories info.\n");
        ShowInfoMsg(strShowMsg);
    }

    return nRC;
}

GetToolInfo::GetToolInfo(PCMDPARAM pCmdParam)
{
    m_bCmd      = pCmdParam->bCmd;
    m_bDetails  = pCmdParam->bDetails;
    m_hInstance = pCmdParam->hInstance;
    m_bWow64    = FALSE;

    memset(m_strAppVer,       0x00, sizeof(m_strAppVer));
    memset(m_strAppPath,      0x00, sizeof(m_strAppPath));
    memset(m_strAppMainPath,  0x00, sizeof(m_strAppMainPath));
    memset(m_strPluginPath,   0x00, sizeof(m_strPluginPath));
    memset(m_strRptPath,      0x00, sizeof(m_strRptPath));
    memset(m_strRptFile,      0x00, sizeof(m_strRptFile));

    memset(m_strLocMacUni,    0x00, sizeof(m_strLocMacUni));
    memset(m_strLocMac,       0x00, sizeof(m_strLocMac));
    memset(m_strCurUsr,       0x00, sizeof(m_strCurUsr));
    memset(m_strCurUsrVirUni, 0x00, sizeof(m_strCurUsrVirUni));
    memset(m_strCurUsrVir,    0x00, sizeof(m_strCurUsrVir));

    m_bHasLocMacUni     = FALSE;
    m_bHasLocMac        = FALSE;
    m_bHasCurUsr        = FALSE;
    m_bHasCurUsrVirUni  = FALSE;
    m_bHasCurUsrVir     = FALSE;
}

GetToolInfo::GetToolInfo(void)
{
    memset(m_strAppVer,       0x00, sizeof(m_strAppVer));
    memset(m_strAppPath,      0x00, sizeof(m_strAppPath));
    memset(m_strAppMainPath,  0x00, sizeof(m_strAppMainPath));
    memset(m_strPluginPath,   0x00, sizeof(m_strPluginPath));
    memset(m_strRptPath,      0x00, sizeof(m_strRptPath));
    memset(m_strRptFile,      0x00, sizeof(m_strRptFile));

    memset(m_strLocMacUni,    0x00, sizeof(m_strLocMacUni));
    memset(m_strLocMac,       0x00, sizeof(m_strLocMac));
    memset(m_strCurUsr,       0x00, sizeof(m_strCurUsr));
    memset(m_strCurUsrVirUni, 0x00, sizeof(m_strCurUsrVirUni));
    memset(m_strCurUsrVir,    0x00, sizeof(m_strCurUsrVir));

    m_bHasLocMacUni     = FALSE;
    m_bHasLocMac        = FALSE;
    m_bHasCurUsr        = FALSE;
    m_bHasCurUsrVirUni  = FALSE;
    m_bHasCurUsrVir     = FALSE;

    m_dwMajVer = 0;
    m_bWow64   = FALSE;
    m_bCmd     = TRUE;
}

GetToolInfo::~GetToolInfo(void)
{
}
