/*! \file
	Copyright Notice:

	Copyright (C) 1990-2014, International Business Machines
	Corporation and others. All rights reserved
*/
#define INCL_EQF_MORPH            // morphological functions
#define INCL_EQF_ANALYSIS         // analysis functions
#define INCL_EQF_TM               // general Transl. Memory functions
#define INCL_EQF_TMREMOTE         // remote Transl. Memory functions (U-Code)
#define INCL_EQF_EDITORAPI        // editor API
#define INCL_EQF_TP               // public translation processor definitions
#include <eqf.h>                  // General Translation Manager include file

#undef   INIT_TABLES
#include "EQFTPI.H"               // private translation processor definitions

#define  INIT_TABLES  1
#include "EQFTMM.H"              // TMM include file
#include "EQFTMM.id"                   // TMM dialog include file

#define MAX_RC_STRING  31
#define TMM_TIMERID    5
#define TMM_TIMER      2000


/**********************************************************************/
/* static variables for text strings                                  */
/**********************************************************************/
CHAR chTargetFile[ MAX_RC_STRING ];
CHAR chSourceFile[ MAX_RC_STRING ];
CHAR chTitle[2048];

static TBSEGMENT tbSegment;

static TBSEGMENT  tbInitSegment = { NULL, 0, QF_PROP0PREFIX, 0, NULL,{0}, 0,{0},0,0,0, 0L, NULL, L"", NULL};
static TBSEGMENT  tbNewLineSegment = { NULL , 1,QF_XLATED ,0, NULL,{0}, 0,{0},0,0,0, 0L, NULL, L"\n", NULL};
static TBSEGMENT  tb2NewLineSegment = { NULL , 2,QF_XLATED ,0, NULL,{0}, 0,{0},0,0,0, 0L, NULL, L"\n\n", NULL};

static PTMMIDA   pstaticTMMIda;        // used to pass IDA ptr during WM_CREATE

static VOID SaveChangedSegs ( PTMMIDA pTMMIda );
static BOOL EmptySegDataCheck(PTBDOCUMENT pDoc, PTBSEGMENT pTgtSeg);
static USHORT TMMSegCountLF ( PSZ_W pData, PUSHORT pusLFAtEnd );
static VOID TMMAddLFsAtEnd ( PTBDOCUMENT, PTBSEGMENT, ULONG);

BOOL TMMTextFitsRect
(
  PSZ         pszText,                 // text being checked
  RECT        *prc,                    // ptr to rectangle for text
  HPS         hps                      // presentation space handle
);
//------------------------------------------------------------------------------
// External function                                                            
//------------------------------------------------------------------------------
// Function name:     EQFTMMaintain                                             
//------------------------------------------------------------------------------
// Function call:     EQFTMMaintain( pszTMName )                                
//------------------------------------------------------------------------------
// Description:       set up the main translation editor window                 
//------------------------------------------------------------------------------
// Input parameter:   PSZ pointer to name of translation memory                 
//------------------------------------------------------------------------------
// Returncode type:   BOOL                                                      
//------------------------------------------------------------------------------
// Returncodes:       True/False                                                
//------------------------------------------------------------------------------
// Function flow:     allocate required structures                              
//                    initialize paramters in main structure                    
//                    register main editor window                               
//                    load resource strings                                     
//                    position window                                           
//------------------------------------------------------------------------------
BOOL EQFTMMaintain( PSZ pszTMName )
{
  ULONG flCreate;                    // Window creation control flags
  HWND hwndFrame;                    // Frame window handle
  PTMMIDA pTMMIda;                   // pointer to main structure
  BOOL fOK = TRUE;                     // success indicator
  HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);

  /******************************************************************/
  /* allocate ida                                                   */
  /******************************************************************/
  if ( (fOK = UtlAlloc( (PVOID *)&pTMMIda, 0L,
                        (LONG)sizeof(TMMIDA), ERROR_STORAGE ) ) == TRUE )
  {
    //allocate pUserSettings to avoid subsequent trapping as user settings
    //are not used but are called up in various functions
    if ( (fOK = UtlAlloc( (PVOID *)&(pTMMIda->pTMIda), 0L,
                         (LONG)sizeof( TMIDA ), ERROR_STORAGE ) ) == TRUE )
    {
      EQFBGetUserSettings( &pTMMIda->TBSourceDoc );
      EQFBGetUserSettings( &pTMMIda->TBTargetDoc );
      pTMMIda->TBSourceDoc.bOperatingSystem = (BYTE) UtlGetOperatingSystemInfo();
      pTMMIda->TBTargetDoc.bOperatingSystem = (BYTE) UtlGetOperatingSystemInfo();
      pTMMIda->pTMIda->ulSystemPrefCP = GetLangOEMCP(NULL);
    } /* endif */
  } /* endif */

  /*******************************************************************/
  /* copy input parameters into Ida                                  */
  /*******************************************************************/
  if ( fOK )
  {
    pTMMIda->fHorizontal = TRUE;
    pTMMIda->TBTargetDoc.EQFBFlags.Reflow = TRUE;
    strcpy( pTMMIda->pTMIda->szFileName, pszTMName );
    pTMMIda->pTMIda->sCurCluster = -1;
    // get TM short name, save TM long name (szFileName is later replaced by
    // the TM path name)
    {
      BOOL fIsNew = FALSE;         // is-new flag
      ObjLongToShortName( pszTMName, pTMMIda->pTMIda->szMemShortName,
                          TM_OBJECT, &fIsNew );
      strcpy( pTMMIda->pTMIda->szMemLongName, pszTMName );
    }
  } /* endif */

  /********************************************************************/
  /* Load prompt string for status line                               */
  /********************************************************************/
  if ( fOK )
  {
    LOADSTRING( NULLHANDLE, hResMod, IDS_TMM_STATUSPROMPT,
                pTMMIda->szStatusPrompt );
    LOADSTRING( NULLHANDLE, hResMod, IDS_TMM_DOCPROMPT,
                pTMMIda->szDocPrompt );
    LOADSTRING( NULLHANDLE, hResMod, IDS_TMM_DATEPROMPT,
                pTMMIda->szDatePrompt );
    LOADSTRING( NULLHANDLE, hResMod, IDS_TMM_CURPROMPT,
                pTMMIda->szCurPrompt );
    LOADSTRING( NULLHANDLE, hResMod, IDS_TMM_TOTALPROMPT,
                pTMMIda->szTotalPrompt );
    LOADSTRING( NULLHANDLE, hResMod, IDS_TMM_COMPLPROMPT,
                pTMMIda->szComplPrompt );
    LOADSTRING( NULLHANDLE, hResMod, IDS_TMM_WORKSTATE,
                pTMMIda->szWorkState );
    LOADSTRING( NULLHANDLE, hResMod, IDS_TMM_SEARCHSTATE,
                pTMMIda->szSearchState );
    LOADSTRING( NULLHANDLE, hResMod, IDS_TMM_DELETESTATE,
                pTMMIda->szDeleteState );
    LOADSTRING( NULLHANDLE, hResMod, IDS_TMM_QUERYSTATE,
                pTMMIda->szQueryState );
  } /* endif */

  if ( fOK )
  {
    HWND hwndParent;                             // parent window for TMM window

    {
      WNDCLASS  wndclass;

      wndclass.style         = 0;
      wndclass.lpfnWndProc   = TMMWNDPROC;
      wndclass.cbClsExtra    = 0;
      wndclass.cbWndExtra    = sizeof(PSZ);
      wndclass.hInstance     = (HINSTANCE)(HAB)UtlQueryULong( QL_HAB );
      wndclass.hIcon         = (HPOINTER) UtlQueryULong(QL_TMMICON); //hiconTMM;
      wndclass.hCursor       = LoadCursor(NULL, IDC_ARROW);
      wndclass.hbrBackground = (HBRUSH)GetStockObject (WHITE_BRUSH);
      wndclass.lpszMenuName  = NULL;
      wndclass.lpszClassName = TMM_CLASS;

      RegisterClass(&wndclass);
    }

    flCreate = FCF_TASKLIST | FCF_SIZEBORDER | FCF_TITLEBAR |
               FCF_MENU | FCF_MINMAX ;

    hwndParent = EqfQueryTwbClient();

    /******************************************************************/
    /* load window title and insert TM name into the string           */
    /******************************************************************/
    {
      ULONG ulLen;                  // length of generated line
      PSZ    pszName;                  // ptr to TM name

      LOADSTRING( NULLHANDLE, hResMod, IDS_TMM_TITLE, chTargetFile );
      OEMTOANSI( pszTMName );
      pszName  = pszTMName;
      DosInsMessage( &pszName, 1, chTargetFile,  strlen(chTargetFile),
                     chTitle, sizeof(chTitle) - 1, &ulLen );
      ANSITOOEM( pszTMName );
      chTitle[ulLen] = EOS;
    }

    /*****************************************************************/
    /* load strings from resource                                    */
    /*****************************************************************/
    LOADSTRING( NULLHANDLE, hResMod, IDS_TMM_TARGETFILE, chTargetFile );
    LOADSTRING( NULLHANDLE, hResMod, IDS_TMM_SOURCEFILE, chSourceFile );

    {
       /*********************************************************/
       /* to avoid flickering during the creation of the MDI    */
       /* Child window we have to position it outside of the    */
       /* screen - we always reposition it to the last stored   */
       /* positions via a WinSetWindowPos function after init.  */
       /*********************************************************/
      HWND  hwndMDIClient;
      MDICREATESTRUCT mdicreate;

      mdicreate.hOwner  = (HAB)UtlQueryULong( QL_HAB );
      mdicreate.szClass = TMM_CLASS;
      mdicreate.szTitle = chTitle;
      mdicreate.x       = -100;
      mdicreate.y       = -100;
      mdicreate.cx      = 1;
      mdicreate.cy      = 1;
      mdicreate.style   = WS_CLIPCHILDREN;
      mdicreate.lParam  = MP2FROMP(pTMMIda);

      hwndMDIClient = (HWND)UtlQueryULong( QL_TWBCLIENT );

      hwndFrame = pTMMIda->hwnd =
      (HWND)SendMessage( hwndMDIClient,
                         WM_MDICREATE, 0,
                         MP2FROMP((LPMDICREATESTRUCT)&mdicreate) );
    }

    if ( hwndFrame )
    {
      pTMMIda->hwndFrame = hwndFrame;
      ANCHORWNDIDA( pTMMIda->hwnd, pTMMIda );

      /****************************************************************/
      /* create the window to fillup the entire client area of the    */
      /* translation workbench                                        */
      /****************************************************************/
      {
        SWP swp;

        WinQueryWindowPos( hwndParent, &swp );

        WinSetWindowPos( hwndFrame, HWND_TOP, 0, 0,
                         (SHORT)(swp.cx * 0.9), (SHORT)(swp.cy * 0.8),
                         EQF_SWP_SIZE | EQF_SWP_MOVE | EQF_SWP_ACTIVATE |
                         EQF_SWP_SHOW | EQF_SWP_MAXIMIZE );
      }
    }
    else
    {
      fOK = FALSE;
    } /* endif */
  } /* endif */

  return( fOK );

} /* end of function EQFTMMaintain */

