//+----------------------------------------------------------------------------+
//| CHKCALC.C                                                                  |
//+----------------------------------------------------------------------------+
//| Copyright (C) 2012-2015, International Business Machines                        |
//| Corporation and others.  All rights reserved.                              |
//+----------------------------------------------------------------------------+
//| Author: Gerhard Queck                                                      |
//+----------------------------------------------------------------------------+
//| Description:                                                               |
//|   Scan history log data for loss of analysis autosubst matches             |
//+----------------------------------------------------------------------------+
#undef  DLL
#undef  _WINDLL
#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <direct.h>
#include <eqf.h>
#include <eqfserno.h>

//typedef int BOOL;
typedef char CHAR;
typedef char * PCHAR;
typedef char * PSZ;
typedef unsigned char UCHAR;
typedef unsigned char BYTE;
typedef unsigned char * PBYTE;
typedef short SHORT;
typedef unsigned short USHORT;
typedef long LONG;
typedef unsigned long ULONG;
typedef void * PVOID;
//typedef unsigned long HWND;

#define TRUE   1
#define FALSE  0

#define END_OF_STRING     '\0'
#define EOS               '\0'

#define MAX_FNAME                9
#define NUM_OF_FOLDER_DICS      10
#define MAX_LANG_LENGTH         20
//#define MAX_PATH               144

#include "EQFHLOG.H"
#include <eqfutpck.h>

typedef struct _DOCTABLEENTRY
{
  CHAR szLongName[513];
  BOOL fAnaAutoSubst;
  BOOL fError;
} DOCTABLEENTRY, *PDOCTABLEENTRY;


BOOL IsEmpty( PCRITERIASUM pSum );
BOOL CheckHistLog
(
  PSZ pszHistlog,                      // ptr to name of histlog file
  PSZ pszFolder                        // name of folder being processed
);
BOOL ExtractHistLog
(
  PSZ pszPackName,                     // name of export folder package
  PSZ pszTempHistLog                   // name for extracted histlog file
);
void Message
(
  PSZ  pszFormat,
  PSZ  pszArg
);
void Message2
(
  PSZ  pszFormat,
  PSZ  pszArg1,
  PSZ  pszArg2
);
BOOL ScanExpFolder
(
  PSZ pszExpFol,
  PBOOL pfFolderProcessed
);

static void showHelp();

PSZ pszInFile = NULL;
PSZ pszOutFile = NULL;
PSZ pszDoc = "";
HISTLOGRECORD LogRecord;
CHAR szTempName[1024];
CHAR szTempPath[1024];
WIN32_FIND_DATA FindBuf;

union
{
  DOCSAVEHIST   DocSave;
  DOCIMPORTHIST DocImport;
  DOCPROPHIST   DocProp;
  FOLPROPHIST   FolProp;
  DOCEXPORTHIST DocExport;
  ANALYSISHIST  Analysis;
  DOCSAVEHIST2  DocSave2;
  DOCSAVEHIST3  DocSave3;
  VERSIONHIST   Version;
  LONGNAMEHIST  LongName;
  DOCIMPORTHIST2 DocImport2;
  CHAR chBuffer[32000];
  FOLPROPHISTSHIPMENT FolPropShipment;
} VarPart;

char szDate[512];
char szTime[512];

char szLongDocName[512];               // buffer for long document names

FILE *hLog = NULLHANDLE;
char szLogName[1024];
PSZ  pszHistLog  = NULL;
PSZ  pszFolder   = NULL;
PSZ  pszExp      = NULL;
BOOL fAllFolders = FALSE;

