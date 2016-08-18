/*! \file
	Description:
	This is the current front end for the ITM program.       
	This will be replaced by an integrated version later on. 
	
	Copyright Notice:

	Copyright (C) 1990-2016, International Business Machines
	Corporation and others. All rights reserved
*/

#define INCL_EQF_TM               // general Transl. Memory functions
#define INCL_EQF_TP               // public translation processor functions
#define INCL_EQF_ANALYSIS         // analysis functions
#define INCL_EQF_EDITORAPI        // editor API
#define INCL_EQF_MORPH            // morphological functions
  // use import DLL defines for hResMod and dbcs_cp ...
#define DLLIMPORTRESMOD
#include <eqf.h>                  // General Translation Manager include file

#include "EQFITM.H"
#include "EQFB.ID"
#include "EQFITMD.id"             // ITM dialog include file
#include <eqfsetup.h>             // twbinit, etc.
#include <eqfdrvex.h>             // twbsetfontcharacteristics

#include <process.h>           // for thread functions (_beginthread)
#include <eqfprogr.h>             // get slider definitions

#include "core\PluginManager\OtmPlugin.h"
#include "core\PluginManager\PluginManager.h"
//#include "InitPlugins.h"
//------------------------------------------------------------------------------
//  Helptable include files                                                     
//     - ID files for dialogs and windows                                       
//     - help ID file                                                           
//     - message ID file                                                        
//     - help table                                                             
//------------------------------------------------------------------------------
#include "eqfstart.h"
#include "eqfutils.id"
#include "eqfdoc01.id"
#include "eqfiana1.id"
#include "eqfmem.id"
#include "eqffol.id"
#include "eqffll.id"
#include "eqftag00.id"                 // tag handler IDs
#include <EQFTIMP.ID>
#include <EQFTEXP.ID>
#include <EQFLNG00.ID>
#include "eqfdic00.id"
#include "eqfutclb.id"
#include "EQFDIMP.ID"
#include "EQFDEX.ID"
#include "EQFTIMP.ID"
#include "EQFTEXP.ID"
#include "EQFCNT01.ID"
#include "EQFFILT.ID"
#include "EQFQDPR.ID"
#include "EQFTwb.ID"                   // IDs of Twb dialogs
#include "EQFBDLG.ID"                  // IDs of Translation Processor dialogs
#include "EQFDPROP.ID"                 // IDs of Dictionary Dialogs - 1 -
#include "EQFDDLG.ID"                  // IDs of Dictionary Dialogs - 2 -
#include "EQFRDICS.ID"                 // IDs for remote dictionary
#include "EQFLIST.ID"                  // IDs of list handler windows and dialogs
#include "EQFTMFUN.ID"                 // IDs of memory...
#include "EQFTMM.ID"                   // IDs of memory ...
#include "EQFMT.ID"                    // IDs of Machine Translation memory ...
#ifdef _WPTMIF
  #include "EQFWPSET.ID"               // IDs of WP API
#endif
#include "eqfhelp.id"                  // help resource IDs
#include "eqfrpt.id"                 // IDs report handler dialog
#include "eqfhlp1.h"                   // first part of help tables
#include "eqfmsg.htb"                  // message help table

#define PROGRESS_LEN   75              // number of dots...
#define PROGRESS_STR   "."             // progress string...


#define MAX_RC_STRING  31
CHAR chTMFile[ MAX_RC_STRING ];
CHAR chTargetFile[ MAX_RC_STRING ];
CHAR chSourceFile[ MAX_RC_STRING ];
CHAR chStatus[ MAX_RC_STRING ];
CHAR chCleanup[ MAX_RC_STRING ];
CHAR chAnalyse[ MAX_RC_STRING ];
CHAR chAligning[ MAX_RC_STRING ];
CHAR chPrepare[ MAX_RC_STRING ];
CHAR chError[ MAX_RC_STRING ];

/*******************************************************************/
/*                             Globals                             */
/*******************************************************************/
CHAR szMsgWindow[] = ">wnd_msg";     // identifier for message help window

static CHAR   EqfSystemMsgFile[MAX_EQF_PATH];  // global message file
static CHAR   EqfSystemHlpFile[MAX_EQF_PATH];  // global help file
static CHAR   szEqfSysLanguage[MAX_EQF_PATH];  // system language
static CHAR   szEqfResFile[MAX_EQF_PATH];      // resource
static CHAR cTermBuf[256];           // display buffer for text
static CHAR cDisp[PROGRESS_LEN+1];   // display buffer for progress indication
LOADEDTABLE LoadedTable ;              // pointer to loaded tag tables

static HMENU  hwndMenu;                // handle of actionbar
/**********************************************************************/
/* prototypes of static functions and setup functions  for the        */
/* TranslationManager/2 environment..                                 */
/**********************************************************************/
static BOOL CheckCmdLine ( USHORT usArgc, PSZ  * ppArgv, PITMIDA pITMIda );
static BOOL ListOfFiles ( PITMIDA,  PSZ , PSZ **);
static BATCHCMD ValidateToken ( PSZ  *ppToken, PCMDLIST  pCmdList );

/**********************************************************************/
/* init colors and keys                                               */
/**********************************************************************/
BOOL ITMInit(PITMIDA);

/**********************************************************************/
/* enable(disable AAB                                                 */
/**********************************************************************/
static VOID VisInitMenu ( HWND, USHORT , PITMIDA);        // enable/disable AAB item
static void ITMCommand (HWND hwnd, WPARAM mp1) ;


/**********************************************************************/
/* list of accepted commandline parameters and their value            */
/* ATTENTION: ValidateToken had to be changed because /QUAL has been  */
/*            recognized as "/QU" ! ( since /QU is substring of /QUAL)*/
/* FUTURE: avoid overlapping of the strings!  (15/03/2000)            */
/**********************************************************************/
CMDLIST ITMCmdList[]=
  {
    {BATCH_MEM,     "/MEM=",      "/ME="     },     // translation memory
    {BATCH_FILES,   "/FILES=",    "/FI="     },     // input file or list of files
    {BATCH_MARKUP,  "/MARKUP=",   "/MA="     },     // markup language
    {BATCH_TYPE,    "/TYPE=",     "/TY="     },     // type of selection
    {BATCH_SGMLMEM, "/SGMLMEM=",  "/SG="     },     // name of the SGML output file
    {BATCH_LEVEL ,  "/LEVEL=",    "/LE="     },     // level of match
    {BATCH_TGTLNG,  "/TGTLNG=",   "/TG="     },     // target language
    {BATCH_SRCLNG,  "/SRCLNG=",   "/SR="     },     // source language
    {BATCH_HWND,    "/HWND=",     "/HW="     },
    {BATCH_QUIET,   "/QUIET",     "/QU"       },            // quiet
    {BATCH_SRCSTARTPATH, "/SRCSTARTPATH=", "/SS="    },     // sourcestart path
    {BATCH_TGTSTARTPATH, "/TGTSTARTPATH=", "/TS="    },     // targetstart path
    {BATCH_QUALITYLEVEL, "/QUAL=", "/QL="    },     // quality level
    {BATCH_SGMLFORMAT,   "/SGMLFORMAT=",   "/SF="    },     // format of sgml-memory
    {BATCH_END,     "",           ""         }      // end of list indicator
  };

/**********************************************************************/
/* list of available type options                                     */
/**********************************************************************/
CMDLIST ITMCmdListTypes[]=
  {
    { BATCH_NOANA,   "NOANA",     "NOANA"    },
    { BATCH_NOTMDB,  "NOTM",      "NOTM"     },
    { BATCH_NOCONF,  "NOCONF",    "NOCONF"   },
    { BATCH_VISUAL,  "VISUAL",    "VISUAL"   },
    { BATCH_PREPARE, "PREPARE",   "PREPARE"  },
    { BATCH_ICON,    "MINIMIZE",  "MIN"      },
    { BATCH_END,   "" }
  };
//    { BATCH_SGMLUNICODE,    "UNICODE",    "UNICODE" },
//    { BATCH_SGMLASCII,      "ASCII",      "ASCII"   },
//    { BATCH_SGMLANSI,       "ANSI",       "ANSI"    },


// rectangles for invalidations..
RECT rectProcess;
RECT rectOther;

USHORT usOldTask;
MRESULT APIENTRY ITMPROCESSDLG
(
  HWND hDlg,                           // Dialog window handle
  WINMSG msg,                          // Message ID
  WPARAM mp1,                          // Message parameter 1
  LPARAM mp2                           // Message parameter 2
);


  static HINSTANCE hinstSlider;                    // slider control lib handle

//------------------------------------------------------------------------------
// External function                                                            
//------------------------------------------------------------------------------
// Function name:     ITMWNDPROC                                                
//------------------------------------------------------------------------------
// Function call:     window procedure                                          
//------------------------------------------------------------------------------
// Description:       This window procedure controls the display of the         
//                    process window and handles the communication with the     
//                    thread. The communication with the thread is timer        
//                    dependant.                                                
//------------------------------------------------------------------------------
// Parameters:        HWND hwnd   window handle                                 
//                    USHORT msg  message number                                
//                    WPARAM mp1  message parameter1                            
//                    LPARAM mp2  message parameter2                            
//------------------------------------------------------------------------------
// Returncode type:   MRESULT                                                   
//------------------------------------------------------------------------------
// Returncodes:       from default window procedure                             
//                                                                              
//------------------------------------------------------------------------------
// Function flow:     dependant on message do case processing                   
//                    WM_CREATE:                                                
//                      post msg to allow window to be primarily painted        
//                    WM_EQF_INIT                                               
//                      disable AAB                                             
//                      create all MAT handlers nec to run ITM                  
//                      read in propertyfile                                    
//                      Check commandline                                       
//                      if not ok or no commandline parameters                  
//                        popup dialog                                          
//                      if ok                                                   
//                        start timer                                           
//                        do the initialization                                 
//                        create windows for the visualization                  
//                      if error force shutdown                                 
//                    WM_ERASEBACKGROUND                                        
//                      get the background cleared in the system default color  
//                    WM_TIMER                                                  
//                      switch (usStatus of ITM)                                
//                       ITM_STAT_CLEANUP                                       
//                         if ok                                                
//                          change display so user sees something is happening  
//                         else post WM_QUIT                                    
//                       ITM_STAT_VISUAL                                        
//                          stop timer and create visualization windows         
//                           or write continuation file (if'Prepare' selected)  
//                           start timer                                        
//                       ITM_STAT_ENDVISUAL                                     
//                          reset to aligning for next filepair                 
//                       default:                                               
//                         if fkill                                             
//                           post a WM_CLOSE                                    
//                         else                                                 
//                           if not busy flag set                               
//                             if still a pair of files available               
//                               copy the files in the appro. variables of IDA  
//                               if params not too long and not srcfile equal   
//                                 to tgtfile                                   
//                                 set the fBusy flag and                       
//                                    reset the progress displ.                 
//                               else set fKill and post WM_CLOSE               
//                             else display successful completion message       
//                                  post WM_CLOSE                               
//                           else update progrss display                        
//                        endif                                                 
//                        force a redisplay of the window                       
//                    WM_PAINT                                                  
//                      repaint the screen                                      
//                    WM_INITMENU                                               
//                      init the menu bar                                       
//                    WM_COMMAND                                                
//                      call ITMCOmmand to handle menu commands                 
//                    WM_CLOSE                                                  
//                      if ITM_STAT_VISUAL, call ITMFuncQuit                    
//                      indicate thread that it had to stop                     
//                      wait until thread finished with current work            
//                      set fKill and ITM_STAT_CLEANUP                          
//                    HM_QUERY_KEYS_HELP:                                       
//                      return help ID of keys help                             
//                    HM_HELPSUBITEM_NOT_FOUND:                                 
//                      IPF didn_t find our window in its help table,           
//                      now we try to do IPF's work                             
//                    HM_EXT_HELP_UNDEFINED:                                    
//                      no ext. help found: display help for TWB                
//                    HM_ERROR:                                                 
//                      handle errors returned by the help instances            
//                    default:                                                  
//                      pass on the message to the default window procedure..   
//                  endswitch                                                   
//                  return mResult                                              
//------------------------------------------------------------------------------