//------------------------------------------------------------------------------
// External function                                                            
//------------------------------------------------------------------------------
// Function name:     TMMWndProc                                                
//------------------------------------------------------------------------------
// Function call:     window procedure                                          
//------------------------------------------------------------------------------
// Description:       This window procedure controls the display of the         
//                    process window                                            
//------------------------------------------------------------------------------
// Parameters:        HWND hwnd   window handle                                 
//                    USHORT msg  message number                                
//                    MPARAM mp1  message parameter1                            
//                    MPARAM mp2  message parameter2                            
//------------------------------------------------------------------------------
// Returncode type:   MRESULT                                                   
//------------------------------------------------------------------------------
// Returncodes:       from default window procedure                             
//------------------------------------------------------------------------------
// Function flow:     dependant on message do case processing                   
//                    WM_CREATE:                                                
//                      post msg to WM_EQF_INITIALIZE                           
//                    WM_EQF_INITIALIZE                                         
//                      start tm edit function                                  
//                      create windows for source and target segments           
//                      start timer                                             
//                      if error force shutdown                                 
//                    WM_INITMENU                                               
//                      initialize editor menu bar                              
//                    WM_EQF_PROCESSTASK                                        
//                      get segment                                             
//                      add to editor                                           
//                      post message for next segment if limit not reached      
//                      spacial handling for loading next bunch of segments     
//                    WM_ERASEBACKGROUND                                        
//                      get the background cleared in the system default color  
//                    WM_TIMER                                                  
//                      repaint window only at particular intevals              
//                      update status bar                                       
//                    WM_PAINT                                                  
//                      repaint the screen                                      
//                    WM_COMMAND                                                
//                      call to handle menu commands                 
//                    WM_CLOSE                                                  
//                      post msg to EQF_CLOSE                                   
//                    WM_EQF_CLOSE:                                             
//                      release allocated memory                                
//                      close application                                       
//                    default:                                                  
//                      pass on the message to the default window procedure..   
//                  endswitch                                                   
//                  return mResult                                              
//------------------------------------------------------------------------------
MRESULT APIENTRY TMMWNDPROC
(
  HWND hwnd,
  WINMSG msg,
  WPARAM mp1,
  LPARAM mp2
)
{
    MRESULT mResult = FALSE;           // window proc return value
    PTMMIDA  pTMMIda;                  // pointer to ida
    BOOL   fOK = TRUE;                 // success indicator
    static  BOOL    fMaximized;



    switch( msg )
    {
      case WM_CREATE:
        {
          LPCREATESTRUCT    pCreateStruct;
          LPMDICREATESTRUCT pMDICreateStruct;

          pCreateStruct = (LPCREATESTRUCT)PVOIDFROMMP2(mp2);
          pMDICreateStruct = (LPMDICREATESTRUCT)pCreateStruct->lpCreateParams;
          pTMMIda = (PTMMIDA)PVOIDFROMMP2(pMDICreateStruct->lParam);
          ANCHORWNDIDA( hwnd, pTMMIda );
		}
	    /************************************************************/
        /* post message to allow window to be primarily painted     */
        /************************************************************/
        // call StartTMFunct
        WinPostMsg( hwnd, WM_EQF_INITIALIZE, NULL, NULL);
        break;

      case WM_SIZE:
        {
          SHORT sHeigth = HIWORD(mp2);   // new height of window
          SHORT sSize   = LOWORD(mp2);   // new width size of window

          mResult = DefMDIChildProc( hwnd, msg, mp1, mp2 );

          /**************************************************************/
          /* Windows only: resize/rearrange controls only for normal    */
          /* size requests                                              */
          /**************************************************************/
          if ( (mp1 == SIZENORMAL) || (mp1 == SIZEFULLSCREEN) )
          {
            pTMMIda = ACCESSWNDIDA( hwnd, PTMMIDA );

            if ( pTMMIda )
            {
              TMMSetWindowPos( pTMMIda, sSize, sHeigth );
            } /* endif */
          } /* endif */
        }
        break;

      case WM_EQF_INITIALIZE:
        pTMMIda = ACCESSWNDIDA( hwnd, PTMMIDA );

        EqfRegisterObject( pTMMIda->pTMIda->szFileName, hwnd, clsTMMAINT );

        // call StartTMFunct
        fOK = StartTMFunct( hwnd, pTMMIda->pTMIda );

        //create windows for visualization
        if ( fOK )
        {
          fOK = UtlAlloc( (PVOID *)&pTMMIda->pTMParms,
                          0L,
                          (LONG)( (LONG)sizeof(TMEXTRAS) *
                                  (LONG)pTMMIda->pTMIda->ulSeg2Load ),
                          ERROR_STORAGE );
          if ( fOK )
          {
            WinStartTimer( WinQueryAnchorBlock(hwnd), hwnd, TMM_TIMERID, TMM_TIMER);

            //initialize current segment no
            pTMMIda->ulSrcSegNum = 1;
            pTMMIda->ulTgtSegNum = 1;

            // set msg for status bar that it can be set in DoVisual
            strcpy( pTMMIda->szMessage, pTMMIda->szSearchState );

            fOK = DoVisualTMM( WinQueryAnchorBlock(hwnd), hwnd, pTMMIda );
          } /* endif */
        } /* endif */

        if ( fOK )
        {
          WinPostMsg( hwnd, WM_EQF_PROCESSTASK, MP1FROMSHORT( GET_SEGMENT ),
                      MP2FROMP(pTMMIda) );
        }
        else
        {
          WinPostMsg( hwnd, WM_CLOSE, NULL, NULL );
        } /* endif */
        break;

      case WM_EQF_INITMENU:
      case WM_INITMENU:
      case WM_INITMENUPOPUP:
        {
          HMENU hwndMenu;
          pTMMIda = ACCESSWNDIDA( hwnd, PTMMIDA );

          hwndMenu = (HMENU)mp1;
          switch ( LOWORD(mp2) )
          {
            case  ID_TMM_ABB_FILE:
              if ( !pTMMIda->pTMIda->fRun )
              {
                // enable/disable next depending on fStop
                SETAABITEM( hwndMenu, ID_TMM_NEXT, !pTMMIda->pTMIda->fStop );

                if ( pTMMIda->pTMIda->ulPrevPos > 0 )
                {
                  SETAABITEM( hwndMenu, ID_TMM_PREVIOUS, TRUE );
                }
                else
                {
                  SETAABITEM( hwndMenu, ID_TMM_PREVIOUS, FALSE );
                } /* endif */
              }
              else
              {
                // disable next
                SETAABITEM( hwndMenu, ID_TMM_NEXT, FALSE );
                SETAABITEM( hwndMenu, ID_TMM_PREVIOUS, FALSE );
              } /* endif */
              break;
            case  ID_TMM_ABB_EDIT:
              {
                PTBDOCUMENT  pDoc;
                PTMMVISDOC   pVisDoc;

                pVisDoc = ACCESSWNDIDA( pTMMIda->hwndFocus, PTMMVISDOC );
                if ( pVisDoc != NULL )
                {
                  pDoc = pVisDoc->pDoc;
                }
                else
                {
                  pDoc = NULL;
                } /* endif */

                // set delete active when not still searching
                // and when in target doc
                if ( pTMMIda->pTMIda->fRun )
                {
                  SETAABITEM( hwndMenu, ID_TMM_DELETE, FALSE );
                }
                else
                {
                  SETAABITEM( hwndMenu, ID_TMM_DELETE, TRUE );
                  SETAABITEM((HMENU) WinWindowFromID((HWND) hwndMenu, ID_TMM_DELETE), ID_TMM_DELM_CUR, TRUE);
                  SETAABITEM((HMENU) WinWindowFromID((HWND) hwndMenu, ID_TMM_DELETE), ID_TMM_SETM_CUR, TRUE);
                  SETAABITEM((HMENU) WinWindowFromID((HWND) hwndMenu, ID_TMM_DELETE), ID_TMM_DELM_ALL, TRUE);
                  SETAABITEM((HMENU) WinWindowFromID((HWND) hwndMenu, ID_TMM_DELETE), ID_TMM_SETM_ALL, TRUE);
                  if ( pDoc )
                  {
                    if ( pDoc->TBCursor.ulSegNum == pDoc->tbActSeg.ulSegNum )
                    {
                      SETAABITEM( (HMENU) WinWindowFromID( (HWND)hwndMenu, ID_TMM_DELETE ),
                                  ID_TMM_DEL_CUR, TRUE );
                    }
                    else
                    {
                      SETAABITEM( (HMENU) WinWindowFromID( (HWND)hwndMenu, ID_TMM_DELETE ),
                                  ID_TMM_DEL_CUR, FALSE );
                    } /* endif */
                  }
                  else
                  {
                    SETAABITEM( (HMENU) WinWindowFromID( (HWND)hwndMenu, ID_TMM_DELETE ),
                                ID_TMM_DEL_CUR, FALSE );
                  } /* endif */
                } /* endif */
                // set find & change active when an entry found and not searching
                if ( !pTMMIda->pTMIda->fRun && (pTMMIda->pTMIda->ulRunNum > 0 ) )
                {
                  SETAABITEM( hwndMenu, ID_TMM_FIND, TRUE );
                }
                else
                {
                  SETAABITEM( hwndMenu, ID_TMM_FIND, FALSE );
                } /* endif */
                if ( (pTMMIda->usActWin == STARGET_DOC) &&
                     (pDoc != NULL)                     &&
                     (pTMMIda->pTMIda->ulRunNum > 0) &&
                     (pDoc->TBCursor.ulSegNum == pDoc->tbActSeg.ulSegNum) )
                {
                  SETAABITEM( hwndMenu, ID_TMM_CUT, TRUE );
                  SETAABITEM( hwndMenu, ID_TMM_PASTE, TRUE );
                  SETAABITEM( hwndMenu, ID_TMM_SPLIT, TRUE );
                  SETAABITEM( hwndMenu, ID_TMM_JOIN, TRUE );
                  SETAABITEM( hwndMenu, ID_TMM_UNDO, TRUE );
                }
                else
                {
                  SETAABITEM( hwndMenu, ID_TMM_CUT, FALSE );
                  SETAABITEM( hwndMenu, ID_TMM_PASTE, FALSE );
                  SETAABITEM( hwndMenu, ID_TMM_SPLIT, FALSE );
                  SETAABITEM( hwndMenu, ID_TMM_JOIN, FALSE );
                  SETAABITEM( hwndMenu, ID_TMM_UNDO, FALSE );
                } /* endif */
              }
              break;
            case  ID_TMM_ABB_OPTIONS:
              SETAABITEMCHECK( hwndMenu, ID_TMM_VERTICAL,
                               (pTMMIda->fHorizontal != TRUE) );
              SETAABITEMCHECK( hwndMenu, ID_TMM_HORIZONTAL,
                               (pTMMIda->fHorizontal == TRUE) );
              // set spellcheck active when not still searching
              if ( pTMMIda->usActWin == STARGET_DOC )
              {
                SETAABITEM( hwndMenu, ID_TMM_SPELLCHECK, !pTMMIda->pTMIda->fRun );
              }
              else
              {
                SETAABITEM( hwndMenu, ID_TMM_SPELLCHECK, FALSE );
              } /* endif */
              break;
            case  ID_TMM_ABB_STYLE:
              SETAABITEMCHECK( hwndMenu, ID_TMM_PROTECT,
                               (pTMMIda->TBTargetDoc.DispStyle ==
                                                        DISP_PROTECTED) );
              SETAABITEMCHECK( hwndMenu, ID_TMM_UNPROTECT,
                               (pTMMIda->TBTargetDoc.DispStyle ==
                                                        DISP_UNPROTECTED) );
              SETAABITEMCHECK( hwndMenu, ID_TMM_HIDE,
                               (pTMMIda->TBTargetDoc.DispStyle ==
                                                        DISP_HIDE) );
              SETAABITEMCHECK( hwndMenu, ID_TMM_COMPACT,
                               (pTMMIda->TBTargetDoc.DispStyle ==
                                                        DISP_COMPACT) );
              break;
            default :
              break;
          } /* endswitch */
        }
        break;

    case WM_EQF_TOOLBAR_ENABLED:
      pTMMIda = ACCESSWNDIDA( hwnd, PTMMIDA );
      switch ( mp1 )
      {
        /**************************************************************/
        /* check for items to be enabled ..                           */
        /**************************************************************/
        case ID_TMM_NEXT:
          mResult = !pTMMIda->pTMIda->fRun && !pTMMIda->pTMIda->fStop;
          break;
        case ID_TMM_PREVIOUS:
          mResult = !pTMMIda->pTMIda->fRun && (pTMMIda->pTMIda->ulPrevPos > 0);
          break;
        case ID_TMM_DELETE:
          mResult = !pTMMIda->pTMIda->fRun;
          break;
        case ID_TMM_DEL_CUR:
          {
            PTBDOCUMENT  pDoc;
            PTMMVISDOC   pVisDoc;

            pVisDoc = ACCESSWNDIDA( pTMMIda->hwndFocus, PTMMVISDOC );
            if ( pVisDoc != NULL )
            {
              pDoc = pVisDoc->pDoc;
            }
            else
            {
              pDoc = NULL;
            } /* endif */

            mResult = !pTMMIda->pTMIda->fRun && pDoc &&
                      ( pDoc->TBCursor.ulSegNum == pDoc->tbActSeg.ulSegNum );
          }
          break;
        case ID_TMM_FIND:
          mResult = !pTMMIda->pTMIda->fRun && (pTMMIda->pTMIda->ulRunNum > 0 );
          break;
        case ID_TMM_CUT:
        case ID_TMM_PASTE:
        case ID_TMM_SPLIT:
        case ID_TMM_JOIN:
        case ID_TMM_UNDO:
          {
            PTBDOCUMENT  pDoc;
            PTMMVISDOC   pVisDoc;

            pVisDoc = ACCESSWNDIDA( pTMMIda->hwndFocus, PTMMVISDOC );
            if ( pVisDoc != NULL )
            {
              pDoc = pVisDoc->pDoc;
            }
            else
            {
              pDoc = NULL;
            } /* endif */

            mResult = (pTMMIda->usActWin == STARGET_DOC) &&
                      (pDoc != NULL)                     &&
                      (pTMMIda->pTMIda->ulRunNum > 0) &&
                      (pDoc->TBCursor.ulSegNum == pDoc->tbActSeg.ulSegNum);
          }
          break;
        case ID_TMM_HORIZONTAL:
        case ID_TMM_VERTICAL:
          mResult = TRUE;
          break;
        case ID_TMM_SPELLCHECK:
          mResult = !pTMMIda->pTMIda->fRun && ( pTMMIda->usActWin == STARGET_DOC );
          break;
        case ID_TMM_PROTECT:
        case ID_TMM_UNPROTECT:
        case ID_TMM_HIDE:
        case ID_TMM_COMPACT:
        case ID_TMM_DELM_CUR:
        case ID_TMM_SETM_CUR:
        case ID_TMM_DELM_ALL:
        case ID_TMM_SETM_ALL:
          mResult = TRUE;
          break;
      } /* endswitch */
      break;

      case WM_EQF_PROCESSTASK:
        pTMMIda = ACCESSWNDIDA( hwnd, PTMMIDA );

        switch ( SHORT1FROMMP1(mp1) )
        {
          case GET_SEGMENT:
            //call the function to be get next segment
            pTMMIda->pTMIda->fRun = TRUE;
            if ( QueryTMEntry( pTMMIda->pTMIda ) )
            {
              if ( !pTMMIda->pTMIda->fStop )
              {
                if ( pTMMIda->pTMIda->ulExtracted == 1 )
                {
                  // clear screen after the first extract
                  EQFDispWindow ( &pTMMIda->TBSourceDoc );
                  EQFDispWindow ( &pTMMIda->TBTargetDoc );
                } /* endif */

                if ( pTMMIda->pTMIda->fFound )
                {
                  // when an entry was found from query add it to editor
                  fOK = AddSourceAndTranslation( pTMMIda );
                } /* endif */

                if ( fOK )
                {
                  //set active segment first time
                  if ( pTMMIda->pTMIda->ulRunNum == 1 )
                  {
                    fOK = ActivateSegment( pTMMIda, 1 );

                    if ( fOK )
                    {
                      EQFBRefreshScreen( &(pTMMIda->TBSourceDoc) ); // refresh the screen
                      EQFBRefreshScreen( &(pTMMIda->TBTargetDoc) );  // refresh the screen

                      //reset qstatus for subsequent pulldown functions
                      pTMMIda->TBSourceDoc.tbActSeg.qStatus = QF_XLATED;
                      pTMMIda->TBTargetDoc.tbActSeg.qStatus = QF_XLATED;
                    } /* endif */
                  } /* endif */

                  UtlDispatch();

                  pTMMIda = ACCESSWNDIDA( hwnd, PTMMIDA );
                  if ( pTMMIda && pTMMIda->pTMIda )
                  {
                    if ( pTMMIda->pTMIda->ulRunNum < pTMMIda->pTMIda->ulSeg2Load )
                    {
                      // post msg to get next entry
                      if ( fOK && !pTMMIda->fKill )
                      {
                        WinPostMsg( hwnd, WM_EQF_PROCESSTASK,
                                    MP1FROMSHORT( GET_SEGMENT ),
                                    mp2 );
                      }
                      else
                      {
                        // when fKill is set WM_CLOSE is already done
                        WinPostMsg( hwnd, WM_EQF_CLOSE, NULL, NULL );
                      } /* endif */
                    }
                    else
                    {
                      // for this time last entry was inserted, do update
                      pTMMIda->szMessage[0] = EOS;
                      UPDSTATUSBAR( pTMMIda );

                      pTMMIda->pTMIda->fRun = FALSE;
                      // set update to false and refresh the window
                      // after the last segment is inserted
                      pTMMIda->fUpdate = FALSE;
                      EQFDispWindow ( &pTMMIda->TBSourceDoc );
                      EQFDispWindow ( &pTMMIda->TBTargetDoc );
                    } /* endif */
                  } /* endif */
                } /* endif */
              }
              else
              {
                // no more entries found in TM
                // update status bar text
                pTMMIda->szMessage[0] = EOS;
                UPDSTATUSBAR( pTMMIda );

                pTMMIda->pTMIda->fRun = FALSE;
                // set update to false and refresh the window
                // after the last segment is inserted
                pTMMIda->fUpdate = FALSE;
                EQFDispWindow ( &pTMMIda->TBSourceDoc );
                EQFDispWindow ( &pTMMIda->TBTargetDoc );

                if ( pTMMIda->pTMIda->ulRunNum == 0 )
                {
                  if ( pTMMIda->pTMIda->ulExtracted != 0 )
                  {
                    // when no entry was found display warning msg
                    UtlError( ERROR_TMM_NO_ENTRY, MB_OK, 0, (PSZ *)NULP, EQF_WARNING );
                    BackToQuery( pTMMIda, hwnd );
                  }
                  else
                  {
                    // TM is empty display message and quit
                    UtlError( ERROR_TMM_EMPTY, MB_OK, 0, (PSZ *)NULP, EQF_WARNING );
                    WinPostMsg( hwnd, WM_CLOSE, NULL, NULL );
                  } /* endif */
                } /* endif */
              } /* endif */
            }
            else
            {
              // something goes wrong during getting segment
              WinPostMsg( hwnd, WM_CLOSE, NULL, NULL );
            } /* endif */
          break;
        } /* endswitch */
        break;


      case WM_PAINT:
        {
          PAINTSTRUCT ps;

          BeginPaint( hwnd, &ps );
          EndPaint( hwnd, &ps );
        }
        mResult = DefMDIChildProc( hwnd, msg, mp1, mp2 );
        break;

      case WM_COMMAND:
      case WM_EQF_COMMAND:
        TMMCommand (hwnd, mp1) ;
        if( msg == WM_COMMAND)
        {
          mResult = DefMDIChildProc( hwnd, msg, mp1, mp2 );
        } /* endif */
        break;

      case WM_MDIACTIVATE :
         /************************************************************/
         /* set our menu if gaining the focus ....                   */
         /************************************************************/
         if ( hwnd == (HWND)mp2 )
         {
           pTMMIda = ACCESSWNDIDA( hwnd, PTMMIDA );
           if ( !pTMMIda->hwndMenu )
           {
             HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
             pTMMIda->hwndMenu = LoadMenu( hResMod, MAKEINTRESOURCE(ID_TMM_WINDOW));
             /********************************************************/
             /* the windows pulldown is the 5th entry in the AAB     */
             /*                                                    | */
             /**************************************************** v */
             pTMMIda->hwndWndMenu = GetSubMenu( pTMMIda->hwndMenu, 4 );
           } /* endif */

           SendMessage( (HWND)UtlQueryULong( QL_TWBCLIENT ),
                        WM_MDISETMENU,
                        (WPARAM)pTMMIda->hwndMenu, (LPARAM)pTMMIda->hwndWndMenu );
           UtlSetUShort( QS_CURMENUID, ID_TMM_WINDOW );

           if (pTMMIda->hwndFocus )
           {
             WinPostMsg( hwnd, WM_EQF_SETFOCUS,
                         0, MP2FROMHWND(pTMMIda->hwndFocus));
           } /* endif */
         } /* endif */

         /************************************************************/
         /* set the TWB Main menu if losing the focus ...            */
         /************************************************************/
         if ( hwnd != (HWND)mp2 )
         {
           /************************************************************/
           /* If TM Edit is maximized, so restore it to avoid maximize */
           /* of the other active windows...                           */
           /************************************************************/
           SendMessage( (HWND)UtlQueryULong(QL_TWBCLIENT),
                        WM_MDIGETACTIVE,
                        0, (LONG)&fMaximized );

           if ( fMaximized  )
           {
             SendMessage( (HWND)UtlQueryULong(QL_TWBCLIENT),
                          WM_MDIRESTORE,
                          (WPARAM) hwnd,
                          0L);
           } /* endif */
           /************************************************************/
           /* set the TWB Main menu if losing the focus ...            */
           /************************************************************/
           SendMessage( (HWND)UtlQueryULong( QL_TWBCLIENT ),
                        WM_MDISETMENU,
                        (WPARAM)(HWND) UtlQueryULong( QL_TWBMENU ) ,
                        (LPARAM)(HWND) UtlQueryULong( QL_TWBWINDOWMENU ) );
           UtlSetUShort( QS_CURMENUID, ID_TWBM_WINDOW );
         } /* endif */

         /************************************************************/
         /* draw the menu ...                                        */
         /************************************************************/
         DrawMenuBar( (HWND)UtlQueryULong( QL_TWBFRAME ));

         EqfActivateInstance( hwnd, mp1 );
         break;

      case WM_ACTIVATE:
         EqfActivateInstance( hwnd, (mp1 != WA_INACTIVE) );
         break;

      case WM_CLOSE:
        pTMMIda = ACCESSWNDIDA( hwnd, PTMMIDA );
        if ( pTMMIda )
         pTMMIda->fKill = TRUE;
        WinSendMsg( hwnd, WM_EQF_CLOSE, NULL, NULL );
        break;

      case  WM_EQF_CLOSE:
        pTMMIda = ACCESSWNDIDA( hwnd, PTMMIDA );
        WinStopTimer( WinQueryAnchorBlock(hwnd), hwnd, TMM_TIMERID );
        if ( pTMMIda )
        {
          BOOL   fDummy;

          if ( pTMMIda->TBTargetDoc.pTBSeg )
            SaveSegment( pTMMIda, &fDummy );
          FreeTMIda( pTMMIda->pTMIda );
          TMMFuncQuit(pTMMIda);
        } /* endif */
        EqfActivateInstance( hwnd, FALSE );
        EqfRemoveObject( TWBFORCE, hwnd);
        break;

      case  WM_EQF_TERMINATE:
        pTMMIda = ACCESSWNDIDA( hwnd, PTMMIDA );
        ANCHORWNDIDA( hwnd, NULL );

		if ( pTMMIda )
        {
          BOOL   fDummy;

          if ( pTMMIda->TBTargetDoc.pTBSeg )
            SaveSegment( pTMMIda, &fDummy );
          FreeTMIda( pTMMIda->pTMIda );
          TMMFuncQuit(pTMMIda);
        } /* endif */

        SendMessage( (HWND)UtlQueryULong( QL_TWBCLIENT), WM_MDIDESTROY, MP1FROMHWND(hwnd), 0L );
        break;

      case  WM_TIMER:
        //refresh the window not every segment
        //but every second
        pTMMIda = ACCESSWNDIDA( hwnd, PTMMIDA );
        UPDSTATUSBAR( pTMMIda );
        if ( pTMMIda->fUpdate )
        {
          pTMMIda->fUpdate = FALSE;
          EQFDispWindow ( &pTMMIda->TBSourceDoc );
          EQFDispWindow ( &pTMMIda->TBTargetDoc );
        } /* endif */
        break;

      case WM_EQF_SETFOCUS:
         pTMMIda = ACCESSWNDIDA( hwnd, PTMMIDA );
         pTMMIda->hwndFocus = HWNDFROMMP2(mp2);
         WinSetFocus( HWND_DESKTOP, HWNDFROMMP2(mp2) );
         WinSetActiveWindow( HWND_DESKTOP, HWNDFROMMP2(mp2) );
         break;

      default:
        mResult = DefMDIChildProc( hwnd, msg, mp1, mp2 );
    } /* end switch */

    return mResult ;

} /* TMMWndProc */

//------------------------------------------------------------------------------
// Internal function                                                            
//------------------------------------------------------------------------------
// Function name:     DoVisualTMM                                               
//------------------------------------------------------------------------------
// Function call:     DoVisualTMM(hab, hwnd, pTMMIda)                           
//------------------------------------------------------------------------------
// Description:       start the source and target windows                       
//------------------------------------------------------------------------------
// Parameters:        HAB   hab             anchor                              
//                    HWND  hwnd            window handle                       
//                    PTMMIDA pTMMIda       TMM ida                             
//------------------------------------------------------------------------------
// Returncode type:   BOOL                                                      
//------------------------------------------------------------------------------
// Returncodes:       True/False                                                
//------------------------------------------------------------------------------
// Function flow:     init editing colors and keys                              
//                    set focusline to line number 10                           
//                    register source/target window class                       
//                    register status bar window                                
//                    create both windows                                       
//                    activate aab items                                        
//                    create status bar                                         
//                    return true/false                                         
//------------------------------------------------------------------------------
BOOL DoVisualTMM
(
  HAB hab,
  HWND hwnd,
  PTMMIDA pTMMIda
)
{
  BOOL        fOK = TRUE;                                  // success indicator
  HMENU       hwndMenu;

  fOK =  EQFBInit();                   // init colors and keys

  /********************************************************************/
  /* adjust profile contents to VISITM requirements                   */
  /********************************************************************/
  pTMMIda->sOrgFocusLine = EQFBFuncGetFocusLine();
  EQFBFuncChangeFocusLine ( 3 );

  /********************************************************************/
  /* register our view window class                                   */
  /********************************************************************/
  if ( fOK )
  {
       WNDCLASSW wndclassW;

       InitUnicode();                  // isuue warning if not Unicode system
       wndclassW.style = CS_DBLCLKS;
       wndclassW.lpfnWndProc = TMMVISWNDPROC;
       wndclassW.cbClsExtra = 0;
       wndclassW.cbWndExtra = sizeof(PSZ);
       wndclassW.hInstance = (HINSTANCE)hab;
       wndclassW.hIcon = (HPOINTER) UtlQueryULong(QL_TMMICON); //hiconTMM;
       wndclassW.hCursor = LoadCursor(NULL, IDC_ARROW);
       wndclassW.hbrBackground = (HBRUSH)GetStockObject (WHITE_BRUSH);
       wndclassW.lpszMenuName = NULL;
       wndclassW.lpszClassName = TMM_VISCLASSW;


       RegisterClassW(&wndclassW);
     {
       WNDCLASS wndclass;

       wndclass.style = CS_DBLCLKS;
       wndclass.lpfnWndProc = TMMSTATUSBARWNDPROC;
       wndclass.cbClsExtra = 0;
       wndclass.cbWndExtra = sizeof(PSZ);
       wndclass.hInstance = (HINSTANCE)hab;
       wndclass.hIcon = (HPOINTER) UtlQueryULong(QL_TMMICON); //hiconTMM;
       wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
       wndclass.hbrBackground = (HBRUSH)GetStockObject (WHITE_BRUSH);
       wndclass.lpszMenuName = NULL;
       wndclass.lpszClassName = TMM_STATUSBAR;

       RegisterClass(&wndclass);
     }

    /********************************************************************/
    /* create both windows                                              */
    /********************************************************************/
    TMMDocInit ( pTMMIda );
  } /* endif */

  /********************************************************************/
  /* set the init values for pDoc variables                           */
  /********************************************************************/
  if ( fOK )  fOK = TMMDocStart(hab, hwnd, &(pTMMIda->stVisDocSrc));
  if ( fOK  ) fOK = TMMDocStart(hab, hwnd, &(pTMMIda->stVisDocTgt));

  /********************************************************************/
  /* activate AAB and create status bar...                            */
  /********************************************************************/
  if ( fOK )
  {
    hwndMenu = GetMenu( pTMMIda->hwnd );
    SETAABITEM( hwndMenu, ID_TMM_ABB_FILE, TRUE );
    SETAABITEM( hwndMenu, ID_TMM_ABB_EDIT, TRUE );
    SETAABITEM( hwndMenu, ID_TMM_ABB_OPTIONS, TRUE );
    SETAABITEM( hwndMenu, ID_TMM_ABB_STYLE, TRUE );
  } /* endif */

  if ( fOK )
  {
    SWP     swp;
    SHORT   sCY = (SHORT)WinQuerySysValue( HWND_DESKTOP, SV_CYMENU );
    WinQueryWindowPos( pTMMIda->hwnd, &swp );

    pTMMIda->hStatusBarWnd = WinCreateWindow( pTMMIda->hwnd, TMM_STATUSBAR, "",
                                              WS_CHILD |
                                              FCF_BORDER | WS_VISIBLE,
                                              0, (SHORT)(swp.cy - sCY - 1),
                                              (SHORT)swp.cx, sCY,
                                              hwnd,
                                              HWND_TOP, ID_TMM_STATUSBAR,
                                              NULL, NULL);

    if ( pTMMIda->hStatusBarWnd != NULLHANDLE )
    {
      ANCHORWNDIDA( pTMMIda->hStatusBarWnd, pTMMIda );
    } /* endif */
  } /* endif */

  if ( fOK )
  {
    RECT rect;
    GetClientRect( pTMMIda->hwnd, &rect );
    pTMMIda->fHorizontal = TRUE;
    TMMSetWindowPos( pTMMIda, (SHORT)(rect.right - rect.left),
                              (SHORT)(rect.bottom - rect.top ) );
    WinSetActiveWindow(HWND_DESKTOP, pTMMIda->TBTargetDoc.hwndFrame );
    pTMMIda->usActWin = STARGET_DOC;
  } /* endif */

  return( fOK );

} /* end of function DoVisualTMM */

