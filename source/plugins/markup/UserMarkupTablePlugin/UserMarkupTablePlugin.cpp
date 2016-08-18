/*! \file
  \author  David Walters 
  \version 1.0

  \section COPYRIGHT Copyright Notice

  Copyright (C) 1990-2016, International Business Machines
  Corporation and others. All rights reserved

  \section DESCRIPTION
 
  This is the implementation of the markup table plugin for 
  the User markup tables

*/

#ifdef _DEBUG
  // activate define to enable markup table plugin logging
  //#define MARKUPTABLE_LOGGING
#endif


#include "windows.h"
#include "core\pluginmanager\PluginManager.h"
#include "core\pluginmanager\OtmMarkupPlugin.h"
#include "UserMarkupTablePlugin.h"

#include "eqftag.h"             // tag table functions

// required to retrieve the DLL path
EXTERN_C IMAGE_DOS_HEADER __ImageBase;


static  char *szXml_Deletable             = "deletable" ;
static  char *szXml_Exportable            = "exportable" ;
static  char *szXml_Files                 = "files" ;
static  char *szXml_Importable            = "importable" ;
static  char *szXml_LongDescription       = "longDescription" ;
static  char *szXml_Markup                = "markup" ;
static  char *szXml_Markups               = "markups" ;
static  char *szXml_Name                  = "name" ;
static  char *szXml_NO                    = "NO" ;
static  char *szXml_NONE                  = "NONE" ;
static  char *szXml_none                  = "none" ;
static  char *szXml_Plugin                = "plugin" ;
static  char *szXml_Protected             = "protected" ;
static  char *szXml_ShortDescription      = "shortDescription" ;
static  char *szXml_ShortName             = "shortName" ;
static  char *szXml_Supplier              = "supplier" ;
static  char *szXml_Table                 = "table" ;
static  char *szXml_TableDirectory        = "tableDirectory" ;
static  char *szXml_UserExit              = "userExit" ;
static  char *szXml_UserExitDirectory     = "userExitDirectory" ;
static  char *szXml_Version               = "version" ;
static  char *szXml_YES                   = "YES" ;

static  char *szXml_NewRecord             = "  <%s>%s</%s>\n" ;
static  char *szContentEmpty = "";

#define  MAX_DEFAULTCONTROL_LINES 22
static char *szDefaultControl[MAX_DEFAULTCONTROL_LINES] = {
               "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n",
               " <!--\n", 
               "	Copyright Notice:\n",
               "	Copyright (C) 2014, International Business Machines\n",
               "	Corporation and others. All rights reserved\n",
               " -->\n",
               "<markups>\n",
               "  <plugin>\n",
               "    <name>UserMarkupTablePlugin</name>\n",
               "	<shortName>User</shortName>\n",
               "	<shortDescription>User Markup Table Plugin</shortDescription>\n",
               "	<longDescription>This is the markup table plugin for the user's markup tables</longDescription>\n",
               "	<version>1.0</version>\n",
               "	<supplier>User</supplier>\n",
               "    <tableDirectory>TABLE</tableDirectory>\n",
               "    <userExitDirectory>BIN</userExitDirectory>\n",
               "    <deletable>yes</deletable>\n",
               "    <exportable>yes</exportable>\n",
               "    <importable>yes</importable>\n",
               "    <protected>yes</protected>\n",
               "  </plugin>\n",
               "</markups>\n"
           } ;


static  char *PluginLogFile               = "UserMarkupTablePlugin" ;
static  char *PluginControlFile           = "UserMarkupTablePlugin.xml" ;

void      GetControlNode( char *, char *, char * ) ;
void      SaveValue( char **, char * ) ;
BOOL      CheckFilesExist( char *, char *, char * ) ;
int       CopyMarkupFiles( char *, char *, char *, char *, char *, BOOL ) ;
int       CopyMarkupFile( char *, char *, BOOL ) ;
BOOL      UpdateXMLControlFile( char *, char *, char *, char *, char *, char *, char *, char *, BOOL );
void      PerformPendingUpdates( char * ) ;
void      ConvertAnsi2UTF8( char *, char *, int ) ;


// number of markups
#define MAX_NUM_MARKUPS  300


#define COPY_MARKUP_NOCHANGE     0
#define COPY_MARKUP_COPIED       1
#define COPY_MARKUP_INUSE        2
#define COPY_MARKUP_ERROR        3


MARKUPINFO  *ptrMarkupInfo ;

OtmMarkup *MarkupObjectList[MAX_NUM_MARKUPS];



/* -------------------------------------------------------------- */
/*   constructor.                                                 */
/* -------------------------------------------------------------- */
UserMarkupTablePlugin::UserMarkupTablePlugin()
{
    FILE      *fControl ;
    char      szTemp[512] ;
    char      szLine[512] ;
    char      szNode[512] ;
    char      szContent[512] ;
    char      szErrMsg[MAX_FILELIST] ;
    int       i ;
    BOOL      bLogOpen = FALSE ;

  pszFileList = NULL;
  pszLongDescription = NULL;
  pszName=NULL;
  pszShortDescription = NULL;
  pszShortName=NULL;
  pszSupplier = NULL;
  pszTableDirectory = NULL;
  pszTablePath = NULL;
  pszUserExitDirectory = NULL;
  pszUserExitPath = NULL;
  pszVersion = NULL;

  bDeletable = TRUE ;
  bExpired = FALSE ;
  bExportable = TRUE ;
  bImportable = TRUE ;
  bProtected = FALSE ;
  iMarkupCount = 0 ;
  MarkupObjectList[0] = NULL ;
  bLogOpen = FALSE ;

  pluginType = OtmPlugin::eMarkupType;
  usableState = OtmPlugin::eUsable;

  // get the path to our plugin data
  szBasePath[0] = '\0';
  ::GetModuleFileName((HINSTANCE)&__ImageBase, szBasePath, sizeof(szBasePath)-1 );
  char *pszPathEnd = strrchr( szBasePath, '\\' );
  if ( pszPathEnd != NULL ) {
      *pszPathEnd = '\0';
  } 

#ifdef MARKUPTABLE_LOGGING
  if ( !this->Log.isOpen() ) {
    this->Log.open( PluginLogFile );
    bLogOpen = TRUE;
  } /* end */     
  this->Log.writef( "Loading User markup table plugin..." );
#endif

  strcpy( szTemp, szBasePath ) ;
  strcat( szTemp, "\\" ) ;
  strcat( szTemp, PluginControlFile ) ;
  
  fControl = fopen( szTemp, "r" ) ;
  if ( ! fControl ) {
     fControl = fopen( szTemp, "w" ) ;
     for( i=0 ; i < MAX_DEFAULTCONTROL_LINES ; ++i ) {
        fputs( szDefaultControl[i], fControl ) ;
     }
     fclose( fControl ) ;
     fControl = fopen( szTemp, "r" ) ;
     if ( ! fControl ) 
        return ;
  }

  bool bInMarkups = FALSE ;
  bool bInPlugin  = FALSE ;
  bool bInMarkup  = FALSE ;

  while(  fgets( szLine, sizeof(szLine), fControl ) != NULL ) {

     GetControlNode( szLine, szNode, szContent ) ;
     if ( szNode[0] ) {
        if ( ! strcmp( szNode, szXml_Markups ) ) {
           bInMarkups = TRUE ;
           continue;
        }
        if ( bInMarkups ) {
           if ( ! strcmp( szNode, szXml_Plugin ) ) {
              bInPlugin = TRUE ;
              bInMarkup = FALSE ;
              continue;
           } 
           if ( ! strcmp( szNode, szXml_Markup ) ) {
              bInMarkup = TRUE ;
              bInPlugin = FALSE ;
              ptrMarkupInfo = (MARKUPINFO*)calloc( sizeof(MARKUPINFO), 1 ) ;
              continue;
           } 

           if ( bInPlugin ) {
              if ( ( szNode[0] == '/' ) &&
                   ( ! strcmp( &szNode[1], szXml_Plugin ) ) ) {
                 bInPlugin = FALSE ;
                 continue;
              } 
              if ( ! strcmp( szNode, szXml_Deletable ) ) {
                 if ( ! stricmp( szContent, szXml_NO ) ) 
                    bDeletable = FALSE ;
                 continue;
              } 
              if ( ! strcmp( szNode, szXml_Exportable ) ) {
                 if ( ! stricmp( szContent, szXml_NO ) ) 
                    bExportable = FALSE ;
                 continue;
              } 
              if ( ! strcmp( szNode, szXml_Importable ) ) {
                 if ( ! stricmp( szContent, szXml_NO ) ) 
                    bImportable = FALSE ;
                 continue;
              } 
              if ( ! strcmp( szNode, szXml_LongDescription ) ) {
                 SaveValue( &pszLongDescription, szContent ) ;
                 continue;
              } 
              if ( ! strcmp( szNode, szXml_Name ) ) {
                 SaveValue( &pszName, szContent ) ;
                 continue;
              } 
              if ( ! strcmp( szNode, szXml_Protected ) ) {
                 if ( ! stricmp( szContent, szXml_YES ) ) 
                    bProtected = TRUE ;
                 continue;
              } 
              if ( ! strcmp( szNode, szXml_ShortDescription ) ) {
                 SaveValue( &pszShortDescription, szContent ) ;
                 continue;
              } 
              if ( ! strcmp( szNode, szXml_ShortName ) ) {
                 SaveValue( &pszShortName, szContent ) ;
                 continue;
              } 
              if ( ! strcmp( szNode, szXml_Supplier ) ) {
                 SaveValue( &pszSupplier, szContent ) ;
                 continue;
              } 
              if ( ! strcmp( szNode, szXml_TableDirectory ) ) {
                 SaveValue( &pszTableDirectory, szContent ) ;
                 strcpy( szTemp, szBasePath ) ;
                 strcat( szTemp, "\\" ) ;
                 strcat( szTemp, szContent ) ;
                 SaveValue( &pszTablePath, szTemp ) ;
                 if ( ! UtlDirExist( pszTablePath ) ) {
                    UtlMkDir( pszTablePath, 0, FALSE ) ;
#ifdef MARKUPTABLE_LOGGING
                    this->Log.writef( "Create:  %s",pszTablePath );
#endif
                 }
                 continue;
              } 
              if ( ! strcmp( szNode, szXml_UserExitDirectory ) ) {
                 SaveValue( &pszUserExitDirectory, szContent ) ;
                 strcpy( szTemp, szBasePath ) ;
                 strcat( szTemp, "\\" ) ;
                 strcat( szTemp, szContent ) ;
                 SaveValue( &pszUserExitPath, szTemp ) ;
                 if ( ! UtlDirExist( pszUserExitPath ) ) {
                    UtlMkDir( pszUserExitPath, 0, FALSE ) ;
#ifdef MARKUPTABLE_LOGGING
                    this->Log.writef( "Create:  %s",pszUserExitPath );
#endif
                 }
                 continue;
              } 
              if ( ! strcmp( szNode, szXml_Version ) ) {
                 SaveValue( &pszVersion, szContent ) ;
                 continue;
              } 
           }
           if ( bInMarkup ) {
              if ( ( szNode[0] == '/' ) &&
                   ( ! strcmp( &szNode[1], szXml_Markup ) ) ) {
                 bInMarkup = FALSE ;
                 if ( iMarkupCount < MAX_NUM_MARKUPS - 1 ) {
                    MarkupObjectList[iMarkupCount++] = new UserMarkupTable( ptrMarkupInfo, szBasePath );
                    MarkupObjectList[iMarkupCount] = NULL ;
                 } else {
                    /*    ERROR    */
                 }
                 continue;
              } 
              if ( ! stricmp( szNode, szXml_Files ) ) {
                 if ( stricmp( szContent, szXml_NONE ) ) {
                    SaveValue( &(ptrMarkupInfo->pszFileList), szContent ) ;
                    if ( ! CheckFilesExist( szContent, szBasePath, szErrMsg ) ) {
#ifdef MARKUPTABLE_LOGGING
                       this->Log.writef( szErrMsg );
#endif
                    }
                 }
              }
              if ( ! strcmp( szNode, szXml_LongDescription ) ) {
                 SaveValue( &(ptrMarkupInfo->pszLongDescription), szContent ) ;
                 continue;
              } 
              if ( ! strcmp( szNode, szXml_Name ) ) {
                 SaveValue( &(ptrMarkupInfo->pszName), szContent ) ;
#ifdef MARKUPTABLE_LOGGING
                 this->Log.writef( "   Loading:  %s",szContent );
#endif
                 continue;
              } 
              if ( ! strcmp( szNode, szXml_ShortDescription ) ) {
                 SaveValue( &(ptrMarkupInfo->pszShortDescription), szContent ) ;
                 continue;
              } 
              if ( ! strcmp( szNode, szXml_Table ) ) {
                 sprintf( szTemp, "%s\\%s", pszTableDirectory, szContent ) ;
                 SaveValue( &(ptrMarkupInfo->pszTable), szTemp ) ;
                 if ( ! CheckFilesExist( szTemp, szBasePath, szErrMsg ) ) {
#ifdef MARKUPTABLE_LOGGING
                    this->Log.writef( szErrMsg );
#endif
                 }
                 continue;
              } 
              if ( ! strcmp( szNode, szXml_UserExit ) ) {
                 if ( stricmp( szContent, szXml_NONE ) ) {
                    sprintf( szTemp, "%s\\%s", pszUserExitDirectory, szContent ) ;
                    SaveValue( &(ptrMarkupInfo->pszUserExit), szTemp ) ;
                    if ( ! CheckFilesExist( szTemp, szBasePath, szErrMsg ) ) {
#ifdef MARKUPTABLE_LOGGING
                       this->Log.writef( szErrMsg );
#endif
                    }
                 }
                 continue;
              } 
              if ( ! strcmp( szNode, szXml_Version ) ) {
                 SaveValue( &(ptrMarkupInfo->pszVersion), szContent ) ;
                 continue;
              } 
           }
        }
     }
  }
  if ( fControl ) 
     fclose( fControl ) ;


  PerformPendingUpdates( pszUserExitPath ) ;


#ifdef MARKUPTABLE_LOGGING
  this->Log.write( "Load completed." );
  if ( bLogOpen ) this->Log.close();
#endif
}