MRESULT APIENTRY ITMWNDPROC
(
  HWND hwnd,
  WINMSG msg,
  WPARAM mp1,
  LPARAM mp2
)
{
    MRESULT mResult = FALSE;           // window proc return value
    PITMIDA  pITMIda;                  // pointer to ida
    BOOL   fOK;                        // success indicator
    SHORT  sRC;
    HMODULE hResMod;

    switch( msg )
    {
        case WM_CREATE:
          /************************************************************/
          /* post message to allow window to be primarily painted     */
          /************************************************************/
          WinPostMsg( hwnd, WM_EQF_INITIALIZE, NULL, NULL);
          break;
        case WM_EQF_INITIALIZE:
          /*******************************************************************/
          /* Attention: AAB now anchored at TWB main window....              */
          /*******************************************************************/
          hwndMenu = GetMenu( hwnd );
          pITMIda = ACCESSWNDIDA( hwnd, PITMIDA );
           /******************************************************************/
           /* Register slider control                                        */
           /******************************************************************/
          RegisterProgressControl( (HINSTANCE) pITMIda->hab );

          /************************************************************/
          /* init the resources                                       */
          /************************************************************/
          hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
          WinLoadString( pITMIda->hab, hResMod, IDS_ITM_STATUS,
                         MAX_RC_STRING, chStatus);
          WinLoadString( pITMIda->hab, hResMod, IDS_ITM_CLEANUP,
                         MAX_RC_STRING, chCleanup);
          WinLoadString( pITMIda->hab, hResMod, IDS_ITM_ANALYSE,
                         MAX_RC_STRING, chAnalyse);
          WinLoadString( pITMIda->hab, hResMod, IDS_ITM_ALIGNING,
                         MAX_RC_STRING, chAligning);
          WinLoadString( pITMIda->hab, hResMod, IDS_ITM_PREPARE,
                         MAX_RC_STRING, chPrepare);
          WinLoadString( pITMIda->hab, hResMod, IDS_ITM_ERROR,
                         MAX_RC_STRING, chError);
          WinLoadString( pITMIda->hab, hResMod, IDS_ITM_TMFILE,
                         MAX_RC_STRING, chTMFile);
          WinLoadString( pITMIda->hab, hResMod, IDS_ITM_TARGETFILE,
                         MAX_RC_STRING, chTargetFile);
          WinLoadString( pITMIda->hab, hResMod, IDS_ITM_SOURCEFILE,
                         MAX_RC_STRING, chSourceFile);
          /****************************************************************/
          /* create all MAT handlers necessary to run ITM tool            */
          /****************************************************************/
          WinQueryProfileString( pITMIda->hab, APPL_Name, KEY_SYSLANGUAGE,
                                   DEFAULT_SYSTEM_LANGUAGE,
                                   szEqfSysLanguage, sizeof( szEqfSysLanguage ));
          fOK = UtlQuerySysLangFile( szEqfSysLanguage, szEqfResFile,
                                     EqfSystemHlpFile, EqfSystemMsgFile );
          if ( fOK )
          {
            fOK = ITMHandlers( pITMIda->hab );
            if(fOK)
            {
               CHAR szPluginPath[ 256 ];
               UtlSetString( QST_PLUGINPATH, "PLUGINS" );
               UtlQueryString( QST_PLUGINPATH, szPluginPath, sizeof( szPluginPath ));
               UtlMakeEQFPath( szPluginPath, NULC, PLUGIN_PATH, NULL );
               //InitializePlugins( szPluginPath );
               PluginManager* thePluginManager = PluginManager::getInstance();
               thePluginManager->loadPluginDlls(szPluginPath);
               InitTMPluginWrapper();
               InitDictPluginWrapper();
               InitMarkupPluginMapper();
               InitDocumentPluginMapper();
            }
          } /* endif */
          if ( fOK )
          {
            /*************************************************************/
            /* Check input parameters                                    */
            /* read in property file                                  */
            /* if error during PropRead, goon nevertheless, donot close ITM */
            /*************************************************************/
            sRC =  EQFITMPropRead(pITMIda);
            /**********************************************************/
            /* dummy flag now, for future use:if user wants no        */
            /* visualization, it must be set to FALSE                 */
            /**********************************************************/
            pITMIda->fCurDispStatus = DISP_ALIGNING;
            pITMIda->fVisual = FALSE;                    //default: no visual
            if ( pITMIda->usArgc != 1 )
            {
              fOK = CheckCmdLine( pITMIda->usArgc, pITMIda->ppArgv, pITMIda );
            } /* endif */

            /************************************************************/
            /* popup dialog if no cmdline arguments or cmdline wrong    */
            /************************************************************/
            if ( (pITMIda->usArgc == 1) ||
                 ((pITMIda->hwndParent != HWND_DESKTOP) && !fOK) )
            {
              INT_PTR iRc;
              DIALOGBOXW( hwnd, EQFBITMDLGPROC, hResMod, ID_ITM_MAIN_DLG,
                         pITMIda, iRc);

              if ( iRc == DID_ERROR )
              {
                if ( pITMIda->fQuiet )
                {

                }
                else
                {
                  ITMUtlError( pITMIda, ERROR_DIALOG_LOAD_FAILED, MB_CANCEL,
                               0 , NULL, EQF_ERROR );
                } /* endif */
                fOK = FALSE;
              }
              else
              {
                fOK = iRc;
              } /* endif */
            } /* endif */
            /************************************************************/
            /* create windows for visualization                         */
            /************************************************************/
            if ( fOK )
            {
              if ( !pITMIda->fMinimized )
              {
                WinSetWindowPos( (HWND) UtlQueryULong( QL_TWBFRAME ),
                         HWND_TOP,
                         40,
                         40,
                         (USHORT) (WinQuerySysValue( HWND_DESKTOP, SV_CXSCREEN) * .9),
                         (USHORT) (WinQuerySysValue( HWND_DESKTOP, SV_CYSCREEN) * .9),
                         EQF_SWP_SIZE | EQF_SWP_MOVE |
                         EQF_SWP_ACTIVATE | EQF_SWP_SHOW | EQF_SWP_MAXIMIZE );
              } /* endif */

              WinLoadString( pITMIda->hab, hResMod, IDS_ITM_TITLE_TM,
                             80, (pITMIda->szBuffer+MAX_SEGMENT_SIZE));
              sprintf( pITMIda->szBuffer, (pITMIda->szBuffer+MAX_SEGMENT_SIZE),
                       pITMIda->chLongTranslMemory );
                 /************************************************************/
                 /* get rid of AAB for the moment                            */
                 /************************************************************/
                 SetMenu( hwnd, NULL );
                 WinSetWindowText( pITMIda->hwnd, pITMIda->szBuffer );


          {
            FARPROC lpfnProc = MakeProcInstance(  (FARPROC)(ITMPROCESSDLG),
                                                  (HAB) UtlQueryULong( QL_HAB ) );
            pITMIda->hProcWnd =
              CreateDialogParam( hResMod,
                                 MAKEINTRESOURCE(ID_ITM_PROCESS_DLG),
                                 hwnd,
                                 (DLGPROC)lpfnProc,
                                 MP2FROMP((PVOID)pITMIda) );
            if ( !pITMIda->hProcWnd )
            {
              fOK = FALSE;
              ITMUtlError( pITMIda, ERROR_DIALOG_LOAD_FAILED, MB_CANCEL,
                           0 , NULL, EQF_ERROR );
            }
            else
            {
              pITMIda->fBusy = TRUE;
            } /* endif */
          }

          } /* endif */
            if ( fOK )
            {
              if (WinStartTimer( pITMIda->hab, hwnd, ITM_TIMER_ID, ITM_TIMER_VAL ))
              {
                pITMIda->fTimer = TRUE;
              }
              fOK = ITMInit( pITMIda );
            } /* endif */
          } /* endif */
          /************************************************************/
          /* in case of error force a shut-down                       */
          /************************************************************/
          if ( !fOK )
          {
            WinPostMsg( hwnd, WM_CLOSE, NULL, NULL );
          } /* endif */
          break;

        case WM_SIZE:
          pITMIda = ACCESSWNDIDA( hwnd, PITMIDA );
          if (pITMIda )
          {
            ITMSetWindowPos( pITMIda, SHORT1FROMMP2(mp2), SHORT2FROMMP2(mp2) );
          } /* endif */
          break;

        case WM_EQF_UPDATESLIDER:
          /**************************************************************/
          /* Check if slider arm position requires update               */
          /**************************************************************/
          pITMIda = ACCESSWNDIDA( hwnd, PITMIDA );
          if ( pITMIda )
          {
            static USHORT usComplete = 0;
            if ( (SHORTFROMMP1(mp1) > -1) &&
                 ( (usComplete != USHORTFROMMP1(mp1)) || (USHORTFROMMP1(mp1)== 0)) )
            {
              HWND hwndSlider;

              usComplete = USHORTFROMMP1( mp1 );

              hwndSlider = GETHANDLEFROMID( pITMIda->hProcWnd,
                                            ID_ITM_PROCESS_SLIDER );
              SendMessage( hwndSlider, PB_SETPOS, usComplete, 0 );
              UpdateWindow( hwndSlider );
            }
            if (SHORTFROMMP1(mp1) == -1)
            {
              HWND hwndText;
              hwndText = GETHANDLEFROMID( pITMIda->hProcWnd, ID_ITM_PROCESS_TASK_TEXT );
              WinSetWindowText( hwndText, (LPCSTR) PVOIDFROMMP2(mp2) );
            }
            else if (SHORTFROMMP1(mp1) == -2)
            {
              HWND hwndText;
              hwndText = GETHANDLEFROMID( pITMIda->hProcWnd, ID_ITM_PROCESS_TASK_TEXT2 );
              WinSetWindowText( hwndText, (LPCSTR) PVOIDFROMMP2(mp2) );
            }
            else if ( mp2 )
            {
              HWND hwndText;
              hwndText = GETHANDLEFROMID( pITMIda->hProcWnd, ID_ITM_PROCESS_TASK_TEXT );
              WinSetWindowText( hwndText, (LPCSTR) PVOIDFROMMP2(mp2) );
            } /* endif */
          } /* endif */
          break;
        case WM_TIMER:
          /************************************************************/
          /* if no error yet update the progress                      */
          /************************************************************/
          pITMIda = ACCESSWNDIDA( hwnd, PITMIDA );
          switch ( pITMIda->usStatus )
          {
            case  ITM_STAT_CLEANUP:
              if ( pITMIda->fThreadActive )
              {
                /********************************************************/
                /* do a change in the display so the user sees something*/
                /* is happening (at least the display changed....)      */
                /********************************************************/
                if ( strlen( cDisp )  >= PROGRESS_LEN )
                {
                  strcpy( cDisp, PROGRESS_STR );
                }
                else
                {
                  strcat( cDisp, PROGRESS_STR );
                } /* endif */
              }
              else
              {
                 if (pITMIda->fTimer )
                 {
                   WinStopTimer( pITMIda->hab, hwnd, ITM_TIMER_ID );
                   pITMIda->fTimer = FALSE;
                 } /* endif */
                 ITMCloseProcessing( pITMIda );
                 EqfStopObjectManager( TWBCLOSE );
                 WinDestroyWindow( hwnd );
              } /* endif */
              break;
            case  ITM_STAT_VISUAL:
              /********************************************************/
              /* stop timer and create visualization windows          */
              /********************************************************/
              TimerStartVisual( hwnd, pITMIda );
              break;
            case  ITM_STAT_ENDVISUAL:
                 SetMenu( hwnd, NULL );
                /******************************************************/
                /* reset to aligning for next file pairs              */
                /******************************************************/
              pITMIda->fCurDispStatus = DISP_ALIGNING;
              pITMIda->usStatus = ITM_STAT_ALIGN;
              pITMIda->fBusy = FALSE;
//            WinShowWindow( pITMIda->stVisDocSrc.pDoc->hwndFrame, FALSE );
//            WinShowWindow( pITMIda->stVisDocTgt.pDoc->hwndFrame, FALSE );
//            WinShowWindow( pITMIda->hStatusBarWnd, FALSE );
              WinDestroyWindow( pITMIda->stVisDocSrc.pDoc->hwndFrame );
              WinDestroyWindow( pITMIda->stVisDocTgt.pDoc->hwndFrame );
              WinDestroyWindow( pITMIda->hStatusBarWnd );

              if (!pITMIda->fTimer )
              {
                if (WinStartTimer (pITMIda->hab, hwnd, ITM_TIMER_ID, ITM_TIMER_VAL) )
                {
                   pITMIda->fTimer = TRUE;
                } /* endif */
              } /* endif */
              /********************************************************/
              /* initiate a new process loop...                       */
              /********************************************************/
              PostMessage( hwnd, WM_EQF_PROCESSTASK,
                           MP1FROMSHORT( ITM_PROCESS_PREPFILE ),
                           MP2FROMSHORT( 0 ));
              break;
            default :
              if ( pITMIda->fKill )
              {
                /********************************************************/
                /* end of processing                                    */
                /********************************************************/
                WinPostMsg( hwnd, WM_CLOSE, NULL, NULL );
              }
              else
              {
                /************************************************************/
                /* waiting for work ????                                    */
                /************************************************************/
                {
                  if ( pITMIda->usStatus != ITM_STAT_ERROR )
                  {
                    /********************************************************/
                    /* do a change in the display so the user sees something*/
                    /* is happening (at least the display changed....)      */
                    /********************************************************/
                    if ( strlen( cDisp )  >= PROGRESS_LEN )
                    {
                      strcpy( cDisp, PROGRESS_STR );
                    }
                    else
                    {
                      strcat( cDisp, PROGRESS_STR );
                    } /* endif */
                  } /* endif */
                } /* endif */
              } /* endif */
              break;
          } /* endswitch */
          break;

        case WM_EQF_PROCESSTASK:
          pITMIda = ACCESSWNDIDA( hwnd, PITMIDA );
          if ( !pITMIda->fKill && (SHORTFROMMP1( mp1 ) != ITM_PROCESS_END))
          {
            USHORT  usNextTask = SHORTFROMMP1( mp1 );
            ULONG   ulAddParam = (ULONG) mp2 ;

//          if ( usOldTask != usNextTask )
//          {
//            strcpy( cDisp, PROGRESS_STR );    // reset info string...
//            INVALIDATERECT( pITMIda->hProcWnd, NULL, TRUE );
//            usOldTask = usNextTask;
//          } /* endif */

            EQFITMProcess(hwnd, pITMIda, &usNextTask, &ulAddParam );
            UtlDispatch();

            /**********************************************************/
            /* if we are not dealing with visualisation, we could     */
            /* process our next task                                  */
            /************************* ********************************/
            if ( usNextTask != ITM_PROCESS_VISUAL )
            {
              PostMessage( hwnd, WM_EQF_PROCESSTASK,
                           MP1FROMSHORT( usNextTask ),
                           ulAddParam );
            } /* endif */
          }
          else
          {
             WinPostMsg( hwnd, WM_CLOSE, NULL, NULL );
          } /* endif */
          break;

        case WM_PAINT:
          {
            PAINTSTRUCT ps;
            HDC    hdc;
            RECT   rect;

            hdc = BeginPaint(hwnd, &ps );
            GetClientRect( hwnd, &rect );
            ERASERECT( hdc, &rect, CLR_PALEGRAY );
            EndPaint(hwnd, &ps);
          }
          break;

       /***************************************************************/
       /* ATTENTION: Under Windows WM_INITMENUPOPUP will be similar   */
       /*            to WM_INITMENU, except that mp2 contains the     */
       /*            zero based position of the SubMenu (POPUP).      */
       /*            This change is taken into consideration in the   */
       /*            EQFB.ID file.                                    */
       /***************************************************************/
        case WM_INITMENUPOPUP:
        case WM_EQF_INITMENU:
            pITMIda = ACCESSWNDIDA( hwnd, PITMIDA );
            hwndMenu = GetMenu( hwnd );
            /**********************************************************/
            /* check if item is disabled or system menu selected...   */
            /*   ... ignore request...                                */
            /**********************************************************/
            if ( ( GetMenuState( hwndMenu,LOWORD( mp2 ),MF_DISABLED)
                         == MF_DISABLED ) || HIWORD(mp2) )
            {
              /********************************************************/
              /* disabled - do nothing                                */
              /********************************************************/
            }
            else
            {
              VisInitMenu(hwnd, LOWORD(mp2), pITMIda);
              mResult = WinDefWindowProc ( hwnd, msg, mp1, mp2 );
            }
            break;

        case WM_COMMAND:
            ITMCommand (hwnd, mp1) ;
            break;

        case WM_SYSCOMMAND:
            mResult = WinDefWindowProc ( hwnd, msg, mp1, mp2 );
            break;

        case WM_CLOSE:
          /************************************************************/
          /* do the close processing, i.e. inform thread it has to    */
          /* stop.                                                    */
          /* Wait until thread is finished                            */
          /************************************************************/
          pITMIda = ACCESSWNDIDA( hwnd, PITMIDA );
          /************************************************************/
          /* we are in visual  - let user decide if he wants to go on */
          /************************************************************/
          if ( pITMIda->usStatus == ITM_STAT_VISUAL )
          {
            ITMFuncQuit(pITMIda, pITMIda->hwnd, pITMIda->hab);
          } /* endif */

          /************************************************************/
          /* check what user decided....                              */
          /************************************************************/
          if ( pITMIda->usStatus != ITM_STAT_VISUAL )
          {
            pITMIda->fKill = TRUE;
            pITMIda->usStatus = ITM_STAT_CLEANUP;
            /************************************************************/
            /* start the timer - just in case it is disabled or not     */
            /* started yet....                                          */
            /************************************************************/
            if (!pITMIda->fTimer )
            {
              if (WinStartTimer (pITMIda->hab, hwnd, ITM_TIMER_ID, ITM_TIMER_VAL) )
              {
                 pITMIda->fTimer = TRUE;
              } /* endif */
            } /* endif */
            /************************************************************/
            /* force a repaint of the whole window...                   */
            /************************************************************/
            INVALIDATERECT( GETPARENT( pITMIda->hwnd ), NULL, TRUE );
          } /* endif */
            /**************************************************************/
            /* Free slider control library                                */
            /**************************************************************/
            FreeLibrary( hinstSlider );
          break;

        case WM_DESTROY:
          WinPostMsg( hwnd, WM_QUIT, NULL, NULL );
          break;

         case (HM_HELPSUBITEM_NOT_FOUND):
           {
             PHELPTABLE    pstHelpEntry;        // ptr for help table processing
             USHORT        usHelpResID;         // resource ID for help panel
             PSHORT        psSubTable;          // ptr into HelpSubTable;
             USHORT        usElementSize;       // element size of help subtable
             SHORT sTopic    = SHORT1FROMMP2(mp2);
             SHORT sSubTopic = SHORT2FROMMP2(mp2);

             pITMIda = ACCESSWNDIDA( hwnd, PITMIDA );
             /**************************************************************/
             /* IPF didn't find our window in its help table               */
             /* now we try to do IPF's work...                             */
             /**************************************************************/
             pstHelpEntry = htblMain;
             usHelpResID = 0;               // no help resid found so far

             /**************************************************************/
             /* First try: search thru the general help table              */
             /**************************************************************/
             while ( pstHelpEntry->phstHelpSubTable && !usHelpResID )
             {
                if ( pstHelpEntry->idAppWindow == (USHORT)sTopic )
                {
                   /********************************************************/
                   /* window has been spotted, now try to locate the help  */
                   /* for the subtopic                                     */
                   /********************************************************/
                   // window has been spotted, now try to locate the help
                   // for the subtopic
                   psSubTable = (PSHORT) pstHelpEntry->phstHelpSubTable;
                   usElementSize = *psSubTable++;
                   while ( *psSubTable && !usHelpResID )
                   {
                      if ( *psSubTable == sSubTopic )
                      {
                         usHelpResID = *(psSubTable+1);
                         if ( !usHelpResID )
                         {
                           // no subtopic help available
                           usHelpResID = pstHelpEntry->idExtPanel;
                         } /* endif */
                      }
                      else
                      {
                         psSubTable += usElementSize;
                      } /* endif */
                   } /* endwhile */

                   if ( !usHelpResID )      // no subtopic help found ...
                   {
                      // ... use the extended help of the window
                      usHelpResID = pstHelpEntry->idExtPanel;
                   } /* endif */
                }
                else
                {
                   pstHelpEntry++;
                } /* endif */
             } /* endwhile */

             /**************************************************************/
             /* second try: loop thru all subtables                        */
             /**************************************************************/
             pstHelpEntry = htblMain;
             while ( pstHelpEntry->phstHelpSubTable && !usHelpResID )
             {
                psSubTable = (PSHORT) pstHelpEntry->phstHelpSubTable;
                usElementSize = *psSubTable++;
                while ( *psSubTable && !usHelpResID )
                {
                   if ( (*psSubTable == sTopic) || (*psSubTable == sSubTopic) )
                   {
                      usHelpResID = *(psSubTable+1);
                   }
                   else
                   {
                      psSubTable += usElementSize;
                   } /* endif */
                } /* endwhile */
                pstHelpEntry++;             // skip to next subtable
             } /* endwhile */

             if ( usHelpResID )
             {
               if ( UtlQueryUShort( QS_CURMSGID ) == usHelpResID )
               {
                 CHAR szMsgBuf[256];      // general purpose message buffer
                 /***********************************************************/
                 /* Help for a message box ==> use secondary help window    */
                 /***********************************************************/
                 strcpy( szMsgBuf, EqfSystemHlpFile );
                 strcat( szMsgBuf, szMsgWindow );
                 WinHelp( hwnd, szMsgBuf, HELP_CONTEXT, (ULONG)usHelpResID );
               }
               else
               {
                 /***********************************************************/
                 /* normal help ==> use primary help window                 */
                 /***********************************************************/
                 WinHelp( hwnd, EqfSystemHlpFile, HELP_CONTEXT, (ULONG)usHelpResID );
               } /* endif */
               return( MRFROMSHORT( TRUE ) );
             }
             else
             {
                WinHelp( hwnd, EqfSystemHlpFile, HELP_CONTEXT, (ULONG)HID_TWBM_EXTHELP );
                return( MRFROMSHORT( FALSE ) );
             } /* endif */
           }
//           return( MRFROMSHORT(FALSE) );           // no default proc required
           break;

     case WM_MENUSELECT:
       // Keep track of currently selected menu item in case of help requests
       if ( !( mp2 & MF_POPUP ) )
       {
         UtlSetUShort( QS_CURMENUITEMID, SHORT1FROMMP1(mp1) );
       }
       else
       {
         UtlSetUShort( QS_CURMENUITEMID, 0 );
       } /* endif */
       break;

        case WM_SETFOCUS:
          /************************************************************/
          /* pass on focus request to active child window             */
          /************************************************************/
          pITMIda = ACCESSWNDIDA( hwnd, PITMIDA );
          if ( pITMIda )
          {
            SetFocus( pITMIda->hwndFocus );
          } /* endif */
          break;

     case WM_QUERYENDSESSION:
       /***************************************************************/
       /* let user decide if he wants to stop...                      */
       /***************************************************************/
       pITMIda = ACCESSWNDIDA( hwnd, PITMIDA );
       return (ITMUtlError( pITMIda, QUERY_CLOSE_TWB, MB_YESNO | MB_DEFBUTTON2,
                            0, NULL, EQF_QUERY) == MBID_YES);
       break;

     case WM_ENDSESSION:
       if ( SHORT1FROMMP1(mp1) )
       {
         EqfStopObjectManager( TWBCLOSE );
       } /* endif */
       break;
        default:
          mResult = WinDefWindowProc ( hwnd, msg, mp1, mp2 );

    } /* switch */

    return mResult ;

} /* ITMWNDPROC */

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

