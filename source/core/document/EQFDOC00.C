/*! \file
	Description: The document anchor creates document instances and handles the EQF and PM messaging.

	Copyright Notice:

	Copyright (C) 1990-2016, International Business Machines
	Corporation and others. All rights reserved
*/

#define INCL_EQF_TM               // general Transl. Memory functions
#define INCL_EQF_TP               // public translation processor functions
#define INCL_EQF_ANALYSIS         // analysis functions
#define INCL_EQF_ASD              // dictionary access functions (Asd...)
#define INCL_EQF_EDITORAPI        // editor API
#define INCL_EQF_FOLDER           // folder list and document list functions
#define INCL_EQF_MORPH            // morphologic functions
#include <eqf.h>                  // General Translation Manager include file
#ifdef FUNCCALLIF
  #include "OTMFUNC.H"            // public defines for function call interface
  #include "EQFFUNCI.H"           // private defines for function call interface
#endif
#include <time.h>
#include "tchar.h"

/**********************************************************************/
/* The next two includes are necessary due to inconsistent or         */
/* not existing interfaces ...                                        */
/**********************************************************************/
#include <eqftpi.h>               // private translation processor include file
#include "EQFDDE.H"               // Batch mode definitions
#include <eqftai.h>               // private text analysis include file

#include "eqfdoc00.h"             // private document handler include file
#include "EQFDDE.H"               // for DDE interface
#include <TIME.H>                 // C time functions
#include <eqfbdlg.id>             // get id for slider dialog
#include "eqfhlog.h"              // history log record layout
#include "eqfstart.id"            // some IDs we are using here

#include <sys\stat.h>             // for time/date function on files

#include <eqfprogr.h>           // get slider definitions
#include <OTMGLOBMem.h>         // Global Memory defines
#include "..\editor\ReImportDoc.h" // Re-Import document defines


static MRESULT DocInitInst ( HWND, PSZ );    // init document instance
static BOOL StartEditor ( PDOCUMENT_IDA );       // start our transl. editor
static BOOL LoadEditor ( PDOCUMENT_IDA );
static MRESULT DocCreateInst( HWND, WPARAM, PSZ, PVOID); // create document instance
static BOOL DocFindEdit ( PDOCUMENT_IDA );   // determine editor to be used
static VOID ReleaseDocAccess ( PSZ );        // release document access
static VOID EQFFontChange ( HWND );          // font changed
static SWP  swpOldStatus;

static BOOL EQFTextSegm( PDOCUMENT_IDA );    // do on the spot text segmentation
static VOID EQFXlateError ( USHORT );

static MRESULT CreateDocInstWnd ( PSZ, PVOID, PHWND, PSZ );
static MRESULT CreateListOfSelDocs ( HWND, PSZ, PSZ **, PPOOL );

static BOOL fEditorClosed[MAX_TASK] = {FALSE}; // editor closing indicator
static BOOL fTerminate[MAX_TASK]    = {FALSE}; // no termination call active
// statics and defines used for editor close down
                                             // address of editor end Dll
static EQF_BOOL (*pfnEditShutDown[MAX_TASK])(PSZ, PSTEQFGEN, BOOL);

static HMODULE hmodEditDll[MAX_TASK] = {NULLHANDLE}; // editor dll active
static CHAR szEditDll[MAX_TASK][9] = {""};     // name of editor dll
static  HWND       hWaitDlg = NULLHANDLE;          // wait dialog ...
static BOOL bProfileRead = FALSE;

#define TIMECLOSE     400;                  // timeout setting

__declspec(dllexport)
EQF_BOOL __cdecl /*APIENTRY*/ EQF_XSTART(  PSZ, PSZ, PSZ, PSTEQFGEN);   // start editor
USHORT DocBatchDocumentOpen( HWND hwndParent, PDDEDOCOPEN pDocOpen );

USHORT DocBatchDocumentDelete( HWND hwndParent, PDDEDOCDEL pDocDel );
INT_PTR CALLBACK ANALYSISWAITDLG ( HWND, WINMSG, WPARAM, LPARAM );
MRESULT DocAnalyseCallBack
(
  PPROCESSCOMMAREA pCommArea,          // ptr to commmunication area
  HWND             hwnd,               // handle of process window
  WINMSG           message,            // message to be processed
  WPARAM           mp1,                // first message parameter
  LPARAM           mp2                 // second message parameter
);

static  PDOCUMENT_IDA pStaticDoc = NULL;
VOID PASCAL FAR DocCleanUp( USHORT usTermCode );

VOID DocStopWaitDlg( VOID );
USHORT DocGetFileDateAndSize
(
  PSZ         pszPath,                 // path of file being checked
  PLONG       plFileDate,              // ptr to buffer for long date
  PLONG       plFileSize               // ptr to buffer for file size
);


// macro to create dictionary property name w/o property path
#define PROPDICT( pszBuffer, pszDict )                       \
{                                                            \
   UtlMakeEQFPath( pszBuffer, NULC, SYSTEM_PATH, NULL );     \
   strcat( pszBuffer, "\\" );                                \
   strcat(pszBuffer, UtlGetFnameFromPath( pszDict ) );       \
}

//------------------------------------------------------------------------------
// Document Handler Window Proc
//------------------------------------------------------------------------------
MRESULT APIENTRY DOCUMENTHANDLERWP
( HWND hwnd, WINMSG message, WPARAM mp1, LPARAM mp2)
{
    PIDA_HEAD       pIda;                    // pointer to instance area
    PSZ             pszObjName;              // pointer to document object name
    MRESULT         mResult = MRFROMSHORT(FALSE);   // WP result
    BOOL            fOK = TRUE;              // success indicator
    PSZ             pBuf;                    // pointer to temp. buffer
    HAB             hab;                         // anchor block handle
    LONG            lLen;                    // length of offset
    PSTEQFGEN       pstEQFGen;               // pointer to generic struct
    PEQFXLATE       pEQFXlate;               // pointer to window handle
    PDOCIMPEXP      pDocImpExp;              // document import instance
    PSZ             *ppFile;                 // pointer to file
    USHORT          usTask;                  // active task
    USHORT          usMBReturn;              // message box return code

    switch( message)
    {
       //----------------------------------------------------------------------
     case WM_CREATE:
       {
         HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);

           DosExitList(EXLST_ADD, (PFNEXITLIST)DocCleanUp );// adds address to the list

          pfnEditShutDown[UtlGetTask()] = NULL;

           // allocate storage for anchor IDA header
          fOK = UtlAlloc( (PVOID *) (PVOID *)&pIda, 0L,
                              (ULONG)sizeof(*pIda), ERROR_STORAGE );
          if ( fOK )
          {
            ANCHORWNDIDA( hwnd, pIda);
          }/*endif*/
          if ( fOK )
          {
            /**********************************************************/
            /* allocate shared segment and set hwnd                   */
            /**********************************************************/
            fOK = ! EQF_Allocate (TRUE);
          } /* endif */
          if ( fOK )
          {
             pIda->hFrame = hwnd;         // save client handle

             //save document object name of achor to anchor IDA header
             pIda->pszObjName = strcpy( pIda->szObjName, DOCUMENTHANDLER );

             //install document anchor
             EqfInstallHandler( DOCUMENTHANDLER, hwnd, clsDOCUMENT);

             {
               WNDCLASS   wndclass;
               ATOM       atom;

               wndclass.style         = 0;
               wndclass.lpfnWndProc   = DOCUMENTWP;
               wndclass.cbClsExtra    = 0;
               wndclass.cbWndExtra    = sizeof(PVOID);
               wndclass.hInstance     = (HINSTANCE)(HAB)UtlQueryULong( QL_HAB );
               wndclass.hIcon         = LoadIcon( hResMod, MAKEINTRESOURCE( DOC_ICON ) );
               wndclass.hCursor       = LoadCursor( NULL, IDC_ARROW );
               wndclass.hbrBackground = (HBRUSH)GetStockObject( GRAY_BRUSH );
               wndclass.lpszMenuName  = NULL;
               wndclass.lpszClassName = DOCUMENT;

               atom = RegisterClass( &wndclass );
               if ( atom == 0 )
               {
                 fOK = FALSE;
               } /* endif */
               }
            } /* endif */
            if ( fOK )
            {
               //register service window class
              InitUnicode();
              {
                 WNDCLASSW  wndclass;
                 ATOM       atom;

                 wndclass.style         = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
                 wndclass.lpfnWndProc   = TWBSERVICEWP;
                 wndclass.cbClsExtra    = 0;
                 wndclass.cbWndExtra    = sizeof(PVOID);
                 wndclass.hInstance     = (HINSTANCE)(HAB)UtlQueryULong( QL_HAB );
                 wndclass.hIcon         = LoadIcon( hResMod, MAKEINTRESOURCE( DOC_ICON ) );
                 wndclass.hCursor       = LoadCursor( NULL, IDC_ARROW );
                 wndclass.hbrBackground = (HBRUSH)GetStockObject( WHITE_BRUSH );
                 wndclass.lpszMenuName  = NULL;
                 wndclass.lpszClassName = TWBS_W;

                 atom = RegisterClassW( &wndclass );
                 if ( atom == 0 )
                 {
                   fOK = FALSE;
                 } /* endif */

                 if ( fOK )
                 {
                   wndclass.style         = CS_DBLCLKS;
                   wndclass.lpfnWndProc   = TWBSERVICEWPRTF;
                   wndclass.cbClsExtra    = 0;
                   wndclass.cbWndExtra    = sizeof(PVOID);
                   wndclass.hInstance     = (HINSTANCE)(HAB)UtlQueryULong( QL_HAB );
                   wndclass.hIcon         = LoadIcon( hResMod, MAKEINTRESOURCE( DOC_ICON ) );
                   wndclass.hCursor       = LoadCursor( NULL, IDC_ARROW );
                   wndclass.hbrBackground = (HBRUSH)GetStockObject( WHITE_BRUSH );
                   wndclass.lpszMenuName  = NULL;
                   wndclass.lpszClassName = TWBSRTF_W;

                   atom = RegisterClassW( &wndclass );
                   if ( atom == 0 )
                   {
                     fOK = FALSE;
                   } /* endif */
                 } /* endif */
		       }
            } /* endif */

            /************************************************************/
            /* register editor class                                    */
            /************************************************************/
            if ( fOK )
            {
              fOK = EQFBInit();
            } /* endif */

            /************************************************************/
            /* start processing...                                      */
            /************************************************************/
            if ( fOK )
            {
              UtlDispatch();
              WinPostMsg( hwnd, WM_EQF_PROCESSTASK,
                          MP1FROMSHORT(EQFXLATE_START), NULL );
            } /* endif */
            mResult = ( fOK ) ? (MRESULT) FALSE : MRFROMSHORT( -1 );
          }
          break;
       //----------------------------------------------------------------------
       case WM_DESTROY:
          /************************************************************/
          /* free resources ...                                       */
          /************************************************************/
          pIda = (PIDA_HEAD)ACCESSWNDIDA( hwnd, PVOID );
          if ( pIda )
          {
            UtlAlloc( (PVOID *) &pIda, 0L, 0L, NOMSG );
          } /* endif */
          break;
       //----------------------------------------------------------------------
        case  WM_EQF_DDE_REQUEST:
          /************************************************************/
          /*     mp1:  (DDETASK) Task                                 */
          /*     mp2:  (PVOID) pTaskIda                               */
          /************************************************************/
          switch ( SHORT1FROMMP1( mp1 ) )
          {
            case  TASK_DOCIMP:
            case  TASK_DOCEXP:
            case  TASK_DOCDEL:
              WinPostMsg( hwnd, WM_EQF_NEXTSTEP, mp1, mp2 );
              break;
            case  TASK_DOCOPEN:
              {
                PDDEDOCOPEN pDocOpen = (PDDEDOCOPEN)mp2;
                DocBatchDocumentOpen( hwnd, pDocOpen );
              }
              break;
            default :
              break;
          } /* endswitch */
          break;

          /************************************************************/
          /*  this message handles the different Batch mode requests  */
          /*    mp1:  task to be accomplished                         */
          /*    mp2: pointer to structure                             */
          /************************************************************/
        case  WM_EQF_NEXTSTEP:
          usTask =  SHORT1FROMMP1( mp1 );
          switch ( usTask )
          {
            case  TASK_DOCDEL:
              {
                PDDEDOCDEL pDocDel = (PDDEDOCDEL)mp2;
                ppFile = pDocDel->ppFileArray;
                if ( *(ppFile+pDocDel->usActFile) )
                {
                  // delete the current document
                  fOK = (DocBatchDocumentDelete( hwnd, pDocDel ) == NO_ERROR );

                  if ( fOK )
                  {
                    pDocDel->usActFile++;       // point to next file
                    UtlDispatch();
                    WinPostMsg( hwnd, WM_EQF_NEXTSTEP,
                                MP1FROMSHORT( TASK_DOCDEL ), mp2);
                  }
                  else
                  {
                    WinPostMsg( pDocDel->hwndOwner, WM_EQF_DDE_ANSWER,
                                0, &pDocDel->DDEReturn);
                  } /* endif */
                }
                else
                {
                  WinPostMsg( pDocDel->hwndOwner, WM_EQF_DDE_ANSWER,
                              0, &pDocDel->DDEReturn);
                } /* endif */
              }
              break;
            case  TASK_DOCIMP:
            case  TASK_DOCEXP:
              pDocImpExp = (PDOCIMPEXP) mp2;
              ppFile = pDocImpExp->ppFileArray;
              if ( *(ppFile+pDocImpExp->usActFile) )
              {
                /******************************************************/
                /* call the function to be processed ..               */
                /******************************************************/
                if ( usTask == TASK_DOCIMP )
                {
                  fOK = DDEDocLoad( pDocImpExp );
                }
                else
                {
                  fOK = DDEDocUnLoad( pDocImpExp );
                } /* endif */
                /******************************************************/
                /* according to return code do ...                    */
                /******************************************************/
                if ( fOK )
                {
                  pDocImpExp->usActFile++;       // point to next file
                  UtlDispatch();
                  WinPostMsg( hwnd, WM_EQF_NEXTSTEP,
                              MP1FROMSHORT( usTask ), mp2);
                }
                else
                {
                  /********************************************************/
                  /* error message is stored in pDocImpExp->DDEReturn     */
                  /********************************************************/
                  WinPostMsg( pDocImpExp->hwndOwner, WM_EQF_DDE_ANSWER,
                              0, &pDocImpExp->DDEReturn);
                } /* endif */
              }
              else
              {
                WinPostMsg( pDocImpExp->hwndOwner, WM_EQF_DDE_ANSWER,
                            0, &pDocImpExp->DDEReturn);
              } /* endif */
              break;
            default :
              break;
          } /* endswitch */
          break;

       //----------------------------------------------------------------------
       case WM_EQF_PROCESSTASK:
          pstEQFGen = EQFGETSTRUCT();
          fOK = (pstEQFGen  != NULL );
          if ( fOK )
          {
            lLen = sizeof(STEQFGEN)+sizeof(STEQFPCMD)+sizeof(STEQFDUMP);
            pEQFXlate = (PEQFXLATE)((PSZ)pstEQFGen + lLen);
            switch ( SHORT1FROMMP1(mp1) )
            {
              case  EQFXLATE_START:
                /**********************************************************/
                /* access shared segment and set hwnd                     */
                /**********************************************************/
                pEQFXlate->hab = (HAB) UtlQueryULong(QL_HAB);
                pEQFXlate->hwnd = hwnd;
                pEQFXlate->hwndTWB = EqfQueryTwbFrame();
                break;
              case  EQFXLATE_UNSEGM:
                /**********************************************************/
                /* unsegment and export the document                      */
                /**********************************************************/
                fOK = DocUnSegm( pEQFXlate->szFolderName,
                                 pEQFXlate->szDocumentName,
                                 pEQFXlate->szOutName);
                if ( !fOK )
                {
                  pEQFXlate->usRc = EQFXLATE_UNSEGM_ERROR;
                } /* endif */
                pEQFXlate->fBusy = FALSE;                   // we are done
                /**********************************************  ********/
                /* if old status was MINIMIZED, so do it again          */
                /********************************************************/
                if ( SWP_FLAG(swpOldStatus) & EQF_SWP_MINIMIZE )
                {
                  WinSetWindowPos( pEQFXlate->hwndTWB,
                                   swpOldStatus.hwndInsertBehind,
                                   swpOldStatus.x,
                                   swpOldStatus.y,
                                   swpOldStatus.cx,
                                   swpOldStatus.cy,
                                   SWP_FLAG(swpOldStatus) );
                } /* endif */
                break;
              case  EQFXLATE_DOC:
                /******************************************************/
                /* get window positions ...                           */
                /******************************************************/
                pEQFXlate->hwndTWB = EqfQueryTwbFrame();
                WinQueryWindowPos( pEQFXlate->hwndTWB, &swpOldStatus );
                /**********************************************************/
                /* import the document                                     */
                /***********************************************************/
                fOK = DocLoad( pEQFXlate->szFolderName,
                               pEQFXlate->szDocumentName);
                /***********************************************************/
                /* start it                                                */
                /***********************************************************/
                if ( fOK )
                {
                  /*********************************************************/
                  /* check that MAT is not minimized -- if so maximize it  */
                  /*********************************************************/
                  if ( SWP_FLAG(swpOldStatus) & EQF_SWP_MINIMIZE )
                  {
                    WinSetWindowPos( pEQFXlate->hwndTWB,
                                     swpOldStatus.hwndInsertBehind,
                                     (SHORT)swpOldStatus.x,
                                     (SHORT)swpOldStatus.y,
                                     (SHORT)swpOldStatus.cx,
                                     (SHORT)swpOldStatus.cy,
                                     (USHORT)((SWP_FLAG(swpOldStatus) & ~EQF_SWP_MINIMIZE) |
                                                           EQF_SWP_MAXIMIZE));
                  } /* endif */
                  fOK = ! ((BOOL) DocCreateInst ( hwnd, 0,
                                                  pEQFXlate->szDocumentName,
                                                  NULL));
                } /* endif */
                /************************************************************/
                /* if something went drastically wrong  --> reset fBusy flag*/
                /************************************************************/
                if ( !fOK )
                {
                  pEQFXlate->fBusy = FALSE;
                  pEQFXlate->usRc = EQFXLATE_TRANSL_ERROR ;
                  /**********************************************  ********/
                  /* if old status was MINIMIZED, so do it again          */
                  /********************************************************/
                  if ( SWP_FLAG(swpOldStatus) & EQF_SWP_MINIMIZE )
                  {
                    WinSetWindowPos( pEQFXlate->hwndTWB,
                                     swpOldStatus.hwndInsertBehind,
                                     swpOldStatus.x,
                                     swpOldStatus.y,
                                     swpOldStatus.cx,
                                     swpOldStatus.cy,
                                     SWP_FLAG(swpOldStatus) );
                  } /* endif */
                } /* endif */
                break;
              case  OPEN_AND_POSITION_TASK:
                {
                  /****************************************************/
                  /* mp2 contains POPENANDPOS structure containing    */
                  /* file name and positions                          */
                  /****************************************************/
                  POPENANDPOS pOpenAndPos = (POPENANDPOS) mp2;
                  DocCreateInst( hwnd,  OPENPOS_USED, pOpenAndPos->szDocName, PVOIDFROMMP2(mp2) );
                }
                break;
              case  REIMPORT_TASK:
                {
                  // mp2 contains re-import handle 
                  ReImport_Process( hwnd,  PVOIDFROMMP2(mp2) );
                }
                break;
              default :
                break;
            } /* endswitch */
          }
          else
          {
            /**********************************************************/
            /* display error message -> access failed to Shared segm. */
            /**********************************************************/
          } /* endif */
          break;

       //----------------------------------------------------------------------
       case WM_EQF_SHUTDOWN:                //come here before EQF_TERMINATE
          // send to the instance that termination is required
          EqfSend2AllObjects(clsDOCUMENT, message, mp1, mp2);
          break;
       //----------------------------------------------------------------------
       case WM_EQF_TERMINATE:
         // stop old editor if available
         pIda = ACCESSWNDIDA( hwnd, PIDA_HEAD );
         if ( pfnEditShutDown[UtlGetTask()] )
         {
            fOK = UtlAlloc( (PVOID *) (PVOID *)&pBuf,
                                0L, (ULONG)MAX_LONGPATH, ERROR_STORAGE );
            if ( fOK )
             {
               UtlMakeEQFPath( pBuf, NULC, PROGRAM_PATH, NULL );
               pfnEditShutDown[UtlGetTask()](pBuf,    // program directory
                                             NULL,    // generic structure
                                             TRUE );  // kill  editor
               pfnEditShutDown[UtlGetTask()] = NULL;  // reset the editor proc
               szEditDll[UtlGetTask()][0] = '\0';     // reset editor Dll
               UtlAlloc ((PVOID *)&pBuf, 0L, 0L, NOMSG );
               if ( hmodEditDll[UtlGetTask()] )
               {                      // free editor dll
                  DosFreeModule(hmodEditDll[UtlGetTask()]);
                  hmodEditDll[UtlGetTask()] = 0;
               } /* endif */
            } /* endif */
         } /* endif */
          EQFCLOSE (TRUE);                  //close EQF request handler
            //now destroy the child windows and at least the window itself
          pIda = ACCESSWNDIDA( hwnd, PIDA_HEAD );
          if ( pIda )
          {
             hab = (HAB) UtlQueryULong(QL_HAB);
             // check if still exist...
             if ( pIda->hFrame  && WinIsWindow (hab, pIda->hFrame) )
             {
               WinDestroyWindow( pIda->hFrame );
             } /* endif */
          } /* endif */
          break;
       //----------------------------------------------------------------------
       case WM_EQF_DELETE:
          pszObjName = (PSZ)mp2;                           //get object name
          usMBReturn = SHORTFROMMP1(mp1);

          if( EqfQueryObject( pszObjName, clsDOCUMENT, 0) ||
              QUERYSYMBOL( pszObjName ) != -1)
          {
             UtlError( ERROR_DELETE_DOCUMENT, MB_CANCEL, 0, NULL, EQF_ERROR );
          }
          else
          {
            DocumentDelete ( pszObjName, TRUE, &usMBReturn );
          }/*endif*/
          mResult = MRFROMSHORT(usMBReturn);
          break;
       //---------------------------------------------------------------------
       case WM_EQF_ABOUTTODELETE :
          // send query to instance
          mResult = (MRESULT)EqfSend2AllObjects(clsDOCUMENT, message, mp1, mp2);
          break;
       //----------------------------------------------------------------------
       case WM_EQF_OPEN:                  // create document instance
          mResult = DocCreateInst ( hwnd, mp1, (PSZ)mp2, NULL );
          break;
       //----------------------------------------------------------------------
       case WM_EQF_CREATE:
          //at this point in time there is no way defined how to create a document
          mResult = (MRESULT) TRUE;
          break;                         // return "not created"
       //----------------------------------------------------------------------
       case WM_EQF_FONTCHANGED:
       case WM_EQF_COLCHANGED:
          // inform service windows of instance about font change
          EqfSend2AllObjects(clsDOCUMENT, message, mp1, mp2);
          break;
       //----------------------------------------------------------------------
       default:
          mResult = UTLDEFHANDLERPROC( hwnd, message, mp1, mp2);
          break;
    }/*end switch*/

    return( mResult );
}/*end DocumenthandlerWP*/



