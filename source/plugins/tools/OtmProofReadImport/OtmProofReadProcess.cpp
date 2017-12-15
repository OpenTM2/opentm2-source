/*! \file
	Description: Procees a group of selected proff read entries and do the updates to the document and/or 
               translation memory

	Copyright Notice:

	Copyright (C) 1990-2017, International Business Machines
	Corporation and others. All rights reserved
*/

#define INCL_EQF_FOLDER           // folder list and document list functions
#define INCL_EQF_TM               // Translation Memory functions
#define INCL_EQF_TP               // public translation processor functions
#define INCL_EQF_EDITORAPI        // editor API
#define INCL_EQF_TAGTABLE         // tagtable defines and functions
#define INCL_EQF_ANALYSIS         // analysis functions
#define INCL_EQF_MORPH            // morphologic functions
#include <eqf.h>                  // General Translation Manager include file
#include "eqfdde.h"               // batch mode definitions
#include "eqffol00.h"             // our .h stuff
#include "eqffol.id"              // our ID file
#include "eqftmi.h"               // TM internal definitions
#include "EQFHLOG.H"            // Translation Processor priv. include file
#include "eqflp.h"
#include <windowsx.h>
#include <commctrl.h>

#include <process.h>              /* _beginthread, _endthread */
  #include <direct.h>

#include "eqfutmdi.h"           // MDI utilities
#include "richedit.h"           // MDI utilities

#include "OtmProposal.h"
#include "core\memory\MemoryFactory.h"
#include "cxmlwriter.h"

#include "OtmProofReadWindow.h"
#include "OtmProofReadImportPlugin.id"

static char szErrorTitle[] = "Validation Document Import Error";
static char szInfoTitle[] = "Validation Document Import Info";

typedef struct _PROOFREADPROCESSDATA
{
  // input parameter
  BOOL   fProcessMem;                  // TRUE = update memory
  BOOL   fProcessDoc;                  // TRUE = update document
  HWND   hwndDialog;                   // handle of proof read import dialog
  HWND   hwndProgressWindow;           // handle of progress window

  // processing flags and progress
  BOOL   fTerminate;                   // TRUE = terminate processing
  int    iProcessed;                   // number of processed entries
  int    iToBeProcessed;               // number of entries to be processed

  // proof read entry information
  OtmProofReadList *pList;             // ptr to list with proof read entries
  int    iCurEntryNum;                 // nmber of current entry (-1 = no entry is active yet)
  OtmProofReadEntry *pEntry;           // pointer to current proof read entry object

  // folder info
  int    iCurFolderNum;                // number of currently active folder (-1 = no current folder)
  char   szFolShortName[MAX_PATH];     // short name of current folder
  char   szFolLongName[MAX_PATH];      // long name of current folder
  char   szFolObjName[MAX_PATH];       // object name of current folder

  // document info
  int    iCurDocNum;                   // number of currently active document (-1 = no current document)
  char   szDocShortName[MAX_PATH];     // short name of current document
  char   szDocLongName[MAX_PATH];      // long name of current document
  char   szDocObjName[MAX_PATH];       // object name of current document
  char   szDocMemory[MAX_PATH];        // memory to be used for this document
  char   szDocMarkup[MAX_PATH];        // document markup table
  char   szDocSourceLang[MAX_LANG_LENGTH]; // document source language
  char   szDocTargetLang[MAX_LANG_LENGTH]; // document target language
  char   szDocAlias[MAX_PATH];         // long alias name of current document
  BOOL   fDocChanged;                  // TRUE = document has been changed
  PTBDOCUMENT pTargetDoc;              // loaded target document
  PTBDOCUMENT pSourceDoc;              // loaded source document
  PLOADEDTABLE pQFTagTable;            // tag table containing internal document tags
  ULONG ulSourceOemCP;                 // ASCII codepage for document source language
  ULONG ulTargetOemCP;                 // ASCII codepage for document target language
  ULONG ulTargetAnsiCP;                // ANSI codepage for document target language
  SHORT sTargetLangID;                 // language ID for document target language
  SHORT sSourceLangID;                 // language ID for document source language

  // markup table info
  char   szCurMarkup[MAX_PATH];        // name of active markup, empty if no markup has been aciviated yet
  PLOADEDTABLE pTagTable;              // tag table for current markup
  PFNGETSEGCONTTEXT pfnGetSegContext;  // get segment context function of markup table

  // memory info
  char   szCurMemory[MAX_PATH];        // long name of active memory object. empty if no memory has been opened yet
  OtmMemory *pMem;                     // ptr to active memory object

  // buffer areas
  CHAR_W szSegmentContext[MAX_SEGMENT_SIZE+1]; // buffer for segment context
  char szMessageBuffer[MAX_SEGMENT_SIZE]; // buffer for error messages
  CHAR_W szAdjustedTarget[MAX_SEGMENT_SIZE+1]; // buffer for target segment with adjusted white space
  BYTE bDiffInputBuffer[30000L];       // input buffer for EQFBFindDiff function
  BYTE bDiffTokenBuffer[10000];        // token buffer for EQFBFindDiff function

  // counters and logging
  int iDocSegmentFailed;               // number of document segment updates failed    
  int iDocSegmentChanged;              // number of document segment updates succeeded
  int iDocSegmentUpdates;              // number of document segment updates processed
  char szLogFile[MAX_PATH];            // name of log file
  FILE *hfLog;                         // log file handle


} PROOFREADPROCESSDATA, *PPROOFREADPROCESSDATA;

