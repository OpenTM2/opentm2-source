/*! \file
	Description: EQF Machine Translation List Handler

	Copyright Notice:

	Copyright (C) 1990-2012, International Business Machines
	Corporation and others. All rights reserved
*/

// use only if we are dealing with MT
#define INCL_EQF_FOLDER           // folder list and document list functions
#define INCL_EQF_ANALYSIS
#define INCL_EQF_TM               // general Transl. Memory functions
#include "eqf.h"                  // General .H for EQF
#include "eqfcolw.id"             // column width IDs
#include "eqfmt.id"               // ids for MT list window handler
#define EQFMT00_C
#include "eqfmt.h"                // ids for MT list window handler
#include "eqfmt00.h"              // prototypes for MT list window handler
// delete later on
#include "eqffll.id"              // IDs for folder lists
#include "eqffll00.h"             // Folder List Handler defines
#include "eqfdde.h"               // batch mode definitions
#include "eqffol00.h"             // Document List Handler defines
#include <process.h>              /* _beginthread, _endthread */
#include <time.h>

#define  TIMER_VAL    5000L      // every five   seconds
#define  TIMER_ID     6          // my cookie

#define  MTJOBLISTOBJECT "MTJobList"

#include "eqfrpt.id"              // private id's for report

static CHAR ColHdr[8][80] =             // Buffer for column header texts
{
  "Job Name",
  "Folder",
  "Status",
  "Creation Date",
  "Source Language",
  "Target Language",
  "free",
  "free"
};
static CLBCOLDATA ColTable[] =
{ { "",               1,             TEXT_DATA,      DT_LEFT          },   // object name
  { "",               1,             TEXT_DATA,      DT_LEFT          },   // internal status
  { ColHdr[0], CLB_MAX_LANG_LENGTH,  AUTOWIDTHTEXT_DATA,  DT_LEFT     },   // jobname
  { ColHdr[1], CLB_MAX_LANG_LENGTH,  AUTOWIDTHTEXT_DATA,  DT_LEFT     },   // folder
  { ColHdr[2], CLB_MAX_MT_STATUS,    AUTOWIDTHTEXT_DATA,  DT_LEFT     },   // status
  { ColHdr[3], CLB_MAX_DATE,         DATETIME_DATA,       DT_LEFT     },   // date
  { ColHdr[4], CLB_MAX_LANG_LENGTH,  AUTOWIDTHTEXT_DATA,  DT_LEFT     },   // Source lang
  { ColHdr[5], CLB_MAX_LANG_LENGTH,  AUTOWIDTHTEXT_DATA,  DT_LEFT     },   // Target lang
  { NULL, 			  0,    (CLBDATATYPE)0,      0        } };

static SHORT sLastUsedView[] = { MT_JOBNAME_IND, MT_STATUS_IND, CLBLISTEND };
static SHORT sDefaultView[]  = { MT_JOBNAME_IND, MT_FOLDER_IND, MT_STATUS_IND, MT_SOURCELANG_IND, MT_TARGETLANG_IND, CLBLISTEND };
static SHORT sNameView[]     = { MT_JOBNAME_IND, CLBLISTEND };
static SHORT sDetailsView[]  = { MT_JOBNAME_IND, MT_FOLDER_IND, MT_STATUS_IND, MT_SOURCELANG_IND, MT_TARGETLANG_IND, CLBLISTEND };
static SHORT sSortCriteria[] = { MT_JOBNAME_IND, CLBLISTEND };

static CLBCTLDATA MTCLBData =
{  sizeof(CLBCTLDATA),                 // size of control structure
   8,                                  // we have 6 data columns
   1,                                  // two character space between columns
   SYSCLR_WINDOWSTATICTEXT,            // paint title in color of static text
   SYSCLR_WINDOW,                      // background is normal window background
   SYSCLR_WINDOWTEXT,                  // paint item in color of window text
   SYSCLR_WINDOW,                      // background is normal window background
   '\x15',                             // use X15 character as data seperator
   sLastUsedView,                      // set current (= last used) view list
   sDefaultView,                       // set default view list
   sDetailsView,                       // set user set details view list
   sNameView,                          // set view list for 'name' view option
   sSortCriteria,                      // set sort criteria list
   ColTable };                         // set address of column definition table

// global flag to avoid nested handling of timer messages
static BOOL fHandlingTimerMessage = FALSE;

USHORT LoadMTJobNames( PSZ folder, HWND hlbox, BOOL flg, PSZ pszBuffer );
VOID UpdateItemText ( PLISTCOMMAREA pCommArea, PVOID hProp , SHORT sIndex,
                      PSZ pName );
static VOID HandleTimer ( HWND hwnd, PLISTCOMMAREA pCommArea );
static VOID MTMergeCompl ( PLISTCOMMAREA pCommArea, PSZ  pszDocObjName );
static VOID MTMergeStatus( PLISTCOMMAREA pCommArea, USHORT s, PSZ pszDocObjName );

static void MTCleanup ( PSZ pszDocObjName, USHORT usTaskID, PSZ pszMTError );

BOOL MTBuildItemText
(
  PSZ         pszObjName,
  PSZ         pszBuffer
);
BOOL MTChangeJobStatus
(
  PSZ     pszObjName,
  LONG    lNewStatus,
  PSZ     pszNewStatus,
  LONG    lNumOfWords
);
BOOL MTEncryptPassword
(
  PSZ         pszPassword,             // in: password or encrypted string
  PSZ         pszBuffer,               // out: encrypted string or password
  int         iBufLen,                 // size of output buffer
  BOOL        fEncrypt                 // TRUE = encrypt, FALSE = decrypt
);

static HMODULE hmodMTDll = NULLHANDLE; // MT dll active
static CHAR szMTDll[13] = "";          // name of MT dll

typedef enum THREADSTATE
{
  ThreadState_NotStarted,
  ThreadState_Stopped,
  ThreadState_Ready,
  ThreadState_Busy,
  ThreadState_Done
} THREADSTATE;

typedef enum THREADTASK
{
  ThreadTask_None,
  ThreadTask_SendFile,
  ThreadTask_ReceiveFile,
  ThreadTask_GetStatus,
  ThreadTask_DeleteJob
} THREADTASK;


typedef struct _MTTHREADDATA
{
  MTPASS      MTPass;                            // MT pass structure
  CHAR        szMTEngineDLL[MAX_MT_NAME];        // DLL containing MT engine IF
  CHAR        szDocName[MAX_LONGFILESPEC];       // buffer for segment file
  CHAR        szMTProfile[MAX_LONGFILESPEC];     // buffer for MT profile name (unused)
  CHAR        szMTError[400];                    // buffer error from MT engine
  CHAR        szJobStatus[MAX_STATUS_LENGTH];    // buffer for server status mesages
  USHORT      usComplete;                        // translation is complete flag
  THREADTASK  Task;                              // current task to be processed by thread
  THREADSTATE State;                             // current state of thread
  BOOL        fKill;                             // thread kill flag
  BOOL        fOK;                               // OK flag of performed task
  CHAR        szObject[MAX_LONGFILESPEC];        // object name of task object
} MTTHREADDATA, *PMTTHREADDATA;


static MTTHREADDATA ThreadData;

BOOL MTStartThread();
void MTProcessThread( void *pvData );


//------------------------------------------------------------------------------
// Function name: MakeTempName
//------------------------------------------------------------------------------
char *MakeTempName( char *Original , char *Temporary )
{
  static int fileNr = 0;
  char *p;

  strcpy( Temporary , Original );
  p = strrchr (Temporary, '\\');
  if (p) p++;
  else p = Temporary;

  do {
    sprintf( p, "%d", fileNr++ );
  } while ( access( Temporary , 0 ) == 0 );

  strcat(Temporary,".tmp");

  return Temporary;
}