MRESULT APIENTRY ITMPROCESSDLG
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
        PITMIDA   pITMIda = (PITMIDA) mp2;
        SWP   swp, swpDlg;
        HWND  hwndSlider = GETHANDLEFROMID( hDlg, ID_ITM_PROCESS_SLIDER );
        SendMessage( hwndSlider, PB_SETRANGE, 100, 0 );
        SendMessage( hwndSlider, PB_SETPOS, 0, 0 );
        SendMessage( hwndSlider, PB_SETCOLOR, 0, RGB(0,0,0) );

        /****************************************************************/
        /* fill TM name into display                                    */
        /****************************************************************/
        {
          HWND hwndText;
          hwndText = GETHANDLEFROMID( hDlg, ID_ITM_PROCESS_ITMFILE_TEXT);
          WinSetWindowText( hwndText, pITMIda->chLongTranslMemory );
        }

        WinQueryWindowPos( hDlg, &swpDlg );
        WinQueryWindowPos( pITMIda->hwnd, &swp );
        swpDlg.x = swp.x + (swp.cx - swpDlg.cx)/2;
        swpDlg.y = swp.y + (swp.cy - swpDlg.cy)/2;

        // position the dialog window -- if minimized size it to one bit
        WinSetWindowPos( hDlg, HWND_TOP, swpDlg.x, swpDlg.y, 0, 0,
                        (USHORT)(EQF_SWP_MOVE | (pITMIda->fMinimized ? EQF_SWP_SIZE : 0)));

        pITMIda->hProcWnd = hDlg;
        ANCHORDLGIDA( hDlg, mp2 );
      }
      break;

    case WM_EQF_CLOSE:
//    DISMISSDLG( hDlg, SHORT1FROMMP2(mp2) );
      WinDestroyWindow( hDlg );
      break;
    case WM_COMMAND:
      if ( WMCOMMANDID( mp1, mp2 ) == DID_CANCEL )
      {
        PITMIDA pIda = ACCESSDLGIDA(hDlg, PITMIDA);
        if ( ITMUtlError( pIda, ERROR_CANCELTA,
                          MB_YESNO, 0, NULL,
                          EQF_QUERY ) == MBID_YES )
        {
          /**********************************************************/
          /* Setting fKill to TRUE will terminate the analysis      */
          /* process at the next possible step                      */
          /**********************************************************/
          *(pIda->pfKillAnalysis) = TRUE;
          POSTEQFCLOSE( hDlg, FALSE );
        } /* endif */

        mResult = MRFROMSHORT(TRUE); // TRUE = command is processed
      }
      else
      {
        mResult = WinDefDlgProc( hDlg, msg, mp1, mp2 );
      } /* endif */
      break;
    default:
      mResult = WinDefDlgProc( hDlg, msg, mp1, mp2 );
      break;
  } /* endswitch */
  return ( mResult );
}
//------------------------------------------------------------------------------
// Internal function                                                            
//------------------------------------------------------------------------------
// Function name:     BOOL GetNextFilePair (HWND, PITMIDA)                      
//------------------------------------------------------------------------------
// Function call:     _                                                         
//------------------------------------------------------------------------------
// Description:       _                                                         
//------------------------------------------------------------------------------
// Parameters:        _                                                         
//------------------------------------------------------------------------------
// Returncode type:   _                                                         
//------------------------------------------------------------------------------
// Returncodes:       _                                                         
//------------------------------------------------------------------------------
// Function flow:     _                                                         
//------------------------------------------------------------------------------

BOOL
GetNextFilePair
(
    HWND      hwnd,
    PITMIDA   pITMIda
)
{
  BOOL      fOK = TRUE;
  PSZ       pTempStatus;                  // pointer to temporary status
  USHORT    usRc;

  if (pITMIda->szSrcStartPath[0] == EOS )
  {
    /****************************************************/
    /* get source and target file name into provided    */
    /* buffers                                          */
    /****************************************************/
    fOK = UtlCopyParameter(pITMIda->chSourceFile,
                           *(pITMIda->ppArgv),
                           sizeof(pITMIda->chSourceFile),
                           FALSE);
    if ( fOK )
    {
      pITMIda->ppArgv ++;
      fOK = UtlCopyParameter(pITMIda->chTargetFile,
                             *(pITMIda->ppArgv),
                             sizeof(pITMIda->chTargetFile),
                             FALSE);
    } /* endif */
  }
  else
  {

    fOK = EQFBITMAddStartToRelPath(pITMIda->chSourceFile,
                                   pITMIda->szSrcStartPath,
                                   *(pITMIda->ppArgv),
                                   sizeof(pITMIda->chSourceFile));
    if (fOK )
    {
      pITMIda->pInFile = &(pITMIda->chSourceFile[strlen(pITMIda->szSrcStartPath)]);
      pITMIda->ppArgv ++;

      fOK = EQFBITMAddStartToRelPath(pITMIda->chTargetFile,
                                     pITMIda->szTgtStartPath,
                                     *(pITMIda->ppArgv),
                                     sizeof(pITMIda->chTargetFile));
    } /* endif */
  } /* endif */

  /****************************************************/
  /* if error happened stop the process else go ahead */
  /* with processing                                  */
  /****************************************************/
  if ( !fOK )
  {
    /************************************************/
    /* end of processing                            */
    /************************************************/
    if (pITMIda->fTimer )
    {
      WinStopTimer( pITMIda->hab, hwnd, ITM_TIMER_ID );
      pITMIda->fTimer = FALSE;
    } /* endif */
    pTempStatus = *(pITMIda->ppArgv);
    pITMIda->fKill = TRUE;
    ITMUtlError( pITMIda, UTL_PARAM_TOO_LONG, MB_CANCEL,
                 1, &pTempStatus, EQF_ERROR );
    if (hwnd != HWND_FUNCIF)
    {
      WinPostMsg( hwnd, WM_CLOSE, NULL, NULL );
    }
    if (!pITMIda->fTimer )
    {
      if (WinStartTimer (pITMIda->hab, hwnd, ITM_TIMER_ID, ITM_TIMER_VAL) )
      {
         pITMIda->fTimer = TRUE;
      } /* endif */
    } /* endif */
  }
  else
  {
    HWND hwndText;

    pITMIda->ppArgv ++;

    hwndText = GETHANDLEFROMID( pITMIda->hProcWnd, ID_ITM_PROCESS_SOURCEFILE_TEXT);
    WinSetWindowText( hwndText, pITMIda->chSourceFile );

    hwndText = GETHANDLEFROMID( pITMIda->hProcWnd, ID_ITM_PROCESS_TARGETFILE_TEXT);
    WinSetWindowText( hwndText, pITMIda->chTargetFile );

    /********************************************************/
    /* extract pointer to segmented source file             */
    /* (need in translation memory)                         */
    /* if startpath exists, pInFile is filled already       */
    /********************************************************/

    if (pITMIda->szSrcStartPath[0] == EOS )
    {
      pITMIda->pInFile = strrchr( pITMIda->chSourceFile,
                                  BACKSLASH );
      if ( pITMIda->pInFile )
      {
        pITMIda->pInFile ++;
      }
      else
      {
        pITMIda->pInFile = strrchr( pITMIda->chSourceFile,
                                    COLON );
        if ( pITMIda->pInFile  )
        {
          pITMIda->pInFile ++;
        }
        else
        {
          pITMIda->pInFile = pITMIda->chSourceFile;
        } /* endif */
      } /* endif */
    } /* endif */
  } /* endif */

  /****************************************************/
  /* check if source and target files are the same ?? */
  /****************************************************/
  if ( fOK )
  {
    if ( !strcmp(pITMIda->chSourceFile,pITMIda->chTargetFile))
    {
      /**************************************************/
      /* display warning and ignore file                */
      /**************************************************/
      if (pITMIda->fTimer )
      {
        WinStopTimer( pITMIda->hab, hwnd, ITM_TIMER_ID );
        pITMIda->fTimer = FALSE;
      } /* endif */
      pTempStatus = pITMIda->chSourceFile;
      usRc = ITMUtlError( pITMIda, ITM_FILENAMES_ARE_EQUAL, MB_YESNO,
                          1, &pTempStatus, EQF_ERROR );
      if ( usRc == MBID_NO )
      {
        pITMIda->fKill = TRUE;
        fOK = FALSE;
        /************************************************/
        /* end of processing                            */
        /************************************************/
        WinPostMsg( hwnd, WM_CLOSE, NULL, NULL );
      } /* endif */
      if (!pITMIda->fTimer )
      {
        if (WinStartTimer (pITMIda->hab, hwnd, ITM_TIMER_ID, ITM_TIMER_VAL) )
        {
           pITMIda->fTimer = TRUE;
        } /* endif */
      } /* endif */
    }
    else
    {
      WinShowWindow( pITMIda->hProcWnd, TRUE );
      pITMIda->fBusy = TRUE;
    } /* endif */

    pITMIda->usArgc -= 2;
    /******************************************************************/
    /* why not  check PROGRESS_LEN???                                 */
    /******************************************************************/
    strcpy( cDisp, PROGRESS_STR );
  } /* endif */

  return (fOK);
} /* end of function BOOL GetNextFilePair */


