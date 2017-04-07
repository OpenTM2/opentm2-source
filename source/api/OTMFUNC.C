/*! \brief OTMFUNC.C
	Copyright (c) 1999-2016, International Business Machines Corporation and others. All rights reserved.
*/

#include <time.h>
#include <process.h>
#define INCL_EQF_FOLDER                // folder functions
#define INCL_EQF_ANALYSIS              // analysis functions
#define INCL_EQF_DLGUTILS              // dialog utilities (for EQFTMI.H!)
#define INCL_EQF_TM                    // general Transl. Memory functions
#define DLLIMPORTRESMOD              // resource module handle imported from DLL
#include "EQF.H"

#include "eqftops.h"
#include "OTMFUNC.H"
#include "EQFFUNCI.H"
#include "EQFSTART.H"                  // for TwbGetCheckProfileData
#define INCL_EQFMEM_DLGIDAS            // include dialog IDA definitions
#include <EQFTMI.H>                    // Private header file of Translation Memory
#include "eqfsetup.h"                  // directory names
#include "eqfrpt.h"
#include "eqfserno.h"
#include "tools\common\InitPlugins.h"
#include "core\PluginManager\PluginManager.h"    // Add for P403138
#include "core\memory\MemoryFactory.h"
#include "core\utilities\LanguageFactory.h"


//#define SESSIONLOG

  //#define DEBUGAPI

#ifdef DEBUGAPI
  FILE *hLog = NULL;
  char szLogFile[512];
  BOOL fLog = FALSE;
  int iLogLevel = 0;
  #define EQFAPI_TRIGGER "EQFAPI.TRG"
  #define EQFAPI_LOGFILE "EQFAPI.LOG"
#endif


#ifdef DEBUGAPI
  #define LOGFORCEWRITE() if ( fLog ) { fclose(hLog); hLog = fopen( szLogFile, "a" ); }
#else
  #define LOGFORCEWRITE()
#endif

#ifdef DEBUGAPI
  #define LOGWRITE1( p1 ) { if ( fLog && hLog ) { fprintf( hLog, p1 ); LOGFORCEWRITE(); } }
#else
  #define LOGWRITE1( p1 )
#endif

#ifdef DEBUGAPI
  #define LOGWRITE2( p1, p2 ) { if ( fLog && hLog ) { fprintf( hLog, p1, p2 ); LOGFORCEWRITE(); } }
#else
  #define LOGWRITE2( p1, p2 )
#endif

#ifdef DEBUGAPI
  #define LOGWRITE3( p1, p2, p3 ) { if ( fLog && hLog ) { fprintf( hLog, p1, p2, p3 ); LOGFORCEWRITE(); } }
#else
  #define LOGWRITE3( p1, p2, p3 )
#endif

#ifdef DEBUGAPI
  #define LOGWRITE4( p1, p2, p3, p4 ) { if ( fLog && hLog ) { fprintf( hLog, p1, p2, p3, p4 ); LOGFORCEWRITE(); }}
#else
  #define LOGWRITE4( p1, p2, p3, p4 )
#endif

#ifdef DEBUGAPI
  #define LOGWRITE5( p1, p2, p3, p4, p5 ) { if ( fLog && hLog ) { fprintf( hLog, p1, p2, p3, p4, p5 ); LOGFORCEWRITE(); } }
#else
  #define LOGWRITE5( p1, p2, p3, p4, p5 )
#endif

#ifdef DEBUGAPI
  #define LOGPARMSTRING( name, value ) { if ( fLog && hLog ) { fprintf( hLog, "  %s=\"%s\"\n", name, (value) ? value : "<NULL>" ); LOGFORCEWRITE(); } }
#else
  #define LOGPARMSTRING( name, value )
#endif

#ifdef DEBUGAPI
  #define LOGPARMSTRINGW( name, value ) { if ( fLog && hLog ) { fprintf( hLog, "  %S=\"%s\"\n", name, (value) ? value : "<NULL>" ); LOGFORCEWRITE(); } }
#else
  #define LOGPARMSTRINGW( name, value )
#endif

#ifdef DEBUGAPI
  #define LOGPARMCHAR( name, value ) { if ( fLog && hLog ) { fprintf( hLog, "  %s=\'%c\'\n", name, (value) ? value : ' ' ); LOGFORCEWRITE(); } }
#else
  #define LOGPARMCHAR( name, value )
#endif

#ifdef DEBUGAPI
  #define LOGPARMOPTION( name, value ) { if ( fLog && hLog ) { fprintf( hLog, "  %s=%8.8lX\n", name, value ); LOGFORCEWRITE(); }}
#else
  #define LOGPARMOPTION( name, value )
#endif

#ifdef DEBUGAPI
  #define LOGPARMUSHORT( name, value ) { if ( fLog && hLog ) { fprintf( hLog, "  %s=%u\n", name, value ); LOGFORCEWRITE(); }}
#else
  #define LOGPARMUSHORT( name, value )
#endif

#ifdef DEBUGAPI
  #define LOGPARMLONG( name, value ) { if ( fLog && hLog ) { fprintf( hLog, "  %s=%ld\n", name, value ); LOGFORCEWRITE(); }}
#else
  #define LOGPARMLONG( name, value )
#endif


#ifdef DEBUGAPI
  #define LOGCLOSE() { if ( hLog ) fclose( hLog ); hLog = NULLHANDLE; }
#else
  #define LOGCLOSE()
#endif



// create a new folder
USHORT EqfCreateFolder
(
  HSESSION    hSession,                // Eqf session handle
  PSZ         pszFolderName,           // name of folder
  PSZ         pszDescription,          // folder description or NULL
  CHAR        chTargetDrive,           // folder target drive
  PSZ         pszMemname,              // folder Translation Memory
  PSZ         pszMarkup,               // folder markup
  PSZ         pszEditor,               // folder editor
  PSZ         pszDictionaries,         // list of dictionaries or NULL
  PSZ         pszSourceLanguage,       // folder source language
  PSZ         pszTargetLanguage,       // folder target language
  PSZ         pszConversion,           // document export conversion
  PSZ         pszReadOnlyMems          // list of readonly TMs or NULL
)
{
  USHORT      usRC = NO_ERROR;         // function return code
  hSession;

  LOGWRITE1( "==EQFCreateFolder==\n" );
  LOGPARMSTRING("Folder", pszFolderName );
  LOGPARMSTRING("Description", pszDescription );
  LOGPARMCHAR( "TargetDrive", chTargetDrive );
  LOGPARMSTRING("Memory", pszMemname );
  LOGPARMSTRING("Markup", pszMarkup );
  LOGPARMSTRING("Editor", pszEditor );
  LOGPARMSTRING("Dicts", pszDictionaries );
  LOGPARMSTRING("Sourcelang", pszSourceLanguage );
  LOGPARMSTRING("TargetLang", pszTargetLanguage );
  LOGPARMSTRING("Conversion", pszConversion );
  LOGPARMSTRING("ReadOnlyMems", pszReadOnlyMems );

  usRC = FolFuncCreateFolder( pszFolderName, pszDescription, chTargetDrive, // XXX
                              pszMemname, pszMarkup, pszEditor,
                              pszDictionaries, pszSourceLanguage,
                              pszTargetLanguage, pszConversion,
                              pszReadOnlyMems );
  if ( usRC == NO_ERROR )
  {
      SetSharingFlag( EQF_REFR_FOLLIST );
  }

  LOGWRITE2( "  RC=%u\n", usRC );

  return( usRC );
} /* end of function EqfCreateFolder */

// create a new controlled folder
USHORT EqfCreateControlledFolder
(
  HSESSION    hSession,                // Eqf session handle
  PSZ         pszFolderName,           // name of folder
  PSZ         pszDescription,          // folder description or NULL
  CHAR        chTargetDrive,           // folder target drive
  PSZ         pszMemname,              // folder Translation Memory
  PSZ         pszMarkup,               // folder markup
  PSZ         pszEditor,               // folder editor
  PSZ         pszDictionaries,         // list of dictionaries or NULL
  PSZ         pszSourceLanguage,       // folder source language
  PSZ         pszTargetLanguage,       // folder target language
  PSZ         pszConversion,           // document export conversion
  PSZ         pszReadOnlyMems,         // list of readonly TMs or NULL
  PSZ         pszPassword,             // password
  PSZ         pszProjCoordName,        // name of the project coordinator
  PSZ         pszProjCoordMail,        // project coordinator's mail
  PSZ         pszTranslatorName,       // name of the translator
  PSZ         pszTranslatorMail,       // translator's mail
  PSZ         pszProductName,          // Name of the product
  PSZ         pszProductFamily,        // Product Famiily
  PSZ         pszSimilarProduct,       // Similar Product Family
  PSZ         pszProductDict,          // Product subject area dictionary
  PSZ         pszProductMem,           // Product subject area memory
  PSZ         pszPreviousVersion,      // Previous version of the product
  PSZ         pszVersion,              // Version of the Product
  PSZ         pszShipmentNumber        // Shipment number
)
{
  USHORT      usRC = NO_ERROR;         // function return code
  hSession;
  LOGWRITE1( "==EQFCreateControlledFolder==\n" );

  usRC = FolFuncCreateControlledFolder( pszFolderName, pszDescription, chTargetDrive, // XXX
                                        pszMemname, pszMarkup, pszEditor,
                                        pszDictionaries, pszSourceLanguage,
                                        pszTargetLanguage, pszConversion,
                                        pszReadOnlyMems, pszPassword, pszProjCoordName,
                                        pszProjCoordMail, pszTranslatorName, pszTranslatorMail,
                                        pszProductName, pszProductFamily, pszSimilarProduct,
                                        pszProductDict, pszProductMem, pszPreviousVersion,
                                        pszVersion, pszShipmentNumber);

  if ( usRC == NO_ERROR )
  {
      SetSharingFlag( EQF_REFR_FOLLIST );
  }

  LOGWRITE2( "  RC=%u\n", usRC );

  return( usRC );
} /* end of function EqfCreateControlledFolder */





USHORT EqfChangeFolProps
(
  HSESSION    hSession,                // Eqf session handle
  PSZ         pszFolderName,           // name of folder
  CHAR        chTargetDrive,           // target drive
  PSZ         pszTargetLanguage,       // folder target language
  PSZ         pszMemName,              // folder Translation Memory or NULL
  PSZ         pszDictionaries          // list of dictionaries or NULL

)
{
  USHORT      usRC = NO_ERROR;         // function return code
   hSession;
  LOGWRITE1( "==EQFChangeFolProps==\n" );

  usRC = FolFuncChangeFolProps( pszFolderName, chTargetDrive, pszTargetLanguage, pszMemName, pszDictionaries, NULL, NULL, NULL, NULL );

  LOGWRITE2( "  RC=%u\n", usRC );

  return( usRC );
} /* end of function EqfChangeFolProps */

USHORT EqfChangeFolPropsEx
(
  HSESSION    hSession,                // Eqf session handle
  PSZ         pszFolderName,           // name of folder
  CHAR        chTargetDrive,           // target drive
  PSZ         pszTargetLanguage,       // target language or NULL
  PSZ         pszMemName,              // folder Translation Memory or NULL
  PSZ         pszDictionaries,         // list of dictionaries or NULL
  PSZ         pszROMemories,           // list of read-only search memories or NULL
  PSZ         pszDescription,          // folder description or NULL
  PSZ         pszProfile,              // calculation report name or NULL
  PSZ         pszShipment              // folder shipment nuber or NULL
)
{
  USHORT      usRC = NO_ERROR;         // function return code
   hSession;
  LOGWRITE1( "==EQFChangeFolPropsEx==\n" );

  usRC = FolFuncChangeFolProps( pszFolderName, chTargetDrive, pszTargetLanguage, pszMemName, pszDictionaries, pszROMemories,
                                pszDescription, pszProfile, pszShipment );


  LOGWRITE2( "  RC=%u\n", usRC );

  return( usRC );
} /* end of function EqfChangeFolProps */



USHORT EqfCreateSubFolder
(
  HSESSION    hSession,                // Eqf session handle
  PSZ         pszParentFolder,         // name of parent (sub)folder
  PSZ         pszSubFolderName,        // subfolders name
  PSZ         pszMemName,              // subfolders Translation Memory or NULL
  PSZ         pszMarkup,               // name of Markup used for subfolder
  PSZ         pszSourceLanguage,       // Source Language used for subfolder
  PSZ         pszTargetLanguage,       // Target Language used for subfolder
  PSZ         pszEditor,
  PSZ         pszConversion,           // Conversion used for subfolder
  PSZ         pszTranslator,           // Name of translator
  PSZ         pszTranslatorMail        // Mail of translator
)
{
  USHORT      usRC = NO_ERROR;         // function return code

  hSession;
  LOGWRITE1( "==EQFCreateSubFolder==\n" );

  usRC = FolFuncCreateSubFolder( pszParentFolder, pszSubFolderName, pszMemName, pszMarkup,
                                 pszSourceLanguage, pszTargetLanguage, pszEditor, pszConversion,
                                 pszTranslator, pszTranslatorMail);


  if ( usRC == NO_ERROR )
  {
      SetSharingFlag( EQF_REFR_FOLLIST );
  }

  LOGWRITE2( "  RC=%u\n", usRC );

  return( usRC );
} /* end of function EqfCreateSubFolder*/




// export a folder, old fashioned
USHORT EqfExportFolder
(
  HSESSION    hSession,                // Eqf session handle
  PSZ         pszFolderName,           // name of folder
  CHAR        chTargetDrive,           // folder target drive
  LONG        lOptions,                // options for the folder export or 0L
  PSZ         pszDocuments,            // list of documents or NULL
  PSZ         pszDescription           // export description or NULL
)
{
  // use the newer interface
  USHORT      usRC = NO_ERROR;
  CHAR        chTempPath[MAX_EQF_PATH];
  PSZ         pszTemp;
  PFCTDATA    pData = NULL;            // ptr to function data area

  // validate session handle
  usRC = FctValidateSession( hSession, &pData );

  if ( pData && (pData->fComplete || (pData->sLastFunction != FCT_EQFEXPORTFOLDER)) )
  {
    LOGWRITE1( "==EQFExportFolder==\n" );
    LOGPARMSTRING( "Folder", pszFolderName );
    LOGPARMCHAR( "TargetDrive", chTargetDrive );
    LOGPARMOPTION( "Options", lOptions );
    LOGPARMSTRING( "Documents", pszDocuments );
    LOGPARMSTRING( "Description", pszDescription );
  } /* endif */

  // check if drive is valid

  sprintf(chTempPath, "%c", chTargetDrive);

  if (strlen(chTempPath) == 0)
  {
    usRC = ERROR_NO_TARGETDRIVE_SELECTED;
  }
  else if(!UtlDirExist(chTempPath))
  {
     usRC = ERROR_EQF_DRIVE_NOT_VALID;
  }

  if (usRC != NO_ERROR)
  {
     pszTemp = chTempPath;
     UtlErrorHwnd( usRC, MB_CANCEL, 1, &pszTemp, EQF_ERROR, HWND_FUNCIF );
     return (usRC);
  }

  UtlMakeEQFPath( chTempPath, chTargetDrive, EXPORT_PATH, NULL );

  usRC =  InternalExportFolder( hSession, pszFolderName, chTempPath,
                                lOptions, pszDocuments, pszDescription, NULL, NULL);

  if ( (usRC == NO_ERROR) && (lOptions & DELETE_OPT) )
  {
      SetSharingFlag( EQF_REFR_FOLLIST );
  }


  if ( pData && pData->fComplete )
  {
    LOGWRITE2( "  RC=%u\n", usRC );
  } /* endif */

  return ( usRC );
} /* end of function EqfExportFolder */

// export a folder, new interface
USHORT EqfExportFolderFP
(
  HSESSION    hSession,                // Eqf session handle
  PSZ         pszFolderName,           // name of folder
  PSZ         chTargetPath,            // folder target path
  LONG        lOptions,                // options for the folder export or 0L
  PSZ         pszDocuments,            // list of documents or NULL
  PSZ         pszDescription           // export description or NULL
)
{
  USHORT usRC = NO_ERROR;
  PFCTDATA    pData = NULL;            // ptr to function data area

  // validate session handle
  usRC = FctValidateSession( hSession, &pData );

  if ( pData && (pData->fComplete || (pData->sLastFunction != FCT_EQFEXPORTFOLDER)) )
  {
    LOGWRITE1( "==EQFExportFolderFP==\n" );
    LOGPARMSTRING( "Folder", pszFolderName );
    LOGPARMSTRING( "TargetPath", chTargetPath );
    LOGPARMOPTION( "Options", lOptions );
    LOGPARMSTRING( "Documents", pszDocuments );
    LOGPARMSTRING( "Description", pszDescription );
  } /* endif */

  usRC = InternalExportFolder(hSession, pszFolderName, chTargetPath, lOptions,
            pszDocuments, pszDescription, NULL, NULL);

  if ( (usRC == NO_ERROR) && (lOptions & DELETE_OPT) )
  {
      SetSharingFlag( EQF_REFR_FOLLIST );
  }


  if ( pData && pData->fComplete )
  {
    LOGWRITE2( "  RC=%u\n", usRC );
  } /* endif */

  return( usRC );
} /* end of function EqfExportFolderFP */

// export a folder, new interface
USHORT EqfExportFolderFPas
(
  HSESSION    hSession,                // Eqf session handle
  PSZ         pszFolderName,           // name of folder
  PSZ         chTargetPath,            // folder target path
  PSZ         pszExportAs,             // export folder as: new folder name
  LONG        lOptions,                // options for the folder export or 0L
  PSZ         pszDocuments,            // list of documents or NULL
  PSZ         pszDescription,           // export description or NULL
  PSZ		  pszMemoryExportAs         // export folder memory with this name or NULL
)
{
  USHORT usRC = NO_ERROR;
  PFCTDATA    pData = NULL;            // ptr to function data area

  // validate session handle
  usRC = FctValidateSession( hSession, &pData );

  if ( pData && (pData->fComplete || (pData->sLastFunction != FCT_EQFEXPORTFOLDER)) )
  {
    LOGWRITE1( "==EQFExportFolderFPas==\n" );
    LOGPARMSTRING( "Folder", pszFolderName );
    LOGPARMSTRING( "TargetPath", chTargetPath );
    LOGPARMOPTION( "Options", lOptions );
    LOGPARMSTRING( "Documents", pszDocuments );
    LOGPARMSTRING( "Description", pszDescription );
    LOGPARMSTRING( "ExportAs", pszExportAs );
    LOGPARMSTRING( "MemoryExportAs", pszMemoryExportAs );
  } /* endif */

  usRC = InternalExportFolder(hSession, pszFolderName, chTargetPath, lOptions,
            pszDocuments, pszDescription, pszExportAs, pszMemoryExportAs);

  if ( (usRC == NO_ERROR) && (lOptions & DELETE_OPT) )
  {
      SetSharingFlag( EQF_REFR_FOLLIST );
  }


  if ( pData && pData->fComplete )
  {
    LOGWRITE2( "  RC=%u\n", usRC );
  } /* endif */

  return( usRC );
} /* end of function EqfExportFolderFPas */

USHORT InternalExportFolder
(
  HSESSION    hSession,                // Eqf session handle
  PSZ         pszFolderName,           // name of folder
  PSZ         chTargetPath,            // folder target path
  LONG        lOptions,                // options for the folder export or 0L
  PSZ         pszDocuments,            // list of documents or NULL
  PSZ         pszDescription,          // export description or NULL
  PSZ         pszExportAs,
  PSZ         pszMemoryExportAs        // export folder memory with this name
)
{
  USHORT      usRC = NO_ERROR;         // function return code
  PFCTDATA    pData = NULL;            // ptr to function data area

  // validate session handle
  usRC = FctValidateSession( hSession, &pData );

  // check sequence of calls
  if ( usRC == NO_ERROR )
  {
    if ( !pData->fComplete && (pData->sLastFunction != FCT_EQFEXPORTFOLDER) )
    {
      usRC = LASTTASK_INCOMPLETE_RC;
    } /* endif */
  } /* endif */

  // call folder export process
  if ( usRC == NO_ERROR )
  {
    pData->sLastFunction = FCT_EQFEXPORTFOLDER;
    usRC = (USHORT)FolFuncExportFolder( pData, pszFolderName, chTargetPath,
                                lOptions, pszDocuments, pszDescription, pszExportAs,
                                pszMemoryExportAs);
  } /* endif */

  if ( (usRC == NO_ERROR) && !pData->fComplete )
  {
    usRC = CONTINUE_RC;
  } /* endif */

  return( usRC );
} /* end of function InternalExportFolder */