//------------------------------------------------------------------------------
// Internal function                                                            
//------------------------------------------------------------------------------
// Function name:     TMMDocInit                                                
//------------------------------------------------------------------------------
// Function call:     TMMDocInit(pTMMIda)                                       
//------------------------------------------------------------------------------
// Description:       init source/target doc structures                         
//------------------------------------------------------------------------------
// Parameters:        PTMMIDA   pTMMIda                                         
//------------------------------------------------------------------------------
// Returncode type:   BOOL                                                      
//------------------------------------------------------------------------------
// Returncodes:       TRUE   success                                            
//                    FALSE  error occurred                                     
//------------------------------------------------------------------------------
// Function flow:     set all pDoc ptrs in Visdoc structure                     
//                    set pDoc next, previous, twin                             
//                    return true/false                                         
//------------------------------------------------------------------------------
VOID TMMDocInit
(
  PTMMIDA   pTMMIda
)
{

  pTMMIda->stVisDocTgt.pTMMIda = pTMMIda->stVisDocSrc.pTMMIda = pTMMIda;
  pTMMIda->stVisDocSrc.pDoc = &pTMMIda->TBSourceDoc;
  pTMMIda->stVisDocTgt.pDoc = &pTMMIda->TBTargetDoc;

  pTMMIda->stVisDocSrc.pDoc->docType = SSOURCE_DOC;
  pTMMIda->stVisDocTgt.pDoc->docType = STARGET_DOC;
  pTMMIda->TBSourceDoc.twin = &(pTMMIda->TBTargetDoc);
  pTMMIda->TBTargetDoc.twin = &(pTMMIda->TBSourceDoc);
  pTMMIda->TBSourceDoc.next = &(pTMMIda->TBTargetDoc);
  pTMMIda->TBTargetDoc.next = &(pTMMIda->TBSourceDoc);
  pTMMIda->TBSourceDoc.prev = &(pTMMIda->TBTargetDoc);
  pTMMIda->TBTargetDoc.prev = &(pTMMIda->TBSourceDoc);

} /* end of function TMMDocInit */

//------------------------------------------------------------------------------
// Internal function                                                            
//------------------------------------------------------------------------------
// Function name:     TMMDocStart                                               
//------------------------------------------------------------------------------
// Function call:     TMMDocStart(hab, hwnd, pstVisDoc)                         
//------------------------------------------------------------------------------
// Description:       create Visdoc window                                      
//------------------------------------------------------------------------------
// Parameters:        HAB   hab       anchor block                              
//                    HWND  hwnd      window handle                             
//                    PTMMVISDOC pstVisDoc  ptr to Visdoc structure             
//------------------------------------------------------------------------------
// Returncode type:   BOOL                                                      
//------------------------------------------------------------------------------
// Returncodes:       TRUE     success                                          
//------------------------------------------------------------------------------
// Function flow:     create window                                             
//                    if ok                                                     
//                      anchor structure                                        
//                      position the window                                     
//                      prepare the AVIO stuff                                  
//                      allocate buffer and Blockmark                           
//                    if still ok                                               
//                      size window                                             
//                      position at top of document                             
//                      load tag table                                          
//                      position window in horizontal position                  
//                    return success indicator                                  
//------------------------------------------------------------------------------
BOOL TMMDocStart
(
  HAB     hab,
  HWND    hwnd,
  PTMMVISDOC pstVisDoc
)
{
  ULONG flFrameFlags = FCF_VERTSCROLL  | FCF_HORZSCROLL | FCF_DLGBORDER |
                       FCF_TITLEBAR ;
  PVIOFONTCELLSIZE pVioFont;               // pointer to font
  BOOL          fOK = TRUE;                // success indicator
  PTBDOCUMENT   pTBDoc;
  PTMMIDA       pTMMIda = NULL;
  PSZ_W         pString;
  PSZ           pszTitle;
  CHAR_W chTitleW[256];
  PSZ_W  pTitleW;

  hab;
  pTBDoc = pstVisDoc->pDoc;

  if ( pTBDoc->docType == SSOURCE_DOC )
  {
    pszTitle = chSourceFile;
  }
  else
  {
    pszTitle = chTargetFile;
  } /* endif */

  pTMMIda = (PTMMIDA)pstVisDoc->pTMMIda;
  // add source and target language to determine if we have BIDI or not ...
  pTBDoc->usLangTypeTgt = MorphGetLanguageType( pTMMIda->pTMIda->szTargetLng );
  pTBDoc->usLangTypeSrc = MorphGetLanguageType( pTMMIda->pTMIda->szSourceLng );
  // use system preferences default language here!
  pTBDoc->ulOemCodePage = GetLangCodePage( OEM_CP, NULL);
  pTBDoc->ulAnsiCodePage = GetLangCodePage( ANSI_CP, NULL);

  /********************************************************/
  /* allow for DBCS input methods ...                     */
  /********************************************************/
  pTBDoc->hlfIME = GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE,
										(LONG)sizeof(LOGFONT));

  pTitleW = ASCII2Unicode(pszTitle,chTitleW, 0L);
  pTBDoc->hwndClient = pTBDoc->hwndFrame =
        CreateWindowExW ( 0,// WS_EX_RTLREADING,
                          TMM_VISCLASSW,
                          pTitleW,    //window caption
                          WS_CHILD |
                          WS_CLIPSIBLINGS |
                          flFrameFlags,
                          CW_USEDEFAULT,
                          CW_USEDEFAULT,
                          CW_USEDEFAULT,
                          CW_USEDEFAULT,
                          hwnd, //parent wnd handle
                          NULL,
                          (HINSTANCE)UtlQueryULong( QL_HAB ),
                          NULL );

  if ( pTBDoc->hwndFrame )
  {
    /********************************************************************/
    /* anchor our TMM ida and activate frame subproc                    */
    /********************************************************************/
    ANCHORWNDIDA( pTBDoc->hwndClient, pstVisDoc );
    SETWINDOWID( pTBDoc->hwndFrame, ID_TMM_DOCWINDOW );
    //allocate buffer for initial dummy segment
    fOK = UtlAlloc( (PVOID *)&pString, 0L, (LONG)MIN_ALLOC* sizeof(CHAR_W), ERROR_STORAGE );

    if ( fOK )
    {
      PSZ_W pTemp = tbInitSegment.pDataW;
      tbInitSegment.pDataW = pString;
      EQFBAddSegW( pTBDoc, &tbInitSegment );      // add dummy segment
      tbInitSegment.pDataW = pTemp;

      WinSetWindowText( pTBDoc->hwndFrame, pszTitle );

      /**************************************************************/
      /* anchor our data pointers                                   */
      /**************************************************************/
      pTBDoc->pTMMaint   = pTMMIda;
      pTBDoc->pfnTMMaint = (PFN)TMEditWork;

      /******************************************************************/
      /* prepare the avio stuff....                                     */
      /******************************************************************/
      // without this call the font size maybe 0 before open document
      EQFBReadProfile();

      pVioFont = get_vioFontSize();

      EQFBSetNewCellSize( pTBDoc, (pVioFont + pTBDoc->docType)->cx,
                           (pVioFont + pTBDoc->docType)->cy );

      fOK = UtlAlloc( (PVOID *)&pTBDoc->pTokBuf, 0L, (LONG) TOK_BUFFER_SIZE,
                      ERROR_STORAGE );
    } /* endif */

    if ( fOK )                    // allocate buffer for marking area
    {
      fOK = UtlAlloc( (PVOID *)&pTBDoc->pBlockMark, 0L,
                      (LONG) max( sizeof( EQFBBLOCK ), MIN_ALLOC) ,
                      ERROR_STORAGE );
    } /* endif */

    if ( fOK && (pTMMIda->pTMIda->szTagTable[0] != EOS) )
    {
      fOK = TALoadTagTable( pTMMIda->pTMIda->szTagTable,
                            (PLOADEDTABLE *)&pTMMIda->TBSourceDoc.pDocTagTable,
                            FALSE, TRUE ) == NO_ERROR;
    } /* endif */

    /******************************************************************/
    /* Note: we load the tag table twice to have the correct use      */
    /*       count in the TagTable handling functions                 */
    /******************************************************************/
    if ( fOK && (pTMMIda->pTMIda->szTagTable[0] != EOS) )
    {
      fOK = TALoadTagTable( pTMMIda->pTMIda->szTagTable,
                            (PLOADEDTABLE *)&pTMMIda->TBTargetDoc.pDocTagTable,
                            FALSE, TRUE ) == NO_ERROR;
    } /* endif */

    if ( fOK )
    {
      strcpy( pTMMIda->TBTargetDoc.szDocName, pTMMIda->pTMIda->szMemShortName );
      strcpy( pTMMIda->TBSourceDoc.szDocName, pTMMIda->TBTargetDoc.szDocName );
    } /* endif */

    if ( fOK )
    {
      fOK = UtlAlloc( (PVOID *)&pTBDoc->pInBuf, 0L, (LONG) IO_BUFFER_SIZE,
                      ERROR_STORAGE );
    } /* endif */

    if ( fOK )
    {
      fOK = UtlAlloc( (PVOID *)&pTBDoc->pUndoSegW, 0L,
                      (LONG)MAX_SEGMENT_SIZE * sizeof(CHAR_W) ,ERROR_STORAGE);
    } /* endif */

    if ( fOK )
    {
      fOK = UtlAlloc((PVOID *) &(pTBDoc->pEQFBWorkSegmentW), 0L,
                     (LONG) MAX_SEGMENT_SIZE * sizeof(CHAR_W), ERROR_STORAGE );
    }

    if ( fOK )
    {
      fOK = UtlAlloc((PVOID *) &(pTBDoc->pSegmentBufferW),
                     0L, (LONG)(MAX_SEGMENT_SIZE + 1) * sizeof(CHAR_W),
                     ERROR_STORAGE);
    }

    if ( fOK )
    {
      pTBDoc->fUndoState = FALSE;           //init fUndoState
      pTBDoc->usUndoSegOff = 0;
    } /* endif */
  }
  else
  {
    fOK = FALSE;
  } /* endif */

   return (fOK);

} /* end of function TMMDocStart */

//------------------------------------------------------------------------------
// External function                                                            
//------------------------------------------------------------------------------
// Function name:     TMMVertWindowPos                                          
//------------------------------------------------------------------------------
// Function call:     TMMVertWindowPos(pTBDoc, hwnd, swp)                       
//------------------------------------------------------------------------------
// Description:       position visdoc windows vertically                        
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT  pTBDoc,                                      
//                    HWND         hwnd,                                        
//                    SWP          swp                                          
//------------------------------------------------------------------------------
// Returncode type:   VOID                                                      
//------------------------------------------------------------------------------
// Returncodes:       none                                                      
//------------------------------------------------------------------------------
// Function flow:     set size & position of src and tgt window                 
//------------------------------------------------------------------------------

VOID TMMVertWindowPos
(
  PTBDOCUMENT  pTBDoc,
  HWND         hwnd,
  SWP          swp
)
{
  USHORT  usCY = (USHORT) WinQuerySysValue( HWND_DESKTOP, SV_CYMENU );
  if ( pTBDoc->docType == SSOURCE_DOC )
  {
    WinSetWindowPos( pTBDoc->hwndFrame, // Set size and pos
                     hwnd,
                     0,
                     usCY,
                     (USHORT)(swp.cx/2),
                     (USHORT)(swp.cy - usCY),
                     EQF_SWP_SIZE | EQF_SWP_MOVE | EQF_SWP_SHOW );
  }
  else
  {
    WinSetWindowPos( pTBDoc->hwndFrame, // Set size and pos
                     hwnd,
                     (USHORT)(swp.cx/2),
                     usCY,
                     (USHORT)(swp.cx/2),
                     (USHORT)(swp.cy - usCY),
                     EQF_SWP_SIZE | EQF_SWP_MOVE | EQF_SWP_SHOW );
  } /* endif */
  return;
} /* end of function TMMVertWindowPos */

//------------------------------------------------------------------------------
// External function                                                            
//------------------------------------------------------------------------------
// Function name:     TMMSetWindowPos                                           
//------------------------------------------------------------------------------
// Function call:     TMMSetWindowPos( pTMMIda, cx, cy)                         
//------------------------------------------------------------------------------
// Description:       position TMM windows inside client area                   
//------------------------------------------------------------------------------
// Parameters:        PTMMIDA      pIda,                                        
//                    SHORT        cx,                                          
//                    SHORT        cy                                           
//------------------------------------------------------------------------------
// Returncode type:   VOID                                                      
//------------------------------------------------------------------------------
// Returncodes:       none                                                      
//------------------------------------------------------------------------------
// Function flow:     set size & position of src/tgt windows and status         
//------------------------------------------------------------------------------
VOID TMMSetWindowPos
(
  PTMMIDA pIda,
  SHORT   cx,
  SHORT   cy
)
{
  /*******************************************************************/
  /* usCY statusbar height                                           */
  /*******************************************************************/
  SHORT sCY = (SHORT) WinQuerySysValue( HWND_DESKTOP, SV_CYMENU );

 if ( pIda->fHorizontal )
 {
   SetWindowPos( pIda->TBSourceDoc.hwndFrame, pIda->hwnd,
                 0, 0, cx, (cy - sCY) / 2,
                 SWP_NOZORDER | SWP_SHOWWINDOW | EQF_SWP_NOADJUST |
                 SWP_NOACTIVATE );
   SetWindowPos( pIda->TBTargetDoc.hwndFrame, pIda->hwnd,
                 0, (cy - sCY) / 2, cx, (cy - sCY) / 2,
                 SWP_NOZORDER | SWP_SHOWWINDOW | EQF_SWP_NOADJUST |
                 SWP_NOACTIVATE );
   SetWindowPos( pIda->hStatusBarWnd, pIda->hwnd,
                 0, cy - sCY,
                 cx, sCY,
                 SWP_NOZORDER | SWP_SHOWWINDOW | EQF_SWP_NOADJUST |
                 SWP_NOACTIVATE );
 }
 else
 {
   SetWindowPos( pIda->TBSourceDoc.hwndFrame, pIda->hwnd,
                 0, 0,
                 (USHORT)(cx/2),
                 cy - sCY,
                 SWP_NOZORDER | SWP_SHOWWINDOW | EQF_SWP_NOADJUST |
                 SWP_NOACTIVATE );
   SetWindowPos( pIda->TBTargetDoc.hwndFrame, pIda->hwnd,
                 (USHORT)(cx/2),
                 0,
                 (USHORT)(cx/2),
                 cy - sCY,
                 SWP_NOZORDER | SWP_SHOWWINDOW | EQF_SWP_NOADJUST |
                 SWP_NOACTIVATE );
   SetWindowPos( pIda->hStatusBarWnd, pIda->hwnd,
                 0, cy - sCY,
                 cx, sCY,
                 SWP_NOZORDER | SWP_SHOWWINDOW | EQF_SWP_NOADJUST |
                 SWP_NOACTIVATE );
 } /* endif */
  UPDSTATUSBAR( pIda );
  return;
} /* end of function TMMSetWindowPos */

//------------------------------------------------------------------------------
// External function                                                            
//------------------------------------------------------------------------------
// Function name:     TMMTile(pTMMIda)                                          
//------------------------------------------------------------------------------
// Function call:     TMMTile(pTMMIda, fHorizontal)                             
//------------------------------------------------------------------------------
// Description:       toggle positioning of both visdoc windows                 
//------------------------------------------------------------------------------
// Parameters:        PTMMIDA   pTMMIda,                                        
//                    BOOL      fHorizontal                                     
//------------------------------------------------------------------------------
// Returncode type:   void                                                      
//------------------------------------------------------------------------------
// Returncodes:       none                                                      
//------------------------------------------------------------------------------
// Function flow:     if windows should not be horizontally                     
//                      set to vertical visdoc windows                          
//                    else                                                      
//                      set windows horizontally                                
//                    paint whole window again                                  
//------------------------------------------------------------------------------
VOID
TMMTile
(
  PTMMIDA pTMMIda,
  BOOL fHorizontal
)
{
  PTMMVISDOC pVisDoc;

  pVisDoc = ACCESSWNDIDA( pTMMIda->hwndFocus, PTMMVISDOC );
  pTMMIda->fHorizontal = fHorizontal;

  {

    RECT rect;

    GetClientRect( pTMMIda->hwnd, &rect );
    TMMSetWindowPos( pTMMIda, (SHORT)(rect.right - rect.left),
                              (SHORT)(rect.bottom - rect.top) );
  }

  if ( pVisDoc )
  {
    WinSetActiveWindow( HWND_DESKTOP, pVisDoc->pDoc->hwndFrame );
  } /* endif */

  WinShowWindow( pTMMIda->hwnd, FALSE );
  WinSendMsg( pTMMIda->hwnd, WM_PAINT, NULL, NULL );
  WinShowWindow( pTMMIda->hwnd, TRUE );
  return;
} /* end of function TMMTile */

//------------------------------------------------------------------------------
// External function                                                            
//------------------------------------------------------------------------------
// Function name:     TMMFuncQuit                                               
//------------------------------------------------------------------------------
// Function call:     TMMFuncQuit(pTMMIda, hwnd, hab)                           
//------------------------------------------------------------------------------
// Description:       quit the visualization                                    
//                    (e.g. user selected Quit)                                 
//------------------------------------------------------------------------------
// Parameters:        PTMMIDA   pTMMIda                                         
//                    HWND      hwnd,                                           
//                    HAB       hab                                             
//------------------------------------------------------------------------------
// Returncode type:   VOID                                                      
//------------------------------------------------------------------------------
// Returncodes:       none                                                      
//------------------------------------------------------------------------------
// Function flow:     if user made changes                                      
//                      ask user whether to quit or not                         
//                    if user wants to quit                                     
//                       free allocated areas                                   
//                       close both windows                                     
//------------------------------------------------------------------------------
VOID
TMMFuncQuit
(
  PTMMIDA pTMMIda
)
{

  //free all allocated space
  if ( pTMMIda->TBSourceDoc.pTBSeg )
    FreeTMMDoc( &pTMMIda->TBSourceDoc );
  if ( &pTMMIda->TBTargetDoc.pTBSeg )
    FreeTMMDoc( &pTMMIda->TBTargetDoc );

  WinPostMsg(pTMMIda->TBSourceDoc.hwndFrame, WM_CLOSE,
                MP1FROMSHORT(TRUE), NULL);
  WinPostMsg(pTMMIda->TBTargetDoc.hwndFrame, WM_CLOSE,
                MP1FROMSHORT(TRUE), NULL);

  // restore original focus line settings
  EQFBFuncChangeFocusLine ( pTMMIda->sOrgFocusLine );

//if ( pTMMIda->TBTargetDoc.pUserSettings )
//  UtlAlloc( &pTMMIda->TBTargetDoc.pUserSettings, 0L, 0L, NOMSG );
//if ( pTMMIda->TBSourceDoc.pUserSettings )
//  UtlAlloc( &pTMMIda->TBSourceDoc.pUserSettings, 0L, 0L, NOMSG );
  if ( pTMMIda->pTMParms )
    UtlAlloc( (PVOID *)&pTMMIda->pTMParms, 0L, 0L, NOMSG );
  if ( pTMMIda->pTMIda )
    UtlAlloc( (PVOID *)&pTMMIda->pTMIda, 0L, 0L, NOMSG );
  if ( pTMMIda )
  {
    ANCHORWNDIDA( pTMMIda->hwnd, 0L );
    UtlAlloc( (PVOID *)&pTMMIda, 0L, 0L, NOMSG );
  }
  return;
} /* end of function TMMFuncQuit */