BOOL
EQFBITMAddStartToRelPath
(
  PSZ      pTarget,
  PSZ      pStartPath,
  PSZ      pRelPath,
  LONG     lMaxTgtLen
)
{
    LONG     lStartPathLen = 0;
    LONG     lMaxRelLen = 0;
    BOOL     fOK = TRUE;

    lStartPathLen = strlen(pStartPath);
    lMaxRelLen = lMaxTgtLen - lStartPathLen;

    strcpy(pTarget, pStartPath );

    if ( lStartPathLen &&
         (pTarget[lStartPathLen-1] != BACKSLASH) &&
         (pRelPath[0] != BACKSLASH )  )
    {
      strcat( pTarget, BACKSLASH_STR );
    } /* endif */

    if ((LONG)strlen(pRelPath) < lMaxRelLen )
    {
      strcat(pTarget,pRelPath);
    }
    else
    {
      fOK = FALSE;
    } /* endif */

  return (fOK);
} /* end of function EQFBITMAddStartToRelPath  */

//------------------------------------------------------------------------------
// Internal function                                                            
//------------------------------------------------------------------------------
// Function name:     ITMComplete (hwnd, pITMIda)                               
//------------------------------------------------------------------------------
// Function call:     _                                                         
//------------------------------------------------------------------------------
// Description:       _                                                         
//------------------------------------------------------------------------------
// Parameters:        HWND   hwnd                                               
//                    PITMIDA  pITMIda                                          
//------------------------------------------------------------------------------
// Returncode type:   none                                                      
//------------------------------------------------------------------------------
// Function flow:     _                                                         
//------------------------------------------------------------------------------
VOID
ITMComplete
(
   HWND       hwnd,
   PITMIDA    pITMIda
)
{
  PSZ    pData[3];                   // pointer to completion data strings

  if ( ! pITMIda->fNoConfirm )
  {
    if (pITMIda->fTimer )
    {
      WinStopTimer( pITMIda->hab, hwnd, ITM_TIMER_ID );
      pITMIda->fTimer = FALSE;
    } /* endif */
    if ( pITMIda->fPrepare )
    {
      ITMUtlError( pITMIda, ITM_SAVETOALI, MB_OK, 0, NULL, EQF_INFO);
    }
    else
    {
      if ( !pITMIda->fVisual )
      {
        /****************************************************/
        /* display completion message                       */
        /****************************************************/
        pData[0] = ltoa (pITMIda->ulSegTotal,
                         pITMIda->szBuffer, 10 );
        pData[1] = ltoa (pITMIda->ulSegAligned,
                         (pITMIda->szBuffer+MAX_SEGMENT_SIZE), 10 );
        ITMUtlError( pITMIda, ITM_COMPLETE, MB_OK,
                     2, &pData[0], EQF_INFO );
      } /* endif */
    } /* endif */

    if (!pITMIda->fTimer )
    {
      if (WinStartTimer (pITMIda->hab, hwnd, ITM_TIMER_ID, ITM_TIMER_VAL) )
      {
         pITMIda->fTimer = TRUE;
      } /* endif */
    } /* endif */
  }
  else
  {
    pITMIda->usRC = ITM_COMPLETE;
  } /* endif */

  return;
} /* end of function ITMComplete (hwnd, pITMIda) */

//------------------------------------------------------------------------------
// Internal function                                                            
//------------------------------------------------------------------------------
// Function name:     ITMInit                                                   
//------------------------------------------------------------------------------
// Function call:     ITMInit( pITMIda );                                       
//------------------------------------------------------------------------------
// Description:       This function will try to load the internal tag tables    
//                    and start the processing thread                           
//------------------------------------------------------------------------------
// Parameters:        PITMIDA  pITMIda   pointer to instance data area          
//------------------------------------------------------------------------------
// Returncode type:   VOID                                                      
//------------------------------------------------------------------------------
// Function flow:     set the internal tag table name                           
//                    if not windows                                            
//                      try to start processing thread                          
//                    else                                                      
//                      post msg WM_EQF_PROCESSTASK                             
//------------------------------------------------------------------------------

BOOL ITMInit
(
  PITMIDA pITMIda
)
{
  BOOL    fOK = TRUE;                  // success indicator
  CHAR    szDrive[ MAX_DRIVE ];        // drives

  /********************************************************************/
  /* set  up the internal tagtable name..                             */
  /********************************************************************/
  strcpy(pITMIda->chQFTagTable, ITM_QFTAGTABLE);
  UtlQueryString(QST_PRIMARYDRIVE, szDrive, MAX_DRIVE) ;
  pITMIda->chQFTagTable[0] = szDrive[0];
  /********************************************************************/
  /* start the processing thread ....                                 */
  /********************************************************************/
  PostMessage( pITMIda->hwnd, WM_EQF_PROCESSTASK, ITM_PROCESS_TMOPEN, 0L );
  return ( fOK );
} /* end of ITMInit */

//------------------------------------------------------------------------------
// Internal function                                                            
//------------------------------------------------------------------------------
// Function name:     ListOfFiles                                               
//------------------------------------------------------------------------------
// Function call:     ListOfFiles( pITMIda, pFile, ppListIndex );               
//------------------------------------------------------------------------------
// Description:       read in the file and try to extract pointers to filenames 
//------------------------------------------------------------------------------
// Parameters:        PITMIDA pITMIda   pointer to ida                          
//                    PSZ     pFile     pointer to file name                    
//                    PSZ     * ppListIndex  pointer to array of files          
//------------------------------------------------------------------------------
// Returncode type:   BOOL                                                      
//------------------------------------------------------------------------------
// Returncodes:       TRUE     file could be read in and pointers extracted     
//                    FALSE    something went wrong - error message  displayed  
//------------------------------------------------------------------------------
// Side Effects:      the allocated memory will not be freed, since only ptrs   
//                    to it will be set up...                                   
//------------------------------------------------------------------------------
// Function flow:     try to load the specified file                            
//                    if okay                                                   
//                      split it up into pointers                               
//                    endif                                                     
//------------------------------------------------------------------------------

static BOOL
ListOfFiles
(
  PITMIDA pITMIda,                  // ptr to ida
  PSZ pFile,                        // name of list file
  PSZ  **pppListIndex               // pointer to listindex array
)
{
  PSZ  pData = NULL;                // pointer to data
  BOOL fOK;                         // success indicator
  USHORT  usSize;                   // size of the file

  /********************************************************************/
  /* load the file - limited to files up to 64k                       */
  /********************************************************************/
  fOK = UtlLoadFile( pFile, (PVOID *)&pData, &usSize, FALSE, TRUE );

  if ( fOK )
  {
    /******************************************************************/
    /* get rid of the file ending symbols ....                        */
    /******************************************************************/
    if ( pData[usSize-1] == EOFCHAR )
    {
      usSize--;
    } /* endif */

    if ( (pData[usSize-1] == LF) && (pData[usSize-2] == CR) )
    {
      usSize -= 2;
    } /* endif */
    pData[usSize] = EOS;

    fOK = UtlValidateList(pData, pppListIndex, 2000 );
    if (!fOK )
    {
                ITMUtlError( pITMIda, ITM_WRONGCMDLINE, MB_CANCEL, 1, &pData, EQF_ERROR );
    }
  } /* endif */

  return ( fOK );
} /* end of function ListOfFiles */

//------------------------------------------------------------------------------
// Internal function                                                            
//------------------------------------------------------------------------------
// Function name:     CheckCmdLine                                              
//------------------------------------------------------------------------------
// Function call:     CheckCmdLine( usArgc, &pArgv, pITMIda );                  
//------------------------------------------------------------------------------
// Description:       get and analyze the commandline                           
//------------------------------------------------------------------------------
// Parameters:        USHORT  usArgc    number of commandline arguments         
//                    PSZ     *ppArgv   pointer to commandline parameters       
//                    PITMIDA pITMIda   pointer to instance data                
//------------------------------------------------------------------------------
// Returncode type:   BOOL                                                      
//------------------------------------------------------------------------------
// Returncodes:       TRUE   commandline is correct                             
//                    FALSE  commandline has an error in it                     
//------------------------------------------------------------------------------
// Function flow:     allocate space to copy command line (4k is large enough)  
//                    if okay                                                   
//                      copy the commandline parameters together                
//                      uppercase it and  add a delimiting slash (makes it      
//                        easier to analyze later on)                           
//                    endif                                                     
//                    if okay                                                   
//                      tokenize it, i.e.                                       
//                     while not end of string                                  
//                      case DELSLASH:                                          
//                        Validate the token and set the appropriate flags and  
//                          parameters                                          
//                        switch (token)                                        
//                        case BATCH_MEM: copy translation memory name          
//                        case BATCH_SGMLMEM: copy name & set indicator         
//                        case BATCH_LEVEL: copy level                          
//                        case BATCH_MARKUP: copy tagtable name                 
//      in windows only:                                                        
//                        case BATCH_TGTLNG: copy target language               
//                                           if lang not ok, issue error        
//                        case BATCH_SRCLNG: copy source language               
//                                           if lang not ok, issue error        
//      end of windows only                                                     
//                        case BATCH_FILES: if Listfile, read it in             
//                                          else work against pair of files     
//                                          if ok eliminate double filepairs    
//                                               check that even num is given   
//                                             fill property filepairlist       
//                        case BATCH_TYPE: validate type specifications         
//                                         issue error if error                 
//                        case BATCH_HWND: use hwnd instead of DESKTOP          
//                              (nec if called from WP, 21.5.95)                
//                        endswitch                                             
//                      default:                                                
//                        point to next character                               
//                     endwhile                                                 
//                     if okay                                                  
//                       do a consistency check                                 
//                     endif                                                    
//                     free the commandline previously allocated                
//                   else                                                       
//                     wrong command line                                       
//                   endif                                                      
//                   return success indicator                                   
//------------------------------------------------------------------------------

