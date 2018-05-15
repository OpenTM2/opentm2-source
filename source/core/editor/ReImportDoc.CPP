/*! \file
	Description: This module contains the code for the re-import document function.

	Copyright Notice:

	Copyright (C) 1990-2015, International Business Machines
	Corporation and others. All rights reserved
*/

#define INCL_EQF_TP               // public translation processor functions
#define INCL_EQF_EDITORAPI        // editor API
#define INCL_EQF_TAGTABLE         // tagtable defines and functions
#define INCL_EQF_MORPH            // morphologic functions
#define INCL_EQF_ANALYSIS         // analysis functions
#include <eqf.h>                  // General Translation Manager include file


#include "EQFTPI.H"               // Translation Processor priv. include file
#include "EQFB.ID"                // Translation Processor IDs
#include "EQFBDLG.ID"             // Translation Processor dialog control IDs
#include <EQFPROGR.H>             // progress control defines 
#include "ReImportDoc.h"

/*! \brief Data area for the re-import document function
*/
typedef struct _REIMPORTDATA
{
  CHAR   szDocStargetPathName[MAX_LONGFILESPEC];   // fully qualified name of STARGET document
  CHAR   szDocObjectName[MAX_LONGFILESPEC];        // document object name
  CHAR   szDocPropName[MAX_LONGFILESPEC];          // document property file name
  CHAR   szDocSourceName[MAX_LONGFILESPEC];        // fully qualified name of SOURCE document
  CHAR   szDocExternalName[MAX_LONGFILESPEC];      // fully qualified name of external source document
  ULONG  ulSegNum;                                 // current position in the editor
  EQF_BOOL fKillAnalysis;                          // analysis kill flag
} REIMPORTDATA, *PREIMPORTDATA;


// prototypes for internal functions
INT_PTR CALLBACK REIMPORTWAITDLG( HWND hDlg, WINMSG msg, WPARAM mp1, LPARAM mp2 );
BOOL MakePropNameFromSTargetFileName( PSZ pszPropName, PSZ pszSTargetFileName );
BOOL MakeObjNameFromSTargetFileName( PSZ pszObjName, PSZ pszSTargetFileName );
BOOL MakeSourceNameFromSTargetFileName( PSZ pszDocSourceName, PSZ pszSTargetFileName );

/*! \brief Checks if document can be re-imported 
  \param pszDocName fully qualified name of the STARGET document
\returns TRUE if external document is available for the re-import
*/
BOOL ReImport_IsAvailable( char *pszDocName )
{
  CHAR szDocPropName[MAX_LONGFILESPEC];

  if ( !MakePropNameFromSTargetFileName( szDocPropName, pszDocName  ) ) return( FALSE );

  // load property file name
  PPROPDOCUMENT pProp = NULL;
  ULONG ulSize = 0;
  if ( !UtlLoadFileL( szDocPropName, (PVOID *)&pProp, &ulSize, FALSE, FALSE ) )
  {
    return( FALSE );
  }

  BOOL fExtDocExists = FALSE;
  if ( pProp->szExternalDocPathName[0] != 0 )
  {
    fExtDocExists = UtlFileExist( pProp->szExternalDocPathName );
  } /* endif */

  UtlAlloc( (PVOID *)&pProp, 0, 0, NOMSG );

  return( fExtDocExists );
}

/*! \brief Prepares the re-import of a document
  \param pszDocName fully qualified name of the STARGET document
  \param ulSegNum number of currently active segment within the document
  \param ppvReImportHandle address of a PVOID pointer which will receive the re-import handle on return
\returns TRUE if successful, FALSE in case of errors
*/
BOOL ReImport_Prepare( char *pszDocName, ULONG ulSegnum, PVOID *ppvReImportHandle )
{
  PREIMPORTDATA pData = NULL;
  BOOL fOK = TRUE;

  // allocate re-import document data area
  fOK = UtlAlloc( (PVOID *)&pData, 0, sizeof(REIMPORTDATA), ERROR_STORAGE );

  // fill-in provided data area
  if ( fOK )
  {
    *ppvReImportHandle = (PVOID)pData;
    strcpy( pData->szDocStargetPathName, pszDocName );
    pData->ulSegNum = ulSegnum;

    fOK = MakePropNameFromSTargetFileName( pData->szDocPropName, pszDocName  );
    if ( fOK ) fOK = MakeObjNameFromSTargetFileName( pData->szDocObjectName, pszDocName  );
    if ( fOK ) fOK = MakeSourceNameFromSTargetFileName( pData->szDocSourceName, pszDocName  );
  } /* endif */

  return( fOK );
}

/*! \brief Starts the re-import of a document
  \param pvReImportHandle the handle returned by a previous ReImport_Prepare call
\returns TRUE if successful, FALSE in case of errors
*/
BOOL ReImport_Start( PVOID pvReImportHandle )
{
  if ( pvReImportHandle != NULL )
  {
    EqfPost2Handler( DOCUMENTHANDLER, WM_EQF_PROCESSTASK, MP1FROMSHORT(REIMPORT_TASK), MP2FROMP(pvReImportHandle) );
  } /* endif */
  return( TRUE );
}