/* -------------------------------------------------------------- */
/*   destructor.                                                  */
/* -------------------------------------------------------------- */
UserMarkupTablePlugin::~UserMarkupTablePlugin()
{
   //   See stopPlugin() method.
}

/* -------------------------------------------------------------- */
/*   getName.                                                     */
/* -------------------------------------------------------------- */
const char* UserMarkupTablePlugin::getName()
{
	return pszName;
}

/* -------------------------------------------------------------- */
/*   getShortName.                                                */
/* -------------------------------------------------------------- */
const char* UserMarkupTablePlugin::getShortName()
{
	return pszShortName;
}

/* -------------------------------------------------------------- */
/*   getShortDescription.                                         */
/* -------------------------------------------------------------- */
const char* UserMarkupTablePlugin::getShortDescription()
{
	return pszShortDescription;
}

/* -------------------------------------------------------------- */
/*   getLongDescription.                                          */
/* -------------------------------------------------------------- */
const char* UserMarkupTablePlugin::getLongDescription()
{
	return pszLongDescription;
}

/* -------------------------------------------------------------- */
/*   getSupplier.                                                 */
/* -------------------------------------------------------------- */
const char* UserMarkupTablePlugin::getSupplier()
{
	return pszSupplier;
}

/* -------------------------------------------------------------- */
/*   getPluginDirectory.                                          */
/* -------------------------------------------------------------- */
const char* UserMarkupTablePlugin::getPluginDirectory()
{
	return szBasePath; 
}

/* -------------------------------------------------------------- */
/*   getTableDirectory.                                           */
/* -------------------------------------------------------------- */
const char* UserMarkupTablePlugin::getTableDirectory()
{
    return pszTableDirectory; 
}

/* -------------------------------------------------------------- */
/*   getTablePath.                                                */
/* -------------------------------------------------------------- */
const char* UserMarkupTablePlugin::getTablePath()
{
	return pszTablePath;
}

/* -------------------------------------------------------------- */
/*   getUserExitDirectory.                                        */
/* -------------------------------------------------------------- */
const char* UserMarkupTablePlugin::getUserExitDirectory()
{
	return pszUserExitDirectory; 
}

/* -------------------------------------------------------------- */
/*   getUserExitPath.                                             */
/* -------------------------------------------------------------- */
const char* UserMarkupTablePlugin::getUserExitPath()
{
	return pszUserExitPath; 
}

/* -------------------------------------------------------------- */
/*   getVersion.                                                  */
/* -------------------------------------------------------------- */
const char* UserMarkupTablePlugin::getVersion()
{
	return pszVersion;
}

/* -------------------------------------------------------------- */
/*   getCount.                                                    */
/* -------------------------------------------------------------- */
/*! \brief Get the number of markup tables provided by this plugin
	\returns Number of markup tables
*/
	int UserMarkupTablePlugin::getCount()
  {
    return( iMarkupCount );
  }
	
/* -------------------------------------------------------------- */
/*   getMarkup (by index).                                        */
/* -------------------------------------------------------------- */
/*! \brief Get the markup table at the given index
	\param iIndex index of the markup table in the range 0 to (getCount - 1)
	\returns Pointer to the requested markup table object or NULL when iIndex is out of range
*/
	OtmMarkup* UserMarkupTablePlugin::getMarkup
  (
		int iIndex
	)
  {
    if ( (iIndex >= 0) && (iIndex < iMarkupCount) && MarkupObjectList[iIndex] )
    {
      return( (OtmMarkup *)MarkupObjectList[iIndex] );
    } /* end */       

    return( NULL );
  }


/* -------------------------------------------------------------- */
/*   getMarkup (by name).                                         */
/* -------------------------------------------------------------- */
/*! 	\brief Get the markup table object for a markup table
	\param pszMarkup Name of the markup table
	\returns Pointer to the requested markup table object or NULL when markup table does
	not belong to this markup table plugin
*/
	OtmMarkup* UserMarkupTablePlugin::getMarkup
  (
		char *pszMarkup
	)
  {
    char   szTemp[512] ;

    for ( int i = 0; i < iMarkupCount && MarkupObjectList[i]; i++  ) {
      MarkupObjectList[i]->getName( szTemp, sizeof(szTemp) ) ;
      if ( ! stricmp( szTemp, pszMarkup ) ) {
        return( (OtmMarkup *)MarkupObjectList[i] );
      }
    } /* end */     

    return( NULL );
  }


/* -------------------------------------------------------------- */
/*   getAllFiles.                                                 */
/* -------------------------------------------------------------- */
/*! 	\brief Get the name of the all files for this markup table
    \param pFileList   Pointer to other markup table files (with TABLE\ or BIN\ prefix) or NULL
	\returns Pointer to the list of files
*/
const char* UserMarkupTablePlugin::getAllFiles()
  {
     return pszFileList ;
  }


/* -------------------------------------------------------------- */
/*   isImportable.                                                */
/* -------------------------------------------------------------- */
/*! 	\brief Can this markup table be imported?
	\returns TRUE if markup table can be imported.
*/
const bool UserMarkupTablePlugin::isImportable()
  {
     return bImportable ;
  }


/* -------------------------------------------------------------- */
/*   isDeletable.                                                 */
/* -------------------------------------------------------------- */
/*! 	\brief Can this markup table be deleted?
	\returns TRUE if markup table can be deleted.
*/
const bool UserMarkupTablePlugin::isDeletable()
  {
     return bDeletable ;
  }