//------------------------------------------------------------------------------
// Document Instance Window Proc
//------------------------------------------------------------------------------

MRESULT APIENTRY DOCUMENTWP
( HWND hwnd, WINMSG message, WPARAM mp1, LPARAM mp2)
{
    PDOCUMENT_IDA   pIda;                    // pointer to instance area
    PPROPDOCUMENT   pProp;                   // pointer to document properties
    BOOL            fOk = TRUE;              // success indicator
    ULONG           ulErrorInfo;             // error indicator from PRHA
    MRESULT         mResult = FALSE;         // result value of window proc
    HAB             hab;                     // anchor block handle
    USHORT          usTimeOut;               // time out indicator
    USHORT          usOff;                   // offset of eqfxlate struct
    PEQFXLATE       pEQFXlate;               // pointer to window handle
    PPROPFOLDER     ppropFolder;             // pointer to folder properties
    static  BOOL    fTENVMaximized;          // TENV maximized   /* @KWT0019A */

    switch( message)
    {
       case WM_CREATE:
          //allocate storage for document IDA
          fOk = UtlAlloc( (PVOID *) (PVOID *)&pIda, 0L,
                          (ULONG)sizeof( DOCUMENT_IDA), ERROR_STORAGE );

          if ( fOk )
          {
            fEditorClosed[UtlGetTask()] = FALSE; // editor not closed yet
            fTerminate[UtlGetTask()]    = FALSE; // no termination call in progress
            pStaticDoc = pIda;
            pIda->tidDA =                      // init thread constant
            pIda->tidMT =                                         /* KIT1086A */
            pIda->sTgtLanguage =
            pIda->sSrcLanguage = (SHORT)-1;    // init source and tgt lng
            ANCHORWNDIDA( hwnd, pIda);         // anchor our ida


             //save instance frame to document IDA header
             pIda->IdaHead.hFrame = GETPARENT( hwnd );

             //associate icon
             CreateTWBSInst( hwnd );
          }
          else
          {
             mResult = (MRESULT) TRUE;       // set return value in error case
          } /* endif */
          break;

       case WM_CLOSE:
          EqfRemoveObject( TWBFORCE, hwnd);   // force a RemoveObject
          pIda = ACCESSWNDIDA( hwnd, PDOCUMENT_IDA );
             // indicate document is saved if editor is not available any more
          hab = (HAB) UtlQueryULong(QL_HAB);
          if ( (pIda && ! pIda->pstEQFGen)  ||
              ( pIda && pIda->pstEQFGen &&
                  ! WinIsWindow( hab, pIda->pstEQFGen->hwndEditorTgt) ))
          {
            WinSendMsg( hwnd, EQFM_DOC_IS_SAVED, NULL, NULL);
          } /* endif */
          break;

       case WM_DESTROY:
          /************************************************************/
          /* free the menu (if loaded)                                */
          /************************************************************/
            pIda = ACCESSWNDIDA( hwnd, PDOCUMENT_IDA );
            if (pIda && pIda->hTPROMenu )
            {
			        HMENU   hmenu;

              DestroyMenu( pIda->hTPROMenu );

              hmenu = (HMENU)UtlQueryULong(QL_TWBMENU);
              if (!hmenu)
              {
                  HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
				          hmenu = LoadMenu( hResMod, MAKEINTRESOURCE(ID_TWBM_WINDOW));
                  UtlSetULong( QL_TWBMENU, (ULONG)hmenu );
		          }
              if (hmenu )
              {
				        SendMessage( (HWND)UtlQueryULong( QL_TWBCLIENT ),
                         WM_MDISETMENU,
                         (WPARAM)(HWND) UtlQueryULong( QL_TWBMENU ),
                         (LPARAM)(HWND) UtlQueryULong( QL_TWBWINDOWMENU ) );

                   UtlSetUShort( QS_CURMENUID, ID_TWBM_WINDOW );
                // force redraw/reload of menu:
                DrawMenuBar((HWND)UtlQueryULong( QL_TWBFRAME ));
		      }
            } /* endif */
          FreeTWBSInst( hwnd );

          break;

       case WM_SETFOCUS:
          pIda = ACCESSWNDIDA( hwnd, PDOCUMENT_IDA );
          if ( pIda )
          {
            /**********************************************************/
            /* gaining the focus                                       */
            /**********************************************************/
            {
              /********************************************************/
              /* our client window gets the focus ...                 */
              /********************************************************/
              if ( pIda->hwndFocusWnd && (pIda->hwndFocusWnd != hwnd) )
              {
//                if (GetParent( pIda->hwndFocusWnd) != hwnd) //@@@
                {
                  WinPostMsg( hwnd, WM_EQF_SETFOCUS,
                              0L, MP2FROMHWND( pIda->hwndFocusWnd ) );
                } /* endif */
              }
              else
              {
                if ( pIda->pstEQFGen && pIda->pstEQFGen->hwndEditorTgt )
                     WinPostMsg( hwnd, WM_EQF_SETFOCUS,
                                0L, MP2FROMHWND(pIda->pstEQFGen->hwndEditorTgt));
              } /* endif */
            } /* endif */
          } /* endif */

          break;

     /*****************************************************************/
     /* activate our MDI windows - WM_ACTIVATE for os/2 done in       */
     /* UtlDefInstanceProc - after handling our modeless find&change  */
     /* dialog                                                        */
     /*****************************************************************/
       case WM_EQF_MDIACTIVATE :
          pIda = ACCESSWNDIDA( hwnd, PDOCUMENT_IDA );
          if ( !pIda->hTPROMenu )
          {
            break;
          }
       case WM_MDIACTIVATE :
          /************************************************************/
          /* set our menu if gaining the focus ....                   */
          /************************************************************/
          pIda = ACCESSWNDIDA( hwnd, PDOCUMENT_IDA );
          if ( hwnd == (HWND)mp2 )
          {
            if ( !pIda->hTPROMenu )
            {
              HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
              pIda->hTPROMenu = LoadMenu( hResMod,
                                          MAKEINTRESOURCE( ID_TP_MAIN_WINDOW));
              /********************************************************/
              /* the windows pulldown is the 8th entry in the AAB     */
              /*                                                |     */
              /************************************************ v *****/
              pIda->hTPROWndMenu = GetSubMenu( pIda->hTPROMenu, 7 );
            } /* endif */

            if ( fTENVMaximized )
            {
               SendMessage( (HWND)UtlQueryULong( QL_TWBCLIENT ),
                            WM_MDIMAXIMIZE,
                            (WPARAM) pIda->pstEQFGen->hwndTWBS,
                            0L );
            } /* endif */

            SendMessage( (HWND)UtlQueryULong( QL_TWBCLIENT ),
                         WM_MDISETMENU,
                         (WPARAM)pIda->hTPROMenu,
                         (LPARAM)pIda->hTPROWndMenu );

            UtlSetUShort( QS_CURMENUID, ID_TP_MAIN_WINDOW );

            if (pIda->hwndFocusWnd )
            {
              WinPostMsg( hwnd, WM_EQF_SETFOCUS,
                          0, MP2FROMHWND(pIda->hwndFocusWnd));
            } /* endif */
          }
          else
          {
            /************************************************************/
            /* set the TWB Main menu if losing the focus ...            */
            /************************************************************/
            SendMessage( (HWND)UtlQueryULong(QL_TWBCLIENT),
                         WM_MDIGETACTIVE,
                         0, (LONG)&fTENVMaximized );

            if ( fTENVMaximized )
                        {
              SendMessage( (HWND)UtlQueryULong(QL_TWBCLIENT),
                            WM_MDIRESTORE,
                            (WPARAM) pIda->pstEQFGen->hwndTWBS,
                            0L);
            } /* endif */

            SendMessage( (HWND)UtlQueryULong( QL_TWBCLIENT ),
                         WM_MDISETMENU,
                         (WPARAM)(HWND) UtlQueryULong( QL_TWBMENU ),
                         (LPARAM)(HWND) UtlQueryULong( QL_TWBWINDOWMENU ) );

            UtlSetUShort( QS_CURMENUID, ID_TWBM_WINDOW );
          } /* endif */

          /************************************************************/
          /* draw the menu ...                                        */
          /************************************************************/
          DrawMenuBar( (HWND)UtlQueryULong( QL_TWBFRAME ));

          EqfActivateInstance( hwnd, mp1 );
          /************************************************************/
          /* show/hide our Find&Change dialog                         */
          /************************************************************/
          if ( !mp1 || !IsIconic( hwnd ) )
          {
            EQFBShowFindChangeDlg( mp1 );
          } /* endif */
          break;
       case WM_ACTIVATE:
          EqfActivateInstance( hwnd, (mp1 != WA_INACTIVE) );
          if ( mp1 != WA_INACTIVE )
          {
            /************************************************************/
            /* clear status bar from any pending display                */
            /************************************************************/
            SendMessage( hwnd, WM_EQF_UPDATESTATUSBAR_TEXT, 0, (LONG) "" );
          } /* endif */
          break;

      case WM_SYSCOMMAND:
        switch ( mp1 )
        {
          case SC_MINIMIZE:
            EQFBShowFindChangeDlg( FALSE );
            break;
          case SC_MAXIMIZE:
          case SC_RESTORE:
            EQFBShowFindChangeDlg( TRUE );
            break;
        } /* endswitch */
        mResult = DefMDIChildProc( hwnd, message, mp1, mp2 );
        break;
      case WM_QUERYOPEN:
        EQFBShowFindChangeDlg( TRUE );
        mResult = DefMDIChildProc( hwnd, message, mp1, mp2 );
        break;

       case WM_KILLFOCUS:
                   WinSendMsg( hwnd, WM_NCACTIVATE, FALSE, NULL );
                   break;

       case WM_EQF_SETFOCUS:
          pIda = ACCESSWNDIDA( hwnd, PDOCUMENT_IDA );
          pIda->hwndFocusWnd = HWNDFROMMP2(mp2);
          SetWindowPos( HWNDFROMMP2( mp2 ), HWND_TOP, 0,0,0,0, SWP_NOSIZE | SWP_NOMOVE | SWP_SHOWWINDOW );
          SetForegroundWindow(  HWNDFROMMP2(mp2) );
          SetActiveWindow( HWNDFROMMP2(mp2) );
          SetFocus( HWNDFROMMP2(mp2) );
          break;

       case WM_EQF_SHOWHTML:
          UtlError( WARNING_SHOWTRANS_NOT_SUPPORTED, MB_OK, 0, NULL, EQF_INFO );
          break;

       case WM_EQF_INITMENU:
//          break;

       case WM_INITMENUPOPUP:
          pIda = ACCESSWNDIDA( hwnd, PDOCUMENT_IDA );
          /************************************************************/
          /* check if we are dealing with the AAB or a popup..        */
          /* if the Submenu hanlde is the one passed, we have to be   */
          /* careful and pass on any messages to the editor window... */
          /* all others are handled by the service window..           */
          /************************************************************/
          if ( LOWORD(mp2) > 0 )
          {
            if ( pIda->fTWBSPopup )
            {
              mResult = SendMessage( pIda->hwndFocusWnd, message, mp1, mp2 );
            }
            else
            if ((pIda->hwndFocusWnd == pIda->hwndDictionary) ||
                (pIda->hwndFocusWnd == pIda->hwndSource) ||
                (pIda->hwndFocusWnd == pIda->hwndProposals))
            {
              /********************************************************/
              /* we first of all have to activate the editor window   */
              /********************************************************/
              SendMessage(pIda->pstEQFGen->hwndTWBS, WM_EQF_SETFOCUS,
                          0, MP2FROMHWND( pIda->pstEQFGen->hwndEditorTgt ) );
              mResult = SendMessage( pIda->pstEQFGen->hwndEditorTgt,
                                     message, mp1, mp2 );
            }
            else
            {
                                mResult = SendMessage( pIda->hwndFocusWnd, message, mp1, mp2 );
            } /* endif */
          }
          else
          {
            mResult = SendMessage( pIda->hwndFocusWnd, message, mp1, mp2 );
          } /* endif */
          break;

       case WM_EQF_TERMINATE:
          pIda = ACCESSWNDIDA( hwnd, PDOCUMENT_IDA );
          fTerminate[UtlGetTask()] = TRUE;       // come from EQF_TERMINATE

          /************************************************************/
          /* Stop any running under-cover analysis                    */
          /************************************************************/
          if ( pIda && pIda->fAnalysisIsRunning )
          {
            if ( hWaitDlg )
            {
              FreeProcInstance( (FARPROC) GetWindowLong( hWaitDlg,
                                                         GWL_WNDPROC ));

              WinDestroyWindow( hWaitDlg ); // remove 'please wait...' dialog
              /********************************************************/
              /* Destroying the dialog window will indicate for the   */
              /* EqfTextSegm function that analysis has not been      */
              /* completed successfully                               */
              /********************************************************/
              hWaitDlg = NULLHANDLE;
            } /* endif */

            /**********************************************************/
            /* Setting fKill to TRUE will terminate the analysis      */
            /* process at the next possible step                      */
            /**********************************************************/
            *(pIda->pfKillAnalysis) = TRUE;

            /**********************************************************/
            /* Close open properties (close by termination routine in */
            /* DOC_IS_SAVED handling comes too late)                  */
            /**********************************************************/
            if ( pIda->IdaHead.hProp )
            {
               CloseProperties( pIda->IdaHead.hProp, PROP_FILE, &ulErrorInfo);
               pIda->IdaHead.hProp = NULL;
            } /* endif */
            if ( pIda->hpropFolder )
            {
               CloseProperties( pIda->hpropFolder, PROP_QUIT, &ulErrorInfo);
               pIda->hpropFolder = NULL;
            } /* endif */
            if ( pIda->hpropEdit )
            {
               CloseProperties( pIda->hpropEdit, PROP_QUIT, &ulErrorInfo);
               pIda->hpropEdit = NULL;
            } /* endif */
          } /* endif */

          if ( pIda  && ! fEditorClosed[UtlGetTask()] )
          {
             if ( pIda->IdaHead.hProp )
             {
                pProp = (PPROPDOCUMENT)MakePropPtrFromHnd( pIda->IdaHead.hProp );
                UtlSaveWindowPos( pIda->IdaHead.hFrame, &pProp->Swp);
                                                 //save document properties
                SaveProperties( pIda->IdaHead.hProp, &ulErrorInfo);
             } /* endif */
             //call function to do exit handling for used editor
             if ( pIda->pfnEQF_XStop )
             {
                UtlMakeEQFPath( pIda->szString, NULC, PROGRAM_PATH, NULL );
                pIda->pfnEQF_XStop( pIda->szString,   // program directory
                                    pIda->pstEQFGen,  // generic structure
                                    FALSE );          // do not kill  editor

                // store adress
                pfnEditShutDown[UtlGetTask()] = pIda->pfnEQF_XStop;
                usTimeOut = TIMECLOSE;       // check for strange cases...
                while ( !fEditorClosed[UtlGetTask()] && usTimeOut--  )
                {
                   UtlDispatch();            // dispatch incoming messages
                } /* endwhile */
                if (! fEditorClosed[UtlGetTask()] )
                {
                   WinPostMsg(hwnd, EQFM_DOC_IS_SAVED, NULL, NULL);
                } /* endif */
             } /* endif */
          } /* endif */
          break;

       case WM_EQF_INITIALIZE:
          pIda = ACCESSWNDIDA( hwnd, PDOCUMENT_IDA );
          mResult = DocInitInst( hwnd, pIda->IdaHead.szObjName );
          break;

       case WM_EQF_ABOUTTODELETE :
          pIda = ACCESSWNDIDA( hwnd, PDOCUMENT_IDA );
                                               //the object must not deleted
          mResult = (MRESULT) EqfQueryObject( (PSZ) mp2, clsDOCUMENT, 0);
          if ( !mResult )                      // do not allow delete of folder
          {
             mResult = (MRESULT) (stricmp((PSZ)mp2, pIda->szFolderObjName ) == 0);
          } /* endif */
          break;

       case EQFM_DOC_IS_LOADED:
          pIda = ACCESSWNDIDA( hwnd, PDOCUMENT_IDA );
                                             //switch on arrow
          SETCURSOR( SPTR_ARROW );
                                             //get write access to properties

          /************************************************************/
          /* activate the window                                      */
          /************************************************************/
          if ( pIda && pIda->pstEQFGen && pIda->pstEQFGen->hwndEditorTgt )
          {
            WinSendMsg( hwnd, WM_EQF_SETFOCUS,
                          0L, MP2FROMHWND( pIda->pstEQFGen->hwndEditorTgt ) );
          } /* endif */
          break;

        case EQFM_DOC_IS_SAVED:      // editor is ready with cleanup
          pIda = ACCESSWNDIDA( hwnd, PDOCUMENT_IDA );
          if ( pIda && !pIda->fEQFM_DOC_IS_SAVED )
          {
            LONG lTime;
            UtlTime( &lTime );                   // get time stamp

            pIda->fEQFM_DOC_IS_SAVED = TRUE;     // we processed this already

            fEditorClosed[UtlGetTask()] = TRUE;     // editor is done
            if( !fTerminate[UtlGetTask()] )         // user closed editor via system icon
            {
               EqfRemoveObject( TWBFORCE, hwnd);   // force a RemoveObject first
            } /* endif */

            /************************************************************/
            /* release documents in list ....                           */
            /************************************************************/
            if ( pIda )
            {
              USHORT usI = 0;
              while ( usI < pIda->usDocNamesUsed )
              {
                 REMOVESYMBOL(pIda->apszDocNames[usI]);
                usI++;
              } /* endwhile */
            } /* endif */
            // check that pstEQFGen exists and is already initialised
            if ( pIda && pIda->pstEQFGen && pIda->pstEQFGen->hwndTWBS )
            {
               if ( pIda->hpropEdit &&
                       SetPropAccess( pIda->hpropEdit, PROP_ACCESS_WRITE) )
               {
                  memcpy( &(pIda->ppropEdit->stEQFGen), pIda->pstEQFGen,
                          sizeof(STEQFGEN));
                  SaveProperties( pIda->hpropEdit, &ulErrorInfo );
                  ResetPropAccess( pIda->hpropEdit, PROP_ACCESS_WRITE);
               } /* endif */

               /*********************************************************/
               /* update last change field in folder properties         */
               /*********************************************************/
               if ( pIda->hpropFolder &&
                     SetPropAccess( pIda->hpropFolder, PROP_ACCESS_WRITE ) )
               {
                 //get pointer to folder properties
                 ppropFolder = (PPROPFOLDER)MakePropPtrFromHnd( pIda->hpropFolder );
                 ppropFolder->ulLastChange = lTime;
                 SaveProperties( pIda->hpropFolder, &ulErrorInfo );
                 ResetPropAccess( pIda->hpropFolder, PROP_ACCESS_WRITE);
               } /* endif */


               /*********************************************************/
               /* indicate that document is translated                  */
               /*********************************************************/
               usOff = sizeof(STEQFGEN)+sizeof(STEQFPCMD)+sizeof(STEQFDUMP);
               pEQFXlate = (PEQFXLATE)((PSZ)pIda->pstEQFGen + usOff);
               /**********************************************  ********/
               /* if old status was MINIMIZED, so do it again          */
               /********************************************************/
               if ( pEQFXlate->fBusy && (SWP_FLAG(swpOldStatus) & EQF_SWP_MINIMIZE) )
               {
                 WinSetWindowPos( pEQFXlate->hwndTWB,
                                  swpOldStatus.hwndInsertBehind,
                                  swpOldStatus.x,
                                  swpOldStatus.y,
                                  swpOldStatus.cx,
                                  swpOldStatus.cy,
                                  SWP_FLAG(swpOldStatus) );
               } /* endif */
               pEQFXlate->fBusy = FALSE;
            } /* endif */

                                      //close properties
            if ( pIda->IdaHead.hProp )
            {
               /*******************************************************************/
               /* get date/time of segmented target file                          */
               /*******************************************************************/
               {
                 struct stat buf;
                 int iResult;
                 iResult = stat( pIda->szSegTarget, &buf );
                 if ( !iResult && (buf.st_mtime != (LONG)pIda->ulSegFileDate))
                 {
                   if ( SetPropAccess( pIda->IdaHead.hProp, PROP_ACCESS_WRITE) )
                   {
                      pIda->ppropDoc->ulTouched = lTime;  // get time stamp
                      SaveProperties( pIda->IdaHead.hProp, &ulErrorInfo );
                      ResetPropAccess( pIda->IdaHead.hProp, PROP_ACCESS_WRITE);
                   } /* endif */
                 } /* endif */
               }
               CloseProperties( pIda->IdaHead.hProp, PROP_FILE, &ulErrorInfo);
            } /* endif */
            if ( pIda->hpropFolder )
            {
               CloseProperties( pIda->hpropFolder, PROP_QUIT, &ulErrorInfo);
            } /* endif */
            if ( pIda->hpropEdit )
            {
               CloseProperties( pIda->hpropEdit, PROP_QUIT, &ulErrorInfo);
               pIda->hpropEdit = NULL;
               pIda->ppropEdit = NULL;
            } /* endif */

            EQFCLOSE ( FALSE );          //shutdown request handler to standby mode
            //  activate document list ((hwndActiveFolder if available)
            //  or do nothing, i.e. the organize slider will be activated since
            //  it was the last active window
            //  IN case we are dealing with our editor ignore any settings..
            if ( pIda->hwndActiveFolder && pIda->ppropEdit &&
                 memicmp( pIda->ppropEdit->szMFEStartProc, "STANDARD", 8 ))
            {
               WinSetFocus( HWND_DESKTOP, pIda->hwndActiveFolder );
            } /* endif */
            if ( pIda->fDictLocked )
            {
               ReleaseDictAccess( pIda->szDicts[0] );
            } /* endif */
            if ( pIda->fDocLocked )
            {
               ReleaseDocAccess( pIda->IdaHead.szObjName );
            } /* endif */
                                      //now destroy the child windows
            hab = (HAB) UtlQueryULong(QL_HAB);
               // check if still exist...
            if ( pIda->IdaHead.hFrame  && WinIsWindow (hab, pIda->IdaHead.hFrame))
            {
               EqfActivateInstance( pIda->IdaHead.hFrame, FALSE);
                 /************************************************************/
                 /* if window is maximized we have to restore it to normal   */
                 /* size -- otherwise the next MDI child will be maximized   */
                 /* again....                                                */
                 /************************************************************/
                 if ( IsZoomed( hwnd ) )
                 {
                   ShowWindow( hwnd, SW_SHOWNORMAL );
                 } /* endif */
                 SendMessage( (HWND)UtlQueryULong( QL_TWBCLIENT), WM_MDIDESTROY, MP1FROMHWND(hwnd), 0L );
             } /* endif */
          } /* endif */
          break;

       case EQFM_DOC_STATUS:
          pIda = ACCESSWNDIDA( hwnd, PDOCUMENT_IDA );
          //get write acces to document properties and save time stamp
          if ( SetPropAccess( pIda->IdaHead.hProp, PROP_ACCESS_WRITE) )
          {
            switch ( SHORT1FROMMP1( mp1 ) )
            {
              case  EQF_DOC_COMPLRATE:
                pIda->ppropDoc->usComplete = SHORT1FROMMP2( mp2 );
                break;
              case  EQF_DOC_SEGFROMSCRATCH:
                pIda->ppropDoc->usScratch = SHORT1FROMMP2( mp2 );
                break;
              case  EQF_DOC_SEGMODIFIED:
                pIda->ppropDoc->usModified = SHORT1FROMMP2( mp2 );
                break;
              case  EQF_DOC_SEGCOPIED:
                pIda->ppropDoc->usCopied = SHORT1FROMMP2( mp2 );
                break;
              case  EQF_DOC_XLATED:
                UtlTime( (PLONG)&pIda->ppropDoc->ulXLated );  // get time stamp
                                                    //save it in properties
                break;
              case  EQF_DOC_ULATED:
                pIda->ppropDoc->ulXLated = 0L;
                break;

              case  EQF_DOC_NOTTOUCHED:   // document only loaded not changed
                pIda->ppropDoc->ulTouched = 0L;
                break;
              case  EQF_DOC_UPDATED:   // document updated
                /*******************************************************************/
                /* get date/time of segmented target file and update folder and    */
                /* document                                                        */
                /*******************************************************************/
                {
                  struct stat buf;
                  if ( ! stat( pIda->szSegTarget, &buf ))
                  {
                    BOOL fWriteAccess = FALSE;
                    UtlTime( (PLONG)&pIda->ppropDoc->ulTouched );  // get time stamp
                    pIda->ppropDoc->fSTargetInUnicode = (EQF_BOOL)pIda->fSTargetInUnicode;
                    if ( pIda->hpropFolder )
                    {
                      fWriteAccess = SetPropAccess( pIda->hpropFolder, PROP_ACCESS_WRITE );
                      if ( !fWriteAccess )
                      {
                        /**********************************************/
                        /* go into R/O and force READ/WRITE again     */
                        /* to get the properties updated              */
                        /**********************************************/
                        SetPropAccess( pIda->hpropFolder, PROP_ACCESS_READ );
                        SetPropAccess( pIda->hpropFolder, PROP_ACCESS_WRITE );
                      } /* endif */
                    } /* endif */
                    if ( pIda->hpropFolder )
                    {
                      //get pointer to folder properties
                      ppropFolder = (PPROPFOLDER)MakePropPtrFromHnd( pIda->hpropFolder );
                      ppropFolder->ulLastChange = pIda->ppropDoc->ulTouched;
                      SaveProperties( pIda->hpropFolder, &ulErrorInfo );
                      if ( fWriteAccess )
                      {
                        ResetPropAccess( pIda->hpropFolder, PROP_ACCESS_WRITE);
                      } /* endif */
                      EqfSend2AllHandlers( WM_EQFN_PROPERTIESCHANGED,
                                           MP1FROMSHORT( clsFOLDER ),
                                           MP2FROMP( pIda->szFolderObjName ) );
                    } /* endif */
                  } /* endif */
                }
                break;

              default :
                break;
            } /* endswitch */
            SaveProperties( pIda->IdaHead.hProp, &ulErrorInfo );
            ResetPropAccess( pIda->IdaHead.hProp, PROP_ACCESS_WRITE);
          } /* endif */
          break;

       case EQFM_DOC_IS_XLATED :
          pIda = ACCESSWNDIDA( hwnd, PDOCUMENT_IDA );
          //get write acces to document properties and save time stamp
          if ( SetPropAccess( pIda->IdaHead.hProp, PROP_ACCESS_WRITE) )
          {
            if (!pIda->ppropDoc->ulXLated )  // already marked as transl.
            {
              UtlTime( (PLONG)&pIda->ppropDoc->ulXLated );  // get time stamp
                                                  //save it in properties
            } /* endif */
            SaveProperties( pIda->IdaHead.hProp, &ulErrorInfo );
            ResetPropAccess( pIda->IdaHead.hProp, PROP_ACCESS_WRITE);
          } /* endif */
          break;

       case EQFM_DOC_IS_ULATED :
          pIda = ACCESSWNDIDA( hwnd, PDOCUMENT_IDA );
          //get write acces to document properties and save time stamp
          if ( SetPropAccess( pIda->IdaHead.hProp, PROP_ACCESS_WRITE) )
          {
            pIda->ppropDoc->ulXLated = 0L;      // reset time stamp
                                                //save it in properties
            SaveProperties( pIda->IdaHead.hProp, &ulErrorInfo );
            ResetPropAccess( pIda->IdaHead.hProp, PROP_ACCESS_WRITE);
          } /* endif */
          break;
       case WM_EQFCMD_INIT       :
          EQFR_Init( ACCESSWNDIDA( hwnd, PDOCUMENT_IDA ) );
          break;
       case WM_EQFCMD_TRANSSEG   :
          EQFR_TransSeg ( ACCESSWNDIDA( hwnd, PDOCUMENT_IDA ) );
          break;
       case WM_EQFCMD_CLEAR      :
          EQFR_Clear ( ACCESSWNDIDA( hwnd, PDOCUMENT_IDA ) );
          break;
       case WM_EQFCMD_CLOSE      :
          EQFR_Close( ACCESSWNDIDA( hwnd, PDOCUMENT_IDA ) );
          break;
       case WM_EQFCMD_GETPROP    :
          EQFR_GetProp( ACCESSWNDIDA( hwnd, PDOCUMENT_IDA ) );
          break;
       case WM_EQFCMD_GETDICT    :
          EQFR_GetDict( ACCESSWNDIDA( hwnd, PDOCUMENT_IDA ) );
          break;
       case WM_EQFCMD_GETSOURCE:
          EQFR_GetSource( ACCESSWNDIDA( hwnd, PDOCUMENT_IDA ) );
          break;
       case WM_EQFCMD_DICTLOOK   :
          EQFR_DictLook( ACCESSWNDIDA( hwnd, PDOCUMENT_IDA ) );
          break;
       case WM_EQFCMD_DICTEDIT   :
          EQFR_DictEdit( ACCESSWNDIDA( hwnd, PDOCUMENT_IDA ) );
          break;
       case WM_EQFCMD_SAVESEG    :
          EQFR_SaveSeg( ACCESSWNDIDA( hwnd, PDOCUMENT_IDA ) );
          break;
       case WM_EQFCMD_WORDCNTPERSEG  :
          EQFR_WordCntPerSeg( ACCESSWNDIDA( hwnd, PDOCUMENT_IDA ) );
          break;
       case WM_EQFCMD_DELSEG     :
          EQFR_DelSeg( ACCESSWNDIDA( hwnd, PDOCUMENT_IDA ) );
          break;
       case WM_EQFCMD_GETSEGNUM     :
          EQFR_GetSegNum( ACCESSWNDIDA( hwnd, PDOCUMENT_IDA ) );
          break;
       case WM_EQFCMD_EXTSEG     :
          EQFR_ExtSeg( ACCESSWNDIDA( hwnd, PDOCUMENT_IDA ) );
          break;
       case WM_EQFCMD_DUMPSEG    :
          EQFR_DumpSeg( ACCESSWNDIDA( hwnd, PDOCUMENT_IDA ) );
          break;
       case WM_EQFCMD_PROOF   :
          EQFR_ProofSeg( ACCESSWNDIDA( hwnd, PDOCUMENT_IDA ) );
          break;
       case WM_EQFCMD_PROOFADD:
          EQFR_ProofAdd ( ACCESSWNDIDA( hwnd, PDOCUMENT_IDA ) );
          break;
       case WM_EQFCMD_PROOFAID:
          EQFR_ProofAid ( ACCESSWNDIDA( hwnd, PDOCUMENT_IDA ) );
          break;
       case WM_EQFCMD_ADDABBREV:
          EQFR_AddAbbrev( ACCESSWNDIDA( hwnd, PDOCUMENT_IDA ) );
          break;

       case WM_EQFCMD_XDOCACT:
          EQFR_XDocAct( ACCESSWNDIDA( hwnd, PDOCUMENT_IDA ) );
          break;

       case WM_EQFCMD_XDOCREMOVE:
          EQFR_XDocRemove( ACCESSWNDIDA( hwnd, PDOCUMENT_IDA ) );
          break;

       case WM_EQFCMD_XDOCADD:
          EQFR_XDocAdd( ACCESSWNDIDA( hwnd, PDOCUMENT_IDA ) );
          break;

       case WM_EQFCMD_XDOCNEXT:
          EQFR_XDocNext( ACCESSWNDIDA( hwnd, PDOCUMENT_IDA ) );
          break;

       case WM_EQFCMD_XDOCNUM:
          EQFR_XDocNum( ACCESSWNDIDA( hwnd, PDOCUMENT_IDA ) );
          break;

       case WM_EQFCMD_XDOCINLIST:
          EQFR_XDocInList( ACCESSWNDIDA( hwnd, PDOCUMENT_IDA ) );
          break;
  /*----------------------------------------------------------------------------
  * Invoke Dict.Lookup
  *---------------------------------------------------------------------------*/
       case WM_EQF_PROCESSTASK:
          pIda = ACCESSWNDIDA( hwnd, PDOCUMENT_IDA );
          switch ( SHORT1FROMMP1(mp1) )
          {
              case DICTIONARY_LOOKUP:         // start the dictionary lookup
                if ( ! pIda->hLUPCB )
                {
                    USHORT usLupID;

                    LupBegin( pIda->hUCB,       // user control block
                              pIda->hDCB,       // dictioanry control block
                              pIda->pstEQFGen->hwndTWBS,   // parent handle for dialogs and messages
                              WM_EQF_WD_MAIN_NOTIFY, // msg used for notific
                              &pIda->pstEQFGen->rclDispPos,  // size/position
                              NULL,             // size/pos or NULL
                              &pIda->hLUPCB,    // Lookup serv control block
                              &usLupID );       // Lookup service ID
                } /* endif */

                if ( pIda->hLUPCB )
                {
                  CHAR_W chTerm[256];
                   SendMessage( (HWND)UtlQueryULong(QL_TWBCLIENT),
                                WM_MDIGETACTIVE,
                                0, (LONG)&fTENVMaximized );
                   SendMessage( (HWND)UtlQueryULong( QL_TWBCLIENT ),
                                WM_MDIRESTORE,
                                (WPARAM) pIda->pstEQFGen->hwndTWBS,
                                0L );
                  /****************************************************/
                  /* allow only one open dict lookup dialog in TENV   */
                  /****************************************************/
                  {
                    HWND hwndLook;
                    if ( (hwndLook = LupGetLookupHandle( pIda->hLUPCB ))!= NULL )
                    {
                      SendMessage( hwndLook, WM_CLOSE, 0L, 0L );
                    } /* endif */
                  }
                  UTF16strcpy( chTerm, (PSZ_W) mp2 );
                  LupLookup (pIda->hLUPCB,    // lookup service control block
                             chTerm   );      // term being looked-up
                }
                else
                {
                    DosBeep (4000, 50);
                } /* endif */
                break;
              case DICTIONARY_EDIT:             // start dictionary edit direct.
                if ( ! pIda->hLUPCB )
                {
                    USHORT usLookupID;

                    LupBegin( pIda->hUCB,       // user control block
                              pIda->hDCB,       // dictioanry control block
                              pIda->pstEQFGen->hwndTWBS,   // parent handle for dialogs and messages
                              WM_EQF_WD_MAIN_NOTIFY, // msg used for notific
                              &pIda->pstEQFGen->rclDispPos,  // size/position
                              NULL,             // size/pos or NULL
                              &pIda->hLUPCB,    // Lookup serv control block
                              &usLookupID );    // Lookup dialog handler
                } /* endif */

                if ( pIda->hLUPCB )
                {
                  CHAR_W chTerm[256];
                  SendMessage( (HWND)UtlQueryULong(QL_TWBCLIENT),
                                  WM_MDIGETACTIVE,
                                  0, (LONG)&fTENVMaximized );

                  UTF16strcpy( chTerm, (PSZ_W) mp2 );
                  LupEdit(pIda->hLUPCB,    // lookup service control block
                          chTerm  );           // term being looked-up
                }
                else
                {
                    DosBeep (4000, 50);
                } /* endif */
                break;
              default:
                break;
            } /* endswitch */
            break;

         case WM_EQF_WD_MAIN_NOTIFY:
            pIda = ACCESSWNDIDA( hwnd, PDOCUMENT_IDA );
            /*-----------------------------------------------------------------------
            * Store the position of DISPLAY dialog.
            *----------------------------------------------------------------------*/
            if (SHORT2FROMMP2(mp2) == DISP_ENTRY_DLG &&
                SHORT1FROMMP2(mp2) == DLG_POSITIONED)
            {
              SWP swp;                     // window position
              if (!WinQueryWindowPos ((HWND)mp1, &swp))          /* @KWT0020M */
                memset (&swp, 0, sizeof (swp));
              RECTL_XLEFT(pIda->pstEQFGen->rclDispPos)  = swp.x;
              RECTL_XRIGHT(pIda->pstEQFGen->rclDispPos) = swp.x + swp.cx;
              RECTL_YBOTTOM(pIda->pstEQFGen->rclDispPos)= swp.y;
              RECTL_YTOP(pIda->pstEQFGen->rclDispPos)   = swp.y + swp.cy;
            }
            /*-----------------------------------------------------------------------
            * Upon EDIT the SAB has to be cleared.
            *----------------------------------------------------------------------*/
            else if (SHORT2FROMMP2(mp2) == EDIT_ENTRY_DLG &&
                     SHORT1FROMMP2(mp2) == DLG_TERM_NORM )
            {
              EQFR_ClearDictUpd( pIda );
            }
            else if ( (SHORT2FROMMP2(mp2) == ID_TB_PROP_DICTIONARY_DLG) &&
                       SHORT1FROMMP2(mp2) == 1 )
            {
				EQFR_ClearDictUpd( pIda );
		    }
            else if ( SHORT2FROMMP2(mp2) == LOOKUP_DLG &&
                      SHORT1FROMMP2(mp2) == DLG_HIDDEN )
            {
              if ( fTENVMaximized )
              {
                 PostMessage( (HWND)UtlQueryULong( QL_TWBCLIENT ),
                              WM_MDIMAXIMIZE,
                              (WPARAM) pIda->pstEQFGen->hwndTWBS,
                              0L );
              } /* endif */
            } /* endif */
            break;

       case WM_EQF_FONTCHANGED:
          // check which window font was changed and force a repaint
          // do it for all three service windows
          EQFFontChange( hwnd );
          break;

       case WM_EQF_COLCHANGED:
          // do a repaint of all visible windows
          EQFRepaint( ACCESSWNDIDA( hwnd, PDOCUMENT_IDA ) );
          break;

       case WM_COMMAND:
       case WM_EQF_COMMAND:
         pIda = ACCESSWNDIDA( hwnd, PDOCUMENT_IDA );
         if ( pIda && pIda->hwndFocusWnd && (pIda->hwndFocusWnd != hwnd) )
         {
           mResult = SendMessage( pIda->hwndFocusWnd, message, mp1, mp2 );
         } /* endif */

         if( message == WM_COMMAND)
         {
           mResult = DefMDIChildProc( hwnd, message, mp1, mp2 );
         } /* endif */
         break;

        case WM_EQF_UPDATESLIDER:
          {
            static USHORT usComplete = 0;
            /**************************************************************/
            /* Check if slider arm position requires update               */
            /**************************************************************/
            if ( mp2 )
            {
              HWND hwndText;
              pIda = ACCESSWNDIDA( hwnd, PDOCUMENT_IDA );
              hwndText = GETHANDLEFROMID( pIda->hWaitDlg, ID_TB_ANALWAIT_PROGRESS_TEXT );
              WinSetWindowText( hwndText, (LPCSTR)PVOIDFROMMP2(mp2) );
            } /* endif */
            if ( (SHORTFROMMP1(mp1) != -1) && ( usComplete != USHORTFROMMP1(mp1)))
            {
              HWND hwndSlider;

              pIda = ACCESSWNDIDA( hwnd, PDOCUMENT_IDA );
              hwndSlider = GETHANDLEFROMID( pIda->hWaitDlg, ID_TB_ANALWAIT_SLIDER );
              if ( hwndSlider )
              {
                usComplete = USHORTFROMMP1( mp1 );
                SendMessage( hwndSlider, PB_SETPOS, usComplete, 0 );
                UpdateWindow( hwndSlider );
              } /* endif */
            } /* endif */
          }
          break;
       default:
          /************************************************************/
          /* pass all not-processed messages to the corresponding     */
          /* default procedure                                        */
          /************************************************************/
            mResult = DefMDIChildProc( hwnd, message, mp1, mp2 );
            break;
    }/*end switch*/
    return( mResult );
}/*end DOCUMENTWP*/