// count the words of documents
USHORT EqfCountWords
(
  HSESSION    hSession,                // Eqf session handle
  PSZ         pszFolderName,           // name of folder
  PSZ         pszDocuments,            // list of documents or NULL
  LONG        lOptions,                // options for the word count
  PSZ         pszOutFile               // fully qualified output file
)
{
  USHORT      usRC = NO_ERROR;         // function return code
  PFCTDATA    pData = NULL;            // ptr to function data area

  // validate session handle
  usRC = FctValidateSession( hSession, &pData );

  if ( pData && (pData->fComplete || (pData->sLastFunction != FCT_EQFCOUNTWORDS)) )
  {
    LOGWRITE1( "==EQFCountWords==\n" );
    LOGPARMSTRING( "Folder", pszFolderName );
    LOGPARMSTRING( "Documents", pszDocuments );
    LOGPARMOPTION( "Options", lOptions );
    LOGPARMSTRING( "OutFile", pszOutFile );
#ifdef SESSIONLOG
    UtlLogWriteString( "EqfCountWords: Starting, Folder = %s", pszFolderName );
#endif
  } /* endif */


  // check sequence of calls
  if ( usRC == NO_ERROR )
  {
    if ( !pData->fComplete && (pData->sLastFunction != FCT_EQFCOUNTWORDS) )
    {
      usRC = LASTTASK_INCOMPLETE_RC;
    } /* endif */
  } /* endif */

  // call word count process
  if ( usRC == NO_ERROR )
  {
    pData->sLastFunction = FCT_EQFCOUNTWORDS;
    usRC = CntFuncCountWords( pData, pszFolderName, pszDocuments,
                             lOptions, pszOutFile );
  } /* endif */

  if ( (usRC == NO_ERROR) && !pData->fComplete )
  {
    usRC = CONTINUE_RC;
  } /* endif */

  if ( pData && pData->fComplete ) 
  {
    LOGWRITE2( "  RC=%u\n", usRC );
#ifdef SESSIONLOG
    CHAR szRC[10];
    sprintf( szRC, "%u", usRC );
    UtlLogWriteString( "EqfCountWords: Process complete, rc=%s", szRC );
#endif
  } /* endif */       

  return( usRC );
} /* end of function EqfCountWords */

// import a dictionary
USHORT EqfImportDict
(
  HSESSION    hSession,                // Eqf session handle
  PSZ         pszInFile,               // fully qualified name of input file
  PSZ         pszDictName,             // name of dictionary
  PSZ         pszPassword,             // password of dictionary
  LONG        lOptions                 // dictionary import options
)
{
  USHORT      usRC = NO_ERROR;         // function return code
  PFCTDATA    pData = NULL;            // ptr to function data area

  LOGWRITE1( "==EQFImportDict==\n" );

  // validate session handle
  usRC = FctValidateSession( hSession, &pData );

  // check sequence of calls
  if ( usRC == NO_ERROR )
  {
    if ( !pData->fComplete && (pData->sLastFunction != FCT_EQFIMPORTDICT) )
    {
      usRC = LASTTASK_INCOMPLETE_RC;
    } /* endif */
  } /* endif */

  // call dictionary import process
  if ( usRC == NO_ERROR )
  {
    pData->sLastFunction = FCT_EQFIMPORTDICT;
    usRC = DicFuncImportDict( pData, pszInFile, pszDictName,
                              pszPassword, lOptions );
  } /* endif */

  if ( (usRC == NO_ERROR) && !pData->fComplete )
  {
    usRC = CONTINUE_RC;
  } /* endif */

  if ( !usRC )
  {
      SetSharingFlag( EQF_REFR_DICLIST );
  }

  LOGWRITE2( "  RC=%u\n", usRC );

  return( usRC );
} /* end of function EqfImportDict */

// export a dictionary
USHORT EqfExportDict
(
  HSESSION    hSession,                // Eqf session handle
  PSZ         pszDictName,             // name of dictionary
  LONG        lOptions,                // dictionary export options or 0L
  PSZ         pszOutFile               // fully qualified name of output file
)
{
  USHORT      usRC = NO_ERROR;         // function return code
  PFCTDATA    pData = NULL;            // ptr to function data area

  LOGWRITE1( "==EQFExportDict==\n" );

  // validate session handle
  usRC = FctValidateSession( hSession, &pData );

  // check sequence of calls
  if ( usRC == NO_ERROR )
  {
    if ( !pData->fComplete && (pData->sLastFunction != FCT_EQFEXPORTDICT) )
    {
      usRC = LASTTASK_INCOMPLETE_RC;
    } /* endif */
  } /* endif */

  // call dictionary export process
  if ( usRC == NO_ERROR )
  {
    pData->sLastFunction = FCT_EQFEXPORTDICT;
    usRC = DicFuncExportDict( pData, pszDictName,lOptions, pszOutFile );
  } /* endif */

  if ( (usRC == NO_ERROR) && !pData->fComplete )
  {
    usRC = CONTINUE_RC;
  } /* endif */

  LOGWRITE2( "  RC=%u\n", usRC );

  return( usRC );
} /* end of function EqfExportDict */

// import a document
USHORT EqfImportDoc
(
  HSESSION    hSession,                // Eqf session handle
  PSZ         pszFolderName,           // name of folder receiving the documents
  PSZ         pszFiles,                // list of input files (documents)
  PSZ         pszMemname,              // document Translation Memory or NULL
  PSZ         pszMarkup,               // document markup or NULL
  PSZ         pszEditor,               // document editor or NULL
  PSZ         pszSourceLanguage,       // document source language or NULL
  PSZ         pszTargetLanguage,       // document target language or NULL
  PSZ         pszAlias,                // alias for document name or NULL
  PSZ         pszStartPath,            // optional start path
  PSZ         pszConversion,           // optional export conversion
  LONG        lOptions                 // document import options or 0L
)
{
  USHORT      usRC = NO_ERROR;         // function return code
  PFCTDATA    pData = NULL;            // ptr to function data area

  // validate session handle
  usRC = FctValidateSession( hSession, &pData );

  if ( pData && (pData->fComplete || (pData->sLastFunction != FCT_EQFIMPORTDOC)) )
  {
    LOGWRITE1( "==EQFImportDoc==\n" );
    LOGPARMSTRING( "Folder", pszFolderName );
    LOGPARMSTRING( "Files", pszFiles );
    LOGPARMSTRING( "Memory", pszMemname );
    LOGPARMSTRING( "Markup", pszMarkup );
    LOGPARMSTRING( "Editor", pszEditor );
    LOGPARMSTRING( "Sourcelang", pszSourceLanguage );
    LOGPARMSTRING( "TargetLang", pszTargetLanguage );
    LOGPARMSTRING( "Alias", pszAlias );
    LOGPARMSTRING( "StartPath", pszStartPath );
    LOGPARMSTRING( "Conversion", pszConversion );
    LOGPARMOPTION( "Options", lOptions );
  } /* endif */



  // check sequence of calls
  if ( usRC == NO_ERROR )
  {
    if ( !pData->fComplete && (pData->sLastFunction != FCT_EQFIMPORTDOC) )
    {
      usRC = LASTTASK_INCOMPLETE_RC;
    } /* endif */
  } /* endif */

  // call document import process
  if ( usRC == NO_ERROR )
  {
    pData->sLastFunction = FCT_EQFIMPORTDOC;
    usRC = DocFuncImportDoc( pData, pszFolderName, pszFiles, pszMemname,
                             pszMarkup, pszEditor,  pszSourceLanguage,
                             pszTargetLanguage, pszAlias, pszStartPath,
                             pszConversion, lOptions );

  } /* endif */

  if ( (usRC == NO_ERROR) && !pData->fComplete )
  {
    usRC = CONTINUE_RC;
  } /* endif */

  if ( pData && pData->fComplete ) LOGWRITE2( "  RC=%u\n", usRC );

  return( usRC );
} /* end of function EqfImportDoc */

// export a document
USHORT EqfExportDoc
(
  HSESSION    hSession,                // Eqf session handle
  PSZ         pszFolderName,           // name of folder
  PSZ         pszFiles,                // list of documents with path information
  PSZ         pszStartPath,            // optional start path
  LONG        lOptions                 // options for document export
)
{
  USHORT      usRC = NO_ERROR;         // function return code
  PFCTDATA    pData = NULL;            // ptr to function data area

  // validate session handle
  usRC = FctValidateSession( hSession, &pData );

  if ( pData && (pData->fComplete || (pData->sLastFunction != FCT_EQFEXPORTDOC)) )
  {
    LOGWRITE1( "==EQFExportDoc==\n" );
    LOGPARMSTRING("Folder", pszFolderName );
    LOGPARMSTRING("Documents", pszFiles );
    LOGPARMSTRING("StartPath", pszStartPath );
    LOGPARMOPTION("Options", lOptions );
  } /* endif */

  // check sequence of calls
  if ( usRC == NO_ERROR )
  {
    if ( !pData->fComplete && (pData->sLastFunction != FCT_EQFEXPORTDOC) )
    {
      usRC = LASTTASK_INCOMPLETE_RC;
    } /* endif */
  } /* endif */

  // call document export process
  if ( usRC == NO_ERROR )
  {
    pData->sLastFunction = FCT_EQFEXPORTDOC;
    usRC = DocFuncExportDoc( pData, pszFolderName, pszFiles, pszStartPath,
                             lOptions );

  } /* endif */

  if ( (usRC == NO_ERROR) && !pData->fComplete )
  {
    usRC = CONTINUE_RC;
  } /* endif */

  if ( pData && pData->fComplete ) LOGWRITE2( "  RC=%u\n", usRC );

  return( usRC );
} /* end of function EqfExportDoc */

// import a Translation Memory
USHORT EqfImportMem
(
  HSESSION    hSession,                // Eqf session handle
  PSZ         pszMemName,              // name of Translation Memory
  PSZ         pszInFile,               // fully qualified name of input file
  LONG        lOptions                 // options for Translation Memory import
)
{
  USHORT      usRC = NO_ERROR;         // function return code
  PFCTDATA    pData = NULL;            // ptr to function data area


  // validate session handle
  usRC = FctValidateSession( hSession, &pData );

  if ( pData && (pData->fComplete || (pData->sLastFunction != FCT_EQFIMPORTMEM)) )
  {
    LOGWRITE1( "==EQFImportMem==\n" );
    LOGPARMSTRING("Memory", pszMemName );
    LOGPARMSTRING("InFile", pszInFile );
    LOGPARMOPTION("Options", lOptions );
  } /* endif */


  // check sequence of calls
  if ( (usRC == NO_ERROR ) && !(lOptions & COMPLETE_IN_ONE_CALL_OPT) )
  {
    if ( !pData->fComplete && (pData->sLastFunction != FCT_EQFIMPORTMEM) )
    {
      usRC = LASTTASK_INCOMPLETE_RC;
    } /* endif */
  } /* endif */

  // call TM import
  if ( usRC == NO_ERROR )
  {
    if ( !( lOptions & COMPLETE_IN_ONE_CALL_OPT ) ) pData->sLastFunction = FCT_EQFIMPORTMEM;
    usRC = MemFuncImportMem( pData, pszMemName, pszInFile, NULL, NULL, NULL, NULL, lOptions );
  } /* endif */

  if ( !( lOptions & COMPLETE_IN_ONE_CALL_OPT ) )
  {
    if ( ( usRC == NO_ERROR ) && !pData->fComplete )
    {
      usRC = CONTINUE_RC;
    } /* endif */
  }

  if ( !usRC )
  {
      SetSharingFlag( EQF_REFR_MEMLIST );
  }

  if ( pData && (pData->fComplete || (lOptions & COMPLETE_IN_ONE_CALL_OPT ) ) ) LOGWRITE2( "  RC=%u\n", usRC );

  return( usRC );
} /* end of function EqfImportMem */

USHORT EqfImportMemEx
(
  HSESSION    hSession,                // mand: Eqf session handle
  PSZ         pszMemName,              // mand: name of Translation Memory
  PSZ         pszInFile,               // mand: fully qualified name of input file
  PSZ         pszTM_ID,                // translation memory ID or NULL if not used
  PSZ         pszStoreID,              // ID for the origin of the translation memory or NULL if not used
  PSZ         pszUnused1,              // not in use, for future enhancements
  PSZ         pszUnused2,              // not in use, for future enhancements
  LONG        lOptions                 // opt: options for Translation Memory import
                                       // @Import Mode: TMX_OPT{CLEANRTF_OPT}, XLIFF_MT_OPT,UTF16_OPT,ANSI_OPT,ASCII_OPT(default)
                                       // @Markup Table Handling: CANCEL_UNKNOWN_MARKUP_OPT(default), SKIP_UNKNOWN_MARKUP_OPT, GENERIC_UNKNOWN_MARKUP_OPT
									                     // @Other: {CLEANRTF_OPT,IGNORE_OPT,FORCENEWMATCHID_OPT}
)
{
  USHORT      usRC = NO_ERROR;         // function return code
  PFCTDATA    pData = NULL;            // ptr to function data area


  // validate session handle
  usRC = FctValidateSession( hSession, &pData );

  if ( pData && (pData->fComplete || (pData->sLastFunction != FCT_EQFIMPORTMEMEX)) )
  {
    LOGWRITE1( "==EQFImportMemEx==\n" );
    LOGPARMSTRING("Memory", pszMemName );
    LOGPARMSTRING("InFile", pszInFile );
    LOGPARMSTRING("TM_ID", pszTM_ID );
    LOGPARMSTRING("StoreID", pszStoreID );
    LOGPARMOPTION("Options", lOptions );
  } /* endif */


  // check sequence of calls
  if ( usRC == NO_ERROR )
  {
    if ( !pData->fComplete && (pData->sLastFunction != FCT_EQFIMPORTMEMEX) )
    {
      usRC = LASTTASK_INCOMPLETE_RC;
    } /* endif */
  } /* endif */

  // call TM import
  if ( usRC == NO_ERROR )
  {
    pData->sLastFunction = FCT_EQFIMPORTMEMEX;
    usRC = MemFuncImportMem( pData, pszMemName, pszInFile, pszTM_ID, pszStoreID, pszUnused1, pszUnused2, lOptions );
    if ( lOptions & COMPLETE_IN_ONE_CALL_OPT )
    {
      if ( ( usRC == NO_ERROR ) && !pData->fComplete ) usRC = CONTINUE_RC;
      while ( usRC == CONTINUE_RC )
      {
        usRC = MemFuncImportMem( pData, pszMemName, pszInFile, pszTM_ID, pszStoreID, pszUnused1, pszUnused2, lOptions );
        if ( ( usRC == NO_ERROR ) && !pData->fComplete ) usRC = CONTINUE_RC;
      } /*endwhile */
    } /* endif */

  } /* endif */

  if ( (usRC == NO_ERROR) && !pData->fComplete )
  {
    usRC = CONTINUE_RC;
  } /* endif */

  if ( !usRC )
  {
      SetSharingFlag( EQF_REFR_MEMLIST );
  }

  if ( pData && pData->fComplete ) LOGWRITE2( "  RC=%u\n", usRC );

  return( usRC );
} /* end of function EqfImportMemEx */


// export a Translation Memory
USHORT EqfExportMem
(
  HSESSION    hSession,                // Eqf session handle
  PSZ         pszMemName,              // name of Translation Memory
  PSZ         pszOutFile,              // fully qualified name of output file
  LONG        lOptions                 // options for Translation Memory export
)
{
  USHORT      usRC = NO_ERROR;         // function return code
  PFCTDATA    pData = NULL;            // ptr to function data area


  // validate session handle
  usRC = FctValidateSession( hSession, &pData );

  if ( pData && (pData->fComplete || (pData->sLastFunction != FCT_EQFEXPORTMEM) || (lOptions & COMPLETE_IN_ONE_CALL_OPT)) )
  {
    LOGWRITE1( "==EQFExportMem==\n" );
    LOGPARMSTRING( "Memory", pszMemName );
    LOGPARMSTRING( "OutFile", pszOutFile );
    LOGPARMOPTION( "Options", lOptions );
  } /* endif */

  // check sequence of calls
  if ( (usRC == NO_ERROR) && !( lOptions & COMPLETE_IN_ONE_CALL_OPT ) )
  {
    if ( !pData->fComplete && (pData->sLastFunction != FCT_EQFEXPORTMEM) )
    {
      usRC = LASTTASK_INCOMPLETE_RC;
    } /* endif */
  } /* endif */

  // call TM export
  if ( usRC == NO_ERROR )
  {
    if ( !( lOptions & COMPLETE_IN_ONE_CALL_OPT ) ) pData->sLastFunction = FCT_EQFEXPORTMEM;
    usRC = MemFuncExportMem( pData, pszMemName, pszOutFile, lOptions );
  } /* endif */

  if ( !( lOptions & COMPLETE_IN_ONE_CALL_OPT ) && (usRC == NO_ERROR) && !pData->fComplete )
  {
    usRC = CONTINUE_RC;
  } /* endif */

  if ( pData && (pData->fComplete || ( lOptions & COMPLETE_IN_ONE_CALL_OPT ) ) ) LOGWRITE2( "  RC=%u\n", usRC );

  return( usRC );
} /* end of function EqfExportMem */



// organize a Translation Memory
USHORT EqfOrganizeMem
(
  HSESSION    hSession,                // Eqf session handle
  PSZ         pszMemName               // name of Translation Memory
)
{
  USHORT      usRC = NO_ERROR;         // function return code
  PFCTDATA    pData = NULL;            // ptr to function data area

  LOGWRITE1( "==EQFOrganizeMem==\n" );

  // validate session handle
  usRC = FctValidateSession( hSession, &pData );

  // check sequence of calls
  if ( usRC == NO_ERROR )
  {
    if ( !pData->fComplete && (pData->sLastFunction != FCT_EQFORGANIZEMEM) )
    {
      usRC = LASTTASK_INCOMPLETE_RC;
    } /* endif */
  } /* endif */

  // call TM organize
  if ( usRC == NO_ERROR )
  {
    pData->sLastFunction = FCT_EQFORGANIZEMEM;
    usRC = MemFuncOrganizeMem( pData, pszMemName );
  } /* endif */

  if ( (usRC == NO_ERROR) && !pData->fComplete )
  {
    usRC = CONTINUE_RC;
  } /* endif */

  LOGWRITE2( "  RC=%u\n", usRC );

  return( usRC );
} /* end of function EqfExportMem */


// add match segment IDs to an existing Translation Memory
USHORT EqfAddMatchSegID
(
  HSESSION    hSession,                // Eqf session handle
  PSZ         pszMemName,               // name of Translation Memory
  PSZ         pszTM_ID,                // translation memory ID or NULL if not used
  PSZ         pszStoreID,              // ID for the origin of the translation memory or NULL if not used
  LONG        lOptions                 // opt: options for Translation Memory import
)
{
  USHORT      usRC = NO_ERROR;         // function return code
  PFCTDATA    pData = NULL;            // ptr to function data area

  LOGWRITE1( "==EQFAddmatchSegID==\n" );

  // validate session handle
  usRC = FctValidateSession( hSession, &pData );

  // check sequence of calls
  //if ( usRC == NO_ERROR )
  //{
  //  if ( !pData->fComplete && (pData->sLastFunction != FCT_EQFADDMATCHSEGID) )
  //  {
  //    usRC = LASTTASK_INCOMPLETE_RC;
  //  } /* endif */
  //} /* endif */

  // call add match segment ID process function
  if ( usRC == NO_ERROR )
  {
    pData->sLastFunction = FCT_EQFADDMATCHSEGID;
    do
    {
      usRC = MemFuncAddMatchSegID( pData, pszMemName, pszTM_ID, pszStoreID, lOptions);
    } while( (usRC == NO_ERROR) && !pData->fComplete );
  } /* endif */

  LOGWRITE2( "  RC=%u\n", usRC );

  return( usRC );
} /* end of function EqfAddMatchSegID */




// create a new Translation Memory
USHORT EqfCreateMem
(
  HSESSION    hSession,                // Eqf session handle
  PSZ         pszMemName,              // name of new Translation Memory
  PSZ         pszDescription,          // description for new Translation Memory or NULL
  CHAR        chToDrive,               // target drive for new Translation Memory
  PSZ         pszSourceLanguage,       // Translation Memory source language
  LONG        lOptions                 // type of new Translation Memory
)
{
  USHORT      usRC = NO_ERROR;         // function return code
  PFCTDATA    pData = NULL;            // ptr to function data area

  LOGWRITE1( "==EQFCreateMem==\n" );
  LOGPARMSTRING("Memory", pszMemName );
  LOGPARMSTRING("Description", pszDescription );
  LOGPARMCHAR("ToDrive", chToDrive );
  LOGPARMSTRING("Sourcelanguage", pszSourceLanguage );
  LOGPARMOPTION("Options", lOptions );

  // validate session handle
  usRC = FctValidateSession( hSession, &pData );

  // call TM create function
  if ( usRC == NO_ERROR )
  {
    usRC = MemFuncCreateMem( pszMemName, pszDescription, chToDrive,
                             pszSourceLanguage, lOptions );
    pData->fComplete = TRUE;   // one-shot function are always complete
  } /* endif */

  if ( !usRC )
  {
      SetSharingFlag( EQF_REFR_MEMLIST );
  }


  LOGWRITE2( "  RC=%u\n", usRC );

  return( usRC );
} /* end of function EqfCreateMem */

