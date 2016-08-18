/*! EQFSETUP.H - Header file for EQFSETUP.C
	Copyright (c) 1990-2012, International Business Machines Corporation and others. All rights reserved.
*/

#ifndef INCL_EQF_SETUP
  #define INCL_EQF_SETUP
//------ macro to initialize the property head area
#define SETPROPHEAD( Head, name, class )                      \
  { strcpy( Head.szName, name );                              \
    sprintf( Head.szPath, "%c:\\%s", chPrimaryDrive, PATH );  \
    Head.usClass = class;                                     \
    Head.chType  = PROP_TYPE_INSTANCE; }


#define PATH           "OTM"           // eqf system directory
#define CTRLDIR        "CTRL"          // control directory
/**********************************************************************/
/* setup different program directories                                */
/**********************************************************************/
#define PROGDIR    "WIN"           // program directory
#define DLLDIR         "DLL"           // DLL directory
#define MEMDIR         "MEM"           // memory database directory
#define DICTDIR        "DICT"          // dictionary directory
#define LISTDIR        "LIST"          // list directory
#define MSGDIR         "MSG"           // message and help directory
#define PRTDIR         "PRTFORM"       // print form directory

#define WINDIR         "WIN"         // Windows DLLs and EXEs directory
#define DOCUDIR        "BOOK"          // documentation directory
#define TABLEDIR       "TABLE"         // fromat table directory
#define PROPDIR        "PROPERTY"      // property directory
#define EXPORTDIR      "EXPORT"        // export directory
#define BACKUPDIR      "BACKUP"        // backup directory
#define SOURCEDIR      "SOURCE"        // source directory
#define SEGSOURCEDIR   "SSOURCE"       // segmented source directory
#define TARGETDIR      "TARGET"        // target directory
#define SEGTARGETDIR   "STARGET"       // segment target directory
#define IMPORTDIR      "IMPORT"        // path for temporary files during import
#define COMMEMDIR      "COMMEM"        // path for server memory files
#define COMPROPDIR     "COMPROP"       // path for server memory property files
#define COMDICTDIR     "COMDICT"       // path for server memory property files
#define SNOMATCHDIR    "SNOMATCH"      // directory for export of SNOMATCH files
#define EADATADIR      "EADATA"        // EA data
#define PLUGINDIR      "PLUGINS"       // path for plugins

#define MSGFILE        "OTM.MSG"       // name of message file
#define HLPFILE        "OTM.HLP"       // name of help file



/**********************************************************************/
/* General constants                                                  */
/**********************************************************************/
#define MAX_STR_LEN      4096           // max. size of a CONFIG.SYS line

/**********************************************************************/
/* defines for "QUICKSTART" Folder                                    */
/**********************************************************************/
#define FOL1           "QUICK.F00"     // first Folder opened after install
#define FOL1_DOC1      "DEVICE.SCR"    // first doc assigned FOLDER1
#define FOL1_DOC2      "TRANS.DOC"     // second doc assigned FOLDER1
#define FOL1_DICTPROP  "QUICK.PRO"     // prop file of dict assigned FOLDER1
#define FOL1_MEMPROP   "QUICK.MEM"     // prop file of TM assigned FOLDER1

#define QUICKTOUR_FOL  "SHOWME.F00"    // name of Quick Tour folder

/**********************************************************************/
/* defines for UpdatePropFiles                                        */
/**********************************************************************/
typedef enum _UPDPROPTYPE
{
   UPD_FOLDER_PROP,
   UPD_DOCUMENT_PROP,
   UPD_DICT_PROP,
   UPD_TM_PROP,
   UPD_SYSTEM_PROP
}  UPDPROPTYPE;


static USHORT DeletePropFile( CHAR chDrive, PSZ pszName );
static USHORT SetupCreateDir( CHAR chDrive, PSZ pFolder, USHORT usPathID );
       VOID   BuildPath( PSZ pszBuffer, CHAR chEqfDrive, PSZ pszFolder, USHORT usPathID );
static USHORT WritePropFile ( CHAR chDrive, PSZ pszName, PVOID pProp, USHORT usSize );
static VOID             DeleteFiles( PSZ, USHORT );
static USHORT           ErrorMessage( USHORT, PSZ, USHORT );
static BOOL             FileExists ( PSZ );
static BOOL             InstCheckAndAdd( PSZ *, PSZ, USHORT );
       PSZ              InstLocateString ( PSZ, PSZ );
       PSZ              InstMatch ( PSZ, PSZ );
static USHORT           ReadAndPrepareConfigSys ( VOID );
       USHORT           UpdateConfigSys ( VOID );

__declspec(dllexport)
         USHORT SetupMAT( HAB hab, CHAR chPrimaryDrive, PSZ  pszSecondaryDrives, CHAR chLanDrive );
static   USHORT CreateEditProperties( CHAR );
static   USHORT CreateFolderListProperties( CHAR );
static   USHORT CreateImexProperties( CHAR );
static   USHORT CreateSystemProperties( CHAR, PSZ, CHAR );
static   USHORT UpdateSystemProperties ( CHAR chPrimaryDrive, CHAR chLanDrive ); /* $KIT0890 A */
static   USHORT CreateDictProperties( CHAR chPrimaryDrive );
         USHORT SetupTmCom (CHAR chPrimaryDrive, PSZ pszSecondaryDrives);
         USHORT SetupDemo  (HAB hab, CHAR chPrimaryDrive, PSZ pszSecondaryDrives, CHAR chLanDrive);
         USHORT SetupDocu  (HAB hab, CHAR chPrimaryDrive, PSZ pszSecondaryDrives);

static VOID DeleteDictCacheFiles( CHAR chPrimaryDrive, USHORT usPathId );
static BOOL UtlPropFileExist( CHAR chDrive, PSZ pszFile );


       USHORT   UpdatePropFile( USHORT usPropType ,CHAR chDrive,
                         PSZ pszFileName, PSZ pszDocName );
static USHORT   UpdateFolderProp( PSZ pszFullFileName, CHAR  chDrive );
static USHORT   UpdateDocumentProp( PSZ pszFullFileName, CHAR  chDrive );
static USHORT   UpdateDictProp( PSZ pszFullFileName, CHAR  chDrive );
static USHORT   UpdateTMProp( PSZ pszFullFileName, CHAR  chDrive );
static USHORT   UpdateSystemProp( PSZ pszFullFileName, PSZ pszFolderName );

       BOOL     SetupUtils( HAB, PSZ );
__declspec(dllexport)
       BOOL     TWBInit( HAB );
       BOOL     TwbStartHandler( HAB, PSZ, PFNWP, PHWND );
       BOOL     TwbStartListHandler( PSZ, PFN_HANDLERCALLBACK, PHWND );
#endif