int main
(
  int  argc,
  char *argv[],
  char *envp[]
)
{
  BOOL fOK = TRUE;                     // O.K. flag

  // show help information if needed
  if(argc==2 && stricmp(argv[1],"-h")==0)
  {
      showHelp();
      return 0;
  }

  // build log file name using exe directory and open log file
  {
    PSZ pszPos;
    strcpy( szLogName, argv[0] );
    pszPos = strrchr( szLogName, '\\' );
    if ( pszPos )
    {
      pszPos++;
      *pszPos = '\0';
    }
    else
    {
      _searchenv( "CHKCALC.EXE", "PATH", szLogName );
      if ( szLogName[0] != 0 )
      {
        pszPos = strrchr( szLogName, '\\' );
        if ( pszPos )
        {
          pszPos++;
          *pszPos = '\0';
        }
        else
        {
          _getcwd( szLogName, sizeof(szLogName) );
          strcat( szLogName, "\\" );
        } /* endif */

      }
      else
      {
        _getcwd( szLogName, sizeof(szLogName) );
        strcat( szLogName, "\\" );
      } /* endif */
    } /* endif */
    strcat( szLogName, "CHKCALC.LOG" );
    // printf( "Info: logging to file %s\n", szLogName );
    hLog = fopen( szLogName, "at" );
    // if ( !hLog ) printf( "Info: Error opening log file %s\n", szLogName );

    _strdate( szDate );
    _strtime( szTime );
    if ( hLog ) fprintf( hLog, "--- CHKCALC %s %s ---\n", szDate, szTime );
  }

  /********************************************************************/
  /* Skip first commandline argument (program name)                   */
  /********************************************************************/
  argc--;
  argv++;

  /********************************************************************/
  /* Check commandline arguments                                      */
  /********************************************************************/
  pszInFile = NULL;
  while ( fOK && argc )
  {
    PSZ pszArg;

    pszArg = *argv;
    if ( (pszArg[0] == '-') || (pszArg[0] == '/') )
    {
      /****************************************************************/
      /* Check for options                                            */
      /****************************************************************/
      if ( strnicmp( pszArg + 1, "HLOG=", 5 ) == 0 )
      {
        pszHistLog = pszArg + 6;
        if ( pszFolder || fAllFolders || pszExp )
        {
          Message( "Warning: \"HLOG=\" may not be used together with \"ALL\", \"FXP=\" or \"FLD=\" option, option is ignored.\n", "" );
          pszHistLog = NULL;
        } /* endif */
      }
      else if ( strnicmp( pszArg + 1, "FLD=", 4 ) == 0 )
      {
        pszFolder = pszArg + 5;
        if ( pszHistLog || fAllFolders || pszExp )
        {
          Message( "Warning: \"FLD=\" may not be used together with \"ALL\", \"FXP=\" or \"HLOG=\" option, option is ignored.\n", "" );
          pszFolder = NULL;
        } /* endif */
      }
      else if ( strnicmp( pszArg + 1, "ALL", 3 ) == 0 )
      {
        fAllFolders = TRUE;
        if ( pszHistLog || pszFolder || pszExp )
        {
          Message( "Warning: \"ALL\" may not be used together with \"FLD=\", \"FXP=\" or \"HLOG=\" option, option is ignored.\n", "" );
          fAllFolders = FALSE;
        } /* endif */
      }
      else if ( strnicmp( pszArg + 1, "FXP=", 4 ) == 0 )
      {
        pszExp = pszArg + 5;
        if ( pszHistLog || fAllFolders || pszFolder )
        {
          Message( "Warning: \"FXP=\" may not be used together with \"ALL\", \"FLD=\" or \"HLOG=\" option, option is ignored.\n", "" );
          pszExp = NULL;
        } /* endif */
      }
      else
      {
        Message( "Warning: Unknown commandline switch '%s' is ignored.\n",
                 pszArg );
      } /* endif */
    }
    else
    {
        Message( "Warning: Superfluous commandline value '%s' is ignored.\n",
                pszArg );
    } /* endif */
    argc--;
    argv++;
  } /* endwhile */

  if ( !pszFolder && !pszHistLog && !fAllFolders && !pszExp )
  {
    Message( "Error: Missing input file specification.\n\n", "" );
    showHelp();
    fOK = FALSE;
  } /* endif */

  // process histlog
  if ( fOK )
  {
    if ( pszHistLog )
    {
      CheckHistLog( pszHistLog, NULL );
    }
    else if ( pszExp )
    {
      BOOL fFolderProcessed = FALSE;

      ScanExpFolder( pszExp, &fFolderProcessed );
      if ( !fFolderProcessed )
      {
        if ( strchr( pszExp, '*' ) )
        {
          Message( "Error: No exported folders found using %s .\n", pszExp );
        }
        else
        {
          Message( "Error: Exported folder %s not found.\n", pszExp );
        } /* endif */
      } /* endif */
    }
    else
    {
      // loop over folders an get the correct one
      char szHistLogFile[60];
      char szSearchFol[60];
      char szDrive[10];
      BOOL fMore = FALSE;
      BOOL fFolderProcessed = FALSE;
      HANDLE hDir = NULLHANDLE;

      GetProfileString( APPL_Name, KEY_Drive, "", szDrive, sizeof(szDrive) );
      //sprintf( szSearchFol, "%s\\EQF\\PROPERTY\\*.F00", szDrive );
      sprintf( szSearchFol, "%s\\OTM\\PROPERTY\\*.F00", szDrive );

      hDir = FindFirstFile( szSearchFol, &FindBuf );
      if ( hDir != INVALID_HANDLE_VALUE )
      {
        do
        {
          // load folder property file
          {
            static PROPFOLDER FolProp;
            FILE *hFol = NULLHANDLE;
            CHAR szFolder[14];
            PSZ pszFolName = NULL;
            sprintf( szSearchFol, "%s\\OTM\\PROPERTY\\%s", szDrive, FindBuf.cFileName );
            //sprintf( szSearchFol, "%s\\EQF\\PROPERTY\\%s", szDrive, FindBuf.cFileName );
            hFol = fopen( szSearchFol, "rb" );
            if ( hFol )
            {
              memset( &FolProp, 0, sizeof(FolProp) );
              fread( &FolProp, sizeof(FolProp), 1, hFol );
              fclose( hFol );

              szHistLogFile[0] = EOS;

              if ( FolProp.szLongName[0] != EOS )
              {
                if ( (pszFolder && (stricmp( pszFolder, FolProp.szLongName ) == 0) ) || fAllFolders )
                {
                  sprintf( szHistLogFile, "%c:\\OTM\\%s\\PROPERTY\\HISTLOG.DAT", FolProp.chDrive, FindBuf.cFileName );
                  //sprintf( szHistLogFile, "%c:\\EQF\\%s\\PROPERTY\\HISTLOG.DAT", FolProp.chDrive, FindBuf.cFileName );
                  pszFolName = FolProp.szLongName;
                } /* endif */
              }
              else if ( FolProp.PropHead.szName[0] )
              {
                PSZ pszPos;
                strcpy( szFolder, FolProp.PropHead.szName );
                pszPos = strchr( szFolder, DOT );
                if ( pszPos ) *pszPos = EOS;

                if ( (pszFolder && (stricmp( pszFolder, szFolder ) == 0) ) || fAllFolders )
                {
                  sprintf( szHistLogFile, "%c:\\OTM\\%s\\PROPERTY\\HISTLOG.DAT", FolProp.chDrive, FindBuf.cFileName );
                  //sprintf( szHistLogFile, "%c:\\EQF\\%s\\PROPERTY\\HISTLOG.DAT", FolProp.chDrive, FindBuf.cFileName );
                  pszFolName = szFolder;
                } /* endif */
              } /* endif */

              if ( szHistLogFile[0] )
              {
                CheckHistLog( szHistLogFile, pszFolName );
                fFolderProcessed = TRUE;
              } /* endif */
            } /* endif */
          }

          // continue with next folder
          fMore = FindNextFile( hDir, &FindBuf );
        } while ( fOK && fMore );
        FindClose( hDir );
        if ( !fFolderProcessed )
        {
          if ( pszFolder )
          {
            Message( "Error: Folder %s not found.\n", pszFolder );
          }
          else
          {
            Message( "Warning: No folders found for processing\n", "" );
          } /* endif */
        } /* endif */
      }
      else
      {
        Message( "Error: scan of Tmgr folders failed!\n", "" );
      } /* endif */
    } /* endif */
  } /* endif */

  if ( hLog ) fclose( hLog );

  return( !fOK );
} /* end of main */

