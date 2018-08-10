/*! \file
	Description: Document related functions and dialogs
	
	Copyright Notice:

	Copyright (C) 1990-2017, International Business Machines
	Corporation and others. All rights reserved
*/

  #define TADUMMYTAG_INIT
  #define INCL_WINSTDFILE
  #define INCL_EQF_ANALYSIS         // analysis functions
  #define INCL_EQF_TAGTABLE         // tagtable defines and functions
  #define INCL_EQF_FOLDER           // folder list and document list functions
  #define INCL_EQF_DLGUTILS         // dialog utilities
  #define INCL_EQF_TP               // public translation processor functions
  #include <eqf.h>                  // General Translation Manager include file

  #include <process.h>              /* _beginthread, _endthread */
  #include <direct.h>

  #include "eqfutpck.h"             // packaging utilities

  #include "eqfdoc01.h"             // internal header file
  #include "eqfdoc01.id"            // internal ID file
  #include <time.h>
  #include "EQFHLOG.H"            // defines for history log processing
  #include "eqfdde.h"             // ... required for EQFFOL00.H
  #include "EQFFOL00.H"           // defines for folder history log functions
#define DEF_EVENT_ENUMS
  #include "EQFEVENT.H"           // defines for event logging
  #include "SHLOBJ.H"           // folder browse function
  #include "eqfstart.h"
  #ifdef FUNCCALLIF
    #include "OTMFUNC.H"            // public defines for function call interface
    #include "EQFFUNCI.H"           // private defines for function call interface
    #include "EQFDDE.H"           // defines for event logging
  #endif

  #include "ValDocExp.h"            // validation format document export defines


// prefix for folder name in folder name dummy file
#define FOLNAMEPREFIX "FolName="

// names for last used value files
#define DOCEXPVALLASTUSED      "DOCEXPVALFORMAT"
#define DOCEXPXMLLASTUSED      "DOCEXPPLAINXML"
#define DOCEXPINTLASTUSED      "DOCEXPINTFORMAT"
#define DOCEXPEXTLASTUSED      "DOCEXPEXTFORMAT"
#define DOCEXPSOURCELASTUSED   "DOCEXPSOURCE"
#define DOCEXPSNOMATCHLASTUSED "DOCEXPSNOMATCH"

// list of formats for validation format export 
// Note: this list must be must be in same order as the VALFORMATID enumeration in EQFTAML.H!
//       the end of the list is indicated by an empty string
static char VALEXPFORMATS[][20] = { "HTML", "XML", "MS Word (.DOC)", "MS Word (.DOCX)", "Symphony (.ODT)","" };


USHORT ExportSNOMATCHDocument( PDOCEXPIDA pIda );
static VOID EnableExportControls (HWND hwnd, PDOCEXPIDA pIda, BOOL fEnable);
static MRESULT DocExpCommand( HWND, SHORT, SHORT );
static MRESULT DocExpControl( HWND, SHORT, SHORT );
static MRESULT DocExpInit( HWND, WPARAM, LPARAM );
BOOL DocExpBrowse( HWND hwnd, PSZ pszBuffer, PSZ pszTitle );
BOOL    CheckPath( PSZ, PSZ );
static MRESULT RevMarkControl( HWND, SHORT, SHORT );
static MRESULT ExpPropSheetNotification ( HWND, WPARAM, LPARAM );

// functions for DocFuncExportVal usage begin
static USHORT DocFuncExportValTerminate(PFCTDATA);
static USHORT DocFuncExportValOp(PFCTDATA,PSZ,BOOL );
static USHORT DocFuncExpMakeSegTargetPath(PFCTDATA,PSZ);
static USHORT DocFuncExportValPre(PFCTDATA,PSZ,PSZ,PSZ,LONG,LONG,LONG,LONG,PSZ_W,PSZ_W);
// functions for DocFuncExportVal usage end


extern HELPSUBTABLE hlpsubtblPropSettingsDlg[];
 
USHORT DocFuncPrepImport
(
  PFCTDATA    pData,                   // function I/F session data
  PSZ         pszFolderName,           // name of folder receiving the documents
  PSZ         pszFiles,                // list of input files (documents)
  PSZ         pszMemname,              // document Translation Memory or NULL
  PSZ         pszMarkup,               // document markup or NULL
  PSZ         pszEditor,               // document editor or NULL
  PSZ         pszSourceLanguage,       // document source language or NULL
  PSZ         pszTargetLanguage,       // document target language or NULL
  PSZ         pszAlias,                // alias for document name or NULL
  PSZ         pszStartPath,            // optional start path
  PSZ         pszConversion,           // optional document export conversion
  LONG        lOptions                 // document import options or 0L
);
USHORT DocFuncImportProcess
(
  PFCTDATA    pData                    // function I/F session data
);
USHORT DocFuncPrepExport
(
  PFCTDATA    pData,                   // function I/F session data
  PSZ         pszFolderName,           // name of folder
  PSZ         pszFiles,                // list of documents with path information
  PSZ         pszStartPath,            // optional start path
  LONG        lOptions                 // options for document export
);
USHORT DocFuncExportProcess
(
PFCTDATA    pData                    // function I/F session data
);
BOOL DocumentLoad_Import
(
  PDOCIMPIDA pIda
);
static int DocValFormatGetMatchFlags
( 
  HWND       hwnd,
  PDOCEXPIDA pIda
);
static int DocValFormatSetMatchFlags
( 
  HWND       hwnd,
  PDOCEXPIDA pIda
);
static int DocValFormatEnableMatchFlags
( 
  HWND       hwnd,
  BOOL       fEnable
);
static int DocValFormatSetMatchStates
( 
  HWND       hwnd,
  PDOCEXPIDA pIda, 
  BOOL       fValidationFormat, 
  BOOL       fGetMatchStates
);




USHORT DocumentLoad( PDOCIMPIDA pIda, BOOL fWaitCursor )
{
  //---------------------------------------------------------------------------
  // load a specified file into the source directory of a specified folder   --
  //---------------------------------------------------------------------------
  //declare variables

  HPROP        hpropRc;               // return form create properties
  PPROPDOCUMENT pDocProp;             // pointer to document properties
  USHORT       usRc;                  // USHORT return code
  USHORT       usResult = LOAD_NOTOK; // init return value
  ULONG        ulErrorInfo;           // error indicator from PRHA
  PSZ          pszReplace;            // pointer to replace string UtlError
  BOOL         fOK = TRUE;            // internal OK flag
  USHORT       usResponse;            // return from UtlError
  USHORT       usMBCode;              // message box return code
  CHAR         szDrive[MAX_DRIVE];    // buffer for drive letter string
  BOOL         fDocDeletePending = FALSE; // document delete operationis pending

  // initialize fDocExists flag
  pIda->fDocExists = FALSE;

  if ( fOK )
  {
    //build document name
    UtlMakeEQFPath( pIda->szDummy, pIda->szTargetFolder[0], DIRSOURCEDOC_PATH, pIda->szToFolder );
    UtlMakeFullPath (pIda->szTarget, (PSZ)NULP, pIda->szDummy, pIda->stFs.szName, pIda->stFs.szExt);

    //initialize usResponse
    usResponse = MBID_YES;

    //clear buffer for old document properties
    memset( &pIda->stDocProp, 0, sizeof(pIda->stDocProp) );

    //file exists in source dir of target folder
    if ( CheckExistence( pIda->szTarget, FALSE, FILE_NORMAL, &usRc) )
    {
      pIda->fDocExists = TRUE;

      //call utility to display error message
      pszReplace = pIda->stFs.szLongName;
      if ( fWaitCursor )
      {
        SETCURSOR( SPTR_ARROW );
      } /* endif */

      if ( pIda->fYesToAll )
      {
        usResponse = MBID_YES;
      }
      else if ( pIda->usSelDocs > 1 )
      {
        usResponse = UtlError( ERROR_FILE_EXISTS_ALREADY, MB_EQF_YESTOALL | MB_DEFBUTTON3, 1,
                               &pszReplace, EQF_QUERY );
        if ( usResponse == MBID_EQF_YESTOALL )
        {
          pIda->fYesToAll = TRUE;
          usResponse = MBID_YES;
        } /* endif */
      }
      else
      {
        usResponse = UtlError( ERROR_FILE_EXISTS_ALREADY, MB_YESNOCANCEL | MB_DEFBUTTON2, 1, &pszReplace, EQF_QUERY );
      } /* endif */

      if ( fWaitCursor )
      {
        SETCURSOR( SPTR_WAIT );
      } /* endif */

      switch ( usResponse )
      {
        case MBID_YES:
          //build object name of document
          UtlMakeFullPath (pIda->szObjName, (PSZ)NULP, pIda->szTargetFolder, pIda->stFs.szName, pIda->stFs.szExt);

          {
            // get copy of document properties for later usage by
            // document property dialog
            {
              PVOID  pvDocProp = &pIda->stDocProp;
              ULONG ulSize;

              UtlMakeEQFPath( pIda->szProperties, pIda->szTargetFolder[0], PROPERTY_PATH, UtlGetFnameFromPath( pIda->szTargetFolder ) );
              strcat( pIda->szProperties, BACKSLASH_STR );
              strcat( pIda->szProperties, pIda->stFs.szName );
              if ( pIda->stFs.szExt[0] != EOS )
              {
                strcat( pIda->szProperties, pIda->stFs.szExt );
              } /* endif */
              UtlLoadFileL( pIda->szProperties, &pvDocProp, &ulSize, FALSE, FALSE );
            } /* endif */

            // postpone delete, will be done after docprop dialog
            fDocDeletePending = TRUE;
          }
          break;
        case MBID_NO:
          fOK = FALSE;
          usResult = LOAD_NOTOK;
          break;
        default:                              // MBID_CANCEL , etc
          usResult = LOAD_CANCEL;
          fOK = FALSE;
          break;
      } /* endswitch */
    }/*end if*/
  }/*end if fOK*/

  // prepare document properties for new document
  if ( fOK )
  {
    fOK = UtlAlloc( (PVOID *)&(pIda->DocInfo.pDocProps), 0L, sizeof(PROPDOCUMENT), TRUE );

    if ( fOK )
    {
      pDocProp = pIda->DocInfo.pDocProps;

      pDocProp->ulParentFolder = pIda->ulParentID;

      if ( pIda->fDocExists )
      {
        strcpy( pDocProp->szAlias,       pIda->stDocProp.szAlias );
        strcpy( pDocProp->szFormat,      pIda->stDocProp.szFormat );
        strcpy( pDocProp->szMemory,      pIda->stDocProp.szMemory );
        strcpy( pDocProp->szLongMemory,  pIda->stDocProp.szLongMemory );
        strcpy( pDocProp->szSourceLang,  pIda->stDocProp.szSourceLang );
        strcpy( pDocProp->szTargetLang,  pIda->stDocProp.szTargetLang );
        strcpy( pDocProp->szEditor,      pIda->stDocProp.szEditor );
        strcpy( pDocProp->szVendor,      pIda->stDocProp.szVendor );
        strcpy( pDocProp->szVendorEMail, pIda->stDocProp.szVendorEMail );
        strcpy( pDocProp->szConversion,  pIda->stDocProp.szConversion );

        // always use long name of imported document (may differ in upper/lowercase characters)
        if ( pIda->stFs.fIsLongName )
        {
          strcpy( pDocProp->szLongName, pIda->stFs.szLongName );
          ANSITOOEM( pDocProp->szLongName );
          strcpy( pIda->stDocProp.szLongName, pDocProp->szLongName );
        } /* endif */

        // for existing documents:
        // copy values from old properties to document info structure if not "use for all" is active
        pIda->DocInfo.szAllias[0] = EOS;
        if ( (pIda->stDocProp.PropHead.szPath[0] != EOS ) && !pIda->DocInfo.fUseForAll )
        {
          strcpy( pIda->DocInfo.szFormat, pIda->stDocProp.szFormat );
          strcpy( pIda->DocInfo.szMemory, pIda->stDocProp.szMemory );
          strcpy( pIda->DocInfo.szLongMemory, pIda->stDocProp.szLongMemory );
          strcpy( pIda->DocInfo.szSourceLang, pIda->stDocProp.szSourceLang );
          strcpy( pIda->DocInfo.szTargetLang, pIda->stDocProp.szTargetLang );
          strcpy( pIda->DocInfo.szAllias, pIda->stDocProp.szAlias );
          strcpy( pIda->DocInfo.szEditor, pIda->stDocProp.szEditor );
          strcpy( pIda->DocInfo.szTranslator, pIda->stDocProp.szVendor);
          strcpy( pIda->DocInfo.szTranslatorEmail, pIda->stDocProp.szVendorEMail);
        } /* endif */
        memcpy( &(pDocProp->PropHead), &(pIda->stDocProp.PropHead), sizeof(pDocProp->PropHead) );
      }
      else
      {
        // preset property header
        strcpy( pDocProp->PropHead.szName, pIda->stFs.szName );
        if ( pIda->stFs.szExt[0] == EOS )
        {
          strcat( pDocProp->PropHead.szName, pIda->stFs.szExt );
        }/*endif*/

        UtlMakeEQFPath( pDocProp->PropHead.szPath, pIda->szTargetFolder[0], SYSTEM_PATH, UtlGetFnameFromPath(pIda->szTargetFolder) );

        // add doc long name
        if ( pIda->stFs.fIsLongName )
        {
          strcpy( pDocProp->szLongName, pIda->stFs.szLongName );
          ANSITOOEM( pDocProp->szLongName );
        } /* endif */

      } /* endif */
    } /* endif */
  } /* endif */

  // show property dialog
  if ( fOK && !pIda->DocInfo.fUseForAll )
  {
    if ( fWaitCursor )
    {
      SETCURSOR( SPTR_ARROW );
    } /* endif */

    fOK = DocPropertyDlg( pIda->hwndDlg, pIda->szProperties, FALSE, &(pIda->DocInfo), NULL );

    if ( fWaitCursor )
    {
      SETCURSOR( SPTR_WAIT );
    } /* endif */
  } /* endif */

  // do any document delete
  if ( fOK && fDocDeletePending )
  {
    USHORT usDummy = 0;
    fOK = DocumentDeleteEx( pIda->szObjName, FALSE, &usDummy, DOCDELETE_NOMTLOGDELETE );

    // write a shipment record to the MTLOG (if there is any)
    if ( fOK && (UtlQueryUShort( QS_MTLOGGING ) != 0) )
    {
      // setup MTLOG file path
      UtlMakeEQFPath( pIda->szBuffer, pIda->szTargetFolder[0], MTLOG_PATH, UtlGetFnameFromPath( pIda->szTargetFolder ) );
      strcat( pIda->szBuffer, BACKSLASH_STR );
      strcat( pIda->szBuffer, pIda->stFs.szName );
      if ( pIda->stFs.szExt[0] != EOS ) strcat( pIda->szBuffer, pIda->stFs.szExt );

      if ( UtlFileExist( pIda->szBuffer ) )
      {
        FILE *hMTLog = fopen( pIda->szBuffer, "ab" );
        if ( hMTLog != NULL  )
        {
          ACTSEGLOGENH2 SegLog;
          memset( &SegLog, 0, sizeof(SegLog) );
          SegLog.ulEyeCatcher = ACTSEGLOG_EYECATCHER;
          SegLog.AddFlags.ShipmentRec = TRUE;
          WriteToMTLog( hMTLog, &SegLog, NULL, NULL, NULL, NULL );
          fclose( hMTLog );          
        } /* endif */             
      
      } /* endif */         
    } /* endif */       


    if ( !fOK )
    {
      if ( fWaitCursor )
      {
        SETCURSOR( SPTR_ARROW );
      } /* endif */
      pszReplace = pIda->szObjName,
      UtlError( NOT_DELETED, MB_CANCEL, 1,
                &pszReplace, EQF_WARNING );
      if ( fWaitCursor )
      {
        SETCURSOR( SPTR_WAIT );
      } /* endif */
    }
    else                                                 /*KIT0925A*/
    {
      /*KIT0925A*/
      // decrease number of docs in folder (doc deleted) /*KIT0925A*/
      pIda->usDocsInFolder--;                            /*KIT0925A*/
    } /* endif */

  } /* endif */

  // create property file for imported document
  if ( fOK )
  {
    // Delete any existing property file of document                
    UtlMakeEQFPath( pIda->szProperties, pIda->szTargetFolder[0], PROPERTY_PATH, UtlGetFnameFromPath( pIda->szTargetFolder ) );
    strcat( pIda->szProperties, BACKSLASH_STR );
    strcat( pIda->szProperties, pIda->stFs.szName );
    if ( pIda->stFs.szExt[0] != EOS )
    {
      strcat( pIda->szProperties, pIda->stFs.szExt );
    } /* endif */

    UtlDelete( pIda->szProperties, 0L, FALSE );

    //call property handler to create document properties
    if ( pIda->stFs.szExt[0] == EOS )
    {
      UtlMakeFullPath (pIda->szProperties, (PSZ)NULP, pIda->szTargetFolder, pIda->stFs.szName, (PSZ)NULP);
    }
    else
    {
      UtlMakeFullPath (pIda->szProperties, (PSZ)NULP, pIda->szTargetFolder, pIda->stFs.szName, pIda->stFs.szExt);
    }/*endif*/

    do
    {
      usMBCode = MBID_OK;

      hpropRc = CreateProperties( pIda->szProperties, NULL, PROP_CLASS_DOCUMENT, &ulErrorInfo );

      if ( !hpropRc )               //error from property handler
      {
        //set fOK to FALSE
        fOK = FALSE;
        usResult = LOAD_CANCEL;
        //display message create property fails
        if ( fWaitCursor )
        {
          SETCURSOR( SPTR_ARROW );
        } /* endif */
        switch ( (USHORT)ulErrorInfo )
        {
          case Err_NoDiskSpace :
            pszReplace = szDrive;
            sprintf( szDrive, "%c:", pIda->szProperties[0] );
            usMBCode = UtlError( ERROR_DISK_IS_FULL, MB_RETRYCANCEL, 1, &pszReplace, EQF_ERROR );
            break;
          case Err_NoStorage :
            usMBCode = UtlError( ERROR_STORAGE, MB_CANCEL, 0, NULL, EQF_ERROR );
            break;
          default :
            pszReplace = pIda->stFs.szLongName;
            usMBCode = UtlError( ERROR_CREATE_PROP, MB_CANCEL, 1, &pszReplace, EQF_ERROR );
            break;
        } /* endswitch */
        if ( fWaitCursor )
        {
          SETCURSOR( SPTR_WAIT );
        } /* endif */
      }
      else
      {
        // store long name in properties (use name from old properties
        // for existing documents)
        pDocProp = (PPROPDOCUMENT)MakePropPtrFromHnd( hpropRc );

        strcpy( pDocProp->szAlias,       pIda->stDocProp.szAlias );
        strcpy( pDocProp->szFormat,      pIda->stDocProp.szFormat );
        strcpy( pDocProp->szMemory,      pIda->stDocProp.szMemory );
        strcpy( pDocProp->szSourceLang,  pIda->stDocProp.szSourceLang );
        strcpy( pDocProp->szTargetLang,  pIda->stDocProp.szTargetLang );
        strcpy( pDocProp->szEditor,      pIda->stDocProp.szEditor );
        strcpy( pDocProp->szVendor,      pIda->stDocProp.szVendor );
        strcpy( pDocProp->szVendorEMail, pIda->stDocProp.szVendorEMail );

        pDocProp->ulParentFolder = pIda->ulParentID;

        if ( pIda->stDocProp.szLongName[0] != EOS )
        {
          strcpy( pDocProp->szLongName, pIda->stDocProp.szLongName );
        }
        else if ( pIda->stFs.fIsLongName )
        {
          strcpy( pDocProp->szLongName, pIda->stFs.szLongName );
          ANSITOOEM( pDocProp->szLongName );
        } /* endif */

        /***********************************************************/
        /* Check if document property dialog is required           */
        /***********************************************************/
        if ( pIda->DocInfo.fUseForAll )
        {
          /*********************************************************/
          /* Use settings from document info structure             */
          /*********************************************************/
          pDocProp = (PPROPDOCUMENT)MakePropPtrFromHnd( hpropRc );
          strcpy( pDocProp->szFormat, pIda->DocInfo.szFormat );
          strcpy( pDocProp->szMemory, pIda->DocInfo.szMemory );
          strcpy( pDocProp->szLongMemory, pIda->DocInfo.szLongMemory );
          strcpy( pDocProp->szSourceLang, pIda->DocInfo.szSourceLang );
          strcpy( pDocProp->szTargetLang, pIda->DocInfo.szTargetLang );
          strcpy( pDocProp->szEditor, pIda->DocInfo.szEditor );
          strcpy( pDocProp->szVendor, pIda->DocInfo.szTranslator );
          strcpy( pDocProp->szVendorEMail, pIda->DocInfo.szTranslatorEmail );
        }
        else
        {
          // use values from property area
          int iPropHeadLen = sizeof(pDocProp->PropHead);
          memcpy( (PBYTE)pDocProp + iPropHeadLen, (PBYTE)pIda->DocInfo.pDocProps + iPropHeadLen, 
                  sizeof(PROPDOCUMENT) - iPropHeadLen );
        } /* endif */

        // remember external document file name (used by re-import document function)
        if ( pIda->stFs.fIsLongName )
        {
          UtlMakeFullPath( pDocProp->szExternalDocPathName, pIda->stFs.szDrive, pIda->stFs.szPath, pIda->stFs.szLongName, NULL );
        }
        else
        {
          UtlMakeFullPath( pDocProp->szExternalDocPathName, pIda->stFs.szDrive, pIda->stFs.szPath, pIda->stFs.szName, pIda->stFs.szExt   );
        } /* endif */

        CloseProperties( hpropRc, PROP_FILE, &ulErrorInfo );
      } /* endif */
    } while ( usMBCode == MBID_RETRY ); /* enddo */
  }/*end if fOK*/

  // do the actual import of the document
  if ( fOK )
  {
    fOK = DocumentLoad_Import( pIda );
  } /* endif */

  // TVT tracking.  Set document number within this folder.
  if ( fOK )
  {
    EQFINFO     ErrorInfo;           // error code of property handler calls
    PPROPFOLDER pFolProp;            // ptr to folder properties
    HPROP       hpropDocument;       // return from create properties
    PVOID       hFolProp;            // handle of folder properties
    OBJNAME     szFolObjName;        // buffer for folder object name
    CHAR        szTmgrDrive[3];      // buffer for Tmgr primary drive
    USHORT      usDocNum ;

    // setup folder object name using target folder name and primary Tmgr drive
    UtlQueryString( QST_PRIMARYDRIVE, szTmgrDrive, sizeof(szTmgrDrive) );
    strcpy( szFolObjName, pIda->szTargetFolder );
    szFolObjName[0] = szTmgrDrive[0];

    hFolProp = OpenProperties( szFolObjName, NULL, PROP_ACCESS_READ, &ErrorInfo);
    if ( hFolProp )
    {
      SetPropAccess(hFolProp , PROP_ACCESS_WRITE);
      pFolProp = (PPROPFOLDER)MakePropPtrFromHnd( hFolProp );
      pFolProp->usLastAssignedDocNum++;
      usDocNum = pFolProp->usLastAssignedDocNum ;
      SaveProperties(hFolProp , &ErrorInfo );
      CloseProperties( hFolProp, PROP_QUIT, &ErrorInfo );
      if ( (hpropDocument = OpenProperties (pIda->szProperties, NULL, PROP_ACCESS_READ, &ulErrorInfo))!= NULL)
      {
        PPROPDOCUMENT ppropDocument;        // pointer to document properties

        SetPropAccess(hpropDocument , PROP_ACCESS_WRITE);
        ppropDocument = (PPROPDOCUMENT)MakePropPtrFromHnd( hpropDocument );
        ppropDocument->usTrackDocNum = usDocNum ;
        SaveProperties(hpropDocument , &ErrorInfo );
        CloseProperties(hpropDocument , PROP_QUIT, &ErrorInfo );
      }//end if
    }
  } /* endif */

 // cleanup
 if ( pIda->DocInfo.pDocProps ) UtlAlloc( (PVOID *)&(pIda->DocInfo.pDocProps), 0L, 0L, NOMSG );

  usResult = ( fOK ) ? LOAD_OK : LOAD_CANCEL;

  return usResult;
}/*end DocumentLoad*/

//
// DocumentLoad_Import
//
// Import part of DocumentLoad
//
BOOL DocumentLoad_Import
(
  PDOCIMPIDA pIda
)
{
  USHORT       usDosRc;               // Return code from Dos(Utl) operations
  ULONG        ulErrorInfo;           // error indicator from PRHA
  ULONG        ErrorInfo;
  BOOL         fOK = TRUE;            // internal OK flag
  CHAR         szShipment[MAX_DESCRIPTION];  // shipment string
  HPROP        hpropDocument;         // return form create properties
  
  if ( fOK )
  {
    //call UtlCopy to copy source file to target folder
    if ( pIda->stFs.fIsLongName )
    {
      UtlMakeFullPath( pIda->szSource, pIda->stFs.szDrive, pIda->stFs.szPath, pIda->stFs.szLongName, NULL );
    }
    else
    {
      UtlMakeFullPath( pIda->szSource, pIda->stFs.szDrive, pIda->stFs.szPath, pIda->stFs.szName, pIda->stFs.szExt   );
    } /* endif */
    usDosRc = UtlCopy( pIda->szSource, pIda->szTarget, 1, 0L, TRUE );

    switch ( usDosRc )
    {
      case NO_ERROR :
        /**********************************************************/
        /* reset any read-only flags of loaded document           */
        /**********************************************************/
        UtlSetFileMode( pIda->szTarget, FILE_NORMAL, 0L, TRUE );
        break;
      default  :
        fOK = FALSE;
        DeleteProperties( pIda->szProperties, NULL, &ulErrorInfo );
        break;
    } /* endswitch */
  }/*end if fOK*/

  // Add long name record to history log for documents with long names
  if ( fOK && (pIda->DocInfo.szLongName[0] != EOS) )
  {
    strcpy( pIda->szDummy, pIda->stFs.szName );
    strcat( pIda->szDummy, pIda->stFs.szExt );

    EQFBWriteHistLog2( pIda->szTargetFolder,
                       pIda->szDummy, LONGNAME_LOGTASK,
                       sizeof(LONGNAMEHIST), (PVOID)pIda->DocInfo.szLongName,
                       TRUE, NULLHANDLE, pIda->DocInfo.szLongName );

  } /* endif */

  /************************************************************/
  /* Add import record to history log                         */
  /************************************************************/
  if ( fOK )
  {
    PDOCIMPORTHIST2 pDocImpHist;  // history record for document import

    fOK = UtlAlloc( (PVOID *)&pDocImpHist, 0L, (ULONG)sizeof(DOCIMPORTHIST2), ERROR_STORAGE );

    if ( fOK )
    {
      pDocImpHist->sType               = EXTERN_SUBTYPE;
      strcpy( pDocImpHist->szPath,       pIda->szSource );
      strcpy( pDocImpHist->szFileName,   pIda->stFs.szName );
      strcat( pDocImpHist->szFileName,   pIda->stFs.szExt );
      strcpy( pDocImpHist->szMarkup,     pIda->DocInfo.szFormat );
      strcpy( pDocImpHist->szMemory,     pIda->DocInfo.szMemory );
      strcpy( pDocImpHist->szSourceLang, pIda->DocInfo.szSourceLang );
      strcpy( pDocImpHist->szTargetLang, pIda->DocInfo.szTargetLang );
      pDocImpHist->fSourceDocReplaced =  pIda->fDocExists;
      pDocImpHist->fTargetDocReplaced =  FALSE;

      // open folder properties to get shipment number (SHIPMENT_HANDLER)
      if ( fOK )
      {
        EQFINFO     ErrorInfo;           // error code of property handler calls
        PPROPFOLDER pFolProp;            // ptr to folder properties
        PVOID       hFolProp;            // handle of folder properties
        OBJNAME     szFolObjName;        // buffer for folder object name
        CHAR        szTmgrDrive[3];      // buffer for Tmgr primary drive

        // setup folder object name using target folder name and primary Tmgr drive
        UtlQueryString( QST_PRIMARYDRIVE, szTmgrDrive, sizeof(szTmgrDrive) );
        strcpy( szFolObjName, pIda->szTargetFolder );
        szFolObjName[0] = szTmgrDrive[0];

        hFolProp = OpenProperties( szFolObjName, NULL, PROP_ACCESS_READ, &ErrorInfo);
        if ( hFolProp )
        {
          pFolProp = (PPROPFOLDER)MakePropPtrFromHnd( hFolProp );
          strcpy(pDocImpHist->szShipment , pFolProp->szShipment);
          strcpy(szShipment , pFolProp->szShipment);
          CloseProperties( hFolProp, PROP_QUIT, &ErrorInfo );
        }
        else
        {
          strcpy(pDocImpHist->szShipment , "Not available");
          strcpy(szShipment , "Not available");
        } /* endif */
      } /* endif */

      strcpy( pIda->szDummy, pIda->stFs.szName );
      strcat( pIda->szDummy, pIda->stFs.szExt );

      EQFBWriteHistLog2( pIda->szTargetFolder,
                         pIda->szDummy, DOCIMPORT_LOGTASK2,
                         sizeof(DOCIMPORTHIST2), (PVOID)pDocImpHist,
                         TRUE, NULLHANDLE, pIda->stFs.szLongName );

      UtlAlloc( (PVOID *)&pDocImpHist, 0L, 0L, NOMSG );

      // Update document properties with szShipment
      if ( (hpropDocument = OpenProperties (pIda->szProperties, NULL, PROP_ACCESS_READ, &ulErrorInfo))== NULL)
      {
      }
      else
      {
        PPROPDOCUMENT ppropDocument;        // pointer to document properties

        SetPropAccess(hpropDocument , PROP_ACCESS_WRITE);
        ppropDocument = (PPROPDOCUMENT)MakePropPtrFromHnd( hpropDocument );
        strcpy(ppropDocument->szShipment, szShipment);
        SaveProperties(hpropDocument , &ErrorInfo );
        CloseProperties(hpropDocument , PROP_QUIT, &ErrorInfo );
      }//end if
    } /* endif */
  } /* endif */
  return( fOK );
}

//f3////////////////////////////////////////////////////////////////////////////
// function DocumentUnLoad, will be included into document object handler     //
////////////////////////////////////////////////////////////////////////////////
USHORT DocumentUnload( PDOCEXPIDA pIda, HWND hwnd )
{
  USHORT      usResult=UNLOAD_OK;          //return of function
  USHORT      usSaveResult=UNLOAD_OK;      //return UnloadTarget
  PSZ         pszLongName;

  // use document name as default export name
  // export with or without relative path information
  if ( pIda->sExportFormatID != ID_DOCEXP_IMPPATH_RB )
  {
    // skip relative path information as we export in normal mode
    PSZ pszPathEnd = strrchr( pIda->szLongName, BACKSLASH );
    if ( pszPathEnd != NULL)
    {
      strcpy( pIda->szExpName, pszPathEnd + 1 );
    }
    else
    {
      strcpy( pIda->szExpName, pIda->szLongName );
    } /* endif */
  }
  else
  {
    strcpy( pIda->szExpName, pIda->szLongName );
  } /* endif */

  INFOEVENT2( DOCUMENTUNLOAD_LOC, FUNCENTRY_EVENT, 0, DOC_GROUP, pIda->szExpName );

  // get a short name for the document if document has a long file name
  // and the target system does not support long file names
  {
    // skip any relative path information in the document name
    PSZ pszPathEnd = strrchr( pIda->szLongName, BACKSLASH );
    if ( pszPathEnd != NULL)
    {
      pszLongName = pszPathEnd + 1;
    }
    else
    {
      pszLongName = pIda->szLongName;
    } /* endif */
  }
  // Check if all drives support longfilenames
  //
  if ( UtlIsLongFileName(pszLongName) && (
                                         (pIda->fTarget   && !UtlSupportsLongNames(pIda->szSelectedTransDrive[0]) ) ||
                                         (pIda->fSource   && !UtlSupportsLongNames(pIda->szSelectedOrgDrive[0]  ) ) ||
                                         (pIda->fSNoMatch && !UtlSupportsLongNames(pIda->szSelectedSnoDrive[0]  ) )    ) )
  {
    BOOL fOK;
    HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
    INFOEVENT2( DOCUMENTUNLOAD_LOC, STATE_EVENT, 1, DOC_GROUP, NULL );
    DIALOGBOX( hwnd, LONGTOSHORTDLGPROC, hResMod, ID_DOCSHORT_DLG, pIda, fOK );
    if ( !fOK )
    {
      usResult = UNLOAD_CANCEL;
    } /* endif */
  } /* endif */

  //
  // TARGET
  //
  if ( (pIda->fTarget == TRUE) &&         //target file will be unloaded
       (usResult != UNLOAD_CANCEL) )
  {
    INFOEVENT2( DOCUMENTUNLOAD_LOC, STATE_EVENT, 2, DOC_GROUP, NULL );
    usResult = UnloadDocument( pIda, TRUE );
    usSaveResult = usResult;
  } /* endif */

  //
  // SOURCE
  //
  if ( usResult != UNLOAD_CANCEL )
  {
    INFOEVENT2( DOCUMENTUNLOAD_LOC, STATE_EVENT, 3, DOC_GROUP, NULL );

    if ( pIda->fSource == TRUE )       //source files will be unloaded
    {
      usResult = UnloadDocument( pIda, FALSE );
    } /* endif */
  } /* endif */

  //
  // SNOMATCH
  //
  if ( (usResult != UNLOAD_CANCEL) )
  {

    if ( pIda->fSNoMatch == TRUE )       //SNOMATCH files will be unloaded
    {
      usResult = ExportSNOMATCHDocument( pIda );
    } /* endif */
  } /* endif */

  //if UnloadTarget returns UNLOAD_NOTOK and UnloadSource returns not CANCEL
  if ( (usSaveResult == UNLOAD_NOTOK) && (usResult != UNLOAD_CANCEL) )
  {
    usResult = usSaveResult;           //return UNLOAD_NOTOK
  } /* endif */

  /************************************************************/
  /* Add export record to history log                         */
  /************************************************************/
  if ( usResult == UNLOAD_OK )
  {
    BOOL fOK = TRUE;

    PDOCEXPORTHIST pDocExpHist;  // history record for document export

    fOK = UtlAlloc( (PVOID *)&pDocExpHist, 0L, (ULONG)sizeof(DOCEXPORTHIST),
                    ERROR_STORAGE );
    if ( fOK )
    {
      pDocExpHist->sType          = EXTERN_SUBTYPE;
      pDocExpHist->fSource        = pIda->fSource;
      pDocExpHist->fTarget        = pIda->fTarget;
      pDocExpHist->fRevisionMarks = pIda->fWithRevMark;
      pDocExpHist->fSNOMATCH      = pIda->fSNoMatch;
      strcpy( pDocExpHist->szSourcePath, pIda->szSourceDir );
      strcpy( pDocExpHist->szTargetPath, pIda->szTargetDir );
      strcpy( pDocExpHist->szSNOMATCH, pIda->szSNoMatchDir );
      EQFBWriteHistLog2( pIda->szFolderObjName,
                         pIda->szName, DOCEXPORT_LOGTASK,
                         sizeof(DOCEXPORTHIST), (PVOID)pDocExpHist,
                         TRUE, NULLHANDLE, pIda->szLongName );
      UtlAlloc( (PVOID *)&pDocExpHist, 0L, 0L, NOMSG );
    }
    else
    {
      usResult = UNLOAD_NOTOK;
    } /* endif */
  } /* endif */

  INFOEVENT2( DOCUMENTUNLOAD_LOC, FUNCEXIT_EVENT, usResult, DOC_GROUP, NULL );
  return( usResult );
}/*end DocumentUnLoad */

//f4////////////////////////////////////////////////////////////////////////////
// function UnloadDocument                                                    //
////////////////////////////////////////////////////////////////////////////////
USHORT UnloadDocument( PDOCEXPIDA pIda, BOOL fTarget )
{
  USHORT       usDosRc;                     //Return code from Dos(Utl) operations
  USHORT       usRc;                        //USHORT return code
  USHORT       usResponse = MBID_YES;       //return from UtlError
  PSZ          pszReplace;                  //pointer to replace string UtlError
  BOOL         fOK = TRUE;                  //error flag and function return
  USHORT       usResult = UNLOAD_NOTOK;
  USHORT       usTrackDocNum ;
  BOOL         fGoOn=FALSE;
  PSZ          pszTemp;
  PSZ          apszRevMark[6];              // revision mark pointers


  INFOEVENT2( UNLOADDOCUMENT_LOC, FUNCENTRY_EVENT, 0, DOC_GROUP, NULL );

  /*******************************************************************/
  /* build fully qualified document name                             */
  /*******************************************************************/
  {
    SHORT sPath;

    if ( fTarget )
    {
      sPath = DIRSEGTARGETDOC_PATH;
    } 
    sPath = DIRSOURCEDOC_PATH;

    UtlMakeEQFPath( pIda->szSource, pIda->szFolderObjName[0], sPath, pIda->szFolderName );
    strcat( pIda->szSource, BACKSLASH_STR );
    strcat( pIda->szSource, pIda->szName );
  }

  if ( !CheckExistence( pIda->szSource, FALSE, FILE_NORMAL, &usRc ) )
  {
    /*****************************************************************/
    /* Document does not exist                                       */
    /*****************************************************************/
    pszReplace = pIda->szLongName;
    UtlError( (SHORT)(( fTarget ) ? ERROR_NOTARGETFILE : FILE_NOT_EXISTS), MB_CANCEL, 1, &pszReplace, EQF_ERROR  );
    fOK = FALSE;
  } /* endif */

  if ( fOK )
  {
    PSZ pszDir = ( fTarget ) ? pIda->szTargetDir : pIda->szSourceDir;

    INFOEVENT2( UNLOADDOCUMENT_LOC, STATE_EVENT, 1, DOC_GROUP, NULL );

    /****************************************************************/
    /* Check if target directory exists                             */
    /****************************************************************/

    if (fTarget)
    {
      strcpy(pIda->szSelectedDrive, pIda->szSelectedTransDrive);

    }
    else
    {
      strcpy(pIda->szSelectedDrive, pIda->szSelectedOrgDrive);
    }//end if

    UtlMakeFullPath( pIda->szTarget, pIda->szSelectedDrive, pszDir, pIda->szExpName, NULL );

    UtlSplitFnameFromPath( pIda->szTarget );

    if ( (*pszDir == EOS) || (strcmp( pszDir, BACKSLASH_STR) == 0) )
    {
      // nothing to do, no directory has been specified
    }
    else if ( CheckExistence( pIda->szTarget, FALSE, FILE_DIRECTORY, &usRc ) )
    {
      // nothing to do, directory exists already
    }
    else
    {
      switch ( usRc )
      {
        case ( ERROR_FILE_NOT_FOUND ) :
        case ( ERROR_NO_MORE_FILES ) :
        case ( ERROR_PATH_NOT_FOUND ) :
        case ( ERROR_DIRECTORY ) :
          //display message that path does not exist
          pszReplace = pIda->szTarget + 3;
          if ( pIda->fCreateAllDirs )
          {
            usResponse = MBID_YES;
          }
          else
          {
            usResponse = UtlError( PATH_NOTEXIST_CREATEIT, MB_EQF_YESTOALL | MB_DEFBUTTON3, 1, &pszReplace, EQF_QUERY );
            if ( usResponse == MBID_EQF_YESTOALL )
            {
              pIda->fCreateAllDirs = TRUE;
              usResponse = MBID_YES;
            } /* endif */
          } /* endif */

          if ( usResponse == MBID_YES )     //YES button selected, create dir
          {
            //call utility to create directory
            usRc = UtlMkMultDir( pIda->szTarget, TRUE );
            if ( usRc != NO_ERROR )      //directory not created
            {
              usResponse = MBID_CANCEL;
              fOK = FALSE;
            } /* endif */
          }
          else                            //YES or CANCEL button selected
          {
            if ( (usResponse == MBID_NO) && pIda->fTarget )
            {
              //do not continue with unload target files
              //but continue with unload source files
              fOK = FALSE;
            } /* endif */
          } /* endif */
          break;
        case ( ERROR_DISK_CHANGE ) :
          usResponse = MBID_CANCEL;
          break;
      }/*end switch*/
    } /* endif */

    /****************************************************************/
    /* build name of file on target disk                            */
    /****************************************************************/
    UtlMakeFullPath( pIda->szTarget, pIda->szSelectedDrive, ( fTarget ) ? pIda->szTargetDir : pIda->szSourceDir,
                     pIda->szExpName, NULL );

    if ( fOK && (usResponse != MBID_CANCEL) )
    {
      INFOEVENT2( UNLOADDOCUMENT_LOC, STATE_EVENT, 2, DOC_GROUP, NULL );
      //reset Response

      usResponse = MBID_YES;

      if ( CheckExistence( pIda->szTarget, FALSE, FILE_NORMAL, &usRc ) )//file exists on
      {
        // target disk
        //display error message if file should be replaced
        if ( pIda->fOverwrite )
        {
          usResponse = MBID_YES;
        }
        else
        {
          pszReplace = pIda->szTarget;
          usResponse = UtlError( ERROR_FILE_EXISTS_ALREADY,
                                 MB_EQF_YESTOALL | MB_DEFBUTTON3,
                                 1, &pszReplace, EQF_WARNING );
          if ( usResponse == MBID_EQF_YESTOALL )
          {
            pIda->fOverwrite = TRUE;
            usResponse = MBID_YES;
          } /* endif */
        } /* endif */
      }
      else
      {
        switch ( usRc )
        {
          case ( ERROR_PATH_NOT_FOUND ) :
          case ( ERROR_DIRECTORY ) :
            //display message that path does not exist
            {
              PSZ pszFName;
              strcpy( pIda->szBuffer, pIda->szTarget + 3 );
              pszFName = strrchr( pIda->szBuffer, BACKSLASH );
              if ( pszFName != NULL )
              {
                *pszFName = EOS;
              } /* endif */
              pszReplace = pIda->szBuffer;
            }
            if ( pIda->fCreateAllDirs )
            {
              usResponse = MBID_YES;
            }
            else
            {
              usResponse = UtlError( PATH_NOTEXIST_CREATEIT,
                                     MB_EQF_YESTOALL | MB_DEFBUTTON3, 1,
                                     &pszReplace, EQF_QUERY );
              if ( usResponse == MBID_EQF_YESTOALL )
              {
                pIda->fCreateAllDirs = TRUE;
                usResponse = MBID_YES;
              } /* endif */
            } /* endif */

            if ( usResponse == MBID_YES )     //YES button selected, create dir
            {
              //call utility to create directory
              pszTemp = UtlGetFnameFromPath( pIda->szTarget ) - 1;
              *pszTemp = EOS;
              usRc = UtlMkMultDir( pIda->szTarget, TRUE );
              *pszTemp = BACKSLASH;
              if ( usRc != NO_ERROR )      //directory not created
              {
                usResponse = MBID_CANCEL;
                fOK = FALSE;
              } /* endif */
            }
            else                            //YES or CANCEL button selected
            {
              if ( (usResponse == MBID_NO) && pIda->fTarget )
              {
                //do not continue with unload target files
                //but continue with unload source files
                fOK = FALSE;
              } /* endif */
            } /* endif */
            break;
          case ( ERROR_DISK_CHANGE ) :
            usResponse = MBID_CANCEL;
            break;
        }/*end switch*/
      }/*end if*/
    } /* endif */
  } /* endif */

  /* check for error condition (abortion ?) */
  if ( usResponse != MBID_YES )
  {
    INFOEVENT2( UNLOADDOCUMENT_LOC, STATE_EVENT, 3, DOC_GROUP, NULL );
    fOK = FALSE;  /* stop further processing */
  } /* endif */
  if ( usResponse == MBID_CANCEL )
  {
    usResult = UNLOAD_CANCEL;
  } /* endif */

  /********************************************************************/
  /* for target documents: unsegment the document                     */
  /********************************************************************/
  if ( fOK && fTarget )
  {
    PSZ pszConversion = NULL;

    INFOEVENT2( UNLOADDOCUMENT_LOC, STATE_EVENT, 4, DOC_GROUP, NULL );

    UtlMakeEQFPath( pIda->szSegTargetFile, pIda->szFolderObjName[0],
                    DIRSEGTARGETDOC_PATH, pIda->szFolderName );
    strcat( pIda->szSegTargetFile, BACKSLASH_STR );
    strcat( pIda->szSegTargetFile, pIda->szName );
    if ( ! UtlFileExist( pIda->szSegTargetFile ))    /* 5-2-14 */
    {
      fOK = FALSE;
      pszTemp = pIda->szLongName;
      UtlError( ERROR_NOTARGETFILE, MB_CANCEL, 1, &pszTemp, EQF_ERROR );
    } 
    if ( fOK ) {
       UtlMakeEQFPath( pIda->szSource, pIda->szFolderObjName[0],
                       DIRTARGETDOC_PATH, pIda->szFolderName );
       strcat( pIda->szSource, BACKSLASH_STR );
       strcat( pIda->szSource, pIda->szName );

       {
         // handle output conversion
         if ( strcmp( pIda->szConversion, pIda->szUseDocConv ) == 0 )
         {
           // use document conversion
           DocQueryInfoEx( pIda->szDocObjName, NULL, NULL, NULL, NULL,
                           NULL, NULL, NULL, pIda->szConversion, NULL, NULL,
                           FALSE, NULLHANDLE );
           if ( pIda->szDocConversion[0] ) pszConversion = pIda->szDocConversion;
         }
         else if ( pIda->szConversion[0] )
         {
           // use specified conversion
           pszConversion = pIda->szConversion;
         } /* endif */
       } /* endif */

       HPROP       hpropDocument;       // document properties
       ULONG       ulErrorInfo;         // error indicator from PRHA
       usTrackDocNum = 0 ;
       if ( ( pIda->fWithTrackID ) &&
            ( (hpropDocument = OpenProperties (pIda->szDocObjName, NULL, PROP_ACCESS_READ, &ulErrorInfo))!= NULL) ) 
       {
         PPROPDOCUMENT ppropDocument;        // pointer to document properties
         EQFINFO       ErrorInfo;            // error code of property handler calls
      
         ppropDocument = (PPROPDOCUMENT)MakePropPtrFromHnd( hpropDocument );
         usTrackDocNum = ppropDocument->usTrackDocNum ;
         CloseProperties(hpropDocument, PROP_QUIT, &ErrorInfo );
       }


       if ( pIda->fWithRevMark  )
       {
         /****************************************************************/
         /* Setup revision mark pointers                                 */
         /****************************************************************/
         apszRevMark[0] = (PSZ)pIda->pRevMark + pIda->pRevMark->usOffsCat1Beg;
         apszRevMark[1] = (PSZ)pIda->pRevMark + pIda->pRevMark->usOffsCat1End;
         apszRevMark[2] = (PSZ)pIda->pRevMark + pIda->pRevMark->usOffsCat2Beg;
         apszRevMark[3] = (PSZ)pIda->pRevMark + pIda->pRevMark->usOffsCat2End;
         apszRevMark[4] = (PSZ)pIda->pRevMark + pIda->pRevMark->usOffsCat3Beg;
         apszRevMark[5] = (PSZ)pIda->pRevMark + pIda->pRevMark->usOffsCat3End;

         /****************************************************************/
         /* Do actual unsegmentation                                     */
         /****************************************************************/
         fOK = EQFUnSegRevMark2( pIda->szSegTargetFile, pIda->szSource,
                                 TADummyTag, &fGoOn, pIda->szFolderName,
                                 apszRevMark, usTrackDocNum, pszConversion );
       }
       else
       {
         fOK = EQFUnSeg2( pIda->szSegTargetFile, pIda->szSource,
                          TADummyTag, &fGoOn, pIda->szFolderName, 
                          usTrackDocNum, pszConversion );
       } /* endif */
    }


    if ( !fOK )
    {
      INFOEVENT2( UNLOADDOCUMENT_LOC, INTFUNCFAILED_EVENT, 1, DOC_GROUP, NULL );
      /***************************************************************/
      /* unsegment of target failed                                  */
      /***************************************************************/
      pszReplace = pIda->szSource;
//     UtlError( ERROR_NOTARGETFILE, MB_CANCEL, 1,
//               &pszReplace, EQF_WARNING );
      usResult = UNLOAD_CANCEL;
      fOK = FALSE;
    } /* endif */
  } /* endif */


  if ( fOK )
  {
    if ( usResponse == MBID_YES )           //file exists and should be replaced
    {
      // return codes which should not pop-up an error message
      static USHORT ausRCList[] = { ERROR_FILENAME_EXCED_RANGE, 0};

      usDosRc = UtlCopyHwnd2( pIda->szSource, pIda->szTarget,
                              1, 0L, TRUE, ausRCList, NULLHANDLE );

      switch ( usDosRc )
      {
        case ERROR_FILENAME_EXCED_RANGE :
          // long file name not supported by target drive, get
          // short name, if long-to-short name hasn't been called yet
          if ( strcmp( pIda->szExpName, pIda->szLongName ) == 0 )
          {
            HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
            // get a short name for the document
            DIALOGBOX( pIda->hwndDlg, LONGTOSHORTDLGPROC, hResMod,
                       ID_DOCSHORT_DLG, pIda, fOK );
            if ( !fOK )
            {
              // dialog has been cancelled
              usResult = UNLOAD_CANCEL;
            } /* endif */

            if ( fOK )
            {
              // setup path to new target file
              UtlMakeFullPath( pIda->szTarget, pIda->szSelectedDrive,
                               ( fTarget ) ? pIda->szTargetDir : pIda->szSourceDir,
                               pIda->szExpName, NULL );

              // check if target file exists
              usResponse = MBID_YES;
              if ( CheckExistence( pIda->szTarget, FALSE, FILE_NORMAL, &usRc ) )//file exists on
              {
                // target disk
                //display error message if file should be replaced
                if ( pIda->fOverwrite )
                {
                  usResponse = MBID_YES;
                }
                else
                {
                  pszReplace = pIda->szTarget;
                  usResponse = UtlError( ERROR_FILE_EXISTS_ALREADY,
                                         MB_EQF_YESTOALL | MB_DEFBUTTON3,
                                         1, &pszReplace, EQF_WARNING );
                  if ( usResponse == MBID_EQF_YESTOALL )
                  {
                    pIda->fOverwrite = TRUE;
                    usResponse = MBID_YES;
                  } /* endif */
                } /* endif */
              } /* endif */

              // try to export the document again
              if ( usResponse == MBID_YES )
              {
                usDosRc = UtlCopy( pIda->szSource, pIda->szTarget, 1, 0L, TRUE );
              } /* endif */
            } /* endif */
          }
          else
          {
            // seems we have shown docshort dialog already, so issue
            // error message here
            PSZ pParm = pIda->szLongName;
            UtlError( usDosRc, MB_CANCEL, 1, &pParm, DOS_ERROR );
          } /* endif */
          break;

        default :
          break;
      } /* endswitch */

      if ( usDosRc == NO_ERROR )
      {
        usResult = UNLOAD_OK;
      }
      else
      {
        usResult = UNLOAD_CANCEL;
      } /* endif */

      /**************************************************************/
      /* Handle any EA data of file                                 */
    }
    else if ( usResponse == MBID_CANCEL )     //stop unloading
    {
      usResult = UNLOAD_CANCEL;
    }/*end if*/

    if ( fTarget )
    {
      // delete created target file
      UtlDelete( pIda->szSource, 0L, FALSE );
    }/*end if*/
  } /*end if fOK*/

  INFOEVENT2( UNLOADDOCUMENT_LOC, FUNCEXIT_EVENT, usResult, DOC_GROUP, NULL );

  return usResult;

}/*end UnloadTarget*/


USHORT ExportSNOMATCHDocument( PDOCEXPIDA pIda )
{
  USHORT       usDosRc;               // return code from Dos functions
  USHORT       usRc;                  // function return code
  USHORT       usResponse;            // return from UtlError
  PSZ          pszReplace;            // pointer to replace string UtlError
  BOOL         fOK = TRUE;            // error flag and function return
  USHORT       usResult = UNLOAD_NOTOK;
  PSZ          pszTemp;

  /*******************************************************************/
  /* build fully qualified document name                             */
  /*******************************************************************/
  UtlMakeEQFPath( pIda->szSource,
                  pIda->szFolderObjName[0],
                  DIRSEGNOMATCH_PATH,
                  pIda->szFolderName );
  strcat( pIda->szSource, BACKSLASH_STR );
  strcat( pIda->szSource, pIda->szName );

  if ( !UtlFileExist( pIda->szSource ) )
  {
    pszReplace = pIda->szLongName;
    usResponse = UtlError( ERROR_NO_SNOMATCH_FOR_DOC, MB_OKCANCEL, 1,
                           &pszReplace, EQF_WARNING );
    switch ( usResponse )
    {
      case MBID_CANCEL :
        usResult = UNLOAD_CANCEL;
        break;
      case MBID_OK :
      default :
        usResult = UNLOAD_NOTOK;
        break;
    } /* endswitch */
  }
  else
  {
    /********************************************************************/
    /* check target file and target path                                */
    /********************************************************************/
    if ( fOK )
    {
      /****************************************************************/
      /* build name of file on target disk                            */
      /****************************************************************/
      UtlMakeFullPath( pIda->szTarget,
                       pIda->szSelectedSnoDrive,
                       pIda->szSNoMatchDir,
                       pIda->szExpName,
                       NULL );

      if ( CheckExistence( pIda->szTarget, FALSE, FILE_NORMAL, &usRc ) )//file exists on
      {
        // target disk
        //display error message if file should be replaced
        if ( pIda->fOverwrite )
        {
          usResponse = MBID_YES;
        }
        else
        {
          pszReplace = pIda->szTarget;
          usResponse = UtlError( ERROR_FILE_EXISTS_ALREADY,
                                 MB_EQF_YESTOALL | MB_DEFBUTTON3,
                                 1, &pszReplace, EQF_WARNING );
          if ( usResponse == MBID_EQF_YESTOALL )
          {
            pIda->fOverwrite = TRUE;
            usResponse = MBID_YES;
          } /* endif */
        } /* endif */
      }
      else
      {
        switch ( usRc )
        {
          case ( ERROR_PATH_NOT_FOUND ) :
            //display message that path does not exist
            pszReplace = pIda->szSNoMatchDir;
            if ( pIda->fCreateAllDirs )
            {
              usResponse = MBID_YES;
            }
            else
            {
              usResponse = UtlError( PATH_NOTEXIST_CREATEIT,
                                     MB_EQF_YESTOALL | MB_DEFBUTTON3, 1,
                                     &pszReplace, EQF_QUERY );
              if ( usResponse == MBID_EQF_YESTOALL )
              {
                pIda->fCreateAllDirs = TRUE;
                usResponse = MBID_YES;
              } /* endif */
            } /* endif */

            if ( usResponse == MBID_YES )     //YES button selected, create dir
            {
              //call utility to create directory
              pszTemp = UtlGetFnameFromPath( pIda->szTarget ) - 1;
              *pszTemp = EOS;
              usRc = UtlMkMultDir( pIda->szTarget, TRUE );
              *pszTemp = BACKSLASH;
              if ( usRc != NO_ERROR )      //directory not created
              {
                usResponse = MBID_CANCEL;
              } /* endif */
            }
            else                            //YES or CANCEL button selected
            {
              if ( usResponse == MBID_NO )
              {
                //do not continue with unload SNOMATCH files
                //but continue with unload of other files
//                    pIda->fSNoMatch = FALSE;                      /*KIT1226C*/
                fOK = FALSE;                                    /*KIT1226A*/
                usResult = UNLOAD_CANCEL;                       /*KIT1229A*/
              } /* endif */
            } /* endif */
            break;
          case ( NO_ERROR ) :
          default:
            usResponse = MBID_YES;
            break;
        }/*end switch*/
      }/*end if*/

      if ( fOK && (usResponse == MBID_YES)) //file exists and should be replaced /*KIT1226C*/
      {
        //copy file to target path
        usDosRc = UtlCopy( pIda->szSource,
                           pIda->szTarget,
                           1, 0L, TRUE );
        if ( usDosRc == NO_ERROR )
        {
          usResult = UNLOAD_OK;
        }
        else
        {
          usResult = UNLOAD_CANCEL;
        } /* endif */
      }
      else if ( usResponse == MBID_CANCEL )     //stop unloading
      {
        usResult = UNLOAD_CANCEL;
      }/*end if*/
    } /* endif */
  } /* endif */

  return usResult;

}/* ExportSNOMATCHDocument */


//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     DocExportDlg   Dialog procedure for the export doc. dlg  |
//+----------------------------------------------------------------------------+
//|Function call:     DocExportDlg ( HWND hwnd, USHORT msg, MPARAM mp1,        |
//|                                  MPARAM mp2 )                              |
//+----------------------------------------------------------------------------+
//|Description:      Dialog procedure for the export of documents dialog.      |
//+----------------------------------------------------------------------------+
//|Input parameter:  HWND    hwnd     handle of window                         |
//|                  USHORT  msg      type of message                          |
//|                  MPARAM  mp1      first message parameter                  |
//|                  MPARAM  mp2      second message parameter                 |
//+----------------------------------------------------------------------------+
//|Returncode type:  MRESULT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:      depends on message type                                   |
//|                  normal return codes are:                                  |
//|                  TRUE  = message has been processed                        |
//|                  FALSE = message has not been processed                    |
//+----------------------------------------------------------------------------+

INT_PTR CALLBACK FEXPORTDLGPROPPROC
(
HWND   hwnd,
WINMSG msg,
WPARAM mp1,
LPARAM mp2
)
{
  MRESULT     mResult = FALSE;
  PDOCEXPIDA  pIda;                        // pointer to document import IDA
  CHAR        szBuffer[200];
  TC_ITEM     TabCtrlItem;

  //UCD_ONE_PROPERTY
  SHORT       sNotification = WMCOMMANDCMD( mp1, mp2 );               // notification

  switch ( msg )
  {
    case WM_EQF_QUERYID: HANDLEQUERYID( ID_FILEXP_DLG, mp2 ); break;

    case WM_INITDLG :             //initialize and display dialogbox
      {
//        CHAR      szBuffer[80];
//        TC_ITEM   TabCtrlItem;
        RECT      rect;
        HINSTANCE hInst       = GETINSTANCE( hwnd );
        HWND      hwndTabCtrl = GetDlgItem( hwnd, ID_DOCEXP_PROP_TABCTRL );
        HWND      hwndDlg;
        USHORT    nItem = 0;
        HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);

        GetClientRect( hwndTabCtrl, &rect );
        TabCtrl_AdjustRect( hwndTabCtrl, FALSE, &rect );

        // leave some additional space at top
        rect.top += 20;
        MapWindowPoints( hwndTabCtrl, hwnd, (POINT *) &rect, 2 );

        TabCtrlItem.mask = TCIF_TEXT;
        /******************************************************************/
        /* create the appropriate TAB control and load the associated     */
        /* dialog                                                         */
        /******************************************************************/

        LOADSTRING( hab, hResMod, SID_DOCEXP_EXTFORMAT, szBuffer );
        TabCtrlItem.pszText = szBuffer;
        SendMessage(hwndTabCtrl, TCM_INSERTITEM, nItem, (LPARAM)&TabCtrlItem);
        nItem++;


        LOADSTRING( hab, hResMod, SID_DOCEXP_IMPPATHFORMAT, szBuffer );
        TabCtrlItem.pszText = szBuffer;
        SendMessage(hwndTabCtrl, TCM_INSERTITEM, nItem, (LPARAM)&TabCtrlItem);
        nItem++;

        LOADSTRING( hab, hResMod, SID_DOCEXP_INTFORMAT, szBuffer );
        TabCtrlItem.pszText = szBuffer;
        SendMessage(hwndTabCtrl, TCM_INSERTITEM, nItem, (LPARAM)&TabCtrlItem);
        nItem++;

        hwndDlg = CreateDialogParam( hInst,
                                     MAKEINTRESOURCE( ID_DOCEXP_DLG ),
                                     hwnd, FEXPORTDLGPROC, (LPARAM)NULL );

        pIda = ACCESSDLGIDA( hwndDlg, PDOCEXPIDA );
        ANCHORDLGIDA( hwnd, pIda );
        pIda->hwndPropertySheet = hwnd;  // anchor property sheet handle

        SetWindowPos( hwndDlg, HWND_TOP,
                      rect.left, rect.top,
                      rect.right-rect.left, rect.bottom-rect.top, SWP_SHOWWINDOW );

        UtlRegisterModelessDlg( hwndDlg );

        //
        //Fill Export Format CB
        //
        //UCD_ONE_PROPERTY
        {
          SHORT sItem = 0;

        CBDELETEALL(hwnd, ID_DOCEXP_FORMAT_CB);

        LOADSTRING (hab, hResMod, SID_DOCEXP_EXTFORMAT, pIda->szBuffer);
          sItem = CBINSERTITEMEND( hwnd, ID_DOCEXP_FORMAT_CB,pIda->szBuffer);
          CBSETITEMHANDLE( hwnd, ID_DOCEXP_FORMAT_CB, sItem, ID_DOCEXP_EXTFORMAT_RB );
          // select first item per default...
          CBSELECTITEM (hwnd, ID_DOCEXP_FORMAT_CB, sItem );

        LOADSTRING (hab, hResMod, SID_DOCEXP_IMPPATHFORMAT, pIda->szBuffer);
          sItem = CBINSERTITEMEND( hwnd, ID_DOCEXP_FORMAT_CB,pIda->szBuffer);
          CBSETITEMHANDLE( hwnd, ID_DOCEXP_FORMAT_CB, sItem, ID_DOCEXP_IMPPATH_RB );
          if ( pIda->sExportFormatID == ID_DOCEXP_IMPPATH_RB ) CBSELECTITEM (hwnd, ID_DOCEXP_FORMAT_CB, sItem );

        LOADSTRING (hab, hResMod, SID_DOCEXP_INTFORMAT, pIda->szBuffer);
          sItem = CBINSERTITEMEND( hwnd, ID_DOCEXP_FORMAT_CB,pIda->szBuffer);
          CBSETITEMHANDLE( hwnd, ID_DOCEXP_FORMAT_CB, sItem, ID_DOCEXP_INTFORMAT_RB );
          if ( pIda->sExportFormatID == ID_DOCEXP_INTFORMAT_RB ) CBSELECTITEM (hwnd, ID_DOCEXP_FORMAT_CB, sItem );

          sItem = CBINSERTITEMEND( hwnd, ID_DOCEXP_FORMAT_CB, "Validation Format" );
          CBSETITEMHANDLE( hwnd, ID_DOCEXP_FORMAT_CB, sItem, ID_DOCEXP_VALFORMAT_RB );
          if ( pIda->sExportFormatID == ID_DOCEXP_VALFORMAT_RB ) CBSELECTITEM (hwnd, ID_DOCEXP_FORMAT_CB, sItem );

          sItem = CBINSERTITEMEND( hwnd, ID_DOCEXP_FORMAT_CB, "Plain XML Format" );
          CBSETITEMHANDLE( hwnd, ID_DOCEXP_FORMAT_CB, sItem, ID_DOCEXP_XMLFORMAT_RB );
          if ( pIda->sExportFormatID == ID_DOCEXP_XMLFORMAT_RB ) CBSELECTITEM (hwnd, ID_DOCEXP_FORMAT_CB, sItem );
        }
        //
        // Select active page, suppress all other pages
        //
        //UCD_ONE_PROPERTY
        TabCtrl_DeleteAllItems(hwndTabCtrl );
      }
      break;

    case WM_COMMAND :         //UCD_ONE_PROPERTY
      pIda = ACCESSDLGIDA(hwnd, PDOCEXPIDA );

      if (WMCOMMANDID( mp1, mp2) == ID_DOCEXP_FORMAT_CB)
      {
        HWND      hwndTabCtrl = GetDlgItem( hwnd, ID_DOCEXP_PROP_TABCTRL );
        RECT      rect;
        HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);

        GetClientRect( hwndTabCtrl, &rect );

        if (sNotification == CBN_SELCHANGE)
        {
          SHORT sItem = CBQUERYSELECTION (hwnd, ID_DOCEXP_FORMAT_CB);
          USHORT usFormat = (USHORT)CBQUERYITEMHANDLE(hwnd, ID_DOCEXP_FORMAT_CB, sItem );

          TabCtrl_DeleteAllItems(hwndTabCtrl );

          // set the tab to full width
          {
            RECT rect;

            GetClientRect( hwndTabCtrl, &rect );
            TabCtrl_SetMinTabWidth(  hwndTabCtrl, rect.right - rect.left -4);
          }


          TabCtrlItem.mask = TCIF_TEXT;

          if (usFormat == ID_DOCEXP_EXTFORMAT_RB)
          {
            LOADSTRING( hab, hResMod, SID_DOCEXP_EXTFORMAT, szBuffer );
            TabCtrlItem.pszText = szBuffer;
            SendMessage(hwndTabCtrl, TCM_INSERTITEM, 0, (LPARAM)&TabCtrlItem);
            DocExpControl( pIda->hwndDlg, ID_DOCEXP_EXTFORMAT_RB, BN_CLICKED );
            pIda->sExportFormatID = ID_DOCEXP_EXTFORMAT_RB;
            ShowWindow( pIda->hwndDlg, SW_SHOW );
          }
          else if (usFormat == ID_DOCEXP_IMPPATH_RB)
          {
            LOADSTRING( hab, hResMod, SID_DOCEXP_IMPPATHFORMAT, szBuffer );
            TabCtrlItem.pszText = szBuffer;
            SendMessage(hwndTabCtrl, TCM_INSERTITEM, 0, (LPARAM)&TabCtrlItem);
            DocExpControl( pIda->hwndDlg, ID_DOCEXP_IMPPATH_RB, BN_CLICKED );
            pIda->sExportFormatID = ID_DOCEXP_IMPPATH_RB;
            ShowWindow( pIda->hwndDlg, SW_SHOW );
          }
          else if (usFormat == ID_DOCEXP_INTFORMAT_RB)
          {
            LOADSTRING( hab, hResMod, SID_DOCEXP_INTFORMAT, szBuffer );
            TabCtrlItem.pszText = szBuffer;
            SendMessage(hwndTabCtrl, TCM_INSERTITEM, 0, (LPARAM)&TabCtrlItem);
            DocExpControl( pIda->hwndDlg, ID_DOCEXP_INTFORMAT_RB, BN_CLICKED );
            pIda->sExportFormatID = ID_DOCEXP_INTFORMAT_RB;
            ShowWindow( pIda->hwndDlg, SW_SHOW );


          }
          else if (usFormat == ID_DOCEXP_VALFORMAT_RB)
          {
            TabCtrlItem.pszText = "Validation Format";
            SendMessage(hwndTabCtrl, TCM_INSERTITEM, 0, (LPARAM)&TabCtrlItem);
            DocExpControl( pIda->hwndDlg, ID_DOCEXP_VALFORMAT_RB, BN_CLICKED );
            pIda->sExportFormatID = ID_DOCEXP_VALFORMAT_RB;
            ShowWindow( pIda->hwndDlg, SW_SHOW );
          }
          else if (usFormat == ID_DOCEXP_XMLFORMAT_RB)
          {
            TabCtrlItem.pszText = "Plain XML Format";
            SendMessage(hwndTabCtrl, TCM_INSERTITEM, 0, (LPARAM)&TabCtrlItem);
            DocExpControl( pIda->hwndDlg, ID_DOCEXP_XMLFORMAT_RB, BN_CLICKED );
            pIda->sExportFormatID = ID_DOCEXP_XMLFORMAT_RB;
            ShowWindow( pIda->hwndDlg, SW_SHOW );
          } // end if
          TabCtrl_SetCurSel( hwndTabCtrl, 0 );
        }// end if
      }
      else
      {
        switch ( pIda->sExportFormatID )
        {
          case ID_DOCEXP_EXTFORMAT_RB:  // default ...
          case ID_DOCEXP_IMPPATH_RB:
          case ID_DOCEXP_INTFORMAT_RB:
          case ID_DOCEXP_VALFORMAT_RB:
          case ID_DOCEXP_XMLFORMAT_RB:
            mResult = DocExpCommand( pIda->hwndDlg, WMCOMMANDID( mp1, mp2 ), WMCOMMANDCMD( mp1, mp2 ) );
            break;
          default:
            switch ( WMCOMMANDID( mp1, mp2) )
            {
			  case PID_PB_HELP:
			    mResult = UtlInvokeHelp();
			    break;
              case PID_PB_CANCEL :             // CANCEL button selected
              case DID_CANCEL :                 // ESCape key pressed
                if ( pIda->fExporting )
                {
                  if ( UtlError( ERROR_DOCEXP_CANCEL,
                                 MB_YESNO | MB_DEFBUTTON2, 0,
                                 NULL, EQF_QUERY ) == MBID_YES )
                  {
                    pIda->fExporting = FALSE;
                  } /* endif */
                }
                else
                {
                  WinPostMsg( hwnd, WM_EQF_CLOSE, NULL, NULL );
                } /* endif */
                break;
            } /* endswitch */
            break;
        } /* endswitch */
      } // end if  UCD_ONE_PROPERTY

      break;

    case WM_HELP:
      /*************************************************************/
      /* pass on a HELP_WM_HELP request                            */
      /*************************************************************/
      EqfDisplayContextHelp( (HWND)((LPHELPINFO) mp2)->hItemHandle,
                             &hlpsubtblPropSettingsDlg[0] );
      mResult = TRUE;  // message processed
      break;

    case WM_EQF_CLOSE:
      //--- get rid off dialog ---
      pIda = ACCESSDLGIDA(hwnd, PDOCEXPIDA );
      if ( pIda )
      {
        UtlUnregisterModelessDlg(pIda->hwndDlg );
      } /* endif */
      DISMISSDLG( hwnd, mp1 );
      break;

    default :
      mResult = WinDefDlgProc( hwnd, msg, mp1, mp2 );
      break;
  } /* endswitch */

  return( mResult );

} /* end of function FEXPORTDLGPROPPROC */



//f1////////////////////////////////////////////////////////////////////////////
// dialog procedure FEXPORTDLGPROC                                            //
////////////////////////////////////////////////////////////////////////////////
INT_PTR CALLBACK FEXPORTDLGPROC( HWND   hwnd,
                        WINMSG msg,
                        WPARAM mp1,
                        LPARAM mp2 )

{
  PDOCEXPIDA  pIda;                      //instance area for EQFFEXPO
  MRESULT     mResult = FALSE;           //function return value
  ULONG       ulErrorInfo;                //error indicator from PRHA

  switch ( msg )
  {
    case WM_EQF_QUERYID: HANDLEQUERYID( ID_FILEXP_DLG, mp2 ); break;

      //------------------------------------------------------------------------
    case ( WM_INITDLG ) :                 //initialize and display dialogbox
      mResult = DocExpInit( hwnd, mp1, mp2 );
      break;
      //------------------------------------------------------------------------
    case ( WM_EQF_CLOSE ) :
      pIda = ACCESSDLGIDA( hwnd, PDOCEXPIDA );
      if ( pIda != NULL )
      {
        if ( pIda->fExporting )
        {
          if ( UtlError( ERROR_DOCEXP_CANCEL,
                         MB_YESNO | MB_DEFBUTTON2, 0,
                         NULL, EQF_QUERY ) == MBID_YES )
          {
            pIda->fExporting = FALSE;
            pIda->fBeingClosed = TRUE;
          } /* endif */
        }
        else
        {
          if ( pIda->fExportInProcess )
          {
            /* export is still running, do nothing */
          }
          else
          {
            /* export process has completed, close dialog */
            DelCtrlFont(hwnd, ID_DOCEXP_DOCS_LB);
            DelCtrlFont(hwnd, ID_DOCEXP_SOURCEPATH_CB);
            DelCtrlFont(hwnd, ID_DOCEXP_VALFORMAT_PATH_CB );
            if ( pIda->hwndPropertySheet )
            {
              POSTEQFCLOSE( pIda->hwndPropertySheet, SHORT1FROMMP1( mp1 ) );
            } /* endif */
            DISMISSDLG( hwnd, 0 );
          } /* endif */
        } /* endif */
      }
      else
      {
        // destroy dialog
        DelCtrlFont(hwnd, ID_DOCEXP_DOCS_LB);
        DelCtrlFont(hwnd, ID_DOCEXP_SOURCEPATH_CB);
        DelCtrlFont(hwnd, ID_DOCEXP_VALFORMAT_PATH_CB );
        if ( pIda->hwndPropertySheet )
        {
          POSTEQFCLOSE( pIda->hwndPropertySheet, SHORT1FROMMP1( mp1 ) );
        } /* endif */
        DISMISSDLG( hwnd, 0 );
      } /* endif */
      break;
      //------------------------------------------------------------------------
      //------------------------------------------------------------------------
    case ( WM_COMMAND ) :
      mResult = DocExpCommand( hwnd, WMCOMMANDID( mp1, mp2 ),
                               WMCOMMANDCMD( mp1, mp2 ) );
      break;
      //------------------------------------------------------------------------
    case ( WM_DESTROY ) :  /*KIT1155M13*/
      pIda = ACCESSDLGIDA( hwnd, PDOCEXPIDA );
      if ( pIda != NULL )
      {
        // close import/export  properties
        CloseProperties( pIda->IdaHead.hProp, 0, &ulErrorInfo);
        if ( pIda->pRevMark )
        {
          UtlAlloc( (PVOID *)&pIda->pRevMark, 0L, 0L, NOMSG );
        } /* endif */

        // destroy our invisible document listbox
        if ( pIda->hwndExportListbox != NULLHANDLE )
        {
          WinDestroyWindow( pIda->hwndExportListbox );
        } /* endif */
        //free storage of IDA
        UtlAlloc( (PVOID *)&pIda, 0L, 0L, NOMSG );
      } /* endif */
      break;
      //------------------------------------------------------------------------
    case ( WM_EQF_NEXTSTEP ) :
      pIda = ACCESSDLGIDA( hwnd, PDOCEXPIDA );
      if ( pIda != NULL )
      {
        pIda->fSNoMatch = QUERYCHECK( hwnd, ID_DOCEXP_SNOMATCH_CHK );
        pIda->fWithRevMark = QUERYCHECK( hwnd, ID_DOCEXP_REVMARK_CHK );
        pIda->fWithTrackID = QUERYCHECK( hwnd, ID_DOCEXP_TRACKID_CHK );
        pIda->fSource = QUERYCHECK( hwnd, ID_DOCEXP_SOURCE_CHK );
        pIda->fTarget = QUERYCHECK( hwnd, ID_DOCEXP_TARGET_CHK );
        EnableExportControls (hwnd, pIda, TRUE);
      } /* endif */
      break;
      //------------------------------------------------------------------------
    default :
      mResult = WinDefDlgProc( hwnd, msg, mp1, mp2 );
      break;
  } /*endswitch */

  return( mResult );
}/*end FEXPORTDLGPROC*/

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EnableExportControls                                     |
//+----------------------------------------------------------------------------+
//|Function call:     EnableExportControls (hwnd, pIda, TRUE);                 |
//+----------------------------------------------------------------------------+
//|Description:       Enable/disable all controls of the export dialog         |
//+----------------------------------------------------------------------------+
//|Input parameter:   HWND hwnd - window handle of export dialog               |
//|                   PDOCEXPIDA pIda - export ida (needed for some flags)     |
//|                   BOOL fEnable - enable or disable controls                |
//+----------------------------------------------------------------------------+
//|Returncode type:   VOID                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:       None                                                     |
//+----------------------------------------------------------------------------+
//|Prerequesits:      _                                                        |
//+----------------------------------------------------------------------------+
//|Side effects:      _                                                        |
//+----------------------------------------------------------------------------+
//|Samples:           _                                                        |
//+----------------------------------------------------------------------------+
//|Function flow:     _                                                        |
//+----------------------------------------------------------------------------+
static VOID EnableExportControls (HWND hwnd, PDOCEXPIDA pIda, BOOL fEnable)
{

  ENABLECTRL( hwnd, PID_PB_OK, fEnable);
  ENABLECTRL( hwnd, PID_PB_HELP, fEnable);

  ENABLECTRL( hwnd, ID_DOCEXP_FORMAT_GB, fEnable);
  ENABLECTRL( hwnd, ID_DOCEXP_INTFORMAT_RB, fEnable);
  ENABLECTRL( hwnd, ID_DOCEXP_EXTFORMAT_RB, fEnable);
  {
    BOOL fEnableImpPath = pIda->fRelPathInfo && fEnable;
    ENABLECTRL( hwnd, ID_DOCEXP_IMPPATH_RB, fEnableImpPath );
  }

  ENABLECTRL( hwnd, ID_DOCEXP_EXPORT_GB, fEnable);
  ENABLECTRL( hwnd, ID_DOCEXP_DOCS_LB, fEnable);

  ENABLECTRL( hwnd, ID_DOCEXP_TO_GB, fEnable);

  ENABLECTRL( hwnd, ID_DOCEXP_ORIGINAL_PB   , fEnable);
  ENABLECTRL( hwnd, ID_DOCEXP_TRANSLATION_PB, fEnable);
  ENABLECTRL( hwnd, ID_DOCEXP_SNOMATCH_PB   , fEnable);

  if ( !(pIda->sExportFormatID == ID_DOCEXP_INTFORMAT_RB) ||
       (fEnable == FALSE))
  {
    /* enable/disable the following controls only for external format */
    /* (not necessary for external, time can be conserved)            */
    ENABLECTRL( hwnd, ID_DOCEXP_TARGET_CHK, fEnable);
    ENABLECTRL( hwnd, ID_DOCEXP_TARGETPATH_TEXT, fEnable & pIda->fTarget);
    ENABLECTRL( hwnd, ID_DOCEXP_TARGETPATH_CB, fEnable & pIda->fTarget);
    HIDECONTROL( hwnd, ID_DOCEXP_CONV_CB );
    HIDECONTROL( hwnd, ID_DOCEXP_CONV_TEXT );
    ENABLECTRL( hwnd, ID_DOCEXP_REVMARK_CHK, fEnable & pIda->fTarget);
    ENABLECTRL( hwnd, ID_DOCEXP_REVMARKNAME_TEXT,
                fEnable & pIda->fTarget & pIda->fWithRevMark);
    ENABLECTRL( hwnd, ID_DOCEXP_REVMARK_CB,
                fEnable & pIda->fTarget & pIda->fWithRevMark);
    ENABLECTRL( hwnd, ID_DOCEXP_REVEDIT_PB,
                fEnable & pIda->fTarget & pIda->fWithRevMark);
    ENABLECTRL( hwnd, ID_DOCEXP_TRACKID_CHK, fEnable & pIda->fTarget);

    ENABLECTRL( hwnd, ID_DOCEXP_SOURCE_CHK, fEnable);
    ENABLECTRL( hwnd, ID_DOCEXP_SOURCEPATH_TEXT, fEnable & pIda->fSource);
    ENABLECTRL( hwnd, ID_DOCEXP_SOURCEPATH_CB, fEnable & pIda->fSource);

    ENABLECTRL( hwnd, ID_DOCEXP_SNOMATCH_CHK, fEnable & pIda->fSNoMatchExist);
    ENABLECTRL( hwnd, ID_DOCEXP_SNOMATCHPATH_TEXT,
                fEnable & pIda->fSNoMatchExist & pIda->fSNoMatch);
    ENABLECTRL( hwnd, ID_DOCEXP_SNOMATCHPATH_CB,
                fEnable & pIda->fSNoMatchExist & pIda->fSNoMatch);
    ENABLECTRL( hwnd, ID_DOCEXP_SNOMATCH_PB,
                fEnable & pIda->fSNoMatchExist & pIda->fSNoMatch);
  } /* endif */
} /* end of function EnableExportControls */

static MRESULT DocExpControl
(
HWND hwnd,
SHORT  sId,                         // id in action
SHORT  sNotification                // notification
)
{
  PDOCEXPIDA        pIda;                        //instance area for EQFFEXPO
  MRESULT     mResult = FALSE;             //function return value

  pIda = ACCESSDLGIDA( hwnd, PDOCEXPIDA );

  switch ( sId )
  {
    //----------------------------------------------------------
    case ID_DOCEXP_EXTFORMAT_RB:
    case ID_DOCEXP_IMPPATH_RB:
      if ( sNotification == BN_CLICKED )
      {
        HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
        // switch to export in external format
        SHOWCONTROL( hwnd, ID_DOCEXP_TARGETPATH_CB );
        SETTEXTFROMRES( hwnd, ID_DOCEXP_EXPORT_GB, pIda->szString, hResMod, SID_DOCEXP_EXTEXPORT );
        if ( sId ==  ID_DOCEXP_IMPPATH_RB )
        {
          SETTEXTFROMRES( hwnd, ID_DOCEXP_TARGETPATH_TEXT, pIda->szString,
                          hResMod, SID_DOCEXP_STARTPATH_TEXT );
          SETTEXTFROMRES( hwnd, ID_DOCEXP_SOURCEPATH_TEXT, pIda->szString,
                          hResMod, SID_DOCEXP_STARTPATH_TEXT );
        }
        else
        {
          SETTEXTFROMRES( hwnd, ID_DOCEXP_TARGETPATH_TEXT, pIda->szString,
                          hResMod, SID_DOCEXP_PATH_TEXT );
          SETTEXTFROMRES( hwnd, ID_DOCEXP_SOURCEPATH_TEXT, pIda->szString,
                          hResMod, SID_DOCEXP_PATH_TEXT );
        } /* endif */

        SHOWCONTROL( hwnd, ID_DOCEXP_TARGET_CHK );
        SHOWCONTROL( hwnd, ID_DOCEXP_TARGETPATH_TEXT );
        SHOWCONTROL( hwnd, ID_DOCEXP_TARGETPATH_CB );
        SHOWCONTROL( hwnd, ID_DOCEXP_SOURCE_CHK );
        SHOWCONTROL( hwnd, ID_DOCEXP_SOURCEPATH_TEXT );
        SHOWCONTROL( hwnd, ID_DOCEXP_SOURCEPATH_CB );
        {
          SHOWCONTROL( hwnd, ID_DOCEXP_REVMARK_CHK );
          SHOWCONTROL( hwnd, ID_DOCEXP_REVMARKNAME_TEXT );
          SHOWCONTROL( hwnd, ID_DOCEXP_REVMARK_CB );
          SHOWCONTROL( hwnd, ID_DOCEXP_REVEDIT_PB );
          SHOWCONTROL( hwnd, ID_DOCEXP_TRACKID_CHK );
          SHOWCONTROL( hwnd, ID_DOCEXP_SNOMATCH_CHK );
          SHOWCONTROL( hwnd, ID_DOCEXP_SNOMATCHPATH_TEXT );
          SHOWCONTROL( hwnd, ID_DOCEXP_SNOMATCHPATH_CB );
          SHOWCONTROL( hwnd, ID_DOCEXP_SNOMATCH_PB    );
          HIDECONTROL( hwnd, ID_DOCEXP_CONV_CB );
          HIDECONTROL( hwnd, ID_DOCEXP_CONV_TEXT );
        } /* endif */
        SHOWCONTROL( hwnd, ID_DOCEXP_ORIGINAL_PB );

        SHOWCONTROL( hwnd, ID_DOCEXP_TRANSLATION_PB );

        HIDECONTROL( hwnd, ID_DOCEXP_VALFORMAT_BROWSE_PB );
        HIDECONTROL( hwnd, ID_DOCEXP_VALFORMAT_PATH_TEXT );
        HIDECONTROL( hwnd, ID_DOCEXP_VALFORMAT_PATH_CB );
        HIDECONTROL( hwnd, ID_DOCEXP_VALFORMAT_NAMEBROWSE_PB );
        HIDECONTROL( hwnd, ID_DOCEXP_VALFORMAT_NAME_TEXT );
        HIDECONTROL( hwnd, ID_DOCEXP_VALFORMAT_NAME_EF );
        HIDECONTROL( hwnd, ID_DOCEXP_VALFORMAT_FORMAT_TEXT );
        HIDECONTROL( hwnd, ID_DOCEXP_VALFORMAT_FORMAT_CB );
        HIDECONTROL( hwnd, ID_DOCEXP_VALFORMAT_COMBINE_CHK );
        HIDECONTROL( hwnd, ID_DOCEXP_VALFORMAT_TYPE_GB );
        HIDECONTROL( hwnd, ID_DOCEXP_VALFORMAT_VALID_RB );
        HIDECONTROL( hwnd, ID_DOCEXP_VALFORMAT_PROOF_RB );
        HIDECONTROL( hwnd, ID_DOCEXP_VALFORMAT_NEW_CHK );
        HIDECONTROL( hwnd, ID_DOCEXP_VALFORMAT_PROT_CHK );
        HIDECONTROL( hwnd, ID_DOCEXP_VALFORMAT_AUTO_CHK );
		HIDECONTROL( hwnd, ID_DOCEXP_VALFORMAT_MODAUTO_CHK );
        HIDECONTROL( hwnd, ID_DOCEXP_VALFORMAT_MODEXACT_CHK );
        HIDECONTROL( hwnd, ID_DOCEXP_VALFORMAT_NOTTRAN_CHK );
        HIDECONTROL( hwnd, ID_DOCEXP_VALFORMAT_FUZZY_CHK );
        HIDECONTROL( hwnd, ID_DOCEXP_VALFORMAT_EXACT_CHK );
        HIDECONTROL( hwnd, ID_DOCEXP_VALFORMAT_GLOBMEM_CHK );
        HIDECONTROL( hwnd, ID_DOCEXP_VALFORMAT_MACHINE_CHK );
        HIDECONTROL( hwnd, ID_DOCEXP_VALFORMAT_REPLACE_CHK );
        HIDECONTROL( hwnd, ID_DOCEXP_VALFORMAT_MATCH_GB );
        HIDECONTROL( hwnd, ID_DOCEXP_VALFORMAT_SELECTALL_PB );
        HIDECONTROL( hwnd, ID_DOCEXP_VALFORMAT_DESELECTALL_PB );
        HIDECONTROL( hwnd, ID_DOCEXP_VALFORMAT_REMOVETAGGING_CHK );
        HIDECONTROL( hwnd, ID_DOCEXP_VALFORMAT_LINKSIMAGES_CHK );
        HIDECONTROL( hwnd, ID_DOCEXP_VALFORMAT_OPTIONS_GB );
        HIDECONTROL( hwnd, ID_DOCEXP_VALFORMAT_MANUALEXACT_CHK );
        HIDECONTROL( hwnd, ID_DOCEXP_VALFORMAT_TRANSONLY_CHK );

        HIDECONTROL( hwnd, ID_DOCEXP_VALFORMAT_INCLCOUNT_CHK );
        HIDECONTROL( hwnd, ID_DOCEXP_VALFORMAT_INCLEXIST_CHK );
        HIDECONTROL( hwnd, ID_DOCEXP_VALFORMAT_MISMATCH_CHK );
        HIDECONTROL( hwnd, ID_DOCEXP_VALFORMAT_ADDINFO_GB );

        HIDECONTROL( hwnd, ID_DOCEXP_VALFORMAT_PROJINFO_GB );
        HIDECONTROL( hwnd, ID_DOCEXP_VALFORMAT_TRANSLATOR_TEXT );
        HIDECONTROL( hwnd, ID_DOCEXP_VALFORMAT_TRANSLATOR_CB );
        HIDECONTROL( hwnd, ID_DOCEXP_VALFORMAT_MANAGER_TEXT );
        HIDECONTROL( hwnd, ID_DOCEXP_VALFORMAT_MANAGER_CB );

        pIda->sExportFormatID = sId;

        QUERYTEXT( hwnd, ID_DOCEXP_TARGETPATH_CB, pIda->szString );
        if ( strlen(pIda->szString) <= 3 ) {       /* 5-23-14  P402115 */
           strcpy(pIda->szString,pIda->szSelectedTransDrive);
           {
             PPROPIMEX   ppropImex;                   // handle of IMEX properties
             ppropImex = (PPROPIMEX)MakePropPtrFromHnd( pIda->IdaHead.hProp);
             strcat(pIda->szString, ppropImex->szSavedDlgFExpoTPath);
           }
           
           SETTEXT( hwnd, ID_DOCEXP_TARGETPATH_CB, pIda->szString);
        }
        UtlLoadLastUsedStrings( hwnd, ID_DOCEXP_TARGETPATH_CB, DOCEXPEXTLASTUSED );

        WinPostMsg (hwnd, WM_EQF_NEXTSTEP, NULL, NULL);
      } /* endif */
      break;
      //----------------------------------------------------------
    case ID_DOCEXP_INTFORMAT_RB:
      if ( sNotification == BN_CLICKED )
      {
        HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);

        // switch to export in internal format
        SETTEXTFROMRES( hwnd, ID_DOCEXP_EXPORT_GB, pIda->szString, hResMod, SID_DOCEXP_INTEXPORT );
        SHOWCONTROL( hwnd, ID_DOCEXP_TARGETPATH_CB );

        HIDECONTROL( hwnd, ID_DOCEXP_TARGET_CHK );

        HIDECONTROL( hwnd, ID_DOCEXP_SOURCE_CHK );
        HIDECONTROL( hwnd, ID_DOCEXP_SOURCEPATH_TEXT );
        HIDECONTROL( hwnd, ID_DOCEXP_SOURCEPATH_CB );
        HIDECONTROL( hwnd, ID_DOCEXP_REVMARK_CHK );
        HIDECONTROL( hwnd, ID_DOCEXP_REVMARKNAME_TEXT );
        HIDECONTROL( hwnd, ID_DOCEXP_REVMARK_CB );
        HIDECONTROL( hwnd, ID_DOCEXP_REVEDIT_PB );
        HIDECONTROL( hwnd, ID_DOCEXP_TRACKID_CHK );
        HIDECONTROL( hwnd, ID_DOCEXP_SNOMATCH_CHK );
        HIDECONTROL( hwnd, ID_DOCEXP_SNOMATCHPATH_TEXT );
        HIDECONTROL( hwnd, ID_DOCEXP_SNOMATCHPATH_CB );
        HIDECONTROL( hwnd, ID_DOCEXP_CONV_CB );
        HIDECONTROL( hwnd, ID_DOCEXP_CONV_TEXT );

        HIDECONTROL( hwnd, ID_DOCEXP_ORIGINAL_PB );
        HIDECONTROL( hwnd, ID_DOCEXP_SNOMATCH_PB    );

        SHOWCONTROL( hwnd, ID_DOCEXP_TRANSLATION_PB );
        HIDECONTROL( hwnd, ID_DOCEXP_VALFORMAT_BROWSE_PB );
        HIDECONTROL( hwnd, ID_DOCEXP_VALFORMAT_PATH_TEXT );
        HIDECONTROL( hwnd, ID_DOCEXP_VALFORMAT_PATH_CB );
        HIDECONTROL( hwnd, ID_DOCEXP_VALFORMAT_NAMEBROWSE_PB );
        HIDECONTROL( hwnd, ID_DOCEXP_VALFORMAT_NAME_TEXT );
        HIDECONTROL( hwnd, ID_DOCEXP_VALFORMAT_NAME_EF );
        HIDECONTROL( hwnd, ID_DOCEXP_VALFORMAT_FORMAT_TEXT );
        HIDECONTROL( hwnd, ID_DOCEXP_VALFORMAT_FORMAT_CB );
        HIDECONTROL( hwnd, ID_DOCEXP_VALFORMAT_COMBINE_CHK );
        HIDECONTROL( hwnd, ID_DOCEXP_VALFORMAT_TYPE_GB );
        HIDECONTROL( hwnd, ID_DOCEXP_VALFORMAT_VALID_RB );
        HIDECONTROL( hwnd, ID_DOCEXP_VALFORMAT_PROOF_RB );
        HIDECONTROL( hwnd, ID_DOCEXP_VALFORMAT_NEW_CHK );
        HIDECONTROL( hwnd, ID_DOCEXP_VALFORMAT_PROT_CHK );
        HIDECONTROL( hwnd, ID_DOCEXP_VALFORMAT_AUTO_CHK );
		HIDECONTROL( hwnd, ID_DOCEXP_VALFORMAT_MODAUTO_CHK );
        HIDECONTROL( hwnd, ID_DOCEXP_VALFORMAT_MODEXACT_CHK );
        HIDECONTROL( hwnd, ID_DOCEXP_VALFORMAT_NOTTRAN_CHK );
        HIDECONTROL( hwnd, ID_DOCEXP_VALFORMAT_FUZZY_CHK );
        HIDECONTROL( hwnd, ID_DOCEXP_VALFORMAT_EXACT_CHK );
        HIDECONTROL( hwnd, ID_DOCEXP_VALFORMAT_GLOBMEM_CHK );
        HIDECONTROL( hwnd, ID_DOCEXP_VALFORMAT_MACHINE_CHK );
        HIDECONTROL( hwnd, ID_DOCEXP_VALFORMAT_REPLACE_CHK );
        HIDECONTROL( hwnd, ID_DOCEXP_VALFORMAT_MATCH_GB );
        HIDECONTROL( hwnd, ID_DOCEXP_VALFORMAT_SELECTALL_PB );
        HIDECONTROL( hwnd, ID_DOCEXP_VALFORMAT_DESELECTALL_PB );
        HIDECONTROL( hwnd, ID_DOCEXP_VALFORMAT_REMOVETAGGING_CHK );
        HIDECONTROL( hwnd, ID_DOCEXP_VALFORMAT_LINKSIMAGES_CHK );
        HIDECONTROL( hwnd, ID_DOCEXP_VALFORMAT_OPTIONS_GB );
        HIDECONTROL( hwnd, ID_DOCEXP_VALFORMAT_MANUALEXACT_CHK );
        HIDECONTROL( hwnd, ID_DOCEXP_VALFORMAT_TRANSONLY_CHK );

        HIDECONTROL( hwnd, ID_DOCEXP_VALFORMAT_INCLCOUNT_CHK );
        HIDECONTROL( hwnd, ID_DOCEXP_VALFORMAT_INCLEXIST_CHK );
        HIDECONTROL( hwnd, ID_DOCEXP_VALFORMAT_MISMATCH_CHK );
        HIDECONTROL( hwnd, ID_DOCEXP_VALFORMAT_ADDINFO_GB );
        HIDECONTROL( hwnd, ID_DOCEXP_VALFORMAT_PROJINFO_GB );
        HIDECONTROL( hwnd, ID_DOCEXP_VALFORMAT_TRANSLATOR_TEXT );
        HIDECONTROL( hwnd, ID_DOCEXP_VALFORMAT_TRANSLATOR_CB );
        HIDECONTROL( hwnd, ID_DOCEXP_VALFORMAT_MANAGER_TEXT );
        HIDECONTROL( hwnd, ID_DOCEXP_VALFORMAT_MANAGER_CB );


        SHOWCONTROL( hwnd, ID_DOCEXP_TARGETPATH_TEXT );
        ENABLECTRL( hwnd, ID_DOCEXP_TARGETPATH_TEXT,TRUE);
        SETTEXT( hwnd, ID_DOCEXP_TARGETPATH_TEXT, "Drive" );

        pIda->sExportFormatID = sId;
        ENABLECTRL( hwnd, ID_DOCEXP_TARGETPATH_CB,TRUE);
        strcpy(pIda->szString,pIda->szSelectedTransDrive);
        SETTEXT( hwnd, ID_DOCEXP_TARGETPATH_CB, pIda->szString);
        UtlLoadLastUsedStrings( hwnd, ID_DOCEXP_TARGETPATH_CB, DOCEXPINTLASTUSED );


        WinPostMsg (hwnd, WM_EQF_NEXTSTEP, NULL, NULL);
      } /* endif */
      break;
      //----------------------------------------------------------
    case ID_DOCEXP_VALFORMAT_RB:
      if ( sNotification == BN_CLICKED )
      {
        HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);

        // switch to export in validation format
        SETTEXTFROMRES( hwnd, ID_DOCEXP_EXPORT_GB, pIda->szString, hResMod, SID_DOCEXP_INTEXPORT );

        HIDECONTROL( hwnd, ID_DOCEXP_TARGETPATH_CB );
        HIDECONTROL( hwnd, ID_DOCEXP_TARGET_CHK );
        HIDECONTROL( hwnd, ID_DOCEXP_SOURCE_CHK );
        HIDECONTROL( hwnd, ID_DOCEXP_SOURCEPATH_TEXT );
        HIDECONTROL( hwnd, ID_DOCEXP_SOURCEPATH_CB );
        HIDECONTROL( hwnd, ID_DOCEXP_REVMARK_CHK );
        HIDECONTROL( hwnd, ID_DOCEXP_REVMARKNAME_TEXT );
        HIDECONTROL( hwnd, ID_DOCEXP_REVMARK_CB );
        HIDECONTROL( hwnd, ID_DOCEXP_REVEDIT_PB );
        HIDECONTROL( hwnd, ID_DOCEXP_TRACKID_CHK );
        HIDECONTROL( hwnd, ID_DOCEXP_SNOMATCH_CHK );
        HIDECONTROL( hwnd, ID_DOCEXP_SNOMATCHPATH_TEXT );
        HIDECONTROL( hwnd, ID_DOCEXP_SNOMATCHPATH_CB );
        HIDECONTROL( hwnd, ID_DOCEXP_CONV_CB );
        HIDECONTROL( hwnd, ID_DOCEXP_CONV_TEXT );
        HIDECONTROL( hwnd, ID_DOCEXP_ORIGINAL_PB );
        HIDECONTROL( hwnd, ID_DOCEXP_SNOMATCH_PB    );
        HIDECONTROL( hwnd, ID_DOCEXP_TARGETPATH_TEXT );
        HIDECONTROL( hwnd, ID_DOCEXP_TRANSLATION_PB );

        SHOWCONTROL( hwnd, ID_DOCEXP_VALFORMAT_BROWSE_PB );
        SHOWCONTROL( hwnd, ID_DOCEXP_VALFORMAT_PATH_TEXT );
        SHOWCONTROL( hwnd, ID_DOCEXP_VALFORMAT_PATH_CB );
        SHOWCONTROL( hwnd, ID_DOCEXP_VALFORMAT_NAMEBROWSE_PB );
        SHOWCONTROL( hwnd, ID_DOCEXP_VALFORMAT_NAME_TEXT );
        SHOWCONTROL( hwnd, ID_DOCEXP_VALFORMAT_NAME_EF );
        SHOWCONTROL( hwnd, ID_DOCEXP_VALFORMAT_FORMAT_TEXT );
        SHOWCONTROL( hwnd, ID_DOCEXP_VALFORMAT_FORMAT_CB );
        SHOWCONTROL( hwnd, ID_DOCEXP_VALFORMAT_COMBINE_CHK );
        SHOWCONTROL( hwnd, ID_DOCEXP_VALFORMAT_TYPE_GB );
        SHOWCONTROL( hwnd, ID_DOCEXP_VALFORMAT_VALID_RB );
        SHOWCONTROL( hwnd, ID_DOCEXP_VALFORMAT_PROOF_RB );
        SHOWCONTROL( hwnd, ID_DOCEXP_VALFORMAT_NEW_CHK );
        SHOWCONTROL( hwnd, ID_DOCEXP_VALFORMAT_PROT_CHK );
        SHOWCONTROL( hwnd, ID_DOCEXP_VALFORMAT_AUTO_CHK );
	    SHOWCONTROL( hwnd, ID_DOCEXP_VALFORMAT_MODAUTO_CHK );
        SHOWCONTROL( hwnd, ID_DOCEXP_VALFORMAT_MODEXACT_CHK );
        SHOWCONTROL( hwnd, ID_DOCEXP_VALFORMAT_NOTTRAN_CHK );
        SHOWCONTROL( hwnd, ID_DOCEXP_VALFORMAT_FUZZY_CHK );
        SHOWCONTROL( hwnd, ID_DOCEXP_VALFORMAT_EXACT_CHK );
        SHOWCONTROL( hwnd, ID_DOCEXP_VALFORMAT_GLOBMEM_CHK );
        SHOWCONTROL( hwnd, ID_DOCEXP_VALFORMAT_MACHINE_CHK );
        SHOWCONTROL( hwnd, ID_DOCEXP_VALFORMAT_REPLACE_CHK );
        SHOWCONTROL( hwnd, ID_DOCEXP_VALFORMAT_MATCH_GB );
        SHOWCONTROL( hwnd, ID_DOCEXP_VALFORMAT_SELECTALL_PB );
        SHOWCONTROL( hwnd, ID_DOCEXP_VALFORMAT_DESELECTALL_PB );
        SHOWCONTROL( hwnd, ID_DOCEXP_VALFORMAT_REMOVETAGGING_CHK );
        SHOWCONTROL( hwnd, ID_DOCEXP_VALFORMAT_LINKSIMAGES_CHK );
        SHOWCONTROL( hwnd, ID_DOCEXP_VALFORMAT_OPTIONS_GB );
        SHOWCONTROL( hwnd, ID_DOCEXP_VALFORMAT_MANUALEXACT_CHK );
        SHOWCONTROL( hwnd, ID_DOCEXP_VALFORMAT_TRANSONLY_CHK );

        SHOWCONTROL( hwnd, ID_DOCEXP_VALFORMAT_INCLCOUNT_CHK );
        SHOWCONTROL( hwnd, ID_DOCEXP_VALFORMAT_INCLEXIST_CHK );
        SHOWCONTROL( hwnd, ID_DOCEXP_VALFORMAT_MISMATCH_CHK );
        SHOWCONTROL( hwnd, ID_DOCEXP_VALFORMAT_ADDINFO_GB );
        SHOWCONTROL( hwnd, ID_DOCEXP_VALFORMAT_PROJINFO_GB );
        SHOWCONTROL( hwnd, ID_DOCEXP_VALFORMAT_TRANSLATOR_TEXT );
        SHOWCONTROL( hwnd, ID_DOCEXP_VALFORMAT_TRANSLATOR_CB );
        SHOWCONTROL( hwnd, ID_DOCEXP_VALFORMAT_MANAGER_TEXT );
        SHOWCONTROL( hwnd, ID_DOCEXP_VALFORMAT_MANAGER_CB );
		

        pIda->sValFormat = CBQUERYSELECTION( hwnd, ID_DOCEXP_VALFORMAT_FORMAT_CB );
        ENABLECTRL( hwnd, ID_DOCEXP_VALFORMAT_LINKSIMAGES_CHK, 
           (((VALFORMATID)pIda->sValFormat == HTML_VALEXPFORMAT) || 
            ((VALFORMATID)pIda->sValFormat == ODT_VALEXPFORMAT) || 
            ((VALFORMATID)pIda->sValFormat == DOC_VALEXPFORMAT) ||
            ((VALFORMATID)pIda->sValFormat == DOCX_VALEXPFORMAT)) );
        UtlLoadLastUsedStrings( hwnd, ID_DOCEXP_VALFORMAT_PATH_CB, DOCEXPVALLASTUSED );

        pIda->sExportFormatID = sId;

        WinPostMsg (hwnd, WM_EQF_NEXTSTEP, NULL, NULL);
      } /* endif */
      break;
      //----------------------------------------------------------
    case ID_DOCEXP_XMLFORMAT_RB:
      if ( sNotification == BN_CLICKED )
      {
        HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);

        // switch to export in plain XML format
        SETTEXTFROMRES( hwnd, ID_DOCEXP_EXPORT_GB, pIda->szString, hResMod, SID_DOCEXP_INTEXPORT );

        HIDECONTROL( hwnd, ID_DOCEXP_TARGETPATH_CB );
        HIDECONTROL( hwnd, ID_DOCEXP_TARGET_CHK );
        HIDECONTROL( hwnd, ID_DOCEXP_SOURCE_CHK );
        HIDECONTROL( hwnd, ID_DOCEXP_SOURCEPATH_TEXT );
        HIDECONTROL( hwnd, ID_DOCEXP_SOURCEPATH_CB );
        HIDECONTROL( hwnd, ID_DOCEXP_REVMARK_CHK );
        HIDECONTROL( hwnd, ID_DOCEXP_REVMARKNAME_TEXT );
        HIDECONTROL( hwnd, ID_DOCEXP_REVMARK_CB );
        HIDECONTROL( hwnd, ID_DOCEXP_REVEDIT_PB );
        HIDECONTROL( hwnd, ID_DOCEXP_TRACKID_CHK );
        HIDECONTROL( hwnd, ID_DOCEXP_SNOMATCH_CHK );
        HIDECONTROL( hwnd, ID_DOCEXP_SNOMATCHPATH_TEXT );
        HIDECONTROL( hwnd, ID_DOCEXP_SNOMATCHPATH_CB );
        HIDECONTROL( hwnd, ID_DOCEXP_CONV_CB );
        HIDECONTROL( hwnd, ID_DOCEXP_CONV_TEXT );
        HIDECONTROL( hwnd, ID_DOCEXP_ORIGINAL_PB );
        HIDECONTROL( hwnd, ID_DOCEXP_SNOMATCH_PB    );
        HIDECONTROL( hwnd, ID_DOCEXP_TARGETPATH_TEXT );
        HIDECONTROL( hwnd, ID_DOCEXP_TRANSLATION_PB );

        SHOWCONTROL( hwnd, ID_DOCEXP_VALFORMAT_BROWSE_PB );
        SHOWCONTROL( hwnd, ID_DOCEXP_VALFORMAT_PATH_TEXT );
        SHOWCONTROL( hwnd, ID_DOCEXP_VALFORMAT_PATH_CB );
        SHOWCONTROL( hwnd, ID_DOCEXP_VALFORMAT_NAMEBROWSE_PB );
        SHOWCONTROL( hwnd, ID_DOCEXP_VALFORMAT_NAME_TEXT );
        SHOWCONTROL( hwnd, ID_DOCEXP_VALFORMAT_NAME_EF );
        HIDECONTROL( hwnd, ID_DOCEXP_VALFORMAT_FORMAT_TEXT );
        HIDECONTROL( hwnd, ID_DOCEXP_VALFORMAT_FORMAT_CB );
        SHOWCONTROL( hwnd, ID_DOCEXP_VALFORMAT_COMBINE_CHK );
        HIDECONTROL( hwnd, ID_DOCEXP_VALFORMAT_TYPE_GB );
        HIDECONTROL( hwnd, ID_DOCEXP_VALFORMAT_VALID_RB );
        HIDECONTROL( hwnd, ID_DOCEXP_VALFORMAT_PROOF_RB );
        HIDECONTROL( hwnd, ID_DOCEXP_VALFORMAT_NEW_CHK );
        HIDECONTROL( hwnd, ID_DOCEXP_VALFORMAT_PROT_CHK );
        HIDECONTROL( hwnd, ID_DOCEXP_VALFORMAT_AUTO_CHK );
    	HIDECONTROL( hwnd, ID_DOCEXP_VALFORMAT_MODAUTO_CHK );
        HIDECONTROL( hwnd, ID_DOCEXP_VALFORMAT_MODEXACT_CHK );
        HIDECONTROL( hwnd, ID_DOCEXP_VALFORMAT_NOTTRAN_CHK );
        HIDECONTROL( hwnd, ID_DOCEXP_VALFORMAT_FUZZY_CHK );
        HIDECONTROL( hwnd, ID_DOCEXP_VALFORMAT_EXACT_CHK );
        HIDECONTROL( hwnd, ID_DOCEXP_VALFORMAT_GLOBMEM_CHK );
        HIDECONTROL( hwnd, ID_DOCEXP_VALFORMAT_MACHINE_CHK );
        HIDECONTROL( hwnd, ID_DOCEXP_VALFORMAT_REPLACE_CHK );
        HIDECONTROL( hwnd, ID_DOCEXP_VALFORMAT_MATCH_GB );
        HIDECONTROL( hwnd, ID_DOCEXP_VALFORMAT_SELECTALL_PB );
        HIDECONTROL( hwnd, ID_DOCEXP_VALFORMAT_DESELECTALL_PB );
        HIDECONTROL( hwnd, ID_DOCEXP_VALFORMAT_REMOVETAGGING_CHK );
        HIDECONTROL( hwnd, ID_DOCEXP_VALFORMAT_LINKSIMAGES_CHK );
        HIDECONTROL( hwnd, ID_DOCEXP_VALFORMAT_OPTIONS_GB );
        HIDECONTROL( hwnd, ID_DOCEXP_VALFORMAT_MANUALEXACT_CHK );
        HIDECONTROL( hwnd, ID_DOCEXP_VALFORMAT_TRANSONLY_CHK );

        HIDECONTROL( hwnd, ID_DOCEXP_VALFORMAT_INCLCOUNT_CHK );
        HIDECONTROL( hwnd, ID_DOCEXP_VALFORMAT_INCLEXIST_CHK );
        HIDECONTROL( hwnd, ID_DOCEXP_VALFORMAT_MISMATCH_CHK );
        HIDECONTROL( hwnd, ID_DOCEXP_VALFORMAT_ADDINFO_GB );
        HIDECONTROL( hwnd, ID_DOCEXP_VALFORMAT_PROJINFO_GB );
        HIDECONTROL( hwnd, ID_DOCEXP_VALFORMAT_TRANSLATOR_TEXT );
        HIDECONTROL( hwnd, ID_DOCEXP_VALFORMAT_TRANSLATOR_CB );
        HIDECONTROL( hwnd, ID_DOCEXP_VALFORMAT_MANAGER_TEXT );
        HIDECONTROL( hwnd, ID_DOCEXP_VALFORMAT_MANAGER_CB );
		
        UtlLoadLastUsedStrings( hwnd, ID_DOCEXP_VALFORMAT_PATH_CB, DOCEXPXMLLASTUSED );

        pIda->sExportFormatID = sId;

        WinPostMsg (hwnd, WM_EQF_NEXTSTEP, NULL, NULL);
      } /* endif */
      break;
      //----------------------------------------------------------
    case ID_DOCEXP_DOCS_LB:    //selection in Export listbox
      if ( sNotification == LN_SELECT)
      {
        DESELECTALL( hwnd, ID_DOCEXP_DOCS_LB );
      } /* endif */
      break;
      //----------------------------------------------------------
    case ID_DOCEXP_SNOMATCH_CHK:    //click of SNOMATCH checkbox
    case ID_DOCEXP_REVMARK_CHK:     //click of revision mark checkbox
    case ID_DOCEXP_TRACKID_CHK:     //click of TVT tracking ID checkbox
    case ID_DOCEXP_SOURCE_CHK:      //click of SOURCE checkbox
    case ID_DOCEXP_TARGET_CHK:      //click of TARGET checkbox
      if ( (sNotification == BN_CLICKED) ||
           (sNotification == BN_DOUBLECLICKED) )
      {
        WinPostMsg (hwnd, WM_EQF_NEXTSTEP, NULL, NULL);
      } /* endif */
      break;
      //----------------------------------------------------------
    default:
      // check for drive buttons


      break;
  } /* endswitch */
  return( mResult );
}

static MRESULT DocExpCommand
(
HWND   hwnd,
SHORT sId,                          // id of button
SHORT sNotification                 // notification type
)
{
  PDOCEXPIDA  pIda;                        //instance area for EQFFEXPO
  PSZ         pszReplace;                  //pointer to replace string
  SHORT       sIndexItem;                  //index of selected item in listb.
  SHORT       sItemCount;                  //number of items in listbox
  BOOL        fOK = TRUE;                  //boolean flag
  BOOL        fSaveRc;                     // boolean return code for funct.
  USHORT      usRc;                        // USHORT return code
  SHORT       sRc;                         // SHORT return code
  ULONG       ulErrorInfo;                 // error indicator from PRHA
  PPROPIMEX   ppropImex;                   // handle of IMEX properties
  MRESULT     mResult = MRFROMSHORT(TRUE); // result of message processing
  PSZ         pszTemp;                     // temporary pointer
  SHORT       sFocusID;                    // ID of control to receive focus

  switch ( sId )             //switch on control ID
  {
    case PID_PB_HELP:
      UtlInvokeHelp();
      break;


    //-------------Browse Buttons----------------------------------------
    case ID_DOCEXP_TRANSLATION_PB:
      pIda = ACCESSDLGIDA( hwnd, PDOCEXPIDA );
      QUERYTEXT( hwnd, ID_DOCEXP_TARGETPATH_CB, pIda->szString );
      if ( DocExpBrowse( hwnd, pIda->szString, "Select Export Translation Directory" ) )
      {
        if ( pIda->sExportFormatID == ID_DOCEXP_INTFORMAT_RB )
        {
          pIda->szString[2] = EOS;
        } //end if

        SETTEXT( hwnd, ID_DOCEXP_TARGETPATH_CB, pIda->szString );
      } /* endif */
      break;

    case ID_DOCEXP_ORIGINAL_PB:
      pIda = ACCESSDLGIDA( hwnd, PDOCEXPIDA );
      QUERYTEXT( hwnd, ID_DOCEXP_SOURCEPATH_CB, pIda->szString );
      if ( DocExpBrowse( hwnd, pIda->szString, "Select Export Original Directory" ) )
      {
        SETTEXT( hwnd, ID_DOCEXP_SOURCEPATH_CB, pIda->szString );
      } /* endif */
      break;

    case ID_DOCEXP_SNOMATCH_PB:
      pIda = ACCESSDLGIDA( hwnd, PDOCEXPIDA );
      QUERYTEXT( hwnd, ID_DOCEXP_SNOMATCHPATH_CB, pIda->szString );
      if ( DocExpBrowse( hwnd, pIda->szString, "Select Export SNomatch Directory" ) )
      {
        SETTEXT( hwnd, ID_DOCEXP_SNOMATCHPATH_CB, pIda->szString );
      } /* endif */
      break;

    case ID_DOCEXP_VALFORMAT_BROWSE_PB:
      pIda = ACCESSDLGIDA( hwnd, PDOCEXPIDA );
      QUERYTEXT( hwnd, ID_DOCEXP_VALFORMAT_PATH_CB, pIda->szString );
      if ( DocExpBrowse( hwnd, pIda->szString, "Select Validation Format Export Directory" ) )
      {
        SETTEXT( hwnd, ID_DOCEXP_VALFORMAT_PATH_CB, pIda->szString );
      } /* endif */
      break;

    case ID_DOCEXP_VALFORMAT_COMBINE_CHK:
      {
        BOOL fEnable = FALSE;

        pIda = ACCESSDLGIDA( hwnd, PDOCEXPIDA );
        QUERYTEXT( hwnd, ID_DOCEXP_VALFORMAT_PATH_CB, pIda->szString );
        fEnable = ( (QUERYITEMCOUNT( hwnd, ID_DOCEXP_DOCS_LB ) <= 1) || QUERYCHECK( hwnd, ID_DOCEXP_VALFORMAT_COMBINE_CHK ) );
        ENABLECTRL( hwnd, ID_DOCEXP_VALFORMAT_NAME_TEXT, fEnable );
        ENABLECTRL( hwnd, ID_DOCEXP_VALFORMAT_NAME_EF, fEnable );
        ENABLECTRL( hwnd, ID_DOCEXP_VALFORMAT_NAMEBROWSE_PB, fEnable );
      }
      break;

    case ID_DOCEXP_VALFORMAT_NAMEBROWSE_PB:
      {
        OPENFILENAME ofn;
        BOOL fOK = TRUE;
        CHAR szPath[MAX_LONGFILESPEC];

        // access IDa and get current path and name
        pIda = ACCESSDLGIDA( hwnd, PDOCEXPIDA );
        QUERYTEXT( hwnd, ID_DOCEXP_VALFORMAT_PATH_CB, szPath );
        if ( szPath[0] != '\0' )
        {
          int iLen = strlen(szPath);
          if ( szPath[iLen-1] == BACKSLASH )
          {
            szPath[iLen-1] = EOS;
          } /* endif */
        } /* endif */

        memset( &ofn, 0, sizeof(ofn) );
        pIda->szString[0]               = EOS;
        ofn.lStructSize                 = sizeof(ofn);
        ofn.hInstance                   = (HINSTANCE)UtlQueryULong(QL_HAB);
        ofn.lpstrFilter                 = NULL;
        ofn.lpstrCustomFilter           = NULL;
        ofn.nMaxCustFilter              = 0;
        ofn.lpstrFileTitle              = NULL; 
        ofn.nMaxFileTitle               = 0;
        ofn.lpstrInitialDir             = NULL;
        ofn.lpstrTitle                  = "Select validation document file" ;
        ofn.nFileOffset                 = 0;
        ofn.nFileExtension              = 0;
        ofn.lCustData                   = 0L;
        ofn.lpfnHook                    = NULL;
        ofn.lpTemplateName              = NULL;
        ofn.hwndOwner                   = hwnd;
        ofn.lpstrFile                   = pIda->szString;
        ofn.nMaxFile                    = sizeof(pIda->szString);
        ofn.Flags                       = OFN_HIDEREADONLY | OFN_NONETWORKBUTTON ;
        ofn.lpstrDefExt                 = NULL;
        ofn.lpstrInitialDir             = szPath; 

        fOK = GetSaveFileName( &ofn );

        if ( fOK )
        {
          PSZ pszName = UtlSplitFnameFromPath( pIda->szString );
          SETTEXT( hwnd, ID_DOCEXP_VALFORMAT_PATH_CB, pIda->szString );
          SETTEXT( hwnd, ID_DOCEXP_VALFORMAT_NAME_EF, pszName );
        } /* endif */
      }
      break;

    case ( PID_PB_CANCEL ) :              //CANCEL button selected
    case ( DID_CANCEL ) :                 //ESC key pressed
      pIda = ACCESSDLGIDA( hwnd, PDOCEXPIDA );

      if ( pIda->fExporting )
      {
        if ( UtlError( ERROR_DOCEXP_CANCEL, MB_YESNO | MB_DEFBUTTON2, 0, NULL, EQF_QUERY ) == MBID_YES )
        {
          pIda->fExporting = FALSE;
        } /* endif */
      }
      else
      {
        fOK = FALSE;
        WinPostMsg( hwnd, WM_EQF_CLOSE, NULL, NULL );
      } /* endif */
      break;
      //------------------------------------------------------------------
    case ( ID_DOCEXP_REVEDIT_PB ) :       //revision mark edit PB

      pIda = ACCESSDLGIDA( hwnd, PDOCEXPIDA );

      QUERYTEXT( hwnd, ID_DOCEXP_REVMARK_CB, pIda->szString );
      UtlStripBlanks( pIda->szString );

      pszTemp = pIda->szString;
      while ( *pszTemp && isalnum(*pszTemp) )
      {
        pszTemp++;
      } /* endwhile */
      if ( *pszTemp )
      {
        pszTemp = pIda->szString;
        UtlErrorHwnd( ERROR_REVMARK_NAME_INVALID, MB_CANCEL,
                      1, &pszTemp, EQF_ERROR, hwnd );
        SETFOCUS( hwnd, ID_DOCEXP_REVMARK_CB );
      }
      else
      {
        HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
        DIALOGBOX( hwnd, REVMARKDLGPROC, hResMod, ID_REVMARK_DLG, pIda, fOK );

        if ( fOK )
        {
          /***********************************************************/
          /* Update combobox                                         */
          /***********************************************************/
          CBDELETEALL( hwnd, ID_DOCEXP_REVMARK_CB );
          UtlMakeEQFPath( pIda->szSource, NULC, TABLE_PATH, NULL );
          strcat( pIda->szSource, BACKSLASH_STR );
          strcat( pIda->szSource, DEFAULT_PATTERN_NAME );
          strcat( pIda->szSource, EXT_OF_REVISION_MARK );
          UtlLoadFileNames( pIda->szSource,
                            FILE_NORMAL,
                            WinWindowFromID( hwnd, ID_DOCEXP_REVMARK_CB ),
                            NAMFMT_NODRV | NAMFMT_NOEXT );

          /***********************************************************/
          /* Set revision mark name to currently saved revision mark */
          /***********************************************************/
          SETTEXT( hwnd, ID_DOCEXP_REVMARK_CB, pIda->szString );

        } /* endif */
      } /* endif */
      break;
      //------------------------------------------------------------------
    case ( PID_PB_OK ) :                  //OK button selected

      pIda = ACCESSDLGIDA( hwnd, PDOCEXPIDA );
      sFocusID = 0;

      /* set flag that export process is running */             /*KIT0891A*/
      INFOEVENT2( DOCEXPCOMMAND_LOC, STATE_EVENT, 0, DOC_GROUP, NULL );
      pIda->fExporting = TRUE;                                  /*KIT0891A*/
      pIda->fExportInProcess = TRUE;                            /*KIT0891A*/
      pIda->fOverwrite = FALSE;

      // disable controls
      EnableExportControls (hwnd, pIda, FALSE);

      if ( (pIda->sExportFormatID == ID_DOCEXP_VALFORMAT_RB) || (pIda->sExportFormatID == ID_DOCEXP_XMLFORMAT_RB) )
      {
        QUERYTEXT( hwnd, ID_DOCEXP_VALFORMAT_PATH_CB, pIda->szPathContent );
        if (pIda->szPathContent[strlen(pIda->szPathContent)-1] != '\\' && strlen(pIda->szPathContent) > 1)
        {
          strcat(pIda->szPathContent, "\\");
        } //end if
        fOK = UtlCheckPath( pIda->szPathContent, 0L, NULL  );
        if ( !fOK )
        {
          sFocusID = ID_DOCEXP_VALFORMAT_PATH_CB;
          UtlError( WRONG_EXPORTDIR, MB_CANCEL, 0, NULL, EQF_ERROR );
        }
        else
        {
          strcpy( pIda->szValDir, pIda->szPathContent );
        } /* endif */

        if ( pIda->sExportFormatID == ID_DOCEXP_XMLFORMAT_RB )
        {
          pIda->sValFormat = PLAINXML_VALEXPFORMAT;
          pIda->fValExportCombine = QUERYCHECK( hwnd, ID_DOCEXP_VALFORMAT_COMBINE_CHK );
          pIda->fValExportValFormat   = TRUE;
          pIda->fValExportProofFormat = FALSE;
          pIda->fValExportRemoveTagging = FALSE;
          pIda->fValExportLinksImages = FALSE;
          pIda->fValExportInclCount    = TRUE;
          pIda->fValExportInclExisting = TRUE;
          pIda->fValExportMismatchOnly = FALSE;

          if ( (QUERYITEMCOUNT( hwnd, ID_DOCEXP_DOCS_LB ) <= 1) || QUERYCHECK( hwnd, ID_DOCEXP_VALFORMAT_COMBINE_CHK ) )
          {
            QUERYTEXT( hwnd, ID_DOCEXP_VALFORMAT_NAME_EF, pIda->szValFormatOutputName );
          }
          else
          {
            pIda->szValFormatOutputName[0] = EOS;
          } /* endif */

          pIda->fValExportNewMatch    = TRUE;
          pIda->fValExportProtMatch   = TRUE;
          pIda->fValExportAutoMatch   = TRUE;
          pIda->fValExportModAutoMatch  = TRUE;
          pIda->fValExportNotTransl   = TRUE;
          pIda->fValExportFuzzyMatch  = TRUE;
          pIda->fValExportExactMatch  = TRUE;
          pIda->fValExportModExactMatch  = TRUE;
          pIda->fValExportGlobMemMatch   = TRUE;
          pIda->fValExportMachMatch   = TRUE;
          pIda->fValExportReplMatch   = TRUE;
        }
        else
        {
          pIda->sValFormat = CBQUERYSELECTION( hwnd, ID_DOCEXP_VALFORMAT_FORMAT_CB );
  //        pIda->fValExportWithProtSegs = QUERYCHECK( hwnd, ID_DOCEXP_VALFORMAT_PROTSEGS_CHK );
          pIda->fValExportCombine = QUERYCHECK( hwnd, ID_DOCEXP_VALFORMAT_COMBINE_CHK );


          pIda->fValExportValFormat   = QUERYCHECK( hwnd, ID_DOCEXP_VALFORMAT_VALID_RB );
          pIda->fValExportProofFormat = QUERYCHECK( hwnd, ID_DOCEXP_VALFORMAT_PROOF_RB );
          pIda->fValExportRemoveTagging = QUERYCHECK( hwnd, ID_DOCEXP_VALFORMAT_REMOVETAGGING_CHK );
          pIda->fValExportLinksImages = QUERYCHECK( hwnd, ID_DOCEXP_VALFORMAT_LINKSIMAGES_CHK );


          pIda->fValExportInclCount    = QUERYCHECK( hwnd, ID_DOCEXP_VALFORMAT_INCLCOUNT_CHK );
          pIda->fValExportInclExisting = QUERYCHECK( hwnd, ID_DOCEXP_VALFORMAT_INCLEXIST_CHK );
          pIda->fValExportMismatchOnly = QUERYCHECK( hwnd, ID_DOCEXP_VALFORMAT_MISMATCH_CHK );
          pIda->fValExportExactFromManual = QUERYCHECK( hwnd, ID_DOCEXP_VALFORMAT_MANUALEXACT_CHK );
          pIda->fValExportTransOnly = QUERYCHECK( hwnd, ID_DOCEXP_VALFORMAT_TRANSONLY_CHK );

          QUERYTEXTW( hwnd, ID_DOCEXP_VALFORMAT_TRANSLATOR_CB, pIda->szValTranslator );
          QUERYTEXTW( hwnd, ID_DOCEXP_VALFORMAT_MANAGER_CB, pIda->szValManager );
          
          UtlSaveLastUsedString( hwnd, ID_DOCEXP_VALFORMAT_MANAGER_CB, "VALDOCPROJMGR", 10 );
          UtlSaveLastUsedString( hwnd, ID_DOCEXP_VALFORMAT_TRANSLATOR_CB, "VALDOCTRANSL", 10 );
		  
          if ( (QUERYITEMCOUNT( hwnd, ID_DOCEXP_DOCS_LB ) <= 1) || QUERYCHECK( hwnd, ID_DOCEXP_VALFORMAT_COMBINE_CHK ) )
          {
            QUERYTEXT( hwnd, ID_DOCEXP_VALFORMAT_NAME_EF, pIda->szValFormatOutputName );
          }
          else
          {
            pIda->szValFormatOutputName[0] = EOS;
          } /* endif */

          if ( pIda->fValExportProofFormat )
          {
            pIda->fValExportNewMatch    = QUERYCHECK( hwnd, ID_DOCEXP_VALFORMAT_NEW_CHK );
            pIda->fValExportProtMatch   = QUERYCHECK( hwnd, ID_DOCEXP_VALFORMAT_PROT_CHK );
            pIda->fValExportAutoMatch   = QUERYCHECK( hwnd, ID_DOCEXP_VALFORMAT_AUTO_CHK );
            pIda->fValExportModAutoMatch   = QUERYCHECK( hwnd, ID_DOCEXP_VALFORMAT_MODAUTO_CHK );
            pIda->fValExportNotTransl   = QUERYCHECK( hwnd, ID_DOCEXP_VALFORMAT_NOTTRAN_CHK );
            pIda->fValExportFuzzyMatch  = QUERYCHECK( hwnd, ID_DOCEXP_VALFORMAT_FUZZY_CHK );
            pIda->fValExportExactMatch  = QUERYCHECK( hwnd, ID_DOCEXP_VALFORMAT_EXACT_CHK );
            pIda->fValExportModExactMatch  = QUERYCHECK( hwnd, ID_DOCEXP_VALFORMAT_MODEXACT_CHK );
            pIda->fValExportGlobMemMatch   = QUERYCHECK( hwnd, ID_DOCEXP_VALFORMAT_GLOBMEM_CHK );
            pIda->fValExportMachMatch   = QUERYCHECK( hwnd, ID_DOCEXP_VALFORMAT_MACHINE_CHK );
            pIda->fValExportReplMatch   = QUERYCHECK( hwnd, ID_DOCEXP_VALFORMAT_REPLACE_CHK );
          } /* endif */
        } /* endif */
      }
      else if ( pIda->sExportFormatID == ID_DOCEXP_INTFORMAT_RB )
      {
        QUERYTEXT( hwnd, ID_DOCEXP_TARGETPATH_CB, pIda->szPathContent );
        strcpy(pIda->szSelectedTransDrive,"X:");
        pIda->szSelectedTransDrive[0] = pIda->szPathContent[0];
      }
      else                               // external format is selected
      {
        //get state of export checkboxes
        pIda->fTarget = QUERYCHECK( hwnd, ID_DOCEXP_TARGET_CHK );
        pIda->fSource = QUERYCHECK( hwnd, ID_DOCEXP_SOURCE_CHK );
        pIda->fSNoMatch = QUERYCHECK( hwnd, ID_DOCEXP_SNOMATCH_CHK );
        pIda->fWithRevMark = QUERYCHECK( hwnd, ID_DOCEXP_REVMARK_CHK ) && pIda->fTarget;
        pIda->fWithTrackID = QUERYCHECK( hwnd, ID_DOCEXP_TRACKID_CHK ) && pIda->fTarget;
        if ( pIda->fWithTrackID ) {                                    /* 6-3-16 */
           USHORT usReply = UtlError( WARNING_DOCEXP_WITHTRACKID,
                               MB_OKCANCEL | MB_DEFBUTTON2, 0, NULL, EQF_QUERY );
           if ( usReply != MBID_OK )
           {
             sFocusID = ID_DOCEXP_TRACKID_CHK;
             fOK = FALSE ;
           } /* endif */
        }
        pIda->szConversion[0] = EOS;  // QUERYTEXT( hwnd, ID_DOCEXP_CONV_CB, pIda->szConversion );

        // if no checkbox for external format is checked
        if ( !pIda->fTarget && !pIda->fSource && !pIda->fSNoMatch )
        {
          UtlError( CHECKONE, MB_CANCEL, 0, NULL, EQF_ERROR );
          sFocusID = ID_DOCEXP_TARGET_CHK;
          fOK = FALSE;
        } /* endif */

        if ( fOK )
        {
          if ( pIda->fTarget)      //target files checkbox is checked
          {
            INFOEVENT2( DOCEXPCOMMAND_LOC, STATE_EVENT, 1, DOC_GROUP, NULL );

            //
            // Target
            //
            // get content of export target.. entry field
            //
            QUERYTEXT( hwnd, ID_DOCEXP_TARGETPATH_CB, pIda->szPathContent );

            strcpy(pIda->szSelectedTransDrive,"X:");
            pIda->szSelectedTransDrive[0] = pIda->szPathContent[0];

            {
              USHORT i;

              for (i=2; i<(USHORT) strlen(pIda->szPathContent); i++)
              {
                pIda->szPathContent[i-2] = pIda->szPathContent[i];
              }//end for
            }//end construction

            pIda->szPathContent[strlen(pIda->szPathContent)-2] = EOS;

            strcpy(pIda->szTargetDir, pIda->szPathContent);

            if (pIda->szPathContent[strlen(pIda->szPathContent)-1] != '\\' &&
                strlen(pIda->szPathContent) > 1)
            {
              strcat(pIda->szPathContent, "\\");
            } //end if
            //check path and save it to IDA
            fOK = CheckPath( pIda->szTargetDir, pIda->szPathContent );
            if ( !fOK )
            {
              sFocusID = ID_DOCEXP_TARGETPATH_CB;
            } /* endif */

            /****************************************************/
            /* Check settings of revision mark controls         */
            /****************************************************/
            if ( fOK && pIda->fWithRevMark )
            {
              /**************************************************/
              /* Get current value of revision mark control     */
              /**************************************************/
              QUERYTEXT( hwnd, ID_DOCEXP_REVMARK_CB, pIda->szString );
              UtlStripBlanks( pIda->szString );

              /************************************************/
              /* Build fully qualified file name for revision */
              /* mark file                                    */
              /************************************************/
              UtlMakeEQFPath( pIda->szRevMarkFile, NULC, TABLE_PATH,
                              NULL );
              strcat( pIda->szRevMarkFile, BACKSLASH_STR );
              strcat( pIda->szRevMarkFile, pIda->szString );
              strcat( pIda->szRevMarkFile, EXT_OF_REVISION_MARK );


              /**************************************************/
              /* Check revison mark file                        */
              /**************************************************/
              if ( pIda->szString[0] == EOS )
              {
                /************************************************/
                /* No revision mark selected                    */
                /************************************************/
                UtlError( ERROR_DOCEXP_NO_REVMARK, MB_CANCEL,
                          0, NULL, EQF_ERROR );
                sFocusID = ID_DOCEXP_REVMARK_CB;
                fOK = FALSE;
              }
              else if ( CBSEARCHITEM( hwnd,
                                      ID_DOCEXP_REVMARK_CB,
                                      pIda->szString ) == LIT_NONE )
              {
                /************************************************/
                /* Revision mark does not exist                 */
                /************************************************/
                pszReplace = pIda->szString;
                UtlError( ERROR_DOCEXP_REVMARK_NOT_EXIST, MB_CANCEL,
                          1, &pszReplace, EQF_ERROR );
                sFocusID = ID_DOCEXP_REVMARK_CB;
                fOK = FALSE;
              }
              else if ( !UtlFileExist( pIda->szRevMarkFile ) )
              {
                /************************************************/
                /* Revision mark does not exist (must have been */
                /* delete in the mean time ...)                 */
                /************************************************/
                pszReplace = pIda->szString;
                UtlError( ERROR_DOCEXP_REVMARK_NOT_EXIST, MB_CANCEL,
                          1, &pszReplace, EQF_ERROR );
                sFocusID = ID_DOCEXP_REVMARK_CB;
                fOK = FALSE;
              }
              else
              {
                /************************************************/
                /* Everything seems to be ok                    */
                /* Load and check revision marks                */
                /************************************************/
                USHORT usTemp;
                ULONG   ulSize;
                ulSize = pIda->usRevMarkSize;
                usTemp = (USHORT)UtlLoadFileL( pIda->szRevMarkFile,
                                  (PVOID *)&pIda->pRevMark,
                                  &ulSize,
                                  FALSE, TRUE );
                pIda->usRevMarkSize = (USHORT)ulSize;
                if ( usTemp )
                {
                  if ( strcmp( pIda->pRevMark->szID, REVMARKID ) != 0 )
                  {
                    /********************************************/
                    /* Revision mark file corrupted             */
                    /********************************************/
                    pszReplace = pIda->szString;
                    UtlError( ERROR_DOCEXP_REVMARK_CORRUPT, MB_CANCEL,
                              1, &pszReplace, EQF_ERROR );
                    sFocusID = ID_DOCEXP_REVMARK_CB;
                    fOK = FALSE;
                  } /* endif */
                }
                else
                {
                  /**********************************************/
                  /* Error has been reported by UtlLoadFile     */
                  /**********************************************/
                  fOK = FALSE;
                } /* endif */
              } /* endif */
            } /* endif */
          }/*end if*/
        }/*end if fOK*/

        if ( fOK )
        {

          //
          // Source
          //
          // get content of export target.. entry field
          //
          if ( pIda->fSource )      //source files checkbox is checked
          {
            QUERYTEXT( hwnd, ID_DOCEXP_SOURCEPATH_CB, pIda->szPathContent );

            strcpy(pIda->szSelectedOrgDrive,"X:");
            pIda->szSelectedOrgDrive[0] = pIda->szPathContent[0];

            {
              USHORT i;

              for (i=2; i<(USHORT)strlen(pIda->szPathContent); i++)
              {
                pIda->szPathContent[i-2] = pIda->szPathContent[i];
              }//end for
            }//end construction

            pIda->szPathContent[strlen(pIda->szPathContent)-2] = EOS;

            strcpy(pIda->szSourceDir, pIda->szPathContent);

            if (pIda->szPathContent[strlen(pIda->szPathContent)-1] != '\\' &&
                strlen(pIda->szPathContent) > 1)
            {
              strcat(pIda->szPathContent, "\\");
            } //end if

            fOK = CheckPath( pIda->szSourceDir, pIda->szPathContent );
            if ( !fOK )
            {
              sFocusID = ID_DOCEXP_SOURCEPATH_CB;
            } /* endif */
          }/*end if*/
        }/*end if fOK*/

        if ( fOK )
        {
          if ( pIda->fSNoMatch )      //SNOMATCH files checkbox is checked
          {
            //get content of export SNOMATCH entry field
            QUERYTEXT( hwnd, ID_DOCEXP_SNOMATCHPATH_CB, pIda->szPathContent );

            strcpy(pIda->szSelectedSnoDrive,"X:");
            pIda->szSelectedSnoDrive[0] = pIda->szPathContent[0];

            {
              USHORT i;

              for (i=2; i<(USHORT)strlen(pIda->szPathContent); i++)
              {
                pIda->szPathContent[i-2] = pIda->szPathContent[i];

              }//end for

            }//end construction

            pIda->szPathContent[strlen(pIda->szPathContent)-2] = EOS;

            strcpy(pIda->szSNoMatchDir, pIda->szPathContent);

            if (pIda->szPathContent[strlen(pIda->szPathContent)-1] != '\\' &&
                strlen(pIda->szPathContent) > 1)
            {
              strcat(pIda->szPathContent, "\\");
            } //end if

            //check path and save it to IDA
            fOK = CheckPath( pIda->szSNoMatchDir, pIda->szPathContent );
            if ( !fOK )
            {
              sFocusID = ID_DOCEXP_SNOMATCHPATH_CB;
            } /* endif */
          }/*end if*/
        }/*end if fOK*/

        /**********************************************************/
        /* Check for unique path names                            */
        /**********************************************************/
        if ( fOK )
        {
          if ( pIda->fTarget && pIda->fSource )  //source and target path selected
          {
            //if source and target path is equal, display message
            if ( pIda->szSelectedTransDrive[0] == pIda->szSelectedOrgDrive[0] && !( stricmp( pIda->szTargetDir, pIda->szSourceDir ) ) )
            {
              fOK = FALSE;
              UtlError( ERROR_SAMEPATH, MB_CANCEL, 0, NULL, EQF_WARNING);
              sFocusID = ID_DOCEXP_TARGETPATH_CB;
            }/*endif*/
          }/*endif*/
        }/*endif*/

        if ( fOK )
        {
          if ( pIda->fSource && pIda->fSNoMatch  )
          {
            //if source and SNOMATCH path is equal, display message
            if ( !( stricmp( pIda->szSourceDir, pIda->szSNoMatchDir ) ) )
            {
              fOK = FALSE;
              UtlError( ERROR_SAMEPATH, MB_CANCEL, 0, NULL, EQF_WARNING);
              sFocusID = ID_DOCEXP_SNOMATCHPATH_CB;
            }/*endif*/
          }/*endif*/
        }/*endif*/

        if ( fOK )
        {
          if ( pIda->fTarget && pIda->fSNoMatch  )
          {
            //if target and SNOMATCH path is equal, display message
            if ( !( stricmp( pIda->szTargetDir, pIda->szSNoMatchDir ) ) )
            {
              fOK = FALSE;
              UtlError( ERROR_SAMEPATH, MB_CANCEL, 0, NULL, EQF_WARNING);
              sFocusID = ID_DOCEXP_SNOMATCHPATH_CB;
            }/*endif*/
          }/*endif*/
        }/*endif*/
      }/*end if*/

      // save last used strings
      if ( fOK )
      {
        switch ( pIda->sExportFormatID )
        {

          case ID_DOCEXP_VALFORMAT_RB: UtlSaveLastUsedString( hwnd, ID_DOCEXP_VALFORMAT_PATH_CB, DOCEXPVALLASTUSED, 10 ); break;
          case ID_DOCEXP_XMLFORMAT_RB: UtlSaveLastUsedString( hwnd, ID_DOCEXP_VALFORMAT_PATH_CB, DOCEXPXMLLASTUSED, 10 ); break;
          case ID_DOCEXP_INTFORMAT_RB: UtlSaveLastUsedString( hwnd, ID_DOCEXP_TARGETPATH_CB, DOCEXPINTLASTUSED, 10 ); break;
          case ID_DOCEXP_EXTFORMAT_RB: if ( pIda->fTarget ) UtlSaveLastUsedString( hwnd, ID_DOCEXP_TARGETPATH_CB, DOCEXPEXTLASTUSED, 10 ); break;
        }
        if ( pIda->fSource ) UtlSaveLastUsedString( hwnd, ID_DOCEXP_SOURCEPATH_CB, DOCEXPSOURCELASTUSED, 10 ); 
        if ( pIda->fSNoMatch ) UtlSaveLastUsedString( hwnd, ID_DOCEXP_SNOMATCHPATH_CB, DOCEXPSNOMATCHLASTUSED, 10 );
      }

      if ( fOK )
      {
        LONG lExportHandle = 0;                            // handle for export in validation format

        INFOEVENT2( DOCEXPCOMMAND_LOC, STATE_EVENT, 2, DOC_GROUP, NULL );

        // do any export type specific initialization
        if ( (pIda->sExportFormatID == ID_DOCEXP_VALFORMAT_RB) || (pIda->sExportFormatID == ID_DOCEXP_XMLFORMAT_RB) )
        {
          int iRC = 0;
          DOCEXPVALOPTIONS Options;

          memset( &Options, 0, sizeof(Options) );

          if ( pIda->sExportFormatID == ID_DOCEXP_XMLFORMAT_RB )
          {
            Options.ValFormat = PLAINXML_VALEXPFORMAT;
          }
          else
          {
            switch ( pIda->sValFormat )
            {
              case 0 : Options.ValFormat = HTML_VALEXPFORMAT; break;
              case 1 : Options.ValFormat = XML_VALEXPFORMAT; break;
              case 2 : Options.ValFormat = DOC_VALEXPFORMAT; break;
              case 3 : Options.ValFormat = DOCX_VALEXPFORMAT; break;
              case 4 : Options.ValFormat = ODT_VALEXPFORMAT; break;
              default: Options.ValFormat = HTML_VALEXPFORMAT; break;
            } /* endswitch */               
          } /* endif */             

          Options.fCombined = pIda->fValExportCombine;
          Options.fExportInValidationFormat = pIda->fValExportValFormat; 
          Options.fNewMatch =   (EQF_BOOL)pIda->fValExportNewMatch;
          Options.fProtMatch =  (EQF_BOOL)pIda->fValExportProtMatch;
          Options.fAutoMatch =  (EQF_BOOL)pIda->fValExportAutoMatch;
		      Options.fModifiedAuto = (EQF_BOOL)pIda->fValExportModAutoMatch;
          Options.fNotTransl =  (EQF_BOOL)pIda->fValExportNotTransl;
          Options.fFuzzyMatch = (EQF_BOOL)pIda->fValExportFuzzyMatch;
          Options.fExactMatch = (EQF_BOOL)pIda->fValExportExactMatch;
          Options.fModifiedExact = (EQF_BOOL)pIda->fValExportModExactMatch;
          Options.fGlobMemMatch =  (EQF_BOOL)pIda->fValExportGlobMemMatch;
          Options.fMachMatch =  (EQF_BOOL)pIda->fValExportMachMatch;
          Options.fReplMatch =  (EQF_BOOL)pIda->fValExportReplMatch;
          Options.fNoInlineTagRemoval = (EQF_BOOL)!pIda->fValExportRemoveTagging;
          Options.fExactFromManual = (EQF_BOOL)pIda->fValExportExactFromManual;
          Options.fTransOnly = (EQF_BOOL)pIda->fValExportTransOnly;

          if ( (Options.ValFormat == HTML_VALEXPFORMAT) || (Options.ValFormat == DOC_VALEXPFORMAT) || (Options.ValFormat == DOCX_VALEXPFORMAT))
          {
            Options.fLinksImages = (EQF_BOOL)pIda->fValExportLinksImages;
          }
          else
          {
            Options.fLinksImages = FALSE;
          } /* endif */
          strcpy( Options.szOutName, pIda->szValFormatOutputName );

          Options.fInclCount    = (EQF_BOOL)pIda->fValExportInclCount;
          Options.fInclExisting = (EQF_BOOL)pIda->fValExportInclExisting;
          Options.fMismatchOnly = (EQF_BOOL)pIda->fValExportMismatchOnly;
		  
          wcscpy( Options.szTranslator, pIda->szValTranslator );
          wcscpy( Options.szManager, pIda->szValManager );

          SubFolObjectNameToName( pIda->szFolderObjName, pIda->szFolderLongName );
          iRC = DocExpValInit( &Options, pIda->szValDir, pIda->szFolderObjName, &lExportHandle, FALSE, hwnd );
          if ( iRC )
          {
            usRc = UNLOAD_CANCEL;
          } else {
            usRc = UNLOAD_OK;
          }

          // run document pre-scan when automatic recognition of exact matches coming from manual translation is needed 
          if ( (usRc != UNLOAD_CANCEL) && (pIda->fValExportExactFromManual) )
          {
            sIndexItem = 0;
            usRc = UNLOAD_OK;
            sItemCount = QUERYITEMCOUNT( hwnd, ID_DOCEXP_DOCS_LB );
            while ( (sIndexItem < sItemCount) && (usRc != UNLOAD_CANCEL) && (usRc != UNLOAD_CLOSED) )
            {
              // get document name
              SHORT sDocIndex;
              QUERYITEMTEXT( hwnd, ID_DOCEXP_DOCS_LB, sIndexItem, pIda->szLongName );
              sDocIndex = (SHORT)QUERYITEMHANDLE( hwnd, ID_DOCEXP_DOCS_LB, sIndexItem );
              QUERYITEMTEXTHWND( pIda->hwndExportListbox, sDocIndex, pIda->szName );
              pIda->fIsLongName = strcmp( pIda->szName, pIda->szLongName ) != 0;

              // build document object name
              UtlMakeEQFPath( pIda->szDocObjName, pIda->szFolderObjName[0], SYSTEM_PATH, pIda->szFolderName );
              strcat( pIda->szDocObjName, BACKSLASH_STR );
              strcat( pIda->szDocObjName, pIda->szName );

              // check if object already exists, i.e. document is in use
              sRc = QUERYSYMBOL( pIda->szDocObjName );
              if ( sRc != -1 )
              {
                pszReplace = pIda->szLongName;
                UtlError( ERROR_DOC_LOCKED, MB_CANCEL, 1, &pszReplace, EQF_ERROR );
                fOK = FALSE;
              }
              else
              {
                SETSYMBOL( pIda->szDocObjName );
                int iRC = DocExpValPreScan( lExportHandle, pIda->szDocObjName );
                if ( iRC ) usRc = UNLOAD_CANCEL;
                REMOVESYMBOL( pIda->szDocObjName );
                fSaveRc &= ( usRc == UNLOAD_OK );
              } /* endif */

              // Dispatch messages before continuing with next file 
              if ( (sIndexItem != LIT_NONE) && (usRc != UNLOAD_CANCEL) )
              {
                UtlDispatch();
                pIda = ACCESSDLGIDA( hwnd, PDOCEXPIDA );
                if ( (pIda == NULL) || !pIda->fExporting ) usRc = UNLOAD_CLOSED;
              } /* endif */
              sIndexItem++;
            }/*end while*/
          }
        } /* endif */

        //initialize fSaveRc,sIndexItem and usRc
        fSaveRc = TRUE;
        sIndexItem = 0;
        usRc = UNLOAD_OK;

        sItemCount = QUERYITEMCOUNT( hwnd, ID_DOCEXP_DOCS_LB );
        while ( (sIndexItem < sItemCount) && (usRc != UNLOAD_CANCEL) && (usRc != UNLOAD_CLOSED) )
        {
          // get document name
          SHORT sDocIndex;
          QUERYITEMTEXT( hwnd, ID_DOCEXP_DOCS_LB, sIndexItem,
                         pIda->szLongName );
          sDocIndex = (SHORT)QUERYITEMHANDLE( hwnd, ID_DOCEXP_DOCS_LB,
                                              sIndexItem );
          QUERYITEMTEXTHWND( pIda->hwndExportListbox, sDocIndex,
                             pIda->szName );
          pIda->fIsLongName = strcmp( pIda->szName, pIda->szLongName ) != 0;
          // build document object name
          UtlMakeEQFPath( pIda->szDocObjName,
                          pIda->szFolderObjName[0], SYSTEM_PATH,
                          pIda->szFolderName );
          strcat( pIda->szDocObjName, BACKSLASH_STR );
          strcat( pIda->szDocObjName, pIda->szName );

          // check if object already exists, i.e. document is in use
          sRc = QUERYSYMBOL( pIda->szDocObjName );
          if ( sRc != -1 )
          {
            pszReplace = pIda->szLongName;
            UtlError( ERROR_DOC_LOCKED, MB_CANCEL, 1, &pszReplace, EQF_ERROR );
            fOK = FALSE;
          }
          else
          {
            SETSYMBOL( pIda->szDocObjName );

            INFOEVENT2( DOCEXPCOMMAND_LOC, STATE_EVENT, 3, DOC_GROUP, pIda->szDocObjName );

            if ( (pIda->sExportFormatID == ID_DOCEXP_VALFORMAT_RB) || (pIda->sExportFormatID == ID_DOCEXP_XMLFORMAT_RB) )
            {
              int iRC = DocExpValProcess( lExportHandle, pIda->szDocObjName );
              if ( iRC )
              {
                usRc = UNLOAD_CANCEL;
              } /* endif */
            }
            else if ( pIda->sExportFormatID == ID_DOCEXP_INTFORMAT_RB )
            {
              strcpy(pIda->szSelectedDrive, pIda->szSelectedTransDrive);
              usRc = DocExpExportInternal( pIda->szFolderObjName, pIda->szLongName, pIda->szName, pIda->szSelectedDrive[0], pIda->hwndDlg, &(pIda->fOverwrite), NULL );
            }
            else
            {
              usRc = DocumentUnload( pIda, hwnd );
            }/*end if*/

            if ( usRc != UNLOAD_OK )
            {
              ERREVENT2( DOCEXPCOMMAND_LOC, INTFUNCFAILED_EVENT, usRc, DOC_GROUP, NULL );
            } /* endif */
            INFOEVENT2( DOCEXPCOMMAND_LOC, STATE_EVENT, 4, DOC_GROUP, NULL );

            REMOVESYMBOL( pIda->szDocObjName );

            fSaveRc &= ( usRc == UNLOAD_OK );

            if ( usRc == UNLOAD_OK )
            {
              INFOEVENT2( DOCEXPCOMMAND_LOC, STATE_EVENT, 5, DOC_GROUP, NULL );
              // update document properties
              fOK = DocExpUpdateDosProps( pIda );

              //deselect item if successfully exported
              if ( fOK ) DESELECTITEM( hwnd, ID_DOCEXP_DOCS_LB, sIndexItem );
            }/*endif*/
          } /* endif */

          /*********************************************************/
          /* Dispatch messages before continuing with next file    */
          /*********************************************************/
          if ( (sIndexItem != LIT_NONE) && (usRc != UNLOAD_CANCEL) )
          {
            UtlDispatch();
            pIda = ACCESSDLGIDA( hwnd, PDOCEXPIDA );

            /*******************************************************/
            /* Check if dialog has been canceled                   */
            /*******************************************************/
            if ( (pIda == NULL) || !pIda->fExporting )
            {
              usRc = UNLOAD_CLOSED;
            } /* endif */
          } /* endif */

          sIndexItem++;
        }/*end while*/

        // call any export type specific termination function
        if ( ((pIda->sExportFormatID == ID_DOCEXP_VALFORMAT_RB) || (pIda->sExportFormatID == ID_DOCEXP_XMLFORMAT_RB)) && 
             (lExportHandle != 0) )
        {
          int iRC = DocExpValTerminate( lExportHandle );
          if ( iRC )
          {
            usRc = UNLOAD_CLOSED;
          } /* endif */
        } /* endif */

        /* set flag that export process is complete */
        if ( pIda != NULL )
        {
          pIda->fExporting = FALSE;
        } /* endif */

        // bt
        // return error message NOT_ALL_EXPORTED if handling fails
        //
        if (usRc == UNLOAD_CANCEL) usRc = UNLOAD_CLOSED;

        if ( (pIda != NULL) && fOK && (usRc != UNLOAD_CANCEL) )
        {
          if ( (!fSaveRc) || (usRc == UNLOAD_CLOSED)  )
          {
            //display message that not all selected files
            //are successfully exported
            UtlError(NOT_ALL_EXPORTED, MB_OK, 0, NULL, EQF_WARNING);
          }
          else
          {
            //
            //Save properties
            //
            //get write access to export/import properties
            //
            if ( SetPropAccess( pIda->IdaHead.hProp, PROP_ACCESS_WRITE) )
            {
              //get pointer
              ppropImex = (PPROPIMEX)MakePropPtrFromHnd( pIda->IdaHead.hProp);

              //save content of entry fields to import/export properties
              if ( pIda->szTargetDir[0] != EOS && pIda->fTarget )
              {
                strcpy( ppropImex->szSavedDlgFExpoTPath, pIda->szTargetDir);
              } /* endif */

              if ( pIda->szSourceDir[0] != EOS && pIda->fSource )
              {
                strcpy( ppropImex->szSavedDlgFExpoSPath, pIda->szSourceDir);
              } /* endif */

              if ( pIda->szSNoMatchDir[0] != EOS && pIda->fSNoMatch )
              {
                strcpy( ppropImex->szSavedDlgFExpoNPath, pIda->szSNoMatchDir);
              } /* endif */

              QUERYTEXT( hwnd, ID_DOCEXP_REVMARK_CB, pIda->szString );
              UtlStripBlanks( pIda->szString );
              strcpy( ppropImex->szSavedRevMark, pIda->szString );

              /* save all remaining flags/info into last used values */
              ppropImex->sSavedDlgFExpoFormat = pIda->sExportFormatID;
              ppropImex->fSavedDlgFExpoTranslation = (EQF_BOOL)pIda->fTarget;
              ppropImex->fSavedDlgFExpoWithRevMark = (EQF_BOOL)pIda->fWithRevMark;
              ppropImex->fSavedDlgFExpoWithTrackID = (EQF_BOOL)pIda->fWithTrackID;
              ppropImex->fSavedDlgFExpoOriginal = (EQF_BOOL)pIda->fSource;
              ppropImex->fSavedDlgFExpoSNOMATCH = (EQF_BOOL)pIda->fSNoMatch;
              ppropImex->cSavedDlgFExpoDrive = pIda->szSelectedDrive[0];
              ppropImex->cSavedDlgFExpoDriveTrans = pIda->szSelectedTransDrive[0];
              ppropImex->cSavedDlgFExpoDriveOrg = pIda->szSelectedOrgDrive[0];
              ppropImex->cSavedDlgFExpoDriveSno = pIda->szSelectedSnoDrive[0];

              // fields of export in validation format
              if ( pIda->sExportFormatID == ID_DOCEXP_VALFORMAT_RB )
              {
                strcpy( ppropImex->szSavedDlgFExpoValPath, pIda->szValDir);
                ppropImex->sSavedDlgFExpoValFormat = pIda->sValFormat;
                ppropImex->fSavedDlgFExpoValWithProtSegs = (EQF_BOOL)pIda->fValExportWithProtSegs;
                ppropImex->fSavedDlgFExpoValCombine     = (EQF_BOOL)pIda->fValExportCombine;
                ppropImex->fSavedDlgFExpoValValFormat   = (EQF_BOOL)pIda->fValExportValFormat;
                ppropImex->fSavedDlgFExpoValProofFormat = (EQF_BOOL)pIda->fValExportProofFormat;
                ppropImex->fSavedDlgFExpoValAllMatch    = (EQF_BOOL)pIda->fValExportAllMatch;
                ppropImex->fSavedDlgFExpoValNewMatch    = (EQF_BOOL)pIda->fValExportNewMatch;
                ppropImex->fSavedDlgFExpoValProtMatch   = (EQF_BOOL)pIda->fValExportProtMatch;
                ppropImex->fSavedDlgFExpoValAutoMatch   = (EQF_BOOL)pIda->fValExportAutoMatch;
				ppropImex->fSavedDlgFExpoValModAutoMatch  = (EQF_BOOL)pIda->fValExportModAutoMatch;
                ppropImex->fSavedDlgFExpoValNotTransl   = (EQF_BOOL)pIda->fValExportNotTransl;
                ppropImex->fSavedDlgFExpoValFuzzyMatch  = (EQF_BOOL)pIda->fValExportFuzzyMatch;
                ppropImex->fSavedDlgFExpoValExactMatch  = (EQF_BOOL)pIda->fValExportExactMatch;
                ppropImex->fSavedDlgFExpoValModExactMatch  = (EQF_BOOL)pIda->fValExportModExactMatch;
                ppropImex->fSavedDlgFExpoValGlobMemMatch   = (EQF_BOOL)pIda->fValExportGlobMemMatch;
                ppropImex->fSavedDlgFExpoValMachMatch   = (EQF_BOOL)pIda->fValExportMachMatch;
                ppropImex->fSavedDlgFExpoValReplMatch   = (EQF_BOOL)pIda->fValExportReplMatch;
                if ( pIda->fValExportRemoveTagging )
                {
                  ppropImex->usSavedDlgFExpoValRemoveTagging = 1; // 1 = remove tagging
                }
                else
                {
                  ppropImex->usSavedDlgFExpoValRemoveTagging = 2; // 2 = leave tagging as-is
                } /* endif */
                ppropImex->fSavedDlgFExpoValInclCount    = (EQF_BOOL)pIda->fValExportInclCount;
                ppropImex->fSavedDlgFExpoValInclExisting = (EQF_BOOL)pIda->fValExportInclExisting;
                ppropImex->fSavedDlgFExpoValMismatchOnly = (EQF_BOOL)pIda->fValExportMismatchOnly;
                ppropImex->fSavedDlgFExpoValLinksImages = (EQF_BOOL)pIda->fValExportLinksImages;
                ppropImex->fSavedDlgFExpoValExactFromManual = (EQF_BOOL)pIda->fValExportExactFromManual;
                ppropImex->fSavedDlgFExpoValTransOnly = (EQF_BOOL)pIda->fValExportTransOnly;

              } /* endif */

              // fields of export in plain XML format
              if ( pIda->sExportFormatID == ID_DOCEXP_XMLFORMAT_RB )
              {
                strcpy( ppropImex->szSavedDlgFExpoValPath, pIda->szValDir);
                ppropImex->sSavedDlgFExpoValFormat = pIda->sValFormat;
                ppropImex->fSavedDlgFExpoValCombine     = (EQF_BOOL)pIda->fValExportCombine;
              } /* endif */

              //save import/exportproperties
              SaveProperties( pIda->IdaHead.hProp, &ulErrorInfo );
              //reset access mode to save import/export properties
              ResetPropAccess( pIda->IdaHead.hProp, PROP_ACCESS_WRITE);

              // update folder properties as well
              {
                PPROPFOLDER pFolProp;            // ptr to folder properties
                PVOID       hFolProp;            // handle of folder properties
                OBJNAME     szFolObjName;        // buffer for folder object name
                CHAR        szTmgrDrive[3];      // buffer for Tmgr primary drive
                ULONG       ErrorInfo = 0;

                // setup folder object name
                UtlQueryString( QST_PRIMARYDRIVE, szTmgrDrive, sizeof(szTmgrDrive) );
                strcpy( szFolObjName, pIda->szFolderObjName );
                szFolObjName[0] = szTmgrDrive[0];

                hFolProp = OpenProperties( szFolObjName, NULL, PROP_ACCESS_READ, &ErrorInfo);
                if ( hFolProp )
                {
                  if ( SetPropAccess( hFolProp, PROP_ACCESS_WRITE) )
                  {
                    pFolProp = (PPROPFOLDER)MakePropPtrFromHnd( hFolProp );
                    pFolProp->fDocExpLastUsed = TRUE;
                    if ( pIda->szTargetDir[0] != EOS && pIda->fTarget )
                    {
                      strcpy( pFolProp->szSavedDlgFExpoTPath, pIda->szTargetDir);
                    } /* endif */
                    if ( pIda->szSourceDir[0] != EOS && pIda->fSource )
                    {
                      strcpy( pFolProp->szSavedDlgFExpoSPath, pIda->szSourceDir);
                    } /* endif */
                    if ( pIda->szSNoMatchDir[0] != EOS && pIda->fSNoMatch )
                    {
                      strcpy( pFolProp->szSavedDlgFExpoNPath, pIda->szSNoMatchDir);
                    } /* endif */
                    QUERYTEXT( hwnd, ID_DOCEXP_REVMARK_CB, pIda->szString );
                    UtlStripBlanks( pIda->szString );
                    strcpy( pFolProp->szSavedRevMark, pIda->szString );
                    pFolProp->sSavedDlgFExpoFormat = pIda->sExportFormatID;
                    pFolProp->fSavedDlgFExpoTranslation = (EQF_BOOL)pIda->fTarget;
                    pFolProp->fSavedDlgFExpoWithRevMark = (EQF_BOOL)pIda->fWithRevMark;
                    pFolProp->fSavedDlgFExpoWithTrackID = (EQF_BOOL)pIda->fWithTrackID;
                    pFolProp->fSavedDlgFExpoOriginal = (EQF_BOOL)pIda->fSource;
                    pFolProp->fSavedDlgFExpoSNOMATCH = (EQF_BOOL)pIda->fSNoMatch;
                    pFolProp->cSavedDlgFExpoDrive = pIda->szSelectedDrive[0];
                    pFolProp->cSavedDlgFExpoDriveTrans = pIda->szSelectedTransDrive[0];
                    pFolProp->cSavedDlgFExpoDriveOrg = pIda->szSelectedOrgDrive[0];
                    pFolProp->cSavedDlgFExpoDriveSno = pIda->szSelectedSnoDrive[0];
                    if ( pIda->sExportFormatID == ID_DOCEXP_VALFORMAT_RB )
                    {
                      pFolProp->fSavedDlgFExpoValCombine = (EQF_BOOL)pIda->fValExportCombine;
                      if ( pIda->fValExportRemoveTagging )
                      {
                        pFolProp->usSavedDlgFExpoValRemoveTagging = 1; // 1 = remove tagging
                      }
                      else
                      {
                        pFolProp->usSavedDlgFExpoValRemoveTagging = 2; // 2 = leave tagging as-is
                      } /* endif */
                      pFolProp->fSavedDlgFExpoValWithProtSegs = (EQF_BOOL)pIda->fValExportWithProtSegs;
                      strcpy( pFolProp->szSavedDlgFExpoValPath, pIda->szValDir );
                      pFolProp->sSavedDlgFExpoValFormat = pIda->sValFormat;
                      pFolProp->fSavedDlgFExpoValValFormat   = (EQF_BOOL)pIda->fValExportValFormat;
                      pFolProp->fSavedDlgFExpoValProofFormat = (EQF_BOOL)pIda->fValExportProofFormat;
                      pFolProp->fSavedDlgFExpoValAllMatch    = (EQF_BOOL)pIda->fValExportAllMatch;
                      pFolProp->fSavedDlgFExpoValNewMatch    = (EQF_BOOL)pIda->fValExportNewMatch;
                      pFolProp->fSavedDlgFExpoValProtMatch   = (EQF_BOOL)pIda->fValExportProtMatch;
                      pFolProp->fSavedDlgFExpoValAutoMatch   = (EQF_BOOL)pIda->fValExportAutoMatch;
                      pFolProp->fSavedDlgFExpoValNotTransl   = (EQF_BOOL)pIda->fValExportNotTransl;
                      pFolProp->fSavedDlgFExpoValFuzzyMatch  = (EQF_BOOL)pIda->fValExportFuzzyMatch;
                      pFolProp->fSavedDlgFExpoValExactMatch  = (EQF_BOOL)pIda->fValExportExactMatch;
                      pFolProp->fSavedDlgFExpoValModExactMatch  = (EQF_BOOL)pIda->fValExportModExactMatch;
                      pFolProp->fSavedDlgFExpoValGlobMemMatch   = (EQF_BOOL)pIda->fValExportGlobMemMatch;
                      pFolProp->fSavedDlgFExpoValMachMatch   = (EQF_BOOL)pIda->fValExportMachMatch;
                      pFolProp->fSavedDlgFExpoValReplMatch   = (EQF_BOOL)pIda->fValExportReplMatch;
                      pFolProp->fSavedDlgFExpoValInclCount    = (EQF_BOOL)pIda->fValExportInclCount;
                      pFolProp->fSavedDlgFExpoValInclExisting = (EQF_BOOL)pIda->fValExportInclExisting;
                      pFolProp->fSavedDlgFExpoValMismatchOnly = (EQF_BOOL)pIda->fValExportMismatchOnly;
                      pFolProp->fSavedDlgFExpoValLinksImages = (EQF_BOOL)pIda->fValExportLinksImages;
                    } /* endif */

                    if ( pIda->sExportFormatID == ID_DOCEXP_XMLFORMAT_RB )
                    {
                      pFolProp->fSavedDlgFExpoValCombine = (EQF_BOOL)pIda->fValExportCombine;
                    } /* endif */

                    SaveProperties( hFolProp, &ulErrorInfo );
                    ResetPropAccess( hFolProp, PROP_ACCESS_WRITE);
                  } /* endif */
                  CloseProperties( hFolProp, PROP_QUIT, &ErrorInfo );
                } /* endif */

              }

              //display message that all selected files
              //are successfully exported
              UtlError( ALL_EXPORTED, MB_OK, 0, NULL, EQF_INFO );
            }
            else                      //property access fails
            {
              UtlError( ERROR_PROPERTY_ACCESS, MB_CANCEL,
                        0, NULL, EQF_ERROR );
            } /* endif */

            //post message to destroy dialog
            WinPostMsg( hwnd, WM_EQF_CLOSE, NULL, NULL );
          }/*end if*/
        }/*end if*/

        if ( (pIda != NULL) && (usRc == UNLOAD_CLOSED) && pIda->fBeingClosed)
        {
          //post message to destroy dialog
          WinPostMsg( hwnd, WM_EQF_CLOSE, NULL, NULL );
        } /* endif */
      }/*end if fOK*/

      // enable controls
      if ( pIda != NULL )
      {
        EnableExportControls( hwnd, pIda, TRUE );

        // set any focus (does not work when control is disabled)
        if ( sFocusID )
        {
          SETFOCUS( hwnd, sFocusID );
        } /* endif */

        // Export process has terminated now, WM_EQF_CLOSE may be processed now
        pIda->fExportInProcess = FALSE;
        pIda->fExporting = FALSE;
      } /* endif */
      break;

    case ID_DOCEXP_VALFORMAT_SELECTALL_PB:
    case ID_DOCEXP_VALFORMAT_DESELECTALL_PB:
      {
        BOOL fNewState = (sId == ID_DOCEXP_VALFORMAT_SELECTALL_PB);

        SETCHECK( hwnd, ID_DOCEXP_VALFORMAT_NEW_CHK, fNewState );
        SETCHECK( hwnd, ID_DOCEXP_VALFORMAT_PROT_CHK, fNewState );
        SETCHECK( hwnd, ID_DOCEXP_VALFORMAT_AUTO_CHK,  fNewState);
		SETCHECK( hwnd, ID_DOCEXP_VALFORMAT_MODAUTO_CHK,  fNewState);
        SETCHECK( hwnd, ID_DOCEXP_VALFORMAT_NOTTRAN_CHK, fNewState );
        SETCHECK( hwnd, ID_DOCEXP_VALFORMAT_FUZZY_CHK, fNewState );
        SETCHECK( hwnd, ID_DOCEXP_VALFORMAT_EXACT_CHK, fNewState );
        SETCHECK( hwnd, ID_DOCEXP_VALFORMAT_MODEXACT_CHK, fNewState );
        SETCHECK( hwnd, ID_DOCEXP_VALFORMAT_GLOBMEM_CHK, fNewState );
        SETCHECK( hwnd, ID_DOCEXP_VALFORMAT_MACHINE_CHK, fNewState );
        SETCHECK( hwnd, ID_DOCEXP_VALFORMAT_REPLACE_CHK, fNewState );
      }
      break;

    case ID_DOCEXP_VALFORMAT_VALID_RB:
      if ( QUERYCHECK( hwnd, ID_DOCEXP_VALFORMAT_VALID_RB ) )
      {
        pIda = ACCESSDLGIDA( hwnd, PDOCEXPIDA );
        DocValFormatSetMatchStates( hwnd, pIda, TRUE, TRUE );
      } /* endif */
      break;

    case ID_DOCEXP_VALFORMAT_FORMAT_CB:
      if ( sNotification == CBN_SELCHANGE )
      {
        pIda = ACCESSDLGIDA( hwnd, PDOCEXPIDA );
        pIda->sValFormat = CBQUERYSELECTION( hwnd, ID_DOCEXP_VALFORMAT_FORMAT_CB );
        if ( ((VALFORMATID)pIda->sValFormat == HTML_VALEXPFORMAT) || ((VALFORMATID)pIda->sValFormat == DOC_VALEXPFORMAT) || ((VALFORMATID)pIda->sValFormat == DOCX_VALEXPFORMAT) )
        {
          ENABLECTRL( hwnd, ID_DOCEXP_VALFORMAT_LINKSIMAGES_CHK, TRUE );
          SETCHECK( hwnd, ID_DOCEXP_VALFORMAT_LINKSIMAGES_CHK, pIda->fValExportLinksImages );
        }
        else
        {
          pIda->fValExportLinksImages = QUERYCHECK( hwnd, ID_DOCEXP_VALFORMAT_LINKSIMAGES_CHK );
          ENABLECTRL( hwnd, ID_DOCEXP_VALFORMAT_LINKSIMAGES_CHK, FALSE );
          SETCHECK_FALSE( hwnd, ID_DOCEXP_VALFORMAT_LINKSIMAGES_CHK );
        } /* endif */
      } 
      break;

    case ID_DOCEXP_VALFORMAT_PROOF_RB:
      if ( QUERYCHECK( hwnd, ID_DOCEXP_VALFORMAT_PROOF_RB ) )
      {
        pIda = ACCESSDLGIDA( hwnd, PDOCEXPIDA );
        DocValFormatSetMatchStates( hwnd, pIda, FALSE, TRUE );
      } /* endif */
      break;

    case ID_DOCEXP_VALFORMAT_INCLEXIST_CHK:
      {
        pIda = ACCESSDLGIDA( hwnd, PDOCEXPIDA );

        if ( !pIda->fInit && (sNotification == BN_CLICKED) )
        {
          if ( QUERYCHECK( hwnd, ID_DOCEXP_VALFORMAT_INCLEXIST_CHK ) )
          {
            ENABLECTRL( hwnd, ID_DOCEXP_VALFORMAT_MISMATCH_CHK, TRUE );
          }
          else
          {
            ENABLECTRL( hwnd, ID_DOCEXP_VALFORMAT_MISMATCH_CHK, FALSE );
            SETCHECK_FALSE( hwnd, ID_DOCEXP_VALFORMAT_MISMATCH_CHK );
            DocValFormatEnableMatchFlags( hwnd, TRUE );
          } /* endif */
        } /* endif */
      }
      break;

    case ID_DOCEXP_VALFORMAT_MISMATCH_CHK:
      {
        pIda = ACCESSDLGIDA( hwnd, PDOCEXPIDA );

        if ( !pIda->fInit && (sNotification == BN_CLICKED) )
        {
          BOOL fEnable = QUERYCHECK( hwnd, ID_DOCEXP_VALFORMAT_MISMATCH_CHK );
          DocValFormatEnableMatchFlags( hwnd, !fEnable );
        } /* endif */
      }
      break;


      /**************************************************************/
      /* Pass notication messages to WM_CONTROL message handler     */
      /**************************************************************/
    case ID_DOCEXP_EXTFORMAT_RB:
    case ID_DOCEXP_INTFORMAT_RB:
    case ID_DOCEXP_DOCS_LB:
    case ID_DOCEXP_SNOMATCH_CHK:
    case ID_DOCEXP_REVMARK_CHK:
    case ID_DOCEXP_TRACKID_CHK:
    case ID_DOCEXP_SOURCE_CHK:
    case ID_DOCEXP_TARGET_CHK:
    case ID_DOCEXP_IMPPATH_RB:
    case ID_DOCEXP_VALFORMAT_RB:
       mResult = DocExpControl( hwnd, sId, sNotification );
      break;
    case ID_DOCEXP_SOURCEPATH_CB:
      if ( sNotification == EN_KILLFOCUS )
      {
        ClearIME( hwnd );
      } /* endif */
      break;
    default :
      break;
  } /* endswitch */
  return( mResult );
} /* end of DocExpCommand */

//------------------------------------------------------------------------------
// DocExpInit                                                                 --
//------------------------------------------------------------------------------


static MRESULT DocExpInit( HWND hwnd, WPARAM mp1, LPARAM mp2 )
{
  PDOCEXPIDA        pIda;                  //instance area for EQFFEXPO
  ULONG       ulErrorInfo;                 //error indicator from PRHA
  BOOL        fOK = TRUE;                  //boolean flag
  PPROPIMEX   ppropImex;                   //handle of IMEX properties
  PSZ         pszTemp;                     //temorary pointer
  SHORT       sItem;                            //item number
  MRESULT     mResult = MRFROMSHORT(TRUE); // result of message processing

  mp1 = mp1;                          // get rid off compiler warning
  mp2 = mp2;                          // get rid off compiler warning

  //allocate storage for instance area (IDA)
  fOK = UtlAlloc( (PVOID *)&pIda, 0L, (ULONG)sizeof( DOCEXPIDA ), ERROR_STORAGE );
  if ( fOK )
  {
    fOK = ANCHORDLGIDA( hwnd, pIda );
    pIda->hwndDlg = hwnd;

    /* initialize fExporting flag to FALSE */
    pIda->fExporting = FALSE;
    pIda->fBeingClosed = FALSE;
    pIda->fExportInProcess = FALSE;
    pIda->fInit = TRUE;

    if ( !fOK )
    {
      // anchor of IDA failed ==> display error message
      UtlError( 0, MB_CANCEL, 0, NULL, SYSTEM_ERROR );
    } /* endif */
  } /* endif */

  if ( fOK )
  {
    //get object name of active folder and save it to IDA
    EqfQueryObjectName( EqfQueryActiveFolderHwnd(), pIda->szFolderObjName );
    strcpy( pIda->szParentObjName, pIda->szFolderObjName );
    if ( FolIsSubFolderObject( pIda->szFolderObjName ) )
    {
      // get main folder object name to allow the remaining code to work
      // without changes
      UtlSplitFnameFromPath( pIda->szFolderObjName ); // cut off subfolder name
      UtlSplitFnameFromPath( pIda->szFolderObjName ); // cut off property directory
    } /* endif */

    //get folder name and save it to IDA
    pszTemp = strrchr( pIda->szFolderObjName, BACKSLASH );
    //save folder name to ida
    strcpy( pIda->szFolderName, pszTemp + 1);

    //open import/export properties
    UtlMakeEQFPath( pIda->szObjName, NULC, SYSTEM_PATH, NULL );
    strcat( pIda->szObjName, BACKSLASH_STR );
    strcat( pIda->szObjName, IMEX_PROPERTIES_NAME );

    if ( (pIda->IdaHead.hProp = OpenProperties
           ( pIda->szObjName, NULL,
             PROP_ACCESS_READ, &ulErrorInfo)) == NULL)
    {                                     //error from propery handler
      //display error message
      pszTemp = pIda->szObjName;
      UtlError( ERROR_PROPERTY_ACCESS, MB_CANCEL, 1,
                &pszTemp, EQF_ERROR );
      fOK = FALSE;
    } /* endif */
  } /* endif */

  if ( fOK )
  {
    //get access to import/export properties
    ppropImex = (PPROPIMEX)MakePropPtrFromHnd( pIda->IdaHead.hProp);

    // use last used values on folder level if there are any
    {
      PPROPFOLDER pFolProp;            // ptr to folder properties
      PVOID       hFolProp;            // handle of folder properties
      OBJNAME     szFolObjName;        // buffer for folder object name
      CHAR        szTmgrDrive[3];      // buffer for Tmgr primary drive
      ULONG       ErrorInfo = 0;

      // setup folder object name
      UtlQueryString( QST_PRIMARYDRIVE, szTmgrDrive, sizeof(szTmgrDrive) );
      strcpy( szFolObjName, pIda->szFolderObjName );
      szFolObjName[0] = szTmgrDrive[0];

      hFolProp = OpenProperties( szFolObjName, NULL, PROP_ACCESS_READ, &ErrorInfo);
      if ( hFolProp )
      {
        pFolProp = (PPROPFOLDER)MakePropPtrFromHnd( hFolProp );
        if ( pFolProp->fDocExpLastUsed )
        {
          if ( pFolProp->szSavedDlgFExpoTPath[0] ) strcpy( ppropImex->szSavedDlgFExpoTPath, pFolProp->szSavedDlgFExpoTPath );
          if ( pFolProp->szSavedDlgFExpoSPath[0] ) strcpy( ppropImex->szSavedDlgFExpoSPath, pFolProp->szSavedDlgFExpoSPath );
          if ( pFolProp->szSavedDlgFExpoNPath[0] ) strcpy( ppropImex->szSavedDlgFExpoNPath, pFolProp->szSavedDlgFExpoNPath );
          strcpy( ppropImex->szSavedRevMark, pFolProp->szSavedRevMark );
          ppropImex->sSavedDlgFExpoFormat = pFolProp->sSavedDlgFExpoFormat;
          ppropImex->fSavedDlgFExpoTranslation = pFolProp->fSavedDlgFExpoTranslation;
          ppropImex->fSavedDlgFExpoWithRevMark = pFolProp->fSavedDlgFExpoWithRevMark;
          ppropImex->fSavedDlgFExpoWithTrackID = pFolProp->fSavedDlgFExpoWithTrackID;
          ppropImex->fSavedDlgFExpoOriginal = pFolProp->fSavedDlgFExpoOriginal;
          ppropImex->fSavedDlgFExpoSNOMATCH = pFolProp->fSavedDlgFExpoSNOMATCH;
          ppropImex->cSavedDlgFExpoDrive = pFolProp->cSavedDlgFExpoDrive;
          ppropImex->cSavedDlgFExpoDriveTrans = pFolProp->cSavedDlgFExpoDriveTrans;
          ppropImex->cSavedDlgFExpoDriveOrg = pFolProp->cSavedDlgFExpoDriveOrg;
          ppropImex->cSavedDlgFExpoDriveSno = pFolProp->cSavedDlgFExpoDriveSno;

          // new fields for export in the validation format
          if ( pFolProp->szSavedDlgFExpoValPath[0] != EOS ) 
          {
            strcpy( ppropImex->szSavedDlgFExpoValPath, pFolProp->szSavedDlgFExpoValPath );
            ppropImex->sSavedDlgFExpoValFormat       = pFolProp->sSavedDlgFExpoValFormat ;
            ppropImex->fSavedDlgFExpoValWithProtSegs = pFolProp->fSavedDlgFExpoValWithProtSegs;
            ppropImex->fSavedDlgFExpoValValFormat    = pFolProp->fSavedDlgFExpoValValFormat;
            ppropImex->fSavedDlgFExpoValProofFormat  = pFolProp->fSavedDlgFExpoValProofFormat;
            ppropImex->fSavedDlgFExpoValAllMatch     = pFolProp->fSavedDlgFExpoValAllMatch;
            ppropImex->fSavedDlgFExpoValNewMatch     = pFolProp->fSavedDlgFExpoValNewMatch;
            ppropImex->fSavedDlgFExpoValProtMatch    = pFolProp->fSavedDlgFExpoValProtMatch;   
            ppropImex->fSavedDlgFExpoValAutoMatch    = pFolProp->fSavedDlgFExpoValAutoMatch;
            ppropImex->fSavedDlgFExpoValNotTransl    = pFolProp->fSavedDlgFExpoValNotTransl;
            ppropImex->fSavedDlgFExpoValFuzzyMatch   = pFolProp->fSavedDlgFExpoValFuzzyMatch;  
            ppropImex->fSavedDlgFExpoValExactMatch   = pFolProp->fSavedDlgFExpoValExactMatch;  
            ppropImex->fSavedDlgFExpoValMachMatch    = pFolProp->fSavedDlgFExpoValMachMatch;   
            ppropImex->fSavedDlgFExpoValReplMatch    = pFolProp->fSavedDlgFExpoValReplMatch;
            ppropImex->fSavedDlgFExpoValInclCount    = pFolProp->fSavedDlgFExpoValInclCount;
            ppropImex->fSavedDlgFExpoValInclExisting = pFolProp->fSavedDlgFExpoValInclExisting;
            ppropImex->fSavedDlgFExpoValMismatchOnly = pFolProp->fSavedDlgFExpoValMismatchOnly;
          } /* endif */
        } /* endif */
        CloseProperties( hFolProp, PROP_QUIT, &ErrorInfo );
      } /* endif */
    }

    //initialze variables
    pIda->szSelectedDrive[1] = COLON;
    pIda->szSelectedDrive[2] = EOS;

    pIda->szSelectedTransDrive[1] = COLON;
    pIda->szSelectedTransDrive[2] = EOS;

    pIda->szSelectedOrgDrive[1] = COLON;
    pIda->szSelectedOrgDrive[2] = EOS;

    pIda->szSelectedSnoDrive[1] = COLON;
    pIda->szSelectedSnoDrive[2] = EOS;

    //set export button as default button
    SETDEFAULTPB_TRUE( hwnd, PID_PB_OK );

    // Create invisible listbox for actual document names
    pIda->hwndExportListbox = WinCreateWindow( hwnd, WC_LISTBOX, "", 0,
                                               0, 0, 0, 0,
                                               hwnd, HWND_TOP, 1, NULL, NULL );

    //get path to active folder
    EqfQueryObjectName( EqfQueryActiveFolderHwnd(), pIda->szObjName );
    strcpy( pIda->szParentObjName, pIda->szObjName );
    if ( FolIsSubFolderObject( pIda->szObjName ) )
    {
      // get main folder object name to allow the remaining code to work
      // without changes
      UtlSplitFnameFromPath( pIda->szObjName ); // cut off subfolder name
      UtlSplitFnameFromPath( pIda->szObjName ); // cut off property directory
    } /* endif */

    //get selected documents of active folder and load them into
    //export listbox
    if ( !(EqfSend2Handler( FOLDERHANDLER, WM_EQF_QUERYSELECTEDNAMES,
                            MP1FROMHWND(pIda->hwndExportListbox),
                            MP2FROMP(pIda->szParentObjName) ) ) )
    {
      fOK = FALSE;
    }/*endif*/

    if ( fOK )
    {
      // fill the visible document listbox with the long names of the
      // document listbox containing the actual (the short) document names
      sItem = QUERYITEMCOUNTHWND( pIda->hwndExportListbox );
      while ( sItem-- )
      {
        SHORT sInsertItem;           // index of inserted item

        // get the document name
        QUERYITEMTEXTHWND( pIda->hwndExportListbox, sItem, pIda->szName );

        // build document object name
        UtlMakeEQFPath( pIda->szDocObjName,
                        pIda->szFolderObjName[0], SYSTEM_PATH,
                        pIda->szFolderName );
        strcat( pIda->szDocObjName, BACKSLASH_STR );
        strcat( pIda->szDocObjName, pIda->szName );

        // get the long name for this document
        pIda->szLongName[0] = EOS;
        DocQueryInfo2( pIda->szDocObjName, NULL, NULL, NULL, NULL,
                       pIda->szLongName, NULL, NULL, TRUE );

        // add long name to visible document listbox
        if ( pIda->szLongName[0] != EOS )
        {
          // check if there are long names with relative path information
          // (a backslash within the long file name is used as indicator)
          if ( !pIda->fRelPathInfo )
          {
            pIda->fRelPathInfo = (strchr( pIda->szLongName, BACKSLASH ) != NULL);
          } /* endif */
          OEMTOANSI( pIda->szLongName );
          sInsertItem = INSERTITEM( hwnd, ID_DOCEXP_DOCS_LB, pIda->szLongName );
        }
        else
        {
          sInsertItem = INSERTITEM( hwnd, ID_DOCEXP_DOCS_LB, pIda->szName );
        } /* endif */

        // set item handle to index of document in invisible document lb
        if ( sInsertItem >= 0 )
        {
          SETITEMHANDLE( hwnd, ID_DOCEXP_DOCS_LB, sInsertItem, sItem );
        } /* endif */
      } /* endwhile */
      {
        HWND hwndLB;

        hwndLB = GetDlgItem(hwnd, ID_DOCEXP_DOCS_LB);
        UtlSetHorzScrollingForLB(hwndLB);
      }
    } /* endif */

    if ( fOK )
    {
      //select all items in export listbox
      sItem = QUERYITEMCOUNT( hwnd, ID_DOCEXP_DOCS_LB );
      while ( sItem-- )
      {
        SELECTITEM( hwnd, ID_DOCEXP_DOCS_LB, sItem );
      } /* endwhile */

      //set entry fields length limit
      SETTEXTLIMIT( hwnd, ID_DOCEXP_TARGETPATH_CB, MAX_PATH144 - 1 );
      SETTEXTLIMIT( hwnd, ID_DOCEXP_SOURCEPATH_CB, MAX_PATH144 - 1 );
      SETTEXTLIMIT( hwnd, ID_DOCEXP_SNOMATCHPATH_CB, MAX_PATH144 - 1 );
      SETTEXTLIMIT( hwnd, ID_DOCEXP_REVMARK_CB, MAX_FNAME - 1 );

      // handle fields for validation format export
      {
        int i = 0;

        SetCtrlFnt(hwnd, GetCharSet(), ID_DOCEXP_VALFORMAT_PATH_CB, 0 );

        SETTEXTLIMIT( hwnd, ID_DOCEXP_VALFORMAT_PATH_CB, MAX_LONGPATH - 1 );
        SETTEXT( hwnd, ID_DOCEXP_VALFORMAT_PATH_CB, ppropImex->szSavedDlgFExpoValPath );
        while ( VALEXPFORMATS[i][0] )
        {
          CBINSERTITEMEND( hwnd, ID_DOCEXP_VALFORMAT_FORMAT_CB, VALEXPFORMATS[i] );
          i++;
        } /*endwhile */
        CBSELECTITEM( hwnd, ID_DOCEXP_VALFORMAT_FORMAT_CB, ppropImex->sSavedDlgFExpoValFormat );

        if ( ppropImex->fSavedDlgFExpoValProofFormat )
        {
          CLICK( hwnd, ID_DOCEXP_VALFORMAT_PROOF_RB );
          ppropImex->fSavedDlgFExpoValProofFormat = TRUE;
          ppropImex->fSavedDlgFExpoValValFormat = FALSE;
        }
        else
        {
          CLICK( hwnd, ID_DOCEXP_VALFORMAT_VALID_RB );
          ppropImex->fSavedDlgFExpoValProofFormat = FALSE;
          ppropImex->fSavedDlgFExpoValValFormat = TRUE;
        } /* endif */

        pIda->fValExportAutoMatch   = ppropImex->fSavedDlgFExpoValAutoMatch;
        pIda->fValExportModAutoMatch = ppropImex->fSavedDlgFExpoValModAutoMatch;
        pIda->fValExportNewMatch   =  ppropImex->fSavedDlgFExpoValNewMatch;
        pIda->fValExportProtMatch  = ppropImex->fSavedDlgFExpoValProtMatch;
        pIda->fValExportNotTransl  = ppropImex->fSavedDlgFExpoValNotTransl;
        pIda->fValExportFuzzyMatch = ppropImex->fSavedDlgFExpoValFuzzyMatch;
        pIda->fValExportExactMatch = ppropImex->fSavedDlgFExpoValExactMatch;
        pIda->fValExportModExactMatch = ppropImex->fSavedDlgFExpoValModExactMatch;
        pIda->fValExportGlobMemMatch  = ppropImex->fSavedDlgFExpoValGlobMemMatch;
        pIda->fValExportMachMatch  = ppropImex->fSavedDlgFExpoValMachMatch;
        pIda->fValExportReplMatch  = ppropImex->fSavedDlgFExpoValReplMatch;

        pIda->fValExportInclCount  = ppropImex->fSavedDlgFExpoValInclCount;
        pIda->fValExportInclExisting = ppropImex->fSavedDlgFExpoValInclExisting;
        pIda->fValExportMismatchOnly = ppropImex->fSavedDlgFExpoValMismatchOnly;
        pIda->fValExportLinksImages = ppropImex->fSavedDlgFExpoValLinksImages;
        pIda->fValExportExactFromManual = ppropImex->fSavedDlgFExpoValExactFromManual;
        pIda->fValExportTransOnly = ppropImex->fSavedDlgFExpoValTransOnly;

        DocValFormatSetMatchStates( hwnd, pIda, ppropImex->fSavedDlgFExpoValValFormat, FALSE );

        if ( ppropImex->usSavedDlgFExpoValRemoveTagging == 0 )
        {
          // not yet set, use current default
          pIda->fValExportRemoveTagging = TRUE;
        }
        else if ( ppropImex->usSavedDlgFExpoValRemoveTagging == 1 )
        {
          // 1 = remove tagging
          pIda->fValExportRemoveTagging = TRUE;
        }
        else
        {
          // anything else = leave tagging as-is
          pIda->fValExportRemoveTagging = FALSE;
        } /* endif */
        SETCHECK( hwnd, ID_DOCEXP_VALFORMAT_REMOVETAGGING_CHK, pIda->fValExportRemoveTagging );
        SETCHECK( hwnd, ID_DOCEXP_VALFORMAT_LINKSIMAGES_CHK, pIda->fValExportLinksImages );
        SETCHECK( hwnd, ID_DOCEXP_VALFORMAT_MANUALEXACT_CHK, pIda->fValExportExactFromManual );
        SETCHECK( hwnd, ID_DOCEXP_VALFORMAT_TRANSONLY_CHK, pIda->fValExportTransOnly );
        
        if ( QUERYITEMCOUNT( hwnd, ID_DOCEXP_DOCS_LB ) > 1 )
        {
          SETCHECK( hwnd, ID_DOCEXP_VALFORMAT_COMBINE_CHK, ppropImex->fSavedDlgFExpoValCombine );
          if ( !ppropImex->fSavedDlgFExpoValCombine )
          {
            ENABLECTRL( hwnd, ID_DOCEXP_VALFORMAT_NAME_TEXT, FALSE );
            ENABLECTRL( hwnd, ID_DOCEXP_VALFORMAT_NAME_EF, FALSE );
            ENABLECTRL( hwnd, ID_DOCEXP_VALFORMAT_NAMEBROWSE_PB, FALSE );
          } /* endif */
        }
        else
        {
          ENABLECTRL( hwnd, ID_DOCEXP_VALFORMAT_COMBINE_CHK, FALSE );
        } /* endif */

        UtlLoadLastUsedStrings( hwnd, ID_DOCEXP_VALFORMAT_MANAGER_CB, "VALDOCPROJMGR" );
        UtlLoadLastUsedStrings( hwnd, ID_DOCEXP_VALFORMAT_TRANSLATOR_CB, "VALDOCTRANSL" );

      }

      // fill conversion combobox
      //UtlHandleConversionStrings( CONVLOAD_MODE,
      //                            WinWindowFromID( hwnd, ID_DOCEXP_CONV_CB ),
      //                            NULL, NULL, NULL  );
      //LOADSTRING( hab, hResMod, SID_DOCEXP_USEDOCCONV, pIda->szUseDocConv );
      //CBINSERTITEM( hwnd, ID_DOCEXP_CONV_CB, pIda->szUseDocConv );
      //CBINSERTITEM( hwnd, ID_DOCEXP_CONV_CB, EMPTY_STRING );

      SetCtrlFnt(hwnd, GetCharSet(), ID_DOCEXP_DOCS_LB,
                 ID_DOCEXP_TARGETPATH_CB );
      SetCtrlFnt(hwnd, GetCharSet(), ID_DOCEXP_SOURCEPATH_CB,
                 ID_DOCEXP_SNOMATCHPATH_CB );
       UtlLoadLastUsedStrings( hwnd, ID_DOCEXP_SOURCEPATH_CB, DOCEXPSOURCELASTUSED ); 
       UtlLoadLastUsedStrings( hwnd, ID_DOCEXP_SNOMATCHPATH_CB, DOCEXPSNOMATCHLASTUSED ); 

      //get available drives
      UtlGetDriveList( (PBYTE)pIda->szDrives );

      //UCD_ONE_PROPERTY
      //
      // get Export format
      //
      pIda->sExportFormatID  = ppropImex->sSavedDlgFExpoFormat;

      // select last used drive if it is valid
      if ( (strchr(pIda->szDrives, toupper(ppropImex->cSavedDlgFExpoDrive))) &&
           (ppropImex->cSavedDlgFExpoDrive != 0) )
      {
        //save drive to ida
        sprintf( pIda->szSelectedDrive, "%c:",
                 ppropImex->cSavedDlgFExpoDrive );
      }
      else
      {
        //save drive to ida
        sprintf( pIda->szSelectedDrive, "%c:", pIda->szDrives[0] );
      } /*end if*/

      // select last used drive if it is valid
      // Translation
      //
      if ( (strchr(pIda->szDrives, toupper(ppropImex->cSavedDlgFExpoDriveTrans))) &&
           (ppropImex->cSavedDlgFExpoDriveTrans != 0) )
      {
        //save drive to ida
        sprintf( pIda->szSelectedTransDrive, "%c:",
                 ppropImex->cSavedDlgFExpoDriveTrans );
      }
      else
      {
        //save drive to ida
        sprintf( pIda->szSelectedTransDrive, "%c:", pIda->szDrives[0] );
      } /*end if*/

      // select last used drive if it is valid
      // Original
      //
      if ( (strchr(pIda->szDrives, toupper(ppropImex->cSavedDlgFExpoDriveOrg))) &&
           (ppropImex->cSavedDlgFExpoDriveOrg != 0) )
      {
        //save drive to ida
        sprintf( pIda->szSelectedOrgDrive, "%c:",
                 ppropImex->cSavedDlgFExpoDriveOrg );
      }
      else
      {
        //save drive to ida
        sprintf( pIda->szSelectedOrgDrive, "%c:", pIda->szDrives[0] );
      } /*end if*/

      // select last used drive if it is valid
      // SNomatch
      //
      if ( (strchr(pIda->szDrives, ppropImex->cSavedDlgFExpoDriveSno)) &&
           (ppropImex->cSavedDlgFExpoDriveSno != 0) )
      {
        //save drive to ida
        sprintf( pIda->szSelectedSnoDrive, "%c:",
                 ppropImex->cSavedDlgFExpoDriveSno );
      }
      else
      {
        //save drive to ida
        sprintf( pIda->szSelectedSnoDrive, "%c:", pIda->szDrives[0] );
      } /*end if*/


      //display target path from import/export properties
      strcpy(pIda->szString,pIda->szSelectedTransDrive);

      if (pIda->sExportFormatID != ID_DOCEXP_INTFORMAT_RB )
      {
        strcat(pIda->szString,ppropImex->szSavedDlgFExpoTPath );
      }//end if

      SETTEXT( hwnd, ID_DOCEXP_TARGETPATH_CB, pIda->szString);

      //display source path from import/export properties

      strcpy(pIda->szString,pIda->szSelectedOrgDrive);
      strcat(pIda->szString,ppropImex->szSavedDlgFExpoSPath );


      SETTEXT( hwnd, ID_DOCEXP_SOURCEPATH_CB,
               pIda->szString);

      //display snomatch path from import/export properties

      strcpy(pIda->szString,pIda->szSelectedSnoDrive);
      strcat(pIda->szString,ppropImex->szSavedDlgFExpoNPath );


      SETTEXT( hwnd, ID_DOCEXP_SNOMATCHPATH_CB, pIda->szString);

      // select last used autoradio button
      // convert EQF_BOOL type of saved format (in older properties)
      // to corresponding ID value
      if ( ppropImex->sSavedDlgFExpoFormat == TRUE )
      {
        ppropImex->sSavedDlgFExpoFormat = ID_DOCEXP_INTFORMAT_RB;
      }
      else if ( ppropImex->sSavedDlgFExpoFormat == FALSE )
      {
        ppropImex->sSavedDlgFExpoFormat = ID_DOCEXP_EXTFORMAT_RB;
      } /* endif */

      // disable "with relative path" option if there are no documents with
      // relative path information
      if ( !pIda->fRelPathInfo )
      {
        ENABLECTRL( hwnd, ID_DOCEXP_IMPPATH_RB, FALSE );
        // change to external format if relative path format was the last
        // used format
        if ( ppropImex->sSavedDlgFExpoFormat == ID_DOCEXP_IMPPATH_RB )
        {
          ppropImex->sSavedDlgFExpoFormat = ID_DOCEXP_EXTFORMAT_RB;
        } /* endif */
      } /* endif */

      // select corresponding radio-button
      CLICK( hwnd, ppropImex->sSavedDlgFExpoFormat );

      PostMessage( hwnd, WM_COMMAND,
                   MP1FROMSHORT(ppropImex->sSavedDlgFExpoFormat),
                   MP2FROM2SHORT( 0, BN_CLICKED ) );

      /*************************************************************/
      /* Check if there are SNOMATCH documents, if not disable     */
      /* SNOMATCH checkbox                                         */
      /*************************************************************/
      ENABLECTRL( hwnd, ID_DOCEXP_SNOMATCH_CHK, FALSE );  /* Default */
      sItem = QUERYITEMCOUNT( hwnd, ID_DOCEXP_DOCS_LB );
      pIda->fSNoMatchExist = FALSE;
      while ( sItem-- )
      {
        SHORT sDocIndex;
        QUERYITEMTEXT( hwnd, ID_DOCEXP_DOCS_LB, sItem, pIda->szLongName );
        sDocIndex = (SHORT)QUERYITEMHANDLE( hwnd, ID_DOCEXP_DOCS_LB,
                                            sItem );
        QUERYITEMTEXTHWND( pIda->hwndExportListbox, sDocIndex,
                           pIda->szName );
        UtlMakeEQFPath( pIda->szSource,
                        pIda->szFolderObjName[0],
                        DIRSEGNOMATCH_PATH,
                        pIda->szFolderName );
        strcat( pIda->szSource, BACKSLASH_STR );
        strcat( pIda->szSource, pIda->szName );
        if ( UtlFileExist( pIda->szSource ) )
        {
          ENABLECTRL( hwnd, ID_DOCEXP_SNOMATCH_CHK, TRUE );
          sItem = 0;               // force end of loop
          pIda->fSNoMatchExist = TRUE;
        } /* endif */
      } /* endwhile */

      /*************************************************************/
      /* Fill revision mark dropdown list with the names of        */
      /* available revision marks                                  */
      /*************************************************************/
      UtlMakeEQFPath( pIda->szSource, NULC, TABLE_PATH, NULL );
      strcat( pIda->szSource, BACKSLASH_STR );
      strcat( pIda->szSource, DEFAULT_PATTERN_NAME );
      strcat( pIda->szSource, EXT_OF_REVISION_MARK );
      UtlLoadFileNames( pIda->szSource,
                        FILE_NORMAL,
                        WinWindowFromID( hwnd, ID_DOCEXP_REVMARK_CB ),
                        NAMFMT_NODRV | NAMFMT_NOEXT );

      /*************************************************************/
      /* Select last used revision mark                            */
      /*************************************************************/
      if ( ppropImex->szSavedRevMark[0] )
      {
        if ( CBSEARCHITEM( hwnd,
                           ID_DOCEXP_REVMARK_CB,
                           ppropImex->szSavedRevMark ) != LIT_NONE )
        {
          SETTEXT( hwnd, ID_DOCEXP_REVMARK_CB, ppropImex->szSavedRevMark );
        } /* endif */
      } /* endif */

      /* Select last used settings of files to export */
      if ( ppropImex->fSavedDlgFExpoTranslation )
      {
        SETCHECK_TRUE( hwnd, ID_DOCEXP_TARGET_CHK);
        DocExpControl( hwnd, ID_DOCEXP_TARGET_CHK, BN_CLICKED );
      } /* endif */

      // select last used setting of RevMark checkbox
      if ( ppropImex->fSavedDlgFExpoWithRevMark )
      {
        SETCHECK_TRUE( hwnd, ID_DOCEXP_REVMARK_CHK );
        DocExpControl( hwnd, ID_DOCEXP_REVMARK_CHK, BN_CLICKED );
      } /* endif */

      // select last used setting of TVT tracking ID checkbox
      if ( ppropImex->fSavedDlgFExpoWithTrackID )
      {
//      SETCHECK_TRUE( hwnd, ID_DOCEXP_TRACKID_CHK );     6-3-16
//      DocExpControl( hwnd, ID_DOCEXP_TRACKID_CHK, BN_CLICKED );
      } /* endif */

      if ( ppropImex->fSavedDlgFExpoOriginal )
      {
        SETCHECK_TRUE( hwnd, ID_DOCEXP_SOURCE_CHK);
        DocExpControl( hwnd, ID_DOCEXP_SOURCE_CHK, BN_CLICKED );
      } /* endif */

      if ( ppropImex->fSavedDlgFExpoSNOMATCH && pIda->fSNoMatchExist )
      {
        SETCHECK_TRUE( hwnd, ID_DOCEXP_SNOMATCH_CHK );
        DocExpControl( hwnd, ID_DOCEXP_SNOMATCH_CHK, BN_CLICKED );
      } /* endif */

    }/*end if fOK*/
  }/*end if fOK*/

  if ( !fOK )
  {
    //post message to remove dialog
    WinPostMsg( hwnd, WM_EQF_CLOSE, NULL, NULL );
  } /* endif */

  if ( pIda ) pIda->fInit = FALSE;

  return( mResult );
} /* end of DocExpInit */




//------------------------------------------------------------------------------
// check source pathn normalize it and copy it to target path                 --
//------------------------------------------------------------------------------
BOOL CheckPath( PSZ pszTargetPath, PSZ pszSourcePath )
{
  BOOL    fOK = TRUE;
  CHAR    szName[MAX_FNAME];           //string for filename
  CHAR    szExt[MAX_FEXT];             //string for extension
  CHAR    szDrive[MAX_DRIVE];          //string for drive
  USHORT  usRc;
  USHORT  usMsgId1;                    //message id
  USHORT  usMsgId2;                    //message id
  PSZ     psz;

  usMsgId1 = NO_EXPORTPATH;
  usMsgId2 = WRONG_EXPORTDIR;

  if ( *pszSourcePath == EOS )             //source path is empty
  {
    pszTargetPath[0] = BACKSLASH;
    pszTargetPath[1] = EOS;
  }
  else
  {
    fOK = UtlCheckPath( pszSourcePath, 0L, NULL  );

    if ( !fOK )
    {
      UtlError( usMsgId2, MB_CANCEL, 0, NULL, EQF_ERROR );
    }
    else
    {
      //split directory, filename and extension from source path
      _splitpath( pszSourcePath,
                  szDrive,
                  pszTargetPath,
                  szName,
                  szExt
                );
      //if szName contains only blanks, empty szName
      psz = strchr( szName, ' ' );
      if ( psz != NULL ) szName[0] = EOS;

      if ( (szDrive[0] != EOS ) || (szExt[0] != EOS )  ||
           (szName[0] != EOS ) )
      {
        UtlError( usMsgId2, MB_CANCEL, 0, NULL, EQF_ERROR );
        fOK = FALSE;
      }
      else
      {
        CHAR szSysDir[20];            // buffer for TM system directory name

        //normalize path
        if ( *pszTargetPath != BACKSLASH )
        {
          memmove( pszTargetPath +1, pszTargetPath,
                   strlen(pszTargetPath ) );
          *pszTargetPath = BACKSLASH;
        }/*endif*/

        //get system directory
        UtlMakeEQFPath( szSysDir, NULC, SYSTEM_PATH, NULL );
        strcat( szSysDir, BACKSLASH_STR );

        //compare if target path contains as first directory the system path
        usRc = (USHORT)strnicmp( pszTargetPath, szSysDir + 2, strlen(szSysDir) - 2 );

        //if target path contains as first directory the system path
        //display error message
        if ( usRc == 0 )
        {
          UtlError( ERROR_EXPORT_TO_SYSTEM, MB_CANCEL, 0, NULL, EQF_ERROR );
          fOK = FALSE;
        } /* endif */
      }/*endif*/
    } /* endif */
  } /* endif */

  return( fOK );
}/*end CheckPath*/


// data area for document export in internal format
typedef struct _DOCEXPINTERNALDATA
{
  CHAR       szDocLog[MAX_LONGPATH];             // name of document history log
  CHAR       szSearchPath[MAX_LONGPATH];       
  CHAR       szExpFolderName[MAX_LONGFILESPEC];  // short folder name to be used for export
  CHAR       szFolderLongName[MAX_LONGFILESPEC]; // long name of folder
  CHAR       szDocLongName[MAX_LONGFILESPEC];    // long name of document being exported
  CHAR       szPackName[MAX_LONGPATH];           // package name
  CHAR       szString[MAX_LONGPATH];             // buffer for strings and file names       
  DOCEXPHEADER Header;                           // header of exported package
  DOCEXPORTHIST HistoryData;                     // data for history log
} DOCEXPINTERNALDATA, *PDOCEXPINTERNALDATA;

// add a single document file to the package
BOOL DocExpAddSingleFile
( 
  PSZ pszFolderObjName,                          // folder object name
  PSZ pszDocShortName,                           // document short name
  PSZ pszString,                                 // pointer to a buffer for file names
  USHORT usPathID,                               // symbolic ID for the document path
  USHORT usPackFileType,                         // file type in the package 
  PVOID pPackCtrl,                               // package handle               
  HWND  hwndErrMsg,                              // window handle for error messages
  BOOL  fMustExist                               // TRUE = show error when file does not exist
)
{
  ULONG ulRC = 0;
  BOOL fOK = TRUE;

  UtlMakeEQFPath( pszString, pszFolderObjName[0], usPathID, UtlGetFnameFromPath( pszFolderObjName ) );
  strcat( pszString, BACKSLASH_STR );
  strcat( pszString, pszDocShortName );
  if ( fMustExist )
  {
    ulRC = UtlPackAddFile( pPackCtrl, pszString, usPackFileType, FILE_COMPRESS );
    fOK = UtlPackHandleError( ulRC, pszString, hwndErrMsg );
  }
  else
  {
    if ( UtlFileExist( pszString ) )
    {
      ulRC = UtlPackAddFile( pPackCtrl, pszString, usPackFileType, FILE_COMPRESS );
      fOK = UtlPackHandleError( ulRC, pszString, hwndErrMsg );
    } /* endif */
  } /* endif */
  return( fOK );
}

// export a document in internal format
//  pIda->szString contains document name
// cv
//static USHORT DocExpExportInternal( PDOCEXPIDA pIda )
USHORT DocExpExportInternal
( 
  PSZ        pszFolderObjName,       // folder object name  
  PSZ        pszDocLongName,         // long name of exported document
  PSZ        pszDocShortName,         // long name of exported document
  CHAR       chTargetDrive,          // target drive letter
  HWND       hwndErrMsg,             // parent window handle for error messages
  PBOOL      pfOverwrite,            // pointer to "overwrite existing files" flag, is changed to TRUE when user selects yes-to-all
  PSZ        pszDocPackName          // NULL or pointer to a buffer receiving the package name actually being used for the export (may be different from document short name)

)
{
  USHORT     usResult=UNLOAD_OK;      // return of function
  USHORT     usResponse;              // response from UtlError calls
  ULONG      ulRC = 0;                // USHORT return code
  BOOL       fOK = TRUE;              // error flag and function return
  USHORT     usCompleted;             // export completion ratio
  PSZ        apszErrParm[4];          // argument for UtlError calls
  BOOL       fNoMarkup = FALSE;       // doc props have no setting for markup
  BOOL       fNoMemory = FALSE;       // doc props have no setting for memory
  BOOL       fNoSrcLng = FALSE;       // doc props have no setting for sourcelng
  BOOL       fNoTgtLng = FALSE;       // doc props have no setting for targetlng
  BOOL       fNoEditor = FALSE;       // doc props have no setting for editor
  PPROPDOCUMENT pPropDoc = NULL;      // ptr to in-memory copy of properties
  ULONG      ulPropDocSize = 0;       // size of in-memory copy of properties
  BOOL       fDocPropsModified = FALSE;  // TRUE = props have been changed
  PDOCEXPINTERNALDATA pData = NULL;   // pointer to our data area
  PVOID      pPackCtrl = NULL;        // package control handle

  // allocate our data area
  if ( !UtlAllocHwnd( (PVOID *)&pData, 0L, (LONG) sizeof(DOCEXPINTERNALDATA), ERROR_STORAGE, hwndErrMsg ) )
  {
    return( UNLOAD_NOTOK );
  } /* endif */

  // fill-in some values
  pData->Header.usHeaderSize = sizeof(DOCEXPHEADER);
  pData->Header.usHeaderRel  = 1;

  // get folder long name
  if ( fOK )
  {
    SubFolObjectNameToName( pszFolderObjName, pData->szFolderLongName );
  } /* endif */

  // search correct target folder
  if ( fOK )
  {
    WIN32_FIND_DATA FindData;
    HANDLE hDir;
    BOOL fFound = FALSE;

    // use actual folder short name as default
    strcpy( pData->szExpFolderName, UtlGetFnameFromPath( pszFolderObjName) );

    // loop over all folders and check folder long name
    UtlMakeEQFPath( pData->szSearchPath, chTargetDrive, EXPORT_PATH, NULL );
    strcat( pData->szSearchPath, BACKSLASH_STR );
    strcat( pData->szSearchPath, "*" );
    strcat( pData->szSearchPath, EXT_FOLDER_MAIN );
    hDir = FindFirstFile( pData->szSearchPath, &FindData );
    if ( hDir != INVALID_HANDLE_VALUE )
    {
      do
      {
        PSZ pLongFile = NULL;
        USHORT usLen = 0;

        // if found file is a directory ...
        if ( FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
        {
          // load folder long name dummy file (if any)
          UtlMakeEQFPath( pData->szSearchPath, chTargetDrive, EXPORT_PATH, NULL );
          strcat( pData->szSearchPath, BACKSLASH_STR );
          strcat( pData->szSearchPath, FindData.cFileName );
          strcat( pData->szSearchPath, BACKSLASH_STR );
          strcat( pData->szSearchPath, FindData.cFileName );

          if ( UtlLoadFile( pData->szSearchPath, (PVOID *)&pLongFile, &usLen, FALSE, FALSE ) )
          {
            int iLen = strlen( FOLNAMEPREFIX );

            if ( strnicmp( pLongFile, FOLNAMEPREFIX, iLen ) == 0 )
            {
              PSZ pszLongName = pLongFile + iLen;
              if( strcmp( pData->szFolderLongName, pszLongName  ) == 0 )
              {
                strcpy( pData->szExpFolderName, FindData.cFileName );
                fFound = TRUE;
              } /* endif */
            } /* endif */

            UtlAlloc( (PVOID *)&pLongFile, 0L, 0L, NOMSG );
          } /* endif */
        } /* endif */
      } while ( !fFound && FindNextFile( hDir, &FindData  ) );
      FindClose( hDir );
    } /* endif */
  } /* endif */

  //--- if o.k. build list of files to be exported ---
  if ( fOK )
  {
    // check if there is more than one package file with
    // the same name (w/o the index number) and overwrite the
    // correct one (the one with the same long name)
    if ( (pszDocLongName[0] != EOS) && UtlIsLongFileName(pszDocLongName) )
    {
      CHAR szShortName[MAX_FILESPEC];

      // setup package search path
      UtlLongToShortName( pszDocLongName, szShortName );

      UtlMakeEQFPath( pData->szSearchPath, chTargetDrive, EXPORT_PATH, NULL );

      strcat( pData->szSearchPath, BACKSLASH_STR );
      strcat( pData->szSearchPath, pData->szExpFolderName );
      strcat( pData->szSearchPath, BACKSLASH_STR );
      strcat( pData->szSearchPath, szShortName );
      strcat( pData->szSearchPath, DEFAULT_PATTERN_EXT );

      // look for packages having the same short name
      {
        USHORT usDosRC = NO_ERROR;         // return code of called DOS functions
        USHORT usCount = 1;                // number of files requested
        HDIR   hDir = HDIR_CREATE;         // file find handle
//          BOOL   fOK;
        BOOL   fFound = FALSE;             // found package having same name
//          CHAR   szShortNameCaseMismatch[MAX_FILESPEC]; // buffer for short name which does not match case
        FILEFINDBUF stResultBuf;                // DOS file find structure

        usDosRC = UtlFindFirst( pData->szSearchPath, &hDir, FILE_NORMAL,
                                &stResultBuf, sizeof(stResultBuf),
                                &usCount, 0L, FALSE );

        while ( !fFound && (usDosRC == NO_ERROR) && usCount )
        {
          PVOID pTempPackCtrl;           // ptr to package control structure


          pTempPackCtrl = UtlPackOpenPackage( chTargetDrive, pData->szExpFolderName, NULL, RESBUFNAME(stResultBuf), hwndErrMsg );

          if ( pTempPackCtrl )
          {
            // get package header (required for document long name)
            memset( &(pData->Header), 0, sizeof(pData->Header) );
            UtlPackReadHeader( pTempPackCtrl, (PBYTE)&(pData->Header), (ULONG)sizeof(pData->Header) );
            if ( stricmp( pData->Header.szLongName, pszDocLongName) == 0 )
            {
              fFound = TRUE;
            } /* endif */
            UtlPackClosePackage( pTempPackCtrl );
          } /* endif */

          // try next document
          if ( !fFound )
          {
            usCount = 1;
            usDosRC = UtlFindNext( hDir, &stResultBuf, sizeof(stResultBuf), &usCount, FALSE );
          } /* endif */
        } /* endwhile */
        UtlFindClose( hDir, FALSE );

        if ( fFound )
        {
          // use name of found document package
          strcpy( pData->szPackName, RESBUFNAME(stResultBuf) );
        }
        else
        {
          // find a unique name if there is no package file for
          // the document
          SHORT i = 0;                       // counter for document extension
          LONG  lPathLen;
          UtlMakeEQFPath( pData->szSearchPath, chTargetDrive, EXPORT_PATH, NULL );
          strcat( pData->szSearchPath, BACKSLASH_STR );
          strcat( pData->szSearchPath, pData->szExpFolderName );
          strcat( pData->szSearchPath, BACKSLASH_STR );
          strcat( pData->szSearchPath, szShortName );
          lPathLen = strlen( pData->szSearchPath );

          do
          {
            sprintf( pData->szSearchPath+lPathLen, ".%3.3X", i++ );
          } while ( (i < 1000) && UtlFileExist( pData->szSearchPath ) ); /* enddo */

          strcpy( pData->szPackName, UtlGetFnameFromPath(pData->szSearchPath) );
        } /* endif */
      }
    }
    else
    {
      // use document name as package name
      strcpy( pData->szPackName, pszDocShortName );
    } /* endif */

    // pass packahe name back to calling function
    if ( pszDocPackName != NULL )
    {
      strcpy( pszDocPackName, pData->szPackName );
    } /* endif */

    //--- initialize package control structure ---
    pPackCtrl = UtlPackInit( chTargetDrive, pData->szExpFolderName, NULL, pData->szPackName, 0, NULL, NULLHANDLE );
    fOK = pPackCtrl != NULL;

    //--- fill fields in package header ---
    if ( fOK )
    {
      UtlTime( &(pData->Header.lDateTime) );
    } /* endif */

  } /* endif */

  // add document to list
  if ( fOK )
  {
    //
    // add property file to list of package files
    //
    fOK = DocExpAddSingleFile( pszFolderObjName, pszDocShortName, pData->szString, PROPERTY_PATH, DOCUMENT_PROP_FILE, pPackCtrl, hwndErrMsg, TRUE );

    // modify document property file to contain settings
    // of folder if no specific settings done
    if ( fOK )
    {
      // load the document property file into memory
      if ( UtlLoadFileL( pData->szString, (PVOID *)&pPropDoc, &ulPropDocSize, FALSE, FALSE ) )
      {
        // remember document property states
        fNoMarkup = pPropDoc->szFormat[0] == EOS;
        fNoMemory = pPropDoc->szMemory[0] == EOS;
        fNoSrcLng = pPropDoc->szSourceLang[0] == EOS;
        fNoTgtLng = pPropDoc->szTargetLang[0] == EOS;
        fNoEditor = pPropDoc->szEditor[0] == EOS;
        strcpy( pData->Header.szLongName, pPropDoc->szLongName );
        // get folder settings for those fields not set in
        // document properties
        if ( fNoMarkup || fNoMemory || fNoSrcLng || fNoTgtLng )
        {
          if ( FolQueryInfo2( pszFolderObjName,
                              fNoMemory ? pPropDoc->szLongMemory : NULL,
                              fNoMarkup ? pPropDoc->szFormat : NULL,
                              fNoSrcLng ? pPropDoc->szSourceLang : NULL,
                              fNoTgtLng ? pPropDoc->szTargetLang : NULL,
                              fNoEditor ? pPropDoc->szEditor : NULL,
                              FALSE ) == NO_ERROR )
          {
            // write modified property file to disk
            if ( UtlWriteFileL( pData->szString, ulPropDocSize, (PVOID)pPropDoc, FALSE ) == NO_ERROR )
            {
              fDocPropsModified = TRUE;
            } /* endif */
          } /* endif */
        } /* endif */

      } /* endif */
    } /* endif */


    // add source file to list of package files
    if ( fOK ) fOK = DocExpAddSingleFile( pszFolderObjName, pszDocShortName, pData->szString, DIRSOURCEDOC_PATH, DOCUMENT_SRC_FILE, pPackCtrl, hwndErrMsg, TRUE );

    // add segmented source file (if any) to list of package files
    if ( fOK ) fOK = DocExpAddSingleFile( pszFolderObjName, pszDocShortName, pData->szString, DIRSEGSOURCEDOC_PATH, DOCUMENT_SEGSRC_FILE, pPackCtrl, hwndErrMsg, FALSE );

    // add segmented target file (if any) to list of package files
    if ( fOK ) fOK = DocExpAddSingleFile( pszFolderObjName, pszDocShortName, pData->szString, DIRSEGTARGETDOC_PATH, DOCUMENT_SEGTGT_FILE, pPackCtrl, hwndErrMsg, FALSE );

    // add EA data file (if any) to list of package files
    if ( fOK ) fOK = DocExpAddSingleFile( pszFolderObjName, pszDocShortName, pData->szString, EADATA_PATH, DOCUMENT_EADATA_FILE, pPackCtrl, hwndErrMsg, FALSE );

    // add RTF file (if any) to list of package files
    if ( fOK ) fOK = DocExpAddSingleFile( pszFolderObjName, pszDocShortName, pData->szString, DIRSEGRTF_PATH, DOCUMENT_SRC_FILE, pPackCtrl, hwndErrMsg, FALSE );

    // add MT log file (if any) to list of package files
    if ( fOK ) fOK = DocExpAddSingleFile( pszFolderObjName, pszDocShortName, pData->szString, MTLOG_PATH, DOCUMENT_MTLOG_FILE, pPackCtrl, hwndErrMsg, FALSE );

    // add MISC file (if any) to list of package files
    if ( fOK ) fOK = DocExpAddSingleFile( pszFolderObjName, pszDocShortName, pData->szString, MISC_PATH, DOCUMENT_MISC_FILE, pPackCtrl, hwndErrMsg, FALSE );

    // add entity file (if any) to list of package files
    if ( fOK ) fOK = DocExpAddSingleFile( pszFolderObjName, pszDocShortName, pData->szString, ENTITY_PATH, DOCUMENT_ENTITY_FILE, pPackCtrl, hwndErrMsg, FALSE );

    // add METADATA file (if any) to list of package files
    if ( fOK ) fOK = DocExpAddSingleFile( pszFolderObjName, pszDocShortName, pData->szString, METADATA_PATH, DOCUMENT_METADATA_FILE, pPackCtrl, hwndErrMsg, FALSE );

    // add XLIFF file (if any) to list of package files
    if ( fOK ) fOK = DocExpAddSingleFile( pszFolderObjName, pszDocShortName, pData->szString, XLIFF_PATH, DOCUMENT_XLIFF_FILE, pPackCtrl, hwndErrMsg, FALSE );
  } /* endif */

  if ( fOK )
  {
    //--- add user header area
    ulRC = UtlPackAddHeader( pPackCtrl, (PBYTE)&(pData->Header), pData->Header.usHeaderSize ); 
    fOK = UtlPackHandleError( ulRC, NULL, NULLHANDLE );
  } /* endif */

  /*******************************************************************/
  /* Check if a package file exists                                  */
  /*******************************************************************/
  if ( fOK )
  {
    USHORT usRC;

    /*****************************************************************/
    /* build fully qualified file name of package file               */
    /*****************************************************************/
    UtlMakeEQFPath( pData->szString, chTargetDrive, EXPORT_PATH, NULL );
    strcat( pData->szString, BACKSLASH_STR );
    strcat( pData->szString, pData->szExpFolderName );
    strcat( pData->szString, BACKSLASH_STR );
    strcat( pData->szString, pData->szPackName );


    /* Handle existing files                                         */
    if ( CheckExistence( pData->szString, FALSE, FILE_NORMAL, &usRC ) )
    {
      /**************************************************************/
      /* Get replace confirmation                                   */
      /**************************************************************/
      if ( *pfOverwrite )
      {
        usResponse = MBID_YES;
      }
      else
      {
        char szDrive[3];
        if ( pData->Header.szLongName[0] != EOS )
        {
          apszErrParm[0] = pData->Header.szLongName;
        }
        else
        {
          apszErrParm[0] = pszDocShortName;
        } /* endif */
        szDrive[0] = chTargetDrive;
        szDrive[1] = ':';
        szDrive[2] = EOS;
        apszErrParm[1] = szDrive;
        usResponse = UtlErrorHwnd( ERROR_EXP_FILE_EXISTS_ALREADY, MB_EQF_YESTOALL | MB_DEFBUTTON3, 2, apszErrParm, EQF_WARNING, hwndErrMsg );
        if ( usResponse == MBID_EQF_YESTOALL )
        {
          *pfOverwrite = TRUE;
          usResponse = MBID_YES;
        } /* endif */
      } /* endif */
      switch ( usResponse )
      {
        case MBID_YES:
          break;

        case MBID_NO:
          fOK = FALSE;
          usResult = LOAD_NOTOK;
          break;

        default:
          usResult = LOAD_CANCEL;
          fOK = FALSE;
      } /* endswitch */
    }
    else
    {
	  ulRC = (ULONG) usRC;
      if ( ulRC == ERROR_DISK_CHANGE )
      {
        /* if disk wasn't changed in CheckExistence, don't try to write file */
        fOK = FALSE;
        usResult = UNLOAD_NOTOK;
      } /* endif */
    } /* endif */
  } /* endif */

  /************************************************************/
  /* Add export record to history log                         */
  /************************************************************/
  if ( fOK )
  {
    pData->HistoryData.sType          = INTERN_SUBTYPE;
    EQFBWriteHistLog2( pszFolderObjName, pszDocShortName, DOCEXPORT_LOGTASK, sizeof(DOCEXPORTHIST), (PVOID)&(pData->HistoryData), TRUE, hwndErrMsg, pData->szDocLongName );
  } /* endif */

  /************************************************************/
  /* Create temporary history log file for document           */
  /************************************************************/
  if ( fOK )
  {
    ULONG ulRC;
    CHAR   szFolderLog[MAX_EQF_PATH]; // name of folder history log


    // setup name of folder history log file
    UtlMakeEQFPath( szFolderLog, pszFolderObjName[0], PROPERTY_PATH, UtlGetFnameFromPath( pszFolderObjName ) );
    strcat( szFolderLog, BACKSLASH_STR  );
    strcat( szFolderLog, HISTLOGFILE );

    // Setup name for temporary document history log file
    {
      PSZ pszExt;                     // location of file name extension
      int i = 0;                      // loop counter

      strcpy( pData->szDocLog, szFolderLog );
      pszExt = strrchr( pData->szDocLog, DOT );
      if ( pszExt == NULL )
      {
        pszExt = pData->szDocLog + strlen(pData->szDocLog);
      } /* endif */
      do
      {
        i++;
        sprintf( pszExt, ".%3.3d", i );
      } while ( (i < 999) && UtlFileExist(pData->szDocLog) ); /* enddo */
    }

    // create document log file
    ulRC = FolFilterHistoryLog( szFolderLog, pData->szDocLog, pszDocShortName, TRUE, hwndErrMsg );
    fOK = (ulRC == NO_ERROR);

    // add document log file to list of package files
    if ( fOK )
    {
      ulRC = UtlPackAddFile( pPackCtrl, pData->szDocLog, HISTLOG_DATA_FILE, FILE_STORE );
      fOK = UtlPackHandleError( ulRC, pData->szDocLog, hwndErrMsg );
    }
    else
    {
      pData->szDocLog[0] = EOS;              // no document logfile created
    } /* endif */
  } /* endif */

  //--- if o.k. start export of document ---
  if ( fOK )
  {
    usCompleted = 0;
    do
    {
      ulRC = UtlPackWritePackage( pPackCtrl, &usCompleted );
    } while ( ulRC == PACK_OK_RC );

    switch ( ulRC )
    {
      case PACK_COMPLETE_RC:
        break;
      default:
        usResult = UNLOAD_CANCEL;

        // Close any open file
        UtlPackCleanup( pPackCtrl );
        pPackCtrl = NULL;

        // Delete package file
        UtlMakeEQFPath( pData->szString, chTargetDrive, EXPORT_PATH, NULL );
        strcat( pData->szString, BACKSLASH_STR );
        strcat( pData->szString, pData->szExpFolderName );
        strcat( pData->szString, BACKSLASH_STR );
        strcat( pData->szString, pData->szPackName );
        UtlDelete( pData->szString, 0L, FALSE );

        // Try to remove folder directory (will fail if directory
        // contains other package files)
        UtlMakeEQFPath( pData->szString, chTargetDrive, EXPORT_PATH, NULL );
        strcat( pData->szString, BACKSLASH_STR );
        strcat( pData->szString, pData->szExpFolderName );
        UtlRmDir( pData->szString, 0L, FALSE );

        break;
    } /* endswitch */
  } /* endif */

  // write dummy file containing folder long name to target location
  if ( fOK )
  {
    FILE *hFile;

    UtlMakeEQFPath( pData->szString, chTargetDrive, EXPORT_PATH, NULL );
    strcat( pData->szString, BACKSLASH_STR );
    strcat( pData->szString, pData->szExpFolderName );
    strcat( pData->szString, BACKSLASH_STR );
    strcat( pData->szString, pData->szExpFolderName );

    hFile = fopen( pData->szString, "w" );
    if ( hFile )
    {
      fprintf( hFile, "%s%s", FOLNAMEPREFIX, pData->szFolderLongName );
      fputc( 0, hFile );
      fclose( hFile );
    } /* endif */
  } /* endif */

  // restore original document properties if properties
  // have been changed for export
  if ( fDocPropsModified )
  {
    UtlMakeEQFPath( pData->szString, pszFolderObjName[0], PROPERTY_PATH, UtlGetFnameFromPath( pszFolderObjName ) );
    strcat( pData->szString, BACKSLASH_STR );
    strcat( pData->szString, pszDocShortName );

    if ( fNoMarkup ) pPropDoc->szFormat[0] = EOS;
    if ( fNoMemory ) pPropDoc->szMemory[0] = EOS;
    if ( fNoSrcLng ) pPropDoc->szSourceLang[0] = EOS;
    if ( fNoTgtLng ) pPropDoc->szTargetLang[0] = EOS;
    if ( fNoEditor ) pPropDoc->szEditor[0] = EOS;

    UtlWriteFileL( pData->szString, ulPropDocSize, (PVOID)pPropDoc, FALSE );
  } /* endif */
  if ( pPropDoc != NULL )
  {
    UtlAlloc( (PVOID *)&pPropDoc, 0L, 0L, NOMSG );
  } /* endif */

  /* clean up package control structure in any case; CHM 07/15/93 */
  if ( pPackCtrl )
  {
    UtlPackCleanup( pPackCtrl );
  } /* endif */

  // delete any document log file
  if ( pData->szDocLog[0] != EOS )
  {
    UtlDelete( pData->szDocLog, 0L, FALSE );
  } /* endif */

  // freee our data area
  if ( pData ) UtlAlloc( (PVOID *)&pData, 0L, 0L, NOMSG );

  return( usResult );
} /* end of DocExpExportInternal */


// import documents in internal format
//   prereqs:  pIda->szString contains document name
//             pIda->szFromFolder contains name of source folder
USHORT DocImpInternal( PDOCIMPIDA pIda )
{
  ULONG        ulRC = 0;              // return code
  USHORT       usResult = LOAD_OK;    // init return value
  PSZ          pszReplace;            //pointer to replace string UtlError
  USHORT       usResponse;            //return from UtlError
  BOOL         fFilesUnloaded = FALSE;// TRUE = files have been unloaded
  USHORT       usDummy;
  BOOL         fSourceRep = FALSE;    // TRUE = source has been replaced
  BOOL         fTargetRep = FALSE;    // TRUE = target has been replaced
  CHAR         szFromFolder[MAX_FILESPEC]; // buffer for actual from folder name used in package file

  //
  // unload document packages into temporary import directory
  //
  pIda->pPackCtrl = UtlPackOpenPackage( pIda->stFs.szDrive[0],
                                        pIda->szFromFolder,
                                        NULL,
                                        pIda->szString, NULLHANDLE );
  if ( pIda->pPackCtrl )
  {
    // get package header (required for document long name)
    memset( &(pIda->Header), 0, sizeof(pIda->Header) );
    UtlPackReadHeader( pIda->pPackCtrl, (PBYTE)&(pIda->Header),
                       (ULONG)sizeof(pIda->Header) );
    OEMTOANSI( pIda->Header.szLongName );

    // adjust document short name which may be different from the
    // package name
    {
      PFILELIST   pFileList;        // ptr to package's file list
      USHORT      usNoOfEntries;    // # of enteries in package's file list
      PFILELISTENTRY pFile;         // ptr for file list processing
      ULONG       ulPackRC;

      ulPackRC = UtlPackGetFileListPtr( pIda->pPackCtrl, &pFileList );
      if ( ulPackRC == PACK_OK_RC )
      {
        usNoOfEntries = (USHORT)pFileList->ulListUsed;
        pFile         = pFileList->pEntries;
        while ( usNoOfEntries )
        {
          if ( pFile->usFileType == DOCUMENT_PROP_FILE )
          {
            PSZ pszDocName = UtlGetFnameFromPath(pFile->pszName);
            PSZ pszExt;
            if ( pszDocName != NULL )
            {
              strcpy( pIda->szString, pszDocName );
              Utlstrccpy( pIda->stFs.szName, pszDocName, DOT );
              pszExt = strchr( pszDocName, DOT );
              if ( pszExt != NULL )
              {
                strcpy( pIda->stFs.szExt, pszExt + 1 );
              }
              else
              {
                pIda->stFs.szExt[0] = EOS;
              } /* endif */
            } /* endif */

            // get real package folder name (may be different from folder property file name...)
            {
              PSZ pszFolderPos = strchr( pFile->pszName, BACKSLASH );
              if ( pszFolderPos ) pszFolderPos = strchr( pszFolderPos + 1, BACKSLASH );
              if ( pszFolderPos )
              {
                Utlstrccpy( szFromFolder, pszFolderPos + 1, BACKSLASH );
              }
              else
              {
                strcpy( szFromFolder, pIda->szFromFolder );
              } /* endif */
            }
          } /* endif */
          usNoOfEntries--;            // skip to next entry in file list
          pFile++;
        } /* endwhile */
      } /* endif */
    }
    memset( &(pIda->Header), 0, sizeof(pIda->Header) );
    UtlPackReadHeader( pIda->pPackCtrl, (PBYTE)&(pIda->Header),
                       (ULONG)sizeof(pIda->Header) );
    OEMTOANSI( pIda->Header.szLongName );

    UtlPackSetTargetDrive( pIda->pPackCtrl, pIda->szTargetFolder[0] );
    do
    {
      ulRC = UtlPackReadPackage( pIda->pPackCtrl );
    } while ( ulRC == PACK_OK_RC );

    // get directory to which the files were unloaded
    UtlPackQueryImportPath( pIda->pPackCtrl, pIda->szImportPath );
    fFilesUnloaded = TRUE;

    if ( ulRC == PACK_COMPLETE_RC )
    {
    }
    else
    {
      // package unload failed
      usResult = LOAD_NOTOK;
    } /* endif */
    UtlPackClosePackage( pIda->pPackCtrl ); // ... close it
  }
  else
  {
    // open on package file failed
    usResult = LOAD_NOTOK;
  } /* endif */

  //
  // check if document already exists in target folder
  //
  if ( usResult == LOAD_OK )
  {
    BOOL fIsNewDoc;

    fSourceRep = FALSE;
    fTargetRep = FALSE;
    UtlMakeEQFPath( pIda->szDummy, pIda->szTargetFolder[0],
                    DIRSOURCEDOC_PATH,
                    pIda->szToFolder );

    UtlMakeFullPath (pIda->szTarget, (PSZ)NULP,
                     pIda->szDummy,
                     pIda->stFs.szName,
                     pIda->stFs.szExt);

    if ( pIda->Header.szLongName[0] != EOS )
    {
      FolLongToShortDocName( pIda->szTargetFolder,
                             pIda->Header.szLongName,
                             pIda->szToDoc,
                             &fIsNewDoc );
      pszReplace = pIda->Header.szLongName;
    }
    else
    {
	  USHORT usTmpRC = (USHORT)ulRC;
      pszReplace = UtlGetFnameFromPath( pIda->szTarget );
      strcpy( pIda->szToDoc, pIda->szString );
      fIsNewDoc = !CheckExistence( pIda->szTarget, FALSE, FILE_NORMAL, &usTmpRC);
      ulRC = (ULONG)usTmpRC;
    } /* endif */

    if ( !fIsNewDoc )
    {
      if ( pIda->fYesToAll )
      {
        usResponse = MBID_YES;
      }
      else if ( pIda->usSelDocs > 1 )
      {
        usResponse = UtlError( ERROR_FILE_EXISTS_ALREADY,
                               MB_EQF_YESTOALL | MB_DEFBUTTON3, 1,
                               &pszReplace, EQF_QUERY );
        if ( usResponse == MBID_EQF_YESTOALL )
        {
          pIda->fYesToAll = TRUE;
          usResponse = MBID_YES;
        } /* endif */
      }
      else
      {
        usResponse = UtlError( ERROR_FILE_EXISTS_ALREADY,
                               MB_YESNOCANCEL | MB_DEFBUTTON2, 1,
                               &pszReplace, EQF_QUERY );
      } /* endif */

      switch ( usResponse )
      {
        case MBID_YES:
          fSourceRep = TRUE;

          // check if there is a segment target document
          DocImpTargetName( pIda->szTarget, pIda->szTargetFolder,
                            DIRSEGTARGETDOC_PATH, pIda->szToDoc );
          fTargetRep = UtlFileExist( pIda->szTarget );
          //build object name of document
          strcpy( pIda->szObjName, pIda->szTargetFolder );
          strcat( pIda->szObjName, BACKSLASH_STR );
          strcat( pIda->szObjName, pIda->szToDoc );
          usDummy = 0;

          if ( !DocumentDelete( pIda->szObjName, FALSE, &usDummy ) )
          {
            pszReplace = pIda->szObjName,
            UtlError( NOT_DELETED, MB_CANCEL, 1,
                      &pszReplace, EQF_WARNING );
            usResult = LOAD_NOTOK;
          }
          else
          {
            // decrease number of docs in folder (doc deleted)
            pIda->usDocsInFolder--;
          } /* endif */
          break;
        case MBID_NO:
          usResult = LOAD_NOTOK;
          break;
        default:                              // MBID_CANCEL , etc
          usResult = LOAD_CANCEL;
          break;
      } /* endswitch */
    } /* endif */
  } /* endif */

  //
  // process files loaded from packaged document
  //
  if ( usResult == LOAD_OK )
  {
    /*******************************************************/
    /* Process imported history log file (if any)          */
    /*******************************************************/
    if ( usResult == LOAD_OK )
    {
      // setup fully qualified path names of history log file
      DocImpSourceName( pIda->szSource, pIda->szImportPath,
                        szFromFolder, PROPERTY_PATH, HISTLOGFILE );
      DocImpTargetName( pIda->szTarget, pIda->szTargetFolder, PROPERTY_PATH,
                        HISTLOGFILE );

      // if there is an imported history log file...
      if ( UtlFileExist( pIda->szSource ) )
      {
        // if there is already a history log file
        if ( UtlFileExist( pIda->szTarget ) )
        {
          // merge imported history log file into existing one
          FolMergeHistoryLog( pIda->szSource, pIda->szTarget,
                              TRUE, NULLHANDLE,
                              pIda->szString, pIda->szToDoc );

          // delete imported history log file
          UtlDelete( pIda->szSource, 0L, FALSE );
        }
        else
        {
          // no history log file in folder, so copy the imported one
          UtlSmartMoveHwnd( pIda->szSource, pIda->szTarget,
                            TRUE, NULLHANDLE );
        } /* endif */
      } /* endif */
    } /* endif */

    //
    // correct/update header part of document property file
    //

    // get path name of source and target property file
    DocImpSourceName( pIda->szSource, pIda->szImportPath,
                      szFromFolder, PROPERTY_PATH, pIda->szString );
    DocImpTargetName( pIda->szTarget, pIda->szTargetFolder, PROPERTY_PATH,
                      pIda->szToDoc );

    // change path information in property header
    UtlMakeEQFPath( pIda->szProperties, pIda->szTargetFolder[0],
                    SYSTEM_PATH,
                    pIda->szToFolder );
    {
      PPROPDOCUMENT pPropDoc = NULL; // ptr to in-memory copy of properties
      ULONG     ulPropDocSize = 0;  // size of in-memory copy of properties

      if ( UtlLoadFileL( pIda->szSource,
                        (PVOID *)&pPropDoc,
                        &ulPropDocSize,
                        FALSE, FALSE ) )
      {
        CHAR szEditor[MAX_FILESPEC];        // folder editor
        CHAR szFormat[MAX_FILESPEC];        // folder format / Tag Table
        CHAR szMemory[MAX_FILESPEC];        // folder Translation Memory
        CHAR szSourceLang[MAX_LANG_LENGTH]; // folder source language
        CHAR szTargetLang[MAX_LANG_LENGTH]; // folder target language
        OBJNAME szFolObjName;                  // folder object name

        // set correct parentid for imported doc
        pPropDoc->ulParentFolder = pIda->ulParentID;

        // build folder object name
        UtlMakeEQFPath( szFolObjName, NULC, SYSTEM_PATH, NULL );
        strcat( szFolObjName, BACKSLASH_STR );
        strcat( szFolObjName, pIda->szToFolder );

        // change path information in property header
        strcpy( pPropDoc->PropHead.szPath, pIda->szProperties );
        strcpy( pPropDoc->PropHead.szName, pIda->szToDoc );

        // get folder info and blank out fields with identical settings
        if ( FolQueryInfo2( szFolObjName, szMemory, szFormat,
                            szSourceLang, szTargetLang, szEditor,
                            FALSE ) == NO_ERROR )
        {
          if ( strcmp( pPropDoc->szMemory, szMemory) == 0 )
          {
            pPropDoc->szMemory[0] = EOS;
          } /* endif */
          if ( strcmp( pPropDoc->szFormat, szFormat) == 0 )
          {
            pPropDoc->szFormat[0] = EOS;
          } /* endif */
          if ( strcmp( pPropDoc->szSourceLang, szSourceLang) == 0 )
          {
            pPropDoc->szSourceLang[0] = EOS;
          } /* endif */
          if ( strcmp( pPropDoc->szTargetLang, szTargetLang) == 0 )
          {
            pPropDoc->szTargetLang[0] = EOS;
          } /* endif */

          if ( strcmp( pPropDoc->szEditor, szEditor) == 0 )
          {
            pPropDoc->szEditor[0] = EOS;
          } /* endif */
        } /* endif */

        // GQ: Fix for P020267: Always blank out memory name and use
        //     folder memory instead
        pPropDoc->szLongMemory[0] = EOS;
        pPropDoc->szMemory[0] = EOS;


        // re-write document property file
        UtlWriteFileL( pIda->szSource, ulPropDocSize, (PVOID)pPropDoc, FALSE );

        // free memory occupied by document properties
        UtlAlloc( (PVOID*)&pPropDoc, 0L, 0L, NOMSG );
      }
      else
      {
        usResult = LOAD_NOTOK;
      } /* endif */
    } /* endif */

    //
    // copy property file
    //
    if ( usResult == LOAD_OK )
    {
      if ( UtlFileExist( pIda->szTarget) )
      {
        UtlDelete( pIda->szTarget, 0L, FALSE );
      } /* endif */
      ulRC = UtlSmartMove( pIda->szSource, pIda->szTarget, TRUE );
      if ( ulRC != NO_ERROR )
      {
        usResult = LOAD_NOTOK;
      } /* endif */
    } /* endif */

    // copy document source file
    if ( usResult == LOAD_OK )
    {
      DocImpSourceName( pIda->szSource, pIda->szImportPath,
                        szFromFolder, DIRSOURCEDOC_PATH, pIda->szString );
      DocImpTargetName( pIda->szTarget, pIda->szTargetFolder, DIRSOURCEDOC_PATH,
                        pIda->szToDoc );
      if ( UtlFileExist( pIda->szTarget) )
      {
        UtlDelete( pIda->szTarget, 0L, FALSE );
      } /* endif */
      ulRC = UtlSmartMove( pIda->szSource, pIda->szTarget, TRUE );
      if ( ulRC != NO_ERROR )
      {
        usResult = LOAD_NOTOK;
      } /* endif */
    } /* endif */


    // copy document segmented source file
    if ( usResult == LOAD_OK )
    {
      DocImpSourceName( pIda->szSource, pIda->szImportPath, szFromFolder,
                        DIRSEGSOURCEDOC_PATH, pIda->szString );
      DocImpTargetName( pIda->szTarget, pIda->szTargetFolder,
                        DIRSEGSOURCEDOC_PATH, pIda->szToDoc );
      if ( UtlFileExist( pIda->szTarget) )
      {
        UtlDelete( pIda->szTarget, 0L, FALSE );
      } /* endif */
      if ( UtlFileExist( pIda->szSource ) )
      {
        ulRC = UtlSmartMove( pIda->szSource, pIda->szTarget, TRUE );
        if ( ulRC != NO_ERROR )
        {
          usResult = LOAD_NOTOK;
        } /* endif */
      } /* endif */
    } /* endif */

    // copy document segmented target file
    if ( usResult == LOAD_OK )
    {
      DocImpSourceName( pIda->szSource, pIda->szImportPath, szFromFolder,
                        DIRSEGTARGETDOC_PATH, pIda->szString );
      DocImpTargetName( pIda->szTarget, pIda->szTargetFolder,
                        DIRSEGTARGETDOC_PATH, pIda->szToDoc );
      if ( UtlFileExist( pIda->szTarget) )
      {
        UtlDelete( pIda->szTarget, 0L, FALSE );
        fTargetRep = TRUE;
      } /* endif */
      if ( UtlFileExist( pIda->szSource ) )
      {
        ulRC = UtlSmartMove( pIda->szSource, pIda->szTarget, TRUE );
        if ( ulRC != NO_ERROR )
        {
          usResult = LOAD_NOTOK;
        } /* endif */
      }
      else
      {
        fTargetRep = FALSE;
      } /* endif */
    } /* endif */

    // delete document in unsegmented target directory
    if ( usResult == LOAD_OK )
    {
      DocImpTargetName( pIda->szTarget, pIda->szTargetFolder, DIRTARGETDOC_PATH,
                        pIda->szToDoc );
      if ( UtlFileExist( pIda->szTarget) )
      {
        UtlDelete( pIda->szTarget, 0L, FALSE );
      } /* endif */
    } /* endif */

    // copy document EA data (if any)
    if ( usResult == LOAD_OK )
    {
      DocImpSourceName( pIda->szSource, pIda->szImportPath, szFromFolder,
                        EADATA_PATH, pIda->szString );
      DocImpTargetName( pIda->szTarget, pIda->szTargetFolder,
                        EADATA_PATH, pIda->szToDoc );
      if ( UtlFileExist( pIda->szTarget) )
      {
        UtlDelete( pIda->szTarget, 0L, FALSE );
      } /* endif */
      if ( UtlFileExist( pIda->szSource ) )
      {
        // create the EA data subdirectory
        {
          PSZ pszPathEnd = strrchr( pIda->szTarget, BACKSLASH );
          if ( pszPathEnd != NULL )
          {
            *pszPathEnd = EOS;
            UtlMkDir( pIda->szTarget, 0L, FALSE );
            *pszPathEnd = BACKSLASH;
          } /* endif */
        }

        ulRC = UtlSmartMove( pIda->szSource, pIda->szTarget, TRUE );
        if ( ulRC != NO_ERROR )
        {
          usResult = LOAD_NOTOK;
        } /* endif */
      }
      else
      {
        fTargetRep = FALSE;
      } /* endif */
    } /* endif */

    // copy document MT LOG file (if any)
    if ( usResult == LOAD_OK )
    {
      DocImpSourceName( pIda->szSource, pIda->szImportPath, szFromFolder,
                        MTLOG_PATH, pIda->szString );
      DocImpTargetName( pIda->szTarget, pIda->szTargetFolder,
                        MTLOG_PATH, pIda->szToDoc );
      if ( UtlFileExist( pIda->szTarget) )
      {
        UtlDelete( pIda->szTarget, 0L, FALSE );
      } /* endif */
      if ( UtlFileExist( pIda->szSource ) )
      {
        // create the MTLOG data subdirectory
        {
          PSZ pszPathEnd = strrchr( pIda->szTarget, BACKSLASH );
          if ( pszPathEnd != NULL )
          {
            *pszPathEnd = EOS;
            UtlMkDir( pIda->szTarget, 0L, FALSE );
            *pszPathEnd = BACKSLASH;
          } /* endif */
        }

        ulRC = UtlSmartMove( pIda->szSource, pIda->szTarget, TRUE );
        if ( ulRC != NO_ERROR )
        {
          usResult = LOAD_NOTOK;
        } /* endif */
      } /* endif */
    } /* endif */

    // copy document MISC file (if any)
    if ( usResult == LOAD_OK )
    {
      DocImpSourceName( pIda->szSource, pIda->szImportPath, szFromFolder, MISC_PATH, pIda->szString );
      DocImpTargetName( pIda->szTarget, pIda->szTargetFolder, MISC_PATH, pIda->szToDoc );
      if ( UtlFileExist( pIda->szTarget) )
      {
        UtlDelete( pIda->szTarget, 0L, FALSE );
      } /* endif */
      if ( UtlFileExist( pIda->szSource ) )
      {
        // create the MISC data subdirectory
        {
          PSZ pszPathEnd = strrchr( pIda->szTarget, BACKSLASH );
          if ( pszPathEnd != NULL )
          {
            *pszPathEnd = EOS;
            UtlMkDir( pIda->szTarget, 0L, FALSE );
            *pszPathEnd = BACKSLASH;
          } /* endif */
        }

        ulRC = UtlSmartMove( pIda->szSource, pIda->szTarget, TRUE );
        if ( ulRC != NO_ERROR )
        {
          usResult = LOAD_NOTOK;
        } /* endif */
      } /* endif */
    } /* endif */

    // copy document XLIFF file (if any)
    if ( usResult == LOAD_OK )
    {
      DocImpSourceName( pIda->szSource, pIda->szImportPath, szFromFolder, XLIFF_PATH, pIda->szString );
      DocImpTargetName( pIda->szTarget, pIda->szTargetFolder, XLIFF_PATH, pIda->szToDoc );
      if ( UtlFileExist( pIda->szTarget) )
      {
        UtlDelete( pIda->szTarget, 0L, FALSE );
      } /* endif */
      if ( UtlFileExist( pIda->szSource ) )
      {
        // create the XLIFF data subdirectory
        {
          PSZ pszPathEnd = strrchr( pIda->szTarget, BACKSLASH );
          if ( pszPathEnd != NULL )
          {
            *pszPathEnd = EOS;
            UtlMkDir( pIda->szTarget, 0L, FALSE );
            *pszPathEnd = BACKSLASH;
          } /* endif */
        }

        ulRC = UtlSmartMove( pIda->szSource, pIda->szTarget, TRUE );
        if ( ulRC != NO_ERROR )
        {
          usResult = LOAD_NOTOK;
        } /* endif */
      } /* endif */
    } /* endif */

    // copy document RTF file (if any)
    if ( usResult == LOAD_OK )
    {
      DocImpSourceName( pIda->szSource, pIda->szImportPath, szFromFolder, DIRSEGRTF_PATH, pIda->szString );
      DocImpTargetName( pIda->szTarget, pIda->szTargetFolder, DIRSEGRTF_PATH, pIda->szToDoc );
      if ( UtlFileExist( pIda->szTarget) )
      {
        UtlDelete( pIda->szTarget, 0L, FALSE );
      } /* endif */
      if ( UtlFileExist( pIda->szSource ) )
      {
        // create the RTF data subdirectory
        {
          PSZ pszPathEnd = strrchr( pIda->szTarget, BACKSLASH );
          if ( pszPathEnd != NULL )
          {
            *pszPathEnd = EOS;
            UtlMkDir( pIda->szTarget, 0L, FALSE );
            *pszPathEnd = BACKSLASH;
          } /* endif */
        }

        ulRC = UtlSmartMove( pIda->szSource, pIda->szTarget, TRUE );
        if ( ulRC != NO_ERROR )
        {
          usResult = LOAD_NOTOK;
        } /* endif */
      } /* endif */
    } /* endif */

    // copy document METADATA file (if any)
    if ( usResult == LOAD_OK )
    {
      DocImpSourceName( pIda->szSource, pIda->szImportPath, szFromFolder, METADATA_PATH, pIda->szString );
      DocImpTargetName( pIda->szTarget, pIda->szTargetFolder, METADATA_PATH, pIda->szToDoc );
      if ( UtlFileExist( pIda->szTarget) )
      {
        UtlDelete( pIda->szTarget, 0L, FALSE );
      } /* endif */
      if ( UtlFileExist( pIda->szSource ) )
      {
        // create the METADATA subdirectory
        {
          PSZ pszPathEnd = strrchr( pIda->szTarget, BACKSLASH );
          if ( pszPathEnd != NULL )
          {
            *pszPathEnd = EOS;
            UtlMkDir( pIda->szTarget, 0L, FALSE );
            *pszPathEnd = BACKSLASH;
          } /* endif */
        }

        ulRC = UtlSmartMove( pIda->szSource, pIda->szTarget, TRUE );
        if ( ulRC != NO_ERROR )
        {
          usResult = LOAD_NOTOK;
        } /* endif */
      } /* endif */
    } /* endif */




    /*********************************************************/
    /* Add import record to history log                      */
    /*********************************************************/
    if ( usResult == LOAD_OK )
    {
      BOOL fOK = TRUE;
      PDOCIMPORTHIST pDocImpHist;  // history record for document import

      fOK = UtlAlloc( (PVOID *)&pDocImpHist, 0L,
                      (ULONG)sizeof(DOCIMPORTHIST),
                      ERROR_STORAGE );
      if ( fOK )
      {
        pDocImpHist->sType              = INTERN_SUBTYPE;
        pDocImpHist->fSourceDocReplaced = fSourceRep;
        pDocImpHist->fTargetDocReplaced = fTargetRep;
        Utlstrccpy( pDocImpHist->szFolder, szFromFolder,
                    DOT );


        // no shipment_handler in the internal format case
        EQFBWriteHistLog2( pIda->szTargetFolder,
                           pIda->szToDoc,
                           DOCIMPORT_LOGTASK,
                           sizeof(DOCIMPORTHIST),
                           (PVOID)pDocImpHist,
                           TRUE, NULLHANDLE,
                           pIda->Header.szLongName );

        UtlAlloc( (PVOID *)&pDocImpHist, 0L, 0L, NOMSG );
      } /* endif */

      if ( fOK )
      {
        /*********************************************************/
        /* Create new target history log record if necessary     */
        /*********************************************************/
        if ( fTargetRep )
        {
          FolWriteNewTargetRec( pIda->szTargetFolder, pIda->szToDoc,
                                TRUE, NULLHANDLE );
        } /* endif */
      } /* endif */
    } /* endif */
  } /* endif */

  //
  // cleanup
  //
  if ( fFilesUnloaded )
  {
    // remove import directory
    UtlRemoveDir( pIda->szImportPath, FALSE );
  } /* endif */

  return usResult;
} /* end of DocImpInternal */


VOID DocImpSourceName( PSZ pszSource, PSZ pszImportPath,
                       PSZ pszFolderName, USHORT usPathType, PSZ pszDocName )
{
  PSZ     pszTemp;                    // working pointer

  //
  // get 'normal' path to requested file
  //
  UtlMakeEQFPath( pszSource, pszImportPath[0], usPathType, pszFolderName );

  //
  // replace path including system directory with pszImportPath
  //

  // locate end of old path
  pszTemp = strchr( pszSource, BACKSLASH );               // skip drive letter
  pszTemp = strchr( pszTemp + 1, BACKSLASH );             // skip system dir
  // pointer should now point to backslash behind d:\EQF

  // make room for new path
  memmove( pszTemp +
           (strlen(pszImportPath) -           // length of new path -
            (pszTemp - pszSource)),            // length of old path
           pszTemp,
           strlen( pszTemp ) + 1 );             // length of string plus EOS

  // replace old path with new one
  memcpy( pszSource, pszImportPath, strlen(pszImportPath) );

  // append document name
  strcat( pszSource, BACKSLASH_STR );
  strcat( pszSource, pszDocName );
} /* end of DocImpSourceName */

VOID DocImpTargetName( PSZ pszTarget, PSZ pszFolderPath, USHORT usPathType,
                       PSZ pszDocName )
{
  UtlMakeEQFPath( pszTarget, pszFolderPath[0], usPathType,
                  UtlGetFnameFromPath( pszFolderPath ) );
  strcat( pszTarget, BACKSLASH_STR );
  strcat( pszTarget, pszDocName );
} /* end of DocImpTargetName */




//------------------------------------------------------------------------------
// DocExpUpdateDosProps                                                       --
//------------------------------------------------------------------------------




BOOL DocExpUpdateDosProps( PDOCEXPIDA pIda )
{
  PSZ         pszReplace;             // pointer to error parameter
  BOOL        fOK = TRUE;             // internal OK lfag, returned to caller
  ULONG       ulErrorInfo;            // error code of property handler
  PPROPDOCUMENT ppropDoc;             // pointer to document properties
  HPROP         hpropDoc;             // handle of document properties

  // build object name of document properties
  UtlMakeEQFPath( pIda->szDocObjName,
                  pIda->szFolderObjName[0], SYSTEM_PATH,
                  pIda->szFolderName );
  strcat( pIda->szDocObjName, BACKSLASH_STR );
  strcat( pIda->szDocObjName, pIda->szName );

  // open document properties
  if ( ( hpropDoc = OpenProperties
          ( pIda->szDocObjName, NULL,
            PROP_ACCESS_READ, &ulErrorInfo))== NULL)
  {
    fOK = FALSE;
    pszReplace = pIda->szDocObjName;
    UtlError( ERROR_OPEN_PROPERTIES, MB_CANCEL, 1,
              &pszReplace, EQF_ERROR);
  }
  else
  {
    //--- open was ok, now update the properties ---

    SetPropAccess( hpropDoc, PROP_ACCESS_WRITE);
    ppropDoc = (PPROPDOCUMENT)MakePropPtrFromHnd( hpropDoc );
    UtlTime( (PLONG)&ppropDoc->ulExp );
    SaveProperties( hpropDoc, &ulErrorInfo );
    ResetPropAccess( hpropDoc, PROP_ACCESS_WRITE);
    CloseProperties( hpropDoc, PROP_FILE, &ulErrorInfo);
  }/*endif*/

  return( fOK );
} /* end of DocExpUpdateDocProps */

INT_PTR CALLBACK REVMARKDLGPROC( HWND   hwnd,
                        WINMSG msg,
                        WPARAM mp1,
                        LPARAM mp2 )

{
  MRESULT     mResult = FALSE;        // function return value
  PDOCEXPIDA  pIda;                   // ptr to document export IDA
  BOOL        fOK;                    // internal OK flag
  PSZ         pszTemp;                // temporary work pointer
  ULONG       ulRequiredSize;         // required buffer size for revision mark
  ULONG       ulCurrOffs = 0;         // current offset into revision mark area
  USHORT      usMBID;                 // return ID from WInMessageBox call

  switch ( msg )
  {
    case WM_EQF_QUERYID: HANDLEQUERYID( ID_REVMARK_DLG, mp2 ); break;

      //------------------------------------------------------------------------
    case ( WM_INITDLG ) :
      /**************************************************************/
      /* Get and anchor document export IDA pointer                 */
      /**************************************************************/
      pIda = (PDOCEXPIDA) PVOIDFROMMP2(mp2);
      ANCHORDLGIDA( hwnd, pIda );

      /**************************************************************/
      /* Set text limit of entry fields                             */
      /**************************************************************/
      SETTEXTLIMIT( hwnd, ID_REVMARK_NAME_EF, MAX_FNAME - 1 );
      SETTEXTLIMIT( hwnd, ID_REVMARK_DESCR_EF, MAX_DESCRIPTION - 1 );
      SETTEXTLIMIT( hwnd, ID_REVMARK_CAT1_BEG_EF, MAX_REVMARK_SIZE - 1 );
      SETTEXTLIMIT( hwnd, ID_REVMARK_CAT1_END_EF, MAX_REVMARK_SIZE - 1 );
      SETTEXTLIMIT( hwnd, ID_REVMARK_CAT2_BEG_EF, MAX_REVMARK_SIZE - 1 );
      SETTEXTLIMIT( hwnd, ID_REVMARK_CAT2_END_EF, MAX_REVMARK_SIZE - 1 );
      SETTEXTLIMIT( hwnd, ID_REVMARK_CAT3_BEG_EF, MAX_REVMARK_SIZE - 1 );
      SETTEXTLIMIT( hwnd, ID_REVMARK_CAT3_END_EF, MAX_REVMARK_SIZE - 1 );

      {
        USHORT  usCharSet;
        usCharSet = GetCharSet();
        SetCtrlFnt(hwnd, usCharSet,ID_REVMARK_CAT1_BEG_EF, ID_REVMARK_CAT1_END_EF );
        SetCtrlFnt(hwnd, usCharSet,ID_REVMARK_CAT2_BEG_EF, ID_REVMARK_CAT2_END_EF );
        SetCtrlFnt(hwnd, usCharSet,ID_REVMARK_CAT3_BEG_EF, ID_REVMARK_CAT3_END_EF );
        SetCtrlFnt(hwnd, usCharSet,ID_REVMARK_DESCR_EF, 0 );
      }

      /**************************************************************/
      /* Fill revision mark file name field                         */
      /**************************************************************/
      SETTEXT( hwnd, ID_REVMARK_NAME_EF, pIda->szString );

      /**************************************************************/
      /* Try to load any specified revision mark file               */
      /*                                                            */
      /* The currently selected revision mark file name has been    */
      /* placed by the document export dialog in the field szString */
      /* of the DOCEXPIDA.                                          */
      /**************************************************************/
      UtlMakeEQFPath( pIda->szRevMarkFile, NULC, TABLE_PATH, NULL );
      strcat( pIda->szRevMarkFile, BACKSLASH_STR );
      strcat( pIda->szRevMarkFile, pIda->szString );
      strcat( pIda->szRevMarkFile, EXT_OF_REVISION_MARK );

      /************************************************/
      /* Load and check revision mark file           */
      /************************************************/
      fOK = UtlFileExist( pIda->szRevMarkFile );

      if ( fOK )
      {
		ULONG ulTemp = pIda->usRevMarkSize;
        fOK = UtlLoadFileL( pIda->szRevMarkFile,
                           (PVOID *)&pIda->pRevMark,
                           &ulTemp,
                           FALSE, TRUE );
        pIda->usRevMarkSize = (USHORT)ulTemp;
      } /* endif */

      if ( fOK )
      {
        fOK = (strcmp( pIda->pRevMark->szID, REVMARKID ) == 0 );
      } /* endif */

      if ( fOK )
      {
        /**********************************************************/
        /* Revision mark file was successfully loaded, now fill   */
        /* the dialog fields                                      */
        /**********************************************************/
        strcpy( pIda->szActRevMark, pIda->szString );
        OEMSETTEXT( hwnd, ID_REVMARK_DESCR_EF, pIda->pRevMark->szDescr );
        RevMarkSetRevisionMarkText( hwnd, ID_REVMARK_CAT1_BEG_EF,
                                    pIda, pIda->pRevMark->usOffsCat1Beg );
        RevMarkSetRevisionMarkText( hwnd, ID_REVMARK_CAT1_END_EF,
                                    pIda, pIda->pRevMark->usOffsCat1End );
        RevMarkSetRevisionMarkText( hwnd, ID_REVMARK_CAT2_BEG_EF,
                                    pIda, pIda->pRevMark->usOffsCat2Beg );
        RevMarkSetRevisionMarkText( hwnd, ID_REVMARK_CAT2_END_EF,
                                    pIda, pIda->pRevMark->usOffsCat2End );
        RevMarkSetRevisionMarkText( hwnd, ID_REVMARK_CAT3_BEG_EF,
                                    pIda, pIda->pRevMark->usOffsCat3Beg );
        RevMarkSetRevisionMarkText( hwnd, ID_REVMARK_CAT3_END_EF,
                                    pIda, pIda->pRevMark->usOffsCat3End );

        /************************************************************/
        /* Set cursor to description field                          */
        /************************************************************/
        SETFOCUS( hwnd, ID_REVMARK_DESCR_EF );


        /************************************************************/
        /* Enable delete pushbutton                                 */
        /************************************************************/
        ENABLECTRL( hwnd, ID_REVMARK_DELETE_PB, TRUE );
      }
      else
      {
        /***********************************************************/
        /* Set focus to name entry field                           */
        /***********************************************************/
        SETFOCUS( hwnd, ID_REVMARK_NAME_EF );
        pIda->szActRevMark[0] = EOS;

        /************************************************************/
        /* Disable delete pushbutton                                */
        /************************************************************/
        ENABLECTRL( hwnd, ID_REVMARK_DELETE_PB, FALSE );
      } /* endif */

      /************************************************************/
      /* Reset entry field changed flags                          */
      /************************************************************/
      SLESETCHANGED( hwnd, ID_REVMARK_NAME_EF, FALSE );
      SLESETCHANGED( hwnd, ID_REVMARK_DESCR_EF, FALSE );
      SLESETCHANGED( hwnd, ID_REVMARK_CAT1_BEG_EF, FALSE );
      SLESETCHANGED( hwnd, ID_REVMARK_CAT1_END_EF, FALSE );
      SLESETCHANGED( hwnd, ID_REVMARK_CAT2_BEG_EF, FALSE );
      SLESETCHANGED( hwnd, ID_REVMARK_CAT2_END_EF, FALSE );
      SLESETCHANGED( hwnd, ID_REVMARK_CAT3_BEG_EF, FALSE );
      SLESETCHANGED( hwnd, ID_REVMARK_CAT3_END_EF, FALSE );

      mResult = MRFROMSHORT(TRUE);

      break;

      //------------------------------------------------------------------------
    case ( WM_EQF_CLOSE ) :
      DelCtrlFont(hwnd, ID_REVMARK_CAT1_BEG_EF );
      DelCtrlFont(hwnd, ID_REVMARK_CAT2_BEG_EF );
      DelCtrlFont(hwnd, ID_REVMARK_CAT3_BEG_EF );
      DelCtrlFont(hwnd, ID_REVMARK_DESCR_EF );
      DISMISSDLG( hwnd, SHORT1FROMMP1(mp1) );
      break;

      //------------------------------------------------------------------------
    case ( WM_COMMAND ) :
      switch ( WMCOMMANDID( mp1, mp2 ) )
      {
        case ID_REVMARK_HELP_PB :
          UtlInvokeHelp();
          break;

        //------------------------------------------------------------------
        case ID_REVMARK_CANCEL_PB :  // CANCEL button selected
        case DID_CANCEL :            // ESC key pressed

          /**********************************************************/
          /* Check if data has been changed                         */
          /**********************************************************/
          if ( QUERYCHANGED( hwnd, ID_REVMARK_NAME_EF ) ||
               QUERYCHANGED( hwnd, ID_REVMARK_DESCR_EF ) ||
               QUERYCHANGED( hwnd, ID_REVMARK_CAT1_BEG_EF ) ||
               QUERYCHANGED( hwnd, ID_REVMARK_CAT1_END_EF ) ||
               QUERYCHANGED( hwnd, ID_REVMARK_CAT2_BEG_EF ) ||
               QUERYCHANGED( hwnd, ID_REVMARK_CAT2_END_EF ) ||
               QUERYCHANGED( hwnd, ID_REVMARK_CAT3_BEG_EF ) ||
               QUERYCHANGED( hwnd, ID_REVMARK_CAT3_END_EF ) )
          {
            usMBID = UtlErrorHwnd( ERROR_REVMARK_CHANGED, MB_YESNOCANCEL,
                                   0, NULL, EQF_QUERY, hwnd );
          }
          else
          {
            usMBID = MBID_NO;
          } /* endif */

          switch ( usMBID )
          {
            case MBID_YES :
              /******************************************************/
              /* Save changes                                       */
              /******************************************************/
              WinPostMsg( hwnd, WM_COMMAND,
                          MP1FROMSHORT( ID_REVMARK_SAVE_PB ), NULL );
              break;
            case MBID_NO :
              /******************************************************/
              /* Leave dialog and discard changes                   */
              /******************************************************/
              WinPostMsg( hwnd, WM_EQF_CLOSE, NULL, NULL );
              break;
            default :
              /******************************************************/
              /* Stay in dialog                                     */
              /******************************************************/
              break;
          } /* endswitch */
          break;
          //------------------------------------------------------------------
        case ID_REVMARK_DELETE_PB :    // revision mark delete PB
          /**********************************************************/
          /* Get access to IDA                                      */
          /**********************************************************/
          fOK = TRUE;
          pIda = ACCESSDLGIDA( hwnd, PDOCEXPIDA );

          /**********************************************************/
          /* Get revision mark name                                 */
          /**********************************************************/
          QUERYTEXT( hwnd, ID_REVMARK_NAME_EF, pIda->szString );
          UtlStripBlanks( pIda->szString );
          strupr( pIda->szString );

          /**********************************************************/
          /* Build fully qualified revision mark file name          */
          /**********************************************************/
          UtlMakeEQFPath( pIda->szRevMarkFile, NULC, TABLE_PATH, NULL );
          strcat( pIda->szRevMarkFile, BACKSLASH_STR );
          strcat( pIda->szRevMarkFile, pIda->szString );
          strcat( pIda->szRevMarkFile, EXT_OF_REVISION_MARK );

          /**********************************************************/
          /* Delete revision mark file                              */
          /**********************************************************/
          UtlDelete( pIda->szRevMarkFile, 0L, TRUE );

          /**********************************************************/
          /* End dialog                                             */
          /**********************************************************/
          pIda->szString[0] = EOS;
          WinPostMsg( hwnd, WM_EQF_CLOSE, MP1FROMSHORT(TRUE), NULL );
          break;
          //------------------------------------------------------------------
        case ID_REVMARK_SAVE_PB :    // revision mark save PB
          /**********************************************************/
          /* Get access to IDA                                      */
          /**********************************************************/
          fOK = TRUE;
          pIda = ACCESSDLGIDA( hwnd, PDOCEXPIDA );

          /**********************************************************/
          /* Check existance of revision mark name                  */
          /**********************************************************/
          OEMQUERYTEXT( hwnd, ID_REVMARK_NAME_EF, pIda->szString );
          UtlStripBlanks( pIda->szString );
          strupr( pIda->szString );
          if ( pIda->szString[0] == EOS )
          {
            UtlErrorHwnd( ERROR_REVMARK_NO_NAME, MB_CANCEL,
                          0, NULL, EQF_ERROR, hwnd );
            SETFOCUS( hwnd, ID_REVMARK_NAME_EF );
            fOK = FALSE;
          } /* endif */

          /**********************************************************/
          /* Check syntax of revision mark file name                */
          /**********************************************************/
          if ( fOK )
          {
            pszTemp = pIda->szString;
            while ( *pszTemp && isalnum(*pszTemp) )
            {
              pszTemp++;
            } /* endwhile */
            if ( *pszTemp )
            {
              pszTemp = pIda->szString;
              UtlErrorHwnd( ERROR_REVMARK_NAME_INVALID, MB_CANCEL,
                            1, &pszTemp, EQF_ERROR, hwnd );
              SETFOCUS( hwnd, ID_REVMARK_NAME_EF );
              fOK = FALSE;
            } /* endif */
          } /* endif */

          /**********************************************************/
          /* Allocate buffer for revision mark data                 */
          /**********************************************************/
          if ( fOK )
          {
            ulRequiredSize = sizeof(REVMARK) + (6 * MAX_REVMARK_SIZE);

            if ( pIda->usRevMarkSize < ulRequiredSize )
            {
              fOK = UtlAlloc( (PVOID *)&pIda->pRevMark,
                              (LONG)pIda->usRevMarkSize,
                              ulRequiredSize,
                              ERROR_STORAGE );
              if ( fOK )
              {
                pIda->usRevMarkSize = (USHORT)ulRequiredSize;
              }
              else
              {
                pIda->usRevMarkSize = 0;
              } /* endif */
            } /* endif */
          } /* endif */

          /**********************************************************/
          /* Initialize revision mark data area                     */
          /**********************************************************/
          if ( fOK )
          {
            memset( pIda->pRevMark, 0, pIda->usRevMarkSize );
            strcpy( pIda->pRevMark->szID, REVMARKID );
            strcpy( pIda->pRevMark->szName, pIda->szString );
          } /* endif */

          /**********************************************************/
          /* Fill-in description                                    */
          /**********************************************************/
          if ( fOK )
          {
            OEMQUERYTEXT( hwnd, ID_REVMARK_DESCR_EF, pIda->pRevMark->szDescr );
          } /* endif */

          /**********************************************************/
          /* Get revision mark strings                              */
          /**********************************************************/
          if ( fOK )
          {
            ulCurrOffs = sizeof(REVMARK);
            RevMarkGetRevisionMarkText( hwnd, ID_REVMARK_CAT1_BEG_EF,
                                        pIda, &ulCurrOffs,
                                        &pIda->pRevMark->usOffsCat1Beg );
            RevMarkGetRevisionMarkText( hwnd, ID_REVMARK_CAT1_END_EF,
                                        pIda, &ulCurrOffs,
                                        &pIda->pRevMark->usOffsCat1End );
            RevMarkGetRevisionMarkText( hwnd, ID_REVMARK_CAT2_BEG_EF,
                                        pIda, &ulCurrOffs,
                                        &pIda->pRevMark->usOffsCat2Beg );
            RevMarkGetRevisionMarkText( hwnd, ID_REVMARK_CAT2_END_EF,
                                        pIda, &ulCurrOffs,
                                        &pIda->pRevMark->usOffsCat2End );
            RevMarkGetRevisionMarkText( hwnd, ID_REVMARK_CAT3_BEG_EF,
                                        pIda, &ulCurrOffs,
                                        &pIda->pRevMark->usOffsCat3Beg );
            RevMarkGetRevisionMarkText( hwnd, ID_REVMARK_CAT3_END_EF,
                                        pIda, &ulCurrOffs,
                                        &pIda->pRevMark->usOffsCat3End );
          } /* endif */

          /**********************************************************/
          /* Write revision mark buffer to disk                     */
          /**********************************************************/
          if ( fOK )
          {
            /********************************************************/
            /* build fully qualified file name                      */
            /********************************************************/
            UtlMakeEQFPath( pIda->szRevMarkFile, NULC, TABLE_PATH, NULL );
            strcat( pIda->szRevMarkFile, BACKSLASH_STR );
            strcat( pIda->szRevMarkFile, pIda->szString );
            strcat( pIda->szRevMarkFile, EXT_OF_REVISION_MARK );

            /********************************************************/
            /* Check for existing files if revision mark buffer     */
            /* is saved under a different name                      */
            /********************************************************/
            if ( stricmp( pIda->szString, pIda->szActRevMark) != 0 )
            {
              if ( UtlFileExist( pIda->szRevMarkFile ) )
              {
                pszTemp = pIda->szString;
                if ( UtlErrorHwnd( WARNING_REVMARK_EXISTS,
                                   MB_OKCANCEL,
                                   1,
                                   &pszTemp,
                                   EQF_WARNING,
                                   hwnd ) != MBID_OK )
                {
                  fOK = FALSE;
                } /* endif */
              } /* endif */
            } /* endif */

            /********************************************************/
            /* Write file to disk                                   */
            /********************************************************/
            if ( fOK )
            {
              fOK = UtlWriteFileHwnd( pIda->szRevMarkFile,
                                      ulCurrOffs,
                                      (PVOID)pIda->pRevMark,
                                      TRUE, hwnd ) == NO_ERROR;
            } /* endif */
          } /* endif */

          /**********************************************************/
          /* Leave dialog if save was successful                    */
          /**********************************************************/
          if ( fOK )
          {
            WinPostMsg( hwnd, WM_EQF_CLOSE, MP1FROMSHORT(TRUE), NULL );
          } /* endif */
        case ID_REVMARK_NAME_EF:
          mResult = RevMarkControl( hwnd, WMCOMMANDID( mp1, mp2 ),
                                    WMCOMMANDCMD( mp1, mp2 ) );
          break;
        case ID_REVMARK_CAT1_BEG_EF:
        case ID_REVMARK_CAT2_BEG_EF:
        case ID_REVMARK_CAT3_BEG_EF:
        case ID_REVMARK_DESCR_EF:
          if ( WMCOMMANDCMD( mp1, mp2 ) == EN_KILLFOCUS )
          {
            ClearIME( hwnd );
          } /* endif */
          break;
      } /* endswitch */
      break;
      //------------------------------------------------------------------------
    default :
      mResult = WinDefDlgProc( hwnd, msg, mp1, mp2 );
      break;
  } /*endswitch */
  return( mResult );
}/* end REVMARKDLGPROC*/

static MRESULT RevMarkControl
(
HWND   hwnd,                        // dialog window handle
SHORT  sId,                         // id in action
SHORT  sNotification                // notification
)
{

  switch ( sId )
  {
    case ID_REVMARK_NAME_EF:
      if ( sNotification == EN_CHANGE )
      {
        /**************************************************************/
        /* Enable/disable delete pushbutton depending on file name    */
        /* entered                                                    */
        /**************************************************************/
        PDOCEXPIDA  pIda;              // ptr to document export IDA
        CHAR szNewName[MAX_FILESPEC];  // large enough to contain more than 8
                                       // chars !!!

        pIda = ACCESSDLGIDA( hwnd, PDOCEXPIDA );

        QUERYTEXT( hwnd, ID_REVMARK_NAME_EF, szNewName );
        ANSITOOEM( szNewName );
        UtlStripBlanks( szNewName );
        strupr( szNewName );
        UtlMakeEQFPath( pIda->szRevMarkFile, NULC, TABLE_PATH, NULL );
        strcat( pIda->szRevMarkFile, BACKSLASH_STR );
        strcat( pIda->szRevMarkFile, szNewName );
        strcat( pIda->szRevMarkFile, EXT_OF_REVISION_MARK );

        ENABLECTRL( hwnd, ID_REVMARK_DELETE_PB,
                    UtlFileExist(pIda->szRevMarkFile));
      } /* endif */
      break;
  } /* endswitch */

  return( MRFROMSHORT(FALSE ) );

} /* end of RevmarkControl */


//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     RevMarkGetRevisionMarkText                               |
//+----------------------------------------------------------------------------+
//|Function call:     RevMarkGetRevisionMarkText( HWND hwnd, USHORT usEFID,    |
//|                                               PDOCEXPIDA pIda,             |
//|                                               PUSHORT pusCurrOffs,         |
//|                                               PUSHORT pusTextOffs );       |
//+----------------------------------------------------------------------------+
//|Description:       Gets a single revision mark string, converts all         |
//|                   <LF> tags to CRLF characters and stores the result       |
//|                   in the revision mark area.                               |
//+----------------------------------------------------------------------------+
//|Input parameter:   HWND         hwnd        handle of dialog window         |
//|                   USHORT       usEFID      ID of entry field containing    |
//|                                            string                          |
//|                   PDOCEXPIDA   pIda        ptr to document export IDA      |
//|                   PUSHORT      pusCurrOffs current offset into buffer area |
//|                   PUSHORT      pusTextOffs place to store offset of current|
//|                                            string                          |
//+----------------------------------------------------------------------------+
//|Returncode type:   VOID                                                     |
//+----------------------------------------------------------------------------+
//|Function flow:     get revision mark string from entry field                |
//|                   loop throug revision mark string                         |
//|                     if linefeed tag found                                  |
//|                       replace linefeed tag with carriage return linefeed   |
//|                     endif                                                  |
//|                   endloop                                                  |
//|                   update offsets and copy revision mark string to          |
//|                      revision mark buffer area                             |
//+----------------------------------------------------------------------------+
VOID RevMarkGetRevisionMarkText
(
HWND         hwnd,                   // handle of dialog window
USHORT       usEFID,                 // ID of entry field containing string
PDOCEXPIDA   pIda,                   // ptr to document export IDA
PULONG       pulCurrOffs,            // current offset into buffer area
PUSHORT      pusTextOffs             // place to store offset of current string
)
{
  PSZ          pszSource, pszTarget;   // ptr for revision mark text processing
  PSZ          pszLFTag;               // ptr to line feed tag ( <LF> )
  ULONG        ulTagLen;               // length of line feed tag

  /********************************************************************/
  /* Get revision mark string and store it in general work area       */
  /********************************************************************/
  OEMQUERYTEXT( hwnd, usEFID, pIda->szBuffer );

  /********************************************************************/
  /* Replace all <LF> tags with CRLF                                  */
  /********************************************************************/
  pszSource = pszTarget = pIda->szBuffer;
  pszLFTag  = REVMARK_LF_TAG;
  ulTagLen  = strlen(pszLFTag);
  while ( *pszSource )
  {
    if ( *pszSource == *pszLFTag )
    {
      if ( memcmp( pszSource, pszLFTag, ulTagLen ) == 0 )
      {
        *pszTarget++ = CR;
        *pszTarget++ = LF;
        pszSource += ulTagLen;

      }
      else
      {
        *pszTarget++ = *pszSource++;
      } /* endif */
    }
    else
    {
      *pszTarget++ = *pszSource++;
    } /* endif */
  } /* endwhile */
  *pszTarget = EOS;

  /********************************************************************/
  /* Store string in revision mark area and update offsets            */
  /********************************************************************/
  *pusTextOffs = (SHORT)(*pulCurrOffs);
  strcpy( (PSZ)pIda->pRevMark + *pusTextOffs, pIda->szBuffer );
  *pulCurrOffs += strlen(pIda->szBuffer) + 1;
} /* end of function RevMarkGetRevisionMarkText */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     RevMarkSetRevisionMarkText                               |
//+----------------------------------------------------------------------------+
//|Function call:     RevMarkGetRevisionMarkText( HWND hwnd, USHORT usEFID,    |
//|                                               PDOCEXPIDA pIda,             |
//|                                               USHORT usTextOffs );         |
//+----------------------------------------------------------------------------+
//|Description:       Sets the entry field text with the given revision mark   |
//|                   string. All CRLF combinationions in the string are       |
//|                   replaced with the linefeed tag.                          |
//+----------------------------------------------------------------------------+
//|Input parameter:   HWND         hwnd        handle of dialog window         |
//|                   USHORT       usEFID      ID of entry field containing    |
//|                                            string                          |
//|                   PDOCEXPIDA   pIda        ptr to document export IDA      |
//|                   USHORT       usTextOffs  offset of current text string   |
//+----------------------------------------------------------------------------+
//|Returncode type:   VOID                                                     |
//+----------------------------------------------------------------------------+
//|Function flow:     copy text from revision mark area to work area           |
//|                      replacing CRLF with <LF>                              |
//|                   set entry field text from data in work area              |
//+----------------------------------------------------------------------------+
VOID RevMarkSetRevisionMarkText
(
HWND         hwnd,                   // handle of dialog window
USHORT       usEFID,                 // ID of entry field containing string
PDOCEXPIDA   pIda,                   // ptr to document export IDA
USHORT       usTextOffs              // offset of current string
)
{
  PSZ          pszSource, pszTarget;   // ptr for revision mark text processing
  PSZ          pszLFTag;               // ptr to line feed tag ( <LF> )
  ULONG        ulTagLen;               // length of line feed tag

  /********************************************************************/
  /* Copy revision mark string from revision mark area to work buffer */
  /* replacing all CRLF characters with <LF> tags                     */
  /********************************************************************/
  pszSource = (PSZ)pIda->pRevMark + usTextOffs;
  pszTarget = pIda->szBuffer;
  pszLFTag  = REVMARK_LF_TAG;
  ulTagLen  = strlen(pszLFTag);
  while ( *pszSource )
  {
    if ( *pszSource == CR )
    {
      pszSource++;
      if ( *pszSource == LF )
      {
        memcpy( pszTarget, pszLFTag, ulTagLen );
        pszTarget += ulTagLen;
        pszSource++;
      } /* endif */
    }
    else
    {
      *pszTarget++ = *pszSource++;
    } /* endif */
  } /* endwhile */
  *pszTarget = EOS;
  /********************************************************************/
  /* Set entry field text from data in work area                      */
  /********************************************************************/
  OEMSETTEXT( hwnd, usEFID, pIda->szBuffer );

} /* end of function RevMarkSetRevisionMarkText */


USHORT DocImpHandlePathInput
(
HWND        hwndDlg                  // dialog window handle
)
{
  PDOCIMPIDA  pIda;                   // document import instance data area
  USHORT      usRC = NO_ERROR;        // function return code

  pIda = ACCESSDLGIDA( hwndDlg, PDOCIMPIDA );

  DocImpStopThread( pIda );           // stop any running thread

  if (pIda->sThreadStatus == THREAD_RUNNING )
  {
    pIda->sThreadStatus = STOP_THREAD;  // stop any running thread
    DosSleep( 0 );
  } /* endif */

  // get current search pattern
  QUERYTEXT( hwndDlg, ID_DOCIMP_PATH_EF, pIda->Controls.szPathContent );
  UtlMakeFNameAndPath( pIda->Controls.szPathContent,
                       pIda->Controls.szDummy,
                       pIda->Controls.szPatternName);

  if ( pIda->Controls.szDummy[0] != EOS )
  {
    int iLen = strlen(pIda->Controls.szDummy);
    if ( pIda->Controls.szDummy[iLen-1] == BACKSLASH )
    {
      pIda->Controls.szDummy[iLen-1] = EOS;
    } /* endif */
  } /* endif */

  if ( pIda->Controls.szPatternName[0] == EOS )
  {
    strcpy( pIda->Controls.szPatternName, DEFAULT_PATTERN );
  } /* endif */

  // get start path from start path entry field
  QUERYTEXT( hwndDlg, ID_DOCIMP_STARTPATH_CB, pIda->szStartPath );
  UtlStripBlanks( pIda->szStartPath );

  // handle input
  if ( pIda->szStartPath[0] != EOS)
  {
    // check for drive specification in start path
    if ( pIda->szStartPath[1] == COLON )
    {
      if ( strchr( pIda->Controls.szDriveList,
                   toupper(pIda->szStartPath[0]) ) == NULL )
      {
        PSZ pszError = pIda->szStartPath;
        UtlErrorHwnd( ERROR_FILENAME_NOT_VALID, MB_CANCEL, 1, &pszError,
                      EQF_ERROR, hwndDlg );
        usRC = ERROR_FILENAME_NOT_VALID;
        SETFOCUS( hwndDlg, ID_DOCIMP_STARTPATH_CB );
      }
      else
      {
        // remove drive from start path and select it
       pIda->Controls.szDrive[0] = (CHAR)toupper(pIda->szStartPath[0]);
        memmove( pIda->szStartPath, pIda->szStartPath + 2,
                 sizeof(pIda->szStartPath) - 2 );
      } /* endif */
    } /* endif */
  }
  else
  {
    // start path is empty, use root directory as default
    {
      /***************************************************************/
      /* No target drive specified, use system drive as default      */
      /***************************************************************/
      CHAR szDrive[MAX_DRIVE];
      UtlQueryString( QST_PRIMARYDRIVE, szDrive, MAX_DRIVE );
      strcpy(pIda->szStartPath, szDrive);
    } /* endif */

    strcat( pIda->szStartPath, BACKSLASH_STR );
    SETTEXT( hwndDlg, ID_DOCIMP_STARTPATH_CB, pIda->szStartPath );
  } /* endif */

  // refresh files listbox if O.K.
  if ( usRC == NO_ERROR )
  {
    DELETEALL( hwndDlg, ID_DOCIMP_IMPPATH_LB );

    sprintf( pIda->szString, "%c:", pIda->Controls.szDrive[0] );
    if ( pIda->szStartPath[0] != BACKSLASH )
    {
      strcat( pIda->szString, BACKSLASH_STR );
    } /* endif */
    strcat( pIda->szString, pIda->szStartPath );
    DocScanDirStartThread( pIda->szString,
                           pIda->Controls.szPatternName,
                           pIda->Controls.szDummy,
                           (USHORT)strlen(pIda->szString),
                           WinWindowFromID( hwndDlg, ID_DOCIMP_IMPPATH_LB ),
                           &pIda->sThreadStatus );
  } /* endif */

  return( usRC );
} /* end of function DocImpHandlePathInput */

USHORT DocExpHandlePathInput
(
HWND        hwndDlg                  // dialog window handle
)
{
  PDOCIMPIDA  pIda;                   // document import instance data area
  USHORT      usRC = NO_ERROR;        // function return code

  pIda = ACCESSDLGIDA( hwndDlg, PDOCIMPIDA );

//  DocImpStopThread( pIda );           // stop any running thread
//
//if (pIda->sThreadStatus == THREAD_RUNNING )
//{
//  pIda->sThreadStatus = STOP_THREAD;  // stop any running thread
//  DosSleep( 0 );
//} /* endif */

  // get current search pattern
  QUERYTEXT( hwndDlg, ID_DOCIMP_PATH_EF, pIda->Controls.szPathContent );
  UtlMakeFNameAndPath( pIda->Controls.szPathContent,
                       pIda->Controls.szDummy,
                       pIda->Controls.szPatternName);

  if ( pIda->Controls.szDummy[0] != EOS )
  {
    int iLen = strlen(pIda->Controls.szDummy);
    if ( pIda->Controls.szDummy[iLen-1] == BACKSLASH )
    {
      pIda->Controls.szDummy[iLen-1] = EOS;
    } /* endif */
  } /* endif */

  if ( pIda->Controls.szPatternName[0] == EOS )
  {
    strcpy( pIda->Controls.szPatternName, DEFAULT_PATTERN );
  } /* endif */

  // get start path from start path entry field
  QUERYTEXT( hwndDlg, ID_DOCIMP_STARTPATH_CB, pIda->szStartPath );
  UtlStripBlanks( pIda->szStartPath );

  // handle input
  if ( pIda->szStartPath[0] != EOS)
  {
    // check for drive specification in start path
    if ( pIda->szStartPath[1] == COLON )
    {

      if ( strchr( pIda->Controls.szDriveList,
                   toupper(pIda->szStartPath[0]) ) == NULL )
      {
        PSZ pszError = pIda->szStartPath;
        UtlErrorHwnd( ERROR_FILENAME_NOT_VALID, MB_CANCEL, 1, &pszError,
                      EQF_ERROR, hwndDlg );
        usRC = ERROR_FILENAME_NOT_VALID;
        SETFOCUS( hwndDlg, ID_DOCIMP_STARTPATH_CB );
      }
      else
      {
        // remove drive from start path and select it
        pIda->Controls.szDrive[0] = (CHAR)toupper(pIda->szStartPath[0]);
        memmove( pIda->szStartPath, pIda->szStartPath + 2,
                 sizeof(pIda->szStartPath) - 2 );
      } /* endif */
    } /* endif */
  }
  else
  {
    // start path is empty, use root directory as default
    {
      /***************************************************************/
      /* No target drive specified, use system drive as default      */
      /***************************************************************/
      CHAR szDrive[MAX_DRIVE];
      UtlQueryString( QST_PRIMARYDRIVE, szDrive, MAX_DRIVE );
      strcpy(pIda->szStartPath, szDrive);
    } /* endif */

    strcat( pIda->szStartPath, BACKSLASH_STR );
    SETTEXT( hwndDlg, ID_DOCIMP_STARTPATH_CB, pIda->szStartPath );
  } /* endif */
  return( usRC );
} /* end of function DocExpHandlePathInput */

// structure for passing parameters to our listbox fill thread
typedef struct _DOCSCANDIRTHREADDATA
{
  PSZ    pszStartDir;                  // pointer to start directory
  PSZ    pszPattern;                   // search pattern
  PSZ    pszPathFilter;                // pattern for path filter or NULL
  SHORT  sStartPathLen;                // length of original start path
  HWND   hwndLB;                       // handle of listbox being filled
  PSHORT psStatus;                      // pointer to status flag
} DOCSCANDIRTHREADDATA, *PDOCSCANDIRTHREADDATA;


USHORT DocScanDirStartThread
(
PSZ    pszStartDir,                  // pointer to start directory
PSZ    pszPattern,                   // search pattern
PSZ    pszPathFilter,                // pattern for path filter or NULL
SHORT  sStartPathLen,                // length of original start path
HWND   hwndLB,                       // handle of listbox being filled
PSHORT psStatus                      // pointer to status flag
)
{
  PDOCSCANDIRTHREADDATA pData = NULL;

  if ( UtlAlloc( (PVOID *)&pData, 0L, sizeof(DOCSCANDIRTHREADDATA), NOMSG ) )
  {
    pData->pszStartDir   = pszStartDir;
    pData->pszPattern    = pszPattern;
    pData->pszPathFilter = pszPathFilter;
    pData->sStartPathLen = sStartPathLen;
    pData->hwndLB        = hwndLB;
    pData->psStatus      = psStatus;
    _beginthread( DocScanDirThread, 0, (void *)pData );
  } /* endif */
  return( NO_ERROR );

} /* end of function DocScanDirStartThread */

void DocScanDirThread
(
void   *pvData                       // pointer to thread data
)
{
  PDOCSCANDIRTHREADDATA pData;
  PSHORT      psStatus;                // copy of status flag pointer

  pData = (PDOCSCANDIRTHREADDATA)pvData;

  psStatus = pData->psStatus;
  *psStatus = THREAD_RUNNING;

  DocImpScanDir( pData->pszStartDir, pData->pszPattern,
                 pData->pszPathFilter, pData->sStartPathLen,
                 pData->hwndLB, pData->psStatus );


  *psStatus = THREAD_STOPPED;


  UtlSetHorzScrollingForLB(pData->hwndLB);


  UtlAlloc( (PVOID *)&pData, 0L, 0L, NOMSG );
  _endthread();
} /* end of function DocScanDirThread */


typedef struct _DOCIMPSCANDIRDATA
{
  LONGFILEFIND ResultBuf;              // Buffer for long file names
  HDIR    hDir;                        // file search handle
  CHAR    szPath[MAX_LONGPATH];        // buffer for search path
  CHAR    szFile[MAX_LONGPATH];        // buffer for file name incl. path
  CHAR    szPathFilter[MAX_LONGPATH];  // pattern for path filter
} DOCIMPSCANDIRDATA, *PDOCIMPSCANDIRDATA;

// Recursive function to scan all files of a directory
// sub-directories are processed recursively

USHORT DocImpScanDir
(
PSZ    pszStartDir,                  // pointer to start directory
PSZ    pszPattern,                   // search pattern
PSZ    pszPathFilter,                // pattern for path filter or NULL
SHORT  sStartPathLen,                // length of original start path
HWND   hwndLB,                       // handle of listbox being filled
PSHORT psStatus                      // pointer to status flag
)
{
  PDOCIMPSCANDIRDATA pData = NULL;      // ptr to private data area
  USHORT usRC = NO_ERROR;

  if ( UtlAlloc( (PVOID *)&pData, 0L, sizeof(DOCIMPSCANDIRDATA), NOMSG ) )
  {
    // initialize data area
    pData->hDir = HDIR_CREATE;

    // setup search path
    strcpy( pData->szPath, pszStartDir );
    if ( pszStartDir[strlen(pszStartDir)-1] != BACKSLASH )
    {
      strcat( pData->szPath, BACKSLASH_STR );
    } /* endif */
    strcat( pData->szPath, pszPattern );

    if ( pszPathFilter != NULL )
    {
      strcpy( pData->szPathFilter, pszPathFilter );
    } /* endif */

    // scan for files
    if ( *psStatus != STOP_THREAD )
    {
      usRC = UtlFindFirstLong( pData->szPath, &(pData->hDir),
                               FILE_NORMAL,
                               &(pData->ResultBuf), FALSE );
    } /* endif */

    while ( !usRC & (*psStatus != STOP_THREAD) )
    {
      BOOL fInsert = FALSE;             // insert file flag

      // build complete file name
      strcpy( pData->szFile, pszStartDir );
      if ( pData->szFile[strlen(pData->szFile)-1] != BACKSLASH )
      {
        strcat( pData->szFile, BACKSLASH_STR );
      } /* endif */
      strcat( pData->szFile, pData->ResultBuf.achName );

      // check if current path matches path name filter
      if ( pData->szPathFilter[0] != EOS )
      {
        if ( pData->szPathFilter[1] == BACKSLASH )
        {
          UtlMatchStrings( pszStartDir + sStartPathLen + 1, pData->szPathFilter + 1, &fInsert );
        }
        else
        {
          UtlMatchStrings( pszStartDir + sStartPathLen + 1, pData->szPathFilter, &fInsert );
        } /* endif */
      }
      else
      {
        fInsert = TRUE;
      } /* endif */

      // add file to listbox (ignoring original start path)
      if ( fInsert )
      {
        PSZ pszItem = pData->szFile + sStartPathLen;
        if (*pszItem == BACKSLASH )
        {
          pszItem++;
        } /* endif */
        INSERTITEMHWND( hwndLB, pszItem );
      } /* endif */

      // continue with next file
      if ( *psStatus != STOP_THREAD )
      {
        usRC = UtlFindNextLong( pData->hDir, &(pData->ResultBuf), FALSE );
      } /* endif */
    } /* endwhile */


    // cleanup
    if ( pData->hDir != HDIR_CREATE )
    {
      UtlFindCloseLong( pData->hDir, FALSE );
      pData->hDir = HDIR_CREATE;
    } /* endif */


    // scan for directories
    usRC = NO_ERROR;
    strcpy( pData->szPath, pszStartDir );
    if ( pszStartDir[strlen(pszStartDir)-1] != BACKSLASH )
    {
      strcat( pData->szPath, BACKSLASH_STR );
    } /* endif */
    strcat( pData->szPath, "*.*" );

    if ( !usRC && (*psStatus != STOP_THREAD) )
    {
      usRC = UtlFindFirstLong( pData->szPath, &(pData->hDir),
                               FILE_DIRECTORY,
                               &(pData->ResultBuf), FALSE );
    } /* endif */

    while ( !usRC && (*psStatus != STOP_THREAD) )
    {
      // build complete file name
      strcpy( pData->szFile, pszStartDir );
      if ( pData->szFile[strlen(pData->szFile)-1] != BACKSLASH )
      {
        strcat( pData->szFile, BACKSLASH_STR );
      } /* endif */
      strcat( pData->szFile, pData->ResultBuf.achName );


      // handle directory
      {
        // scan the subdirectory
        if ( (strcmp(pData->ResultBuf.achName, CURRENT_DIR_NAME ) != 0) &&
             (strcmp(pData->ResultBuf.achName, PARENT_DIR_NAME ) != 0) )
        {

          usRC = DocImpScanDir( pData->szFile, pszPattern, pszPathFilter,
                                sStartPathLen, hwndLB, psStatus );
        } /* endif */
      }

      // continue with next directory
      if ( *psStatus != STOP_THREAD )
      {
        usRC = UtlFindNextLong( pData->hDir, &(pData->ResultBuf), FALSE );
      } /* endif */
    } /* endwhile */
    // cleanup
    if ( pData->hDir != HDIR_CREATE )
    {
      UtlFindCloseLong( pData->hDir, FALSE );
    } /* endif */
    UtlAlloc( (PVOID *)&pData, 0L, 0L, NOMSG  );
  } /* endif */
  return( usRC );
} /* end of function DocImpScanDir */


// handle ENTER key in Document import dialog
MRESULT DocImpDMGETDEFID
(
HWND             hwndDlg,            // dialog window handle
WPARAM           mp1,                // first message parameter
LPARAM           mp2                 // second message parameter
)
{
  MRESULT     mResult = (MRESULT)FALSE; //return code of this function
  mp1; mp2;
  /************************************************************/
  /* check if user pressed the ENTER key, but wants only to   */
  /* select/deselect an item of the listbox via a simulated   */
  /* (keystroke) double click or if the user is in the        */
  /* file name entry field and tries to activate a new        */
  /* search pattern                                           */
  /************************************************************/
  if ( GetKeyState(VK_RETURN) & 0x8000  )
  {
    HWND hwndFocus = GetFocus();
    HWND hwndParent = GetParent( hwndFocus );
    HWND hwndCombo = GetDlgItem( hwndDlg, ID_DOCIMP_STARTPATH_CB );
    if ( (hwndFocus == hwndCombo) || (hwndParent == hwndCombo) || (hwndFocus == GetDlgItem( hwndDlg, ID_DOCIMP_PATH_EF)) )
    {
      DocImpHandlePathInput( hwndDlg );
      mResult = TRUE;                  // do not process as default pushbutton
    } /* endif */
  } /* endif */
  return( mResult );
} /* end of function DocImpGETDEFID */

// stop any running scan dir thread
USHORT DocImpStopThread
(
PDOCIMPIDA  pIda                    // document import instance data area
)
{
  switch ( pIda->sThreadStatus )
  {
    case THREAD_INACTIVE:
      // nothing to do - no thread is running
      break;

    case THREAD_RUNNING:
    case STOP_THREAD:
      // switch to thread stop status and give thread some time slices to stop itself
      {
        long i = 10;                      // count down timer
        pIda->sThreadStatus = STOP_THREAD;
        do
        {
          DosSleep( 20 );
          i--;
        }
        while ( (i >= 0) && (pIda->sThreadStatus == STOP_THREAD) );
      }
      break;

    case THREAD_STOPPED:
      // switch to thread inactive mode
      pIda->sThreadStatus = THREAD_INACTIVE;
      break;
  } /* endswitch */

  // for debugginf purposes: check thread status
  //if ( (pIda->sThreadStatus == STOP_THREAD) ||
  //     (pIda->sThreadStatus == THREAD_RUNNING) )
  //{
  //  int breakpoint = 0;
  //} /* endif */
  return( NO_ERROR );
} /* end of function DocImpStopThread */

  #ifdef FUNCCALLIF
USHORT DocFuncImportDoc
(
PFCTDATA    pData,                   // function I/F session data
PSZ         pszFolderName,           // name of folder receiving the documents
PSZ         pszFiles,                // list of input files (documents)
PSZ         pszMemName,              // document Translation Memory or NULL
PSZ         pszMarkup,               // document markup or NULL
PSZ         pszEditor,               // document editor or NULL
PSZ         pszSourceLanguage,       // document source language or NULL
PSZ         pszTargetLanguage,       // document target language or NULL
PSZ         pszAlias,                // alias for document name or NULL
PSZ         pszStartPath,            // optional start path
PSZ         pszConversion,           // optional document export conversion
LONG        lOptions                 // document import options or 0L
)
{
  USHORT      usRC = NO_ERROR;         // function return code

  // prepare a new document import or continue current one
  if ( pData->fComplete )              // has last run been completed
  {
    // prepare a new document import
    usRC = DocFuncPrepImport( pData, pszFolderName, pszFiles, pszMemName,
                              pszMarkup, pszEditor, pszSourceLanguage,
                              pszTargetLanguage, pszAlias, pszStartPath,
                              pszConversion, lOptions );
  }
  else
  {
    // continue current document import
    usRC = DocFuncImportProcess( pData );
  } /* endif */
  return( usRC );
} /* end of function DocFuncImportDoc */

// Prepare the import of documents
USHORT DocFuncPrepImport
(
PFCTDATA    pData,                   // function I/F session data
PSZ         pszFolderName,           // name of folder receiving the documents
PSZ         pszFiles,                // list of input files (documents)
PSZ         pszMemName,              // document Translation Memory or NULL
PSZ         pszMarkup,               // document markup or NULL
PSZ         pszEditor,               // document editor or NULL
PSZ         pszSourceLanguage,       // document source language or NULL
PSZ         pszTargetLanguage,       // document target language or NULL
PSZ         pszAlias,                // alias for document name or NULL
PSZ         pszStartPath,            // optional start path
PSZ         pszConversion,           // optional document export conversion
LONG        lOptions                 // document import options or 0L
)
{
  USHORT      usRC = NO_ERROR;         // function return code
  PDOCIMPEXP  pDocImpExp = NULL;       // document import /export struct
  BOOL        fOK = TRUE;              // internal O.K. flag

  // allocate document import/export IDA
  fOK = UtlAllocHwnd( (PVOID *)&pDocImpExp, 0L, (LONG) sizeof(DOCIMPEXP),
                      ERROR_STORAGE, HWND_FUNCIF );
  if ( !fOK )
  {
    usRC = ERROR_STORAGE;
  } /* endif */

  // copy options to import/export IDA
  if ( fOK )
  {
    pDocImpExp->lOptions = lOptions;
  } /* endif */     


  // check folder name
  if ( fOK )
  {
    if ( (pszFolderName == NULL) || (*pszFolderName == EOS) )
    {
      fOK = FALSE;
      usRC = TA_MANDFOLDER;
      UtlErrorHwnd( usRC, MB_CANCEL, 0, NULL, EQF_ERROR, HWND_FUNCIF );
    }
    else if ( !SubFolNameToObjectName( pszFolderName,  pDocImpExp->szBuffer ) )
    {
      PSZ pszParm = pszFolderName;
      fOK = FALSE;
      usRC = ERROR_XLATE_FOLDER_NOT_EXIST;
      UtlErrorHwnd( usRC, MB_CANCEL, 1, &pszParm, EQF_ERROR, HWND_FUNCIF );
    }
    else if ( QUERYSYMBOL( pDocImpExp->szBuffer  ) != -1 )
    {
      PSZ pszParm = pszFolderName;
      fOK = FALSE;
      usRC = ERROR_FOLDER_LOCKED;
      UtlErrorHwnd( usRC, MB_CANCEL, 1, &pszParm, EQF_ERROR, HWND_FUNCIF );
    } /* endif */
  } /* endif */

  // check if document names have been specified
  if ( fOK )
  {
    if ( (pszFiles == NULL) || (*pszFiles == EOS) )
    {
      fOK = FALSE;
      usRC = FUNC_MANDFILES;
      UtlErrorHwnd( usRC, MB_CANCEL, 0, NULL, EQF_ERROR, HWND_FUNCIF );
    } /* endif */
  } /* endif */

  // check TM name if specified
  if ( fOK )
  {
    if ( (pszMemName != NULL) && (*pszMemName != EOS) )
    {
      if ( !UtlCheckIfExist( pszMemName, TM_OBJECT ) )
      {
        PSZ pszParm = pszMemName;
        fOK = FALSE;
        usRC = ERROR_MEMORY_NOTFOUND;
        UtlErrorHwnd( usRC, MB_CANCEL, 1,
                      &pszParm, EQF_ERROR, HWND_FUNCIF );
      } /* endif */
    }
    else
    {
      pszMemName = NULL;
    } /* endif */
  } /* endif */

  //  check markup name if specified
  if ( fOK )
  {
    if ( (pszMarkup != NULL) && (*pszMarkup != EOS) )
    {
      if ( !UtlCheckIfExist( pszMarkup, MARKUP_OBJECT ) )
      {
        PSZ pszParm = pszMarkup;
        fOK = FALSE;
        usRC = ERROR_NO_FORMAT_TABLE_AVA;
        UtlErrorHwnd( usRC, MB_CANCEL, 1,
                      &pszParm, EQF_ERROR, HWND_FUNCIF );
      } /* endif */
    }
    else
    {
      pszMarkup = NULL;
    } /* endif */
  } /* endif */

  // check source language if specified
  if ( fOK )
  {
    if ( (pszSourceLanguage != NULL) && (*pszSourceLanguage != EOS) )
    {
      if ( !UtlCheckIfExist( pszSourceLanguage, SOURCE_LANGUAGE_OBJECT ) )
      {
        PSZ pszParm = pszSourceLanguage;
        usRC = ERROR_PROPERTY_LANG_DATA;
        UtlErrorHwnd( ERROR_PROPERTY_LANG_DATA, MB_CANCEL, 1,
                      &pszParm, EQF_ERROR, HWND_FUNCIF );
        fOK = FALSE;
      } /* endif */
    }
    else
    {
      pszSourceLanguage = NULL;
    } /* endif */
  } /* endif */

  // check target language if specified
  if ( fOK )
  {
    if ( (pszTargetLanguage != NULL) && (*pszTargetLanguage != EOS) )
    {
      if ( !UtlCheckIfExist( pszTargetLanguage, TARGET_LANGUAGE_OBJECT ) )
      {
        PSZ pszParm = pszTargetLanguage;
        usRC = ERROR_PROPERTY_LANG_DATA;
        UtlErrorHwnd( ERROR_PROPERTY_LANG_DATA, MB_CANCEL, 1,
                      &pszParm, EQF_ERROR, HWND_FUNCIF );
        fOK = FALSE;
      } /* endif */
    }
    else
    {
      pszTargetLanguage = NULL;
    } /* endif */
  } /* endif */

  // Check editor if specified
  if ( fOK )
  {
    if ( (pszEditor != NULL) && (*pszEditor != EOS) )
    {
      if ( !UtlCheckIfExist( pszEditor, EDITOR_OBJECT ) )
      {
        PSZ pszParm = pszEditor;
        fOK = FALSE;
        usRC = ERROR_EDITOR_NOT_FOUND;
        UtlErrorHwnd( usRC, MB_CANCEL, 1,
                      &pszParm, EQF_ERROR, HWND_FUNCIF );
      } /* endif */
    }
    else
    {
      pszEditor = NULL;
    } /* endif */
  } /* endif */

  // Check alias if specified
  if ( fOK )
  {
    if ( (pszAlias != NULL) && (*pszAlias != EOS) )
    {
      // currently no check for alias names
    }
    else
    {
      pszAlias = NULL;
    } /* endif */
  } /* endif */

  // process specified document names
  if ( fOK )
  {
  } /* endif */

  // fill IDA fields
  if ( fOK )
  {
    pDocImpExp->hwndErrMsg = HWND_FUNCIF;

    pData->pvDocImpExpIda = pDocImpExp;
    pData->fComplete = FALSE;

    strcpy( pDocImpExp->chFldName, pszFolderName );
    UtlStripBlanks(pDocImpExp->chFldName);

    pDocImpExp->fOverwrite = (lOptions & OVERWRITE_OPT) != 0L;

    if ( pszMemName )
    {
      strcpy( pDocImpExp->szMem, pszMemName);
      strupr( pDocImpExp->szMem );
    } /* endif */
    if ( pszMarkup )
    {
      strcpy( pDocImpExp->szFormat, pszMarkup );
      strupr( pDocImpExp->szFormat );
    } /* endif */
    if ( pszEditor )
    {
      strcpy( pDocImpExp->szEdit, pszEditor );
      strupr( pDocImpExp->szEdit );
    } /* endif */
    if ( pszAlias )
    {
      int iMax = sizeof(pDocImpExp->szAlias) - 1;
      strncpy( pDocImpExp->szAlias, pszAlias, iMax );
      pDocImpExp->szAlias[iMax] = EOS;
    } /* endif */
    if ( pszSourceLanguage )
    {
      strcpy( pDocImpExp->szSourceLang, pszSourceLanguage );
      UtlUpper( pDocImpExp->szSourceLang );
    } /* endif */
    if ( pszTargetLanguage )
    {
      strcpy( pDocImpExp->szTargetLang, pszTargetLanguage );
      UtlUpper( pDocImpExp->szTargetLang );
    } /* endif */
    if ( pszConversion )
    {
      strncpy( pDocImpExp->szConversion, pszConversion,
               sizeof(pDocImpExp->szConversion)-1 );
    } /* endif */
    if ( pszStartPath )
    {
      strncpy( pDocImpExp->szStartPath, pszStartPath,
               sizeof(pDocImpExp->szStartPath)-1 );
    } /* endif */

    if ( *pszFiles == LISTINDICATOR )
    {
      strncpy( pDocImpExp->chListName, pszFiles + 1,
               sizeof(pDocImpExp->chListName)-1 );
      UtlStripBlanks( pDocImpExp->chListName );

      fOK = UtlListOfFiles( NULL, pDocImpExp->chListName, &(pDocImpExp->ppFileArray) );
      if ( fOK )
      {
        // get number of files given
        pDocImpExp->usFileNums = 0;
        while ( pDocImpExp->ppFileArray[pDocImpExp->usFileNums] )
        {
          pDocImpExp->usFileNums++;
        } /* endwhile */
      }
      else
      {
        PSZ pData = pDocImpExp->chListName;
        usRC = DDE_ERRFILELIST;
        fOK = FALSE;
        UtlErrorHwnd( usRC, MB_CANCEL, 1,
                      &pData, EQF_ERROR, HWND_FUNCIF );
      } /* endif */
    }
    else
    {
      // work against list of files
      // make work copy of file name list
      {
        ULONG ulLen;                     // length field

        ulLen = strlen(pszFiles) + 1;
        ulLen = max( ulLen, MIN_ALLOC );
        fOK = UtlAllocHwnd( (PVOID *)&(pDocImpExp->pszFileNameBuffer), 0L,
                            ulLen, ERROR_STORAGE, HWND_FUNCIF );
        if ( fOK )
        {
          strcpy( pDocImpExp->pszFileNameBuffer, pszFiles );
        }
        else
        {
          usRC = ERROR_STORAGE;
        } /* endif */
      }

      // get max number of documents within string (= number of delimiters + 1)
      if ( fOK )
      {
        PSZ pszTemp = pDocImpExp->pszFileNameBuffer;
        pDocImpExp->usFileNums = 1;
        while ( *pszTemp )
        {
          if ( *pszTemp == NAME_DELIMITER )
          {
            pDocImpExp->usFileNums++;
          } /* endif */
          pszTemp++;
        } /* endwhile */
      } /* endif */

      // allocate file name pointer array
      if ( fOK )
      {
        ULONG ulLen;                     // length field

        ulLen = (pDocImpExp->usFileNums + 1) * sizeof(PSZ);
        ulLen = max( ulLen, MIN_ALLOC );
        fOK = UtlAllocHwnd( (PVOID *)&(pDocImpExp->ppFileArray), 0L,
                            ulLen, ERROR_STORAGE, HWND_FUNCIF );
        if ( !fOK )
        {
          usRC = ERROR_STORAGE;
        } /* endif */
      } /* endif */

      // fill file name pointers
      if ( fOK )
      {
        PSZ pszCurrent = pDocImpExp->pszFileNameBuffer;
        PSZ pszNext = NULL;
        pDocImpExp->usFileNums = 0;

        while ( UtlGetNextFileFromCommaList( &pszCurrent, &pszNext ) )
        {
          pDocImpExp->ppFileArray[pDocImpExp->usFileNums++] = pszCurrent;
        } /* endwhile */
      } /* endif */
    } /* endif */
  } /* endif */

  // free allocated resource
  if ( !fOK )
  {
    pData->fComplete = TRUE;
    if ( pDocImpExp )
    {
      if ( pDocImpExp->pszFileNameBuffer )
      {
        UtlAlloc( (PVOID *)&pDocImpExp->pszFileNameBuffer, 0L, 0L, NOMSG );
      } /* endif */
      if ( pDocImpExp->ppFileArray )
      {
        UtlAlloc( (PVOID *)&pDocImpExp->ppFileArray, 0L, 0L, NOMSG );
      } /* endif */
      UtlAlloc( (PVOID *)&pDocImpExp, 0L, 0L, NOMSG );
    } /* endif */
    pData->pvDocImpExpIda = NULL;
  } /* endif */

  return( usRC );
} /* end of function DocFuncPrepImport */

// Do the actual document export
USHORT DocFuncImportProcess
(
PFCTDATA    pData                    // function I/F session data
)
{
  PDOCIMPEXP   pDocImpIda;             // document import ida
  BOOL         fOK = TRUE;             // error flag
  PSZ          pInFile;                // input file name
  PSZ          pFileList = NULL;       // pointer to file list
  USHORT       usRC = NO_ERROR;        // function return code

  pDocImpIda = (PDOCIMPEXP)pData->pvDocImpExpIda;

  pInFile = *(pDocImpIda->ppFileArray + pDocImpIda->usActFile);

  if ( fOK )
  {
    fOK = ResolveMultFileNamesEx( pInFile, pDocImpIda->szStartPath, &pFileList, (pDocImpIda->lOptions & NOSUBDIRSEARCH_OPT) != 0 );
  } /* endif */

  if ( fOK )
  {
    if ( pFileList )
    {
      pInFile = pFileList;
      while ( fOK && *pInFile )
      {
        fOK = DDEDocLoadWork( pDocImpIda, pInFile );
        pInFile += strlen(pInFile) + 1;
      } /* endwhile */
      UtlAlloc( (PVOID *) &pFileList, 0L, 0L, NOMSG );
    }
    else
    {
      fOK = DDEDocLoadWork( pDocImpIda, pInFile );
    } /* endif */
  } /* endif */

  if ( fOK )
  {
    pDocImpIda->usActFile++;
    if ( pDocImpIda->usActFile >= pDocImpIda->usFileNums )
    {
      // all documents are processed
      pData->fComplete = TRUE;
    } /* endif */
  }
  else
  {
    usRC = UtlQueryUShort( QS_LASTERRORMSGID );
    pData->fComplete = TRUE;
  } /* endif */

  // cleanup if process is terminated
  if ( pData->fComplete )
  {
    if ( pDocImpIda )
    {
      if ( pDocImpIda->pszFileNameBuffer )
      {
        UtlAlloc( (PVOID *)&pDocImpIda->pszFileNameBuffer, 0L, 0L, NOMSG );
      } /* endif */
      if ( pDocImpIda->ppFileArray )
      {
        UtlAlloc( (PVOID *)&pDocImpIda->ppFileArray, 0L, 0L, NOMSG );
      } /* endif */
      UtlAlloc( (PVOID *)&pDocImpIda, 0L, 0L, NOMSG );
    } /* endif */
    pData->pvDocImpExpIda = NULL;
  } /* endif */

  return( usRC );

}/* end of function DocFuncImportProcess */

USHORT DocFuncExportDoc
(
PFCTDATA    pData,                   // function I/F session data
PSZ         pszFolderName,           // name of folder
PSZ         pszFiles,                // list of documents with path information
PSZ         pszStartPath,            // optional start path
LONG        lOptions                 // options for document export
)
{
  USHORT      usRC = NO_ERROR;         // function return code

  // prepare a new document import or continue current one
  if ( pData->fComplete )              // has last run been completed
  {
    // prepare a new document export
    usRC = DocFuncPrepExport( pData, pszFolderName, pszFiles, pszStartPath, lOptions );
  }
  else
  {
    // continue current document export
    usRC = DocFuncExportProcess( pData );
  } /* endif */
  return( usRC );
} /* end of function DocFuncExportDoc */


USHORT DocFuncPrepExport
(
PFCTDATA    pData,                   // function I/F session data
PSZ         pszFolderName,           // name of folder
PSZ         pszFiles,                // list of documents with path information
PSZ         pszStartPath,            // optional start path
LONG        lOptions                 // options for document export
)
{
  USHORT      usRC = NO_ERROR;         // function return code
  PDOCIMPEXP  pDocImpExp = NULL;       // document import /export struct
  BOOL        fOK = TRUE;              // internal O.K. flag


  // allocate document import/export IDA
  fOK = UtlAllocHwnd( (PVOID *)&pDocImpExp, 0L, (LONG) sizeof(DOCIMPEXP),
                      ERROR_STORAGE, HWND_FUNCIF );
  if ( !fOK )
  {
    usRC = ERROR_STORAGE;
  } /* endif */

  // copy options to import/export IDA
  if ( fOK )
  {
    pDocImpExp->lOptions = lOptions;
  } /* endif */     

  // check folder name
  if ( fOK )
  {
    if ( (pszFolderName == NULL) || (*pszFolderName == EOS) )
    {
      fOK = FALSE;
      usRC = TA_MANDFOLDER;
      UtlErrorHwnd( usRC, MB_CANCEL, 0, NULL, EQF_ERROR, HWND_FUNCIF );
    }
    else 
    {
      // SubFolNameToObjectName expects the folder name to be in ASCII encoding...
      strcpy( pDocImpExp->chFldName, pszFolderName );
      ANSITOOEM( pDocImpExp->chFldName );

      if ( !SubFolNameToObjectName( pDocImpExp->chFldName,  pDocImpExp->szBuffer ) )
      {
        PSZ pszParm = pszFolderName;
        fOK = FALSE;
        usRC = ERROR_XLATE_FOLDER_NOT_EXIST;
        UtlErrorHwnd( usRC, MB_CANCEL, 1, &pszParm, EQF_ERROR, HWND_FUNCIF );
      }
      else if ( QUERYSYMBOL( pDocImpExp->szBuffer  ) != -1 )
      {
        PSZ pszParm = pszFolderName;
        fOK = FALSE;
        usRC = ERROR_FOLDER_LOCKED;
        UtlErrorHwnd( usRC, MB_CANCEL, 1, &pszParm, EQF_ERROR, HWND_FUNCIF );
      }
      else
      {
        // get subfolder/folder ID
        pDocImpExp->ulSubFolderID = FolGetSubFolderIdFromObjName( pDocImpExp->szBuffer );
      } /* endif */
    } /* endif */
  } /* endif */

  // check specified export options
  if ( fOK )
  {
    // validation format export may not be used together with the other export options
    int iFormats = 0;
    if ( lOptions & (SOURCE_OPT | TARGET_OPT | SNOMATCH_OPT) ) iFormats++; 
    if ( lOptions & PLAINXML_OPT ) iFormats++; 
    if ( lOptions & VALFORMAT_XML_OPT ) iFormats++; 
    if ( lOptions & VALFORMAT_HTML_OPT ) iFormats++; 
    if ( lOptions & VALFORMAT_DOC_OPT ) iFormats++; 
    if ( lOptions & VALFORMAT_DOCX_OPT ) iFormats++; 
    if ( lOptions & VALFORMAT_ODT_OPT ) iFormats++; 
    if ( iFormats > 1 )
    {
      fOK = FALSE;
      usRC = WRONG_OPTIONS_RC;
    } /* endif */
    pDocImpExp->fValCombine  = (lOptions &  VALFORMAT_COMBINE_OPT) != 0;
    pDocImpExp->fValProtSegs = (lOptions &  VALFORMAT_PROTSEGS_OPT) != 0;
    pDocImpExp->fValFormat = (lOptions &  (VALFORMAT_XML_OPT | VALFORMAT_HTML_OPT | VALFORMAT_DOC_OPT | VALFORMAT_DOCX_OPT | VALFORMAT_ODT_OPT)) != 0;
    pDocImpExp->lValOutputFormat = (LONG)(lOptions &  (VALFORMAT_XML_OPT | VALFORMAT_HTML_OPT | VALFORMAT_DOC_OPT | VALFORMAT_DOCX_OPT | VALFORMAT_ODT_OPT));
    pDocImpExp->fPlainXML = (lOptions & PLAINXML_OPT) != 0;
    pDocImpExp->fWithTrackID = (lOptions & WITHTRACKID_OPT) != 0;
  } /* endif */

  // check if document names have been specified
  if ( fOK )
  {
    if ( (pszFiles == NULL) || (*pszFiles == EOS) )
    {
      fOK = FALSE;
      usRC = FUNC_MANDFILES;
      UtlErrorHwnd( usRC, MB_CANCEL, 0, NULL, EQF_ERROR, HWND_FUNCIF );
    } /* endif */
  } /* endif */

  // fill IDA fields
  if ( fOK )
  {
    pDocImpExp->hwndErrMsg = HWND_FUNCIF;
    strncpy( pDocImpExp->chFldName, pszFolderName, sizeof(pDocImpExp->chFldName)-1 );


    if ( pszStartPath != NULL )
    {
      strncpy( pDocImpExp->szStartPath, pszStartPath, sizeof(pDocImpExp->szStartPath)-1 );
    } /* endif */

    if ( *pszFiles == LISTINDICATOR )
    {
      // store the list file
      strncpy( pDocImpExp->chListName, pszFiles,
               sizeof(pDocImpExp->chListName)-1 );
      fOK = UtlListOfFiles( NULL, pDocImpExp->chListName, &(pDocImpExp->ppFileArray) );
      if ( fOK )
      {
        // get number of files given
        pDocImpExp->usFileNums = 0;
        while ( pDocImpExp->ppFileArray[pDocImpExp->usFileNums] )
        {
          pDocImpExp->usFileNums++;
        } /* endwhile */
      }
      else
      {
        usRC = DDE_ERRFILELIST;
        UtlErrorHwnd( usRC, MB_CANCEL, 1,
                      &pszFiles, EQF_ERROR, HWND_FUNCIF );
      } /* endif */
    }
    else
    {
      // work against list of files
      {
        ULONG ulLen;                     // length field

        ulLen = strlen(pszFiles) + 1;
        ulLen = max( ulLen, MIN_ALLOC );
        fOK = UtlAllocHwnd( (PVOID *)&(pDocImpExp->pszFileNameBuffer), 0L,
                            ulLen, ERROR_STORAGE, HWND_FUNCIF );
        if ( fOK )
        {
          strcpy( pDocImpExp->pszFileNameBuffer, pszFiles );
        }
        else
        {
          usRC = ERROR_STORAGE;
        } /* endif */
      }

      // get max number of documents within string (= number of delimiters + 1)
      if ( fOK )
      {
        PSZ pszTemp = pDocImpExp->pszFileNameBuffer;
        pDocImpExp->usFileNums = 1;
        while ( *pszTemp )
        {
          if ( *pszTemp == NAME_DELIMITER )
          {
            pDocImpExp->usFileNums++;
          } /* endif */
          pszTemp++;
        } /* endwhile */
      } /* endif */

      // allocate file name pointer array
      if ( fOK )
      {
        ULONG ulLen;                     // length field

        ulLen = (pDocImpExp->usFileNums + 1) * sizeof(PSZ);
        ulLen = max( ulLen, MIN_ALLOC );
        fOK = UtlAllocHwnd( (PVOID *)&(pDocImpExp->ppFileArray), 0L,
                            ulLen, ERROR_STORAGE, HWND_FUNCIF );
        if ( !fOK )
        {
          usRC = ERROR_STORAGE;
        } /* endif */
      } /* endif */

      // fill file name pointers
      if ( fOK )
      {
        PSZ pszCurrent = pDocImpExp->pszFileNameBuffer;
        PSZ pszNext = NULL;
        pDocImpExp->usFileNums = 0;
        while ( UtlGetNextFileFromCommaList( &pszCurrent, &pszNext ) )
        {
          pDocImpExp->ppFileArray[pDocImpExp->usFileNums++] = pszCurrent;
        } /* endwhile */
      } /* endif */
    } /* endif */

    // set options
    if ( fOK )
    {
//    LONG lTest1 = lOptions & OVERWRITE_OPT;
//    LONG lTest2 = lOptions & OPENTM2FORMAT_OPT;
      if ( lOptions & OVERWRITE_OPT )
      {
        pDocImpExp->fOverwrite = TRUE;
      } /* endif */

      if ( lOptions & OPENTM2FORMAT_OPT )
      {
        pDocImpExp->fInternal = TRUE;
      }
      else
      {
        if ( lOptions & SOURCE_OPT )
        {
          pDocImpExp->fSource = TRUE;
        } /* endif */
        if ( lOptions & SNOMATCH_OPT )
        {
          pDocImpExp->fSNOMATCH = TRUE;
        } /* endif */
      }
    } /* endif */
  } /* endif */

  if ( fOK )
  {
    pData->pvDocImpExpIda = pDocImpExp;
    pData->fComplete = FALSE;

    if ( pDocImpExp->fValFormat || pDocImpExp->fPlainXML )
    {
      int iRC = 0;
      DOCEXPVALOPTIONS Options;

      memset( &Options, 0, sizeof(Options) );
      if ( pDocImpExp->fPlainXML )
      {
        Options.ValFormat = PLAINXML_VALEXPFORMAT;
      }
      else if ( pDocImpExp->lValOutputFormat & VALFORMAT_ODT_OPT )
      {
        Options.ValFormat = ODT_VALEXPFORMAT;
      }
      else if ( pDocImpExp->lValOutputFormat & VALFORMAT_XML_OPT )
      {
        Options.ValFormat = XML_VALEXPFORMAT;
      }
      else if ( pDocImpExp->lValOutputFormat & VALFORMAT_DOC_OPT )
      {
        Options.ValFormat = DOC_VALEXPFORMAT;
      }
      else if ( pDocImpExp->lValOutputFormat & VALFORMAT_DOCX_OPT )
      {
        Options.ValFormat = DOCX_VALEXPFORMAT;
      }
      else
      {
        Options.ValFormat = HTML_VALEXPFORMAT;
      } /* endif */
      Options.fCombined  = pDocImpExp->fValCombine;

      // as the individual options cannot be set right now, we use the defaults
      Options.fNewMatch   = TRUE;
      Options.fProtMatch = (EQF_BOOL)pDocImpExp->fValProtSegs;
      Options.fAutoMatch  = TRUE;
      Options.fNotTransl  = TRUE;
      Options.fFuzzyMatch = TRUE;
      Options.fExactMatch = TRUE;
      Options.fMachMatch  = TRUE;
      Options.fReplMatch  = TRUE;
      Options.fModifiedAuto = TRUE;
      Options.fModifiedExact = TRUE;
      Options.fGlobMemMatch = TRUE;


      // use directory of first specified document as output path if no startpath has been specified
      if ( pDocImpExp->szStartPath[0] == EOS )
      {
        PSZ pszPathEnd;

        strcpy( pDocImpExp->szStartPath, pDocImpExp->ppFileArray[0] );
        pszPathEnd = strrchr( pDocImpExp->szStartPath, BACKSLASH );
        if ( pszPathEnd )
        {
          *pszPathEnd = EOS;
        }
        else
        {
          // use current directory as startpath
          _getcwd( pDocImpExp->szStartPath, sizeof(pDocImpExp->szStartPath) );
        } /* endif */
      } /* endif */
      iRC = DocExpValInit( &Options, pDocImpExp->szStartPath, pDocImpExp->szBuffer, 
                           &(pDocImpExp->lValExportHandle), pDocImpExp->fOverwrite, HWND_FUNCIF );
    } /* endif */ 
  }
  else
  {
    pData->fComplete = TRUE;
    if ( pDocImpExp )
    {
      if ( pDocImpExp->pszFileNameBuffer )
      {
        UtlAlloc( (PVOID *)&pDocImpExp->pszFileNameBuffer, 0L, 0L, NOMSG );
      } /* endif */
      if ( pDocImpExp->ppFileArray )
      {
        UtlAlloc( (PVOID *)&pDocImpExp->ppFileArray, 0L, 0L, NOMSG );
      } /* endif */
      UtlAlloc( (PVOID *)&pDocImpExp, 0L, 0L, NOMSG );
    } /* endif */
    pData->pvDocImpExpIda = NULL;
  } /* endif */

  return( usRC );
} /* end of function DocFuncPrepExport */

USHORT DocFuncExportProcess
(
PFCTDATA    pData                    // function I/F session data
)
{
  PDOCIMPEXP   pDocExpIda;             // document export/import ida
  BOOL         fOK = TRUE;             // error flag
  PSZ          pFileList = NULL;       // pointer to file list
  USHORT       usRC = NO_ERROR;        // function return code
  CHAR         szBuf[ MAX_FULLPATH ];
  PSZ          pExpFile;               // pointer to export file name (full)
  PSZ          pTempFile;
  PSZ          pFile;                  // pointer to export file name
  PSZ          pFolder = NULL;         // pointer to folder name
  PDDERETURN   pDDEReturn;
  CHAR         szFolderBuf[ MAX_EQF_PATH ];
  ULONG        ulFolderPathLen= 0;        // length of folder path in fully qualified document names
  CHAR         szDoc[ MAX_LONGPATH ];

  pDocExpIda = (PDOCIMPEXP)pData->pvDocImpExpIda;
  pDDEReturn = &pDocExpIda->DDEReturn;

  pExpFile = *(pDocExpIda->ppFileArray + pDocExpIda->usActFile);

  if ( fOK )
  {
    HPROP        hpropFolder;            // return from folder props
    PPROPFOLDER  pProp;                  // pointer to folder props
    ULONG        ulErrorInfo;            // error indicator from PRHA

    // create folder name ...
    {
      BOOL fIsNew;
      CHAR szShortName[MAX_FILESPEC];
      PSZ pszDelimiter;

      strcpy( pDocExpIda->szBuffer, pDocExpIda->chFldName );
      pszDelimiter = strchr( pDocExpIda->szBuffer, BACKSLASH );
      if ( pszDelimiter ) *pszDelimiter = EOS;

      ANSITOOEM( pDocExpIda->szBuffer );
      ObjLongToShortName( pDocExpIda->szBuffer, szShortName, FOLDER_OBJECT, &fIsNew );
      UtlMakeEQFPath( szFolderBuf, NULC, SYSTEM_PATH, szShortName );
    }

    strcat( szFolderBuf, EXT_FOLDER_MAIN );

    if ( (hpropFolder = OpenProperties( szFolderBuf, NULL,
                                         PROP_ACCESS_READ, &ulErrorInfo)) == NULL)
    {
      fOK = FALSE;
      // display error message if not already displayed
      if ( ulErrorInfo != Err_NoStorage )
      {
        pFolder = pDocExpIda->chFldName;                 // set folder name
        pDDEReturn->usRc = ERROR_XLATE_FOLDER_NOT_EXIST;
        UtlErrorHwnd( pDDEReturn->usRc, MB_CANCEL, 1, &pFolder, EQF_ERROR, HWND_FUNCIF );
      } /* endif */
    }
    else
    {
      pProp = (PPROPFOLDER)MakePropPtrFromHnd( hpropFolder );
      szFolderBuf[0] = pProp->chDrive;
      CloseProperties( hpropFolder, PROP_QUIT, &ulErrorInfo );
      pFolder = UtlGetFnameFromPath( szFolderBuf );
    }/*endif*/
  } /* endif */

  // construct filename depending on requested file type and export
  // the file.
  if ( fOK )
  {
    if ( pDocExpIda->fValFormat || pDocExpIda->fPlainXML )
    {
      // use STARGET path and later on remove STARGET dir from the name...
      UtlMakeEQFPath( szBuf, szFolderBuf[0], DIRSEGTARGETDOC_PATH, pFolder );
    }
    else if ( pDocExpIda->fSource )
    {
      UtlMakeEQFPath( szBuf, szFolderBuf[0], DIRSOURCEDOC_PATH, pFolder );
    }
    else if ( pDocExpIda->fSNOMATCH )
    {
      UtlMakeEQFPath( szBuf, szFolderBuf[0], DIRSEGNOMATCH_PATH, pFolder );
    }
    else
    {
      UtlMakeEQFPath( szBuf, szFolderBuf[0], DIRSEGTARGETDOC_PATH, pFolder );
    } /* endif */

    // when startpath is given the document names may not contain the fully qualified target path
    if ( pDocExpIda->szStartPath[0] != EOS )
    {
      if ( strncmp( pDocExpIda->szStartPath, pExpFile, strlen(pDocExpIda->szStartPath) ) == 0 )
      {
        pFile = pExpFile + strlen(pDocExpIda->szStartPath);
        if ( *pFile == BACKSLASH ) pFile++;
      }
      else
      {
        pFile = pExpFile;
      } /* endif */
    }
    else
    {
      pFile = UtlGetFnameFromPath( pExpFile );
      if ( pFile == NULL ) pFile = pExpFile;
    } /* endif */

    strcat( szBuf, BACKSLASH_STR );
    ulFolderPathLen = strlen(szBuf);
    strcat( szBuf, pFile );

    // special handling for single/multiple substitution characters
    // in document name as document names with path information are not handled
    // correctly in ResolveLongMultFileNames
    if ( (strchr( szBuf, '*') != NULL) || (strchr( szBuf, '?') != NULL) )
    {
      // remove any path information of document name from szBuf and replace it
      // by search pattern
      strcpy( pDocExpIda->szBuffer, szBuf + ulFolderPathLen );
      szBuf[ulFolderPathLen] = EOS;
      strcat( szBuf, DEFAULT_PATTERN );
      fOK = ResolveLongMultFileNames( szBuf, pDocExpIda->szBuffer, &pFileList, pDocExpIda->ulSubFolderID );
    } /* endif */

    if (!fOK)
    {
      PSZ apszErrParm[2];

      apszErrParm[0] = pFileList;
      apszErrParm[1] = pDocExpIda->chFldName;
      pDDEReturn->usRc = DDE_DOC_NOT_IN_FOLDR;
      UtlErrorHwnd( DDE_DOC_NOT_IN_FOLDR, MB_CANCEL, 2, apszErrParm,
                    EQF_ERROR, HWND_FUNCIF );
    }/*endif*/
  } /* endif */

  if ( fOK )
  {
    if ( strlen( pExpFile ) < sizeof( szBuf ) )
    {
      strcpy( szBuf, pExpFile );
    }
    else
    {
      // passed name is not valid
      fOK = FALSE;
      pDDEReturn->usRc = UTL_PARAM_TOO_LONG;
      UtlErrorHwnd( pDDEReturn->usRc, MB_CANCEL, 1,
                    &pExpFile, EQF_ERROR, HWND_FUNCIF );
    } /* endif */
  } /* endif */

  if ( fOK )
  {
    // pFile now points to start of file name in szBuf...
    if ( pDocExpIda->szStartPath[0] != EOS )
    {
      if ( strncmp( pDocExpIda->szStartPath, szBuf, strlen(pDocExpIda->szStartPath) ) == 0 )
      {
        pFile = szBuf + strlen(pDocExpIda->szStartPath);
        if ( *pFile == BACKSLASH ) pFile++;
      }
      else
      {
        pFile = szBuf;
      } /* endif */
    }
    else
    {
      pFile = UtlGetFnameFromPath( szBuf );
      if ( pFile == NULL ) pFile = szBuf;
    } /* endif */

    if ( pFileList )
    {
      PSZ pszOrgExpFile = pExpFile;

      pExpFile = pFileList;
      while ( fOK && *pExpFile )
      {
        PSZ pszExportFile;

        pTempFile = pExpFile + ulFolderPathLen;
        strcpy( pFile, pTempFile );
        pszExportFile = pFile;

        // skip relative path name when exporting in non-relative-path-mode
        if ( pDocExpIda->lOptions & WITHOUTRELATIVEPATH_OPT )
        {
          pszExportFile = UtlGetFnameFromPath( pFile );
          if ( pszExportFile == NULL ) pszExportFile = pFile;
        }

        // copy name of current document to temporary buffer
        strcpy( szDoc, pszExportFile );

        // prefix document name with start path (is required by DDEDocUnloadWork)
        if ( pDocExpIda->szStartPath[0] != EOS )
        {
          // do this only if document name is not already prefixed with the startpath ...
          if ( strncmp( pDocExpIda->szStartPath, pszExportFile, strlen(pDocExpIda->szStartPath) ) != 0 )
          {
            strcpy( szDoc, pDocExpIda->szStartPath );
            if ( (szDoc[strlen(szDoc)-1] != BACKSLASH) && (szBuf[0] != BACKSLASH) )
            {
              strcat( szDoc, BACKSLASH_STR );
            } /* endif */
            strcat( szDoc, pszExportFile );
          } /* endif */
        }
        else
        {
          PSZ pszPathEnd;

          // prefix document with path information from original export file
          strcpy( szDoc, pszOrgExpFile );
          pszPathEnd = strrchr( szDoc, BACKSLASH );
          if ( pszPathEnd )
          {
            pszPathEnd[1] = EOS;
          }
          else
          {
            szDoc[0] = EOS;
          } /* endif */
          strcat( szDoc, pszExportFile );
        } /* endif */

        if ( pDocExpIda->fValFormat || pDocExpIda->fPlainXML )
        {

          OBJNAME szDocObjName;
          int iRC = 0;
          BOOL fIsNew = FALSE;

          strcpy( szDocObjName, szFolderBuf );
          strcat( szDocObjName, BACKSLASH_STR );
          FolLongToShortDocName( szFolderBuf , pFile, szDocObjName + strlen(szDocObjName), &fIsNew );
          iRC = DocExpValProcess( pDocExpIda->lValExportHandle, szDocObjName );
          if ( iRC ) fOK = FALSE;
        }
        else if ( pDocExpIda->fInternal )
        {

          CHAR szDocShortName[MAX_EQF_PATH];
          USHORT usExportRC = 0;
          BOOL fIsNew = FALSE;

          FolLongToShortDocName( szFolderBuf, pFile, szDocShortName, &fIsNew );

          usExportRC = DocExpExportInternal( szFolderBuf, pFile, szDocShortName, pDocExpIda->szStartPath[0], HWND_FUNCIF, &(pDocExpIda->fOverwrite), NULL );

          if ( usExportRC != UNLOAD_OK ) fOK = FALSE;
        }
        else
        {
          fOK = DDEDocUnLoadWork( pDocExpIda, szFolderBuf, szDoc, pFile );
        } /* endif */
        pExpFile += strlen(pExpFile) + 1;
      } /* endwhile */
      UtlAlloc( (PVOID *) &pFileList, 0L, 0L, NOMSG );
    }
    else
    {
      // copy name of current document to temporary buffer
      strcpy( szDoc, szBuf );

      // prefix document name with start path (is required by DDEDocUnloadWork)
      if ( pDocExpIda->szStartPath[0] != EOS )
      {
        // do this only if document name is not already prefixed with the startpath ...
        if ( strncmp( pDocExpIda->szStartPath, szBuf, strlen(pDocExpIda->szStartPath) ) != 0 )
        {
          strcpy( szDoc, pDocExpIda->szStartPath );
          if ( (szDoc[strlen(szDoc)-1] != BACKSLASH) && (szBuf[0] != BACKSLASH) )
          {
            strcat( szDoc, BACKSLASH_STR );
          } /* endif */
          strcat( szDoc, szBuf );
        } /* endif */
      } /* endif */
      if ( pDocExpIda->fValFormat || pDocExpIda->fPlainXML )
      {
          OBJNAME szDocObjName;
          BOOL fIsNew = FALSE;
          int iRC = 0;

          strcpy( szDocObjName, szFolderBuf );
          strcat( szDocObjName, BACKSLASH_STR );
          FolLongToShortDocName( szFolderBuf , pFile, szDocObjName + strlen(szDocObjName), &fIsNew );
          iRC = DocExpValProcess( pDocExpIda->lValExportHandle, szDocObjName );
          if ( iRC ) fOK = FALSE;
      }
      else if ( pDocExpIda->fInternal )
      {
        OBJNAME szDocShortName;
        BOOL fIsNew = FALSE;
        USHORT usRC = 0;
        FolLongToShortDocName( szFolderBuf , pFile, szDocShortName, &fIsNew );
        usRC = DocExpExportInternal( szFolderBuf, pFile, szDocShortName, pDocExpIda->szStartPath[0], HWND_FUNCIF, &(pDocExpIda->fOverwrite), NULL );
        if ( usRC ) fOK = FALSE;
      }
      else
      {
        fOK = DDEDocUnLoadWork( pDocExpIda, szFolderBuf, szDoc, pFile );
      } /* endif */
    } /* endif */
  } /* endif */

  if ( fOK )
  {
    pDocExpIda->usActFile++;
    if ( pDocExpIda->usActFile >= pDocExpIda->usFileNums )
    {
      // all documents are processed
      pData->fComplete = TRUE;

      // call termination function if exporting in validation format
      if ( (pDocExpIda->fValFormat || pDocExpIda->fPlainXML) && (pDocExpIda->lValExportHandle != 0) )
      {
        int iRC = DocExpValTerminate( pDocExpIda->lValExportHandle );
        if ( iRC ) usRC = (USHORT)iRC;
      } /* endif */
    } /* endif */
  }
  else
  {
    usRC = UtlQueryUShort( QS_LASTERRORMSGID );
    pData->fComplete = TRUE;
  } /* endif */

  // cleanup if process is terminated
  if ( pData->fComplete )
  {
    if ( pDocExpIda )
    {
      if ( pDocExpIda->pszFileNameBuffer )
      {
        UtlAlloc( (PVOID *)&pDocExpIda->pszFileNameBuffer, 0L, 0L, NOMSG );
      } /* endif */
      if ( pDocExpIda->ppFileArray )
      {
        UtlAlloc( (PVOID *)&pDocExpIda->ppFileArray, 0L, 0L, NOMSG );
      } /* endif */
      UtlAlloc( (PVOID *)&pDocExpIda, 0L, 0L, NOMSG );
    } /* endif */
    pData->pvDocImpExpIda = NULL;
  } /* endif */

  return( usRC );
} /* end of function DocFuncExportProcess */

  #else
USHORT DocFuncImportDoc( USHORT usDummy );
USHORT DocFuncImportDoc( USHORT usDummy )
{ return( 0 );}
USHORT DocFuncExportDoc( USHORT usDummy );
USHORT DocFuncExportDoc( USHORT usDummy )
{ return( 0 );}
  #endif

// Functions for DocFuncExportVal BEGIN
USHORT DocFuncExportValPre
(
  PFCTDATA    pData,              
  PSZ         pszFolderName,   
  PSZ         pszFiles,     
  PSZ         pszStartPath,
  LONG        lFormat,                          
  LONG        lOptions,
  LONG        lMatchTypes,
  LONG        lType,
  PSZ         pszTranslator,
  PSZ         pszPM
)
{
  USHORT      usRC = NO_ERROR;         // function return code
  PDOCIMPEXP  pDocImpExp = NULL;       // document import /export struct
  BOOL        fOK = TRUE;              // internal O.K. flag


  // allocate document import/export IDA
  fOK = UtlAllocHwnd( (PVOID *)&pDocImpExp, 0L, (LONG) sizeof(DOCIMPEXP),
                      ERROR_STORAGE, HWND_FUNCIF );
  if ( !fOK )
  {
    usRC = ERROR_STORAGE;
  } 

  // check folder name
  if ( fOK )
  {
    pDocImpExp->lOptions = lFormat | lOptions | lOptions | lMatchTypes;

    if ( (pszFolderName == NULL) || (*pszFolderName == EOS) )
    {
      fOK = FALSE;
      usRC = TA_MANDFOLDER;
      UtlErrorHwnd( usRC, MB_CANCEL, 0, NULL, EQF_ERROR, HWND_FUNCIF );
    }
    else 
    {
      // SubFolNameToObjectName expects the folder name to be in ASCII encoding...
      strcpy( pDocImpExp->chFldName, pszFolderName );
      ANSITOOEM( pDocImpExp->chFldName );

      if ( !SubFolNameToObjectName( pDocImpExp->chFldName,  pDocImpExp->szBuffer ) )
      {
        PSZ pszParm = pszFolderName;
        fOK = FALSE;
        usRC = ERROR_XLATE_FOLDER_NOT_EXIST;
        UtlErrorHwnd( usRC, MB_CANCEL, 1, &pszParm, EQF_ERROR, HWND_FUNCIF );
      }
      else if ( QUERYSYMBOL( pDocImpExp->szBuffer  ) != -1 )
      {
        PSZ pszParm = pszFolderName;
        fOK = FALSE;
        usRC = ERROR_FOLDER_LOCKED;
        UtlErrorHwnd( usRC, MB_CANCEL, 1, &pszParm, EQF_ERROR, HWND_FUNCIF );
      }
      else
      {
        // get subfolder/folder ID
        pDocImpExp->ulSubFolderID = FolGetSubFolderIdFromObjName( pDocImpExp->szBuffer );
      } 
    } 
  }//end if 

  // check specified export options
  if ( fOK )
  {
    // validation format export may not be used together with the other export options
    pDocImpExp->fValCombine  = (lOptions &  VAL_COMBINE_OPT) != 0;
    pDocImpExp->fValProtSegs = (lOptions &  VAL_PROTECTED_OPT) != 0;
    pDocImpExp->fValFormat = (lFormat &  (VALFORMAT_XML_OPT | VALFORMAT_HTML_OPT | VALFORMAT_DOC_OPT | VALFORMAT_DOCX_OPT | VALFORMAT_ODT_OPT)) != 0;
    pDocImpExp->lValOutputFormat = (LONG)(lFormat &  (VALFORMAT_XML_OPT | VALFORMAT_HTML_OPT | VALFORMAT_DOC_OPT | VALFORMAT_DOCX_OPT | VALFORMAT_ODT_OPT));
  } 


  // fill IDA fields
  if ( fOK )
  {
    pDocImpExp->hwndErrMsg = HWND_FUNCIF;
    strncpy( pDocImpExp->chFldName, pszFolderName, sizeof(pDocImpExp->chFldName)-1 );

    if ( pszStartPath != NULL )
    {
      strncpy( pDocImpExp->szStartPath, pszStartPath, sizeof(pDocImpExp->szStartPath)-1 );
    } 

    if ( *pszFiles == LISTINDICATOR )
    {
      // store the list file
      strncpy( pDocImpExp->chListName, pszFiles,
               sizeof(pDocImpExp->chListName)-1 );
      fOK = UtlListOfFiles( NULL, pDocImpExp->chListName, &(pDocImpExp->ppFileArray) );
      if ( fOK )
      {
        // get number of files given
        pDocImpExp->usFileNums = 0;
        while ( pDocImpExp->ppFileArray[pDocImpExp->usFileNums] )
        {
          pDocImpExp->usFileNums++;
        } 
      }
      else
      {
        usRC = DDE_ERRFILELIST;
        UtlErrorHwnd( usRC, MB_CANCEL, 1,
                      &pszFiles, EQF_ERROR, HWND_FUNCIF );
      } 
    }
    else
    {
      // work against list of files
      {
        ULONG ulLen;                     // length field

        ulLen = strlen(pszFiles) + 1;
        ulLen = max( ulLen, MIN_ALLOC );
        fOK = UtlAllocHwnd( (PVOID *)&(pDocImpExp->pszFileNameBuffer), 0L,
                            ulLen, ERROR_STORAGE, HWND_FUNCIF );
        if ( fOK )
        {
          strcpy( pDocImpExp->pszFileNameBuffer, pszFiles );
        }
        else
        {
          usRC = ERROR_STORAGE;
        } 
      }

      // get max number of documents within string (= number of delimiters + 1)
      if ( fOK )
      {
        PSZ pszTemp = pDocImpExp->pszFileNameBuffer;
        pDocImpExp->usFileNums = 1;
        while ( *pszTemp )
        {
          if ( *pszTemp == NAME_DELIMITER )
          {
            pDocImpExp->usFileNums++;
          } 
          pszTemp++;
        } 
      } 

      // allocate file name pointer array
      if ( fOK )
      {
        ULONG ulLen;                     // length field

        ulLen = (pDocImpExp->usFileNums + 1) * sizeof(PSZ);
        ulLen = max( ulLen, MIN_ALLOC );
        fOK = UtlAllocHwnd( (PVOID *)&(pDocImpExp->ppFileArray), 0L,
                            ulLen, ERROR_STORAGE, HWND_FUNCIF );
        if ( !fOK )
        {
          usRC = ERROR_STORAGE;
        }
      }

      // fill file name pointers
      if ( fOK )
      {
        PSZ pszCurrent = pDocImpExp->pszFileNameBuffer;
        PSZ pszNext = NULL;
        pDocImpExp->usFileNums = 0;
        while ( UtlGetNextFileFromCommaList( &pszCurrent, &pszNext ) )
        {
          pDocImpExp->ppFileArray[pDocImpExp->usFileNums++] = pszCurrent;
        }
      } 
    }//end else 
  } //end if

  if ( fOK )
  {
    pData->pvDocImpExpIda = pDocImpExp;
    pData->fComplete = FALSE;
    pDocImpExp->fOverwrite =  (lOptions & OVERWRITE_OPT)!=0;
    {
      int iRC = 0;
      DOCEXPVALOPTIONS Options;

      memset( &Options, 0, sizeof(Options) );
      if ( pDocImpExp->lValOutputFormat & VALFORMAT_ODT_OPT )
      {
        Options.ValFormat = ODT_VALEXPFORMAT;
      }
      else if ( pDocImpExp->lValOutputFormat & VALFORMAT_XML_OPT )
      {
        Options.ValFormat = XML_VALEXPFORMAT;
      }
      else if ( pDocImpExp->lValOutputFormat & VALFORMAT_DOC_OPT )
      {
        Options.ValFormat = DOC_VALEXPFORMAT;
      }
      else if ( pDocImpExp->lValOutputFormat & VALFORMAT_DOCX_OPT )
      {
        Options.ValFormat = DOCX_VALEXPFORMAT;
      }
      else
      {
        Options.ValFormat = HTML_VALEXPFORMAT;
      }
      
      //proof
      Options.fExportInValidationFormat = (lType&VAL_VALIDATION_OPT)!=0;

      Options.fInclCount    = (lOptions & VAL_INCLUDE_COUNT_OPT)!=0;
      Options.fInclExisting = (lOptions & VAL_INCLUDE_MATCH_OPT)!=0;
      Options.fMismatchOnly = (lOptions & VAL_MISMATCHES_ONLY_OPT)!=0;
      
      Options.fNewMatch   =  (lMatchTypes & VAL_NEW_OPT)!=0 ;
      Options.fProtMatch  =  (lMatchTypes & VAL_PROTECTED_OPT)!=0;
      Options.fAutoMatch  =  (lMatchTypes & VAL_AUTOSUBST_OPT)!=0;
      Options.fNotTransl  =  (lMatchTypes & VAL_NOT_TRANS_OPT)!=0;
      Options.fFuzzyMatch =  (lMatchTypes & VAL_FUZZY_OPT)!=0;
      Options.fExactMatch =  (lMatchTypes & VAL_EXACT_OPT)!=0;
      Options.fMachMatch  =  (lMatchTypes & VAL_MACHINE_OPT)!=0;
      Options.fReplMatch  =  (lMatchTypes & VAL_REPLACE_OPT)!=0;
      Options.fModifiedAuto  =  (lMatchTypes & VAL_MOD_AUTOSUBST_OPT)!=0;
      Options.fModifiedExact =  (lMatchTypes & VAL_MOD_EXACT_OPT)!=0;
      Options.fGlobMemMatch  =  (lMatchTypes & VAL_GLOBAL_MEM_OPT)!=0;

      
      Options.fNoInlineTagRemoval = (lOptions & VAL_REMOVE_INLINE_OPT)==0;
      Options.fExactFromManual = (lOptions & VAL_MAN_EXACTMATCH_OPT)!=0;
      Options.fTransOnly =  (lOptions & VAL_TRANSTEXT_ONLY_OPT)!=0;
      Options.fLinksImages = (lOptions & VAL_PRESERVE_LINKS_OPT)!=0;
      Options.fCombined =  (lOptions & VAL_COMBINE_OPT)!=0;
      
      //
      if(pszTranslator != NULL)
	  {
           MultiByteToWideChar(CP_UTF8, 0, pszTranslator, -1, Options.szTranslator, sizeof(Options.szTranslator)/sizeof(Options.szTranslator[0])-1 );
	  }

      if(pszPM != NULL)
	  { 
          MultiByteToWideChar(CP_UTF8, 0, pszPM, -1, Options.szManager, sizeof(Options.szManager)/sizeof(Options.szManager[0])-1 );
	  }

      if ( pDocImpExp->szStartPath[0] == EOS )
      {
        // use current directory as startpath
        _getcwd( pDocImpExp->szStartPath, sizeof(pDocImpExp->szStartPath) );
      }

      iRC = DocExpValInit( &Options, pDocImpExp->szStartPath, pDocImpExp->szBuffer, 
                            &(pDocImpExp->lValExportHandle), pDocImpExp->fOverwrite, HWND_FUNCIF );
    } 
  }
  else
  {
    pData->fComplete = TRUE;
    if ( pDocImpExp )
    {
      if ( pDocImpExp->pszFileNameBuffer )
      {
        UtlAlloc( (PVOID *)&pDocImpExp->pszFileNameBuffer, 0L, 0L, NOMSG );
      } 
      if ( pDocImpExp->ppFileArray )
      {
        UtlAlloc( (PVOID *)&pDocImpExp->ppFileArray, 0L, 0L, NOMSG );
      } 
      UtlAlloc( (PVOID *)&pDocImpExp, 0L, 0L, NOMSG );
    } 
    pData->pvDocImpExpIda = NULL;
  } 

  return( usRC );
} 


USHORT DocFuncExpMakeSegTargetPath
(
  PFCTDATA    pData,
  PSZ        *ppTgtPath         
)
{
  PDOCIMPEXP   pDocExpIda;             // document export/import ida
  BOOL         fOK = TRUE;             // error flag
  USHORT       usRC = NO_ERROR;        // function return code
  PSZ          pFolder = NULL;         // pointer to folder name
  PDDERETURN   pDDEReturn;
  CHAR         szFolderBuf[ MAX_EQF_PATH ];
  CHAR         szBuf[ MAX_FULLPATH ];
  HPROP        hpropFolder;            // return from folder props
  PPROPFOLDER  pProp;                  // pointer to folder props
  ULONG        ulErrorInfo;            // error indicator from PRHA

  pDocExpIda = (PDOCIMPEXP)pData->pvDocImpExpIda;
  pDDEReturn = &pDocExpIda->DDEReturn;
  
  // create folder name ...
  {
    BOOL fIsNew;
    CHAR szShortName[MAX_FILESPEC];
    PSZ pszDelimiter;

    strcpy( pDocExpIda->szBuffer, pDocExpIda->chFldName );
    pszDelimiter = strchr( pDocExpIda->szBuffer, BACKSLASH );
    if ( pszDelimiter ) *pszDelimiter = EOS;

    ANSITOOEM( pDocExpIda->szBuffer );
    ObjLongToShortName( pDocExpIda->szBuffer, szShortName, FOLDER_OBJECT, &fIsNew );
    UtlMakeEQFPath( szFolderBuf, NULC, SYSTEM_PATH, szShortName );
  }

  strcat( szFolderBuf, EXT_FOLDER_MAIN );

  if ( (hpropFolder = OpenProperties( szFolderBuf, NULL,
                                        PROP_ACCESS_READ, &ulErrorInfo)) == NULL)
  {
    fOK = FALSE;
    // display error message if not already displayed
    if ( ulErrorInfo != Err_NoStorage )
    {
      pFolder = pDocExpIda->chFldName;                 // set folder name
      pDDEReturn->usRc = ERROR_XLATE_FOLDER_NOT_EXIST;
      UtlErrorHwnd( pDDEReturn->usRc, MB_CANCEL, 1, &pFolder, EQF_ERROR, HWND_FUNCIF );
      usRC = ERROR_XLATE_FOLDER_NOT_EXIST;
    }
  }
  else
  {
    pProp = (PPROPFOLDER)MakePropPtrFromHnd( hpropFolder );
    szFolderBuf[0] = pProp->chDrive;
    CloseProperties( hpropFolder, PROP_QUIT, &ulErrorInfo );
    pFolder = UtlGetFnameFromPath( szFolderBuf );
  }

  if(fOK)
  {
    PSZ  pTgtPath;
    fOK = UtlAlloc( (PVOID *)&pTgtPath, 0L, MAX_FULLPATH, ERROR_STORAGE );
    if(!fOK)
    {
      usRC = ERROR_STORAGE;
      pDDEReturn->usRc = ERROR_STORAGE;
      UtlErrorHwnd( pDDEReturn->usRc, MB_CANCEL, 1, &pFolder, EQF_ERROR, HWND_FUNCIF );
    }
    else
    {
      UtlMakeEQFPath( szBuf, szFolderBuf[0], DIRSEGTARGETDOC_PATH, pFolder );
      strncpy(pTgtPath,szBuf,MAX_FULLPATH-1);
      *ppTgtPath = pTgtPath;
    }
  }

  return usRC;
}

USHORT DocFuncExportValProcFiles
(
PFCTDATA    pData,                   
PSZ         pszTgtFolderPath,
PSZ         pFileList,
BOOL        fExpand,                 // 1 means expand from *, 0 means one file
BOOL        fScan                    // 1 means pre scan, 0 means process 
)
{
  PDOCIMPEXP   pDocExpIda;             // document export/import ida
  BOOL         fOK = TRUE;             // error flag
  USHORT       usRC = NO_ERROR;        // function return code
  PSZ          pExpFile;               // pointer to export file name (full)
  PSZ          pFile;                  // pointer to export file name
  PDDERETURN   pDDEReturn;
  CHAR         szFolderPath[ MAX_FULLPATH ];

  pDocExpIda = (PDOCIMPEXP)pData->pvDocImpExpIda;
  pDDEReturn = &pDocExpIda->DDEReturn;

  pExpFile = pFileList;
  while(fOK && *pExpFile)
  {
    pFile = pExpFile;
    if( strncmp(pExpFile,pszTgtFolderPath,strlen(pszTgtFolderPath))==0 )
    {
      pFile = pExpFile + strlen(pszTgtFolderPath);
      if ( *pFile == BACKSLASH ) pFile++;
    }

    {
      OBJNAME szDocObjName;
      int iRC = 0;
      BOOL fIsNew = FALSE;

      //FolderPath
      strncpy(szFolderPath,pszTgtFolderPath,sizeof(szFolderPath)/sizeof(szFolderPath[0])-1);
      PSZ pTemp = strrchr(szFolderPath,BACKSLASH);
      if(pTemp != NULL)
        *pTemp = EOS;
      
      strcpy( szDocObjName, szFolderPath );
      strcat( szDocObjName, BACKSLASH_STR );
      FolLongToShortDocName( szFolderPath , pFile, szDocObjName + strlen(szDocObjName), &fIsNew );
      
      if(!fScan)
      {
        iRC = DocExpValProcess( pDocExpIda->lValExportHandle, szDocObjName );
		if(iRC == ERROR_FILE_NOT_FOUND)
		{
			iRC = ERROR_DOC_NOT_FOUND;
			UtlErrorHwnd( iRC, MB_CANCEL, 1, &pFile, EQF_ERROR, HWND_FUNCIF );
		}
      }
      else
      {
        iRC = DocExpValPreScan( pDocExpIda->lValExportHandle, szDocObjName );
      }

      if( iRC )
      {
        fOK = FALSE;
        usRC = (USHORT)iRC;
      }
    }
    
    if(fExpand) 
    { 
      pExpFile += strlen(pExpFile) + 1;
    }
    else 
    {
      break;
    }

  }//end while

  return( usRC );
} 

// list all files in pszFiles if it have '*' or '?'
USHORT  DocFuncExportValOp
(
  PFCTDATA    pData,
  PSZ         pszTgtPath,
  BOOL        fScan
)
{
  PDOCIMPEXP   pDocExpIda;             // document export/import ida
  BOOL         fOK = TRUE;             // error flag
  USHORT       usRC = NO_ERROR;        // function return code
  PSZ          pExpFile;               // pointer to export file name (full)

  PSZ          pFile;                  // pointer to export file name
  PDDERETURN   pDDEReturn;

  ULONG        ulFolderPathLen= 0;        // length of folder path in fully qualified document names
  CHAR         szBuf[ MAX_FULLPATH ];
  USHORT       usCur = 0;

  pDocExpIda = (PDOCIMPEXP)pData->pvDocImpExpIda;
  pDDEReturn = &pDocExpIda->DDEReturn;
  
  // begin to process files, loops here
  while( fOK && usCur<pDocExpIda->usFileNums )
  {
    PSZ   pFileList = NULL;
    pExpFile = *(pDocExpIda->ppFileArray + usCur);
    strncpy(szBuf,pszTgtPath,sizeof(szBuf)/sizeof(szBuf[0])-1);

    // when startpath is given the document names may not contain the fully qualified target path
    if ( pDocExpIda->szStartPath[0] != EOS )
    {
      if ( strncmp( pDocExpIda->szStartPath, pExpFile, strlen(pDocExpIda->szStartPath) ) == 0 )
      {
        pFile = pExpFile + strlen(pDocExpIda->szStartPath);
        if ( *pFile == BACKSLASH ) pFile++;
      }
      else
      {
        pFile = pExpFile;
      } 
    }
    else
    {
      pFile = UtlGetFnameFromPath( pExpFile );
      if ( pFile == NULL ) pFile = pExpFile;
    }
    
    strcat( szBuf, BACKSLASH_STR );
    ulFolderPathLen = strlen(szBuf);
    strcat( szBuf, pFile );

    // special handling for single/multiple substitution characters
    if ( (strchr( szBuf, '*') != NULL) || (strchr( szBuf, '?') != NULL) )
    {
      strcpy( pDocExpIda->szBuffer, szBuf + ulFolderPathLen );
      szBuf[ulFolderPathLen] = EOS;
      strcat( szBuf, DEFAULT_PATTERN );
      fOK = ResolveLongMultFileNames( szBuf, pDocExpIda->szBuffer, &pFileList, pDocExpIda->ulSubFolderID );
    
      if (!fOK)
      {
        PSZ apszErrParm[2];

        apszErrParm[0] = pFileList;
        apszErrParm[1] = pDocExpIda->chFldName;
        pDDEReturn->usRc = DDE_DOC_NOT_IN_FOLDR;
        UtlErrorHwnd( DDE_DOC_NOT_IN_FOLDR, MB_CANCEL, 2, apszErrParm,
                      EQF_ERROR, HWND_FUNCIF );
      }
    }//endif

    // process files
    if(fOK)
    {
      if( pFileList != NULL )
      {
        usRC = DocFuncExportValProcFiles(pData,pszTgtPath,pFileList,TRUE,fScan);
      }
      else
      {
        usRC =  DocFuncExportValProcFiles(pData,pszTgtPath,pFile,FALSE,fScan);
      }

      if(usRC) fOK = FALSE;
    }
    
    // release pointer memory
    if( pFileList != NULL )
    {
      UtlAlloc( (PVOID *)&pFileList, 0L, 0L, NOMSG );
      pFileList = NULL;
    }

    // next file
    usCur++;

  }//end while

  // end
  if(fOK && !fScan)
  {
    pDocExpIda->usActFile = pDocExpIda->usFileNums;
    pData->fComplete = TRUE;
  }
  else if(!fOK)
  {

    usRC = UtlQueryUShort( QS_LASTERRORMSGID );
    pData->fComplete = TRUE;
  }

  return( usRC );
}

USHORT DocFuncExportValTerminate
(
PFCTDATA    pData
)
{
  PDOCIMPEXP   pDocExpIda;             // document export/import ida
  USHORT       usRC = NO_ERROR;        // function return code

  pDocExpIda = (PDOCIMPEXP)pData->pvDocImpExpIda;
  
  if(pDocExpIda!=NULL && pDocExpIda->lValExportHandle!=NULL)
  {
    int iRC = DocExpValTerminate( pDocExpIda->lValExportHandle );
    if ( iRC ) usRC = (USHORT)iRC;
  }
  
  // clean resources
  if ( pData->fComplete )
  {
    if ( pDocExpIda )
    {
      if ( pDocExpIda->pszFileNameBuffer )
      {
        UtlAlloc( (PVOID *)&pDocExpIda->pszFileNameBuffer, 0L, 0L, NOMSG );
      }
      if ( pDocExpIda->ppFileArray )
      {
        UtlAlloc( (PVOID *)&pDocExpIda->ppFileArray, 0L, 0L, NOMSG );
      }
      UtlAlloc( (PVOID *)&pDocExpIda, 0L, 0L, NOMSG );
    } 
    pData->pvDocImpExpIda = NULL;
  } 

  return( usRC );
}


USHORT DocFuncExportDocVal
(
  PFCTDATA    pData,                   // function I/F session data
  PSZ         pszFolderName,   
  PSZ         pszFiles,     
  PSZ         pszStartPath,
  LONG        lFormat,                          
  LONG        lOptions,
  LONG        lMatchTypes,
  LONG        lType,
  PSZ         pszTranslator,
  PSZ         pszPM
)
{
  USHORT      usRC = NO_ERROR;         // function return code
  PSZ         pTgtpath = NULL;

  // initialize
  usRC = DocFuncExportValPre( pData, pszFolderName, pszFiles, pszStartPath,lFormat,
                                 lOptions,lMatchTypes,lType,pszTranslator,pszPM);
  
  // make target segment path
  if(!usRC)
  {
    usRC = DocFuncExpMakeSegTargetPath(pData,&pTgtpath);
  }

  // pre scan
  if( !usRC )
  {
    PDOCIMPEXP pDocExpIda = (PDOCIMPEXP)pData->pvDocImpExpIda;
    BOOL fScan = ( pDocExpIda!=NULL && (pDocExpIda->lOptions&VAL_MAN_EXACTMATCH_OPT)!=0 );
    if(fScan)
      usRC = DocFuncExportValOp(pData,pTgtpath,TRUE);
  }
  
  // export process
  if(!usRC)
  {
    usRC = DocFuncExportValOp(pData,pTgtpath,FALSE);
  }
  
  // terminate
  if(pData != NULL)
  {
    if(usRC)
    {
      // keep previous error code not lost
      DocFuncExportValTerminate(pData);
    }
    else
    {
      usRC = DocFuncExportValTerminate(pData);
    }
  }

  // release resources
  if(pTgtpath != NULL)
  {
    UtlAlloc( (PVOID *) &pTgtpath, 0L, 0L, NOMSG );
  }
  
  return usRC;
}

// Functions for DocFuncExportVal End

BOOL DocExpBrowse( HWND hwnd, PSZ pszBuffer, PSZ pszTitle )
{
  BOOL fOK = FALSE;
  BROWSEINFO bi;
  LPITEMIDLIST pidlBrowse;    // PIDL selected by user

  bi.hwndOwner = hwnd;
  bi.pidlRoot = NULL;
  bi.pszDisplayName = pszBuffer;
  bi.lpszTitle = pszTitle;
  bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_USENEWUI;

  // if UtlBrowseForFolderCallbackProc is specified as callback the lParam
  // parameter must contain the initial folder directory!!!
  bi.lpfn = UtlBrowseForFolderCallbackProc;
  bi.lParam = (LPARAM)pszBuffer;

  // Browse for a folder and return its PIDL.
  pidlBrowse = SHBrowseForFolder(&bi);

  if (pidlBrowse != NULL)
  {
    // get the selected directory path
    SHGetPathFromIDList( pidlBrowse, pszBuffer );

    fOK = TRUE;

    // Free the PIDL returned by SHBrowseForFolder.
    {
      LPMALLOC pMalloc;
      SHGetMalloc( &pMalloc );
#ifdef __cplusplus
      pMalloc->Free(pidlBrowse );
#else
      pMalloc->lpVtbl->Free(pMalloc, pidlBrowse );
#endif
    }
  } /* endif */
  return( fOK );
} /* end of function DocExpBrowse */


static int DocValFormatGetMatchFlags
( 
  HWND       hwnd,
  PDOCEXPIDA pIda
)
{
  pIda->fValExportNewMatch    = QUERYCHECK( hwnd, ID_DOCEXP_VALFORMAT_NEW_CHK );
  pIda->fValExportProtMatch   = QUERYCHECK( hwnd, ID_DOCEXP_VALFORMAT_PROT_CHK );
  pIda->fValExportAutoMatch   = QUERYCHECK( hwnd, ID_DOCEXP_VALFORMAT_AUTO_CHK );
  pIda->fValExportModAutoMatch   = QUERYCHECK( hwnd, ID_DOCEXP_VALFORMAT_MODAUTO_CHK );
  pIda->fValExportNotTransl   = QUERYCHECK( hwnd, ID_DOCEXP_VALFORMAT_NOTTRAN_CHK );
  pIda->fValExportFuzzyMatch  = QUERYCHECK( hwnd, ID_DOCEXP_VALFORMAT_FUZZY_CHK );
  pIda->fValExportExactMatch  = QUERYCHECK( hwnd, ID_DOCEXP_VALFORMAT_EXACT_CHK );
  pIda->fValExportModExactMatch  = QUERYCHECK( hwnd, ID_DOCEXP_VALFORMAT_MODEXACT_CHK );
  pIda->fValExportGlobMemMatch   = QUERYCHECK( hwnd, ID_DOCEXP_VALFORMAT_GLOBMEM_CHK );
  pIda->fValExportMachMatch   = QUERYCHECK( hwnd, ID_DOCEXP_VALFORMAT_MACHINE_CHK );
  pIda->fValExportReplMatch   = QUERYCHECK( hwnd, ID_DOCEXP_VALFORMAT_REPLACE_CHK );
  return( 0 );
} /* end of function DocValFormatGetMatchFlags */

static int DocValFormatSetMatchFlags
( 
  HWND       hwnd,
  PDOCEXPIDA pIda
)
{
  SETCHECK( hwnd, ID_DOCEXP_VALFORMAT_NEW_CHK, pIda->fValExportNewMatch );
  SETCHECK( hwnd, ID_DOCEXP_VALFORMAT_PROT_CHK, pIda->fValExportProtMatch );
  SETCHECK( hwnd, ID_DOCEXP_VALFORMAT_AUTO_CHK, pIda->fValExportAutoMatch );
  SETCHECK( hwnd, ID_DOCEXP_VALFORMAT_MODAUTO_CHK, pIda->fValExportModAutoMatch );
  SETCHECK( hwnd, ID_DOCEXP_VALFORMAT_NOTTRAN_CHK, pIda->fValExportNotTransl );
  SETCHECK( hwnd, ID_DOCEXP_VALFORMAT_FUZZY_CHK, pIda->fValExportFuzzyMatch );
  SETCHECK( hwnd, ID_DOCEXP_VALFORMAT_EXACT_CHK, pIda->fValExportExactMatch );
  SETCHECK( hwnd, ID_DOCEXP_VALFORMAT_MODEXACT_CHK, pIda->fValExportModExactMatch );
  SETCHECK( hwnd, ID_DOCEXP_VALFORMAT_GLOBMEM_CHK, pIda->fValExportGlobMemMatch );
  SETCHECK( hwnd, ID_DOCEXP_VALFORMAT_MACHINE_CHK, pIda->fValExportMachMatch );
  SETCHECK( hwnd, ID_DOCEXP_VALFORMAT_REPLACE_CHK, pIda->fValExportReplMatch );
  return( 0 );
} /* end of function DocValFormatSetMatchFlags */

static int DocValFormatEnableMatchFlags
( 
  HWND       hwnd,
  BOOL       fEnable
)
{
  ENABLECTRL( hwnd, ID_DOCEXP_VALFORMAT_NEW_CHK, fEnable );
  ENABLECTRL( hwnd, ID_DOCEXP_VALFORMAT_PROT_CHK, fEnable );
  ENABLECTRL( hwnd, ID_DOCEXP_VALFORMAT_AUTO_CHK, fEnable );
  ENABLECTRL( hwnd, ID_DOCEXP_VALFORMAT_MODAUTO_CHK, fEnable );
  ENABLECTRL( hwnd, ID_DOCEXP_VALFORMAT_NOTTRAN_CHK, fEnable );
  ENABLECTRL( hwnd, ID_DOCEXP_VALFORMAT_FUZZY_CHK, fEnable );
  ENABLECTRL( hwnd, ID_DOCEXP_VALFORMAT_EXACT_CHK, fEnable );
  ENABLECTRL( hwnd, ID_DOCEXP_VALFORMAT_MODEXACT_CHK, fEnable );
  ENABLECTRL( hwnd, ID_DOCEXP_VALFORMAT_MACHINE_CHK, fEnable );
  ENABLECTRL( hwnd, ID_DOCEXP_VALFORMAT_GLOBMEM_CHK, fEnable );
  ENABLECTRL( hwnd, ID_DOCEXP_VALFORMAT_REPLACE_CHK, fEnable );
  return( 0 );
} /* end of function DocValFormatEnableMatchFlags */

static int DocValFormatSetMatchStates
( 
  HWND       hwnd,
  PDOCEXPIDA pIda, 
  BOOL       fValidationFormat, 
  BOOL       fGetMatchStates
)
{
  int iRC = 0;

  if ( fValidationFormat )
  {
    // save current state of match checkboxes to IDA
    if ( fGetMatchStates )
    {
      DocValFormatGetMatchFlags( hwnd, pIda );
    } /* endif */

    // use fixed settings for validation format
    SETCHECK_TRUE( hwnd, ID_DOCEXP_VALFORMAT_NEW_CHK );
    SETCHECK_FALSE( hwnd, ID_DOCEXP_VALFORMAT_PROT_CHK );
    SETCHECK_TRUE( hwnd, ID_DOCEXP_VALFORMAT_AUTO_CHK );
	SETCHECK_TRUE( hwnd, ID_DOCEXP_VALFORMAT_MODAUTO_CHK );
	SETCHECK( hwnd, ID_DOCEXP_VALFORMAT_MODAUTO_CHK, pIda->fValExportModAutoMatch );
    SETCHECK_TRUE( hwnd, ID_DOCEXP_VALFORMAT_NOTTRAN_CHK );
    SETCHECK_TRUE( hwnd, ID_DOCEXP_VALFORMAT_FUZZY_CHK );
    SETCHECK_TRUE( hwnd, ID_DOCEXP_VALFORMAT_EXACT_CHK );
    SETCHECK_TRUE( hwnd, ID_DOCEXP_VALFORMAT_MODEXACT_CHK );
    SETCHECK_TRUE( hwnd, ID_DOCEXP_VALFORMAT_GLOBMEM_CHK );
    SETCHECK_TRUE( hwnd, ID_DOCEXP_VALFORMAT_MACHINE_CHK );
    SETCHECK_TRUE( hwnd, ID_DOCEXP_VALFORMAT_REPLACE_CHK );

    // disable checkboxes and pushbuttons
    DocValFormatEnableMatchFlags( hwnd, FALSE );
    ENABLECTRL( hwnd, ID_DOCEXP_VALFORMAT_SELECTALL_PB, FALSE );
    ENABLECTRL( hwnd, ID_DOCEXP_VALFORMAT_DESELECTALL_PB, FALSE );

    // remenber current state and disable count info and exist match checkboxes
    pIda->fValExportInclCount    = QUERYCHECK( hwnd, ID_DOCEXP_VALFORMAT_INCLCOUNT_CHK );
    pIda->fValExportInclExisting = QUERYCHECK( hwnd, ID_DOCEXP_VALFORMAT_INCLEXIST_CHK );
    pIda->fValExportMismatchOnly = QUERYCHECK( hwnd, ID_DOCEXP_VALFORMAT_MISMATCH_CHK );
    pIda->fValExportTransOnly    = QUERYCHECK( hwnd, ID_DOCEXP_VALFORMAT_TRANSONLY_CHK );

    ENABLECTRL( hwnd, ID_DOCEXP_VALFORMAT_INCLCOUNT_CHK, FALSE );
    ENABLECTRL( hwnd, ID_DOCEXP_VALFORMAT_INCLEXIST_CHK, FALSE );
    ENABLECTRL( hwnd, ID_DOCEXP_VALFORMAT_MISMATCH_CHK, FALSE );
    ENABLECTRL( hwnd, ID_DOCEXP_VALFORMAT_TRANSONLY_CHK, FALSE );

    SETCHECK_FALSE( hwnd, ID_DOCEXP_VALFORMAT_INCLCOUNT_CHK );
    SETCHECK_FALSE( hwnd, ID_DOCEXP_VALFORMAT_INCLEXIST_CHK );
    SETCHECK_FALSE( hwnd, ID_DOCEXP_VALFORMAT_MISMATCH_CHK );
    SETCHECK_FALSE( hwnd, ID_DOCEXP_VALFORMAT_TRANSONLY_CHK );
  }
  else
  {
    // enable checkboxes and pushbuttons
    DocValFormatEnableMatchFlags( hwnd, TRUE );
    ENABLECTRL( hwnd, ID_DOCEXP_VALFORMAT_SELECTALL_PB, TRUE );
    ENABLECTRL( hwnd, ID_DOCEXP_VALFORMAT_DESELECTALL_PB, TRUE );

    // restore match settings from IDA
    DocValFormatSetMatchFlags( hwnd, pIda );

    // activate and set state of additional info checkboxes
    ENABLECTRL( hwnd, ID_DOCEXP_VALFORMAT_INCLCOUNT_CHK, TRUE );
    ENABLECTRL( hwnd, ID_DOCEXP_VALFORMAT_INCLEXIST_CHK, TRUE );
    ENABLECTRL( hwnd, ID_DOCEXP_VALFORMAT_MISMATCH_CHK, TRUE );
    ENABLECTRL( hwnd, ID_DOCEXP_VALFORMAT_TRANSONLY_CHK, TRUE );

    SETCHECK( hwnd, ID_DOCEXP_VALFORMAT_INCLCOUNT_CHK, pIda->fValExportInclCount );
    SETCHECK( hwnd, ID_DOCEXP_VALFORMAT_INCLEXIST_CHK, pIda->fValExportInclExisting );
    SETCHECK( hwnd, ID_DOCEXP_VALFORMAT_MISMATCH_CHK, pIda->fValExportMismatchOnly );
    SETCHECK( hwnd, ID_DOCEXP_VALFORMAT_TRANSONLY_CHK, pIda->fValExportTransOnly );
    if ( pIda->fValExportInclExisting )
    {
      ENABLECTRL( hwnd, ID_DOCEXP_VALFORMAT_MISMATCH_CHK, TRUE );
      if ( pIda->fValExportMismatchOnly )
      {
        DocValFormatEnableMatchFlags( hwnd, FALSE );
        SETCHECK_TRUE( hwnd, ID_DOCEXP_VALFORMAT_NEW_CHK );
        SETCHECK_FALSE( hwnd, ID_DOCEXP_VALFORMAT_PROT_CHK );
        SETCHECK_FALSE( hwnd, ID_DOCEXP_VALFORMAT_AUTO_CHK );
		SETCHECK_FALSE( hwnd, ID_DOCEXP_VALFORMAT_MODAUTO_CHK );
        SETCHECK_TRUE( hwnd, ID_DOCEXP_VALFORMAT_NOTTRAN_CHK );
        SETCHECK_TRUE( hwnd, ID_DOCEXP_VALFORMAT_FUZZY_CHK );
        SETCHECK_TRUE( hwnd, ID_DOCEXP_VALFORMAT_EXACT_CHK );
        SETCHECK_TRUE( hwnd, ID_DOCEXP_VALFORMAT_MODEXACT_CHK );
        SETCHECK_TRUE( hwnd, ID_DOCEXP_VALFORMAT_GLOBMEM_CHK );
        SETCHECK_TRUE( hwnd, ID_DOCEXP_VALFORMAT_MACHINE_CHK );
        SETCHECK_TRUE( hwnd, ID_DOCEXP_VALFORMAT_REPLACE_CHK );
        ENABLECTRL( hwnd, ID_DOCEXP_VALFORMAT_SELECTALL_PB, FALSE );
        ENABLECTRL( hwnd, ID_DOCEXP_VALFORMAT_DESELECTALL_PB, FALSE );
      } /* endif */
    } /* endif */
  } /* endif */

  // handling for links and images checkbox
  pIda->sValFormat = CBQUERYSELECTION( hwnd, ID_DOCEXP_VALFORMAT_FORMAT_CB );
  if ( ((VALFORMATID)pIda->sValFormat == HTML_VALEXPFORMAT) || 
       ((VALFORMATID)pIda->sValFormat == ODT_VALEXPFORMAT) || 
       ((VALFORMATID)pIda->sValFormat == DOC_VALEXPFORMAT) ||
       ((VALFORMATID)pIda->sValFormat == DOCX_VALEXPFORMAT) )
  {
    ENABLECTRL( hwnd, ID_DOCEXP_VALFORMAT_LINKSIMAGES_CHK, TRUE );
    SETCHECK( hwnd, ID_DOCEXP_VALFORMAT_LINKSIMAGES_CHK, pIda->fValExportLinksImages );
  }
  else
  {
    ENABLECTRL( hwnd, ID_DOCEXP_VALFORMAT_LINKSIMAGES_CHK, FALSE );
    SETCHECK_FALSE( hwnd, ID_DOCEXP_VALFORMAT_LINKSIMAGES_CHK );
  } /* endif */

  return( iRC );
} /* end of function DocValFormatSetMatchStates */