// analyze one or more documents
USHORT EqfAnalyzeDoc
(
  HSESSION    hSession,                // Eqf session handle
  PSZ         pszFolderName,           // name of folder
  PSZ         pszDocuments,            // list with document names or NULL
  PSZ         pszMemName,              // Translation Memory for analysis
  LONG        lOptions                 // options for analysis
)
{
  USHORT      usRC = NO_ERROR;         // function return code
  PFCTDATA    pData = NULL;            // ptr to function data area

  // validate session handle
  usRC = FctValidateSession( hSession, &pData );

  if ( pData && (pData->fComplete || (pData->sLastFunction != FCT_EQFANALYZEDOC)) )
  {
    LOGWRITE1( "==EQFAnalyzeDoc==\n" );
    LOGPARMSTRING( "Folder", pszFolderName );
    LOGPARMSTRING( "Documents", pszDocuments );
    LOGPARMSTRING( "Memory", pszMemName );
    LOGPARMOPTION( "Options", lOptions );
#ifdef SESSIONLOG
    UtlLogWriteString( "EqfAnalyzeDoc: Starting, Folder = %s", pszFolderName );
#endif

  } /* endif */

  // check sequence of calls
  if ( usRC == NO_ERROR )
  {
    if ( !pData->fComplete && (pData->sLastFunction != FCT_EQFANALYZEDOC) )
    {
      usRC = LASTTASK_INCOMPLETE_RC;
    } /* endif */
  } /* endif */

  // call analyis process
  if ( usRC == NO_ERROR )
  {
    pData->sLastFunction = FCT_EQFANALYZEDOC;
    usRC = TAFuncAnalyzeDoc( pData, pszFolderName, pszDocuments, pszMemName, NULL, NULL, lOptions );
  } /* endif */

  if ( (usRC == NO_ERROR) && !pData->fComplete )
  {
    usRC = CONTINUE_RC;
  } /* endif */

  if ( pData && pData->fComplete )
  {
    LOGWRITE2( "  RC=%u\n", usRC );
#ifdef SESSIONLOG
    CHAR szRC[10];
    sprintf( szRC, "%u", usRC );
    UtlLogWriteString( "EqfAnalyzeDoc: Process complete, rc=%s", szRC );
#endif
  }

  return( usRC );
} /* end of function EqfAnalyzeDoc */

// analyze one or more documents
USHORT EqfAnalyzeDocEx
(
  HSESSION    hSession,                // Eqf session handle
  PSZ         pszFolderName,           // name of folder
  PSZ         pszDocuments,            // list with document names or NULL
  PSZ         pszMemName,              // Translation Memory for analysis
  PSZ         pszProfile,              // name of an analyis profile or NULL
  PSZ         pszMTOutputOptions,      // MT output file options
  LONG        lOptions                 // options for analysis
)
{
  USHORT      usRC = NO_ERROR;         // function return code
  PFCTDATA    pData = NULL;            // ptr to function data area

  // validate session handle
  usRC = FctValidateSession( hSession, &pData );

  if ( pData && (pData->fComplete || (pData->sLastFunction != FCT_EQFANALYZEDOC)) )
  {
    LOGWRITE1( "==EQFAnalyzeDocEx==\n" );
    LOGPARMSTRING( "Folder", pszFolderName );
    LOGPARMSTRING( "Documents", pszDocuments );
    LOGPARMSTRING( "Memory", pszMemName );
    LOGPARMSTRING( "Profile", pszProfile );
    LOGPARMOPTION( "Options", lOptions );
  } /* endif */

  // check sequence of calls
  if ( usRC == NO_ERROR )
  {
    if ( !pData->fComplete && (pData->sLastFunction != FCT_EQFANALYZEDOC) )
    {
      usRC = LASTTASK_INCOMPLETE_RC;
    } /* endif */
  } /* endif */

  // call analyis process
  if ( usRC == NO_ERROR )
  {
    pData->sLastFunction = FCT_EQFANALYZEDOC;
    usRC = TAFuncAnalyzeDoc( pData, pszFolderName, pszDocuments, pszMemName, pszProfile, pszMTOutputOptions,
                             lOptions );
  } /* endif */

  if ( (usRC == NO_ERROR) && !pData->fComplete )
  {
    usRC = CONTINUE_RC;
  } /* endif */

  if ( pData && pData->fComplete ) LOGWRITE2( "  RC=%u\n", usRC );

  return( usRC );
} /* end of function EqfAnalyzeDocEx */


// delete a Translation Memory
USHORT EqfDeleteMem
(
  HSESSION    hSession,                // Eqf session handle
  PSZ         pszMemName               // Translation Memory being deleted
)
{
  USHORT      usRC = NO_ERROR;         // function return code
  PFCTDATA    pData = NULL;            // ptr to function data area

  LOGWRITE1( "==EQFDeleteMem==\n" );
  LOGPARMSTRING( "Memory", pszMemName );

  // validate session handle
  usRC = FctValidateSession( hSession, &pData );

  // call TM delete function
  if ( usRC == NO_ERROR )
  {
    usRC = MemFuncDeleteMem( pszMemName );
    pData->fComplete = TRUE;   // one-shot function are always complete
  } /* endif */

  if ( !usRC )
  {
      SetSharingFlag( EQF_REFR_MEMLIST );
  }


  LOGWRITE2( "  RC=%u\n", usRC );

  return( usRC );
} /* end of function EqfDeleteMem */


USHORT EqfImportFolder
(
  HSESSION    hSession,                // Eqf session handle
  PSZ         pszFolderName,           // name of folder being imported
  CHAR        chFromDrive,             // drive containing the imported folder
  CHAR        chToDrive,               // target drive for folder
  LONG        lOptions                 // folder import options
)
{
  // use the newer interface

  USHORT      usRC;
  CHAR        chTempPath[MAX_EQF_PATH];
  PSZ         pszTemp;
  PFCTDATA    pData = NULL;            // ptr to function data area

  // validate session handle
  usRC = FctValidateSession( hSession, &pData );

  if ( pData && (pData->fComplete || (pData->sLastFunction != FCT_EQFIMPORTFOLDER)) )
  {
    LOGWRITE1( "==EQFImportFolder==\n" );
    LOGPARMSTRING( "Folder", pszFolderName );
    LOGPARMCHAR( "FromDrive", chFromDrive );
    LOGPARMCHAR( "ToDrive", chToDrive );
    LOGPARMOPTION( "Options", lOptions );
  } /* endif */

  // check if drive is valid
  sprintf(chTempPath, "%c", chFromDrive);

  if (!UtlDirExist(chTempPath))
  {
     usRC = ERROR_EQF_DRIVE_NOT_VALID;
     pszTemp = chTempPath;
     UtlErrorHwnd( usRC, MB_CANCEL, 1, &pszTemp, EQF_ERROR, HWND_FUNCIF );
     return (usRC);
  }

  UtlMakeEQFPath( chTempPath, chFromDrive, EXPORT_PATH, NULL );

  // added 19032001 bt - it's not at all guaranteed the an UtlMakeEQFPath generates a valid path

  if (!UtlDirExist(chTempPath))
  {
     usRC = ERROR_EQF_DRIVE_NOT_VALID;
     pszTemp = chTempPath;
     UtlErrorHwnd( usRC, MB_CANCEL, 1, &pszTemp, EQF_ERROR, HWND_FUNCIF );
     return (usRC);
  }

  usRC = InternalImportFolder(hSession, pszFolderName, chTempPath, chToDrive, NULL, lOptions);

  if ( !usRC )
  {
      ULONG ulSetOpt = EQF_REFR_FOLLIST;
      if (lOptions & WITHMEM_OPT)
      {
          ulSetOpt |= EQF_REFR_MEMLIST;
      }
      if (lOptions & WITHDICT_OPT)
      {
          ulSetOpt |= EQF_REFR_DICLIST;
      }
      SetSharingFlag( ulSetOpt );
  }

  if ( pData && pData->fComplete ) LOGWRITE2( "  RC=%u\n", usRC );

  return( usRC );
} /* end of function EqfImportFolder */

/* new interface */
USHORT EqfImportFolderFP
(
  HSESSION    hSession,                // Eqf session handle
  PSZ         pszFolderName,           // name of folder being imported
  PSZ         pszFromPath,             // path containing the imported folder
  CHAR        chToDrive,               // target drive for folder
  LONG        lOptions                 // folder import options
)
{
  USHORT usRC = NO_ERROR;
  PFCTDATA    pData = NULL;            // ptr to function data area

  // validate session handle
  usRC = FctValidateSession( hSession, &pData );

  if ( pData && (pData->fComplete || (pData->sLastFunction != FCT_EQFIMPORTFOLDER)) )
  {
    LOGWRITE1( "==EQFImportFolder==\n" );
    LOGPARMSTRING( "Folder", pszFolderName );
    LOGPARMSTRING( "FromPath", pszFromPath );
    LOGPARMCHAR( "ToDrive", chToDrive );
    LOGPARMOPTION( "Options", lOptions );
  } /* endif */


  usRC = InternalImportFolder(hSession, pszFolderName, pszFromPath, chToDrive, NULL, lOptions);

  if ( !usRC )
  {
      ULONG ulSetOpt = EQF_REFR_FOLLIST;
      if (lOptions & WITHMEM_OPT)
      {
          ulSetOpt |= EQF_REFR_MEMLIST;
      }
      if (lOptions & WITHDICT_OPT)
      {
          ulSetOpt |= EQF_REFR_DICLIST;
      }
      SetSharingFlag( ulSetOpt );
  }


  if ( pData && pData->fComplete ) LOGWRITE2( "  RC=%u\n", usRC );

  return( usRC );
} /* end of function EqfImportFolderFP */

USHORT EqfImportFolderAs
(
  HSESSION    hSession,                // Eqf session handle
  PSZ         pszFolderName,           // name of folder being imported
  PSZ         pszFromPath,             // path containing the imported folder
  CHAR        chToDrive,               // target drive for folder
  PSZ         pszNewFolderName,        // new name for the folder
  LONG        lOptions                 // folder import options
)
{
  USHORT usRC = NO_ERROR;
  PFCTDATA    pData = NULL;            // ptr to function data area

  // validate session handle
  usRC = FctValidateSession( hSession, &pData );

  if ( pData && (pData->fComplete || (pData->sLastFunction != FCT_EQFIMPORTFOLDER)) )
  {
    LOGWRITE1( "==EQFImportFolderAs==\n" );
    LOGPARMSTRING( "Folder", pszFolderName );
    LOGPARMSTRING( "FromPath", pszFromPath );
    LOGPARMCHAR( "ToDrive", chToDrive );
    LOGPARMSTRING( "NewName", pszNewFolderName );
    LOGPARMOPTION( "Options", lOptions );
  } /* endif */


  usRC = InternalImportFolder(hSession, pszFolderName, pszFromPath, chToDrive, pszNewFolderName, lOptions);

  if ( !usRC )
  {
      ULONG ulSetOpt = EQF_REFR_FOLLIST;
      if (lOptions & WITHMEM_OPT)
      {
          ulSetOpt |= EQF_REFR_MEMLIST;
      }
      if (lOptions & WITHDICT_OPT)
      {
          ulSetOpt |= EQF_REFR_DICLIST;
      }
      SetSharingFlag( ulSetOpt );
  }


  if ( pData && pData->fComplete ) LOGWRITE2( "  RC=%u\n", usRC );

  return( usRC );
} /* end of function EqfImportFolderAs */


USHORT InternalImportFolder
(
  HSESSION    hSession,                // Eqf session handle
  PSZ         pszFolderName,           // name of folder being imported
  PSZ         pszFromPath,             // path containing the imported folder
  CHAR        chToDrive,               // target drive for folder
  PSZ         pszNewFolderName,        // new name for the folder
  LONG        lOptions                 // folder import options
)
{
  USHORT      usRC = NO_ERROR;         // function return code
  PFCTDATA    pData = NULL;            // ptr to function data area

  // validate session handle
  usRC = FctValidateSession( hSession, &pData );

  // check sequence of calls
  if ( usRC == NO_ERROR )
  {
    if ( !pData->fComplete && (pData->sLastFunction != FCT_EQFIMPORTFOLDER) )
    {
      usRC = LASTTASK_INCOMPLETE_RC;
    } /* endif */
  } /* endif */

  // call folder import
  if ( usRC == NO_ERROR )
  {
    pData->sLastFunction = FCT_EQFIMPORTFOLDER;
    usRC = FolFuncImportFolder( pData, pszFolderName, pszFromPath, chToDrive, pszNewFolderName, lOptions );
  } /* endif */

  if ( (usRC == NO_ERROR) && !pData->fComplete )
  {
    usRC = CONTINUE_RC;
  } /* endif */
  return( usRC );
} /* end of function InternalImportFolder */

USHORT EqfDeleteFolder
(
  HSESSION    hSession,                // Eqf session handle
  PSZ         pszFolderName            // name of folder being deleted
)
{
  USHORT      usRC = NO_ERROR;         // function return code
  PFCTDATA    pData = NULL;            // ptr to function data area

  LOGWRITE1( "==EQFDeleteFolder==\n" );
  LOGPARMSTRING( "Folder", pszFolderName );

  // validate session handle
  usRC = FctValidateSession( hSession, &pData );

  // call folder delete function
  if ( usRC == NO_ERROR )
  {
    usRC = FolFuncDeleteFolder( pszFolderName );
    pData->fComplete = TRUE;   // one-shot function are always complete
  } /* endif */

  if ( usRC == NO_ERROR )
  {
      SetSharingFlag( EQF_REFR_FOLLIST );
  }

  LOGWRITE2( "  RC=%u\n", usRC );

  return( usRC );
} /* end of function EqfDeleteFolder */


USHORT EqfGetFolderProp
(
  HSESSION    hSession,                // Eqf session handle
  PSZ         pszFolderName,           // name of Folder to get Properties from
  PEXTFOLPROP pExtFolProp              // Structure to return Folderproperties
)
{
  USHORT      usRC = NO_ERROR;         // function return code
  PFCTDATA    pData = NULL;            // ptr to function data area

  LOGWRITE1( "==EQFGetFolderProp==\n" );

  // validate session handle
  usRC = FctValidateSession( hSession, &pData );

  // call folder delete function
  if ( usRC == NO_ERROR )
  {
    usRC = FolFuncGetFolProps( pszFolderName, pExtFolProp );
  } /* endif */

  LOGWRITE2( "  RC=%u\n", usRC );

  return( usRC );
} /* end of function EqfDeleteFolder */

USHORT EqfGetFolderPropEx
(
  HSESSION    hSession,                // mand: Eqf session handle
  PSZ         pszFolderName,           // mand: name of the folder to get the property value from
  PSZ         pszKey,                  // mand: name of the requested value
                                       //@: DRIVE,TARGETLANGUAGE,SOURCELANGUAGE,MEMORY,DICTIONARIES,ROMEMORIES,DESCRIPTION,PROFILE,SHIPMENT
  PSZ         pszBuffer,               // mand: buffer for the returned value
  int         iBufSize                 // mand: size of the buffer
)
{
  USHORT      usRC = NO_ERROR;         // function return code
  PFCTDATA    pData = NULL;            // ptr to function data area

  LOGWRITE1( "==EQFGetFolderPropEx==\n" );

  // validate session handle
  usRC = FctValidateSession( hSession, &pData );

  // call folder delete function
  if ( usRC == NO_ERROR )
  {
    usRC = FolFuncGetFolPropEx( pszFolderName, pszKey, pszBuffer, iBufSize );
  } /* endif */

  LOGWRITE2( "  RC=%u\n", usRC );

  return( usRC );
} /* end of function EqfGetFolderPropEx */


USHORT EqfDeleteDoc
(
  HSESSION    hSession,                // Eqf session handle
  PSZ         pszFolderName,           // name of folder containing the documents
  PSZ         pszDocuments             // list of documents being deleted
)
{
  USHORT      usRC = NO_ERROR;         // function return code
  PFCTDATA    pData = NULL;            // ptr to function data area

  LOGWRITE1( "==EQFDeleteDoc==\n" );

  // validate session handle
  usRC = FctValidateSession( hSession, &pData );

  // call document delete function
  if ( usRC == NO_ERROR )
  {
    usRC = DocFuncDeleteDoc( pszFolderName, pszDocuments );
    pData->fComplete = TRUE;   // one-shot function are always complete
  } /* endif */

  LOGWRITE2( "  RC=%u\n", usRC );

  return( usRC );
} /* end of function EqfDeleteDoc */

USHORT EqfArchiveTM
(
  HSESSION    hSession,                // Eqf session handle
  PSZ         pszFolderName,           // name of folder
  CHAR        chToDrive,               // Drive (folder)
  PSZ         pszDocuments,            // list with document names or NULL
  PSZ         pszMemName,              // output Translation Memory
  LONG        lOptions                 // options (use as folder memory f.e.)
)
{
  USHORT      usRC = NO_ERROR;         // function return code
  PFCTDATA    pData = NULL;            // ptr to function data area

  // validate session handle
  usRC = FctValidateSession( hSession, &pData );

  if ( pData && (pData->fComplete || (pData->sLastFunction != FCT_EQFARCHIVETM)) )
  {
    LOGWRITE1( "==EQFArchiveTM==\n" );
    LOGPARMSTRING( "Folder", pszFolderName );
    LOGPARMCHAR( "Drive", chToDrive );
    LOGPARMSTRING( "Documents", pszDocuments );
    LOGPARMSTRING( "Memory", pszMemName );
    LOGPARMOPTION( "Options", lOptions );
  } /* endif */

  // check sequence of calls
  if ( usRC == NO_ERROR )
  {
    if ( !pData->fComplete && (pData->sLastFunction != FCT_EQFARCHIVETM) )
    {
      usRC = LASTTASK_INCOMPLETE_RC;
    } /* endif */
  } /* endif */

  // call archive TM function
  if ( usRC == NO_ERROR )
  {
    pData->sLastFunction = FCT_EQFARCHIVETM;
    usRC = TAFuncArchTM( pData, pszFolderName, chToDrive, pszDocuments, pszMemName,
                         lOptions );
  } /* endif */

  if ( (usRC == NO_ERROR) && !pData->fComplete )
  {
    usRC = CONTINUE_RC;
  } /* endif */

  if ( pData && pData->fComplete ) LOGWRITE2( "  RC=%u\n", usRC );

  return( usRC );
} /* end of function EqfArchiveTM */


// create a Initial Translation Memory
USHORT EqfCreateITM
(
  HSESSION    hSession,                // Eqf session handle
  PSZ         pszMemName,              // name of new Translation Memory
  PSZ         pszFilePairs,            // list of filepairs or NULL
  PSZ         pszMarkup,               // use markup table
  PSZ         pszSGMLMemFile,          // external memory name or NUULL
  PSZ         pszSourceLanguage,       // Translation Memory source language
  PSZ         pszTargetLanguage,       // Translation Memory target language
  PSZ         pszSourceStartPath,      // Startpath not stored for source files or NULL
  PSZ         pszTargetStartPath,      // Startpath not stored for target files or NULL
  LONG        lType                    // type of creation
)
{
  USHORT      usRC = NO_ERROR;         // function return code

  PFCTDATA    pData = NULL;            // ptr to function data area

  LOGWRITE1( "==EQFCreateITM==\n" );

  // validate session handle
  usRC = FctValidateSession( hSession, &pData );

  // call TM create function
  if ( usRC == NO_ERROR )
  {
    usRC = MemFuncCreateITM( pszMemName, pszFilePairs, pszMarkup, pszSGMLMemFile,
                             pszSourceLanguage, pszTargetLanguage,
                             pszSourceStartPath, pszTargetStartPath, lType );
    pData->fComplete = TRUE;   // one-shot function are always complete
  } /* endif */

  LOGWRITE2( "  RC=%u\n", usRC );

  return( usRC );
} /* end of function EqfCreateMem */




USHORT EqfChangeMFlag
(
  HSESSION    hSession,                // Eqf session handle
  PSZ         pszMemName,              // name of new Translation Memory
  LONG        lAction                  // type of action
)
{
  USHORT      usRC = NO_ERROR;         // function return code

  PFCTDATA    pData = NULL;            // ptr to function data area

  LOGWRITE1( "==EQFChangeMFlag==\n" );

  // validate session handle
  usRC = FctValidateSession( hSession, &pData );

  // call TM create function
  if ( usRC == NO_ERROR )
  {
    usRC = MemFuncChangeMFlag( pszMemName, lAction );
    pData->fComplete = TRUE;   // one-shot function are always complete
  } /* endif */

  LOGWRITE2( "  RC=%u\n", usRC );

  return( usRC );
} /* end of function EqfCreateMem */

// create a new Markup Table
USHORT EqfCreateMarkup
(
  HSESSION    hSession,                // Eqf session handle
  PSZ         pszInFile,               // input file for markup table (TBX)
  PSZ         pszOutFile               // output file for markup table (TBL)
)
{
  USHORT      usRC = NO_ERROR;         // function return code
  PFCTDATA    pData = NULL;            // ptr to function data area

  LOGWRITE1( "==EQFCreateMarkup==\n" );
  LOGPARMSTRING("InFile", pszInFile );
  LOGPARMSTRING("OutFile", pszOutFile );

  // validate session handle
  usRC = FctValidateSession( hSession, &pData );

  // call TM create function
  if ( usRC == NO_ERROR )
  {
    usRC = MarkupFuncCreateMarkup( pszInFile, pszOutFile ) ;
    pData->fComplete = TRUE;   // one-shot function are always complete
  } /* endif */

  LOGWRITE2( "  RC=%u\n", usRC );

  return( usRC );
} /* end of function EqfCreateMarkup */


