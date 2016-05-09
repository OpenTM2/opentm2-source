//+----------------------------------------------------------------------------+
//|EQFSETUP.C                                                                  |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|      Copyright (C) 1990-2013, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+
//|Author:         G. Queck (QSoft)                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Description:    Setup utilities for MAT and MAT TM server.                  |
//|                These utilities are used by EQF*INST.                       |
//|                Define INSTCOM to compile for MAT TM server                 |
//+----------------------------------------------------------------------------+
//|Entry Points:                                                               |
//|   SetupMAT         - Setup MAT environment (OS2.INI+directories+properties)|
//|   SetupDemo        - Setup EQF\EXPORT path                                 |
//|   BuildPath        - Build path to MAT directories                         |
//|   InstReadSysProps - Read system property file into meory                  |
//|   InstLocateString - Locate a given string                                 |
//|   InstMatch        - Test if pszS1 starts with string pszS2                |
//|                                                                           |
//|+-- status ("H"=Header,"D"=Design,"C"=Code,"T"=Test, " "=complete,          |
//|            "Q"=Quick-and-dirty )                                           |
//+----------------------------------------------------------------------------+
//|Externals:                                                                  |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Internals:                                                                  |
//|   CreateSystemProperties     - create system properties                    |
//|   CreateFolderListProperties - create folderlist properties                |
//|   CreateImexProperties       - create document import/export properties    |
//|   CreateDictProperties       - create dictionary list properties           |
//|   CreateEditProperties       - create editor properties                    |
//|   SetupCreateDir             - create a MAT directory                      |
//|   DeletePropFile             - delete a property file                      |
//|   WritePropFile              - write properties to a file                  |
//|                                                                           |
//|+-- status ("H"=Header,"D"=Design,"C"=Code,"T"=Test, " "=complete,          |
//|            "Q"=Quick-and-dirty )                                           |
//+----------------------------------------------------------------------------+
//|To be done / known limitations / caveats:                                   |
//|                                                                            |
//+----------------------------------------------------------------------------+
#define STANDARDEDIT

/**********************************************************************/
/* Include files                                                      */
/**********************************************************************/
#define INCL_VIO
#define INCL_AVIO
#define INCL_EQF_EDITORAPI        // editor API
#define INCL_EQF_ANALYSIS         // analysis functions
#define INCL_EQF_TM               // general Transl. Memory functions
#define INCL_EQF_TMREMOTE         // remote Transl. Memory functions (U-Code)
#define INCL_EQF_DLGUTILS         // dialog utilities
#define INCL_EQF_DAM
#include <eqf.h>                  // General Translation Manager include file
#include "eqfsetup.h"                   // MAT setup functions
#include <eqfserno.h>                   // Serial number
#ifdef _WINDOWS
  #include <direct.h>
#endif
/**********************************************************************/
/* Global Variables                                                   */
/**********************************************************************/
static CHAR  chTempPath[CCHMAXPATH+1];// buffer for path names
static LONG  cyDesktop;               // desktop width in pels
static LONG  cxDesktop;               // desktop height in pels
static LONG  cyDesktopDiv20;          // desktop width / 20
static LONG  cxDesktopDiv20;          // desktop height / 20

CHAR    szLineBuf[MAX_STR_LEN];        // line buffer for CONFIG.SYS lines
PSZ     pConfLine[1000];               // table holding lines of CONFIG.SYS
BOOL    fConfChanged;                  // CONGIG.SYS-needs-update flag
CHAR    chBootDrive[4];                // boot drive (to find CONFIG.SYS)

PPROPSYSTEM InstReadSysProps( VOID );

USHORT UpdateTMProp( PSZ   pszFullFileName, CHAR  chDrive );
USHORT UpdateDictProp( PSZ   pszFullFileName, CHAR  chDrive );
USHORT UpdateDocumentProp( PSZ   pszFullFileName, CHAR  chDrive );
USHORT UpdateFolderProp( PSZ   pszFullFileName, CHAR  chDrive );

#ifdef _WINDOWS
  #define DosOpen(a,b,c,d,e,f,g,h)    UtlOpen(a,b,c,d,e,f,g,h,FALSE)
  #define DosFindFirst(a,b,c,d,e,f,g) UtlFindFirst(a,b,c,d,e,f,g,FALSE)
  #define DosFindClose( a )           UtlFindClose( a, FALSE )
  #define DosQFileInfo( a, b, c, d )  UtlQFileInfo( a, b, c, d, FALSE )