//------------------------------------------------------------------------------
// Function name: MTListHandlerCallBack
//------------------------------------------------------------------------------
MRESULT MTListHandlerCallBack
(
  PHANDLERCOMMAREA pCommArea,
  HWND             hwnd,
  WINMSG           message,
  WPARAM           mp1,
  LPARAM           mp2
)
{
  MRESULT          mResult = MRFROMSHORT(FALSE);

  switch ( message )
  {
    /******************************************************************/
    /* WM_CREATE: fill variables of communication area                */
    /******************************************************************/
    case WM_CREATE :
      pCommArea->pfnCallBack          = MTListCallBack;
      strcpy( pCommArea->szHandlerName, MTLISTHANDLER );
      pCommArea->sBaseClass           = clsMTLIST;
      pCommArea->sListWindowID        = ID_MTLIST_WINDOW;
      pCommArea->sListboxID           = PID_MTLIST_LB;
      pCommArea->asNotifyClassList[0] = clsMTLIST;
      pCommArea->asNotifyClassList[1] = 0;       // end of list

      // start our processing thread
      MTStartThread();

      // open MT job list window if there are active MT jobs available and
      // correct status of incomplete MTJobs
      {
        LONGFILEFIND stResultBuf;             // DOS file find struct
        HDIR        hDirHandle = HDIR_CREATE;// DosFind routine handle
        USHORT      usRC;                    // return code of called functions
        BOOL        fActiveMTJobs = FALSE;   // TRUE = there are active MT jobs

        // setup MT job path (create directory if necessary
        UtlMakeEQFPath( pCommArea->szBuffer, NULC, DIRSEGMT_PATH, NULL );
        if ( !UtlDirExist( pCommArea->szBuffer ) )
        {
          UtlMkDir( pCommArea->szBuffer, 0L, FALSE );
        } /* endif */
        strcat( pCommArea->szBuffer, "\\*" );
        strcat( pCommArea->szBuffer, EXT_OF_MTPROP );

        // loop over all MT jobs
        usRC = UtlFindFirstLong( pCommArea->szBuffer, &hDirHandle, FILE_NORMAL, &stResultBuf, FALSE );
        while ( usRC == NO_ERROR )
        {
          // process MT job properties
          PMTJOBPROP pProp = NULL;
          ULONG ulBytes = 0;

          UtlSplitFnameFromPath( pCommArea->szBuffer );
          strcat( pCommArea->szBuffer, BACKSLASH_STR );
          strcat( pCommArea->szBuffer, stResultBuf.achName );
          if ( UtlLoadFileL( pCommArea->szBuffer, (PVOID *)&pProp, &ulBytes, FALSE, FALSE ) )
          {
            switch( pProp->lStatus )
            {
              case MTJOB_Preparing:
                pProp->lStatus = MTJOB_Failed;
                strcpy( pProp->szStatus, "Analysis failed" );
                UtlWriteFile( pCommArea->szBuffer, sizeof(MTJOBPROP), pProp, FALSE );
                break;
              case MTJOB_Sending:
                pProp->lStatus = MTJOB_Failed;
                strcpy( pProp->szStatus, "Send failed" );
                UtlWriteFile( pCommArea->szBuffer, sizeof(MTJOBPROP), pProp, FALSE );
                break;
              case MTJOB_ServerWait:
                fActiveMTJobs = TRUE;
                break;
              case MTJOB_Receiving:
                pProp->lStatus = MTJOB_Failed;
                strcpy( pProp->szStatus, "Receive failed" );
                UtlWriteFile( pCommArea->szBuffer, sizeof(MTJOBPROP), pProp, FALSE );
                break;
              case MTJOB_Merging:
                pProp->lStatus = MTJOB_Failed;
                strcpy( pProp->szStatus, "Merge failed" );
                UtlWriteFile( pCommArea->szBuffer, sizeof(MTJOBPROP), pProp, FALSE );
                break;
              case MTJOB_Received:
              case MTJOB_Translated:
              case MTJOB_Merged:
              case MTJOB_Completed:
              case MTJOB_Failed:
              case MTJOB_Prepared:
              case MTJOB_Sended:
              default:
                break;
            } /* endswitch */
            UtlAlloc( (PVOID *)&pProp, 0L, 0L, NOMSG );
          } /* endif */

          // continue with next MT job property file
          usRC = UtlFindNextLong( hDirHandle, &stResultBuf, FALSE );
        } /* endif */
        if ( hDirHandle != HDIR_CREATE ) UtlFindCloseLong( hDirHandle, FALSE );

        // activate MT job list if there are active MT jobs
        if ( fActiveMTJobs )
        {
          PPROPSYSTEM pSysp;                   // EQF system properties

          pSysp = (PPROPSYSTEM)MakePropPtrFromHnd( EqfQuerySystemPropHnd());
          strcpy( pSysp->RestartMTList, X15_STR );
          strcat( pSysp->RestartMTList, MTJOBLISTOBJECT );
        } /* endif */
      }
      break;

    case WM_EQF_OPEN:
      {
        HWND       hwndObj;
        if( (hwndObj = EqfQueryObject( MTJOBLISTOBJECT, clsMTLIST, 0)) != NULLHANDLE )
        {
          ActivateMDIChild( hwndObj );
          mResult = MRFROMSHORT( TRUE );
        }
        else
        {
          mResult = WinSendMsg( hwnd, WM_EQF_CREATELISTWINDOW, mp1, MTJOBLISTOBJECT );
        } /* endif */
      }
      break;

    case WM_EQF_CREATE:
      UtlError( ERROR_CREATE_FOLDERLIST, MB_CANCEL, 0, (PSZ *)NULP, EQF_ERROR );
      mResult = MRFROMSHORT( TRUE );
      break;

    case WM_EQF_INSERTNAMES:
      mResult = WinSendMsg( EqfQueryActiveFolderlistHwnd(), message, mp1, mp2);
      break;

    case WM_EQF_COMMAND:
      {
        HWND hwndList = EqfQueryObject( MTJOBLISTOBJECT, clsMTLIST, 0);
        if ( hwndList ) WinPostMsg( hwndList, WM_EQF_COMMAND, mp1, mp2 );
      }
      break;

    case WM_DESTROY:
      // wait until MT thread closes down
      if ( ThreadData.State != ThreadState_NotStarted )
      {
        int iWaits = 100;
        ThreadData.fKill = TRUE;
        while ( iWaits && (ThreadData.State != ThreadState_Stopped) )
        {
          Sleep( 100 );
          iWaits--;
        } /* endwhile */
      } /* endif */

      if ( hmodMTDll )
      {
        USHORT usRc = DosGetProcAddr( hmodMTDll,"MT_TERMINATE", (PFN*) (&pfnMTTerminate));
        if (usRc == NO_ERROR)
        {
          pfnMTTerminate();
        } /* endif */
        DosFreeModule( hmodMTDll );
        hmodMTDll = NULL;
      } /* endif */
      break;


    /******************************************************************/
    /* Process notification messages WM_EQFN_PROPERTIESCHANGED,       */
    /* WM_EQFN_DELETED and WM_EQFN_CREATED:                           */
    /*   update internal filter list listbox and                      */
    /*   broadcast message to all filter controls                     */
    /******************************************************************/
    case WM_EQFN_PROPERTIESCHANGED:
    case WM_EQFN_DELETED:
    case WM_EQFN_CREATED:
      if ( SHORT1FROMMP1(mp1) == clsMTJOB )
      {
         BOOL fHereIAm = TRUE;
         fHereIAm;
      } /* endif */
      break;

    // handle task requests
    case WM_EQF_PROCESSTASK:
      switch ( SHORT1FROMMP1(mp1) )
      {
        case PID_FILE_MI_NEW:
          // create a new MT job property files
          {
            PANAMTDATA pAnaData = (PANAMTDATA)PVOIDFROMMP2( mp2 );
            BOOL fOK = TRUE;
            HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
            DIALOGBOX( pAnaData->hwndParent, EQFMTPROPERTYDLGPROC, hResMod, ID_MTPROP_DLG, mp2, fOK );
            if ( fOK )
            {
              EqfSend2Handler( MTLISTHANDLER, WM_EQF_OPEN, MP1FROMHWND(NULL), MP2FROMP(NULL)) ;
              EqfSend2Handler( MTLISTHANDLER, WM_EQFN_CREATED, MP1FROMSHORT( clsMTJOB  ),
                               MP2FROMP( pAnaData->szMTJobObject ));
            } /* endif */
            mResult = (MRESULT)fOK;
          }
          break;

        case PID_FILE_MI_MOVE:
          // change state of a MT job
          {
            PMTSTATUSCHANGE pStatusChange = (PMTSTATUSCHANGE)PVOIDFROMMP2( mp2 );

            MTChangeJobStatus( pStatusChange->pszMTJobObject, pStatusChange->iStatus,
                               pStatusChange->szStatus, pStatusChange->lNumOfWords );
          }
          break;

        case PID_FILE_MI_DELETE:
          break;

        case PID_FILE_MI_ANALYZE:
          break;
      } /* endswitch */

    default:
      break;
  } /* endswitch */
  return( mResult );
} /* end of function MTListHandlerCallBack */