// prototypes of internal functions
MRESULT ProofReadProcessCallBack( PPROCESSCOMMAREA pCommArea, HWND hwnd, WINMSG message, WPARAM mp1, LPARAM mp2 );
BOOL ProofReadCloseMem( PPROOFREADPROCESSDATA pData );
BOOL ProofReadFreeDoc( PPROOFREADPROCESSDATA pData );
BOOL ProofReadFreeMarkup( PPROOFREADPROCESSDATA pData );
BOOL ProofReadLoadMarkup( PPROOFREADPROCESSDATA pData );
BOOL ProofReadPrepareDocInfo( PPROOFREADPROCESSDATA pData );
BOOL ProofReadLoadDoc( PPROOFREADPROCESSDATA pData );
BOOL ProofReadOpenMem( PPROOFREADPROCESSDATA pData );
BOOL ProofReadUpdateDoc( PPROOFREADPROCESSDATA pData );
BOOL ProofReadUpdateMem( PPROOFREADPROCESSDATA pData );
BOOL ProofReadSingleDoc( PPROOFREADPROCESSDATA pData, BOOL fTargetDoc );
VOID ProofReadProcess( HWND hwnd, PPROOFREADPROCESSDATA pData );
void ProofReadShowProgress( PPROOFREADPROCESSDATA pData );
BOOL ProofReadSingleDoc( PPROOFREADPROCESSDATA pData, BOOL fTargetDoc );
BOOL ProofReadWriteEntryToLog( PPROOFREADPROCESSDATA pData, PSZ_W pszSegSource, PSZ_W pszError, BOOL fSource  );
BOOL ProofReadAdjustWhiteSpaceInTarget( PPROOFREADPROCESSDATA pData, PSZ_W pszOrgTarget, PSZ_W pszValDocTarget, PSZ_W pszValDocModTarget, PSZ_W pszNewTarget );
SHORT ProofReadstrcmp( PSZ_W  pSrc, PSZ_W  pTgt );

BOOL ProofReadProcessList( OtmProofReadList *pListIn, BOOL fProcessDocIn, BOOL fProcessMemIn, HWND hwndDialog, int iNumOfRuns )
{
  BOOL fOK = TRUE;
  PPROOFREADPROCESSDATA pData = NULL;

  fOK = UtlAlloc( (PVOID *)&pData, 0, sizeof(PROOFREADPROCESSDATA), ERROR_STORAGE );
  if ( !fOK ) return( fOK );

  // setup logfile name
  UtlMakeEQFPath( pData->szLogFile, NULC, LOG_PATH, NULL );
  sprintf( pData->szLogFile + strlen(pData->szLogFile), "\\ValidationImport-%ld.log", iNumOfRuns );

  pData->pList = pListIn;
  pData->fProcessMem = fProcessMemIn;
  pData->fProcessDoc = fProcessDocIn;
  pData->hwndDialog = hwndDialog;

  fOK = CreateProcessWindow( "ValidationDocumentImportProcessing", ProofReadProcessCallBack, (PVOID)pData );

  return( fOK );
}


MRESULT ProofReadProcessCallBack
(
  PPROCESSCOMMAREA pCommArea,          // ptr to commmunication area
  HWND             hwnd,               // handle of process window
  WINMSG           message,            // message to be processed
  WPARAM           mp1,                // first message parameter
  LPARAM           mp2                 // second message parameter
)
{
  PPROOFREADPROCESSDATA pData;         // pointer to process data
  MRESULT         mResult = FALSE;     // return code for proc

  switch( message)
  {
    // Fill fields in communication area
    case WM_CREATE :
      pData                =(PPROOFREADPROCESSDATA)mp2;
      pData->hwndProgressWindow = hwnd;
      pCommArea->pUserIDA = (PVOID)pData;

      pCommArea->sProcessWindowID = ID_PROOFIMP_PROCESS_WINDOW;
      pCommArea->sProcessObjClass = clsSERVICES;
      pCommArea->Style            = PROCWIN_TEXTSLIDER;
      pCommArea->sSliderID        = ID_PROOFIMP_PROCESS_SLIDER;
      strcpy( pCommArea->szTitle, "Processing selected validation document entries" );
      pCommArea->hIcon            = (HPOINTER) UtlQueryULong(QL_ANAICON); //hiconANA;
      pCommArea->fNoClose         = FALSE;

      pCommArea->swpSizePos.x     = 200;
      pCommArea->swpSizePos.y     = 200;
      pCommArea->swpSizePos.cx    = (SHORT) UtlQueryULong( QL_AVECHARWIDTH ) * 80;
      pCommArea->swpSizePos.cy    = (SHORT) UtlQueryULong( QL_PELSPERLINE ) * 15;
      pCommArea->sTextID          = ID_PROOFIMP_PROCESS_TEXT;
      pCommArea->asMsgsWanted[0]  = WM_EQF_SLIDER_SETTEXT;
      pCommArea->asMsgsWanted[1]  = 0;
      pCommArea->usComplete       = 0;
      break;


    // Process proof read entries
    case WM_EQF_INITIALIZE:
      pData     = (PPROOFREADPROCESSDATA) pCommArea->pUserIDA;
      ProofReadProcess( hwnd, pData ); 
      break;

    // Prepare/initialize shutdown of process
    case WM_CLOSE:
      pData     = (PPROOFREADPROCESSDATA) pCommArea->pUserIDA;
      if ( pData )
      {
         mResult = MRFROMSHORT( TRUE );   // = do not close right now
         pData->fTerminate = TRUE;
      }
      else
      {
         mResult = MRFROMSHORT( FALSE );  // = continue with close
      } /* endif */
      break;

    // Cleanup all resources used by the process                      
    case WM_DESTROY:
      pData     = (PPROOFREADPROCESSDATA) pCommArea->pUserIDA;
      if ( pData )
      {
        ProofReadCloseMem( pData );
        ProofReadFreeDoc( pData );
        ProofReadFreeMarkup( pData );
        if ( pData->pQFTagTable ) TAFreeTagTable( pData->pQFTagTable );
        PostMessage( pData->hwndDialog, WM_EQF_PROCESSTASK, MP1FROMSHORT(ENABLE_TASK), WPARAM(TRUE) );
        PostMessage( pData->hwndDialog, WM_EQF_PROCESSTASK, MP1FROMSHORT(REFRESH_TASK), 0 );
        UtlAlloc( (PVOID *) &pData, 0L, 0L, NOMSG );
        pCommArea->pUserIDA = NULL;
      } /* endif */
      break;

    case WM_EQF_TERMINATE:
      mResult = MRFROMSHORT( FALSE );          // = continue with close
      break;

    case WM_INITMENU:
      UtlMenuDisableAll( hwnd, SHORT1FROMMP1( mp1) );
      break;

    case WM_EQF_SLIDER_SETTEXT:
      pData     = (PPROOFREADPROCESSDATA) pCommArea->pUserIDA;
      if ( PVOIDFROMMP2(mp2) != NULL )
      {
        strcpy( pCommArea->szText, (char*) PVOIDFROMMP2(mp2) );
        WinSendMsg( hwnd, WM_EQF_UPDATESLIDER, NULL, NULL );
      } /* endif */
      return( mResult );
      break;
  } /* endswitch */
  return( mResult );
} /* end of function ProofReadProcessCallBack */

