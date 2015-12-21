//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|      Copyright (C) 1990-2012, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+
#include "eqf.h"                  // General Translation Manager include file
#include "Utility.h"

//------------------------------------------------------------------------------
// Function Name:     UtlQueryULong          Query a MAT ulong value            
//------------------------------------------------------------------------------
// Description:       Return the value of a MAT ULONG. MAT ULONGs are values    
//                    which are of general interest within MAT                  
//                    (e.g. sizes of default font, anchor block handle)         
//------------------------------------------------------------------------------
// Function Call:     UtlQueryULong( SHORT sid );                               
//------------------------------------------------------------------------------
ULONG UtlQueryULong( SHORT sID )
{
   if ( (sID > QL_FIRST) && (sID < QL_LAST) )
   {
      return( UtiVar[UtlGetTask()].ulQueryArea[sID] );
   }
   else
   {
      return( 0L );
   } /* endif */
}


//------------------------------------------------------------------------------
// Function Name:     UtlQueryUShort         Query a MAT ushort value           
//------------------------------------------------------------------------------
// Description:       Return the value of a MAT ushort.                         
//------------------------------------------------------------------------------
// Function Call:     UtlQueryUShort( SHORT sid );                              
//------------------------------------------------------------------------------
USHORT UtlQueryUShort( SHORT sID )
{
   if ( (sID > QS_FIRST) && (sID < QS_LAST) )
   {
      return( UtiVar[UtlGetTask()].usQueryArea[sID] );
   }
   else
   {
      return( 0 );
   } /* endif */
}

//------------------------------------------------------------------------------
// Function Name:     UtlQueryString         Query a MAT string value           
//------------------------------------------------------------------------------
// Description:       Return the value of a MAT string. MAT strings are strings 
//                    from the MAT profile entries or from the system property  
//                    structure.                                                
//                    Using UtlQueryString is a more convenient way to access   
//                    these string without having to deal with the system       
//                    property structure.                                       
//------------------------------------------------------------------------------
// Function Call:     UtlQueryString( SHORT sid, PSZ pBuffer, USHORT usLength); 
//------------------------------------------------------------------------------
BOOL UtlQueryString( SHORT sID, PSZ pszBuffer, USHORT usBufLength )
{
   USHORT usLength;                    // actual string length

   if ( (sID > QST_FIRST) && (sID < QST_LAST) )
   {
      if ( UtiVar[UtlGetTask()].pszQueryArea[sID] )
      {
         usLength = (USHORT)min( strlen( UtiVar[UtlGetTask()].pszQueryArea[sID]) + 1, usBufLength );
         strncpy( pszBuffer, UtiVar[UtlGetTask()].pszQueryArea[sID], usLength );
         pszBuffer[usLength-1] = NULC;
      }
      else
      {
         pszBuffer[0] = NULC;
      } /* endif */
      return( TRUE );
   }
   else
   {
      return( FALSE );
   } /* endif */
}

//------------------------------------------------------------------------------
// Function Name:     UtlSetULong            Set a MAT ulong value              
//------------------------------------------------------------------------------
// Description:       Sets a MAT ulong value.                                   
//                    This procedure is NOT a public procedure!                 
//------------------------------------------------------------------------------
// Function Call:     VOID UtlSetULong( SHORT sID, ULONG  ulValue );           
// Parameters:        sID is the ulong ID (QL_...)                              
//                    ulValue is the value for the ulong                        
//------------------------------------------------------------------------------
VOID UtlSetULong( SHORT sID, ULONG ulValue )
{
   if ( (sID > QL_FIRST) && (sID < QL_LAST) )
   {
      UtiVar[UtlGetTask()].ulQueryArea[sID] = ulValue;
      // special handling if general resource module handle is set:
      // set global vriable hResMod accordingly
//      if ( sID == QL_HRESMOD )
//      {
//        hResMod = (HMODULE)ulValue;
//      } /* endif */
   } /* endif */
}