//------------------------------------------------------------------------------
// Function name: FillMTPass
//------------------------------------------------------------------------------
BOOL FillMTPass
(
   PMTPASS    pMTPass,
   PMTJOBPROP pMTJobProp,
   PSZ        pszMTJobProp
)
{
  BOOL fOk = TRUE;

  memset( pMTPass, 0, sizeof(MTPASS) );
  if ( pMTJobProp->fUseFTP )
  {
    strcpy( pMTPass->chServer,        pMTJobProp->szFTPServer );
    strcpy( pMTPass->chQueueMgr,      pMTJobProp->szFTPUserID );
    memset( pMTPass->chServerInQueue, 0, sizeof(pMTPass->chServerInQueue) );
    strcpy( pMTPass->chServerOutQueue, pMTJobProp->szServerOutQueue );
    strcpy( pMTPass->chMESSAGEID, pMTJobProp->szMessageID );
    MTEncryptPassword( pMTJobProp->szFTPPassword, pMTPass->chTEMP3, sizeof(pMTPass->chTEMP3),
                       FALSE );
    strcpy( pMTPass->chPORT,          pMTJobProp->szFTPPort );
    strcpy( pMTPass->chProxy,         pMTJobProp->szProxyAddress );
    strcpy( pMTPass->chProxyPort,     pMTJobProp->szProxyPort );
    pMTPass->fPassiveMode =           pMTJobProp->fPassiveMode;
    switch ( pMTJobProp->FTPMode )
    {
      case FTPMODE_SOCKSPROXY :       pMTPass->iFTPMode = 1;    break;
      case FTPMODE_FTPPROXY :         pMTPass->iFTPMode = 2;    break;
      default:                        pMTPass->iFTPMode = 0;    break;
    } /*endswitch */
  }
  else
  {
    strcpy( pMTPass->chServer,        pMTJobProp->szMQServer );
    strcpy( pMTPass->chQueueMgr,      pMTJobProp->szQueueMgr );
    strcpy( pMTPass->chServerInQueue, pMTJobProp->szServerInQueue );
    strcpy( pMTPass->chServerOutQueue, pMTJobProp->szServerOutQueue );
    memset( pMTPass->chMESSAGEID, 0, sizeof(pMTPass->chMESSAGEID) );
    memcpy( pMTPass->chMESSAGEID, pMTJobProp->szMessageID, min(sizeof(pMTPass->chMESSAGEID),sizeof(pMTJobProp->szMessageID)) );
  } /* endif */
  strcpy( pMTPass->chTEMP1,         pMTJobProp->szFolderName );
  strcpy( pMTPass->chSourceLang,    pMTJobProp->szSourceLang );
  strcpy( pMTPass->chTargetLang,    pMTJobProp->szTargetLang );
  strcpy( pMTPass->chSUBJAREA,      pMTJobProp->szSubjArea );
  strcpy( pMTPass->chSENDEREMAIL,   pMTJobProp->szSenderEMail );
  Utlstrccpy( pMTPass->chZIPFILE,   pszMTJobProp, DOT );
  strcat( pMTPass->chZIPFILE, ".ZIP" );
  if ( pMTJobProp->lNumOfWords ) sprintf( pMTPass->chTEMP2, "%ld", pMTJobProp->lNumOfWords );

  return (fOk);

} /* end of function FillMTPass */

// load MT engine DLL if not loaded already
BOOL MTLoadEngine( PMTPASS  pMTPass, PSZ pszEngineDll, PSZ pszMTError)
{
  USHORT   usRc = NO_ERROR;
  BOOL     fOK = TRUE;

  pMTPass->fDLLLoadFailed = FALSE;

  // load the MT dll ...
  if ( (strcmp( pszEngineDll, szMTDll ) != 0) || !hmodMTDll )
  {
    // free old one and load new one...
    if ( hmodMTDll )
    {
      usRc = DosGetProcAddr( hmodMTDll,"MT_TERMINATE", (PFN*) (&pfnMTTerminate));

      if (usRc == NO_ERROR)
      {
        pfnMTTerminate();
      } /* endif */
      usRc = NO_ERROR;

      DosFreeModule( hmodMTDll );
      hmodMTDll = NULL;
    } /* endif */

    usRc = DosLoadModule( NULL, 0 , pszEngineDll, &hmodMTDll );
    if ( usRc == NO_ERROR )
    {
      // store name of currently active one
      strcpy( szMTDll, pszEngineDll );

      // call init routine of engine
      usRc = DosGetProcAddr( hmodMTDll,"MT_INIT", (PFN*) (&pfnMTInit));

      if (usRc == NO_ERROR)
      {
        pfnMTInit( pMTPass );
      } /* endif */
      usRc = NO_ERROR;
    }
    else
    {
      fOK=FALSE;
      strcpy( pszMTError, "Load of MT I/F failed" );
      pMTPass->fDLLLoadFailed = TRUE;
    } /* endif */
  }
  else
  {
    // we can use the loaded one...
  } /* endif */
  return( fOK );
} /* end of function MTLoadEngine */


//------------------------------------------------------------------------------
// Function name: MTSendFile
//------------------------------------------------------------------------------
BOOL MTSendFile
(
  PMTPASS  pMTPass,
  PSZ      pszEngineDll,
  PSZ      pszDocName,
  PSZ      pszMTProfile,
  PSZ      pszMTError
)
{
  USHORT   usRc = NO_ERROR;
  BOOL     fOK = TRUE;

  fOK = MTLoadEngine( pMTPass, pszEngineDll, pszMTError );

  if ( fOK )
  {
     usRc = DosGetProcAddr( hmodMTDll,"MT_SEND_FILE", (PFN*) (&pfnMTSendFile));

     if (usRc == NO_ERROR)
     {

        fOK = pfnMTSendFile( pMTPass, pszDocName, pMTPass->chSourceLang, pMTPass->chTargetLang,
                             "", pszMTProfile, pszMTError );
     }
     else
     {
        fOK = FALSE;
        strcpy( pszMTError, pszEngineDll);
        strcat( pszMTError," : unable to load dll");
     } /* end if */
  } /* endif */

  return fOK;

} /* end of function MTSendFile */


//------------------------------------------------------------------------------
// Function name: MTQueryStatus
//------------------------------------------------------------------------------
BOOL MTQueryStatus
(
  PMTPASS  pMTPass,
  PSZ      pszEngineDll,
  PSZ      pszMTId,
  PSZ      pszMTError,
  PSZ      pszJobStatus,
  PUSHORT  pusComplete
)
{
  USHORT   usRc = NO_ERROR;
  BOOL     fOK = TRUE;

  fOK = MTLoadEngine( pMTPass, pszEngineDll, pszMTError );

  if ( fOK )
  {
     usRc = DosGetProcAddr( hmodMTDll,"MT_GET_STATUS",
                            (PFN*) (&pfnMTGetStatus));

     if (usRc == NO_ERROR)
     {
        LONG lJobStatus = 0L;
        fOK = pfnMTGetStatus( pMTPass, pszMTId, pszJobStatus, &lJobStatus, pusComplete );
        if ( fOK && lJobStatus && *pusComplete )
        {
          // job failed on server
          strcpy( pszMTError, pszJobStatus );
        } /* endif */
     }
     else
     {
        fOK = FALSE;
        strcpy( pszMTError, pszEngineDll);
     } /* end if */
  } /* endif */

  return fOK;

} /* end of function MTQueryStatus */

//------------------------------------------------------------------------------
// Function name: MTReceiveFile
//------------------------------------------------------------------------------
BOOL MTReceiveFile
(
  PMTPASS  pMTPass,
  PSZ      pszEngineDll,
  PSZ      pszMTId,
  PSZ      pszDocument,
  PSZ      pszMTError
)
{
  USHORT   usRc = NO_ERROR;
  BOOL     fOK = TRUE;

  fOK = MTLoadEngine( pMTPass, pszEngineDll, pszMTError );


  if ( fOK )
  {
     usRc = DosGetProcAddr( hmodMTDll,"MT_RECEIVE_FILE",
                            (PFN*) (&pfnMTReceiveFile));

     if (usRc == NO_ERROR)
     {
        fOK = pfnMTReceiveFile(pMTPass, pszMTId, pszDocument, pszMTError );
     }
     else
     {
        fOK = FALSE;
        strcpy( pszMTError, pszEngineDll);
     } /* end if */
  } /* endif */

  return fOK;

} /* end of function MTReceiveFile */




//------------------------------------------------------------------------------
// Function name: MtDeleteJob
//------------------------------------------------------------------------------
BOOL MTDeleteJob
(
  PMTPASS  pMTPass,
  PSZ      pszEngineDll,
  PSZ      pszMTJobId,
  PSZ      pszMTError
)
{
  USHORT   usRc = NO_ERROR;
  BOOL     fOK = TRUE;

  fOK = MTLoadEngine( pMTPass, pszEngineDll, pszMTError );

  if ( fOK )
  {
     usRc = DosGetProcAddr( hmodMTDll,"MT_DELETE_JOB",
                            (PFN*) (&pfnMTDeleteJob));

     if (usRc == NO_ERROR)
     {
        fOK = pfnMTDeleteJob(pMTPass, pszMTJobId, pszMTError );
     }
     else
     {
        fOK = FALSE;
        strcpy( pszMTError, pszEngineDll);
     } /* end if */
  } /* endif */

  return fOK;

} /* end of function MtDeleteJob */




//------------------------------------------------------------------------------
// Function name: MTListCallBack
//------------------------------------------------------------------------------
static CHAR   szMTError[200];

