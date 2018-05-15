//+----------------------------------------------------------------------------+
//| EQFSEGEXPORT.C                                                             |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|          Copyright (C) 1990-2013, International Business Machines          |
//|          Corporation and others. All rights reserved                       |
//+----------------------------------------------------------------------------+
//| Author: Gerhard Queck                                                      |
//|                                                                            |
//+----------------------------------------------------------------------------+
//| Description: Export of segments within a specific tag group                |
//+----------------------------------------------------------------------------+
#define INCL_EQF_TM               // general Transl. Memory functions
#define INCL_EQF_ANALYSIS         // analysis functions
#define INCL_EQF_TAGTABLE         // tagtable defines and functions
#define INCL_EQF_TP               // public translation processor functions
#define INCL_EQF_FOLDER           // folder list and document list functions
#include <eqf.h>                  // General Translation Manager include file

#include "eqfiana1.id"            // Analysis IDs
#include "eqftai.h"               // Private include file of analysis
#include "OTMFUNC.H"            // function call interface public defines
#include "eqffunci.h"           // function call interface private defines


#define EXPORTSEGS_TASK    USER_TASK + 2

// processing steps
typedef enum _EXPORTSEG_STEP
{
  PROCESS_INIT,                        // intialize the process
  NEXT_DOCUMENT,                       // start next document
  START_DOCUMENT,                      // prepare current document
  PROCESS_DOCUMENT,                    // process current document
  END_DOCUMENT,                        // cleanup current document
  PROCESS_COMPLETE,                    // complete the process
  END_PROCESS                          // end process loop
} EXPORTSEG_STEP;

#define MAX_TAG_LENGTH 256

// element of start/stop tag array 
typedef struct _PEXPSEGSSTARTSTOP 
{
  CHAR_W      szStartTag[MAX_TAG_LENGTH]; // start tag
  CHAR_W      szEndTag[MAX_TAG_LENGTH];   // end tag
} EXPSEGSSTARTSTOP, *PEXPSEGSSTARTSTOP;

/**********************************************************************/
/* Instance Data Area (IDA) for build archive TM function             */
/**********************************************************************/
typedef struct _EXPORTSEGS_IDA
{
  CHAR        szOutFile[MAX_LONGPATH]; // fully qualified output file name
  CHAR        szStartStopFile[MAX_LONGPATH]; // fully qualified name of file with start/stop tags
  BOOL        fKill;                   // TRUE = end current process immediately
  HWND        hwndDocLB;               // listbox for document names
  OBJNAME     szParentObjName;         // object name of calling (sub)folder
  OBJNAME     szFolObjName;            // object name of calling main folder
  CHAR        szFolder[MAX_FILESPEC];  // main folder name (with folder extension)
  CHAR        szFolName[MAX_FILESPEC]; // main folder name (without folder extension)
  CHAR        szFolLongName[MAX_LONGFILESPEC]; // long folder name
  OBJNAME     szObjName;               // build archive TM object name
  BOOL        fErrorStop;              // TRUE = process stopped by an error
  USHORT      usComplete;              // current completion rate
  USHORT      usLastComplete;          // completion rate as displayed by slider
  EXPORTSEG_STEP CurStep;                // current processing step
  ULONG       ulSegments;              // number of segments added to archive TM
  SHORT       sCurDoc;                 // number of current document (within listbox)
  CHAR        szCurDoc[MAX_FILESPEC];  // name of current document
  CHAR        szLongName[MAX_LONGFILESPEC];  // name of current document
  OBJNAME     szDocObjName;            // object name of current document
  SHORT       sMaxDocs;                // number of documents to process
  PLOADEDTABLE pLoadedQFTable;         // ptr to loaded QF TagTable
  CHAR        szDocFormat[MAX_FNAME];  // name of document markup table
  CHAR        szDocSourceLang[MAX_LANG_LENGTH]; // document source language
  CHAR        szDocTargetLang[MAX_LANG_LENGTH]; // document target language
  CHAR        szSourceDocName[MAX_EQF_PATH]; // buffer for source document name
  CHAR        szTargetDocName[MAX_EQF_PATH]; // buffer for target document name
  PTBDOCUMENT pSourceDoc;              // ptr to loaded source document
  PTBDOCUMENT pTargetDoc;              // ptr to loaded target document
  ULONG       ulSegNum;                // number of currently active segment
  HWND        hwndErrMsg;              // handle of parent window for error messages
  BOOL        fBatch;                  // TRUE = we are in batch mode
  PSZ         pszDocNames;             // list of document names
  ULONG       ulAddSegNum;             // segment number in additional table
  ULONG       ulActiveTable;           // active segment table
  BOOL        fFolderLocked;           // folder has been locked flag
  BOOL          fInTagGroup;            // TRUE = we are currently inside a tag group
  CHAR_W        szEndTag[MAX_TAG_LENGTH]; // buffer for current end tag
  FILE          *hOutFile;              // output file handle
  CHAR          szTagBuffer[MAX_TAG_LENGTH]; // buffer for tags
  CHAR          szLine[512];            // buffer for lines from input file
  int           iArrayEntries;          // number of used entries in start/stop array
  int           iArraySize;             // size of start/stop array (in number of entries)
  PEXPSEGSSTARTSTOP pStartStop;         // array with start/stop tags
} EXPORTSEGS_IDA, *PEXPORTSEGS_IDA;


MRESULT ExportSegsInit( HWND hwndDlg, WPARAM mp1, LPARAM mp2 );
MRESULT ExportSegsCommand( HWND hwndDlg, SHORT sId, SHORT sNotification );
MRESULT ExportSegsControl( HWND hwndDlg, SHORT sId, SHORT sNotification );
MRESULT ExportSegsClose( HWND hwndDlg, WPARAM mp1, LPARAM mp2 );
USHORT ExportSegsProccess( PEXPORTSEGS_IDA pIda );
BOOL ExpLoadStartStopFile( PEXPORTSEGS_IDA pIda );

USHORT TAFuncPrepExportSegs
(
  PFCTDATA    pData,                   // function I/F session data
  PSZ         pszFolderName,           // name of folder
  PSZ         pszDocuments,            // list with document names or NULL
  PSZ         pszStartStopFile,        // file containing start/stop tag list
  PSZ         pszOutFile,              // name of output file
  LONG        lOptions                 // options for archive TM
);
USHORT TAFuncExportSegsProcess
(
  PFCTDATA    pData                    // ptr to function interface data area
);

MRESULT TABatchExportSegsProcCallBack
(
PPROCESSCOMMAREA pCommArea,          // ptr to commmunication area
HWND             hwnd,               // handle of process window
WINMSG           message,            // message to be processed
WPARAM           mp1,                // first message parameter
LPARAM           mp2                 // second message parameter
);
BOOL ExpSegsFindTag( PSZ_W pszTag, PSZ_W pszData, int *piCharsBefore, int *piCharsAfter );
BOOL ExpSegsWriteExpStart( PEXPORTSEGS_IDA pIda );
BOOL ExpSegsWriteExpEnd( PEXPORTSEGS_IDA pIda );
BOOL ExpSegsWriteExpSegment( PEXPORTSEGS_IDA pIda, ULONG ulSegNum, PSZ_W pszSource, PSZ_W pszTarget );
BOOL ExpSegsFindStartTag( PEXPORTSEGS_IDA pIda, PSZ_W pszData, int *piCharsAfter );
BOOL ExpSegsBrowseForFile( PEXPORTSEGS_IDA pIda, HWND hwnd, PSZ pszTitle, PSZ pszFileBuffer, int iBufSize, BOOL fOpen  );
BOOL ExportSegsExtractTag( PSZ *ppszText, PSZ pszBuffer );