//f1////////////////////////////////////////////////////////////////////////////
// function InitMFE                                                           //
////////////////////////////////////////////////////////////////////////////////
BOOL InitMFE( PDOCUMENT_IDA pIda )          //pointer to documnet IDA
//------------------------------------------------------------------------------
// initialize translation editor                                              --
//------------------------------------------------------------------------------
{
   PSZ                 apszTlm[MAX_NUM_OF_READONLY_MDB+2]; // doc memory + readonly TMs
   PSZ                 apszLDct[NUM_OF_FOLDER_DICS+1]; //folder dictionaries
                                                     //add 1 for NULL termination
   PSZ                 pszInsert;                    //pointer for UtlError
   BOOL                fOk = TRUE;                   //BOOL return code

   //call function to fill interface structure for request handler
   fOk = BuildGenericStructure( pIda, pIda->pstEQFGen, apszTlm, apszLDct, TRUE );

   if ( fOk )
   {
     fOk = SetDictAccess( pIda->szDicts[0] );
     pIda->fDictLocked = fOk;                // locking was okay
   } /* endif */

   if ( fOk )
   {
     pIda->fDocLocked = fOk;                // locking was okay
   } /* endif */

   //initialize TWBS session
   if (fOk && EQFINIT ( pIda->pstEQFGen, apszTlm, apszLDct) != EQFRC_OK)
   {
      //switch on arrow
      SETCURSOR( SPTR_ARROW );
      fOk = FALSE;

      if ( pIda->pstEQFGen->usRC != ERROR_MSG_HANDLED  ) // already displayed
      {
         pszInsert = EQFERRINS ();         //get error message text
         UtlError( pIda->pstEQFGen->usRC, MB_CANCEL,
                   1, &pszInsert, EQF_ERROR );
         pIda->pstEQFGen->szMsgBuffer[0] = '\0';
      } /* endif */

   }/*endif*/

   if ( fOk)
   {
      if ( !LoadEditor( pIda ) )
      {
        fOk = FALSE;
        //set editor name and display error message
        if ( pIda && pIda->ppropEdit )
        {
          pszInsert = pIda->ppropEdit->szMFEStartProc ;
          UtlError( ERROR_START_EDITOR, MB_CANCEL, 1, &pszInsert, EQF_ERROR);
        } /* endif */
      } /* endif */
   } /* endif */

   return ( fOk );
}/*end InitMFE*/