/* -------------------------------------------------------------- */
/*   isExpired.                                                   */
/* -------------------------------------------------------------- */
/*! 	\brief Is this markup table expired?      
	\returns TRUE if markup table can be expired.
*/
const bool UserMarkupTablePlugin::isExpired()
  {
     return bExpired ;
  }


/* -------------------------------------------------------------- */
/*   isExportable.                                                */
/* -------------------------------------------------------------- */
/*! 	\brief Can this markup table be exported?
	\returns TRUE if markup table can be exported.
*/
const bool UserMarkupTablePlugin::isExportable()
  {
     return bExportable ;
  }


/* -------------------------------------------------------------- */
/*   isProtected.                                                 */
/* -------------------------------------------------------------- */
/*! 	\brief Can this markup table be changed?
	\returns TRUE if markup table is protected and cannot be changed.
*/
const bool UserMarkupTablePlugin::isProtected()
  {
     return bProtected ;
  }


/* -------------------------------------------------------------- */
/*   updateFiles.                                                 */
/* -------------------------------------------------------------- */
/*! 	\brief Update the files contained in this markup table       

  \param pszMarkupName   Pointer to name of markup table
  \param pszShortDescription  Pointer to markup table short description or NULL
  \param pszLongDescription   Pointer to markup table long description or NULL
  \param pszVersion   Pointer to version of markup table or NULL
  \param pTableFileName   Pointer to name of TBL file
  \param pUserExitFileName   Pointer to name of user exit DLL file or NULL
  \param pFileList   Pointer to other markup table files (with TABLE\ or BIN\ prefix) or NULL
  
	\returns  0 when the update failed
              1 when the markup table files have been updated
              2 when the update has been delayed and will occur at restart
*/
const int UserMarkupTablePlugin::updateFiles(
   char   *pszMarkupName,
   char   *pszShortDescription,
   char   *pszLongDescription,
   char   *pszVersion,
   char   *pszTableFileName,
   char   *pszUserExitFileName,
   char   *pszFileList )
  {

     PTAGTABLE  m_pTagTable;

     char   szInLine[512] ;
     char   szNode[512] ;
     char   szContent[512] ;

     char   szMarkupName[512] ;
     char   szShortDescription[512] ;
     char   szLongDescription[512] ;
     char   szVersion[512] ;
     char   szTableDirFileName[512] ;
     char   szTableFileName[512] ;                /* xxx.TBL         */
     char   szUserExitDirFileName[512] ;          /* TABLE\xxx.TBL   */
     char   szUserExitFileName[512] ;             /* xxx.DLL         */
     char   szFileList[MAX_FILELIST] ;            /* BIN\xxx.DLL     */

     char   szTemp[MAX_FILELIST] ;
     char   *ptrChar, *ptrChar2, *ptrChar3 ;

     int    iRC ;
     bool   bNewMarkup = FALSE ;
     bool   bUpdateMarkup = FALSE ;
     bool   bLogOpen = FALSE ;
     int    iReturn = UPDATE_MARKUP_ERROR ;

     /* ------------------------------------------------------------------- */
     /*  Initialization.                                                    */
     /* ------------------------------------------------------------------- */
     strcpy( szMarkupName, pszMarkupName ) ;
     strupr( szMarkupName ) ;
     strcpy( szShortDescription, "-" ) ;
     strcpy( szLongDescription, "-" ) ;
     strcpy( szVersion, "1.0" ) ;
     sprintf( szTableFileName, "%s.TBL", szMarkupName ) ;
     sprintf( szTableDirFileName, "%s\\%s", pszTableDirectory, szTableFileName ) ;
     szUserExitFileName[0] = '\0' ;
     szUserExitDirFileName[0] = '\0' ;
     szFileList[0] = '\0' ;
     bLogOpen = FALSE ;

     if ( ( pszShortDescription  ) &&
          ( *pszShortDescription ) ) {
        strcpy( szShortDescription, pszShortDescription ) ;
     }
     if ( ( pszLongDescription  ) &&
          ( *pszLongDescription ) ) {
        strcpy( szLongDescription, pszLongDescription ) ;
     }
     if ( ( pszVersion  ) &&
          ( *pszVersion ) ) {
        strcpy( szVersion, pszVersion ) ;
     }
     if ( pszTableFileName ) {
        ptrChar = strrchr( pszTableFileName, '\\' ) ;
        if ( ptrChar ) {
           strcpy( szTableFileName, ptrChar+1 ) ;
           if ( ! strstr( szTableFileName, ".TBL" ) ) 
              strcat( szTableFileName, ".TBL" ) ;
           sprintf( szTableDirFileName, "%s\\%s", pszTableDirectory, szTableFileName ) ;
        }
     }
     if ( ( pszUserExitFileName ) &&
          ( *pszUserExitFileName ) ) {
        ptrChar = strrchr( pszUserExitFileName, '\\' ) ;
        if ( ptrChar ) {
           strcpy( szUserExitFileName, ptrChar+1 ) ;
        } else {
           strcpy( szUserExitFileName, pszUserExitFileName ) ;
        }
        if ( ! strstr( szUserExitFileName, ".DLL" ) ) 
           strcat( szUserExitFileName, ".DLL" ) ;
        sprintf( szUserExitDirFileName, "%s\\%s", pszUserExitDirectory, szUserExitFileName ) ;
     }
     if ( pszFileList ) {
        if ( *pszFileList ) {
           szFileList[0] = '\0' ;
           sprintf( szNode, "\\%s\\", pszTableDirectory ) ;
           sprintf( szContent, "\\%s\\", pszUserExitDirectory ) ;
           strcpy( szTemp, pszFileList ) ;
           strupr( szTemp ) ;
           ptrChar = strtok( szTemp, " ," ) ;
           while( ptrChar ) {
              ptrChar2 = strstr( ptrChar, szNode ) ;
              if ( ! ptrChar2 ) 
                 ptrChar2 = strstr( ptrChar, szContent ) ;
              if ( ! ptrChar2 ) {
                 ptrChar2 = strrchr( ptrChar, '\\' ) ;
                 if ( ! ptrChar2 ) 
                    ptrChar2 = ptrChar - 1  ;
                 ptrChar3 = strrchr( ptrChar, '.' ) ;
                 if ( ( ptrChar3 ) &&
                      ( ( ! strcmp( ptrChar3+1, "DLL" ) ) ||
                        ( ! strcmp( ptrChar3+1, "EXE" ) ) ) ) 
                    sprintf( szInLine, " %s\\%s", pszUserExitDirectory, ptrChar2+1 ) ;
                 else
                    sprintf( szInLine, " %s\\%s", pszTableDirectory, ptrChar2+1 ) ;
                 ptrChar2 = szInLine ;
              }
              if ( szFileList[0] ) 
                 strcat( szFileList, "," ) ;
              strcat( szFileList, ptrChar2+1 ) ;
              ptrChar = strtok( NULL, " ," ) ;
           }
        }
     }
     strupr( szTableFileName ) ;
     strupr( szTableDirFileName ) ;
     strupr( szUserExitFileName ) ;
     strupr( szUserExitDirFileName ) ;

     ConvertAnsi2UTF8( szMarkupName, NULL, sizeof(szMarkupName) ) ;
     ConvertAnsi2UTF8( szShortDescription, NULL, sizeof(szShortDescription) ) ;
     ConvertAnsi2UTF8( szLongDescription, NULL, sizeof(szLongDescription) ) ;
     ConvertAnsi2UTF8( szVersion, NULL, sizeof(szVersion) ) ;
     ConvertAnsi2UTF8( szTableFileName, NULL, sizeof(szTableFileName) ) ;
     ConvertAnsi2UTF8( szUserExitFileName, NULL, sizeof(szUserExitFileName) ) ;
     ConvertAnsi2UTF8( szFileList, NULL, sizeof(szFileList) ) ;
     ConvertAnsi2UTF8( szTableDirFileName, NULL, sizeof(szTableDirFileName) ) ;
     ConvertAnsi2UTF8( szUserExitDirFileName, NULL, sizeof(szUserExitDirFileName) ) ;


#ifdef MARKUPTABLE_LOGGING
     if ( !this->Log.isOpen() ) {
       this->Log.open( PluginLogFile );
       bLogOpen = TRUE;
     }      
     this->Log.writef( "Updating markup table...  %s",szMarkupName ) ;
#endif
     
     /* ------------------------------------------------------------------- */
     /*  Get markup table object.  If it does not exist, then this is a     */
     /*  brand new markup table.                                            */
     /* ------------------------------------------------------------------- */
     OtmMarkup *markup = getMarkup( szMarkupName );
     if ( markup == NULL ) {
        if ( iMarkupCount < MAX_NUM_MARKUPS - 1 ) {
           bNewMarkup = TRUE ;
           ptrMarkupInfo = (MARKUPINFO*)calloc( sizeof(MARKUPINFO), 1 ) ;
           markup = new UserMarkupTable( ptrMarkupInfo, szBasePath );
           MarkupObjectList[iMarkupCount++] = markup ;
           MarkupObjectList[iMarkupCount] = NULL ;
#ifdef MARKUPTABLE_LOGGING
           this->Log.writef( "  Adding new....... %d  %s",iMarkupCount-1,szMarkupName);
#endif
        }
     } 

     /* ------------------------------------------------------------------- */
     /*  Process this request.                                              */
     /* ------------------------------------------------------------------- */
     if ( markup != NULL ) {
        iReturn = UPDATE_MARKUP_OK ;

        /* ------------------------------------------------------------------- */
        /*  If no description was provided, get the text from the TBL file.    */
        /* ------------------------------------------------------------------- */
        if ( ( szShortDescription[0] == NULL ) &&
             ( szTableFileName[0] != NULL ) &&
             ( UtlFileExist( pszTableFileName ) ) ) {
           UtlAlloc( (PVOID *)&m_pTagTable, 0L, sizeof(PTAGTABLE), NOMSG );
           ULONG ulRead = 0;
           if ( UtlLoadFileL( pszTableFileName, (PVOID *)&(m_pTagTable), &ulRead, FALSE, TRUE ) ) {
              strcpy( szShortDescription, m_pTagTable->szDescription ) ;
           }
           UtlAlloc( (PVOID *)&m_pTagTable, 0L, 0L, NOMSG );   /* Free */
        }

        /* ------------------------------------------------------------------- */
        /*  Log that this markup table is being updated.                       */
        /* ------------------------------------------------------------------- */
#ifdef MARKUPTABLE_LOGGING
        if ( pszShortDescription   this->Log.writef( "  SDs:    [%s]",pszShortDescription);
        if ( pszLongDescription )  this->Log.writef( "  LDs:    [%s]",pszLongDescription);
        if ( pszVersion )          this->Log.writef( "  Ver:    [%s]",pszVersion);
        if ( pszTableFileName )    this->Log.writef( "  TBL:    [%s]",pszTableFileName);
        if ( pszUserExitFileName ) this->Log.writef( "  DLL:    [%s]",pszUserExitFileName);
        if ( pszFileList )         this->Log.writef( "  List:   [%s]",pszFileList);
#endif

        /* ------------------------------------------------------------------- */
        /*  Update any new or more recent markup table files.                  */
        /* ------------------------------------------------------------------- */

        if ( bNewMarkup ) {
           markup->updateFiles( szMarkupName, szShortDescription, szLongDescription, szVersion,
                       szTableDirFileName, szUserExitDirFileName, szFileList ) ;
        }

        bUpdateMarkup = FALSE ;
        if ( ( pszTableFileName ) &&
             ( *pszTableFileName != NULL ) &&
             ( *(pszTableFileName+1) == ':' ) ) {
           markup->getTableFileName( szTemp, sizeof(szTemp) ) ;
           if ( ! stricmp( pszTableFileName, szTemp ) ) {
              bUpdateMarkup = TRUE ; 
           } else {
              iRC = CopyMarkupFile( pszTableFileName, szTemp, FALSE ) ;
              if ( iRC != COPY_MARKUP_NOCHANGE ) {
                 if ( iRC == COPY_MARKUP_COPIED ) {
                    bUpdateMarkup = TRUE ; 
                 } else {
                    iReturn = UPDATE_MARKUP_ERROR ;
#ifdef MARKUPTABLE_LOGGING
                    this->Log.writef( "  Copy file failed: %s -> %s",pszTableFileName,szTemp);
#endif
                 }
              }
           }
        }
        if ( ( iReturn == UPDATE_MARKUP_OK ) &&
             ( pszUserExitFileName ) &&
             ( *pszUserExitFileName != NULL ) &&
             ( *(pszUserExitFileName+1) == ':' ) ) {
           markup->getUserExitFileName( szTemp, sizeof(szTemp) ) ;
           if ( ! stricmp( pszUserExitFileName, szTemp ) ) {
              bUpdateMarkup = TRUE ; 
           } else {
              iRC = CopyMarkupFile( pszUserExitFileName, szTemp, TRUE ) ;
              if ( iRC != COPY_MARKUP_NOCHANGE ) {
                 if ( iRC == COPY_MARKUP_COPIED ) {
                    bUpdateMarkup = TRUE ; 
                 } else
                 if ( iRC == COPY_MARKUP_INUSE ) {
                    bUpdateMarkup = TRUE ; 
                    iReturn = iRC ;
                 } else {
                    if ( ! bNewMarkup ) {
                       iReturn = UPDATE_MARKUP_ERROR ;
#ifdef MARKUPTABLE_LOGGING
                       this->Log.writef( "  Copy file failed: %s -> %s",pszUserExitFileName,szTemp);
#endif
                    }
                 }
              }
           }
        }
        if ( ( iReturn == UPDATE_MARKUP_OK ) &&
             ( pszFileList ) &&
             ( *pszFileList != NULL ) ) {
           markup->getAllFiles( szTemp, sizeof(szTemp) ) ;
           iRC = CopyMarkupFiles( pszFileList, szTemp, szBasePath, pszTableDirectory, pszUserExitDirectory, TRUE ) ;
           if ( iRC != COPY_MARKUP_NOCHANGE ) {
              if ( iRC == COPY_MARKUP_COPIED )  {
                 bUpdateMarkup = TRUE ; 
              } else
              if ( iRC == COPY_MARKUP_INUSE ) {
                 bUpdateMarkup = TRUE ; 
                 iReturn = iRC ;
              } else {
                 if ( ! bNewMarkup ) {
                    iReturn = UPDATE_MARKUP_ERROR ;
#ifdef MARKUPTABLE_LOGGING
                    this->Log.writef( "  Copy file failed: %s -> %s",pszFileList,szTemp);
#endif
                 }
              }
           }
        }


        /* ------------------------------------------------------------------- */
        /*  Update the markup table object with this new information.          */
        /* ------------------------------------------------------------------- */
        if ( ( iReturn == UPDATE_MARKUP_OK ) &&
             ( ( bNewMarkup ) ||
               ( ( bUpdateMarkup ) &&
                 ( markup->updateFiles( szMarkupName, szShortDescription, szLongDescription,
                             szVersion, szTableDirFileName, szUserExitDirFileName,
                             szFileList ) ) ) ) ) {

           iReturn = UpdateXMLControlFile( szMarkupName, szShortDescription, 
                                           szLongDescription, szVersion, 
                                           szTableFileName, szUserExitFileName,
                                           szFileList, szBasePath, bNewMarkup ) ;
#ifdef MARKUPTABLE_LOGGING
           if ( iReturn == UPATE_MARKUP_OK ) {
              this->Log.write( "  Update control file failed: %s",szMarkupName );
           } else {
              this->Log.write( "  Update control file OK: %s",szMarkupName );
           }
#endif

//           strcpy( szOldFile, szBasePath ) ;
//           strcat( szOldFile, "\\" ) ;
//           strcat( szOldFile, PluginControlFile ) ;
//           strcpy( szNewFile, szOldFile ) ;
//           strcat( szNewFile, ".NEW" ) ;
//
//           fInControl  = fopen( szOldFile, "r" ) ;
//           fOutControl = fopen( szNewFile, "w" ) ;
//           if ( fInControl && fOutControl ) {
//              bool bInMarkups = FALSE ;
//              bool bInMarkup  = FALSE ;
//              bUpdateMarkup = FALSE ;
//             
//              while(  fgets( szInLine, sizeof(szInLine), fInControl ) != NULL ) {
//             
//                 strcpy( szOutLine, szInLine ) ;
//                 GetControlNode( szInLine, szNode, szContent ) ;
//
//                 if ( szNode[0] ) {
//                    if ( ! strcmp( szNode, szXml_Markups ) ) {
//                       bInMarkups = TRUE ;
//                    } else
//                    if ( bInMarkups ) {
//                       if ( ( szNode[0] == '/' ) &&
//                            ( ! strcmp( &szNode[1], szXml_Markups ) ) ) {
//                          bInMarkups = FALSE ;
//                          if ( bNewMarkup ) {
//                             bFileChanged = TRUE ;
//                             fprintf( fOutControl, "<%s>\n", szXml_Markup ) ;
//                             fprintf( fOutControl, szXml_NewRecord, szXml_Name, szMarkupName, szXml_Name ) ;
//                             fprintf( fOutControl, szXml_NewRecord, szXml_ShortDescription, szDescription, szXml_ShortDescription ) ;
//                             fprintf( fOutControl, szXml_NewRecord, szXml_LongDescription, szDescription, szXml_LongDescription ) ;
//                             fprintf( fOutControl, szXml_NewRecord, szXml_Version, szVersion, szXml_Version ) ;
//                             fprintf( fOutControl, szXml_NewRecord, szXml_Table, szTableFileName, szXml_Table ) ;
//                             if ( szUserExitFileName[0] ) 
//                                fprintf( fOutControl, szXml_NewRecord, szXml_UserExit, szUserExitFileName, szXml_UserExit ) ;
//                             else 
//                                fprintf( fOutControl, szXml_NewRecord, szXml_UserExit, szXml_none, szXml_UserExit ) ;
//                             if ( szFileList[0] ) 
//                                fprintf( fOutControl, szXml_NewRecord, szXml_Files, szFileList, szXml_Files ) ;
//                             else
//                                fprintf( fOutControl, szXml_NewRecord, szXml_Files, szXml_none, szXml_Files ) ;
//                             fprintf( fOutControl, "</%s>\n", szXml_Markup ) ;
//                          }
//                          bUpdateMarkup = FALSE ;
//                       } else 
//                       if ( ! strcmp( szNode, szXml_Markup ) ) {
//                          bInMarkup = TRUE ;
//                       } else
//                       if ( bInMarkup ) {
//                          if ( ( szNode[0] == '/' ) &&
//                               ( ! strcmp( &szNode[1], szXml_Markup ) ) ) {
//                             bInMarkup = FALSE ;
//                             bUpdateMarkup = FALSE ;
//                          } else 
//                          if ( ! strcmp( szNode, szXml_Name ) ) {
//                             if ( ! stricmp( szContent, szMarkupName ) ) {
//                                bUpdateMarkup = TRUE ;
//#ifdef MARKUPTABLE_LOGGING
//                                this->Log.writef( "   Updating:  %s",szContent );
//#endif
//                             }
//                          } else
//                          if ( bUpdateMarkup ) {
//                             if ( ! stricmp( szNode, szXml_Files ) ) {
//                                if ( szFileList[0] ) {
//                                   if ( stricmp( szContent, szFileList ) ) {
//                                      bFileChanged = TRUE ;
//                                      sprintf( szOutLine, szXml_NewRecord, szXml_Files, szFileList, szXml_Files ) ;
//                                   }
//                                } else {
//                                   if ( stricmp( szContent, szXml_NONE ) ) {
//                                      bFileChanged = TRUE ;
//                                      sprintf( szOutLine, szXml_NewRecord, szXml_Files, szXml_none, szXml_Files ) ;
//                                   }
//                                }
//                             } else
//                             if ( ! strcmp( szNode, szXml_LongDescription ) ) {
//                                if ( szDescription[0] ) {
//                                   if ( stricmp( szContent, szDescription ) ) {
//                                      bFileChanged = TRUE ;
//                                      sprintf( szOutLine, szXml_NewRecord, szXml_LongDescription, szDescription, szXml_LongDescription ) ;
//                                   }
//                                }
//                             } else
//                             if ( ! strcmp( szNode, szXml_ShortDescription ) ) {
//                                if ( szDescription[0] ) {
//                                   if ( stricmp( szContent, szDescription ) ) {
//                                      bFileChanged = TRUE ;
//                                      sprintf( szOutLine, szXml_NewRecord, szXml_ShortDescription, szDescription, szXml_ShortDescription ) ;
//                                   }
//                                }
//                             } else
//                             if ( ! strcmp( szNode, szXml_Table ) ) {
//                             } else
//                             if ( ! strcmp( szNode, szXml_UserExit ) ) {
//                                if ( szUserExitFileName[0] ) {
//                                   if ( stricmp( szContent, szUserExitFileName ) ) {
//                                      bFileChanged = TRUE ;
//                                      sprintf( szOutLine, szXml_NewRecord, szXml_UserExit, szUserExitFileName, szXml_UserExit ) ;
//                                   }
//                                } else {
//                                   if ( stricmp( szContent, szXml_NONE ) ) {
//                                      bFileChanged = TRUE ;
//                                      sprintf( szOutLine, szXml_NewRecord, szXml_UserExit, szXml_none, szXml_UserExit ) ;
//                                   }
//                                }
//                             } else
//                             if ( ! strcmp( szNode, szXml_Version ) ) {
//                                if ( szVersion[0] ) {
//                                   if ( stricmp( szContent, szVersion ) ) {
//                                      bFileChanged = TRUE ;
//                                      sprintf( szOutLine, szXml_NewRecord, szXml_Version, szVersion, szXml_Version ) ;
//                                   }
//                                }
//                             } 
//                          }
//                       }
//                    }
//                 }
//                 fputs( szOutLine, fOutControl ) ;
//              }
//           } else {
//#ifdef MARKUPTABLE_LOGGING
//              this->Log.write( "  Update failed for control file." );
//#endif
//              iReturn = UPDATE_MARKUP_ERROR ;
//           }
//           if ( fInControl ) 
//              fclose( fInControl ) ;
//           if ( fOutControl ) 
//              fclose( fOutControl ) ;
//           if ( bFileChanged ) {
//              if ( UtlCopy( szNewFile, szOldFile, 1, 0L, TRUE ) ) {
//                 iReturn = UPDATE_MARKUP_ERROR ;
//#ifdef MARKUPTABLE_LOGGING
//                 this->Log.write( "Copy failed for control file." );
//#endif
//              }
//           }
//           UtlDelete( szNewFile, 0L, FALSE ) ;

#ifdef MARKUPTABLE_LOGGING
           this->Log.write( "Update completed." );
#endif
        }
     } 

#ifdef MARKUPTABLE_LOGGING
     if ( bLogOpen ) this->Log.close();
#endif

     return( iReturn ) ;
  }


/* -------------------------------------------------------------- */
/*   updateInfo.                                                  */
/* -------------------------------------------------------------- */
/*! \brief Update XML information for the markup table

  \param pszMarkupName   Pointer to name of markup table (input only)
  \param pszShortDescription   Pointer to markup table short description or NULL
  \param pszLongDescription   Pointer to markup table long description or NULL
  \param pszVersion   Pointer to version of markup table or NULL
  \param pUserExitFileName   Pointer to name of user exit DLL file or NULL

    \returns  0 when the update failed
              1 when the markup table information has been updated

*/
const int UserMarkupTablePlugin::updateInfo(
   char   *pszMarkupName,
   char   *pszShortDescription,
   char   *pszLongDescription,
   char   *pszVersion,
   char   *pszUserExitFileName )
{
   char   szMarkupName[512] ;
   char   szShortDescription[512] ;
   char   szLongDescription[512] ;
   char   szVersion[512] ;
   char   szUserExitFileName[512] ;        
   int    iReturn ;

   strcpy( szMarkupName, pszMarkupName ) ;
   strupr( szMarkupName ) ;

   ConvertAnsi2UTF8( szMarkupName, NULL, sizeof(szMarkupName) ) ;
   ConvertAnsi2UTF8( pszShortDescription, szShortDescription, sizeof(szShortDescription) ) ;
   ConvertAnsi2UTF8( pszLongDescription, szLongDescription, sizeof(szLongDescription) ) ;
   ConvertAnsi2UTF8( pszVersion, szVersion, sizeof(szVersion) ) ;
   ConvertAnsi2UTF8( pszUserExitFileName, szUserExitFileName, sizeof(szUserExitFileName) ) ;

   iReturn = UpdateXMLControlFile( szMarkupName, szShortDescription, 
                                   szLongDescription, szVersion, 
                                   NULL, szUserExitFileName,
                                   NULL, szBasePath, FALSE ) ;

   return( iReturn ) ;
}


/* -------------------------------------------------------------- */
/*   deleteMarkup                                                 */
/* -------------------------------------------------------------- */
/*!     \brief Delete a markup table

  \param pszMarkupName   Pointer to name of markup table
	
	\returns TRUE when the markup table was deleted and FALSE when the delete is 
  not possible

*/
const bool UserMarkupTablePlugin::deleteMarkup(
   char   *pszMarkupName )
{
     FILE   *fInControl, *fOutControl  ;

     char   szNewFile[512] ;
     char   szOldFile[512] ;
     char   szInLine[512] ;
     char   szOutLine[512] ;
     char   szNode[512] ;
     char   szContent[512] ;
     char   szTemp[512] ;

     char   szMarkupName[512] ;

     long   save_pos1, save_pos2 ;
     bool   bDeleteMarkup = FALSE ;
     bool   bFileChanged = FALSE ;
     bool   bLogOpen;
     bool   bReturn = FALSE ;

     /* ------------------------------------------------------------------- */
     /*  Initialization.                                                    */
     /* ------------------------------------------------------------------- */
     strcpy( szMarkupName, pszMarkupName ) ;
     strupr( szMarkupName ) ;
     bLogOpen = FALSE ;
//fdaw=fopen("C:\\daw.daw","a");
//fprintf(fdaw,"Deleting....%s\n",szMarkupName);
//fclose(fdaw);

#ifdef MARKUPTABLE_LOGGING
     if ( !this->Log.isOpen() ) {
       this->Log.open( PluginLogFile );
       bLogOpen = TRUE;
     }      
#endif
     
     /* ------------------------------------------------------------------- */
     /*  Get markup table object.  If it does not exist, then there is      */
     /*  nothing to do.                                                     */
     /* ------------------------------------------------------------------- */
     OtmMarkup *markup = getMarkup( szMarkupName );
     if ( markup != NULL ) {
#ifdef MARKUPTABLE_LOGGING
        this->Log.writef( "Deleting markup table...  %s",szMarkupName ) ;
#endif

        /* ------------------------------------------------------------------- */
        /*  Update the markup table object with this new information.          */
        /* ------------------------------------------------------------------- */
        for ( int i = 0; i < iMarkupCount && MarkupObjectList[i]; i++  ) {
           MarkupObjectList[i]->getName( szTemp, sizeof(szTemp) ) ;
           if ( ! stricmp( szTemp, pszMarkupName ) ) {
              if ( iMarkupCount > 1 ) 
                 MarkupObjectList[i] = MarkupObjectList[iMarkupCount-1] ; 
              if ( iMarkupCount > 0 ) 
                 MarkupObjectList[--iMarkupCount] = NULL ;
              break ;
           }
        } 

        markup->deleteMarkup( szMarkupName ) ;

        /* ------------------------------------------------------------------- */
        /*  Update control XML file with new information.                      */
        /* ------------------------------------------------------------------- */
        strcpy( szOldFile, szBasePath ) ;
        strcat( szOldFile, "\\" ) ;
        strcat( szOldFile, PluginControlFile ) ;
        strcpy( szNewFile, szOldFile ) ;
        strcat( szNewFile, ".NEW" ) ;

        fInControl  = fopen( szOldFile, "r" ) ;
        fOutControl = fopen( szNewFile, "w" ) ;
        if ( fInControl && fOutControl ) {
           bool bInMarkups = FALSE ;
           bool bInMarkup  = FALSE ;
           bDeleteMarkup = FALSE ;
          
           for( save_pos1 = ftell(fInControl) ;
                fgets( szInLine, sizeof(szInLine), fInControl ) != NULL ;
                save_pos1 = ftell(fInControl) ) {
          
              strcpy( szOutLine, szInLine ) ;
              GetControlNode( szInLine, szNode, szContent ) ;

              if ( szNode[0] ) {
                 if ( ! strcmp( szNode, szXml_Markups ) ) {
                    bInMarkups = TRUE ;
                 } else
                 if ( bInMarkups ) {
                    if ( ( szNode[0] == '/' ) &&
                         ( ! strcmp( &szNode[1], szXml_Markups ) ) ) {
                       bInMarkups = FALSE ;
                    } else 
                    if ( ! strcmp( szNode, szXml_Markup ) ) {
                       bInMarkup = TRUE ;
                       save_pos2 = ftell(fInControl) ; 
                       if ( fgets( szTemp, sizeof(szTemp), fInControl ) != NULL ) {
                          GetControlNode( szTemp, szNode, szContent ) ;
                          if ( ( ! strcmp( szNode, szXml_Name ) ) &&
                               ( ! stricmp( szContent, szMarkupName ) ) ) {
                             bDeleteMarkup = TRUE ;
                             bFileChanged = TRUE ;
                             bReturn = TRUE ;
                             continue;
                          }
                       }
                       fseek( fInControl, save_pos2, 0 ) ;
                    } else
                    if ( bInMarkup ) {
                       if ( ( szNode[0] == '/' ) &&
                            ( ! strcmp( &szNode[1], szXml_Markup ) ) ) {
                          bInMarkup = FALSE ;
                          if ( bDeleteMarkup ) {
                             bDeleteMarkup = FALSE ;
                             continue ;
                          }
                       } 
                    }
                 }
              }
              if ( ! bDeleteMarkup ) 
                 fputs( szOutLine, fOutControl ) ;
           }
        } else {
#ifdef MARKUPTABLE_LOGGING
           this->Log.write( "  Update failed for control file." );
#endif
           bReturn = false ;
        }
        if ( fInControl ) 
           fclose( fInControl ) ;
        if ( fOutControl ) 
           fclose( fOutControl ) ;
        if ( bFileChanged ) {
           if ( UtlCopy( szNewFile, szOldFile, 1, 0L, TRUE ) ) {
              bReturn = false ;
#ifdef MARKUPTABLE_LOGGING
              this->Log.write( "Copy failed for control file." );
#endif
           }
           UtlDelete( szNewFile, 0L, FALSE ) ;
        }

#ifdef MARKUPTABLE_LOGGING
        this->Log.write( "Delete completed." );
#endif
        bReturn = true ;
     } 

#ifdef MARKUPTABLE_LOGGING
     if ( bLogOpen ) this->Log.close();
#endif

     return( bReturn ) ;
}


/* -------------------------------------------------------------- */
/*   stopPlugin                                                   */
/* -------------------------------------------------------------- */
/*!     \brief Stop the markup table plugin
*/
bool UserMarkupTablePlugin::stopPlugin( bool fForce  )
{
   bool   bLogOpen = FALSE ;

  // TODO: check for active objects..
  bool fActiveObjects = false;

  // decline stop if we have active objects
  if ( !fForce && fActiveObjects )
  {
    return( false );
  }

  // TODO: terminate active objects, cleanup, free allocated resources



  bLogOpen = FALSE ;
#ifdef MARKUPTABLE_LOGGING
  if ( !this->Log.isOpen() ) {
    this->Log.open( PluginLogFile );
    bLogOpen = TRUE;
  } /* end */     
  this->Log.writef( "Stopping User markup table plugin..." );
#endif

  if ( pszName )  free(pszName) ;
  if ( pszShortName )  free(pszShortName) ;
  if ( pszShortDescription )  free(pszShortDescription);
  if ( pszLongDescription )  free(pszLongDescription);
  if ( pszVersion )  free(pszVersion) ;
  if ( pszSupplier )  free(pszSupplier) ;
  if ( pszTableDirectory )  free(pszTableDirectory) ;
  if ( pszTablePath )  free(pszTablePath) ;
  if ( pszUserExitDirectory )  free(pszUserExitDirectory) ;
  if ( pszUserExitPath )  free(pszUserExitPath) ;
  if ( pszFileList )  free(pszFileList) ;

  for( int i=0 ; i<iMarkupCount && MarkupObjectList[i] ; ++i ) {
     delete MarkupObjectList[i] ;
  }

#ifdef MARKUPTABLE_LOGGING
  this->Log.write( "Stop completed." );
  if ( bLogOpen ) this->Log.close();
#endif

  // de-register plugin
	PluginManager *pPluginManager = PluginManager::getInstance();
	pPluginManager->deregisterPlugin( (OtmPlugin *)this );

  return( true );
}

/* -------------------------------------------------------------- */
/*   registerPlugin.                                              */
/* -------------------------------------------------------------- */
/*! 	\brief registers the plugin
*/
extern "C" {
__declspec(dllexport)
USHORT registerPlugins()
{
	PluginManager::eRegRc eRc = PluginManager::eSuccess;
	PluginManager *manager = PluginManager::getInstance();
	UserMarkupTablePlugin* plugin = new UserMarkupTablePlugin();
	eRc = manager->registerPlugin((OtmPlugin*) plugin);
    USHORT usRC = (USHORT) eRc;
    return usRC;
}

}




/* -------------------------------------------------------------- */
/*   GetControlNode                                               */
/*                                                                */
/*   Parse the XML line to identify the XML tag and its content.  */
/* -------------------------------------------------------------- */
/*! 
  \brief Split line to identify XML node and content 
  \param pszLine  Input line to parse
  \param pszNode  Name of XML element found on this line
  \param pszContent  Content of XML element found on this line
  \returns pszNode, pszContent
*/

void  GetControlNode( char *pszLine, char *pszNode, char *pszContent )
{
   CHAR_W  szTempW[150] ;
   char *ptrChar, *ptrChar2 ;

   pszNode[0] = 0 ;
   pszContent[0] = 0 ;

   for( ptrChar=pszLine ; *ptrChar && strchr(" \t\r\n", *ptrChar) ; ++ptrChar ) ;
   /*  Line must start with an XML element.   */
   ptrChar = strchr( ptrChar, '<' ) ;  
   if ( ptrChar ) {
      for( ++ptrChar ; *ptrChar && strchr(" \t\r\n", *ptrChar) ; ++ptrChar ) ;
      ptrChar2 = strchr( ptrChar+1, '>' ) ;
      if ( ptrChar2 ) {
         for( --ptrChar2 ; ptrChar2>ptrChar && strchr(" \t\r\n", *ptrChar2) ; --ptrChar2 ) ;
         *(++ptrChar2) = 0 ;
         strcpy( pszNode, ptrChar ) ;

         ptrChar = ptrChar2 + 1 ;
         for( ptrChar ; *ptrChar && strchr(" \t\r\n", *ptrChar) ; ++ptrChar ) ;
         /*  Line must end with an XML element.   */
         ptrChar2 = strstr( ptrChar+1, "</" ) ; 
         if ( ptrChar2 ) {
            for( --ptrChar2 ; ptrChar2>ptrChar && strchr(" \t\r\n", *ptrChar2) ; --ptrChar2 ) ;
            *(++ptrChar2) = 0 ;
            strcpy( pszContent, ptrChar ) ;
            MultiByteToWideChar( CP_UTF8, 0, (LPCSTR)pszContent, -1, (LPWSTR)szTempW, sizeof(szTempW) );
            WideCharToMultiByte( CP_ACP, 0, (LPCWSTR)szTempW, -1, (LPSTR)pszContent, 150, NULL, NULL );
         }
      }
   }
}

/* -------------------------------------------------------------- */
/*   SaveValue                                                    */
/*                                                                */
/*   Allocate storage and save the information there.             */
/* -------------------------------------------------------------- */
/*! 
  \brief Copies string to new allocated storage
  \param Output   Pointer to location to save new memory pointer
  \param Input    Pointer to string to allocate memory for and copy
*/

void  SaveValue( char **pszOutput, char *pszInput )
{
   if ( *pszOutput != NULL ) 
      free( *pszOutput ) ;
   *pszOutput = (char*)malloc( strlen(pszInput)+1 ) ;
   strcpy( *pszOutput, pszInput ) ;
}

/* -------------------------------------------------------------- */
/*   CheckFilesExist                                              */
/*                                                                */
/*   Check if this list of markup table files exist or not        */
/* -------------------------------------------------------------- */
/*! 
  \brief Check if file(s) exist
  \param pszFile   Single file or comma-separated list of files to check
  \param pszBasePath   Base path containing pszFile files
  \param pszErrMsg   Error message if files not found.
  \returns TRUE if files exist, 
           FALSE if one of more missing file and pszErrMsg set
*/

BOOL  CheckFilesExist( char *pszFile, char *pszBasePath, char *pszErrMsg  ) 
{
   char    szTemp[512] ;
   char    *ptrToken ;
   BOOL    bReturn = TRUE ; 


   strcpy(pszErrMsg, "       Missing files:   " ) ;

   ptrToken = strtok( pszFile, " ," ) ;
   while( ptrToken ) {
      if ( pszBasePath[0] ) {
         strcpy( szTemp, pszBasePath ) ;
         strcat( szTemp, "\\" );
         strcat( szTemp, ptrToken ) ;
      } else {
         strcpy( szTemp, ptrToken ) ;
      }
      if ( ! UtlFileExist( szTemp ) ) {
         bReturn = FALSE ;
         strcat( pszErrMsg, ptrToken ) ;
         strcat( pszErrMsg, "   " ) ;
      }
      ptrToken = strtok( NULL, " ," ) ;
   }
   return( bReturn ) ;
}

/* -------------------------------------------------------------- */
/*   CopyMarkupFiles                                              */
/*                                                                */
/*   Copy a set of files when a file is newer that the old files. */
/*                                                                */
/*   Returns:   0 = File not changed and not copied.              */
/*              1 = File new/change and was copied.               */
/*              2 = File in use, not copied, sharing violation.   */
/*              3 = Unsuccessful.                                 */
/* -------------------------------------------------------------- */
/*! 
  \brief Copy a set of files when newer than old files
  \param pszNewFile   Comma-separated list of files to copy from
  \param pszOldFile   Comma-separated list of old files to copy to
  \param pszBasePath   Base path containing old files
  \param pszTableDirectory  Directory containing TBL files (TABLE)
  \param pszUserExitDirectory  Directory containing user exit DLL files (BIN)
  \param bSaveInUse  TRUE=Save file if in use
  \returns 0 = File has not changed and was not copied
           1 = File is new/changed and was copied
           2 = File in use, not copied, sharing violation. 
           3 = Unsuccessful copy
*/

int  CopyMarkupFiles( char *pszNewFile, char *pszOldFile, 
                      char *pszBasePath, char *pszTableDirectory,
                      char *pszUserExitDirectory, BOOL bSaveInUse ) 
{
   char    szTemp[8096];
   char    szTemp2[8096];
   char    *ptrNewFile, *ptrOldFile ;
   char    *ptrChar, *ptrChar2 ; 
   int     iRC ;
   int     iReturn = COPY_MARKUP_NOCHANGE ;


   strcpy( szTemp2, pszNewFile ) ;
   ptrNewFile = strtok( szTemp2, " ," ) ;
   while( ( ptrNewFile ) &&
          ( iReturn != COPY_MARKUP_ERROR ) ) {
      ptrChar = strrchr( ptrNewFile, '\\' ) ;
      if ( ptrChar ) {
         strcpy( szTemp, pszOldFile ) ;
         ptrChar2 = strstr( szTemp, ptrChar ) ;
         if ( ptrChar2 ) {
            *(ptrChar2+strlen(ptrChar)) = '\0' ;
            ptrOldFile = strrchr( szTemp, ',' ) ;
            if ( ! ptrOldFile ) 
               ptrOldFile = szTemp ; 
            else
               ++ptrOldFile ;
         } else {
            ptrChar2 = strrchr( pszNewFile, '.' ) ;
            if ( ( ptrChar2 ) &&
                 ( ( ! strcmp( ptrChar2+1, "DLL" ) ) ||
                   ( ! strcmp( ptrChar2+1, "EXE" ) ) ) ) 
               sprintf( szTemp, "%s\\%s\\%s", pszBasePath, pszUserExitDirectory, ptrChar+1 ) ;
            else
               sprintf( szTemp, "%s\\%s\\%s", pszBasePath, pszTableDirectory, ptrChar+1 ) ;
            ptrOldFile = szTemp ;
         }

         iRC = CopyMarkupFile( ptrNewFile, ptrOldFile, bSaveInUse ) ;
         if ( ( iRC == COPY_MARKUP_INUSE ) ||
              ( iRC == COPY_MARKUP_ERROR ) ||
              ( ( iRC == COPY_MARKUP_COPIED ) &&
                ( iReturn != COPY_MARKUP_INUSE ) ) ) {
            iReturn = iRC ;
         }
      }
      ptrNewFile = strtok( NULL, " ," ) ;
   }
   return( iReturn ) ;
}

/* -------------------------------------------------------------- */
/*   CopyMarkupFile                                               */
/*                                                                */
/*   Copy a single file if it is newer that the old file.         */
/*                                                                */
/*   Returns:   0 = File not changed and not copied.              */
/*              1 = File new/change and was copied.               */
/*              2 = File in use, not copied, sharing violation.   */
/*              3 = Unsuccessful.                                 */
/* -------------------------------------------------------------- */
/*! 
  \brief Copy a single file when newer than old file
  \param pszNewFile   Fully qualified new file to copy from
  \param pszMarkupFile   Fully qualified current file to copy to
  \param bSaveInUse  TRUE=Save file if in use
  \returns 0 = File has not changed and was not copied
           1 = File is new/changed and was copied
           2 = File in use, sharing violation
           3 = Unsuccessful copy
*/

int  CopyMarkupFile( char *pszNewFile, char *pszMarkupFile,
                     BOOL bSaveInUse )
{
   char    szTemp[MAX_LONGFILESPEC];
   char    *ptrChar ; 
   short   sResult ;
   int     iReturn = COPY_MARKUP_NOCHANGE ;


   if ( UtlFileExist( pszNewFile ) ) {
      if ( UtlFileExist( pszMarkupFile ) ) {
         if ( ( UtlCompFDates( pszNewFile, pszMarkupFile, &sResult, FALSE ) == 0 ) &&
              ( sResult > 0 ) ) {
            sResult = UtlCopy( pszNewFile, pszMarkupFile, 1, 0L, FALSE ); 
            if ( ( bSaveInUse ) &&
                 ( sResult == ERROR_SHARING_VIOLATION ) ) {
               iReturn = COPY_MARKUP_INUSE ;

               // copy markup table file to "copy pending user exit" directory
               strcpy( szTemp, pszMarkupFile ) ;
               ptrChar = strrchr( szTemp, BACKSLASH ) ; 
               if ( ptrChar ) {
                  int iLen = strlen(PENDINGEXITS_DIR) ;
                  memmove( ptrChar+iLen+1, ptrChar, strlen(ptrChar)+1 ) ;
                  *ptrChar = BACKSLASH ;
                  strncpy( ptrChar+1, PENDINGEXITS_DIR, iLen ) ;
                  ptrChar += iLen + 1 ;
                  *ptrChar = 0 ;
                  UtlMkDir( szTemp, 0, FALSE );
                  *ptrChar = BACKSLASH ;
                  iLen = CopyMarkupFile( pszNewFile, szTemp, FALSE ) ;
                  if ( iLen == COPY_MARKUP_ERROR ) {
                     iReturn = iLen ; 
                  }
               } else {
                  iReturn = COPY_MARKUP_ERROR ;
               }
            }
            else
            if ( ! sResult ) {
               iReturn = COPY_MARKUP_COPIED ;
            } else {
               iReturn = COPY_MARKUP_ERROR ;
            }
         }
      } else {
         sResult = UtlCopy( pszNewFile, pszMarkupFile, 1, 0L, TRUE );
         if ( ! sResult ) 
            iReturn = COPY_MARKUP_COPIED ;
         else
            iReturn = COPY_MARKUP_ERROR ;
      }
   } else {
      iReturn = COPY_MARKUP_ERROR ;
   }
   return( iReturn ) ;
}


/* -------------------------------------------------------------- */
/*   UpdateXMLControlFile                                         */
/*                                                                */
/*   Update the markup table information in the control file.     */
/*                                                                */
/*     <markups>                                                  */
/*      <markup>                                                  */
/*       <name>IBMJDK11</name>                                    */
/*       <shortDescription>Java ResourceBundle</shortDescription> */
/*       <longDescription>Markup for Java RB</longDescription>    */
/*       <version>1.0.0</version>                                 */
/*       <table>IBMJDK11.TBL</table>                              */
/*       <userExit>IBMJDK11.DLL</userExit>                        */
/*       <files>none</files>                                      */
/*      </markup>                                                 */
/*     <markups>                                                  */
/* -------------------------------------------------------------- */
BOOL UpdateXMLControlFile( char   *pszMarkupName,
                           char   *pszShortDescription,
                           char   *pszLongDescription,
                           char   *pszVersion,
                           char   *pszTableFileName,
                           char   *pszUserExitFileName,
                           char   *pszFileList,
                           char   *pszBasePath,
                           BOOL   bNewMarkup
                           )
{
   FILE   *fInControl, *fOutControl  ;

   char   szNewFile[512] ;
   char   szOldFile[512] ;
   char   szInLine[512] ;
   char   szOutLine[512] ;
   char   szNode[512] ;
   char   szContent[512] ;

   ULONG   ulPrevMarkupNode = 0 ;

   bool   bFileChanged = FALSE ;
   bool   bUpdateMarkup = FALSE ;
   int    iReturn = UPDATE_MARKUP_ERROR ;

   /* ------------------------------------------------------------------- */
   /*  Update control XML file with new information.                      */
   /* ------------------------------------------------------------------- */

   strcpy( szOldFile, pszBasePath ) ;
   strcat( szOldFile, "\\" ) ;
   strcat( szOldFile, PluginControlFile ) ;
   strcpy( szNewFile, szOldFile ) ;
   strcat( szNewFile, ".NEW" ) ;

   fInControl  = fopen( szOldFile, "r" ) ;
   fOutControl = fopen( szNewFile, "w" ) ;
   if ( fInControl && fOutControl ) {
      bool bInMarkups = FALSE ;
      bool bInMarkup  = FALSE ;
      bUpdateMarkup = FALSE ;

      while(  fgets( szInLine, sizeof(szInLine), fInControl ) != NULL ) {

         strcpy( szOutLine, szInLine ) ;
         GetControlNode( szInLine, szNode, szContent ) ;

         if ( szNode[0] ) {
            if ( ! strcmp( szNode, szXml_Markups ) ) {              /* Found <markups> tag */
               bInMarkups = TRUE ;
            } else
            if ( bInMarkups ) {
               if ( ( szNode[0] == '/' ) &&
                    ( ! strcmp( &szNode[1], szXml_Markups ) ) ) {   /* Found </markups> tag */
                  bInMarkups = FALSE ;
                  bUpdateMarkup = FALSE ;

                  /* ------------------------------------------------------------------- */
                  /*  Add information for this new markup table.                         */
                  /* ------------------------------------------------------------------- */
                  if ( bNewMarkup ) {
                     bFileChanged = TRUE ;
                     iReturn = UPDATE_MARKUP_OK ;
                     fprintf( fOutControl, "<%s>\n", szXml_Markup ) ;
                     fprintf( fOutControl, szXml_NewRecord, szXml_Name, pszMarkupName, szXml_Name ) ;
                     fprintf( fOutControl, szXml_NewRecord, szXml_ShortDescription, pszShortDescription, szXml_ShortDescription ) ;
                     fprintf( fOutControl, szXml_NewRecord, szXml_LongDescription, pszLongDescription, szXml_LongDescription ) ;
                     fprintf( fOutControl, szXml_NewRecord, szXml_Version, pszVersion, szXml_Version ) ;
                     fprintf( fOutControl, szXml_NewRecord, szXml_Table, pszTableFileName, szXml_Table ) ;
                     if ( *pszUserExitFileName ) 
                        fprintf( fOutControl, szXml_NewRecord, szXml_UserExit, pszUserExitFileName, szXml_UserExit ) ;
                     else 
                        fprintf( fOutControl, szXml_NewRecord, szXml_UserExit, szXml_none, szXml_UserExit ) ;
                     if ( *pszFileList ) 
                        fprintf( fOutControl, szXml_NewRecord, szXml_Files, pszFileList, szXml_Files ) ;
                     else
                        fprintf( fOutControl, szXml_NewRecord, szXml_Files, szXml_none, szXml_Files ) ;
                     fprintf( fOutControl, "</%s>\n", szXml_Markup ) ;
                  }

               } else 

               if ( ! strcmp( szNode, szXml_Markup ) ) {               /* Found <markup> tag */
                  bInMarkup = TRUE ;
                  if ( bNewMarkup ) 
                     ulPrevMarkupNode = ftell( fOutControl ) ;
               } else

               if ( bInMarkup ) {
                  if ( ( szNode[0] == '/' ) &&
                       ( ! strcmp( &szNode[1], szXml_Markup ) ) ) {    /* Found </markup> tag */
                     bInMarkup = FALSE ;
                     bUpdateMarkup = FALSE ;
                  } else 

                  if ( ! strcmp( szNode, szXml_Name ) ) {              /* Found <name> tag */
                     if ( ! stricmp( szContent, pszMarkupName ) ) {
                        bUpdateMarkup = TRUE ;
                     }
                  } else

                  /* ------------------------------------------------------------------- */
                  /*  Update this markup table with new information.                     */
                  /* ------------------------------------------------------------------- */
                  if ( bUpdateMarkup ) {
                     iReturn = UPDATE_MARKUP_OK ;
                     if ( ( ! stricmp( szNode, szXml_Files ) ) &&            /* Found <files> tag */
                          ( pszFileList ) ) {
                        if ( *pszFileList ) {
                           if ( stricmp( szContent, pszFileList ) ) {
                              bFileChanged = TRUE ;
                              sprintf( szOutLine, szXml_NewRecord, szXml_Files, pszFileList, szXml_Files ) ;
                           }
                        } else {
                           if ( stricmp( szContent, szXml_NONE ) ) {
                              bFileChanged = TRUE ;
                              sprintf( szOutLine, szXml_NewRecord, szXml_Files, szXml_none, szXml_Files ) ;
                           }
                        }
                     } else

                     if ( ( ! strcmp( szNode, szXml_LongDescription ) ) &&   /* Found <longDescription> tag */
                          ( pszLongDescription ) ) {
                        if ( *pszLongDescription ) {
                           if ( stricmp( szContent, pszLongDescription ) ) {
                              bFileChanged = TRUE ;
                              sprintf( szOutLine, szXml_NewRecord, szXml_LongDescription, pszLongDescription, szXml_LongDescription ) ;
                           }
                        }
                     } else

                     if ( ( ! strcmp( szNode, szXml_ShortDescription ) ) &&  /* Found <shortDescription> tag */
                          ( pszShortDescription ) ) {
                        if ( *pszShortDescription ) {
                           if ( stricmp( szContent, pszShortDescription ) ) {
                              bFileChanged = TRUE ;
                              sprintf( szOutLine, szXml_NewRecord, szXml_ShortDescription, pszShortDescription, szXml_ShortDescription ) ;
                           }
                        }
                     } else

                     if ( ! strcmp( szNode, szXml_Table ) ) {                /* Found <table> tag */
                     } else

                     if ( ( ! strcmp( szNode, szXml_UserExit ) ) &&
                          ( pszUserExitFileName)  ) {
                        if ( *pszUserExitFileName ) {
                           if ( stricmp( szContent, pszUserExitFileName ) ) {
                              bFileChanged = TRUE ;
                              sprintf( szOutLine, szXml_NewRecord, szXml_UserExit, pszUserExitFileName, szXml_UserExit ) ;
                           }
                        } else {
                           if ( stricmp( szContent, szXml_NONE ) ) {
                              bFileChanged = TRUE ;
                              sprintf( szOutLine, szXml_NewRecord, szXml_UserExit, szXml_none, szXml_UserExit ) ;
                           }
                        }
                     } else

                     if ( ( ! strcmp( szNode, szXml_Version ) ) &&           /* Found <version> tag */
                          ( pszVersion ) ) {
                        if ( *pszVersion ) {
                           if ( stricmp( szContent, pszVersion ) ) {
                              bFileChanged = TRUE ;
                              sprintf( szOutLine, szXml_NewRecord, szXml_Version, pszVersion, szXml_Version ) ;
                           }
                        }
                     } 
                  }

                  /* ------------------------------------------------------------------- */
                  /*  If new markup table, then there should be no existing records.     */
                  /*  Remove these records if they exist.                                */
                  /* ------------------------------------------------------------------- */
                  if ( ( bUpdateMarkup ) &&
                       ( bNewMarkup    ) ) {
                     if ( ulPrevMarkupNode ) {
                        fseek( fOutControl, ulPrevMarkupNode, 0 ) ;
                        ulPrevMarkupNode = 0 ;
                     }
                     continue ;
                  }
               }
            }
         }
         fputs( szOutLine, fOutControl ) ;
      }
   } else {
      iReturn = UPDATE_MARKUP_ERROR ;
   }
   if ( fInControl ) 
      fclose( fInControl ) ;
   if ( fOutControl ) 
      fclose( fOutControl ) ;
   if ( bFileChanged ) {
      if ( UtlCopy( szNewFile, szOldFile, 1, 0L, TRUE ) ) {
         iReturn = UPDATE_MARKUP_ERROR ;
      }
   }
   UtlDelete( szNewFile, 0L, FALSE ) ;

   return( iReturn ) ;
} /* end of function UpdateXMLControlFile */


/* -------------------------------------------------------------- */
/*   PerformPendingUpdate                                         */
/*                                                                */
/*   Perform any pending markup table updates during startup.     */
/* -------------------------------------------------------------- */
void PerformPendingUpdates( char *pszBasePath )
{
  FILEFINDBUF stFile;              // Output buffer of UtlFindFirst
  USHORT usCount;                  // For UtlFindFirst
  HDIR hSearch = HDIR_CREATE;      // Directory handle for UtlFindFirst
  USHORT usRC;
  CHAR szPending[MAX_EQF_PATH];    // buffer for directory with pending DLLs
  CHAR szSearch[MAX_EQF_PATH];     // buffer for search pattern
  CHAR szSource[MAX_EQF_PATH];     // buffer for source file
  CHAR szTarget[MAX_EQF_PATH];     // buffer for target file
  BOOL fUserExitsMoved = FALSE;    // TRUE = user exits have been moved

  //
  // copy any user exits
  //

  // setup search path
  strcpy( szPending, pszBasePath ) ;
  strcat( szPending, BACKSLASH_STR );
  strcat( szPending, PENDINGEXITS_DIR );
  strcat( szPending, BACKSLASH_STR );

  strcpy( szSearch, szPending ) ;
  strcat( szSearch, DEFAULT_PATTERN_NAME );
  strcat( szSearch, EXT_OF_DLL );

  // loop over all user exit DLLs in  'pending user exit copy' directory
  memset( &stFile, 0, sizeof(stFile) );
  usCount = 1;
  usRC = UtlFindFirst( szSearch, &hSearch, 0, &stFile, sizeof(stFile), &usCount, 0L, FALSE );
  while ( (usRC == NO_ERROR) && usCount )
  {
    // setup source and target path names
    strcpy( szSource, szPending );
    strcat( szSource, RESBUFNAME(stFile) );
    strcpy( szTarget, pszBasePath );
    strcat( szTarget, BACKSLASH_STR );
    strcat( szTarget, RESBUFNAME(stFile) );

    // move user exit
    UtlDelete( szTarget, 0L, FALSE );
    UtlMove( szSource, szTarget, 0L, FALSE );
    fUserExitsMoved = TRUE;

    // next user exit DLL
    usRC = UtlFindNext( hSearch, &stFile, sizeof(stFile), &usCount, FALSE );
  } /* endwhile */
  if ( hSearch != HDIR_CREATE ) UtlFindClose( hSearch, FALSE );

  // reset folder-is-disabled flags in folder properties
  if ( fUserExitsMoved )
  {
    UtlRemoveDir( szPending, FALSE );

    // loop over all folder properties and reset flag if required
    hSearch = HDIR_CREATE;      
    UtlMakeEQFPath( szSearch, NULC, PROPERTY_PATH, NULL );
    strcat( szSearch, BACKSLASH_STR );
    strcat( szSearch, DEFAULT_PATTERN_NAME );
    strcat( szSearch, EXT_FOLDER_MAIN );
    memset( &stFile, 0, sizeof(stFile) );
    usCount = 1;
    usRC = UtlFindFirst( szSearch, &hSearch, 0, &stFile, sizeof(stFile), &usCount, 0L, FALSE );
    while ( (usRC == NO_ERROR) && usCount )
    {
      PPROPFOLDER pProp = NULL;
      ULONG       ulSize = 0;

      UtlMakeEQFPath( szSource, NULC, PROPERTY_PATH, NULL );
      strcat( szSource, BACKSLASH_STR );
      strcat( szSource, RESBUFNAME(stFile) );
      if ( UtlLoadFileL( szSource, (PVOID *)&pProp, &ulSize, FALSE, FALSE ) )
      {
        if ( pProp->fDisabled_UserExitRefresh )
        {
          pProp->fDisabled_UserExitRefresh = FALSE;
          UtlWriteFileL( szSource, ulSize, pProp, FALSE );
        } /* endif */           
        UtlAlloc( (PVOID *)&pProp, 0, 0, NOMSG );
      } /* endif */     

      // next property file
      usRC = UtlFindNext( hSearch, &stFile, sizeof(stFile), &usCount, FALSE );
    } /* endwhile */
    if ( hSearch != HDIR_CREATE ) UtlFindClose( hSearch, FALSE );
  } /* endif */     
} /* end of function PerformPendingUpdates */


void  ConvertAnsi2UTF8( char *pszInput, char *pszOutput, int iOutLen ) 
{
   CHAR_W  szTempW[150] ;
   char    *pszOut ;

   if ( pszOutput )
      pszOut = pszOutput ;
   else
      pszOut = pszInput ;

   MultiByteToWideChar( CP_ACP, 0, (LPCSTR)pszInput, -1, (LPWSTR)szTempW, sizeof(szTempW) );
   WideCharToMultiByte( CP_UTF8, 0, (LPWSTR)szTempW, -1, (LPSTR)pszOut, iOutLen, NULL, NULL );
}