/*! \brief Performs the actual re-import of a document
    \param hwnd window handle of caller
    \param pvReImportHandle the handle returned by a previous ReImport_Prepare call
    \returns TRUE if successful, FALSE in case of errors
*/
BOOL ReImport_Process( HWND hwnd,  PVOID pvReImportHandle )
{
  BOOL       fOK = TRUE;
  PREIMPORTDATA pData = (PREIMPORTDATA)pvReImportHandle;
  HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);

  if ( hResMod )
  {
    INT_PTR iRc;
    DIALOGBOX( (HWND)UtlQueryULong( QL_TWBCLIENT ), REIMPORTWAITDLG, hResMod, ID_TB_REIMPORTWAIT_DLG, pvReImportHandle, iRc);

    if ( iRc == DID_ERROR )
    {
      UtlError( ERROR_DIALOG_LOAD_FAILED, MB_CANCEL, 0 , NULL, EQF_ERROR );
      fOK = FALSE;
    } /* endif */
  } /* endif */

  // adjust document properties
  if ( fOK )
  {
    //UtlTime( (PLONG)&pIda->ppropDoc->ulSeg );  // get time stamp
    //pIda->ppropDoc->ulTouched = 0L;
    //pIda->ppropDoc->ulXLated = 0L;
  } /* endif */

  // open document
  if ( fOK )
  {
    POPENANDPOS pOpen = NULL;
    if ( UtlAlloc( (PVOID *)&pOpen, 0L, (LONG)sizeof(OPENANDPOS), ERROR_STORAGE ) )
    {
      CHAR   szDocFullPath[MAX_EQF_PATH];
      FILEFINDBUF stResultBuf;            // DOS file find struct
      USHORT     usCount = 1;             // number of files requested
      HDIR       hDirHandle = HDIR_CREATE;// DosFind routine handle


      // Setup open-and-pos structure and pass request to document handler 
      MakeObjNameFromSTargetFileName( pOpen->szDocName, pData->szDocStargetPathName );
      pOpen->ulSeg = pData->ulSegNum;
      UtlAlloc( (PVOID *)&pData, 0, 0, NOMSG );
      EqfPost2Handler( DOCUMENTHANDLER, WM_EQF_PROCESSTASK, MP1FROMSHORT(OPEN_AND_POSITION_TASK), MP2FROMP(pOpen) );
    }
  } /* endif */
  return fOK;
}

INT_PTR CALLBACK REIMPORTWAITDLG( HWND hDlg, WINMSG msg, WPARAM mp1, LPARAM mp2 )
{
  MRESULT mResult = FALSE;
  switch ( msg )
  {
    case WM_INITDLG:
      {
        PREIMPORTDATA pData = (PREIMPORTDATA)mp2;
        ANCHORDLGIDA( hDlg, pData );
        SWP   swp, swpDlg;
        HWND  hwndSlider = GetDlgItem( hDlg, ID_TB_REIMPORTWAIT_SLIDER );
        if ( hwndSlider )
        {
           SendMessage( hwndSlider, PB_SETRANGE, 100, 0 );
           SendMessage( hwndSlider, PB_SETPOS, 0, 0 );
           SendMessage( hwndSlider, PB_SETCOLOR, 0, RGB(0,0,0) );
        } /* endif */
        WinPostMsg( hDlg, WM_EQF_INITIALIZE, NULL, mp2 );
      }
      break;

  case WM_EQF_INITIALIZE:
    {
      BOOL fOK = TRUE;
      PREIMPORTDATA pData = ACCESSDLGIDA( hDlg, PREIMPORTDATA );

      // get external file name from document properties
      if ( fOK )
      {
        // load property file name
        PPROPDOCUMENT pProp = NULL;
        ULONG ulSize = 0;
        fOK = UtlLoadFileL( pData->szDocPropName, (PVOID *)&pProp, &ulSize, FALSE, FALSE );
        if ( fOK )
        {
          strcpy( pData->szDocExternalName, pProp->szExternalDocPathName );
          UtlAlloc( (PVOID *)&pProp, 0, 0, NOMSG );
        } /* endif */
      } /* endif */

      // do the import of the source file
      if ( fOK )
      {
        fOK = UtlCopy( pData->szDocExternalName, pData->szDocSourceName, 1, 0L, TRUE ) == 0;
        UtlSetFileMode( pData->szDocSourceName, FILE_NORMAL, 0L, TRUE );
      } /* endif */

      // analyze the file
      if ( fOK )
      {
        PEQF_BOOL pfKillAnalysis = &pData->fKillAnalysis;
        fOK = TAAnalyzeFile( pData->szDocObjectName, (HWND)UtlQueryULong( QL_TWBCLIENT ), TA_INSERTTMMATCH | TA_USEFOLDERSETTINGS, NULL, &pfKillAnalysis );
      } /* endif */

      // update document properties


      WinPostMsg( hDlg, WM_EQF_CLOSE, NULL, MP1FROMSHORT(fOK) );
    }
    break;

    case WM_EQF_CLOSE:
      DISMISSDLG( hDlg, SHORT1FROMMP2(mp2) );
      break;

    case WM_COMMAND:
      if ( WMCOMMANDID( mp1, mp2 ) == DID_CANCEL )
      {
        if ( UtlError( ERROR_CANCELTA, MB_YESNO, 0, NULL, EQF_QUERY ) == MBID_YES )
        {
          // Setting fKill to TRUE will terminate the analysis process at the next possible step                
          PREIMPORTDATA pData = ACCESSDLGIDA( hDlg, PREIMPORTDATA );
          pData->fKillAnalysis = TRUE;
          POSTEQFCLOSE( hDlg, FALSE );
        } /* endif */
        mResult = MRFROMSHORT(TRUE); // TRUE = command is processed
      }
      else
      {
        mResult = UTLDEFDIALOGPROC( hDlg, msg, mp1, mp2 );
      } /* endif */
      break;
    default:
      mResult = UTLDEFDIALOGPROC( hDlg, msg, mp1, mp2 );
      break;
  } /* endswitch */
  return ( mResult );

} /* end of function REIMPORTWAITDLG */