MRESULT MTListCallBack
(
  PLISTCOMMAREA    pCommArea,
  HWND             hwnd,
  WINMSG           message,
  WPARAM           mp1,
  LPARAM           mp2
)
{
  MRESULT          mResult = MRFROMSHORT(FALSE);

  switch ( message )
  {
    case WM_CREATE :
      {
        BOOL       fOK = TRUE;         // initialisation is O.K. flag
        PMTLISTPROP pProp = NULL;      // ptr to MT list properties
        ULONG       ulBytes = 0;

        /**************************************************************/
        /* Load MT list propertes                                     */
        /**************************************************************/
        UtlMakeEQFPath( pCommArea->szBuffer, NULC, PROPERTY_PATH, NULL );
        strcat( pCommArea->szBuffer, "\\EQFPROP.MTL" );
        if ( !UtlLoadFileL( pCommArea->szBuffer, (PVOID *)&pProp, &ulBytes, FALSE, FALSE ) )
        {
          fOK = UtlAlloc( (PVOID *)&pProp, 0L, sizeof(MTLISTPROP), ERROR_STORAGE );
        } /* endif */

        /**************************************************************/
        /* Load column listbox title strings                          */
        /**************************************************************/
        if ( fOK  )
        {
//          LOADSTRING( NULLHANDLE, hResMod, SID_MT_DOC_COLTITLE,   ColHdr[1]);
//          UtlLoadWidth( NULLHANDLE, hResMod, SID_MT_DOC_COLWIDTH,
//                        &(ColTable[1].usWidth) );
//          LOADSTRING( NULLHANDLE, hResMod, SID_MT_STATUS_COLTITLE,  ColHdr[2]);
//          UtlLoadWidth( NULLHANDLE, hResMod, SID_MT_STATUS_COLWIDTH,
//                        &(ColTable[2].usWidth) );
//          LOADSTRING( NULLHANDLE, hResMod, SID_MT_DATE_COLTITLE,   ColHdr[3]);
//          UtlLoadWidth( NULLHANDLE, hResMod, SID_MT_DATE_COLWIDTH,
//                        &(ColTable[3].usWidth) );
//          LOADSTRING( NULLHANDLE, hResMod, SID_MT_ENGINE_COLTITLE, ColHdr[4]);
//          UtlLoadWidth( NULLHANDLE, hResMod, SID_MT_ENGINE_COLWIDTH,
//                        &(ColTable[4].usWidth) );
//          LOADSTRING( NULLHANDLE, hResMod, SID_MT_SOURCELANG_COLTITLE, ColHdr[5]);
//          UtlLoadWidth( NULLHANDLE, hResMod, SID_MT_SOURCELANG_COLWIDTH,
//                        &(ColTable[5].usWidth) );
//          LOADSTRING( NULLHANDLE, hResMod, SID_MT_TARGETLANG_COLTITLE, ColHdr[6]);
//          UtlLoadWidth( NULLHANDLE, hResMod, SID_MT_TARGETLANG_COLWIDTH,
//                        &(ColTable[6].usWidth) );
        } /* endif */

        /**************************************************************/
        /* Set column listbox view lists                              */
        /**************************************************************/
        if ( fOK  )
        {

          memcpy( pCommArea->asCurView,
                  (pProp->sLastUsedMTViewList[0] != 0) ?
                     pProp->sLastUsedMTViewList : sLastUsedView,
                  sizeof(pCommArea->asCurView) );
          MTCLBData.psLastUsedViewList = pCommArea->asCurView;

          memcpy( pCommArea->asDetailsView,
                  (pProp->sDetailsMTViewList[0] != 0) ?
                     pProp->sDetailsMTViewList : sDetailsView,
                  sizeof(pCommArea->asDetailsView) );
          MTCLBData.psDetailsViewList = pCommArea->asDetailsView;

          memcpy( pCommArea->asSortList,
                  (pProp->sSortListMT[0] != 0) ?
                     pProp->sSortListMT : sSortCriteria,
                  sizeof(pCommArea->asSortList) );
          MTCLBData.psSortList = pCommArea->asSortList;
        } /* endif */

        /****************************************************************/
        /* supply all information required to create a MT list          */
        /****************************************************************/
        if ( fOK )
        {
          pCommArea->sListObjClass = clsMTLIST;
          // LOADSTRING( NULLHANDLE, hResMod, SID_MT_TITLE, pCommArea->szTitle );
          strcpy( pCommArea->szTitle, "MT Job List" );
          pCommArea->hIcon          = (HPOINTER) UtlQueryULong(QL_FOLICON); //hiconFOL;
          pCommArea->fNoClose       = FALSE;
          pCommArea->sObjNameIndex  = MT_OBJECT_IND;
          pCommArea->sNameIndex     = MT_JOBNAME_IND;
          pCommArea->sListWindowID  = ID_MTLIST_WINDOW;
          pCommArea->sListboxID     = PID_MTLIST_LB;
          pCommArea->sPopupMenuID   = ID_MT_POPUP;
          pCommArea->sGreyedPopupMenuID   = ID_MT_POPUP;
          pCommArea->sNoSelPopupMenuID = ID_MT_POPUP_NOSEL;
          pCommArea->pColData       = &MTCLBData;
          pCommArea->fMultipleSel   = TRUE;
          pCommArea->sDefaultAction = PID_FILE_MI_PROPERTIES;
          memcpy( &(pCommArea->swpSizePos), &(pProp->SwpMTListWindow), sizeof(EQF_SWP) );

          /************************************************************/
          /* set defaults (if not yet initialized                     */
          /************************************************************/
          if ( (pCommArea->swpSizePos.cx == 0) && (pCommArea->swpSizePos.cy == 0) )
          {
            pCommArea->swpSizePos.cx = pCommArea->swpSizePos.cy = 300;
            pCommArea->swpSizePos.x  = pCommArea->swpSizePos.y  = 100;
            pCommArea->swpSizePos.fs = EQF_SWP_SIZE | EQF_SWP_MOVE | EQF_SWP_SHOW;
          } /* endif */
          pCommArea->sItemClass     = clsMTJOB;
          pCommArea->sItemPropClass = clsMTJOB;
          pCommArea->asMsgsWanted[0] = WM_TIMER;
          pCommArea->asMsgsWanted[1] = WM_EQFN_TASKDONE;
          pCommArea->asMsgsWanted[2] = WM_EQF_UPDATESLIDER;
          pCommArea->asMsgsWanted[3] = 0; // WM_EQFN_CREATED;
          pCommArea->asMsgsWanted[4] = WM_EQFN_DELETED;
          pCommArea->asMsgsWanted[5] = 0;        // ends list of messages
        } /* endif */

        /**************************************************************/
        /* Close properties                                           */
        /**************************************************************/
        if ( pProp ) UtlAlloc( (PVOID *)&pProp, 0L, 0L, NOMSG );

        /**************************************************************/
        /* In case of errors set error return code                    */
        /**************************************************************/
        if ( !fOK )
        {
          mResult = MRFROMSHORT(DO_NOT_CREATE);
        } /* endif */
      }
      break;

    case WM_CLOSE :
    case WM_EQF_TERMINATE :
      /**************************************************************/
      /* Save view lists for WM_EQF_TERMINATE only if save flag is  */
      /* on                                                         */
      /**************************************************************/
      if ( (message == WM_EQF_TERMINATE) &&
            pCommArea->fUserFlag         &&
            !(SHORT1FROMMP1(mp1) & TWBFORCE) )
      {
        /**************************************************************/
        /* Reject termination request                                 */
        /**************************************************************/
        mResult = MRFROMSHORT( TRUE );
      }
      else if ( (message == WM_CLOSE) || (SHORT1FROMMP1(mp1) & TWBSAVE) )
      {
        PMTLISTPROP pProp = NULL;      // ptr to MT list properties
        ULONG       ulBytes = 0;

        /**************************************************************/
        /* Load MT list propertes                                     */
        /**************************************************************/
        UtlMakeEQFPath( pCommArea->szBuffer, NULC, PROPERTY_PATH, NULL );
        strcat( pCommArea->szBuffer, "\\EQFPROP.MTL" );
        if ( !UtlLoadFileL( pCommArea->szBuffer, (PVOID *)&pProp, &ulBytes, FALSE, FALSE ) )
        {
          UtlAlloc( (PVOID *)&pProp, 0L, sizeof(MTLISTPROP), ERROR_STORAGE );
        } /* endif */

        memcpy( &pProp->SwpMTListWindow, &(pCommArea->swpSizePos), sizeof(EQF_SWP) );
        memcpy( pProp->sLastUsedMTViewList, pCommArea->asCurView,
                sizeof(pProp->sLastUsedMTViewList) );
        memcpy( pProp->sDetailsMTViewList, pCommArea->asDetailsView,
                sizeof(pProp->sDetailsMTViewList) );
        memcpy( pProp->sSortListMT, pCommArea->asSortList,
                sizeof(pProp->sSortListMT) );

        UtlWriteFile( pCommArea->szBuffer, sizeof(MTLISTPROP), pProp, FALSE );
        UtlAlloc( (PVOID *)&pProp, 0L, 0L, NOMSG );

        WinStopTimer( WinQueryAnchorBlock( hwnd ), hwnd, TIMER_ID );
      }
      else
      {
        WinStopTimer( WinQueryAnchorBlock( hwnd ), hwnd, TIMER_ID );
      } /* endif */
      break;

    case WM_DESTROY:
      /****************************************************************/
      /* Free all resource allocated by list instance callback        */
      /* function                                                     */
      /* nothing to do for folder list                                */
      /****************************************************************/
      break;

    case WM_EQF_INITIALIZE:
      /****************************************************************/
      /* Fill column listbox                                          */
      /****************************************************************/
      LoadMTJobNames( pCommArea->szObjName+2, pCommArea->hwndLB, TRUE, pCommArea->szBuffer );
      if (! WinStartTimer( WinQueryAnchorBlock(hwnd), hwnd, TIMER_ID, TIMER_VAL ) )
      {
        BOOL fOK = FALSE;
        fOK = TRUE;
      } /* endif */
      break;

    case WM_EQF_BUILDITEMTEXT :
      /****************************************************************/
      /* Setup item text for the object passed in mp2 parameter       */
      /****************************************************************/
      mResult = (MRESULT)MTBuildItemText( (PSZ)PVOIDFROMMP2(mp2), pCommArea->szBuffer );
      break;


    case WM_EQF_INITMENU:
    case WM_INITMENU:
      {
        int iSelItems = SendMessage( pCommArea->hwndLB, LB_GETSELCOUNT, 0, 0L );


        UtlMenuDisableAll( hwnd, SHORT1FROMMP1( mp1) );
        UtlMenuEnableItem( PID_VIEW_MI_NAMES );
        UtlMenuEnableItem( PID_VIEW_MI_DETAILSDLG );
        UtlMenuEnableItem( PID_VIEW_MI_DETAILS );
        if ( iSelItems == 1 )
        {
          UtlMenuEnableItem( PID_FILE_MI_PROPERTIES );
        } /* endif */
        if ( iSelItems >= 1 )
        {
          UtlMenuEnableItem( PID_FILE_MI_DELETE );
        } /* endif */
        UtlMenuEnableItem( PID_VIEW_MI_SORT );
        UtlMenuEnableItem( PID_VIEW_MI_SOME );
        UtlMenuEnableItem( PID_VIEW_MI_ALL );
      }
      break;

    case WM_EQF_TOOLBAR_ENABLED:
      switch ( mp1 )
      {
        /**************************************************************/
        /* check for items to be enabled ..                           */
        /**************************************************************/
        case PID_VIEW_MI_NAMES:
        case PID_VIEW_MI_DETAILSDLG:
        case PID_VIEW_MI_DETAILS:
        case PID_VIEW_MI_SORT:
        case PID_VIEW_MI_SOME:
        case PID_VIEW_MI_ALL:
          mResult = MRFROMSHORT(TRUE);
          break;
        case PID_FILE_MI_PROPERTIES:
          {
            int iSelItems = SendMessage( pCommArea->hwndLB, LB_GETSELCOUNT, 0, 0L );
            if ( iSelItems == 1 )
            {
              mResult = MRFROMSHORT(TRUE);
            } /* endif */
          }
          break;
        case PID_FILE_MI_DELETE:
          {
            int iSelItems = SendMessage( pCommArea->hwndLB, LB_GETSELCOUNT, 0, 0L );
            if ( iSelItems >= 1 )
            {
              mResult = MRFROMSHORT(TRUE);
            } /* endif */
          }
          break;
        default:
          break;
      } /* endswitch */
      break;

    case WM_TIMER:
      WinStopTimer( WinQueryAnchorBlock( hwnd ), hwnd, TIMER_ID );
      HandleTimer( hwnd, pCommArea );
      WinStartTimer( WinQueryAnchorBlock(hwnd), hwnd, TIMER_ID, TIMER_VAL );
      break;

    case WM_EQFN_TASKDONE:
      if ( SHORT1FROMMP1( mp1 ) )
      {
        MTMergeCompl( pCommArea, (PSZ) mp2 );
      } /* endif */
      break;

    case WM_EQF_UPDATESLIDER:
      if ( SHORT1FROMMP1( mp1 ) )
      {
        MTMergeStatus( pCommArea, (SHORT) mp1,  (PSZ) mp2 );
      } /* endif */
      break;


    case WM_EQF_COMMAND:
    case WM_COMMAND:
      mResult = MRFROMSHORT( TRUE ); // default return code for COMMAND msgs
      switch ( SHORT1FROMMP1(mp1) )
      {
      //------------------------
        case PID_FILE_MI_KILL:
        case PID_FILE_MI_DELETE:
          {
            // delete all selected MT jobs
            SHORT sItem = QUERYNEXTSELECTIONHWND( pCommArea->hwndLB, LIT_FIRST );

            while ( sItem != LIT_NONE )
            {
              CHAR szObjName[MAX_EQF_PATH];
              SendMessage( pCommArea->hwndLB, LM_EQF_QUERYITEMTEXT, (WPARAM)sItem, (LPARAM)pCommArea->szBuffer );

              strcpy( szObjName, UtlParseX15( pCommArea->szBuffer, MT_OBJECT_IND) );
              MTCleanup( szObjName, SHORT1FROMMP1( mp1 ), szMTError );
              UtlDelete( szObjName, 0L, FALSE );
              EqfSend2Handler( MTLISTHANDLER, WM_EQFN_DELETED,  MP1FROMSHORT( clsMTJOB  ), MP2FROMP( szObjName ));

              sItem = QUERYNEXTSELECTIONHWND( pCommArea->hwndLB, LIT_FIRST );
            } /* endwhile */
          }
          break;


        case PID_FILE_MI_PROPERTIES:
          {
            SHORT sItem = QUERYNEXTSELECTIONHWND( pCommArea->hwndLB, LIT_FIRST );

            if ( sItem != LIT_NONE )
            {
              BOOL fOk = TRUE;
              HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
              CHAR szObjName[MAX_EQF_PATH];
              SendMessage( pCommArea->hwndLB, LM_EQF_QUERYITEMTEXT, (WPARAM)sItem, (LPARAM)pCommArea->szBuffer );

              strcpy( szObjName, UtlParseX15( pCommArea->szBuffer, MT_OBJECT_IND) );
              DIALOGBOX( hwnd, EQFMTPROPERTYDLGPROC, hResMod, ID_MTPROP_DLG, szObjName, fOk);
            } /* endif */
          }
          break;

      }
      break;


//    case WM_EQFN_CREATED :
      /**************************************************************/
      /* update files listed                                        */
      /**************************************************************/


//      LoadMTJobNames( pCommArea->szObjName+2, pCommArea->hwndLB, TRUE,
//                         pCommArea->szBuffer );
//      break;

//    case WM_EQFN_DELETED :
      // kill the job and reload the list
      //   -- mp2 contains SNOMATCH name to be deleted....
//      MTCleanup( (PSZ) mp2, PID_FILE_MI_DELETE, szMTError );

//      LoadMTJobNames( pCommArea->szObjName+2, pCommArea->hwndLB, TRUE,
//                         pCommArea->szBuffer );
      break;
    default:
      break;
  } /* endswitch */
  return( mResult );
} /* end of function MTListCallBack */