static BOOL
CheckCmdLine
(
  USHORT usArgc,                      // number of commandline arguments
  PSZ    * ppArgv,                    // pointer to string array
  PITMIDA  pITMIda                    // pointer to ida to be filled
)
{
  BOOL  fOK;                          // success indicator
  PSZ   pCmdLine;                     // commandline buffer
  PSZ   pStart;                       // pointer to start of current token
  PSZ   pActive;                      // pointer to active position
  CHAR  c;                            // active character
  BYTE  bFlags = 0;                   // found flags
  PSZ   *ppListIndex;                 // pointer to listindex array
  PSZ   pList[2];                     // list pointer ...
  USHORT usI;                         // pointer in list index array
  BATCHCMD  BatchCmd;                  // batch command
  BOOL      fSrcStartPath = FALSE;
  BOOL      fTgtStartPath = FALSE;
  BOOL      fMFlag = FALSE;

  usArgc--;
  ppArgv++;                           // skip name of exe
  if ( usArgc )
  {

    fOK = UtlAlloc( (PVOID *) &pCmdLine, 0L, 4096L, ERROR_STORAGE);

    pITMIda->szSrcStartPath[0] = EOS;      // default if none is used
    /******************************************************************/
    /* combine the commandline                                        */
    /******************************************************************/
    if ( fOK )
    {
      while ( usArgc )
      {
        strcat( pCmdLine, *ppArgv );
        ppArgv++;
        usArgc--;
      } /* endwhile */
      /****************************************************************/
      /* append '/' to allow for easier processing of last token.     */
      /****************************************************************/
      strcat( pCmdLine, DELSLASH_STR );
    } /* endif */
    /******************************************************************/
    /* first run thru and check for QUIET attribute                   */
    /******************************************************************/
    if (fOK )
    {
      BOOL   fQuietFound = FALSE;
      pStart = pCmdLine;
      pActive = pCmdLine;
      while ( ((c=*pActive)!=0) && fOK && !fQuietFound )
      {
        if ((c == DELSLASH) && (pStart != pActive) )
        {
          *pActive = EOS;
          if (ValidateToken( &pStart, ITMCmdList ) == BATCH_QUIET)
          {
            pITMIda->fQuiet = TRUE;
            pITMIda->fNoConfirm = TRUE;
            fQuietFound = TRUE;
          }
          *pActive = DELSLASH;
          pStart = pActive;
        }
        else
        {
          pActive++;
        } /* endif */
      } /* endwhile */
    } /* endif */
    /******************************************************************/
    /* tokenize and analyze it ...                                    */
    /******************************************************************/
    if ( fOK )
    {
      pStart = pCmdLine;
      pActive = pCmdLine;
      while ( ((c=*pActive)!=0) && fOK  )
      {
        switch ( c )
        {
          case DELSLASH:
            if ( pStart != pActive )
            {
              *pActive = EOS;

              switch ( ValidateToken( &pStart, ITMCmdList ) )
              {
                case  BATCH_MEM:                // translation memory
                  bFlags |= B_MEM;
                  fOK = UtlCopyParameter(pITMIda->chTranslMemory,
                                         pStart,
                                         MAX_LONGFILESPEC - 1,
                                         TRUE);
                  strcpy( pITMIda->chLongTranslMemory, pITMIda->chTranslMemory );

                  break;
                case  BATCH_HWND:               // use hwnd instead of DESKTOP
                  pITMIda->hwndParent = (HWND) atol(pStart);
                  break;
                case  BATCH_SGMLMEM:            // translation memory in SGML..
                  fOK = UtlCopyParameter(pITMIda->chSGMLMem,
                                         pStart,
                                         sizeof(pITMIda->chSGMLMem),
                                         TRUE);
                  pITMIda->fSGMLITM = TRUE;
                  break;
                case  BATCH_LEVEL:              // level of match
                  pITMIda->usLevel = (USHORT)(atoi(pStart));
                  break;
                case  BATCH_MARKUP:             // tag table
                  strupr( pStart );
                  fOK = UtlCopyParameter(pITMIda->chTagTableName,
                                         pStart,
                                         MAX_FNAME,
                                         TRUE);
                  bFlags |= B_MARKUP;
                  break;

                  case BATCH_TGTLNG:
                  {
                    CHAR  chLanguage[MAX_LANGUAGE_PROPERTIES];
                    strupr( pStart );
                    fOK = UtlCopyParameter(pITMIda->szTargetLang,
                                         pStart,
                                         sizeof(pITMIda->szTargetLang),
                                         TRUE);
                    strupr( pStart );
                    fOK = UtlCopyParameter(pITMIda->szTargetInputLang,
                                         pStart,
                                         sizeof(pITMIda->szTargetInputLang),
                                         TRUE);
                    if (fOK )
                    {
                      bFlags |= B_TGTLNG;
                    /****************************************************/
                    /* check that language is ok                        */
                    /****************************************************/
                      strcpy(chLanguage, pITMIda->szTargetLang);
                      if (!isValidLanguage( chLanguage, FALSE ))
                      {
                        ITMUtlError( pITMIda, ITM_LING_SUPPORT_MISSING, MB_CANCEL,
                                     1, &pStart, EQF_ERROR );
                        fOK = FALSE;
                      }
                    }
                  }
                  break;
                case  BATCH_SRCLNG:
                  {
                    CHAR  chLanguage[MAX_LANGUAGE_PROPERTIES];
                    strupr( pStart );
                    fOK = UtlCopyParameter(pITMIda->szSourceLang,
                                           pStart,
                                           sizeof (pITMIda->szSourceLang),
                                           TRUE);
                    /****************************************************/
                    /* check that language is ok                        */
                    /****************************************************/
                    if (fOK )
                    {
                      strcpy(chLanguage, pITMIda->szSourceLang);
                      if (!isValidLanguage( chLanguage, FALSE ))
                      {
                        ITMUtlError( pITMIda, ITM_LING_SUPPORT_MISSING, MB_CANCEL,
                                     1, &pStart, EQF_ERROR );
                        fOK = FALSE;
                      }
                    }
                  }
                  break;

                case  BATCH_FILES:
                  bFlags |= B_FILES;
                  if ( *pStart == LISTINDICATOR )
                  {
                    /**************************************************/
                    /* prepare the list ...                           */
                    /**************************************************/
                    fOK = ListOfFiles( pITMIda, (pStart+1), &ppListIndex );
                  }
                  else
                  {
                    /**************************************************/
                    /* work against pair of files                     */
                    /**************************************************/
                    fOK = UtlValidateList( pStart, &ppListIndex, 2000 );
                    if (!fOK )
                    {
                       ITMUtlError( pITMIda, ITM_WRONGCMDLINE,
                                    MB_CANCEL, 1, &pStart, EQF_ERROR );
                    }
                  } /* endif */
                  if ( fOK )
                  {
                    /**************************************************/
                    /* set parameters and check that a even number is */
                    /* available, else somewhere a correspondent part */
                    /* of a file is missing                           */
                    /**************************************************/
                    CheckDoubleFilePairs (&ppListIndex);
                    usI = 0;
                    while ( ppListIndex[usI] )
                    {
                      usI++;
                    } /* endwhile */
                    if ( (usI % 2) )
                    {
                      fOK = FALSE;
                      ITMUtlError( pITMIda, ITM_NUMOFFILES, MB_CANCEL,
                                   0, NULL, EQF_ERROR );
                      UtlAlloc( (PVOID *)&ppListIndex, 0L, 0L, NOMSG );
                    }
                    else
                    {
                      pITMIda->ppArgv = ppListIndex;
                      pITMIda->usArgc = usI;
                      fOK = FillPropFilePairs(pITMIda);
                      if ( fOK )
                      {
                        pITMIda->pstPropItm->szSrcDirectory[0] = EOS;
                        pITMIda->pstPropItm->szTgtDirectory[0] = EOS;
                      } /* endif */
                    } /* endif */
                  } /* endif */
                  break;

                case  BATCH_SRCSTARTPATH:
                  fOK = UtlCopyParameter(pITMIda->szSrcStartPath,
                                         pStart,
                                         sizeof(pITMIda->szSrcStartPath),
                                         TRUE);
                  fSrcStartPath = TRUE;
                  break;
                case  BATCH_TGTSTARTPATH:
                  fOK = UtlCopyParameter(pITMIda->szTgtStartPath,
                                         pStart,
                                         sizeof(pITMIda->szTgtStartPath),
                                         TRUE);
                  fTgtStartPath = TRUE;
                  break;

#ifdef RAS400_ITM
                case BATCH_QUALITYLEVEL:
                  pITMIda->sMPerCent = (SHORT)(atoi(pStart));
                  if ((pITMIda->sMPerCent > 101) ||
                      (pITMIda->sMPerCent < 0) )
                  {
                    pITMIda->sMPerCent = -1;
                  } /* endif */
                  fMFlag = TRUE;
                  break;
#endif
                case BATCH_SGMLFORMAT:
                   {
                       CHAR  chTemp[256];
                       strupr( pStart );
                       fOK = UtlCopyParameter(chTemp,
                                             pStart,
                                             sizeof(chTemp),
                                             TRUE);

                       if (strcmp(chTemp, FORMAT_UNICODE ) == 0)
                       {
                           pITMIda->usSGMLFormat = SGMLFORMAT_UNICODE;
                           pITMIda->ulSGMLFormatCP = 0L;
                           pITMIda->ulAnsiCP = 0L;
                       }
                       else if ( strcmp(chTemp, FORMAT_ASCII) == 0 )
                       {
                           pITMIda->usSGMLFormat = SGMLFORMAT_ASCII;
                           pITMIda->ulSGMLFormatCP = GetLangOEMCP(NULL);
                           pITMIda->ulAnsiCP = GetLangAnsiCP(NULL);
                       }
                       else if ( strcmp(chTemp, FORMAT_ANSI) == 0)
                       {
                           pITMIda->usSGMLFormat = SGMLFORMAT_ANSI;
                           pITMIda->ulSGMLFormatCP = GetLangOEMCP(NULL);
                           pITMIda->ulAnsiCP = GetLangAnsiCP(NULL);
                       }
                       else
                       {
                           pITMIda->usSGMLFormat = SGMLFORMAT_UNICODE;
                           pITMIda->ulSGMLFormatCP = 0L;
                           pITMIda->ulAnsiCP = 0L;
                           fOK = FALSE;
                           ITMUtlError( pITMIda, ITM_WRONGCMDLINE,
                                            MB_CANCEL, 1, &pStart, EQF_ERROR );
                       }
                   }
                   break;
                case  BATCH_TYPE:
                  /****************************************************/
                  /* check if special type is specified               */
                  /****************************************************/
                  if ( *pStart == LISTSTART )
                  {
                    /**************************************************/
                    /* work against list of options ...               */
                    /**************************************************/

                    fOK = UtlValidateList( pStart, &ppListIndex, 2000  );
                    if (!fOK )
                    {
                       ITMUtlError( pITMIda, ITM_WRONGCMDLINE,
                                 MB_CANCEL, 1, &pStart, EQF_ERROR );
                    }
                  }
                  else
                  {
                    pList[0] = pStart;
                    pList[1] = NULL;
                    ppListIndex = &pList[0];
                  } /* endif */
                  while ( *ppListIndex && fOK )
                  {
                    pStart = *ppListIndex;
                    ppListIndex++;
                    BatchCmd = ValidateToken( &pStart, ITMCmdListTypes );
                    if ( pStart && *pStart )
                    {
                       ITMUtlError( pITMIda, ITM_WRONGCMDLINE, MB_CANCEL,
                                    1, &pStart, EQF_ERROR );
                       fOK = FALSE;
                    }
                    else
                    {
                      switch ( BatchCmd )
                      {
                        case  BATCH_NOANA:
                          pITMIda->fNoAna = TRUE;   // markup is nec.too!
                          break;
                        case  BATCH_NOTMDB:
                          pITMIda->fNoTMDB = TRUE;
                          break;
                        case  BATCH_NOCONF:
                          pITMIda->fNoConfirm = TRUE;
                          break;

                        case  BATCH_VISUAL:
                          pITMIda->fVisual = TRUE;
                          break;
                        case  BATCH_PREPARE:
                          pITMIda->fPrepare = TRUE;
                          break;
                        case  BATCH_ICON:
                          pITMIda->fMinimized = TRUE;
                          break;
                        default :
                          ITMUtlError( pITMIda, ITM_WRONGCMDLINE, MB_CANCEL,
                                       1, &pStart, EQF_ERROR );
                          fOK = FALSE;
                          break;
                      } /* endswitch */
                      if ( pITMIda->fVisual && pITMIda->fPrepare )
                      {
                        fOK = FALSE;
                        ITMUtlError( pITMIda, ITM_WRONGCMDLINE, MB_CANCEL,
                                     1, &pStart, EQF_ERROR );
                      } /* endif */
                    } /* endif */

                  } /* endwhile */
                  break;
                case  BATCH_QUIET:
                  pITMIda->fQuiet = TRUE;
                  pITMIda->fNoConfirm = TRUE;
                  break;

                default :
                  fOK = FALSE;
                  ITMUtlError( pITMIda, ITM_WRONGCMDLINE, MB_CANCEL,
                               1, &pStart, EQF_ERROR );
                  break;
              } /* endswitch */
              *pActive = DELSLASH;
              pStart = pActive;
            }
            else
            {
              pActive++;
            } /* endif */
            break;
          default :
            pActive++;
            break;
        } /* endswitch */
      } /* endwhile */
      /****************************************************************/
      /* do a consistency check                                       */
      /****************************************************************/
      if ( fOK )
      {
        fOK = EQFITMPropFill (pITMIda );
        if (fOK &&
            (bFlags != (B_MEM | B_FILES | B_MARKUP | B_TGTLNG)) ||
              ( pITMIda->fNoTMDB && !pITMIda->fSGMLITM ) )
        {
          fOK = FALSE;
          if (pITMIda->hwndParent == HWND_DESKTOP )
          {
            ITMUtlError( pITMIda, ITM_MANDCMDLINE, MB_CANCEL,
                         0, NULL, EQF_ERROR );
          } /* endif */
        } /* endif */
        if (fOK && (fSrcStartPath != fTgtStartPath) )
        {
          fOK = FALSE;
          ITMUtlError(pITMIda, ITM_STARTPATHNOTEQUAL, MB_CANCEL,
                      0, NULL, EQF_ERROR);
        } /* endif */
#ifdef RAS400_ITM
        if (!fMFlag )
        {
          pITMIda->sMPerCent = -1;
        } /* endif */
#endif
      } /* endif */

    } /* endif */
    /******************************************************************/
    /* the commandline buffer will be freed at the end of the program */
    /******************************************************************/
  }
  else
  {
    /******************************************************************/
    /* no commandline parameter passed ...                            */
    /******************************************************************/
    ITMUtlError( pITMIda, ITM_EMPTYCMDLINE, MB_CANCEL, 0, NULL,EQF_ERROR );
    fOK = FALSE;
  } /* endif */

  return( fOK );
} /* end of function CheckCmdLine */

//------------------------------------------------------------------------------
// Internal function                                                            
//------------------------------------------------------------------------------
// Function name:     ValidateToken                                             
//------------------------------------------------------------------------------
// Function call:     ValidateToken( ppToken, pCmdList );                       
//------------------------------------------------------------------------------
// Description:       loop through list of passed command list and try to find  
//                    matching token; return address to datapart of this token  
//------------------------------------------------------------------------------
// Parameters:        PSZ *ppToken    pointer to token                          
//                    PCMDLIST pCmdList  pointer to avail. command list         
//------------------------------------------------------------------------------
// Returncode type:   BATCHCMD                                                  
//------------------------------------------------------------------------------
// Returncodes:       matching command line parameter or end of list            
//------------------------------------------------------------------------------
// Function flow:     get pointer to data                                       
//                    while not end of available commands                       
//                      compare data with command string                        
//                      if match found                                          
//                        set token pointer to data part of command             
//                      else                                                    
//                        increase list pointer                                 
//                      endif                                                   
//                    endwhile                                                  
//                    return matching command                                   
//------------------------------------------------------------------------------