//------------------------------------------------------------------------------
// External function                                                            
//------------------------------------------------------------------------------
// Function name:     FreeForNext                                               
//------------------------------------------------------------------------------
// Function call:     fOK = FreeForNext( pTMMIda )                              
//------------------------------------------------------------------------------
// Description:       free not needed memory to get next set of entries         
//------------------------------------------------------------------------------
// Parameters:        PITMIDA   pITMIda                                         
//------------------------------------------------------------------------------
// Returncode type:   BOOL                                                      
//------------------------------------------------------------------------------
// Returncodes:       TRUE/FALSE                                                
//------------------------------------------------------------------------------
// Function flow:                                                               
//------------------------------------------------------------------------------
BOOL  FreeForNext
(
  PTMMIDA pTMMIda
)
{
  USHORT   usMBID = MBID_YES;
  BOOL     fOK = TRUE;

  if ( pTMMIda->TBTargetDoc.EQFBFlags.workchng )
  {
    usMBID = UtlError( TB_QUIT, MB_YESNO, 0, NULL, EQF_ERROR);
  } /* endif */

  if ( usMBID == MBID_YES )
  {
    //free all allocated space
    FreeTMMDoc( &pTMMIda->TBSourceDoc );
    FreeTMMDoc( &pTMMIda->TBTargetDoc );
  } /* endif */

  pTMMIda->pTMIda->fStop = FALSE;
  pTMMIda->pTMIda->ulRunNum = 0;
  pTMMIda->ulSrcSegNum = 1;
  pTMMIda->ulTgtSegNum = 1;

  TMMDocInit ( pTMMIda );

  if ( fOK )
  {
    fOK = TMMDocNext( &(pTMMIda->stVisDocSrc) );
  } /* endif */

  if ( fOK )
  {
    fOK = TMMDocNext( &(pTMMIda->stVisDocTgt) );
  } /* endif */

  return( fOK );

} /* end of function FreeForNext */

//------------------------------------------------------------------------------
// Internal function                                                            
//------------------------------------------------------------------------------
// Function name:     FreeTMMDoc                                                
//------------------------------------------------------------------------------
// Function call:     FreeTMMDoc( pDoc );                                       
//------------------------------------------------------------------------------
// Description:       Free the allocated space for the document structure       
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT  pDoc   pointer to document instance data     
//------------------------------------------------------------------------------
// Returncode type:   VOID                                                      
//------------------------------------------------------------------------------
// Function flow:     free buffers for input and tokens                         
//                    loop through seg table and free all these buffer          
//                    free segtable and undo segment                            
//------------------------------------------------------------------------------
VOID FreeTMMDoc
(
  PTBDOCUMENT pDoc
)
{
  PTBSEGMENTTABLE pSegTable;          // ptr for segment table deleting
  ULONG           ulI, ulJ;           // loop counter
  PTBSEGMENT      pSegment;           // ptr for segment deleting

  if ( pDoc->pTokBuf )
    UtlAlloc( (PVOID *)&pDoc->pTokBuf, 0L, 0L, NOMSG );
  if ( pDoc->pBlockMark )
    UtlAlloc( &pDoc->pBlockMark, 0L, 0L, NOMSG );

  pSegTable = pDoc->pSegTables;
  for ( ulI = 0; ulI < pDoc->ulSegTables; ulI++ )
  {
     pSegment = pSegTable->pSegments;
     for ( ulJ = 0; ulJ < pSegTable->ulSegments; ulJ++ )
     {
        if ( pSegment->pData )
        {
           UtlAlloc( (PVOID *)&pSegment->pData, 0L, 0L, NOMSG );
        } /* endif */
        if ( pSegment->pDataW )
        {
          // GQ-Fix: if pDataW is identical to pDoc->pEQFBWorkSegmentW clear
          //         pDoc->pEQFBWorkSegmentW to avoid duplicate free of memory area
          if ( pDoc->pEQFBWorkSegmentW == pSegment->pDataW )
          {
            pDoc->pEQFBWorkSegmentW = NULL;
          } /* endif */
          UtlAlloc( (PVOID *)&pSegment->pDataW, 0L, 0L, NOMSG );
        } /* endif */
        if ( pSegment->pusBPET )
        {
           UtlAlloc( (PVOID *)&pSegment->pusBPET, 0L, 0L, NOMSG );
        } /* endif */
        if ( pSegment->pusHLType )
        {
           UtlAlloc( (PVOID *)&pSegment->pusHLType, 0L, 0L, NOMSG );
        } /* endif */
        if (pSegment->pContext) UtlAlloc((PVOID *)&(pSegment->pContext),0L,0L,NOMSG);
        pSegment++;
     } /* endfor */
     UtlAlloc( (PVOID *)&pSegTable->pSegments, 0L, 0L, NOMSG );
     pSegTable++;
  } /* endfor */
  pDoc->ulSegTables = 0;
  if ( pDoc->pDocTagTable != NULL )
  {
    TAFreeTagTable( (PLOADEDTABLE)pDoc->pDocTagTable );
  } /* endif */
  UtlAlloc( (PVOID *)&pDoc->pSegTables, 0L, 0L, NOMSG );
  if (pDoc->pUndoSeg) UtlAlloc( (PVOID *)&pDoc->pUndoSeg, 0L, 0L, NOMSG );             //free storage of Undo
  if (pDoc->pUndoSegW) UtlAlloc( (PVOID *)&pDoc->pUndoSegW, 0L, 0L, NOMSG );
  if (pDoc->pEQFBWorkSegmentW)  UtlAlloc((PVOID *) &(pDoc->pEQFBWorkSegmentW), 0L, 0L, NOMSG );
  if (pDoc->pSegmentBufferW) UtlAlloc((PVOID *) &(pDoc->pSegmentBufferW), 0L, 0L, NOMSG );
  if (pDoc->pContext )  UtlAlloc((PVOID *) &pDoc->pContext, 0L, 0L, NOMSG );
	 /********************************************************/
	 /* free memory allocated for DBCS input methods ...     */
	 /********************************************************/

	if ( pDoc->hlfIME )
	 {
       ImeMoveConvertWin( pDoc, pDoc->hwndClient, -1, -1);
	   GlobalFree(pDoc->hlfIME);
	   pDoc->hlfIME = 0;
	 } /* endif */

} /* end of function FreeTMMDoc */

//------------------------------------------------------------------------------
// Internal function                                                            
//------------------------------------------------------------------------------
// Function name:     TMMCommand                                                
//------------------------------------------------------------------------------
// Function call:     _                                                         
//------------------------------------------------------------------------------
// Description:       handle all commands of main TM edit window                
//                    (menuitem commands)                                       
//------------------------------------------------------------------------------
// Parameters:        HWND hwnd,                                                
//                    MPARAM mp1                                                
//------------------------------------------------------------------------------
// Returncode type:   void                                                      
//------------------------------------------------------------------------------
// Returncodes:       none                                                      
//------------------------------------------------------------------------------
// Function flow:     ID_TMM_EXIT: quit visualization without save to memory    
//                    ID_TMM_SAVE: save to memory and quit                      
//                    ID_TMM_NEXT: load next lot of segments                    
//                    ID_TMM_TOGGLEDOC: set focus to other window               
//                    ID_TMM_FIND: find and change                              
//                    ID_TMM_CUT: cut at cursor position                        
//                    ID_TMM_COPY: copy at cursor position                      
//                    ID_TMM_PASTE: paste at cursor position                    
//                    ID_TMM_JOIN:join line                                     
//                    ID_TMM_SPLIT: split line                                  
//                    ID_TMM_UNMARK: unmark block                               
//                    ID_TMM_DEL_ALL: delete all segment in window from TM      
//                    ID_TMM_DEL_CUR: delete active segment in window from TM   
//                    ID_TMM_FONTCOL: set fonts & colors                        
//                    ID_TMM_FONTSIZES: set font sizes                          
//                    ID_TMM_HORIZONTAL: arrange windows horizontally           
//                    ID_TMM_VERTICAL: arrange windows vertically               
//                    ID_TMM_PROOFSEG: spell check for active segment           
//                    ID_TMM_PROOFALL: spell check for whole file               
//                    ID_TMM_PROTECT: set protected style                       
//                    ID_TMM_UNPROTECT: set unprotected style                   
//                    ID_TMM_HIDE: set shrink style                             
//                    ID_TMM_COMPACT: set compact style                         
//------------------------------------------------------------------------------
void TMMCommand
(
 HWND    hwnd,
 WPARAM  mp1
)
{
  PTMMIDA     pTMMIda;
  HMENU       hwndMenu;
  PTMMVISDOC  pVisDoc;
  PTBDOCUMENT pDoc;
  BOOL        fOK = TRUE;
  PTBDOCUMENT pTargetDoc;
  ULONG       ulSegNum;

  pTMMIda = ACCESSWNDIDA( hwnd, PTMMIDA );
  hwndMenu = GETMENU( pTMMIda->hwnd );
  pVisDoc = ACCESSWNDIDA( pTMMIda->hwndFocus, PTMMVISDOC );
  if ( pVisDoc )
  {
    pDoc = pVisDoc->pDoc;
  }
  else
  {
    pDoc = NULL;
  } /* endif */

  switch ( SHORT1FROMMP1( mp1 ) )
  {
    case ID_TMM_EXIT:
      WinPostMsg( hwnd, WM_CLOSE, NULL, NULL );
      break;
    case ID_TMM_SAVE:
      {
        BOOL fDummy;
        fOK = SaveSegment( pTMMIda, &fDummy );
        if ( fOK )
        {
          BackToQuery( pTMMIda, hwnd );
        }
        else
        {
          // give error message and then close
          WinPostMsg( hwnd, WM_CLOSE, NULL, NULL );
        } /* endif */
      }
      break;
    case  ID_TMM_NEXT:
      NextPrevious( pTMMIda, hwnd, TRUE, FALSE );
      break;
    case  ID_TMM_PREVIOUS:
      NextPrevious( pTMMIda, hwnd, FALSE, FALSE );
      break;
    case ID_TMM_TOGGLEDOC:
      if ( pDoc )
      {
        if ( pDoc->docType == STARGET_DOC )
        {
          WinSetActiveWindow( HWND_DESKTOP,
                              pTMMIda->TBSourceDoc.hwndFrame );
          pTMMIda->usActWin = SSOURCE_DOC;
        }
        else
        {
          WinSetActiveWindow( HWND_DESKTOP,
                              pTMMIda->TBTargetDoc.hwndFrame );
          pTMMIda->usActWin = STARGET_DOC;
        } /* endif */
      } /* endif */
      break;
    case  ID_TMM_UNDO:
      if ( pDoc )
      {
        if ( (pDoc->TBCursor.ulSegNum == pDoc->tbActSeg.ulSegNum) &&
             (pTMMIda->pTMIda->ulRunNum > 0) &&
             (pDoc->docType == STARGET_DOC) )
        {
          EQFBFuncUndo( pDoc );
        }
        else
          WinAlarm( HWND_DESKTOP, WA_WARNING );
      } /* endif */
      break;
    case ID_TMM_FIND:
      if ( pDoc )
      {
        if ( pDoc->docType == SSOURCE_DOC )
        {

          ULONG  ulDummy;
          //disable change button on dialog
          ulDummy = pDoc->tbActSeg.ulSegNum;
          pDoc->tbActSeg.ulSegNum = 0;
          EQFBFuncFind( pDoc );
          pDoc->tbActSeg.ulSegNum = ulDummy;
        }
        else
        {
          if ( pDoc->pTBSeg )
          {
            BOOL  fDummy;
            SaveSegment( pTMMIda, &fDummy );
            EQFBWorkSegIn(pDoc);
          } /* endif */

          pDoc->EQFBFlags.PostEdit = TRUE;
          if ( pDoc->pTBSeg->SegFlags.Current )
          {
            //set so not marked as active
            pDoc->pTBSeg->qStatus = QF_XLATED;
            pDoc->pTBSeg->SegFlags.Current = FALSE;
          } /* endif */
          /**************************************************************/
          /* call find function                                         */
          /**************************************************************/
          EQFBFuncFind( pDoc );
          /**************************************************************/
          /* now write all changed == bookmarked segs into memory       */
          /**************************************************************/
          SaveChangedSegs(pTMMIda);
          pDoc->EQFBFlags.PostEdit = FALSE;
          /**************************************************************/
          /* adjust other document to be at same place                  */
          /**************************************************************/
          pDoc->twin->TBCursor.ulSegNum =
                   pDoc->TBCursor.ulSegNum;          // current seg num

          pDoc->ulWorkSeg = pDoc->twin->ulWorkSeg = pDoc->TBCursor.ulSegNum;

          fOK = ActivateSegment( pTMMIda, pDoc->TBCursor.ulSegNum );
          if ( !fOK )
          {
            WinPostMsg( hwnd, WM_CLOSE, NULL, NULL );
          }
          else
          {
            EQFBGotoSeg( pDoc, pDoc->TBCursor.ulSegNum,
                         pDoc->TBCursor.usSegOffset);
            EQFBGotoSeg( pDoc->twin,
                         pDoc->twin->TBCursor.ulSegNum,
                         pDoc->twin->TBCursor.usSegOffset);
          } /* endif */
        } /* endif */
      } /* endif */
      break;
    case ID_TMM_CUT:
      if ( pDoc && (pDoc->docType == STARGET_DOC) )
      {
        EQFBFuncCutToClip( pDoc );
      }
      else
        WinAlarm( HWND_DESKTOP, WA_WARNING );
      break;
    case ID_TMM_COPY:
      // copy to clipboard
      if ( pDoc )
      {
        EQFBFuncCopyToClip( pDoc );
      } /* endif */
      break;
    case ID_TMM_PASTE:
      if ( pDoc &&
           (pDoc->TBCursor.ulSegNum == pDoc->tbActSeg.ulSegNum) &&
           (pTMMIda->pTMIda->ulRunNum > 0) &&
           (pDoc->docType == STARGET_DOC) )
      {
        EQFBFuncPasteFromClip( pDoc );
      }
      else
      {
        WinAlarm( HWND_DESKTOP, WA_WARNING );
      } /* endif */
      break;
    case ID_TMM_SPLIT:
      if ( pDoc && (pDoc->docType == STARGET_DOC) )
      {
        ULONG  ulLen;
        PSZ_W pWorkArea= NULL;
        if ( (pDoc->TBCursor.ulSegNum == pDoc->tbActSeg.ulSegNum) &&
             (pTMMIda->pTMIda->ulRunNum > 0) )
        {
          if ( UtlAlloc( (PVOID *)&pWorkArea, 0L, (LONG)(MAX_SEGMENT_SIZE + 1)* sizeof(CHAR_W),
                         ERROR_STORAGE ))
          {
            UTF16strcpy( pWorkArea, pDoc->pTBSeg->pDataW );

            EQFBFuncSplitLine( pDoc );

            //check for trailing LF in target segment
            if ( UTF16strcmp( pDoc->pTBSeg->pDataW, pWorkArea ) != 0 )
            {
              if ( pDoc->pTBSeg->qStatus != QF_NOP )
              {
                ulLen = UTF16strlenCHAR( pDoc->pTBSeg->pDataW );
                if ( *(pDoc->pTBSeg->pDataW+ulLen-1) == LF )
                {
                  //there is at least 1 so delete it
                  DelTrailingLF( pDoc );
                }
                else
                {
                  //there are no LFs so add one to source to adjust alignment
                  AddLFAndUpdate( pDoc->twin );
                } /* endif */
              } /* endif */
            } /* endif */
          }
          else
          {
            WinAlarm( HWND_DESKTOP, WA_WARNING );
          } /* endif */
          if ( pWorkArea )
          {
            UtlAlloc( (PVOID *)&(pWorkArea), 0L, 0L, NOMSG );
          } /* endif */
        }
        else
        {
          WinAlarm( HWND_DESKTOP, WA_WARNING );
        } /* endif */
      }
      else
      {
        WinAlarm( HWND_DESKTOP, WA_WARNING );
      } /* endif */
      break;
    case ID_TMM_JOIN:
      if ( pDoc && (pDoc->docType == STARGET_DOC) )
      {
        ULONG ulLen;
        PSZ_W pWorkArea = NULL;
        if ( pDoc->TBCursor.ulSegNum == pDoc->tbActSeg.ulSegNum &&
             (pTMMIda->pTMIda->ulRunNum > 0) )
        {
          if ( UtlAlloc( (PVOID *)&pWorkArea, 0L, (LONG)(MAX_SEGMENT_SIZE + 1)*sizeof(CHAR_W),
                         ERROR_STORAGE ))
          {
            UTF16strcpy( pWorkArea, pDoc->pTBSeg->pDataW );
            EQFBFuncJoinLine( pDoc );

            //check for trailing LF in source segment
            if ( UTF16strcmp( pDoc->pTBSeg->pDataW, pWorkArea ) != 0 )
            {
              if ( pDoc->pTBSeg->qStatus != QF_NOP )
              {
                ulLen = UTF16strlenCHAR( pDoc->twin->tbActSeg.pDataW );
                if ( *(pDoc->twin->tbActSeg.pDataW+ulLen-1) == LF )
                {
                  //there is at least 1 so delete it
                  DelTrailingLF( pDoc->twin );
                }
                else
                {
                  //there are no LFs so add one to target to adjust alignment
                  AddLFAndUpdate( pDoc );
                } /* endif */
              } /* endif */
            } /* endif */
          }
          else
          {
            WinAlarm( HWND_DESKTOP, WA_WARNING );
          } /* endif */
          if ( pWorkArea )
          {
            UtlAlloc( (PVOID *)&(pWorkArea), 0L, 0L, NOMSG );
          } /* endif */
        }
        else
        {
          WinAlarm( HWND_DESKTOP, WA_WARNING );
        } /* endif */
      }
      else
      {
        WinAlarm( HWND_DESKTOP, WA_WARNING );
      } /* endif */
      break;
    case ID_TMM_UNMARK:
      if ( pDoc )
      {
        // unmark in both windows
        EQFBFuncMarkClear( pDoc );
        // eqfbscreendata already done by unmark function
        EQFBFuncMarkClear( pDoc->twin );
      } /* endif */
      break;
    case ID_TMM_DEL_ALL:
      {
        USHORT   usResponse = 0;
        BOOL  fOK = TRUE;

        usResponse = UtlError( WARNING_TMM_DELETE_ALL,
                               MB_YESNO | MB_DEFBUTTON2,
                               0, (PSZ *)NULP, EQF_QUERY );
        if ( usResponse == MBID_YES )
        {
          // set message for delete in status bar
          strcpy( pTMMIda->szMessage, pTMMIda->szDeleteState );
          UPDSTATUSBAR( pTMMIda );

          UtlDispatch();
          pTMMIda = ACCESSWNDIDA( hwnd, PTMMIDA );

          if ( (pTMMIda != NULL) && !pTMMIda->fKill )
          {
            // ensure that current work segment is back in our segment table
            {
              PTBDOCUMENT pTargetDoc;
              pTargetDoc = &pTMMIda->TBTargetDoc;
              pTargetDoc->EQFBFlags.workchng = FALSE;              // reset changes
              EQFBWorkSegOut( pTargetDoc );
              EQFBWorkSegOut( pTargetDoc->twin );
             }

            fOK = DeleteAllSegments( pTMMIda );
            pTMMIda->fDelSeg = FALSE;                 //set del flag
            // reset message text but
            // status bar needs no update because editor will be closed
            pTMMIda->szMessage[0] = EOS;
          } /* endif */

          if ( pTMMIda != NULL )
          {
            if ( fOK )
            {
              // refrsh screen
              EQFDispWindow ( &pTMMIda->TBSourceDoc );
              EQFDispWindow ( &pTMMIda->TBTargetDoc );
              // display msg if back to query or continue or what else
              LastSegDel( hwnd, pTMMIda );
            }
            else
            {
              //tm error msg so quit editor
              WinPostMsg( hwnd, WM_CLOSE, NULL, NULL );
            } /* endif */
          } /* endif */

        } /* endif */
      }
      break;
    case ID_TMM_DEL_CUR:
      if ( pDoc )
      {
        ULONG  ulSegNum;                     // current seg number
//        LONG   lJ;                            // for loop counter
        ULONG  ulPos;                        // counter
        PTBSEGMENT pSeg = NULL;              // pointer to segment
        BOOL fOK = TRUE;
        USHORT   usResponse = 0;

        usResponse = UtlError( WARNING_TMM_DELETE_SEG,
                               MB_YESNO | MB_DEFBUTTON2,
                               0, (PSZ *)NULP, EQF_QUERY );
        if ( usResponse == MBID_YES )
        {
          ulSegNum = pDoc->ulWorkSeg;

          pTMMIda->fDelSeg = TRUE;                           //set del flag

          // check if segment to be deleted is marked
          CheckBlock( pDoc, ulSegNum );

          // ensure that current work segment is back in our segment table
          {
            PTBDOCUMENT pTargetDoc;
            pTargetDoc = &pTMMIda->TBTargetDoc;
            pTargetDoc->EQFBFlags.workchng = FALSE;              // reset changes

            EQFBWorkSegOut( pTargetDoc );
            EQFBWorkSegOut( pTargetDoc->twin );
          }

          //delete segment in tm
          fOK = DeleteSegments( pTMMIda );
          pTMMIda->fDelSeg = FALSE;                 //set del flag
          if ( fOK )
          {
            //update for subsequent redraw of screens
            UpdateSegTable( pDoc );
            UpdateSegTable( pDoc->twin );

            //update table containing the actual tm segment numbers
            //so that the deleted segment number and double spacing are removed
            ulPos = (ulSegNum - 1)/2;
            memmove( pTMMIda->pTMParms+ulPos,     //destination
                     pTMMIda->pTMParms+ulPos+1,    //source
                     ((pTMMIda->pTMIda->ulSeg2Load - (ulPos + 1)) *
                                                    sizeof( TMEXTRAS )) );

            // activate segment moved to segnum that was deleted
            // if not last segment
            if ( pDoc->pSegTables )
            {
                pSeg = EQFBGetSegW(pDoc, ulSegNum);
                if ( pSeg && pSeg->pDataW )
                {
                  fOK = ActivateSegment( pTMMIda, ulSegNum );
                } /* endif */
                WinPostMsg( hwnd, WM_EQF_SETFOCUS, 0,
                          MP2FROMHWND(pTMMIda->hwndFocus) );
            }
            else
            {
              // refrsh screen
              EQFDispWindow ( &pTMMIda->TBSourceDoc );
              EQFDispWindow ( &pTMMIda->TBTargetDoc );
              // last segment of this editing deleted, back to query
              LastSegDel( hwnd, pTMMIda );
            } /* endif */
          }
          else
          {
            //tm error msg so quit editor
            WinPostMsg( hwnd, WM_CLOSE, NULL, NULL );
          } /* endif */
        } /* endif */
      }
      break;

    case ID_TMM_DELM_CUR:
      fOK = setMFlag(pTMMIda, pDoc, TRANSLFLAG_NORMAL );
      UPDSTATUSBAR( pTMMIda );
      break;

    case ID_TMM_SETM_CUR:
      fOK = setMFlag(pTMMIda, pDoc, TRANSLFLAG_MACHINE );

      UPDSTATUSBAR( pTMMIda );
      break;

    case ID_TMM_DELM_ALL:
      pTargetDoc = &pTMMIda->TBTargetDoc;
      ulSegNum = 1;
      while (fOK && (ulSegNum < pTargetDoc->ulMaxSeg)) {
      pTargetDoc->ulWorkSeg = ulSegNum;
      pDoc->ulWorkSeg = ulSegNum;
      fOK = setMFlag(pTMMIda, pDoc, TRANSLFLAG_NORMAL );
      ulSegNum += 2;
      }
      UPDSTATUSBAR( pTMMIda );
      break;

    case ID_TMM_SETM_ALL:
      pTargetDoc = &pTMMIda->TBTargetDoc;
      ulSegNum = 1;
      while (fOK && (ulSegNum < pTargetDoc->ulMaxSeg)) {
      pTargetDoc->ulWorkSeg = ulSegNum;
      pDoc->ulWorkSeg = ulSegNum;
      fOK = setMFlag(pTMMIda, pDoc, TRANSLFLAG_MACHINE );
      ulSegNum += 2;
      }
      UPDSTATUSBAR( pTMMIda );
      break;

    case ID_TMM_FONTCOL:
      if ( pDoc )
      {
        EQFBFuncFonts( pDoc );
        EQFBGetColors(pTMMIda->pColorTable);             //get color table
      } /* endif */
      break;
    case ID_TMM_FONTSIZES:
      if ( pDoc )
      {
        EQFBFuncFontSize( pDoc );
      } /* endif */
      break;
    case ID_TMM_HORIZONTAL:
      TMMTile(pTMMIda, TRUE);            // arrange horizontally
      break;
    case ID_TMM_VERTICAL:
      TMMTile(pTMMIda, FALSE);           // arrange vertically
      break;
    case ID_TMM_PROOFSEG:                // spellcheck of current segment
      if ( pDoc && (pDoc->docType == STARGET_DOC) )
        EQFBFuncSpellSeg(pDoc);
      break;
    case ID_TMM_PROOFALL:                // spellcheck for file
      if ( pDoc && (pDoc->docType == STARGET_DOC) )
        EQFBFuncSpellFile(pDoc);
      break;
    case ID_TMM_PROTECT:                  // switch to protected mode
      if ( pDoc )
      {
        EQFBChangeStyle( pDoc, DISP_PROTECTED);
        EQFBChangeStyle( pDoc->twin, DISP_PROTECTED);
      } /* endif */
      break;
    case ID_TMM_UNPROTECT:
      // set unprotect style only in target doc
      // so we get rid of beep
      if ( pDoc )
      {
        if ( pTMMIda->usActWin == STARGET_DOC )
        {
          EQFBChangeStyle( pDoc,  DISP_UNPROTECTED);
        }
        else
        {
          EQFBChangeStyle( pDoc->twin,  DISP_UNPROTECTED);
        } /* endif */
      } /* endif */
      break;
    case ID_TMM_HIDE:
      if ( pDoc )
      {
        EQFBChangeStyle( pDoc,  DISP_HIDE);
        EQFBChangeStyle( pDoc->twin,  DISP_HIDE);
      } /* endif */
      break;
    case ID_TMM_COMPACT:                    // switch to compact style
      if ( pDoc )
      {
        EQFBChangeStyle( pDoc,  DISP_COMPACT);
        EQFBChangeStyle( pDoc->twin,  DISP_COMPACT);
      } /* endif */
      break;
    case PID_SYS_CLOSE:
      WinPostMsg( hwnd, WM_SYSCOMMAND, SC_CLOSE, NULL );
      break;
    case PID_SYS_SIZE:
      WinSendMsg( hwnd, WM_SYSCOMMAND, SC_SIZE, NULL );
      break;
    case PID_SYS_MOVE:
      WinSendMsg( hwnd, WM_SYSCOMMAND, SC_MOVE, NULL );
      break;
  } /* command switch */
  /******************************************************************/
  /* refresh the screen only if necessary ....                      */
  /******************************************************************/

  if ( pTMMIda->TBSourceDoc.Redraw != REDRAW_NONE )
  {
    EQFBRefreshScreen( &(pTMMIda->TBSourceDoc) );          // refresh the screen
  } /* endif */
  if ( pTMMIda->TBTargetDoc.Redraw != REDRAW_NONE )
  {
    EQFBRefreshScreen( &(pTMMIda->TBTargetDoc) );          // refresh the screen
  } /* endif */

} /* TMMCommand  */