USHORT EqfStartSession
(
  PHSESSION   phSession                // ptr to callers Eqf session handle variable
)
{
  USHORT      usRC = NO_ERROR;         // function return code
  PFCTDATA    pData = NULL;            // ptr to function data area

  // allocate internal data area
  UtlAlloc( (PVOID *)&pData, 0L, sizeof(FCTDATA), NOMSG );
  if ( pData != NULL )
  {
    pData->lMagicWord = FCTDATA_IDENTIFIER;
    pData->hwndErrMsg = HWND_FUNCIF;
//    FctBuildCheckSum( pData, &pData->lCheckSum );
    pData->fComplete = TRUE;   // no incomplete function yet
  }
  else
  {
    usRC = ERROR_NOT_ENOUGH_MEMORY;
  } /* endif */

  // initialize utilities
  if ( usRC == NO_ERROR )
  {
    UtlSetUShort( QS_RUNMODE, FUNCCALL_RUNMODE );
    UtlSetUShort( QS_PROGRAMID, NONDDEAPI_PROGID );

    if ( !UtlInitUtils( NULLHANDLE ) )
    {
      usRC = ERROR_READ_SYSTEMPROPERTIES;
    } /* endif */
  } /* endif */

  // get profile data and initialize error handler
  if ( usRC == NO_ERROR )
  {
    BOOL fContinue = TwbGetCheckProfileData( pData->szMsgFile,
                                             pData->szSystemPropPath,
                                             pData->szEqfResFile );

    //  Initialize error handler to allow handling of error messages
    UtlInitError( NULLHANDLE, HWND_FUNCIF, (HWND)&(pData->LastMessage),
                  pData->szMsgFile );

    if ( !fContinue )
    {
      usRC = 1;
    } /* endif */
  } /* endif */

  // set directory strings and main drive
  if ( usRC == NO_ERROR )
  {
    PPROPSYSTEM  pPropSys = GetSystemPropPtr();

    if ( pPropSys != NULL )
    {
      UtlSetString( QST_PRIMARYDRIVE, pPropSys->szPrimaryDrive );
      UtlSetString( QST_LANDRIVE,     pPropSys->szLanDrive );
      UtlSetString( QST_PROPDIR,      pPropSys->szPropertyPath );
      UtlSetString( QST_CONTROLDIR,   pPropSys->szCtrlPath );
      UtlSetString( QST_PROGRAMDIR,   pPropSys->szProgramPath );
      UtlSetString( QST_DICDIR,       pPropSys->szDicPath );
      UtlSetString( QST_MEMDIR,       pPropSys->szMemPath );
      UtlSetString( QST_TABLEDIR,     pPropSys->szTablePath );
      UtlSetString( QST_LISTDIR,      pPropSys->szListPath );
      UtlSetString( QST_SOURCEDIR,    pPropSys->szDirSourceDoc );
      UtlSetString( QST_SEGSOURCEDIR, pPropSys->szDirSegSourceDoc );
      UtlSetString( QST_SEGTARGETDIR, pPropSys->szDirSegTargetDoc );
      UtlSetString( QST_TARGETDIR,    pPropSys->szDirTargetDoc );
      UtlSetString( QST_DLLDIR,       pPropSys->szDllPath );
      UtlSetString( QST_MSGDIR,       pPropSys->szMsgPath );
      UtlSetString( QST_EXPORTDIR,    pPropSys->szExportPath );
      UtlSetString( QST_BACKUPDIR,    pPropSys->szBackupPath );
      UtlSetString( QST_IMPORTDIR,    pPropSys->szDirImport );
      UtlSetString( QST_COMMEMDIR,    pPropSys->szDirComMem );
      UtlSetString( QST_COMPROPDIR,   pPropSys->szDirComProp );
      UtlSetString( QST_COMDICTDIR,   pPropSys->szDirComDict );
      UtlSetString( QST_SYSTEMDIR,    pPropSys->PropHead.szPath + 3 );
      UtlSetString( QST_DIRSEGNOMATCHDIR, "SNOMATCH" );
      UtlSetString( QST_MTLOGPATH,    "MTLOG" );
      UtlSetString( QST_DIRSEGMTDIR, "MT" );
      UtlSetString( QST_DIRSEGRTFDIR, "RTF" );
      UtlSetString( QST_PRTPATH,      pPropSys->szPrtPath );
      if ( pPropSys->szWinPath[0] )
      {
        UtlSetString( QST_WINPATH,    pPropSys->szWinPath );
      }
      else
      {
        UtlSetString( QST_WINPATH,      WINDIR );
      } /* endif */
      if ( pPropSys->szEADataPath[0] )
      {
        UtlSetString( QST_EADATAPATH,   pPropSys->szEADataPath );
      }
      else
      {
        UtlSetString( QST_EADATAPATH,   EADATADIR );
      } /* endif */
      UtlSetString( QST_LOGPATH,           "LOGS" );
      UtlSetString( QST_XLIFFPATH,         "XLIFF" );
      UtlSetString( QST_METADATAPATH,      "METADATA" );
      UtlSetString( QST_JAVAPATH,          "JAVA" );
      UtlSetString( QST_MISCPATH,          "MISC" );

      if ( pPropSys->szPluginPath[0] )
      {
        UtlSetString( QST_PLUGINPATH,   pPropSys->szPluginPath );
      }
      else
      {
        UtlSetString( QST_PLUGINPATH,        "PLUGINS" );
      }
      UtlSetString( QST_ENTITY,            "ENTITY" );
      UtlSetString( QST_REMOVEDDOCDIR,     "REMOVED" );

      UtlSetString( QST_ORGEQFDRIVES, pPropSys->szDriveList );
      UtlSetString( QST_VALIDEQFDRIVES, pPropSys->szDriveList );

      // force a refresh of QST_VALIDEQFDRIVES string
      {
        CHAR  szDriveList[MAX_DRIVELIST];
        UtlGetCheckedEqfDrives( szDriveList );
      }

      // set fuzziness limits
      if ( !pPropSys->lSmallLkupFuzzLevel )  pPropSys->lSmallLkupFuzzLevel = 3300;
      UtlSetULong( QL_SMALLLOOKUPFUZZLEVEL,    pPropSys->lSmallLkupFuzzLevel );
      if ( !pPropSys->lMediumLkupFuzzLevel ) pPropSys->lMediumLkupFuzzLevel = 3300;
      UtlSetULong( QL_MEDIUMLOOKUPFUZZLEVEL,   pPropSys->lMediumLkupFuzzLevel );
      if ( !pPropSys->lLargeLkupFuzzLevel )  pPropSys->lLargeLkupFuzzLevel = 3300;
      UtlSetULong( QL_LARGELOOKUPFUZZLEVEL,    pPropSys->lLargeLkupFuzzLevel );

      if ( !pPropSys->lSmallFuzzLevel )  pPropSys->lSmallFuzzLevel = 3300;
	  UtlSetULong( QL_SMALLFUZZLEVEL,    pPropSys->lSmallFuzzLevel );
	  if ( !pPropSys->lMediumFuzzLevel ) pPropSys->lMediumFuzzLevel = 3300;
	  UtlSetULong( QL_MEDIUMFUZZLEVEL,   pPropSys->lMediumFuzzLevel );
	  if ( !pPropSys->lLargeFuzzLevel )  pPropSys->lLargeFuzzLevel = 3300;
      UtlSetULong( QL_LARGEFUZZLEVEL,    pPropSys->lLargeFuzzLevel );

    // set SGML/DITA processing flag
    UtlSetUShort( QS_SGMLDITAPROCESSING, (USHORT)!pPropSys->fNoSgmlDitaProcessing );

    UtlSetUShort( QS_ENTITYPROCESSING, (USHORT)pPropSys->fEntityProcessing );

    UtlSetUShort( QS_MEMIMPMRKUPACTION, (USHORT)pPropSys->usMemImpMrkupAction );

    }
    else
    {
      // access to system properties failed
      usRC = ERROR_READ_SYSTEMPROPERTIES;
    } /* endif */
  } /* endif */

  // load resource file
  if ( usRC == NO_ERROR )
  {
    HMODULE  hmod;                      // buffer for resource module handle

    DosLoadModule( NULL, NULLHANDLE, pData->szEqfResFile, &hmod );
    //hResMod = hmod;
    UtlSetULong( QL_HRESMOD, (ULONG)hmod );
  } /* endif */

  // set caller's session handle
  if ( usRC == NO_ERROR )
  {
#ifdef DEBUGAPI
    {
      FILETIME  ftSysTime;
      LARGE_INTEGER liTime;
      LONG lTime = 0;

        iLogLevel = 0;

      
      GetSystemTimeAsFileTime( &ftSysTime );
      memcpy( &liTime, &ftSysTime, sizeof(liTime ) );

      UtlMakeEQFPath( szLogFile, NULC, LOG_PATH, NULL );
      strcat( szLogFile, "\\" );
      //strcat( szLogFile, EQFAPI_LOGFILE );
      sprintf( szLogFile + strlen(szLogFile), "OTMFUNC-%I64d-%ld.log", liTime, (LONG)pData );

      hLog = fopen( szLogFile, "a" ); 
      if ( hLog != NULLHANDLE ) 
      { 
        fLog = TRUE;
        time( &lTime ); 
        fprintf( hLog, "===== EQFAPI Log Started %s", ctime( &lTime ) ); 
      } 
      else
      {
        fLog = FALSE;
      } /* endif */
    }
#endif

    LOGWRITE1( "==EQFSTARTSESSION==\n" );

    LOGWRITE1( "   Starting plugins...\n" );

  // initialie plugins
  if ( usRC == NO_ERROR )
  {
    char szPluginPath[MAX_EQF_PATH];
    UtlMakeEQFPath( szPluginPath, NULC, PLUGIN_PATH, NULL );
		usRC = InitializePlugins( szPluginPath );    // add return value for P402974
        // Add for P403115;
        if (usRC != PluginManager::ePluginExpired)
        {
            // Add for P403138
            char strParam[1024];
            memset(strParam, 0x00, sizeof(strParam));

            PluginManager* thePluginManager = PluginManager::getInstance();
            if (!thePluginManager->ValidationCheck(strParam))
            {
                usRC = NO_ERROR;
            }
            else
            {
                usRC = ERROR_PLUGIN_INVALID;
                PSZ pszParam = strParam;
                UtlError(ERROR_PLUGIN_INVALID, MB_OK , 1, &pszParam, EQF_WARNING);
            }
            // Add end
        }
        else
        {
            usRC = ERROR_PLUGIN_EXPIRED;
        }
        // Add end
  }

    LOGWRITE1( "   ...Plugins have been started\n" );

#ifdef SESSIONLOG
    {
      char szBuf[10];
      int i = 0;
      UtlLogStart( "TMSession" );
      i = _getpid();
      sprintf( szBuf, "%ld", i );
      UtlLogWriteString( "EqfStartSession: Process ID is %s", szBuf );
    }
#endif
    *phSession = (LONG)pData;
  }
  else
  {
    *phSession = NULLHANDLE;
  } /* endif */

  LOGWRITE2( "  RC=%u\n", usRC );

  return( usRC );
} /* end of function EqfStartSession */


USHORT EqfEndSession
(
  HSESSION    hSession                 // Eqf session handle
)
{
  PFCTDATA    pData = NULL;            // ptr to function data area
  USHORT      usRC = NO_ERROR;         // function return code

  LOGWRITE1( "==EQFENDSESSION==\n" );

  // validate session handle
  usRC = FctValidateSession( hSession, &pData );

#ifdef SESSIONLOG
    UtlLogStop();
#endif
  // cleanup any resoures left over by last function
  if ( usRC == NO_ERROR )
  {
    switch ( pData->sLastFunction )
    {
      case FCT_EQFEXPORTFOLDER :
      case FCT_EQFCOUNTWORDS :
      case FCT_EQFIMPORTDICT :
      case FCT_EQFEXPORTDICT :
      case FCT_EQFIMPORTDOC :
      case FCT_EQFEXPORTDOC :
      case FCT_EQFIMPORTMEM :
      case FCT_EQFEXPORTMEM :
      case FCT_EQFANALYZEDOC :
      case FCT_EQFIMPORTFOLDER :
      case FCT_EQFCREATEMARKUP :
        break;
      default :
        // no cleanup required for one-shot functions
        break;
    }
  } /* endif */

  // free any parser API handles
  if ( usRC == NO_ERROR )
  {
    while ( pData->pSegFile )
    {
      EqfFreeSegFile( (HPARSSEGFILE)pData->pSegFile );
    } /* endwhile */
  } /* endif */


  // terminate utils
  UtlTerminateUtils();
  UtlTerminateError();
//  DosFreeModule ( hResMod );
	{
		HMODULE  hmod;                      // buffer for resource module handle
		hmod = (HMODULE) UtlQueryULong( QL_HRESMOD );
		if( hmod )
		{
			DosFreeModule ( hmod );
			UtlSetULong( QL_HRESMOD, 0 );
		}
	}


  LOGWRITE2( "  RC=%u\n", usRC );
  LOGCLOSE();

  // free our data area
  if ( usRC != ERROR_INVALID_SESSION_HANDLE )
  {
    UtlAlloc( (PVOID *)&pData, 0L, 0L, NOMSG );
  } /* endif */

  return( usRC );
} /* end of function EqfEndSession */

USHORT EqfGetLastError
(
  HSESSION    hSession,                // Eqf session handle
  PUSHORT     pusRC,                   // ptr to buffer for last return code
  PSZ         pszMsgBuffer,            // ptr to buffer receiving the message
  USHORT      usBufSize                // size of message buffer in bytes
)
{
  PFCTDATA    pData = NULL;            // ptr to function data area
  USHORT      usRC = NO_ERROR;         // function return code

  // validate session handle
  usRC = FctValidateSession( hSession, &pData );

  if ( usRC == NO_ERROR )
  {
    *pusRC = (USHORT)pData->LastMessage.sErrNumber;
    strncpy( pszMsgBuffer, pData->LastMessage.chMsgText, usBufSize );
    pszMsgBuffer[usBufSize-1] = EOS;

    pData->LastMessage.sErrNumber = 0;
    pData->LastMessage.chMsgText[0] = EOS;
  } /* endif */

  return( usRC );
} /* end of function EqfGetLastError */

USHORT EqfGetLastErrorW
(
  HSESSION    hSession,                // Eqf session handle
  PUSHORT     pusRC,                   // ptr to buffer for last return code
  wchar_t    *pszMsgBuffer,            // ptr to buffer receiving the message
  USHORT      usBufSize                // size of message buffer in bytes
)
{
  PFCTDATA    pData = NULL;            // ptr to function data area
  USHORT      usRC = NO_ERROR;         // function return code

  // validate session handle
  usRC = FctValidateSession( hSession, &pData );

  if ( usRC == NO_ERROR )
  {
    *pusRC = (USHORT)pData->LastMessage.sErrNumber;
    MultiByteToWideChar( CP_OEMCP, 0, pData->LastMessage.chMsgText, -1, pszMsgBuffer, usBufSize );

    pData->LastMessage.sErrNumber = 0;
    pData->LastMessage.chMsgText[0] = EOS;
  } /* endif */

  return( usRC );
} /* end of function EqfGetLastErrorW */



// FctValidateSession: check and convert a session handle to a FCTDATA pointer
USHORT FctValidateSession
(
  HSESSION    hSession,                // session handle being validated
  PFCTDATA   *ppData                   // address of caller's FCTDATA pointer
)
{
  PFCTDATA    pData;                   // pointer to FCTDATA area
  USHORT      usRC = NO_ERROR;         // function return code

  // convert handle to pointer
  pData = (PFCTDATA)hSession;

  // test magic word and checksum
  if ( pData == NULL )
  {
    usRC = ERROR_INVALID_SESSION_HANDLE;
  }
  else if ( pData->lMagicWord != FCTDATA_IDENTIFIER )
  {
    usRC = ERROR_INVALID_SESSION_HANDLE;
  }
  else
  {
//     LONG      lCheckSum;               // buffer for actual checksum

//    FctBuildCheckSum( pData, &lCheckSum );
//    if ( lCheckSum != pData->lCheckSum )
//    {
//       usRC =
//    } /* endif */
  } /* endif */

  if ( usRC == NO_ERROR )
  {
     *ppData = pData;
  } /* endif */
  return( usRC );
} /* end of function FctValidateSession */

// FctBuildCheckSum: check and convert a session handle to a FUNCDATA pointer
USHORT FctBuildCheckSum
(
  PFCTDATA    pData,                   // FCTDATA area
  PLONG       plCheckSum               // ptr to checksum variable
)
{
  PBYTE       pbData;                  // pointer for access to FCTDATA area
  LONG        lLength;                  // length oa rea to process
  LONG        lCheckSum = 0L;         // buffer for computed checksum

  pbData = (PBYTE)&(pData->lCheckSum) + sizeof(pData->lCheckSum);
  lLength = sizeof(FCTDATA) - (pbData - (PBYTE)pData);

  while ( lLength != 0L )
  {
     lCheckSum += *pbData++;
    lLength--;
  } /* endwhile */

  *plCheckSum = lCheckSum;

  return( NO_ERROR );
} /* end of function FctBuildCheckSum */


void SetSharingFlag(ULONG ulRefreshFlag)
{
    HANDLE        hMapObject = NULL;

    hMapObject = OpenFileMapping (FILE_MAP_WRITE, FALSE, EQFNDDE_SHFLAG );
    /*
    if(!hMapObject)
    {
        hMapObject = CreateFileMapping(
                                      (HANDLE)0xFFFFFFFF,   // use page file
                                      NULL,                 // no security attrib
                                      PAGE_READWRITE,       // read/write access
                                      0,                    // size: high 32bits
                                      sizeof(ULONG),        // size: low 32bit
                                      EQFNDDE_SHFLAG );     // name of file mapping
    }
    if ( hMapObject == NULL )
    {
        usRC = ERROR_STORAGE;
    }*/
    if ( hMapObject )
    {
        ULONG *ulActFlag = (ULONG *)MapViewOfFile (hMapObject,
                                  FILE_MAP_WRITE,
                                  0,
                                  0,
                                  0);
        *ulActFlag |= ulRefreshFlag;
        UnmapViewOfFile(ulActFlag);

    }
}



USHORT EqfCreateCntReport(HSESSION hSession,
                          CHAR chDriveLetter,
                                          PSZ pszFolderName,
                                                  PSZ pszDocuments,
                                                  PREPORTTYPE pReportType,
                                                  PSZ pszOutfileName,
                                                  PSZ pszFormat,
                                                  PSZ pszProfile,
                                                  PREPORTSETTINGS pRepSettings,
                                                  PFACTSHEET pFactSheet,
                              USHORT usColumn,
                              USHORT usCategory,
                                                  PFINALFACTORS pFinalFactors,
                                                  LONG lOptSecurity,
                                                  BOOL bSingleShipment)
{
  USHORT          usRC = NO_ERROR;
  PFCTDATA    pData = NULL;            // ptr to function data area

  LOGWRITE1( "==EQFCreateCntReportt==\n" );

  // validate session handle
  usRC = FctValidateSession( hSession, &pData );

  // call TM create function
  if (usRC == NO_ERROR)
  {
        usRC = MemFuncCreateCntReport(hSession,
                                  chDriveLetter,
                                                                  pszFolderName,
                                                                  pszDocuments,
                                                                  pReportType,
                                                                  pszOutfileName,
                                                                  pszFormat,
                                                                  pszProfile,
                                                                  pRepSettings,
                                                                  pFactSheet,
                                      usColumn,
                                      usCategory,
                                                                  pFinalFactors,
                                                                  lOptSecurity,
                                                                  bSingleShipment,
                                                                  NULL );

    pData->fComplete = TRUE;   // one-shot function are always complete
  } // endif

  LOGWRITE2( "  RC=%u\n", usRC );

  return usRC;
} // end of EqfCreateCntReport

USHORT EqfCreateCntReport(HSESSION hSession,
                          CHAR chDriveLetter,
                                          PSZ pszFolderName,
                                                  PSZ pszDocuments,
                                                  PREPORTTYPE pReportType,
                                                  PSZ pszOutfileName,
                                                  PSZ pszFormat,
                                                  PSZ pszProfile,
                                                  PREPORTSETTINGS pRepSettings,
                                                  PFACTSHEET pFactSheet,
                              USHORT usColumn,
                              USHORT usCategory,
                                                  PFINALFACTORS pFinalFactors,
                                                  LONG lOptSecurity,
  PSZ pszShipment,                     // opt: shipment number or one of the keywords "All Shipments" or "Single Shipments"
  PSZ pszUnused1,                      // opt: for future enhancements, currently unused
  PSZ pszUnused2                       // opt: for future enhancements, currently unused
  )
{
  USHORT          usRC = NO_ERROR;
  PFCTDATA    pData = NULL;            // ptr to function data area

  pszUnused1; pszUnused2;

  LOGWRITE1( "==EQFCreateCntReportt==\n" );

  // validate session handle
  usRC = FctValidateSession( hSession, &pData );

  // call TM create function
  if (usRC == NO_ERROR)
  {
        usRC = MemFuncCreateCntReport(hSession,
                                  chDriveLetter,
                                                                  pszFolderName,
                                                                  pszDocuments,
                                                                  pReportType,
                                                                  pszOutfileName,
                                                                  pszFormat,
                                                                  pszProfile,
                                                                  pRepSettings,
                                                                  pFactSheet,
                                      usColumn,
                                      usCategory,
                                                                  pFinalFactors,
                                                                  lOptSecurity,
                                                                  FALSE,
                                                                  pszShipment );

    pData->fComplete = TRUE;   // one-shot function are always complete
  } // endif

  LOGWRITE2( "  RC=%u\n", usRC );

  return usRC;
} // end of EqfCreateCntReportEx