enum PROOFREADPROCESSPHASES{ PROOFREAD_INIT, PROOFREAD_NEXTENTRY, PROOFREAD_DONE, PROOFREAD_PREPAREDOCINFO, PROOFREAD_LOADDOC, PROOFREAD_OPENMEM, PROOFREAD_UPDATEDOC, PROOFREAD_UPDATEMEM, PROOFREAD_TERMINATE };

VOID ProofReadProcess( HWND hwnd, PPROOFREADPROCESSDATA pData )
{
  USHORT usPhase = PROOFREAD_INIT;

  while ( !pData->fTerminate && (usPhase != PROOFREAD_DONE) )
  {
    UtlDispatch(); 

    if ( !WinIsWindow( NULLHANDLE, hwnd ) || pData->fTerminate ) 
    {
      // terminate current action 
      usPhase = PROOFREAD_TERMINATE;
    }
      
    switch ( usPhase )
    {
      case PROOFREAD_INIT:         // initialize the processing of proof read entries
        pData->iCurDocNum = -1;
        pData->iCurEntryNum = -1;
        pData->iCurFolderNum = -1;
        pData->iToBeProcessed = pData->pList->getNumOfSelected();
        pData->iProcessed = 0;
        pData->iDocSegmentFailed = 0;
        pData->iDocSegmentChanged = 0;
        pData->iDocSegmentUpdates = 0;

        usPhase = PROOFREAD_NEXTENTRY;
        //ShowWindow( pData->hwndDialog, SW_HIDE );
        SendMessage( pData->hwndDialog, WM_EQF_PROCESSTASK, MP1FROMSHORT(ENABLE_TASK), WPARAM(FALSE) );
        UtlDispatch();
        break;

      case PROOFREAD_PREPAREDOCINFO:  // prepare document and folder info for current entry
        if ( ProofReadPrepareDocInfo( pData ) )
        {
          usPhase = PROOFREAD_LOADDOC;
        }
        else
        {
          usPhase = PROOFREAD_TERMINATE;
        } /* endif */
        break;

      case PROOFREAD_LOADDOC:      // load document into memory (if not loaded yet)
        if ( ProofReadLoadDoc( pData ) )
        {
          usPhase = PROOFREAD_OPENMEM;
        }
        else
        {
          usPhase = PROOFREAD_TERMINATE;
        } /* endif */
        break;

      case PROOFREAD_OPENMEM:      // open the associated translation memory (if not open yet)
        if ( !pData->fProcessMem )
        {
          usPhase = PROOFREAD_UPDATEDOC;
        }
        else if ( ProofReadOpenMem( pData ) )
        {
          usPhase = PROOFREAD_UPDATEDOC;
        }
        else
        {
          usPhase = PROOFREAD_TERMINATE;
        } /* endif */
        break;

      case PROOFREAD_UPDATEDOC:    // update document with data from curretn entry
        if ( !pData->fProcessDoc )
        {
          usPhase = PROOFREAD_UPDATEMEM;
        }
        else if ( ProofReadUpdateDoc( pData ) )
        {
          usPhase = PROOFREAD_UPDATEMEM;
        }
        else
        {
          usPhase = PROOFREAD_TERMINATE;
        } /* endif */
        break;

      case PROOFREAD_UPDATEMEM:    // update memory with data from current entry
        if ( !pData->fProcessMem )
        {
          usPhase = PROOFREAD_NEXTENTRY;
        }
        else if ( ProofReadUpdateMem( pData ) )
        {
          usPhase = PROOFREAD_NEXTENTRY;
        }
        else
        {
          usPhase = PROOFREAD_TERMINATE;
        } /* endif */
        break;

      case PROOFREAD_NEXTENTRY:    // move to the next process entry in the list
        {
          // update progress bar
          if ( pData->iCurEntryNum != -1 )
          {
            if ( pData->pEntry ) pData->pEntry->setProcessed( TRUE );
            pData->iProcessed += 1;
            ProofReadShowProgress( pData );
          } /* endif */

          // find next selected entry
          int iNumOfEntries = pData->pList->size();
          BOOL fSelected = FALSE;
          do
          {
            pData->iCurEntryNum += 1;
            if ( pData->iCurEntryNum < iNumOfEntries )
            {
              pData->pEntry = (*pData->pList)[pData->iCurEntryNum];
              fSelected = pData->pEntry->getSelected();
            }
          } while ( (pData->iCurEntryNum < iNumOfEntries) && !fSelected );

          ProofReadShowProgress( pData );

          if ( fSelected )
          {
            usPhase = PROOFREAD_PREPAREDOCINFO;
          }
          else
          {
            usPhase = PROOFREAD_TERMINATE;
          } /* endif */
        }
        break;

      case PROOFREAD_TERMINATE:    // terminate current processing
        pData->fTerminate = TRUE;
        if ( pData->iDocSegmentFailed == 0 )
        {
          sprintf( pData->szMessageBuffer, "Processing complete. %ld entries have been processed", pData->iProcessed );
        }
        else
        {
          sprintf( pData->szMessageBuffer, "Processing complete. %ld entries have been processed, %ld entries could not be updated. See log file %s for details", 
            pData->iDocSegmentUpdates, pData->iDocSegmentFailed, pData->szLogFile );
          if ( pData->hfLog )
          {
            fclose( pData->hfLog );
            pData->hfLog = NULL;
          } /* endif */
        } /* endif */
        MessageBox( pData->hwndProgressWindow, pData->szMessageBuffer, szInfoTitle, MB_OK );
        ProofReadCloseMem( pData );
        ProofReadFreeDoc( pData );
        ProofReadFreeMarkup( pData );
        EqfRemoveObject( TWBFORCE, pData->hwndProgressWindow );
        break;
    }  /* endswitch */
  } /* endwhile */
} /* end of ProofReadProcess */

