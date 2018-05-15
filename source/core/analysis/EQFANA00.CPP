
/*! \brief EQFANA00.CPP  Analysis Handler
	Copyright (c) 1999-2017, International Business Machines Corporation and others. All rights reserved.
*/

#define TADUMMYTAG_INIT
#define INCL_EQF_TM               // general Transl. Memory functions
#define INCL_EQF_ANALYSIS         // analysis functions
#define INCL_EQF_TAGTABLE         // tagtable defines and functions
#define INCL_EQF_TP               // public translation processor functions
#define INCL_EQF_FOLDER           // folder list and document list functions
#include "eqf.h"                  // General Translation Manager include file

#include "EQFDDE.H"               // Batch mode definitions
#include "eqftai.h"               // text analysis private defines
#include "eqfiana1.id"            // PM IDs for Text Analysis
  #include "OTMFUNC.H"            // function call interface public defines
  #include "eqffunci.h"           // function call interface private defines

#ifdef _DEBUG
  #define SESSIONLOG
#endif

MRESULT AnalysisCallBack
(
  PPROCESSCOMMAREA pCommArea,          // ptr to commmunication area
  HWND             hwnd,               // handle of process window
  WINMSG           message,            // message to be processed
  WPARAM           mp1,                // first message parameter
  LPARAM           mp2                 // second message parameter
);
USHORT TABatchAnalysis( HWND, PDDEANALYSIS );

/*
.cp
*******************  Analysis anchor (base handler) ***************************/

MRESULT APIENTRY ANALYSISHANDLERWP
(
  HWND hwnd,
  WINMSG message,
  WPARAM mp1,
  LPARAM mp2
)
{
    PIDA_HEAD       pIda;                // pointer to instance area
    MRESULT         mResult = FALSE;     // return code for handler proc
    PANALYSIS_IDA   pAnaIda = NULL;      // pointer to analysis ida

    switch( message)
    {


      case WM_CREATE:
        {
          BOOL     fOK;                // internal O.K. flag

          fOK = UtlAlloc( (PVOID *) &pIda, 0L , (LONG) sizeof( IDA_HEAD ), ERROR_STORAGE);

          if ( fOK )
          {
             pIda->hFrame = hwnd;
             ANCHORWNDIDA( hwnd, pIda );
          } /* endif */

          if ( fOK )
          {
             pIda->pszObjName = pIda->szObjName;
             strcpy( pIda->szObjName, ANALYSISHANDLER );

             //if error registering analysis handler (NULL = no error )
             if ( EqfInstallHandler( ANALYSISHANDLER, hwnd, clsANALYSIS ) )
             {
                //display error message
                UtlError( 0, MB_CANCEL, 0, NULL, INTERNAL_ERROR );
                fOK = FALSE;
             }/*endif*/
          } /* endif */

          if ( !fOK )
          {
             mResult = MRFROMSHORT(DO_NOT_CREATE);   // do not create the window
          } /* endif */
        }
        return( mResult);


/*--------------------------------------------------------------------------*/
      case WM_EQF_SHUTDOWN:            // come here before EQF_TERMINATE
        // send to the instance that termination is required
        EqfSend2AllObjects( clsANALYSIS, message, mp1, mp2 );
        return( (MRESULT) FALSE);            // allow termination

/*--------------------------------------------------------------------------*/
      case WM_EQF_DELETE:
        return( (MRESULT)TRUE);        // nothing deleted

/*--------------------------------------------------------------------------*/
      case WM_EQF_ABOUTTODELETE:
        if( SHORT1FROMMP1( mp1) == clsFOLDER)
        {
          if(EqfQueryObject( (PSZ) PVOIDFROMMP2(mp2), clsANALYSIS, 0))
             mResult = (MRESULT)TRUE;    // do not delete while analysis is up
        }
        return(mResult);

/*--------------------------------------------------------------------------*/
      case WM_EQF_OPEN:
//      there is no way defined at the moment to open an existing ANALYSIS obj
        return( (MRESULT)TRUE);        // object not opened

/*--------------------------------------------------------------------------*/
       case WM_EQFN_PROPERTIESCHANGED:
          if ( SHORT1FROMMP1(mp1) == clsTAGIMP )
          {
             TAInvalidateTagTable ( (PSZ) PVOIDFROMMP2(mp2) );
          } /* endif */
          break;
/*--------------------------------------------------------------------------*/
      case WM_EQF_CREATE:
        // mp1 = 0: mp2 points to an object name; 1: mp2 points to list of objects
        // mp2 = object name of selected folder or list of selected folders
        pIda = ACCESSWNDIDA( hwnd, PIDA_HEAD );

        if( !mp2)                   // no folder name passed
        {
           UtlError( ERROR_NODOCS_ANALYSIS, MB_CANCEL, 0, NULL, EQF_WARNING );
           mResult = (MRESULT)TRUE;
        }
        else
        {
          // allocate analysis instance ida
          if( !( UtlAlloc( (PVOID *) &pAnaIda, 0L, (ULONG)sizeof( ANALYSIS_IDA),
                 ERROR_STORAGE)))
          {
            mResult =  (MRESULT)TRUE;
          } /* endif */
        } /* endif */

        if (!mResult)     // processing ok so far
        {
           HWND          hframe;       // Text Analysis Window Handles

           strcpy (pAnaIda->szFolder,(char*) PVOIDFROMMP2(mp2) ); // should be a folder name
           sprintf(pAnaIda->szObjName, "ANALYSIS%s", pAnaIda->szFolder);
           if ( mp1 == 1 )
           {
             pAnaIda->pszObjList = (PSZ)mp2;
           } /* endif */

           // analysis for this folder already started
           hframe = EqfQueryObject( pAnaIda->szObjName, clsANALYSIS, 0);
           if ( hframe )
           {                         // analysis for this folder already started
             ActivateMDIChild( hframe );
             UtlAlloc( (PVOID *) &(pAnaIda), 0L, 0L, NOMSG) ;
             mResult =  (MRESULT)TRUE;
           } /* endif */
        } /* endif */


        if (!mResult)     // processing ok so far
        {
           pAnaIda->hwndDocLB = WinCreateWindow( hwnd, WC_LISTBOX, "",
                                        WS_CHILDWINDOW | LBS_STANDARD,
                                        0, 0, 10, 10, hwnd,
                                        HWND_TOP, ID_TATODOLB, NULL, NULL);
           mResult = (MRESULT) (!pAnaIda->hwndDocLB );
        } /* endif */


        if (!mResult)     // processing ok so far
        {
           INT_PTR        iRc;         // return from WinDlgBox of Analysis
           USHORT         usRc;        // return code
           HWND           hwndActive;  // handle of active window
           HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);

           //get Active folder and fill listbox with selected documents
           hwndActive = EqfQueryActiveFolderHwnd();
           //get status of active folder
           usRc = EqfQueryObjectStatus( hwndActive );
           if ( usRc & OBJ_FOCUS )             //documents selected in folder
           {
              EqfSend2Handler( FOLDERHANDLER, WM_EQF_QUERYSELECTEDNAMES,
                               MP1FROMHWND( pAnaIda->hwndDocLB ),
                               MP2FROMP(pAnaIda->szFolder) );
           }
           else
           {
              EqfSend2Handler( FOLDERHANDLER, WM_EQF_INSERTNAMES,
                               MP1FROMHWND( pAnaIda->hwndDocLB ),
                               MP2FROMP(pAnaIda->szFolder) );
           } /* endif */

           pIda->fMustNotClose = TRUE;   // twb close not allowed

           // load analysis dialog
           DIALOGBOX( EqfQueryTwbClient(), ANALYSISPROPDLGPROC,
                      hResMod, ID_ANAPROP_DLG, pAnaIda, iRc );

           if ( iRc == -1 ) // dialog load failed ???
           {
             iRc = 0;
             mResult = FALSE;
           } /* endif */

           if( iRc && WinIsWindow( (HAB)NULL, hwnd))   // WM_DESTROYed ?
           {
             if ( !CreateProcessWindow( pAnaIda->szObjName,
                                        AnalysisCallBack,
                                        pAnaIda ) )
             {
               mResult = (MRESULT) TRUE;
             }
             else
             {
               // handler is not busy any more
               pIda->fMustNotClose = FALSE;
               EqfSend2AllHandlers( WM_EQFN_CREATED,
                                    MP1FROMSHORT( clsANALYSIS ),
                                    MP2FROMP(pAnaIda->szFolder) );
             } /* endif */
           } /* endif */

           if (mResult)
           {
              pIda->fMustNotClose = FALSE;
              WinDestroyWindow( pAnaIda->hwndDocLB );
              UtlAlloc( (PVOID *) &(pAnaIda), 0L, 0L, NOMSG) ;
           } /* endif */
        } /* endif */
        return( mResult);

      case WM_CLOSE:
         pIda = ACCESSWNDIDA( hwnd, PIDA_HEAD );
         if (pIda)
         {
           EqfRemoveHandler( TWBFORCE, pIda->szObjName );
         }
         break;

      case WM_DESTROY:
         pIda = ACCESSWNDIDA( hwnd, PIDA_HEAD );
         UtlAlloc( (PVOID *) (PVOID *)&pIda, 0L, 0L, NOMSG );
         break;

      case WM_EQF_TERMINATE:
         pIda = ACCESSWNDIDA( hwnd, PIDA_HEAD );
         WinDestroyWindow( pIda->hFrame);
         break;

      case  WM_EQF_DDE_REQUEST:
        /************************************************************/
        /*     mp1:  (DDETASK) Task                                 */
        /*     mp2:  (PVOID) pTaskIda                               */
        /************************************************************/
        switch ( SHORT1FROMMP1( mp1 ) )
        {
          case  TASK_ANALYSIS:
            {
              PDDEANALYSIS pAnalysis = (PDDEANALYSIS)PVOIDFROMMP2(mp2);
              TABatchAnalysis( hwnd, pAnalysis );
            }
            break;
          case  TASK_ARCHTM:
            {
              PDDEARCHTM pArchTM = (PDDEARCHTM)PVOIDFROMMP2(mp2);
              TABatchArchTM( hwnd, pArchTM );
            }
            break;

          default :
            break;
        } /* endswitch */
        break;
      default:
         mResult = WinDefWindowProc( hwnd, message, mp1, mp2 );
         break;

    } /* endswitch */
    return( mResult );
} /* end of ANALYSIS HANDLER */