USHORT EqfCreateCountReport
(
  HSESSION    hSession,                // Eqf session handle
  PSZ         pszFolderName,           // name of folder containing the documents
  PSZ         pszDocuments,            // list of documents being counted
  PSZ         pszOutFile,              // fully qualified name of output file
  USHORT      usReport,                // ID of report being created
  USHORT      usType,                  // type of report being created
  PSZ         pszProfile,              // name of profile
  LONG        lOptions                 // OVERWRITE_OPT or 0L
)
{
  USHORT          usRC = NO_ERROR;
  PFCTDATA    pData = NULL;            // ptr to function data area

  LOGWRITE1( "==EQFCreateCountReportt==\n" );

  // validate session handle
  usRC = FctValidateSession( hSession, &pData );

  // call report creation code
  if (usRC == NO_ERROR)
  {
    REPORTTYPE RepType;
    REPORTSETTINGS RepSettings;
    PSZ pszFormat = "ASCII";

    memset( &RepType, 0, sizeof(RepType) );
    switch ( usReport )
    {
      case CALCULATING_REP       : RepType.pszReport = "Calculating Report"; break;
      case HISTORY_REP           : RepType.pszReport = "History Report"; break; 
      case COUNTING_REP          : RepType.pszReport = "Counting Report"; break;
      case PREANALYSIS_REP       : RepType.pszReport = "Pre-Analysis Report"; break;
      case REDUNDANCY_REP        : RepType.pszReport = "Redundancy Report"; break;
      case REDUNDANCYSEGMENT_REP : RepType.pszReport = "Redundant Segment List"; break;
      default:
        break;
    } /*endswitch */

    switch ( usType )
    {
      case BASE_REPTYPE                   : RepType.lRepType = BASE_TYP;                      break;
      case BASE_SUMMARY_REPTYPE           : RepType.lRepType = BASE_TYP | SUM_TYP;            break;
      case BASE_SUMMARY_FACTSHEET_REPTYPE : RepType.lRepType = BASE_TYP | SUM_TYP | FACT_TYP; break;
      case SUMMARY_FACTSHEET_REPTYPE      : RepType.lRepType = SUM_TYP | FACT_TYP;            break;
      case FACTSHEET_REPTYPE              : RepType.lRepType = FACT_TYP;                      break;
    } /*endswitch */

    memset( &RepSettings, 0, sizeof(RepSettings) );
    RepSettings.pszCountType = "Source Words";

    if ( lOptions & HTML_OUTPUT_OPT )
    {
      pszFormat = "HTML";
    }
    else if ( lOptions & XML_OUTPUT_OPT )
    {
      pszFormat = "XML";
    }
    else  /* TEXT_OUTPUT_OPT or any other */
    {
      pszFormat = "ASCII";
    } /* endif */

    usRC = MemFuncCreateCntReport( hSession, EOS, pszFolderName, pszDocuments, &RepType,
                                   pszOutFile, pszFormat, pszProfile, &RepSettings, 
                                   NULL, /* no factsheet */
                                   1 /* dummy value for usColumn */, 1 /* dummy value for usCategory */, 
                                   NULL, /* no final factors */
                                   0, FALSE, NULL ); /* no lOptSecurity, no bSingleShipment, no shipment string */
  } /* endif */
  return( usRC );
}

USHORT EqfCreateCountReportEx
(
  HSESSION    hSession,                // Eqf session handle
  PSZ         pszFolderName,           // name of folder containing the documents
  PSZ         pszDocuments,            // list of documents being counted
  PSZ         pszOutFile,              // fully qualified name of output file
  USHORT      usReport,                // ID of report being created
  USHORT      usType,                  // type of report being created
  PSZ         pszProfile,              // name of profile
  PSZ         pszShipment,             // opt: shipment number or one of the keywords "All Shipments" or "Single Shipments"
  PSZ         pszUnused1,              // opt: for future enhancements, currently unused
  PSZ         pszUnused2,              // opt: for future enhancements, currently unused
  LONG        lOptions                 // OVERWRITE_OPT or 0L
)
{
  USHORT          usRC = NO_ERROR;
  PFCTDATA    pData = NULL;            // ptr to function data area

  pszUnused1; pszUnused2;

  LOGWRITE1( "==EQFCreateCountReportEx==\n" );

  // validate session handle
  usRC = FctValidateSession( hSession, &pData );

  // call report creation code
  if (usRC == NO_ERROR)
  {
    REPORTTYPE RepType;
    REPORTSETTINGS RepSettings;
    PSZ pszFormat = "ASCII";

    memset( &RepType, 0, sizeof(RepType) );
    switch ( usReport )
    {
      case CALCULATING_REP       : RepType.pszReport = "Calculating Report"; break;
      case HISTORY_REP           : RepType.pszReport = "History Report"; break; 
      case COUNTING_REP          : RepType.pszReport = "Counting Report"; break;
      case PREANALYSIS_REP       : RepType.pszReport = "Pre-Analysis Report"; break;
      case REDUNDANCY_REP        : RepType.pszReport = "Redundancy Report"; break;
      case REDUNDANCYSEGMENT_REP : RepType.pszReport = "Redundant Segment List"; break;
      default:
        break;
    } /*endswitch */

    switch ( usType )
    {
      case BASE_REPTYPE                   : RepType.lRepType = BASE_TYP;                      break;
      case BASE_SUMMARY_REPTYPE           : RepType.lRepType = BASE_TYP | SUM_TYP;            break;
      case BASE_SUMMARY_FACTSHEET_REPTYPE : RepType.lRepType = BASE_TYP | SUM_TYP | FACT_TYP; break;
      case SUMMARY_FACTSHEET_REPTYPE      : RepType.lRepType = SUM_TYP | FACT_TYP;            break;
      case FACTSHEET_REPTYPE              : RepType.lRepType = FACT_TYP;                      break;
    } /*endswitch */

    memset( &RepSettings, 0, sizeof(RepSettings) );
    RepSettings.pszCountType = "Source Words";

    if ( lOptions & HTML_OUTPUT_OPT )
    {
      pszFormat = "HTML";
    }
    else if ( lOptions & XML_OUTPUT_OPT )
    {
      pszFormat = "XML";
    }
    else  /* TEXT_OUTPUT_OPT or any other */
    {
      pszFormat = "ASCII";
    } /* endif */

    usRC = MemFuncCreateCntReport( hSession, EOS, pszFolderName, pszDocuments, &RepType,
                                   pszOutFile, pszFormat, pszProfile, &RepSettings, 
                                   NULL, /* no factsheet */
                                   1 /* dummy value for usColumn */, 1 /* dummy value for usCategory */, 
                                   NULL, /* no final factors */
                                   0, FALSE, pszShipment ); /* no lOptSecurity, no bSingleShipment */
  } /* endif */
  return( usRC );
}




USHORT EqfBuildSegDocName
(
  HSESSION         hSession,           // non-DDE session handle
  PSZ              pszFolderName,      // pointer to long folder name w/p drive, path and ext
  PSZ              pszDocumentName,    // pointer to long document name
  EQF_BOOL         fSource,            // TRUE (1)  = build segmented source file name
                                       // FALSE (0) = build segmented target file name
  PSZ              pszSegFile          // points to buffer for full file name
                                       // must have a size of at least 60 characters
)
{
  PFCTDATA    pData;                   // pointer to FCTDATA area
  USHORT      usRC = NO_ERROR;         // function return code

  LOGWRITE1( "==EQFBuildSegDocName==\n" );

  // validate session handle
  usRC = FctValidateSession( hSession, &pData );

  // check parameters
  if ( usRC  == NO_ERROR )
  {
    if ( (pszFolderName == NULL) || (pszDocumentName == NULL) || (pszSegFile == NULL) )
    {
      usRC = ERROR_INVALID_PARAMETER;
    } /* endif */
  } /* endif */

  // get folder object name
  if ( usRC  == NO_ERROR )
  {
    BOOL fIsNew = !SubFolNameToObjectName( pszFolderName, pData->szObjName );
    if ( fIsNew )
    {
      usRC = FILE_NOT_EXISTS;
    } /* endif */
  } /* endif */

  // build complete file name
  if ( usRC  == NO_ERROR )
  {
    BOOL fIsNew = FALSE;
    USHORT  usPathID = (USHORT)(( fSource ) ? DIRSEGSOURCEDOC_PATH : DIRSEGTARGETDOC_PATH);
    UtlMakeEQFPath( pszSegFile, pData->szObjName[0], usPathID,
                    UtlGetFnameFromPath( pData->szObjName ));
    strcat( pszSegFile, BACKSLASH_STR );
    FolLongToShortDocName( pData->szObjName, pszDocumentName, pszSegFile + strlen(pszSegFile), &fIsNew );
    if ( fIsNew )
    {
      usRC = FILE_NOT_EXISTS;
    } /* endif */
  } /* endif */

  LOGWRITE2( "  RC=%u\n", usRC );

  return( usRC );
} /* end of function EqfBuildSegDocName */

// load a segmented document
USHORT EqfLoadSegFile
(
  HSESSION    hSession,                // Eqf session handle
  PSZ         pszFullDocName,          // fully qualified name of segmented document
  HPARSSEGFILE *phLoadedFile           // pointer to buffer for handle of loaded file
)
{
  PFCTDATA    pData = NULL;            // pointer to FCTDATA area
  PFCTSEGFILE pSegFile = NULL;         // pointer to FCTSEGFILE area
  USHORT      usRC = NO_ERROR;         // function return code

//  LOGWRITE1( "==EQFLoadSegFile==\n" );

  // validate session handle
  usRC = FctValidateSession( hSession, &pData );

  // check parameters
  if ( usRC  == NO_ERROR )
  {
    if ( (pszFullDocName == NULL) || (phLoadedFile == NULL)  )
    {
      usRC = ERROR_INVALID_PARAMETER;
    } /* endif */
  } /* endif */

  // allocate control area for loaded file
  if ( usRC  == NO_ERROR )
  {
    UtlAlloc( (PVOID *)&pSegFile, 0L, sizeof(FCTSEGFILE), NOMSG );
    if ( pSegFile != NULL )
    {
      pSegFile->lMagicWord = FCTSEGFILE_IDENTIFIER;
      pSegFile->pNext = (PVOID)pData->pSegFile;
      pSegFile->hSession = hSession;  // anchor session handle
      pData->pSegFile = pSegFile;
    }
    else
    {
      usRC = ERROR_NOT_ENOUGH_MEMORY;
    } /* endif */
  } /* endif */

  // initialize parser API in nonDDE mode
  if ( usRC  == NO_ERROR )
  {
    usRC = (USHORT)ParsInitialize( &(pSegFile->hParser), "<NONDDE>" );
  } /* endif */

  // load segmented document
  if ( usRC  == NO_ERROR )
  {
   usRC = (USHORT)ParsLoadSegFile( pSegFile->hParser, pszFullDocName, &(pSegFile->hParsSegFile));
  } /* endif */

  // return handle of pSegFile area or cleanup
  if ( usRC == NO_ERROR )
  {
    *phLoadedFile = (HANDLE)pSegFile;
  }
  else if ( pSegFile )
  {
    if ( pSegFile == pData->pSegFile ) pData->pSegFile = (PFCTSEGFILE)pSegFile->pNext;
    if ( pSegFile->hParser ) ParsTerminate( pSegFile->hParser );
    UtlAlloc( (PVOID *)&pSegFile, 0L, 0L, NOMSG );
  } /* endif */

//  LOGWRITE2( "  RC=%u\n", usRC );

  return( usRC );
} /*`end of function EqfLoadSegFile */

// get number of segments in loaded file
USHORT EqfGetSegNum
(
  HPARSSEGFILE hLoadedFile,            // handle of loaded segmented file
  PLONG        plNumOfSegs             // pointer to buffer for number of segments in file
)
{
  PFCTSEGFILE pSegFile;                // pointer to FCTSEGFILE area
  USHORT      usRC = NO_ERROR;         // function return code

  LOGWRITE1( "==EQFGetSegNum==\n" );

  // validate segfile handle
  pSegFile = (PFCTSEGFILE)hLoadedFile;
  if ( pSegFile == NULL )
  {
    usRC = ERROR_INVALID_SESSION_HANDLE;
  }
  else if ( pSegFile->lMagicWord != FCTSEGFILE_IDENTIFIER )
  {
    usRC = ERROR_INVALID_SESSION_HANDLE;
  } /* endif */

  // check parameters
  if ( usRC  == NO_ERROR )
  {
    if ( plNumOfSegs == NULL )
    {
      usRC = ERROR_INVALID_PARAMETER;
    } /* endif */
  } /* endif */

  // call parser API
  if ( usRC  == NO_ERROR )
  {
    usRC = (USHORT)ParsGetSegNum( pSegFile->hParsSegFile, plNumOfSegs );
  } /* endif */

  LOGWRITE2( "  RC=%u\n", usRC );

  return( usRC );
} /*`end of function EqfGetSegNum */

// free all memory occupied by a loaded file
USHORT EqfFreeSegFile
(
  HPARSSEGFILE  hLoadedFile            // handle of loaded segmented file
)
{
  PFCTSEGFILE pSegFile = NULL;         // pointer to FCTSEGFILE area
  USHORT      usRC = NO_ERROR;         // function return code
  BOOL        fSegFileValid = FALSE;

  LOGWRITE1( "==EQFFreeSegFile==\n" );

  // validate segfile handle
  pSegFile = (PFCTSEGFILE)hLoadedFile;
  if ( pSegFile == NULL )
  {
    usRC = ERROR_INVALID_SESSION_HANDLE;
  }
  else if ( pSegFile->lMagicWord != FCTSEGFILE_IDENTIFIER )
  {
    usRC = ERROR_INVALID_SESSION_HANDLE;
  }
  else
  {
    fSegFileValid = TRUE;
  } /* endif */

  // call parser API
  if ( usRC  == NO_ERROR )
  {
    usRC = (USHORT)ParsFreeSegFile( pSegFile->hParsSegFile );
  } /* endif */

  // free parser API
  if ( usRC  == NO_ERROR )
  {
    usRC = (USHORT)ParsTerminate( pSegFile->hParser );
  } /* endif */

  // remove seg file area from chain and free area memory
  if ( fSegFileValid )
  {
    PFCTDATA    pData = NULL;            // ptr to function data area
    FctValidateSession( pSegFile->hSession, &pData );
    if ( pData->pSegFile == pSegFile )
    {
      pData->pSegFile = (PFCTSEGFILE)pSegFile->pNext;
    }
    else
    {
      PFCTSEGFILE pPrevEntry = pData->pSegFile;
      while ( pPrevEntry->pNext && (pPrevEntry->pNext != pSegFile) )
      {
        pPrevEntry = (PFCTSEGFILE)pPrevEntry->pNext;
      } /* endwhile */
      if ( pPrevEntry->pNext == pSegFile )
      {
        pPrevEntry->pNext = pSegFile->pNext;
      } /* endif */
    } /* endif */
    UtlAlloc( (PVOID *)&pSegFile, 0L, 0L, NOMSG );
  } /* endif */

  LOGWRITE2( "  RC=%u\n", usRC );

  return( usRC );
} /*`end of function EqfFreeSegFile */

// write a segmented document to disk
USHORT EqfWriteSegFile
(
  HPARSSEGFILE hLoadedFile,            // handle of loaded segmented file
  PSZ          pszFullDocName          // fully qualified name of segmented document
)
{
  PFCTSEGFILE pSegFile;                // pointer to FCTSEGFILE area
  USHORT      usRC = NO_ERROR;         // function return code

  LOGWRITE1( "==EQFWriteSegFile==\n" );

  // validate segfile handle
  pSegFile = (PFCTSEGFILE)hLoadedFile;
  if ( pSegFile == NULL )
  {
    usRC = ERROR_INVALID_SESSION_HANDLE;
  }
  else if ( pSegFile->lMagicWord != FCTSEGFILE_IDENTIFIER )
  {
    usRC = ERROR_INVALID_SESSION_HANDLE;
  } /* endif */

  // check parameters
  if ( usRC  == NO_ERROR )
  {
    if ( pszFullDocName == NULL )
    {
      usRC = ERROR_INVALID_PARAMETER;
    } /* endif */
  } /* endif */

  // call parser API
  if ( usRC  == NO_ERROR )
  {
    usRC = (USHORT)ParsWriteSegFile( pSegFile->hParsSegFile, pszFullDocName );
  } /* endif */

  LOGWRITE2( "  RC=%u\n", usRC );

  return( usRC );
} /*`end of function EqfWriteSegFile */

// get the data for a specific segment
USHORT EqfGetSegW
(
  HPARSSEGFILE  hLoadedFile,           // handle of loaded segmented file
  LONG          lSegNum,               // number of requested segment
  PPARSSEGMENTW pSeg                   // ptr to buffer for segment data
)
{
  PFCTSEGFILE pSegFile;                // pointer to FCTSEGFILE area
  USHORT      usRC = NO_ERROR;         // function return code

  LOGWRITE1( "==EQFGetSegW==\n" );

  // validate segfile handle
  pSegFile = (PFCTSEGFILE)hLoadedFile;
  if ( pSegFile == NULL )
  {
    usRC = ERROR_INVALID_SESSION_HANDLE;
  }
  else if ( pSegFile->lMagicWord != FCTSEGFILE_IDENTIFIER )
  {
    usRC = ERROR_INVALID_SESSION_HANDLE;
  } /* endif */

  // check parameters
  if ( usRC  == NO_ERROR )
  {
    if ( pSeg == NULL )
    {
      usRC = ERROR_INVALID_PARAMETER;
    } /* endif */
  } /* endif */

  // call parser API
  if ( usRC  == NO_ERROR )
  {
    usRC = (USHORT)ParsGetSegW( pSegFile->hParsSegFile, lSegNum, pSeg );
  } /* endif */

  LOGWRITE2( "  RC=%u\n", usRC );

  return( usRC );
} /*`end of function EqfGetSegW */


// update the data of a specific segment
USHORT EqfUpdateSegW
(
  HPARSSEGFILE  hLoadedFile,           // handle of loaded segmented file
  LONG          lSegNum,               // number of segment to update
  PPARSSEGMENTW pSeg                   // ptr new segment data
)
{
  PFCTSEGFILE pSegFile;                // pointer to FCTSEGFILE area
  USHORT      usRC = NO_ERROR;         // function return code

  LOGWRITE1( "==EQFUpdateSegW==\n" );

  // validate segfile handle
  pSegFile = (PFCTSEGFILE)hLoadedFile;
  if ( pSegFile == NULL )
  {
    usRC = ERROR_INVALID_SESSION_HANDLE;
  }
  else if ( pSegFile->lMagicWord != FCTSEGFILE_IDENTIFIER )
  {
    usRC = ERROR_INVALID_SESSION_HANDLE;
  } /* endif */

  // check parameters
  if ( usRC  == NO_ERROR )
  {
    if ( pSeg == NULL )
    {
      usRC = ERROR_INVALID_PARAMETER;
    } /* endif */
  } /* endif */

  // call parser API
  if ( usRC  == NO_ERROR )
  {
    usRC = (USHORT)ParsUpdateSegW( pSegFile->hParsSegFile, lSegNum, pSeg );
  } /* endif */

  LOGWRITE2( "  RC=%u\n", usRC );

  return( usRC );
} /*`end of function EqfUpdateSegW */


// Emulate a specific operating system language we are running on....
// The selected system language will be active from now on.
// Example:
//     usRC = EqfSetSysLanaguage( hSession, "English(U.S.)" );
//
USHORT EqfSetSysLanguage
(
  HSESSION   hSession,                // callers Eqf session handle variable
  PSZ        pSystemPropLang          // System language to be set
)
{
  PFCTDATA    pData = NULL;            // pointer to FCTDATA area
  USHORT      usRC = NO_ERROR;         // function return code

  LOGWRITE1( "==EQFSetSysLanguage==\n" );
  LOGPARMSTRING( "Langage", pSystemPropLang );

  // validate session handle
  usRC = FctValidateSession( hSession, &pData );

  // check parameters
  if ( usRC  == NO_ERROR )
  {
    usRC = SetSystemLanguage( pSystemPropLang );
	if (usRC)
	{
		UtlError( usRC, MB_CANCEL, 1, &pSystemPropLang, EQF_ERROR );
	}

    pData->fComplete = TRUE;   // one-shot function are always complete
  }

  LOGWRITE2( "  RC=%u\n", usRC );

  return( usRC );
} /* end of function EqfSetSysLanguage */


USHORT EqfGetSysLanguage
(
  HSESSION   hSession,                // callers Eqf session handle variable
  PSZ        pSystemPropLang          // System language filled at output
)
{
  PFCTDATA    pData = NULL;            // pointer to FCTDATA area
  USHORT      usRC = NO_ERROR;         // function return code

  LOGWRITE1( "==EQFGetSysLanguage==\n" );

  // validate session handle
  usRC = FctValidateSession( hSession, &pData );

  // check parameters
  if ( usRC  == NO_ERROR )
  {
    strcpy( pSystemPropLang, GetSystemLanguage());

    pData->fComplete = TRUE;   // one-shot function are always complete
  }

  LOGPARMSTRING( "returned Langage", pSystemPropLang );
  LOGWRITE2( "  RC=%u\n", usRC );

  return( usRC );
} /* end of function EqfGetSysLanguage */


USHORT EqfGetProgress
(
  HSESSION   hSession,                // callers Eqf session handle variable
  PUSHORT    pusProgress              // pointer to buffer for current progress
)
{
  PFCTDATA    pData = NULL;            // pointer to FCTDATA area
  USHORT      usRC = NO_ERROR;         // function return code

#ifdef DEBUGAPI
  if ( iLogLevel >= 1)
  {
    LOGWRITE1( "==EQFGetProgress==\n" );
  } /* endif */
#endif


  // validate session handle
  usRC = FctValidateSession( hSession, &pData );

  // check parameters
  if ( usRC  == NO_ERROR )
  {
    if ( pusProgress == NULL )
    {
      usRC = ERROR_INVALID_PARAMETER;
    } /* endif */
  } /* endif */

  // set callers progress
  if ( usRC == NO_ERROR )
  {
    *pusProgress = pData->usProgress;
  }

#ifdef DEBUGAPI
  if ( iLogLevel >= 1)
  {
    LOGWRITE2( "  RC=%u\n", usRC );
  } /* endif */
#endif

  return( usRC );
} /* end of function EqfGetProgress */