//------------------------------------------------------------------------------
// Internal function                                                            
//------------------------------------------------------------------------------
// Function name:     AddSourceAndTranslation                                   
//------------------------------------------------------------------------------
// Function call:     AddSourceAndTranslation( pTMMIda )                        
//+---------------------------------------------------------------------------- +
// Description:       get source and target segment and add to segment          
//                    structure                                                 
//------------------------------------------------------------------------------
// Input parameter:   PTMMIDA pTMMIda                                           
// Parameters:                                                                  
//------------------------------------------------------------------------------
// Returncode type:   BOOL                                                      
//------------------------------------------------------------------------------
// Returncodes:       BOOL                                                      
//------------------------------------------------------------------------------
// Function flow:     Get length of incoming strings                            
//                    calculate no of linefeeds in these strings                
//                    remember tm segment no, file name, date, server name      
//                    allocate memory for strings                               
//                    add segments to segment table                             
//------------------------------------------------------------------------------
BOOL  AddSourceAndTranslation
(
  PTMMIDA pTMMIda
)
{
  ULONG  ulSrcSegLen;                  // length of text in segment
  ULONG  ulTgtSegLen;                  // length of text in segment
  ULONG  ulLen;                        // length of text in segment
  ULONG  ulSegNum;                     // position counter
  PSZ_W pSrcString = NULL;             // pinter to source string
  PSZ_W pTgtString = NULL;             // pointer to target string
  BOOL fOK = TRUE;                     // success indicator
  SHORT sRes = 0;                      // counter
  PTMEXTRAS pTMParms;                 // pointer to tm paramter structure

  ulSegNum = (pTMMIda->ulSrcSegNum - 1) / 2;
  pTMParms = pTMMIda->pTMParms + ulSegNum;

  // truncate segments to MAX_SEGMENT_SIZE -1 because this is the
  // maximum size the TPRO functions can/will handle
  pTMMIda->pTMIda->szSourceSeg[MAX_SEGMENT_SIZE-1] = EOS;
  pTMMIda->pTMIda->szTargetSeg[MAX_SEGMENT_SIZE-1] = EOS;

  //get length of source string
  ulSrcSegLen = UTF16strlenCHAR( pTMMIda->pTMIda->szSourceSeg )+1;

  // count number of LFs at end of data strings
  pTMParms->sSourceLF = (BYTE)TMMCountLFAtEnd( pTMMIda->pTMIda->szSourceSeg);
  pTMParms->sTargetLF = (BYTE)TMMCountLFAtEnd( pTMMIda->pTMIda->szTargetSeg);

  //get max lines of source string
  SetLineNumber( &pTMMIda->TBSourceDoc, pTMMIda->pTMIda->szSourceSeg );

  //get length of target string
  ulTgtSegLen = UTF16strlenCHAR( pTMMIda->pTMIda->szTargetSeg )+1;

  //get max lines of target string
  SetLineNumber( &pTMMIda->TBTargetDoc, pTMMIda->pTMIda->szTargetSeg );

  //remember correlating tm segment number and source doc name
  pTMParms->ulTmSegNum  =          pTMMIda->pTMIda->ulSegNum;
  pTMParms->usTranslationFlag =  pTMMIda->pTMIda->usTranslationFlag;
  pTMParms->sSourceDocInd = TMMFindInTable( pTMMIda->pTMIda,
                                            pTMMIda->pTMIda->szSourceDoc,
                                            TMM_DOCUMENTS );
  pTMParms->sTagTableInd  = TMMFindInTable( pTMMIda->pTMIda,
                                            pTMMIda->pTMIda->szTagTable,
                                            TMM_TAGTABLES );
  pTMParms->sTargetLangInd = TMMFindInTable( pTMMIda->pTMIda,
                                             pTMMIda->pTMIda->szTargetLng,
                                             TMM_LANGUAGES );
  if ( pTMMIda->pTMIda->szLongName[0] != EOS )
  {
    pTMParms->sLongNameInd = TMMFindInTable( pTMMIda->pTMIda,
                                             pTMMIda->pTMIda->szLongName,
                                             TMM_LONGNAMES );
  }
  else
  {
    pTMParms->sLongNameInd = NOSTRINGINDEX;
  } /* endif */

  pTMParms->lDate = pTMMIda->pTMIda->lDate;
  if ( pTMMIda->pTMIda->szContext[0] )
  {
    pTMParms->pszContext = PoolAddStringW( pTMMIda->pTMIda->pPool,
                                           pTMMIda->pTMIda->szContext );
  }
  else
  {
    pTMParms->pszContext = NULL;
  } /* endif */

  strcpy( pTMParms->szKey, pTMMIda->pTMIda->szProposalKey );

  //calculate difference in line numbers
  sRes = (SHORT)pTMMIda->TBSourceDoc.ulMaxLine -
         (SHORT)pTMMIda->TBTargetDoc.ulMaxLine;

  if ( sRes > 0 )
  {
    //allocate buffer for source
    fOK = UtlAlloc( (PVOID *)&pSrcString, 0L, (LONG) ulSrcSegLen * sizeof(CHAR_W), ERROR_STORAGE );
    if ( fOK )
    {
      //allocate buffer for target
      ulLen = sRes + ulTgtSegLen;
      fOK = UtlAlloc( (PVOID *)&pTgtString, 0L, (LONG)ulLen * sizeof(CHAR_W), ERROR_STORAGE );
    } /* endif */

    if ( fOK )
    {
      //copy strings
      UTF16strcpy( pSrcString, pTMMIda->pTMIda->szSourceSeg );
      UTF16strcpy( pTgtString, pTMMIda->pTMIda->szTargetSeg );

      //add extra lines to target segment
      fOK = AddSegment( &pTMMIda->TBTargetDoc, pTgtString,
                        ulTgtSegLen, &pTMMIda->ulTgtSegNum, sRes );
      if ( fOK )
      {
        AddSegment( &pTMMIda->TBSourceDoc, pSrcString,
                    ulSrcSegLen, &pTMMIda->ulSrcSegNum, 0 );
        pTMMIda->fUpdate = TRUE;
      } /* endif */
    } /* endif */
  }
  else if ( sRes < 0 )
  {
    //allocate buffer for target
    fOK = UtlAlloc( (PVOID *)&pTgtString, 0L, (LONG)ulTgtSegLen * sizeof(CHAR_W), ERROR_STORAGE );
    if ( fOK )
    {
      //allocate buffer for source
      ulLen = -sRes + ulSrcSegLen;
      fOK = UtlAlloc( (PVOID *)&pSrcString, 0L, (LONG)ulLen* sizeof(CHAR_W), ERROR_STORAGE );
    } /* endif */

    if ( fOK )
    {
      //copy strings
      UTF16strcpy( pSrcString, pTMMIda->pTMIda->szSourceSeg );
      UTF16strcpy( pTgtString, pTMMIda->pTMIda->szTargetSeg );

      //add extra lines to source segment
      fOK = AddSegment( &pTMMIda->TBSourceDoc, pSrcString, ulSrcSegLen,
                        &pTMMIda->ulSrcSegNum, (USHORT)(-sRes) );
      if ( fOK )
      {
        AddSegment( &pTMMIda->TBTargetDoc, pTgtString, ulTgtSegLen,
                    &pTMMIda->ulTgtSegNum, 0 );
        pTMMIda->fUpdate = TRUE;
      } /* endif */
    } /* endif */
  }
  else
  {
    fOK = UtlAlloc( (PVOID *)&pSrcString, 0L, (LONG)ulSrcSegLen * sizeof(CHAR_W), ERROR_STORAGE );
    if ( fOK )
    {
      fOK = UtlAlloc( (PVOID *)&pTgtString, 0L, (LONG)ulTgtSegLen* sizeof(CHAR_W), ERROR_STORAGE );
      if ( fOK )
      {
        //copy strings
        UTF16strcpy( pSrcString, pTMMIda->pTMIda->szSourceSeg );
        UTF16strcpy( pTgtString, pTMMIda->pTMIda->szTargetSeg );
        pTMMIda->TBTargetDoc.usLangTypeTgt |= MorphGetLanguageType( pTMMIda->pTMIda->szTargetLng );
        //no extra lines need to be added
        fOK = AddSegment( &pTMMIda->TBSourceDoc, pSrcString, ulSrcSegLen,
                          &pTMMIda->ulSrcSegNum, 0 );
        if ( fOK )
        {
          fOK = AddSegment( &pTMMIda->TBTargetDoc, pTgtString, ulTgtSegLen,
                            &pTMMIda->ulTgtSegNum, 0 );
          pTMMIda->fUpdate = TRUE;
        } /* endif */
      } /* endif */
    } /* endif */
  } /* endif */

  return( fOK );

} /* end of function AddSourceAndTranslation */

//------------------------------------------------------------------------------
// Internal function                                                            
//------------------------------------------------------------------------------
// Function name:     AdjustLines                                               
//------------------------------------------------------------------------------
// Function call:     AdjustLines( pTBDoc, pucSeg, pusPropLen, sAddedLines      
//------------------------------------------------------------------------------
// Description:       add sAddedLines no of linefeeds to segment                
//------------------------------------------------------------------------------
// Input parameter:   PTBDOCUMENT pTBDoc,                                       
// Parameters:        PSZ pucSeg,                                               
//                    PUSHORT pusPropLen,                                       
//                    SHORT sAddedLines                                         
//------------------------------------------------------------------------------
// Output parameter:                                                            
//------------------------------------------------------------------------------
// Returncode type:   VOID                                                      
//------------------------------------------------------------------------------
// Returncodes:       VOID                                                      
//------------------------------------------------------------------------------
// Function flow:     add linefeeds needed to synchronize, sAddedLines          
//                    indicates no to add                                       
//------------------------------------------------------------------------------

VOID AdjustLines
(
  PTBDOCUMENT pTBDoc,
  PSZ_W pucSeg,
  PUSHORT pusPropLen,
  SHORT sAddedLines
)
{
  while ( sAddedLines && (*pusPropLen < (MAX_SEGMENT_SIZE-1)) )
  {
    UTF16strcat( pucSeg, L"\n" );
    (*pusPropLen)++;
    pTBDoc->ulMaxLine++;
    sAddedLines--;
  } /* endwhile */
} /* end of function AdjustLines */