MRESULT AnalysisCallBack
(
  PPROCESSCOMMAREA pCommArea,          // ptr to commmunication area
  HWND             hwnd,               // handle of process window
  WINMSG           message,            // message to be processed
  WPARAM           mp1,                // first message parameter
  LPARAM           mp2                 // second message parameter
)
{
  PANALYSIS_IDA   pIda;                // pointer to instance area
  MRESULT         mResult = FALSE;     // return code for proc

  switch( message)
  {
    /******************************************************************/
    /* WM_CREATE:                                                     */
    /*                                                                */
    /* Fill fields in communication area                              */
    /* Initialize data of callback function                           */
    /******************************************************************/
    case WM_CREATE :
      {
        HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);

        /**************************************************************/
        /* Anchor IDA                                                 */
        /**************************************************************/
        pIda                =(PANALYSIS_IDA) PVOIDFROMMP2(mp2);
        pCommArea->pUserIDA = pIda;

        /****************************************************************/
        /* supply all information required to create the process        */
        /* window                                                       */
        /****************************************************************/
        pCommArea->sProcessWindowID = ID_TEXTANALYSIS;
        pCommArea->sProcessObjClass = clsANALYSIS;
        if ( pIda->ProcParm->fBatch )
        {
          pCommArea->Style            = PROCWIN_BATCH;

          /**************************************************************/
          /* Using a process window of type PROCWIN_BATCH will NOT      */
          /* create oru document listboxes so we will create these      */
          /* listboxes here instead                                     */
          /**************************************************************/
          pCommArea->hwndLB1 = WinCreateWindow( hwnd, WC_LISTBOX, "",
                                               WS_CHILDWINDOW | LBS_STANDARD,
                                               0,0,10,10, NULLHANDLE, HWND_TOP,
                                               1, NULL, NULL);
          pCommArea->hwndLB2 = WinCreateWindow( hwnd, WC_LISTBOX, "",
                                               WS_CHILDWINDOW | LBS_STANDARD,
                                               0,0,10,10, NULLHANDLE, HWND_TOP,
                                               2, NULL, NULL);
        }
        else
        {
          pCommArea->Style            = PROCWIN_TEXTSLIDERLISTBOX;
        } /* endif */

        pCommArea->sSliderID        = ID_TASLIDER;
        LOADSTRING( NULLHANDLE, hResMod, IDS_TATITLEBAR, pCommArea->szTitle );

        SubFolObjectNameToName( pIda->szFolder, pCommArea->szBuffer );
        OEMTOANSI(pCommArea->szBuffer);
        strcpy( pCommArea->szTitle + strlen(pCommArea->szTitle), pCommArea->szBuffer );

        pCommArea->hIcon            = (HPOINTER) UtlQueryULong(QL_ANAICON); //hiconANA;
        pCommArea->fNoClose         = FALSE;

        pCommArea->swpSizePos.x     = ANALYSIS_X;
        pCommArea->swpSizePos.y     = ANALYSIS_Y;
        pCommArea->swpSizePos.cx    = (SHORT) UtlQueryULong( QL_AVECHARWIDTH ) * 80;
        pCommArea->swpSizePos.cy    = (SHORT) UtlQueryULong( QL_PELSPERLINE ) * 15;

        pCommArea->sTextID          = ID_TAINFO_TEXT;
        pCommArea->sLB1TextID       = ID_TATODO_TEXT;
        pCommArea->sLB2TextID       = ID_TADONE_TEXT;
        pCommArea->sLB1ID           = 1;
        pCommArea->sLB2ID           = 2;
        LOADSTRING( NULLHANDLE, hResMod, IDS_TATODO, pCommArea->szLB1Text );
        LOADSTRING( NULLHANDLE, hResMod, IDS_TADONE, pCommArea->szLB2Text );

        pCommArea->asMsgsWanted[0]  = WM_EQF_SLIDER_SETTEXT;
        pCommArea->asMsgsWanted[1]  = 0;

        pCommArea->usComplete       = 0;
      }
      break;


      /****************************************************************/
      /* Run the analysis process                                     */
      /****************************************************************/
    case WM_EQF_INITIALIZE:
      {
        PTAINPUT   pTAInput;           // ptr to input structure

        pIda     = (PANALYSIS_IDA) pCommArea->pUserIDA;
        pTAInput = pIda->ProcParm;
        pTAInput->pszObjList = pIda->pszObjList;
        pTAInput->pszActiveFolder = pIda->pszObjList;

        pTAInput->hwndProcWin = hwnd;
        if ( !pTAInput->fBatch )
        {
          pTAInput->hwndErrMsg = hwnd;
        } /* endif */

        pTAInput->hwndToDo    = pCommArea->hwndLB1;
        pTAInput->hwndDone    = pCommArea->hwndLB2;

        // set fForceEqualWhiteSpace flag if the file EQFANAWS
        // exists in the EQF\PROPERTY directory
        {
          CHAR szCheckMe[MAX_EQF_PATH];
          UtlMakeEQFPath( szCheckMe, NULC, PROPERTY_PATH, NULL );
          strcat( szCheckMe, "\\EQFANAWS" );
          pTAInput->fForceEqualWhiteSpace = UtlFileExist( szCheckMe );
        }

        pTAInput->szEditName[0] = EOS;

        {
          USHORT usCount;

          for (usCount = 0; usCount < pTAInput->stSourcefiles.usNumber; usCount++)
          {
            INSERTITEMHWND( pCommArea->hwndLB1,
                            pTAInput->apszLongNames[usCount] );
          } /* endfor */
        }
        WinDestroyWindow( pIda->hwndDocLB );

        pTAInput->ppvCallersTAInput =(PVOID*) (&(pIda->ProcParm)); // set address of our PTAINPUT pointer (it may be
                                                         // neccessary to re-alloc TAINPUT within EQFTAMnl)

        EQFTAMnl( hwnd, &pTAInput );    // call text analysis process
        pIda->ProcParm = pTAInput;      // refresh pointer
      }
      break;

    /******************************************************************/
    /* WM_CLOSE:                                                      */
    /*                                                                */
    /* Prepare/initialize shutdown of process                         */
    /******************************************************************/
    case WM_CLOSE:
      pIda = (PANALYSIS_IDA)pCommArea->pUserIDA;
      if ( pIda )
      {
         mResult = MRFROMSHORT( TRUE );   // = do not close right now
         if (pIda->ProcParm)
         {
            PTAINPUT   pTAInput;           // ptr to input structure

            pTAInput =  pIda->ProcParm;
            if ( pTAInput->fBatch )
            {
              pTAInput->fKill = TRUE;
            }
            else if ( !pTAInput->pInD->fTerminate )  // not already terminating ???
            {
              if ( UtlError( ERROR_CANCELTA,
                             MB_YESNO,
                             0,
                             NULL,
                             EQF_QUERY ) == MBID_YES )
              {
                pTAInput->fKill = TRUE;
              } /* endif */
            } /* endif */
         } /* endif */
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
      pIda = (PANALYSIS_IDA)pCommArea->pUserIDA;
      if ( pIda )
      {
         if ( pIda->ProcParm )
         {
           PTAINPUT pTAInput = (PTAINPUT)pIda->ProcParm;
           if ( pTAInput->fBatch )
           {
             WinPostMsg( pTAInput->pDDEAnalysis->hwndOwner, WM_EQF_DDE_ANSWER,
                         NULL, MP2FROMP(&pTAInput->pDDEAnalysis->DDEReturn) );
           } /* endif */
           if (pTAInput->pWSList) UtlAlloc( (PVOID *) &(pTAInput->pWSList), 0L, 0L, NOMSG );
           UtlAlloc( (PVOID *) &pTAInput, 0L, 0L, NOMSG );
         } /* endif */
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

    /******************************************************************/
    /* WM_INITMENU:                                                   */
    /*                                                                */
    /* Enable/Disable actionbar items                                 */
    /******************************************************************/
    case WM_INITMENU:
      UtlMenuDisableAll( hwnd, SHORT1FROMMP1( mp1) );
      break;

    /******************************************************************/
    /* other messages:                                                */
    /*                                                                */
    /* requested from generic process window procedure using          */
    /* asMsgsWanted array in communication area                       */
    /******************************************************************/
    case WM_EQF_SLIDER_SETTEXT:
      {
        PTAINPUT   pTAInput;           // ptr to input structure

        pIda     = (PANALYSIS_IDA)pCommArea->pUserIDA;
        pTAInput = pIda->ProcParm;
        if ( PVOIDFROMMP2(mp2) != NULL )
        {
          strcpy( pCommArea->szText, (char*) PVOIDFROMMP2(mp2) );
          WinSendMsg( hwnd, WM_EQF_UPDATESLIDER, NULL, NULL );
        } /* endif */
      }
      return( mResult );
      break;
  } /* endswitch */
  return( mResult );
} /* end of function AnalysisCallBack */

USHORT TABatchAnalysis
(
  HWND             hwndParent,         // handle of analysis handler window
  PDDEANALYSIS     pAnalysis           // analysis data structure
)
{
   HWND             hwndLB = NULLHANDLE; // handle of invisible listbox
   PSZ              pszParm;           // pointer for error parameters
   BOOL             fOK = TRUE;        // internal O.K. flag
   PTAINPUT         pTAInput = NULL;   // analysis input structure
   PANALYSIS_IDA    pIda = NULL;       // analsysis instance data area
   PSZ              pszAct, pszStart;  // ptr for TAINPUT buffer processing

   pszAct = NULL;
   pszStart = NULL;
   /********************************************************************/
   /* Create invisible listbox for names of folder/memories/...        */
   /********************************************************************/
   hwndLB = WinCreateWindow( hwndParent, WC_LISTBOX, "", 0L,
                             0, 0, 0, 0,
                             hwndParent, HWND_TOP, 1, NULL, NULL );

   /*******************************************************************/
   /* Allocate analysis instance data area                            */
   /*******************************************************************/
   fOK = UtlAlloc( (PVOID *) &pIda, 0L, (LONG) sizeof(ANALYSIS_IDA), NOMSG );
   if ( !fOK )
   {
     pAnalysis->DDEReturn.usRc = ERROR_STORAGE;
     UtlErrorHwnd( pAnalysis->DDEReturn.usRc, MB_CANCEL, 0,
                   NULL, EQF_ERROR, pAnalysis->hwndErrMsg );
   } /* endif */

   /*******************************************************************/
   /* Check if folder exists                                          */
   /*******************************************************************/
   if ( fOK )
   {
     /******************************************************************/
     /* Fill our listbox with the names of all folder                  */
     /******************************************************************/
     DELETEALLHWND( hwndLB );
     {
       BOOL fIsNew;

       ObjLongToShortName( pAnalysis->szFolder, pIda->szFolder, FOLDER_OBJECT, &fIsNew );
       if ( fIsNew )
       {
         fOK = FALSE;
         pszParm = pAnalysis->szFolder;
         pAnalysis->DDEReturn.usRc = ERROR_XLATE_FOLDER_NOT_EXIST;
         UtlErrorHwnd( pAnalysis->DDEReturn.usRc, MB_CANCEL, 1,
                       &pszParm, EQF_ERROR, pAnalysis->hwndErrMsg );
       } /* endif */
     }
   } /* endif */


   /*******************************************************************/
   /* Check if documents exist                                        */
   /*******************************************************************/
   if ( fOK )
   {
     SHORT sItem;                      // index of listbox items
     USHORT i = 0;                     // loop index

     /*****************************************************************/
     /* Build folder object name (access to folder properties is      */
     /* required to correct folder drive letter)                      */
     /*****************************************************************/
     {
       PPROPFOLDER  ppropFolder;        // pointer to folder properties
       HPROP        hpropFolder;        // folder properties handle
       ULONG        ulErrorInfo;        // error indicator from property handler

       UtlMakeEQFPath( pIda->szFolObjName, NULC, SYSTEM_PATH, NULL );
       strcat( pIda->szFolObjName, BACKSLASH_STR );
       strcat( pIda->szFolObjName, pIda->szFolder );
       strcat( pIda->szFolObjName, EXT_FOLDER_MAIN );
       hpropFolder = OpenProperties( pIda->szFolObjName, NULL,
                                         PROP_ACCESS_READ, &ulErrorInfo);
       if( hpropFolder )
       {
         ppropFolder = (PPROPFOLDER)MakePropPtrFromHnd( hpropFolder );
         if ( (ppropFolder->chDrive != EOS) && (ppropFolder->chDrive != ' ') )
         {
           pIda->szFolObjName[0] = ppropFolder->chDrive;
         } /* endif */
         CloseProperties( hpropFolder, PROP_QUIT, &ulErrorInfo );
       } /* endif */
     }

     /******************************************************************/
     /* Fill our listbox with the names of the documents of the        */
     /* selected folder                                               */
     /******************************************************************/
     DELETEALLHWND( hwndLB );
     EqfSend2Handler( FOLDERHANDLER, WM_EQF_INSERTNAMES,
                      MP1FROMHWND( hwndLB ), MP2FROMP(pIda->szFolObjName)  );

     /******************************************************************/
     /* Search document names in listbox                               */
     /******************************************************************/
     while ( fOK && (i < pAnalysis->usFileNums) )
     {

       CHAR szDocShortName[MAX_FILESPEC];        // buffer for short doc name
       BOOL fIsNew = FALSE;

       FolLongToShortDocName( pIda->szFolObjName, pAnalysis->ppFileArray[i],
                                szDocShortName, &fIsNew );

       if ( fIsNew )
       {
         sItem = -1;
#ifdef SESSIONLOG
         UtlLogWriteString( "Analysis: Error in FolLongToShortDocName for document %s", pAnalysis->ppFileArray[i] );
#endif
       }
       else
       {
         sItem = SEARCHITEMHWND( hwndLB, szDocShortName );
#ifdef SESSIONLOG
         if ( sItem < 0 )
         {
          UtlLogWriteString( "Analysis: Error: Document %s not in processing LB", pAnalysis->ppFileArray[i] );
         } /* endif */
#endif
       } /* endif */

       if ( sItem < 0 )
       {
         fOK = FALSE;
         pszParm = pAnalysis->ppFileArray[i];
         pAnalysis->DDEReturn.usRc = ERROR_TA_SOURCEFILE;
         UtlErrorHwnd( pAnalysis->DDEReturn.usRc, MB_CANCEL, 1,
                       &pszParm, EQF_ERROR, pAnalysis->hwndErrMsg );
       } /* endif */
       i++;
     } /* endwhile */
   } /* endif */

   /*******************************************************************/
   /* Check if TMs exist                                              */
   /*******************************************************************/
   if ( fOK )
   {
     SHORT sItem;                      // index of listbox items
     USHORT i = 0;                     // loop index

     /******************************************************************/
     /* Fill our listbox with the names of the TMs                     */
     /******************************************************************/
     DELETEALLHWND( hwndLB );
     EqfSend2Handler( MEMORYHANDLER, WM_EQF_INSERTNAMES,
                      MP1FROMHWND( hwndLB ), MP2FROMP( MEMORY_ALL ) );

     /******************************************************************/
     /* Search document names in listbox                               */
     /******************************************************************/
     while ( fOK && (i < pAnalysis->usMemNums) )
     {
       sItem = SEARCHITEMHWND( hwndLB, pAnalysis->ppMemArray[i] );
       if ( sItem < 0 )
       {
         fOK = FALSE;
         pszParm = pAnalysis->ppMemArray[i];
         pAnalysis->DDEReturn.usRc = ERROR_MEMORY_NOTFOUND;
         UtlErrorHwnd( pAnalysis->DDEReturn.usRc, MB_CANCEL, 1,
                       &pszParm, EQF_ERROR, pAnalysis->hwndErrMsg );
       } /* endif */
       i++;
     } /* endwhile */
   } /* endif */

   /*******************************************************************/
   /* Fill IDA fields                                                 */
   /*******************************************************************/
   if ( fOK )
   {
     CHAR szFolderName[MAX_FILESPEC];
     // the short folder name has been stored in pIda->szFolder temporarly!
     strcpy( szFolderName, pIda->szFolder );
     UtlMakeEQFPath( pIda->szFolder, NULC, SYSTEM_PATH, NULL );
     strcat( pIda->szFolder, BACKSLASH_STR );
     strcat( pIda->szFolder, szFolderName );
     strcat( pIda->szFolder, EXT_FOLDER_MAIN );

     sprintf( pIda->szObjName, "ANALYSIS%s", pIda->szFolObjName );
     pIda->hwndDocLB = hwndLB;
     DELETEALLHWND( hwndLB );
     if ( pAnalysis->usFileNums != 0 )
     {
       /***************************************************************/
       /* Add specified documents to listbox                          */
       /***************************************************************/
       USHORT i = 0;

       while ( i < pAnalysis->usFileNums )
       {
         CHAR szDocShortName[MAX_FILESPEC];        // buffer for short doc name
         BOOL fIsNew = FALSE;

         FolLongToShortDocName( pIda->szFolObjName, pAnalysis->ppFileArray[i],
                                  szDocShortName, &fIsNew );

         INSERTITEMHWND( hwndLB, szDocShortName );
         i++;
       } /* endwhile */
     }
     else
     {
       /***************************************************************/
       /* Add all documents of folder to listbox                      */
       /***************************************************************/
       EqfSend2Handler( FOLDERHANDLER, WM_EQF_INSERTNAMES, MP1FROMHWND(hwndLB),
                        MP2FROMP(pIda->szFolObjName) );
     } /* endif */
   } /* endif */

   /*******************************************************************/
   /* allocate storage for analysis interface                         */
   /*******************************************************************/
   if ( fOK )
   {
     USHORT usBufferSize = ( (QUERYITEMCOUNTHWND( hwndLB ) +
                              pAnalysis->usMemNums +
                              1 ) * // number of morph dictionaries!!
                              MAX_EQF_PATH );
     fOK = UtlAlloc( (PVOID *) &pTAInput, 0L,
                     (LONG)(sizeof(TAINPUT) + usBufferSize ), NOMSG );
     if ( !fOK )
     {
       pAnalysis->DDEReturn.usRc = ERROR_STORAGE;
       UtlErrorHwnd( pAnalysis->DDEReturn.usRc, MB_CANCEL, 0,
                     NULL, EQF_ERROR, pAnalysis->hwndErrMsg );
     } /* endif */
   } /* endif */

   /*******************************************************************/
   /* Fill analysis input structure                                   */
   /*******************************************************************/
   if ( fOK )
   {
     pIda->ProcParm                   = pTAInput;
     pTAInput->pDDEAnalysis           = pAnalysis;
     pTAInput->hwndErrMsg             = pAnalysis->hwndErrMsg;
     pTAInput->fBatch                 = TRUE;
     pTAInput->szOutDictName[0]       = EOS;
     pTAInput->szNTLname[0]           = EOS;
     pTAInput->fNTLcontext            = FALSE;
     pTAInput->usNTLNumOccurences     = 0;
     pTAInput->szFTLname[0]           = EOS;
     pTAInput->fFTLcontext            = FALSE;
     pTAInput->usFTLNumOccurences     = 0;
     pTAInput->szExclusionList[0]     = EOS;
     pTAInput->szExclDictname[0]      = EOS;
     pTAInput->stInputDict.usNumber   = 0;
     pTAInput->stSourcefiles.usNumber = QUERYITEMCOUNTHWND( hwndLB );
     pTAInput->stInputMemDb.usNumber  = pAnalysis->usMemNums;
     pTAInput->fInsertToTM            = pAnalysis->fAddToMem;
     pTAInput->fInsertTMMatches       = pAnalysis->fReplExactMatchs;
     pTAInput->fUseLatestTMMatch      = pAnalysis->fUseLatestMatch;
     pTAInput->fInsertNewMatch        = pAnalysis->fUNSEG;
     pTAInput->fAutoJoin              = pAnalysis->fAutoJoin;
     pTAInput->fNTLProcessing         = FALSE;
     pTAInput->fExactContextTMMatch   = pAnalysis->fExactContextTMMatch;
     pTAInput->fTMMatch               = pAnalysis->fTMMatch;
     pTAInput->fRedundCount           = pAnalysis->fRedundCount;
     pTAInput->fLeadingWS             = pAnalysis->fLeadingWS;
     pTAInput->fTrailingWS            = pAnalysis->fTrailingWS;
     pTAInput->fRespectCRLF           = (EQF_BOOL) pAnalysis->fRespectCRLF;
     pTAInput->pWSList = NULL;

     memcpy( pTAInput->TATag, TADummyTag, sizeof(TATAG) * MAX_SEG_CLASSES );
     TATATag2Unicode( pTAInput->TATag, pTAInput->TATagW );

     if ( pTAInput->stSourcefiles.usNumber == 0 )  //no files in analysis LB
     {
       UtlErrorHwnd( ERROR_NODOCS_ANALYSIS,
                     MB_CANCEL, 0, NULL, EQF_WARNING, pAnalysis->hwndErrMsg );
       fOK = FALSE;
     } /* endif */
   } /* endif */

   if ( fOK && pTAInput->fInsertToTM && (pTAInput->stInputMemDb.usNumber == 0) )
   {
     UtlErrorHwnd( ERROR_NO_INMDB_SELECTED, MB_CANCEL,
                   0, NULL, EQF_ERROR, pAnalysis->hwndErrMsg );
     fOK = FALSE;
   } /* endif */

   /*******************************************************************/
   /* Retrieve values from folder properties                          */
   /*******************************************************************/
   if ( fOK )
   {
      PPROPFOLDER  ppropFolder;        // pointer to folder properties
      HPROP        hpropFolder;        // folder properties handle
      ULONG        ulErrorInfo;        // error indicator from property handler

     strcpy( pTAInput->szFolderProp, pIda->szFolder );

      if( ( hpropFolder = OpenProperties( pTAInput->szFolderProp, NULL,
                                           PROP_ACCESS_READ, &ulErrorInfo))== NULL)
      {
         /*************************************************************/
         /* Error accessing folder properties                         */
         /*************************************************************/
         if ( ulErrorInfo != Err_NoStorage )
         {
            PSZ pError = pAnalysis->szFolder;
            UtlErrorHwnd( ERROR_PROPERTY_ACCESS, MB_CANCEL, 1,
                          &pError, EQF_ERROR, pAnalysis->hwndErrMsg );
         } /* endif */
         fOK = FALSE;
      }
      else
      {
        /**************************************************************/
        /* Access to folder properties successful, now get some       */
        /* data from folder properties                                */
        /**************************************************************/
        ppropFolder =(PPROPFOLDER) MakePropPtrFromHnd( hpropFolder );
        pTAInput->sMatchLevel = ppropFolder->usMatchLevel;

        UtlMakeEQFPath( pTAInput->szTagTableName, NULC, TABLE_PATH, NULL);
        strcat( pTAInput->szTagTableName, BACKSLASH_STR );
        strcat( pTAInput->szTagTableName, ppropFolder->szFormat );
        strcat( pTAInput->szTagTableName, EXT_OF_FORMAT );

        /**************************************************************/
        /* Correct drive letter of folder object name                 */
        /**************************************************************/
        if ( (ppropFolder->chDrive != EOS) && (ppropFolder->chDrive != ' ') )
        {
          pIda->szFolObjName[0] = ppropFolder->chDrive;
        } /* endif */

        if ( ppropFolder->szLongMemory[0] != EOS )
          strcpy( pIda->szMemProp, ppropFolder->szLongMemory );
        else
          strcpy( pIda->szMemProp, ppropFolder->szMemory );

        if ( pTAInput->fInsertToTM || pTAInput->fInsertTMMatches ||
             pTAInput->fInsertNewMatch || pTAInput->fMachTrans )
        {
            BOOL fIsNew = FALSE;
            ObjLongToShortName( pIda->szMemProp, pTAInput->szOutputMemDb,
                                TM_OBJECT, &fIsNew );

            fOK = (USHORT)EqfSend2Handler( MEMORYHANDLER, WM_EQF_PROCESSTASK,
                                           MP1FROMSHORT( CREATE_PATH ),
                                           MP2FROMP( pTAInput->szOutputMemDb ));
            if ( !fOK )
            {
              // CREATE_PATH fails if TM does not exist
              PSZ pszParm = ppropFolder->szMemory;
              UtlErrorHwnd( ERROR_TM_FOR_DOC_NOT_EXIST, MB_CANCEL, 1, &pszParm,
                            EQF_ERROR, pAnalysis->hwndErrMsg );
            } /* endif */
        } /* endif */
        CloseProperties( hpropFolder, PROP_QUIT, &ulErrorInfo );
      } /* endif */
   } /* endif */

   /*******************************************************************/
   /* Add folder memory to TA input structure                         */
   /*******************************************************************/
   if ( fOK )
   {
      //--- copy folder memory database to mdb handler property buffer
      //--- call memory data base handler to get mdb properties
      USHORT usRc = (USHORT)EqfSend2Handler( MEMORYHANDLER,
                                             WM_EQF_PROCESSTASK,
                                      MP1FROMSHORT( QUERY_LANG_PROP_INPUT_MEM ),
                                      MP2FROMP( pIda->szMemProp ) );
      if ( !usRc )                          //no mdb properties found
      {
         fOK = FALSE;
      } /* endif */
   } /* endif */

   if ( fOK )
   {

     PSZ pszFolder =  UtlGetFnameFromPath( pIda->szFolObjName);

     strcpy( pTAInput->szFolder, pIda->szFolObjName );

     UtlMakeEQFPath( pTAInput->szSOURCE_Path, pIda->szFolObjName[0],
                     DIRSOURCEDOC_PATH,
                     pszFolder );

     UtlMakeEQFPath( pTAInput->szSEGSOURCE_Path, pIda->szFolObjName[0],
                     DIRSEGSOURCEDOC_PATH,
                     pszFolder );

     UtlMakeEQFPath( pTAInput->szSEGTARGET_Path, pIda->szFolObjName[0],
                     DIRSEGTARGETDOC_PATH,
                     pszFolder );

     UtlMakeEQFPath( pTAInput->szSEGNEWMATCH_Path, pIda->szFolObjName[0],
                     DIRSEGNOMATCH_PATH,
                     pszFolder );
     UtlMakeEQFPath( pTAInput->szMACHTRANS_Path, pIda->szFolObjName[0], DIRSEGMT_PATH, pszFolder);



      //set actual pointer to analysis IF data buffer
      pszAct = (PSZ)(pTAInput) + sizeof( TAINPUT );

      //--- set pointer actual pointer
      pszStart = pszAct;
   } /* endif */

   // allocate pointer array for long names
   if ( fOK )
   {
     USHORT        sNumOfDocs;         // number of documents to be analyzed
     LONG          lSize;

     sNumOfDocs =  QUERYITEMCOUNTHWND( hwndLB );
     lSize = (LONG)sizeof(PSZ) * (LONG)sNumOfDocs;
     if ( lSize < MIN_ALLOC ) lSize = MIN_ALLOC;

     fOK = UtlAllocHwnd( (PVOID *)&(pTAInput->apszLongNames), 0L, lSize,
                         ERROR_STORAGE, pAnalysis->hwndErrMsg );
     if ( fOK )
     {
       // allocate pointer array for alias names
       fOK = UtlAllocHwnd( (PVOID *)&(pTAInput->apszAlias), 0L, lSize,
                       ERROR_STORAGE, pAnalysis->hwndErrMsg );
     } /* endif */
   } /* endif */

   // create string pool
   if ( fOK )
   {
     USHORT        sNumOfDocs;         // number of documents to be analyzed

     sNumOfDocs =  QUERYITEMCOUNTHWND( hwndLB );
     pTAInput->pPool = PoolCreate( 32000 );
     fOK = (pTAInput->pPool != NULL);
   } /* endif */

   /*******************************************************************/
   /* Add documents to pTAInput                                       */
   /*******************************************************************/
   if ( fOK )
   {
     USHORT         sIndex;             // index of files to be analyzed
     USHORT        sNumOfDocs;         // number of docuemnts to be analyzed
     HPROP         hPropDocument;      // handle to document properties
     PPROPDOCUMENT pPropDocument;      // pointer to documnet properties
     ULONG         ulErrorInfo;        // error indicator from PRHA
     OBJNAME       szDocObjName;       // document object name

     sIndex = 0;
     sNumOfDocs =  QUERYITEMCOUNTHWND( hwndLB );
     while ( (sIndex < sNumOfDocs) && fOK )
     {
        CHAR  szDocument[MAX_FILESPEC];

        QUERYITEMTEXTHWND( hwndLB, sIndex, szDocument );
        strcpy( pszAct, szDocument );
        sprintf( szDocObjName, "%s\\%s",pIda->szFolObjName, pszAct );

        if ( EqfQueryObject( szDocObjName, clsANY, 0 ) )
        {
           UtlErrorHwnd( ERROR_DOC_INUSE, MB_CANCEL, 1, &pszAct,
                         EQF_ERROR, pAnalysis->hwndErrMsg );
           fOK = FALSE;
        }
        else if( ( hPropDocument = OpenProperties( szDocument,
                                                    pIda->szFolObjName,
                                                    PROP_ACCESS_READ,
                                                    &ulErrorInfo) ) == NULL)
        {
           if ( ulErrorInfo != Err_NoStorage )
           {
              UtlErrorHwnd( ERROR_OPEN_PROPERTIES, MB_CANCEL, 1, &pszAct,
                            EQF_ERROR, pAnalysis->hwndErrMsg );
           } /* endif */
           fOK = FALSE;
        }
        else
        {
           pPropDocument =(PPROPDOCUMENT) MakePropPtrFromHnd( hPropDocument );
           if ( pPropDocument->szLongName[0] != EOS )
           {
             OEMTOANSI( pPropDocument->szLongName );
             pTAInput->apszLongNames[sIndex] =
                     PoolAddString( pTAInput->pPool,
                                    pPropDocument->szLongName );
             ANSITOOEM( pPropDocument->szLongName );
           }
           else
           {
             pTAInput->apszLongNames[sIndex] =
                     PoolAddString( pTAInput->pPool, szDocument );
           } /* endif */
           if ( pPropDocument->szAlias[0] != EOS )
           {
             pTAInput->apszAlias[sIndex] =
                PoolAddString( pTAInput->pPool,
                               pPropDocument->szAlias );
           } /* endif */

           if ( pPropDocument->ulTouched && !pAnalysis->fOverWrite )
           {
             PSZ pszErrParm = pTAInput->apszLongNames[sIndex];
             UtlErrorHwnd( ERROR_DOC_TOUCHED, MB_CANCEL, 1, &pszErrParm,
                           EQF_ERROR, pAnalysis->hwndErrMsg );
             fOK = FALSE;
           }
           else
           {
              pszAct += strlen(szDocument) + 1;
           } /* endif */
           CloseProperties( hPropDocument, PROP_QUIT, &ulErrorInfo );
        } /* endif */
        sIndex ++;
     } /* endwhile */
   } /* endif */

   if ( fOK )
   {
      //save offset to analysis interface
      pTAInput->stSourcefiles.ulOffset = pszStart - (PSZ)(pTAInput);

      //save length to analysis interface
      pTAInput->stSourcefiles.ulLength = pszAct - pszStart;
   }/* endif */

   if ( fOK )
   {
     PSZ pszTemp;                      // buffer for temporary pointers

      //--- save actual pointer to start pointer
      pszStart = pszAct;

      //save offset to analysis interface
      pTAInput->stInputDict.ulOffset = pszStart - (PSZ)(pTAInput);

      //save length to analysis interface
      pTAInput->stInputDict.ulLength = pszAct - pszStart;

      //--- save actual pointer to start pointer
      pszStart = pszAct;

      //--- save selected input mdbs to analysis interface
      {
        USHORT sIndex;                           //dictionary index

        for ( sIndex = 0; sIndex < pTAInput->stInputMemDb.usNumber;
              sIndex++ )
        {
           strcpy( pszAct, pAnalysis->ppMemArray[sIndex] );
           pszAct += strlen( pszAct ) + 1;
        }/*end for*/

        //save offset to analysis buffer
        pTAInput->stInputMemDb.ulOffset = pszStart - (PSZ)(pTAInput);

        //save length to analysis interface
        pTAInput->stInputMemDb.ulLength = pszAct - pszStart;
      }

      //--- save actual pointer to start pointer
      pszStart = pszAct;

      //--- get pointer to language from mdb properties
      pszTemp =  UtlParseX15( pIda->szMemProp, LANG_LANGUAGE_IND );

      //--- if no language in mdb properies
      if ( !pszTemp)
      {
         //--- save default morphological dicitonary to analysis interface
         pszStart = EMPTY_STRING;
      }
      else
      {
         //---  save name of language to analysis interface
         strcpy( pszStart, pszTemp );
      }/*end if*/

      //--- save length of language to analysis interface
      pTAInput->stMorphDict.ulLength = strlen( pszStart );

      //--- save offset to language analysis interface
      pTAInput->stMorphDict.ulOffset = pszStart - (PSZ)( pTAInput);

      // -- add segments to transl. memory CB is checked or
      //--- add exact matches to document cb is checked
   } /* endif */

   /*******************************************************************/
   /* Start analysis process                                          */
   /*******************************************************************/
   if ( fOK )
   {
     /*****************************************************************/
     /* Create analysis process window                                */
     /*****************************************************************/
     fOK = CreateProcessWindow( pIda->szObjName, AnalysisCallBack, pIda );
   } /* endif */

   /*******************************************************************/
   /* Cleanup                                                         */
   /*******************************************************************/
   if ( !fOK )
   {
      if ( hwndLB != NULLHANDLE ) WinDestroyWindow( hwndLB );
      if ( pTAInput != NULL )
      {
        if ( pTAInput->pPool ) PoolDestroy( pTAInput->pPool );
        if ( pTAInput->apszLongNames )
           UtlAlloc( (PVOID *)&(pTAInput->apszLongNames), 0L, 0L, NOMSG );
        if ( pTAInput->apszAlias )
           UtlAlloc( (PVOID *)&(pTAInput->apszAlias), 0L, 0L, NOMSG );
        if (pTAInput->pWSList)
           UtlAlloc( (PVOID *) &(pTAInput->pWSList), 0L, 0L, NOMSG );
        UtlAlloc( (PVOID *) &pTAInput, 0L, 0L, NOMSG) ;
      } /* endif */
      if ( pIda != NULL )     UtlAlloc( (PVOID *) &pIda, 0L, 0L, NOMSG) ;

      /****************************************************************/
      /* report end of task to DDE handler                            */
      /****************************************************************/
      WinPostMsg( pAnalysis->hwndOwner, WM_EQF_DDE_ANSWER, NULL,
                  MP2FROMP(&pAnalysis->DDEReturn) );
   } /* endif */

   if ( !fOK ) pAnalysis->DDEReturn.usRc = UtlGetDDEErrorCode( pAnalysis->hwndErrMsg );

   return( pAnalysis->DDEReturn.usRc );

} /* end of function TABatchAnalysis                                  */


USHORT TAFuncAnalyzeDoc
(
  PFCTDATA    pData,                   // function I/F session data
  PSZ         pszFolderName,           // name of folder
  PSZ         pszDocuments,            // list with document names or NULL
  PSZ         pszMemName,              // Translation Memory for analysis
  PSZ         pszProfile,              // analyiss profile name or NULL
  PSZ         pszMTOutputOptions,      // MT output options
  LONG        lOptions                 // options for analysis
)
{
  USHORT      usRC = NO_ERROR;         // function return code

  // prepare a new analysis run or continue current one
  if ( pData->fComplete )              // has last run been completed
  {
    // prepare a new analysis run
    pData->usProgress = 0;
    usRC = TAFuncPrepAnalysis( pData, pszFolderName, pszDocuments, pszMemName, pszProfile, pszMTOutputOptions, lOptions );
  }
  else
  {
    // continue current analysis process
    usRC = TAFuncAnalysisProcess( pData );
  } /* endif */
  return( usRC );
} /* end of function TAFuncAnalyzeDoc */

// prepare the function I/F analysis
USHORT TAFuncPrepAnalysis
(
  PFCTDATA    pData,                   // function I/F session data
  PSZ         pszFolderName,           // name of folder
  PSZ         pszDocuments,            // list with document names or NULL
  PSZ         pszMemName,              // Translation Memory for analysis
  PSZ         pszProfile,              // analyiss profile name or NULL
  PSZ         pszMTOutputOptions,      // MT output options
  LONG        lOptions                 // options for analysis
)
{
  PSZ         pszParm;                 // pointer for error parameters
  BOOL        fOK = TRUE;              // internal O.K. flag
  PTAINPUT    pTAInput = NULL;         // analysis input structure
  PSZ         pszAct, pszStart;        // ptr for TAINPUT buffer processing
  USHORT      usRC = NO_ERROR;         // function return code
  USHORT      usDocuments = 0;         // number of documents being analyzed
  PSZ         pDocNameBuffer = NULL;   // document name buffer
  LONG        lBufferSize = 0L;        // size of document buffer
  LONG        lBufferUsed = 0L;        // used bytes in document buffer
  OBJNAME     szFolObject;             // folder object name
  CHAR        szFolShortName[MAX_FILESPEC];// buffer for folder short name
  USHORT      usNumOfDicts = 0;        // number of dictionaries specified
  USHORT      usNumOfMems = 0;         // number of memory databases specified

  pszStart = NULL;
  pszAct = NULL;

  UtlSetUShort( QS_LASTERRORMSGID, 0 );


   // check if folder exists
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
       PSZ pszParm = pszFolderName;
       BOOL fIsNew;

       fIsNew = !SubFolNameToObjectName( pszFolderName,  pData->szObjName );

       if ( !fIsNew )
       {
         PSZ pszDelim;
         strcpy( pData->szObjName, pszFolderName );
         pszDelim = strchr( pData->szObjName, BACKSLASH );
         if ( pszDelim ) *pszDelim = EOS;
         ObjLongToShortName( pData->szObjName, szFolShortName, FOLDER_OBJECT, &fIsNew );
       } /* endif */

       if ( fIsNew )
       {
         fOK = FALSE;
         pszParm = pszFolderName;
         usRC = ERROR_XLATE_FOLDER_NOT_EXIST;
         UtlErrorHwnd( usRC, MB_CANCEL, 1, &pszParm, EQF_ERROR, HWND_FUNCIF );
       } /* endif */
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

       UtlMakeEQFPath( pData->szObjName, NULC, SYSTEM_PATH, NULL );
       strcat( pData->szObjName, BACKSLASH_STR );
       strcat( pData->szObjName, szFolShortName );
       strcat( pData->szObjName, EXT_FOLDER_MAIN );
       hpropFolder = OpenProperties( pData->szObjName, NULL,
                                     PROP_ACCESS_READ, &ulErrorInfo);
       if( hpropFolder )
       {
         ppropFolder = (PPROPFOLDER)MakePropPtrFromHnd( hpropFolder );
         if ( (ppropFolder->chDrive != EOS) && (ppropFolder->chDrive != ' ') )
         {
           pData->szObjName[0] = ppropFolder->chDrive;
         } /* endif */
         CloseProperties( hpropFolder, PROP_QUIT, &ulErrorInfo );
       } /* endif */
     }

  //  isolate current document name
     pszDocNameStart = pszDocuments;
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
         if ( *pszDocNameStart == DOUBLEQUOTE )
         {
           pszDocNameStart++;
           pszTemp = pszDocNameStart;
           while ( *pszTemp && (*pszTemp != DOUBLEQUOTE) )
           {
             pszTemp++;
           } /* endwhile */
           chTemp = *pszTemp;
           *pszTemp = EOS;
         }
         else
         {
           pszTemp = pszDocNameStart;
           while ( *pszTemp && (*pszTemp != COMMA) )
           {
             pszTemp++;
           } /* endwhile */
           chTemp = *pszTemp;
           *pszTemp = EOS;
         } /* endif */
       }

       if ( *pszDocNameStart != EOS)
       {
         CHAR szDocShortName[MAX_FILESPEC];

         FolLongToShortDocName( pData->szObjName, pszDocNameStart,
                                szDocShortName, &fIsNew );

         // TODO: Check if FolLongToShortDocName really works for all types
         //       of document names
         if ( fIsNew )
         {
           fOK = FALSE;
           pszParm = pszDocNameStart;
#ifdef SESSIONLOG
           UtlLogWriteString( "TAFuncPrepAnalysis: Error in FolLongToShortDocName for document %s", pszDocNameStart );
#endif
           usRC = ERROR_TA_SOURCEFILE;
           UtlErrorHwnd( usRC, MB_CANCEL, 1, &pszParm, EQF_ERROR, HWND_FUNCIF );
         }
         else
         {
           // add document short name to document name buffer
           LONG lAddLen = strlen(szDocShortName) + 1;
           if ( lBufferSize < (lBufferUsed + lAddLen) )
           {
             UtlAllocHwnd( (PVOID *)&pDocNameBuffer, lBufferSize,
                           lBufferSize + 8096L, ERROR_STORAGE, HWND_FUNCIF );
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
       if ( chTemp == DOUBLEQUOTE ) pszTemp++;
       pszDocNameStart = pszTemp;
     } /* endwhile */
   } /* endif */

   // check if TM(s) exist
   if ( fOK && (pszMemName != NULL) && (pszMemName[0] != EOS ))
   {
     // loop over list of specified TMs
     PSZ pszEnd = NULL;
     PSZ pszCurrent = pszMemName;
     PSZ pszListEnd = NULL;

     // cut off any list delimiters = parenthesis
     if ( *pszCurrent == '(' )
     {
       pszCurrent++;
       pszListEnd = pszCurrent + (strlen(pszCurrent) - 1);
       if ( *pszListEnd == ')' )
       {
         *pszListEnd = EOS;
       }
       else
       {
         pszListEnd = NULL;
       } /* endif */
     } /* endif */

     do
     {
       // isolate current name and check it
       while( isspace(*pszCurrent) ) pszCurrent++;
       if ( *pszCurrent )
       {
         pszEnd = strchr( pszCurrent, ',' );
         if ( pszEnd ) *pszEnd = EOS;

         if ( !UtlCheckIfExist( pszCurrent, TM_OBJECT ) )
         {
           fOK = FALSE;
           pszParm = pszCurrent;
           usRC = ERROR_MEMORY_NOTFOUND;
           UtlErrorHwnd( usRC, MB_CANCEL, 1, &pszParm, EQF_ERROR, HWND_FUNCIF );
         }
         else
         {
           usNumOfMems++;
         } /* endif */

         if ( pszEnd )
         {
           *pszEnd = ',';
           pszCurrent = pszEnd + 1;
         }
         else
         {
           pszCurrent += strlen(pszCurrent);
         } /* endif */
       } /* endif */
     } while ( fOK && *pszCurrent );
     if ( pszListEnd ) *pszListEnd = ')';
   } /* endif */

   // get number of documents if no specific documents have been specified
   if ( fOK )
   {
     // get folder object name
     SubFolNameToObjectName( pszFolderName, pData->szObjName );

     if ( usDocuments == 0 )
     {
       usDocuments = LoadDocumentNames( pData->szObjName, HWND_FUNCIF,
                                        LOADDOCNAMES_INCLSUBFOLDERS, (PSZ)&pDocNameBuffer );
     } /* endif */
   } /* endif */

   // allocate storage for analysis interface
   if ( fOK )
   {
     LONG lBufferSize = (usDocuments + usNumOfMems + usNumOfDicts + 1 + 1) * MAX_EQF_PATH;
     fOK = UtlAlloc( (PVOID *)&pTAInput, 0L,
                     (LONG)(sizeof(TAINPUT) + lBufferSize ), NOMSG );
     if ( !fOK )
     {
       usRC = ERROR_STORAGE;
       UtlErrorHwnd( usRC, MB_CANCEL, 0, NULL, EQF_ERROR, HWND_FUNCIF );
     } /* endif */
   } /* endif */

   // fill analysis input structure
   if ( fOK )
   {
     pData->pvAnaInputData            = pTAInput;
     pTAInput->pDDEAnalysis           = NULL;
     pTAInput->hwndErrMsg             = HWND_FUNCIF;
     pTAInput->fBatch                 = TRUE;
     pTAInput->szOutDictName[0]       = EOS;
     pTAInput->szNTLname[0]           = EOS;
     pTAInput->fNTLcontext            = FALSE;
     pTAInput->usNTLNumOccurences     = 0;
     pTAInput->szFTLname[0]           = EOS;
     pTAInput->fFTLcontext            = FALSE;
     pTAInput->usFTLNumOccurences     = 0;
     pTAInput->szExclusionList[0]     = EOS;
     pTAInput->szExclDictname[0]      = EOS;
     pTAInput->stInputDict.usNumber   = usNumOfDicts;
     pTAInput->stSourcefiles.usNumber = usDocuments;
     pTAInput->stInputMemDb.usNumber  = usNumOfMems;
     pTAInput->fInsertToTM            = (lOptions & ADDTOMEM_OPT) != 0;
     pTAInput->fInsertTMMatches       = (lOptions & AUTOSUBST_OPT) != 0;
     pTAInput->fUseLatestTMMatch      = (lOptions & AUTOLAST_OPT) != 0;
     pTAInput->fInsertNewMatch        = (lOptions & UNTRANSLATED_OPT) != 0;
     pTAInput->fAutoJoin              = (lOptions & AUTOJOIN_OPT) != 0;
     pTAInput->fNTLProcessing         = FALSE;
     pTAInput->fExactContextTMMatch   = (lOptions & AUTOCONTEXT_OPT) != 0;
     pTAInput->fTMMatch               = (lOptions & TMMATCH_OPT) != 0;
     pTAInput->fRedundCount           = (lOptions & REDUNDCOUNT_OPT) != 0;
     pTAInput->fLeadingWS             = (lOptions & ADJUSTLEADWS_OPT) != 0;
     pTAInput->fTrailingWS            = (lOptions & ADJUSTTRAILWS_OPT) != 0;
     pTAInput->fRespectCRLF           = (EQF_BOOL)((lOptions & RESPECTCRLF_OPT) != 0);
     pTAInput->pWSList = NULL;
     pTAInput->fIgnoreIdentical       = (lOptions & NOSUBSTIFIDENTICAL_OPT) != 0;
     pTAInput->fNoAddBlank            = (lOptions & NOBLANKATSEGEND_OPT) != 0;
     pTAInput->fMachTrans             = (lOptions & SENDTOMT_OPT) != 0;

     if ( (lOptions & PROTECTXMPSCREEN_OPT) != 0 )
     {
       pTAInput->fProtXmp             = TRUE;
       pTAInput->fProtMsgNum          = TRUE;
       pTAInput->fProtMeta            = TRUE;
       pTAInput->fProtScreen          = TRUE;
       pTAInput->fProtCodeBlock       = TRUE;
     }
     else
     {
       pTAInput->fProtXmp             = (lOptions & PROTECTXMP_OPT ) != 0;      /* 4-13-14 */
       pTAInput->fProtMsgNum          = (lOptions & PROTECTMSGNUM_OPT ) != 0;
       pTAInput->fProtMeta            = (lOptions & PROTECTMETA_OPT ) != 0;
       pTAInput->fProtScreen          = (lOptions & PROTECTSCREEN_OPT ) != 0;
       pTAInput->fProtCodeBlock       = (lOptions & PROTECTCODEBLOCK_OPT ) != 0;
     }
     pTAInput->fIgnorePath            = (lOptions & IGNOREPATH_OPT) != 0;
     pTAInput->fStopAtFirstExact      = (lOptions & STOPATFIRSTEXACT_OPT) != 0;
     pTAInput->fStopAtFirstExactAddMem = (lOptions & STOPATFIRSTEXACT_OPT) != 0;
     pTAInput->fIgnoreComment         = (lOptions & IGNORECOMMENTED_OPT) != 0;

     memcpy( pTAInput->TATag, TADummyTag, sizeof(TATAG) * MAX_SEG_CLASSES );
     TATATag2Unicode( pTAInput->TATag, pTAInput->TATagW );

     if ( pszProfile != NULL )
     {
       strcpy( pTAInput->szProfile, pszProfile );
       pTAInput->fProfile = TRUE;
     } /* endif */

     if ( pszMTOutputOptions != NULL ) strcpy( pTAInput->szMtOptions, pszMTOutputOptions );

     if ( pTAInput->stSourcefiles.usNumber == 0 )  //no files in analysis LB
     {
       UtlErrorHwnd( ERROR_NODOCS_ANALYSIS,
                     MB_CANCEL, 0, NULL, EQF_WARNING, HWND_FUNCIF );
       fOK = FALSE;
     } /* endif */
   } /* endif */

   if ( fOK && pTAInput->fInsertToTM && (pTAInput->stInputMemDb.usNumber == 0) )
   {
     UtlErrorHwnd( ERROR_NO_INMDB_SELECTED, MB_CANCEL,
                   0, NULL, EQF_ERROR, HWND_FUNCIF );
     fOK = FALSE;
   } /* endif */

   // retrieve values from folder properties
   if ( fOK )
   {
      PPROPFOLDER  ppropFolder;        // pointer to folder properties
      HPROP        hpropFolder;        // folder properties handle
      ULONG        ulErrorInfo;        // error indicator from property handler

      UtlMakeEQFPath( szFolObject, NULC, SYSTEM_PATH, NULL );
      strcat( szFolObject, BACKSLASH_STR );
      strcat( szFolObject, szFolShortName );

      strcat( szFolObject, EXT_FOLDER_MAIN );
      strcpy( pTAInput->szFolderProp, szFolObject );

      if( ( hpropFolder = OpenProperties( szFolObject, NULL,
                                           PROP_ACCESS_READ, &ulErrorInfo))== NULL)
      {
         // Error accessing folder properties
         if ( ulErrorInfo != Err_NoStorage )
         {
            PSZ pError = pszFolderName;
            UtlErrorHwnd( ERROR_PROPERTY_ACCESS, MB_CANCEL, 1,
                          &pError, EQF_ERROR, HWND_FUNCIF );
         } /* endif */
         fOK = FALSE;
      }
      else
      {
        // Access to folder properties successful, now get some
        // data from folder properties
        ppropFolder =(PPROPFOLDER) MakePropPtrFromHnd( hpropFolder );
        pTAInput->sMatchLevel = ppropFolder->usMatchLevel;

        UtlMakeEQFPath( pTAInput->szTagTableName, NULC, TABLE_PATH, NULL);
        strcat( pTAInput->szTagTableName, BACKSLASH_STR );
        strcat( pTAInput->szTagTableName, ppropFolder->szFormat );
        strcat( pTAInput->szTagTableName, EXT_OF_FORMAT );

        // Correct drive letter of folder object name
        if ( (ppropFolder->chDrive != EOS) && (ppropFolder->chDrive != ' ') )
        {
          szFolObject[0] = ppropFolder->chDrive;
        } /* endif */


        if ( pTAInput->fInsertToTM || pTAInput->fInsertTMMatches ||
             pTAInput->fInsertNewMatch || pTAInput->fMachTrans )
        {
          if ( ppropFolder->szLongMemory[0] != EOS )
          {
            strcpy( pTAInput->szOutputMemDb, ppropFolder->szLongMemory ); 
          }
          else
          {
            strcpy( pTAInput->szOutputMemDb, ppropFolder->szMemory );
          } /* endif */
        } /* endif */
        CloseProperties( hpropFolder, PROP_QUIT, &ulErrorInfo );
      } /* endif */
   } /* endif */

   if ( fOK )
   {

     PSZ pszFolder =  UtlGetFnameFromPath( szFolObject );

     strcpy( pTAInput->szFolder, szFolObject );
     UtlMakeEQFPath( pTAInput->szSOURCE_Path, szFolObject[0],
                     DIRSOURCEDOC_PATH,
                     pszFolder );
     UtlMakeEQFPath( pTAInput->szSEGSOURCE_Path, szFolObject[0],
                     DIRSEGSOURCEDOC_PATH,
                     pszFolder );
     UtlMakeEQFPath( pTAInput->szSEGTARGET_Path, szFolObject[0],
                     DIRSEGTARGETDOC_PATH,
                     pszFolder );
     UtlMakeEQFPath( pTAInput->szSEGNEWMATCH_Path, szFolObject[0],
                     DIRSEGNOMATCH_PATH,
                     pszFolder );
     UtlMakeEQFPath( pTAInput->szMACHTRANS_Path, szFolObject[0], DIRSEGMT_PATH, pszFolder);

      //set actual pointer to analysis IF data buffer
      pszAct = (PSZ)(pTAInput) + sizeof( TAINPUT );
      pszStart = pszAct;
   } /* endif */

   // allocate pointer array for long names
   if ( fOK )
   {
     LONG lSize = (LONG)sizeof(PSZ) * (LONG)usDocuments;
     if ( lSize < MIN_ALLOC ) lSize = MIN_ALLOC;
     fOK = UtlAllocHwnd( (PVOID *)&(pTAInput->apszLongNames), 0L, lSize,
                         ERROR_STORAGE, HWND_FUNCIF );
     if ( fOK )
     {
       // allocate pointer array for alias names
       fOK = UtlAllocHwnd( (PVOID *)&(pTAInput->apszAlias), 0L, lSize,
                       ERROR_STORAGE, HWND_FUNCIF );
     } /* endif */
   } /* endif */

   // create string pool
   if ( fOK )
   {
     pTAInput->pPool = PoolCreate( 16000 );
     fOK = (pTAInput->pPool != NULL);
   } /* endif */

   // Add documents to pTAInput
   if ( fOK )
   {
     HPROP         hPropDocument;      // handle to document properties
     PPROPDOCUMENT pPropDocument;      // pointer to documnet properties
     ULONG         ulErrorInfo;        // error indicator from PRHA
     OBJNAME       szDocObjName;       // document object name
     PSZ           pszDocName = pDocNameBuffer;
     USHORT        i = 0;

     while ( *pszDocName && (i < usDocuments) && fOK )
     {
        strcpy( pszAct, pszDocName );
        sprintf( szDocObjName, "%s\\%s", szFolObject, pszAct );

        if ( EqfQueryObject( szDocObjName, clsANY, 0 ) )
        {
           UtlErrorHwnd( ERROR_DOC_INUSE, MB_CANCEL, 1, &pszAct,
                         EQF_ERROR, HWND_FUNCIF );
           fOK = FALSE;
        }
        else if( ( hPropDocument = OpenProperties( pszDocName,
                                                    szFolObject,
                                                    PROP_ACCESS_READ,
                                                    &ulErrorInfo) ) == NULL)
        {
           if ( ulErrorInfo != Err_NoStorage )
           {
              UtlErrorHwnd( ERROR_OPEN_PROPERTIES, MB_CANCEL, 1, &pszAct,
                            EQF_ERROR, HWND_FUNCIF );
           } /* endif */
           fOK = FALSE;
        }
        else
        {
           pPropDocument = (PPROPDOCUMENT) MakePropPtrFromHnd( hPropDocument );

           if ( pPropDocument->szLongName[0] != EOS )
           {
             OEMTOANSI( pPropDocument->szLongName );
             pTAInput->apszLongNames[i] =
                     PoolAddString( pTAInput->pPool,
                                    pPropDocument->szLongName );
             ANSITOOEM( pPropDocument->szLongName );
           }
           else
           {
             pTAInput->apszLongNames[i] =
                     PoolAddString( pTAInput->pPool, pszDocName );
           } /* endif */
           if ( pPropDocument->szAlias[0] != EOS )
           {
             pTAInput->apszAlias[i] =
                PoolAddString( pTAInput->pPool,
                               pPropDocument->szAlias );
           } /* endif */

           if ( pPropDocument->ulTouched && !(lOptions & OVERWRITE_OPT) )
           {
             PSZ pszErrParm = pTAInput->apszLongNames[i];
             UtlErrorHwnd( ERROR_DOC_TOUCHED, MB_CANCEL, 1, &pszErrParm,
                           EQF_ERROR, HWND_FUNCIF );
             fOK = FALSE;
           }
           else
           {
              pszAct += strlen(pszDocName) + 1;
           } /* endif */
           CloseProperties( hPropDocument, PROP_QUIT, &ulErrorInfo );
        } /* endif */
        pszDocName = pszDocName + (strlen(pszDocName) + 1);
        i++;
     } /* endwhile */
   } /* endif */

   if ( fOK )
   {
      //save offset to analysis interface
      pTAInput->stSourcefiles.ulOffset = pszStart - (PSZ)(pTAInput);

      //save length to analysis interface
      pTAInput->stSourcefiles.ulLength = pszAct - pszStart;
   }/* endif */

   if ( fOK )
   {
      //--- save actual pointer to start pointer
      pszStart = pszAct;

      //save offset to analysis interface
      pTAInput->stInputDict.ulOffset = pszStart - (PSZ)(pTAInput);

      //save length to analysis interface
      pTAInput->stInputDict.ulLength = pszAct - pszStart;

      //--- save actual pointer to start pointer
      pszStart = pszAct;

      //--- save selected input mdbs to analysis interface
      {
        if ( (pszMemName != NULL) && (*pszMemName != EOS) )
        {
         // loop over list of specified TMs
         PSZ pszEnd = NULL;
         PSZ pszCurrent = pszMemName;
         PSZ pszListEnd = NULL;

         // cut off any list delimiters = parenthesis
         if ( *pszCurrent == '(' )
         {
           pszCurrent++;
           pszListEnd = pszCurrent + (strlen(pszCurrent) - 1);
           if ( *pszListEnd == ')' )
           {
             *pszListEnd = EOS;
           }
           else
           {
             pszListEnd = NULL;
           } /* endif */
         } /* endif */

         do
         {
           // isolate current name and check it
           while( isspace(*pszCurrent) ) pszCurrent++;
           if ( *pszCurrent )
           {
             pszEnd = strchr( pszCurrent, ',' );
             if ( pszEnd ) *pszEnd = EOS;

             strcpy( pszAct, pszCurrent );
             pszAct += strlen(pszAct) + 1;

             if ( pszEnd )
             {
               *pszEnd = ',';
               pszCurrent = pszEnd + 1;
             }
             else
             {
               pszCurrent += strlen(pszCurrent);
             } /* endif */
           } /* endif */
         } while ( fOK && *pszCurrent );
         if ( pszListEnd ) *pszListEnd = ')';
        } /* endif */

        //save offset to analysis buffer
        pTAInput->stInputMemDb.ulOffset = pszStart - (PSZ)(pTAInput);

        //save length to analysis interface
        pTAInput->stInputMemDb.ulLength = pszAct - pszStart;
      }

      //--- save actual pointer to start pointer
      pszStart = pszAct;

      // no default morphological dictionary anymore
      // morph dict is determined by folder/document sourc language
      pszStart = EMPTY_STRING;

      //--- save length of language to analysis interface
      pTAInput->stMorphDict.ulLength = strlen( pszStart );

      //--- save offset to language analysis interface
      pTAInput->stMorphDict.ulOffset = pszStart - (PSZ)( pTAInput);

      // -- add segments to transl. memory CB is checked or
      //--- add exact matches to document cb is checked
   } /* endif */

   /*******************************************************************/
   /* Cleanup                                                         */
   /*******************************************************************/
   if ( pDocNameBuffer ) UtlAlloc( (PVOID *)&pDocNameBuffer, 0L, 0L, NOMSG );
   if ( !fOK )
   {
      if ( pTAInput != NULL )
      {
        if ( pTAInput->pPool ) PoolDestroy( pTAInput->pPool );
        if ( pTAInput->apszLongNames )
           UtlAlloc( (PVOID *)&(pTAInput->apszLongNames), 0L, 0L, NOMSG );
        if ( pTAInput->apszAlias )
           UtlAlloc( (PVOID *)&(pTAInput->apszAlias), 0L, 0L, NOMSG );
        if (pTAInput->pWSList)
           UtlAlloc( (PVOID *) &(pTAInput->pWSList), 0L, 0L, NOMSG );
        UtlAlloc( (PVOID *) &pTAInput, 0L, 0L, NOMSG) ;
      } /* endif */
   } /* endif */

   if ( fOK )
   {
     pData->fComplete = FALSE;
     pData->usAnalysisPhase = TAFUNC_INIT;
   }
   else
   {
     usRC = UtlQueryUShort( QS_LASTERRORMSGID );
   } /* endif */
   return( usRC );

} /* end of function TAFuncPrepAnalysis */

VOID  TATATag2Unicode
(
  PTATAG    pTATagTemp,
  PTATAG_W  pTATagW
)
{
  while ( pTATagTemp->chSTag[0]  )
  {
    ASCII2Unicode( pTATagTemp->chSTag, pTATagW->chSTag, 0L );
    ASCII2Unicode( pTATagTemp->chETag, pTATagW->chETag, 0L );
    pTATagTemp++;
    pTATagW++;
  }
  return;
}