// get the line information for a specific segment
USHORT EqfGetSourceLine
(
  HPARSSEGFILE hLoadedFile,            // handle of loaded segmented file
  LONG         lSegNum,                // number of requested segment
  PLONG        plStartLine,            // points to buffer for segment start line
  PLONG        plEndLine               // points to buffer for segment end line
)
{
  PFCTSEGFILE pSegFile;                // pointer to FCTSEGFILE area
  USHORT      usRC = NO_ERROR;         // function return code

  LOGWRITE1( "==EQFGetSourceLine==\n" );

  // validate segfile handle
  pSegFile = (PFCTSEGFILE)hLoadedFile;
  if ( pSegFile == NULL )
  {
    usRC = ERROR_INVALID_SESSION_HANDLE;
  }
  else if ( pSegFile->lMagicWord != FCTSEGFILE_IDENTIFIER )
  {
    usRC = ERROR_INVALID_SESSION_HANDLE;
  } /* endif */

  // check parameters
  if ( usRC  == NO_ERROR )
  {
    if ( (plStartLine == NULL ) || (plEndLine == NULL) )
    {
      usRC = ERROR_INVALID_PARAMETER;
    } /* endif */
  } /* endif */

  // call parser API
  if ( usRC  == NO_ERROR )
  {
     usRC = (USHORT)ParsGetSourceLine( pSegFile->hParsSegFile, lSegNum, plStartLine, plEndLine );
  } /* endif */

  LOGWRITE2( "  RC=%u\n", usRC );

  return( usRC );
} /*`end of function EqfGetSourceLine */

// get the segment number for a given line/column position
USHORT EqfGetSegmentNumber
(
  HPARSSEGFILE hLoadedFile,            // handle of loaded segmented file
  LONG         lLine,                  // line position of segment
  LONG         lColumn,                // columns position of segment
  PLONG        plSegNum                // number of segment found at given position
)
{
  PFCTSEGFILE pSegFile;                // pointer to FCTSEGFILE area
  USHORT      usRC = NO_ERROR;         // function return code

  LOGWRITE1( "==EQFGetSegmentNumber==\n" );

  // validate segfile handle
  pSegFile = (PFCTSEGFILE)hLoadedFile;
  if ( pSegFile == NULL )
  {
    usRC = ERROR_INVALID_SESSION_HANDLE;
  }
  else if ( pSegFile->lMagicWord != FCTSEGFILE_IDENTIFIER )
  {
    usRC = ERROR_INVALID_SESSION_HANDLE;
  } /* endif */

  // check parameters
  if ( usRC  == NO_ERROR )
  {
    if ( plSegNum == NULL )
    {
      usRC = ERROR_INVALID_PARAMETER;
    } /* endif */
  } /* endif */

  // call parser API
  if ( usRC  == NO_ERROR )
  {
    usRC = (USHORT)ParsGetSegmentNumber( pSegFile->hParsSegFile, lLine, lColumn, plSegNum );
  } /* endif */

  LOGWRITE2( "  RC=%u\n", usRC );

  return( usRC );
} /* end of function EqfGetSegmentNumber */

// function removing irrelevant (for the given folder) segments from an external memory
// the relevant segments are either stored in an internal or external memory
USHORT EqfCleanMemory
(
  HSESSION         hSession,           // Eqf session handle
  PSZ              pszFolder,          // name of folder containing the translatable material
  PSZ              pszInMemory,        // fully qualified name of external input memory (encoding: UTF-16)
  PSZ              pszOutMemory,       // name of internal output memory or fully qualified name of external output memory
  LONG             lOptions            // options for processing
)
{
  USHORT      usRC = NO_ERROR;         // function return code
  PFCTDATA    pData = NULL;            // ptr to function data area

  // validate session handle
  usRC = FctValidateSession( hSession, &pData );

  if ( pData && (pData->fComplete || (pData->sLastFunction != FCT_EQFCLEANMEMORY)) )
  {
    LOGWRITE1( "==EQFCleanMemory==\n" );
    LOGPARMSTRING( "Folder", pszFolder );
    LOGPARMSTRING( "InMemory", pszInMemory );
    LOGPARMSTRING( "OutMemory", pszOutMemory );
    LOGPARMOPTION( "Options", lOptions );
#ifdef SESSIONLOG
    UtlLogWriteString( "EqfCleanMemory: Starting, Folder = %s", pszFolder );
#endif
  } /* endif */

  // check sequence of calls
  if ( usRC == NO_ERROR )
  {
    if ( !pData->fComplete && (pData->sLastFunction != FCT_EQFCLEANMEMORY) )
    {
      usRC = LASTTASK_INCOMPLETE_RC;
    } /* endif */
  } /* endif */

  // call clean memory function
  if ( usRC == NO_ERROR )
  {
    pData->sLastFunction = FCT_EQFCLEANMEMORY;
    usRC = MemFuncCleanMemory( pData, pszFolder, pszInMemory, pszOutMemory, lOptions );
    if ( lOptions & CLEANMEM_COMPLETE_IN_ONE_CALL_OPT )
    {
      if ( (usRC == NO_ERROR) && !pData->fComplete ) usRC = CONTINUE_RC;
      while ( usRC == CONTINUE_RC )
      {
        usRC = MemFuncCleanMemory( pData, pszFolder, pszInMemory, pszOutMemory, lOptions );
        if ( (usRC == NO_ERROR) && !pData->fComplete ) usRC = CONTINUE_RC;
      } /*endwhile */
    } /* endif */
  } /* endif */

  if ( (usRC == NO_ERROR) && !pData->fComplete )
  {
    usRC = CONTINUE_RC;
  } /* endif */

  if ( pData && pData->fComplete )
  {
    LOGWRITE2( "  RC=%u\n", usRC );
#ifdef SESSIONLOG
    CHAR szRC[10];
    sprintf( szRC, "%u", usRC );
    UtlLogWriteString( "EqfCleanMemory: Process complete, rc=%s", szRC );
#endif
  }

  return( usRC );

} /* end of function EqfCleanMemory */


USHORT EqfRename
(
  HSESSION         hSession,           // Eqf session handle
  USHORT           usMode,             // rename mode: RENAME_FOLDER, RENAME_MEMORY, RENAME_DICTIONARY
  PSZ              pszName,            // name of object being renamed
  PSZ              pszNewName,         // new name of object
  LONG             lOptions            // options for processing, valid are ADJUSTREFERENCES_OPT
)
{
  USHORT      usRC = NO_ERROR;         // function return code
  PFCTDATA    pData = NULL;            // ptr to function data area

  // validate session handle
  usRC = FctValidateSession( hSession, &pData );

  if ( pData && (pData->fComplete || (pData->sLastFunction != FCT_EQFRENAME)) )
  {
    PSZ pszMode = "Unknown";

    if ( usMode == RENAME_FOLDER )     pszMode = "Folder";
    if ( usMode == RENAME_MEMORY )     pszMode = "Memory";
    if ( usMode == RENAME_DICTIONARY ) pszMode = "Dictionary";

    LOGWRITE1( "==EQFRENAME==\n" );
    LOGPARMSTRING( "Mode",    pszMode );
    LOGPARMSTRING( "Name",    pszName );
    LOGPARMSTRING( "NewName", pszNewName );
    LOGPARMOPTION( "Options", lOptions );
  } /* endif */

  // check sequence of calls
  if ( usRC == NO_ERROR )
  {
    if ( !pData->fComplete && (pData->sLastFunction != FCT_EQFRENAME) )
    {
      usRC = LASTTASK_INCOMPLETE_RC;
    } /* endif */
  } /* endif */

  // call rename object function
  if ( usRC == NO_ERROR )
  {
    pData->sLastFunction = FCT_EQFRENAME;
    usRC = UtlFuncRenameObject( usMode, pszName, pszNewName, lOptions );
    pData->fComplete = TRUE;
  } /* endif */

  LOGWRITE2( "  RC=%u\n", usRC );

  return( usRC );

} /* end of function EqfRename */

// API function MemFuncProcessNomatch
// which runs one or more SNOMATCH files against a memory and copies all relevant memory proposals
// to the output memory, for the segments contained in the SNOMATCH files a memory 
// match count report and a duplicate word count report is created
USHORT EqfProcessNomatch
(
  HSESSION    hSession,                // Eqf session handle
  PSZ         pszNomatch,              // fully qualified name of the SNOMATCh file, folder name or search path
  PSZ         pszInMemory,             // name of the internal input memory
  PSZ         pszOutMemory,            // name of internal output memory (is created if it does not exist)
  PSZ         pszMemMatchReportText,   // fully qualified file name of the memory match count report (text format)
  PSZ         pszMemMatchReportXml,    // fully qualified file name of the memory match count report (XML format)
  PSZ         pszDupReportText,        // fully qualified file name of the duplicate word count report (text format)
  PSZ         pszDupReportXml,         // fully qualified file name of the duplicate word count report (XML format)
  LONG        lOptions                 // options for processing
)
{
  USHORT      usRC = NO_ERROR;         // function return code
  PFCTDATA    pData = NULL;            // ptr to function data area

  // validate session handle
  usRC = FctValidateSession( hSession, &pData );

  if ( pData && (pData->fComplete || (pData->sLastFunction != FCT_EQFPROCESSNOMATCH)) )
  {
    LOGWRITE1( "==EQFProcessNomatch==\n" );
    LOGPARMSTRING( "NoMatch", pszNomatch );
    LOGPARMSTRING( "InMemory", pszInMemory );
    LOGPARMSTRING( "OutMemory", pszOutMemory );
    LOGPARMSTRING( "MemMatchReportText", pszMemMatchReportText );
    LOGPARMSTRING( "MemMatchReportXml", pszMemMatchReportXml );
    LOGPARMSTRING( "DupReportText", pszDupReportText );
    LOGPARMSTRING( "DupReportXml", pszDupReportText );
    LOGPARMOPTION( "Options", lOptions );
#ifdef SESSIONLOG
    UtlLogWriteString2( "EqfProcessNomatch: Starting, pszNoMatch = %s, pszInMemory= %s", pszNomatch, pszInMemory );
    UtlLogWriteString( "EqfProcessNomatch:           pszOutMemory = %s, ", pszOutMemory );
#endif
  } /* endif */

  // check sequence of calls
  if ( usRC == NO_ERROR )
  {
    if ( !pData->fComplete && (pData->sLastFunction != FCT_EQFPROCESSNOMATCH) )
    {
      usRC = LASTTASK_INCOMPLETE_RC;
    } /* endif */
  } /* endif */

  // call the API function
  if ( usRC == NO_ERROR )
  {
    pData->sLastFunction = FCT_EQFPROCESSNOMATCH;
    usRC = MemFuncProcessNomatch( pData, pszNomatch, pszInMemory, pszOutMemory, pszMemMatchReportText, 
              pszMemMatchReportXml, pszDupReportText, pszDupReportXml, NULL, NULL, lOptions );
    if ( lOptions & COMPLETE_IN_ONE_CALL_OPT )
    {
      if ( (usRC == NO_ERROR) && !pData->fComplete ) usRC = CONTINUE_RC;
      while ( usRC == CONTINUE_RC )
      {
        usRC = MemFuncProcessNomatch( pData, pszNomatch, pszInMemory, pszOutMemory, pszMemMatchReportText, 
                 pszMemMatchReportXml, pszDupReportText, pszDupReportXml, NULL, NULL, lOptions );
        if ( (usRC == NO_ERROR) && !pData->fComplete ) usRC = CONTINUE_RC;
      } /*endwhile */
    } /* endif */
  } /* endif */

  if ( (usRC == NO_ERROR) && !pData->fComplete )
  {
    usRC = CONTINUE_RC;
  } /* endif */

  if ( pData && pData->fComplete )
  {
    LOGWRITE2( "  RC=%u\n", usRC );
#ifdef SESSIONLOG
    CHAR szRC[10];
    sprintf( szRC, "%u", usRC );
    UtlLogWriteString( "EqfProcessNomatch: Process complete, rc=%s", szRC );
#endif
  } /* endif */     

  return( usRC );

} /* end of function EqfProcessNomatch */


// API function to delete the MT log files of a specific folder
USHORT EqfDeleteMTLog
(
  HSESSION    hSession,                // Eqf session handle
  PSZ         pszFolderName            // name of the folder
)
{
  USHORT      usRC = NO_ERROR;         // function return code
  PFCTDATA    pData = NULL;            // ptr to function data area

  // validate session handle
  usRC = FctValidateSession( hSession, &pData );

  if ( pData  )
  {
    LOGWRITE1( "==EQFDeleteMTLog==\n" );
    LOGPARMSTRING( "Folder", pszFolderName );
  } /* endif */

  // call the API function
  if ( usRC == NO_ERROR )
  {
    usRC = FolFuncDeleteMTLog( pszFolderName );
  } /* endif */

  return( usRC );
} /* end of function EqfDeleteMTLog */

// API function MemFuncProcessNomatch
// which runs one or more SNOMATCH files against a memory and copies all relevant memory proposals
// to the output memory, for the segments contained in the SNOMATCH files a memory 
// match count report and a duplicate word count report is created
USHORT EqfProcessNomatchEx
(
  HSESSION    hSession,                // Eqf session handle
  PSZ         pszNomatch,              // fully qualified name of the SNOMATCh file, folder name or search path
  PSZ         pszInMemory,             // name of the internal input memory
  PSZ         pszOutMemory,            // name of internal output memory (is created if it does not exist)
  PSZ         pszMemMatchReportText,   // fully qualified file name of the memory match count report (text format)
  PSZ         pszMemMatchReportXml,    // fully qualified file name of the memory match count report (XML format)
  PSZ         pszDupReportText,        // fully qualified file name of the duplicate word count report (text format)
  PSZ         pszDupReportXml,         // fully qualified file name of the duplicate word count report (XML format)
  PSZ         pszOutNomatchXml,        // fully qualified file name of the output nomatch file in nfluent XML format
  PSZ         pszOutNomatchExp,        // fully qualified file name of the output nomatch file in TM EXP format
  LONG        lOptions                 // options for processing
)
{
  USHORT      usRC = NO_ERROR;         // function return code
  PFCTDATA    pData = NULL;            // ptr to function data area

  // validate session handle
  usRC = FctValidateSession( hSession, &pData );

  if ( pData && (pData->fComplete || (pData->sLastFunction != FCT_EQFPROCESSNOMATCH)) )
  {
    LOGWRITE1( "==EQFProcessNomatchEx==\n" );
    LOGPARMSTRING( "NoMatch", pszNomatch );
    LOGPARMSTRING( "InMemory", pszInMemory );
    LOGPARMSTRING( "OutMemory", pszOutMemory );
    LOGPARMSTRING( "MemMatchReportText", pszMemMatchReportText );
    LOGPARMSTRING( "MemMatchReportXml", pszMemMatchReportXml );
    LOGPARMSTRING( "DupReportText", pszDupReportText );
    LOGPARMSTRING( "DupReportXml", pszDupReportText );
    LOGPARMSTRING( "OutNomatchXml", pszOutNomatchXml );
    LOGPARMSTRING( "OutNomatchExp", pszOutNomatchExp );
    LOGPARMOPTION( "Options", lOptions );
#ifdef SESSIONLOG
    UtlLogWriteString2( "EqfProcessNomatchEx: Starting, pszNoMatch = %s, pszInMemory= %s", pszNomatch, pszInMemory );
    UtlLogWriteString( "EqfProcessNomatchEx:           pszOutMemory = %s, ", pszOutMemory );
#endif
  } /* endif */

  // check sequence of calls
  if ( usRC == NO_ERROR )
  {
    if ( !pData->fComplete && (pData->sLastFunction != FCT_EQFPROCESSNOMATCH) )
    {
      usRC = LASTTASK_INCOMPLETE_RC;
    } /* endif */
  } /* endif */

  // call the API function
  if ( usRC == NO_ERROR )
  {
    pData->sLastFunction = FCT_EQFPROCESSNOMATCH;
    usRC = MemFuncProcessNomatch( pData, pszNomatch, pszInMemory, pszOutMemory, pszMemMatchReportText, 
              pszMemMatchReportXml, pszDupReportText, pszDupReportXml, pszOutNomatchXml, pszOutNomatchExp, lOptions );
    if ( lOptions & COMPLETE_IN_ONE_CALL_OPT )
    {
      if ( (usRC == NO_ERROR) && !pData->fComplete ) usRC = CONTINUE_RC;
      while ( usRC == CONTINUE_RC )
      {
        usRC = MemFuncProcessNomatch( pData, pszNomatch, pszInMemory, pszOutMemory, pszMemMatchReportText, 
                 pszMemMatchReportXml, pszDupReportText, pszDupReportXml, pszOutNomatchXml, pszOutNomatchExp, lOptions );
        if ( (usRC == NO_ERROR) && !pData->fComplete ) usRC = CONTINUE_RC;
      } /*endwhile */
    } /* endif */
  } /* endif */

  if ( (usRC == NO_ERROR) && !pData->fComplete )
  {
    usRC = CONTINUE_RC;
  } /* endif */

  if ( pData && pData->fComplete )
  {
    LOGWRITE2( "  RC=%u\n", usRC );
#ifdef SESSIONLOG
    CHAR szRC[10];
    sprintf( szRC, "%u", usRC );
    UtlLogWriteString( "EqfProcessNomatchEx: Process complete, rc=%s", szRC );
#endif
  } /* endif */     

  return( usRC );

} /* end of function EqfProcessNomatchEx */



// API function to open a document in translation manager
USHORT EqfOpenDoc
(
  HSESSION    hSession,                // Eqf session handle
  PSZ         pszFolderName,           // name of the folder
  PSZ         pszDocument,             // name of document
  ULONG       ulSegNum,                // segment number of segment to be activated
  ULONG       ulLine                   // line to be activated (ulSegNum has to be 0)
)
{
  return( EqfOpenDocEx( hSession, pszFolderName, pszDocument, ulSegNum, ulLine, NULL ) );
} /* end of function EqfOpenDoc */

USHORT EqfOpenDocEx
(
  HSESSION    hSession,                // Eqf session handle
  PSZ         pszFolderName,           // name of the folder
  PSZ         pszDocument,             // name of document
  ULONG       ulSegNum,                // segment number of segment to be activated
  ULONG       ulLine,                  // line to be activated (ulSegNum has to be 0)
  PSZ_W       pszSearch                // UTF-16 search string (ulSegNum and ulLine have to be 0)
)
{
  USHORT      usRC = NO_ERROR;         // function return code
  PFCTDATA    pData = NULL;            // ptr to function data area

  // validate session handle
  usRC = FctValidateSession( hSession, &pData );

  if ( pData  )
  {
    LOGWRITE1( "==EQFOpenDoc==\n" );
    LOGPARMSTRING( "Folder", pszFolderName );
    LOGPARMSTRING( "Document", pszDocument );
  } /* endif */

  // call the API function
  if ( usRC == NO_ERROR )
  {
    usRC = FolFuncOpenDoc( pszFolderName, pszDocument, ulSegNum, ulLine, pszSearch, NULL );
  } /* endif */

  return( usRC );
} /* end of function EqfOpenDocEx */



// API function to open a document in editor by TVT tracking ID
USHORT EqfOpenDocByTrack
(
  HSESSION    hSession,                // Eqf session handle
  PSZ         pszFolderName,           // name of the folder
  PSZ         pszTrackID               // TVT tracking ID to activate
)
{
   USHORT      usRC = NO_ERROR;         // function return code
   PFCTDATA    pData = NULL;            // ptr to function data area

   // validate session handle
   usRC = FctValidateSession( hSession, &pData );

   if ( pData  )
   {
     LOGWRITE1( "==EQFOpenDocByTrack==\n" );
     LOGPARMSTRING( "Folder", pszFolderName );
     LOGPARMSTRING( "TrackID", pszTrackID );
   } /* endif */

   // call the API function
   if ( usRC == NO_ERROR )
   {
     usRC = FolFuncOpenDoc( pszFolderName, NULL, 0L, 0L, NULL, pszTrackID );
   } /* endif */

   return( usRC );
} /* end of function EqfOpenDocByTrack */


// API function to convert the long folder/dictionaries/memories/document names to the internally used short name
USHORT EqfGetShortName
(
  HSESSION    hSession,                // Eqf session handle
  USHORT      ObjectType,              // type of object being processed
  PSZ         pszLongName,             // long name of the object
  PSZ         pszShortName             // buffer for retured short name
)
{
  USHORT      usRC = NO_ERROR;         // function return code
  PFCTDATA    pData = NULL;            // ptr to function data area

  // validate session handle
  usRC = FctValidateSession( hSession, &pData );

  if ( pData  )
  {
    LOGWRITE1( "==EqfGetShortName==\n" );
    LOGPARMSTRING( "LongName", pszLongName );
    LOGPARMUSHORT( "ObjectType", ObjectType );
  } /* endif */

  // call the API function
  if ( usRC == NO_ERROR )
  {
    usRC = UtlFuncGetShortName( ObjectType, pszLongName, pszShortName );
  } /* endif */

  if ( pData ) LOGWRITE2( "  RC=%u\n", usRC );

  return( usRC );
} /* end of function EqfGetShortName */