//------------------------------------------------------------------------------
// Internal function                                                            
//------------------------------------------------------------------------------
// Function name:     AddSegment                                                
//------------------------------------------------------------------------------
// Function call:     AddSegment( pTBDoc, pucSeg, usPropLen, pulSegNum,         
//                                sLines )                                      
//------------------------------------------------------------------------------
// Description:       add segment to segment table                              
//------------------------------------------------------------------------------
// Input parameter:   PTBDOCUMENT pTBDoc,                                       
// Parameters:        PSZ pucSeg,                                               
//                    USHORT usPropLen,                                         
//                    PUSHORT pulSegNum,                                        
//                    SHORT sLines                                              
//------------------------------------------------------------------------------
// Returncode type:   BOOL                                                      
//------------------------------------------------------------------------------
// Returncodes:       BOOL                                                      
//------------------------------------------------------------------------------
// Function flow:     fill the tbSegment structure                              
//                    add extra linefeeds if need to synchronize                
//                    add to segment table                                      
//                    add double line feed as separator between segments        
//------------------------------------------------------------------------------
BOOL  AddSegment
(
  PTBDOCUMENT pTBDoc,
  PSZ_W pucSeg,
  ULONG ulPropLen,
  PULONG pulSegNum,
  SHORT sLines
)
{
  BOOL fOK = TRUE;
  BOOL fAddLF = FALSE;
  PSZ_W pString;

  tbSegment.pDataW = pucSeg;
  tbSegment.usLength = (USHORT)( ulPropLen - 1); // segment length is the length of the data w/o EOS!

  tbSegment.qStatus = QF_XLATED;
  tbSegment.ulSegNum = *pulSegNum;
  (*pulSegNum)++;
  tbSegment.pusBPET = NULL;
  tbSegment.pusHLType = NULL;

  //do I need to add a newline at end of string
  if ( *(tbSegment.pDataW+ulPropLen-1) != LF )
    fAddLF = TRUE;

  //if sLines not zero, add extra empty lines for synchronization
  if ( sLines != 0 )
  {
    AdjustLines( pTBDoc, tbSegment.pDataW, &tbSegment.usLength, sLines );
  } /* endif */

  //add segment to either source or target window
  EQFBAddSegW( pTBDoc, &tbSegment );

  //add linefeed if not at eos
  if ( fAddLF )
  {
    fOK = UtlAlloc( (PVOID *)&pString, 0L, (LONG)MIN_ALLOC, ERROR_STORAGE );
    if ( fOK )
    {
      PSZ_W pTemp = tb2NewLineSegment.pDataW;

      UTF16strcpy( pString, tb2NewLineSegment.pDataW );
      tb2NewLineSegment.pDataW = pString;
      tb2NewLineSegment.ulSegNum = *pulSegNum;
      (*pulSegNum)++;
      //add newline so that next segment start at the beginning of a new line
      SetLineNumber( pTBDoc, tb2NewLineSegment.pDataW );
      //add segment with just newline
      EQFBAddSegW( pTBDoc, &tb2NewLineSegment );
      tb2NewLineSegment.pDataW = pTemp;
    } /* endif */
  } /* endif */

  //update active segment in case of reallocation in segment table
  pTBDoc->pTBSeg = EQFBGetSegW( pTBDoc, pTBDoc->tbActSeg.ulSegNum);

  return( fOK );

} /* end of function AddSegment */

//------------------------------------------------------------------------------
// Internal function                                                            
//------------------------------------------------------------------------------
// Function name:     SetLineNumber                                             
//------------------------------------------------------------------------------
// Function call:     SetLineNumber                                             
//------------------------------------------------------------------------------
// Description:       Determine no of linefeeds in data string                  
//------------------------------------------------------------------------------
// Input parameter:   PTBDOCUMENT pTBDoc                                        
// Parameters:        PSZ pData                                                 
//------------------------------------------------------------------------------
// Returncode type:   VOID                                                      
//------------------------------------------------------------------------------
// Returncodes:       VOID                                                      
//------------------------------------------------------------------------------
// Function flow:     determine if linefeed in data string and if so            
//                    increase usMaxLine                                        
//------------------------------------------------------------------------------

VOID SetLineNumber
(
  PTBDOCUMENT pTBDoc,              // pointer to document
  PSZ_W         pData                // pointer to data
)
{
  CHAR_W  ch;

  while ( (ch = *pData) != NULC )
  {
    if ( ch == '\n')
    {
      pTBDoc->ulMaxLine++;
    } /* endif */
    pData ++;
  } /* endwhile */
} /* end of function SetLineNumber */

//------------------------------------------------------------------------------
// Internal function                                                            
//------------------------------------------------------------------------------
// Function name:     EQFDispWindow                                             
//------------------------------------------------------------------------------
// Function call:     EQFDispWindow( pTBDoc )                                   
//------------------------------------------------------------------------------
// Description:       display changes in windows                                
//------------------------------------------------------------------------------
// Input parameter:   PTBDOCUMENT pTBDoc                                        
// Parameters:                                                                  
//------------------------------------------------------------------------------
// Returncode type:   VOID                                                      
//------------------------------------------------------------------------------
// Returncodes:       VOID                                                      
//------------------------------------------------------------------------------
// Function flow:     redraws both windows when any changes have been made      
//------------------------------------------------------------------------------

static
VOID  EQFDispWindow
(
   PTBDOCUMENT  pTBDoc
)
{
  USHORT  usRow;

  EQFBScrnLinesFromSeg                   // build line table
         ( pTBDoc,
           pTBDoc->lCursorRow,           // starting row
           (pTBDoc->lScrnRows-pTBDoc->lCursorRow), // number of rows
           &(pTBDoc->TBCursor) );        // starting segment

  // Set vertical scroll slider
  usRow = (USHORT)max(pTBDoc->ulMaxSeg - 2, 2);
  SetScrollRange( pTBDoc->hwndFrame, SB_VERT, 0, usRow, FALSE);
  SetScrollPos( pTBDoc->hwndFrame, SB_VERT, 0, TRUE );

  SetScrollRange( pTBDoc->hwndFrame, SB_HORZ, 0, 255, FALSE);
  if (pTBDoc->fTARight)
  {
	SetScrollPos(pTBDoc->hwndFrame, SB_HORZ,255, TRUE );
  }
  else
  {
    SetScrollPos(pTBDoc->hwndFrame, SB_HORZ,0, TRUE );
  }

  pTBDoc->Redraw |= REDRAW_ALL;          // redraw the screen
  EQFBScreenData( pTBDoc );
  EQFBScreenCursor( pTBDoc );            // update cursor and sliders
} /* end of function EQFDispWindow */

//------------------------------------------------------------------------------
// Internal function                                                            
//------------------------------------------------------------------------------
// Function name:     AddLFAndUpdate                                            
//------------------------------------------------------------------------------
// Function call:     AddLFAndUpdate( pDoc )                                    
//------------------------------------------------------------------------------
// Description:       add linefeed to end of data string                        
//------------------------------------------------------------------------------
// Input parameter:   PTBDOCUMENT pTBDoc                                        
// Parameters:                                                                  
//------------------------------------------------------------------------------
// Returncode type:   VOID                                                      
//------------------------------------------------------------------------------
// Returncodes:       VOID                                                      
//------------------------------------------------------------------------------
// Function flow:     Add linefeed to end of data string                        
//------------------------------------------------------------------------------
VOID AddLFAndUpdate
(
  PTBDOCUMENT pDoc
)
{
  ULONG ulLen;

  ulLen = UTF16strlenCHAR( pDoc->pTBSeg->pDataW );
  *(pDoc->pTBSeg->pDataW+ulLen) = LF;
  *(pDoc->pTBSeg->pDataW+ulLen+1) = EOS;
  pDoc->pTBSeg->usLength++;

  pDoc->EQFBFlags.workchng = TRUE;
  pDoc->Redraw |= REDRAW_ALL;          // redraw the screen
  pDoc->pTBSeg->SegFlags.Typed = TRUE;

  //mode readjust screen start positions
  EQFBScrnLinesFromSeg ( pDoc,              // pointer to doc ida
                         0,                 // starting row
                         pDoc->lScrnRows,  // number of rows
                                            // starting segment
                         (pDoc->TBRowOffset+1));

} /* end of function AddLFAndUpdate */

//------------------------------------------------------------------------------
// External function                                                            
//------------------------------------------------------------------------------
// Function name:     DelTrailingLF                                             
//------------------------------------------------------------------------------
// Function call:     DelTrailingLF( pDoc )                                     
//------------------------------------------------------------------------------
// Description:       remove a linefeed from end of data string                 
//------------------------------------------------------------------------------
// Input parameter:   PTBDOCUMENT pDoc                                          
// Parameters:                                                                  
//------------------------------------------------------------------------------
// Returncode type:   VOID                                                      
//------------------------------------------------------------------------------
// Returncodes:       VOID                                                      
//------------------------------------------------------------------------------
// Function flow:     remove a linefeed from the end of the string              
//------------------------------------------------------------------------------
VOID DelTrailingLF
(
  PTBDOCUMENT pDoc
)
{
  ULONG ulLen;

  ulLen = UTF16strlenCHAR( pDoc->pTBSeg->pDataW );
  *(pDoc->pTBSeg->pDataW+ulLen-1) = EOS;
  pDoc->pTBSeg->usLength--;

  pDoc->EQFBFlags.workchng = TRUE;
  pDoc->Redraw |= REDRAW_ALL;          // redraw the screen
  pDoc->pTBSeg->SegFlags.Typed = TRUE;

  //mode readjust screen start positions
  EQFBScrnLinesFromSeg ( pDoc,              // pointer to doc ida
                         0,                 // starting row
                         pDoc->lScrnRows,   // number of rows
                                            // starting segment
                         (pDoc->TBRowOffset+1));

} /* endDelTrailingLF */

//------------------------------------------------------------------------------
// Internal function                                                            
//------------------------------------------------------------------------------
// Function name:     ActivateSegment                                           
//------------------------------------------------------------------------------
// Function call:     ActivateSegment( pTMMIda, ulSegNum )                      
//------------------------------------------------------------------------------
// Description:       activate segment                                          
//------------------------------------------------------------------------------
// Input parameter:   PTMMIda pTMMIda                                           
// Parameters:        USHORT ulSegNum                                           
//------------------------------------------------------------------------------
// Returncode type:   VOID                                                      
//------------------------------------------------------------------------------
// Returncodes:       VOID                                                      
//------------------------------------------------------------------------------
// Function flow:     if not the first segment                                  
//                      get the current actively src and tgt segment and        
//                      reset status to xlated                                  
//                                                                              
//                    go to segments that correlate with the passed segnum      
//                    worksegin for both src and tgt docs                       
//                    get both segments                                         
//                    mark status for both as current                           
//------------------------------------------------------------------------------
BOOL ActivateSegment
(
  PTMMIDA pTMMIda,                        // pointer to main structure
  ULONG   ulSegNum                        // new segment number
)
{
  PTBSEGMENT pSrcSeg;                     // pointer to segment
  PTBSEGMENT pTgtSeg;                     // pointer to segment
  PTBDOCUMENT pDoc;                       // pointer to document instance
  BOOL        fOK = TRUE;
  BOOL        fSaved = TRUE;

  pDoc = &pTMMIda->TBTargetDoc;
  /********************************************************************/
  /* if not in first invocation reset our special settings and        */
  /* save any changes....                                             */
  /* reset any active segment (yellow )                               */
  /********************************************************************/
  if ( pDoc->twin && (pDoc->twin->tbActSeg.ulSegNum != 0)  )
  {
    //update status so no longer marked as active for translation window
    pSrcSeg = EQFBGetSegW( pDoc->twin, pDoc->twin->tbActSeg.ulSegNum );
    if ( pSrcSeg )
    {
      pSrcSeg->qStatus = QF_XLATED;
      pSrcSeg->SegFlags.Current = FALSE;
    } /* endif */
  } /* endif */
  if ( pDoc->tbActSeg.ulSegNum != 0)         // first invocation
  {
     pTgtSeg = EQFBGetSegW(pDoc, pDoc->tbActSeg.ulSegNum ); // get seg
     if ( pTgtSeg )
     {
       pTgtSeg->qStatus = QF_XLATED;
       pTgtSeg->SegFlags.Current = FALSE;
     } /* endif */
     fOK = SaveSegment( pTMMIda, &fSaved );
     if ( fOK && !fSaved )
     {
       // segment is empty and user wants not to save
       // so activate the current segment
       ulSegNum = pDoc->tbActSeg.ulSegNum;
     } /* endif */
  }
  else
  {
    /******************************************************************/
    /* HELP HELP - someone came in without the wrong pointer, he      */
    /* missed a EQFBWorkOut if the checked for pointers are equal...  */
    /* PS: Check is not necessary for Source, since noone is able to  */
    /*     modify the source                                          */
    /******************************************************************/
    if ( (pDoc->pTBSeg) && (pDoc->pTBSeg->pDataW == (PSZ_W)&pDoc->pEQFBWorkSegmentW) )
    {
      EQFBWorkSegOut( pDoc );
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* position at new segment and activate it...                      */
  /********************************************************************/
  if ( fOK )
  {
    EQFBGotoSeg( pDoc, ulSegNum, 0); // position at this segment
    //position source document
    EQFBGotoSeg( pDoc->twin, ulSegNum, 0 );
    EQFBWorkSegIn( pDoc );    // copy contents of current target segment
    EQFBWorkSegIn( pDoc->twin );   // copy contents of current target segment

    pTgtSeg = EQFBGetSegW( pDoc, ulSegNum);         // get target seg
    if ( pTgtSeg )
    {
      memcpy( &(pDoc->tbActSeg), pTgtSeg, sizeof(TBSEGMENT) );

      pTgtSeg->qStatus = QF_CURRENT;            // it's the active segment
      pTgtSeg->SegFlags.Current = TRUE;            // it's the active segment
    } /* endif */

    pSrcSeg = EQFBGetSegW( pDoc->twin, ulSegNum);             // get source seg
    if ( pSrcSeg )
    {
      memcpy( &(pDoc->twin->tbActSeg), pSrcSeg, sizeof(TBSEGMENT) );

      pSrcSeg->qStatus = QF_CURRENT;            // it's the active segment
      pSrcSeg->SegFlags.Current = TRUE;              // it's the active segment
    } /* endif */

    pDoc->Redraw |= REDRAW_ALL;            // redraw screen
    pDoc->twin->Redraw |= REDRAW_ALL;                        // redraw screen

    UPDSTATUSBAR( pTMMIda );
  } /* endif */

  return( fOK );

} /* end of function ActivateSegment */


//------------------------------------------------------------------------------
// Internal function                                                            
//------------------------------------------------------------------------------
// Function name:     UnchangedSegment                                          
//------------------------------------------------------------------------------
// Function call:     UnchangedSegment( pTMMIda, ulSegNum )                     
//------------------------------------------------------------------------------
// Description:       activate next segment but do not save previous            
//------------------------------------------------------------------------------
// Input parameter:   PTMMIda pTMMIda                                           
// Parameters:        USHORT ulSegNum                                           
//------------------------------------------------------------------------------
// Returncode type:   VOID                                                      
//------------------------------------------------------------------------------
// Returncodes:       VOID                                                      
//------------------------------------------------------------------------------
// Function flow:     if not the first segment                                  
//                      get the current actively src and tgt segment and        
//                      reset status to xlated                                  
//                                                                              
//                    go to segments that correlate with the passed segnum      
//                    worksegin for both src and tgt docs                       
//                    get both segments                                         
//                    mark status for both as current                           
//------------------------------------------------------------------------------
BOOL UnchangedSegment
(
  PTMMIDA pTMMIda,                        // pointer to main structure
  ULONG ulSegNum                         // new segment number
)
{
  PTBSEGMENT pSrcSeg = NULL;                     // pointer to segment
  PTBSEGMENT pTgtSeg = NULL;                     // pointer to segment
  PTBDOCUMENT pDoc;                       // pointer to document instance
  BOOL        fOK = TRUE;

  pDoc = &pTMMIda->TBTargetDoc;
  /********************************************************************/
  /* if not in first invocation reset our special settings and        */
  /* save any changes....                                             */
  /********************************************************************/
  if ( pDoc->tbActSeg.ulSegNum != 0)         // first invocation
  {
     pTgtSeg = EQFBGetSegW(pDoc, pDoc->tbActSeg.ulSegNum ); // get seg
     if ( pTgtSeg )
     {
       pTgtSeg->qStatus = pDoc->tbActSeg.qStatus;
       pTgtSeg->SegFlags.Current = FALSE;

       //update status so no longer marked as active for translation window
       pSrcSeg = EQFBGetSegW( pDoc->twin, pDoc->twin->tbActSeg.ulSegNum );
       if ( pSrcSeg )
       {
         pSrcSeg->qStatus = pDoc->twin->tbActSeg.qStatus;
         pSrcSeg->SegFlags.Current = FALSE;
       } /* endif */
     } /* endif */

     if ( pDoc->pSaveSegW )
     {
       pTgtSeg->pDataW = pDoc->pSaveSegW;
     } /* endif */
     pDoc->EQFBFlags.workchng = FALSE;
     pTgtSeg->usLength = (USHORT)UTF16strlenCHAR( pTgtSeg->pDataW );     // set new size
     UtlAlloc( (PVOID *)&(pTgtSeg->pusBPET) ,0L ,0L , NOMSG); // force recompilation

     if ( pTgtSeg->pusHLType )
     {
        UtlAlloc( (PVOID *)&pTgtSeg->pusHLType, 0L, 0L, NOMSG );
     } /* endif */

     pDoc->pSaveSegW = NULL;

     if ( pDoc->twin->pSaveSegW )
     {
       pSrcSeg->pDataW = pDoc->twin->pSaveSegW;
     } /* endif */
     pDoc->twin->EQFBFlags.workchng = FALSE;
     pSrcSeg->usLength = (USHORT)UTF16strlenCHAR( pSrcSeg->pDataW );     // set new size
     UtlAlloc( (PVOID *)&(pSrcSeg->pusBPET), 0L ,0L , NOMSG);   // force recompilation
    if ( pSrcSeg->pusHLType )
    {
	   UtlAlloc( (PVOID *)&pSrcSeg->pusHLType, 0L, 0L, NOMSG );
	} /* endif */

     pSrcSeg->usLength = (USHORT)UTF16strlenCHAR( pSrcSeg->pDataW );
     pDoc->twin->pSaveSegW = NULL;

     EQFBWorkSegOut( pDoc );
     EQFBWorkSegOut( pDoc->twin );
  }

  /********************************************************************/
  /* position at new segment, but activate it...                      */
  /********************************************************************/
  if ( fOK )
  {
    EQFBGotoSeg( pDoc, ulSegNum, 0); // position at this segment
    //position source document
    EQFBGotoSeg( pDoc->twin, ulSegNum, 0 );
    EQFBWorkSegIn( pDoc );    // copy contents of current target segment
    EQFBWorkSegIn( pDoc->twin );   // copy contents of current target segment

    pTgtSeg = EQFBGetSegW( pDoc, ulSegNum);         // get target seg
    if ( pTgtSeg )
    {
      memcpy( &(pDoc->tbActSeg), pTgtSeg, sizeof(TBSEGMENT) );

      pTgtSeg->qStatus = QF_CURRENT;            // it's the active segment
      pTgtSeg->SegFlags.Current = TRUE;            // it's the active segment
    } /* endif */

    pSrcSeg = EQFBGetSegW( pDoc->twin, ulSegNum);             // get source seg
    if ( pSrcSeg )
    {
      memcpy( &(pDoc->twin->tbActSeg), pSrcSeg, sizeof(TBSEGMENT) );

      pSrcSeg->qStatus = QF_CURRENT;            // it's the active segment
      pSrcSeg->SegFlags.Current = TRUE;              // it's the active segment
    } /* endif */

    pDoc->Redraw |= REDRAW_ALL;            // redraw screen
    pDoc->twin->Redraw |= REDRAW_ALL;                        // redraw screen

    UPDSTATUSBAR( pTMMIda );
  } /* endif */

  return( fOK );

} /* end of function UnchangedSegment */

//------------------------------------------------------------------------------
// Internal function                                                            
//------------------------------------------------------------------------------
// Function name:     TMMDocNext                                                
//------------------------------------------------------------------------------
// Function call:     TMMDocNext ( pstVisDoc )                                  
//------------------------------------------------------------------------------
// Description:       create Visdoc window                                      
//------------------------------------------------------------------------------
// Parameters:        PTMMVISDOC pstVisDoc                                      
//------------------------------------------------------------------------------
// Returncode type:   BOOL                                                      
//------------------------------------------------------------------------------
// Returncodes:       TRUE     success                                          
//------------------------------------------------------------------------------
// Function flow:                                                               
//------------------------------------------------------------------------------
BOOL TMMDocNext
(
  PTMMVISDOC pstVisDoc
)
{
  BOOL          fOK = TRUE;                // success indicator
  PTBDOCUMENT   pTBDoc;
  PTMMIDA       pTMMIda;
  PSZ_W         pString;

  pTBDoc = pstVisDoc->pDoc;

  pTBDoc->ulMaxSeg = 0;

  //allocate buffer for initial dummy segment
  fOK = UtlAlloc( (PVOID *)&pString, 0L, (LONG)MIN_ALLOC * sizeof(CHAR_W), ERROR_STORAGE );
  if ( fOK )
  {
    PSZ_W pTemp = tbInitSegment.pDataW;
    tbInitSegment.pDataW = pString;
    EQFBAddSegW( pTBDoc, &tbInitSegment );      // add dummy segment
    tbInitSegment.pDataW = pTemp;

     pTMMIda = (PTMMIDA)pstVisDoc->pTMMIda;

    fOK = UtlAlloc( (PVOID *)&pTBDoc->pTokBuf, 0L, (LONG) TOK_BUFFER_SIZE,
                    ERROR_STORAGE );
  } /* endif */

  if ( fOK )                    // allocate buffer for marking area
  {
    fOK = UtlAlloc( &pTBDoc->pBlockMark, 0L,
                    (LONG) max( sizeof( EQFBBLOCK ), MIN_ALLOC) ,
                    ERROR_STORAGE );
  } /* endif */

  if ( fOK )
  {
    fOK = UtlAlloc( (PVOID *)&pTBDoc->pInBuf, 0L, (LONG) IO_BUFFER_SIZE,
                    ERROR_STORAGE );
  } /* endif */

  if ( fOK )
  {
    fOK = UtlAlloc( (PVOID *)&pTBDoc->pUndoSegW,
                    0L, (LONG)MAX_SEGMENT_SIZE * sizeof(CHAR_W), ERROR_STORAGE );
  } /* endif */

  if ( fOK )
  {
    fOK = UtlAlloc( (PVOID *)&pTBDoc->pEQFBWorkSegmentW,
                    0L, (LONG)MAX_SEGMENT_SIZE * sizeof(CHAR_W), ERROR_STORAGE );
  } /* endif */


  if ( fOK )
  {
    fOK = UtlAlloc((PVOID *) &(pTBDoc->pSegmentBufferW),
                   0L, (LONG)(MAX_SEGMENT_SIZE + 1) * sizeof(CHAR_W),
                   ERROR_STORAGE);
  }



  if ( fOK )
  {
    pTBDoc->fUndoState = FALSE;           //init fUndoState
    pTBDoc->usUndoSegOff = 0;
  } /* endif */

  return (fOK);

} /* end of function TMMDocNext */

//------------------------------------------------------------------------------
// External function                                                            
//------------------------------------------------------------------------------
// Function name:     TMMSTATUSBARWNDPROC                                       
//------------------------------------------------------------------------------
// Function call:     _                                                         
//------------------------------------------------------------------------------
// Description:       _                                                         
//------------------------------------------------------------------------------
// Parameters:        HWND hwnd,                                                
//                    USHORT msg                                                
//                    WPARAM mp1                                                
//                    LPARAM mp2                                                
//------------------------------------------------------------------------------
// Returncode type:   MRESULT APIENTRY                                          
//------------------------------------------------------------------------------
// Returncodes:       _                                                         
//------------------------------------------------------------------------------
// Function flow:     position statusbar and create it                          
//                    and fill statusbar during WM_PAINT                        
//------------------------------------------------------------------------------
MRESULT APIENTRY TMMSTATUSBARWNDPROC
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
        break;

      case  WM_PAINT:
        {
          PAINTSTRUCT ps;
          HDC    hdc;

          hdc = BeginPaint(hwnd, &ps );
          TMMFillStatusBar( hwnd, hdc  );
          EndPaint(hwnd, &ps);
        }
        break;
      default:
        mResult = WinDefWindowProc ( hwnd, msg, mp1, mp2 );
        break;

  } /* switch */

  return mResult ;
} /* end of function TMMSTATUSBARWNDPROC */