MRESULT TAExportSegsProcCallBack
(
PPROCESSCOMMAREA pCommArea,          // ptr to commmunication area
HWND             hwnd,               // handle of process window
WINMSG           message,            // message to be processed
WPARAM           mp1,                // first message parameter
LPARAM           mp2                 // second message parameter
)
{
  PEXPORTSEGS_IDA      pIda;                // pointer to instance area
  MRESULT          mResult = FALSE;     // return code of procedure

  switch ( message)
  {
    /******************************************************************/
    /* WM_CREATE:                                                     */
    /*                                                                */
    /* Fill fields in communication area                              */
    /* Initialize data of callback function                           */
    /******************************************************************/
    case WM_CREATE :
      /**************************************************************/
      /* Anchor IDA                                                 */
      /**************************************************************/
      pIda                = (PEXPORTSEGS_IDA)PVOIDFROMMP2(mp2);
      pCommArea->pUserIDA = pIda;
      pIda->hwndErrMsg = hwnd;

      /****************************************************************/
      /* supply all information required to create the process        */
      /* window                                                       */
      /****************************************************************/
      pCommArea->sProcessWindowID = ID_EXPORTSEGS_PROC_WINDOW;
      pCommArea->sProcessObjClass = clsANALYSIS;
      pCommArea->Style            = PROCWIN_SLIDERONLY;
      pCommArea->sSliderID        = ID_TASLIDER;
      strcpy( pCommArea->szTitle, "Exporting Segments" );
      pCommArea->hIcon            = (HPOINTER) UtlQueryULong(QL_ANAICON);;
      pCommArea->fNoClose         = FALSE;
      pCommArea->swpSizePos.x     = 100;
      pCommArea->swpSizePos.y     = 100;
      pCommArea->swpSizePos.cx    = (SHORT)UtlQueryULong( QL_AVECHARWIDTH ) * 60;
      pCommArea->swpSizePos.cy    = (SHORT)UtlQueryULong( QL_PELSPERLINE ) * 10;
      pCommArea->asMsgsWanted[0]  = WM_EQF_PROCESSTASK;
      pCommArea->asMsgsWanted[1]  = 0;
      pCommArea->usComplete       = 0;
      break;

      /******************************************************************/
      /* WM_EQF_INITIALIZE:                                             */
      /*                                                                */
      /* Start the process                                              */
      /******************************************************************/
    case WM_EQF_INITIALIZE:
      {
        pIda     = (PEXPORTSEGS_IDA)pCommArea->pUserIDA;

        WinPostMsg( hwnd, WM_EQF_PROCESSTASK, MP1FROMSHORT(EXPORTSEGS_TASK), 0L );
      }
      break;

      /******************************************************************/
      /* WM_CLOSE:                                                      */
      /*                                                                */
      /* Prepare/initialize shutdown of process                         */
      /******************************************************************/
    case WM_CLOSE:
      pIda = (PEXPORTSEGS_IDA)pCommArea->pUserIDA;
      if ( pIda )
      {
        pIda->fKill = TRUE;
        mResult = MRFROMSHORT( TRUE );   // = do not close right now
      }
      else
      {
        mResult = MRFROMSHORT( FALSE );  // = continue with close
      } /* endif */
      break;

      /******************************************************************/
      /* WM_DESTROY:                                                    */
      /*                                                                */
      /* Cleanup all resources used by the process                      */
      /******************************************************************/
    case WM_DESTROY:
      pIda = (PEXPORTSEGS_IDA)pCommArea->pUserIDA;
      if ( pIda )
      {
        if ( pIda->hwndDocLB != NULLHANDLE ) WinDestroyWindow( pIda->hwndDocLB );
        UtlAlloc( (PVOID *) &pIda, 0L, 0L, NOMSG );
        pCommArea->pUserIDA = NULL;
      } /* endif */
      break;

      /******************************************************************/
      /* WM_EQF_TERMINATE:                                              */
      /*                                                                */
      /* Allow or disable termination of process                        */
      /******************************************************************/
    case WM_EQF_TERMINATE:
      mResult = MRFROMSHORT( FALSE );          // = continue with close
      break;

    case WM_INITMENU:
      UtlMenuDisableAll( hwnd, SHORT1FROMMP1( mp1) );
      break;

      /******************************************************************/
      /* WM_EQF_PROCESSTASK:                                            */
      /*                                                                */
      /* Do the actual processing                                       */
      /******************************************************************/
    case WM_EQF_PROCESSTASK:
      if ( SHORT1FROMMP1(mp1) == EXPORTSEGS_TASK )
      {
        pIda = (PEXPORTSEGS_IDA)pCommArea->pUserIDA;
        if ( pIda->fKill )
        {
          USHORT  usMBCode;        // return code of message box call

          usMBCode = UtlError( ERROR_CANCELEXPORTSEGS, MB_YESNO, 0, NULL, EQF_QUERY );
          if ( usMBCode == MBID_YES )
          {
            // cleanup here!
            EqfRemoveObject( TWBFORCE, hwnd );
          }
          else
          {
            pIda->fKill = FALSE;
            WinPostMsg( hwnd, WM_EQF_PROCESSTASK, MP1FROMSHORT(EXPORTSEGS_TASK), 0l );
          } /* endif */
        }
        else
        {
          // process current step
          ExportSegsProccess( pIda );

          /************************************************************/
          /* Prepare the next step                                    */
          /************************************************************/
          if ( pIda->CurStep != END_PROCESS )
          {
            UtlDispatch();
            WinPostMsg( hwnd, WM_EQF_PROCESSTASK, MP1FROMSHORT(EXPORTSEGS_TASK), 0l );

            if ( pIda->usComplete != pIda->usLastComplete )
            {
              WinSendMsg( hwnd, WM_EQF_UPDATESLIDER, MP1FROMSHORT(pIda->usComplete), NULL );
              pIda->usLastComplete = pIda->usComplete;
            } /* endif */
          }
          else
          {
            // set slider to 100%
            if ( !pIda->fErrorStop && !pIda->fBatch )
            {
              WinSendMsg( hwnd, WM_EQF_UPDATESLIDER, MP1FROMSHORT(100), NULL );
            } /* endif */

            // show completion message
            if ( pIda->fErrorStop )
            {
              PSZ pszErrParm = pIda->szOutFile;

              UtlErrorHwnd( ERROR_EXPORTSEGS_TERMINATED, MB_CANCEL, 1, &pszErrParm, EQF_ERROR, pIda->hwndErrMsg );
            }
            else if ( pIda->ulSegments == 0L )
            {
              PSZ pszErrParm = pIda->szOutFile;

              UtlErrorHwnd( ERROR_EXPORTSEGS_NOSEGSFOUND, MB_CANCEL, 1, &pszErrParm, EQF_ERROR, pIda->hwndErrMsg );
            }
            else
            {
              PSZ   apszErrParm[2];
              CHAR  szSegNum[20];

              apszErrParm[0] = pIda->szOutFile;
              ltoa( pIda->ulSegments, szSegNum, 10 );
              apszErrParm[1] = szSegNum;

              UtlErrorHwnd( INFO_EXPORTSEGS_COMPLETED, MB_OK, 2, apszErrParm, EQF_INFO, pIda->hwndErrMsg );
            } /* endif */

            // end process window
            EqfRemoveObject( TWBFORCE, hwnd);
          } /* endif */
        } /* endif */
      } /* endif */
      break;
  } /* endswitch */
  return( mResult );
} /* end of function TAExportSegsProcCallBack */


INT_PTR CALLBACK EXPORTSEGSDLGPROC
(
HWND hwndDlg,                       /* handle of dialog window             */
WINMSG msg,
WPARAM mp1,
LPARAM mp2
)
{
  MRESULT  mResult = MRFROMSHORT( FALSE );      // result value of procedure

  switch ( msg )
  {
    case WM_EQF_QUERYID: HANDLEQUERYID( ID_EXPORTSEGS_DLG, mp2 ); break;

    case WM_INITDLG:
      mResult = ExportSegsInit( hwndDlg, mp1, mp2 );
      break;

    case WM_COMMAND:
      mResult = ExportSegsCommand( hwndDlg, WMCOMMANDID( mp1, mp2 ), WMCOMMANDCMD( mp1, mp2 ) );
      break;

    case WM_CLOSE:
      mResult = ExportSegsClose( hwndDlg, mp1, mp2 );
      break;

    default:
      mResult = WinDefDlgProc( hwndDlg, msg, mp1, mp2 );
      break;
  } /* endswitch */

  return mResult;
} /* end of ExportSegsDLGPROC */