//------------------------------------------------------------------------------
// Function name:     LoadEditor
//------------------------------------------------------------------------------
static
BOOL LoadEditor
(
  PDOCUMENT_IDA pIda
)
{
  BOOL fOk = TRUE;
  USHORT usRc = 0;
  ULONG  ulLen;

  // if editor already initialised do not load it again
  // else free old DLL and load new one
  ulLen = strlen(pIda->ppropEdit->szMFEStartProc) + 1;
  if ( strnicmp( pIda->ppropEdit->szMFEStartProc,
                 szEditDll[UtlGetTask()], ulLen ) == 0 )
  {
     usRc = 0;                  // done
  }
  else
  {
     // stop old editor if available
     if ( pfnEditShutDown[UtlGetTask()] )
     {
        UtlMakeEQFPath( pIda->szString, NULC, PROGRAM_PATH, NULL );
        pfnEditShutDown[UtlGetTask()](pIda->szString,  // program directory
                                      pIda->pstEQFGen, // generic structure
                                      TRUE );          // kill  editor
        pfnEditShutDown[UtlGetTask()] = NULL;          // reset the editor proc
     } /* endif */

                            // free editor dll
     if ( hmodEditDll[UtlGetTask()] )
     {
        DosFreeModule(hmodEditDll[UtlGetTask()]);
        hmodEditDll[UtlGetTask()] = NULLHANDLE;
        pIda->pfnEQF_XStop = NULL;
        szEditDll[UtlGetTask()][0] = '\0';       // reset editor Dll
     } /* endif */

     // try to load editor DLL
     // DLL has to be a filename with extension under windows..
     {
       char szEditDllName[MAX_FILESPEC];

       strcpy( szEditDllName, pIda->ppropEdit->szMFEStartProc );
       strcat( szEditDllName, EXT_OF_DLL );
       usRc = DosLoadModule( NULL, 0 ,
                            szEditDllName,
                            &(hmodEditDll[UtlGetTask()]));
    }
  } /* endif */
  if ( usRc == NO_ERROR )
  {
    strcpy(szEditDll[UtlGetTask()], pIda->ppropEdit->szMFEStartProc) ;
    // load addresses of start and stop functions
    usRc = DosGetProcAddr( hmodEditDll[UtlGetTask()],
                           EQFXSTARTEX,
                           (PFN*)(&(pIda->pfnEQF_XStartEx)));
    if ( usRc != NO_ERROR )
    {
      usRc = DosGetProcAddr( hmodEditDll[UtlGetTask()],
                             EQFXSTART,
                             (PFN*)(&(pIda->pfnEQF_XStart)));
    } /* endif */

    if ( usRc == NO_ERROR )
    {
       usRc = DosGetProcAddr( hmodEditDll[UtlGetTask()],
                              EQFXSTOP,
                              (PFN*) (&(pIda->pfnEQF_XStop)));
      if ( usRc == NO_ERROR )
      {
         pfnEditShutDown[UtlGetTask()] = pIda->pfnEQF_XStop;  // set the editor proc
      } /* endif */
    } /* endif */
  } /* endif */
  return fOk;
} /* end of function LoadEditor */




//
//  DocInitInst: Handle the WM_EQF_INITIALIZE, i.e. create document instance
//    Return:   mResult :  TRUE  if failed
//                         FALSE if okay
//
static
MRESULT DocInitInst
(
   HWND hwnd,                          // window handle
   PSZ  pszObjName
)
{
   BOOL  fOk = TRUE;                       // success indicator
   PDOCUMENT_IDA   pIda;                   //pointer to instance area
   ULONG           ulErrorInfo;            //error indicator from PRHA
   PSZ    pData;                           // pointer to data
   PPROPSYSTEM   pSysProp;                 // ptr to EQF system properties
   CHAR   chOrgDrive;                      // original drive

   pIda = ACCESSWNDIDA( hwnd, PDOCUMENT_IDA );
   //get handle of activ folder and save it to IDA
   pIda->hwndActiveFolder = GETPARENT( EqfQueryActiveFolderHwnd() );

   pIda->hwnd = hwnd;                    // anchor handle
                                         //register instance
   EqfRegisterObject( pszObjName, hwnd, clsDOCUMENT);
   pIda->IdaHead.fMustNotClose = TRUE;   // indicate processing mode

   //open document properties
   if( (pIda->IdaHead.hProp = OpenProperties( pszObjName, NULL,
                                    PROP_ACCESS_READ, &ulErrorInfo)) == NULL)
   {
      fOk = FALSE;
      // display error message if not already displayed
      if ( ulErrorInfo != Err_NoStorage )
      {
         UtlError( ERROR_OPEN_PROPERTIES, MB_CANCEL, 1, &pszObjName, EQF_ERROR);
      } /* endif */
   }/*endif*/

   if ( fOk )
   {
     fOk = AddDocToList( pIda, pszObjName, TOP_OF_LIST );
   } /* endif */

   /*******************************************************************/
   /* Get memory, tag table and languages for this document           */
   /*******************************************************************/
   if ( fOk )
   {
     if ( DocQueryInfo2( pszObjName,            // document object name
                         pIda->szDocMemory,     // document translation memory
                         pIda->szDocFormat,     // format of document
                         pIda->szDocSourceLang, // document source language
                         pIda->szDocTargetLang, // document target language
                         pIda->szDocLongName,   // long name of document
                         pIda->szAlias,         // document alias name
                         pIda->szEditor,        // document editor
                         TRUE ) != NO_ERROR )   // handle errors in function
     {
       fOk = FALSE;
     } /* endif */

     if ( fOk )
     {
       // if an alias is used, check for and handle long alias names
       if ( pIda->szAlias[0] != EOS )
       {
         if ( UtlIsLongFileName( pIda->szAlias ) )
         {
           UtlLongToShortName( pIda->szAlias, pIda->szShortAlias );
         }
         else
         {
           strcpy( pIda->szShortAlias, pIda->szAlias );
           pIda->szAlias[0] = EOS;
         } /* endif */
       } /* endif */
     } /* endif */
   } /* endif */

   if ( fOk )
   {
      //save pointer to documnent properties to IDA
      pIda->ppropDoc = (PPROPDOCUMENT)MakePropPtrFromHnd( pIda->IdaHead.hProp );
      pIda->fSTargetInUnicode = pIda->ppropDoc->fSTargetInUnicode;
      //open folder properties
      pSysProp = (PPROPSYSTEM)MakePropPtrFromHnd( EqfQuerySystemPropHnd());
      chOrgDrive = pIda->szFolderObjName[0];          // original drive
      pIda->szFolderObjName[0] = pSysProp->szPrimaryDrive[0];
      if( (pIda->hpropFolder = OpenProperties( pIda->szFolderObjName, NULL,
                                       PROP_ACCESS_WRITE, &ulErrorInfo))== NULL)
      {
         fOk = FALSE;
         // display error message if not already displayed
         if ( ulErrorInfo != Err_NoStorage )
         {
            pData = pIda->szFolderObjName;
            UtlError( ERROR_OPEN_PROPERTIES, MB_CANCEL, 1, &pData, EQF_ERROR);
         } /* endif */
      }
      else
      {
        // load any global memory option file 
        PPROPFOLDER pFolProp = (PPROPFOLDER)MakePropPtrFromHnd( pIda->hpropFolder ); 
        if ( pFolProp->szGlobalMemOptFile[0] != EOS )
        {
          UtlMakeEQFPath( pIda->szBuf, pFolProp->chDrive, SYSTEM_PATH, pFolProp->PropHead.szName );
          strcat( pIda->szBuf, BACKSLASH_STR  );
          strcat( pIda->szBuf, pFolProp->szGlobalMemOptFile );
          GlobMemLoadOptionFile( pIda->szBuf, &(pIda->pvGlobalMemOptFile) );
        } /* endif */           

      }/*endif*/
      pIda->szFolderObjName[0] = chOrgDrive;          // reset orgdrive again
   }/*endif fOk*/

   // show any language or editor related warning or info message
   if ( fOk )
   {
     if ( (UtlQueryUShort( QS_VIETNAMESEKEYBOARDMESSAGESHOWN ) == 0) && (stricmp(pIda->szDocTargetLang, "Vietnamese" ) == 0) )
     {
        UtlError( VIETNAMESE_KEYBOARD_INFO, MB_OK, 0, NULL, EQF_INFO );
        UtlSetUShort( QS_VIETNAMESEKEYBOARDMESSAGESHOWN, 1 );
     } /*endif*/
   }/*endif fOk*/

   if ( fOk )
   {
     fOk = DocFindEdit( pIda );
   }/*endif fOk*/

   if ( fOk )
   {
      //save pointer to editor properties to IDA
      pIda->ppropEdit = (PPROPEDIT)MakePropPtrFromHnd( pIda->hpropEdit );
      //switch on hour glass
      SETCURSOR( SPTR_WAIT );
      //check if all resources for translation are available
      fOk = CheckResources( pIda );
      if ( fOk )
      {
         //initialize the translation editor environment
         fOk = InitMFE( pIda );
      }/*endif fOk*/
   }/*endif fOk*/

   if ( fOk )
   {
     fOk = StartEditor( pIda );
   }
   else
   {
      SETCURSOR( SPTR_ARROW );
      WinPostMsg( hwnd, WM_CLOSE, NULL, NULL ); //close instance
      EQFXlateError( EQFXLATE_TRANSL_ERROR );
   }/*endif*/


    // write version record to history log
    if ( fOk )
    {
      VERSIONHIST Version;
      CHAR        szPath[MAX_EQF_PATH];
      HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);

      // fill version record fields
      memset( &Version, 0, sizeof(Version) );
      Version.fWindows = TRUE;
      UtlMakeEQFPath( szPath, NULC, PROGRAM_PATH, NULL );
      strcat( szPath, "\\EQFSTART.EXE" );
      DocGetFileDateAndSize( szPath, &Version.lEqfstartDateTime,
                             &Version.lEqfstartSize );
      UtlMakeEQFPath( szPath, NULC, PROGRAM_PATH, NULL );
      strcat( szPath, "\\EQFDLL.DLL" );
      DocGetFileDateAndSize( szPath, &Version.lEqfdllDateTime,
                             &Version.lEqfdllSize );

      UtlMakeEQFPath( szPath, NULC, PROGRAM_PATH, NULL );

      strcat( szPath, "\\" );
      UtlQueryString( QST_RESFILE, szPath + strlen(szPath), MAX_FILESPEC );
      DocGetFileDateAndSize( szPath, &Version.lEqfresDateTime,
                             &Version.lEqfresSize );

      // get version string from resource DLL
      LOADSTRING( NULLHANDLE, hResMod, SID_LOGO_REVISION,
                  Version.szVersionString );

      EQFBWriteHistLog( pIda->szFolderObjName, "", VERSION_LOGTASK,
                        sizeof(VERSIONHIST), (PVOID)&Version,
                        FALSE, NULLHANDLE );
    } /* endif */

   return ( (MRESULT) !fOk);
}
//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     StartEditor
//------------------------------------------------------------------------------
// Function call:     fOK = StartEditor( pIda );
//------------------------------------------------------------------------------
// Description:       Invoke the editor as specified
//------------------------------------------------------------------------------
// Input parameter:   _
// Parameters:        _
//------------------------------------------------------------------------------
// Output parameter:  _
//------------------------------------------------------------------------------
// Returncode type:   _
//------------------------------------------------------------------------------
// Returncodes:       _
//------------------------------------------------------------------------------
// Prerequesits:      _
//------------------------------------------------------------------------------
// Side effects:      _
//------------------------------------------------------------------------------
// Samples:           _
//------------------------------------------------------------------------------
// Function flow:     _
//------------------------------------------------------------------------------
static BOOL
StartEditor
(
  PDOCUMENT_IDA pIda
)
{
  BOOL fOK = TRUE;
  UtlMakeEQFPath( pIda->szString, NULC, PROGRAM_PATH, NULL );
  if ( pIda->pfnEQF_XStartEx )
  {
    fOK =  pIda->pfnEQF_XStartEx              //call function to start edit
                    ( pIda->szString,         //pointer to program path
                      pIda->apszDocNames,     //list of selected documents
                      pIda->pstEQFGen );      //pointer to generic structure

  }
  else
  {
    fOK =  pIda->pfnEQF_XStart                //call function to start edit
                    ( pIda->szString,         //pointer to program path
                      pIda->szSegSource,      //full qualified segm.source doc
                      pIda->szSegTarget,      //full qualified segm.target doc
                      pIda->pstEQFGen );      //pointer to generic structure

  } /* endif */
  if ( !fOK )
  {
    fEditorClosed[UtlGetTask()] = TRUE;// editor did not start
                                            //switch on arrow
    SETCURSOR( SPTR_ARROW );
    WinPostMsg( pIda->hwnd, WM_CLOSE, NULL, NULL );         //close instance
    EQFXlateError( EQFXLATE_TRANSL_ERROR );
  } /* endif */
  return fOK;
} /* end of function StartEditor */