//------------------------------------------------------------------------------
// Internal function                                                            
//------------------------------------------------------------------------------
// Function name:     TMMFillStatusBar                                          
//------------------------------------------------------------------------------
// Function call:     TMMFillStatusBar(hwnd, hps)                               
//------------------------------------------------------------------------------
// Description:       fill and draw status bar                                  
//------------------------------------------------------------------------------
// Parameters:        HWND  hwnd                                                
//                    HPS hps                                                   
//------------------------------------------------------------------------------
// Returncode type:   VOID                                                      
//------------------------------------------------------------------------------
// Returncodes:       none                                                      
//------------------------------------------------------------------------------
// Function flow:     display:                                                  
//                     - number of srcsegments to be translated                 
//                     - number of unaligned src segments                       
//                     - number of crossed-out src segments                     
//                     - source or target, whatever is active                   
//                     - kind of active alignment (1:1, 1:2, 2:1, 2:2)          
//                     - number of crossed-out tgt segments                     
//                     - number of unaligned tgt segments                       
//                     - number of tgtsegments to be translated                 
//------------------------------------------------------------------------------
static VOID TMMFillStatusBar
(
  HWND  hwnd,                          // window handle
  HPS hps                              // presentation space
)
{
  RECT        rectl;
  PTMMIDA     pTMMIda = ACCESSWNDIDA( hwnd, PTMMIDA);
  SWP         swp;
  USHORT      usOneUnit;
  ULONG       ulFilledLeft;       // statusbar filled up...
  ULONG       ulSegLoaded = 0;
  ULONG       ulActSegNum;
  USHORT      usCY = (USHORT)WinQuerySysValue( HWND_DESKTOP, SV_CYMENU );
  HFONT       hOldFont;

  DWORD   dwRGB_PaleGray = CLR_PALEGRAY;
  DWORD   dwRGB_BLACK = CLR_BLACK;
  DWORD   dwRGB_CLR_DARKCYAN = CLR_DARKCYAN;
  int     iBrush = BLACK_BRUSH;

  if (UtlIsHighContrast())
  {
	  dwRGB_PaleGray = GetSysColor(COLOR_BTNFACE);
	  dwRGB_BLACK = GetSysColor(COLOR_BTNTEXT);
	  dwRGB_CLR_DARKCYAN = GetSysColor(COLOR_INACTIVECAPTION);
	  iBrush = GRAY_BRUSH;
  }
  // get current segment number
  ulActSegNum = 0;
  if ( pTMMIda->TBSourceDoc.tbActSeg.ulSegNum != 0 )
  {
    ulActSegNum = ((pTMMIda->TBSourceDoc.tbActSeg.ulSegNum - 1) / 2);
  } /* endif */

  WinQueryWindowPos( hwnd, &swp );
  usOneUnit = swp.cx /
                ( MSG_FIELD + DOC_FIELD + DATE_FIELD +
                  CUR_SEG_FIELD + NUM_FIELD + PERC_FIELD );

  rectl.top = 0;

  // use default GUI font
  hOldFont = (HFONT)SelectObject( hps, GetStockObject(DEFAULT_GUI_FONT) );

  /********************************************************************/
  /* display message for current status                               */
  /********************************************************************/
  rectl.left   = 0;
  ulFilledLeft = usOneUnit * MSG_FIELD;
  rectl.right  = (USHORT) ulFilledLeft;
  rectl.bottom  = usCY - 1;
  ERASERECT( hps, (const RECT*) &rectl, dwRGB_PaleGray );
  if ( pTMMIda->szMessage[0] != EOS )
  {
    DRAWTEXT( hps, pTMMIda->szMessage, rectl, dwRGB_CLR_DARKCYAN, dwRGB_PaleGray,
              DT_CENTER | DT_VCENTER );
  }
  else if ( pTMMIda->TBSourceDoc.tbActSeg.ulSegNum != 0 )
  {
    PSZ  pszText = "M-Flag: ?";
    if ( pTMMIda->pTMParms[ulActSegNum].usTranslationFlag == TRANSLFLAG_NORMAL )
    {
      pszText = "M-Flag: off";
    }
    else if ( pTMMIda->pTMParms[ulActSegNum].usTranslationFlag == TRANSLFLAG_MACHINE )
    {
      pszText = "M-Flag: on";
    }
    else if ( pTMMIda->pTMParms[ulActSegNum].usTranslationFlag == TRANSLFLAG_GLOBMEM )
    {
      pszText = "M-Flag: GobalMemory";
    } /* endif */
    DRAWTEXT( hps, pszText, rectl, dwRGB_BLACK, dwRGB_PaleGray, DT_CENTER | DT_VCENTER );
  } /* endif */

  rectl.bottom  = usCY;
  FrameRect( hps, &rectl, (HBRUSH)GetStockObject(iBrush) );

  /********************************************************************/
  /* display name of document from which translation results          */
  /********************************************************************/
  rectl.left   = ulFilledLeft ;
  ulFilledLeft = rectl.right  = ulFilledLeft + usOneUnit * DOC_FIELD;
  rectl.bottom  = usCY - 1;
  ERASERECT( hps, (const RECT*) &rectl, dwRGB_PaleGray );
  if ( pTMMIda->TBSourceDoc.tbActSeg.ulSegNum != 0 )
  {
    // get current segnum
    if ( pTMMIda->pTMParms )
    {
      if ( pTMMIda->pTMParms[ulActSegNum].sLongNameInd != NOSTRINGINDEX)
      {
        TMMGetNameForInd( pTMMIda->pTMIda,
                          pTMMIda->pTMParms[ulActSegNum].sLongNameInd,
                          pTMMIda->szDocName, TMM_LONGNAMES );
      }
      else
      {
        TMMGetNameForInd( pTMMIda->pTMIda,
                          pTMMIda->pTMParms[ulActSegNum].sSourceDocInd,
                          pTMMIda->szDocName, TMM_DOCUMENTS );
      } /* endif */

      sprintf( pTMMIda->szBuffer, "%s %s(%u)", pTMMIda->szDocPrompt,
               pTMMIda->szDocName, pTMMIda->pTMParms[ulActSegNum].ulTmSegNum );
      if ( TMMTextFitsRect( pTMMIda->szBuffer, &rectl, hps ) )
      {
        DRAWTEXT( hps, pTMMIda->szBuffer, rectl, dwRGB_BLACK, dwRGB_PaleGray,
                  DT_CENTER | DT_VCENTER );
      }
      else
      {
        sprintf( pTMMIda->szBuffer, "%s(%u)",
                 pTMMIda->szDocName, pTMMIda->pTMParms[ulActSegNum].ulTmSegNum );
        DRAWTEXT( hps, pTMMIda->szDocName, rectl, dwRGB_BLACK, dwRGB_PaleGray,
                  DT_CENTER | DT_VCENTER );
      } /* endif */
    } /* endif */
  }
  else
  {
    DRAWTEXT( hps, pTMMIda->szDocPrompt, rectl, dwRGB_BLACK, dwRGB_PaleGray,
              DT_CENTER | DT_VCENTER );
  } /* endif */
  rectl.bottom  = usCY;
  FrameRect( hps, &rectl, (HBRUSH)GetStockObject(iBrush) );

  /********************************************************************/
  /* display date when translation was done                           */
  /********************************************************************/
  rectl.left   = ulFilledLeft ;
  ulFilledLeft = rectl.right  = ulFilledLeft + usOneUnit * DATE_FIELD;
  rectl.bottom    = usCY - 1;
  ERASERECT( hps, (const RECT*) &rectl, dwRGB_PaleGray );
  if ( pTMMIda->TBSourceDoc.tbActSeg.ulSegNum != 0 )
  {
    if ( pTMMIda->pTMParms )
    {
      if ( pTMMIda->pTMIda->lBufDate != pTMMIda->pTMParms[ulActSegNum].lDate )
      {
        pTMMIda->pTMIda->lBufDate = pTMMIda->pTMParms[ulActSegNum].lDate;
        UtlLongToDateString( pTMMIda->pTMIda->lBufDate,
                             pTMMIda->pTMIda->szBufDate,
                             sizeof(pTMMIda->pTMIda->szBufDate) );
      } /* endif */
      sprintf( pTMMIda->szBuffer, "%s %s", pTMMIda->szDatePrompt,
               pTMMIda->pTMIda->szBufDate );
      if ( TMMTextFitsRect( pTMMIda->szBuffer, &rectl, hps ) )
      {
        DRAWTEXT( hps,
                  pTMMIda->szBuffer,
                  rectl, dwRGB_BLACK, dwRGB_PaleGray,
                  DT_CENTER | DT_VCENTER );
      }
      else
      {
        DRAWTEXT( hps,
                  pTMMIda->pTMIda->szBufDate,
                  rectl, dwRGB_BLACK, dwRGB_PaleGray,
                  DT_CENTER | DT_VCENTER );
      } /* endif */
    } /* endif */
  }
  else
  {
    DRAWTEXT( hps,
              pTMMIda->szDatePrompt,
              rectl, dwRGB_BLACK, dwRGB_PaleGray,
              DT_CENTER | DT_VCENTER );
  } /* endif */
  rectl.bottom  = usCY;
  FrameRect( hps, &rectl, (HBRUSH)GetStockObject(iBrush) );

  /********************************************************************/
  /* display number of current segment and total segments             */
  /********************************************************************/
  rectl.left   = ulFilledLeft ;
  ulFilledLeft = rectl.right  = ulFilledLeft + usOneUnit * CUR_SEG_FIELD;
  rectl.bottom    = usCY - 1;
  ERASERECT( hps, (const RECT*) &rectl, dwRGB_PaleGray );

  {
    CHAR szCurSeg[10];
    CHAR szTotalSeg[10];
    PSZ  pszTable[2];
    ULONG ulLen;

    ltoa( ulActSegNum + 1, szCurSeg, 10 );

    if ( pTMMIda->TBSourceDoc.ulMaxSeg != 0 )
    {
      ulSegLoaded = (pTMMIda->TBSourceDoc.ulMaxSeg - 1) / 2;
    }
    else
    {
      ulSegLoaded = 0;
    } /* endif */
    ltoa( ulSegLoaded, szTotalSeg, 10 );

    pszTable[0] = szCurSeg;
    pszTable[1] = szTotalSeg;

    DosInsMessage( pszTable, 2, pTMMIda->szCurPrompt,
                   (strlen(pTMMIda->szCurPrompt) + 1),
                   pTMMIda->szBuffer,
                   sizeof(pTMMIda->szBuffer) - 1,
                   &ulLen );
    pTMMIda->szBuffer[ulLen] = EOS;

    if ( TMMTextFitsRect( pTMMIda->szBuffer, &rectl, hps ) )
    {
      DRAWTEXT( hps, pTMMIda->szBuffer,
                rectl, dwRGB_BLACK, dwRGB_PaleGray,
                DT_CENTER | DT_VCENTER );
    }
    else
    {
      sprintf( pTMMIda->szBuffer, "%s/%s", szCurSeg, szTotalSeg );
      DRAWTEXT( hps, pTMMIda->szBuffer,
                rectl, dwRGB_BLACK, dwRGB_PaleGray,
                DT_CENTER | DT_VCENTER );
    } /* endif */
  }
  rectl.bottom  = usCY;
  FrameRect( hps, &rectl, (HBRUSH)GetStockObject(iBrush) );

  /********************************************************************/
  /* display percent of TM loaded                                     */
  /********************************************************************/
  sprintf( pTMMIda->szBuffer, "%s %ld%%", pTMMIda->szComplPrompt,
           pTMMIda->pTMIda->lPercent );
  rectl.left   = ulFilledLeft ;
  ulFilledLeft = rectl.right  = swp.x + swp.cx;
  rectl.bottom    = usCY - 1;
  ERASERECT( hps, (const RECT*) &rectl, dwRGB_PaleGray );
  if ( TMMTextFitsRect( pTMMIda->szBuffer, &rectl, hps ) )
  {
    DRAWTEXT( hps, pTMMIda->szBuffer, rectl, dwRGB_BLACK, dwRGB_PaleGray,
              DT_CENTER | DT_VCENTER );
  }
  else
  {
    sprintf( pTMMIda->szBuffer, "%ld%%", pTMMIda->pTMIda->lPercent );
    DRAWTEXT( hps, pTMMIda->szBuffer, rectl, dwRGB_BLACK, dwRGB_PaleGray,
              DT_CENTER | DT_VCENTER );
  } /* endif */
  rectl.bottom  = usCY;
  FrameRect( hps, &rectl, (HBRUSH)GetStockObject(iBrush) );

  // restore original font
  SelectObject( hps, hOldFont );

} /* end of function TMMFillStatusBar */


//------------------------------------------------------------------------------
// Internal function                                                            
//------------------------------------------------------------------------------
// Function name:     NextPrevious                                              
//------------------------------------------------------------------------------
// Function call:     NextPrevious( pTMMIda, hwnd, fNext );                     
//------------------------------------------------------------------------------
// Description:       frees for next or previous entries to load                
//------------------------------------------------------------------------------
// Parameters:        _                                                         
//------------------------------------------------------------------------------
// Function flow:     _                                                         
//------------------------------------------------------------------------------
VOID NextPrevious
(
   PTMMIDA         pTMMIda,
   HWND            hwnd,
   BOOL            fNext,
   BOOL            fDeleted
)
{
   BOOL            fOK = TRUE;
   USHORT          usRc = 0;
   BOOL            fDummy;

   fOK = SaveSegment( pTMMIda, &fDummy );
   /****************************************************************/
   /* indicate we dont want to have any further updates...         */
   /****************************************************************/
   pTMMIda->TBSourceDoc.Redraw = REDRAW_NONE;
   pTMMIda->TBTargetDoc.Redraw = REDRAW_NONE;
   if ( fOK )
   {
     UtlDispatch();
     pTMMIda = ACCESSWNDIDA( hwnd, PTMMIDA );
     if ( pTMMIda != NULL )
     {
       usRc = (USHORT)(!FreeForNext( pTMMIda ));
       pTMMIda->pTMIda->ulAddrCount = 0;
       if ( fNext )
       {
         pTMMIda->pTMIda->ulPrevPos++;
       }
       else
       {
         pTMMIda->pTMIda->ulPrevPos--;
         pTMMIda->pTMIda->fPrevious = TRUE;
         if ( !fDeleted )
         {
           pTMMIda->pTMIda->fStop = FALSE;
         } /* endif */
       } /* endif */
     } /* endif */
   } /* endif */

   if ( (pTMMIda != NULL) && fOK && !usRc && !pTMMIda->fKill )
   {
     // update status bar that is is searching
     strcpy( pTMMIda->szMessage, pTMMIda->szSearchState );
     UPDSTATUSBAR( pTMMIda );

     WinPostMsg( hwnd, WM_EQF_PROCESSTASK,
                 MP1FROMSHORT( GET_SEGMENT ),
                 pTMMIda );
   }
   else
   {
     // give error message and then close
     WinPostMsg( hwnd, WM_CLOSE, NULL, NULL );
   } /* endif */

} /* end of function NextPrevious */