MRESULT ExportSegsInit
(
HWND    hwndDlg,                    // handle of folder export dialog window
WPARAM  mp1,                        // first parameter of WM_INITDLG
LPARAM  mp2                         // second parameter of WM_INITDLG
)
{
  PEXPORTSEGS_IDA  pIda;                  // ptr to archive TM IDA

  mp1 = mp1;                          // suppress 'unreferenced parameter' msg

  //--- adress IDA and store pointer to it ---
  pIda = (PEXPORTSEGS_IDA) mp2;
  if ( pIda )
  {
    ANCHORDLGIDA( hwndDlg, pIda );
    pIda->szOutFile[0] = EOS;
    pIda->szStartStopFile[0] = EOS;
  } /* endif */

  // fill document listbox 
  if ( pIda )
  {
    SHORT i = 0;                      // listbox item index
    HWND     hwndLB = GETHANDLEFROMID( hwndDlg, ID_EXPORTSEGS_DOC_LB );

    pIda->sMaxDocs = QUERYITEMCOUNTHWND( pIda->hwndDocLB );

    while ( i < pIda->sMaxDocs )
    {
      QUERYITEMTEXTHWND( pIda->hwndDocLB, i, pIda->szCurDoc );

      // Get document info
      strcpy( pIda->szDocObjName, pIda->szFolObjName );
      strcat( pIda->szDocObjName, BACKSLASH_STR );
      strcat( pIda->szDocObjName, pIda->szCurDoc );
      pIda->szLongName[0] = EOS;

      DocQueryInfo2( pIda->szDocObjName,   // document object name
                     NULL,                 // memory of document
                     pIda->szDocFormat,    // format of document
                     NULL,                 // document source language
                     NULL,                 // --   target language
                     pIda->szLongName,     // long document name
                     NULL,                 // alias
                     NULL,                 // editor
                     TRUE );               // handle errors in function


      // insert document into our listbox
      if ( pIda->szLongName[0] == EOS )
      {
        INSERTITEMHWND( hwndLB, pIda->szCurDoc );
      }
      else
      {
        OEMTOANSI( pIda->szLongName );
        INSERTITEMHWND( hwndLB, pIda->szLongName );
      } /* endif */

      i++;                      // next document
    } /* endwhile */
  } /* endif */

  {
    HWND hwndLB1 = GetDlgItem(hwndDlg, ID_EXPORTSEGS_DOC_LB);
    UtlSetHorzScrollingForLB(hwndLB1);
  }

  // get last used values from folder properties or use last used values from config file
  {
    ULONG ulErrorInfo = 0;
    HPROP hProp = OpenProperties( pIda->szFolObjName, NULL, PROP_ACCESS_READ, &ulErrorInfo );
    if ( hProp != NULL )
    {
      PPROPFOLDER pProp = (PPROPFOLDER)MakePropPtrFromHnd( hProp );
      if ( pProp != NULL )
      {
        strcpy( pIda->szOutFile, pProp->szLastOutputFile ); 
        strcpy( pIda->szStartStopFile, pProp->szLastStartStopFile ); 
      } /* endif */         
      CloseProperties( hProp, PROP_QUIT, &ulErrorInfo );
    } /* endif */       

    if ( (pIda->szOutFile[0] == EOS) && (pIda->szStartStopFile[0] == EOS) )
    {
      // get values from config file    
      GetProfileString( APPL_Name, "ExpSegsOutFile", "", pIda->szOutFile, sizeof(pIda->szOutFile) );
      GetProfileString( APPL_Name, "ExpSegsStartStopFile", "", pIda->szStartStopFile, sizeof(pIda->szStartStopFile) );
    } /* endif */       

    SETTEXT( hwndDlg, ID_EXPORTSEGS_OUTFILE_EF, pIda->szOutFile );
    SETTEXT( hwndDlg, ID_EXPORTSEGS_STARTSTOP_EF, pIda->szStartStopFile );

  }

  // ensure that dialog is not positioned outside the screen area
  UtlCheckDlgPos( hwndDlg, FALSE );

  return( DIALOGINITRETURN(FALSE) );
} /* end of ExportSegsInit */

MRESULT ExportSegsCommand
(
HWND hwndDlg,                       // dialog handle
SHORT sId,                          // id of button
SHORT sNotification                 // notification type
)
{
  MRESULT mResult = MRFROMSHORT(TRUE);
  PEXPORTSEGS_IDA pIda;                   // ptr to archive TM IDA
  BOOL        fOK;                    // internal OK flag

  sNotification;

  // --- get IDA pointer ---
  pIda = ACCESSDLGIDA( hwndDlg, PEXPORTSEGS_IDA );

  switch ( sId )
  {
    case ID_EXPORTSEGS_HELP_PB:
      UtlInvokeHelp();
      break;

    case ID_EXPORTSEGS_OUTFILE_PB:
      QUERYTEXT( hwndDlg, ID_EXPORTSEGS_OUTFILE_EF, pIda->szOutFile );
      if ( ExpSegsBrowseForFile( pIda, hwndDlg, "Select output file", pIda->szOutFile, sizeof(pIda->szOutFile), FALSE ) )
      {
        SETTEXT( hwndDlg, ID_EXPORTSEGS_OUTFILE_EF, pIda->szOutFile );
      } /* endif */         
      break;

    case ID_EXPORTSEGS_STARTSTOP_PB:
      QUERYTEXT( hwndDlg, ID_EXPORTSEGS_STARTSTOP_EF, pIda->szStartStopFile );
      if ( ExpSegsBrowseForFile( pIda, hwndDlg, "Select file containing list of start/stop tags", pIda->szStartStopFile, sizeof(pIda->szStartStopFile), TRUE ) )
      {
        SETTEXT( hwndDlg, ID_EXPORTSEGS_STARTSTOP_EF, pIda->szStartStopFile );
      } /* endif */         
      break;

    case ID_EXPORTSEGS_START_PB:
      fOK = TRUE;                   // assume everything is o.k.

      // get specified file names
      if ( fOK )
      {
        QUERYTEXT( hwndDlg, ID_EXPORTSEGS_OUTFILE_EF, pIda->szOutFile );
        UtlStripBlanks( pIda->szOutFile );
        QUERYTEXT( hwndDlg, ID_EXPORTSEGS_STARTSTOP_EF, pIda->szStartStopFile );
        UtlStripBlanks( pIda->szStartStopFile );
      } /* endif */

      // check start/stop file
      if ( fOK )
      {
        if ( pIda->szStartStopFile[0] == EOS  )
        {
          UtlError( TQM_MANDATORY_VALUE_MISSING, MB_CANCEL, 0, NULL, EQF_ERROR );
          fOK = FALSE;
          SETFOCUS( hwndDlg, ID_EXPORTSEGS_STARTSTOP_EF );
        } /* endif */           
      } /* endif */

      // check if start/stop file exists
      if ( fOK )
      {
        if ( !UtlFileExist( pIda->szStartStopFile ) )
        {
          PSZ pszErrParm = pIda->szStartStopFile;
          UtlError( QDPR_NO_VALID_FILE_SELECTED, MB_CANCEL, 1, &pszErrParm, EQF_ERROR );
          fOK = FALSE;
          SETFOCUS( hwndDlg, ID_EXPORTSEGS_STARTSTOP_EF );
        } /* endif */           
      } /* endif */

      // check if output file has been specified
      if ( fOK )
      {
        if ( pIda->szOutFile[0] == EOS  )
        {
          UtlError( TQM_MANDATORY_VALUE_MISSING, MB_CANCEL, 0, NULL, EQF_ERROR );
          fOK = FALSE;
          SETFOCUS( hwndDlg, ID_EXPORTSEGS_OUTFILE_EF );
        } /* endif */           
      } /* endif */

      // get overwrite confirmation when output file exists
      if ( fOK )
      {
        if ( UtlFileExist( pIda->szOutFile ) )
        {
          PSZ pszErrParm = pIda->szOutFile;
          if ( UtlError( ERROR_FILE_EXISTS_ALREADY, MB_YESNOCANCEL | MB_DEFBUTTON2, 1, &pszErrParm, EQF_QUERY ) != MBID_YES )
          {
            fOK = FALSE;
            SETFOCUS( hwndDlg, ID_EXPORTSEGS_STARTSTOP_EF );
          } /* endif */             
        } /* endif */           
      } /* endif */         

      // save last used values to folder properties
      if ( fOK )
      {
        ULONG ulErrorInfo = 0;
        HPROP hProp = OpenProperties( pIda->szFolObjName, NULL, PROP_ACCESS_READ, &ulErrorInfo );
        if ( hProp != NULL )
        {
          if ( SetPropAccess( hProp, PROP_ACCESS_WRITE) )
          {
            PPROPFOLDER pProp = (PPROPFOLDER)MakePropPtrFromHnd( hProp );
            strcpy( pProp->szLastOutputFile, pIda->szOutFile );
            strcpy( pProp->szLastStartStopFile, pIda->szStartStopFile );
            SaveProperties( hProp, &ulErrorInfo );
            ResetPropAccess( hProp, PROP_ACCESS_WRITE );
          } /* endif */             
          CloseProperties( hProp, PROP_QUIT, &ulErrorInfo );
        } /* endif */       

        WinSetProfileString( hAB, APPL_Name, "ExpSegsOutFile", pIda->szOutFile );
        WinSetProfileString( hAB, APPL_Name, "ExpSegsStartStopFile", pIda->szStartStopFile );
      } /* endif */         

      // end dialog and start export segments process if O.K.
      if ( fOK )
      {
        POSTCLOSE( hwndDlg, TRUE );
      } /* endif */
      break;

    case ID_EXPORTSEGS_CANCEL_PB:
    case DID_CANCEL:
      POSTCLOSE( hwndDlg, 0 );
      break;
  } /* endswitch */

  return( mResult );
} /* end of ExportSegsCommand */