BOOL ProofReadCloseMem( PPROOFREADPROCESSDATA pData )
{
  BOOL fOK = TRUE;
  if ( pData->pMem )
  {
    MemoryFactory *pFactory = MemoryFactory::getInstance();
    pFactory->closeMemory( pData->pMem );
    pData->pMem = NULL;
    memset( pData->szCurMemory, 0, sizeof(pData->szCurMemory) );
  } /* endif */
  return( fOK );
}

BOOL ProofReadFreeDoc( PPROOFREADPROCESSDATA pData )
{
  BOOL fOK = TRUE;


  if ( pData->pTargetDoc )
  {
    if ( pData->pTargetDoc->flags.changed  )
    {
      USHORT usCPConv = 0;
      pData->pTargetDoc->docType = STARGET_DOC; // enable hist log processing
      SHORT sRc = EQFBFileWriteEx( pData->pTargetDoc->szDocName, pData->pTargetDoc, DOCSAVE_LOGTASK2, usCPConv );
      if ( sRc )
      {
        PSZ    pszParm = pData->szDocLongName;
        UtlError( ERROR_TA_SAVE_SEGFILE, MB_OK, 1, &pszParm, EQF_QUERY );
      }
      else
      {
        // set document touch date
        HPROP           hPropDocument;     // handle to document properties
        PPROPDOCUMENT   pPropDocument;     // pointer to document properties
        ULONG           ulErrorInfo;       // error indicator from PRHA


        if ( (hPropDocument = OpenProperties( pData->szDocObjName, NULL, PROP_ACCESS_READ, &ulErrorInfo)) != NULL)
        {
          pPropDocument = (PPROPDOCUMENT)MakePropPtrFromHnd( hPropDocument );
          if ( SetPropAccess( hPropDocument, PROP_ACCESS_WRITE) )
          {
            UtlTime( (PLONG)&pPropDocument->ulTouched );
            SaveProperties( hPropDocument, &ulErrorInfo );
            ResetPropAccess( hPropDocument, PROP_ACCESS_WRITE);
          } /* endif */
          CloseProperties( hPropDocument, PROP_FILE, &ulErrorInfo);
        } /* endif */
      } /* endif */
    } /* endif */
    EQFBFreeDoc( &(pData->pTargetDoc), EQFBFREEDOC_NOTAGTABLEFREE | EQFBFREEDOC_NOPROTECTFREE );
    pData->pTargetDoc = NULL;
  } /* endif */

  if ( pData->pSourceDoc != NULL ) EQFBFreeDoc( &(pData->pSourceDoc), EQFBFREEDOC_NOTAGTABLEFREE | EQFBFREEDOC_NOPROTECTFREE );
  pData->pSourceDoc = NULL;


  return( fOK );
}

BOOL ProofReadPrepareDocInfo( PPROOFREADPROCESSDATA pData )
{
  BOOL fOK = TRUE;

  // build folder object name and test folder existence if folder is not the current one
  int iNewFolNum = pData->pEntry->getFolderNumber();
  if ( pData->iCurFolderNum != iNewFolNum )
  {
    strcpy( pData->szFolLongName, pData->pList->getFolder(iNewFolNum).c_str() );
    BOOL fIsNew = !SubFolNameToObjectName( pData->szFolLongName, pData->szFolObjName );
    if ( fIsNew )
    {
      fOK = FALSE;
      PSZ pszParm = pData->szFolLongName;
      UtlError( ERROR_XLATE_FOLDER_NOT_EXIST, MB_CANCEL, 1, &pszParm, EQF_ERROR );
    }
    else
    {
      Utlstrccpy( pData->szFolShortName, UtlGetFnameFromPath(pData->szFolObjName), '.' );
    } /* endif */
  } /* endif */

  // build document object name and get document info if document is not the current one
  int iNewDocNum = pData->pEntry->getDocumentNumber();
  if ( fOK && (iNewDocNum != pData->iCurDocNum) )
  {
    BOOL fIsNew = FALSE;
    strcpy( pData->szDocLongName, pData->pList->getDocument( iNewDocNum ).c_str() );
    FolLongToShortDocName( pData->szFolObjName, pData->szDocLongName, pData->szDocShortName, &fIsNew );
    if ( fIsNew )
    {
      fOK = FALSE;
      sprintf( pData->szMessageBuffer, "Document %s does not exist in folder %s or cannot be accessed", pData->szDocLongName, pData->szFolLongName );
      MessageBox( pData->hwndProgressWindow, pData->szMessageBuffer, szErrorTitle, MB_OK );
    } /* endif */

    if ( fOK )
    {
      strcpy( pData->szDocObjName, pData->szFolObjName );
      strcat( pData->szDocObjName, BACKSLASH_STR );
      strcat( pData->szDocObjName, pData->szDocShortName );
    } /* endif */

    fOK = ( DocQueryInfo2( pData->szDocObjName, pData->szDocMemory, pData->szDocMarkup, pData->szDocSourceLang, pData->szDocTargetLang, pData->szDocLongName, pData->szDocAlias, NULL, TRUE ) == NO_ERROR );
  } /* endif */

  // get codepages and IDs for source and target language
  if ( fOK )
  {
    pData->ulSourceOemCP = GetLangCodePage( OEM_CP, pData->szDocSourceLang );
    pData->ulTargetOemCP = GetLangCodePage( OEM_CP, pData->szDocTargetLang );
    pData->ulTargetAnsiCP = GetLangCodePage( ANSI_CP, pData->szDocTargetLang );
    MorphGetLanguageID( pData->szDocTargetLang, &(pData->sTargetLangID) );
    MorphGetLanguageID( pData->szDocTargetLang , &(pData->sSourceLangID) );
  } /* endif */

  return( fOK );
}