static BATCHCMD
ValidateToken
(
  PSZ  *ppToken,                     // token to be analysed
  PCMDLIST  pCmdList                // pointer to command list
)
{
  BOOL fFound = FALSE;              // success indicator
  PSZ  pData = *ppToken;
  PSZ  pszTempDesc;
  ULONG ulTempLen = 0;
  CHAR    chChar;
  PCMDLIST  pStartCmdList;

  pStartCmdList = pCmdList;
  /********************************************************************/
  /* Note: we can loop through commands without the need of any fancy */
  /*       search algorithm because there are not as many commands and*/
  /*       we do it only once!                                        */
  /********************************************************************/
  while ( !fFound && pCmdList->BatchCmd != BATCH_END)
  {
    pszTempDesc = pCmdList->szShortCut;
    ulTempLen = strlen(pszTempDesc);
    chChar = *(pszTempDesc + ulTempLen - 1);
    if (chChar == '=' )
    {
      if ( !strnicmp(pszTempDesc, pData, ulTempLen) )
      {
        fFound = TRUE;
        *ppToken += ulTempLen;
      }
      else
      {
        pCmdList++;
      } /* endif */
    }
    else
    {
      chChar = *(pData + ulTempLen);
      if (((chChar == ' ') || ( strlen(pData) == ulTempLen ) )
          && !strnicmp(pszTempDesc, pData, ulTempLen))
      {
         fFound = TRUE;
         *ppToken += ulTempLen;
      }
      else
      {
        pCmdList++;
      } /* endif */
    } /* endif */
  } /* endwhile */
  if (!fFound )
  {
    pCmdList = pStartCmdList;
  } /* endif */
  while ( !fFound && pCmdList->BatchCmd != BATCH_END)
  {
    pszTempDesc = pCmdList->szDesc;
    ulTempLen = strlen(pszTempDesc);
    chChar = *(pszTempDesc + ulTempLen - 1);
    if (chChar == '=' )
    {
      if ( !strnicmp(pszTempDesc, pData, ulTempLen) )
      {
        fFound = TRUE;
        *ppToken += strlen(pszTempDesc);
      }
      else
      {
        pCmdList++;
      } /* endif */
    }
    else
    {
      chChar = *(pData + ulTempLen);
      if (((chChar == ' ') || ( strlen(pData) == ulTempLen ) )
          && !strnicmp(pszTempDesc, pData, ulTempLen))
      {
         fFound = TRUE;
         *ppToken += strlen(pszTempDesc);
      }
      else
      {
        pCmdList++;
      } /* endif */
    } /* endif */
  } /* endwhile */
  return( pCmdList->BatchCmd );
} /* end of function ValidateToken */

//------------------------------------------------------------------------------
// Internal function                                                            
//------------------------------------------------------------------------------
// Function name:     VisInitMenu                                               
//------------------------------------------------------------------------------
// Function call:     _                                                         
//------------------------------------------------------------------------------
// Description:       init menu bar                                             
//------------------------------------------------------------------------------
// Parameters:        HWND       hwnd,                                          
//                    WPARAM      mp1,                                          
//                    PITMIDA    pITMIda                                        
//------------------------------------------------------------------------------
// Returncode type:   VOID                                                      
//------------------------------------------------------------------------------
// Returncodes:       _                                                         
//------------------------------------------------------------------------------
// Function flow:     enable / disable menuitems according to                   
//                    current alignment                                         
//------------------------------------------------------------------------------
static
VOID VisInitMenu
(
  HWND       hwnd,
  USHORT     usId,                                     // message parameter
  PITMIDA    pITMIda
)
{
   HMENU       hwndMenu;                                   // handle of action bar
   PITMVISDOC  pVisDoc;
   PTBDOCUMENT pDoc;
   PTBSEGMENT  pSeg;
   BOOL        fActive = TRUE;

   /*******************************************************************/
   /* Attention: AAB now anchored at TWB main window....              */
   /*******************************************************************/
   hwndMenu = GetMenu( hwnd );

   switch (usId)
   {
      case IDM_ITM_FILE_MENU:
           SETAABITEM( hwndMenu, IDM_ITMQUIT , TRUE );
           SETAABITEM( hwndMenu, IDM_ENDSAVE, TRUE );
           SETAABITEM( hwndMenu, IDM_CONTINUE, TRUE );
           SETAABITEM( hwndMenu, IDM_TOGGLE, TRUE );
        break;

      case IDM_ALIGN_MENU:
           SETAABITEM( hwndMenu, IDM_CONNECT, TRUE );
           SETAABITEM( hwndMenu, IDM_DELCONNECT, TRUE );
           SETAABITEM( hwndMenu, IDM_DELUSERCON, TRUE );
           SETAABITEM( hwndMenu, IDM_CROSSOUT, TRUE );
           SETAABITEM( hwndMenu, IDM_UNDOCROSSOUT, TRUE );
           SETAABITEM( hwndMenu, IDM_ITMJOINSEG, TRUE );
           pVisDoc = ACCESSWNDIDA( pITMIda->hwndFocus, PITMVISDOC );
           if ( pVisDoc )
           {
             pDoc = pVisDoc->pDoc;
             pSeg = EQFBGetSegW(pDoc, pDoc->TBCursor.ulSegNum);
             if (pSeg->SegFlags.JoinStart   )
             {
               SETAABITEM( hwndMenu, IDM_ITMSPLITSEG, TRUE );
             }
             else
             {
               SETAABITEM( hwndMenu, IDM_ITMSPLITSEG, FALSE );
             } /* endif */
           } /* endif */
        break;
      case IDM_EDIT_MENU:
           {
             DOSVALUE usInfo;            // info about clipboard
             PEQFBBLOCK  pstBlock;

             SETAABITEM( hwndMenu, IDM_FIND,       TRUE );
             pVisDoc = ACCESSWNDIDA( pITMIda->hwndFocus, PITMVISDOC );
             pDoc = pVisDoc->pDoc;
             pSeg = EQFBGetSegW(pDoc, pDoc->TBCursor.ulSegNum);
             fActive = FALSE;
             pstBlock = (PEQFBBLOCK) pDoc->pBlockMark;      // ptr to blockstruct
             if ( pDoc->docType == VISTGT_DOC )
             {
                if (pstBlock->pDoc == pDoc )
                {
                  if (( pSeg->qStatus == QF_VISACT )
                       && (pstBlock->ulSegNum == pSeg->ulSegNum)
                       && (pstBlock->ulEndSegNum == pSeg->ulSegNum) )
                  {
                    fActive = TRUE;
                  }
                  else
                  {
                    fActive = FALSE;
                  } /* endif */
                }
                else
                {
                  fActive = FALSE;
                }
             } /* endif */
             SETAABITEM( hwndMenu, IDM_CUT, fActive);

             SETAABITEM( hwndMenu, IDM_COPY,  (pstBlock->pDoc == pDoc) );

             if ( pDoc->docType == VISTGT_DOC )
             {
               fActive = (pSeg->qStatus == QF_VISACT);
             }
             else
             {
               fActive = FALSE;
             } /* endif */

             SETAABITEM( hwndMenu, IDM_UNDO, fActive );

             SETAABITEM( hwndMenu, IDM_SPLIT, fActive );
             SETAABITEM( hwndMenu, IDM_JOIN,  fActive );
             SETAABITEM( hwndMenu, IDM_INITCAPS, fActive );
             usInfo = 0;
             if (fActive )
             {
               fActive = (ISCLIPBOARDFORMATAVAILABLE( CF_TEXT, &usInfo ) || ISCLIPBOARDFORMATAVAILABLE( CF_UNICODETEXT, &usInfo ));
             } /* endif */

             SETAABITEM( hwndMenu, IDM_PASTE, fActive );
           }
        break;

       case  IDM_GOTO_MENU:
           SETAABITEM( hwndMenu, IDM_NEXTANCHOR, TRUE );
           SETAABITEM( hwndMenu, IDM_PREVANCHOR, TRUE );
           SETAABITEM( hwndMenu, IDM_NEXTLONE, TRUE );
           SETAABITEM( hwndMenu, IDM_PREVLONE, TRUE );
           SETAABITEM( hwndMenu, IDM_SYNCHRONIZE, TRUE );
           SETAABITEM( hwndMenu, IDM_MARKSEGMENT, TRUE );
           SETAABITEM( hwndMenu, IDM_UNMARKSEG, TRUE );
           SETAABITEM( hwndMenu, IDM_MARKSEG,     TRUE );
           pVisDoc = ACCESSWNDIDA( pITMIda->hwndFocus, PITMVISDOC );
           if ( pVisDoc )
           {
             pDoc = pVisDoc->pDoc;
             pSeg = EQFBGetSegW(pDoc, pDoc->TBCursor.ulSegNum);
                                    // Disable Goto Mark if no mark availab.
             SETAABITEM( hwndMenu, IDM_GOTOMARK, pDoc->EQFBFlags.MarkedSeg );
                                  // disable clear mark if no mark is avail.
             if (pSeg )
             {
               SETAABITEM( hwndMenu, IDM_CLEARMARK, pSeg->SegFlags.Marked );
             } /* endif */
           } /* endif */
         break;
       case  IDM_VIEW_MENU:
           SETAABITEM( hwndMenu, IDM_FONTS,    TRUE  );
           SETAABITEM( hwndMenu, IDM_TILE,     TRUE  );
             SETAABITEM( hwndMenu, IDM_HORIZONTAL, TRUE );
             SETAABITEM( hwndMenu, IDM_VERTICAL,   TRUE );
           SETAABITEM( hwndMenu, IDM_PARALLEL, TRUE  );
           SETAABITEM( hwndMenu, IDM_AUTOWRAP, TRUE  );
           SETAABITEM( hwndMenu, IDM_SIZES,    TRUE );
           SETAABITEMCHECK( hwndMenu, IDM_HORIZONTAL,
                            (pITMIda->fHorizontal == TRUE) );
           SETAABITEMCHECK( hwndMenu, IDM_VERTICAL,
                            (pITMIda->fHorizontal != TRUE) );
           SETAABITEMCHECK( hwndMenu, IDM_PARALLEL,
                            (pITMIda->fParallel == TRUE) );
           SETAABITEMCHECK( hwndMenu, IDM_AUTOWRAP,
                            (pITMIda->fAutoWrap == TRUE) );

           pVisDoc = ACCESSWNDIDA( pITMIda->hwndFocus, PITMVISDOC );
           if ( pVisDoc )
           {
             pDoc = pVisDoc->pDoc;
             fActive = pDoc->pUserSettings->UserOptFlags.bVisibleSpace;
             SETAABITEMCHECK( hwndMenu, IDM_VISIBLESPACE, fActive);
           } /* endif */
         break;


       case  IDM_STYLES:
         SETAABITEM( hwndMenu, IDM_PROTECTED, TRUE );
         SETAABITEM( hwndMenu, IDM_UNPROTECTED, TRUE );
         SETAABITEM( hwndMenu, IDM_SHRINK,    TRUE );
         SETAABITEM( hwndMenu, IDM_COMPACT,   TRUE );
         SETAABITEM( hwndMenu, IDM_UNPROTECTED,    TRUE );
         SETAABITEM( hwndMenu, IDM_HIDE, TRUE );
         SETAABITEM( hwndMenu, IDM_SHORTEN, TRUE );

         // set check mark
         pDoc = &(pITMIda->TBSourceDoc);
         if ( pDoc )
         {
           SETAABITEMCHECK( hwndMenu, IDM_PROTECTED,
                            (pDoc->DispStyle == DISP_PROTECTED) );
           SETAABITEMCHECK( hwndMenu, IDM_UNPROTECTED,
                            (pDoc->DispStyle == DISP_UNPROTECTED) );
           SETAABITEMCHECK( hwndMenu, IDM_SHRINK,
                            (pDoc->DispStyle == DISP_SHRINK) );
           SETAABITEMCHECK( hwndMenu, IDM_COMPACT,
                            (pDoc->DispStyle == DISP_COMPACT) );
           SETAABITEMCHECK( hwndMenu, IDM_HIDE,
                            (pDoc->DispStyle == DISP_HIDE) );
           SETAABITEMCHECK( hwndMenu, IDM_SHORTEN,
                            (pDoc->DispStyle == DISP_SHORTEN) );
         } /* endif */
         break;
   } /* endswitch */
}