MRESULT ExportSegsControl
(
HWND hwndDlg,                       // dialog handle
SHORT sId,                          // id of button
SHORT sNotification                 // notification type
)
{
  MRESULT mResult = MRFROMSHORT(TRUE);
  PEXPORTSEGS_IDA pIda;                   // ptr to archive TM IDA

    sNotification; sId;

  // --- get IDA pointer ---
  pIda = ACCESSDLGIDA( hwndDlg, PEXPORTSEGS_IDA );

  //switch ( sId )
  //{
    //case ID_EXPORTSEGS_TOTM_COMBO:
    //  if ( sNotification == CBN_EFCHANGE )
    //  {
    //    QUERYTEXT( hwndDlg, ID_EXPORTSEGS_TOTM_COMBO, pIda->szExportSegs );
    //    UtlStripBlanks( pIda->szExportSegs );
    //    ENABLECTRL( hwndDlg, ID_EXPORTSEGS_START_PB, (pIda->szExportSegs[0] != EOS) );
    //  }
    //  else if ( sNotification == CBN_SELCHANGE )
    //  {
    //    SHORT sItem;

    //    CBQUERYSELECTEDITEMTEXT( sItem, hwndDlg, ID_EXPORTSEGS_TOTM_COMBO,
    //                             pIda->szExportSegs );
    //    UtlStripBlanks( pIda->szExportSegs );
    //    ENABLECTRL( hwndDlg, ID_EXPORTSEGS_START_PB, (pIda->szExportSegs[0] != EOS) );
    //  } /* endif */
    //  break;
  //} /* endswitch */

  return( mResult );
} /* end of ExportSegsControl */

MRESULT ExportSegsClose
(
HWND hwndDlg,
WPARAM mp1,
LPARAM mp2
)
{
  MRESULT mResult = FALSE;
  PEXPORTSEGS_IDA pIda;                   // ptr to folder export IDA

  mp2 = mp2;                          // supress compiler warning

  // get IDA pointer
  pIda = ACCESSDLGIDA( hwndDlg, PEXPORTSEGS_IDA );

  if ( pIda )
  {
  } /* endif */

  //--- get rid off archive TM dialog ---
  WinDismissDlg( hwndDlg, SHORT1FROMMP1(mp1) );

  return( mResult );
} /* end of ExportSegsClose */


VOID ExportSegs( HWND hwnd, PSZ pSelFolderName )
{
  PEXPORTSEGS_IDA     pIda = NULL;        // archive TM IDA
  BOOL            fOK = TRUE;         // return value

  hwnd;

  // create export segments IDA
  fOK = UtlAlloc( (PVOID *)&pIda, 0L, (LONG) sizeof(EXPORTSEGS_IDA), ERROR_STORAGE );

  // create document listbox
  if ( fOK )
  {
    strcpy( pIda->szParentObjName, pSelFolderName );
    strcpy( pIda->szFolObjName, pSelFolderName );
    if ( FolIsSubFolderObject( pIda->szFolObjName ) )
    {
      // get main folder object name to allow the remaining code to work
      // without changes
      UtlSplitFnameFromPath( pIda->szFolObjName ); // cut off subfolder name
      UtlSplitFnameFromPath( pIda->szFolObjName ); // cut off property directory
    } /* endif */
    strcpy( pIda->szFolder, UtlGetFnameFromPath( pIda->szFolObjName ) );
    Utlstrccpy( pIda->szFolName, pIda->szFolder, DOT );




    pIda->hwndDocLB = WinCreateWindow( hwnd, WC_LISTBOX, "", WS_CHILDWINDOW | LBS_STANDARD, 0, 0, 10, 10, hwnd, 
                                       HWND_TOP, ID_TATODOLB, NULL, NULL);
    fOK = pIda->hwndDocLB != NULLHANDLE;
  } /* endif */

  // fill document listbox with document names
  if ( fOK )
  {
    USHORT         usRc;        // return code
    HWND           hwndActive;  // handle of active window

    //get Active folder and fill listbox with selected documents
    hwndActive = EqfQueryActiveFolderHwnd();

    //get status of active folder
    usRc = EqfQueryObjectStatus( hwndActive );
    if ( usRc & OBJ_FOCUS )             //documents selected in folder
    {
      EqfSend2Handler( FOLDERHANDLER, WM_EQF_QUERYSELECTEDNAMES,
                       MP1FROMHWND(pIda->hwndDocLB),
                       MP2FROMP(pIda->szParentObjName) );
    }
    else
    {
      EqfSend2Handler( FOLDERHANDLER, WM_EQF_INSERTNAMES,
                       MP1FROMHWND(pIda->hwndDocLB),
                       MP2FROMP(pIda->szFolObjName) );
    } /* endif */
  } /* endif */

  // show export segments dialog
  if ( fOK )
  {
    HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);    
    DIALOGBOX( EqfQueryTwbClient(), EXPORTSEGSDLGPROC, hResMod, ID_EXPORTSEGS_DLG, pIda, fOK );
  } /* endif */

  // start export segments process
  if ( fOK )
  {
    strcpy( pIda->szObjName, "ExportSegs:" );
    strcat( pIda->szObjName, pIda->szOutFile );
    fOK = CreateProcessWindow( pIda->szObjName, TAExportSegsProcCallBack, pIda );
  } /* endif */

  // cleanup
  if ( !fOK )
  {
    if ( pIda != NULL )
    {
      if ( pIda->hwndDocLB != NULLHANDLE ) WinDestroyWindow( pIda->hwndDocLB );
      UtlAlloc( (PVOID *)&(pIda), 0L, 0L, NOMSG) ;
    } /* endif */
  } /* endif */
}

USHORT TAFuncExportSegs
(
PFCTDATA    pData,                   // function I/F session data
PSZ         pszFolderName,           // name of folder
PSZ         pszDocuments,            // list with document names or NULL
PSZ         pszStartStopFile,        // file containing start/stop tag list
PSZ         pszOutFile,              // name of output file
LONG        lOptions                 // options for analysis
)
{
  USHORT      usRC = NO_ERROR;         // function return code

  // prepare a new analysis run or continue current one
  if ( pData->fComplete )              // has last run been completed
  {
    // prepare a new archive TM run
    usRC = TAFuncPrepExportSegs( pData, pszFolderName, pszDocuments, pszStartStopFile, pszOutFile,  lOptions );
  }
  else
  {
    // continue current archive TM process
    usRC = TAFuncExportSegsProcess( pData );
  } /* endif */
  return( usRC );
} /* end of function TAFuncExportSegs */