//
//  DocCreateInst   create the document instance
//         mp2 contains the object name
//
//    Return value: MRESULT    TRUE - creation failed
//                             FALSE - okay
static
MRESULT DocCreateInst ( HWND hwnd,
                        WPARAM  mp1,
                        PSZ  pszObjName,
                        PVOID pOpenAndPos )
{
   MRESULT         mResult = FALSE;         // return false as default
   PIDA_HEAD       pIda;                    //pointer to instance area
   HWND            hclient;                 //handle to folderlist frame, client
   PDOCUMENT_IDA   pDocIda = NULL;          //pointer to document ida
   CHAR  szSingleEditor[MAX_FILESPEC];  // document editor selected

	if (!bProfileRead)
	{
		EQFBReadProfile();
		bProfileRead = TRUE;
	}


   /*******************************************************************/
   /* check if we are opening in extended mode (several docs allowed) */
   /*******************************************************************/
   if ( SHORTFROMMP1(mp1) == MULT_DOCUMENTS )
   {
     PPOOL pPool = PoolCreate( MAX_EQF_PATH );
     PSZ   * apDocNames = NULL;
     CHAR  szEditor[MAX_FILESPEC];        // document editor
     HWND  hDocObject = NULLHANDLE;       // handle of already loaded object

     pIda = ACCESSWNDIDA( hwnd, PIDA_HEAD );
     /*****************************************************************/
     /* use folder object name and create list of selected documents  */
     /*****************************************************************/
     if ( pPool )
     {
       SHORT sI = 0;
       mResult = CreateListOfSelDocs( hwnd, pszObjName, &apDocNames, pPool );
       /*****************************************************************/
       /* loop through selected documents and add them if appropriate   */
       /* to documents to be processed by editor ...                    */
       /*****************************************************************/
       szSingleEditor[0] = 0;
       while ( apDocNames[sI] && !mResult )
       {
         /*************************************************************/
         /* add checking here...                                      */
         /*************************************************************/
         PSZ pszDocObjName = apDocNames[sI];
         USHORT usRC =  DocQueryInfo2(pszDocObjName,
                                      NULL,NULL,NULL,NULL, NULL,
                                      NULL,szEditor,FALSE);
         if ( !usRC )
         {
           if ( szSingleEditor[0] && strcmp( szEditor, szSingleEditor ) != 0 )
           {
             /***********************************************************/
             /* we only support single editors ...                      */
             /***********************************************************/
             UtlError( ERROR_ONLY_ONE_TRANSLATION, MB_CANCEL,
                       0, NULL, EQF_WARNING );
             mResult = (MRESULT) TRUE;
             break;
           }
           else
           {
             if ( !szSingleEditor[0] )
             {
               strcpy( szSingleEditor, szEditor );
             } /* endif */
           } /* endif */
         }
         else
         {
           mResult = (MRESULT) TRUE;
         } /* endif */
         sI++;
       } /* endwhile */
       /***************************************************************/
       /* check against editor of documents already loaded            */
       /***************************************************************/
       if ( !mResult )
       {
         hDocObject = EqfQueryObject( NULL, clsDOCUMENT, 0);
         if( hDocObject )
         {
           pDocIda = ACCESSWNDIDA( hDocObject, PDOCUMENT_IDA );
           if ( !pDocIda || strcmp(pDocIda->ppropEdit->szMFEStartProc, szSingleEditor) != 0 )
           {
             UtlError( ERROR_ONLY_ONE_TRANSLATION, MB_CANCEL,
                       0, NULL, EQF_WARNING );
             mResult = (MRESULT) TRUE;
           } /* endif */
         } /* endif */
       } /* endif */
       /***************************************************************/
       /* check that selected editor supports more than one document  */
       /***************************************************************/
       if ( !mResult )
       {
         if ( hDocObject )
         {
           if ( pDocIda && pDocIda->pfnEQF_XStartEx )
           {
             /*******************************************************/
             /* okay -- the selected editor supports multiple docs..*/
             /*******************************************************/
           }
           else
           {
             UtlError( ERROR_ONLY_ONE_TRANSLATION, MB_CANCEL,
                       0, NULL, EQF_WARNING );
             mResult = (MRESULT) TRUE;
           } /* endif */
         }
         else if ( sI > 1 )
         {
           /*********************************************************/
           /* more than one documents selected -- we have to check  */
           /* if editor works with multiple documents...            */
           /*********************************************************/
           HMODULE hEditDll = NULLHANDLE;
           char    szEditDllName[MAX_FILESPEC];
           USHORT  usRc;

           strcpy( szEditDllName, szSingleEditor );
           strcat( szEditDllName, EXT_OF_DLL );
           usRc = DosLoadModule( NULL, 0 , szEditDllName, &hEditDll);

           if ( usRc == NO_ERROR )
           {
             // load addresses of start and stop functions
             PFN_EQF_XSTARTEX pfnEQF_XStartEx;
             usRc = DosGetProcAddr( hEditDll, EQFXSTARTEX,
                                    (PFN *)&pfnEQF_XStartEx );
           } /* endif */
           if ( usRc != NO_ERROR )
           {
             UtlError( ERROR_ONLY_ONE_TRANSLATION, MB_CANCEL,
                       0, NULL, EQF_WARNING );
             mResult = (MRESULT) TRUE;
           } /* endif */

           if ( hEditDll )
           {
              DosFreeModule(hEditDll);
           } /* endif */
         }
         else
         {
           /***********************************************************/
           /* okay -- all of our editors support at least a single    */
           /* document                                                */
           /***********************************************************/
         } /* endif */
       } /* endif */
       /***************************************************************/
       /* pass-on info                                                */
       /***************************************************************/
       if ( !mResult )
       {
         BOOL fOK = TRUE;
         if ( hDocObject )
         {
           int j = sI - 1;
           pDocIda = ACCESSWNDIDA( hDocObject, PDOCUMENT_IDA );
           while ( fOK && (j >= 0))
           {
             fOK = AddDocToList( pDocIda, apDocNames[j], TOP_OF_LIST );
             j--;
           } /* endwhile */
           ActivateMDIChild( hDocObject );
         }
         else
         {
           HWND hClient;
           mResult = CreateDocInstWnd( apDocNames[0], pOpenAndPos, &hClient,
                                       szSingleEditor );
           if ( !mResult )
           {
             int j = 0;
             pDocIda = ACCESSWNDIDA( hClient, PDOCUMENT_IDA );
             while ( fOK && apDocNames[j] )
             {
               fOK = AddDocToList( pDocIda, apDocNames[j], BOTTOM_OF_LIST );
               j++;
             } /* endwhile */
             mResult = (MRESULT) !fOK;            // return FALSE if Okay
           } /* endif */
         } /* endif */

         if ( !mResult )
         {
           if ( pDocIda->pfnEQF_XStartEx )
           {
             UtlMakeEQFPath( pDocIda->szString, NULC, PROGRAM_PATH, NULL );
             mResult = (MRESULT) ! pDocIda->pfnEQF_XStartEx         //call function to start edit
                                       ( pDocIda->szString,         //pointer to program path
                                         pDocIda->apszDocNames,     //list of selected documents
                                         pDocIda->pstEQFGen );       //pointer to generic structure
           } /* endif */
         } /* endif */
       } /* endif */

       PoolDestroy( pPool );
       UtlAlloc( (PVOID *) &apDocNames, 0L, 0L, NOMSG );
     }
     else
     {
       mResult = (MRESULT) TRUE;
     } /* endif */
   }
   else
   {
     POPENANDPOS pOpen = NULL;

     // check if we have been called with an pOpenPos instead of an object name
     if ( SHORTFROMMP1(mp1) == OPENPOS_USED )
     {
       pOpen = (POPENANDPOS)pszObjName;
       pszObjName = pOpen->szDocName;
     } /* endif */

     /*****************************************************************/
     /* only one document allowed to be open...                       */
     /*****************************************************************/
     pIda = ACCESSWNDIDA( hwnd, PIDA_HEAD );
     hclient = EqfQueryObject( pszObjName, clsDOCUMENT, 0);
     if( hclient )
     {
        ActivateMDIChild( hclient );
     }
     else
     {
        //do not create a second instance of an open document
        if( EqfQueryObject( NULL, clsDOCUMENT, 0))
        {
          UtlError( ERROR_ONLY_ONE_TRANSLATION, MB_CANCEL,
                    0, NULL, EQF_WARNING );
        }
        else
        {
          HWND hClient;
          DocQueryInfo2(pszObjName,
                        NULL,NULL,NULL,NULL, NULL,
                        NULL,szSingleEditor,FALSE);
          mResult = CreateDocInstWnd( pszObjName, pOpen, &hClient,
                                      szSingleEditor );
        } /* endif */
     } /* endif */
   } /* endif */
   return mResult;
}
//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     CreateDocInstWnd
//------------------------------------------------------------------------------
// Function call:     mResult = CreateDocInstWnd(pObjName,pOpenAndPos,&hClient)
//------------------------------------------------------------------------------
// Description:       Create the document instance windows, position it and
//                    start the further editor loading.
//                    Take care if we try to open with a specific position
//------------------------------------------------------------------------------
// Parameters:        PSZ pszObjName,    -- object name
//                    PVOID pOpenAndPos  -- pointer to open position
//                    PHWND phClient     -- handle of created window
//------------------------------------------------------------------------------
// Returncode type:   MRESULT
//------------------------------------------------------------------------------
// Returncodes:       FALSE    -- everything okay (standard for window proced)
//                    TRUE     -- object not created
//------------------------------------------------------------------------------
// Function flow:     create the window (either standard window under OS/2 or
//                    MDIclient window under Windows)
//                    position window on the screen
//                    post message to do further initialisation
//                    return success
//------------------------------------------------------------------------------
static MRESULT
CreateDocInstWnd
(
  PSZ pszObjName,
  PVOID pOpenAndPos,
  PHWND phClient,
  PSZ   pEditor
)
{
  MRESULT         mResult = FALSE;         // return false as default
  HWND            hframe, hclient;         //handle to folderlist frame, client
  ULONG           flStyle;                 //fram creation flags
  RECTL           Rectl;                   // rectangle structure
  PDOCUMENT_IDA   pDocIda;                 //pointer to document ida
  BOOL            fStartSpellCheck = FALSE;
  HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);


  *phClient = NULLHANDLE;
  flStyle = FCF_TITLEBAR | FCF_SIZEBORDER | FCF_MINBUTTON;
    {
       /*********************************************************/
       /* to avoid flickering during the creation of the MID    */
       /* Child window we have to position it outside of the    */
       /* screen - we always reposition it to the last stored   */
       /* positions via a WinSetWindowPos function after init.  */
       /*********************************************************/
      HWND  hwndMDIClient;
      MDICREATESTRUCT mdicreate;

      mdicreate.hOwner  = (HAB)UtlQueryULong( QL_HAB );
      mdicreate.szClass = DOCUMENT;
      mdicreate.szTitle = pszObjName;
      mdicreate.x       = -100;
      mdicreate.y       = -100;
      mdicreate.cx      = 1;
      mdicreate.cy      = 1;
      mdicreate.style   = WS_CLIPCHILDREN;
      mdicreate.lParam  = 0L;

      hwndMDIClient = (HWND)UtlQueryULong( QL_TWBCLIENT );

      hframe = hclient =
      (HWND)SendMessage( hwndMDIClient,
                         WM_MDICREATE, 0,
                         MP2FROMP((LPMDICREATESTRUCT)&mdicreate) );
    }
  if( hframe )
  {
    pDocIda = ACCESSWNDIDA( hclient, PDOCUMENT_IDA );
    *phClient = hclient;
    /****************************************************/
    /* set the number of the segment where editor should*/
    /* be displayed                                     */
    /****************************************************/
    pDocIda->pOpenAndPos = (POPENANDPOS)pOpenAndPos;

    //save document object name to document IDA header
    strcpy( pDocIda->IdaHead.szObjName, pszObjName );
    pDocIda->hwnd = hclient;            // save client handle
    //set pointer document ida header to document object name
    //this must deleted if pszObjName is removed from IDAHEAD
    pDocIda->IdaHead.pszObjName = pDocIda->IdaHead.szObjName;

    SetDocWindowText( pDocIda, hframe, pDocIda->IdaHead.szObjName );

     WinQueryWindowRect( EqfQueryTwbClient(), &Rectl );

     //set window position and maximize client window
     WinSetWindowPos( hframe, HWND_TOP,
                      (SHORT) RECTL_XLEFT(Rectl),
                      (SHORT) RECTL_YBOTTOM(Rectl),
                      (SHORT) (RECTL_XRIGHT(Rectl) - RECTL_XLEFT(Rectl)),
                      (SHORT) (RECTL_YTOP(Rectl) - RECTL_YBOTTOM(Rectl)),
                      EQF_SWP_ACTIVATE | EQF_SWP_SHOW | EQF_SWP_SIZE |
                      EQF_SWP_MAXIMIZE | EQF_SWP_MOVE );

     WinInvalidateRegion( hclient, NULLHANDLE, FALSE );
    //post message to instance to create instance
    UtlDispatch();                   // wait for object to come up
    WinPostMsg( hclient, WM_EQF_INITIALIZE, NULL ,NULL);

    if ( pDocIda->hTPROMenu )
    {
      /**************************************************************/
      /* free menu resource                                         */
      /**************************************************************/
      DestroyMenu( pDocIda->hTPROMenu );
      pDocIda->hTPROMenu = pDocIda->hTPROWndMenu = NULL;
    } /* endif */
    // set the correct window handle
    if ( pEditor && (stricmp( pEditor, RTFEDIT_EDITOR_STR ) == 0) )
    {
      // get active MDI window and set correct Menubar
      pDocIda->hTPROMenu = LoadMenu( hResMod,
                                  MAKEINTRESOURCE( ID_TP_MAINRFONT_WINDOW));
      /********************************************************/
      /* the windows pulldown is the 8th entry in the AAB     */
      /*                                                |     */
      /************************************************ v *****/
      pDocIda->hTPROWndMenu = GetSubMenu( pDocIda->hTPROMenu, 7 );
      pDocIda->usEditor = RTFEDIT_EDITOR;
    }
    else
    {
      // get active MDI window and set correct Menubar
      pDocIda->hTPROMenu = LoadMenu( hResMod,
                                  MAKEINTRESOURCE( ID_TP_MAIN_WINDOW));
      /********************************************************/
      /* the windows pulldown is the 8th entry in the AAB     */
      /*                                                |     */
      /************************************************ v *****/
      pDocIda->hTPROWndMenu = GetSubMenu( pDocIda->hTPROMenu, 7 );

      pDocIda->usEditor = STANDARD_EDITOR;
    }
    SendMessage( (HWND)UtlQueryULong( QL_TWBCLIENT ),
                 WM_MDISETMENU,
                 (WPARAM)pDocIda->hTPROMenu,
                 (LPARAM)pDocIda->hTPROWndMenu );
  }
  else
  {
     mResult = (MRESULT)TRUE ;         //object not opened
  }/*endif*/
  return mResult;
} /* end of function CreateDocInstWnd */


//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     CreateListOfSelDocs
//------------------------------------------------------------------------------
// Function call:     mResult=CreateListOfSelDocs(hwnd,pFolName,&aDocs,pPool);
//------------------------------------------------------------------------------
// Description:       Create a list of selceted documents
//                    Use standard handler functions for retrieval of results
//------------------------------------------------------------------------------
// Parameters:        HWND hwnd           -- window handle of list
//                    PSZ  pszFolObjName  -- folder object name
//                    PSZ  **pppszDocNames -- ptr to array for docname ptrs.
//                    PPOOL pDocPool       -- string pool
//------------------------------------------------------------------------------
// Returncode type:   MRESULT
//------------------------------------------------------------------------------
// Returncodes:       FALSE                -- selected doc names retrieved
//                    TRUE                 -- error during retrieval ...
//------------------------------------------------------------------------------
// Function flow:     create helper listbox and use WM_EQF_QUERYSELECTEDNAMES
//                    from the handler to fill provided listbox
//                    destroy helper window
//                    return success
//------------------------------------------------------------------------------
static MRESULT
CreateListOfSelDocs
(
  HWND hwnd,
  PSZ  pszFolObjName,
  PSZ  **pppszDocNames,
  PPOOL pDocPool
)
{
  MRESULT mResult = FALSE;
  HWND    hwndDocLB;
  PSZ     * apszDocNames;

  hwndDocLB = WinCreateWindow( hwnd, WC_LISTBOX, "",
                               WS_CHILDWINDOW | LBS_STANDARD,
                               0, 0, 10, 10, hwnd,
                               HWND_TOP, 0, NULL, NULL);
  mResult = (MRESULT) (!hwndDocLB);

  if (!mResult)     // processing ok so far
  {
     OBJNAME szDocObjName;
     OBJNAME szFolObjName;
     CHAR  szDocument[MAX_FILESPEC];
     SHORT   sNumOfDocs, sIndex;

     EqfSend2Handler( FOLDERHANDLER, WM_EQF_QUERYSELECTEDNAMES,
                      MP1FROMHWND( hwndDocLB ),
                      MP2FROMP(pszFolObjName) );

     strcpy( szFolObjName, pszFolObjName );

     // correct folder object name if is a subfolder
     if ( FolIsSubFolderObject( szFolObjName ) )
     {
       UtlSplitFnameFromPath( szFolObjName );
       UtlSplitFnameFromPath( szFolObjName );
     } /* endif */

     sIndex = 0;
     sNumOfDocs =  QUERYITEMCOUNTHWND( hwndDocLB );

     mResult = (MRESULT) ! UtlAlloc( (PVOID *)&apszDocNames, 0L,
                           (LONG)max( (sizeof(PSZ)*(sNumOfDocs+1)), MIN_ALLOC ),
                           ERROR_STORAGE);
     if ( !mResult )
     {
       *pppszDocNames = apszDocNames;
       while ( sIndex < sNumOfDocs )
       {
          QUERYITEMTEXTHWND( hwndDocLB, sIndex, szDocument );
          sprintf( szDocObjName, "%s\\%s", szFolObjName, szDocument );
          apszDocNames[ sIndex ] = PoolAddString( pDocPool, szDocObjName );
          sIndex++;
       } /* endwhile */
     } /* endif */

     WinDestroyWindow( hwndDocLB );
  } /* endif */

  return mResult;
} /* end of function CreateListOfSelDocs */

//------------------------------------------------------------------------------
// Function name:     CreateTWBSInst
//------------------------------------------------------------------------------
// Function call:     CreateTWBSInst( HWND )
//------------------------------------------------------------------------------
// Description:       allocate necessary memory for services
//------------------------------------------------------------------------------
// Parameters:        HWND    window handle
//------------------------------------------------------------------------------
// Returncode type:   USHORT
//------------------------------------------------------------------------------
// Returncodes:       NO_ERROR  or error from accessing shared segment
//------------------------------------------------------------------------------
// Function flow:     query window ptr to ida
//                    allocate storage to dictionary area
//                    if okay then
//                      allocate storage for transl. proposal area
//                    endif
//                    if okay then
//                      allocate storage for thread stack
//                    endif
//                    if okay then
//                      allocate or access shared segment
//                      if okay
//                        anchor pointer to it in ida
//                      endif
//                    endif
//                    return return code
USHORT
CreateTWBSInst
(
   HWND  hwnd
)
{
    PDOCUMENT_IDA   pIda;              // pointer to instance area
    USHORT   usRc = 0;
    BOOL     fOK;

    pIda = ACCESSWNDIDA( hwnd, PDOCUMENT_IDA );

    fOK = UtlAlloc( (PVOID *) (PVOID *)&pIda->pstEQFSrcDct, 0L,
                    (ULONG) (EQF_NSAB *sizeof(STEQFSRCDCT)), ERROR_STORAGE );

    if ( fOK )
    {
       // GQ: changed factor from 2 to 3 for additional data area of memory proposal
       fOK = UtlAlloc( (PVOID *) (PVOID *)&pIda->pucEQFTgt, 0L,
                       (ULONG) (3 * EQF_TGTLEN + 1)*sizeof(CHAR_W), ERROR_STORAGE );
    } /* endif */

    if ( fOK )
    {
       fOK = UtlAlloc( (PVOID *) &pIda->pStackDA, 0L,
                       (LONG) REQ_STACKSIZE, ERROR_STORAGE );
    } /* endif */

    if ( fOK )
    {
         // get pointer to generic structure....
         pIda->pstEQFGen = EQFGETSTRUCT();
         if (pIda->pstEQFGen)
         {
           pIda->pstEQFGen->pDoc = pIda;
         }
         else
         {
           usRc = ERROR_STORAGE;
         } /* endif */
    } /* endif */
    return (usRc);
}

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     FreeTWBSInst
//------------------------------------------------------------------------------
// Function call:     FreeTWBSInst( HWND )
//------------------------------------------------------------------------------
// Description:       free allocate memory for services
//------------------------------------------------------------------------------
// Parameters:        HWND     window handle
//------------------------------------------------------------------------------
// Returncode type:   USHORT
//------------------------------------------------------------------------------
// Returncodes:       NO_ERROR or error code from DosGetShrSeg call
//------------------------------------------------------------------------------
// Function flow:     get pointer to ida via WinQueryWindowPtr
//                    if ptr available then
//                      if dict lookup pending thne
//                        call LupEnd
//                      endif
//                      free dictionary area
//                      free proposal area
//                      free dictionary stack
//                      free generic segment
//                      free ida
//                      reset window ptr and static doc pointer
//                      return return code
//------------------------------------------------------------------------------
USHORT
FreeTWBSInst
(
   HWND  hwnd
)
{
    PDOCUMENT_IDA   pIda;                   //pointer to instance area

    pIda = ACCESSWNDIDA( hwnd, PDOCUMENT_IDA );

    if ( pIda )
    {

       if ( pIda->hLUPCB )
       {
          LupEnd ( pIda->hLUPCB );           // Lookup services control block
       } /* endif */
       UtlAlloc( (PVOID *) (PVOID *)&pIda->pstEQFSrcDct, 0L, 0L, NOMSG );

       UtlAlloc( (PVOID *) (PVOID *)&pIda->pucEQFTgt, 0L, 0L, NOMSG );

       UtlAlloc( (PVOID *) (PVOID *)&pIda->pStackDA, 0L, 0L, NOMSG );

       UtlAlloc( (PVOID *) (PVOID *)&(pIda->pStackMT), 0L, 0L, NOMSG );
    } /* endif */

    /******************************************************************/
    /* check if someone is using EQFXLATE -- if so do not free memory */
    /******************************************************************/
    {
      PSTEQFGEN pstEQFGen;
      pstEQFGen = EQFGETSTRUCT();
      if ( pstEQFGen )
      {
        LONG lLen = sizeof(STEQFGEN)+sizeof(STEQFPCMD)+sizeof(STEQFDUMP);
        PEQFXLATE pEXlate = (PEQFXLATE)((PSZ)pstEQFGen + lLen);
        if ( ! WinIsWindow((HAB)NULL, pEXlate->hwnd) ||
             ! ((PEQFXLATE)((PSZ)pstEQFGen + lLen))->hwnd  )
        {
          EQFFREESTRUCT();
        } /* endif */
      } /* endif */
    }
    /******************************************************************/
    /* free allocated string resource                                 */
    /******************************************************************/
    PoolDestroy( pIda->pDocNamePool );
    UtlAlloc( (PVOID *)&pIda->apszDocNames, 0L, 0L, NOMSG );
    /******************************************************************/
    /* cleanup our service windows (before freeing our ida)...       */
    /******************************************************************/
    CleanUp( pIda->hwndSource );
    CleanUp( pIda->hwndProposals );
    CleanUp( pIda->hwndDictionary );

    if ( pIda->pvGlobalMemOptFile ) GlobMemFreeOptionFile( &(pIda->pvGlobalMemOptFile) );

    UtlAlloc( (PVOID *)&pIda, 0L, 0L, NOMSG );
    pStaticDoc = pIda;
    ANCHORWNDIDA( hwnd, pIda);         // anchor our ida

    return ( 0 );
}


//
//   Determine the editor to be used
//     first try to determine it according to the format table,
//     if not available use the default editor
//
static
BOOL DocFindEdit ( PDOCUMENT_IDA pIda )
{
   PPROPFOLDER  ppropFolder;                //pointer to folder properties
   BOOL         fOk = TRUE;                 //error flag
   PSZ          pEditor ;                   // name of editor property
   ULONG        ulErrorInfo;                //error indicator from PRHA
   PSZ          pErrData;                    // error data


      //get name of Edit DLL via pointer to folder properties
   ppropFolder = (PPROPFOLDER)MakePropPtrFromHnd( pIda->hpropFolder );
   pEditor  = pIda->szEditor;

   //get object name of editor properties
   UtlMakeEQFPath( pIda->szEditObjName, NULC, SYSTEM_PATH, NULL );
   if ( *pEditor  )                          //  editor selected
   {
      sprintf( pIda->szEditObjName, "%s\\%s%s",
              pIda->szEditObjName, pEditor , EXT_OF_EDITOR );
   }
   else
   {
      sprintf( pIda->szEditObjName, "%s\\%s",
              pIda->szEditObjName, EDITOR_PROPERTIES_NAME );
   } /* endif */
   //open editor properties for selected editor
   if( (pIda->hpropEdit = OpenProperties
                            ( pIda->szEditObjName, NULL,
                              PROP_ACCESS_READ, &ulErrorInfo))== NULL)
   {
      fOk = FALSE;
      // display error message if not already displayed
      if ( ulErrorInfo != Err_NoStorage )
      {
         pErrData = pIda->szEditObjName;
         UtlError( ERROR_PROPERTY_ACCESS, MB_CANCEL,
                   1, &pErrData, EQF_ERROR );
      } /* endif */
   }/*endif */

   return ( fOk );
}

//////////////////////////////////////////////////////////////////////////
// Release document accessed
/////////////////////////////////////////////////////////////////////////
//
//    release document access
//
/////////////////////////////////////////////////////////////////////////


static
VOID ReleaseDocAccess
(
    PSZ   pszDocName                           // pointer to document
)
{

   // remove the symbol from the object manager - don't care about errors
   REMOVESYMBOL( pszDocName );

   // remove symbol for locked folder
   {
     OBJNAME szFolObjName;
     PSZ     pszDocNamePos;
     CHAR    szSysDrive[MAX_DRIVE];

     strcpy( szFolObjName, pszDocName );
     pszDocNamePos = strrchr( szFolObjName, BACKSLASH );
     if ( pszDocNamePos )
     {
       *pszDocNamePos = EOS;
       UtlQueryString( QST_PRIMARYDRIVE, szSysDrive, sizeof(szSysDrive) );
       szFolObjName[0] = szSysDrive[0];
       REMOVESYMBOL( szFolObjName );
     } /* endif */
   }

}