static void MTCleanup
(
  PSZ pMTJobObject,
  USHORT usTaskID,
  PSZ pszMTError
)
{
  MTPASS MTPass;
  BOOL fOk = TRUE;
  PMTJOBPROP pProp = NULL;
  ULONG      ulBytes = 0;

  if ( UtlLoadFileL( pMTJobObject, (PVOID *)&pProp, &ulBytes, FALSE, FALSE ) )
  {
    if (usTaskID == PID_FILE_MI_DELETE)
    {
       UtlDelete( pProp->szSegFile, 0L, FALSE );
       UtlDelete( pProp->szZipFile, 0L, FALSE );
    } /* end if */

    FillMTPass( &MTPass, pProp, pMTJobObject );

    // delete job on the MT server
    if ( pProp->fUseFTP )
    {
      if ( pProp->lSendDate )
      {
        fOk = MTDeleteJob( &MTPass, pProp->szMTEngineDLL, pProp->szMessageID, pszMTError);
      } /* endif */
    }
    else if ( pProp->szMessageID[0] )
    {
      fOk = MTDeleteJob( &MTPass, pProp->szMTEngineDLL, pProp->szMessageID, pszMTError);
    } /* endif */

    if ( !fOk )
    {
      // display error message with string szMTError
      UtlError( ERROR_MT_MACHINE, MB_CANCEL, 1, &pszMTError , EQF_ERROR);
    }/* end if*/

    // delete message ID in properties
    if ( fOk )
    {
      memset( pProp->szMessageID, 0, sizeof(pProp->szMessageID) );
      pProp->lStatus = MTJOB_Completed;

      UtlWriteFile( pMTJobObject, sizeof(MTJOBPROP), pProp, FALSE );
      EqfSend2Handler( MTLISTHANDLER, WM_EQFN_PROPERTIESCHANGED,
                       MP1FROMSHORT( clsMTJOB  ),
                       MP2FROMP( pMTJobObject ));
    } /* endif */

    UtlAlloc( (PVOID *)&pProp, 0L, 0L, NOMSG );
  } /* endif */
} /* end of function MTCleanup */


//------------------------------------------------------------------------------
//   Fill List Box with MT job names
//------------------------------------------------------------------------------