// prepare the function I/F archive TM
USHORT TAFuncPrepExportSegs
(
  PFCTDATA    pData,                   // function I/F session data
  PSZ         pszFolderName,           // name of folder
  PSZ         pszDocuments,            // list with document names or NULL
  PSZ         pszStartStopFile,        // file containing start/stop tag list
  PSZ         pszOutFile,              // name of output file
  LONG        lOptions                 // options for archive TM
)
{
  PSZ         pszParm;                 // pointer for error parameters
  BOOL        fOK = TRUE;              // internal O.K. flag
  USHORT      usRC = NO_ERROR;         // function return code
  USHORT      usDocuments = 0;         // number of documents being analyzed
  PSZ         pDocNameBuffer = NULL;   // document name buffer
  LONG        lBufferSize = 0L;        // size of document buffer
  LONG        lBufferUsed = 0L;        // used bytes in document buffer
  OBJNAME     szFolObject;             // folder object name
  PEXPORTSEGS_IDA pIda = NULL;
  CHAR        szFolShortName[MAX_FILESPEC];// buffer for folder short name
  CHAR        szFolderName[MAX_LONGFILESPEC];
  CHAR        szDocuments[MAX_LONGFILESPEC];

  lOptions;

  // check if folder exists
  if ( fOK )
  {
    if ( (pszFolderName == NULL) || (*pszFolderName == EOS) )
    {
      fOK = FALSE;
      usRC = TA_MANDFOLDER;
      UtlErrorHwnd( usRC, MB_CANCEL, 0, NULL, EQF_ERROR, pData->hwndErrMsg );
    }
    else
    {
      BOOL fIsNew;
      strcpy( szFolderName, pszFolderName);

      fIsNew = !SubFolNameToObjectName( szFolderName,  pData->szObjName );

      if ( fOK)
      {
        if ( !fIsNew )
        {
          PSZ pszDelim;
          pszDelim = strchr( szFolderName, BACKSLASH );
          if ( pszDelim ) *pszDelim = EOS;
          ObjLongToShortName( szFolderName, szFolShortName, FOLDER_OBJECT, &fIsNew );
          if ( pszDelim ) *pszDelim = BACKSLASH;
        } /* endif */

        if ( fIsNew )
        {
          fOK = FALSE;
          pszParm = pszFolderName;
          usRC = ERROR_XLATE_FOLDER_NOT_EXIST;
          UtlErrorHwnd( usRC, MB_CANCEL, 1, &pszParm, EQF_ERROR, HWND_FUNCIF );
        } /* endif */
      }
    } /* endif */
  } /* endif */

  // check if a start/stop file has been specified
  if ( fOK )
  {
    if ( (pszStartStopFile == NULL) || (*pszStartStopFile == EOS) )
    {
      fOK = FALSE;
      UtlErrorHwnd( TMT_MANDCMDLINE, MB_CANCEL, 0, NULL, EQF_ERROR, pData->hwndErrMsg );
    } /* endif */
  } /* endif */

  // check if a output file has been specified
  if ( fOK )
  {
    if ( (pszOutFile == NULL) || (*pszOutFile == EOS) )
    {
      fOK = FALSE;
      UtlErrorHwnd( TMT_MANDCMDLINE, MB_CANCEL, 0, NULL, EQF_ERROR, pData->hwndErrMsg );
    } /* endif */
  } /* endif */

  // check if documents exist
  if ( fOK && (pszDocuments != NULL) && (*pszDocuments != EOS))
  {
    PSZ    pszTemp = pszDocuments;    // ptr for document list processing
    PSZ    pszDocNameStart;           // ptr for document list processing
    CHAR   chTemp;                    // buffer for current character

    // build folder object name (access to folder properties is
    // required to correct folder drive letter)
    {
      PPROPFOLDER  ppropFolder;        // pointer to folder properties
      HPROP        hpropFolder;        // folder properties handle
      ULONG        ulErrorInfo;        // error indicator from property handler

      UtlMakeEQFPath( szFolObject, pData->szObjName[0], SYSTEM_PATH, NULL );
      strcat( szFolObject, BACKSLASH_STR );
      strcat( szFolObject, szFolShortName );
      strcat( szFolObject, EXT_FOLDER_MAIN );
      hpropFolder = OpenProperties( szFolObject, NULL,
                                    PROP_ACCESS_READ, &ulErrorInfo);
      if ( hpropFolder )
      {
        ppropFolder = (PPROPFOLDER)MakePropPtrFromHnd( hpropFolder );
        if ( (ppropFolder->chDrive != EOS) && (ppropFolder->chDrive != ' ') )
        {
          szFolObject[0] = ppropFolder->chDrive;
        } /* endif */
        CloseProperties( hpropFolder, PROP_QUIT, &ulErrorInfo );
      } /* endif */
    }

    // isolate current document name
    strcpy(szDocuments, pszDocuments);
    pszDocNameStart = szDocuments;
    while ( fOK && (*pszDocNameStart != EOS) )
    {
      BOOL fIsNew = FALSE;

      // isolate current document name
      {
        // skip leading whitespace and seperators
        while ( (*pszDocNameStart == ' ') || (*pszDocNameStart == COMMA) )
        {
          pszDocNameStart++;
        } /* endwhile */

        // find end of document name
        pszTemp = pszDocNameStart;
        while ( *pszTemp && (*pszTemp != ' ') && (*pszTemp != COMMA) )
        {
          pszTemp++;
        } /* endwhile */
        chTemp = *pszTemp;
        *pszTemp = EOS;
      }

      if ( *pszDocNameStart != EOS)
      {
        CHAR szDocShortName[MAX_FILESPEC];

        FolLongToShortDocName( szFolObject, pszDocNameStart,
                               szDocShortName, &fIsNew );
        if ( fIsNew )
        {
          PSZ pszParms[2];
          fOK = FALSE;
          pszParms[0] = pszDocNameStart;
          pszParms[1] = pszFolderName;
          usRC = DDE_DOC_NOT_IN_FOLDR;
          UtlErrorHwnd( usRC, MB_CANCEL, 2, pszParms, EQF_ERROR,
                        pData->hwndErrMsg );
        }
        else
        {
          // add document short name to document name buffer
          LONG lAddLen = strlen(szDocShortName) + 1;
          if ( lBufferSize < (lBufferUsed + lAddLen) )
          {
            UtlAllocHwnd( (PVOID *)&pDocNameBuffer, lBufferSize,
                          lBufferSize + 8096L, ERROR_STORAGE,
                          pData->hwndErrMsg );
            lBufferSize += 8096L;
          } /* endif */

          if ( pDocNameBuffer != NULL )
          {
            strcpy( pDocNameBuffer + lBufferUsed, szDocShortName );
            lBufferUsed += lAddLen;
          } /* endif */
          usDocuments++;
        } /* endif */
      } /* endif */

      // next document name
      *pszTemp = chTemp;
      pszDocNameStart = pszTemp;
    } /* endwhile */
  } /* endif */

  // get number of documents if none specific documents have been specified
  if ( fOK )
  {
    if ( usDocuments == 0 )
    {
      usDocuments = LoadDocumentNames( pData->szObjName, HWND_FUNCIF,
                                       LOADDOCNAMES_INCLSUBFOLDERS,
                                       (PSZ)&pDocNameBuffer );
    } /* endif */
  } /* endif */

  // allocate storage for export segments IDA
  if ( fOK )
  {
    fOK = UtlAlloc( (PVOID *)&pIda, 0L, (LONG)sizeof(EXPORTSEGS_IDA), NOMSG );
    if ( !fOK )
    {
      usRC = ERROR_STORAGE;
      UtlErrorHwnd( usRC, MB_CANCEL, 0, NULL, EQF_ERROR, HWND_FUNCIF );
    } /* endif */
  } /* endif */

  // prepare IDA fields
  if ( fOK )
  {
    strcpy( pIda->szOutFile, pszOutFile );
    strcpy( pIda->szStartStopFile, pszStartStopFile );
    pIda->hwndErrMsg       = pData->hwndErrMsg;
    pIda->fBatch = TRUE;
    pIda->pszDocNames = pDocNameBuffer;
    strcpy( pIda->szFolObjName, pData->szObjName );
    strcpy( pIda->szFolder,     UtlGetFnameFromPath(pData->szObjName) );
    strcpy( pIda->szFolLongName, szFolderName );


    // check if folder is locked and lock it if not locked yet
    if ( fOK ) 
    {
      if ( QUERYSYMBOL( pIda->szFolObjName ) != -1 )
      {
        fOK = FALSE;
        pszParm = szFolderName;
        usRC = ERROR_FOLDER_LOCKED;
        UtlErrorHwnd( ERROR_FOLDER_LOCKED, MB_CANCEL, 1, &pszParm, EQF_ERROR, HWND_FUNCIF );
      }
      else
      {
        SETSYMBOL( pIda->szFolObjName );
        pIda->fFolderLocked = TRUE;
      } /* endif */
    } /* endif */
  } /* endif */

  // Cleanup
  if ( !fOK )
  {
    if ( pIda != NULL )
    {
      if ( pIda->fFolderLocked ) REMOVESYMBOL( pIda->szFolObjName );
      UtlAlloc( (PVOID *)&pIda->pszDocNames, 0L, 0L, NOMSG) ;
      UtlAlloc( (PVOID *)&pIda, 0L, 0L, NOMSG) ;
    } /* endif */
  } /* endif */

  if ( fOK )
  {
    pData->fComplete = FALSE;
    pIda->CurStep = PROCESS_INIT;
    pData->pvExportSegsIda = (PVOID)pIda;
    pIda->sMaxDocs = usDocuments;
  }
  else
  {
    usRC = UtlQueryUShort( QS_LASTERRORMSGID );
  } /* endif */
  return( usRC );

} /* end of function TAFuncPrepExportSegs */

//----------------------------------------------------------------------------
// TaFuncExportSegsProcess: Archive TM Function Of The Function Call Interface
//----------------------------------------------------------------------------
USHORT TAFuncExportSegsProcess
(
PFCTDATA    pData                    // ptr to function interface data area
)
{
  PEXPORTSEGS_IDA      pIda;                // pointer to instance area
  USHORT     usRC = NO_ERROR;         // function return code

  pIda = (PEXPORTSEGS_IDA)pData->pvExportSegsIda;

  UtlSetUShort( QS_LASTERRORMSGID, 0 );

  // handle the current processing step

  ExportSegsProccess( pIda );

  if ( pIda->fErrorStop )
  {
    usRC = UtlQueryUShort( QS_LASTERRORMSGID );
  } /* endif */

  if ( pIda->CurStep == END_PROCESS )
  {
    if ( pData->hwndErrMsg == HWND_FUNCIF )    // if we are in normal function call mode ...
    {
      // ... free our internal memory areas
      UtlAlloc( (PVOID *)&pIda->pszDocNames, 0L, 0L, NOMSG );    // free instance data space
      UtlAlloc( (PVOID *)&pIda, 0L, 0L, NOMSG );    // free instance data space
    } /* endif */
    pData->fComplete = TRUE;
  } /* endif */

  return( usRC );
} /* end of function TAFuncExportSegsProcess */