////////////////////////////////////////////////////////////////////////
// check which window font was changed and force a repaint
// do it for all three service windows
//  SERVPROP_DOC,
//  SERVDICT_DOC,
//  SERVSOURCE_DOC,
////////////////////////////////////////////////////////////////////////
static
VOID EQFFontChange
(
    HWND hwnd
)
{
    PDOCUMENT_IDA   pIda;               // pointer to instance area
    PTWBSDEVICE   ptbDevTemp;           // ptr to device context
    PTBDOCUMENT   pDoc;                 // ptr to document
    SHORT         cx, cy;               // device cell size
    PVIOFONTCELLSIZE pVioFont;          // pointer to font
    pVioFont = get_vioFontSize();
    pIda = ACCESSWNDIDA( hwnd, PDOCUMENT_IDA );

    //  dictionary window
    ptbDevTemp = &(pIda->tbDevDictionary);
    pDoc =  &(ptbDevTemp->tbDoc);
    GETDEVICECELLSIZE( cy, cx, pDoc->hwndClient );

    if (( cy != (pVioFont+SERVDICT_DOC)->cy )
          || (cx != (pVioFont+SERVDICT_DOC)->cx) )
    {
       EQFBSetNewCellSize( pDoc, (pVioFont+SERVDICT_DOC)->cx,
                                 (pVioFont+SERVDICT_DOC)->cy );
       pDoc->Redraw |= REDRAW_ALL;
    } /* endif */

    if ( pDoc->pvGlyphSet ) UtlAlloc( &pDoc->pvGlyphSet, 0, 0, NOMSG );

    if ( pDoc->hwndRichEdit )
    {
      PostMessage( pDoc->hwndRichEdit, WM_EQF_FONTCHANGED, 0, 0l );
    } /* endif */

    // proposal window
    ptbDevTemp = &(pIda->tbDevProposal);
    pDoc =  &(ptbDevTemp->tbDoc);
    GETDEVICECELLSIZE( cy, cx, pDoc->hwndClient );

    if (( cy != (pVioFont+SERVPROP_DOC)->cy )
          || (cx != (pVioFont+SERVPROP_DOC)->cx) )
    {
       EQFBSetNewCellSize( pDoc, (pVioFont+SERVPROP_DOC)->cx,
                            (pVioFont+SERVPROP_DOC)->cy );
       pDoc->Redraw |= REDRAW_ALL;
    } /* endif */

    if ( pDoc->pvGlyphSet ) UtlAlloc( &pDoc->pvGlyphSet, 0, 0, NOMSG );

    if ( pDoc->hwndRichEdit )
    {
      PostMessage( pDoc->hwndRichEdit, WM_EQF_FONTCHANGED, 0, 0l );
    } /* endif */

    // source window
    ptbDevTemp = &(pIda->tbDevSource);
    pDoc =  &(ptbDevTemp->tbDoc);
    GETDEVICECELLSIZE( cy, cx, pDoc->hwndClient );

    if (  (cy != (pVioFont+SERVSOURCE_DOC)->cy) ||
          (cx != (pVioFont+SERVSOURCE_DOC)->cx) )
    {
       EQFBSetNewCellSize( pDoc, (pVioFont+SERVSOURCE_DOC)->cx,
                            (pVioFont+SERVSOURCE_DOC)->cy );
       pDoc->Redraw |= REDRAW_ALL;
    } /* endif */

    if ( pDoc->pvGlyphSet ) UtlAlloc( &pDoc->pvGlyphSet, 0, 0, NOMSG );

    if ( pDoc->hwndRichEdit )
    {
      PostMessage( pDoc->hwndRichEdit, WM_EQF_FONTCHANGED, 0, 0l );
    } /* endif */
    return;
}

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     EQFTextSegm
//------------------------------------------------------------------------------
// Function call:     EQFTextSegm( PDOCUMENT_IDA );
//------------------------------------------------------------------------------
// Description:       do on the spot invocation of text segmentation
//------------------------------------------------------------------------------
// Parameters:        _
//------------------------------------------------------------------------------
// Returncode type:   BOOL
//------------------------------------------------------------------------------
// Returncodes:       TRUE     text file segmented
//                    FALSE    error happened during segmentation
//------------------------------------------------------------------------------
// Side effects:      Allocated memory for TAINPUT structure will be freed in
//                    Text Analysis.
//------------------------------------------------------------------------------
// Function flow:     allocate TAINPUT structure
//                    if okay then
//                      fill in initial values
//                      call TextSegmentation; set fOK
//                    endif
//                    return fOK
//------------------------------------------------------------------------------

static
BOOL EQFTextSegm
(
   PDOCUMENT_IDA  pIda
)
{
  BOOL       fOK = TRUE;

  HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
  if ( hResMod )
  {
    INT_PTR iRc;
    DIALOGBOX( pIda->hwnd, ANALYSISWAITDLG, hResMod, ID_TB_ANALWAIT_DLG,
               pIda, iRc);

    if ( iRc == DID_ERROR )
    {
      UtlError( ERROR_DIALOG_LOAD_FAILED, MB_CANCEL, 0 , NULL, EQF_ERROR );
      fOK = FALSE;
    } /* endif */
  } /* endif */


   //check segmented target file
  if ( fOK )
  {
    //check if segmented target file exists
    fOK = UtlFileExist( pIda->szSegTarget );
    if( !fOK )
    {
       //display that segmented target file does not exits
       //UtlError( ERROR_NOT_ANALYZED, MB_CANCEL, NULL, NULL, EQF_ERROR );
    }
    else
    {
       /***************************************************************/
       /* set analysed message                                        */
       /***************************************************************/
       UtlTime( (PLONG)&pIda->ppropDoc->ulSeg );  // get time stamp
       pIda->ppropDoc->ulTouched = 0L;
       pIda->ppropDoc->ulXLated = 0L;
    }/* endif fOK*/
  }/* endif fOK*/
  return fOK;
} /* end of function EQFTextSegm */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     EQFXlateError
//------------------------------------------------------------------------------
// Function call:     EQFXlateError ( USHORT )
//------------------------------------------------------------------------------
// Description:       set the error code in the generic structure
//------------------------------------------------------------------------------
// Parameters:        _
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Prerequesits:      EQFXLATE is allocated together with the shared data segm.
//------------------------------------------------------------------------------
// Function flow:     _
//------------------------------------------------------------------------------

static
VOID EQFXlateError
(
  USHORT  usError
)
{
   PEQFXLATE  pEQFXlate;
   PSTEQFGEN  pstEQFGen;
   LONG       lLen;

   pstEQFGen = EQFGETSTRUCT();
   if (pstEQFGen)
   {
     lLen = sizeof(STEQFGEN)+sizeof(STEQFPCMD)+sizeof(STEQFDUMP);
     pEQFXlate = (PEQFXLATE)((PSZ)pstEQFGen + lLen);
     pEQFXlate->fBusy = FALSE;
     pEQFXlate->usRc = usError ;
     /**********************************************  ********/
     /* if old status was MINIMIZED, so do it again          */
     /********************************************************/
     if ( SWP_FLAG(swpOldStatus) & EQF_SWP_MINIMIZE )
     {
       WinSetWindowPos( pEQFXlate->hwndTWB,
                        swpOldStatus.hwndInsertBehind,
                        swpOldStatus.x,
                        swpOldStatus.y,
                        swpOldStatus.cx,
                        swpOldStatus.cy,
                        SWP_FLAG(swpOldStatus) );
     } /* endif */
   }  /* endif */
}


//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     ANALYSISWAITDLG
//------------------------------------------------------------------------------
// Function call:     _
//------------------------------------------------------------------------------
// Description:       Displays the Wait Dlg during interactive segmentation
//------------------------------------------------------------------------------
// Parameters:        HWND hDlg,          Dialog window handle
//                    USHORT msg,         Message ID
//                    WPARAM mp1,         Message parameter 1
//                    LPARAM mp2          Message parameter 2
//------------------------------------------------------------------------------
// Returncode type:   MRESULT
//------------------------------------------------------------------------------
// Returncodes:       _
//------------------------------------------------------------------------------
// Function flow:     _
//------------------------------------------------------------------------------