BOOL IsEmpty
(
  PCRITERIASUM pSum
)
{
  return( (pSum->SimpleSum.ulSrcWords == 0) &&
          (pSum->MediumSum.ulSrcWords == 0) &&
          (pSum->ComplexSum.ulSrcWords == 0) );
}


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

BOOL CheckHistLog
(
  PSZ pszHistLog,                      // ptr to name of histlog file
  PSZ pszFolder                        // name of folder being processed
)
{
  BOOL fOK = TRUE;                     // O.K. flag
  int iRecNum = 0;                     // current record number
  ULONG ulLength;                      // overall file length
  int iMaxDocs = 0;                    // number of entries in doc table
  PDOCTABLEENTRY pDocs = NULL;         // table containing document names
  BOOL fDataLost = FALSE;              // TRUE = data may have been lost
  FILE     *hInput = NULL;             // input file handle
  struct tm   *pTimeDate;              // time/date structure

  /********************************************************************/
  /* Open input file                                                  */
  /********************************************************************/
  if ( fOK )
  {
    hInput = fopen( pszHistLog, "rb" );
    if ( hInput == NULL )
    {
      if ( pszFolder && *pszFolder )
      {
        Message2( "Folder %s: Error: folder history log file \"%s\" could not be opened.\n", pszFolder, pszHistLog );
      }
      else
      {
        Message( "Error: history log file \"%s\" could not be opened.\n", pszHistLog );
      } /* endif */
      fOK = FALSE;
    }
    else
    {
      ulLength = _filelength( _fileno(hInput) );
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* Loop through input file                                          */
  /********************************************************************/
  if ( fOK )
  {
    LONG lRecordStart;                 // start positionof record within file

    while ( fOK && (ulLength != 0L) && !feof( hInput ) )
    {
      BOOL fRecord = TRUE;
      BOOL fListRecord = TRUE;
      iRecNum++;

      // get fixed length record part
      lRecordStart = ftell( hInput );
      if ( fread( (PVOID)&LogRecord, sizeof(HISTLOGRECORD), 1, hInput ) != 1 )
      {
        if ( pszFolder && *pszFolder )
        {
          Message( "Folder %s: Error: reading of folder history log file failed (code 1).\n", pszFolder );
        }
        else
        {
          Message( "Error: Read of history log %s failed (code 1).\n", pszHistLog );
        } /* endif */
        fOK = FALSE;
      }
      else
      {
        ulLength -= (ULONG)sizeof(HISTLOGRECORD);
      } /* endif */

      // check if record is valid
      if ( fOK )
      {
        if ( LogRecord.lEyeCatcher != HISTLOGEYECATCHER )
        {
          LONG lCurrent;
          LONG lSkipped = 0L;

          // re-position to start of log record
          ulLength += (ULONG)sizeof(HISTLOGRECORD);
          fseek( hInput, lRecordStart, SEEK_SET );

          // skip data up to start of next valid record
          do
          {
            fseek( hInput, lRecordStart, SEEK_SET );
            fread( (PVOID)&lCurrent, sizeof(LONG), 1, hInput );
            if ( lCurrent != HISTLOGEYECATCHER )
            {
              // try next byte
              ulLength--;
              lRecordStart++;
              lSkipped++;
            }
            else
            {
              // position back to start of record
              fseek( hInput, lRecordStart, SEEK_SET );
            } /* endif */
          } while ( (ulLength >= 2L) &&
                    (lCurrent != HISTLOGEYECATCHER) );
          fRecord = FALSE;
        } /* endif */
      } /* endif */

      // correct record sizes if required
      if ( fOK && fRecord )
      {
        HistLogCorrectRecSizes( &LogRecord );
      } /* endif */

      // get any long document name
      if ( fOK && fRecord )
      {
        USHORT usLongNameLength = LogRecord.usSize - LogRecord.usAddInfoLength -
                                  sizeof(HISTLOGRECORD);

        szLongDocName[0] = EOS;
        if  ( LogRecord.fLongNameRecord && (usLongNameLength != 0) )
        {
          if ( fread( (PVOID)szLongDocName, usLongNameLength, 1, hInput ) != 1 )
          {
            if ( pszFolder && *pszFolder )
            {
              Message( "Folder %s: Error: reading of folder history log file failed (code 2).\n", pszFolder );
            }
            else
            {
              Message( "Error: Read of history log %s failed (code 2).\n", pszHistLog );
            } /* endif */
            fOK = FALSE;
          }
          else
          {
            ulLength -= usLongNameLength;
          } /* endif */
        } /* endif */
      } /* endif */

      // get variable length record part
      if ( fOK && fRecord && (LogRecord.usAddInfoLength != 0) )
      {
        if ( fread( (PVOID)&VarPart, LogRecord.usAddInfoLength, 1,
                    hInput ) != 1 )
        {
          if ( pszFolder && *pszFolder )
          {
            Message( "Folder %s: Error: reading of folder history log file failed (code 3).\n", pszFolder );
          }
          else
          {
            Message( "Error: Read of history log %s failed (code 3).\n", pszHistLog );
          } /* endif */
          fOK = FALSE;
        }
        else
        {
          ulLength -= (ULONG)LogRecord.usAddInfoLength;
        } /* endif */
      } /* endif */

      // prepare date of records
      if ( fOK )
      {
        pTimeDate = localtime( &(LogRecord.lTime) );
      } /* endif */

      // handle record contents
      if ( fOK && fRecord )
      {
        PSZ pszDocName;
        int iDoc = 0;

        if ( szLongDocName[0] != EOS )
        {
          pszDocName = szLongDocName;
        }
        else
        {
          pszDocName = LogRecord.szDocName;
        } /* endif */

        // lookup document in our document table
        while ( (iDoc < iMaxDocs) &&
                (stricmp( pDocs[iDoc].szLongName, pszDocName) != 0) )
        {
          iDoc++;
        } /* endif */

        // add new document to document table
        if ( fOK && (iDoc >= iMaxDocs) )
        {
          LONG lNewSize = (iMaxDocs + 1) * sizeof(DOCTABLEENTRY);
          LONG lOldSize = iMaxDocs * sizeof(DOCTABLEENTRY);
          PDOCTABLEENTRY pNewTable = (PDOCTABLEENTRY)malloc( lNewSize );
          if ( pNewTable )
          {
            memset( pNewTable, 0, lNewSize );
            if ( pDocs )
            {
              memcpy( pNewTable, pDocs, lOldSize );
              free( pDocs );
            } /* endif */
            iDoc = iMaxDocs;
            iMaxDocs += 1;
            pDocs = pNewTable;
            strcpy( pDocs[iDoc].szLongName, pszDocName );
          }
          else
          {
            Message( "Error: Memory allocation failed (code 7).\n", "" );
            fOK = FALSE;
          } /* endif */
        } /* endif */

        // handle task specific data
        if ( fOK && ( (pTimeDate->tm_year + 1900) >= 2002 ) )
        {
          switch ( LogRecord.Task )
          {
            case  ANALYSIS_LOGTASK  :
            case  DOCIMPNEWTARGET_LOGTASK3 :
            case  DOCDELETE_LOGTASK :
            case  DOCIMPNEWTARGET_LOGTASK :
              pDocs[iDoc].fAnaAutoSubst = FALSE;
              break;

            case  AUTOMATICSUBST_LOGTASK :
              pDocs[iDoc].fAnaAutoSubst = !IsEmpty( &(VarPart.DocSave.AnalAutoSubst) );
              break;

            case  AUTOMATICSUBST_LOGTASK3 :
              pDocs[iDoc].fAnaAutoSubst = !IsEmpty( &(VarPart.DocSave3.AnalAutoSubst) );
              break;

            case  DOCSAVE_LOGTASK   :
              if ( pDocs[iDoc].fAnaAutoSubst && IsEmpty( &(VarPart.DocSave.AnalAutoSubst) ) )
              {
                fDataLost = TRUE;
                pDocs[iDoc].fError = TRUE;
              } /* endif */
              break;

            case  DOCSAVE_LOGTASK2   :
            case  DOCAPI_LOGTASK :
              if ( pDocs[iDoc].fAnaAutoSubst && IsEmpty( &(VarPart.DocSave2.AnalAutoSubst) ) )
              {
                fDataLost = TRUE;
                pDocs[iDoc].fError = TRUE;
              } /* endif */
              break;

            case  DOCSAVE_LOGTASK3   :
            case  DOCAPI_LOGTASK3 :
              if ( pDocs[iDoc].fAnaAutoSubst && IsEmpty( &(VarPart.DocSave3.AnalAutoSubst) ) )
              {
                fDataLost = TRUE;
                pDocs[iDoc].fError = TRUE;
              } /* endif */
              break;


            default :
              // ingore this record ...
              fRecord = FALSE;
              break;
          } /* endswitch */
        } /* endif */
      } /* endif */
    } /* endwhile */
  } /* endif */

  // close message
  if ( fOK )
  {
    PSZ pszError = "Error condition found. Re-create calc. report with TP5.5.2.3 or higher for correction.";
    PSZ pszOK = "History data correct. No action required.";
    if ( fDataLost )
    {
      if ( pszExp )
      {
        Message2( "Exported folder %-20s: %s\n", pszExp, pszError );
      }
      else if ( pszFolder && *pszFolder )
      {
        Message2( "Folder %-20s: %s\n", pszFolder, pszError );
      }
      else
      {
        Message2( "History log %s: %s\n", pszHistLog, pszError  );
      } /* endif */
    }
    else
    {
      if ( pszExp )
      {
        Message2( "Exported folder %-20s: %s\n", pszExp, pszOK );
      }
      else if ( pszFolder && *pszFolder )
      {
        Message2( "Folder %-20s: %s\n", pszFolder, pszOK );
      }
      else
      {
        Message2( "History log %s: %s\n", pszHistLog, pszOK );
      } /* endif */
    } /* endif */
  } /* endif */

  // Cleanup
  if ( hInput )  fclose( hInput );
  if ( pDocs )   free( pDocs );

  return( fOK );
} /* end of CheckHistLog */

PACKHEADER2 PackHead;
PACKHEADER  OldPackHead;

BOOL ExtractHistLog
(
  PSZ pszPackName,                     // name of export folder package
  PSZ pszTempHistLog                   // name for extracted histlog file
)
{
  BOOL fOK = TRUE;
  FILE *hPack = NULLHANDLE;
  FILE *hHistLog = NULLHANDLE;
  FILELISTENTRY FileEntry;
  static CHAR chBuffer[4096];

  // open package file
  hPack = fopen( pszPackName, "rb" );
  if ( hPack == NULLHANDLE )
  {
    Message( "Error: could not open exported folder %s\n", pszPackName );
    fOK = FALSE;
  } /* endif */

  // read package header
  if ( fOK )
  {
    // read package header
    if ( fread( &PackHead, sizeof(PACKHEADER2), 1, hPack ) != 1 )
    {
      Message( "Error: read of exported folder %s failed\n", pszPackName );
      fOK = FALSE;
    } /* endif */
  } /* endif */

  // convert package header to new format if necessary and correct
  // file pointer position
  if ( fOK )
  {
    if ( (memcmp( PackHead.bPackID, PACKHEADID, 4 ) == 0) ||
         (memcmp( PackHead.bPackID, PACKHEAD2ID, 4 ) == 0) )
    {
      // old format header, // convert to new layout
      memcpy( &OldPackHead, &PackHead, sizeof( OldPackHead) );
      memset( &PackHead, 0, sizeof(PACKHEADER2) );
      memcpy( PackHead.bPackID, OldPackHead.bPackID, 4 );
      PackHead.usVersion              = OldPackHead.usVersion;
      PackHead.ulPackDate             = OldPackHead.ulPackDate;
      PackHead.usSequence             = OldPackHead.usSequence;
      PackHead.fCompleted             = OldPackHead.fCompleted;
      PackHead.fLastFileOfPackage     = OldPackHead.fLastFileOfPackage;
      PackHead.ulUserHeaderSize       = OldPackHead.usUserHeaderSize;
      PackHead.ulFileListSize         = OldPackHead.usFileListSize;
      PackHead.ulFileListEntries      = OldPackHead.usFileListEntries;
      PackHead.ulFileNameBufferSize   = OldPackHead.usFileNameBufferSize;

      // adjust file pointer position
      {
        fseek( hPack, sizeof(OldPackHead), SEEK_SET );
      }
    } /* endif */
  } /* endif */

  // scan file list for histlog file
  if ( fOK )
  {
    //--- position to start of file list in package file ---
    {
      LONG lTablePos;

      if ( (memcmp( PackHead.bPackID, PACKHEADID, 4 ) == 0) ||
           (memcmp( PackHead.bPackID, PACKHEAD2ID, 4 ) == 0) )
      {
        // version 0 or 1 header
        lTablePos = sizeof(PACKHEADER);
      }
      else
      {
        // version 2 header
        lTablePos = sizeof(PACKHEADER2);
      } /* endif */

      lTablePos += PackHead.ulUserHeaderSize;
      fseek( hPack, lTablePos, SEEK_SET );
    }

    // read file list entries until histlog file found or all entries processed
    {
      ULONG ulFile = 0;
      memset( &FileEntry, 0, sizeof(FILELISTENTRY) );
      do
      {
        if ( fread( &FileEntry, sizeof(FILELISTENTRY), 1, hPack ) != 1 )
        {
          Message( "Error: read of exported folder %s failed\n", pszPackName );
          fOK = FALSE;
        } /* endif */
        ulFile++;
      } while ( fOK &&
                (ulFile < PackHead.ulFileListEntries) &&
                (FileEntry.usFileType != HISTLOG_DATA_FILE) );
    }

    // check if histlog was contained in file list
    if ( fOK )
    {
      if ( FileEntry.usFileType != HISTLOG_DATA_FILE )
      {
        Message( "Error: no history log found in exported folder %s\n", pszPackName );
        fOK = FALSE;
      } /* endif */
    } /* endif */
  } /* endif */

  // extract histlog to temporary file
  if ( fOK )
  {
    // open histlog file
    hHistLog = fopen( pszTempHistLog, "wb" );
    if ( hHistLog == NULLHANDLE )
    {
      Message( "Error: unexpected error writing temp file %s\n", pszTempHistLog );
      fOK = FALSE;
    } /* endif */

    // position to start of histlog in package file
    if ( fOK )
    {
      fseek ( hPack, FileEntry.ulFilePos, SEEK_SET );
    } /* endif */

    // read from package and write to histlog until complete
    if ( fOK )
    {
      ULONG ulRemaining = FileEntry.ulFileSize;
      while ( fOK && ulRemaining )
      {
        ULONG ulBytesToRead = min( ulRemaining, sizeof(chBuffer) );
        if ( fread( chBuffer, ulBytesToRead, 1, hPack ) != 1 )
        {
          Message( "Error: read of exported folder %s failed\n", pszPackName );
          fOK = FALSE;
        } /* endif */

        if ( fOK )
        {
          ulRemaining -= ulBytesToRead;
          if ( fwrite( chBuffer, ulBytesToRead, 1, hHistLog ) != 1 )
          {
            Message( "Error: unexpected error writing temp file %s\n", pszTempHistLog );
            fOK = FALSE;
          } /* endif */
        } /* endif */
      } /* endwhile */
    } /* endif */
  } /* endif */

  // cleanup
  if ( hPack ) fclose( hPack );
  if ( hHistLog ) fclose( hHistLog );

  return ( fOK );
} /* end of function ExtractHistLog */


void Message
(
  PSZ  pszFormat,
  PSZ  pszArg
)
{
  if ( hLog ) fprintf( hLog, pszFormat, pszArg );
  printf( pszFormat, pszArg );
} /* end of function Message */

void Message2
(
  PSZ  pszFormat,
  PSZ  pszArg1,
  PSZ  pszArg2
)
{
  if ( hLog ) fprintf( hLog, pszFormat, pszArg1, pszArg2 );
  printf( pszFormat, pszArg1, pszArg2 );
} /* end of function Message2 */

static CHAR szExpFolder[1024];

BOOL ScanExpFolder
(
  PSZ pszExpFol,
  PBOOL pfFolderProcessed
)
{
  HANDLE hDir = NULLHANDLE;
  BOOL fMore = FALSE;
  BOOL fOK = TRUE;
  PSZ  pszSearch = (PSZ)malloc( 1024 );
  PSZ  pszSubDir = (PSZ)malloc( 1024 );
  PSZ  pszCriteria = (PSZ)malloc( 1024 );

  // split search criteria from search string
  {
    PSZ pszPos = strrchr( pszExpFol, '\\' );
    if ( pszPos )
    {
      pszPos++;
      strcpy( pszCriteria, pszPos );
    }
    else
    {
      strcpy( pszCriteria, pszExpFol );
    } /* endif */
  }

  // loop over all exported folders
  hDir = FindFirstFile( pszExpFol, &FindBuf );
  if ( hDir != INVALID_HANDLE_VALUE )
  {
    do
    {
      // setup folder package name
      {
        PSZ pszPos;
        strcpy( szExpFolder, pszExpFol );
        pszPos = strrchr( szExpFolder, '\\' );
        if ( pszPos )
        {
          pszPos++;
          strcpy( pszPos, FindBuf.cFileName );
        }
        else
        {
          strcpy( szExpFolder, FindBuf.cFileName );
        } /* endif */
      }

      // build temp name for extracted history log
      GetTempPath( sizeof(szTempPath)-1, szTempPath );
      //GetTempFileName( szTempPath, "EQF", 0, szTempName );
      GetTempFileName( szTempPath, "OTM", 0, szTempName );

      // extract history log
      fOK = ExtractHistLog( szExpFolder, szTempName );
      *pfFolderProcessed = TRUE;

      if ( fOK )
      {
        pszExp = szExpFolder;
        CheckHistLog( szTempName, NULL );
      }
      else
      {
        fOK = TRUE; // reset error condition for next folder ...
      } /* endif */

      // cleanup
      _unlink( szTempName );

      // continue with next exported folder
      fMore = FindNextFile( hDir, &FindBuf );
    } while ( fOK && fMore );
    FindClose( hDir );
  } /* endif */

  // scan directory for subfolders if specification contains '*' or '?'
  if ( strchr( pszExpFol, '*' ) || strchr( pszExpFol, '?' ) )
  {
    PSZ pszEnd;
    strcpy( pszSearch, pszExpFol );
    pszEnd = strrchr( pszSearch, '\\' );
    if ( pszEnd )
    {
      pszEnd++;
      strcpy( pszEnd, "*.*" );
    }
    else
    {
      pszEnd = strrchr( pszSearch, ':' );
      if ( pszEnd )
      {
        pszEnd++;
        strcpy( pszEnd, "*.*" );
      }
      else
      {
        strcpy( pszSearch, "*.*" );
      } /* endif */
    } /* endif */

    hDir = FindFirstFile( pszSearch, &FindBuf );
    if ( hDir != INVALID_HANDLE_VALUE )
    {
      do
      {
        if ( (FindBuf.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
             (strcmp( FindBuf.cFileName, ".." ) != 0) &&
             (strcmp( FindBuf.cFileName, "." ) != 0) )
        {
          strcpy( pszSubDir, pszSearch );
          pszSubDir[strlen(pszSubDir)-3] = '\0';
          strcat( pszSubDir, FindBuf.cFileName );
          strcat( pszSubDir, "\\" );
          strcat( pszSubDir, pszCriteria );

          ScanExpFolder( pszSubDir, pfFolderProcessed );
        } /* endif */

        // continue with next file
        fMore = FindNextFile( hDir, &FindBuf );
      } while ( fMore );
      FindClose( hDir );
    } /* endif */
  } /* endif */

  // cleanup
  free( pszSearch );
  free( pszSubDir );
  free( pszCriteria );

  return( fOK );
} /* end of function ScanExpFolder */

void showHelp()
{
    printf ("\n");
    printf( "OtmChkCalc.EXE           : History log data checker\n" );
    printf( "Version               : %s\n", STR_DRIVER_LEVEL_NUMBER );
    printf( "Copyright             : %s\n",STR_COPYRIGHT );
    printf( "Purpose               : Check histoy log for loss of data\n" );
    printf( "Syntax format         : OtmChkCalc /FLD=foldername \n" );
    printf( "                                  or         \n" );
    printf( "                        OtmChkCalc /HLOG=histlogname \n" );
    printf( "                                  or         \n" );
    printf( "                        OtmChkCalc /FXP=expfolder  \n" );
    printf( "                                  or         \n" );
    printf( "                        OtmChkCalc /ALL\n" );
    printf( "Options and parameters:\n" );
    printf( "    /FLD        the name of a Tmgr folder (w/o path and ext)\n");
    printf( "    /FXP        the fully qualified name of an exported Tmgr folder\n");
    printf( "    /HLOG       the fully qualified history log file name\n");
    printf( "    /ALL        check all installed folders\n");
}