//------------------------------------------------------------------------------
// Internal function                                                            
//------------------------------------------------------------------------------
// Function name:     ITMCommand                                                
//------------------------------------------------------------------------------
// Function call:     _                                                         
//------------------------------------------------------------------------------
// Description:       handle all commands of main ITM window                    
//                    (menuitem commands)                                       
//------------------------------------------------------------------------------
// Parameters:        HWND hwnd,                                                
//                    MPARAM mp1                                                
//------------------------------------------------------------------------------
// Returncode type:   void                                                      
//------------------------------------------------------------------------------
// Returncodes:       none                                                      
//------------------------------------------------------------------------------
// Function flow:     IDM_ITMQUIT: quit visualization without save to memory    
//                    IDM_ENDSAVE: save to memory and quit                      
//                    IDM_CONTINUE: save for continue                           
//                    IDM_TOGGLE:  set focus to other  window                   
//                    IDM_CONNECT: set an anchor                                
//                    IDM_DELCONNECT   delete an anchor                         
//                    IDM_DELUSERCON    - not implemented yet                   
//                    IDM_CROSSOUT:     - cross out marked segment              
//                    IDM_UNDOCROSSOUT: - undo crossed out segment              
//                    IDM_ITMJOINSEG:   - join segment                          
//                    IDM_ITMSPLITSEG:  - split segment                         
//                    IDM_NEXTANCHOR:   - activate &position to next anchor     
//                    IDM_PREVANCHOR:   - activate & position to prev anchor    
//                    IDM_NEXTLONE:     - goto next unaligned segment           
//                    IDM_PREVLONE:     - goto previous unaligned segment       
//                    IDM_FONTS:        - set fonts & colors                    
//                    IDM_PROTECTED:    - set protected style                   
//                    IDM_SHRINK:       - set shrink style                      
//                    IDM_COMPACT:      - set compact style                     
//                    IDM_SYNCHRONIZE:  - set active alignment to cursor        
//                    IDM_MARKSEGMENT:  - mark cursor segment                   
//                    IDM_UNMARKSEG:    - unmark segment                        
//                    IDM_SIZES:        - set fontsizes                         
//                    IDM_TILE:         - select arrange                        
//                    IDM_HORIZONTAL:   - arrange windows horizontally          
//                    IDM_VERTICAL:     - arrange windows vertically            
//                    IDM_PARALLEL:     - make alignments parallel              
//                    IDM_AUTOWRAP:     - make auto linewrapping...             
//------------------------------------------------------------------------------
static
void ITMCommand
(
 HWND hwnd,
 WPARAM mp1
)
{
  PITMIDA     pITMIda;
  BOOL        fUpdate = TRUE;        // force screen update
  PITMVISDOC  pVisDoc;


  pITMIda = ACCESSWNDIDA( hwnd, PITMIDA );

  switch ( SHORT1FROMMP1 ( mp1 ) )
  {
    case  IDM_ITMQUIT:
      ITMFuncQuit(pITMIda, hwnd, pITMIda->hab);
    fUpdate = FALSE;
      break;
    case  IDM_ENDSAVE:
      ITMFuncSave(pITMIda, hwnd, pITMIda->hab);
    fUpdate = FALSE;
      break;
    case  IDM_CONTINUE:
      /****************************************************************/
      /* save alignment temporarily for later  continuation           */
      /****************************************************************/
      ITMFuncContinue(pITMIda);
      break;
    case IDM_TOGGLE:
      pVisDoc = ACCESSWNDIDA( pITMIda->hwndFocus, PITMVISDOC );
      if ( !pVisDoc )
      {
        pVisDoc = &(pITMIda->stVisDocSrc);
      } /* endif */
      if ( pVisDoc )
      {
        if (pVisDoc->pDoc->docType == VISSRC_DOC )
        {
           WinSetActiveWindow(HWND_DESKTOP, pITMIda->TBTargetDoc.hwndFrame );
        }
        else
        {
           WinSetActiveWindow(HWND_DESKTOP, pITMIda->TBSourceDoc.hwndFrame );
        } /* endif */
      } /* endif */

      UPDSTATUSBAR( pITMIda );
      break;
    case  IDM_CONNECT:
      ITMFuncSetAnchor(pITMIda);
      break;
    case  IDM_DELCONNECT:
      ITMFuncDelAnchor(pITMIda);
      break;
    case  IDM_DELUSERCON:
      ITMFuncDelAllUser(pITMIda);
      break;
    case  IDM_CROSSOUT:
      ITMFuncCrossOut(pITMIda);
      break;
    case  IDM_UNDOCROSSOUT:
      ITMFuncUndoCrossOut(pITMIda);
      break;
    case  IDM_ITMJOINSEG:
      pVisDoc = ACCESSWNDIDA( pITMIda->hwndFocus, PITMVISDOC );
      if ( !pVisDoc )
      {
        pVisDoc = &(pITMIda->stVisDocSrc);
      } /* endif */
      ITMFuncJoinSeg(pITMIda, pVisDoc);
      break;
    case  IDM_ITMSPLITSEG:
      pVisDoc = ACCESSWNDIDA( pITMIda->hwndFocus, PITMVISDOC );
      if ( !pVisDoc )
      {
        pVisDoc = &(pITMIda->stVisDocSrc);
      } /* endif */
      ITMFuncSplitSeg(pITMIda, pVisDoc);
      break;
    case IDM_ITMADDABBR:
      {
       PTBDOCUMENT pDoc ;
       PSZ         pLang;
       SHORT       sLangID;

        pVisDoc = ACCESSWNDIDA( pITMIda->hwndFocus, PITMVISDOC );
        if ( !pVisDoc )
        {
          pVisDoc = &(pITMIda->stVisDocSrc);
        } /* endif */
        pDoc = pVisDoc->pDoc;
        pLang = (pDoc->docType==VISSRC_DOC ) ?
                     pITMIda->szSourceLang : pITMIda->szTargetLang;

        if ( !MorphGetLanguageID( pLang, &sLangID ) )
        {
          EQFBAddAbbrevFunc(pDoc, sLangID);
        } /* endif */
      }
      break;
    case  IDM_NEXTANCHOR:
      ITMFuncNextAnchor(pITMIda, NEXT);
      break;
    case  IDM_PREVANCHOR:
      ITMFuncNextAnchor(pITMIda, PREVIOUS);
      break;
    case  IDM_NEXTLONE:
      ITMFuncNextLone(pITMIda, NEXT);
      break;
    case  IDM_PREVLONE:
      ITMFuncNextLone(pITMIda, PREVIOUS);
      break;
    case IDM_NEXTBAD:
      ITMFuncNextIrregular(pITMIda, NEXT);
      break;
    case IDM_PREVBAD:
      ITMFuncNextIrregular(pITMIda, PREVIOUS);
      break;
    case IDM_FIND:
      pVisDoc = ACCESSWNDIDA( pITMIda->hwndFocus, PITMVISDOC );
      if ( !pVisDoc )
      {
        pVisDoc = &(pITMIda->stVisDocSrc);
      } /* endif */
      if ( pVisDoc )
      {
        if (pVisDoc->pDoc->docType == VISSRC_DOC )
        {
           EQFBFuncFind( &pITMIda->TBSourceDoc );
        }
        else
        {
           EQFBFuncFind( &pITMIda->TBTargetDoc );
        } /* endif */
      } /* endif */
      break;
    case IDM_CUT:
      pVisDoc = ACCESSWNDIDA( pITMIda->hwndFocus, PITMVISDOC );
      ITMFuncCutToClip( pVisDoc);
      break;
    case IDM_COPY:
      pVisDoc = ACCESSWNDIDA( pITMIda->hwndFocus, PITMVISDOC );
      ITMFuncCopyToClip( pVisDoc);
      break;
    case IDM_PASTE:
      pVisDoc = ACCESSWNDIDA( pITMIda->hwndFocus, PITMVISDOC );
      ITMFuncPasteFromClip( pVisDoc);
      break;
    case IDM_CLEAR:
      pVisDoc = ACCESSWNDIDA( pITMIda->hwndFocus, PITMVISDOC );
      ITMFuncPasteFromClip( pVisDoc);
      break;
    case IDM_UNDO:
      pVisDoc = ACCESSWNDIDA( pITMIda->hwndFocus, PITMVISDOC );
      ITMFuncUndo( pVisDoc);
      break;
    case IDM_SPLIT:                //splitline
      pVisDoc = ACCESSWNDIDA( pITMIda->hwndFocus, PITMVISDOC );
      ITMFuncSplitLine( pVisDoc);
      break;
    case IDM_JOIN:                //joinline
      pVisDoc = ACCESSWNDIDA( pITMIda->hwndFocus, PITMVISDOC );
      ITMFuncJoinLine ( pVisDoc);
      break;
    case IDM_INITCAPS:           //initcaps
      pVisDoc = ACCESSWNDIDA( pITMIda->hwndFocus, PITMVISDOC );
      ITMFuncCaps (pVisDoc);
      break;
    case  IDM_FONTS:
      EQFBFuncFonts( &(pITMIda->TBSourceDoc) );
      EQFBGetColors(pITMIda->pColorTable);                  //get color table
      break;
     case IDM_PROTECTED:                  // switch to protected mode
        ITMFuncStyle(pITMIda, DISP_PROTECTED);
        break;
     case IDM_SHRINK:                     // switch to shrink style
        ITMFuncStyle(pITMIda, DISP_SHRINK);
        break;
     case IDM_COMPACT:                    // switch to compact style
        ITMFuncStyle(pITMIda, DISP_COMPACT);
        break;
     case IDM_HIDE:                    // switch to compact style
        ITMFuncStyle(pITMIda, DISP_HIDE);
        break;
     case IDM_SHORTEN:                    // switch to compact style
        ITMFuncStyle(pITMIda, DISP_SHORTEN);
        break;
      case IDM_UNPROTECTED:
        ITMFuncStyle(pITMIda, DISP_UNPROTECTED);
        break;
      case  IDM_HORIZONTAL:
        ITMTile(pITMIda, TRUE);            // arrange horizontally
        break;
      case  IDM_VERTICAL:
        ITMTile(pITMIda, FALSE);           // arrange vertically
        break;
    case  IDM_SYNCHRONIZE:
      pVisDoc = ACCESSWNDIDA( pITMIda->hwndFocus, PITMVISDOC );
      if ( !pVisDoc )
      {
        pVisDoc = &(pITMIda->stVisDocSrc);
      } /* endif */
      ITMFuncSynch(pITMIda, pVisDoc );
      break;
    case  IDM_MARKSEGMENT:
     {
       PTBDOCUMENT pDoc ;
        pVisDoc = ACCESSWNDIDA( pITMIda->hwndFocus, PITMVISDOC );
        pDoc = pVisDoc->pDoc;
        EQFBFuncMarkSegment( pDoc );
        EQFBScreenData( pDoc );                   // display screen
     }
     break;
    case  IDM_UNMARKSEG:
      {
       PTBDOCUMENT pDoc ;
        pVisDoc = ACCESSWNDIDA( pITMIda->hwndFocus, PITMVISDOC );
        pDoc = pVisDoc->pDoc;
        EQFBFuncMarkClear( pDoc );
        EQFBScreenData( pDoc );                   // display screen
      }
      break;
    case IDM_GOTOLINE:
      pVisDoc = ACCESSWNDIDA( pITMIda->hwndFocus, PITMVISDOC );
      ITMGotoLine(pVisDoc);
      break;
    case IDM_QUERYLINE:
      pVisDoc = ACCESSWNDIDA( pITMIda->hwndFocus, PITMVISDOC );
      ITMQueryLine(pVisDoc);
      break;
    case  IDM_SIZES:
      EQFBFuncFontSize( &(pITMIda->TBSourceDoc) );
      break;
    case  IDM_PARALLEL:
      pVisDoc = ACCESSWNDIDA( pITMIda->hwndFocus, PITMVISDOC );
      ITMFuncParallel(pITMIda, pVisDoc);
      break;
    case IDM_AUTOWRAP:
      pITMIda->fAutoWrap = !pITMIda->fAutoWrap;
      ITMAutoWrap(pITMIda, pITMIda->fAutoWrap);
      break;
    case IDM_VISIBLESPACE:
      pVisDoc = ACCESSWNDIDA( pITMIda->hwndFocus, PITMVISDOC );
      ITMFuncVisibleSpace(pVisDoc);
      break;
    case IDM_MARKSEG:                    //bookmark
      {
       PTBDOCUMENT pDoc ;
        pVisDoc = ACCESSWNDIDA( pITMIda->hwndFocus, PITMVISDOC );
        pDoc = pVisDoc->pDoc;
        EQFBMark(pDoc);
      }
      break;
    case IDM_GOTOMARK:                     // goto bookmark
      {
       PTBDOCUMENT pDoc ;
        pVisDoc = ACCESSWNDIDA( pITMIda->hwndFocus, PITMVISDOC );
        pDoc = pVisDoc->pDoc;
        EQFBFindMark(pDoc);
      }
      break;
    case IDM_CLEARMARK:                    // clear bookmark
      {
       PTBDOCUMENT pDoc ;
        pVisDoc = ACCESSWNDIDA( pITMIda->hwndFocus, PITMVISDOC );
        pDoc = pVisDoc->pDoc;
        EQFBClearMark(pDoc);
      }
      break;
    case IDM_PROOFSEG:
      pVisDoc = ACCESSWNDIDA( pITMIda->hwndFocus, PITMVISDOC );
      ITMFuncSpellSeg(pVisDoc);
      break;
    case IDM_PROOFALL:
      pVisDoc = ACCESSWNDIDA( pITMIda->hwndFocus, PITMVISDOC );
      ITMFuncSpellFile(pVisDoc);
      break;
    case IDM_NEXTMISSPELL:
      break;

    case PID_HELP_MI_INDEX :
       WinHelp( hwnd, EqfSystemHlpFile, HELP_CONTENTS, 0L );
       break;
    case PID_HELP_FOR_HELP :
       /********************************************************/
       /* Handle help for help request                         */
       /********************************************************/
       WinHelp( hwnd, EqfSystemHlpFile, HELP_HELPONHELP, 0L );
       break;
    case PID_HELP_MI_PRODUCTINFO:
       /********************************************************/
       /* Handle request for product Info panel                */
       /********************************************************/
       TwbShowLogo( 0L, hwnd, szEqfResFile, EQFITM, ITM_REVISION );
       break;
     case PID_FILE_MI_EBUS:
	  StartBrowser("home");
	  break;

   case PID_FILE_MI_EBUS1:
	  StartBrowser("docu");
	  break;

   case PID_FILE_MI_TECHGUIDE:
	  StartBrowser("techguide");
	  break;

   case PID_FILE_MI_EBUS2:
	  StartBrowser("docu");
	  break;
  } /* command switch */
  /******************************************************************/
  /* refresh the screen only if necessary ....                      */
  /******************************************************************/
  if ( fUpdate )
  {
    EQFBRefreshScreen( &(pITMIda->TBSourceDoc) );     // refresh the screen
    EQFBRefreshScreen( &(pITMIda->TBTargetDoc) );     // refresh the screen
  } /* endif */
} /* ITMCommand  */

//------------------------------------------------------------------------------
// External function                                                            
//------------------------------------------------------------------------------
// Function name:     ITMPROCWNDPROC                                            
//------------------------------------------------------------------------------
// Function call:     _                                                         
//------------------------------------------------------------------------------
// Description:       window procedure for progress window                      
//------------------------------------------------------------------------------
// Parameters:        HWND hwnd,                                                
//                    USHORT msg,                                               
//                    WPARAM mp1,                                               
//                    LPARAM mp2                                                
//------------------------------------------------------------------------------
// Returncode type:   MRESULT APIENTRY                                          
//------------------------------------------------------------------------------
// Returncodes:       _                                                         
//------------------------------------------------------------------------------
// Function flow:     display and paint progress window                         
//------------------------------------------------------------------------------