USHORT LoadMTJobNames( PSZ folder, HWND hlbox, BOOL flg, PSZ pszBuffer )
{
  LONGFILEFIND stResultBuf;             // DOS file find struct
  USHORT      usCount = 1;             // number of files requested
  USHORT      usRC;                    // return code of called functions
  HDIR        hDirHandle = HDIR_CREATE;// DosFind routine handle
  CHAR        szSearchPath[MAX_EQF_PATH];   // buffer for search path
  CHAR        szPropFile[MAX_EQF_PATH];     // buffer for MT job property file name
  PSZ  pszName = stResultBuf.achName;     // address name field in result buffer

  folder; flg;
  ENABLEUPDATEHWND_FALSE( hlbox );
  SETCURSOR( SPTR_WAIT );

  DELETEALLHWND( hlbox );

  UtlMakeEQFPath( szSearchPath, NULC, DIRSEGMT_PATH, NULL );
  strcat( szSearchPath, "\\*" );
  strcat( szSearchPath, EXT_OF_MTPROP );
  usRC = UtlFindFirstLong( szSearchPath, &hDirHandle, FILE_NORMAL, &stResultBuf, FALSE );
  while ( (usRC == NO_ERROR) && usCount )
  {
    UtlMakeEQFPath( szPropFile, NULC, DIRSEGMT_PATH, NULL );
    strcat( szPropFile, BACKSLASH_STR );
    strcat( szPropFile, pszName );
    if ( MTBuildItemText( szPropFile, pszBuffer ) ) INSERTITEMHWND( hlbox, pszBuffer );
    usRC = UtlFindNextLong( hDirHandle, &stResultBuf, 0);
  } /* endwhile */

  if ( hDirHandle != HDIR_CREATE ) UtlFindClose( hDirHandle, FALSE );

  ENABLEUPDATEHWND_TRUE( hlbox );
  SETCURSOR( SPTR_ARROW );


  usCount = QUERYITEMCOUNTHWND( hlbox );

  return( usCount);
}

BOOL MTBuildItemText
(
  PSZ         pszObjName,
  PSZ         pszBuffer
)
{
  CHAR        chMTStatus[80];
  ULONG       ulBytes = 0;                // size of file
  PMTJOBPROP pJobProp = NULL;        // ptr to in-memory copy of property file
  BOOL fOK = FALSE;

  if ( UtlLoadFileL( pszObjName, (PVOID *)&pJobProp, &ulBytes, FALSE, FALSE ) )
  {
    PSZ pszTemp = strrchr( pJobProp->szJobObject, DOT );
    if ( pszTemp ) *pszTemp = EOS;

    fOK = TRUE;

    MTStatusString( pJobProp->lStatus, pJobProp->szStatus, chMTStatus );

    sprintf( pszBuffer,
    "%s\x15%ld\x15%s\x15%s\x15%s\x15%lu\x15%s\x15%s",
             pszObjName,
             pJobProp->lStatus,
             UtlGetFnameFromPath( pJobProp->szJobObject ),
             pJobProp->szFolderName,
             chMTStatus,
             pJobProp->lJobCreateDate,
             pJobProp->szSourceLang,
             pJobProp->szTargetLang );
    UtlAlloc( (PVOID *)&pJobProp, 0L, 0L, NOMSG );
  } /* endif */
  return( fOK );
} /* end of function MTBuildItemtext */

BOOL MTStatusString( LONG lStatus, PSZ pszStatus, PSZ pszBuffer )
{
 switch( lStatus )
  {
    case MTJOB_Preparing   : strcpy( pszBuffer, "Preparing..." ); break;

    case MTJOB_Prepared    : strcpy( pszBuffer, "Prepared" ); break;

    case MTJOB_Sending     : strcpy( pszBuffer, "Sending..." ); break;

    case MTJOB_Sended      : strcpy( pszBuffer, "Job send to server" ); break;

    case MTJOB_ServerWait  :
      if ( pszStatus[0] )
        strcpy( pszBuffer, pszStatus );
      else
        strcpy( pszBuffer, "Waiting for server" );
      break;

    case MTJOB_Translated  : strcpy( pszBuffer, "Translated" ); break;

    case MTJOB_Receiving   : strcpy( pszBuffer, "Receiving..." ); break;

    case MTJOB_Received    : strcpy( pszBuffer, "Received" ); break;

    case MTJOB_Merging     :
      if ( pszStatus[0] )
        strcpy( pszBuffer, pszStatus );
      else
        strcpy( pszBuffer, "Merging..." );
      break;

    case MTJOB_Completed   : strcpy( pszBuffer, "Completed" ); break;

    case MTJOB_Failed      :
      if ( pszStatus[0] )
        strcpy( pszBuffer, pszStatus );
      else
        strcpy( pszBuffer, "Job failed" );
      break;
    default:                 strcpy( pszBuffer, "unknown" ); break;
  } /* endswitch */
  return( TRUE );
}


// Update the item text
VOID
UpdateItemText
(
  PLISTCOMMAREA pCommArea,
  PVOID       hProp ,                  // handle of document properties
  SHORT       sIndex,
  PSZ         pszName
)
{
  EQFINFO     ErrorInfo;               // error info from EQF API call
  PPROPSYSTEM pSysp;                   // EQF system properties
  char       *ppath;
  PSZ         folder = pCommArea->szObjName+2;
  PSZ         pFolder;


  CHAR        chMTStatus[40];
  char szSourceLang[MAX_LANG_LENGTH];
  char szTargetLang[MAX_LANG_LENGTH];
  char        szDocObjName[200];
  PSZ         pszBuffer = NULL;
  PPROPDOCUMENT   pProp;              // ptr to document properties

  pProp = (PPROPDOCUMENT)MakePropPtrFromHnd( hProp );

  pSysp = (PPROPSYSTEM)MakePropPtrFromHnd( EqfQuerySystemPropHnd());
  UtlAlloc( (PVOID *)&ppath, 0L, (LONG) (2 * MAX_PATH144), ERROR_STORAGE );
  pFolder = UtlGetFnameFromPath( folder );
  UtlMakeEQFPath( ppath+MAX_PATH144, folder[0], DIRSEGNOMATCH_PATH, pFolder );

  UtlAlloc( (PVOID *) &pszBuffer, 0L, 4000L, ERROR_STORAGE );

  // status not set
  // --------------------------
  if (!pProp->lMTStatus)
  {
     // set write access to properties
     SetPropAccess (hProp, PROP_ACCESS_WRITE);
     pProp->lMTStatus = MTEngine_NotReady;
     SaveProperties ( hProp, &ErrorInfo);
  } // end if

  // engine or memory not set
  // ------------------------
  if (!pProp->chMTEngine[0] || !pProp->chMTMemory[0])
  {
     // set write access to properties
     SetPropAccess (hProp, PROP_ACCESS_WRITE);
     pProp->lMTStatus = MTEngine_NotReady;
     SaveProperties ( hProp, &ErrorInfo);
  } // end if


  switch(pProp->lMTStatus)
  {

    case MTEngine_NotReady :
      strcpy(chMTStatus,STR_MTEngine_0);
      break;
    case MTEngine_Prepared:
      strcpy(chMTStatus,STR_MTEngine_1);
      break;
    case MTEngine_Waiting:
      strcpy(chMTStatus,STR_MTEngine_2);
      break;
    case MTEngine_Processing:
      if ( pProp->usMTComplete != 0 )
      {
        sprintf(chMTStatus,STR_MTEngine_3_ALL, pProp->usMTComplete);
      }
      else
      {
        strcpy(chMTStatus,STR_MTEngine_3);
      } /* endif */
      break;
    case MTEngine_Translated:
      strcpy(chMTStatus,STR_MTEngine_4);
      break;
    case MTEngine_Merged:
      strcpy(chMTStatus,STR_MTEngine_5);
      break;
    case MTEngine_Failed:
      strcpy(chMTStatus,STR_MTEngine_6);
      break;
    case MTEngine_Merging:
      sprintf( chMTStatus,STR_MTEngine_7, pProp->usMerged );
      break;
    case MTEngine_MergeStart:
      strcpy(chMTStatus,STR_MTEngine_8);
      break;
    default:
      strcpy(chMTStatus,STR_MTEngine_NA);
      break;
  } // end switch


    sprintf( szDocObjName, "%s\\%s", folder, pszName );
    DocQueryInfo2(szDocObjName,NULL,NULL,
                  szSourceLang, szTargetLang,
                  NULL,NULL,NULL,FALSE);

    sprintf( pszBuffer,
    "%s\\%s\x15%s\x15%s\x15%lu\x15%s\x15%s\x15%s",
             folder,
             pszName,
             pProp->szLongName[0] ? pProp->szLongName : pszName,
             chMTStatus,
             pProp->ulSeg,   // display touched date
             pProp->chMTEngine,
             szSourceLang,
             szTargetLang );

   SETITEMTEXTHWND( pCommArea->hwndLB, sIndex, pszBuffer );

   UtlAlloc( (PVOID *)&ppath, 0L, 0L, NOMSG );
   UtlAlloc( (PVOID *)&pszBuffer, 0L, 0L, NOMSG );
}


