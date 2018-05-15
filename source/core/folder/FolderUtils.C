//+----------------------------------------------------------------------------+
//|  EQFFOL00.C - EQF Folder Handler  - Functions Part 1                       |
//+----------------------------------------------------------------------------+
//|  Copyright Notice:                                                         |
//|                                                                            |
//|      Copyright (C) 1990-2014, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+

#define INCL_EQF_FOLDER           // folder list and document list functions
#define INCL_EQF_DLGUTILS         // dialog utilities
#define INCL_EQF_TAGTABLE         // required for symbolic markup names
#include <eqf.h>                  // General Translation Manager include file

#include "eqfdde.h"               // batch mode defines
#include "eqffol00.h"             // Folder Handler defines
#include "OTMFUNC.H"            // public defines for function call interface
#include "EQFFUNCI.H"           // private defines for function call interface
#include "EQFHLOG.H"                 // defines for history log processing

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     FolRenameFolder                                          |
//+----------------------------------------------------------------------------+
//|Description:       Physically rename folder and all                         |
//|                   associated files                                         |
//+----------------------------------------------------------------------------+
//|Input parameter:   BOOL        fMsg,              show-error-messages flag  |
//|                   PSZ         pszOldAndNewName   old and new folder name   |
//|                                                  seperated by 0x15         |
//|                                                  (old name = object name,  |
//|                                                  new name = file name only)|
//|                              e.g. H:\EQF\SAMPLE1.F000x15NEWNAME0x00        |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT              error code or 0 if success           |
//+----------------------------------------------------------------------------+
BOOL FolRenameFolder
(
PSZ         pszOldAndNewName,        // old and new folder name (x15 seperated)
BOOL        fMsg                     // show-error-messages flag
)
{
  // our private data area
  typedef struct _PRIVATEDATA
  {
    CHAR       szOldObject[MAX_EQF_PATH];// buffer for old object name
    CHAR       szNewObject[MAX_EQF_PATH];// buffer for new object name
    CHAR       szFolProp[MAX_EQF_PATH]; // buffer for folder property file name
    CHAR       szFolPropDir[MAX_EQF_PATH]; // buffer for folder property directory
    CHAR       szDocPropName[MAX_EQF_PATH]; // buffer for document prop. file name
    CHAR       szOldName[MAX_FNAME];    // buffer for old name
    CHAR       szNewName[MAX_FNAME];    // buffer for new name
    CHAR       szOldPath[MAX_EQF_PATH]; // buffer for old path name
    CHAR       szNewPath[MAX_EQF_PATH]; // buffer for new path name
    CHAR       szFullFolderName[MAX_FILESPEC]; // buffer for new folder name (w/ .EXT)
    PROPFOLDER stFolProp;               // buffer for folder properties
    CHAR       szLongName[MAX_LONGFILESPEC]; // buffer for new long name
    CHAR       szOldLongName[MAX_LONGFILESPEC]; // buffer for old long name
  } PRIVATEDATA, *PPRIVATEDATA;

  BOOL        fOK = TRUE;              // internal O.K. flag and return value
  PPRIVATEDATA pData = NULL;           // ptr to private data area

  // allocate our private data area
  fOK = UtlAlloc( (PVOID *)&pData, 0L, (LONG)sizeof(PRIVATEDATA),
                  (USHORT)(( fMsg ) ? ERROR_STORAGE : NOMSG ) );

  // split input data and store old and new folder name
  if ( fOK )
  {
    PSZ pszNewName = strchr( pszOldAndNewName, X15 );
    if ( pszNewName != NULL )
    {
      PSZ pszNewLongName;
      *pszNewName = EOS;
      pszNewName++;
      pszNewLongName = strchr( pszNewName, X15 );
      if ( pszNewLongName != NULL )
      {
        *pszNewLongName = EOS;
        pszNewLongName++;
        strcpy( pData->szLongName, pszNewLongName );
      }
      else
      {
        pData->szLongName[0] = EOS;
      } /* endif */
      strcpy( pData->szNewName, pszNewName );
    } /* endif */
    strcpy( pData->szOldObject, pszOldAndNewName );
    Utlstrccpy( pData->szOldName, UtlGetFnameFromPath( pData->szOldObject ),
                DOT );
  } /* endif */

  // setup folder property file name
  if ( fOK )
  {
    UtlMakeEQFPath( pData->szFolProp, NULC, PROPERTY_PATH, NULL );
    strcat( pData->szFolProp, BACKSLASH_STR );
    strcat( pData->szFolProp, pData->szOldName );
    strcat( pData->szFolProp, EXT_FOLDER_MAIN );
  } /* endif */

  // load folder property file
  if ( fOK )
  {
    ULONG ulRead;     // number of bytes read from disk
    PVOID pvTemp = (PVOID)&(pData->stFolProp);
    fOK = UtlLoadFileL( pData->szFolProp, &pvTemp, &ulRead,
                       FALSE, fMsg );
  } /* endif */

  // rename folder directory
  if ( fOK )
  {
    // setup old path name
    UtlMakeEQFPath( pData->szOldPath, pData->stFolProp.chDrive,
                    SYSTEM_PATH, NULL );
    strcat( pData->szOldPath, BACKSLASH_STR );
    strcat( pData->szOldPath, pData->szOldName );
    strcat( pData->szOldPath, EXT_FOLDER_MAIN );

    // setup new path name
    UtlMakeEQFPath( pData->szNewPath, pData->stFolProp.chDrive,
                    SYSTEM_PATH, NULL );
    strcat( pData->szNewPath, BACKSLASH_STR );
    strcat( pData->szNewPath, pData->szNewName );
    strcat( pData->szNewPath, EXT_FOLDER_MAIN );

    // actually rename directory using UtlMove...
    fOK = UtlMove( pData->szOldPath, pData->szNewPath, 0L,
                   fMsg ) == NO_ERROR;
  } /* endif */

  // rename folder property file
  if ( fOK )
  {
    // setup old path name
    UtlMakeEQFPath( pData->szOldPath, NULC, PROPERTY_PATH, NULL );
    strcat( pData->szOldPath, BACKSLASH_STR );
    strcat( pData->szOldPath, pData->szOldName );
    strcat( pData->szOldPath, EXT_FOLDER_MAIN );

    // setup new path name
    UtlMakeEQFPath( pData->szNewPath, NULC, PROPERTY_PATH, NULL );
    strcat( pData->szNewPath, BACKSLASH_STR );
    strcat( pData->szNewPath, pData->szNewName );
    strcat( pData->szNewPath, EXT_FOLDER_MAIN );

    // actually rename property file using UtlMove...
    fOK = UtlMove( pData->szOldPath, pData->szNewPath, 0L,
                   fMsg ) == NO_ERROR;
  } /* endif */

  // adjust name in folder property file
  if ( fOK )
  {
    if ( pData->stFolProp.szLongName[0] != EOS )
    {
      strcpy( pData->szOldLongName, pData->stFolProp.szLongName );
    }
    else
    {
      Utlstrccpy( pData->szOldLongName, pData->stFolProp.PropHead.szName, DOT );
    } /* endif */
    strcpy( pData->stFolProp.PropHead.szName, pData->szNewName );
    strcat( pData->stFolProp.PropHead.szName, EXT_FOLDER_MAIN );

    if ( pData->stFolProp.szOrgName[0] == EOS )
    {
      strcpy( pData->stFolProp.szOrgName, pData->szOldName );
    } /* endif */

    if ( pData->stFolProp.szOrgLongName[0] == EOS )
    {
      if ( pData->stFolProp.szLongName[0] != EOS )
      {
        strcpy( pData->stFolProp.szOrgLongName, pData->stFolProp.szLongName );
      } /* endif */
    } /* endif */

    // adjust long name
    if ( strcmp( pData->szLongName, pData->szNewName ) != EOS )
    {
      strcpy( pData->stFolProp.szLongName, pData->szLongName );
    }
    else
    {
      pData->stFolProp.szLongName[0] = EOS;
    } /* endif */
  } /* endif */

  // rewrite folder property file
  if ( fOK )
  {
    fOK = UtlWriteFile( pData->szNewPath,
                        sizeof(pData->stFolProp),
                        &(pData->stFolProp), fMsg ) == NO_ERROR;
  } /* endif */

  // adjust names in property files of the documents contained in this folder
  if ( fOK )
  {
    WIN32_FIND_DATA FileFindData;
    HANDLE hDir = HDIR_CREATE;

    // setup search path: folder document source directory
    {
      strcpy( pData->szFullFolderName, pData->szNewName );
      strcat( pData->szFullFolderName, EXT_FOLDER_MAIN );
      UtlMakeEQFPath( pData->szFolPropDir, pData->stFolProp.chDrive,
                      DIRSOURCEDOC_PATH, pData->szFullFolderName );
      strcat( pData->szFolPropDir, BACKSLASH_STR );
      strcat( pData->szFolPropDir, DEFAULT_PATTERN );
    }

    // loop over all documents and correct property header
    hDir = FindFirstFile( pData->szFolPropDir, &FileFindData );
    if ( hDir != INVALID_HANDLE_VALUE )
    {
      BOOL fMoreFiles = TRUE;

      // setup new path for document property header
      UtlMakeEQFPath( pData->szFolPropDir, pData->stFolProp.chDrive, SYSTEM_PATH, NULL );
      strcat( pData->szFolPropDir, BACKSLASH_STR );
      strcat( pData->szFolPropDir, pData->szNewName );
      strcat( pData->szFolPropDir, EXT_FOLDER_MAIN );

      while ( fOK && fMoreFiles )
      {
        if ( (FileFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0 )
        {
          // get property file name of current item
          UtlMakeEQFPath( pData->szDocPropName, pData->stFolProp.chDrive, PROPERTY_PATH, pData->szFullFolderName );
          strcat( pData->szDocPropName, BACKSLASH_STR );
          strcat( pData->szDocPropName, FileFindData.cFileName );

          // adjust property header
          fOK = FolCorrectPropHead( pData->szDocPropName, NULC, pData->szFolPropDir, NULL, NULLHANDLE );
        } /* endif */

        // continue with next document
        fMoreFiles = FindNextFile( hDir, &FileFindData );
      } /* endwhile */
      FindClose( hDir );
    }
  } /* endif */

  // broadcast changed folder name
  if ( fOK )
  {
    // remove original folder name
    EqfSend2AllHandlers( WM_EQFN_DELETED,
                         MP1FROMSHORT(clsFOLDER),
                         MP2FROMP(pData->szOldObject) );
    OEMTOANSI( pData->szOldLongName );
    EqfSend2AllHandlers( WM_EQFN_DELETEDNAME,
                         MP1FROMSHORT( clsFOLDER ),
                         MP2FROMP( pData->szOldLongName ));

    // add new folder name
    UtlMakeEQFPath( pData->szNewObject, NULC, SYSTEM_PATH, NULL );
    strcat( pData->szNewObject, BACKSLASH_STR );
    strcat( pData->szNewObject, pData->stFolProp.PropHead.szName );
    EqfSend2AllHandlers( WM_EQFN_CREATED, MP1FROMSHORT( clsFOLDER ),
                         MP2FROMP(pData->szNewObject) );
  } /* endif */
  return( fOK );
} /* end of function FolRenameFolder */

BOOL FolCorrectPropHead
(
PSZ         pszPropName,             // ptr to path name of property file
CHAR        chNewDrive,              // new drive or NULC if no set req.
PSZ         pszNewPath,              // new path or NULL if no set req.
PSZ         pszNewName,              // new name of NULL if no set req.
HWND        hwnd
)
{
  BOOL        fOK = TRUE;            // internal OK flag
  HFILE       hFile = NULL;          // code returned by DosXXX calls
  USHORT      usDosRC;               // code returned by DosXXX calls
  USHORT      usOpenAction;          // action performed by DosOpen
  ULONG       ulBytesWritten;        // # of bytes written to file
  ULONG       ulBytesRead;           // # of bytes read from file
  ULONG       ulFilePos;             // position of file pointer
  PPROPHEAD   pPropHead;             // ptr to properties head

  // correct PROPHEAD part in property file
  fOK = UtlAllocHwnd( (PVOID *)&pPropHead, 0L,
                      (LONG) sizeof(PROPHEAD), ERROR_STORAGE, hwnd );
  if ( fOK )
  {
    usDosRC = UtlOpenHwnd( pszPropName,
                           &hFile,
                           &usOpenAction, 0L,
                           FILE_NORMAL,
                           FILE_OPEN,
                           OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYWRITE,
                           0L,
                           TRUE, hwnd );
    fOK = ( usDosRC == 0 );
  } /* endif */
  if ( fOK )
  {
    usDosRC = UtlReadHwnd( hFile,
                           pPropHead,
                           sizeof( PROPHEAD ),
                           &ulBytesRead,
                           TRUE, hwnd );
    fOK = ( usDosRC == 0 );
  } /* endif */
  if ( fOK )
  {
    // automatically correct path information in TranslationManager folders
    if ( stricmp( pPropHead->szPath + 1, ":\\eqf"  ) == 0 )
    {
      UtlMakeEQFPath( pPropHead->szPath, pPropHead->szPath[0], SYSTEM_PATH, NULL );
    } /* endif */       

    if ( pszNewPath != NULL )
    {
      strcpy( pPropHead->szPath, pszNewPath );
    } /* endif */

    if ( chNewDrive != NULC )
    {
      pPropHead->szPath[0] = chNewDrive;
    } /* endif */

    if ( pszNewName != NULL )
    {
      strcpy( pPropHead->szName, pszNewName );
    } /* endif */

    UtlChgFilePtrHwnd( hFile, 0L, FILE_BEGIN, &ulFilePos, TRUE, hwnd );
    usDosRC = UtlWriteHwnd( hFile,
                            pPropHead,
                            sizeof( PROPHEAD ),
                            &ulBytesWritten,
                            TRUE, hwnd );
    fOK = ( usDosRC == 0 );
  } /* endif */
  UtlClose( hFile, FALSE );
  UtlAlloc( (PVOID *)&pPropHead, 0L, 0L, NOMSG );

  return( fOK );

} /* end of FolCorrectPropHead */

BOOL FolIsFolderEmpty( PSZ pszFolObjName )
{
  CHAR szDocPath[MAX_EQF_PATH];
  USHORT    usCount = 1;
  FILEFINDBUF ResultBuf;
  USHORT    usRC;
  HDIR      hDirHandle = HDIR_CREATE;
  BOOL      fIsEmpty = TRUE;

  // setup path to folder documents
  strcpy( szDocPath, pszFolObjName );
  strcat( szDocPath, BACKSLASH_STR );
  UtlQueryString( QST_SOURCEDIR, szDocPath + strlen(szDocPath), MAX_FILESPEC );
  strcat( szDocPath, BACKSLASH_STR );
  strcat( szDocPath, DEFAULT_PATTERN );

  // look for documents
  usRC = UtlFindFirst( szDocPath, &hDirHandle, FILE_NORMAL, &ResultBuf,
                       sizeof( ResultBuf), &usCount, 0L, FALSE );
  if ( usRC == NO_ERROR )
  {
    UtlFindClose( hDirHandle, FALSE );
    fIsEmpty = (usCount == 0);
  } /* endif */
  return( fIsEmpty );
} /* end of function FolIsFolderEmpty */


//
// function removing corrupted records from a history log
//
USHORT FolCleanHistlog( PSZ pszHistLogFile, BOOL fMsg, HWND hwnd )
{
  return( FolCleanHistlogEx( pszHistLogFile, fMsg, hwnd, FALSE ) );
}


USHORT FolCleanHistlogEx( PSZ pszHistLogFile, BOOL fMsg, HWND hwnd, BOOL fClean )
{
  USHORT      usRC = NO_ERROR;         // function return code
  CHAR        szTempLog[MAX_EQF_PATH]; // name of temporary (= output) logfile
  BOOL        fEOFInFile = FALSE;      // TRUE = EOF for hInFile reached
  HFILE       hInFile = NULLHANDLE;    // handle of input log file
  HFILE       hOutFile = NULLHANDLE;   // handle of output log file
  PHISTLOGRECORD pRecord = NULL;       // ptr to buffer for log records
  ULONG       ulLastRecord = 0;        // position of last written record in out file
  UCHAR       ucLastTask = 0;          // ID of last record
  CHAR        szLastDocName[MAX_FILESPEC];    // name of document of last written record
  CHAR        szLastVersion[20];       // version of last version record

  // allocate history log record buffers
  szLastVersion[0] = EOS;
  if ( usRC == NO_ERROR )
  {
    if ( !UtlAllocHwnd( (PVOID *)&pRecord, 0, MAXHISTLOGRECORDSIZE, NOMSG, hwnd ) )
    {
      usRC = ERROR_NOT_ENOUGH_MEMORY;
    } /* endif */
  } /* endif */

  // open input log file
  if ( usRC == NO_ERROR )
  {
    USHORT usOpenAction;

    usRC = UtlOpenHwnd( pszHistLogFile, &hInFile, &usOpenAction, 0L,
                        FILE_NORMAL, FILE_OPEN,
                        OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYWRITE,
                        0L, fMsg, hwnd );
  } /* endif */

  // setup name of temporary output file                              
  if ( usRC == NO_ERROR )
  {
    PSZ pszExt;

    strcpy( szTempLog, pszHistLogFile );
    pszExt = strrchr( szTempLog, DOT );
    if ( pszExt != NULL )
    {
      strcpy( pszExt, ".$$$" );
    }
    else
    {
      strcat( szTempLog, ".$$$" );
    } /* endif */
  } /* endif */

  // create temporary log file
  if ( usRC == NO_ERROR )
  {
    USHORT usOpenAction;

    usRC = UtlOpenHwnd( szTempLog, &hOutFile, &usOpenAction, 0L,
                        FILE_NORMAL,
                        FILE_TRUNCATE | FILE_CREATE,
                        OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYWRITE,
                        0L, fMsg, hwnd );
  } /* endif */

  // read first record of merge-from log file 
  if ( usRC == NO_ERROR )
  {
    usRC = FolReadLogRecord( hInFile, pRecord, &fEOFInFile, fMsg, hwnd );
  } /* endif */


  // process records until EOF
  while ( (usRC == NO_ERROR) && !fEOFInFile )
  {
    BOOL fWriteCurrentRecord = TRUE;

    // check if previous record can be overwritten (e.g. multiple save records for same document)
    if ( fClean )
    {
      if ( (pRecord->Task == ucLastTask) && 
           ( (pRecord->Task == DOCSAVE_LOGTASK) || (pRecord->Task == DOCSAVE_LOGTASK2) || (pRecord->Task == DOCSAVE_LOGTASK3) ) &&                   // save of document in DOCSAVEHIST3 format
           (strcmp( szLastDocName, pRecord->szDocName ) == 0) )
      {
        // overwrite last written record
        ULONG ulCurrentPos = 0;
        UtlChgFilePtr( hOutFile, ulLastRecord, FILE_BEGIN,  &ulCurrentPos, FALSE );
      } /* endif */
    } /* endif */

    // skip superfluos records
    if ( fClean )
    {
      // skip superfluos version records
      if ( pRecord->Task == VERSION_LOGTASK )
      {
        PVERSIONHIST pVersion = (PVERSIONHIST)((PBYTE)pRecord + sizeof(HISTLOGRECORD));

        if ( strcmp( szLastVersion, pVersion->szVersionString ) == 0 )
        {
          fWriteCurrentRecord = FALSE;
        } /* endif */
      } /* endif */

    } /* endif */

    if ( fWriteCurrentRecord )
    {
      // remember current write position
      UtlChgFilePtr( hOutFile, 0L, FILE_CURRENT,  &ulLastRecord, FALSE);


      // write record to output log file
      usRC = FolWriteLogRecord( hOutFile, pRecord, fMsg, hwnd );

      // remember some record settings
      strcpy( szLastDocName, pRecord->szDocName );
      ucLastTask = pRecord->Task;
    } /* endif */

    // read next valid record
    if ( usRC == NO_ERROR )
    {
      usRC = FolReadLogRecord( hInFile, pRecord, &fEOFInFile, fMsg, hwnd );
    } /* endif */
  } /* endwhile */

  // close all logfiles                                               
  if ( hInFile != NULLHANDLE )  UtlClose( hInFile, FALSE );
  if ( hOutFile != NULLHANDLE ) UtlClose( hOutFile, FALSE );

  // delete old log file and rename tempory log file to old name
  if ( usRC == NO_ERROR )
  {
    UtlDeleteHwnd( pszHistLogFile, 0L, fMsg, hwnd );
    usRC = UtlMoveHwnd( szTempLog, pszHistLogFile, 0L, fMsg, hwnd );
  } /* endif */

  // cleanup                                                          
  if ( pRecord != NULL ) UtlAlloc( (PVOID *)&pRecord, 0L, 0L, NOMSG );

  return( usRC );

} /* end of function FolCleanHistlog */

/**********************************************************************/
/* FolReadLogRecord                                                   */
/*                                                                    */
/* Read the next record from the log file into the supplied buffer.   */
/* The buffer must be large enough to contain the fixed part of log   */
/* records, the document long name  and the largest variable part of  */
/* log records.                                                       */
/* The function sets the caller end-of-file flag if the end of the    */
/* log file is exceeded.                                              */
/* The function returns NO_ERROR or any DOS error return code         */
/* encountered                                                        */
/**********************************************************************/
USHORT FolReadLogRecord
(
HFILE       hFile,                   // handle of (open) log file
PHISTLOGRECORD pRecord,              // adress of ptr to buffer for record (fix and var part)
PBOOL       pfEOF,                   // ptr to caller's end-of-file flag
BOOL        fMsg,                    // TRUE = show errors directly
HWND        hwnd                     // window to be used as parent for messages
)
{
  USHORT      usRC = NO_ERROR;         // function return code
  ULONG       ulBytesRead;             // number of bytes read
  BOOL        fRecordDefect = FALSE;

  /********************************************************************/
  /* Read fixed part of log record                                    */
  /********************************************************************/
  if ( (usRC == NO_ERROR) && !*pfEOF )
  {
    usRC = UtlReadHwnd( hFile, pRecord, sizeof(HISTLOGRECORD), &ulBytesRead, fMsg, hwnd );
    if ( (usRC == NO_ERROR) && (ulBytesRead < sizeof(HISTLOGRECORD)) )
    {
      *pfEOF = TRUE; // assume end-of-file reached
    } /* endif */
  } /* endif */

  // loop until valid record or end-of-file
  do
  {
    /********************************************************************/
    /* Check if fixed part of record is valid                           */
    /********************************************************************/
    if ( (usRC == NO_ERROR) && !*pfEOF )
    {
      fRecordDefect = FALSE;

      // check eye catcher
      if ( pRecord->lEyeCatcher != HISTLOGEYECATCHER )
      {
        fRecordDefect = TRUE;
      } /* endif */

      // check size values
      if ( !fRecordDefect )
      {
        if ( pRecord->usAddInfoLength > (pRecord->usSize + sizeof(HISTLOGRECORD)) )
        {
          fRecordDefect = TRUE;
        } /* endif */
      } /* endif */

      // check record type
      if ( !fRecordDefect )
      {
        switch ( pRecord->Task )
        {
          case DOCIMPORT_LOGTASK:
          case DOCIMPNEWTARGET_LOGTASK:
          case ANALYSIS_LOGTASK:
          case AUTOMATICSUBST_LOGTASK:
          case DOCDELETE_LOGTASK:
          case DOCSAVE_LOGTASK:
          case DOCEXPORT_LOGTASK:
          case DOCPROP_LOGTASK:
          case FOLPROP_LOGTASK:
          case FOLPROP_LOGTASK2:
          case LONGNAME_LOGTASK:
          case DOCSAVE_LOGTASK2:
          case DOCIMPNEWTARGET_LOGTASK2:
          case HISTDATA_INVALID_LOGTASK:
          case HISTDATA_RESET_LOGTASK:
          case DOCAPI_LOGTASK:
          case HISTDATA_INCONSISTENT_LOGTASK:
          case VERSION_LOGTASK:
          case FOLPROPSHIPMENT_LOGTASK:
          case DOCIMPORT_LOGTASK2:
          case AUTOMATICSUBST_LOGTASK3:
          case DOCSAVE_LOGTASK3:
          case ANALYSIS_LOGTASK3:
          case DOCAPI_LOGTASK3:
          case DOCIMPNEWTARGET_LOGTASK3:
          case STARGET_CORRUPT_LOGTASK:
            break;
        default:
            fRecordDefect = TRUE;
            break;
        } /*endswitch */
      } /* endif */

      // skip corrupted data up to next valid record
      if ( fRecordDefect )
      {
        // re-position to start of log record
        {
          ULONG ulCurrentPos = 0L;
          UtlChgFilePtr( hFile, 0L, FILE_CURRENT,  &ulCurrentPos, FALSE);
          ulCurrentPos = ulCurrentPos - sizeof(HISTLOGRECORD) + 1;
          UtlChgFilePtr( hFile, ulCurrentPos, FILE_BEGIN,  &ulCurrentPos, FALSE);
        }

        // skip data up to start of next valid record
        {
          LONG lCurrent = 0;

          do
          {

            ULONG ulCurrentPos = 0L;

            UtlChgFilePtr( hFile, 0L, FILE_CURRENT,  &ulCurrentPos, FALSE);

            usRC = UtlReadHwnd( hFile, &lCurrent, sizeof(LONG), &ulBytesRead, fMsg, hwnd );
            if ( (usRC == NO_ERROR) && (ulBytesRead < sizeof(LONG)) )
            {
              *pfEOF = TRUE; // assume end-of-file reached
            } /* endif */

            if ( !*pfEOF )
            {
              if ( lCurrent == HISTLOGEYECATCHER )
              {
                // re-position to start of log record
                {
                  UtlChgFilePtr( hFile, ulCurrentPos, FILE_BEGIN,  &ulCurrentPos, FALSE);
                }

                // read complete record
                usRC = UtlReadHwnd( hFile, pRecord, sizeof(HISTLOGRECORD), &ulBytesRead, fMsg, hwnd );
                if ( (usRC == NO_ERROR) && (ulBytesRead < sizeof(HISTLOGRECORD)) )
                {
                  *pfEOF = TRUE; // assume end-of-file reached
                } /* endif */
              }
              else
              {
                // postiion to original read position + 1 
                UtlChgFilePtr( hFile, ulCurrentPos + 1, FILE_BEGIN,  &ulCurrentPos, FALSE);
              } /* endif */
            } /* endif */
          } while ( !*pfEOF && (lCurrent != HISTLOGEYECATCHER) );
        }
      } /* endif */
    } /* endif */
  } while ( !*pfEOF && fRecordDefect );

  // adjust record sizes of old records (were fill incorreclty)
  if ( !*pfEOF && (usRC == NO_ERROR) )
  {
    HistLogCorrectRecSizes( pRecord );
  } /* endif */


  /********************************************************************/
  /* Read rest of log record if any                                   */
  /********************************************************************/
  if ( (usRC == NO_ERROR) && !*pfEOF && (pRecord->usSize > sizeof(HISTLOGRECORD)) )
  {
    USHORT usRest = pRecord->usSize - sizeof(HISTLOGRECORD);
    usRC = UtlReadHwnd( hFile, (PBYTE)pRecord + sizeof(HISTLOGRECORD), usRest,
                        &ulBytesRead, fMsg, hwnd );
    if ( (usRC == NO_ERROR) && (ulBytesRead < usRest) )
    {
      *pfEOF = TRUE; // assume end-of-file reached
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* Set lDate to maximum value if EOF condition raised in order      */
  /* to allow processing of records from the other input log file    */
  /********************************************************************/
  if ( (usRC == NO_ERROR) && *pfEOF )
  {
    pRecord->lTime = 0x7FFFFFFF;
  } /* endif */

  /********************************************************************/
  /* Return to caller                                                 */
  /********************************************************************/
  return( usRC );
} /* end of function FolReadLogRecord */

/**********************************************************************/
/* FolWriteLogRecord                                                  */
/*                                                                    */
/* Writes the given record to the log file. The record pointer pRecord*/
/* points to a memory area containing the fixed record part followed  */
/* by the variable length record part.                                */
/* The function returns NO_ERROR or any DOS error return code         */
/* encountered                                                        */
/**********************************************************************/
USHORT FolWriteLogRecord
(
HFILE       hFile,                   // handle of (open) log file
PHISTLOGRECORD pRecord,              // ptr to record (fixed and var part)
BOOL        fMsg,                    // TRUE = show errors directly
HWND        hwnd                     // window to be used as parent for messages
)
{
  USHORT      usRC = NO_ERROR;         // function return code
  ULONG       ulBytesWritten;          // number of bytes written to disk

  /********************************************************************/
  /* Write complete record                                           */
  /********************************************************************/
  usRC = UtlWriteHwnd( hFile, pRecord, pRecord->usSize, &ulBytesWritten,
                       fMsg, hwnd );
  return( usRC );
} /* end of function FolWriteLogRecord */

// function to correct size values in older history log records (in these records
// the usSize value was not set correctly)
void HistLogCorrectRecSizes( PHISTLOGRECORD pRecord )
{
  // only process older records ....
  if ( !pRecord->fLongNameRecord )
  {
    USHORT usTotalSize, usAddInfoSize, usNewSize;
    usTotalSize = pRecord->usSize;
    usAddInfoSize = pRecord->usAddInfoLength;
    if ( usAddInfoSize != usTotalSize - sizeof(HISTLOGRECORD) )
    {
      switch ( pRecord->Task )
      {
        case DOCIMPORT_LOGTASK :
          usNewSize = sizeof(DOCIMPORTHIST);
          break;
        case DOCIMPORT_LOGTASK2 :
          usNewSize = sizeof(DOCIMPORTHIST2); //SHIPMENT_HANDLER
          break;

        case ANALYSIS_LOGTASK :
          usNewSize = sizeof(ANALYSISHIST);
          break;
        case DOCDELETE_LOGTASK :
          usNewSize = 0;
          break;
        case AUTOMATICSUBST_LOGTASK :
        case DOCIMPNEWTARGET_LOGTASK :
        case DOCSAVE_LOGTASK  :
          usNewSize = sizeof(DOCSAVEHIST);
          break;
        case DOCEXPORT_LOGTASK :
          usNewSize = sizeof(DOCEXPORTHIST);
          break;
        case DOCPROP_LOGTASK :
          usNewSize = sizeof(DOCPROPHIST);
          break;
        case FOLPROP_LOGTASK :
          usNewSize = sizeof(FOLPROPHIST);
          break;
        case FOLPROP_LOGTASK2 :
          usNewSize = sizeof(FOLPROPHIST2);
          break;
        case FOLPROPSHIPMENT_LOGTASK :
          usNewSize = sizeof(FOLPROPHISTSHIPMENT); //SHIPMENT_HANDLER
          break;

        case LONGNAME_LOGTASK :
          usNewSize = sizeof(LONGNAMEHIST);
          break;
        case DOCIMPNEWTARGET_LOGTASK2 :
        case DOCSAVE_LOGTASK2 :
        case DOCAPI_LOGTASK :
          usNewSize = sizeof(DOCSAVEHIST2);
          break;
        case ANALYSIS_LOGTASK3 :
        case DOCIMPNEWTARGET_LOGTASK3 :
        case DOCSAVE_LOGTASK3 :
        case DOCAPI_LOGTASK3 :
          usNewSize = sizeof(DOCSAVEHIST3);
          break;
        case VERSION_LOGTASK :
          usNewSize = sizeof(VERSIONHIST);
          break;
        default:
          usNewSize = usTotalSize - sizeof(HISTLOGRECORD);
      } /* endswitch */
      pRecord->usSize = usNewSize + sizeof(HISTLOGRECORD);
      pRecord->usAddInfoLength = usNewSize;
    } /* endif */
  } /* endif */
} /* end of function HistLogCorrectRecSizes */