MRESULT APIENTRY
ITMPROCWNDPROC
(
  HWND hwnd,
  WINMSG msg,
  WPARAM mp1,
  LPARAM mp2
)
{
  MRESULT mResult = FALSE;           // window proc return value

  switch( msg )
  {
      case WM_CREATE:
        rectProcess.left = ITM_XPOS;
        rectProcess.top =  ITM_YPOS + ITM_YDELTA * 4;
        rectProcess.right =
         (USHORT) (WinQuerySysValue( HWND_DESKTOP, SV_CXSCREEN) * .7)-ITM_XPOS;
        rectProcess.bottom = rectProcess.top + ITM_YDELTA;
        rectOther.left = ITM_XPOS;
        rectOther.top =  ITM_YPOS;
        rectOther.right =
         (USHORT) (WinQuerySysValue( HWND_DESKTOP, SV_CXSCREEN) * .7)-ITM_XPOS;
        rectOther.bottom = rectOther.top + ITM_YDELTA;
        break;
      case WM_PAINT:
        {
          SHORT   x, y;                          // String screen coordinates
          HDC      hdc;                          // device context
          PAINTSTRUCT ps;                        // pointer to paint struct
          PITMIDA  pITMIda;                      // pointer to ida
          PSZ    pTempStatus;                    // pointer to temporary status
          RECT   rect;

          pITMIda = ACCESSWNDIDA( hwnd, PITMIDA );

          hdc = BeginPaint(hwnd, &ps );
          if ( pITMIda && (pITMIda->usStatus != ITM_STAT_VISUAL)
              && !(pITMIda->fVisual && (pITMIda->usStatus == ITM_STAT_CLEANUP)))
          {

            /**********************************************************/
            /* we will always paint the process indication...         */
            /**********************************************************/
            if (IntersectRect(&rect, &rectProcess, &ps.rcPaint))
            {
              x = ITM_XPOS;
              y = ITM_YPOS + ITM_YDELTA * 4;       // Set the text coordinates
              strcpy( cTermBuf, cDisp );
              strcat( cTermBuf, ITM_SPACESTRING );
              strcat( cTermBuf, ITM_SPACESTRING );
              TextOut (hdc, x, y, cTermBuf, strlen( cTermBuf ));
            } /* endif */

            /**********************************************************/
            /* we are forced to repaint everything from top to bottom */
            /**********************************************************/
            if (IntersectRect(&rect, &rectOther, &ps.rcPaint))
            {
              x = ITM_XPOS;
              y = ITM_YPOS + ITM_YDELTA * 3;      // Set the text coordinates

              switch ( pITMIda->usStatus  )
              {
                case  ITM_STAT_CLEANUP:
                  pTempStatus = chCleanup;
                  break;
                case  ITM_STAT_ANALYSIS:
                  pTempStatus = chAnalyse;
                  break;
                case  ITM_STAT_ALIGN:
                  pTempStatus = chAligning;
                  break;
                case  ITM_STAT_PREPARE:
                  pTempStatus = chPrepare;
                  break;
                case  ITM_STAT_ERROR:
                  pTempStatus = chError;
                  break;
                default :
                  pTempStatus = ITM_SPACESTRING;
                  break;
              } /* endswitch */
              sprintf( cTermBuf, chStatus, pTempStatus );
              strcat( cTermBuf, ITM_SPACESTRING );
              TextOut (hdc, x, y, cTermBuf, strlen( cTermBuf ));

              x = ITM_XPOS;
              y = ITM_YPOS + ITM_YDELTA * 2;      // Set the text coordinates,
              sprintf( cTermBuf, chTargetFile, pITMIda->chTargetFile );
              strcat( cTermBuf, ITM_SPACESTRING );
              strcat( cTermBuf, ITM_SPACESTRING );
              TextOut (hdc, x, y, cTermBuf, strlen( cTermBuf ));

              x = ITM_XPOS;
              y = ITM_YPOS + ITM_YDELTA * 1;      // Set the text coordinates,
              sprintf( cTermBuf, chSourceFile, pITMIda->chSourceFile );
              strcat( cTermBuf, ITM_SPACESTRING );
              strcat( cTermBuf, ITM_SPACESTRING );
              TextOut (hdc, x, y, cTermBuf, strlen( cTermBuf ));

              x = ITM_XPOS;
              y = ITM_YPOS + ITM_YDELTA * 0; // Set the text coordinates,
              {
                CHAR chTM[ MAX_FNAME ];
                Utlstrccpy( chTM, pITMIda->chTransMemFname, DOT );
                sprintf( cTermBuf, chTMFile, chTM );
              }
              strcat( cTermBuf, ITM_SPACESTRING );
              strcat( cTermBuf, ITM_SPACESTRING );
              TextOut (hdc, x, y, cTermBuf, strlen( cTermBuf ));
            } /* endif */
          } /* endif */

          EndPaint(hwnd, &ps);
        }

        break;

      default:
        mResult = WinDefWindowProc ( hwnd, msg, mp1, mp2 );
        break;

  } /* switch */

  return mResult ;
} /* end of function ITMPROCWNDPROC */

//------------------------------------------------------------------------------
// External function                                                            
//------------------------------------------------------------------------------
// Function name:     ItmSetAAB                                                 
//------------------------------------------------------------------------------
// Function call:     ItmSetAAB(hwnd, fAttr)                                    
//------------------------------------------------------------------------------
// Description:       disable AAB items  (if fAttr = FALSE)                     
//------------------------------------------------------------------------------
// Parameters:        HWND   hwndMenu                                           
//                    BOOL   fAttr                                              
//------------------------------------------------------------------------------
// Returncode type:   VOID                                                      
//------------------------------------------------------------------------------
// Returncodes:       none                                                      
//------------------------------------------------------------------------------
// Function flow:     if fattr = FALSE, disable all AAB items                   
//                    else enable all                                           
//------------------------------------------------------------------------------
VOID
ItmSetAAB
(
  HMENU   hwndMenu,
  BOOL    fAttr
)
{
   SETAABITEM( hwndMenu, IDM_ITM_FILE_MENU, fAttr );
   SETAABITEM( hwndMenu, IDM_EDIT_MENU, fAttr );
   SETAABITEM( hwndMenu, IDM_GOTO_MENU, fAttr );
   SETAABITEM( hwndMenu, IDM_VIEW_MENU, fAttr );
   /*******************************************************************/
   /* items of file menuitem                                          */
   /*******************************************************************/
   SETAABITEM( hwndMenu, IDM_ITMQUIT , fAttr);
   SETAABITEM( hwndMenu, IDM_ENDSAVE, fAttr );
   SETAABITEM( hwndMenu, IDM_CONTINUE, fAttr );
   SETAABITEM( hwndMenu, IDM_TOGGLE, fAttr );
   /*******************************************************************/
   /* items of align menuitem                                         */
   /*******************************************************************/
   SETAABITEM( hwndMenu, IDM_CONNECT, fAttr);
   SETAABITEM( hwndMenu, IDM_DELCONNECT, fAttr );
   SETAABITEM( hwndMenu, IDM_DELUSERCON, fAttr );
   SETAABITEM( hwndMenu, IDM_CROSSOUT, fAttr );
   SETAABITEM( hwndMenu, IDM_UNDOCROSSOUT, fAttr );
   SETAABITEM( hwndMenu, IDM_ITMJOINSEG, fAttr );
   SETAABITEM( hwndMenu, IDM_ITMSPLITSEG, fAttr );
   SETAABITEM( hwndMenu, IDM_ITMADDABBR,  fAttr );
   /*******************************************************************/
   /* items of goto menuitem                                          */
   /*******************************************************************/
   SETAABITEM( hwndMenu, IDM_NEXTANCHOR, fAttr);
   SETAABITEM( hwndMenu, IDM_PREVANCHOR, fAttr );
   SETAABITEM( hwndMenu, IDM_NEXTLONE, fAttr );
   SETAABITEM( hwndMenu, IDM_PREVLONE, fAttr );
   SETAABITEM( hwndMenu, IDM_SYNCHRONIZE, fAttr );
   SETAABITEM( hwndMenu, IDM_MARKSEGMENT, fAttr );
   SETAABITEM( hwndMenu, IDM_UNMARKSEG, fAttr );
   SETAABITEM( hwndMenu, IDM_FONTS,    fAttr  );
   SETAABITEM( hwndMenu, IDM_STYLES,   fAttr );
     SETAABITEM( hwndMenu, IDM_PROTECTED, fAttr );
     SETAABITEM( hwndMenu, IDM_SHRINK,    fAttr );
     SETAABITEM( hwndMenu, IDM_COMPACT,   fAttr );
     SETAABITEM( hwndMenu, IDM_HIDE,   fAttr );
     SETAABITEM( hwndMenu, IDM_SHORTEN,   fAttr );
   SETAABITEM( hwndMenu, IDM_TILE, fAttr );
     SETAABITEM( hwndMenu, IDM_HORIZONTAL, fAttr );
     SETAABITEM( hwndMenu, IDM_VERTICAL, fAttr );
   SETAABITEM( hwndMenu, IDM_PARALLEL, fAttr );
   SETAABITEM( hwndMenu, IDM_FIND, fAttr );

   SETAABITEM( hwndMenu, IDM_MARKSEG, fAttr );
   SETAABITEM( hwndMenu, IDM_GOTOMARK, fAttr );
   SETAABITEM( hwndMenu, IDM_CLEARMARK, fAttr );


   SETAABITEM( hwndMenu, IDM_SIZES, fAttr );
} /* end of function ItmSetAAB */


//------------------------------------------------------------------------------
// Internal function                                                            
//------------------------------------------------------------------------------
// Function name:     TimerStartVisual                                          
//------------------------------------------------------------------------------
// Function call:     _                                                         
//------------------------------------------------------------------------------
// Description:       _                                                         
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
void
TimerStartVisual
(
  HWND     hwnd,
  PITMIDA  pITMIda
)
{
  SHORT sRC;

  if (pITMIda->fVisual )
  {
    if (pITMIda->fTimer )
    {
      WinStopTimer( pITMIda->hab, hwnd, ITM_TIMER_ID );
      pITMIda->fTimer = FALSE;
    } /* endif */
    pITMIda->fCurDispStatus = DISP_VISUAL;
      /****************************************************************/
      /* display the actionbar                                        */
      /****************************************************************/
      SetMenu( hwnd, hwndMenu );

    WinShowWindow( pITMIda->hProcWnd, FALSE );
    VisualStart ( pITMIda->hab, hwnd, pITMIda );
  }
  else if (pITMIda->fPrepare)
  {
    if (pITMIda->fTimer )
    {
      WinStopTimer( pITMIda->hab, hwnd, ITM_TIMER_ID );
      pITMIda->fTimer = FALSE;
    } /* endif */
    sRC = ITMFuncContinue(pITMIda);
    FreeAll(pITMIda);              //free allocated space
    if ( !sRC )
    {
      pITMIda->usStatus = ITM_STAT_ALIGN;
      pITMIda->fBusy = FALSE;
      pITMIda->fCurDispStatus = DISP_ALIGNING;
      /********************************************************/
      /* initiate a new process loop...                       */
      /********************************************************/
      PostMessage( hwnd, WM_EQF_PROCESSTASK,
                   MP1FROMSHORT( ITM_PROCESS_PREPFILE ),
                   MP2FROMSHORT( 0 ));
    }
    else
    {
      pITMIda->fKill = TRUE;
      /************************************************/
      /* end of processing                            */
      /************************************************/
      pITMIda->usStatus = ITM_STAT_ENDVISUAL;
      WinPostMsg( hwnd, WM_CLOSE, NULL, NULL );
    } /* endif */
    if (!pITMIda->fTimer )
    {
      if (WinStartTimer (pITMIda->hab, hwnd, ITM_TIMER_ID, ITM_TIMER_VAL) )
      {
         pITMIda->fTimer = TRUE;
      } /* endif */
    } /* endif */
  } /* endif */
} /* end of function TimerStartVisual */


/**********************************************************************/
/* ITMUtlError:                                                       */
/**********************************************************************/
//------------------------------------------------------------------------------
// Internal function                                                            
//------------------------------------------------------------------------------
// Function name:     ITMUtlError                                               
//------------------------------------------------------------------------------
// Function call:     _                                                         
//------------------------------------------------------------------------------
// Description:       Encapsulate UtlError and set ITM return code              
//------------------------------------------------------------------------------
// Parameters:        _                                                         
//------------------------------------------------------------------------------
// Returncode type:   USHORT                                                    
//------------------------------------------------------------------------------
// Returncodes:       MBID_CANCEL  -- default in case of fQuiet                 
//                    usMBId       -- return code from UtlError                 
//------------------------------------------------------------------------------
// Function flow:     set ITM return code and call UtlError if necessary        
//------------------------------------------------------------------------------

USHORT ITMUtlError
(
  PITMIDA  pITMIda,
  SHORT   sErrorNumber,               // number of message
  USHORT  usMsgType,                  // type of message
  USHORT  usNoOfParms,                // number of message parameters
  PSZ     *pParmTable,                // pointer to parameter table
  ERRTYPE ErrorType                   // type of error
)
{
  USHORT usMBId = MBID_CANCEL;
  if ( pITMIda )
  {
    pITMIda->usRC = sErrorNumber;
    if ( (!pITMIda->fQuiet) || (pITMIda->hwnd == HWND_FUNCIF)  )
    {
      usMBId = UtlErrorHwnd( sErrorNumber, usMsgType,
                         usNoOfParms, pParmTable, ErrorType, pITMIda->hwnd );
    } /* endif */
  }
  else
  {
    usMBId = UtlError( sErrorNumber, usMsgType,
                       usNoOfParms, pParmTable, ErrorType );
  } /* endif */
  return usMBId;
} /* end of function ITMUtlError */

USHORT ITMUtlErrorW
(
  PITMIDA  pITMIda,
  SHORT   sErrorNumber,               // number of message
  USHORT  usMsgType,                  // type of message
  USHORT  usNoOfParms,                // number of message parameters
  PSZ_W   *pParmTableW,               // pointer to parameter table
  ERRTYPE ErrorType                   // type of error
)
{
  USHORT usMBId = 0;
  PSZ    pParmTable[5];
  int    i = 0;

  if ( pITMIda )
  {
    pITMIda->usRC = sErrorNumber;
    if ( !pITMIda->fQuiet )
    {
      PSZ pData = NULL;
      UtlAlloc( (PVOID*) &pData, 0L, usNoOfParms * 256, NOMSG );
      if (pData)
      {
       for ( i=0; i<usNoOfParms; i++ )
       {
         pParmTable[i] = pData + i*256;
         Unicode2ASCII( pParmTableW[i], pParmTable[i], 0L );
       }

       usMBId =  UtlError( sErrorNumber, usMsgType,
                           usNoOfParms, pParmTable, ErrorType );
     UtlAlloc( (PVOID*) &pData, 0L, 0L, NOMSG );
      }
    } /* endif */
  }
  else
  {
    PSZ pData = NULL;
    UtlAlloc( (PVOID*) &pData, 0L, usNoOfParms * 256, NOMSG );
    if (pData)
    {
      for ( i=0; i<usNoOfParms; i++ )
    {
      pParmTable[i] = pData + i*256;
      Unicode2ASCII( pParmTableW[i], pParmTable[i], 0L );
    }
      usMBId =  UtlError( sErrorNumber, usMsgType,
                        usNoOfParms, pParmTable, ErrorType );
    UtlAlloc( (PVOID*) &pData, 0L, 0L, NOMSG );
    }
  } /* endif */
  return usMBId;
}