// API function to remove a group of documents based on a list of document names in a plain text file
USHORT EqfRemoveDocs
(
  HSESSION    hSession,                // Eqf session handle
  PSZ         pszFolderName,           // name of the folder
  PSZ         pszListFile              // name of text file containing the list of document names
)
{
  USHORT      usRC = NO_ERROR;         // function return code
  PFCTDATA    pData = NULL;            // ptr to function data area

  // validate session handle
  usRC = FctValidateSession( hSession, &pData );

  if ( pData  )
  {
    LOGWRITE1( "==EqfRemoveDocs==\n" );
    LOGPARMSTRING( "FolderName", pszFolderName );
    LOGPARMSTRING( "ListFileName", pszListFile );
  } /* endif */

  // call the API function
  if ( usRC == NO_ERROR )
  {
    usRC = FolFuncRemoveDocs( pszFolderName, pszListFile );
  } /* endif */

  if ( pData ) LOGWRITE2( "  RC=%u\n", usRC );

  return( usRC );
}

// API function to restore a group of documents removed using the EqfRemoveDocs API call
USHORT EqfRestoreDocs
(
  HSESSION    hSession,                // Eqf session handle
  PSZ         pszFolderName            // name of the folder
)
{
  USHORT      usRC = NO_ERROR;         // function return code
  PFCTDATA    pData = NULL;            // ptr to function data area

  // validate session handle
  usRC = FctValidateSession( hSession, &pData );

  if ( pData  )
  {
    LOGWRITE1( "==EqfRestoreDocs==\n" );
    LOGPARMSTRING( "FolderName", pszFolderName );
  } /* endif */

  // call the API function
  if ( usRC == NO_ERROR )
  {
    usRC = FolFuncRestoreDocs( pszFolderName );
  } /* endif */

  if ( pData ) LOGWRITE2( "  RC=%u\n", usRC );

  return( usRC );
}

// API function to associate a Global memory CTID options file to a folder
USHORT EqfAddCTIDList
(
  HSESSION    hSession,                // Eqf session handle
  PSZ         pszFolderName,           // name of the folder
  PSZ         pszCTIDListFile          // fully qualified name of the CTID list file
)
{
  USHORT      usRC = NO_ERROR;         // function return code
  PFCTDATA    pData = NULL;            // ptr to function data area

  // validate session handle
  usRC = FctValidateSession( hSession, &pData );

  if ( pData  )
  {
    LOGWRITE1( "==EqfAddCTIDList==\n" );
    LOGPARMSTRING( "FolderName", pszFolderName );
    LOGPARMSTRING( "CTIDListFile", pszCTIDListFile );
  } /* endif */

  // call the API function
  if ( usRC == NO_ERROR )
  {
    usRC = FolFuncAddCTIDList( pszFolderName, pszCTIDListFile );
  } /* endif */

  if ( pData ) LOGWRITE2( "  RC=%u\n", usRC );

  return( usRC );
}

// API function to export segments 
USHORT EqfExportSegs
(
  HSESSION    hSession,                // Eqf session handle
  PSZ         pszFolderName,           // name of folder
  PSZ         pszDocuments,            // list with document names or NULL
  PSZ         pszStartStopFile,        // file containing start/stop tag list
  PSZ         pszOutFile,              // name of output file
  LONG        lOptions                 // options 
)
{
  USHORT      usRC = NO_ERROR;         // function return code
  PFCTDATA    pData = NULL;            // ptr to function data area

  // validate session handle
  usRC = FctValidateSession( hSession, &pData );

  if ( pData && (pData->fComplete || (pData->sLastFunction != FCT_EQFEXPORTSEGS)) )
  {
    LOGWRITE1( "==EQFExportSegs==\n" );
    LOGPARMSTRING( "Folder", pszFolderName );
    LOGPARMSTRING( "Documents", pszDocuments );
    LOGPARMSTRING( "StartStopFile", pszStartStopFile );
    LOGPARMSTRING( "OutFile", pszOutFile );
    LOGPARMOPTION( "Options", lOptions );
#ifdef SESSIONLOG
    UtlLogWriteString2( "EqfExportSegs: Starting, pszFolder = %s, pszStartStopFile= %s", pszFolderName, pszStartStopFile );
    UtlLogWriteString( "EqfExportSegs:           pszOutFile = %s, ", pszOutFile );
#endif
  } /* endif */
 
  // check sequence of calls
  if ( usRC == NO_ERROR )
  {
    if ( !pData->fComplete && (pData->sLastFunction != FCT_EQFEXPORTSEGS) )
    {
      usRC = LASTTASK_INCOMPLETE_RC;
    } /* endif */
  } /* endif */

  // call the API function
  if ( usRC == NO_ERROR )
  {
    pData->sLastFunction = FCT_EQFEXPORTSEGS;
    // TODO migrate TAFuncExportSegs from TM to OTM
    // usRC = TAFuncExportSegs( pData, pszFolderName, pszDocuments, pszStartStopFile, pszOutFile, lOptions );
    if ( lOptions & COMPLETE_IN_ONE_CALL_OPT )
    {
      if ( (usRC == NO_ERROR) && !pData->fComplete ) usRC = CONTINUE_RC;
      while ( usRC == CONTINUE_RC )
      {
        // TODO migrate TAFuncExportSegs from TM to OTM
        // usRC = TAFuncExportSegs( pData, pszFolderName, pszDocuments, pszStartStopFile, pszOutFile, lOptions );
        if ( (usRC == NO_ERROR) && !pData->fComplete ) usRC = CONTINUE_RC;
      } /*endwhile */
    } /* endif */
  } /* endif */

  if ( (usRC == NO_ERROR) && !pData->fComplete )
  {
    usRC = CONTINUE_RC;
  } /* endif */

  if ( pData && pData->fComplete )
  {
    LOGWRITE2( "  RC=%u\n", usRC );
#ifdef SESSIONLOG
    CHAR szRC[10];
    sprintf( szRC, "%u", usRC );
    UtlLogWriteString( "EqfExportSegs: Process complete, rc=%s", szRC );
#endif
  } /* endif */     

  return( usRC );

} /* end of function EqfExportSegs */


// API function checking the existence of a folder
// 
USHORT EqfFolderExists
(
  HSESSION    hSession,                // Eqf session handle
  PSZ         pszFolderName            // name of folder
)
{
  USHORT      usRC = NO_ERROR;         // function return code
  PFCTDATA    pData = NULL;            // ptr to function data area

  // validate session handle
  usRC = FctValidateSession( hSession, &pData );

  // check if folder exists
  if ( usRC == NO_ERROR )
  {
    if ( (pszFolderName == NULL) || (*pszFolderName == EOS) )
    {
      usRC = TA_MANDFOLDER;
      UtlErrorHwnd( usRC, MB_CANCEL, 0, NULL, EQF_ERROR, HWND_FUNCIF );
    }
    else
    {
      BOOL fIsNew = FALSE;
      CHAR szShortName[MAX_FILESPEC];

      ObjLongToShortName( pszFolderName, szShortName, FOLDER_OBJECT, &fIsNew);
      if ( fIsNew )
      {
        PSZ pszParm = pszFolderName;
        usRC = ERROR_XLATE_FOLDER_NOT_EXIST;
        UtlErrorHwnd( usRC, MB_CANCEL, 1, &pszParm, EQF_ERROR, HWND_FUNCIF );
      } /* endif */
    } /* endif */
  } /* endif */

  return( usRC );
} /* end of function EqfFolderExists */

// API function checking the existence of a translation memory
// 
USHORT EqfMemoryExists
(
  HSESSION    hSession,                // Eqf session handle
  PSZ         pszMemoryName            // name of the memory
)
{
  USHORT      usRC = NO_ERROR;         // function return code
  PFCTDATA    pData = NULL;            // ptr to function data area

  // validate session handle
  usRC = FctValidateSession( hSession, &pData );

  // check if folder exists
  if ( usRC == NO_ERROR )
  {
    if ( (pszMemoryName == NULL) || (*pszMemoryName == EOS) )
    {
      usRC = TA_MANDFOLDER;
      UtlErrorHwnd( usRC, MB_CANCEL, 0, NULL, EQF_ERROR, HWND_FUNCIF );
    }
    else
    {
      BOOL fIsNew = FALSE;
      CHAR szShortName[MAX_FILESPEC];

      ObjLongToShortName( pszMemoryName, szShortName, TM_OBJECT, &fIsNew);
      if ( fIsNew )
      {
        PSZ pszParm = pszMemoryName;
        usRC = ERROR_MEMORY_NOTFOUND;
        UtlErrorHwnd( usRC, MB_CANCEL, 1, &pszParm, EQF_ERROR, HWND_FUNCIF );
      } /* endif */
    } /* endif */
  } /* endif */

  return( usRC );
} /* end of function EqfMemoryExists */


// API function checking the existence of a dictionary
// 
USHORT EqfDictionaryExists
(
  HSESSION    hSession,                // Eqf session handle
  PSZ         pszDictionaryName        // name of the dictionary
)
{
  USHORT      usRC = NO_ERROR;         // function return code
  PFCTDATA    pData = NULL;            // ptr to function data area

  // validate session handle
  usRC = FctValidateSession( hSession, &pData );

  if ( usRC == NO_ERROR )
  {
    if ( (pszDictionaryName == NULL) || (*pszDictionaryName == EOS) )
    {
      usRC = TA_MANDFOLDER;
      UtlErrorHwnd( usRC, MB_CANCEL, 0, NULL, EQF_ERROR, HWND_FUNCIF );
    }
    else
    {
      BOOL fIsNew = FALSE;
      CHAR szShortName[MAX_FILESPEC];

      ObjLongToShortName( pszDictionaryName, szShortName, DICT_OBJECT, &fIsNew);
      if ( fIsNew )
      {
        PSZ pszParm = pszDictionaryName;
        usRC = ERROR_DIC_NOTFOUND;
        UtlErrorHwnd( usRC, MB_CANCEL, 1, &pszParm, EQF_ERROR, HWND_FUNCIF );
      } /* endif */
    } /* endif */
  } /* endif */

  return( usRC );
} /* end of function EqfDictionaryExists */


// API function checking the existence of a document within the given folder
// 
USHORT EqfDocumentExists
(
  HSESSION    hSession,                // Eqf session handle
  PSZ         pszFolderName ,          // name of folder containing the document
  PSZ         pszDocumentName          // name of the document
)
{
  USHORT      usRC = NO_ERROR;         // function return code
  PFCTDATA    pData = NULL;            // ptr to function data area

  // validate session handle
  usRC = FctValidateSession( hSession, &pData );

  if ( usRC == NO_ERROR )
  {
    if ( (pszFolderName == NULL) || (*pszFolderName == EOS) )
    {
      usRC = TA_MANDFOLDER;
      UtlErrorHwnd( usRC, MB_CANCEL, 0, NULL, EQF_ERROR, HWND_FUNCIF );
    }
    else if ( (pszDocumentName == NULL) || (*pszDocumentName == EOS) )
    {
      usRC = TA_MANDFOLDER;
      UtlErrorHwnd( usRC, MB_CANCEL, 0, NULL, EQF_ERROR, HWND_FUNCIF );
    }
    else
    {
      BOOL fIsNew = FALSE;
      CHAR szFolObject[MAX_EQF_PATH];

      fIsNew = !SubFolNameToObjectName( pszFolderName,  szFolObject );
      if ( fIsNew )
      {
        PSZ pszParm = pszFolderName;
        usRC = ERROR_XLATE_FOLDER_NOT_EXIST;
        UtlErrorHwnd( usRC, MB_CANCEL, 1, &pszParm, EQF_ERROR, HWND_FUNCIF );
      }
      else
      {
        CHAR szShortName[MAX_FILESPEC];
        FolLongToShortDocName( szFolObject, pszDocumentName, szShortName, &fIsNew );
        if ( fIsNew )
        {
           PSZ apszParms[2];
           apszParms[0] = pszDocumentName;
           apszParms[1] = pszFolderName;
           usRC = DDE_DOC_NOT_IN_FOLDR;
           UtlErrorHwnd( usRC, MB_CANCEL, 2, apszParms, EQF_ERROR, HWND_FUNCIF );
        } /* endif */           
      } /* endif */
    } /* endif */
  } /* endif */

  return( usRC );
} /* end of function EqfDocumentExists */


/**
* \brief Count the number of words in a given string
*
* \param hSession OpenTM2 session handle returned by EqfStartSession
*	\param pszMarkup name of the markup table to be used for the recognition of in-line tags,
*  if this parameter is NULL no in-line tag recognition will be performed
*	\param pszLanguage OpenTM2 name for the language of the given text
*	\param pszText null-terminated string containing the text to be counted, the encoding is UTF-16
* \param pulWords points to an unsigned long value receiving the number of words in the text
* \param pulInlineTags points to an unsigned long value receiving the number of inline tags in the text
*	\returns 0 if successful or an error code
*	
**/
unsigned short EqfCountWordsInString
(
  HSESSION    hSession,                // Eqf session handle
  char        *pszMarkup,              // name of the markup table to be used for the recognition of in-line tags
  char        *pszLanguage,            // OpenTM2 name for the language of the given text
  wchar_t     *pszText,                // null-terminated string containing the text to be counted, the encoding is UTF-16
  unsigned long *pulWords,             // points to an unsigned long value receiving the number of words in the text
  unsigned long *pulInlineTags         // points to an unsigned long value receiving the number of inline tags in the text
)
{
  unsigned short usRC = NO_ERROR;      // function return code
  PFCTDATA    pData = NULL;            // ptr to function data area

  // validate session handle
  usRC = FctValidateSession( hSession, &pData );

  // check parameters
  if ( usRC == NO_ERROR )
  {
    if ( (pszLanguage == NULL) || (pszText == NULL) || (pulWords == NULL) || (pulInlineTags == NULL) )
    {
      usRC = ERROR_INVALID_PARAMETER;
    } /* endif */
  } /* endif */

  // return when input string is empty
  if ( usRC == NO_ERROR )
  {
    *pulWords = 0;
    *pulInlineTags = 0;
    if ( *pszText == 0 ) return( 0 );
  } /* endif */

  // call word counting code
  if ( usRC == NO_ERROR )
  {
    usRC = CntFuncCountWordsInstring( pszMarkup, pszLanguage, pszText, pulWords, pulInlineTags );
  }

  return( usRC );
};


/**
* \brief Check the spelling of a list of words
*
* \param hSession OpenTM2 session handle returned by EqfStartSession
*	\param pszLanguage name of the language being used for the spell checking
*	\param pszInputTerms a comma seperated list of terms or NULL if a input file is being used
*	\param pszInputFile the fully qualified name of a plain text file containing the terms, one term per line or NULL if pszInputTerms is being used
* \param pszReport name of the report file receiving the results of the operation
* \param lOption options for the output of the report: TEXT_OUTPUT_OPT for plain text output (CSV) or XML_OUTPUT_OPT (default) for XML output

*	\returns 0 if successful or an error code
*	
**/
unsigned short EqfCheckSpelling
(
  HSESSION    hSession,                // Eqf session handle
  char *pszLanguage,                   // name of the language being used for the spell checking
  char *pszInputTerms,                 // a comma seperated list of terms or NULL if a input file is being used
  char *pszInputFile,                  // the fully qualified name of a plain text file containing the terms, one term per line or NULL if pszInputTemrs is being used
  char *pszReport,                     // name of the report file receiving the results of the operation, the report is in the XML format
  long lOptions
)
{
  unsigned short usRC = NO_ERROR;      // function return code
  PFCTDATA    pData = NULL;            // ptr to function data area

  // validate session handle
  usRC = FctValidateSession( hSession, &pData );

  // check parameters
  if ( usRC == NO_ERROR )
  {
    if ( (pszLanguage == NULL) || (pszReport == NULL) || (*pszReport == '\0') || 
         ((pszInputTerms == NULL) && (pszInputFile == NULL))  )
    {
      usRC = ERROR_INVALID_PARAMETER;
    } /* endif */
  } /* endif */

  // call spell checking function
  if ( usRC == NO_ERROR )
  {
    usRC = MorphFuncCheckSpelling( pszLanguage, pszInputTerms, pszInputFile, pszReport, lOptions );
  }

  return( usRC );
}

/**
* \brief Reduce a list of words to their stem form
*
* \param hSession OpenTM2 session handle returned by EqfStartSession
*	\param pszLanguage name of the language being used for the spell checking
*	\param pszInputTerms a comma seperated list of terms or NULL if a input file is being used
*	\param pszInputFile the fully qualified name of a plain text file containing the terms, one term per line or NULL if pszInputTerms is being used
* \param pszReport name of the report file receiving the results of the operation
* \param lOption options for the output of the report: TEXT_OUTPUT_OPT for plain text output (CSV) or XML_OUTPUT_OPT (default) for XML output
*	\returns 0 if successful or an error code
*	
**/
unsigned short EqfReduceToStemForm
(
  HSESSION    hSession,                // Eqf session handle
  char *pszLanguage,                   // name of the language being used for the stem form reduction
  char *pszInputTerms,                 // a comma seperated list of terms or NULL if a input file is being used
  char *pszInputFile,                  // the fully qualified name of a plain text file containing the terms, one term per line or NULL if pszInputTemrs is being used
  char *pszReport,                     // name of the report file receiving the results of the operation, the report is in the XML format
  long lOptions
)
{
  unsigned short usRC = NO_ERROR;      // function return code
  PFCTDATA    pData = NULL;            // ptr to function data area

  // validate session handle
  usRC = FctValidateSession( hSession, &pData );

  // check parameters
  if ( usRC == NO_ERROR )
  {
    if ( (pszLanguage == NULL) || (pszReport == NULL) || (*pszReport == '\0') || 
         ((pszInputTerms == NULL) && (pszInputFile == NULL))  )
    {
      usRC = ERROR_INVALID_PARAMETER;
    } /* endif */
  } /* endif */

  // call spell checking function
  if ( usRC == NO_ERROR )
  {
    usRC = MorphFuncReduceToStemForm( pszLanguage, pszInputTerms, pszInputFile, pszReport, lOptions );
  }

  return( usRC );
}

/*! \brief Clears the MT flag of an external memory in the EXp format
  \param pszInMemory fully qualified file name of the input memory
  \param pszOutMemory fully qualified file name of the output memory, if not specified the output memory overwrites the input memory
	\returns 0 if successful or an error code in case of failures
*/
unsigned short EqfClearMTFlag
(
  HSESSION    hSession,                // Eqf session handle
  char *pszInMemory,                   // fully qualified file name of the input memory (the file is in EXP format)
  char *pszOutMemory                   // fully qualified file name of the output memory, if not specified the input memory is changed directly
)
{
  unsigned short usRC = NO_ERROR;      // function return code
  PFCTDATA    pData = NULL;            // ptr to function data area

  // validate session handle
  usRC = FctValidateSession( hSession, &pData );

  // check parameters
  if ( usRC == NO_ERROR )
  {
    if ( (pszInMemory == NULL) || (*pszInMemory == '\0') )
    {
      usRC = ERROR_INVALID_PARAMETER;
    } /* endif */
  } /* endif */

  // call spell checking function
  if ( usRC == NO_ERROR )
  {
    usRC = MemFuncClearMTFlag( pszInMemory, pszOutMemory );
  }

  return( usRC );
}

/*! \brief Checks matches from a NOMATCH file against a memory and applies any Global Memory option fileClears the MT flag of an external memory in the EXP format
  
  \param hSession the session handle returned by the EqfStartSession call
  \param pszInNoMatchXML fully qualified file name of the input NOMATCH file in XML format
  \param pszGlobMemOptionFile fully qualified file name of the Global Memory option file
  \param pszMemory Name of the internal memory being used for the lookup. 
  \param pszOutNoMatchXML fully qualified file name of the new NOMATCH file in the XML format (can be NULL when not used)
  \param pszOutNoMatchEXP fully qualified file name of the new NOMATCH file in the EXP format (can be NULL when not used)
  \param pszWordCountReport fully qualified file name of the created memory match word count report (can be NULL when not used)
  \param lOptions options for the processing
    - COMPLETE_IN_ONE_CALL_OPT to do the processing in call (rather than doing the processing in samll units)

	\returns 0 if successful or an error code in case of failures
*/
unsigned short EqfFilterNoMatchFile
(
  HSESSION    hSession,            
  char *pszInNoMatchXML,           
  char *pszGlobMemOptionFile,      
  char *pszMemory,                 
  char *pszOutNoMatchXML,          
  char *pszOutNoMatchEXP,          
  char *pszWordCountReport,        
  long lOptions                    
)
{
  USHORT      usRC = NO_ERROR;         // function return code
  PFCTDATA    pData = NULL;            // ptr to function data area

  // validate session handle
  usRC = FctValidateSession( hSession, &pData );

  if ( pData && (pData->fComplete || (pData->sLastFunction != FCT_EQFCLEANMEMORY)) )
  {
    LOGWRITE1( "==EqfFilterNoMatchFile==\n" );
    LOGPARMSTRING( "Input Nomatch File (XML)", pszInNoMatchXML );
    LOGPARMSTRING( "GlobalMemory Option File", pszGlobMemOptionFile );
    LOGPARMSTRING( "Input Memory", pszMemory );
    LOGPARMSTRING( "In Nomatch (XML)", pszOutNoMatchXML );
    LOGPARMSTRING( "In Nomatch (EXP)", pszOutNoMatchEXP );
    LOGPARMSTRING( "WordCountReport", pszWordCountReport );
    LOGPARMOPTION( "Options", lOptions );
#ifdef SESSIONLOG
    UtlLogWriteString( "EqfFilterNoMatchFile: Starting, NoMatch = %s", pszInNoMatchXML );
#endif
  } /* endif */

  // check sequence of calls
  if ( usRC == NO_ERROR )
  {
    if ( !pData->fComplete && (pData->sLastFunction != FCT_EQFFILTERNOMATCHFILE) )
    {
      usRC = LASTTASK_INCOMPLETE_RC;
    } /* endif */
  } /* endif */

  // call clean the processing function
  if ( usRC == NO_ERROR )
  {
    pData->sLastFunction = FCT_EQFFILTERNOMATCHFILE;
    usRC = FilterNoMatchFile( pData, pszInNoMatchXML, pszGlobMemOptionFile, pszMemory, pszOutNoMatchXML, pszOutNoMatchEXP, pszWordCountReport, lOptions );

    if ( lOptions & CLEANMEM_COMPLETE_IN_ONE_CALL_OPT )
    {
      if ( (usRC == NO_ERROR) && !pData->fComplete ) usRC = CONTINUE_RC;
      while ( usRC == CONTINUE_RC )
      {
        usRC = FilterNoMatchFile( pData, pszInNoMatchXML, pszGlobMemOptionFile, pszMemory, pszOutNoMatchXML, pszOutNoMatchEXP, pszWordCountReport, lOptions );
        if ( (usRC == NO_ERROR) && !pData->fComplete ) usRC = CONTINUE_RC;
      } /*endwhile */
    } /* endif */
  } /* endif */

  if ( (usRC == NO_ERROR) && !pData->fComplete )
  {
    usRC = CONTINUE_RC;
  } /* endif */

  if ( pData && pData->fComplete )
  {
    LOGWRITE2( "  RC=%u\n", usRC );
#ifdef SESSIONLOG
    CHAR szRC[10];
    sprintf( szRC, "%u", usRC );
    UtlLogWriteString( "EqfFilterNoMatchFile: Process complete, rc=%s", szRC );
#endif
  }

  return( usRC );

} /* end of function EqfFilterNoMatchFile */