INT_PTR CALLBACK ANALYSISWAITDLG
(
  HWND hDlg,                           // Dialog window handle
  WINMSG msg,                          // Message ID
  WPARAM mp1,                          // Message parameter 1
  LPARAM mp2                           // Message parameter 2
)
{
  MRESULT mResult = FALSE;
  switch ( msg )
  {
    case WM_INITDLG:
      {
        PDOCUMENT_IDA   pIda = (PDOCUMENT_IDA) mp2;
        SWP   swp, swpDlg;
        HWND  hwndSlider = GETHANDLEFROMID( hDlg, ID_TB_ANALWAIT_SLIDER );
        if ( hwndSlider )
        {
           SendMessage( hwndSlider, PB_SETRANGE, 100, 0 );
           SendMessage( hwndSlider, PB_SETPOS, 0, 0 );
           SendMessage( hwndSlider, PB_SETCOLOR, 0, RGB(0,0,0) );
        } /* endif */
        WinQueryWindowPos( hDlg, &swpDlg );
        WinQueryWindowPos( pIda->hwnd, &swp );
        swpDlg.x = swp.x + (swp.cx - swpDlg.cx)/2;
        swpDlg.y = swp.y + (swp.cy - swpDlg.cy)/2;

        // position the dialog window
        WinSetWindowPos( hDlg, HWND_TOP, swpDlg.x, swpDlg.y, 0, 0, EQF_SWP_MOVE );

        pIda->hWaitDlg = hDlg;
        WinPostMsg( hDlg, WM_EQF_INITIALIZE, NULL, mp2 );
      }
      break;
  case WM_EQF_INITIALIZE:
    {
      BOOL  fOK;
      PDOCUMENT_IDA   pIda = (PDOCUMENT_IDA) mp2;
      ANCHORDLGIDA( hDlg, pIda );
      pIda->fAnalysisIsRunning = TRUE;
      REMOVESYMBOL( pIda->IdaHead.pszObjName );            // temporarily remove in-use
      fOK = TAAnalyzeFile( pIda->IdaHead.pszObjName , pIda->hwnd, 0, NULL,
                           &pIda->pfKillAnalysis );
      SETSYMBOL( pIda->IdaHead.pszObjName );               // set in use again...
      pIda->fAnalysisIsRunning = FALSE;
      WinPostMsg( hDlg, WM_EQF_CLOSE, NULL, MP1FROMSHORT(fOK) );
    }
    break;

    case WM_EQF_CLOSE:
      DISMISSDLG( hDlg, SHORT1FROMMP2(mp2) );
      break;
    case WM_COMMAND:
      if ( WMCOMMANDID( mp1, mp2 ) == DID_CANCEL )
      {
        if ( UtlError( ERROR_CANCELTA,
                       MB_YESNO,
                       0,
                       NULL,
                       EQF_QUERY ) == MBID_YES )
        {
          /**********************************************************/
          /* Setting fKill to TRUE will terminate the analysis      */
          /* process at the next possible step                      */
          /**********************************************************/
          PDOCUMENT_IDA pIda = ACCESSDLGIDA(hDlg, PDOCUMENT_IDA);
          *(pIda->pfKillAnalysis) = TRUE;
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

} /* end of function ANALYSISWAITDLG */

USHORT DocBatchDocumentOpen
(
  HWND             hwndParent,         // handle of document handler window
  PDDEDOCOPEN      pDocOpen            // folder import data structure
)
{
   BOOL            fOK = TRUE;         // internal O.K. flag
   HWND            hwndLB = NULLHANDLE;// listbox for folder and document names
   PSZ             pszParm;            // error parameter pointer
   OBJNAME         szObjName;          // buffer for object names
   CHAR            szDocShortName[MAX_FILESPEC];// buffer for document short name
   CHAR             szShortName[MAX_FILESPEC]; // buffer for folder short name

   /********************************************************************/
   /* Create invisible listbox for names of folders and documents      */
   /********************************************************************/
   hwndLB = WinCreateWindow( hwndParent, WC_LISTBOX, "", 0L, 0, 0, 0, 0,
                             hwndParent, HWND_TOP, 1, NULL, NULL );


   /*******************************************************************/
   /* Check if folder exists                                          */
   /*******************************************************************/
   if ( fOK )
   {
     BOOL fIsNew;

     ObjLongToShortName( pDocOpen->szFolder, szShortName, FOLDER_OBJECT, &fIsNew );
     if ( fIsNew )
     {
       fOK = FALSE;
       pszParm = pDocOpen->szFolder;
       OEMTOANSI(pDocOpen->szFolder);
       pDocOpen->DDEReturn.usRc = ERROR_XLATE_FOLDER_NOT_EXIST;
       UtlErrorHwnd( pDocOpen->DDEReturn.usRc, MB_CANCEL, 1,
                     &pszParm, EQF_ERROR, pDocOpen->hwndErrMsg );
       ANSITOOEM(pDocOpen->szFolder);
     } /* endif */
   } /* endif */

   /*******************************************************************/
   /* Check if folder is locked                                       */
   /*******************************************************************/
   if ( fOK )
   {
      SHORT sRC;                       // return code of SYMBOL functions

      UtlMakeEQFPath( szObjName, NULC, SYSTEM_PATH, NULL );
      strcat( szObjName, BACKSLASH_STR );
      strcat( szObjName, szShortName );
      strcat( szObjName, EXT_FOLDER_MAIN );
      sRC = QUERYSYMBOL( szObjName );
      if ( sRC != -1 )
      {
         // folder is in use ...
         fOK = FALSE;
         pszParm = pDocOpen->szFolder;
         pDocOpen->DDEReturn.usRc = ERROR_FOLDER_LOCKED;
         UtlErrorHwnd( ERROR_FOLDER_LOCKED, MB_CANCEL, 1,
                       &pszParm, EQF_INFO, pDocOpen->hwndErrMsg );
      } /* endif */
   } /* endif */

   /*******************************************************************/
   /* Check if document exists                                        */
   /*******************************************************************/
   if ( fOK )
   {
     /*****************************************************************/
     /* Build folder object name (access to folder properties is      */
     /* required to correct folder drive letter)                      */
     /*****************************************************************/
     {
       PPROPFOLDER  ppropFolder;        // pointer to folder properties
       HPROP        hpropFolder;        // folder properties handle
       ULONG        ulErrorInfo;        // error indicator from property handler

       UtlMakeEQFPath( szObjName, NULC, SYSTEM_PATH, NULL );
       strcat( szObjName, BACKSLASH_STR );
       strcat( szObjName, szShortName );
       strcat( szObjName, EXT_FOLDER_MAIN );
       hpropFolder = OpenProperties( szObjName, NULL,
                                     PROP_ACCESS_READ, &ulErrorInfo);
       if( hpropFolder )
       {
         ppropFolder = (PPROPFOLDER)MakePropPtrFromHnd( hpropFolder );
         if ( (ppropFolder->chDrive != EOS) && (ppropFolder->chDrive != ' ') )
         {
           szObjName[0] = ppropFolder->chDrive;
         } /* endif */
         CloseProperties( hpropFolder, PROP_QUIT, &ulErrorInfo );
       } /* endif */
     }
     {
       BOOL fIsNew = FALSE;

       FolLongToShortDocName( szObjName, pDocOpen->szName,
                                  szDocShortName, &fIsNew );
       if ( fIsNew )
       {
         fOK = FALSE;
         pszParm = pDocOpen->szName;
         pDocOpen->DDEReturn.usRc = ERROR_OPENING_CONTEXT_FILE;
         UtlErrorHwnd( pDocOpen->DDEReturn.usRc, MB_CANCEL, 1,
                       &pszParm, EQF_ERROR, pDocOpen->hwndErrMsg );
       } /* endif */
     }
   } /* endif */

   /*******************************************************************/
   /* Check if document is locked                                     */
   /*******************************************************************/
   if ( fOK )
   {
      SHORT sRC;                       // return code of SYMBOL functions

      /****************************************************************/
      /* szObjName contains the object name of the folder where the   */
      /* document is located                                          */
      /****************************************************************/
      strcat( szObjName, BACKSLASH_STR );
      strcat( szObjName, szDocShortName );
      sRC = QUERYSYMBOL( szObjName );
      if ( sRC != -1 )
      {
         // document is in use ...
         fOK = FALSE;
         pszParm = pDocOpen->szName;
         pDocOpen->DDEReturn.usRc = ERROR_DOC_LOCKED;
         UtlErrorHwnd( ERROR_DOC_LOCKED, MB_CANCEL, 1,
                       &pszParm, EQF_INFO, pDocOpen->hwndErrMsg );
      } /* endif */
   } /* endif */

   /*******************************************************************/
   /* Open document by posting a WM_EQF_CREATE                        */
   /*******************************************************************/
   if ( fOK )
   {
     POPENANDPOS pOpen = NULL;

    if ( UtlAlloc( (PVOID *)&pOpen, 0L, (LONG)sizeof(OPENANDPOS), ERROR_STORAGE ) )
    {
      strcpy( pOpen->szDocName, szObjName);
      pOpen->ulSeg = pDocOpen->lSegNum;
      pOpen->usOffs= 0;
      pOpen->usLen = 0;

     EqfSend2Handler( DOCUMENTHANDLER, WM_EQF_OPEN, OPENPOS_USED, MP2FROMP( pOpen ) );
     } /* endif */
   } /* endif */

   /*******************************************************************/
   /* Cleanup                                                         */
   /*******************************************************************/
   if ( hwndLB )  WinDestroyWindow( hwndLB );

   /****************************************************************/
   /* report end of task to DDE handler                            */
   /****************************************************************/
   WinPostMsg( pDocOpen->hwndOwner, WM_EQF_DDE_ANSWER,
                  NULL, &pDocOpen->DDEReturn );

   return( pDocOpen->DDEReturn.usRc );

} /* end of function DocBatchDocumentOpen */

USHORT DocBatchDocumentDelete
(
  HWND             hwndParent,         // handle of document handler window
  PDDEDOCDEL       pDocDel             // document delete data structure
)
{
   BOOL            fOK = TRUE;         // internal O.K. flag
   HWND            hwndLB = NULLHANDLE;// listbox for folder and document names
   PSZ             pszParm;            // error parameter pointer
   OBJNAME         szObjName;          // buffer for object names
   CHAR             szShortName[MAX_FILESPEC]; // buffer for folder short name
   /********************************************************************/
   /* Create invisible listbox for names of folders and documents      */
   /********************************************************************/
   hwndLB = WinCreateWindow( hwndParent, WC_LISTBOX, "", 0L, 0, 0, 0, 0,
                             hwndParent, HWND_TOP, 1, NULL, NULL );


   /*******************************************************************/
   /* Check if folder exists                                          */
   /*******************************************************************/
   if ( fOK )
   {
     BOOL fIsNew;

     ObjLongToShortName( pDocDel->szFldName, szShortName, FOLDER_OBJECT, &fIsNew );
     if ( fIsNew )
     {
       fOK = FALSE;
       pszParm = pDocDel->szFldName;
       pDocDel->DDEReturn.usRc = ERROR_XLATE_FOLDER_NOT_EXIST;
       OEMTOANSI(pDocDel->szFldName);
       UtlErrorHwnd( pDocDel->DDEReturn.usRc, MB_CANCEL, 1,
                     &pszParm, EQF_ERROR, pDocDel->hwndErrMsg );
       ANSITOOEM(pDocDel->szFldName);
     } /* endif */
   } /* endif */

   /*******************************************************************/
   /* Check if document exists                                        */
   /*******************************************************************/
   if ( fOK )
   {
     PPROPFOLDER  ppropFolder;        // pointer to folder properties
     HPROP        hpropFolder;        // folder properties handle
     ULONG        ulErrorInfo;        // error indicator from property handler

     UtlMakeEQFPath( szObjName, NULC, SYSTEM_PATH, NULL );
     strcat( szObjName, BACKSLASH_STR );
     strcat( szObjName, szShortName );
     strcat( szObjName, EXT_FOLDER_MAIN );
     hpropFolder = OpenProperties( szObjName, NULL,
                                   PROP_ACCESS_READ, &ulErrorInfo);
     if( hpropFolder )
     {
       ppropFolder = (PPROPFOLDER)MakePropPtrFromHnd( hpropFolder );
       if ( (ppropFolder->chDrive != EOS) && (ppropFolder->chDrive != ' ') )
       {
         szObjName[0] = ppropFolder->chDrive;
       } /* endif */
       CloseProperties( hpropFolder, PROP_QUIT, &ulErrorInfo );
     } /* endif */
   } /* endif */

   // check if document exist by calling the long-to-short name function
   if ( fOK )
   {
     BOOL fIsNew = FALSE;
     PSZ pszCurFile = *(pDocDel->ppFileArray + pDocDel->usActFile);
     FolLongToShortDocName( szObjName, pszCurFile, pDocDel->szName, &fIsNew );
     if ( fIsNew )
     {
       // document does not exist
       pszParm = *(pDocDel->ppFileArray + pDocDel->usActFile);
       pDocDel->DDEReturn.usRc = ERROR_OPENING_CONTEXT_FILE;
       UtlErrorHwnd( pDocDel->DDEReturn.usRc, MB_CANCEL, 1,
                     &pszParm, EQF_INFO, pDocDel->hwndErrMsg );
     } /* endif */
   } /* endif */

   /*******************************************************************/
   /* Check if document is locked                                     */
   /*******************************************************************/
   if ( fOK )
   {
      SHORT sRC;                       // return code of SYMBOL functions

      /****************************************************************/
      /* szObjName contains the object name of the folder where the   */
      /* document is located                                          */
      /****************************************************************/
      strcat( szObjName, BACKSLASH_STR );
      strcat( szObjName, pDocDel->szName );
      sRC = QUERYSYMBOL( szObjName );
      if ( sRC != -1 )
      {
         // document is in use ...
         fOK = FALSE;
         pszParm = *(pDocDel->ppFileArray + pDocDel->usActFile);
         pDocDel->DDEReturn.usRc = ERROR_DOC_LOCKED;
         UtlErrorHwnd( ERROR_DOC_LOCKED, MB_CANCEL, 1,
                       &pszParm, EQF_INFO, pDocDel->hwndErrMsg );
      } /* endif */
   } /* endif */

   /*******************************************************************/
   /* Delete the document                                             */
   /*******************************************************************/
   if ( fOK )
   {
     USHORT usDummy = 0;

     if ( !DocumentDelete( szObjName, FALSE, &usDummy ) )
     {
       fOK = FALSE;
       pszParm = *(pDocDel->ppFileArray + pDocDel->usActFile);
       pDocDel->DDEReturn.usRc = NOT_DELETED;
       UtlErrorHwnd( pDocDel->DDEReturn.usRc, MB_CANCEL, 1,
                     &pszParm, EQF_INFO, pDocDel->hwndErrMsg );
     }
     else
     {
       // broadcast document deleted message
       EqfSend2AllHandlers( WM_EQFN_DELETED,
                            MP1FROMSHORT( clsDOCUMENT ),
                            MP2FROMP( szObjName ) );
     } /* endif */
   } /* endif */

   if ( hwndLB )  WinDestroyWindow( hwndLB );
   return( pDocDel->DDEReturn.usRc );
} /* end of function DocBatchDocumentDelete */

#ifdef FUNCCALLIF
USHORT DocFuncDeleteDoc
(
  PSZ         pszFolderName,           // name of folder containing the documents
  PSZ         pszDocuments             // list of documents being deleted
)
{
  PSZ         pszParm;                 // pointer for error parameters
  BOOL        fOK = TRUE;              // internal O.K. flag
  USHORT      usRC = NO_ERROR;         // function return code
  OBJNAME     szFolObject;             // folder object name
  OBJNAME     szDocObject;             // document object name
  SHORT       sNumOfDocs = 0;          // number of documents specified
  CHAR        szFolShortName[MAX_FILESPEC];// buffer for folder short name
  PSZ         pDocNameBuffer = NULL;   // list of documents of specified (sub)folder
  CHAR        szDocShortName[MAX_FILESPEC]; // buffer for document short name
  PSZ         pDocumentListBuffer = NULL; // buffer for list of documents

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
       BOOL fIsNew;

       fIsNew = !SubFolNameToObjectName( pszFolderName,  szFolObject );

       if ( !fIsNew )
       {
         PSZ pszDelim;
         pszDelim = strchr( pszFolderName, BACKSLASH );
         if ( pszDelim ) *pszDelim = EOS;
         ObjLongToShortName( pszFolderName, szFolShortName, FOLDER_OBJECT, &fIsNew );
         if ( pszDelim ) *pszDelim = BACKSLASH;
       } /* endif */

       if ( fIsNew )
       {
         fOK = FALSE;
         pszParm = pszFolderName;
         usRC = ERROR_XLATE_FOLDER_NOT_EXIST;
         OEMTOANSI(pszFolderName);
         UtlErrorHwnd( usRC, MB_CANCEL, 1, &pszParm, EQF_ERROR, HWND_FUNCIF );
         ANSITOOEM(pszFolderName);
       }
       else if ( QUERYSYMBOL( szFolObject ) != -1 )
       {
          pszParm = pszFolderName;
          fOK = FALSE;
          usRC = ERROR_FOLDER_LOCKED;
          OEMTOANSI(pszFolderName);
          UtlErrorHwnd( usRC, MB_CANCEL, 1, &pszParm, EQF_ERROR, HWND_FUNCIF );
          ANSITOOEM(pszFolderName);
       } /* endif */
     } /* endif */
   } /* endif */

   // check for empty document list
   if ( fOK )
   {
     if ( (pszDocuments == NULL) || (*pszDocuments == EOS) )
     {
       fOK = FALSE;
       usRC = FUNC_MANDFILES;
       UtlErrorHwnd( usRC, MB_CANCEL, 0, NULL, EQF_ERROR, HWND_FUNCIF );
     } /* endif */
   } /* endif */

   // check if documents exist
   if ( fOK )
   {
     // load short names of documents of (sub)folder
     LoadDocumentNames( szFolObject, HWND_FUNCIF,
                        LOADDOCNAMES_INCLSUBFOLDERS, (PSZ)&pDocNameBuffer );

     // build folder object name (access to folder properties is
     // required to correct folder drive letter)
     {
       PPROPFOLDER  ppropFolder;        // pointer to folder properties
       HPROP        hpropFolder;        // folder properties handle
       ULONG        ulErrorInfo;        // error indicator from property handler

       UtlMakeEQFPath( szFolObject, NULC, SYSTEM_PATH, NULL );
       strcat( szFolObject, BACKSLASH_STR );
       strcat( szFolObject, szFolShortName );
       strcat( szFolObject, EXT_FOLDER_MAIN );
       hpropFolder = OpenProperties( szFolObject, NULL,
                                     PROP_ACCESS_READ, &ulErrorInfo);
       if( hpropFolder )
       {
         ppropFolder = (PPROPFOLDER)MakePropPtrFromHnd( hpropFolder );
         if ( (ppropFolder->chDrive != EOS) && (ppropFolder->chDrive != ' ') )
         {
           szFolObject[0] = ppropFolder->chDrive;
         } /* endif */
         CloseProperties( hpropFolder, PROP_QUIT, &ulErrorInfo );
       } /* endif */
     }

     // allocate buffer for specified documents names as the list is modified
     // during processing
     if ( fOK )
     {
       int iLen = max( (strlen(pszDocuments)+2), 512 );

       fOK = UtlAllocHwnd( (PVOID *)&pDocumentListBuffer , 0L, iLen, ERROR_STORAGE, HWND_FUNCIF );
     } /* endif */

     if ( fOK )
     {
       PSZ pszCurDoc = pDocumentListBuffer;
       PSZ pszNext = NULL;
       BOOL fIsNew = FALSE;

       strcpy( pDocumentListBuffer, pszDocuments );
       while ( UtlGetNextFileFromCommaList( &pszCurDoc, &pszNext ) )
       {
         sNumOfDocs++;
         FolLongToShortDocName( szFolObject, pszCurDoc,
                                szDocShortName, &fIsNew );

         if ( fIsNew )
         {
           PSZ apszParms[2];
           fOK = FALSE;
           apszParms[0] = pszCurDoc;
           apszParms[1] = pszFolderName;
           usRC = DDE_DOC_NOT_IN_FOLDR;
           UtlErrorHwnd( usRC, MB_CANCEL, 2, apszParms, EQF_ERROR, HWND_FUNCIF );
         }
         else
         {
           // check if document is contained in document list of (sub)folder
           if ( pDocNameBuffer )
           {
             PSZ pszCurrent = pDocNameBuffer;
             fIsNew = TRUE;
             while( *pszCurrent && fIsNew )
             {
               if ( stricmp( pszCurrent, szDocShortName ) == 0 )
               {
                 // document is in list
                 fIsNew = FALSE;
               }
               else
               {
                 // continue with next one
                 pszCurrent += strlen(pszCurrent) + 1;
               } /* endif */
             } /* endwhile */

             if ( fIsNew )
             {
               PSZ apszParms[2];
               fOK = FALSE;
               apszParms[0] = pszCurDoc;
               apszParms[1] = pszFolderName;
               usRC = DDE_DOC_NOT_IN_FOLDR;
               UtlErrorHwnd( usRC, MB_CANCEL, 2, apszParms, EQF_ERROR, HWND_FUNCIF );
             } /* endif */
           } /* endif */
         } /* endif */

         // check if document is locked
         if ( fOK )
         {
            SHORT sRC;                       // return code of SYMBOL functions

            // set up document object name
            strcpy( szDocObject, szFolObject );
            strcat( szDocObject, BACKSLASH_STR );
            strcat( szDocObject, szDocShortName );
            sRC = QUERYSYMBOL( szDocObject );
            if ( sRC != -1 )
            {
               // document is in use ...
               fOK = FALSE;
               pszParm = pszCurDoc;
               usRC = ERROR_DOC_LOCKED;
               UtlErrorHwnd( ERROR_DOC_LOCKED, MB_CANCEL, 1,
                             &pszParm, EQF_INFO, HWND_FUNCIF );
            } /* endif */
         } /* endif */

         // delete the document
         if ( fOK )
         {
           USHORT usDummy = 0;

           if ( !DocumentDelete( szDocObject, FALSE, &usDummy ) )
           {
             fOK = FALSE;
             pszParm = pszCurDoc;
             usRC = NOT_DELETED;
             UtlErrorHwnd( usRC, MB_CANCEL, 1,
                           &pszParm, EQF_INFO, HWND_FUNCIF );
           }
           else
           {
             // broadcast document deleted message
             ObjBroadcast( WM_EQFN_DELETED, clsDOCUMENT, szDocObject );
           } /* endif */
         } /* endif */
       } /* endwhile */
     }

     // check if document list was empty
     if ( fOK && (sNumOfDocs == 0) )
     {
       fOK = FALSE;
       usRC = FUNC_MANDFILES;
       UtlErrorHwnd( usRC, MB_CANCEL, 0, NULL, EQF_ERROR, HWND_FUNCIF );
     } /* endif */
   }
   else
   {
   } /* endif */

   if ( pDocNameBuffer ) UtlAlloc( (PVOID *)&pDocNameBuffer, 0L, 0L, NOMSG );
   if ( pDocumentListBuffer ) UtlAlloc( (PVOID *)&pDocumentListBuffer, 0L, 0L, NOMSG );

   return( usRC );
}
#else
USHORT DocFuncDeleteDoc
(
  PSZ         pszFolderName,           // name of folder containing the documents
  PSZ         pszDocuments             // list of documents being deleted
)
{ return( 0 ); }
#endif

BOOL EQFOverTransWindow
(
  PDOCUMENT_IDA pIda,
  LONG          x,
  LONG           y
)
{
  BOOL fOK = FALSE;
  if ( pIda && pIda->pstEQFGen && pIda->pstEQFGen->hwndEditorTgt )
  {
    if ( IsWindow( pIda->pstEQFGen->hwndEditorTgt ) )
    {
      /****************************************************************/
      /* first check if cursor is over translation window             */
      /* -- if so check if we are over active segment                 */
      /****************************************************************/
      RECT rect;
      POINT pt;
      POINT pt1, pt2;
      pt.x = x;
      pt.y = y;
      ClientToScreen( pIda->hwnd, &pt );
      GetClientRect( pIda->pstEQFGen->hwndEditorTgt, &rect );
      pt1.x = rect.left; pt1.y = rect.top;
      pt2.x = rect.right; pt2.y = rect.bottom;
      ClientToScreen( pIda->pstEQFGen->hwndEditorTgt, &pt1 );
      ClientToScreen( pIda->pstEQFGen->hwndEditorTgt, &pt2 );

      if (pt1.x < pt.x && pt.x < pt2.x &&
          pt1.y < pt.y && pt.y < pt2.y )
      {
        PTBDOCUMENT pTBDoc = ACCESSWNDIDA( pIda->pstEQFGen->hwndEditorTgt,
                                           PTBDOCUMENT);
        fOK = EQFBMouseOnActSeg ( pTBDoc, pt.x - pt1.x, pt.y - pt1.y );
      } /* endif */
    } /* endif */
  } /* endif */

  return fOK;
}

USHORT DocGetFileDateAndSize
(
  PSZ         pszPath,                 // path of file being checked
  PLONG       plFileDate,              // ptr to buffer for long date
  PLONG       plFileSize               // ptr to buffer for file size
)
{
   FILEFINDBUF stFile;              // Output buffer of UtlFindFirst
   USHORT usCount;                  // For UtlFindFirst
   HDIR hSearch;                    // Directory handle for UtlFindFirst
   USHORT usRC;

   *plFileDate = 0L;
   *plFileSize = 0L;

   hSearch = HDIR_CREATE;
   memset( &stFile, 0, sizeof(stFile) );
   usCount = 1;
   usRC = UtlFindFirst( pszPath, &hSearch, 0, &stFile, sizeof(stFile),
                        &usCount, 0L, FALSE );
   UtlFindClose( hSearch, FALSE );

   if ( usRC == NO_ERROR )
   {
     FDATE FileDate;
     FTIME FileTime;
     struct tm DateTime;
     time_t lTime;

     FileTimeToDosDateTime( &stFile.ftLastWriteTime,
                            (LPWORD)&FileDate, (LPWORD)&FileTime );
     DateTime.tm_sec  = FileTime.twosecs * 2;
     DateTime.tm_min  = FileTime.minutes;
     DateTime.tm_hour = FileTime.hours;
     DateTime.tm_mday = FileDate.day;
     DateTime.tm_mon  = FileDate.month - 1;
     DateTime.tm_year = FileDate.year + 80;
     DateTime.tm_wday = 0;
     DateTime.tm_yday = 0;

     _tzset();
     DateTime.tm_isdst = _daylight;

     lTime = mktime( &DateTime );
     *plFileDate = lTime - 10800L; // correction: - 3 hours
     *plFileSize = RESBUFSIZE(stFile);
   } /* endif */

   return( usRC );
} /* end of function DocGetFileDateAndSize */

BOOL FolSpellcheck
(
  PSZ              pszFolObjName,      // folder object name
  BOOL             fFromFolderList,    // TRUE = called from folder list
  BOOL             fMultipleObjs       // TRUE = called with a list of folder object names
)
{
  POPENANDPOS pOpen = NULL;
  BOOL fOK = TRUE;
  PSZ pszList = NULL;                  // pointer to list of document object names
  LONG lListSize = 0;                  // current size (in bytes) of list of document object names
  LONG lListUsed = 0;                  // number of bytes already in use in the list of document object names

  // allocate open-and-pos structure
  if ( !UtlAlloc( (PVOID *)&pOpen, 0L, (LONG)sizeof(OPENANDPOS), ERROR_STORAGE ) )
  {
    return( FALSE );
  } /* endif */

  if ( FolIsSubFolderObject( pszFolObjName ) )
  {
    // convert pszFolObjName to folder object name by cutting the subfolder part and property part from the name
    strcpy( pOpen->szDocName, pszFolObjName );
    UtlSplitFnameFromPath( pOpen->szDocName );
    UtlSplitFnameFromPath( pOpen->szDocName );
  }
  else
  {
    // pszFolObjName contains the folder object name already
    strcpy( pOpen->szDocName, pszFolObjName );
  } /* endif */

  // get list of documents and add them to the list of spellchecked documents
  if ( fOK )
  {

    HWND hwndTemp = WinCreateWindow( NULLHANDLE, WC_LISTBOX, "", 0L, 0, 0, 0, 0, NULLHANDLE, HWND_TOP, 2, NULL, NULL );
    do
    {
      LONG lAddSize = 0;               // additional room needed in document object name list

      if ( fMultipleObjs )
      {
        int i = 0;
        int docs = 0;

        // fill temp listbox with documents of folder
        DELETEALLHWND( hwndTemp );
        strcpy( pOpen->szDocName, pszFolObjName );
        EqfSend2Handler( FOLDERHANDLER, WM_EQF_INSERTNAMES, MP1FROMHWND(hwndTemp), MP2FROMP(pszFolObjName) );

        // next folder
        pszFolObjName += strlen(pszFolObjName) + 1;
      }
      else if ( fFromFolderList )
      {
        EqfSend2Handler( FOLDERHANDLER, WM_EQF_INSERTNAMES, MP1FROMHWND(hwndTemp), MP2FROMP(pOpen->szDocName) );
      }
      else
      {
        EqfSend2Handler( FOLDERHANDLER, WM_EQF_QUERYSELECTEDNAMES, MP1FROMHWND(hwndTemp), MP2FROMP(pOpen->szDocName) );
      } /* endif */

      // compute additional size required for the documents in the listbox
      {
        lAddSize = 0;
        int iNumOfDocs = QUERYITEMCOUNTHWND( hwndTemp );
        int i = 0;
        int iFolNameLength = strlen( pOpen->szDocName ) + 1;
        while ( i < iNumOfDocs )
        {
          int iLen = SendMessage( hwndTemp, LB_GETTEXTLEN, i, 0 );
          if ( iLen != LB_ERR ) lAddSize += (iLen + iFolNameLength + 1); // add length of document object name (which is folder object name plus backslash plus document name)
          i++;
        } /*endwhile */
      }

      // enlarge the document list area
      if ( lAddSize )
      {
        fOK = UtlAlloc( (PVOID *)&pszList, lListSize, (lListSize + lAddSize + 1), ERROR_STORAGE );
        if ( fOK ) lListSize += lAddSize + 1;
      } /* endif */

      // add the document object names to the list
      if ( fOK && lAddSize )
      {
        int iNumOfDocs = QUERYITEMCOUNTHWND( hwndTemp );
        int i = 0;
        int iFolNameLength = strlen( pOpen->szDocName );
        while ( i < iNumOfDocs )
        {
          strcpy( pszList + lListUsed, pOpen->szDocName );
          lListUsed += iFolNameLength;
          pszList[lListUsed++] = '\\';

          SendMessage( hwndTemp,LB_GETTEXT, i, (LPARAM)(LPCSTR)(pszList + lListUsed) );
          lListUsed += strlen( pszList + lListUsed ) + 1; 
          i++;
        } /*endwhile */
      } /* endif */
    } while ( fOK && fMultipleObjs && *pszFolObjName );

    // terminate document object name list
    pszList[lListUsed] = EOS;

    WinDestroyWindow( hwndTemp );
  } /* endif */

  // check if all documents use the same target language and if there is spellcheck support for the target language
  if ( fOK )
  {
    PSZ pszCurDoc = pszList;
    char szLastTargetLang[MAX_LANG_LENGTH];      // last used target language
    char szDocTargetLang[MAX_LANG_LENGTH];       // target language of current document

    szLastTargetLang[0] = EOS;
    while ( fOK && (*pszCurDoc != EOS) )
    {
      szDocTargetLang[0] = EOS;
      DocQueryInfo( pszCurDoc, NULL, NULL, NULL, szDocTargetLang, FALSE );
      if ( szDocTargetLang[0] != EOS )
      {
        if ( (szLastTargetLang[0] != EOS) && (strcmp( szLastTargetLang, szDocTargetLang ) != 0) )
        {
          PSZ pszParms[2];
          pszParms[0] = szLastTargetLang;
          pszParms[1] = szDocTargetLang;
          UtlError( ERROR_DIFFERENT_TARGET_LANG, MB_CANCEL, 2, pszParms, EQF_ERROR );
          fOK = FALSE;
        } /* endif */
        strcpy( szLastTargetLang, szDocTargetLang );
      } /* endif */
      pszCurDoc += strlen(pszCurDoc) + 1;        // continue with next document
    } /* endwhile */

    // test target language
    if ( fOK && (szLastTargetLang[0] != EOS) )
    {
	    fOK = EQFBCheckSpellLang( szLastTargetLang );
      if ( !fOK  )
      {
        PSZ pszParm;
        pszParm = szLastTargetLang;
        UtlError( ERROR_NO_SPELL_SUPPORT_FOR_TARGET_LANG, MB_CANCEL, 1, &pszParm, EQF_ERROR );
      } /* endif */
    } /* endif */
  } /* endif */

  if ( fOK )
  {
    strcpy( pOpen->szDocName, pszList ); // start with first document in the list
    pOpen->ulSeg = 0;
    pOpen->usOffs= 0;
    pOpen->usLen = 0;
    pOpen->chFind[0] = 0;
    pOpen->fSpellcheck = TRUE;
    pOpen->pszDocumentList = pszList;
    EqfPost2Handler( DOCUMENTHANDLER, WM_EQF_PROCESSTASK, MP1FROMSHORT(OPEN_AND_POSITION_TASK), MP2FROMP(pOpen) );
  } /* endif */

  // cleanup in case of errors
  if ( !fOK )
  {
    if ( pszList != NULL ) UtlAlloc( (PVOID *)&pszList, 0, 0, NOMSG );
    if ( pOpen != NULL ) UtlAlloc( (PVOID *)&pOpen, 0, 0, NOMSG );
  } /* endif */

  return( fOK );
} /* end of function FolSpellcheck */


//------------------------------------------------------------------------------
// Function name:     SetDocWindowText
//------------------------------------------------------------------------------
// Function call:     display the correct title text for the document window
//------------------------------------------------------------------------------
// Description:       set the title for the Translation Environment
//------------------------------------------------------------------------------
// Parameters:        PDOCUMENT_IDA   ptr to document ida
//                    HWND            frame handle
//                    PSZ             document object name
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:     prepare the title for the Translation Environment
//                    using the resource string, the long filename (if applic)
//                    and set it.
VOID SetDocWindowText
(
  PDOCUMENT_IDA pDocIda,
  HWND          hframe,
  PSZ           pszObjName
)
{
  PSZ             pData[2];                // pointer to data
  PSZ             pTemp;                   // temp. data pointer
  CHAR            c;     

  HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
  DocQueryInfo2( pszObjName, NULL, NULL, NULL, NULL, pDocIda->szDocLongName, NULL, NULL, FALSE );
  if ( pDocIda->szDocLongName[0] )
  {
    OEMTOANSI( pDocIda->szDocLongName );
  }

  // display own title
  //  pDocIda->szBuf is of size 3*MAX_LONGPATH
  WinLoadString((HAB) UtlQueryULong( QL_HAB ), hResMod,
                SID_DOCWND_TITLE,
                MAX_LONGPATH,
                (pDocIda->szBuf+2*MAX_LONGPATH));

  /************************************************************/
  /* get folder name ...                                      */
  /************************************************************/
  pTemp = UtlGetFnameFromPath( pszObjName );
  pTemp --;
  c = *pTemp;
  *pTemp = EOS;
  strcpy(pDocIda->szFolderObjName, pszObjName );
  pDocIda->hwndActiveFolder =
        GETPARENT( EqfQueryObject( pszObjName, clsFOLDER, 0 ) );
  //strcpy( pDocIda->szBuf+2*MAX_LONGPATH,
  //        UtlGetFnameFromPath( pszObjName ));
  *pTemp = c;
  //pTemp = strchr( pDocIda->szBuf+2*MAX_LONGPATH, DOT );
  //if ( pTemp )
  //{
  //  *pTemp = EOS;
  //} /* endif */

  Utlstrccpy( pDocIda->szLongFolderName,
              UtlGetFnameFromPath( pDocIda->szFolderObjName ), DOT );
  ObjShortToLongName( pDocIda->szLongFolderName, pDocIda->szLongFolderName,
                      FOLDER_OBJECT );
  pData[1] = pDocIda->szLongFolderName;
  OEMTOANSI(pDocIda->szLongFolderName);

  if ( pDocIda->szDocLongName[0] != EOS )
  {
    pData[0] = pDocIda->szDocLongName;
  }
  else
  {
    pData[0] = UtlGetFnameFromPath( pszObjName );
  } /* endif */

  {
    ULONG Length;

    DosInsMessage( &pData[0], 2,
                   (pDocIda->szBuf+2*MAX_LONGPATH), MAX_LONGPATH,
                   pDocIda->szBuf, 2*MAX_LONGPATH,
                   &Length );
    // add MT Logging identifier to titlebar
    if ( UtlQueryUShort( QS_MTLOGGING ) != 0 )    
    {
        strcat( pDocIda->szBuf, " [MT logging is active]" );  
    } /* endif */     
  }
  if ( hframe )
  {
    CHAR_W chTitleW[(2*MAX_LONGPATH)+25];
    ASCII2Unicode( pDocIda->szBuf, chTitleW, 0 );
    SetWindowTextW( hframe, chTitleW );
  } /* endif */

} /* end of function SetDocWindowText */

//////////////////////////////////////////////////////////////////////////
// Set dictionary as accessed
/////////////////////////////////////////////////////////////////////////
//
//   run through the list and check that you can access the requested dicts
//
/////////////////////////////////////////////////////////////////////////
BOOL SetDictAccess
(
    PSZ   pszLDct                              // pointer to dictionary array
)
{
   BOOL  fOK = TRUE;                           // success indicator
   SHORT sRc;                                  // return code
   CHAR  szDictName[ MAX_LONGFILESPEC ];
   PSZ   pData;

   /*******************************************************************/
   /*  check if one of the dictionaries is locked ...                 */
   /*******************************************************************/
   pData = pszLDct;                            // store pointer
   while ( *pszLDct && fOK )
   {

      PROPDICT( szDictName, pszLDct );
      // check if symbol already exists, i.e. dict is used
      sRc = (SHORT) WinSendMsg( EqfQueryObjectManager(),
                                WM_EQF_QUERYSYMBOL,
                                NULL,
                                MP2FROMP( szDictName ));
      if ( sRc != -1 )
      {
         pData = UtlGetFnameFromPath( szDictName );
         pData = Utlstrccpy(szDictName,pData,'.');  // get rid of extension
         UtlError( ERROR_DICT_LOCKED, MB_CANCEL,
                   1, &pData, EQF_ERROR );
         fOK = FALSE;
      } /* endif */
      pszLDct += MAX_LONGFILESPEC;                      // point to next name
   } /* endwhile */

   /*******************************************************************/
   /*  set symbol if okay so far ...                                  */
   /*******************************************************************/
   if ( fOK )                                   // okay so far, set it
   {
     pszLDct = pData;                           // restore pointer
     while ( *pszLDct )
     {
        PROPDICT( szDictName, pszLDct );
        WinSendMsg( EqfQueryObjectManager(),
                    WM_EQF_SETSYMBOL,
                    MP1FROMSHORT( TRUE ),
                    MP2FROMP( szDictName ));
        pszLDct += MAX_LONGFILESPEC;                      // point to next name
     } /* endwhile */
   } /* endif */

   return fOK;
}

//////////////////////////////////////////////////////////////////////////
// Release dictionary accessed
/////////////////////////////////////////////////////////////////////////
//
//   run through the list and release any dictionary accessed
//
/////////////////////////////////////////////////////////////////////////
VOID ReleaseDictAccess
(
    PSZ   pszLDct                              // pointer to dictionary array
)
{
   CHAR  szDictName[ MAX_LONGFILESPEC ];

   while ( *pszLDct != EOS )
   {
      PROPDICT( szDictName, pszLDct );
      // remove the symbol from the object manager - don't care about errors
      WinSendMsg( EqfQueryObjectManager(),
                  WM_EQF_REMOVESYMBOL,
                  NULL,
                  MP2FROMP( szDictName ));
      pszLDct += MAX_LONGFILESPEC;
   } /* endwhile */

}

//f4////////////////////////////////////////////////////////////////////////////
// function CheckResources                                                    //
////////////////////////////////////////////////////////////////////////////////
BOOL CheckResources( PDOCUMENT_IDA pIda )
{
//------------------------------------------------------------------------------
// check if all resources for translation are available                       --
//------------------------------------------------------------------------------
   BOOL         fOk = TRUE;                 //error flag
   PSZ          pszTemp;                    //temorary pointer
   PSZ          apszReplace[3];             // replace strings in utl error
   HWND         hwnd;                       // pointer to window handle

   // check if symbol already exists
   {
      pszTemp = UtlGetFnameFromPath( pIda->IdaHead.szObjName );

      //save document name to IDA
      strcpy( pIda->szDocName, pszTemp );

      //get folder name
      pszTemp = UtlGetFnameFromPath( pIda->szFolderObjName );
      //save folder name to ida
      strcpy( pIda->szFolderName, pszTemp );
      //build full path to source  and save it to IDA
      UtlMakeEQFPath( pIda->szSource, pIda->szFolderObjName[0],
                      DIRSOURCEDOC_PATH, pIda->szFolderName );
      //append document name to path
      strcat( pIda->szSource, "\\" );
      strcat( pIda->szSource, pIda->szDocName );

   } /* endif */


   //check segmented target file
   if ( fOk )
   {
      //build full path to segmented target file and save it to IDA
      UtlMakeEQFPath( pIda->szSegTarget, pIda->szFolderObjName[0],
                      DIRSEGTARGETDOC_PATH,
                      pIda->szFolderName );
      //append document name to path
      strcat( pIda->szSegTarget, "\\" );
      strcat( pIda->szSegTarget, pIda->szDocName );
      //check if segmented target file exists
      fOk = UtlFileExist( pIda->szSegTarget );
      //check segmented source file
      if ( fOk )
      {
         //build full path to segmented source file and save it to IDA
         UtlMakeEQFPath( pIda->szSegSource, pIda->szFolderObjName[0],
                         DIRSEGSOURCEDOC_PATH,
                         pIda->szFolderName );
         //append document name to path
         strcat( pIda->szSegSource, "\\" );
         strcat( pIda->szSegSource, pIda->szDocName );
         //check if segmented source file exists
         fOk = UtlFileExist( pIda->szSegSource );
      }/* endif fOk*/
      if( !fOk )
      {
        /**************************************************************/
        /* start the segmentation of the file                         */
        /**************************************************************/
        hwnd = pIda->hwnd;

        /**************************************************************/
        /* capture mouse ...                                          */
        /**************************************************************/
        SETCAPTURE( hwnd );
        fOk = EQFTextSegm( pIda );
        /**************************************************************/
        /* free mouse ...                                             */
        /**************************************************************/
        RELEASECAPTURE;
        UtlDispatch();
        pIda = ACCESSWNDIDA( hwnd, PDOCUMENT_IDA );
        if ( pIda )
        {
          fTerminate[UtlGetTask()] = FALSE;          // not yet terminated via icon...
        }
        else
        {
          fOk = FALSE;
        } /* endif */
        //   UtlError( ERROR_NOT_ANALYZED, MB_CANCEL, NULL, NULL, EQF_ERROR );
      }/* endif fOk*/
   }/* endif fOk*/

   //check segmented source file
   if ( fOk )
   {
      //build full path to segmented source file and save it to IDA
      UtlMakeEQFPath( pIda->szSegSource, pIda->szFolderObjName[0],
                      DIRSEGSOURCEDOC_PATH,
                      pIda->szFolderName );
      //append document name to path
      strcat( pIda->szSegSource, "\\" );
      strcat( pIda->szSegSource, pIda->szDocName );
      //check if segmented source file exists
      fOk = UtlFileExist( pIda->szSegSource );
      if( !fOk )
      {
         UtlError( ERROR_NOT_ANALYZED, MB_CANCEL, 0, NULL, EQF_ERROR );
      }/* endif fOk*/
   }/* endif fOk*/
   if ( fOk )
   {
      CHAR szMemShortName[MAX_FILESPEC];
      BOOL fIsNew = FALSE;
      ObjLongToShortName( pIda->szDocMemory, szMemShortName, TM_OBJECT, &fIsNew );
      if ( fIsNew )
      {
         apszReplace[0] = pIda->szDocMemory;
         OEMTOANSI(pIda->szDocMemory);
         UtlError( ERROR_MEMORY_NOTFOUND, MB_CANCEL, 1, apszReplace, EQF_ERROR );
         ANSITOOEM(pIda->szDocMemory);
         fOk = FALSE;
      }
      else
      {
        strcpy( pIda->szMemory[0], pIda->szDocMemory );
      }/* endif fOk*/

   }/* endif fOk*/

   /*******************************************************************/
   /* get date/time of segmented target file                          */
   /*******************************************************************/
   if ( fOk )
   {
     struct stat buf;
     int iResult;
     iResult = stat( pIda->szSegTarget, &buf );
     if ( !iResult )
     {
       pIda->ulSegFileDate = buf.st_mtime;
     } /* endif */
   } /* endif */
   /*******************************************************************/
   /* set error code into EQFXLATE struct                             */
   /*******************************************************************/
   if ( !fOk )
   {
     EQFXlateError( EQFXLATE_TRANSL_ERROR );
   } /* endif */

   return ( fOk );
}/*CheckResources */
//f5////////////////////////////////////////////////////////////////////////////
// function BuildGenericStructure                                            //
//    fill the eqf generic structure with the appropriate information,
//    i.e.   segmented source, segmented target,
//           translation memory, dictionary
//    and check for the existence
////////////////////////////////////////////////////////////////////////////////
BOOL BuildGenericStructure( PDOCUMENT_IDA  pIda,
                            PSTEQFGEN      pstEQFGen,
                            PSZ            *apszTlm,
                            PSZ            *apszLDct,
                            BOOL           fInit )
{
   PPROPFOLDER  ppropFolder;                //pointer to folder properties
   PSZ          pszToken;
   USHORT       usI;
   BOOL         fOK = TRUE;

   //get access(pointer) to folder properties
   ppropFolder = (PPROPFOLDER)MakePropPtrFromHnd( pIda->hpropFolder );
   /*******************************************************************/
   /* init pstEQFGen only during our initialisation step              */
   /*******************************************************************/
   if ( fInit )
   {
     memcpy( pstEQFGen, &(pIda->ppropEdit->stEQFGen), sizeof( STEQFGEN ));
   } /* endif */
   pstEQFGen->pDoc = pIda;                       // get pointer to ida
   pstEQFGen->pstEQFPCmd = (PSTEQFPCMD) (pstEQFGen + 1);
   pstEQFGen->pOpenAndPos = pIda->pOpenAndPos;
   pIda->pOpenAndPos = NULL;
                                             // store format table name
   strcpy( (PSZ)pstEQFGen->szTagTable, pIda->szDocFormat );

   //set editor, proposal etc.  window positions
   pstEQFGen->fUseCoords = TRUE;


  pstEQFGen->hwndTWBS = pIda->hwnd;    // set client handle as parent

   //dictionary + translation memory accessed + MT access
   pstEQFGen->fsConfiguration = EQFF_DA_CONF + EQFF_TM_CONF + EQFF_MT_CONF ;

   //source file without path !!!!!!!!!!!!
   strcpy ((PSZ)pstEQFGen->szFileName, pIda->szDocName);
   strcpy( (PSZ)pstEQFGen->szLongName, pIda->szDocLongName );

   apszTlm[0]  = pIda->szMemory[0];          //full qualified memory db
   apszTlm[1]  = NULL;                       //NULL terminated

   // get stop at first exact match
   pIda->fStopAtFirstExact = ppropFolder->fStopAtFirstExact;

   //get list of folder R/O memories from folder properies
   if ( ppropFolder->aLongMemTbl[0][0] != EOS )
   {
     // BOOL fIsNew = FALSE;         // folder-is-new flag
     int i = 0;
     usI = 1;
     while ( (ppropFolder->aLongMemTbl[i][0] != EOS) &&
             (i < EQF_MAX_TM_BASES) && fOK )
     {
       //ObjLongToShortName( ppropFolder->aLongMemTbl[i],
       //                    pIda->szMemory[usI], TM_OBJECT, &fIsNew );
       strcpy( pIda->szMemory[usI], ppropFolder->aLongMemTbl[i] );

       i++;
      // fOK = (BOOL) EqfSend2Handler( MEMORYHANDLER,
      //                               WM_EQF_PROCESSTASK,
      //                               MP1FROMSHORT( CREATE_PATH ),
      //                               MP2FROMP(pIda->szMemory[usI]) );
      // if( !fOK )
      // {
      //    pszToken = ppropFolder->aLongMemTbl[i-1];
      //    OEMTOANSI(pszToken);
      //    UtlError( ERROR_TM_NOT_EXIST, MB_CANCEL, 1,
      //              &pszToken, EQF_ERROR );
      //    ANSITOOEM(pszToken);
      //}
      //else
       {
         /*****************************************************************/
         /* check that same mem is already used as write mem too          */
         /* only compare with first one nec,- other compares done in      */
         /* folder selection dialog                                       */
         /*****************************************************************/
         if (strcmp(pIda->szMemory[usI], pIda->szMemory[0]) )
         {
           apszTlm[usI] = pIda->szMemory[usI];
           usI++;
         }
         else
         {
           /*************************************************************/
           /* skip this memory                                          */
           /*************************************************************/
           pIda->szMemory[usI][0] = EOS;
         } /* endif */
       } /* endif */
     } /* endwhile */
     //terminate with NULL
     apszTlm[usI] = NULL;
   }
   else
   {
     strcpy( pIda->szString, ppropFolder->MemTbl );

     //build path to memory properties
     UtlMakeEQFPath(pIda->szDicPath, NULC, MEM_PATH, NULL );
     strcat( pIda->szDicPath, "\\" );

     //get first R/O memory from list
     pszToken = strtok( pIda->szString, X15_STR );
     usI = 1;
     while ( (pszToken != NULL) && fOK )
     {
       strcpy( pIda->szMemory[usI], pszToken );
       //fOK = (BOOL) EqfSend2Handler( MEMORYHANDLER,
       //                              WM_EQF_PROCESSTASK,
       //                              MP1FROMSHORT( CREATE_PATH ),
       //                              MP2FROMP(pIda->szMemory[usI]) );
       //if( !fOK )
       //{
       //   UtlError( ERROR_TM_NOT_EXIST, MB_CANCEL, 1,
       //             &pszToken, EQF_ERROR );
       //}
       //else
       {
         /*****************************************************************/
         /* check that same mem is already used as write mem too          */
         /* only compare with first one nec,- other compares done in      */
         /* folder selection dialog                                       */
         /*****************************************************************/
         if (strcmp(pIda->szMemory[usI], pIda->szMemory[0]) )
         {
           apszTlm[usI] = pIda->szMemory[usI];
           usI++;
         }
         else
         {
           /*************************************************************/
           /* skip this memory                                          */
           /*************************************************************/
           pIda->szMemory[usI][0] = EOS;
         } /* endif */
         // get next one
         pszToken = strtok( NULL, X15_STR );
       } /* endif */
     } /* endwhile */
     //terminate with NULL
     apszTlm[usI] = NULL;
   }

   if ( fOK )
   {
     if ( ppropFolder->aLongDicTbl[0][0] != EOS )
     {
       BOOL fIsNew = FALSE;         // folder-is-new flag
       int i = 0;

       //build path to dictionary properties
       UtlMakeEQFPath(pIda->szDicPath, NULC, PROPERTY_PATH, NULL );
       strcat( pIda->szDicPath, "\\" );

       usI = 0;
       while ( (ppropFolder->aLongDicTbl[i][0] != EOS) &&
               (i < MAX_NUM_OF_FOLDER_DICS) && fOK )
       {
         ObjLongToShortName( ppropFolder->aLongDicTbl[i],
                             pIda->szString, DICT_OBJECT, &fIsNew );
         i++;

          strcpy(pIda->szDicts[usI], pIda->szDicPath);
          strcat(pIda->szDicts[usI], pIda->szString );
          strcat( pIda->szDicts[usI], EXT_OF_DICTPROP );

          apszLDct[usI] = pIda->szDicts[usI];
          usI++;
       } /* endwhile */
       //terminate with NULL
       apszLDct[usI] = NULL;
     }
     else
     {
       //get list of folder dictionaries from folder properies
       strcpy( pIda->szString, ppropFolder->DicTbl );

       //build path to dictionary properties
       UtlMakeEQFPath(pIda->szDicPath, NULC, PROPERTY_PATH, NULL );
       strcat( pIda->szDicPath, "\\" );

       //get first dictionary from list
       memset(&pIda->szDicts[0], 0, sizeof(pIda->szDicts));
       pszToken = strtok( pIda->szString, X15_STR );
       usI = 0;
       while ( pszToken != NULL )
       {
          strcpy(pIda->szDicts[usI], pIda->szDicPath);
          strcat(pIda->szDicts[usI], pszToken);
          strcat( pIda->szDicts[usI], EXT_OF_DICTPROP );

          apszLDct[usI] = pIda->szDicts[usI];
          usI++;
          //get next dictionary name
          pszToken = strtok( NULL, X15_STR );
       } /* endwhile */
       //terminate with NULL
       apszLDct[usI] = NULL;
     }
   } /* endif */
   return fOK;
}/*end BuildGenericStructure */



/**********************************************************************/
/* GetLanguage id for the selected source/target language             */
/**********************************************************************/
USHORT GetLangID( PDOCUMENT_IDA pIda )
{
  WORD wLangID = 0;
  SHORT sLangType = MorphGetLanguageType( pIda->szDocTargetLang );
  if ( sLangType == MORPH_UNDEFINED_LANGTYPE )
  {
    sLangType = MorphGetLanguageType( pIda->szDocSourceLang );
  } /* endif */


  switch ( sLangType )
  {
    case MORPH_THAI_LANGTYPE:
      wLangID = MAKELANGID(LANG_THAI, SUBLANG_DEFAULT);
      break;
    case MORPH_BIDI_H_LANGTYPE:
      wLangID = MAKELANGID(LANG_HEBREW, SUBLANG_DEFAULT);
      break;
    case MORPH_BIDI_A_LANGTYPE:
      wLangID = MAKELANGID(LANG_ARABIC, SUBLANG_DEFAULT);
      break;

    default:
      break;
  } /* endswitch */

  return wLangID;
}