//------------------------------------------------------------------------------
// Function Name:     UtlSetUShort          Set a MAT ushort value              
//------------------------------------------------------------------------------
// Description:       Sets a MAT ushort value.                                  
//                    This procedure is NOT a public procedure!                 
//------------------------------------------------------------------------------
// Function Call:     VOID UtlSetUShort( SHORT sID, USHORT usValue );           
//------------------------------------------------------------------------------
// Parameters:        sID is the ushort ID (QS_...)                             
//                    usValue is the value for the ushort                       
//------------------------------------------------------------------------------
VOID UtlSetUShort( SHORT sID, USHORT usValue )
{
   if ( (sID > QS_FIRST) && (sID < QS_LAST) )
   {
      UtiVar[UtlGetTask()].usQueryArea[sID] = usValue;
   } /* endif */
}

//------------------------------------------------------------------------------
// Function Name:     UtlSetString          Set a MAT string value              
//------------------------------------------------------------------------------
// Description:       Sets a MAT string value.                                  
//                    This procedure is NOT a public procedure!                 
//------------------------------------------------------------------------------
// Function Call:     VOID UtlSetString( SHORT sID, PSZ pszString );            
//------------------------------------------------------------------------------
// Parameters:        sID is the string ID (QST_...)                            
//                    pszString is the value for the string                     
//------------------------------------------------------------------------------
VOID UtlSetString( SHORT sID, PSZ pszString )
{
  PSZ  pszBuffer;                      // buffer for new string value

   if ( (sID > QST_FIRST) && (sID < QST_LAST) )
   {
     if ( UtlAlloc( (PVOID *)&pszBuffer, 0L,
                    (LONG)  max( MIN_ALLOC, strlen(pszString)+1 ),
                    ERROR_STORAGE ) )
     {
       strcpy( pszBuffer, pszString );
       UtiVar[UtlGetTask()].pszQueryArea[sID] = pszBuffer;
     } /* endif */
   } /* endif */
}

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function Name:     UtlMakeEQFPath         Make path to MAT directories      |
//+----------------------------------------------------------------------------+
//|Description:       Make complete path for EQF directories                   |
//+----------------------------------------------------------------------------+
//|Parameters:        1. PSZ    - buffer for target path                       |
//|                   2. CHAR   - drive letter ( NULC = EQF system drive )     |
//|                   3. USHORT - id of path to make                           |
//|                      SYSTEM_PATH            path to EQF root directory or  |
//|                                             to folder root if used with    |
//|                                             folder name                    |
//|                      PROPERTY_PATH          path to properties             |
//|                      PROGRAM_PATH           path to program files          |
//|                      DIC_PATH               path to dictionary data bases  |
//|                      MEM_PATH               path to memory data bases      |
//|                      TABLE_PATH             path to translation tables     |
//|                      LIST_PATH              path to lists                  |
//|                      CTRL_PATH              path to control files (profiles)
//|                      DIRSOURCEDOC_PATH      path to source documents       |
//|                      DIRSEGSOURCEDOC_PATH   path to segmented source docs  |
//|                      DIRSEGTARGETDOC_PATH   path to segmented target docs  |
//|                      DIRTARGETDOC_PATH      path to target documents       |
//|                      DLL_PATH               path to DLLs                   |
//|                      MSG_PATH               path to message and help files |
//|                      EXPORT_PATH            path to exported files         |
//|                      IMPORT_PATH            path to exported files         |
//|                      BACKUP_PATH            path to backup directory       |
//|                      COMMEM_PATH            path to server TMs             |
//|                      COMPROP_PATH           path to server TM properties   |
//|                      DIRSEGNOMATCH_PATH     path to segmented not found    |
//|                                                                    matches |
//|                   4. PSZ    - folder name (used for DIRSOURCEDOC_PATH,     |
//|                               DIRSEGSOURCEDOC_PATH, DIRSEGTARGETDOC_PATH,  |
//|                               DIRTARGETDOC_PATH paths only) or NULL        |
//+----------------------------------------------------------------------------+
//|Returncode type:   PSZ                                                      |
//+----------------------------------------------------------------------------+
//|Returncodes:      ptr to target path                                        |
//+----------------------------------------------------------------------------+
//|Samples:           UtlMakeEQFPath( chTemp, NULC, MEM_PATH, NULL )           |
//|                      would store "C:\EQF\MEM" in chTemp (assuming C is the |
//|                      EQF system drive)                                     |
//|                   UtlMakeEQFPath( chTemp, 'X', TABLE_PATH, NULL )          |
//|                      would store "X:\EQF\TABLE" in chTemp                  |
//|                   UtlMakeEQFPath( chTemp, 'D', DIRTARGETDOC_PATH,          |
//|                                   "HUGO.F00" )                             |
//|                      would store "D:\EQF\HUGO.F00\TARGET" in chTemp        |
//+----------------------------------------------------------------------------+
//|Function flow:     if no drive is given then                                |
//|                     use primary EQF drive                                  |
//|                   endif                                                    |
//|                   get QST_ value for requested path ID                     |
//|                   add drive letter to output path                          |
//|                   add system directory to output path                      |
//|                   if folder diretory specified then                        |
//|                     add folder directory to output path                    |
//|                   endif                                                    |
//|                   if subdirectoy required then                             |
//|                     get subdirectory using UtlQueryString and add it to    |
//|                      output path                                           |
//|                   end                                                      |
//|                   return address of output path                            |
//+----------------------------------------------------------------------------+
PSZ UtlMakeEQFPath
(
   PSZ    pszPath,                     // ptr to buffer for path name
   CHAR   chDrive,                     // drive letter or NULC
   USHORT usPathID,                    // ID of path
   PSZ    pszFolder                    // folder name or NULL
)
{
    USHORT      usQSTSubDir;           // QST_ value for subdirectory
    CHAR        szDrive[4];            // buffer for drive letter string
    CHAR        szLanDrive[4];         // buffer for LAN drive letter string

    *pszPath = NULC;                   // no path info yet

    /******************************************************************/
    /* Build drive string                                             */
    /******************************************************************/
    UtlQueryString( QST_LANDRIVE, szLanDrive, sizeof(szLanDrive) );
    UtlQueryString( QST_PRIMARYDRIVE, szDrive, sizeof(szDrive) );
    if (!szLanDrive[0])
    {
      strcpy(szLanDrive, szDrive);
    }
    if ( chDrive )
    {
       szDrive[0] = chDrive;
    } /* endif */

    /******************************************************************/
    /* Get query string ID and folder subdirectory for requested path */
    /******************************************************************/
    switch ( usPathID )
    {
       case PROPERTY_PATH         :
          usQSTSubDir = QST_PROPDIR;
          break;
       case CTRL_PATH             :
          usQSTSubDir = QST_CONTROLDIR;
          break;
       case PROGRAM_PATH          :
          usQSTSubDir = QST_PROGRAMDIR;
          pszFolder = NULL;
          szDrive[0] = szLanDrive[0];
          break;
       case DIC_PATH              :
          usQSTSubDir = QST_DICDIR;
          pszFolder = NULL;
          break;
       case POE_PATH              :
          usQSTSubDir = QST_DICDIR;
          pszFolder = NULL;
          szDrive[0] = szLanDrive[0];
          break;
       case MEM_PATH              :
          usQSTSubDir = QST_MEMDIR;
          pszFolder = NULL;
          break;
       case TABLE_PATH            :
          usQSTSubDir = QST_TABLEDIR;
          pszFolder = NULL;
          szDrive[0] = szLanDrive[0];
          break;
       case TABLEIMPORT_PATH            :
          usQSTSubDir = QST_TABLEDIR;
          pszFolder = NULL;
          break;
       case LIST_PATH             :
          usQSTSubDir = QST_LISTDIR;
          pszFolder = NULL;
          break;
       case DIRSOURCEDOC_PATH     :
          usQSTSubDir = QST_SOURCEDIR;
          break;
       case DIRSEGSOURCEDOC_PATH  :
          usQSTSubDir = QST_SEGSOURCEDIR;
          break;
       case DIRSEGTARGETDOC_PATH  :
          usQSTSubDir = QST_SEGTARGETDIR;
          break;
       case DIRTARGETDOC_PATH     :
          usQSTSubDir = QST_TARGETDIR;
          break;
       case DLL_PATH              :
          usQSTSubDir = QST_DLLDIR;
          pszFolder = NULL;
          szDrive[0] = szLanDrive[0];
          break;
       case DLLIMPORT_PATH              :
          usQSTSubDir = QST_DLLDIR;
          pszFolder = NULL;
          break;
       case MSG_PATH              :
          usQSTSubDir = QST_MSGDIR;
          pszFolder = NULL;
          szDrive[0] = szLanDrive[0];
          break;
       case EXPORT_PATH              :
          usQSTSubDir = QST_EXPORTDIR;
          pszFolder = NULL;
          break;
       case BACKUP_PATH              :
          usQSTSubDir = QST_BACKUPDIR;
          pszFolder = NULL;
          break;
       case IMPORT_PATH           :
          usQSTSubDir = QST_IMPORTDIR;
          break;
       case SYSTEM_PATH:
          // set to QST_LAST because nothing else will be concatenated
          usQSTSubDir = QST_LAST;
          break;
       case COMMEM_PATH              :
          usQSTSubDir = QST_COMMEMDIR;
          pszFolder = NULL;
          break;
       case COMPROP_PATH              :
          usQSTSubDir = QST_COMPROPDIR;
          pszFolder = NULL;
          break;
       case DIRSEGMT_PATH        :
          usQSTSubDir = QST_DIRSEGMTDIR;
          break;
       case DIRSEGRTF_PATH        :
          usQSTSubDir = QST_DIRSEGRTFDIR;
          break;
       case ENTITY_PATH        :
          usQSTSubDir = QST_ENTITY;
          break;
       case DIRSEGNOMATCH_PATH        :
          usQSTSubDir = QST_DIRSEGNOMATCHDIR;
          break;
       case COMDICT_PATH              :
          usQSTSubDir = QST_COMDICTDIR;
          pszFolder = NULL;
          break;
       case PRT_PATH              :
          usQSTSubDir = QST_PRTPATH;
          pszFolder = NULL;
          szDrive[0] = szLanDrive[0];
          break;
       case WIN_PATH              :
          usQSTSubDir = QST_WINPATH;
          pszFolder = NULL;
          szDrive[0] = szLanDrive[0];
          break;
       case WINIMPORT_PATH        :
          usQSTSubDir = QST_WINPATH;
          pszFolder = NULL;
          break;
       case EADATA_PATH              :
          usQSTSubDir = QST_EADATAPATH;
          break;
       case TQMPROJ_PATH              :
          usQSTSubDir = QST_TQMPROJECTPATH;
          break;
       case TQMEVAL_PATH              :
          usQSTSubDir = QST_TQMEVALUATIONPATH;
          break;
       case TQMARCHIVE_PATH              :
          usQSTSubDir = QST_TQMARCHIVEPATH;
          break;
       case TQMREPORT_PATH              :
          usQSTSubDir = QST_TQMREPORTPATH;
          break;
       case TQMVENDOR_PATH              :
          usQSTSubDir = QST_TQMVENDORPATH;
          break;
       case MTLOG_PATH              :
          usQSTSubDir = QST_MTLOGPATH;
          break;
       case MISC_PATH              :
          usQSTSubDir = QST_MISCPATH;
          break;
       case LOG_PATH              :
          usQSTSubDir = QST_LOGPATH;
          break;
       case XLIFF_PATH              :
          usQSTSubDir = QST_XLIFFPATH;
          break;
       case METADATA_PATH              :
          usQSTSubDir = QST_METADATAPATH;
          break;
       case JAVA_PATH              :
          usQSTSubDir = QST_JAVAPATH;
          break;
       case PLUGIN_PATH:
          usQSTSubDir = QST_PLUGINPATH;
          break;
       default:
          usQSTSubDir = QST_LAST;
          pszFolder = NULL;
          break;
    } /* endswitch */

    /******************************************************************/
    /* Build path up to system directory                              */
    /******************************************************************/
    sprintf( pszPath, "%c:\\", szDrive[0] );
    UtlQueryString( QST_SYSTEMDIR, pszPath + strlen(pszPath), MAX_EQF_PATH );

    /******************************************************************/
    /* Add folder directory if appropriate and specified              */
    /******************************************************************/
    if ( pszFolder )
    {
       strcat( pszPath, "\\" );
       strcat( pszPath, pszFolder );
    } /* endif */

    /******************************************************************/
    /* Add subdirectory if any                                        */
    /******************************************************************/
    if ( usQSTSubDir != QST_LAST )
    {
       strcat( pszPath, "\\" );
       UtlQueryString( usQSTSubDir, pszPath + strlen(pszPath), MAX_EQF_PATH );
    } /* endif */

    /******************************************************************/
    /* Return address of path                                         */
    /******************************************************************/
    return( pszPath);

} /* end of UtlMakeEQFPath */