/*! \brief Convert a fully qualified STARGET file name to a fully qualified document property file name
  \param pszPropName points to buffer for document property file name
  \param pszSTargetFileName fully qualified name of the STARGET document
\returns TRUE if property name could be created
*/
BOOL MakePropNameFromSTargetFileName( PSZ pszPropName, PSZ pszSTargetFileName )
{
  if ( (pszSTargetFileName == NULL ) || (*pszSTargetFileName == EOS) ) return( FALSE );

  PSZ pszNamePart; 

  // convert STARGET name to a document property file name
  strcpy( pszPropName, pszSTargetFileName );
  pszNamePart = UtlSplitFnameFromPath( pszPropName );
  if ( pszNamePart == NULL ) return( FALSE );

  CHAR szName[MAX_FILESPEC];
  strcpy( szName, pszNamePart );

  UtlSplitFnameFromPath( pszPropName );
  strcat( pszPropName, BACKSLASH_STR );
  UtlQueryString( QST_PROPDIR, pszPropName + strlen(pszPropName), MAX_EQF_PATH );
  strcat( pszPropName, BACKSLASH_STR );
  strcat( pszPropName, szName );

  return( TRUE );
}

/*! \brief Convert a fully qualified STARGET file name to a fully qualified document source file name
  \param pszDocSourceName points to buffer for document source file name
  \param pszSTargetFileName fully qualified name of the STARGET document
\returns TRUE if property name could be created
*/
BOOL MakeSourceNameFromSTargetFileName( PSZ pszDocSourceName, PSZ pszSTargetFileName )
{
  if ( (pszSTargetFileName == NULL ) || (*pszSTargetFileName == EOS) ) return( FALSE );

  PSZ pszNamePart; 

  // convert STARGET name to a document source file name
  strcpy( pszDocSourceName, pszSTargetFileName );
  pszNamePart = UtlSplitFnameFromPath( pszDocSourceName );
  if ( pszNamePart == NULL ) return( FALSE );

  CHAR szName[MAX_FILESPEC];
  strcpy( szName, pszNamePart );

  UtlSplitFnameFromPath( pszDocSourceName );
  strcat( pszDocSourceName, BACKSLASH_STR );
  UtlQueryString( QST_SOURCEDIR, pszDocSourceName + strlen(pszDocSourceName), MAX_EQF_PATH );
  strcat( pszDocSourceName, BACKSLASH_STR );
  strcat( pszDocSourceName, szName );

  return( TRUE );
}

/*! \brief Convert a fully qualified STARGET file name to a document object name
  \param pszObjName points to buffer for document object name
  \param pszSTargetFileName fully qualified name of the STARGET document
\returns TRUE if object name could be created
*/
BOOL MakeObjNameFromSTargetFileName( PSZ pszObjName, PSZ pszSTargetFileName )
{
  if ( (pszSTargetFileName == NULL ) || (*pszSTargetFileName == EOS) ) return( FALSE );

  PSZ pszNamePart; 

  // convert STARGET name to a document property file name
  strcpy( pszObjName, pszSTargetFileName );
  pszNamePart = UtlSplitFnameFromPath( pszObjName );
  if ( pszNamePart == NULL ) return( FALSE );

  CHAR szName[MAX_FILESPEC];
  strcpy( szName, pszNamePart );

  UtlSplitFnameFromPath( pszObjName );
  strcat( pszObjName, BACKSLASH_STR );
  strcat( pszObjName, szName );

  return( TRUE );
}