#endif
//+----------------------------------------------------------------------------+
//|Function name:     SetupMat                                                 |
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function call:     SetupMAT( HAB hab, CHAR chPrimaryDrive,                  |
//|                             PSZ pszSecondaryDrives, CHAR chLanDrive        |
//+----------------------------------------------------------------------------+
//|Description:       Create EQF environment: create required directories,     |
//|                   create property files and add entries to OS2 profile     |
//|                   OS2.INI.                                                 |
//+----------------------------------------------------------------------------+
//|Input parameter:   HAB  hab                main process anchor block handle |
//|                   CHAR chPrimaryDrive     primary MAT drive                |
//|                   PSZ  pszSecondaryDrives list of secondary MAT drives     |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       0  = function completed successfully                     |
//|                   !0 = error code of called Dos functions                  |
//+----------------------------------------------------------------------------+
//|Side effects:      Directories are created, property files are created and  |
//|                   OS2.INI is updated.                                      |
//+----------------------------------------------------------------------------+
//|Function flow:     delete old entries of MAT from OS2.INI;                  |
//|                   add MAT entries to OS2.INI;                              |
//|                   create all necessary directories on primary drive;       |
//|                   while there are secondary drives;                        |
//|                      create all necessary directories on secondary drive;  |
//|                   endif;                                                   |
//|                   delete old property files;                               |
//|                   create new property files;                               |
//+----------------------------------------------------------------------------+
__declspec(dllexport)
USHORT SetupMAT
(
  HAB  hab,                            // main process anchor block handle
  CHAR chPrimaryDrive,                 // primary MAT drive
  PSZ  pszSecondaryDrives,             // list of secondary MAT drives
  CHAR chLanDrive                      // LAN drive letter
)
{
   PCHAR   pchSecondaryDrive;          // ptr drive letter of secondary drives
   USHORT  usRC = 0;                   // function return code
   HKEY hKeySoftware = NULL;

   hab;

   /*******************************************************************/
   /* to be downward compatible                                       */
   /*******************************************************************/
   chPrimaryDrive = (char)toupper(chPrimaryDrive);
   if ( !chLanDrive )
   {
     chLanDrive = chPrimaryDrive;
   } /* endif */


   /*******************************************************************/
   /* Get size of desktop window                                      */
   /*******************************************************************/
   cxDesktop = WinQuerySysValue( HWND_DESKTOP, SV_CXSCREEN );
   cyDesktop = WinQuerySysValue( HWND_DESKTOP, SV_CYSCREEN );
   cxDesktopDiv20 = cxDesktop / 20L;
   cyDesktopDiv20 = cyDesktop / 20L;

   if ( RegOpenKeyEx( HKEY_LOCAL_MACHINE, "Software", 0, KEY_READ, &hKeySoftware ) == ERROR_SUCCESS )
   {
     HKEY hSubKey = NULL;
     int iSuccess = RegOpenKeyEx( hKeySoftware, APPL_Name, 0, KEY_ALL_ACCESS, &hSubKey );
     if ( iSuccess != ERROR_SUCCESS )
     {
       DWORD dwDisp = 0;
       iSuccess = RegCreateKeyEx( hKeySoftware, APPL_Name, 0L, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hSubKey, &dwDisp );
     } /* endif */       

     if( iSuccess == ERROR_SUCCESS )
     {
       RegSetValueEx( hSubKey, KEY_Vers, 0, REG_SZ, (LPBYTE)STR_DRIVER_LEVEL, strlen(STR_DRIVER_LEVEL )+ 1);
       sprintf( chTempPath, "%c:", chPrimaryDrive );
       RegSetValueEx( hSubKey, KEY_Drive, 0, REG_SZ, (LPBYTE)chTempPath , strlen(chTempPath )+ 1);
       sprintf( chTempPath, "%c:", chPrimaryDrive );
       RegSetValueEx( hSubKey, KEY_LanDrive, 0, REG_SZ, (LPBYTE)chTempPath , strlen(chTempPath )+ 1);
       strcpy( chTempPath, PATH );
       RegSetValueEx( hSubKey, KEY_Path, 0, REG_SZ, (LPBYTE)chTempPath , strlen(chTempPath )+ 1);
       strcpy( chTempPath, DEFAULT_SYSTEM_LANGUAGE );
       RegSetValueEx( hSubKey, KEY_SYSLANGUAGE, 0, REG_SZ, (LPBYTE)chTempPath , strlen(chTempPath )+ 1);
       sprintf( chTempPath, "%c:\\%s\\%s\\%s", chPrimaryDrive, PATH, PROPDIR, SYSTEM_PROPERTIES_NAME );
       RegSetValueEx( hSubKey, KEY_SysProp, 0, REG_SZ, (LPBYTE)chTempPath , strlen(chTempPath )+ 1);
       sprintf( chTempPath, "%c:\\%s\\%s\\%s", chPrimaryDrive, PATH, MSGDIR, MSGFILE );
       RegSetValueEx( hSubKey, KEY_MsgFile, 0, REG_SZ, (LPBYTE)chTempPath , strlen(chTempPath )+ 1);
       sprintf( chTempPath, "%c:\\%s\\%s\\%s", chPrimaryDrive, PATH, MSGDIR, HLPFILE );
       RegSetValueEx( hSubKey, KEY_HlpFile, 0, REG_SZ, (LPBYTE)chTempPath , strlen(chTempPath )+ 1);

       RegCloseKey(hSubKey);
     } /* endif */        

     RegCloseKey( hKeySoftware );
   } /* endif */     

   /****************************************************************/
   /* Create directories on primary MAT drive                      */
   /****************************************************************/

   SetupCreateDir( chPrimaryDrive, NULL, SYSTEM_PATH );
   if ( !usRC )
   {
      SetupCreateDir( chPrimaryDrive, NULL, PROPERTY_PATH );
      SetupCreateDir( chPrimaryDrive, NULL, LIST_PATH );
      SetupCreateDir( chPrimaryDrive, NULL, MEM_PATH );
      SetupCreateDir( chPrimaryDrive, NULL, MSG_PATH );
      SetupCreateDir( chPrimaryDrive, NULL, TABLE_PATH );
      SetupCreateDir( chPrimaryDrive, NULL, DIC_PATH );
      SetupCreateDir( chPrimaryDrive, NULL, PRT_PATH );
   }

   /****************************************************************/
   /* Create directories on secondary drives                       */
   /****************************************************************/
   pchSecondaryDrive = pszSecondaryDrives;
   while ( *pchSecondaryDrive != NULC )
   {
      SetupCreateDir( *pchSecondaryDrive, NULL, SYSTEM_PATH );
      SetupCreateDir( *pchSecondaryDrive, NULL, LIST_PATH );
      SetupCreateDir( *pchSecondaryDrive, NULL, MEM_PATH );
      SetupCreateDir( *pchSecondaryDrive, NULL, TABLE_PATH );
      SetupCreateDir( *pchSecondaryDrive, NULL, DIC_PATH );
      pchSecondaryDrive++;
   } /* endwhile */

   /****************************************************************/
   /* Build property files                                         */
   /****************************************************************/
   if (!UtlPropFileExist (chPrimaryDrive, SYSTEM_PROPERTIES_NAME))
   {
     CreateSystemProperties( chPrimaryDrive, pszSecondaryDrives, chLanDrive );
   }                                                /* $KIT0890 A4 */
   else
   {
     UpdateSystemProperties ( chPrimaryDrive, chLanDrive );
   } /* endif */
   if (!UtlPropFileExist (chPrimaryDrive, DEFAULT_FOLDERLIST_NAME))
   {
     CreateFolderListProperties( chPrimaryDrive );
   } /* endif */
#if defined(STANDARDEDIT)
   if (!UtlPropFileExist (chPrimaryDrive, EDITOR_PROPERTIES_NAME))
   {
     CreateEditProperties( chPrimaryDrive );
   } /* endif */
#endif
   if (!UtlPropFileExist (chPrimaryDrive, DICT_PROPERTIES_NAME))
   {
     CreateDictProperties( chPrimaryDrive );
   } /* endif */
   if (!UtlPropFileExist (chPrimaryDrive, IMEX_PROPERTIES_NAME))
   {
     CreateImexProperties( chPrimaryDrive );
   } /* endif */

   /****************************************************************/
   /* Delete dictionary cache files                                */
   /****************************************************************/
   DeleteDictCacheFiles( chPrimaryDrive, DIC_PATH );

   // update properties of sample folder
   sprintf( chTempPath, "%c:\\%s\\%s\\%s", chPrimaryDrive, PATH, PROPDIR, "SHOWM000.F00" );
   UpdateFolderProp( chTempPath, chPrimaryDrive );
   sprintf( chTempPath, "%c:\\%s\\%s\\%s", chPrimaryDrive, PATH, PROPDIR, "SHOWM000.MEM" );
   UpdateTMProp( chTempPath, chPrimaryDrive );
   sprintf( chTempPath, "%c:\\%s\\%s\\%s", chPrimaryDrive, PATH, PROPDIR, "SHOWDICT.PRO" );
   UpdateDictProp( chTempPath, chPrimaryDrive);
   sprintf( chTempPath, "%c:\\%s\\%s\\%s\\%s", chPrimaryDrive, PATH, "SHOWM000.F00", PROPDIR, "SHOWMEHT.000" );
   UpdateDocumentProp( chTempPath, chPrimaryDrive);


   return( usRC );
} /* end of function SetupMat */

//+----------------------------------------------------------------------------+
//|Function name:     CreateSystemProperties                                   |
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function call:     CreateSystemProperties( CHAR chPrimaryDrive,             |
//|                                           PSZ  pszSecondaryDrives,         |
//|                                           CHAR chLanDrive )                |
//+----------------------------------------------------------------------------+
//|Description:       Create system properties and write the data to disk.     |
//+----------------------------------------------------------------------------+
//|Input parameter:   CHAR   chPrimaryDrive       MAT primary drive            |
//|                   PSZ    pszSecondaryDrives   MAT secondary drives         |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       0  = function completed successfully                     |
//|                   !0 = error code of called Dos functions                  |
//+----------------------------------------------------------------------------+
//|Function flow:     allocate data area for properties;                       |
//|                   fill property heading area;                              |
//|                   fill property data area with default values;             |
//|                   add editor information;                                  |
//|                   if old properties exist;                                 |
//|                      copy selected data from old properties;               |
//|                   endif;                                                   |
//|                   write properties and free data area;                     |
//+----------------------------------------------------------------------------+
USHORT CreateSystemProperties
(
  CHAR chPrimaryDrive,                 // MAT primary drive
  PSZ  pszSecondaryDrives,             // MAT secondary drives
  CHAR chLanDrive                      // MAT LAN drive
)
{
    PPROPSYSTEM  pSysPropsOld = 0;     // buffer for ol system properties
    PPROPSYSTEM  pSysProps = 0;        // buffer for new system properties
    PSZ          pszEditor = 0;        // ptr for editor name processing
    PSZ          pszTemp = 0;          // general purpose pointer
    USHORT       usRC = 0;             // function return code

	chLanDrive;

    /******************************************************************/
    /* Allocate area for new system properties                        */
    /******************************************************************/
    pSysProps    = (PPROPSYSTEM) malloc( sizeof( PROPSYSTEM) );
    if ( pSysProps )
    {
      memset( pSysProps, NULC, sizeof( PROPSYSTEM) );
    }
    else
    {
      usRC = ERROR_NOT_ENOUGH_MEMORY;
    } /* endif */

    /******************************************************************/
    /* Fill in property heading area                                  */
    /******************************************************************/
    if ( !usRC )
    {
      SETPROPHEAD( pSysProps->PropHead, SYSTEM_PROPERTIES_NAME,
                   PROP_CLASS_SYSTEM );
    } /* endif */


    /******************************************************************/
    /* Fill rest of properties with default values                    */
    /******************************************************************/
    if ( !usRC )
    {
      sprintf( pSysProps->szPrimaryDrive, "%c:", chPrimaryDrive );
      sprintf( pSysProps->szLanDrive, "%c:", chPrimaryDrive );
      sprintf( pSysProps->szDriveList, "%c%s", chPrimaryDrive, pszSecondaryDrives );
      strcpy( pSysProps->szPropertyPath,    PROPDIR );
      strcpy( pSysProps->szProgramPath,     WINDIR );
      strcpy( pSysProps->szDicPath,         DICTDIR );
      strcpy( pSysProps->szMemPath,         MEMDIR );
      strcpy( pSysProps->szTablePath,       TABLEDIR );
      strcpy( pSysProps->szCtrlPath,        CTRLDIR );
      strcpy( pSysProps->szDllPath,         DLLDIR );
      strcpy( pSysProps->szListPath,        LISTDIR );
      strcpy( pSysProps->szMsgPath,         MSGDIR );
      strcpy( pSysProps->szPrtPath,         PRTDIR );
      strcpy( pSysProps->szExportPath,      EXPORTDIR );
      strcpy( pSysProps->szBackupPath,      BACKUPDIR );
      strcpy( pSysProps->szDirSourceDoc,    SOURCEDIR );
      strcpy( pSysProps->szDirSegSourceDoc, SEGSOURCEDIR );
      strcpy( pSysProps->szDirTargetDoc,    TARGETDIR );
      strcpy( pSysProps->szDirSegTargetDoc, SEGTARGETDIR );
      strcpy( pSysProps->szDirImport,       IMPORTDIR );
      strcpy( pSysProps->szDirComMem,       COMMEMDIR );
      strcpy( pSysProps->szDirComDict,      COMDICTDIR );
      strcpy( pSysProps->szDirComProp,      COMPROPDIR );
      strcpy( pSysProps->szWinPath,         WINDIR );
      sprintf( pSysProps->RestartFolderLists, "\x15%c:\\%s\\%s",
               chPrimaryDrive, PATH, DEFAULT_FOLDERLIST_NAME );
      sprintf( pSysProps->FocusObject, "%c:\\%s\\%s",
               chPrimaryDrive, PATH, DEFAULT_FOLDERLIST_NAME );
      sprintf( pSysProps->RestartMemory, "\x15%s", MEMORY_PROPERTIES_NAME );
      sprintf( pSysProps->RestartDicts, "\x15%c:\\%s\\%s",
               chPrimaryDrive, PATH, DICT_PROPERTIES_NAME );
      pSysProps->fUseIELikeListWindows = TRUE;
      strcpy( pSysProps->szPluginPath,      PLUGINDIR );
    } /* endif */

    /******************************************************************/
    /* Set intial restore size to 4/5 of desktop size and center      */
    /* window inside desktop window                                   */
    /******************************************************************/
    if ( !usRC )
    {
      pSysProps->Swp.x  = (SHORT) (cxDesktop / 5L / 2L);
      pSysProps->Swp.y  = (SHORT) (cyDesktop / 5L / 2L);
      pSysProps->Swp.cx = (SHORT) (cxDesktop * 4L / 5L);
      pSysProps->Swp.cy = (SHORT) (cyDesktop * 4L / 5L);
      pSysProps->Swp.fs = EQF_SWP_SIZE | EQF_SWP_MOVE | EQF_SWP_ACTIVATE |
                          EQF_SWP_SHOW | EQF_SWP_MAXIMIZE;
      pSysProps->SwpDef = pSysProps->Swp;
    } /* endif */

    /******************************************************************/
    /* Add editor information                                         */
    /******************************************************************/
    if ( !usRC )
    {
      pszEditor = EDITOR_PROPERTIES_NAME;
      pszTemp = strchr( pszEditor, '.' );
      strncpy( pSysProps->szDefaultEditor, pszEditor, pszTemp - pszEditor );
    } /* endif */

    /******************************************************************/
    /* Try to read old system properties                              */
    /******************************************************************/
    if ( !usRC )
    {
      pSysPropsOld = InstReadSysProps();
    } /* endif */

    /******************************************************************/
    /* Save some of the values from the old properties to the new ones*/
    /******************************************************************/
    if ( !usRC && pSysPropsOld )
    {
       strcpy( pSysProps->szServerList, pSysPropsOld->szServerList );

       free( pSysPropsOld );           // free storage used for old properties
    } /* endif */

    /******************************************************************/
    /* Write properties and free data area                            */
    /******************************************************************/
    if ( !usRC )
    {
      WritePropFile( chPrimaryDrive, SYSTEM_PROPERTIES_NAME, pSysProps,
                     sizeof( PROPSYSTEM) );
      free( pSysProps );
    } /* endif */

    return( usRC );
} /* end of function CreateSystemProperties */

/* $KIT0890 A59 */
//+----------------------------------------------------------------------------+
//|Function name:     UpdateSystemProperties                                   |
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function call:     UpdateSystemProperties (CHAR chPrimaryDrive, CHAR chLan) |
//+----------------------------------------------------------------------------+
//|Description:       Update system properties and write the data to disk.     |
//+----------------------------------------------------------------------------+
//|Input parameter:   CHAR   chPrimaryDrive       MAT primary drive            |
//|                   CHAR   chLanDrive           MAT Lan drive                |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       0  = function completed successfully                     |
//|                   !0 = error code of called Dos functions                  |
//+----------------------------------------------------------------------------+
//|Function flow:     allocate data area for properties;                       |
//|                   fill property heading area;                              |
//|                   fill property data area with default values;             |
//|                   add editor information;                                  |
//|                   if old properties exist;                                 |
//|                      copy selected data from old properties;               |
//|                   endif;                                                   |
//|                   write properties and free data area;                     |
//+----------------------------------------------------------------------------+
USHORT UpdateSystemProperties ( CHAR chPrimaryDrive, CHAR chLanDrive )
{
    PPROPSYSTEM  pSysProps = 0;        // buffer for system properties
    USHORT       usRc = 0;             // return code

	chLanDrive;

    /******************************************************************/
    /* Try to read old system properties                              */
    /******************************************************************/
    pSysProps = InstReadSysProps();

    /********************************************************************/
    /* Now fill properties with new contents for actual version of TM/2 */
    /********************************************************************/
    if ( pSysProps )
    {
      strcpy( pSysProps->szPrtPath,    PRTDIR );
      strcpy( pSysProps->szWinPath,    WINDIR );
      strcpy( pSysProps->szDirComDict, COMDICTDIR );
      sprintf( pSysProps->szLanDrive, "%c", chPrimaryDrive );
    } /* endif */

#ifdef _PTM
    /******************************************************************/
    /* Now activate the quick tour folder and set the focus onto it   */
    /******************************************************************/
    if ( pSysProps )
    {
      if ( strstr( pSysProps->RestartFolders, QUICKTOUR_FOL ) == NULL )
      {
        /* Quick Tour Folder not in Restart List, add it */
        CHAR   szQuickFol[MAX_EQF_PATH];

        sprintf( szQuickFol, "\x15%c:\\%s\\%s",
                 chPrimaryDrive, PATH, QUICKTOUR_FOL );
        strcat( pSysProps->RestartFolders, szQuickFol );
      } /* endif */

      sprintf( pSysProps->FocusObject, "%c:\\%s\\%s",
               chPrimaryDrive, PATH, QUICKTOUR_FOL );
    } /* endif */
#endif

    /******************************************************************/
    /* Write properties and free data area                            */
    /******************************************************************/
    if ( pSysProps )
    {
      WritePropFile( chPrimaryDrive, SYSTEM_PROPERTIES_NAME, pSysProps,
                     sizeof( PROPSYSTEM) );
      free( pSysProps );
    }
    else
    {
      usRc = 1;
    } /* endif */

    return ( usRc );
} /* end of function UpdateSystemProperties */

//+----------------------------------------------------------------------------+
//|Function name:     CreateFolderListProperties                               |
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function call:     CreateFolderListProperties( CHAR chPrimaryDrive )        |
//+----------------------------------------------------------------------------+
//|Description:       Create folder list properties and write the data to disk.|
//+----------------------------------------------------------------------------+
//|Input parameter:   CHAR   chPrimaryDrive       MAT primary drive            |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       0  = function completed successfully                     |
//|                   !0 = error code of called Dos functions                  |
//+----------------------------------------------------------------------------+
//|Function flow:     allocate data area for properties;                       |
//|                   fill property heading area;                              |
//|                   fill property data area with default values;             |
//|                   write properties and free data area;                     |
//+----------------------------------------------------------------------------+
USHORT CreateFolderListProperties
(
  CHAR chPrimaryDrive                  // MAT primary drive
)
{
    PPROPFOLDERLIST pPropFll;          // ptr to folderlist properties
    USHORT       usRC = 0;             // function return code

    /******************************************************************/
    /* Allocate area for new properties                               */
    /******************************************************************/
    pPropFll = (PPROPFOLDERLIST)malloc( sizeof( PROPFOLDERLIST));
    if ( pPropFll )
    {
      memset( pPropFll, NULC, sizeof( *pPropFll) );
    }
    else
    {
      usRC = ERROR_NOT_ENOUGH_MEMORY;
    } /* endif */

    /******************************************************************/
    /* Fill in property heading area                                  */
    /******************************************************************/
    if ( !usRC )
    {
      SETPROPHEAD( pPropFll->PropHead, DEFAULT_FOLDERLIST_NAME,
                   PROP_CLASS_FOLDERLIST );
    } /* endif */


    /******************************************************************/
    /* Fill properties with default values                            */
    /******************************************************************/
    if ( !usRC )
    {
      pPropFll->Swp.x =  (SHORT) (cxDesktopDiv20 * 1);
      pPropFll->Swp.y  = (SHORT) (cyDesktopDiv20 * 5);
      pPropFll->Swp.cx = (SHORT) (cxDesktopDiv20 * 7L);
      pPropFll->Swp.cy = (SHORT) (cyDesktopDiv20 * 8L);
      pPropFll->Swp.fs = EQF_SWP_SIZE | EQF_SWP_MOVE |
                         EQF_SWP_ACTIVATE | EQF_SWP_SHOW;
#ifdef _PTM
      pPropFll->Swp.fs |= EQF_SWP_MINIMIZE;
#endif
      *pPropFll->szDriveList = pPropFll->PropHead.szPath[0];
    } /* endif */

    /******************************************************************/
    /* Write properties and free data area                            */
    /******************************************************************/
    if ( !usRC )
    {
      WritePropFile( chPrimaryDrive, DEFAULT_FOLDERLIST_NAME, pPropFll,
                     sizeof( PROPFOLDERLIST) );
      free( pPropFll );
    } /* endif */

   return( usRC );

} /* end of function CreateFolderListProperties */

//+----------------------------------------------------------------------------+
//|Function name:     CreateImexProperties                                     |
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function call:     CreateImexProperties( CHAR chPrimaryDrive )              |
//+----------------------------------------------------------------------------+
//|Description:       Create import/export properties.                         |
//+----------------------------------------------------------------------------+
//|Input parameter:   CHAR   chPrimaryDrive       MAT primary drive            |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       0  = function completed successfully                     |
//|                   !0 = error code of called Dos functions                  |
//+----------------------------------------------------------------------------+
//|Function flow:     allocate data area for properties;                       |
//|                   fill property heading area;                              |
//|                   fill property data area with default values;             |
//|                   write properties and free data area;                     |
//+----------------------------------------------------------------------------+
USHORT CreateImexProperties
(
  CHAR chPrimaryDrive                  // MAT primary drive
)
{
    PPROPIMEX pPropImex;               // pointer to properties
    USHORT       usRC = 0;             // function return code


    /******************************************************************/
    /* Allocate area for new system properties                        */
    /******************************************************************/
    pPropImex = (PPROPIMEX)malloc( sizeof( PROPIMEX));
    if ( pPropImex )
    {
      memset( pPropImex, NULC, sizeof( *pPropImex));
    }
    else
    {
      usRC = ERROR_NOT_ENOUGH_MEMORY;
    } /* endif */


    /******************************************************************/
    /* Load property heading area                                     */
    /******************************************************************/
    if ( !usRC )
    {
      SETPROPHEAD( pPropImex->PropHead, IMEX_PROPERTIES_NAME, PROP_CLASS_IMEX );
    } /* endif */

    /******************************************************************/
    /* Fill properties with default values                            */
    /******************************************************************/
    if ( !usRC )
    {
      sprintf( pPropImex->szSavedDlgLoadDrive, "%c:", chPrimaryDrive );
      strcpy(  pPropImex->szSavedDlgLoadPath, "\\");
      strcpy(  pPropImex->szSavedDlgLoadPatternName, "*");
      strcpy(  pPropImex->szSavedDlgLoadPatternExt, ".SCR");
      sprintf( pPropImex->szSavedDlgFExpoTPath, "\\%s\\", TARGETDIR );
      sprintf( pPropImex->szSavedDlgFExpoSPath, "\\%s\\", SOURCEDIR );
      sprintf( pPropImex->szSavedDlgFExpoNPath, "\\%s\\", SNOMATCHDIR );
    } /* endif */

    /******************************************************************/
    /* Write properties and free data area                            */
    /******************************************************************/
    if ( !usRC )
    {
      WritePropFile( chPrimaryDrive, IMEX_PROPERTIES_NAME, pPropImex,
                     sizeof( PROPIMEX) );
      free( pPropImex );
    } /* endif */

   return( usRC );

} /* end of function CreateImexProperties */

//+----------------------------------------------------------------------------+
//|Function name:     CreateDictProperties                                     |
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function call:     CreateDictProperties( CHAR chPrimaryDrive )              |
//+----------------------------------------------------------------------------+
//|Description:       Create dictionary list properties and write the data to  |
//|                   disk.                                                    |
//+----------------------------------------------------------------------------+
//|Input parameter:   CHAR   chPrimaryDrive       MAT primary drive            |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       0  = function completed successfully                     |
//|                   !0 = error code of called Dos functions                  |
//+----------------------------------------------------------------------------+
//|Function flow:     allocate data area for properties;                       |
//|                   fill property heading area;                              |
//|                   fill property data area with default values;             |
//|                   write properties and free data area;                     |
//+----------------------------------------------------------------------------+
USHORT CreateDictProperties
(
  CHAR chPrimaryDrive                  // MAT primary drive
)
{
    PPROPDICTLIST pPropDict;           // pointer to dictionary properties
    USHORT       usRC = 0;             // function return code


    /******************************************************************/
    /* Allocate area for new system properties                        */
    /******************************************************************/
    pPropDict = (PPROPDICTLIST) malloc( sizeof( PROPDICTLIST) );
    if ( pPropDict )
    {
      memset( pPropDict, NULC, sizeof( *pPropDict));
    }
    else
    {
      usRC = ERROR_NOT_ENOUGH_MEMORY;
    } /* endif */

    /******************************************************************/
    /* Fill in property heading area                                  */
    /******************************************************************/
    if ( !usRC )
    {
      SETPROPHEAD( pPropDict->PropHead, DICT_PROPERTIES_NAME,
                   PROP_CLASS_DICTLIST );
    } /* endif */

    /******************************************************************/
    /* Fill properties with default values                            */
    /******************************************************************/
    if ( !usRC )
    {
      pPropDict->Swp.x =  (SHORT) (cxDesktopDiv20 * 8);
      pPropDict->Swp.y =  (SHORT) (cyDesktopDiv20 * 5);
      pPropDict->Swp.cx = (SHORT) (cxDesktopDiv20 * 7L);
      pPropDict->Swp.cy = (SHORT) (cyDesktopDiv20 * 8L);
      pPropDict->Swp.fs = EQF_SWP_SIZE | EQF_SWP_MOVE | EQF_SWP_ACTIVATE |
                          EQF_SWP_SHOW;
#ifdef _PTM
      pPropDict->Swp.fs |= EQF_SWP_MINIMIZE;
#endif
    } /* endif */

    /******************************************************************/
    /* Write properties and free data area                            */
    /******************************************************************/
    if ( !usRC )
    {
      WritePropFile( chPrimaryDrive, DICT_PROPERTIES_NAME, pPropDict,
                     sizeof( PROPDICTLIST ) );
      free( pPropDict );
    } /* endif */

   return( usRC );

} /* end of function CreateDictProperties */

//+----------------------------------------------------------------------------+
//|Function name:     CreateEditProperties                                     |
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function call:     CreateEditProperties( CHAR chPrimaryDrive )              |
//+----------------------------------------------------------------------------+
//|Description:       Create editor properties and write the data to disk.     |
//+----------------------------------------------------------------------------+
//|Input parameter:   CHAR   chPrimaryDrive       MAT primary drive            |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       0  = function completed successfully                     |
//|                   !0 = error code of called Dos functions                  |
//+----------------------------------------------------------------------------+
//|Function flow:     allocate data area for properties;                       |
//|                   fill property heading area;                              |
//|                   fill property data area with default values;             |
//|                   write properties and free data area;                     |
//+----------------------------------------------------------------------------+
USHORT CreateEditProperties
(
  CHAR chPrimaryDrive                  // MAT primary drive
)
{
    PPROPEDIT pPropEdit;               // ptr to editor properties
    PSTEQFGEN  pstEQFGen = NULL;       // pointer to generic editor structure
    USHORT       usRC = 0;             // function return code
    LONG       cyTitleBar,             // height of title bar
               cyMenu,                 // height of action bar
               cyHorzSlider,           // height of horizontal scroll bar
               cyBorder,               // height of horizontal border
               cyProposals,            // height of TM/dict proposals windows
               cyProposalsClient;      // height of client window of proposals
#ifndef _WINDOWS
    HVPS       hvps;                   // presentation space
    SHORT      sCx;
#endif
    SHORT      sCy;               // character cell size

    /******************************************************************/
    /* Allocate area for new system properties                        */
    /******************************************************************/
    pPropEdit = (PPROPEDIT)malloc( sizeof( PROPEDIT));
    if ( pPropEdit )
    {
      memset( pPropEdit, NULC, sizeof( *pPropEdit));
      pstEQFGen = &(pPropEdit->stEQFGen);// set pointer to generic structure
    }
    else
    {
      usRC = ERROR_NOT_ENOUGH_MEMORY;
    } /* endif */

    /******************************************************************/
    /* Fill in property heading area                                  */
    /******************************************************************/
    if ( !usRC )
    {
      SETPROPHEAD( pPropEdit->PropHead, EDITOR_PROPERTIES_NAME,
                   PROP_CLASS_EDITOR );
    } /* endif */

    /******************************************************************/
    /* Fill properties with default values                            */
    /******************************************************************/
    if ( !usRC )
    {
      cyTitleBar = WinQuerySysValue( HWND_DESKTOP, SV_CYTITLEBAR );
      cyMenu = WinQuerySysValue( HWND_DESKTOP, SV_CYMENU );
      cyHorzSlider = WinQuerySysValue( HWND_DESKTOP, SV_CYHSCROLL );
      cyBorder = WinQuerySysValue( HWND_DESKTOP, SV_CYBORDER );

#ifndef _WINDOWS
      /* get default character cell size by creating a dummy present. space */
      VioCreatePS (&hvps, 1, 1, 0, 3, (HVPS) NULL);
      VioGetDeviceCellSize (&sCy, &sCx, hvps);
      VioDestroyPS (hvps);
#else
      {
        TEXTMETRIC   txmtr;
        HDC          hdc;

        hdc = GetDC( NULL );  /* get device context from screen */
        SelectObject( hdc, GetStockObject( OEM_FIXED_FONT ) );
        GetTextMetrics( hdc, &txmtr );
        ReleaseDC( NULL, hdc );
        sCy = (SHORT)txmtr.tmHeight;
      }
#endif

      /* calculate approx. value for size of TM and dict proposals */
      cyProposals = (cyDesktop - 2L * cyTitleBar - cyMenu) / 4L;
      cyProposalsClient = cyProposals - cyTitleBar - cyHorzSlider - 2L*cyBorder;
      if ( cyProposalsClient % (LONG) sCy )
      {
        /* client area is larger than multiple of characters */
        /* size it to next value                             */
        cyProposalsClient = ((cyProposalsClient / (LONG) sCy) + 1L) * (LONG) sCy;
      } /* endif */

      /* now calculate size of complete window */
      cyProposals = cyProposalsClient + cyTitleBar + cyHorzSlider + 2L*cyBorder;

      strcpy( pPropEdit->szMFEStartProc, "STANDARD" );
      RECTL_XLEFT(pstEQFGen->rclEditorTgt)      = 0L;
      RECTL_XRIGHT(pstEQFGen->rclEditorTgt)     = cxDesktop;
      RECTL_YBOTTOM(pstEQFGen->rclEditorTgt)    = 2L * cyProposals;
#ifndef _WINDOWS
      RECTL_YTOP(pstEQFGen->rclEditorTgt)       = cyDesktop - 2L * cyTitleBar
                                           - cyMenu - 1L;
#else
      RECTL_YTOP(pstEQFGen->rclEditorTgt)       = cyDesktop;
#endif

#ifdef _WINDOWS
      pstEQFGen->flEditTgtStyle = FCF_TITLEBAR | FCF_SIZEBORDER | FCF_SYSMENU |
                                  FCF_VERTSCROLL  | FCF_HORZSCROLL;
#else
      pstEQFGen->flEditTgtStyle = FCF_TITLEBAR | FCF_SIZEBORDER | FCF_SYSMENU |
                                  FCF_VERTSCROLL  | FCF_HORZSCROLL | FCF_MENU;
#endif
      RECTL_XLEFT(pstEQFGen->rclEditorSrc)      = cxDesktop / 10L;
      RECTL_XRIGHT(pstEQFGen->rclEditorSrc)     = (cxDesktop * 9L) / 10L;
      RECTL_YBOTTOM(pstEQFGen->rclEditorSrc)    = (cyDesktop * 60L) / 100L;
      RECTL_YTOP(pstEQFGen->rclEditorSrc)       = (cyDesktop * 75L) / 100L;
      pstEQFGen->flEditSrcStyle = FCF_TITLEBAR | FCF_SIZEBORDER | FCF_SYSMENU |
                                  FCF_VERTSCROLL  | FCF_HORZSCROLL;
      RECTL_XLEFT(pstEQFGen->rclSource)         = cxDesktop / 10L;
      RECTL_XRIGHT(pstEQFGen->rclSource)        = (cxDesktop * 9L) / 10L;
      RECTL_YBOTTOM(pstEQFGen->rclSource)       = (cyDesktop * 25L) / 100L;
      RECTL_YTOP(pstEQFGen->rclSource)          = (cyDesktop * 40L) / 100L;
      pstEQFGen->flSrcStyle = FCF_TITLEBAR | FCF_SIZEBORDER | FCF_SYSMENU |
                                  FCF_VERTSCROLL  | FCF_HORZSCROLL;
      RECTL_XLEFT(pstEQFGen->rclDictionary)     = 0L;
      RECTL_XRIGHT(pstEQFGen->rclDictionary)    = cxDesktop;
      RECTL_YBOTTOM(pstEQFGen->rclDictionary)   = 0L;
      RECTL_YTOP(pstEQFGen->rclDictionary)      = cyProposals - 1L;
      pstEQFGen->flDictStyle = FCF_TITLEBAR | FCF_SIZEBORDER  | FCF_SYSMENU |
                               FCF_VERTSCROLL;
      RECTL_XLEFT(pstEQFGen->rclProposals)      = 0L;
      RECTL_XRIGHT(pstEQFGen->rclProposals)     = cxDesktop;
      RECTL_YBOTTOM(pstEQFGen->rclProposals)    = cyProposals;
      RECTL_YTOP(pstEQFGen->rclProposals)       = 2L * cyProposals - 1L;
      pstEQFGen->flPropStyle = FCF_TITLEBAR | FCF_SIZEBORDER  | FCF_SYSMENU |
                               FCF_VERTSCROLL  | FCF_HORZSCROLL;
#ifdef _WINDOWS
      pstEQFGen->flEditOtherStyle = FCF_TITLEBAR | FCF_SIZEBORDER | FCF_SYSMENU |
                                  FCF_VERTSCROLL  | FCF_HORZSCROLL;
#else
      pstEQFGen->flEditOtherStyle = FCF_TITLEBAR | FCF_SIZEBORDER | FCF_SYSMENU |
                                  FCF_VERTSCROLL  | FCF_HORZSCROLL | FCF_MENU;
#endif
    } /* endif */

    /******************************************************************/
    /* Write properties and free data area                            */
    /******************************************************************/
    if ( !usRC )
    {
      WritePropFile( chPrimaryDrive, EDITOR_PROPERTIES_NAME, pPropEdit,
                     sizeof( PROPEDIT ) );
      free( pPropEdit );
    } /* endif */

   return( usRC );
} /* end of function CreateEditProperties */
//#endif

//+----------------------------------------------------------------------------+
//|Function name:     SetupCreateDir                                           |
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function call:     SetupCreateDir( CHAR chDrive, USHORT usPathID )          |
//+----------------------------------------------------------------------------+
//|Description:       Create the requested directory on the specified drive.   |
//+----------------------------------------------------------------------------+
//|Input parameter:   CHAR   chDrive              drive where the new directory|
//|                                               should be created            |
//|                   PSZ    pFolderName          name of the folder           |
//|                   USHORT usPathID             symolic name for the path    |
//|                                               (same values as for          |
//|                                                UtlMakeEqfPath function)    |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       0  = function completed successfully                     |
//|                   !0 = error code of called Dos functions                  |
//+----------------------------------------------------------------------------+
//|Samples:           SetupCreateDir( 'C', SYSTEM_PATH );                      |
//+----------------------------------------------------------------------------+
//|Function flow:     call BuildPath to build directory name;;                 |
//|                   use DosMKDir to create directory;                        |
//|                   return return code to caller;                            |
//+----------------------------------------------------------------------------+
USHORT SetupCreateDir
(
   CHAR    chDrive,                    // drive for new directory
   PSZ     pszFolder,                  // name of the folder
   USHORT  usPathID                    // symbolic name for path
)
{
  CHAR     szPath[MAX_EQF_PATH];       // buffer for path name
  USHORT   usRC = NO_ERROR;            // function return code

  BuildPath( szPath, chDrive, pszFolder, usPathID );
  SetErrorMode( SEM_FAILCRITICALERRORS | SEM_NOOPENFILEERRORBOX );
  if ( !CreateDirectory( szPath, NULL ) )
  {
    usRC = (USHORT)GetLastError();
  } /* endif */
  SetErrorMode( 0 );
  if ( usRC <= 5 )                     // directory exists already
  {
     usRC = 0;
  } /* endif */
  return( usRC );
} /* end of function SetupCreateDir */

//+----------------------------------------------------------------------------+
//|Function name:     DeletePropFile                                           |
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function call:     DeletePropFile( CHAR chDrive, PSZ pszName )              |
//+----------------------------------------------------------------------------+
//|Description:       Delete the specified property file.                      |
//+----------------------------------------------------------------------------+
//|Input parameter:   CHAR   chDrive              drive of property file       |
//|                   PSZ    pszName              name of property file        |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       0  = function completed successfully                     |
//|                   !0 = error code of called Dos functions                  |
//+----------------------------------------------------------------------------+
//|Samples:           DeletePropFile( 'C', "EQFSYS.PRP" );                     |
//+----------------------------------------------------------------------------+
//|Function flow:     call BuildPath to build property path;                   |
//|                   add property name to path;                               |
//|                   use DosDelete to delete the property file;               |
//|                   return return code to caller;                            |
//+----------------------------------------------------------------------------+
USHORT DeletePropFile
(
   CHAR    chDrive,                    // drive of property file
   PSZ     pszName                     // name of property file
)
{
  CHAR     szPath[MAX_EQF_PATH];       // buffer for path name
  USHORT   usRC;                       // function return code

  BuildPath( szPath, chDrive, NULL, PROPERTY_PATH );
  strcat( szPath, BACKSLASH_STR );
  strcat( szPath, pszName );
  usRC = (USHORT)DosDelete( szPath, 0L );
  return( usRC );
} /* end of function DeletePropFile */

//+----------------------------------------------------------------------------+
//|Function name:     WritePropFile                                            |
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function call:     WritePropFile( CHAR chDrive, PSZ pszName, PVOID  pProp,  |
//|                                  USHORT usSize )                           |
//+----------------------------------------------------------------------------+
//|Description:       Delete the specified property file.                      |
//+----------------------------------------------------------------------------+
//|Input parameter:   CHAR   chDrive              drive of property file       |
//|                   PSZ    pszName              name of property file        |
//|                   PVOID  pProp                ptr to property data         |
//|                   USHORT usSize               size of property data        |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       0  = function completed successfully                     |
//|                   !0 = error code of called Dos functions                  |
//+----------------------------------------------------------------------------+
//|Samples:           WritePropFile( 'C', "EQFSYS.PRP", pSysProp,              |
//|                                  sizeof(*pSysProp) );                      |
//+----------------------------------------------------------------------------+
//|Function flow:     call BuildPath to build property path;                   |
//|                   add property name to path;                               |
//|                   open the property file;                                  |
//|                   write properties to disk;                                |
//|                   close property file;                                     |
//|                   return return code to caller;                            |
//+----------------------------------------------------------------------------+
USHORT WritePropFile
(
   CHAR    chDrive,                    // drive of property file
   PSZ     pszName,                    // name of property file
   PVOID   pProp,                      // ptr to property data
   USHORT  usSize                      // size of property data
)
{
  CHAR    szPath[MAX_EQF_PATH];        // buffer for path name
  USHORT  usRC = NO_ERROR;             // function return code
  FILE   *hFile = NULL;                // file handle for property files

  /********************************************************************/
  /* Build name of property file                                      */
  /********************************************************************/
  BuildPath( szPath, chDrive, NULL, PROPERTY_PATH );
  strcat( szPath, BACKSLASH_STR );
  strcat( szPath, pszName );

  /********************************************************************/
  /* Open the property file                                           */
  /********************************************************************/
  hFile = fopen( szPath, "wb" );
  if ( hFile == NULL )
  {
    usRC = ERROR_PATH_NOT_FOUND;
  } /* endif */

  /********************************************************************/
  /* Write property data to disk                                      */
  /********************************************************************/
  if ( !usRC  )
  {
    if ( fwrite( pProp, usSize, 1, hFile ) != 1 )
    {
      usRC = ERROR_WRITE_FAULT;
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* Close property file                                              */
  /********************************************************************/
  if ( hFile )
  {
    fclose( hFile);
  } /* endif */

  return( usRC );
} /* end of function WritePropFile */


//+----------------------------------------------------------------------------+
//|Function name:     BuildPath                                                |
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function call:     BuildPath( PSZ pszBuffer, CHAR chEqfDrive, PSZ pszFolder,|
//|                              USHORT usPathID )                             |
//+----------------------------------------------------------------------------+
//|Description:       Build the path to the specified MAT directory.           |
//|                   The function works simular to the function UtlMakeEQFPath|
//+----------------------------------------------------------------------------+
//|Input parameter:   CHAR   chEqfDrive           drive for the path name      |
//|                   PSZ    pszFolder            name of a folder or NULL     |
//|                   USHORT usPathID             symbolic path identifier     |
//+----------------------------------------------------------------------------+
//|Output parameter:  PSZ    pszBuffer            buffer is filled with path   |
//+----------------------------------------------------------------------------+
//|Returncode type:   VOID                                                     |
//+----------------------------------------------------------------------------+
//|Samples:           BuildPath( pszBuffer, 'C', NULL, PROPERTY_PATH )         |
//|                      will fill pszBuffer with "C:\EQF\PROPERTY"            |
//+----------------------------------------------------------------------------+
//|Function flow:     get directory name for given path ID;                    |
//|                   build path name;                                         |
//+----------------------------------------------------------------------------+
VOID BuildPath
(
  PSZ pszBuffer,                       // points to buffer for path name
  CHAR chEqfDrive,                     // drive for the path name
  PSZ pszFolder,                       // name of a folder or NULL
  USHORT usPathID                      // symbolic path identifier
)
{
   PSZ         pszSubDir;             // ptr to subdirectory info

   *pszBuffer = NULC;                 // no path info yet

   /*******************************************************************/
   /* Get pointer to name of requested subdirectory                   */
   /*******************************************************************/
   switch ( usPathID )
   {
      case PROPERTY_PATH         :
         pszSubDir = PROPDIR;
         break;
      case CTRL_PATH             :
         pszSubDir = CTRLDIR;
         break;
      case PROGRAM_PATH          :
         pszSubDir = PROGDIR;
         pszFolder = NULL;
         break;
      case DIC_PATH              :
         pszSubDir = DICTDIR;
         pszFolder = NULL;
         break;
      case MEM_PATH              :
         pszSubDir = MEMDIR;
         pszFolder = NULL;
         break;
      case TABLE_PATH            :
         pszSubDir = TABLEDIR;
         pszFolder = NULL;
         break;
      case LIST_PATH             :
         pszSubDir = LISTDIR;
         pszFolder = NULL;
         break;
      case DLL_PATH              :
         pszSubDir = DLLDIR;
         pszFolder = NULL;
         break;
      case MSG_PATH              :
         pszSubDir = MSGDIR;
         pszFolder = NULL;
         break;
      case PRT_PATH              :
         pszSubDir = PRTDIR;
         pszFolder = NULL;
         break;
      case EXPORT_PATH           :
         pszSubDir = EXPORTDIR;
         pszFolder = NULL;
         break;
      case DOCU_PATH             :
         pszSubDir = DOCUDIR;
         pszFolder = NULL;
         break;
      case SYSTEM_PATH           :
         pszSubDir = NULL;
         break;
      case COMMEM_PATH           :
         pszSubDir = COMMEMDIR;
         pszFolder = NULL;
         break;
      case COMPROP_PATH          :
         pszSubDir = COMPROPDIR;
         pszFolder = NULL;
         break;
      case COMDICT_PATH          :
         pszSubDir = COMDICTDIR;
         pszFolder = NULL;
         break;
       case  DIRSOURCEDOC_PATH:
         pszSubDir = SOURCEDIR;
         break;
       case  DIRSEGSOURCEDOC_PATH:
         pszSubDir = SEGSOURCEDIR;
         break;
       case  DIRSEGTARGETDOC_PATH:
         pszSubDir = SEGTARGETDIR;
         break;
       case  DIRTARGETDOC_PATH:
         pszSubDir = TARGETDIR;
         break;
      default:
         pszSubDir = NULL;
         pszFolder = NULL;
         break;
   } /* endswitch */

   /*******************************************************************/
   /* Build path name                                                 */
   /*******************************************************************/
   sprintf( pszBuffer, "%c:\\%s", chEqfDrive, PATH );
   if ( pszFolder )
   {
      strcat( pszBuffer, "\\" );
      strcat( pszBuffer, pszFolder );
   } /* endif */
   if ( pszSubDir )
   {
      strcat( pszBuffer, "\\" );
      strcat( pszBuffer, pszSubDir );
   } /* endif */
} /* end of function BuildPath */



//+----------------------------------------------------------------------------+
//|Function name:     InstReadSysProps                                         |
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function call:     InstReadSysProps()                                       |
//+----------------------------------------------------------------------------+
//|Description:       Try to read MAT system properties                        |
//+----------------------------------------------------------------------------+
//|Returncode type:   PPROPSYSTEM                                              |
//+----------------------------------------------------------------------------+
//|Returncodes:       NULL   = no system properties could be read              |
//|                   other  = pointer to loaded system properties             |
//+----------------------------------------------------------------------------+
//|Function flow:     get name of system properties from OS2.INI;              |
//|                   open system property file;                               |
//|                   allocate buffer and load system properties into memory;  |
//|                   close open files and, in case of errors, free allocated; |
//|                   buffers;                                                 |
//+----------------------------------------------------------------------------+
PPROPSYSTEM InstReadSysProps( VOID )
{
   BOOL             fOK = TRUE;        // Procssing status flag
   CHAR    szSysProp[MAX_EQF_PATH];    // path to system properties
   FILE             *hSysProp = NULL;  // File handle for system properties file
   PPROPSYSTEM      pSysProp = NULL;   // ptr to system properties
   LONG             lSize = 0;         // size of file

   /*******************************************************************/
   /* Get name of system properties file                              */
   /*******************************************************************/
   GetStringFromRegistry( APPL_Name, KEY_SysProp, szSysProp, sizeof(szSysProp), "" );

   /*******************************************************************/
   /* Open system property file                                       */
   /*******************************************************************/
   if ( fOK )
   {
      hSysProp = fopen( szSysProp, "rb" );
      if ( hSysProp == NULL  )
      {
        fOK = FALSE;
      } /* endif */
   } /* endif */

   /*******************************************************************/
   /* Get size of system properties                                   */
   /*******************************************************************/
   if ( fOK )
   {
      lSize = _filelength( fileno( hSysProp ) );
   } /* endif */

   /*******************************************************************/
   /* Allocate storage for system properties                          */
   /*******************************************************************/
   if ( fOK )
   {
      pSysProp = (PPROPSYSTEM) malloc( (USHORT)lSize );
      fOK = (pSysProp != NULL );
   } /* endif */

   /*******************************************************************/
   /* Load data from propery file                                     */
   /*******************************************************************/
   if ( fOK )
   {
      if ( fread( pSysProp, (USHORT)lSize, 1, hSysProp ) != 1 )
      {
        fOK = FALSE;
      } /* endif */
   } /* endif */

   /*******************************************************************/
   /* Cleanup                                                         */
   /*******************************************************************/
   if ( hSysProp )
   {
      fclose( hSysProp );
   } /* endif */

   if ( !fOK )
   {
      if ( pSysProp )
      {
         free( pSysProp );
         pSysProp = NULL;
      } /* endif */
   } /* endif */

   return( pSysProp );
} /* end of function InstReadSysProps */


//+----------------------------------------------------------------------------+
//|Function name:     InstLocateString                                         |
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function call:     InstLocateString( PSZ pszHayStack, PSZ pszNeedle )       |
//+----------------------------------------------------------------------------+
//|Description:       Locate the given string pszNeedle in string pszHayStack  |
//|                   ignoring case and blanks.             .          .       |
//+----------------------------------------------------------------------------+
//|Input parameter:   PSZ    pszHayStack  string being searched through        |
//|                   PSZ    pszNeedle    string being searched for            |
//+----------------------------------------------------------------------------+
//|Returncode type:   PSZ                                                      |
//+----------------------------------------------------------------------------+
//|Returncodes:       NULL  = string not found                                 |
//|                   other = position of pszNeedle in pszHayStack             |
//+----------------------------------------------------------------------------+
//|Function flow:     loop through haystack and use InstMatch to check for     |
//|                   needle;                                                  |
//|                   return pointer to found location or NULL if not found;   |
//+----------------------------------------------------------------------------+
PSZ InstLocateString
(
  PSZ    pszHayStack,                  // string being searched through
  PSZ    pszNeedle                     // string being searched for
)
{
   int i, j, k;                        // length and loop variables

   i = strlen(pszHayStack);
   j = strlen(pszNeedle);
   for (k = 0; k < i - j; ++k )
   {
      if ( pszHayStack[k] != ' ' )
      {
         if ( InstMatch( pszHayStack+k, pszNeedle ) != NULL )
         {
            return( pszHayStack + k );
         } /* endif */
      } /* endif */
   } /* endfor */

   return( NULL );
}

//+----------------------------------------------------------------------------+
//|Function name:     InstMatch                                                |
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function call:     InstMatch( PSZ pszS1, PSZ pszS2 )                        |
//+----------------------------------------------------------------------------+
//|Description:       Test if pszS1 starts with string pszS2 ignoring blanks   |
//|                   and case.                             .          .       |
//+----------------------------------------------------------------------------+
//|Input parameter:   PSZ    pszS1        first string                         |
//|                   PSZ    pszS2        second string                        |
//+----------------------------------------------------------------------------+
//|Returncode type:   PSZ                                                      |
//+----------------------------------------------------------------------------+
//|Returncodes:       NULL  = string not found                                 |
//|                   other = position of first character in pszS1 not         |
//|                           belonging to pszS2                               |
//+----------------------------------------------------------------------------+
//|Function flow:     compare only non-space characters and stop at end of     |
//|                   first string;                                            |
//+----------------------------------------------------------------------------+
PSZ InstMatch
(
   PSZ pszS1,
   PSZ pszS2
)
{
  CHAR  c1, c2;
  BOOL  fNotThrough = TRUE;

  while ( fNotThrough )
  {
     c1 = (CHAR) toupper( *pszS1 );
     c2 = (CHAR) toupper( *pszS2 );

     if (c1 == NULC)
     {
        pszS1 = NULL;                  // no match found
        fNotThrough = FALSE;
     }
     else if (c1 == ' ')
     {
        ++pszS1;                       // skip over blanks in string 1
     }
     else if (c2 == NULC)
     {
        fNotThrough = FALSE;           // end of string 2 ==> we're OK
     }
     else if (c1 == c2)
     {
        ++pszS1; ++pszS2;              // skip over equal characters
     }
     else
     {
        pszS1 = NULL;                  // bad match
        fNotThrough = FALSE;
     } /* endif */
  } /* endwhile */
  return( pszS1 );
} /* end of InstMatch */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     DeleteDictCacheFiles                                     |
//+----------------------------------------------------------------------------+
//|Function call:     DeleteDictCacheFiles( chPrimaryDrive, usPathId )         |
//+----------------------------------------------------------------------------+
//|Description:       delete morph. dictionary cache files                     |
//+----------------------------------------------------------------------------+
//|Input parameter:   CHAR - chPrimaryDrive : TM/2 drive                       |
//|                   USHORT - usPathId : dictionary path id                   |
//|Parameters:        _                                                        |
//+----------------------------------------------------------------------------+

VOID DeleteDictCacheFiles( CHAR chPrimaryDrive, USHORT usPathId )
{
  CHAR     szPath[MAX_EQF_PATH];       // buffer for path name
  CHAR     szCommand[255];             // buffer for commandline

  BuildPath( szPath, chPrimaryDrive, NULL, usPathId );
  sprintf (szCommand, "%s%s*.STC", szPath, BACKSLASH_STR);
  remove( szCommand );

  sprintf (szCommand, "%s%s*.NLC", szPath, BACKSLASH_STR);
  remove( szCommand );

  sprintf (szCommand, "%s%s*.P?C", szPath, BACKSLASH_STR);
  remove( szCommand );
} /* end of function DeleteDictCacheFiles */


//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function Name:     UtlPropFileExist    Check if a property file exists      |
//+----------------------------------------------------------------------------+
//|Description:       Checks if a file exists.                                 |
//+----------------------------------------------------------------------------+
//|Function Call:     BOOL UtlPropFileExist( CHAR chDrive, PSZ pszFileName )   |
//+----------------------------------------------------------------------------+
//|Parameters:        - chDrive is the primare TM/2 drive                      |
//|                   - pszFileName is the property filename                   |
//+----------------------------------------------------------------------------+
//|Returncode type:   BOOL                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:       TRUE   file exists                                       |
//|                   FALSE  file does not exist                               |
//+----------------------------------------------------------------------------+
//|Function flow:     call UtlFindFirst to check if file exists                |
//|                   close file search handle                                 |
//|                   return result                                            |
//+----------------------------------------------------------------------------+
BOOL UtlPropFileExist( CHAR chDrive, PSZ pszFile )
{
   CHAR     szPath[MAX_EQF_PATH];      // buffer for path name
#ifndef _WINDOWS
   FILEFINDBUF3 ResultBuf;              // DOS file find struct
#else
   FILEFINDBUF ResultBuf;              // DOS file find struct
#endif
   DOSVALUE usCount = 1L;
   HDIR     hDirHandle = HDIR_CREATE;  // DosFind routine handle
   USHORT   usDosRC;                   // return code of Dos... alias Utl...

   BuildPath( szPath, chDrive, NULL, PROPERTY_PATH );
   strcat( szPath, BACKSLASH_STR );
   strcat( szPath, pszFile );

#ifndef _WINDOWS
   usDosRC = DosFindFirst( szPath, &hDirHandle, FILE_NORMAL | FILE_DIRECTORY,
                           &ResultBuf, sizeof( ResultBuf),
                           &usCount, FIL_STANDARD );
#else
   usDosRC = DosFindFirst( szPath, &hDirHandle, FILE_NORMAL | FILE_DIRECTORY,
                           &ResultBuf, sizeof( ResultBuf),
                           &usCount, 0L );
#endif
   DosFindClose( hDirHandle );

   return( usDosRC == 0 );
} /* endof UtlPropFileExist */

USHORT UpdateFolderProp
(
  PSZ   pszFullFileName,
  CHAR  chDrive
)
{
  USHORT   usRC;
  USHORT   usAction;
  PPROPFOLDER pFolProp = NULL;         // ptr to folder properties
  HFILE    hFile = 0L;                 // File handle for folder properties file
  USHORT           usBytesRead;       // number of bytes read
  USHORT  usSizeWritten;               // Number of bytes written by DosWrite
  USHORT  usPropFileSize = 0;          // Size of property file
  ULONG   ulFilePos = 0L;              // position of file pointer

  chDrive = (CHAR)toupper (chDrive);                              // change driveletter UPPERCASE

   usRC = UtlOpen( pszFullFileName, &hFile, &usAction,
                   0L,                            // Create file size
                   FILE_NORMAL,
                   FILE_OPEN,
                   OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYWRITE,
                   0L, FALSE );                   // Reserved
   if ( !usRC )
   {
      /******************************************************************/
      /* Allocate area for folder  properties                           */
      /******************************************************************/
      pFolProp = (PPROPFOLDER) malloc ( sizeof( PROPFOLDER ));
      if ( pFolProp )
      {
        memset( pFolProp, NULC, sizeof( PROPFOLDER));
      }
      else
      {
        usRC = ERROR_NOT_ENOUGH_MEMORY;
      } /* endif */
   } /* endif */

   /*******************************************************************/
   /* Load data from property file                                    */
   /*******************************************************************/
   if ( !usRC )
   {
      usPropFileSize = sizeof( PROPFOLDER );
      usRC = UtlRead( hFile, pFolProp, usPropFileSize,
                      &usBytesRead, FALSE );
   }

   /*******************************************************************/
   /* copy EQF drive into properties                                  */
   /*******************************************************************/
   if ( !usRC )
   {
     pFolProp->PropHead.szPath[0] = chDrive;
     pFolProp->chDrive = chDrive;
   } /* endif */

   /********************************************************************/
   /* Write property data to disk                                      */
   /********************************************************************/
   if ( !usRC  )
     usRC = UtlChgFilePtr( hFile, 0L, FILE_BEGIN, &ulFilePos, FALSE );
   if ( !usRC  )
   {
     usRC = UtlWrite( hFile, (PVOID)pFolProp, usPropFileSize, &usSizeWritten,
                      FALSE );
     if( !usRC && (usPropFileSize != usSizeWritten) )
     {
       usRC = ERROR_WRITE_FAULT;
     } /* endif */
   } /* endif */

   /********************************************************************/
   /* Close property file                                              */
   /********************************************************************/
   if ( hFile )
   {
     UtlClose( hFile, FALSE );
   } /* endif */

   /********************************************************************/
   /* Free allocated area                                              */
   /********************************************************************/
   if ( pFolProp )
    free( pFolProp );           // free storage used for old properties

   return( usRC );
} /* end of function UpdateFolderProp  */

USHORT UpdateDocumentProp
(
  PSZ   pszFullFileName,
  CHAR  chDrive
)
{
  USHORT   usRC;
  USHORT   usAction;
  PPROPDOCUMENT pDocProp = NULL;         // ptr to dictionary properties
  HFILE    hFile = 0L;                 // File handle for folder properties file
  USHORT           usBytesRead;       // number of bytes read
  USHORT  usSizeWritten;               // Number of bytes written by DosWrite
  USHORT  usPropFileSize = 0;          // Size of property file
  ULONG   ulFilePos = 0L;              // position of file pointer

  chDrive = (CHAR)toupper (chDrive);                              // change driveletter UPPERCASE

   usRC = UtlOpen( pszFullFileName, &hFile, &usAction,
                   0L,                            // Create file size
                   FILE_NORMAL,
                   FILE_OPEN,
                   OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYWRITE,
                   0L, FALSE );                          // Reserved
   if ( !usRC )
   {
      /******************************************************************/
      /* Allocate area for document properties                          */
      /******************************************************************/
      pDocProp = (PPROPDOCUMENT) malloc ( sizeof( PROPDOCUMENT ));
      if ( pDocProp )
      {
        memset( pDocProp, NULC, sizeof( PROPDOCUMENT));
      }
      else
      {
        usRC = ERROR_NOT_ENOUGH_MEMORY;
      } /* endif */
   } /* endif */

   /*******************************************************************/
   /* Load data from property file                                    */
   /*******************************************************************/
   if ( !usRC )
   {
      usPropFileSize = sizeof( PROPDOCUMENT );
      usRC = UtlRead( hFile, pDocProp, usPropFileSize,
                      &usBytesRead, FALSE );
   }

   /*******************************************************************/
   /* copy EQF drive into properties                                  */
   /*******************************************************************/
    if ( !usRC )
      pDocProp->PropHead.szPath[0] = chDrive;

   /********************************************************************/
   /* Write property data to disk                                      */
   /********************************************************************/
   if ( !usRC  )
      usRC = UtlChgFilePtr( hFile, 0L, FILE_BEGIN, &ulFilePos, FALSE );
   if ( !usRC  )
   {
     usRC = UtlWrite( hFile, (PVOID)pDocProp, usPropFileSize, &usSizeWritten,
                      FALSE );
     if( !usRC && (usPropFileSize != usSizeWritten) )
     {
       usRC = ERROR_WRITE_FAULT;
     } /* endif */
   } /* endif */

   /********************************************************************/
   /* Close property file                                              */
   /********************************************************************/
   if ( hFile )
   {
     UtlClose( hFile, FALSE );
   } /* endif */
   /********************************************************************/
   /* Free allocated area                                              */
   /********************************************************************/
   if ( pDocProp )
    free( pDocProp );           // free storage used for old properties

   return( usRC );
} /* end of function UpdateDocumentProp  */

USHORT UpdateDictProp
(
  PSZ   pszFullFileName,
  CHAR  chDrive
)
{
  USHORT   usRC;
  USHORT   usAction;
  PPROPDICTIONARY  pDictProp = NULL;   // ptr to dictionary properties
  HFILE    hFile = 0L;                 // File handle for folder properties file
  USHORT           usBytesRead;       // number of bytes read
  USHORT  usSizeWritten;               // Number of bytes written by DosWrite
  USHORT  usPropFileSize = 0;          // Size of property file
  ULONG   ulFilePos = 0L;              // position of file pointer

  chDrive = (CHAR)toupper (chDrive);                              // change driveletter UPPERCASE

   usRC = UtlOpen( pszFullFileName, &hFile, &usAction,
                   0L,                            // Create file size
                   FILE_NORMAL,
                   FILE_OPEN,
                   OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYWRITE,
                   0L, FALSE );                          // Reserved
   if ( !usRC )
   {
      /******************************************************************/
      /* Allocate area for dictionary properties                        */
      /******************************************************************/
      pDictProp = (PPROPDICTIONARY) malloc ( sizeof( PROPDICTIONARY ));
      if ( pDictProp)
      {
        memset( pDictProp, NULC, sizeof( PROPDICTIONARY));
      }
      else
      {
        usRC = ERROR_NOT_ENOUGH_MEMORY;
      } /* endif */
   } /* endif */

   /*******************************************************************/
   /* Load data from property file                                    */
   /*******************************************************************/
   if ( !usRC )
   {
      usPropFileSize = sizeof( PROPDICTIONARY );
      usRC = UtlRead( hFile, pDictProp, usPropFileSize,
                      &usBytesRead, FALSE );
   }

   /*******************************************************************/
   /* copy EQF drive into properties                                  */
   /*******************************************************************/
    if ( !usRC )
    {
      pDictProp->szDictPath[0] = chDrive;
      pDictProp->szIndexPath[0] = chDrive;
      pDictProp->PropHead.szPath[0] = chDrive;
    }
   /********************************************************************/
   /* Write property data to disk                                      */
   /********************************************************************/
   if ( !usRC  )
     usRC = UtlChgFilePtr( hFile, 0L, FILE_BEGIN, &ulFilePos, FALSE );
   if ( !usRC  )
   {
     usRC = UtlWrite( hFile, (PVOID)pDictProp, usPropFileSize,
                      &usSizeWritten, FALSE);
     if( !usRC && (usPropFileSize != usSizeWritten) )
     {
       usRC = ERROR_WRITE_FAULT;
     } /* endif */
   } /* endif */

   /********************************************************************/
   /* Close property file                                              */
   /********************************************************************/
   if ( hFile )
   {
     UtlClose( hFile, FALSE );
   } /* endif */

   /********************************************************************/
   /* Free allocated area                                              */
   /********************************************************************/
   if ( pDictProp )
    free( pDictProp);           // free storage used for properties

   return( usRC );
} /* end of function UpdateDictProp  */

USHORT UpdateTMProp
(
  PSZ   pszFullFileName,
  CHAR  chDrive
)
{
  USHORT   usRC;
  USHORT   usAction;
  PPROPTRANSLMEM   pTMProp = NULL;     // ptr to folder properties
  PPROP_NTM pNTMProp = NULL;
  HFILE    hFile = 0L;                 // File handle for folder properties file
  USHORT           usBytesRead;        // number of bytes read
  USHORT  usSizeWritten;               // Number of bytes written by DosWrite
  USHORT  usPropFileSize = 0;          // Size of property file
  ULONG   ulFilePos = 0L;              // position of file pointer

  chDrive = (CHAR)toupper (chDrive);                              // change driveletter UPPERCASE

   usRC = UtlOpen( pszFullFileName, &hFile, &usAction,
                   0L,                            // Create file size
                   FILE_NORMAL,
                   FILE_OPEN,
                   OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYWRITE,
                   0L, FALSE );                          // Reserved
   if ( !usRC )
   {
      /******************************************************************/
      /* Allocate area for TM properties                                */
      /******************************************************************/
      pTMProp = (PPROPTRANSLMEM) malloc ( sizeof( PROPTRANSLMEM  ));
      if ( pTMProp)
      {
        memset( pTMProp, NULC, sizeof( PROPTRANSLMEM));
      }
      else
      {
        usRC = ERROR_NOT_ENOUGH_MEMORY;
      } /* endif */
   } /* endif */

   /*******************************************************************/
   /* Load data from property file                                    */
   /*******************************************************************/
   if ( !usRC )
   {
      usPropFileSize = sizeof( PROPTRANSLMEM );
      usRC = UtlRead( hFile, pTMProp, usPropFileSize,
                      &usBytesRead, FALSE );
   }

   /*******************************************************************/
   /* copy EQF drive into properties                                  */
   /*******************************************************************/
    if ( !usRC )
    {
      /* check whether this is a new TM property file */
      pNTMProp = (PPROP_NTM) pTMProp;
      if ( strcmp( pNTMProp->szNTMMarker, NTM_MARKER) == 0 )
      {
        pNTMProp->stPropHead.szPath[0] = chDrive;
        pNTMProp->szFullMemName[0] = chDrive;
      }
      else
      {
        pTMProp->stPropHead.szPath[0] = chDrive;
        pTMProp->szPath[0]            = chDrive;
        pTMProp->szMemPath[0]         = chDrive;
        pTMProp->szFullMemName[0]     = chDrive;
        pTMProp->szXTagsListFile[0]   = chDrive;
        pTMProp->szXWordsListFile[0]  = chDrive;
        pTMProp->szLanguageFile[0]    = chDrive;
        pTMProp->szFullFtabPath[0]    = chDrive;
      } /* endif */
   }

   /********************************************************************/
   /* Write property data to disk                                      */
   /********************************************************************/
   if ( !usRC  )
     usRC = UtlChgFilePtr( hFile, 0L, FILE_BEGIN, &ulFilePos, FALSE );
   if ( !usRC  )
   {
     usRC = UtlWrite( hFile, (PVOID)pTMProp, usPropFileSize , &usSizeWritten,
                      FALSE );
     if( !usRC && (usPropFileSize != usSizeWritten) )
     {
       usRC = ERROR_WRITE_FAULT;
     } /* endif */
   }  /* endif */

   /********************************************************************/
   /* Close property file                                              */
   /********************************************************************/
   if ( hFile )
   {
     UtlClose( hFile, FALSE );
   } /* endif */

   /********************************************************************/
   /* Free allocated area                                              */
   /********************************************************************/
   if ( pTMProp )
    free( pTMProp );           // free storage used for properties

   return( usRC );
} /* end of function UpdateTMProp  */