// do the export segments processing
USHORT ExportSegsProccess
(
  PEXPORTSEGS_IDA      pIda                 // pointer to instance area
)
{
  USHORT usRC = 0;

  // handle the current processing step
  switch ( pIda->CurStep )
  {
    case PROCESS_INIT :
      /********************************************************/
      /* Initialize the process                               */
      /*                                                      */
      /* Prepare next step                                    */
      /********************************************************/
      {
        BOOL fOK = TRUE;       // O.K. flag

        // load start/stop file
        if ( fOK )
        {
          fOK = ExpLoadStartStopFile( pIda );
        } /* endif */           

        // open output file
        if ( fOK )
        {
          pIda->hOutFile = fopen( pIda->szOutFile, "wb" );
          if ( pIda->hOutFile == NULL )
          {
            PSZ pszErrParm = pIda->szOutFile;

            UtlErrorHwnd( QDPR_NO_VALID_FILE_SELECTED, MB_CANCEL, 1, &pszErrParm, EQF_ERROR, pIda->hwndErrMsg );

            fOK = FALSE;
          }
          else
          {
            ExpSegsWriteExpStart( pIda );
          } /* endif */             
        } /* endif */

        // load QF tag table
        if ( fOK )
        {
          USHORT usRC;

          usRC = TALoadTagTableHwnd( QFTAG_TABLE, &(pIda->pLoadedQFTable), TRUE, TRUE, pIda->hwndErrMsg );
          fOK = (usRC == NO_ERROR);
        } /* endif */

        // prepare next processing step
        if ( fOK )
        {
          pIda->sCurDoc  = -1;
          pIda->CurStep = NEXT_DOCUMENT;
        }
        else
        {
          pIda->CurStep = PROCESS_COMPLETE;
          pIda->fErrorStop = TRUE;
        } /* endif */
      }
      break;

    case NEXT_DOCUMENT :
      /********************************************************/
      /* Position to next document, check if document has     */
      /* analyzed                                             */
      /********************************************************/
      pIda->sCurDoc++;
      if ( pIda->sCurDoc < pIda->sMaxDocs )
      {
        // Check existence of segmented source document (may not be there if document has not been analyzed yet)

        // get name of active document
        if ( pIda->fBatch )
        {
          int i = pIda->sCurDoc;
          PSZ pszCurDoc = pIda->pszDocNames;
          while ( i )
          {
            pszCurDoc += strlen(pszCurDoc) + 1;
            i--;
          } /* endwhile */
          strcpy( pIda->szCurDoc, pszCurDoc );
        } 
        else
        {
          QUERYITEMTEXTHWND( pIda->hwndDocLB, pIda->sCurDoc, pIda->szCurDoc );
        } /* endif */

        if ( FolIsSubFolderObject( pIda->szFolObjName ) )
        {
          // get main folder object name to allow the remaining code to work
          // without changes
          UtlSplitFnameFromPath( pIda->szFolObjName ); // cut off subfolder name
          UtlSplitFnameFromPath( pIda->szFolObjName ); // cut off property directory
        } /* endif */
        strcpy( pIda->szFolder, UtlGetFnameFromPath( pIda->szFolObjName ) );


        UtlMakeEQFPath( pIda->szSourceDocName, pIda->szFolObjName[0],
                        DIRSEGSOURCEDOC_PATH, pIda->szFolder );
        strcat( pIda->szSourceDocName, BACKSLASH_STR );
        strcat( pIda->szSourceDocName, pIda->szCurDoc );

        // check if file exists
        if ( UtlFileExist(pIda->szSourceDocName) )
        {
          // continue with this document
          pIda->CurStep = START_DOCUMENT;
        }
        else
        {
          // try next document
          pIda->CurStep = NEXT_DOCUMENT;

          // update slider position (actual slider update is done at
          // the end of the processing switch)
          pIda->usComplete = (USHORT)((LONG)pIda->sCurDoc * 100L / (LONG)pIda->sMaxDocs);
        } /* endif */
      }
      else
      {
        pIda->CurStep = PROCESS_COMPLETE;
      } /* endif */
      break;

    case START_DOCUMENT :
      /********************************************************/
      /* Load document                                        */
      /********************************************************/
      {
        BOOL fOK = TRUE;       // O.K. flag

        UtlMakeEQFPath( pIda->szDocObjName, pIda->szFolObjName[0], SYSTEM_PATH, pIda->szFolder );
        strcat( pIda->szDocObjName, BACKSLASH_STR );
        strcat( pIda->szDocObjName, pIda->szCurDoc );

        DocQueryInfo2Hwnd( pIda->szDocObjName, NULL, pIda->szDocFormat,  pIda->szDocSourceLang,
                           pIda->szDocTargetLang, pIda->szLongName, NULL, NULL, TRUE, pIda->hwndErrMsg );   

        {
          // allocate structure for segmented source document
          fOK = UtlAllocHwnd( (PVOID *) &pIda->pSourceDoc, 0L, sizeof(TBDOCUMENT), ERROR_STORAGE, pIda->hwndErrMsg );

          // load document tag table
          if ( fOK )
          {
            fOK = (TALoadTagTableExHwnd( pIda->szDocFormat, (PLOADEDTABLE *)&(pIda->pSourceDoc->pDocTagTable),
                                         FALSE, TALOADUSEREXIT | TALOADGETSEGCONTEXTFUNC, TRUE, pIda->hwndErrMsg  ) == NO_ERROR);
          } /* endif */

          // load segmented source document
          if ( fOK )
          {

            pIda->pSourceDoc->pQFTagTable = pIda->pLoadedQFTable;
            UtlMakeEQFPath( pIda->szSourceDocName, pIda->szFolObjName[0],
                            DIRSEGSOURCEDOC_PATH, pIda->szFolder );
            strcat( pIda->szSourceDocName, BACKSLASH_STR );
            strcat( pIda->szSourceDocName, pIda->szCurDoc );


            // set TBDOCUMENT ulOEMCodePage/ulAnsiCodePage acc. to SrcLang
            pIda->pSourceDoc->ulOemCodePage = GetLangCodePage(OEM_CP, pIda->szDocSourceLang);
            pIda->pSourceDoc->ulAnsiCodePage = GetLangCodePage(ANSI_CP, pIda->szDocSourceLang);

            fOK = EQFBFileReadExW( pIda->szSourceDocName, pIda->pSourceDoc, FILEREAD_METADATA  ) == NO_ERROR;
          } /* endif */

          // allocate structure for segmented target document
          if ( fOK )
          {
            fOK = UtlAllocHwnd( (PVOID *) &pIda->pTargetDoc, 0L, sizeof(TBDOCUMENT), ERROR_STORAGE, pIda->hwndErrMsg );
          } /* endif */

          // load segmented target document
          if ( fOK )
          {
            pIda->pTargetDoc->pQFTagTable = pIda->pLoadedQFTable;
            pIda->pTargetDoc->docType = STARGET_DOC;
            UtlMakeEQFPath( pIda->szTargetDocName, pIda->szFolObjName[0],
                            DIRSEGTARGETDOC_PATH, pIda->szFolder );
            strcat( pIda->szTargetDocName, BACKSLASH_STR );
            strcat( pIda->szTargetDocName, pIda->szCurDoc );
            strcpy( pIda->pTargetDoc->szDocName, pIda->szTargetDocName );


            // set TBDOCUMENT ulOEMCodePage/ulAnsiCodePage acc. to SrcLang
            pIda->pTargetDoc->ulOemCodePage = GetLangCodePage(OEM_CP, pIda->szDocTargetLang);
            pIda->pTargetDoc->ulAnsiCodePage = GetLangCodePage(ANSI_CP, pIda->szDocTargetLang);

            fOK = EQFBFileReadExW( pIda->szTargetDocName, pIda->pTargetDoc, FILEREAD_METADATA  ) == NO_ERROR;
          } /* endif */

          // determine next processing step
          if ( fOK )
          {
            pIda->ulSegNum = 1;
            pIda->ulAddSegNum = 1;
            pIda->ulActiveTable = STANDARDTABLE;
            pIda->CurStep = PROCESS_DOCUMENT;
          }
          else
          {
            pIda->fErrorStop = TRUE;
            pIda->CurStep = PROCESS_COMPLETE;
          } /* endif */
        }

      }
      break;

    case PROCESS_DOCUMENT :
      /********************************************************/
      /* Work on loaded document                              */
      /********************************************************/
      {
        BOOL fOK = TRUE;       // O.K. flag
        int  i = 20;           // segment counter
        PTBSEGMENT pSourceSeg; // pointer to source segment
        PTBSEGMENT pTargetSeg; // pointer to target segment

        // process next 20 segments
        while ( fOK && (i > 0) && (pIda->ulSegNum <= pIda->pSourceDoc->ulMaxSeg) )
        {
          // get segment pointers
          ULONG ulTempTable, ulTempSeg, ulTempAddSeg, ulCurrentSegNum;

          ulTempTable  = pIda->ulActiveTable;
          ulTempSeg    = pIda->ulSegNum;
          ulTempAddSeg = pIda->ulAddSegNum;
          ulCurrentSegNum = pIda->ulSegNum;

          // get segment pointers
          pSourceSeg = EQFBGetFromBothTables( pIda->pSourceDoc, &(pIda->ulSegNum), &(pIda->ulAddSegNum), &(pIda->ulActiveTable));
          pTargetSeg = EQFBGetFromBothTables( pIda->pTargetDoc, &ulTempSeg, &ulTempAddSeg, &ulTempTable );

          // GQ: fix for P031940: ensure that source and target segments are in-sync
          if ( (pSourceSeg != NULL) && (pTargetSeg != NULL) )
          {
            if ( pTargetSeg->ulSegNum != pSourceSeg->ulSegNum )
            {
              pSourceSeg = EQFBGetSeg( pIda->pSourceDoc, pTargetSeg->ulSegNum );
            } /* endif */

            if ( (pSourceSeg == NULL) || (pTargetSeg->ulSegNum != pSourceSeg->ulSegNum) )
            {
              // no matching segment found...
              int iError = 1;             // dummy statement to have a breakpoint location
              iError += 1;
            }
          } /* endif */             

          if ( (pSourceSeg != NULL) && (pTargetSeg != NULL) )
          {
            if ( pIda->fInTagGroup )
            {
              int iCharsBeforeTag = 0;

              if ( ExpSegsFindTag( pIda->szEndTag, pSourceSeg->pDataW, &iCharsBeforeTag, NULL ) )
              {
                pIda->fInTagGroup = FALSE;
                if ( iCharsBeforeTag != 0 )
                {
                  fOK = ExpSegsWriteExpSegment( pIda, pTargetSeg->ulSegNum, pSourceSeg->pDataW, pTargetSeg->pDataW );
                } /* endif */                   
              }
              else
              {
                fOK = ExpSegsWriteExpSegment( pIda, pTargetSeg->ulSegNum, pSourceSeg->pDataW, pTargetSeg->pDataW );
              } /* endif */               
            }
            else
            {
              int iCharsAfterTag = 0;
              if ( ExpSegsFindStartTag( pIda, pSourceSeg->pDataW, &iCharsAfterTag ) )
              {
                pIda->fInTagGroup = TRUE;
                if ( iCharsAfterTag != 0 )
                {
                  fOK = ExpSegsWriteExpSegment( pIda, pTargetSeg->ulSegNum, pSourceSeg->pDataW, pTargetSeg->pDataW );
                } /* endif */                   
              } /* endif */
            } /* endif */               
          } /* endif */

          // continue with next segment
          i--;
        } /* endwhile */

        // update slider position (actual slider update is done at
        // the end of the processing switch)
        if ( fOK )
        {
          LONG  lPosInDoc, lSlotPerDoc, lPosOfDoc;

          lPosInDoc = (LONG)pIda->ulSegNum * 100L / (LONG)pIda->pSourceDoc->ulMaxSeg;
          lSlotPerDoc = 100L / (LONG)pIda->sMaxDocs; 
          lPosOfDoc   = (LONG)pIda->sCurDoc * 100L / (LONG)pIda->sMaxDocs;
          pIda->usComplete = (USHORT)(lPosOfDoc + (lPosInDoc * lSlotPerDoc / 100L));
        } /* endif */

        // set next processing step
        if ( !fOK )
        {
          pIda->CurStep = PROCESS_COMPLETE;
          pIda->fErrorStop = TRUE;
        }
        else if ( pIda->ulSegNum > pIda->pSourceDoc->ulMaxSeg )
        {
          pIda->CurStep = END_DOCUMENT;
        }
        else
        {
          pIda->CurStep = PROCESS_DOCUMENT;
        } /* endif */
      }
      break;

    case END_DOCUMENT :
      /********************************************************/
      /* Terminate current document                           */
      /********************************************************/

      // free source document
      if ( pIda->pSourceDoc )
      {
        if ( pIda->pSourceDoc->pDocTagTable ) TAFreeTagTable( (PLOADEDTABLE)pIda->pSourceDoc->pDocTagTable );
        SegFileFreeDoc( (PVOID *)&(pIda->pSourceDoc) );
        pIda->pSourceDoc = NULL;
      } /* endif */

      // free target document
      if ( pIda->pTargetDoc )
      {
        SegFileFreeDoc( (PVOID *)&(pIda->pTargetDoc) );
        pIda->pTargetDoc = NULL;
      } /* endif */

      pIda->CurStep = NEXT_DOCUMENT;
      break;

    case PROCESS_COMPLETE :
      /********************************************************/
      /* Do some cleanup here                                 */
      /********************************************************/
      if ( pIda->hOutFile != NULLHANDLE )
      {
        ExpSegsWriteExpEnd( pIda );
        fclose( pIda->hOutFile );
        pIda->hOutFile = NULL;
      } /* endif */

      if ( pIda->pSourceDoc )
      {
        if ( pIda->pSourceDoc->pDocTagTable ) TAFreeTagTable( (PLOADEDTABLE)pIda->pSourceDoc->pDocTagTable  );
        SegFileFreeDoc( (PVOID *)&(pIda->pSourceDoc) );
        pIda->pSourceDoc = NULL;
      } /* endif */

      if ( pIda->pTargetDoc )
      {
        SegFileFreeDoc( (PVOID *)&(pIda->pTargetDoc) );
        pIda->pTargetDoc = NULL;
      } /* endif */

      if ( pIda->pLoadedQFTable )
      {
        TAFreeTagTable( pIda->pLoadedQFTable );
      } /* endif */

      if ( pIda->pStartStop )
      {
        UtlAlloc( (PVOID *)&(pIda->pStartStop), 0, 0, NOMSG );
      } /* endif */

      pIda->CurStep = END_PROCESS;
      break;

    default:
      break;
  } /* endswitch */

  if ( pIda->fErrorStop )
  {
    usRC = UtlQueryUShort( QS_LASTERRORMSGID );
  } /* endif */
  return( usRC );
} /* end of ExportSegsProccess*/