static VOID HandleTimer
(
  HWND hwnd,
  PLISTCOMMAREA pCommArea
)
{
  SHORT sCountNum = QUERYITEMCOUNTHWND( pCommArea->hwndLB );
  SHORT sIndex = 0;
  BOOL fPropChanged = FALSE;
  PSZ  pszTempObjName = NULL;

  // return asap if already handling timer message
  if ( fHandlingTimerMessage )
  {
    return;
  } /* endif */

  fHandlingTimerMessage = TRUE;

  // allocate buffer to temporarely store object name (my be changed while processing notification
  // messages
  UtlAlloc( (PVOID *)&pszTempObjName, 0L, MAX_EQF_PATH, ERROR_STORAGE );
  if ( pszTempObjName == NULL ) return;

  if ( ThreadData.State == ThreadState_Done )
  {
    // do postprocessing for completed task
    switch( ThreadData.Task )
    {
      case ThreadTask_SendFile:
        if ( ThreadData.fOK )
        {
          // store message ID in MT Job properties
          PMTJOBPROP pProp = NULL;
          ULONG      ulBytes = 0;
          if ( UtlLoadFileL( ThreadData.szObject, (PVOID *)&pProp, &ulBytes, FALSE, FALSE ) )
          {
            memset( pProp->szMessageID, 0, sizeof(pProp->szMessageID) );
            if ( pProp->fUseFTP )
            {
              strcpy( pProp->szMessageID, ThreadData.MTPass.chMESSAGEID );
            }
            else
            {
              memcpy( pProp->szMessageID, ThreadData.MTPass.chMESSAGEID,
                      min(sizeof(ThreadData.MTPass.chMESSAGEID),sizeof(pProp->szMessageID)) );
            } /* endif */
            UtlWriteFileL( ThreadData.szObject, ulBytes, pProp, FALSE );
            UtlAlloc( (PVOID *)&pProp, 0L, 0L, NOMSG );
            MTChangeJobStatus( ThreadData.szObject, MTJOB_ServerWait, "", 0L );
          }
          else
          {
            // MT job does not exist anymore, ignore it
          } /* endif */
        }
        else
        {
          MTChangeJobStatus( ThreadData.szObject, MTJOB_Failed, ThreadData.szMTError, 0L );
          if ( ThreadData.MTPass.fDLLLoadFailed )
          {
            UtlError( ERROR_MTMQS_LOAD_FAILED, MB_CANCEL, 0, NULL, EQF_ERROR);
          } /* endif */
        } /* endif */
        break;

      case ThreadTask_ReceiveFile:
        if ( ThreadData.fOK )
        {
          // switch to MT job received state
          MTChangeJobStatus( ThreadData.szObject, MTJOB_Received, "", 0L );
        }
        else
        {
          MTChangeJobStatus( ThreadData.szObject, MTJOB_Failed, ThreadData.szMTError, 0L );
        } /* endif */
        break;

      case ThreadTask_GetStatus:
        if ( ThreadData.fOK )
        {
          if ( ThreadData.usComplete )
          {
            // switch to MT job translated state or error state
            if ( ThreadData.szMTError[0] )
            {
              MTChangeJobStatus( ThreadData.szObject, MTJOB_Failed, ThreadData.szMTError, 0L );
            }
            else
            {
              MTChangeJobStatus( ThreadData.szObject, MTJOB_Translated, "", 0L );
            } /* endif */
          }
          else
          {
            // show new MT job status
            MTChangeJobStatus( ThreadData.szObject, MTJOB_ServerWait, ThreadData.szJobStatus, 0L );
          } /* endif */
        }
        else
        {
//        MTChangeJobStatus( ThreadData.szObject, MTJOB_Failed, ThreadData.szMTError, 0L );
        } /* endif */
        break;

      case ThreadTask_DeleteJob:
        // no postprocessign required
        break;
      default:
        break;
    } /* endswitch */

    // allow new tasks to be performed by processing thread
    ThreadData.Task = ThreadTask_None;
    ThreadData.State = ThreadState_Ready;
  } /* endif */

  // work always from top to bottom -- but update only one status at a time
  while ( (sIndex < sCountNum) && !fPropChanged )
  {
    PMTJOBPROP pProp = NULL;
    ULONG      ulBytes = 0;
    PSZ   pszObjName;
    LONG  lStatus;

    QUERYITEMTEXTHWND( pCommArea->hwndLB, sIndex, pCommArea->szBuffer );
    pszObjName = UtlParseX15( pCommArea->szBuffer, MT_OBJECT_IND );
    lStatus = atol( UtlParseX15( pCommArea->szBuffer, MT_INTSTATUS_IND ) );
    strcpy( pszTempObjName, pszObjName );

     if ( UtlLoadFileL( pszTempObjName, (PVOID *)&pProp, &ulBytes, FALSE, FALSE ) )
     {
         switch ( pProp->lStatus )
         {
           case MTJOB_Prepared:
             {
               if ( ThreadData.State == ThreadState_Ready )
               {
                 MTChangeJobStatus( pszTempObjName, MTJOB_Sending, "", 0L );
                 fPropChanged = TRUE;

                 // prepare receive task
                 FillMTPass( &(ThreadData.MTPass), pProp, pszTempObjName );
                 ThreadData.fOK = TRUE;
                 strcpy( ThreadData.szMTEngineDLL, pProp->szMTEngineDLL );
                 strcpy( ThreadData.szObject, pszTempObjName );
                 strcpy( ThreadData.szDocName, pProp->szSegFile );
                 ThreadData.szMTError[0] = EOS;
                 ThreadData.Task = ThreadTask_SendFile;
               } /* endif */
             }
             break;
           case MTJOB_Translated:
             {
               if ( ThreadData.State == ThreadState_Ready )
               {
                 MTChangeJobStatus( pszTempObjName, MTJOB_Receiving, "", 0L );
                 fPropChanged = TRUE;

                 // prepare receive task
                 FillMTPass( &(ThreadData.MTPass), pProp, pszTempObjName );
                 ThreadData.fOK = TRUE;
                 strcpy( ThreadData.szMTEngineDLL, pProp->szMTEngineDLL );
                 strcpy( ThreadData.szObject, pszTempObjName );
                 strcpy( ThreadData.szDocName, pProp->szSegFile );
                 ThreadData.szMTError[0] = EOS;
                 ThreadData.Task = ThreadTask_ReceiveFile;
               } /* endif */
             }
             break;

           case MTJOB_Merging:
             break;

           case MTJOB_Received:
           {
             MT_TMMERGE TMMerge;
             PSZ        pTM;
             SHORT      sRC;

             strcpy( TMMerge.chMemory, pProp->szFolderName );
             strcat( TMMerge.chMemory, "_MT" );
             strcpy( TMMerge.szObjName, pszTempObjName );
             strcpy( TMMerge.szTargetLang, pProp->szTargetLang );
             TMMerge.hwndNotify = hwnd;
             strcpy( TMMerge.chSGMLFile, pProp->szSegFile );
             SubFolNameToObjectName( pProp->szFolderName, TMMerge.szFolObjName );


             /*********************************************************/
             /* check if another process is already running  -- if so */
             /* postpone our request until first process ended..      */
             /*********************************************************/
             pTM = UtlGetFnameFromPath( TMMerge.chMemory );
             if ( !pTM )
             {
               pTM = TMMerge.chMemory;
             } /* endif */

             sprintf( pCommArea->szBuffer, "MEMIMP: %s", pTM );
             sRC = QUERYSYMBOL( pCommArea->szBuffer );

             if ( sRC == -1 )
             {
               MTChangeJobStatus( pszTempObjName, MTJOB_Merging, "", 0L );
               fPropChanged = TRUE;

               EqfSend2Handler( MEMORYHANDLER, WM_EQF_PROCESSTASK,
                                MP1FROMSHORT( WM_EQF_MT_TMMERGE ),
                                MP2FROMP( &TMMerge ) );
             }
             else
             {
               /*******************************************************/
               /* ignore request and wait until first one is finished */
               /*******************************************************/
             } /* endif */

           }
           break;

           case MTJOB_Merged:
             {
               // switch to complete status
               pProp->lStatus = MTJOB_Completed;
               memset( pProp->szMessageID, 0, sizeof(pProp->szMessageID) );
               fPropChanged = TRUE;
               UtlWriteFile( pszTempObjName, sizeof(MTJOBPROP), pProp, FALSE );

               // remove ZIP and NOMATCh file of job
               DeleteFile( pProp->szZipFile );
               DeleteFile( pProp->szSegFile );

               // broadcast changed properties
               EqfSend2Handler( MTLISTHANDLER, WM_EQFN_PROPERTIESCHANGED,
                                MP1FROMSHORT( clsMTJOB  ),
                                MP2FROMP( pszTempObjName ));
             }
             break;

           case MTJOB_ServerWait:
             // get status from server
             {
              // for FTP mode, check if an error status has been retrieved
              // check first if thread is busy but has already a new server state message
              if ( (ThreadData.State == ThreadState_Busy) &&
                    (ThreadData.Task == ThreadTask_GetStatus) &&
                    ThreadData.MTPass.fStateAvailable &&
                    (strcmp( ThreadData.szObject, pszTempObjName ) == 0) )
              {
                MTChangeJobStatus( ThreadData.szObject, MTJOB_ServerWait, ThreadData.szJobStatus, 0L );
              }
              else if ( ThreadData.State == ThreadState_Ready )
              {
                LONG lCurTime = 0;
                LONG lDelta = 0;

                time( &lCurTime );
                lDelta = lCurTime - pProp->lLastStatusQuery;

                if ( !pProp->fUseFTP || (lDelta > 60) )
                {
                  // update properties with new status query time
                  pProp->lLastStatusQuery = lCurTime;
                  UtlWriteFile( pszTempObjName, sizeof(MTJOBPROP), pProp, FALSE );

                  // prepare get state task
                  FillMTPass( &(ThreadData.MTPass), pProp, pszTempObjName );
                  ThreadData.fOK = TRUE;
                  strcpy( ThreadData.szMTEngineDLL, pProp->szMTEngineDLL );
                  strcpy( ThreadData.szObject, pszTempObjName );
                  ThreadData.szMTError[0] = EOS;
                  ThreadData.Task = ThreadTask_GetStatus;
                } /* endif */

              } // end if
             }
             break;
           case MTJOB_Failed:
           default:
              // nothing to do
              break;
         } /* endswitch */


       UtlAlloc( (PVOID *)&pProp, 0L, 0L, NOMSG );
     } /* endif */

     sIndex ++;
  } /* endwhile */

  UtlAlloc( (PVOID *)&pszTempObjName, 0L, 0L, NOMSG );

  fHandlingTimerMessage = FALSE;

  return;
} /* end of function HandleTimer */