/*! \brief Deletes the given dictionary.
  \param hSession the session handle returned by the EqfStartSession call
  \param pszDict name of the dictionary being deleted
	\returns 0 if successful or an error code in case of failures
*/
USHORT EqfDeleteDict
(
  HSESSION    hSession,                // Eqf session handle
  PSZ         pszDictName              // name of dictionary
)
{
  USHORT      usRC = NO_ERROR;         // function return code
  PFCTDATA    pData = NULL;            // ptr to function data area

  // validate session handle
  usRC = FctValidateSession( hSession, &pData );

  if ( pData )
  {
    LOGWRITE1( "==EqfDeleteDict==\n" );
    LOGPARMSTRING( "Dictionary", pszDictName );
  } /* endif */

  // call clean the processing function
  if ( usRC == NO_ERROR )
  {
    usRC = DicFuncDeleteDict( pszDictName );
  } /* endif */

  if ( pData )
  {
    LOGWRITE2( "  RC=%u\n", usRC );
  }

  return( usRC );
}

/*! \brief Get OpenTM2 version information.
	\returns version information
*/
__declspec(dllexport)
USHORT EqfGetVersionEx(PSZ  pszVersion, int length)
{
    if(pszVersion == NULL || length<=0)
        return ERROR_INVALID_PARAMETER;

	strncpy(pszVersion,STR_DRIVER_LEVEL_NUMBER,length);
    //_snprintf(pszVersion,length,"%d.%d.%d.%d",EQF_DRIVER_VERSION,EQF_DRIVER_RELEASE,EQF_DRIVER_SUBRELEASE,EQF_DRIVER_BUILD);

    return 0 ;
}

/*! \brief Search segments having fuzzy memory proposals
  \param hSession the session handle returned by the EqfStartSession call
  \param pszFolderName name of the folder containing the searched documents
  \param pszDocuments list of documents being searched or NULL to search all documents of the folder
  \param pszOutputFile fully qualified of the file receiving the list of segments having fuzzy proposals
  \param iSearchMode search mode, one of the values UPTOSELECTEDCLASS_MODE, SELECTEDCLASSANDHIGHER_MODE, and ONLYSELECTEDCLASS_MODE
  \param iClass search class, a value from 0 to 6
  \param lOptions options for searching fuzzy segments
         - OVERWRITE_OPT overwrite any existing output file
         - MARKDIFFERENCES_OPT mark the differences between segment source and fuzzy proposal in teh output
  \returns 0 if successful or an error code in case of failures
*/
__declspec(dllexport)
USHORT EqfSearchFuzzySegments
(
  HSESSION    hSession,                // mand: Eqf session handle
  PSZ         pszFolderName,           // mand: name of folder
  PSZ         pszDocuments,            // opt: list of documents or NULL 
  PSZ         pszOutputFile,           // mand: fully qualified name of output file
  int         iSearchMode,             // mand: search mode
                                       // @Modes: {UPTOSELECTEDCLASS_MODE,SELECTEDCLASSANDHIGHER_MODE,ONLYSELECTEDCLASS_MODE}
  int         iClass,                  // mand: searched class
                                       // @Classes: {0,1,2,3,4,5,6}
  LONG        lOptions                 // opt: processing options
									                     // @Other: {OVERWRITE_OPT,MARKDIFFERENCES_OPT}
)
{
  USHORT      usRC = NO_ERROR;         // function return code
  PFCTDATA    pData = NULL;            // ptr to function data area

  // validate session handle
  usRC = FctValidateSession( hSession, &pData );

  if ( pData )
  {
    LOGWRITE1( "==EqfSearchFuzzySegments==\n" );
    LOGPARMSTRING( "Folder", pszFolderName );
    LOGPARMSTRING( "Documents", pszDocuments );
    LOGPARMSTRING( "OutputFile", pszOutputFile );
    LOGPARMOPTION( "SearchMode", iSearchMode );
    LOGPARMOPTION( "SearchClass", iClass );
    LOGPARMOPTION( "Options", lOptions );
  } /* endif */

  // call clean the processing function
  if ( usRC == NO_ERROR )
  {
    usRC = FolFuncFuzzySearch( pszFolderName, pszDocuments, pszOutputFile, iSearchMode, iClass, lOptions );
  } /* endif */

  if ( pData )
  {
    LOGWRITE2( "  RC=%u\n", usRC );
  }

  return( usRC );

}

/*! \brief Import a memory using the internal memory files
  \param hSession the session handle returned by the EqfStartSession call
  \param pszMemory name of the memory being imported
  \param pszMemoryPackage name of a ZIP archive containing the memory files
  \param lOptions options for searching fuzzy segments
         - OVERWRITE_OPT overwrite any existing memory with the given name
  \returns 0 if successful or an error code in case of failures
*/
__declspec(dllexport)
USHORT EqfImportMemInInternalFormat
(
  HSESSION    hSession, 
  PSZ         pszMemoryName,
  PSZ         pszMemoryPackage,
  LONG        lOptions 
)
{
  USHORT      usRC = NO_ERROR;         // function return code
  PFCTDATA    pData = NULL;            // ptr to function data area

  // validate session handle
  usRC = FctValidateSession( hSession, &pData );

  if ( pData )
  {
    LOGWRITE1( "==EqfImportMemInInternalFormat==\n" );
    LOGPARMSTRING( "Memory", pszMemoryName );
    LOGPARMSTRING( "Package", pszMemoryPackage );
    LOGPARMOPTION( "Options", lOptions );
  } /* endif */

  // call the memory factory to process the request
  if ( usRC == NO_ERROR )
  {
    MemoryFactory *pFactory = MemoryFactory::getInstance();

    usRC = pFactory->APIImportMemInInternalFormat( pszMemoryName, pszMemoryPackage, lOptions );
  } /* endif */

  if ( pData )
  {
    LOGWRITE2( "  RC=%u\n", usRC );
  }

  return( usRC );
}


/*! \brief Export a memory to a ZIP package
  \param hSession the session handle returned by the EqfStartSession call
  \param pszMemory name of the memory being exported
  \param pszMemoryPackage name of a new ZIP archive receiving the memory files
  \returns 0 if successful or an error code in case of failures
*/
__declspec(dllexport)
USHORT EqfExportMemInInternalFormat
(
  HSESSION    hSession, 
  PSZ         pszMemoryName,
  PSZ         pszMemoryPackage,
  LONG        lOptions 
)
{
  USHORT      usRC = NO_ERROR;         // function return code
  PFCTDATA    pData = NULL;            // ptr to function data area

  // validate session handle
  usRC = FctValidateSession( hSession, &pData );

  if ( pData )
  {
    LOGWRITE1( "==EqfExportMemInInternalFormat==\n" );
    LOGPARMSTRING( "Memory", pszMemoryName );
    LOGPARMSTRING( "Package", pszMemoryPackage );
    LOGPARMOPTION( "Options", lOptions );
  } /* endif */

  // call the memory factory to process the request
  if ( usRC == NO_ERROR )
  {
    MemoryFactory *pFactory = MemoryFactory::getInstance();

    usRC = pFactory->APIExportMemInInternalFormat( pszMemoryName, pszMemoryPackage, lOptions );
  } /* endif */

  if ( pData )
  {
    LOGWRITE2( "  RC=%u\n", usRC );
  }

  return( usRC );
}


/*! \brief Open the specified translation memory
  \param hSession the session handle returned by the EqfStartSession call
  \param pszMemory name of the memory being opened
  \param plHandle buffer to a long value receiving the handle of the opened memory or -1 in case of failures
  \param lOptions processing options 
  \returns 0 if successful or an error code in case of failures
*/
__declspec(dllexport)
USHORT EqfOpenMem
(
  HSESSION    hSession,
  PSZ         pszMemoryName, 
  LONG        *plHandle,
  LONG        lOptions 
)
{
  USHORT      usRC = NO_ERROR;         // function return code
  PFCTDATA    pData = NULL;            // ptr to function data area

  // validate session handle
  usRC = FctValidateSession( hSession, &pData );

  if ( pData )
  {
    LOGWRITE1( "==EqfOpenMem==\n" );
    LOGPARMSTRING( "Memory", pszMemoryName );
    LOGPARMOPTION( "Options", lOptions );
  } /* endif */

  // call the memory factory to process the request
  if ( usRC == NO_ERROR )
  {
    MemoryFactory *pFactory = MemoryFactory::getInstance();

    usRC = pFactory->APIOpenMem( pszMemoryName, plHandle, lOptions );
  } /* endif */

  if ( pData )
  {
    LOGWRITE2( "  RC=%u\n", usRC );
  }

  return( usRC );
}


/*! \brief Close the translation memory referred by the handle
  \param hSession the session handle returned by the EqfStartSession call
  \param lHandle handle of a previously opened memory
  \param lOptions processing options 
  \returns 0 if successful or an error code in case of failures
*/
__declspec(dllexport)
USHORT EqfCloseMem
(
  HSESSION    hSession, 
  LONG        lHandle,
  LONG        lOptions 
)
{
  USHORT      usRC = NO_ERROR;         // function return code
  PFCTDATA    pData = NULL;            // ptr to function data area

  // validate session handle
  usRC = FctValidateSession( hSession, &pData );

  if ( pData )
  {
    LOGWRITE1( "==EqfCloseMem==\n" );
    LOGPARMLONG( "Handle", lHandle );
    LOGPARMOPTION( "Options", lOptions );
  } /* endif */

  // call the memory factory to process the request
  if ( usRC == NO_ERROR )
  {
    MemoryFactory *pFactory = MemoryFactory::getInstance();

    usRC = pFactory->APICloseMem( lHandle, lOptions );
  } /* endif */

  if ( pData )
  {
    LOGWRITE2( "  RC=%u\n", usRC );
  }

  return( usRC );
}

/*! \brief Lookup a segment in the memory
  \param hSession the session handle returned by the EqfStartSession call
  \param lHandle handle of a previously opened memory
  \param pSearchKey pointer to a MemProposal structure containing the searched criteria
  \param *piNumOfProposals pointer to the number of requested memory proposals, will be changed on return to the number of proposals found
  \param pProposals pointer to a array of MemProposal structures receiving the search results
  \param lOptions processing options 
  \returns 0 if successful or an error code in case of failures
*/
__declspec(dllexport)
USHORT EqfQueryMem
(
  HSESSION    hSession,    
  LONG        lHandle,          
  PMEMPROPOSAL pSearchKey, 
  int         *piNumOfProposals,
  PMEMPROPOSAL pProposals, 
  LONG        lOptions     
)
{
  USHORT      usRC = NO_ERROR;         // function return code
  PFCTDATA    pData = NULL;            // ptr to function data area

  // validate session handle
  usRC = FctValidateSession( hSession, &pData );

  if ( pData )
  {
    LOGWRITE1( "==EqfQueryMem==\n" );
    LOGPARMLONG( "Handle", lHandle );
    LOGPARMOPTION( "Options", lOptions );
  } /* endif */

  // call the memory factory to process the request
  if ( usRC == NO_ERROR )
  {
    MemoryFactory *pFactory = MemoryFactory::getInstance();

    usRC = pFactory->APIQueryMem( lHandle, pSearchKey, piNumOfProposals, pProposals, lOptions );
  } /* endif */

  if ( pData )
  {
    LOGWRITE2( "  RC=%u\n", usRC );
  }

  return( usRC );
}



/*! \brief Search the given text string in the memory
\param hSession the session handle returned by the EqfStartSession call
\param lHandle handle of a previously opened memory
\param pszSearchString pointer to the search string (in UTF-16 encoding)
\param pszStartPosition pointer to a buffer (min size = 20 charachters) containing the start position, on completion this buffer is filled with the next search position
\param pProposal pointer to an MemProposal structure receiving the next matching segment
\param lSearchTime max time in milliseconds to search for a matching proposal, 0 for no search time restriction
\param lOptions processing options
SEARCH_IN_SOURCE_OPT  search in source
SEARCH_IN_TARGET_OPT  search in target
\returns 0 if successful or an error code in case of failures
*/
__declspec( dllexport )
USHORT EqfSearchMem
(
  HSESSION    hSession,
  LONG        lHandle,
  CHAR_W      *pszSearchString,
  PSZ         pszStartPosition,
  PMEMPROPOSAL pProposal,
  LONG        lSearchTime,
  LONG        lOptions
)
{
  USHORT      usRC = NO_ERROR;         // function return code
  PFCTDATA    pData = NULL;            // ptr to function data area

  // validate session handle
  usRC = FctValidateSession( hSession, &pData );

  if ( pData )
  {
    LOGWRITE1( "==EqfSearchMem==\n" );
    LOGPARMLONG( "Handle", lHandle );
    LOGPARMSTRINGW( "SearchString", pszSearchString );
    LOGPARMSTRING( "StartPosition", pszStartPosition );
    LOGPARMOPTION( "Options", lOptions );
  } /* endif */

  // call the memory factory to process the request
  if ( usRC == NO_ERROR )
  {
    MemoryFactory *pFactory = MemoryFactory::getInstance();

    usRC = pFactory->APISearchMem( lHandle, pszSearchString, pszStartPosition, pProposal, lSearchTime, lOptions );
  } /* endif */

  if ( pData )
  {
    LOGWRITE2( "  RC=%u\n", usRC );
  }

  return( usRC );
}


/*! \brief Update a segment in the memory
  \param hSession the session handle returned by the EqfStartSession call
  \param lHandle handle of a previously opened memory
  \param pNewProposal pointer to an MemProposal structure containing the segment data
  \param lOptions processing options 
  \returns 0 if successful or an error code in case of failures
*/
__declspec(dllexport)
USHORT EqfUpdateMem
(
  HSESSION    hSession,
  LONG        lHandle, 
  PMEMPROPOSAL pNewProposal,
  LONG        lOptions
)
{
  USHORT      usRC = NO_ERROR;         // function return code
  PFCTDATA    pData = NULL;            // ptr to function data area

  // validate session handle
  usRC = FctValidateSession( hSession, &pData );

  if ( pData )
  {
    LOGWRITE1( "==EqfUpdateMem==\n" );
    LOGPARMLONG( "Handle", lHandle );
    LOGPARMOPTION( "Options", lOptions );
  } /* endif */

  // call the memory factory to process the request
  if ( usRC == NO_ERROR )
  {
    MemoryFactory *pFactory = MemoryFactory::getInstance();

    usRC = pFactory->APIUpdateMem( lHandle, pNewProposal, lOptions );
  } /* endif */

  if ( pData )
  {
    LOGWRITE2( "  RC=%u\n", usRC );
  }

  return( usRC );
}

/*! \brief List the name of all memories
\param hSession the session handle returned by the EqfStartSession call
\param pszBuffer pointer to a buffer reiceiving the comma separated list of memory names or NULL to get the required length for the list
\param plLength pointer to a variable containing the size of the buffer area, on return the length of name list is stored in this variable
\returns 0 if successful or an error code in case of failures
*/
__declspec( dllexport )
USHORT EqfListMem
(
  HSESSION    hSession,
  PSZ         pszBuffer,
  PLONG       plLength
)
{
  USHORT      usRC = NO_ERROR;         // function return code
  PFCTDATA    pData = NULL;            // ptr to function data area

                                       // validate session handle
  usRC = FctValidateSession( hSession, &pData );

  if ( pData )
  {
    LOGWRITE1( "==EqfUpdateMem==\n" );
    LOGPARMLONG( "Handle", lHandle );
    LOGPARMOPTION( "Options", lOptions );
  } /* endif */

    // call the memory factory to process the request
  if ( usRC == NO_ERROR )
  {
    MemoryFactory *pFactory = MemoryFactory::getInstance();

    usRC = pFactory->APIListMem( pszBuffer, plLength );
  } /* endif */

  if ( pData )
  {
    LOGWRITE2( "  RC=%u\n", usRC );
  }

  return( usRC );
}

/*! \brief Get the OpenTM2 language name for a ISO language identifier
\param hSession the session handle returned by the EqfStartSession call
\param pszISOLang pointer to ISO language name (e.g. en-US )
\param pszOpenTM2Lang buffer for the OpenTM2 language name
\returns 0 if successful or an error code in case of failures
*/
__declspec( dllexport )
USHORT EqfGetOpenTM2Lang
(
  HSESSION    hSession,
  PSZ         pszISOLang,
  PSZ         pszOpenTM2Lang
)
{
  USHORT      usRC = NO_ERROR;         // function return code
  PFCTDATA    pData = NULL;            // ptr to function data area

                                       // validate session handle
  usRC = FctValidateSession( hSession, &pData );

  if ( pData )
  {
    LOGWRITE1( "==EqfGetOpenTM2Lang==\n" );
    LOGPARMSTRING( "ISOLang", pszIsoLanguage );
  } /* endif */

  if ( (usRC == NO_ERROR ) && (pszISOLang == NULL) ) 
  {
    PSZ pszParm = "pointer to ISO language id";
    UtlErrorHwnd( DDE_MANDPARAMISSING, MB_CANCEL, 1, &pszParm, EQF_ERROR, HWND_FUNCIF );
    usRC = DDE_MANDPARAMISSING;
  } /* endif */

  if ( (usRC == NO_ERROR ) && (pszOpenTM2Lang == NULL) ) 
  {
    PSZ pszParm = "buffer for OpenTM2 language name";
    UtlErrorHwnd( DDE_MANDPARAMISSING, MB_CANCEL, 1, &pszParm, EQF_ERROR, HWND_FUNCIF );
    usRC = DDE_MANDPARAMISSING;
  } /* endif */


  // use the language factory to process the request
  if ( usRC == NO_ERROR )
  {
    LanguageFactory *pLangFactory = LanguageFactory::getInstance();
    pLangFactory->getOpenTM2NameFromISO( pszISOLang, pszOpenTM2Lang );
  } /* endif */

  if ( pData )
  {
    LOGWRITE2( "  RC=%u\n", usRC );
  }

  return( usRC );
}

/*! \brief Get the ISO language identifier for a OpenTM2 language name
\param hSession the session handle returned by the EqfStartSession call
\param pszOpenTM2Lang pointer to the OpenTM2 language name
\param pszISOLang pointer to a buffer for the ISO language identifier
\returns 0 if successful or an error code in case of failures
*/
__declspec( dllexport )
USHORT EqfGetIsoLang
(
  HSESSION    hSession,
  PSZ         pszOpenTM2Lang,
  PSZ         pszISOLang
)
{
  USHORT      usRC = NO_ERROR;         // function return code
  PFCTDATA    pData = NULL;            // ptr to function data area

                                       // validate session handle
  usRC = FctValidateSession( hSession, &pData );

  if ( pData )
  {
    LOGWRITE1( "==EqfGetIsoTM2Lang==\n" );
    LOGPARMSTRING( "OpenTM2Lang", pszOpenTM2Language );
  } /* endif */

  if ( (usRC == NO_ERROR ) && (pszOpenTM2Lang == NULL) ) 
  {
    PSZ pszParm = "pointer to OpenTM2 language name";
    UtlErrorHwnd( DDE_MANDPARAMISSING, MB_CANCEL, 1, &pszParm, EQF_ERROR, HWND_FUNCIF );
    usRC = DDE_MANDPARAMISSING;
  } /* endif */

  if ( (usRC == NO_ERROR ) && (pszOpenTM2Lang == NULL) ) 
  {
    PSZ pszParm = "buffer for ISO language id";
    UtlErrorHwnd( DDE_MANDPARAMISSING, MB_CANCEL, 1, &pszParm, EQF_ERROR, HWND_FUNCIF );
    usRC = DDE_MANDPARAMISSING;
  } /* endif */


  // use the language factory to process the request
  if ( usRC == NO_ERROR )
  {
    LanguageFactory *pLangFactory = LanguageFactory::getInstance();
    pLangFactory->getISOName( pszOpenTM2Lang, pszISOLang );
  } /* endif */

  if ( pData )
  {
    LOGWRITE2( "  RC=%u\n", usRC );
  }

  return( usRC );
}