BOOL ProofReadLoadDoc( PPROOFREADPROCESSDATA pData )
{
  BOOL fOK = TRUE;

  // free any previously loaded document
  if ( fOK ) ProofReadFreeDoc( pData );

  // ensure that the markup table is loaded
  if ( fOK ) fOK = ProofReadLoadMarkup( pData );

  // open document memory if not done yet
  if ( fOK ) fOK = ProofReadOpenMem( pData );

  // load the target document
  if ( fOK ) fOK = ProofReadSingleDoc( pData, TRUE );

  // load the source document
  if ( fOK ) fOK = ProofReadSingleDoc( pData, FALSE );

  return( fOK );
}

BOOL ProofReadOpenMem( PPROOFREADPROCESSDATA pData )
{
  BOOL fOK = TRUE;
  USHORT        usRc = NO_ERROR;
  PSZ           pTemp;                 // pointer to TM Name

  MemoryFactory *pFactory = MemoryFactory::getInstance();

  if ( strcmp( pData->szCurMemory, pData->szDocMemory ) != 0 )
  {
    ProofReadCloseMem( pData );
  } /* endif */

  if ( !pFactory->exists( NULL, pData->szDocMemory ) )
  {
    PSZ pszErrParm[2];
    pszErrParm[0] = pData->szDocMemory;
    pszErrParm[1] = pData->szDocLongName;
    UtlError( ERROR_TM_FOR_DOC_NOT_EXIST, MB_CANCEL, 2, pszErrParm, EQF_ERROR );
    fOK = FALSE;
  }
  else
  {
    int iRC = 0;
    pData->pMem = pFactory->openMemory( NULL, pData->szDocMemory, NONEXCLUSIVE, &iRC );

    if ( pData->pMem == NULL )
    {
      usRc = (USHORT)iRC;
      switch ( usRc )
      {
        case FILE_MIGHT_BE_CORRUPTED:
        case VERSION_MISMATCH:
        case CORRUPT_VERSION_MISMATCH:
          pTemp = pData->szDocMemory;
          UtlError( ITM_TM_NEEDS_ORGANIZE, MB_CANCEL, 1, &pTemp, EQF_ERROR );
          break;
        default:
          pFactory->showLastError( NULL, pData->szDocMemory, NULL, pData->hwndProgressWindow );
          break;
      } /* endswitch */
    } /* endif */
  } /* endif */

  return( fOK );
}

BOOL ProofReadUpdateDoc( PPROOFREADPROCESSDATA pData )
{
  BOOL fOK = TRUE;

  PTBSEGMENT pSourceSeg;                 
  PTBSEGMENT pTargetSeg;            

  ULONG ulSegNum = pData->pEntry->getSegmentNumber();

  pData->iDocSegmentUpdates++;

  // get source and target text
  PSZ_W pszSourceText = (PSZ_W)pData->pEntry->getSource();
  PSZ_W pszTargetText = (PSZ_W)pData->pEntry->getTarget();
  PSZ_W pszModTargetText = (PSZ_W)pData->pEntry->getNewTarget();
  if ( pszModTargetText[0] == 0 ) pszModTargetText = (PSZ_W)pData->pEntry->getModTarget();

  // get segment data
  pSourceSeg = EQFBGetSegW( pData->pSourceDoc, ulSegNum );
  pTargetSeg = EQFBGetSegW( pData->pTargetDoc, ulSegNum );

  // do some checking
  int iMBCode = IDYES;
  if ( (pSourceSeg == NULL) || (pTargetSeg == NULL) )
  {
    sprintf( pData->szMessageBuffer, "Could not access segment %lu of document %s in folder %s.\n\nDo you want to continue with the next entry?", ulSegNum, pData->szDocLongName, pData->szFolLongName );
    iMBCode = MessageBox( pData->hwndProgressWindow, pData->szMessageBuffer, szErrorTitle, MB_YESNO );
    ProofReadWriteEntryToLog( pData, NULL, L"Could not access the segment in the document", TRUE );
    pData->iDocSegmentFailed++;
  }
  else if ( ProofReadstrcmp( pszSourceText, pSourceSeg->pDataW ) != 0 )
  {
    sprintf( pData->szMessageBuffer, "Source text of segment %lu in document %s in folder %s does not match the entry in the validation document.\n\nDo you want to continue with the next entry?", ulSegNum, pData->szDocLongName, pData->szFolLongName );
    iMBCode = MessageBox( pData->hwndProgressWindow, pData->szMessageBuffer, szErrorTitle, MB_YESNO );
    ProofReadWriteEntryToLog( pData, pSourceSeg->pDataW, L"Segment source in document does not match the entry in the validation document", TRUE );
    pData->iDocSegmentFailed++;
  } 
  //else if ( ProofReadstrcmp( pszTargetText, pTargetSeg->pDataW ) != 0 )
  //{
  //  sprintf( pData->szMessageBuffer, "Target text of segment %lu in document %s in folder %s does not match the entry in the validation document.\n\nDo you want to continue with the next entry?", ulSegNum, pData->szDocLongName, pData->szFolLongName );
  //  iMBCode = MessageBox( pData->hwndProgressWindow, pData->szMessageBuffer, szErrorTitle, MB_YESNO );
  //  ProofReadWriteEntryToLog( pData, pTargetSeg->pDataW, L"Segment target in document does not match the entry in the validation document", FALSE );
  //  pData->iDocSegmentFailed++;
  //} 
  else if ( wcslen( pszTargetText ) >= MAX_SEGMENT_SIZE )
  {
    sprintf( pData->szMessageBuffer, "The new text of segment %lu in document %s in folder %s is too large, segment could not be changed.\n\nDo you want to continue with the next entry?", ulSegNum, pData->szDocLongName, pData->szFolLongName );
    iMBCode = MessageBox( pData->hwndProgressWindow, pData->szMessageBuffer, szErrorTitle, MB_YESNO );
    ProofReadWriteEntryToLog( pData, NULL, L"The new segment text is too large", TRUE );
    pData->iDocSegmentFailed++;
  } /* endif */
  if ( iMBCode != IDYES )
  {
    fOK = FALSE;
  } 

  // adjust white space in target segment
  if ( fOK )
  {
    fOK = ProofReadAdjustWhiteSpaceInTarget( pData, pTargetSeg->pDataW, pszTargetText, pszModTargetText, pData->szAdjustedTarget );
  } /* endif */

  // update the segment target text
  if ( fOK )
  {
    ULONG ulDataLen = wcslen( pTargetSeg->pDataW ) + 1;
    ULONG ulNewLen = wcslen( pData->szAdjustedTarget ) + 1;

    // Allocate segment data buffer for changed segment and change segment data 
    PSZ_W pNewData;

    fOK = UtlAlloc( (PVOID *)&pNewData, 0L, ulNewLen * sizeof(CHAR_W), ERROR_STORAGE );
    if ( fOK )
    {
      wcscpy( pNewData, pData->szAdjustedTarget );
      UtlAlloc( (PVOID *)&pTargetSeg->pDataW, 0L, 0L, NOMSG );
      pTargetSeg->pDataW = pNewData;
      pTargetSeg->usLength = (USHORT)ulNewLen;
      pTargetSeg->SegFlags.Changed = TRUE; 
      pData->pTargetDoc->flags.changed = TRUE;
      pData->iDocSegmentChanged++;

      // re-compute target words
      ULONG ulTgtWords = 0;
      ULONG ulTgtMarkUp = 0;
      EQFBWordCntPerSeg( pData->pTagTable, (PTOKENENTRY)pData->pTargetDoc->pTokBuf, pTargetSeg->pDataW, pData->sTargetLangID, &ulTgtWords, &ulTgtMarkUp, pData->ulTargetOemCP);
      pTargetSeg->usTgtWords = (USHORT) ulTgtWords;
    } /* endif */
  } /* endif */

  return( fOK );
}