/**********************************************************************/
/* merge completed message                                            */
/**********************************************************************/
static void MTMergeCompl
(
  PLISTCOMMAREA pCommArea,
  PSZ  pszObjName
)
{
  PVOID       hProp;               // handle of folder properties
  EQFINFO     ulErrorInfo;         // error info from EQF API call
  BOOL        fOK = TRUE;
  PMTJOBPROP pProp = NULL;
  ULONG ulBytes = 0;

  // load MT job properties
  fOK = UtlLoadFileL( pszObjName, (PVOID *)&pProp, &ulBytes, FALSE, FALSE );

  // update folder properties with new search translation memory
  if ( fOK )
  {
    SubFolNameToObjectName( pProp->szFolderName,  pCommArea->szBuffer );
    hProp = OpenProperties( pCommArea->szBuffer, NULL, PROP_ACCESS_READ, &ulErrorInfo );
    if ( hProp )
    {
      PPROPFOLDER pPropFolder = (PPROPFOLDER)MakePropPtrFromHnd( hProp );
      int i = 0;
      BOOL fDone   = FALSE;
      BOOL fUpdate = FALSE;

      // build name of new search TM
      strcpy( pCommArea->szBuffer, pProp->szFolderName );
      strcat( pCommArea->szBuffer, "_MT" );

      // search TM name in memory table or find a free slot
      i = 0;
      while ( (i < MAX_NUM_OF_READONLY_MDB) && !fDone )
      {
        if( pPropFolder->aLongMemTbl[i][0] == EOS )
        {
          // a free slot, insert the MT memory
          strcpy( pPropFolder->aLongMemTbl[i], pCommArea->szBuffer );
          fDone   = TRUE;
          fUpdate = TRUE;
        }
        else if ( strcmp( pPropFolder->aLongMemTbl[i], pCommArea->szBuffer ) == 0 )
        {
          // MT memory already in search list, nothing to do
          fDone = TRUE;
        }
        else
        {
          // try next slot
          i++;
        }
      } /* endwhile */

      if ( fUpdate )
      {
        SetPropAccess (hProp, PROP_ACCESS_WRITE);
        SaveProperties ( hProp, &ulErrorInfo);
      } /* endif */
      CloseProperties( hProp, PROP_QUIT, &ulErrorInfo );
    } /* endif */
  } /* endif */

  // switch to completed state
  MTChangeJobStatus( pszObjName, MTJOB_Completed, "", 0L );

  // cleanup
  if ( pProp ) UtlAlloc( (PVOID *)&pProp, 0L, 0L, NOMSG );

} /* endif */


static void MTMergeStatus
(
  PLISTCOMMAREA pCommArea,
  USHORT usPercent,
  PSZ  pszObjName
)
{  // show progress of process
  CHAR szProgress[20];
  pCommArea;
  sprintf( szProgress, "Merged %d%%", usPercent );
  MTChangeJobStatus( pszObjName, MTJOB_Merging, szProgress, 0L );
}

// change state of a MT job
BOOL MTChangeJobStatus
(
  PSZ     pszObjName,
  LONG    lNewStatus,
  PSZ     pszNewStatus,
  LONG    lNumOfWords
)
{
  ULONG    ulBytes = 0;
  PMTJOBPROP pProp = NULL;
  BOOL       fOK = TRUE;

  if ( UtlLoadFileL( pszObjName, (PVOID *)&pProp, &ulBytes, FALSE, FALSE ) )
  {
    LONG lOldStatus = pProp->lStatus;
    pProp->lStatus = lNewStatus;

    switch( lNewStatus )
    {
      case MTJOB_ServerWait:
        if ( lOldStatus == MTJOB_Sending ) UtlTime( &(pProp->lSendDate) );
        break;
      case MTJOB_Translated  : UtlTime( &(pProp->lTranslatedDate) );  break;
      case MTJOB_Received    : UtlTime( &(pProp->lReceivedDate) );    break;
      case MTJOB_Completed   : UtlTime( &(pProp->lMergedDate) );      break;
    } /* endswitch */

    strcpy( pProp->szStatus, pszNewStatus );
    if ( lNumOfWords ) pProp->lNumOfWords = lNumOfWords;
    UtlWriteFile( pszObjName, sizeof(MTJOBPROP), pProp, FALSE );
    UtlAlloc( (PVOID *)&pProp, 0L, 0L, NOMSG );
    EqfSend2Handler( MTLISTHANDLER, WM_EQFN_PROPERTIESCHANGED,
                     MP1FROMSHORT( clsMTJOB  ),
                     MP2FROMP( pszObjName ));
    UtlDispatch();
  }
  else
  {
    fOK = FALSE;
  } /* endif */
  return( fOK );
} /* end of function MTChangeJobStatus */

BOOL MTStartThread()
{
  memset( &ThreadData, 0, sizeof(ThreadData) );
  ThreadData.Task  = ThreadTask_None;
  ThreadData.State = ThreadState_NotStarted;
  _beginthread( MTProcessThread, 0, (void *)&ThreadData );
  return( TRUE );
} /* end of function MTStartThread */

#define THREADWAITTIME  100

void MTProcessThread
(
  void   *pvData                       // pointer to thread data
)
{
  PMTTHREADDATA pData;

  pData = (PMTTHREADDATA )pvData;

  pData->Task  = ThreadTask_None;
  pData->State = ThreadState_Ready;

  do
  {
    if ( pData->State == ThreadState_Ready )
    {
      switch( pData->Task )
      {
        case ThreadTask_SendFile:
          pData->State = ThreadState_Busy;
          pData->fOK = MTSendFile( &(pData->MTPass), pData->szMTEngineDLL, pData->szDocName,
                                   pData->szMTProfile, pData->szMTError );
          pData->State = ThreadState_Done;
          break;
        case ThreadTask_ReceiveFile:
          pData->State = ThreadState_Busy;
          pData->fOK = MTReceiveFile( &(pData->MTPass), pData->szMTEngineDLL, pData->MTPass.chMESSAGEID,
                                      pData->szDocName, pData->szMTError );
          pData->State = ThreadState_Done;
          break;
        case ThreadTask_GetStatus:
          pData->State = ThreadState_Busy;
          pData->szMTError[0] = EOS;
          pData->fOK = MTQueryStatus( &(pData->MTPass), pData->szMTEngineDLL, pData->MTPass.chMESSAGEID,
                                      pData->szMTError,
                                      pData->szJobStatus, &(pData->usComplete) );
          pData->State = ThreadState_Done;
          break;
        case ThreadTask_DeleteJob:
          pData->State = ThreadState_Busy;
          pData->fOK = MTDeleteJob( &(pData->MTPass), pData->szMTEngineDLL, pData->MTPass.chMESSAGEID, pData->szMTError );
          pData->State = ThreadState_Done;
          break;
        default:
          // do nothing
          break;
      } /* endswitch */
    } /* endif */
    Sleep( THREADWAITTIME );
  } while( !pData->fKill );

  pData->State = ThreadState_Stopped;

  _endthread();
} /* end of function MTProcessThread */

//   End of EQFMT00.C