// write end sequence to output file
BOOL ExpSegsWriteExpEnd( PEXPORTSEGS_IDA pIda )
{
  fputws( L"</NTMMemoryDb>\r\n", pIda->hOutFile );
  return( TRUE );
}

// write start sequence to output file
BOOL ExpSegsWriteExpStart( PEXPORTSEGS_IDA pIda )
{
  fwrite( UNICODEFILEPREFIX, 1, 2, pIda->hOutFile );
  fputws( L"<NTMMemoryDb>\r\n", pIda->hOutFile );
  fputws( L"<Description>\r\n", pIda->hOutFile  );
  fputws( L"Memory generated by segment export utility\r\n", pIda->hOutFile  );
  fputws( L"\r\n</Description>\r\n", pIda->hOutFile  );
  fputws( L"<Codepage>UTF16</Codepage>\r\n", pIda->hOutFile  );

  return( TRUE );
}

// write segment in EXP format to output file
BOOL ExpSegsWriteExpSegment( PEXPORTSEGS_IDA pIda, ULONG ulSegNum, PSZ_W pszSource, PSZ_W pszTarget )
{
  LONG lTime = 0; 


  // write segment
  pIda->ulSegments++;
  UtlTime( &lTime );
  fwprintf( pIda->hOutFile, L"<Segment>%10.10ld\r\n", pIda->ulSegments );
  fwprintf( pIda->hOutFile,
            L"%s%06lu%s%u%s%016lu%s%S%s%S%s%S%s%S%s%S%s%S%s",
            L"<Control>\r\n",
            ulSegNum,
            X15_STRW,
            0,
            X15_STRW,
            lTime,
            X15_STRW,
            pIda->szDocSourceLang,
            X15_STRW,
            pIda->szDocTargetLang,
            X15_STRW,
            "",
            X15_STRW,
            pIda->szDocFormat,
            X15_STRW,
            UtlGetFnameFromPath( pIda->szSourceDocName ),
            X15_STRW,
            pIda->szLongName,
            L"\r\n</Control>\r\n");
  fputws( L"<Source>", pIda->hOutFile );
  fputws( pszSource, pIda->hOutFile );
  fputws( L"</Source>\r\n",  pIda->hOutFile );
  fputws( L"<Target>", pIda->hOutFile );
  fputws( pszTarget, pIda->hOutFile );
  fputws( L"</Target>\r\n", pIda->hOutFile );
  fputws( L"</Segment>\r\n", pIda->hOutFile );

  return( TRUE );
}
// find the tag in the supplied data
BOOL ExpSegsFindTag( PSZ_W pszTag, PSZ_W pszData, int *piCharsBefore, int *piCharsAfter )
{
  int iBefore = 0;
  int iAfter = 0;
  int iTagLen = wcslen( pszTag );
  BOOL fFound = FALSE;

  do
  {
    // find start character
    while ( *pszData && (*pszData != *pszTag) ) 
    {
      iBefore++;
      pszData++;
    } /* endwhile */     

    // compare complete tag
    if ( *pszData  ) 
    {
      if ( wcsnicmp( pszData, pszTag, iTagLen ) == 0 )
      {
        fFound = TRUE;
        pszData += iTagLen;
        iAfter = wcslen( pszData );
      }
      else
      {
        iBefore++;
        *pszData++;
      } /* endif */       
    } /* endif */     
  } while ( !fFound && *pszData ); /* enddo */   

  if ( piCharsBefore != NULL ) *piCharsBefore = iBefore; 
  if ( piCharsAfter != NULL ) *piCharsAfter = iAfter; 

  return( fFound );
} /* end of function ExpSegsFindTag */