BOOL ProofReadUpdateMem( PPROOFREADPROCESSDATA pData )
{
  BOOL fOK = TRUE;
  OtmProposal Prop;

  if ( !pData->fProcessMem ) return( TRUE );

  Prop.setSource( (wchar_t *)pData->pEntry->getSource() );
  const wchar_t *pszTarget = pData->pEntry->getNewTarget();
  if ( *pszTarget == 0 ) pszTarget = pData->pEntry->getModTarget();
  Prop.setTarget( (wchar_t *)pszTarget  );
  Prop.setDocShortName( pData->szDocShortName );
  Prop.setDocName( pData->szDocLongName );
  Prop.setSegmentNum( pData->pEntry->getSegmentNumber() );
  Prop.setMarkup( pData->szDocMarkup );
  Prop.setSourceLanguage( pData->szDocSourceLang );
  Prop.setTargetLanguage( pData->szDocTargetLang );

  // handle any segment context
  if ( pData->pTargetDoc->pfnGetSegContext )
  {
    ULONG ulSegNum = pData->pEntry->getSegmentNumber();
    PTBSEGMENT pPrevSeg = EQFBGetSegW( pData->pSourceDoc, ulSegNum - 1 );
    PTBSEGMENT pSeg = EQFBGetSegW( pData->pSourceDoc, ulSegNum );
    PTBSEGMENT pNextSeg = EQFBGetSegW( pData->pSourceDoc, ulSegNum + 1 );
    PSZ_W pszPrevSegData = ( pPrevSeg ) ? pPrevSeg->pDataW : NULL;
    PSZ_W pszNextSegData = ( pNextSeg ) ? pNextSeg->pDataW : NULL;

    (pData->pTargetDoc->pfnGetSegContext)( pSeg->pDataW, pszPrevSegData, pszNextSegData, pData->szSegmentContext, (LONG)pData->pSourceDoc, ulSegNum );

    Prop.setContext( pData->szSegmentContext );
  } /* endif */

  if ( pData->pMem != NULL ) pData->pMem->putProposal( Prop );

  return( fOK );
}

BOOL ProofReadFreeMarkup( PPROOFREADPROCESSDATA pData )
{
  BOOL fOK = TRUE;

  if ( pData->pTagTable ) TAFreeTagTable( pData->pTagTable );
  pData->pTagTable = NULL;
  memset( pData->szCurMarkup, 0, sizeof(pData->szCurMarkup) );

  return( fOK );
}

BOOL ProofReadLoadMarkup( PPROOFREADPROCESSDATA pData )
{
  BOOL fOK = TRUE;

  if ( strcmp( pData->szCurMarkup, pData->szDocMarkup ) != 0 )
  {
    // load QF tag table if not done yet
    if ( pData->pQFTagTable == NULL )
    {
      SHORT  sRc;
      sRc = TALoadTagTable( DEFAULT_QFTAG_TABLE, &pData->pQFTagTable, TRUE, TRUE );       
      fOK = (sRc == NO_ERROR);
    } /* endif */

    // load document markup table
    if ( fOK )
    {
      SHORT  sRc;

      sRc = TALoadTagTableExHwnd( pData->szDocMarkup, &pData->pTagTable, FALSE, TALOADPROTTABLEFUNC | TALOADUSEREXIT | TALOADGETSEGCONTEXTFUNC, TRUE, pData->hwndProgressWindow ); 
      fOK = (sRc == NO_ERROR);
      if ( fOK ) strcpy( pData->szCurMarkup, pData->szDocMarkup );
    } /* endif */
  }
  return( fOK );
}