//------------------------------------------------------------------------------
// Internal function                                                            
//------------------------------------------------------------------------------
// Function name:     BackToQuery                                               
//------------------------------------------------------------------------------
// Function call:     BackToQuery( pTMMIda, hwnd );                             
//------------------------------------------------------------------------------
// Description:       frees some parameter and goes back to query               
//------------------------------------------------------------------------------
// Parameters:        _                                                         
//------------------------------------------------------------------------------
// Function flow:     _                                                         
//------------------------------------------------------------------------------
VOID BackToQuery
(
   PTMMIDA         pTMMIda,
   HWND            hwnd
)
{
   BOOL            fOK = TRUE;
   USHORT          usRc = 0;


   /****************************************************************/
   /* indicate we dont want to have any further updates...         */
   /****************************************************************/
   pTMMIda->TBSourceDoc.Redraw = REDRAW_NONE;
   pTMMIda->TBTargetDoc.Redraw = REDRAW_NONE;
   UtlDispatch();
   pTMMIda = ACCESSWNDIDA( hwnd, PTMMIDA );
   if ( pTMMIda != NULL )
   {
     usRc = (USHORT)(!FreeForNext( pTMMIda ));

     // free parms
     if ( pTMMIda->pTMParms )
     {
       UtlAlloc( (PVOID *)&pTMMIda->pTMParms, 0L, 0L, NOMSG );
     } /* endif */

     if ( !usRc )
     {
       HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
       FreeTMIdaForSave( pTMMIda->pTMIda );
       pTMMIda->TBSourceDoc.tbActSeg.ulSegNum = 0;
       pTMMIda->TBTargetDoc.tbActSeg.ulSegNum = 0;
       EQFDispWindow ( &pTMMIda->TBSourceDoc );
       EQFDispWindow ( &pTMMIda->TBTargetDoc );

       // update status bar that new query can be made
       strcpy( pTMMIda->szMessage, pTMMIda->szQueryState );
       UPDSTATUSBAR( pTMMIda );

       // call query dialog
       DIALOGBOXW( hwnd, TMQUERYDLG, hResMod, ID_TM_QUERY99_DLG,
                  pTMMIda->pTMIda, fOK );
       if ( fOK )
       {
         // alloc new for parms
         fOK = UtlAlloc( (PVOID *)&pTMMIda->pTMParms,
                         0L,
                         (LONG)( sizeof(TMEXTRAS) *
                                 pTMMIda->pTMIda->ulSeg2Load ),
                         ERROR_STORAGE );

         if ( fOK && (pTMMIda->pTMIda->szTagTable[0] != EOS) )
         {
           fOK = TALoadTagTable( pTMMIda->pTMIda->szTagTable,
                                 (PLOADEDTABLE *)&pTMMIda->TBSourceDoc.pDocTagTable,
                                 FALSE, TRUE ) == NO_ERROR;
         } /* endif */

         /******************************************************************/
         /* Note: we load the tag table twice to have the correct use      */
         /*       count in the TagTable handling functions                 */
         /******************************************************************/
         if ( fOK && (pTMMIda->pTMIda->szTagTable[0] != EOS) )
         {
           fOK = TALoadTagTable( pTMMIda->pTMIda->szTagTable,
                                 (PLOADEDTABLE *)&pTMMIda->TBTargetDoc.pDocTagTable,
                                 FALSE, TRUE ) == NO_ERROR;
         } /* endif */
       } /* endif */
     } /* endif */

     if ( fOK && !usRc && !pTMMIda->fKill )
     {
       // update status bar that it is searching
       strcpy( pTMMIda->szMessage, pTMMIda->szSearchState );
       UPDSTATUSBAR( pTMMIda );

       WinSetActiveWindow( HWND_DESKTOP, pTMMIda->TBTargetDoc.hwndFrame );

       WinPostMsg( hwnd, WM_EQF_PROCESSTASK, MP1FROMSHORT( GET_SEGMENT ),
                   pTMMIda );
     }
     else
     {
       // give error message and then close
       WinPostMsg( hwnd, WM_CLOSE, NULL, NULL );
     } /* endif */

   } /* endif */

} /* end of function BackToQuery */


//------------------------------------------------------------------------------
// Internal function                                                            
//------------------------------------------------------------------------------
// Function name:     CheckBlock                                                
//------------------------------------------------------------------------------
// Function call:     CheckBlock( pDoc );                                       
//------------------------------------------------------------------------------
// Description:       checks if seg to be deleted is marked                     
//------------------------------------------------------------------------------
// Parameters:        _                                                         
//------------------------------------------------------------------------------
// Function flow:     _                                                         
//------------------------------------------------------------------------------
VOID  CheckBlock
(
   PTBDOCUMENT     pDoc,
   ULONG           ulSegNum
)
{
   PEQFBBLOCK      pstBlock;

   pstBlock = (PEQFBBLOCK)pDoc->pBlockMark;
   if ( pstBlock->pDoc == pDoc )
   {
     if ( pstBlock->ulSegNum == ulSegNum )
     {
       // clear mark before deleting
       EQFBFuncMarkClear ( pDoc );
     }
     else
     {
       if ( (pstBlock->ulSegNum !=0) && (pstBlock->ulSegNum > 1) )
       {
         pstBlock->ulSegNum-=2;
       } /* endif */
     } /* endif */
   }
   else
   {
     pstBlock = (PEQFBBLOCK)pDoc->twin->pBlockMark;
     if ( pstBlock->pDoc == pDoc->twin )
     {
       if ( pstBlock->ulSegNum == ulSegNum )
       {
         // clear mark before deleting
         EQFBFuncMarkClear ( pDoc->twin );
       }
       else
       {
         if ( (pstBlock->ulSegNum !=0) && (pstBlock->ulSegNum > 1) )
         {
           pstBlock->ulSegNum-=2;
         } /* endif */
       } /* endif */
     } /* endif */
   } /* endif */

   return;

} /* end of function CheckBlock */


//------------------------------------------------------------------------------
// Internal function                                                            
//------------------------------------------------------------------------------
// Function name:     LastSegDel                                                
//------------------------------------------------------------------------------
// Function call:     LastSegDel( hwnd, pTMMIda );                              
//------------------------------------------------------------------------------
// Description:       displays msg how to continue ...                          
//------------------------------------------------------------------------------
// Parameters:        _                                                         
//------------------------------------------------------------------------------
// Function flow:     _                                                         
//------------------------------------------------------------------------------
VOID  LastSegDel
(
   HWND            hwnd,
   PTMMIDA         pTMMIda
)
{
   USHORT          usResponse = 0;

   if ( !pTMMIda->fKill )
   {
     if ( !pTMMIda->pTMIda->fStop )
     {
       // display msg if next entries should be loaded or back to query
       usResponse = UtlError( WARNING_TMM_DEL_NEXT, MB_YESNO,
                              0, (PSZ *)NULP, EQF_QUERY );
       if ( usResponse == MBID_YES )
       {
         NextPrevious( pTMMIda, hwnd, TRUE, FALSE );
       }
       else
       {
         BackToQuery( pTMMIda, hwnd );
       } /* endif */
     }
     else if ( pTMMIda->pTMIda->ulPrevPos > 0 )
     {
       // display msg if previous entries should be loaded or back to query
       usResponse = UtlError( WARNING_TMM_DEL_PREV, MB_YESNO,
                              0, (PSZ *)NULP, EQF_QUERY );
       if ( usResponse == MBID_YES )
       {
         NextPrevious( pTMMIda, hwnd, FALSE, TRUE );
       }
       else
       {
         BackToQuery( pTMMIda, hwnd );
       } /* endif */
     }
     else
     {
       // display msg that TM is empty and return to TM/2
       UtlError( WARNING_TMM_DEL_EMPTY_TM, MB_OK, 0, (PSZ *)NULP, EQF_WARNING );
       WinPostMsg( hwnd, WM_CLOSE, NULL, NULL );
     } /* endif */
   } /* endif */

   return;

} /* end of function LastSegDel */


//------------------------------------------------------------------------------
// Internal function                                                            
//------------------------------------------------------------------------------
// Function name:     SaveChangedSegs                                           
//------------------------------------------------------------------------------
// Function call:     SAveChangedSegs( pTMMIda );                               
//------------------------------------------------------------------------------
// Description:       after find&change: save changes in memory                 
//------------------------------------------------------------------------------
// Parameters:        PTMMIDA  pTMMIda                                          
//------------------------------------------------------------------------------
// Function flow:     start at ulSegNum= 1 in target file                       
//                    while not at end of target file                           
//                      if segment is bookmarked ( == changed)                  
//                         get corresponding src seg                            
//                         truncate lf's at end of src & tgt                    
//                         fill src&tgt in memory                               
//------------------------------------------------------------------------------
static VOID          SaveChangedSegs
(
  PTMMIDA        pTMMIda
  )
{
   ULONG        ulCurSeg;
   ULONG        ulSegNum;
   PTBDOCUMENT  pTargetDoc, pSourceDoc;
   PTMEXTRAS    pTMParms;
   PTBSEGMENT   pSrcSeg, pTgtSeg;
   BOOL         fOK = TRUE;
   PSZ_W          pSrcData = NULL;
   PSZ_W          pTgtData = NULL;
   USHORT       usSrcNumLF, usTgtNumLF;
   ULONG        ulI;
   USHORT       usSrcLFAtEnd, usTgtLFAtEnd;


   pSourceDoc = &pTMMIda->TBSourceDoc;
   pTargetDoc = &pTMMIda->TBTargetDoc;
   /*******************************************************************/
   /* save all updates, independent   of workseg                      */
   /*******************************************************************/
//   pTargetDoc->EQFBFlags.workchng = FALSE;

   ulCurSeg = 1;
   pTgtSeg = EQFBGetSegW(pTargetDoc, ulCurSeg);
   while ( pTgtSeg && fOK  )
   {
     fOK = TRUE;
     if (pTgtSeg->SegFlags.Marked )
     {
       pSrcSeg = EQFBGetSegW(pSourceDoc, ulCurSeg);
       fOK =  EmptySegDataCheck(pTargetDoc,pTgtSeg);
       if (fOK && pSrcSeg)
       {
         /*************************************************************/
         /* readjust number of LF's in tgt segment                    */
         /* ( Find&change can del or add lf's )                       */
         /*************************************************************/
         usSrcNumLF = usTgtNumLF = 0;
         usSrcLFAtEnd = usTgtLFAtEnd = 0;
         pSrcData = pSrcSeg->pDataW;
         pTgtData = pTgtSeg->pDataW;
         if (pSrcData && pTgtData )
         {
           usTgtNumLF = TMMSegCountLF (pTgtData, &usTgtLFAtEnd );
           usSrcNumLF = TMMSegCountLF (pSrcData, &usSrcLFAtEnd );
           ulI = UTF16strlenCHAR(pSrcData) - usSrcLFAtEnd;
           *(pSrcData+ ulI) = EOS;
           ulI = UTF16strlenCHAR(pTgtData) - usTgtLFAtEnd;
           *(pTgtData+ ulI) = EOS;
           if ((usSrcNumLF - usSrcLFAtEnd) < (usTgtNumLF - usTgtLFAtEnd) )
           {
              //add missing lf's at src
              TMMAddLFsAtEnd ( pSourceDoc,
                               pSrcSeg,
                  ((usTgtNumLF - usTgtLFAtEnd)- (usSrcNumLF - usSrcLFAtEnd)));

           }
           else if ((usSrcNumLF - usSrcLFAtEnd) >
                              (usTgtNumLF - usTgtLFAtEnd) )
           {
             //add missing lf's at tgt
              TMMAddLFsAtEnd ( pTargetDoc,
                               pTgtSeg,
                 ((usSrcNumLF - usSrcLFAtEnd)-(usTgtNumLF - usTgtLFAtEnd)) );
           } /* endif */
         } /* endif */
       } /* endif */

       if ( fOK )
       {
         //get position in parameters structure
         ulSegNum = (ulCurSeg - 1) / 2;
         pTMParms = pTMMIda->pTMParms + ulSegNum;

         //update segment in tm and delete lf's added for display
         fOK  = TMMReplace( pTMMIda->pTMIda,
                            pSrcSeg->pDataW,     //source segment
                            pTgtSeg->pDataW,     //target segment
                            pTMMIda->pTMIda->szFileName,  //tm name
                            pTMMIda->pTMIda->pMemory,    //tm handle
                            pTMParms );         //seg no & doc name
       } /* endif */
        pTgtSeg->SegFlags.Marked = FALSE;
     } /* endif */

     ulCurSeg ++;
     pTgtSeg = EQFBGetSegW(pTargetDoc, ulCurSeg);
   } /* endwhile */

   return;

} /* end of function SaveChangedSegs */

//------------------------------------------------------------------------------
// Internal function                                                            
//------------------------------------------------------------------------------
// Function name:     EmptySegDataCheck                                         
//------------------------------------------------------------------------------
// Function call:     EmptySegDataCheck(pDoc, pSeg)                             
//------------------------------------------------------------------------------
// Description:       after find&change: check pSeg->pData whether it is empty  
//                    if so, insert NONE tag                                    
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT pDoc                                          
//                    PTBSEGMENT  pSeg                                          
//------------------------------------------------------------------------------
// Function flow:     check for non-whitespace character                        
//                    if postedit and segment is empty                          
//                      make space for empty tag and copy it in                 
//------------------------------------------------------------------------------
static
BOOL         EmptySegDataCheck
(
  PTBDOCUMENT    pDoc,
  PTBSEGMENT     pSeg
)
{
  PSZ_W    pData = pSeg->pDataW;
  CHAR_W   c;
  BOOL     fWHITESPfound = FALSE;        // TRUE if blank or LF found
  BOOL     fNonEmpty = FALSE;
  ULONG    ulResult;
  BOOL     fResult = TRUE;
  PSZ_W    pTmpBuf = NULL;


   // check for none white space character in pData
   while ( !fNonEmpty && ((c=*pData) != NULC))
   {
      switch ( c )
      {
         case LF:
         case BLANK:
            fWHITESPfound = TRUE;                              /* @KIT975A */
            break;
         default:
            fNonEmpty = TRUE;
            break;
      } /* endswitch */
      pData ++;
   } /* endwhile */


   if ( !fNonEmpty && pDoc->EQFBFlags.PostEdit)
   {
      /****************************************************************/
      /* in post edit fill in automaticly a :NONE tag                 */
      /****************************************************************/
      if ( !fWHITESPfound )                                  /* @KIT975A */
      {                                                      /* @KIT975A */
         // make space for EMPTY_TAG and copy it in
         ulResult = max(1 +  UTF16strlenCHAR(EMPTY_TAG), MIN_ALLOC);
         if (pSeg->ulSegNum != pDoc->ulWorkSeg )
         {
           pTmpBuf = pSeg->pDataW;
           fResult = UtlAlloc((PVOID *) (&pSeg->pDataW), 0L,
                              (LONG)ulResult* sizeof(CHAR_W), ERROR_STORAGE);
           if (fResult )
           {
             UTF16strcpy(pSeg->pDataW, EMPTY_TAG);
             if (pTmpBuf )
             {
               UtlAlloc((PVOID *)&pTmpBuf, 0L, 0L, NOMSG);
             } /* endif */
           } /* endif */
         }
         else
         {
           memcpy(pDoc->pEQFBWorkSegmentW, EMPTY_TAG, ulResult* sizeof(CHAR_W)); // copy empty tag

           pDoc->EQFBFlags.workchng = TRUE;    //  change in work seg
         } /* endif */
         EQFBCompSeg( pSeg );
         pDoc->Redraw |= REDRAW_ALL;
      } /* endif */                                          /* @KIT975A */
   } /* endif */

   return(fResult);
} /* end of function EmptySegDataCheck */


//------------------------------------------------------------------------------
// Internal function                                                            
//------------------------------------------------------------------------------
// Function name:     TMMSegCountLF                                             
//------------------------------------------------------------------------------
// Function call:     TMMSegCountLF(pData, pusLFAtEnd)                          
//------------------------------------------------------------------------------
// Description:       count all LF's in given segment                           
//------------------------------------------------------------------------------
// Parameters:        PSZ    pData                                              
//                    PUSHORT  pusLFAtEnd                                       
//------------------------------------------------------------------------------
// Returncode type:   USHORT                                                    
//------------------------------------------------------------------------------
// Returncodes:       usNumLF   total Number of LF's found                      
//------------------------------------------------------------------------------
// Function flow:     get ptr to segment data                                   
//                    while not at end of segment                               
//                      if current char is lf, increase counter                 
//                      else reset num of lf's at end to zero                   
//                    endwhile                                                  
//------------------------------------------------------------------------------
static
USHORT
TMMSegCountLF
(
    PSZ_W          pData,
    PUSHORT      pusLFAtEnd
)
{
  USHORT  usNumLF = 0;
  USHORT  usLFAtEnd = 0;

  while ( *pData != EOS )
  {
    if ( *pData == LF )
    {
      usNumLF++;
      usLFAtEnd++;
    }
    else
    {
      usLFAtEnd = 0;
    } /* endif */
    pData++;
  } /* endwhile */

  *pusLFAtEnd = usLFAtEnd;
  return (usNumLF);
} /* end of function TTMSegCountLF */

//------------------------------------------------------------------------------
// Internal function                                                            
//------------------------------------------------------------------------------
// Function name:     TMMAddLFsAtEnd                                            
//------------------------------------------------------------------------------
// Function call:     TMMAddLFsAtEnd(pDoc, pSeg, usCount)                       
//------------------------------------------------------------------------------
// Description:       add count many lf's at end of segment                     
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT pDoc                                          
//                    PTBSEGMENT  pSeg                                          
//                    USHORT  usCount                                           
//------------------------------------------------------------------------------
// Returncode type:   VOID                                                      
//------------------------------------------------------------------------------
// Function flow:     if seg is not worksegment                                 
//                      alloc new string                                        
//                    else set workchng flag                                    
//                    add count many lf's at end                                
//------------------------------------------------------------------------------
static
VOID
TMMAddLFsAtEnd
(
    PTBDOCUMENT  pDoc,
    PTBSEGMENT   pSeg,
    ULONG        ulCount
)
{
  PSZ_W    pTmpBuf;
  ULONG    ulLen;
  PSZ_W    pszString;
  BOOL     fResult = TRUE;

  // add missing LF characters
  ulLen = UTF16strlenCHAR(pSeg->pDataW) + ulCount + 1;
  ulLen = max(ulLen, MIN_ALLOC);
  if (pSeg->ulSegNum != pDoc->ulWorkSeg )
  {
    pTmpBuf = pSeg->pDataW;
    fResult = UtlAlloc((PVOID *) (&pSeg->pDataW), 0L,
                       (LONG)ulLen * sizeof(CHAR_W), ERROR_STORAGE);
    if (fResult )
    {
      UTF16strcpy(pSeg->pDataW, pTmpBuf);      // copy text
      if (pTmpBuf )
      {
        UtlAlloc((PVOID *)&pTmpBuf, 0L, 0L, NOMSG);
      } /* endif */
    } /* endif */
  }
  else
  {
    pDoc->EQFBFlags.workchng = TRUE;    //  change in work seg
  } /* endif */

  pszString = pSeg->pDataW;

  ulLen = UTF16strlenCHAR(pSeg->pDataW);
  while ( ulCount )
  {
    pszString[ulLen] = LF;
    ulLen++;
    ulCount--;
  } /* endwhile */
  pszString[ulLen] = EOS;


  EQFBCompSeg( pSeg );
  pDoc->Redraw |= REDRAW_ALL;

  return ;
} /* end of function TMMAddLFsAtEnd */

/**********************************************************************/
/* Function TMMTextFitsRect                                           */
/*                                                                    */
/* Check if the given text fits into the supplied rectangle           */
/**********************************************************************/
BOOL TMMTextFitsRect
(
  PSZ         pszText,                 // text being checked
  RECT        *prc,                    // ptr to rectangle for text
  HPS         hps                      // presentation space handle
)
{
  RECT    rc;                          // temporary rectangle
  DWORD   dwRGB_PaleGray = CLR_PALEGRAY;
  DWORD   dwRGB_BLACK = CLR_BLACK;

  memcpy( &rc, prc, sizeof(RECT) );
  rc.left  = 0;
  rc.right = 1024;

  if (UtlIsHighContrast())
  {
	  dwRGB_PaleGray = GetSysColor(COLOR_BTNFACE);
	  dwRGB_BLACK = GetSysColor(COLOR_BTNTEXT);
  }
  DRAWTEXT( hps, pszText, rc, dwRGB_BLACK, dwRGB_PaleGray, DT_CALCRECT );

  return( rc.right <= (prc->right - prc->left - 4) );
} /* end of function TMMTextFitsRect */

BOOL setMFlag(PTMMIDA pTMMIda, PTBDOCUMENT pDoc, USHORT usNewTranslationFlag) {
  ULONG         ulCurSeg;
  PTMEXTRAS     pTMParms;
  OtmProposal *pProposal = new OtmProposal();

  ulCurSeg = (pDoc->ulWorkSeg - 1) / 2;
  pTMParms = pTMMIda->pTMParms + ulCurSeg;
  pProposal->setInternalKey( pTMParms->szKey );
  switch ( usNewTranslationFlag )
  {
    case  TRANSLFLAG_NORMAL: pProposal->setType( OtmProposal::eptManual ); break;
    case  TRANSLFLAG_GLOBMEM: pProposal->setType( OtmProposal::eptGlobalMemory ); break;
    case  TRANSLFLAG_MACHINE: pProposal->setType( OtmProposal::eptMachine ); break;
    default: pProposal->setType( OtmProposal::eptManual ); break;
  } /* endswitch */
  pTMParms->usTranslationFlag = usNewTranslationFlag;
  return( pTMMIda->pTMIda->pMemory->updateProposal( *pProposal, OtmMemory::UPDATE_MTFLAG ) == 0);
}