// look for a start tag in the supplied data
BOOL ExpSegsFindStartTag( PEXPORTSEGS_IDA pIda, PSZ_W pszData, int *piCharsAfter )
{
  int iBefore = 0;
  int iAfter = 0;
  BOOL fFound = FALSE;
  int iEntry = 0;

  while ( !fFound && (iEntry < pIda->iArrayEntries) )
  {
    if ( ExpSegsFindTag( pIda->pStartStop[iEntry].szStartTag, pszData, &iBefore, &iAfter ) )
    {
      fFound = TRUE;
      wcscpy( pIda->szEndTag, pIda->pStartStop[iEntry].szEndTag );
      if ( piCharsAfter != NULL ) *piCharsAfter = iAfter; 
    }
    else
    {
      iEntry++;
    } /* endif */       
  } /* endwhile */     

  return( fFound );
} /* end of function ExpSegsFindStartTag */

BOOL ExpSegsBrowseForFile( PEXPORTSEGS_IDA pIda, HWND hwnd, PSZ pszTitle, PSZ pszFileBuffer, int iBufSize, BOOL fOpen )
{
  OPENFILENAME ofn;
  BOOL fOk = FALSE;

  pIda = ACCESSDLGIDA (hwnd, PEXPORTSEGS_IDA );

  ofn.lStructSize                 = sizeof(ofn);
  ofn.hInstance                   = (HINSTANCE) UtlQueryULong(QL_HAB);
  ofn.lpstrFilter                 = NULL;
  ofn.nFilterIndex                = 0;
  ofn.lpstrCustomFilter           = NULL;
  ofn.nMaxCustFilter              = 0;
  ofn.lpstrFileTitle              = pszFileBuffer; 
  ofn.nMaxFileTitle               = iBufSize;
  ofn.lpstrInitialDir             = NULL;
  ofn.lpstrTitle                  = pszTitle;
  ofn.nFileOffset                 = 0;
  ofn.nFileExtension              = 0;
  ofn.lCustData                   = 0L;
  ofn.lpfnHook                    = NULL;
  ofn.lpTemplateName              = NULL;
  ofn.hwndOwner                   = hwnd;
  ofn.lpstrFile                   = pszFileBuffer;
  ofn.nMaxFile                    = iBufSize;
  ofn.Flags                       = OFN_FILEMUSTEXIST | OFN_LONGNAMES;
  ofn.lpstrDefExt                 = NULL;

  if ( fOpen )
  {
    fOk = GetOpenFileName( &ofn );
  }
  else
  {
    fOk = GetSaveFileName( &ofn );
  } /* endif */     

  return( fOk );
} /* end of function ExpSegsBrowseForFile */

// load start/stop tag file and preprocess its contents
BOOL ExpLoadStartStopFile( PEXPORTSEGS_IDA pIda )
{
  BOOL fOK = TRUE;
  FILE *hInFile = NULL;

  // open input file
  hInFile = fopen( pIda->szStartStopFile, "r" );
  if ( hInFile == NULL )
  {
    PSZ pszParms[2]; 
    CHAR szReturnCode[10];

    sprintf( szReturnCode, "%ld", GetLastError() );
    pszParms[0] = pIda->szStartStopFile;
    pszParms[1] = szReturnCode;
    UtlError( ERROR_IN_STARTSTOPFILE, MB_CANCEL, 2, pszParms, EQF_ERROR );

    fOK = FALSE;
  } /* endif */     

  // loop over the file until complete
  if ( fOK )
  {
    memset( pIda->szLine, 0, sizeof(pIda->szLine) );
    fgets( pIda->szLine, sizeof(pIda->szLine)-1, hInFile );

    while ( pIda->szLine[0] != EOS )
    {
      // strip-off LF at end of line
      int iLen = strlen(pIda->szLine);
      if ( iLen && (pIda->szLine[iLen-1] == '\n') ) pIda->szLine[iLen-1] = '\0';

      // strop whitepsace from line
      UtlStripBlanks( pIda->szLine );

      // get start and stop tag
      if ( (pIda->szLine[0] != EOS) && (pIda->szLine[0] != '*') )
      {
        // enlarge tag array when necessary
        if ( pIda->iArrayEntries >= pIda->iArraySize )
        {
          int iOldSize = pIda->iArraySize * sizeof(EXPSEGSSTARTSTOP);
          int iNewSize = (pIda->iArraySize + 20) * sizeof(EXPSEGSSTARTSTOP);
          fOK = UtlAllocHwnd( (PVOID *)&(pIda->pStartStop), iOldSize, iNewSize, ERROR_STORAGE, pIda->hwndErrMsg );

          if ( fOK ) pIda->iArraySize += 20;
        } /* endif */           

        // extract tags
        if ( fOK )
        {
          PSZ pszTemp = pIda->szLine;
          if ( ExportSegsExtractTag( &pszTemp, pIda->szTagBuffer ) )
          {
            ASCII2Unicode( pIda->szTagBuffer, pIda->pStartStop[pIda->iArrayEntries].szStartTag, 0L );

            // skip any delimiter
            while( *pszTemp == ' ' ) pszTemp++;
            if ( *pszTemp == ',' ) pszTemp++;
            while( *pszTemp == ' ' ) pszTemp++;

            // extract end tag
            if ( ExportSegsExtractTag( &pszTemp, pIda->szTagBuffer ) )
            {
              ASCII2Unicode( pIda->szTagBuffer, pIda->pStartStop[pIda->iArrayEntries].szEndTag, 0L );
              pIda->iArrayEntries++;
            } /* endif */               
          } /* endif */             
        
        } /* endif */           
      } /* endif */         
      
      // get next line from file
      memset( pIda->szLine, 0, sizeof(pIda->szLine) );
      fgets( pIda->szLine, sizeof(pIda->szLine)-1, hInFile );
    } /* endwhile */
  } /* endif */

  if ( hInFile ) fclose( hInFile );

  // check if start/stop tag were found
  if ( fOK  )
  {
    if ( pIda->iArrayEntries == 0 )
    {
      PSZ pszParms[2]; 
      pszParms[0] = pIda->szStartStopFile;
      UtlError( NO_VALID_STARTSTOP_TAGS, MB_CANCEL, 1, pszParms, EQF_ERROR );
    } /* endif */       
  } /* endif */     

  return( fOK );
} /* end of function ExpSegsFindStartTag */

// extract a tag into the supplied buffer
BOOL ExportSegsExtractTag( PSZ *ppszText, PSZ pszBuffer )
{
  int iLength = 0;
  PSZ pszText = *ppszText;

  if ( *pszText == '\"' )
  {
    // copy string up to closing double-quote or end-of-string delimiter
    pszText++;
    while ( (*pszText != EOS) && (*pszText != '\"') )
    {
      *pszBuffer++ = *pszText++;
      iLength++;
      if ( (*pszText == '\"') && (*(pszText+1) == '\"') )
      {
        // convert doubled double-quote to single double-quote
        *pszBuffer++ = *pszText++;
        pszText++;
        iLength++;
      } /* endif */         
    } /* endwhile */     

    // skip end delimiter
    if ( *pszText == '\"' )
    {
      pszText++;
    } /* endif */         
  }
  else
  {
    // copy string up to comma or end-of-string delimiter
    while ( (*pszText != EOS) && (*pszText != ',') )
    {
      *pszBuffer++ = *pszText++;
      iLength++;
    } /* endwhile */       
  } /* endif */     

  *pszBuffer = EOS;

  *ppszText = pszText;

  return( iLength != 0 );
} /* end of function ExportSegsExtractTag */