BOOL ProofReadSingleDoc( PPROOFREADPROCESSDATA pData, BOOL fTargetDoc )
{
  BOOL fOK = TRUE;

  PTBDOCUMENT *ppDoc = ( fTargetDoc ) ? &(pData->pTargetDoc) : &(pData->pSourceDoc);

  // allocate TBDOCUMENT structure
  if ( fOK ) fOK = UtlAllocHwnd( (PVOID *)ppDoc, 0L, (LONG)sizeof(TBDOCUMENT), ERROR_STORAGE, pData->hwndProgressWindow );

  // fill TBDOCUMENT structure
  if ( fOK )
  {
    (*ppDoc)->pDocTagTable = pData->pTagTable;
    (*ppDoc)->pQFTagTable = pData->pQFTagTable;
    strcpy( (*ppDoc)->szDocLongName, pData->szDocLongName );
    (*ppDoc)->ulOemCodePage = pData->ulTargetOemCP;
    (*ppDoc)->ulAnsiCodePage = pData->ulTargetAnsiCP;
    (*ppDoc)->docType = ( fTargetDoc ) ? STARGET_DOC : SSOURCE_DOC;
  } /* endif */

  // load the document
  if ( fOK )
  {
    CHAR   szDocFullPath[MAX_EQF_PATH];
    SHORT  sRc;

    // build fully qualified name of segmented document file
    UtlMakeEQFPath( szDocFullPath, pData->szFolObjName[0], ( fTargetDoc ) ? DIRSEGTARGETDOC_PATH : DIRSEGSOURCEDOC_PATH, UtlGetFnameFromPath( pData->szFolObjName) );
    strcat( szDocFullPath, BACKSLASH_STR );
    strcat( szDocFullPath, pData->szDocShortName );

    // check if segmented file exists, issue warning if file does not exist
    if ( UtlFileExist( szDocFullPath ) )
    {
      PSZ    pszErrParm = pData->szDocLongName;
      strcpy( (*ppDoc)->szDocName, szDocFullPath );
      sRc = EQFBFileRead( szDocFullPath, *ppDoc );
      if ( sRc != NO_ERROR )
      {
        UtlErrorHwnd( ERROR_FOLFIND_DOCLOAD, MB_OK, 1, &pszErrParm, EQF_ERROR, pData->hwndProgressWindow );
        fOK = FALSE;
      } /* endif */
    }
    else
    {
      PSZ    pszErrParm = pData->szDocLongName;
      UtlErrorHwnd( ERROR_FS_DOC_NOT_ANALYZED, MB_OK, 1, &pszErrParm, EQF_ERROR, pData->hwndProgressWindow );
      fOK = FALSE;
    } /* endif */
  } /* endif */
  return( fOK );
}


void ProofReadShowProgress( PPROOFREADPROCESSDATA pData )
{
  SHORT sCompletionRate = (SHORT)( pData->iProcessed * 100 / pData->iToBeProcessed );

  sprintf( pData->szMessageBuffer, "%ld entries of %ld processed", pData->iProcessed, pData->iToBeProcessed );
  WinSendMsg( pData->hwndProgressWindow, WM_EQF_UPDATESLIDER, MP1FROMSHORT(sCompletionRate), MP2FROMP(pData->szMessageBuffer) );
}


BOOL ProofReadWriteEntryToLog( PPROOFREADPROCESSDATA pData, PSZ_W pszSegSource, PSZ_W pszError, BOOL fSource )
{
  // open log file if not done yet
  if ( pData->hfLog == NULL )
  {
    pData->hfLog = fopen( pData->szLogFile, "wb" );
    if ( pData->hfLog != NULL )
    {
      fwrite( UNICODEFILEPREFIX, 1, 2, pData->hfLog );
    } /* endif */
  } /* endif */

  // write entry
  ULONG ulSegNum = pData->pEntry->getSegmentNumber();
  PSZ_W pszSourceText = (PSZ_W)pData->pEntry->getSource();
  PSZ_W pszTargetText = (PSZ_W)pData->pEntry->getNewTarget();
  PSZ_W pszModTargetText = (PSZ_W)pData->pEntry->getNewTarget();
  if ( pszModTargetText[0] == 0 ) pszModTargetText = (PSZ_W)pData->pEntry->getModTarget();

  fwprintf( pData->hfLog, L"Error when processing entry %ld\r\n", pData->iProcessed );
  fwprintf( pData->hfLog, L"  Folder     : %S\r\n", pData->szFolLongName );
  fwprintf( pData->hfLog, L"  Document   : %S\r\n", pData->szDocLongName );
  fwprintf( pData->hfLog, L"  Segment    : %lu\r\n", ulSegNum );
  fwprintf( pData->hfLog, L"  Error      : %s\r\n", pszError );

  if ( fSource )
  {
    if ( pszSegSource != NULL )
    {
      fwprintf( pData->hfLog, L"--- Segment source from OpenTM2 document ---\r\n%s\r\n", pszSegSource );
    } /* endif */
    fwprintf( pData->hfLog, L"--- Segment source in validation document ---\r\n%s\r\n", pszSourceText );
    fwprintf( pData->hfLog, L"--- New target from validation document ---\r\n%s\r\n----------------------------\r\n\r\n", pszModTargetText );
  }
  else
  {
    if ( pszSegSource != NULL )
    {
      fwprintf( pData->hfLog, L"--- Segment target from OpenTM2 document ---\r\n%s\r\n", pszSegSource );
    } /* endif */
    fwprintf( pData->hfLog, L"--- Segment target in validation document ---\r\n%s\r\n", pszTargetText );
    fwprintf( pData->hfLog, L"--- New target from validation document ---\r\n%s\r\n----------------------------\r\n\r\n", pszModTargetText );
  } /* endif */

  return( TRUE );
}

// compare two strings but ignore whitespace differences
SHORT ProofReadstrcmp( PSZ_W  pSrc, PSZ_W  pTgt )
{
  SHORT sResult = 0;
  CHAR_W s, t;
  BOOL fContinue = TRUE;
  // loop thru source and target string and compare the values, but
  // ignore any changes in white spaces..
  while ( fContinue && !sResult )
  {
    s = *pSrc++;
    t = *pTgt++;
    if ( s != t )
    {
      if ( iswspace(s) && iswspace(t) )
      {
        // fine - both strings contain white space characters

        // skip any following whitespace
        while (iswspace( *pSrc ) ) pSrc++;
        while (iswspace( *pTgt ) ) pTgt++;
      }
      else
      {
        sResult = (s>t)? 1 : -1;
      } /* endif */
    }
    else if ( (s == 0) && (t == 0) )
    {
      // both strings end here
      fContinue = FALSE;
    }
    else
    {
      // we are dealing with Equal characters

      // for whitespaces: skip any folloing whitespace
      if ( iswspace(s) && iswspace(t) )
      {
        while (iswspace( *pSrc ) ) pSrc++;
        while (iswspace( *pTgt ) ) pTgt++;
      } /* endif */

    } /* endif */
  } /* endwhile */
  return sResult;
}

// copy data from original target until given position in validation document target has been reached
BOOL ProofReadCopyUpTo( PSZ_W *ppszOrgTarget, PSZ_W *ppszValDocTarget, PSZ_W *ppszNewTarget, PSZ_W pszStopPos ) 
{
  PSZ_W pszOrgTarget = *ppszOrgTarget;
  PSZ_W pszValDocTarget = *ppszValDocTarget;
  PSZ_W pszNewTarget = *ppszNewTarget;

  while ( pszValDocTarget < pszStopPos )
  {

  } /* endwhile */

  *ppszOrgTarget = pszOrgTarget;
  *ppszOrgTarget = pszValDocTarget;
  *ppszNewTarget = pszNewTarget;

  return( TRUE );
}

// adjust white space in the modified target string to macth the white space in the original target segment
BOOL ProofReadAdjustWhiteSpaceInTarget( PPROOFREADPROCESSDATA pData, PSZ_W pszOrgTarget, PSZ_W pszValDocTarget, PSZ_W pszValDocModTarget, PSZ_W pszNewTarget ) 
{
  BOOL fOK = TRUE;
  PFUZZYTOK pFuzzyTgt = NULL;
  PFUZZYTOK pFuzzyTok = NULL;
  PSZ_W pszOutPos = pszNewTarget;

  // find differences between target and modified target from the validation document
  if ( fOK )
  {
    fOK = EQFBFindDiffEx( pData->pTagTable, pData->bDiffInputBuffer, pData->bDiffTokenBuffer, pszOrgTarget, pszValDocModTarget, pData->sTargetLangID, 
      (PVOID *)&pFuzzyTok, (PVOID *)&pFuzzyTgt, pData->ulTargetOemCP );
    //fOK = EQFBFindDiffEx( pData->pTagTable, pData->bDiffInputBuffer, pData->bDiffTokenBuffer, pszValDocTarget, pszValDocModTarget, pData->sTargetLangID, 
    //  (PVOID *)&pFuzzyTok, (PVOID *)&pFuzzyTgt, pData->ulTargetOemCP );
  } /* endif */

  // combine original target and imported modifications
  PFUZZYTOK pToken = pFuzzyTok;
  PFUZZYTOK pOldToken = pFuzzyTgt;
  while ( pToken->ulHash ) 
  {
    // handle tokens
    if ( pToken->sType == MARK_EQUAL )
    {
      // copy orginal target
      int iLen = (pOldToken->usStop - pOldToken->usStart) + 1;
      wcsncpy( pszOutPos, pOldToken->pData, iLen );
      pszOutPos += iLen;
    }
    else if ( pToken->sType == MARK_INSERTED ) 
    {
      // copy new text token
      int iLen = (pToken->usStop - pToken->usStart) + 1;
      wcsncpy( pszOutPos, pToken->pData, iLen );
      pszOutPos += iLen;
    }
    else if ( pToken->sType == MARK_MODIFIED ) 
    {
      // copy new text token but use trailing white space from old target

      // find whitespace at end of changed text
      int iLen = (pToken->usStop - pToken->usStart) + 1;
      while ( iLen && iswspace( pToken->pData[iLen-1] ) ) iLen--;

      // find whitespace at end of original text
      USHORT usWhiteSpacePos = pOldToken->usStop;
      while ( (usWhiteSpacePos > pOldToken->usStart) && iswspace( pszOrgTarget[usWhiteSpacePos] ) ) usWhiteSpacePos--;
      int iWhiteSpaceLen = (pOldToken->usStop - usWhiteSpacePos);
      PSZ_W pszOldWhiteSpace = pszOrgTarget + (usWhiteSpacePos + 1);


      // copy changed text w/o tailing whitespace
      if ( iLen )
      {
        wcsncpy( pszOutPos, pToken->pData, iLen );
        pszOutPos += iLen;
      } /* endif */

      // copy original whitespace
      if ( iWhiteSpaceLen )
      {
        wcsncpy( pszOutPos, pszOldWhiteSpace, iWhiteSpaceLen );
        pszOutPos += iWhiteSpaceLen;
      } /* endif */
    }
    else if ( pToken->sType == MARK_DELETED ) 
    {
      // ignore deleted text
    } /* endif */
    pToken++;
    pOldToken++;
  } /* endwhile */

  *pszOutPos = 0;

  // cleanup
  if ( pFuzzyTgt ) UtlAlloc( (PVOID *)&pFuzzyTgt, 0L, 0L, NOMSG );
  if ( pFuzzyTok ) UtlAlloc( (PVOID *)&pFuzzyTok, 0L, 0L, NOMSG );

  return( fOK );
}